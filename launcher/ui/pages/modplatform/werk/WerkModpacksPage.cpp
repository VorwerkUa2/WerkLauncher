#include "WerkModpacksPage.h"
#include "ui/dialogs/NewInstanceDialog.h"
#include "Application.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>

WerkModpacksPage::WerkModpacksPage(NewInstanceDialog *dialog, QWidget *parent)
    : QWidget(parent), m_dialog(dialog)
{
    m_layout = new QVBoxLayout(this);
    m_statusLabel = new QLabel(tr("Завантаження..."), this);
    m_layout->addWidget(m_statusLabel);
    
    m_listWidget = new QListWidget(this);
    m_listWidget->hide();
    m_layout->addWidget(m_listWidget);
    
    QNetworkRequest request(QUrl("https://werklauncher.com/modpacks.json"));
    m_reply = APPLICATION->network()->get(request);
    connect(m_reply, &QNetworkReply::finished, this, &WerkModpacksPage::onDownloadFinished);
}

WerkModpacksPage::~WerkModpacksPage()
{
    if (m_reply) {
        m_reply->deleteLater();
    }
}

void WerkModpacksPage::openedImpl()
{
}

void WerkModpacksPage::onDownloadFinished()
{
    if (m_reply->error() != QNetworkReply::NoError) {
        m_shouldDisplay = false;
        if (m_container) m_container->refreshContainer();
        m_reply->deleteLater();
        m_reply = nullptr;
        return;
    }
    
    QByteArray data = m_reply->readAll();
    m_reply->deleteLater();
    m_reply = nullptr;
    
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isArray() || doc.array().isEmpty()) {
        m_shouldDisplay = false;
        if (m_container) m_container->refreshContainer();
        return;
    }
    
    QJsonArray arr = doc.array();
    for (int i = 0; i < arr.size(); ++i) {
        QJsonObject obj = arr[i].toObject();
        QString name = obj.value("name").toString();
        m_listWidget->addItem(name);
    }
    
    m_statusLabel->hide();
    m_listWidget->show();
}
