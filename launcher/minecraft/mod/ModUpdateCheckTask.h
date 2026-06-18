#pragma once

#include <QObject>
#include <QRunnable>
#include <QByteArray>
#include <QNetworkReply>
#include <QMap>
#include <QString>
#include <QList>

class BaseInstance;
class ModFolderModel;

class ModUpdateCheckTask : public QObject
{
    Q_OBJECT
public:
    explicit ModUpdateCheckTask(ModFolderModel *model, BaseInstance *inst, QObject *parent = nullptr);
    ~ModUpdateCheckTask();

    void start();

signals:
    void finished();

private slots:
    void onReplyFinished();

private:
    ModFolderModel *m_model;
    BaseInstance *m_inst;
    QNetworkReply *m_reply = nullptr;
    QMap<QString, int> m_hashToRow; // Map sha1 hex string to model row index
};
