#pragma once

#include "CurseForgeData.h"
#include "net/NetJob.h"

#include <QAbstractListModel>
#include <QVector>
#include <nonstd/optional>

namespace CurseForge {

using LogoCallback = std::function<void(QString)>;

class ListModel : public QAbstractListModel {
    Q_OBJECT

public:
    explicit ListModel(QObject *parent);
    ~ListModel() override;

    QVariant data(const QModelIndex &index, int role) const override;
    int columnCount(const QModelIndex &parent) const override;
    int rowCount(const QModelIndex &parent) const override;
    bool canFetchMore(const QModelIndex &parent) const override;
    void fetchMore(const QModelIndex &parent) override;

    struct Category {
        int id;
        QString name;
        QUrl iconUrl;
    };

    static const QVector<Category>& getCategories();
    void fetchCategories();

    void searchWithTerm(const QString &term, int sortField, int categoryId = 0);
    void getPackDetails(int id);
    nonstd::optional<Modpack> getModpackById(int id);

signals:
    void packDataChanged(int id);
    void categoriesLoaded();

private slots:
    void searchRequestFinished();
    void searchRequestFailed();

    void descriptionRequestFinished();
    void descriptionRequestFailed();

    void filesRequestFinished();
    void filesRequestFailed();

    void categoriesRequestFinished();
    void categoriesRequestFailed();

private:
    void performPaginatedSearch();

    void logoFailed(const QString &logo);
    void logoLoaded(const QString &logo, const QIcon &out);
    void requestLogo(const QString &logo, const QUrl &url);

    nonstd::optional<int> getIndexFromId(int id);

    bool isPackDetailInProgress();
    void cancelPackDetail();
    void checkDetailsDone();

    QVector<Modpack> modpacks;
    QStringList m_failedLogos;
    QStringList m_loadingLogos;
    QMap<QString, QIcon> m_logoMap;
    QMap<QString, LogoCallback> waitingCallbacks;

    QString currentSearchTerm;
    int currentSortField = 2; // Popularity
    int currentCategoryId = 0;
    int nextSearchOffset = 0;
    enum SearchState {
        None,
        CanPossiblyFetchMore,
        ResetRequested,
        Finished
    } searchState = None;

    int queuedPackDetailRequest = 0;
    int currentPackDetailRequest = 0;
    NetJob::Ptr jobPtr;
    QByteArray response;

    NetJob::Ptr descriptionPtr;
    QByteArray descriptionResponse;

    NetJob::Ptr filesPtr;
    QByteArray filesResponse;

    NetJob::Ptr categoriesPtr;
    QByteArray categoriesResponse;

    static QVector<Category> s_categories;
    static bool s_categoriesLoaded;
    static bool s_categoriesLoading;
};

}
