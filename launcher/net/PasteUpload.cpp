#include "PasteUpload.h"
#include "Application.h"
#include "BuildConfig.h"

#include <QDebug>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

PasteUpload::PasteUpload(QWidget *window, QString text, QString key)
    : m_window(window) {
  m_key = key;
  QByteArray temp;
  QJsonObject topLevelObj;
  QJsonObject sectionObject;
  sectionObject.insert("contents", text);
  QJsonArray sectionArray;
  sectionArray.append(sectionObject);
  topLevelObj.insert("description", "Log Upload");
  topLevelObj.insert("sections", sectionArray);
  QJsonDocument docOut;
  docOut.setObject(topLevelObj);
  m_jsonContent = docOut.toJson();
}

PasteUpload::~PasteUpload() {
  if (m_reply) {
    m_reply->disconnect(this);
    m_reply->abort();
    m_reply->deleteLater();
    m_reply = nullptr;
  }
}

bool PasteUpload::abort() {
  if (m_reply) {
    m_reply->disconnect(this);
    m_reply->abort();
    m_reply->deleteLater();
    m_reply = nullptr;
  }
  return Task::abort();
}

bool PasteUpload::validateText() { return m_jsonContent.size() <= maxSize(); }

void PasteUpload::executeTask() {
  QNetworkRequest request(QUrl("https://api.paste.ee/v1/pastes"));
  request.setHeader(QNetworkRequest::UserAgentHeader,
                    BuildConfig.USER_AGENT_UNCACHED);

  request.setRawHeader("Content-Type", "application/json");
  request.setRawHeader("Content-Length",
                       QByteArray::number(m_jsonContent.size()));
  request.setRawHeader("X-Auth-Token", m_key.toStdString().c_str());

  QNetworkReply *rep = APPLICATION->network()->post(request, m_jsonContent);

  m_reply = rep;
  setStatus(tr("Uploading to paste.ee"));
  connect(rep, &QNetworkReply::uploadProgress, this, &Task::setProgress,
          Qt::UniqueConnection);
  connect(rep, SIGNAL(error(QNetworkReply::NetworkError)), this,
          SLOT(downloadError(QNetworkReply::NetworkError)),
          Qt::UniqueConnection);
  connect(rep, SIGNAL(finished()), this, SLOT(downloadFinished()),
          Qt::UniqueConnection);
}

void PasteUpload::downloadError(QNetworkReply::NetworkError error) {
  // error happened during download.
  qCritical() << "Network error: " << error;
  emitFailed(m_reply->errorString());
}

void PasteUpload::downloadFinished() {
  QByteArray data = m_reply->readAll();
  // if the download succeeded
  if (m_reply->error() == QNetworkReply::NetworkError::NoError) {
    m_reply->deleteLater();
    m_reply = nullptr;
    QJsonParseError jsonError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &jsonError);
    if (jsonError.error != QJsonParseError::NoError) {
      emitFailed(jsonError.errorString());
      return;
    }
    if (!parseResult(doc)) {
      emitFailed(tr("paste.ee returned an error. Please consult the logs for "
                    "more information"));
      return;
    }
  } else {
    emitFailed(QString("Network error: %1").arg(m_reply->errorString()));
    m_reply->deleteLater();
    m_reply = nullptr;
    return;
  }
  emitSucceeded();
}

bool PasteUpload::parseResult(QJsonDocument doc) {
  auto object = doc.object();
  auto status = object.value("success").toBool();
  if (!status) {
    qCritical() << "paste.ee reported error:"
                << QString(object.value("error").toString());
    return false;
  }
  m_pasteLink = object.value("link").toString();
  m_pasteID = object.value("id").toString();
  qDebug() << m_pasteLink;
  return true;
}
