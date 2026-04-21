#include "MSAStep.h"

#include <QNetworkRequest>
#include <QDesktopServices>

#include "minecraft/auth/AuthRequest.h"
#include "minecraft/auth/Parsers.h"

#include "Application.h"

MSAStep::MSAStep(AccountData *data, Action action)
    : AuthStep(data), m_action(action) {
  
  m_clientId = APPLICATION->msaClientId();
  
  auto replyHandler = new QOAuthHttpServerReplyHandler(0, this);
  replyHandler->setCallbackText("Login successful! You can close this window and return to the launcher.");
  m_oauth2.setReplyHandler(replyHandler);

  m_oauth2.setAuthorizationUrl(QUrl("https://login.microsoftonline.com/consumers/oauth2/v2.0/authorize"));
  m_oauth2.setAccessTokenUrl(QUrl("https://login.microsoftonline.com/consumers/oauth2/v2.0/token"));
  m_oauth2.setScope("XboxLive.SignIn XboxLive.offline_access");
  m_oauth2.setClientIdentifier(m_clientId);
  m_oauth2.setNetworkAccessManager(APPLICATION->network().get());

  connect(&m_oauth2, &QOAuth2AuthorizationCodeFlow::granted, this, [this] {
      m_data->msaToken.issueInstant = QDateTime::currentDateTimeUtc();
      m_data->msaToken.notAfter = m_oauth2.expirationAt();
      m_data->msaToken.extra = m_oauth2.extraTokens();
      m_data->msaToken.refresh_token = m_oauth2.refreshToken();
      m_data->msaToken.token = m_oauth2.token();
      emit finished(AccountTaskState::STATE_WORKING, tr("Got MSA token"));
  });

  connect(&m_oauth2, &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser, this, &MSAStep::authorizeWithBrowser);

  connect(&m_oauth2, &QOAuth2AuthorizationCodeFlow::requestFailed, this, [this](const QAbstractOAuth2::Error err) {
      auto state = AccountTaskState::STATE_FAILED_HARD;
      if (err == QAbstractOAuth2::Error::NetworkError) {
          state = AccountTaskState::STATE_OFFLINE;
      }
      auto message = tr("Microsoft user authentication failed.");
      qWarning() << message;
      emit finished(state, message);
  });

  connect(&m_oauth2, &QOAuth2AuthorizationCodeFlow::error, this,
          [this](const QString& error, const QString& errorDescription, const QUrl& uri) {
              qWarning() << "Failed to login because" << error << errorDescription;
              emit finished(AccountTaskState::STATE_FAILED_HARD, errorDescription);
          });

  connect(&m_oauth2, &QOAuth2AuthorizationCodeFlow::extraTokensChanged, this,
          [this](const QVariantMap& tokens) { m_data->msaToken.extra = tokens; });
}

QString MSAStep::describe() { return tr("Logging in with Microsoft account."); }

void MSAStep::perform() {
  switch (m_action) {
  case Refresh: {
    if (m_data->msaToken.refresh_token.isEmpty()) {
        emit finished(AccountTaskState::STATE_FAILED_HARD, tr("Microsoft user authentication failed - refresh token is empty."));
        return;
    }
    m_oauth2.setRefreshToken(m_data->msaToken.refresh_token);
    m_oauth2.refreshAccessToken();
    return;
  }
  case Login: {
    m_oauth2.setModifyParametersFunction(
        [](QAbstractOAuth::Stage stage, QMultiMap<QString, QVariant> *map) { 
            if (stage == QAbstractOAuth::Stage::RequestingAuthorization) {
                map->insert("prompt", "select_account"); 
            }
        });

    *m_data = AccountData();
    m_oauth2.grant();
    return;
  }
  }
}

void MSAStep::authorizeWithBrowser(const QUrl &url) {
    QDesktopServices::openUrl(url);
}
