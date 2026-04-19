#pragma once

#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLocalSocket>
#include <QObject>
#include <QTimer>

class DiscordRPC : public QObject {
  Q_OBJECT
public:
  explicit DiscordRPC(QObject *parent = nullptr);
  ~DiscordRPC();

  void setActivity(const QString &details, const QString &state,
                   bool startTimer = true);
  void clearActivity();
  void setEnabled(bool enabled);

private slots:
  void onConnected();
  void onDisconnected();
  void onError(QLocalSocket::LocalSocketError error);
  void onReadyRead();
  void sendHandshake();
  void reconnect();

private:
  void sendPayload(int opcode, const QJsonObject &object);
  QString getPipePath(int index);

  QLocalSocket *m_socket = nullptr;
  QTimer *m_reconnectTimer = nullptr;
  bool m_enabled = true;
  bool m_connected = false;
  bool m_handshakeComplete = false;
  QString m_clientId = "1476261502846631956";
  QDateTime m_startTime;

  // Pending activity (queued before handshake completes)
  bool m_hasPending = false;
  QString m_pendingDetails;
  QString m_pendingState;
  bool m_pendingStartTimer = true;
};
