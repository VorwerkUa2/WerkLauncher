#pragma once

#include <QDateTime>
#include <QString>
#include <QStringList>
#include <QUrl>
#include <QMetaType>

namespace Unified {

enum class Platform {
    Modrinth,
    CurseForge,
    Both
};

enum class ProjectType {
    Mod,
    ResourcePack,
    ShaderPack,
    DataPack
};

struct UnifiedModData {
    QString modrinthId;
    QString curseforgeId;
    QString name;
    QUrl iconUrl;
    QString author;
    QString summary;
    qulonglong downloadCount = 0;
    Platform platform = Platform::Modrinth;
    ProjectType projectType = ProjectType::Mod;
};

struct UnifiedModDetails {
    QString modrinthId;
    QString curseforgeId;
    QString fullDescriptionHtml;
    QVector<QUrl> screenshots;
    // We will add versions later
};

struct UnifiedModVersion {
    QString modrinthId;
    QString curseforgeId;
    QString name;
    QString fileName;
    QString downloadUrl;
    QDateTime datePublished;
    qulonglong fileSize = 0;
    QString releaseType;  // release, beta, alpha
    QStringList gameVersions;
};

}

Q_DECLARE_METATYPE(Unified::UnifiedModData)
Q_DECLARE_METATYPE(Unified::UnifiedModDetails)
Q_DECLARE_METATYPE(Unified::UnifiedModVersion)
