#include "LanguageSelectionWidget.h"

#include "Application.h"
#include "translations/TranslationsModel.h"
#include <QHeaderView>
#include <QLabel>
#include <QTreeView>
#include <QVBoxLayout>

LanguageSelectionWidget::LanguageSelectionWidget(QWidget *parent)
    : QWidget(parent) {
  verticalLayout = new QVBoxLayout(this);
  verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
  languageView = new QTreeView(this);
  languageView->setObjectName(QStringLiteral("languageView"));
  languageView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  languageView->setAlternatingRowColors(true);
  languageView->setRootIsDecorated(false);
  languageView->setItemsExpandable(false);
  languageView->setWordWrap(true);
  languageView->header()->setCascadingSectionResizes(true);
  languageView->header()->setStretchLastSection(false);
  verticalLayout->addWidget(languageView);

  auto translations = APPLICATION->translations();
  auto index = translations->selectedIndex();
  languageView->setModel(translations.get());
  languageView->setCurrentIndex(index);
  languageView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
  languageView->header()->setSectionResizeMode(0, QHeaderView::Stretch);
  connect(languageView->selectionModel(),
          &QItemSelectionModel::currentRowChanged, this,
          &LanguageSelectionWidget::languageRowChanged);
  verticalLayout->setContentsMargins(0, 0, 0, 0);
}

QString LanguageSelectionWidget::getSelectedLanguageKey() const {
  auto translations = APPLICATION->translations();
  return translations->data(languageView->currentIndex(), Qt::UserRole)
      .toString();
}

void LanguageSelectionWidget::retranslate() {}

void LanguageSelectionWidget::languageRowChanged(const QModelIndex &current,
                                                 const QModelIndex &previous) {
  if (current == previous) {
    return;
  }
  auto translations = APPLICATION->translations();
  QString key = translations->data(current, Qt::UserRole).toString();
  translations->selectLanguage(key);
  translations->updateLanguage(key);
}
