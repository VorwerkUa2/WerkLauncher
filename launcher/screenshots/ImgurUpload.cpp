#include "ImgurUpload.h"
#include "BuildConfig.h"

#include <QDebug>
#include <QFile>
#include <QHttpMultiPart>
#include <QHttpPart>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkRequest>
#include <QUrl>

ImgurUpload::ImgurUpload(ScreenShot::Ptr shot) : NetAction(), m_shot(shot) {
  m_url = BuildConfig.IMGUR_BASE_URL + "upload.json";
  m_status = Job_NotStarted;
}

void ImgurUpload::startImpl() {
  finished = false;
  m_status = Job_InProgress;
  QNetworkRequest request(m_url);
  request.setHeader(QNetworkRequest::UserAgentHeader,
                    BuildConfig.USER_AGENT_UNCACHED);
  request.setRawHeader("Authorization", QString("Client-ID %1")
                                            .arg(BuildConfig.IMGUR_CLIENT_ID)
                                            .toStdString()
                                            .c_str());
  request.setRawHeader("Accept", "application/json");

  QFile f(m_shot->m_file.absoluteFilePath());
  if (!f.open(QFile::ReadOnly)) {
    emit failed(m_index_within_job);
    return;
  }

  QHttpMultiPart *multipart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
  QHttpPart filePart;
  filePart.setBody(f.readAll().toBase64());
  filePart.setHeader(QNetworkRequest::ContentTypeHeader, "image/png");
  filePart.setHeader(QNetworkRequest::ContentDispositionHeader,
                     "form-data; name=\"image\"");
  multipart->append(filePart);
  QHttpPart typePart;
  typePart.setHeader(QNetworkRequest::ContentDispositionHeader,
                     "form-data; name=\"type\"");
  typePart.setBody("base64");
  multipart->append(typePart);
  QHttpPart namePart;
  namePart.setHeader(QNetworkRequest::ContentDispositionHeader,
                     "form-data; name=\"name\"");
  namePart.setBody(m_shot->m_file.baseName().toUtf8());
  multipart->append(namePart);

  QNetworkReply *rep = m_network->post(request, multipart);

  if (m_reply) {
    m_reply->disconnect(this);
    m_reply->abort();
    m_reply->deleteLater();
  }
  m_reply = rep;
  connect(rep, &QNetworkReply::uploadProgress, this,
          &ImgurUpload::downloadProgress, Qt::UniqueConnection);
  connect(rep, &QNetworkReply::finished, this, &ImgurUpload::downloadFinished,
          Qt::UniqueConnection);
  connect(rep, SIGNAL(error(QNetworkReply::NetworkError)),
          SLOT(downloadError(QNetworkReply::NetworkError)),
          Qt::UniqueConnection);
}
void ImgurUpload::downloadError(QNetworkReply::NetworkError error) {
  if (sender() != m_reply)
    return;
  qCritical() << "ImgurUpload failed with error" << m_reply->errorString()
              << "Server reply:\n"
              << m_reply->readAll();
  if (finished) {
    qCritical() << "Double finished ImgurUpload!";
    return;
  }
  m_status = Job_Failed;
  finished = true;
  m_reply->deleteLater();
  m_reply = nullptr;
  emit failed(m_index_within_job);
}
void ImgurUpload::downloadFinished() {
  if (sender() != m_reply)
    return;
  if (finished) {
    qCritical() << "Double finished ImgurUpload!";
    return;
  }
  QByteArray data = m_reply->readAll();
  m_reply->deleteLater();
  m_reply = nullptr;
  QJsonParseError jsonError;
  QJsonDocument doc = QJsonDocument::fromJson(data, &jsonError);
  if (jsonError.error != QJsonParseError::NoError) {
    qDebug() << "imgur server did not reply with JSON"
             << jsonError.errorString();
    finished = true;
    emit failed(m_index_within_job);
    return;
  }
  auto object = doc.object();
  if (!object.value("success").toBool()) {
    qDebug() << "Screenshot upload not successful:" << doc.toJson();
    finished = true;
    emit failed(m_index_within_job);
    return;
  }
  m_shot->m_imgurId = object.value("data").toObject().value("id").toString();
  m_shot->m_url = object.value("data").toObject().value("link").toString();
  m_shot->m_imgurDeleteHash =
      object.value("data").toObject().value("deletehash").toString();
  m_status = Job_Finished;
  finished = true;
  emit succeeded(m_index_within_job);
  return;
}
void ImgurUpload::downloadProgress(qint64 bytesReceived, qint64 bytesTotal) {
  if (sender() != m_reply)
    return;
  m_total_progress = bytesTotal;
  m_progress = bytesReceived;
  emit netActionProgress(m_index_within_job, bytesReceived, bytesTotal);
}
