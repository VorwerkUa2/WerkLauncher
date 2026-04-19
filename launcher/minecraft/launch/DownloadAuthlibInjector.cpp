#include "DownloadAuthlibInjector.h"
#include <Application.h>
#include <QDir>
#include <QFile>
#include <launch/LaunchTask.h>
#include <minecraft/MinecraftInstance.h>
#include <net/Download.h>

DownloadAuthlibInjector::DownloadAuthlibInjector(LaunchTask *parent,
                                                 AuthSessionPtr session)
    : LaunchStep(parent), m_session(session) {}

void DownloadAuthlibInjector::executeTask() {
  if (!m_session || m_session->sync_url.isEmpty()) {
    emitSucceeded();
    return;
  }

  auto instance = m_parent->instance();
  auto minecraftInstance =
      std::dynamic_pointer_cast<MinecraftInstance>(instance);
  if (!minecraftInstance) {
    emitSucceeded();
    return;
  }

  QString binRoot = minecraftInstance->binRoot();
  QString injectorPath = QDir(binRoot).absoluteFilePath("authlib-injector.jar");

  if (QFile::exists(injectorPath)) {
    emitSucceeded();
    return;
  }

  // Ensure bin directory exists
  QDir().mkpath(binRoot);

  // Use a reliable GitHub releases link for version 1.2.5
  QUrl url("https://github.com/yushijinhun/authlib-injector/releases/download/"
           "v1.2.5/authlib-injector-1.2.5.jar");

  emit logLine(
      QString("Downloading authlib-injector from %1 ...").arg(url.toString()),
      MessageLevel::Launcher);

  m_job = new NetJob("Download authlib-injector", APPLICATION->network());
  m_job->addNetAction(Net::Download::makeFile(url, injectorPath));

  connect(m_job.get(), &NetJob::succeeded, this,
          &DownloadAuthlibInjector::downloadSucceeded);
  connect(m_job.get(), &NetJob::failed, this,
          &DownloadAuthlibInjector::downloadFailed);

  m_job->start();
}

void DownloadAuthlibInjector::downloadSucceeded() {
  emit logLine("Authlib-injector downloaded successfully.",
               MessageLevel::Launcher);
  m_job.reset();
  emitSucceeded();
}

void DownloadAuthlibInjector::downloadFailed(QString reason) {
  emit logLine("Failed to download authlib-injector: " + reason,
               MessageLevel::Warning);
  emit logLine("Skins might not work in-game.", MessageLevel::Warning);
  m_job.reset();
  // We don't fail the whole launch task because of this,
  // as the game can still run without skins.
  emitSucceeded();
}

bool DownloadAuthlibInjector::abort() {
  if (m_job) {
    return m_job->abort();
  }
  return true;
}
