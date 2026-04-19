/*
 * Copyright 2023 arthomnix
 *
 * This source is subject to the Microsoft Public License (MS-PL).
 * Please see the COPYING.md file for more information.
 */

#include "ModrinthHashLookupRequest.h"
#include "BuildConfig.h"
#include "Json.h"
#include <QJsonArray>
#include <QJsonDocument>

namespace Modrinth {

HashLookupRequest::HashLookupRequest(QList<HashLookupData> hashes,
                                     QList<HashLookupResponseData> *output)
    : NetAction(), m_hashes(hashes), m_output(output) {
  m_url = "https://api.modrinth.com/v2/version_files";
  m_status = Job_NotStarted;
}

void HashLookupRequest::startImpl() {
  finished = false;
  m_status = Job_InProgress;

  QNetworkRequest request(m_url);
  request.setHeader(QNetworkRequest::UserAgentHeader,
                    BuildConfig.USER_AGENT_UNCACHED);
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

  QJsonObject requestObject;
  QJsonArray hashes;

  for (const auto &data : m_hashes) {
    hashes.append(data.hash);
  }

  requestObject.insert("hashes", hashes);
  requestObject.insert("algorithm", QJsonValue("sha512"));

  QNetworkReply *rep =
      m_network->post(request, QJsonDocument(requestObject).toJson());
  if (m_reply) {
    m_reply->disconnect(this);
    m_reply->abort();
    m_reply->deleteLater();
  }
  m_reply = rep;
  connect(rep, &QNetworkReply::uploadProgress, this,
          &HashLookupRequest::downloadProgress, Qt::UniqueConnection);
  connect(rep, &QNetworkReply::finished, this,
          &HashLookupRequest::downloadFinished, Qt::UniqueConnection);
  connect(rep, SIGNAL(error(QNetworkReply::NetworkError)),
          SLOT(downloadError(QNetworkReply::NetworkError)),
          Qt::UniqueConnection);
}

void HashLookupRequest::downloadError(QNetworkReply::NetworkError error) {
  if (sender() != m_reply)
    return;
  qCritical() << "Modrinth hash lookup request failed with error"
              << m_reply->errorString() << "Server reply:\n"
              << m_reply->readAll();
  if (finished) {
    qCritical() << "Double finished ModrinthHashLookupRequest!";
    return;
  }
  m_status = Job_Failed;
  finished = true;
  m_reply->deleteLater();
  m_reply = nullptr;
  emit failed(m_index_within_job);
}

void HashLookupRequest::downloadProgress(qint64 bytesReceived,
                                         qint64 bytesTotal) {
  if (sender() != m_reply)
    return;
  m_total_progress = bytesTotal;
  m_progress = bytesReceived;
  emit netActionProgress(m_index_within_job, bytesReceived, bytesTotal);
}

void HashLookupRequest::downloadFinished() {
  if (sender() != m_reply)
    return;
  if (finished) {
    qCritical() << "Double finished ModrinthHashLookupRequest!";
    return;
  }

  QByteArray data = m_reply->readAll();
  m_reply->deleteLater();
  m_reply = nullptr;

  try {
    auto document = Json::requireDocument(data);
    auto rootObject = Json::requireObject(document);

    for (const auto &hashData : m_hashes) {
      if (rootObject.contains(hashData.hash)) {
        auto versionObject = Json::requireObject(rootObject, hashData.hash);

        auto files =
            Json::requireIsArrayOf<QJsonObject>(versionObject, "files");

        QJsonObject file;

        for (const auto &fileJson : files) {
          auto hashes = Json::requireObject(fileJson, "hashes");
          QString sha512 = Json::requireString(hashes, "sha512");

          if (sha512 == hashData.hash) {
            file = fileJson;
          }
        }

        m_output->append(HashLookupResponseData{hashData.fileInfo, true, file});
      } else {
        m_output->append(
            HashLookupResponseData{hashData.fileInfo, false, QJsonObject()});
      }
    }

    m_status = Job_Finished;
    finished = true;
    emit succeeded(m_index_within_job);
  } catch (const Json::JsonException &e) {
    qCritical() << "Failed to parse Modrinth hash lookup response: "
                << e.cause();
    m_status = Job_Failed;
    finished = true;
    emit failed(m_index_within_job);
  }
}
} // namespace Modrinth