#include "CurseForgeModel.h"
#include "CurseForgePage.h"
#include "ui/pages/modplatform/modrinth/ModrinthDocument.h" // reuse document handler
#include "ui/dialogs/NewInstanceDialog.h"
#include "ui/pages/modplatform/ModpackDelegate.h"

#include "ui_CurseForgePage.h"

#include <QKeyEvent>
#include <InstanceImportTask.h>

CurseForgePage::CurseForgePage(NewInstanceDialog *dialog, QWidget *parent) : QWidget(parent), ui(new Ui::CurseForgePage), dialog(dialog)
{
    ui->setupUi(this);
    connect(ui->searchButton, &QPushButton::clicked, this, &CurseForgePage::triggerSearch);
    ui->searchEdit->installEventFilter(this);
    model = new CurseForge::ListModel(this);
    ui->packView->setModel(model);
    ui->packView->setItemDelegate(new ModpackDelegate(this));

    ui->packDescription->hide();
    connect(ui->packView, &QListView::doubleClicked, this, [this](const QModelIndex&) {
        ui->packDescription->show();
    });

    ui->versionSelectionBox->view()->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ui->versionSelectionBox->view()->parentWidget()->setMaximumHeight(300);

    // Sort options: 1=Featured, 2=Popularity, 3=LastUpdated, 4=Name, 5=Author, 6=TotalDownloads
    ui->sortByBox->addItem(tr("Sort by Popularity"), 2);
    ui->sortByBox->addItem(tr("Sort by Last Updated"), 3);
    ui->sortByBox->addItem(tr("Sort by Name"), 4);
    ui->sortByBox->addItem(tr("Sort by Total Downloads"), 6);

    connect(ui->sortByBox, SIGNAL(currentIndexChanged(int)), this, SLOT(triggerSearch()));
    connect(ui->categoryBox, SIGNAL(currentIndexChanged(int)), this, SLOT(triggerSearch()));
    connect(ui->packView->selectionModel(), &QItemSelectionModel::currentChanged, this, &CurseForgePage::onSelectionChanged);
    connect(ui->versionSelectionBox, &QComboBox::currentTextChanged, this, &CurseForgePage::onVersionSelectionChanged);
    connect(model, &CurseForge::ListModel::packDataChanged, this, &CurseForgePage::onPackDataChanged);
    connect(model, &CurseForge::ListModel::categoriesLoaded, this, &CurseForgePage::onCategoriesLoaded);

    ui->categoryBox->addItem(tr("All Categories"), 0);
    model->fetchCategories();
}

CurseForgePage::~CurseForgePage()
{
    delete ui;
}

void CurseForgePage::openedImpl()
{
    BasePage::openedImpl();
    triggerSearch();
}

bool CurseForgePage::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == ui->searchEdit && event->type() == QEvent::KeyPress) {
        auto *keyEvent = reinterpret_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Return) {
            this->triggerSearch();
            keyEvent->accept();
            return true;
        }
    }
    return QObject::eventFilter(watched, event);
}

void CurseForgePage::triggerSearch() {
    int categoryId = ui->categoryBox->currentData().toInt();
    model->searchWithTerm(ui->searchEdit->text(), ui->sortByBox->currentData().toInt(), categoryId);
}

void CurseForgePage::onCategoriesLoaded() {
    // Preserve current selection if possible
    int currentId = ui->categoryBox->currentData().toInt();
    
    ui->categoryBox->blockSignals(true);
    ui->categoryBox->clear();
    ui->categoryBox->addItem(tr("All Categories"), 0);
    
    const auto& categories = model->getCategories();
    for (const auto& cat : categories) {
        ui->categoryBox->addItem(cat.name, cat.id);
    }
    
    int idx = ui->categoryBox->findData(currentId);
    if (idx != -1) {
        ui->categoryBox->setCurrentIndex(idx);
    }
    ui->categoryBox->blockSignals(false);
}

void CurseForgePage::onSelectionChanged(QModelIndex first, QModelIndex second) {
    if(!first.isValid())
    {
        if(isOpened)
        {
            dialog->setSuggestedPack();
        }
        return;
    }

    current = model->data(first, Qt::UserRole + 2).value<CurseForge::Modpack>();
    model->getPackDetails(current.id);
    updateCurrentPackUI();
    suggestCurrent();
}

void CurseForgePage::onVersionSelectionChanged(const QString& version) {
    if(version.isEmpty() || ui->versionSelectionBox->count() == 0) {
        currentFile = CurseForge::ModFile();
    }
    else {
        currentFile = ui->versionSelectionBox->currentData().value<CurseForge::ModFile>();
    }
    suggestCurrent();
}

void CurseForgePage::suggestCurrent()
{
    if(!isOpened)
    {
        return;
    }

    if (currentFile.downloadUrl.isEmpty())
    {
        dialog->setSuggestedPack();
        return;
    }

    dialog->setSuggestedPack(current.name + " " + currentFile.displayName, new InstanceImportTask(currentFile.downloadUrl));
    MetaEntryPtr entry = APPLICATION->metacache()->resolveEntry("CurseForgePacks", QString("logos/%1").arg(current.id));
    dialog->setSuggestedIconFromFile(entry->getFullPath(), QString("curseforge-%1").arg(current.id));
}

void CurseForgePage::onPackDataChanged(int id)
{
    if(id != current.id) {
        return;
    }
    auto newData = model->getModpackById(id);
    if(newData) {
        current = *newData;
        updateCurrentPackUI();
    }
}

QString versionToStringCF(const CurseForge::ModFile& version) {
    QString relStr;
    switch(version.releaseType) {
        case CurseForge::FileReleaseType::Release: relStr = "(Release)"; break;
        case CurseForge::FileReleaseType::Beta: relStr = "(Beta)"; break;
        case CurseForge::FileReleaseType::Alpha: relStr = "(Alpha)"; break;
        default: relStr = "(?)"; break;
    }
    return QString("%1 %2").arg(version.displayName, relStr);
}

void CurseForgePage::updateCurrentPackUI()
{
    switch(current.detailsLoaded) {
        case CurseForge::LoadState::Errored: {
            ui->packDescription->setText(tr("Failed to get CurseForge modpack details..."));
            break;
        }
        case CurseForge::LoadState::NotLoaded: {
            ui->packDescription->setText(tr("Loading..."));
            break;
        }
        case CurseForge::LoadState::Loaded: {
            // Using ModrinthDocument for rendering HTML
            auto document = new Modrinth::ModrinthDocument(current.description);
            connect(document, &Modrinth::ModrinthDocument::layoutUpdateRequired, this, &CurseForgePage::forceDocumentLayout);
            ui->packDescription->setDocument(document);
            break;
        }
    }
    
    if(current.files.size() == 0) {
        ui->versionSelectionBox->clear();
    }
    else {
        ui->versionSelectionBox->clear();
        int releaseFound = -1;
        int i = 0;
        for(auto & version: current.files) {
            ui->versionSelectionBox->addItem(versionToStringCF(version), QVariant::fromValue(version));
            if(releaseFound == -1 && version.releaseType == CurseForge::FileReleaseType::Release) {
                releaseFound = i;
            }
            i++;
        }
        if(releaseFound != -1) {
            ui->versionSelectionBox->setCurrentIndex(releaseFound);
        }
        else if(current.files.size() != 0) {
            ui->versionSelectionBox->setCurrentIndex(0);
        }
    }
    suggestCurrent();
}

void CurseForgePage::forceDocumentLayout() {
    ui->packDescription->document()->adjustSize();
}
