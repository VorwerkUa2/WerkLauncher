#include "UnifiedModModel.h"
#include "Application.h"
#include "BuildConfig.h"
#include "Json.h"

#include <QIcon>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrl>
#include <algorithm>
#include <QImageReader>
#include <QBuffer>

namespace Unified {

UnifiedModModel::UnifiedModModel(QObject *parent) : QAbstractListModel(parent) {}

UnifiedModModel::~UnifiedModModel() = default;

int UnifiedModModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) return 0;
    return m_mods.size();
}

void UnifiedModModel::setPage(int page) {
    if (page < 0) page = 0;
    m_currentPage = page;
    beginResetModel();
    m_mods.clear();
    int start = page * 20;
    int end = qMin(start + 20, m_allMods.size());
    for (int i = start; i < end; ++i) {
        m_mods.append(m_allMods.at(i));
    }
    endResetModel();
}

bool UnifiedModModel::hasMorePages() const {
    return (m_currentPage + 1) * 20 < m_allMods.size();
}

int UnifiedModModel::columnCount(const QModelIndex &parent) const {
    return 1;
}

QVariant UnifiedModModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() < 0 || index.row() >= m_mods.size())
        return QVariant();

    const auto &mod = m_mods.at(index.row());
    QString primaryId = mod.modrinthId.isEmpty() ? mod.curseforgeId : mod.modrinthId;

    if (role == Qt::DisplayRole) {
        return mod.name;
    } else if (role == Qt::ToolTipRole) {
        return mod.summary;
    } else if (role == Qt::DecorationRole) {
        if (m_logoMap.contains(primaryId)) {
            return m_logoMap.value(primaryId);
        } else {
            if (!mod.iconUrl.isEmpty() && mod.iconUrl.isValid()) {
                const_cast<UnifiedModModel*>(this)->fetchLogo(primaryId, mod.iconUrl);
            }
            return QVariant();
        }
    } else if (role == Qt::UserRole) {
        return QVariant::fromValue(mod);
    }
    return QVariant();
}

void UnifiedModModel::search(const QString &term, ProjectType projectType, const QString &mcVersion, const QString &loader, const QStringList &modrinthCategories, const QList<int> &curseforgeCategoryIds) {
    if (m_modrinthJob) m_modrinthJob->abort();
    if (m_curseForgeJob) m_curseForgeJob->abort();

    beginResetModel();
    m_allMods.clear();
    m_mods.clear();
    m_currentPage = 0;
    m_currentSearchType = projectType;
    endResetModel();

    m_modrinthDone = false;
    m_curseforgeDone = false;
    m_pendingModrinth.clear();
    m_pendingCurseForge.clear();

    // Start Modrinth Search
    QString modrinthType;
    switch (projectType) {
        case ProjectType::Mod: modrinthType = "mod"; break;
        case ProjectType::ResourcePack: modrinthType = "resourcepack"; break;
        case ProjectType::ShaderPack: modrinthType = "shader"; break;
        case ProjectType::DataPack: modrinthType = "datapack"; break;
    }

    QString modrinthUrl = QString("https://api.modrinth.com/v2/search?limit=100");
    
    QStringList facets;
    facets.append(QString("[\"project_type:%1\"]").arg(modrinthType));
    if (!mcVersion.isEmpty()) {
        facets.append(QString("[\"versions:%1\"]").arg(mcVersion));
    }
    if (!loader.isEmpty()) {
        facets.append(QString("[\"categories:%1\"]").arg(loader.toLower()));
    }
    if (!modrinthCategories.isEmpty()) {
        QStringList catItems;
        for (const auto& cat : modrinthCategories) {
            catItems.append(QString("\"categories:%1\"").arg(cat));
        }
        facets.append("[" + catItems.join(",") + "]");
    }
    
    modrinthUrl += "&facets=[" + facets.join(",") + "]";
    
    if (!term.isEmpty()) {
        modrinthUrl += "&query=" + QUrl::toPercentEncoding(term);
    }

    m_modrinthJob = new NetJob("Unified::ModrinthSearch", APPLICATION->network());
    m_modrinthJob->addNetAction(Net::Download::makeByteArray(QUrl(modrinthUrl), &m_modrinthResponse));
    m_modrinthJob->start();
    connect(m_modrinthJob.get(), &NetJob::succeeded, this, &UnifiedModModel::modrinthSearchFinished);
    connect(m_modrinthJob.get(), &NetJob::failed, this, &UnifiedModModel::modrinthSearchFailed);

    // Start CurseForge Search
    int cfClassId;
    switch (projectType) {
        case ProjectType::Mod: cfClassId = 6; break;
        case ProjectType::ResourcePack: cfClassId = 12; break;
        case ProjectType::ShaderPack: cfClassId = 6552; break;
        case ProjectType::DataPack: cfClassId = 6945; break;
    }

    QString cfUrl = QString("https://api.curseforge.com/v1/mods/search?gameId=432&classId=%1&pageSize=50&sortField=2&sortOrder=desc").arg(cfClassId);
    if (!term.isEmpty()) {
        cfUrl += "&searchFilter=" + QUrl::toPercentEncoding(term);
    }
    if (!curseforgeCategoryIds.isEmpty() && curseforgeCategoryIds.first() > 0) {
        cfUrl += "&categoryId=" + QString::number(curseforgeCategoryIds.first());
    }
    if (!mcVersion.isEmpty()) {
        cfUrl += "&gameVersion=" + QUrl::toPercentEncoding(mcVersion);
    }
    if (!loader.isEmpty()) {
        int modLoaderType = 0;
        if (loader.toLower() == "forge") modLoaderType = 1;
        else if (loader.toLower() == "cauldron") modLoaderType = 2;
        else if (loader.toLower() == "liteloader") modLoaderType = 3;
        else if (loader.toLower() == "fabric") modLoaderType = 4;
        else if (loader.toLower() == "quilt") modLoaderType = 5;
        
        if (modLoaderType != 0) {
            cfUrl += "&modLoaderType=" + QString::number(modLoaderType);
        }
    }

    if (!modrinthCategories.isEmpty() && curseforgeCategoryIds.isEmpty()) {
        // The user selected categories, but none of them map to CurseForge.
        // Skip CurseForge search so we don't return unfiltered results.
        m_curseforgeDone = true;
        checkBothFinished();
    } else {
        m_curseForgeJob = new NetJob("Unified::CurseForgeSearch", APPLICATION->network());
        auto dl = Net::Download::makeByteArray(QUrl(cfUrl), &m_curseForgeResponse);
        dl->setHeader("x-api-key", BuildConfig.CURSEFORGE_API_KEY.toUtf8());
        m_curseForgeJob->addNetAction(dl);
        m_curseForgeJob->start();
        connect(m_curseForgeJob.get(), &NetJob::succeeded, this, &UnifiedModModel::curseForgeSearchFinished);
        connect(m_curseForgeJob.get(), &NetJob::failed, this, &UnifiedModModel::curseForgeSearchFailed);
    }
}

void UnifiedModModel::modrinthSearchFinished() {
    m_modrinthJob.reset();
    m_modrinthDone = true;

    QJsonParseError parse_error;
    QJsonDocument doc = QJsonDocument::fromJson(m_modrinthResponse, &parse_error);
    if (parse_error.error == QJsonParseError::NoError) {
        try {
            auto obj = Json::requireObject(doc);
            auto hits = Json::requireArray(obj, "hits");
            for (auto hitRaw : hits) {
                auto hitObj = hitRaw.toObject();
                UnifiedModData mod;
                mod.modrinthId = Json::requireString(hitObj, "project_id");
                mod.name = Json::requireString(hitObj, "title");
                mod.author = Json::requireString(hitObj, "author");
                mod.summary = Json::requireString(hitObj, "description");
                mod.downloadCount = Json::ensureInteger(hitObj, "downloads", 0);
                if (hitObj.contains("icon_url") && hitObj["icon_url"].isString()) {
                    mod.iconUrl = QUrl(hitObj["icon_url"].toString());
                }
                mod.platform = Platform::Modrinth;
                mod.projectType = m_currentSearchType;
                m_pendingModrinth.append(mod);
            }
        } catch (const JSONValidationError &e) {
            qWarning() << "Error parsing Modrinth unified search: " << e.cause();
        }
    } else {
        qWarning() << "Error while parsing JSON response from Modrinth: " << parse_error.errorString();
    }

    checkBothFinished();
}

void UnifiedModModel::modrinthSearchFailed() {
    m_modrinthJob.reset();
    m_modrinthDone = true;
    checkBothFinished();
}

void UnifiedModModel::curseForgeSearchFinished() {
    m_curseForgeJob.reset();
    m_curseforgeDone = true;

    QJsonParseError parse_error;
    QJsonDocument doc = QJsonDocument::fromJson(m_curseForgeResponse, &parse_error);
    if (parse_error.error == QJsonParseError::NoError) {
        try {
            auto obj = Json::requireObject(doc);
            auto dataArray = Json::requireArray(obj, "data");
            for (auto packRaw : dataArray) {
                auto packObj = packRaw.toObject();
                UnifiedModData mod;
                mod.curseforgeId = QString::number(Json::requireInteger(packObj, "id"));
                mod.name = Json::requireString(packObj, "name");
                mod.summary = Json::requireString(packObj, "summary");
                mod.downloadCount = Json::ensureInteger(packObj, "downloadCount", 0);
                
                if (packObj.contains("logo") && packObj["logo"].isObject()) {
                    auto logoObj = packObj["logo"].toObject();
                    auto thumbnailUrl = Json::ensureString(logoObj, "thumbnailUrl", "");
                    if (!thumbnailUrl.isEmpty()) {
                        mod.iconUrl = QUrl(thumbnailUrl);
                    }
                }

                if (packObj.contains("authors") && packObj["authors"].isArray()) {
                    auto authors = packObj["authors"].toArray();
                    if (!authors.isEmpty()) {
                        mod.author = Json::requireString(authors[0].toObject(), "name");
                    }
                }
                mod.platform = Platform::CurseForge;
                m_pendingCurseForge.append(mod);
            }
        } catch (const JSONValidationError &e) {
            qWarning() << "Error parsing CurseForge unified search: " << e.cause();
        }
    } else {
        qWarning() << "Error while parsing JSON response from CurseForge: " << parse_error.errorString();
    }

    checkBothFinished();
}

void UnifiedModModel::curseForgeSearchFailed() {
    m_curseForgeJob.reset();
    m_curseforgeDone = true;
    checkBothFinished();
}

void UnifiedModModel::checkBothFinished() {
    if (!m_modrinthDone || !m_curseforgeDone) {
        return;
    }

    m_allMods.clear();

    // Prioritize Modrinth
    for (const auto& mod : m_pendingModrinth) {
        m_allMods.append(mod);
    }

    // Merge CurseForge
    for (const auto& cfMod : m_pendingCurseForge) {
        bool found = false;
        for (auto& existingMod : m_allMods) {
            if (existingMod.name.toLower() == cfMod.name.toLower()) {
                existingMod.platform = Platform::Both;
                existingMod.curseforgeId = cfMod.curseforgeId;
                found = true;
                break;
            }
        }
        if (!found) {
            m_allMods.append(cfMod);
        }
    }

    // Sort by downloads across both platforms
    std::sort(m_allMods.begin(), m_allMods.end(), [](const UnifiedModData& a, const UnifiedModData& b) {
        return a.downloadCount > b.downloadCount;
    });

    setPage(0); // This populates m_mods and emits endResetModel
    emit searchFinished();
}

void UnifiedModModel::fetchLogo(const QString &id, const QUrl &url) {
    if (m_loadingLogos.contains(id) || m_failedLogos.contains(id)) {
        return;
    }
    m_loadingLogos.append(id);

    auto job = new NetJob("Unified Logo " + id, APPLICATION->network());
    QByteArray *data = new QByteArray();
    auto dl = Net::Download::makeByteArray(url, data);
    job->addNetAction(dl);
    
    QObject::connect(job, &NetJob::succeeded, this, [this, id, data]() {
        QBuffer buffer(data);
        buffer.open(QIODevice::ReadOnly);
        QImageReader reader(&buffer);
        if (!reader.canRead()) {
            logoFailed(id);
            delete data;
            return;
        }
        QImage img = reader.read();
        delete data;
        
        if (img.isNull()) {
            logoFailed(id);
            return;
        }
        
        QPixmap pixmap = QPixmap::fromImage(img);
        QIcon icon(pixmap.scaled(64, 64, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
        logoLoaded(id, icon);
    });

    QObject::connect(job, &NetJob::failed, this, [this, id, data]() {
        logoFailed(id);
        delete data;
    });

    job->start();
}

void UnifiedModModel::logoFailed(const QString &id) {
    m_loadingLogos.removeAll(id);
    m_failedLogos.append(id);
}

void UnifiedModModel::logoLoaded(const QString &id, const QIcon &out) {
    m_loadingLogos.removeAll(id);
    m_logoMap.insert(id, out);
    
    // Find index and emit dataChanged
    for (int i = 0; i < m_mods.size(); ++i) {
        QString primaryId = m_mods.at(i).modrinthId.isEmpty() ? m_mods.at(i).curseforgeId : m_mods.at(i).modrinthId;
        if (primaryId == id) {
            emit dataChanged(index(i, 0), index(i, 0), {Qt::DecorationRole});
            break;
        }
    }
}
void UnifiedModModel::fetchModDetails(const UnifiedModData &mod) {
    if (m_detailsJob) m_detailsJob->abort();
    if (m_cfDescJob) m_cfDescJob->abort();
    
    m_currentDetailsMod = mod;
    m_detailsResponse.clear();
    m_cfDescResponse.clear();
    
    m_cfDescDone = false;
    m_cfDetailsDone = false;
    m_pendingCfDetails = UnifiedModDetails();

    // If it's on Modrinth or Both, we prefer Modrinth for details since it has a unified endpoint for HTML and gallery
    if (mod.platform == Platform::Modrinth || mod.platform == Platform::Both) {
        QString url = QString("https://api.modrinth.com/v2/project/%1").arg(mod.modrinthId);
        m_detailsJob = new NetJob("Unified::ModrinthDetails", APPLICATION->network());
        m_detailsJob->addNetAction(Net::Download::makeByteArray(QUrl(url), &m_detailsResponse));
        connect(m_detailsJob.get(), &NetJob::succeeded, this, &UnifiedModModel::modrinthDetailsFinished);
        connect(m_detailsJob.get(), &NetJob::failed, this, [this](){ emit modDetailsFailed(m_currentDetailsMod.modrinthId); });
        m_detailsJob->start();
    } else if (mod.platform == Platform::CurseForge) {
        // Fetch screenshots and metadata
        QString url = QString("https://api.curseforge.com/v1/mods/%1").arg(mod.curseforgeId);
        m_detailsJob = new NetJob("Unified::CurseForgeDetails", APPLICATION->network());
        auto dl = Net::Download::makeByteArray(QUrl(url), &m_detailsResponse);
        dl->setHeader("x-api-key", BuildConfig.CURSEFORGE_API_KEY.toUtf8());
        m_detailsJob->addNetAction(dl);
        connect(m_detailsJob.get(), &NetJob::succeeded, this, &UnifiedModModel::curseForgeDetailsFinished);
        connect(m_detailsJob.get(), &NetJob::failed, this, [this](){ 
            m_cfDetailsDone = true; 
            checkCurseForgeDetailsFinished(); 
        });
        m_detailsJob->start();
        
        // Fetch HTML description
        QString descUrl = QString("https://api.curseforge.com/v1/mods/%1/description").arg(mod.curseforgeId);
        m_cfDescJob = new NetJob("Unified::CurseForgeDesc", APPLICATION->network());
        auto dlDesc = Net::Download::makeByteArray(QUrl(descUrl), &m_cfDescResponse);
        dlDesc->setHeader("x-api-key", BuildConfig.CURSEFORGE_API_KEY.toUtf8());
        m_cfDescJob->addNetAction(dlDesc);
        connect(m_cfDescJob.get(), &NetJob::succeeded, this, &UnifiedModModel::curseForgeDescFinished);
        connect(m_cfDescJob.get(), &NetJob::failed, this, [this](){ 
            m_cfDescDone = true; 
            checkCurseForgeDetailsFinished(); 
        });
        m_cfDescJob->start();
    }
}

void UnifiedModModel::modrinthDetailsFinished() {
    m_detailsJob.reset();
    QJsonParseError parse_error;
    QJsonDocument doc = QJsonDocument::fromJson(m_detailsResponse, &parse_error);
    if (parse_error.error == QJsonParseError::NoError) {
        try {
            auto obj = Json::requireObject(doc);
            UnifiedModDetails details;
            details.modrinthId = m_currentDetailsMod.modrinthId;
            details.curseforgeId = m_currentDetailsMod.curseforgeId;
            details.fullDescriptionHtml = Json::ensureString(obj, "body", "");
            
            if (obj.contains("gallery") && obj["gallery"].isArray()) {
                auto gallery = obj["gallery"].toArray();
                for (auto itemRaw : gallery) {
                    auto itemObj = itemRaw.toObject();
                    QString url = Json::ensureString(itemObj, "url", "");
                    if (!url.isEmpty()) details.screenshots.append(QUrl(url));
                }
            }
            emit modDetailsFetched(details);
            return;
        } catch (const JSONValidationError &e) {
            qWarning() << "Error parsing Modrinth details: " << e.cause();
        }
    }
    emit modDetailsFailed(m_currentDetailsMod.modrinthId);
}

void UnifiedModModel::curseForgeDetailsFinished() {
    m_cfDetailsDone = true;
    QJsonParseError parse_error;
    QJsonDocument doc = QJsonDocument::fromJson(m_detailsResponse, &parse_error);
    if (parse_error.error == QJsonParseError::NoError) {
        try {
            auto rootObj = Json::requireObject(doc);
            auto obj = Json::requireObject(rootObj, "data");
            m_pendingCfDetails.modrinthId = m_currentDetailsMod.modrinthId;
            m_pendingCfDetails.curseforgeId = m_currentDetailsMod.curseforgeId;
            
            if (obj.contains("screenshots") && obj["screenshots"].isArray()) {
                auto gallery = obj["screenshots"].toArray();
                for (auto itemRaw : gallery) {
                    auto itemObj = itemRaw.toObject();
                    QString url = Json::ensureString(itemObj, "url", "");
                    if (!url.isEmpty()) m_pendingCfDetails.screenshots.append(QUrl(url));
                }
            }
        } catch (const JSONValidationError &e) {
            qWarning() << "Error parsing CurseForge details: " << e.cause();
        }
    }
    checkCurseForgeDetailsFinished();
}

void UnifiedModModel::curseForgeDescFinished() {
    m_cfDescJob.reset();
    m_cfDescDone = true;
    QJsonParseError parse_error;
    QJsonDocument doc = QJsonDocument::fromJson(m_cfDescResponse, &parse_error);
    if (parse_error.error == QJsonParseError::NoError) {
        try {
            auto obj = Json::requireObject(doc);
            m_pendingCfDetails.fullDescriptionHtml = Json::requireString(obj, "data");
        } catch (const JSONValidationError &e) {
            qWarning() << "Error parsing CurseForge description: " << e.cause();
        }
    }
    checkCurseForgeDetailsFinished();
}

void UnifiedModModel::checkCurseForgeDetailsFinished() {
    if (!m_cfDetailsDone || !m_cfDescDone) {
        return;
    }
    
    if (m_pendingCfDetails.fullDescriptionHtml.isEmpty() && m_pendingCfDetails.screenshots.isEmpty()) {
        // Both failed or returned empty
        emit modDetailsFailed(m_currentDetailsMod.curseforgeId);
    } else {
        emit modDetailsFetched(m_pendingCfDetails);
    }
}

void UnifiedModModel::fetchModVersions(const UnifiedModData &mod, const QString &mcVersion, const QString &loader) {
    if (m_mrVersionsJob) m_mrVersionsJob->abort();
    if (m_cfVersionsJob) m_cfVersionsJob->abort();
    
    m_currentVersionsMod = mod;
    m_currentVersionsMcVersion = mcVersion;
    m_currentVersionsLoader = loader;
    
    m_mrVersionsResponse.clear();
    m_cfVersionsResponse.clear();
    
    m_mrVersionsDone = false;
    m_cfVersionsDone = false;
    m_pendingVersions.clear();

    if (mod.platform == Platform::Modrinth || mod.platform == Platform::Both) {
        QString url = QString("https://api.modrinth.com/v2/project/%1/version").arg(mod.modrinthId);
        QStringList params;
        if (!mcVersion.isEmpty()) {
            params.append("game_versions=[\"" + QUrl::toPercentEncoding(mcVersion) + "\"]");
        }
        if (!loader.isEmpty() && mod.projectType == ProjectType::Mod) {
            // Include datapack so datapack-based mods like Terralith still appear
            params.append("loaders=[\"" + loader.toLower() + "\", \"datapack\"]");
        }
        if (!params.isEmpty()) {
            url += "?" + params.join("&");
        }

        m_mrVersionsJob = new NetJob("Unified::ModrinthVersions", APPLICATION->network());
        m_mrVersionsJob->addNetAction(Net::Download::makeByteArray(QUrl(url), &m_mrVersionsResponse));
        connect(m_mrVersionsJob.get(), &NetJob::succeeded, this, &UnifiedModModel::modrinthVersionsFinished);
        connect(m_mrVersionsJob.get(), &NetJob::failed, this, [this](){ 
            m_mrVersionsDone = true; 
            checkVersionsFinished(); 
        });
        m_mrVersionsJob->start();
    } else {
        m_mrVersionsDone = true;
    }
    
    if (mod.platform == Platform::CurseForge || mod.platform == Platform::Both) {
        QString url = QString("https://api.curseforge.com/v1/mods/%1/files").arg(mod.curseforgeId);
        QStringList params;
        if (!mcVersion.isEmpty()) {
            params.append("gameVersion=" + QUrl::toPercentEncoding(mcVersion));
        }
        if (!loader.isEmpty() && mod.projectType == ProjectType::Mod) {
            int modLoaderType = 0;
            if (loader.toLower() == "forge") modLoaderType = 1;
            else if (loader.toLower() == "cauldron") modLoaderType = 2;
            else if (loader.toLower() == "liteloader") modLoaderType = 3;
            else if (loader.toLower() == "fabric") modLoaderType = 4;
            else if (loader.toLower() == "quilt") modLoaderType = 5;
            
            if (modLoaderType != 0) {
                params.append("modLoaderType=" + QString::number(modLoaderType));
            }
        }
        if (!params.isEmpty()) {
            url += "?" + params.join("&");
        }

        m_cfVersionsJob = new NetJob("Unified::CurseForgeVersions", APPLICATION->network());
        auto dl = Net::Download::makeByteArray(QUrl(url), &m_cfVersionsResponse);
        dl->setHeader("x-api-key", BuildConfig.CURSEFORGE_API_KEY.toUtf8());
        m_cfVersionsJob->addNetAction(dl);
        connect(m_cfVersionsJob.get(), &NetJob::succeeded, this, &UnifiedModModel::curseForgeVersionsFinished);
        connect(m_cfVersionsJob.get(), &NetJob::failed, this, [this](){ 
            m_cfVersionsDone = true; 
            checkVersionsFinished(); 
        });
        m_cfVersionsJob->start();
    } else {
        m_cfVersionsDone = true;
    }
}

void UnifiedModModel::modrinthVersionsFinished() {
    m_mrVersionsJob.reset();
    m_mrVersionsDone = true;
    QJsonParseError parse_error;
    QJsonDocument doc = QJsonDocument::fromJson(m_mrVersionsResponse, &parse_error);
    if (parse_error.error == QJsonParseError::NoError) {
        try {
            auto arr = Json::requireArray(doc);
            for (auto itemRaw : arr) {
                auto itemObj = itemRaw.toObject();
                UnifiedModVersion v;
                v.modrinthId = Json::requireString(itemObj, "id");
                v.name = Json::requireString(itemObj, "name");
                v.datePublished = Json::requireDateTime(itemObj, "date_published");
                v.releaseType = Json::requireString(itemObj, "version_type");
                
                auto gvArr = Json::requireArray(itemObj, "game_versions");
                for (auto gv : gvArr) {
                    v.gameVersions.append(gv.toString());
                }

                auto filesArr = Json::requireArray(itemObj, "files");
                for (auto fileRaw : filesArr) {
                    auto fileObj = fileRaw.toObject();
                    if (Json::ensureBoolean(fileObj, "primary", false) || v.downloadUrl.isEmpty()) {
                        v.fileName = Json::requireString(fileObj, "filename");
                        v.downloadUrl = Json::requireString(fileObj, "url");
                        v.fileSize = Json::ensureInteger(fileObj, "size", 0);
                    }
                }
                if (!v.downloadUrl.isEmpty()) {
                    m_pendingVersions.append(v);
                }
            }
        } catch (const JSONValidationError &e) {
            qWarning() << "Error parsing Modrinth versions: " << e.cause();
        }
    }
    checkVersionsFinished();
}

void UnifiedModModel::curseForgeVersionsFinished() {
    m_cfVersionsJob.reset();
    m_cfVersionsDone = true;
    QJsonParseError parse_error;
    QJsonDocument doc = QJsonDocument::fromJson(m_cfVersionsResponse, &parse_error);
    if (parse_error.error == QJsonParseError::NoError) {
        try {
            auto rootObj = Json::requireObject(doc);
            auto dataArr = Json::requireArray(rootObj, "data");
            
            for (auto itemRaw : dataArr) {
                auto itemObj = itemRaw.toObject();
                UnifiedModVersion v;
                v.curseforgeId = QString::number(Json::requireInteger(itemObj, "id"));
                v.name = Json::requireString(itemObj, "displayName");
                v.fileName = Json::requireString(itemObj, "fileName");
                v.downloadUrl = Json::ensureString(itemObj, "downloadUrl", "");
                v.datePublished = Json::requireDateTime(itemObj, "fileDate");
                v.fileSize = Json::ensureInteger(itemObj, "fileLength", 0);
                
                int releaseTypeInt = Json::ensureInteger(itemObj, "releaseType", 1);
                if (releaseTypeInt == 1) v.releaseType = "release";
                else if (releaseTypeInt == 2) v.releaseType = "beta";
                else if (releaseTypeInt == 3) v.releaseType = "alpha";
                else v.releaseType = "unknown";

                if (itemObj.contains("gameVersions") && itemObj["gameVersions"].isArray()) {
                    for (auto gv : itemObj["gameVersions"].toArray()) {
                        v.gameVersions.append(gv.toString());
                    }
                }

                if (!v.downloadUrl.isEmpty()) {
                    m_pendingVersions.append(v);
                }
            }
        } catch (const JSONValidationError &e) {
            qWarning() << "Error parsing CurseForge versions: " << e.cause();
        }
    }
    checkVersionsFinished();
}

void UnifiedModModel::checkVersionsFinished() {
    if (!m_mrVersionsDone || !m_cfVersionsDone) return;
    
    if (m_pendingVersions.isEmpty()) {
        QString primaryId = m_currentVersionsMod.modrinthId.isEmpty() ? m_currentVersionsMod.curseforgeId : m_currentVersionsMod.modrinthId;
        emit modVersionsFailed(primaryId);
        return;
    }
    
    // Deduplicate versions based on fileName
    QList<UnifiedModVersion> uniqueVersions;
    for (const auto& v : m_pendingVersions) {
        bool found = false;
        for (const auto& existing : uniqueVersions) {
            if (existing.fileName == v.fileName || existing.fileSize == v.fileSize) {
                found = true;
                break;
            }
        }
        if (!found) {
            uniqueVersions.append(v);
        }
    }
    
    // Sort descending by date
    std::sort(uniqueVersions.begin(), uniqueVersions.end(), [](const UnifiedModVersion& a, const UnifiedModVersion& b) {
        return a.datePublished > b.datePublished;
    });

    emit modVersionsFetched(uniqueVersions);
}

}
