#pragma once

#include "tasks/Task.h"
#include "minecraft/mod/Mod.h"
#include "net/NetJob.h"
#include <QList>
#include <QString>

class BaseInstance;
class ModFolderModel;

class ModUpdateInstallTask : public Task
{
    Q_OBJECT
public:
    explicit ModUpdateInstallTask(BaseInstance *inst, ModFolderModel *model, const QList<Mod> &mods, QObject *parent = nullptr);
    virtual ~ModUpdateInstallTask();

protected:
    virtual void executeTask() override;

private slots:
    void onNetJobSucceeded();
    void onNetJobFailed(QString reason);
    void onNetJobProgress(qint64 current, qint64 total);

private:
    BaseInstance *m_inst;
    ModFolderModel *m_model;
    QList<Mod> m_mods;
    NetJob::Ptr m_job;
    
    // Map temporary paths back to Mod objects
    QMap<QString, Mod> m_tempToMod;
};
