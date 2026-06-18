#include "CurseForgeModel.h"
#include "Application.h"
#include "BuildConfig.h"
#include "Json.h"

#include <QIcon>
#include <QNetworkRequest>

static const char * CURSEFORGE_API_BASE = "https://api.curseforge.com";

// Minecraft gameId = 432, classId for Modpacks = 4471

QVector<CurseForge::ListModel::Category> CurseForge::ListModel::s_categories;
bool CurseForge::ListModel::s_categoriesLoaded = false;
bool CurseForge::ListModel::s_categoriesLoading = false;

CurseForge::ListModel::ListModel(QObject *parent) : QAbstractListModel(parent) {}

CurseForge::ListModel::~ListModel() = default;

const QVector<CurseForge::ListModel::Category>& CurseForge::ListModel::getCategories() {
    return s_categories;
}

void CurseForge::ListModel::fetchCategories() {
    if (s_categoriesLoaded || s_categoriesLoading) {
        if (s_categoriesLoaded) {
            emit categoriesLoaded();
        }
        return;
    }
    s_categoriesLoading = true;
    
    auto *netJob = new NetJob("CurseForge::Categories", APPLICATION->network());
    QString url = QString("%1/v1/categories?gameId=432&classId=4471").arg(CURSEFORGE_API_BASE);
    
    auto dl = Net::Download::makeByteArray(QUrl(url), &categoriesResponse);
    dl->setHeader("x-api-key", BuildConfig.CURSEFORGE_API_KEY.toUtf8());
    netJob->addNetAction(dl);
    categoriesPtr = netJob;
    categoriesPtr->start();
    
    QObject::connect(netJob, &NetJob::succeeded, this, &ListModel::categoriesRequestFinished);
    QObject::connect(netJob, &NetJob::failed, this, &ListModel::categoriesRequestFailed);
}

void CurseForge::ListModel::categoriesRequestFinished() {
    categoriesPtr.reset();
    s_categoriesLoading = false;

    QJsonParseError parse_error;
    QJsonDocument doc = QJsonDocument::fromJson(categoriesResponse, &parse_error);
    if (parse_error.error == QJsonParseError::NoError) {
        try {
            auto obj = Json::requireObject(doc);
            auto dataArray = Json::requireArray(obj, "data");
            
            s_categories.clear();
            for (auto catRaw : dataArray) {
                auto catObj = catRaw.toObject();
                Category cat;
                cat.id = Json::requireInteger(catObj, "id");
                cat.name = Json::requireString(catObj, "name");
                cat.iconUrl = QUrl(Json::ensureString(catObj, "iconUrl", ""));
                s_categories.append(cat);
            }
            s_categoriesLoaded = true;
            emit categoriesLoaded();
        } catch (const JSONValidationError &e) {
            qWarning() << "Error parsing CurseForge categories: " << e.cause();
        }
    }
}

void CurseForge::ListModel::categoriesRequestFailed() {
    categoriesPtr.reset();
    s_categoriesLoading = false;
    qWarning() << "Failed to fetch CurseForge categories.";
}

QVariant CurseForge::ListModel::data(const QModelIndex &index, int role) const {
  int pos = index.row();
  if (pos >= modpacks.size() || pos < 0 || !index.isValid()) {
    return QString("INVALID INDEX %1").arg(pos);
  }

  auto pack = modpacks.at(pos);
  if (role == Qt::DisplayRole) {
    return pack.name;
  } else if (role == Qt::DecorationRole) {
    auto key = QString::number(pack.id);
    if (m_logoMap.contains(key)) {
      return (m_logoMap.value(key));
    }
    QIcon icon = APPLICATION->getThemedIcon("screenshot-placeholder");
    ((ListModel *)this)->requestLogo(key, pack.iconUrl);
    return icon;
  } else if (role == Qt::ToolTipRole) {
    return pack.summary;
  } else if (role == Qt::UserRole) {
    return pack.author;
  } else if (role == Qt::UserRole + 1) {
    return (qulonglong)pack.downloadCount;
  } else if (role == Qt::UserRole + 2) {
    QVariant v;
    v.setValue(pack);
    return v;
  }
  return QVariant();
}

bool CurseForge::ListModel::canFetchMore(const QModelIndex &parent) const {
  return searchState == CanPossiblyFetchMore;
}

void CurseForge::ListModel::fetchMore(const QModelIndex &parent) {
  if (parent.isValid())
    return;
  if (nextSearchOffset == 0) {
    qWarning() << "fetchMore with 0 offset is wrong...";
    return;
  }
  performPaginatedSearch();
}

int CurseForge::ListModel::columnCount(const QModelIndex &parent) const {
  return 1;
}

int CurseForge::ListModel::rowCount(const QModelIndex &parent) const {
  return modpacks.size();
}

void CurseForge::ListModel::searchWithTerm(const QString &term, int sortField, int categoryId) {
  if (currentSearchTerm == term &&
      currentSearchTerm.isNull() == term.isNull() && currentSortField == sortField && currentCategoryId == categoryId) {
    return;
  }
  currentSearchTerm = term;
  currentSortField = sortField;
  currentCategoryId = categoryId;
  if (jobPtr) {
    jobPtr->abort();
    searchState = ResetRequested;
    return;
  } else {
    beginResetModel();
    modpacks.clear();
    endResetModel();
    searchState = None;
  }
  nextSearchOffset = 0;
  performPaginatedSearch();
}

void CurseForge::ListModel::performPaginatedSearch() {
  auto *netJob = new NetJob("CurseForge::Search", APPLICATION->network());

  QString searchUrl = QString("%1/v1/mods/search?gameId=432&classId=4471"
                              "&sortField=%2&sortOrder=desc&pageSize=25&index=%3")
                          .arg(CURSEFORGE_API_BASE)
                          .arg(currentSortField)
                          .arg(nextSearchOffset);

  if (currentCategoryId != 0) {
      searchUrl += QString("&categoryId=%1").arg(currentCategoryId);
  }

  if (!currentSearchTerm.isEmpty()) {
    searchUrl += "&searchFilter=" + QUrl::toPercentEncoding(currentSearchTerm);
  }

  auto dl = Net::Download::makeByteArray(QUrl(searchUrl), &response);
  dl->setHeader("x-api-key", BuildConfig.CURSEFORGE_API_KEY.toUtf8());
  netJob->addNetAction(dl);
  jobPtr = netJob;
  jobPtr->start();
  QObject::connect(netJob, &NetJob::succeeded, this,
                   &ListModel::searchRequestFinished);
  QObject::connect(netJob, &NetJob::failed, this,
                   &ListModel::searchRequestFailed);
}

void CurseForge::ListModel::searchRequestFinished() {
  jobPtr.reset();

  QJsonParseError parse_error;
  QJsonDocument doc = QJsonDocument::fromJson(response, &parse_error);
  if (parse_error.error != QJsonParseError::NoError) {
    qWarning() << "Error while parsing JSON response from CurseForge at "
               << parse_error.offset
               << " reason: " << parse_error.errorString();
    qWarning() << response;
    return;
  }

  QVector<CurseForge::Modpack> newList;
  int totalCount = 0;

  try {
    auto obj = Json::requireObject(doc);
    auto dataArray = Json::requireArray(obj, "data");
    auto pagination = Json::requireObject(obj, "pagination");
    totalCount = Json::requireInteger(pagination, "totalCount");

    for (auto packRaw : dataArray) {
      auto packObj = packRaw.toObject();
      CurseForge::Modpack pack;
      try {
        pack.id = Json::requireInteger(packObj, "id");
        pack.name = Json::requireString(packObj, "name");
        pack.summary = Json::requireString(packObj, "summary");
        pack.downloadCount = Json::ensureInteger(packObj, "downloadCount", 0);

        // Get icon URL from logo object
        if (packObj.contains("logo") && packObj["logo"].isObject()) {
          auto logoObj = packObj["logo"].toObject();
          auto thumbnailUrl = Json::ensureString(logoObj, "thumbnailUrl", "");
          if (!thumbnailUrl.isEmpty()) {
            pack.iconUrl = QUrl(thumbnailUrl);
          }
        }

        // Get author
        if (packObj.contains("authors") && packObj["authors"].isArray()) {
          auto authors = packObj["authors"].toArray();
          if (!authors.isEmpty()) {
            pack.author = Json::requireString(authors[0].toObject(), "name");
          }
        }

        newList.append(pack);
      } catch (const JSONValidationError &e) {
        qWarning() << "Error while loading pack from CurseForge: " << e.cause();
        continue;
      }
    }
  } catch (const JSONValidationError &e) {
    qWarning() << "Error while parsing response from CurseForge: " << e.cause();
    return;
  }

  if ((totalCount - nextSearchOffset) <= 25)
    searchState = Finished;
  else {
    nextSearchOffset += 25;
    searchState = CanPossiblyFetchMore;
  }
  if (!newList.isEmpty()) {
    beginInsertRows(QModelIndex(), modpacks.size(),
                    modpacks.size() + newList.size() - 1);
    for (auto item : newList) {
      modpacks.append(item);
    }
    endInsertRows();
  }
}

void CurseForge::ListModel::searchRequestFailed() {
  jobPtr.reset();

  if (searchState == ResetRequested) {
    beginResetModel();
    modpacks.clear();
    endResetModel();

    nextSearchOffset = 0;
    performPaginatedSearch();
  } else {
    searchState = Finished;
  }
}

void CurseForge::ListModel::logoLoaded(const QString &logo, const QIcon &out) {
  m_loadingLogos.removeAll(logo);
  m_logoMap.insert(logo, out);
  for (int i = 0; i < modpacks.size(); i++) {
    if (QString::number(modpacks[i].id) == logo) {
      emit dataChanged(createIndex(i, 0), createIndex(i, 0),
                       {Qt::DecorationRole});
    }
  }
}

void CurseForge::ListModel::logoFailed(const QString &logo) {
  m_failedLogos.append(logo);
  m_loadingLogos.removeAll(logo);
}

void CurseForge::ListModel::requestLogo(const QString &logo, const QUrl &url) {
  if (m_loadingLogos.contains(logo) || m_failedLogos.contains(logo) || !url.isValid()) {
    return;
  }

  MetaEntryPtr entry = APPLICATION->metacache()->resolveEntry(
      "CurseForgePacks", QString("logos/%1").arg(logo));
  auto *job = new NetJob(QString("CurseForge Icon Download %1").arg(logo),
                         APPLICATION->network());
  job->addNetAction(Net::Download::makeCached(url, entry));

  auto fullPath = entry->getFullPath();
  QObject::connect(job, &NetJob::succeeded, this, [this, logo, fullPath] {
    QIcon icon(fullPath);
    QSize size = icon.actualSize(QSize(48, 48));
    if (size.width() < 48 && size.height() < 48) {
      icon = icon.pixmap(48, 48).scaled(48, 48, Qt::KeepAspectRatioByExpanding,
                                        Qt::SmoothTransformation);
    }
    logoLoaded(logo, icon);
    if (waitingCallbacks.contains(logo)) {
      waitingCallbacks.value(logo)(fullPath);
    }
  });

  QObject::connect(job, &NetJob::failed, this,
                   [this, logo] { logoFailed(logo); });

  job->start();

  m_loadingLogos.append(logo);
}

void CurseForge::ListModel::getPackDetails(int id) {
  auto index = getIndexFromId(id);
  if (!index) {
    return;
  }

  if (isPackDetailInProgress()) {
    queuedPackDetailRequest = id;
    cancelPackDetail();
    return;
  }

  currentPackDetailRequest = id;
  auto idStr = QString::number(id);

  auto &modpack = modpacks[*index];

  // Get description (HTML)
  if (modpack.detailsLoaded != LoadState::Loaded) {
    QString descUrl = QString("%1/v1/mods/%2/description").arg(CURSEFORGE_API_BASE).arg(idStr);
    auto *netJob = new NetJob("CurseForge::PackDescription", APPLICATION->network());
    auto dl = Net::Download::makeByteArray(QUrl(descUrl), &descriptionResponse);
    dl->setHeader("x-api-key", BuildConfig.CURSEFORGE_API_KEY.toUtf8());
    netJob->addNetAction(dl);
    descriptionPtr = netJob;
    descriptionPtr->start();
    QObject::connect(netJob, &NetJob::succeeded, this,
                     &ListModel::descriptionRequestFinished);
    QObject::connect(netJob, &NetJob::failed, this,
                     &ListModel::descriptionRequestFailed);
  }

  // Get files
  if (modpack.filesLoaded != LoadState::Loaded) {
    QString filesUrl = QString("%1/v1/mods/%2/files?pageSize=50").arg(CURSEFORGE_API_BASE).arg(idStr);
    auto *netJob = new NetJob("CurseForge::PackFiles", APPLICATION->network());
    auto dl = Net::Download::makeByteArray(QUrl(filesUrl), &filesResponse);
    dl->setHeader("x-api-key", BuildConfig.CURSEFORGE_API_KEY.toUtf8());
    netJob->addNetAction(dl);
    filesPtr = netJob;
    filesPtr->start();
    QObject::connect(netJob, &NetJob::succeeded, this,
                     &ListModel::filesRequestFinished);
    QObject::connect(netJob, &NetJob::failed, this,
                     &ListModel::filesRequestFailed);
  }
}

bool CurseForge::ListModel::isPackDetailInProgress() {
  return descriptionPtr || filesPtr;
}

void CurseForge::ListModel::cancelPackDetail() {
  if (descriptionPtr) {
    descriptionPtr->abort();
  }
  if (filesPtr) {
    filesPtr->abort();
  }
}

nonstd::optional<int> CurseForge::ListModel::getIndexFromId(int id) {
  for (int i = 0; i < modpacks.size(); i++) {
    if (modpacks[i].id == id) {
      return i;
    }
  }
  return nonstd::nullopt;
}

nonstd::optional<CurseForge::Modpack>
CurseForge::ListModel::getModpackById(int id) {
  auto index = getIndexFromId(id);
  if (!index) {
    return nonstd::nullopt;
  }
  return modpacks[*index];
}

void CurseForge::ListModel::descriptionRequestFinished() {
  auto index = getIndexFromId(currentPackDetailRequest);
  if (index) {
    auto &modpack = modpacks[*index];

    QJsonParseError parse_error;
    QJsonDocument doc = QJsonDocument::fromJson(descriptionResponse, &parse_error);
    if (parse_error.error == QJsonParseError::NoError) {
      try {
        auto obj = Json::requireObject(doc);
        modpack.description = Json::requireString(obj, "data");
        modpack.detailsLoaded = LoadState::Loaded;
      } catch (const JSONValidationError &e) {
        qWarning() << "Error parsing CurseForge description: " << e.cause();
        modpack.detailsLoaded = LoadState::Errored;
      }
    } else {
      modpack.detailsLoaded = LoadState::Errored;
    }
    emit packDataChanged(currentPackDetailRequest);
  }
  descriptionPtr.reset();
  checkDetailsDone();
}

void CurseForge::ListModel::descriptionRequestFailed() {
  auto index = getIndexFromId(currentPackDetailRequest);
  if (index) {
    auto &modpack = modpacks[*index];
    if (modpack.detailsLoaded == LoadState::NotLoaded) {
      modpack.detailsLoaded = LoadState::Errored;
      emit packDataChanged(currentPackDetailRequest);
    }
  }
  descriptionPtr.reset();
  checkDetailsDone();
}

void CurseForge::ListModel::filesRequestFinished() {
  auto index = getIndexFromId(currentPackDetailRequest);
  if (index) {
    auto &modpack = modpacks[*index];

    QJsonParseError parse_error;
    QJsonDocument doc = QJsonDocument::fromJson(filesResponse, &parse_error);
    if (parse_error.error == QJsonParseError::NoError) {
      try {
        auto obj = Json::requireObject(doc);
        auto dataArray = Json::requireArray(obj, "data");

        QVector<CurseForge::ModFile> fileList;
        for (auto fileRaw : dataArray) {
          auto fileObj = fileRaw.toObject();
          CurseForge::ModFile file;
          file.id = Json::requireInteger(fileObj, "id");
          file.displayName = Json::requireString(fileObj, "displayName");
          file.fileName = Json::requireString(fileObj, "fileName");
          file.downloadUrl = Json::ensureString(fileObj, "downloadUrl", "");
          file.fileDate = Json::requireDateTime(fileObj, "fileDate");

          int releaseTypeInt = Json::ensureInteger(fileObj, "releaseType", 0);
          switch(releaseTypeInt) {
            case 1: file.releaseType = FileReleaseType::Release; break;
            case 2: file.releaseType = FileReleaseType::Beta; break;
            case 3: file.releaseType = FileReleaseType::Alpha; break;
            default: file.releaseType = FileReleaseType::Unknown; break;
          }

          // Game versions
          if (fileObj.contains("gameVersions") && fileObj["gameVersions"].isArray()) {
            for (auto v : fileObj["gameVersions"].toArray()) {
              file.gameVersions.append(v.toString());
            }
          }

          // Only include files that have a download URL
          if (!file.downloadUrl.isEmpty()) {
            fileList.append(file);
          }
        }
        modpack.files = fileList;
        modpack.filesLoaded = LoadState::Loaded;
      } catch (const JSONValidationError &e) {
        qWarning() << "Error parsing CurseForge files: " << e.cause();
        modpack.filesLoaded = LoadState::Errored;
      }
    } else {
      modpack.filesLoaded = LoadState::Errored;
    }
    emit packDataChanged(currentPackDetailRequest);
  }
  filesPtr.reset();
  checkDetailsDone();
}

void CurseForge::ListModel::filesRequestFailed() {
  auto index = getIndexFromId(currentPackDetailRequest);
  if (index) {
    auto &modpack = modpacks[*index];
    if (modpack.filesLoaded == LoadState::NotLoaded) {
      modpack.filesLoaded = LoadState::Errored;
      emit packDataChanged(currentPackDetailRequest);
    }
  }
  filesPtr.reset();
  checkDetailsDone();
}

void CurseForge::ListModel::checkDetailsDone() {
  if (isPackDetailInProgress()) {
    return;
  }

  currentPackDetailRequest = 0;

  if (queuedPackDetailRequest != 0) {
    getPackDetails(queuedPackDetailRequest);
    queuedPackDetailRequest = 0;
  }
}
