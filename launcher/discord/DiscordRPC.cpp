#include "DiscordRPC.h"
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#ifndef Q_OS_WIN
#include <unistd.h>
#endif
DiscordRPC::DiscordRPC(QObject *parent) : QObject(parent) {
  m_socket = new QLocalSocket(this);
  m_reconnectTimer = new QTimer(this);
  m_reconnectTimer->setInterval(5000);

  connect(m_socket, &QLocalSocket::connected, this, &DiscordRPC::onConnected);
  connect(m_socket, &QLocalSocket::disconnected, this,
          &DiscordRPC::onDisconnected);
  connect(m_socket, &QLocalSocket::errorOccurred, this, &DiscordRPC::onError);
  connect(m_socket, &QLocalSocket::readyRead, this, &DiscordRPC::onReadyRead);
  connect(m_reconnectTimer, &QTimer::timeout, this, &DiscordRPC::reconnect);

  reconnect();
}

DiscordRPC::~DiscordRPC() {
  if (m_connected) {
    clearActivity();
    m_socket->flush();
    m_socket->waitForBytesWritten(500);
  }
  m_socket->disconnectFromServer();
}

void DiscordRPC::setEnabled(bool enabled) {
  m_enabled = enabled;
  if (!m_enabled) {
    clearActivity();
    m_socket->disconnectFromServer();
    m_reconnectTimer->stop();
    m_connected = false;
    m_handshakeComplete = false;
  } else {
    reconnect();
  }
}

void DiscordRPC::reconnect() {
  if (!m_enabled || m_connected ||
      m_socket->state() != QLocalSocket::UnconnectedState) {
    return;
  }

  m_handshakeComplete = false;

  // Non-blocking: try one pipe at a time, rely on connected/error signals
  if (m_currentPipeIndex >= 10) {
    m_currentPipeIndex = 0;
    qDebug() << "Discord RPC: exhausted all pipes, retrying in 5s";
    if (!m_reconnectTimer->isActive()) {
      m_reconnectTimer->start();
    }
    return;
  }

  QString path = getPipePath(m_currentPipeIndex);
  qDebug() << "Discord RPC: trying pipe" << path;
  m_currentPipeIndex++;
  m_socket->connectToServer(path);
  // If connection fails, onError() will fire and schedule next attempt
}

QString DiscordRPC::getPipePath(int index) {
#ifdef Q_OS_WIN
  return QString(R"(\\.\pipe\discord-ipc-%1)").arg(index);
#else
  QString tempDir = QDir::tempPath();
  QStringList commonPaths = {tempDir, "/run/user/" + QString::number(getuid()),
                             "/var/run"};
  for (const auto &base : commonPaths) {
    QString path = base + "/discord-ipc-" + QString::number(index);
    if (QFile::exists(path))
      return path;
  }
  return tempDir + "/discord-ipc-" + QString::number(index);
#endif
}

void DiscordRPC::onConnected() {
  m_connected = true;
  m_currentPipeIndex = 0;
  m_reconnectTimer->stop();
  qDebug() << "Discord RPC: pipe connected, sending handshake";
  sendHandshake();
}

void DiscordRPC::onDisconnected() {
  m_connected = false;
  m_handshakeComplete = false;
  qDebug() << "Discord RPC: disconnected";
  if (m_enabled) {
    m_reconnectTimer->start();
  }
}

void DiscordRPC::onError(QLocalSocket::LocalSocketError error) {
  Q_UNUSED(error);
  m_connected = false;
  m_handshakeComplete = false;
  if (m_enabled) {
    // Try next pipe immediately (non-blocking)
    QTimer::singleShot(0, this, &DiscordRPC::reconnect);
  }
}

void DiscordRPC::sendHandshake() {
  QJsonObject obj;
  obj["v"] = 1;
  obj["client_id"] = m_clientId;

  qDebug() << "Discord RPC: handshake payload:"
           << QJsonDocument(obj).toJson(QJsonDocument::Compact);
  sendPayload(0, obj);
}

void DiscordRPC::sendPayload(int opcode, const QJsonObject &object) {
  if (m_socket->state() != QLocalSocket::ConnectedState)
    return;

  QByteArray data = QJsonDocument(object).toJson(QJsonDocument::Compact);
  quint32 length = static_cast<quint32>(data.size());

  QByteArray header(8, '\0');
  // Little-endian opcode
  header[0] = static_cast<char>((opcode >> 0) & 0xFF);
  header[1] = static_cast<char>((opcode >> 8) & 0xFF);
  header[2] = static_cast<char>((opcode >> 16) & 0xFF);
  header[3] = static_cast<char>((opcode >> 24) & 0xFF);
  // Little-endian length
  header[4] = static_cast<char>((length >> 0) & 0xFF);
  header[5] = static_cast<char>((length >> 8) & 0xFF);
  header[6] = static_cast<char>((length >> 16) & 0xFF);
  header[7] = static_cast<char>((length >> 24) & 0xFF);

  m_socket->write(header);
  m_socket->write(data);
  m_socket->flush();
}

void DiscordRPC::setActivity(const QString &details, const QString &state,
                             bool startTimer) {
  if (!m_connected || !m_handshakeComplete) {
    // Queue the activity for when handshake completes
    m_pendingDetails = details;
    m_pendingState = state;
    m_pendingStartTimer = startTimer;
    m_hasPending = true;
    return;
  }

  if (startTimer) {
    m_startTime = QDateTime::currentDateTime();
  }

  QJsonObject activity;
  activity["details"] = details;
  activity["state"] = state;

  QJsonObject assets;
  assets["large_image"] = "werklauncher";
  assets["large_text"] = "WerkLauncher";
  activity["assets"] = assets;

  if (startTimer) {
    QJsonObject timestamps;
    timestamps["start"] = m_startTime.toSecsSinceEpoch();
    activity["timestamps"] = timestamps;
  }

  QJsonObject args;
  args["pid"] = static_cast<qint64>(QCoreApplication::applicationPid());
  args["activity"] = activity;

  QJsonObject payload;
  payload["cmd"] = "SET_ACTIVITY";
  payload["args"] = args;
  payload["nonce"] = QString::number(QDateTime::currentMSecsSinceEpoch());

  qDebug() << "Discord RPC: SET_ACTIVITY" << details << state;
  sendPayload(1, payload);
}

void DiscordRPC::clearActivity() {
  if (!m_connected || !m_handshakeComplete)
    return;

  QJsonObject args;
  args["pid"] = static_cast<qint64>(QCoreApplication::applicationPid());
  args["activity"] = QJsonValue::Null;

  QJsonObject payload;
  payload["cmd"] = "SET_ACTIVITY";
  payload["args"] = args;
  payload["nonce"] = QString::number(QDateTime::currentMSecsSinceEpoch());

  sendPayload(1, payload);
}

void DiscordRPC::onReadyRead() {
  QByteArray buf = m_socket->readAll();
  qDebug() << "Discord RPC: received" << buf.size() << "bytes";

  if (!m_handshakeComplete && buf.size() >= 8) {
    // Parse the opcode from the first 4 bytes (little-endian)
    quint32 opcode = static_cast<quint8>(buf[0]) |
                     (static_cast<quint8>(buf[1]) << 8) |
                     (static_cast<quint8>(buf[2]) << 16) |
                     (static_cast<quint8>(buf[3]) << 24);

    if (opcode == 1) { // FRAME opcode = handshake acknowledged
      m_handshakeComplete = true;
      qDebug() << "Discord RPC: handshake complete!";

      // Send any pending activity
      if (m_hasPending) {
        m_hasPending = false;
        setActivity(m_pendingDetails, m_pendingState, m_pendingStartTimer);
      }
    }
  }
}
