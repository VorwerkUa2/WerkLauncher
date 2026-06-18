#include "ModUpdateCheckTask.h"
#include "ModFolderModel.h"
#include "minecraft/MinecraftInstance.h"
#include "minecraft/PackProfile.h"
#include "Application.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkRequest>

ModUpdateCheckTask::ModUpdateCheckTask(ModFolderModel *model, BaseInstance *inst, QObject *parent)
    : QObject(parent), m_model(model), m_inst(inst)
{
}

ModUpdateCheckTask::~ModUpdateCheckTask()
{
    if (m_reply) {
        m_reply->deleteLater();
    }
}

void ModUpdateCheckTask::start()
{
    if (!m_model || !m_inst) {
        emit finished();
        return;
    }

    auto mcInst = dynamic_cast<MinecraftInstance*>(m_inst);
    if (!mcInst) {
        emit finished();
        return;
    }

    auto profile = mcInst->getPackProfile();
    if (!profile) {
        emit finished();
        return;
    }

    QString mcVersion;
    auto mcComp = profile->getComponent("net.minecraft");
    if (mcComp) {
        mcVersion = mcComp->getVersion();
    }

    QString loaderType;
    if (profile->getComponent("net.fabricmc.fabric-loader")) loaderType = "fabric";
    else if (profile->getComponent("net.minecraftforge")) loaderType = "forge";
    else if (profile->getComponent("org.quiltmc.quilt-loader")) loaderType = "quilt";

    if (mcVersion.isEmpty()) {
        emit finished();
        return;
    }

    QJsonArray hashesArray;
    for (int i = 0; i < m_model->rowCount(); ++i) {
        auto mod = m_model->at(i);
        QString sha1 = mod.sha1();
        if (!sha1.isEmpty()) {
            hashesArray.append(sha1);
            m_hashToRow[sha1] = i;
        }
    }

    if (hashesArray.isEmpty()) {
        emit finished();
        return;
    }

    QJsonObject payload;
    payload["hashes"] = hashesArray;
    payload["algorithm"] = "sha1";
    
    QJsonArray gameVersions;
    gameVersions.append(mcVersion);
    payload["game_versions"] = gameVersions;

    if (!loaderType.isEmpty()) {
        QJsonArray loaders;
        loaders.append(loaderType);
        payload["loaders"] = loaders;
    }

    QNetworkRequest request(QUrl("https://api.modrinth.com/v2/version_files/update"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QByteArray data = QJsonDocument(payload).toJson();
    m_reply = APPLICATION->network()->post(request, data);
    connect(m_reply, &QNetworkReply::finished, this, &ModUpdateCheckTask::onReplyFinished);
}

void ModUpdateCheckTask::onReplyFinished()
{
    if (m_reply->error() == QNetworkReply::NoError) {
        QByteArray data = m_reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (doc.isObject()) {
            QJsonObject root = doc.object();
            for (auto it = root.begin(); it != root.end(); ++it) {
                QString oldHash = it.key();
                QJsonObject versionObj = it.value().toObject();
                
                if (m_hashToRow.contains(oldHash)) {
                    int row = m_hashToRow[oldHash];
                    auto mod = m_model->at(row);
                    
                    QJsonArray files = versionObj["files"].toArray();
                    QString newHash;
                    for (int i = 0; i < files.size(); ++i) {
                        auto fileObj = files[i].toObject();
                        if (fileObj["primary"].toBool()) {
                            newHash = fileObj["hashes"].toObject()["sha1"].toString();
                            break;
                        }
                    }
                    if (newHash.isEmpty() && files.size() > 0) {
                        newHash = files[0].toObject()["hashes"].toObject()["sha1"].toString();
                    }
                    
                    if (!newHash.isEmpty() && newHash != oldHash) {
                        mod.setHasUpdate(true);
                        mod.setLatestVersion(versionObj["version_number"].toString());
                        
                        m_model->updateMod(row, mod);
                    }
                }
            }
        }
    }
    m_reply->deleteLater();
    m_reply = nullptr;
    emit finished();
}
