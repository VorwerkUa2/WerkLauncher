#include "JavaChecker.h"
#include "Logger.h"

#include <QDebug>
#include <QFile>
#include <QMap>
#include <QProcess>

#include "Application.h"
#include "Commandline.h"
#include "FileSystem.h"
#include "JavaUtils.h"

JavaChecker::JavaChecker(QObject *parent) : QObject(parent) {}

void JavaChecker::performCheck() {
  QString checkerJar =
      FS::PathCombine(APPLICATION->getJarsPath(), "JavaCheck.jar");

  QStringList args;

  process.reset(new QProcess());
  if (m_args.size()) {
    auto extraArgs = Commandline::splitArgs(m_args);
    args.append(extraArgs);
  }
  if (m_minMem != 0) {
    args << QString("-Xms%1m").arg(m_minMem);
  }
  if (m_maxMem != 0) {
    args << QString("-Xmx%1m").arg(m_maxMem);
  }
  if (m_permGen != 64) {
    args << QString("-XX:PermSize=%1m").arg(m_permGen);
  }

  args.append({"-jar", checkerJar});
  process->setArguments(args);
  process->setProgram(m_path);
  process->setProcessChannelMode(QProcess::SeparateChannels);
  process->setProcessEnvironment(CleanEnviroment());
  qCDebug(L_JAVA) << "Running java checker: " << m_path << args.join(" ");
  ;

  connect(process.get(), &QProcess::finished, this, &JavaChecker::finished);
  connect(process.get(), &QProcess::errorOccurred, this, &JavaChecker::error);
  connect(process.get(), &QProcess::readyReadStandardOutput, this,
          &JavaChecker::stdoutReady);
  connect(process.get(), &QProcess::readyReadStandardError, this,
          &JavaChecker::stderrReady);
  connect(&killTimer, &QTimer::timeout, this, &JavaChecker::timeout);
  killTimer.setSingleShot(true);
  qCDebug(L_JAVA) << "Starting Java checker process for path:" << m_path
                  << "with ID:" << m_id;
  killTimer.start(25000);
  process->start();
}

void JavaChecker::stdoutReady() {
  QByteArray data = process->readAllStandardOutput();
  QString added = QString::fromLocal8Bit(data);
  added.remove('\r');
  m_stdout += added;
}

void JavaChecker::stderrReady() {
  QByteArray data = process->readAllStandardError();
  QString added = QString::fromLocal8Bit(data);
  added.remove('\r');
  m_stderr += added;
}

void JavaChecker::finished(int exitcode, QProcess::ExitStatus status) {
  if (m_isFinished)
    return;
  m_isFinished = true;
  killTimer.stop();

  qCDebug(L_JAVA) << "Java checker finished for path:" << m_path
                  << "with ID:" << m_id << "status:" << status
                  << "exitCode:" << exitcode;

  QProcessPtr _process = process;
  process.reset();

  JavaCheckResult result;
  result.path = m_path;
  result.id = m_id;
  result.errorLog = m_stderr;
  result.outLog = m_stdout;
  qCDebug(L_JAVA) << "STDOUT" << m_stdout;
  qCWarning(L_JAVA) << "STDERR" << m_stderr;
  qCDebug(L_JAVA) << "Java checker finished with status " << status
                  << " exit code " << exitcode;

  if (status == QProcess::CrashExit || exitcode == 1) {
    result.validity = JavaCheckResult::Validity::Errored;
    emit checkFinished(result);
    return;
  }

  bool success = true;

  QMap<QString, QString> results;
  QStringList lines = m_stdout.split("\n", Qt::SkipEmptyParts);
  for (QString line : lines) {
    line = line.trimmed();
    // NOTE: workaround for GH-4125, where garbage is getting printed into
    // stdout on bedrock linux
    if (line.contains("/bedrock/strata")) {
      continue;
    }

    auto parts = line.split('=', Qt::SkipEmptyParts);
    if (parts.size() != 2 || parts[0].isEmpty() || parts[1].isEmpty()) {
      continue;
    } else {
      results.insert(parts[0], parts[1]);
    }
  }

  if (!results.contains("os.arch") || !results.contains("java.version") ||
      !results.contains("java.vendor") || !success) {
    result.validity = JavaCheckResult::Validity::ReturnedInvalidData;
    emit checkFinished(result);
    return;
  }

  auto os_arch = results["os.arch"];
  auto java_version = results["java.version"];
  auto java_vendor = results["java.vendor"];

  result.validity = JavaCheckResult::Validity::Valid;
  result.architecture = Sys::Architecture::fromOSArch(os_arch);
  result.javaVersion = java_version;
  result.javaVendor = java_vendor;
  qDebug() << "Java checker succeeded.";
  emit checkFinished(result);
}

void JavaChecker::error(QProcess::ProcessError err) {
  if (m_isFinished)
    return;
  m_isFinished = true;
  killTimer.stop();

  qDebug() << "Java checker error for path:" << m_path << "with ID:" << m_id
           << "error:" << err;

  if (err == QProcess::FailedToStart) {
    if (process) {
      qDebug() << "Process environment:";
      qDebug() << process->environment();
    }
    qDebug() << "Native environment:";
    qDebug() << QProcessEnvironment::systemEnvironment().toStringList();
  }

  JavaCheckResult result;
  result.path = m_path;
  result.id = m_id;
  emit checkFinished(result);
}

void JavaChecker::timeout() {
  if (m_isFinished)
    return;
  m_isFinished = true;

  if (process) {
    qDebug() << "Java checker killed by timeout for path:" << m_path
             << "with ID:" << m_id;
    process->kill();
  }

  JavaCheckResult result;
  result.path = m_path;
  result.id = m_id;
  emit checkFinished(result);
}
