#pragma once

#include "Application.h"
#include "ui/pages/BasePage.h"
#include <QWidget>
#include <QNetworkReply>

class NewInstanceDialog;
class QListWidget;
class QVBoxLayout;
class QLabel;

class WerkModpacksPage : public QWidget, public BasePage
{
    Q_OBJECT

public:
    explicit WerkModpacksPage(NewInstanceDialog *dialog, QWidget *parent = nullptr);
    ~WerkModpacksPage() override;

    QString displayName() const override { return tr("Мої Збірки"); }
    QIcon icon() const override { return APPLICATION->getThemedIcon("star"); }
    QString id() const override { return "werk_modpacks"; }

    bool shouldDisplay() const override { return m_shouldDisplay; }
    void openedImpl() override;

private slots:
    void onDownloadFinished();

private:
    NewInstanceDialog *m_dialog = nullptr;
    QListWidget *m_listWidget = nullptr;
    QVBoxLayout *m_layout = nullptr;
    QLabel *m_statusLabel = nullptr;
    QNetworkReply *m_reply = nullptr;
    bool m_shouldDisplay = true; // initially true to fetch
    bool m_loaded = false;
};
