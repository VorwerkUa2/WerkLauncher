#include "ImgurAlbumCreation.h"

#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkRequest>
#include <QStringList>
#include <QUrl>

#include "Application.h"
#include "BuildConfig.h"

ImgurAlbumCreation::ImgurAlbumCreation(QList<ScreenShot::Ptr> screenshots)
    : NetAction(), m_screenshots(screenshots) {
  m_url = BuildConfig.IMGUR_BASE_URL + "album.json";
  m_status = Job_NotStarted;
}

void ImgurAlbumCreation::startImpl() {
  m_status = Job_InProgress;
  QNetworkRequest request(m_url);
  request.setHeader(QNetworkRequest::UserAgentHeader,
                    BuildConfig.USER_AGENT_UNCACHED);
  request.setHeader(QNetworkRequest::ContentTypeHeader,
                    "application/x-www-form-urlencoded");
  request.setRawHeader("Authorization", QString("Client-ID %1")
                                            .arg(BuildConfig.IMGUR_CLIENT_ID)
                                            .toStdString()
                                            .c_str());
  request.setRawHeader("Accept", "application/json");

  QStringList hashes;
  for (auto shot : m_screenshots) {
    hashes.append(shot->m_imgurDeleteHash);
  }

  const QByteArray data = "deletehashes=" + hashes.join(',').toUtf8() +
                          "&title=Minecraft%20Screenshots&privacy=hidden";

  QNetworkReply *rep = APPLICATION->network()->post(request, data);

  if (m_reply) {
    m_reply->disconnect(this);
    m_reply->abort();
    m_reply->deleteLater();
  }
  m_reply = rep;
  connect(rep, &QNetworkReply::uploadProgress, this,
          &ImgurAlbumCreation::downloadProgress, Qt::UniqueConnection);
  connect(rep, &QNetworkReply::finished, this,
          &ImgurAlbumCreation::downloadFinished, Qt::UniqueConnection);
  connect(rep, SIGNAL(error(QNetworkReply::NetworkError)),
          SLOT(downloadError(QNetworkReply::NetworkError)),
          Qt::UniqueConnection);
}
void ImgurAlbumCreation::downloadError(QNetworkReply::NetworkError error) {
  if (sender() != m_reply)
    return;
  qDebug() << m_reply->errorString();
  m_status = Job_Failed;
  emit failed(m_index_within_job);
}
void ImgurAlbumCreation::downloadFinished() {
  if (sender() != m_reply)
    return;
  if (m_status != Job_Failed) {
    QByteArray data = m_reply->readAll();
    m_reply->deleteLater();
    m_reply = nullptr;
    QJsonParseError jsonError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &jsonError);
    if (jsonError.error != QJsonParseError::NoError) {
      qDebug() << jsonError.errorString();
      emit failed(m_index_within_job);
      return;
    }
    auto object = doc.object();
    if (!object.value("success").toBool()) {
      qDebug() << doc.toJson();
      emit failed(m_index_within_job);
      return;
    }
    m_deleteHash =
        object.value("data").toObject().value("deletehash").toString();
    m_id = object.value("data").toObject().value("id").toString();
    m_status = Job_Finished;
    emit succeeded(m_index_within_job);
    return;
  } else {
    qDebug() << m_reply->readAll();
    m_reply->deleteLater();
    m_reply = nullptr;
    emit failed(m_index_within_job);
    return;
  }
}
void ImgurAlbumCreation::downloadProgress(qint64 bytesReceived,
                                          qint64 bytesTotal) {
  if (sender() != m_reply)
    return;
  m_total_progress = bytesTotal;
  m_progress = bytesReceived;
  emit netActionProgress(m_index_within_job, bytesReceived, bytesTotal);
}
