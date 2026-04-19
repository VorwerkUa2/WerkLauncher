
#include "GetSkinStep.h"

#include <QDebug>
#include <QNetworkRequest>

#include "minecraft/auth/AuthRequest.h"
#include "minecraft/auth/Parsers.h"

GetSkinStep::GetSkinStep(AccountData *data) : AuthStep(data) {}

GetSkinStep::~GetSkinStep() noexcept = default;

QString GetSkinStep::describe() { return tr("Getting skin."); }

void GetSkinStep::perform() {
  if (m_data->minecraftProfile.skin.url.isEmpty()) {
    emit finished(AccountTaskState::STATE_SUCCEEDED, tr("No skin URL set."));
    return;
  }
  auto url = QUrl(m_data->minecraftProfile.skin.url);
  qDebug() << "MSSS: Attempting to download skin from:" << url.toString();
  QNetworkRequest request = QNetworkRequest(url);
  AuthRequest *requestor = new AuthRequest(this);
  connect(requestor, &AuthRequest::finished, this, &GetSkinStep::onRequestDone);
  requestor->get(request);
}

void GetSkinStep::onRequestDone(QNetworkReply::NetworkError error,
                                QByteArray data,
                                QList<QNetworkReply::RawHeaderPair> headers) {
  auto requestor = qobject_cast<AuthRequest *>(QObject::sender());
  requestor->deleteLater();

  if (error == QNetworkReply::NoError) {
    qDebug() << "[MSSS_DIAG] Successfully downloaded skin data (" << data.size()
             << "bytes)";
    QImage testImg;
    if (testImg.loadFromData(data)) {
      qDebug() << "[MSSS_DIAG] Downloaded data is a VALID image:"
               << testImg.width() << "x" << testImg.height();
      m_data->minecraftProfile.skin.data = data;
      emit finished(AccountTaskState::STATE_SUCCEEDED, tr("Got skin"));
    } else {
      qWarning() << "[MSSS_DIAG] Downloaded data is NOT a valid image!";
      emit finished(AccountTaskState::STATE_SUCCEEDED, tr("Invalid skin data"));
    }
  } else {
    qWarning() << "[MSSS_DIAG] Skin download failed with error:" << error
               << "URL:" << m_data->minecraftProfile.skin.url;
    emit finished(AccountTaskState::STATE_SUCCEEDED,
                  tr("Offline: using cached skin data"));
  }
}
