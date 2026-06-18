#include "ModDownloaderDialog.h"
#include "ui_ModDownloaderDialog.h"
#include <QRegularExpression>
#include <QTextDocument>
#include <QImageReader>
#include <QBuffer>
#include "net/NetJob.h"
#include "Application.h"
#include <QMessageBox>
#include <QDir>
#include "FileSystem.h"

#include "ui/pages/modplatform/unified/UnifiedModModel.h"
#include "ui/pages/modplatform/unified/UnifiedModData.h"
#include "ui/widgets/CustomTitleBar.h"
#include "BuildConfig.h"

#include "BaseInstance.h"
#include "minecraft/MinecraftInstance.h"
#include "minecraft/PackProfile.h"
#include "ui/pages/modplatform/unified/ModListDelegate.h"

ModDownloaderDialog::ModDownloaderDialog(BaseInstance *inst, Unified::ProjectType projectType, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ModDownloaderDialog),
    m_model(new Unified::UnifiedModModel(this)),
    m_inst(inst),
    m_projectType(projectType)
{
    setWindowFlags(Qt::FramelessWindowHint | windowFlags());
    ui->setupUi(this);

    auto titleBar = new CustomTitleBar(this);
    QString titleType = "Mods";
    if (m_projectType == Unified::ProjectType::ResourcePack) titleType = "Resource Packs";
    else if (m_projectType == Unified::ProjectType::ShaderPack) titleType = "Shaders";
    
    titleBar->setTitle(tr("Download %1 - %2").arg(titleType, BuildConfig.LAUNCHER_NAME));
    this->setWindowTitle(tr("Download %1").arg(titleType));
    
    ui->verticalLayout->setContentsMargins(0, 0, 0, 0);
    ui->verticalLayout->setSpacing(0);
    ui->verticalLayout->insertWidget(0, titleBar);

    setupUi();

    QObject::connect(ui->searchBtn, &QPushButton::clicked, this, [this](){ m_currentPage = 0; performSearch(); });
    QObject::connect(ui->searchBox, &QLineEdit::returnPressed, this, [this](){ m_currentPage = 0; performSearch(); });
    QObject::connect(ui->platformFilter, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](){ m_currentPage = 0; performSearch(); });
    QObject::connect(ui->resultsList, &QListView::clicked, this, &ModDownloaderDialog::onModSelected);
    QObject::connect(ui->backBtn, &QPushButton::clicked, this, &ModDownloaderDialog::onBackClicked);
    QObject::connect(ui->categoryList, &QListWidget::itemClicked, this, [this](QListWidgetItem *item){
        ui->categoryList->blockSignals(true);
        if (ui->categoryList->row(item) == 0) {
            if (item->isSelected()) {
                for (int i = 1; i < ui->categoryList->count(); ++i) {
                    ui->categoryList->item(i)->setSelected(false);
                }
            }
        } else {
            if (item->isSelected()) {
                ui->categoryList->item(0)->setSelected(false);
            }
        }
        
        if (ui->categoryList->selectedItems().isEmpty()) {
            ui->categoryList->item(0)->setSelected(true);
        }
        ui->categoryList->blockSignals(false);
        
        m_currentPage = 0; 
        performSearch(); 
    });
    QObject::connect(ui->installBtn, &QPushButton::clicked, this, &ModDownloaderDialog::onInstallClicked);
    
    QObject::connect(ui->prevPageBtn, &QPushButton::clicked, this, &ModDownloaderDialog::onPrevPage);
    QObject::connect(ui->nextPageBtn, &QPushButton::clicked, this, &ModDownloaderDialog::onNextPage);
    
    connect(m_model, &Unified::UnifiedModModel::modDetailsFetched, this, &ModDownloaderDialog::onModDetailsFetched);
    connect(m_model, &Unified::UnifiedModModel::modDetailsFailed, this, &ModDownloaderDialog::onModDetailsFailed);
    connect(m_model, &Unified::UnifiedModModel::modVersionsFetched, this, &ModDownloaderDialog::onModVersionsFetched);
    connect(m_model, &Unified::UnifiedModModel::modVersionsFailed, this, &ModDownloaderDialog::onModVersionsFailed);
    
    connect(m_model, &Unified::UnifiedModModel::searchFinished, this, [this](){
        ui->nextPageBtn->setEnabled(m_model->hasMorePages());
        ui->pageLabel->setText(tr("Сторінка %1").arg(m_currentPage + 1));
        ui->prevPageBtn->setEnabled(m_currentPage > 0);
    });
}

ModDownloaderDialog::~ModDownloaderDialog() {
    delete ui;
}

void ModDownloaderDialog::loadHtmlImages(const QString &html) {
    QRegularExpression regex("<img[^>]+src=[\"'](https?://[^\"']+)[\"']");
    QRegularExpressionMatchIterator i = regex.globalMatch(html);
    while (i.hasNext()) {
        QRegularExpressionMatch match = i.next();
        QString urlStr = match.captured(1);
        QUrl url(urlStr);
        
        if (m_downloadedImages.contains(urlStr)) {
            continue;
        }
        
        m_downloadedImages.insert(urlStr);
        
        auto job = new NetJob("HTML Image " + urlStr, APPLICATION->network());
        QByteArray *data = new QByteArray();
        auto dl = Net::Download::makeByteArray(url, data);
        job->addNetAction(dl);
        
        QObject::connect(job, &NetJob::succeeded, this, [this, urlStr, data]() {
            QImage img;
            if (img.loadFromData(*data)) {
                int maxWidth = ui->descriptionText->viewport()->width() - 40;
                if (maxWidth > 0 && img.width() > maxWidth) {
                    img = img.scaledToWidth(maxWidth, Qt::SmoothTransformation);
                }
                ui->descriptionText->document()->addResource(QTextDocument::ImageResource, QUrl(urlStr), img);
                // Trigger re-layout
                ui->descriptionText->setLineWrapColumnOrWidth(ui->descriptionText->lineWrapColumnOrWidth());
            }
            delete data;
        });
        
        QObject::connect(job, &NetJob::failed, this, [data]() {
            delete data;
        });
        
        job->start();
    }
}

void ModDownloaderDialog::setupUi() {
    ui->resultsList->setModel(m_model);
    ui->resultsList->setItemDelegate(new Unified::ModListDelegate(this));
    
    ui->descriptionText->setOpenExternalLinks(true);
    
    // Set up Categories
    ui->categoryList->clear();
    QListWidgetItem* allItem = new QListWidgetItem(tr("Усі категорії"));
    allItem->setData(Qt::UserRole, ""); // empty means all
    ui->categoryList->addItem(allItem);
    
    struct CatDef { QString name; QString modrinthId; int curseforgeId; };
    QList<CatDef> cats;
    
    if (m_projectType == Unified::ProjectType::Mod) {
        cats = {
            {tr("Пригоди і RPG"), "adventure", 422},
            {tr("Декорації"), "decoration", 424},
            {tr("Спорядження"), "equipment", 434},
            {tr("Бібліотеки та API"), "library", 421},
            {tr("Магія"), "magic", 419},
            {tr("Моби"), "mobs", 411},
            {tr("Оптимізація"), "optimization", 6814},
            {tr("Сховища"), "storage", 420},
            {tr("Технології"), "technology", 412},
            {tr("Утиліти"), "utility", 5191},
            {tr("Генерація світу"), "worldgen", 406}
        };
    } else if (m_projectType == Unified::ProjectType::ResourcePack) {
        cats = {
            {"16x", "16x", 393},
            {"32x", "32x", 394},
            {"64x", "64x", 395},
            {"128x", "128x", 396},
            {"256x", "256x", 397},
            {"3D", "3d", 0},
            {tr("Анімовані"), "animated", 404},
            {tr("Темні"), "dark", 0},
            {tr("Ванільні"), "default-like", 403},
            {tr("Сучасні"), "modern", 401},
            {tr("Реалістичні"), "realistic", 400},
            {tr("Тематичні"), "themed", 0}
        };
    } else if (m_projectType == Unified::ProjectType::ShaderPack) {
        cats = {
            {tr("Кінематографічні"), "cinematic", 0},
            {tr("Фентезі"), "fantasy", 6554},
            {tr("Хардкорні"), "hardcore", 0},
            {tr("Висококонтрастні"), "high-contrast", 0},
            {tr("Контурні"), "outline", 0},
            {tr("Пастельні"), "pastel", 0},
            {tr("Фотореалістичні"), "photorealistic", 6553},
            {tr("Ретро"), "retro", 0},
            {tr("Ванільні"), "vanilla-like", 6555}
        };
    }
    
    ui->categoryList->setSelectionMode(QAbstractItemView::MultiSelection);
    
    for (const auto& cat : cats) {
        QListWidgetItem* item = new QListWidgetItem(cat.name);
        item->setData(Qt::UserRole, cat.modrinthId);
        item->setData(Qt::UserRole + 1, cat.curseforgeId);
        ui->categoryList->addItem(item);
    }
    ui->categoryList->setCurrentRow(0);

    showList();
    // Trigger initial search to populate the list
    m_currentPage = 0;
    performSearch();
}

void ModDownloaderDialog::getGameVersionAndLoader(QString &mcVersion, QString &loader) {
    if (auto mcInst = dynamic_cast<MinecraftInstance*>(m_inst)) {
        auto profile = mcInst->getPackProfile();
        if (profile) {
            mcVersion = profile->getComponentVersion("net.minecraft");
            if (!profile->getComponentVersion("net.fabricmc.fabric-loader").isEmpty()) {
                loader = "fabric";
            } else if (!profile->getComponentVersion("net.minecraftforge").isEmpty()) {
                loader = "forge";
            } else if (!profile->getComponentVersion("org.quiltmc.quilt-loader").isEmpty()) {
                loader = "quilt";
            }
        }
    }
}

void ModDownloaderDialog::onSearchTriggered() {
    m_currentPage = 0;
    performSearch();
}

void ModDownloaderDialog::performSearch() {
    ui->pageLabel->setText(tr("Сторінка %1").arg(m_currentPage + 1));
    ui->prevPageBtn->setEnabled(m_currentPage > 0);
    ui->nextPageBtn->setEnabled(false); // will be enabled after search finishes if results > 0
    
    QString term = ui->searchBox->text();
    QString mcVersion;
    QString loader;
    
    getGameVersionAndLoader(mcVersion, loader);
    
    QStringList modrinthCategories;
    QList<int> curseforgeCategories;
    
    for (auto item : ui->categoryList->selectedItems()) {
        QString mCat = item->data(Qt::UserRole).toString();
        if (!mCat.isEmpty()) modrinthCategories.append(mCat);
        
        int cCat = item->data(Qt::UserRole + 1).toInt();
        if (cCat > 0) curseforgeCategories.append(cCat);
    }
    
    m_model->search(term, m_projectType, mcVersion, loader, modrinthCategories, curseforgeCategories);
}

void ModDownloaderDialog::onPrevPage() {
    if (m_currentPage > 0) {
        m_currentPage--;
        m_model->setPage(m_currentPage);
        ui->pageLabel->setText(tr("Сторінка %1").arg(m_currentPage + 1));
        ui->prevPageBtn->setEnabled(m_currentPage > 0);
        ui->nextPageBtn->setEnabled(m_model->hasMorePages());
    }
}

void ModDownloaderDialog::onNextPage() {
    if (m_model->hasMorePages()) {
        m_currentPage++;
        m_model->setPage(m_currentPage);
        ui->pageLabel->setText(tr("Сторінка %1").arg(m_currentPage + 1));
        ui->prevPageBtn->setEnabled(m_currentPage > 0);
        ui->nextPageBtn->setEnabled(m_model->hasMorePages());
    }
}

void ModDownloaderDialog::onModSelected(const QModelIndex &index) {
    if (!index.isValid()) return;
    
    QVariant dataVar = index.data(Qt::UserRole);
    if (!dataVar.canConvert<Unified::UnifiedModData>()) return;
    
    Unified::UnifiedModData mod = dataVar.value<Unified::UnifiedModData>();
    
    ui->modTitle->setText(mod.name);
    ui->modAuthor->setText(QString("by %1").arg(mod.author));
    ui->descriptionText->setText(tr("Завантаження деталей..."));
    
    QIcon icon = index.data(Qt::DecorationRole).value<QIcon>();
    if (!icon.isNull()) {
        ui->modIcon->setPixmap(icon.pixmap(64, 64));
    } else {
        ui->modIcon->clear();
    }
    
    // Fetch full details
    m_model->fetchModDetails(mod);

    QString mcVersion;
    QString loader;
    getGameVersionAndLoader(mcVersion, loader);
    m_model->fetchModVersions(mod, mcVersion, loader);

    showDetails();
}

void ModDownloaderDialog::onModDetailsFetched(const Unified::UnifiedModDetails &details) {
    // We can show the full HTML description
    if (!details.fullDescriptionHtml.isEmpty()) {
        QString styledHtml = QString(
        "<html><head><style>"
        "body { font-family: 'Segoe UI', 'Inter', sans-serif; font-size: 14px; line-height: 1.5; margin: 10px; }"
        "a { color: #4DA6FF; text-decoration: none; }"
        "h1, h2, h3 { margin-top: 15px; margin-bottom: 10px; }"
        "</style></head><body>%1</body></html>"
    ).arg(details.fullDescriptionHtml);
    
    ui->descriptionText->setHtml(styledHtml);
    loadHtmlImages(styledHtml);
    } else {
        // Fallback or leave summary
        ui->descriptionText->setText(tr("Опис недоступний."));
    }
    // Populate gallery
    QLayoutItem *child;
    while ((child = ui->galleryLayout->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }

    int col = 0;
    int row = 0;
    for (const QUrl &url : details.screenshots) {
        QLabel *imgLabel = new QLabel(this);
        imgLabel->setText(tr("Loading..."));
        imgLabel->setAlignment(Qt::AlignCenter);
        ui->galleryLayout->addWidget(imgLabel, row, col);
        
        QByteArray *data = new QByteArray();
        auto job = new NetJob("Gallery image", APPLICATION->network());
        job->addNetAction(Net::Download::makeByteArray(url, data));
        QObject::connect(job, &NetJob::succeeded, this, [imgLabel, data]() {
            QPixmap pixmap;
            pixmap.loadFromData(*data);
            if (!pixmap.isNull()) {
                imgLabel->setPixmap(pixmap.scaled(400, 400, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            } else {
                imgLabel->setText(tr("Error loading image"));
            }
            delete data;
        });
        QObject::connect(job, &NetJob::failed, this, [imgLabel, data]() {
            imgLabel->setText(tr("Failed to load"));
            delete data;
        });
        job->start();

        col++;
        if (col >= 2) {
            col = 0;
            row++;
        }
    }
}

void ModDownloaderDialog::onModDetailsFailed(const QString &id) {
    ui->descriptionText->setText("Failed to load details.");
}

void ModDownloaderDialog::onBackClicked() {
    showList();
}

void ModDownloaderDialog::showDetails() {
    ui->stackedWidget->setCurrentWidget(ui->detailsPage);
}

void ModDownloaderDialog::showList() {
    ui->stackedWidget->setCurrentWidget(ui->searchPage);
}



void ModDownloaderDialog::onModVersionsFetched(const QList<Unified::UnifiedModVersion> &versions) {
    m_currentVersions = versions;
    ui->versionsTable->setColumnCount(4);
    ui->versionsTable->setRowCount(versions.size());
    for (int i = 0; i < versions.size(); ++i) {
        const auto &v = versions[i];
        ui->versionsTable->setItem(i, 0, new QTableWidgetItem(v.name));
        ui->versionsTable->setItem(i, 1, new QTableWidgetItem(v.releaseType));
        ui->versionsTable->setItem(i, 2, new QTableWidgetItem(QString::number(v.fileSize)));
        ui->versionsTable->setItem(i, 3, new QTableWidgetItem(v.datePublished.toString(Qt::ISODate)));
    }
    ui->versionsTable->resizeColumnsToContents();
}

void ModDownloaderDialog::onModVersionsFailed(const QString &id) {
    ui->versionsTable->setRowCount(1);
    ui->versionsTable->setItem(0, 0, new QTableWidgetItem("Failed to load versions."));
    m_currentVersions.clear();
}

void ModDownloaderDialog::downloadFile(const QString &url, const QString &filename) {
    QString targetPath;
    auto mcInst = dynamic_cast<MinecraftInstance*>(m_inst);
    if (mcInst) {
        if (m_projectType == Unified::ProjectType::Mod) {
            targetPath = mcInst->modsRoot();
        } else if (m_projectType == Unified::ProjectType::ResourcePack) {
            targetPath = mcInst->resourcePacksDir();
        } else if (m_projectType == Unified::ProjectType::ShaderPack) {
            targetPath = FS::PathCombine(mcInst->gameRoot(), "shaderpacks");
        }
    } else {
        QString subDir;
        if (m_projectType == Unified::ProjectType::Mod) {
            subDir = "mods";
        } else if (m_projectType == Unified::ProjectType::ResourcePack) {
            subDir = "resourcepacks";
        } else if (m_projectType == Unified::ProjectType::ShaderPack) {
            subDir = "shaderpacks";
        }
        targetPath = FS::PathCombine(m_inst->instanceRoot(), subDir);
    }
    
    QDir().mkpath(targetPath);
    targetPath = FS::PathCombine(targetPath, filename);

    NetJob *job = new NetJob(tr("Download %1").arg(filename), APPLICATION->network());
    job->addNetAction(Net::Download::makeFile(QUrl(url), targetPath));
    
    QObject::connect(job, &NetJob::succeeded, this, [this, job, filename]() {
        QMessageBox::information(this, tr("Success"), tr("%1 downloaded successfully.").arg(filename));
        job->deleteLater();
    });
    QObject::connect(job, &NetJob::failed, this, [this, job, filename](QString reason) {
        QMessageBox::critical(this, tr("Error"), tr("Failed to download %1: %2").arg(filename, reason));
        job->deleteLater();
    });
    
    job->start();
}

void ModDownloaderDialog::onInstallClicked() {
    int row = ui->versionsTable->currentRow();
    if (row >= 0 && row < m_currentVersions.size()) {
        const auto &v = m_currentVersions[row];
        downloadFile(v.downloadUrl, v.fileName);
    } else {
        QMessageBox::warning(this, tr("Warning"), tr("Please select a version to install."));
    }
}
