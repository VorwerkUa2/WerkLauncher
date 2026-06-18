#pragma once

#include "UnifiedModData.h"
#include "net/NetJob.h"
#include <QAbstractListModel>
#include <QVector>
#include <QIcon>
#include <QMap>
#include <QStringList>

namespace Unified {

class UnifiedModModel : public QAbstractListModel {
    Q_OBJECT

public:
    explicit UnifiedModModel(QObject *parent = nullptr);
    ~UnifiedModModel() override;

    QVariant data(const QModelIndex &index, int role) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    void search(const QString &term, ProjectType projectType = ProjectType::Mod, const QString &mcVersion = QString(), const QString &loader = QString(), const QStringList &modrinthCategories = QStringList(), const QList<int> &curseforgeCategoryIds = QList<int>());
    
    void setPage(int page);
    bool hasMorePages() const;
    int currentPage() const { return m_currentPage; }
    
    void fetchModDetails(const UnifiedModData &mod);
    void fetchModVersions(const UnifiedModData &mod, const QString &mcVersion, const QString &loader);

signals:
    void searchFinished();
    void modDetailsFetched(const Unified::UnifiedModDetails &details);
    void modDetailsFailed(const QString &id);
    void modVersionsFetched(const QList<Unified::UnifiedModVersion> &versions);
    void modVersionsFailed(const QString &id);

private slots:
    void modrinthSearchFinished();
    void modrinthSearchFailed();

    void curseForgeSearchFinished();
    void curseForgeSearchFailed();

    void modrinthVersionsFinished();
    void curseForgeVersionsFinished();

private:
    void checkBothFinished();

    void fetchLogo(const QString &id, const QUrl &url);
    void logoFailed(const QString &id);
    void logoLoaded(const QString &id, const QIcon &out);

    NetJob::Ptr m_detailsJob;
    QByteArray m_detailsResponse;
    UnifiedModData m_currentDetailsMod;
    
    // CF uses a two-step detail fetch
    NetJob::Ptr m_cfDescJob;
    QByteArray m_cfDescResponse;
    bool m_cfDescDone = false;
    bool m_cfDetailsDone = false;
    UnifiedModDetails m_pendingCfDetails;
    
    void modrinthDetailsFinished();
    void curseForgeDetailsFinished();
    void curseForgeDescFinished();
    void checkCurseForgeDetailsFinished();

    NetJob::Ptr m_mrVersionsJob;
    QByteArray m_mrVersionsResponse;
    NetJob::Ptr m_cfVersionsJob;
    QByteArray m_cfVersionsResponse;
    
    bool m_mrVersionsDone = false;
    bool m_cfVersionsDone = false;
    QList<UnifiedModVersion> m_pendingVersions;
    
    UnifiedModData m_currentVersionsMod;
    QString m_currentVersionsMcVersion;
    QString m_currentVersionsLoader;
    
    void checkVersionsFinished();

    QStringList m_failedLogos;
    QStringList m_loadingLogos;
    QMap<QString, QIcon> m_logoMap;

    int m_currentPage = 0;
    ProjectType m_currentSearchType = ProjectType::Mod;
    QList<UnifiedModData> m_allMods;
    QVector<UnifiedModData> m_mods;

    NetJob::Ptr m_modrinthJob;
    QByteArray m_modrinthResponse;

    NetJob::Ptr m_curseForgeJob;
    QByteArray m_curseForgeResponse;
    
    bool m_modrinthDone = false;
    bool m_curseforgeDone = false;
    QVector<UnifiedModData> m_pendingModrinth;
    QVector<UnifiedModData> m_pendingCurseForge;
};

}
