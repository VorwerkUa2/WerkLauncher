#pragma once

#include <QDialog>
#include <QModelIndex>
#include <QTableWidgetItem>

namespace Ui {
class ModDownloaderDialog;
}

#include "ui/pages/modplatform/unified/UnifiedModData.h"

namespace Unified {
class UnifiedModModel;
}

class BaseInstance;

class ModDownloaderDialog : public QDialog {
    Q_OBJECT

public:
    explicit ModDownloaderDialog(BaseInstance *inst, Unified::ProjectType projectType, QWidget *parent = nullptr);
    ~ModDownloaderDialog();

private slots:
    void onSearchTriggered();
    void onModSelected(const QModelIndex &index);
    void onBackClicked();
    void onInstallClicked();

    void onModDetailsFetched(const Unified::UnifiedModDetails &details);
    void onModDetailsFailed(const QString &id);

    void onModVersionsFetched(const QList<Unified::UnifiedModVersion> &versions);
    void onModVersionsFailed(const QString &id);

    void onPrevPage();
    void onNextPage();

private:
    void setupUi();
    void showDetails();
    void showList();
    void performSearch();
    void getGameVersionAndLoader(QString &mcVersion, QString &loader);
    void downloadFile(const QString &url, const QString &filename);
    void loadHtmlImages(const QString &html);

    Ui::ModDownloaderDialog *ui;
    Unified::UnifiedModModel *m_model;
    int m_currentPage = 0;
    QSet<QString> m_downloadedImages;
    BaseInstance *m_inst;
    Unified::ProjectType m_projectType;
    QList<Unified::UnifiedModVersion> m_currentVersions;
    std::shared_ptr<QObject> m_downloadJob; // We will just use NetJob* and manage it, or include NetJob.h
    QString m_selectedModId;
    QString m_selectedVersionId;
};
