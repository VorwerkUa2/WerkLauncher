#include "YggdrasilProfileStep.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkRequest>

#include "minecraft/auth/AuthRequest.h"
#include "minecraft/auth/Parsers.h"

YggdrasilProfileStep::YggdrasilProfileStep(AccountData *data)
    : AuthStep(data) {}

YggdrasilProfileStep::~YggdrasilProfileStep() noexcept = default;

QString YggdrasilProfileStep::describe() {
  return tr("Fetching Yggdrasil profile...");
}

void YggdrasilProfileStep::perform() {
  QString syncUrl = m_data->syncUrl;
  if (syncUrl.isEmpty()) {
    syncUrl = "https://vorwerkua.alwaysdata.net/api/yggdrasil";
    qDebug() << "[MSSS_DIAG] syncUrl is empty, using fallback:" << syncUrl;
  }

  if (m_data->minecraftProfile.id.isEmpty()) {
    qWarning() << "[MSSS_DIAG] Profile ID is empty, cannot fetch profile.";
    emit finished(AccountTaskState::STATE_WORKING, "");
    return;
  }

  QString urlStr = syncUrl;
  if (urlStr.endsWith("/authenticate")) {
    urlStr.chop(13);
  }
  if (urlStr.endsWith("/minecraft/profile/skins")) {
    urlStr.chop(24);
  }
  // ensure no trailing slash
  while (urlStr.endsWith("/"))
    urlStr.chop(1);

  QUrl url(urlStr + "/sessionserver/session/minecraft/profile/" +
           m_data->minecraftProfile.id);

  QNetworkRequest request = QNetworkRequest(url);
  // Yggdrasil profile endpoint shouldn't strictly require auth, but we can pass
  // it
  request.setRawHeader(
      "Authorization",
      QString("Bearer %1").arg(m_data->yggdrasilToken.token).toUtf8());

  AuthRequest *requestor = new AuthRequest(this);
  connect(requestor, &AuthRequest::finished, this,
          &YggdrasilProfileStep::onRequestDone);
  requestor->get(request, 10000);
}

void YggdrasilProfileStep::onRequestDone(QNetworkReply::NetworkError error,
                                         QByteArray data,
                                         QList<QNetworkReply::RawHeaderPair>) {
  auto requestor = qobject_cast<AuthRequest *>(QObject::sender());
  requestor->deleteLater();

  if (error != QNetworkReply::NoError) {
    if (!m_data->minecraftProfile.id.isEmpty()) {
      // Offline fallback: already have profile ID, so we can proceed.
      emit finished(AccountTaskState::STATE_WORKING, "");
      return;
    }
    emit finished(AccountTaskState::STATE_FAILED_SOFT,
                  tr("Failed to fetch Yggdrasil profile: %1")
                      .arg(requestor->errorString_));
    return;
  }

  QJsonDocument doc = QJsonDocument::fromJson(data);
  QJsonObject root = doc.object();

  // Yggdrasil profile format: { "id": "...", "name": "...", "properties": [ {
  // "name": "textures", "value": "base64..." } ] }
  QJsonArray properties = root.value("properties").toArray();
  for (auto propVal : properties) {
    QJsonObject prop = propVal.toObject();
    if (prop.value("name").toString() == "textures") {
      QString b64 = prop.value("value").toString();
      qDebug() << "[MSSS_DIAG] Found textures property, decoding base64...";
      QByteArray decoded = QByteArray::fromBase64(b64.toUtf8());
      QJsonDocument texDoc = QJsonDocument::fromJson(decoded);
      QJsonObject texRoot = texDoc.object();
      QJsonObject textures = texRoot.value("textures").toObject();
      QJsonObject skin = textures.value("SKIN").toObject();
      if (!skin.isEmpty()) {
        QString skinUrl = skin.value("url").toString();
        qDebug() << "[MSSS_DIAG] Extracted skin URL:" << skinUrl;
        if (!skinUrl.isEmpty()) {
          m_data->minecraftProfile.skin.url = skinUrl;

          // Parse variant (SLIM or CLASSIC)
          QJsonObject metadata = skin.value("metadata").toObject();
          QString model = metadata.value("model").toString();
          qDebug() << "[MSSS_DIAG] Model from metadata:" << model;
          if (model == "slim") {
            m_data->minecraftProfile.skin.variant = "SLIM";
          } else {
            m_data->minecraftProfile.skin.variant = "CLASSIC";
          }

          // Proactive load from backup if available
          QFile backup("local_skin_backup.png");
          if (backup.exists() && backup.size() > 0) {
            qDebug() << "[MSSS_DIAG] Found local_skin_backup.png ("
                     << backup.size() << " bytes), loading...";
            if (backup.open(QFile::ReadOnly)) {
              m_data->minecraftProfile.skin.data = backup.readAll();
              backup.close();
              qDebug() << "[MSSS_DIAG] Loaded"
                       << m_data->minecraftProfile.skin.data.size()
                       << "bytes from local backup.";
            }
          } else if (backup.exists()) {
            qWarning() << "[MSSS_DIAG] Found local_skin_backup.png but it is "
                          "EMPTY. Ignoring.";
          }
        }
      } else {
        qWarning() << "[MSSS_DIAG] SKIN object is empty in textures property!";
      }
      break;
    }
  }

  emit finished(AccountTaskState::STATE_WORKING, "");
}
