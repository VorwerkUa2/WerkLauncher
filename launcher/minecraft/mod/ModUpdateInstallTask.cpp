#include "ModUpdateInstallTask.h"
#include "minecraft/mod/ModFolderModel.h"
#include "BaseInstance.h"
#include "Application.h"
#include "FileSystem.h"

#include <QDir>
#include <QFile>
#include <QUrl>

ModUpdateInstallTask::ModUpdateInstallTask(BaseInstance *inst, ModFolderModel *model, const QList<Mod> &mods, QObject *parent)
    : Task(parent), m_inst(inst), m_model(model), m_mods(mods)
{
    setObjectName("ModUpdateInstallTask");
}

ModUpdateInstallTask::~ModUpdateInstallTask()
{
}

void ModUpdateInstallTask::executeTask()
{
    if (m_mods.isEmpty()) {
        emitSucceeded();
        return;
    }

    setStatus(tr("Downloading mod updates..."));
    
    m_job.reset(new NetJob(tr("Mod Updates"), APPLICATION->network()));

    QString modsDir = m_model->dir().absolutePath();

    for (const Mod &mod : m_mods) {
        if (!mod.hasUpdate() || mod.updateUrl().isEmpty() || mod.updateFileName().isEmpty()) {
            continue;
        }

        QString targetTempPath = FS::PathCombine(modsDir, mod.updateFileName() + ".tmp");
        m_tempToMod[targetTempPath] = mod;

        auto action = Net::Download::makeFile(QUrl(mod.updateUrl()), targetTempPath);
        m_job->addNetAction(action);
    }

    if (m_job->size() == 0) {
        emitSucceeded();
        return;
    }

    connect(m_job.get(), &NetJob::succeeded, this, &ModUpdateInstallTask::onNetJobSucceeded);
    connect(m_job.get(), &NetJob::failed, this, &ModUpdateInstallTask::onNetJobFailed);
    connect(m_job.get(), &NetJob::progress, this, &ModUpdateInstallTask::onNetJobProgress);

    m_job->start();
}

void ModUpdateInstallTask::onNetJobProgress(qint64 current, qint64 total)
{
    setProgress(current, total);
}

void ModUpdateInstallTask::onNetJobSucceeded()
{
    setStatus(tr("Installing updates..."));
    
    // Now replace the old files with the newly downloaded ones
    for (auto it = m_tempToMod.begin(); it != m_tempToMod.end(); ++it) {
        QString tempPath = it.key();
        Mod oldMod = it.value();
        QString oldPath = oldMod.filename().absoluteFilePath();
        QString finalPath = FS::PathCombine(m_model->dir().absolutePath(), oldMod.updateFileName());
        
        // Remove old file
        if (QFile::exists(oldPath)) {
            QFile::remove(oldPath);
        }
        
        // Sometimes the new filename is exactly the same as the old filename
        // If not, we rename it. If it is, QFile::rename will fail if target exists (it shouldn't now)
        if (QFile::exists(finalPath)) {
            QFile::remove(finalPath);
        }
        
        QFile::rename(tempPath, finalPath);
    }
    
    // Reload model
    m_model->update();
    m_model->startWatching();

    emitSucceeded();
}

void ModUpdateInstallTask::onNetJobFailed(QString reason)
{
    // Clean up temporary files
    for (auto it = m_tempToMod.begin(); it != m_tempToMod.end(); ++it) {
        QFile::remove(it.key());
    }
    
    emitFailed(reason);
}
