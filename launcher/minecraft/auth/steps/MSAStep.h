#pragma once
#include <QObject>

#include "QObjectPtr.h"
#include "minecraft/auth/AuthStep.h"

#include <QtNetworkAuth/QOAuth2AuthorizationCodeFlow>
#include <QtNetworkAuth/QOAuthHttpServerReplyHandler>

class MSAStep : public AuthStep {
    Q_OBJECT
public:
    enum Action {
        Refresh,
        Login
    };
public:
    explicit MSAStep(AccountData *data, Action action);
    virtual ~MSAStep() noexcept = default;

    void perform() override;

    QString describe() override;

private slots:
    void authorizeWithBrowser(const QUrl &url);

private:
    QOAuth2AuthorizationCodeFlow m_oauth2;
    Action m_action;
    QString m_clientId;
};
