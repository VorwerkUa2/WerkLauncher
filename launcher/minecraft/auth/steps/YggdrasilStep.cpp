/* Copyright 2025 MultiMC Contributors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "YggdrasilStep.h"
#include "minecraft/auth/AuthRequest.h"
#include "minecraft/auth/Parsers.h"
#include <QFile>
#include <QHttpMultiPart>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkRequest>

// YggdrasilLoginStep
YggdrasilLoginStep::YggdrasilLoginStep(AccountData *data) : AuthStep(data) {}
YggdrasilLoginStep::~YggdrasilLoginStep() noexcept = default;

QString YggdrasilLoginStep::describe() {
  return tr("Logging in to skin synchronization service...");
}

void YggdrasilLoginStep::perform() {
  QString syncUrl = m_data->syncUrl;
  if (syncUrl.isEmpty()) {
    syncUrl = "https://vorwerkua.alwaysdata.net/api/yggdrasil";
    qDebug() << "[MSSS_DIAG] Login syncUrl is empty, using fallback:"
             << syncUrl;
  }

  QUrl url(syncUrl);
  if (url.scheme().isEmpty())
    url.setScheme("https");
  url.setPath(url.path() + "/authenticate");

  QJsonObject agent;
  agent["name"] = "Minecraft";
  agent["version"] = 1;

  QJsonObject body;
  body["agent"] = agent;
  QString username = m_data->syncUsername;
  if (username.isEmpty()) {
    username = m_data->minecraftProfile.name;
  }
  username = username.toLower(); // Server normalizes to lowercase
  body["username"] = username;
  body["password"] = m_data->syncPassword;
  body["requestUser"] = true;

  QNetworkRequest request(url);
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

  AuthRequest *requestor = new AuthRequest(this);
  connect(requestor, &AuthRequest::finished, this,
          &YggdrasilLoginStep::onRequestDone);
  requestor->post(request, QJsonDocument(body).toJson(), 10000);
}

void YggdrasilLoginStep::onRequestDone(QNetworkReply::NetworkError error,
                                       QByteArray data,
                                       QList<QNetworkReply::RawHeaderPair>) {
  auto requestor = qobject_cast<AuthRequest *>(QObject::sender());
  requestor->deleteLater();

  if (error != QNetworkReply::NoError) {
    if (!m_data->minecraftProfile.id.isEmpty()) {
      // Fallback: If we have no internet but we have a profile ID, use a dummy
      // token and proceed. The game will use its internal cache for textures.
      m_data->yggdrasilToken.token = "offline_fallback_token";
      m_data->yggdrasilToken.validity = Katabasis::Validity::Certain;
      emit finished(AccountTaskState::STATE_WORKING, "");
      return;
    }

    QString errorMsg;
    if (error == QNetworkReply::ContentAccessDenied) {
      errorMsg = tr("Invalid nickname or password.");
    } else {
      errorMsg = tr("Failed to authenticate with skin service: %1")
                     .arg(requestor->errorString_);
    }
    emit finished(AccountTaskState::STATE_FAILED_SOFT, errorMsg);
    return;
  }

  QJsonDocument doc = QJsonDocument::fromJson(data);
  QJsonObject root = doc.object();
  QString token = root.value("accessToken").toString();
  if (token.isEmpty()) {
    emit finished(AccountTaskState::STATE_FAILED_SOFT,
                  tr("Invalid nickname or password. Please check your "
                     "credentials and try again."));
    return;
  }

  m_data->yggdrasilToken.token = token;
  m_data->yggdrasilToken.validity = Katabasis::Validity::Certain;

  // Store profile info if returned (standard for /authenticate)
  QJsonObject selectedProfile = root.value("selectedProfile").toObject();
  if (!selectedProfile.isEmpty()) {
    QString profileId = selectedProfile.value("id").toString();
    QString profileName = selectedProfile.value("name").toString();
    if (!profileId.isEmpty()) {
      m_data->minecraftProfile.id = profileId;
    }
    if (!profileName.isEmpty()) {
      m_data->minecraftProfile.name = profileName;
    }

    // Immediate skin sync from properties
    QJsonArray properties = selectedProfile.value("properties").toArray();
    for (auto propVal : properties) {
      QJsonObject prop = propVal.toObject();
      if (prop.value("name").toString() == "textures") {
        QString b64 = prop.value("value").toString();
        QByteArray decoded = QByteArray::fromBase64(b64.toUtf8());
        QJsonDocument texDoc = QJsonDocument::fromJson(decoded);
        QJsonObject texRoot = texDoc.object();
        QJsonObject textures = texRoot.value("textures").toObject();
        QJsonObject skin = textures.value("SKIN").toObject();
        if (!skin.isEmpty()) {
          QString skinUrl = skin.value("url").toString();
          if (!skinUrl.isEmpty()) {
            m_data->minecraftProfile.skin.url = skinUrl;
            QJsonObject metadata = skin.value("metadata").toObject();
            QString model = metadata.value("model").toString();
            m_data->minecraftProfile.skin.variant =
                (model == "slim" ? "SLIM" : "CLASSIC");

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
        }
        break;
      }
    }
  }

  emit finished(AccountTaskState::STATE_WORKING, "");
}

// YggdrasilSetSkinStep
YggdrasilSetSkinStep::YggdrasilSetSkinStep(AccountData *data,
                                           Skins::Model model,
                                           QByteArray skinData)
    : AuthStep(data), m_model(model), m_skinData(skinData) {}
YggdrasilSetSkinStep::~YggdrasilSetSkinStep() noexcept = default;

QString YggdrasilSetSkinStep::describe() {
  return tr("Uploading skin to synchronization service...");
}

void YggdrasilSetSkinStep::perform() {
  QString syncUrl = m_data->syncUrl;
  if (syncUrl.isEmpty()) {
    syncUrl = "https://vorwerkua.alwaysdata.net/api/yggdrasil";
    qDebug() << "[MSSS_DIAG] SetSkin syncUrl is empty, using fallback:"
             << syncUrl;
  }

  if (m_data->yggdrasilToken.token.isEmpty()) {
    qWarning() << "[MSSS_DIAG] No token, skipping skin upload.";
    emit finished(AccountTaskState::STATE_WORKING, "");
    return;
  }

  QUrl url(syncUrl);
  if (url.scheme().isEmpty())
    url.setScheme("https");
  url.setPath(url.path() + "/minecraft/profile/skins");

  QNetworkRequest request(url);
  request.setRawHeader(
      "Authorization",
      QString("Bearer %1").arg(m_data->yggdrasilToken.token).toUtf8());

  if (m_skinData.isEmpty()) {
    AuthRequest *requestor = new AuthRequest(this);
    connect(requestor, &AuthRequest::finished, this,
            &YggdrasilSetSkinStep::onRequestDone);
    requestor->deleteResource(request);
  } else {
    QHttpMultiPart *multiPart =
        new QHttpMultiPart(QHttpMultiPart::FormDataType);
    QHttpPart skin;
    skin.setHeader(QNetworkRequest::ContentTypeHeader, "image/png");
    skin.setHeader(QNetworkRequest::ContentDispositionHeader,
                   "form-data; name=\"file\"; filename=\"skin.png\"");
    skin.setBody(m_skinData);

    QHttpPart model;
    model.setHeader(QNetworkRequest::ContentDispositionHeader,
                    "form-data; name=\"variant\"");
    model.setBody(m_model == Skins::Model::Slim ? "SLIM" : "CLASSIC");

    multiPart->append(skin);
    multiPart->append(model);

    AuthRequest *requestor = new AuthRequest(this);
    multiPart->setParent(requestor);
    connect(requestor, &AuthRequest::finished, this,
            &YggdrasilSetSkinStep::onRequestDone);
    requestor->post(request, multiPart);
  }
}

void YggdrasilSetSkinStep::onRequestDone(QNetworkReply::NetworkError error,
                                         QByteArray data,
                                         QList<QNetworkReply::RawHeaderPair>) {
  auto requestor = qobject_cast<AuthRequest *>(QObject::sender());
  requestor->deleteLater();

  if (error != QNetworkReply::NoError) {
    auto parsedError = MojangError::fromJSON(data, error);
    emit finished(AccountTaskState::STATE_FAILED_SOFT,
                  tr("Failed to upload skin: %1").arg(parsedError.toString()));
    return;
  }

  // Proactively update local data so the UI reflects the change immediately
  m_data->minecraftProfile.skin.data = m_skinData;
  m_data->minecraftProfile.skin.variant =
      (m_model == Skins::Model::Slim ? "SLIM" : "CLASSIC");

  // Save a local backup for offline use
  if (!m_skinData.isEmpty()) {
    QFile backup("local_skin_backup.png");
    if (backup.open(QFile::WriteOnly)) {
      backup.write(m_skinData);
      backup.close();
    }
  }

  emit finished(AccountTaskState::STATE_WORKING, "");
}
