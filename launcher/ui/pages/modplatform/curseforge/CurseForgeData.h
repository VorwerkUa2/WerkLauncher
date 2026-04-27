#pragma once

#include <QString>
#include <QMetaType>
#include <QUrl>
#include <QDateTime>
#include <QVector>

namespace CurseForge {

enum class LoadState {
    NotLoaded = 0,
    Loaded = 1,
    Errored = 2
};

enum class FileReleaseType {
    Release = 1,
    Beta = 2,
    Alpha = 3,
    Unknown = 0
};

struct ModFile {
    int id = 0;
    QString displayName;
    QString fileName;
    QString downloadUrl;
    QDateTime fileDate;
    FileReleaseType releaseType = FileReleaseType::Unknown;
    QStringList gameVersions;
};

struct Modpack {
    int id = 0;

    QString name;
    QUrl iconUrl;
    QString author;
    QString summary;
    int downloadCount = 0;

    LoadState detailsLoaded = LoadState::NotLoaded;
    QString description; // HTML

    LoadState filesLoaded = LoadState::NotLoaded;
    QVector<ModFile> files;
};

}

Q_DECLARE_METATYPE(CurseForge::ModFile)
Q_DECLARE_METATYPE(CurseForge::Modpack)
