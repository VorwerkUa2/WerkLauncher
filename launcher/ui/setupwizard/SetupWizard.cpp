#include "SetupWizard.h"

#include "JavaWizardPage.h"
#include "LanguageWizardPage.h"

#include "translations/TranslationsModel.h"
#include <Application.h>
#include <FileSystem.h>

#include "../widgets/CustomTitleBar.h"
#include <BuildConfig.h>
#include <QAbstractButton>
#include <QBoxLayout>
#include <QVBoxLayout>

SetupWizard::SetupWizard(QWidget *parent) : QWizard(parent) {
  setObjectName(QStringLiteral("SetupWizard"));
  resize(615, 659);
  setWindowFlags(Qt::FramelessWindowHint | windowFlags());

  // make it look better with custom title bar
  setWizardStyle(QWizard::ModernStyle);
  setOptions(QWizard::NoCancelButton | QWizard::IndependentPages |
             QWizard::HaveCustomButton1 | QWizard::NoDefaultButton);

  // Add custom title bar
  auto titleBar = new CustomTitleBar(this);
  titleBar->setTitle(tr("%1 Quick Setup").arg(BuildConfig.LAUNCHER_NAME));
  titleBar->setObjectName("titleBar");

  retranslate();

  connect(this, &QWizard::currentIdChanged, this, &SetupWizard::pageChanged);
}

void SetupWizard::resizeEvent(QResizeEvent *event) {
  QWizard::resizeEvent(event);
  if (auto titleBar = findChild<CustomTitleBar *>("titleBar")) {
    titleBar->resize(width(), 32);
    titleBar->move(0, 0);
  }
}

void SetupWizard::retranslate() {
  setButtonText(QWizard::NextButton, tr("&Next >"));
  setButtonText(QWizard::BackButton, tr("< &Back"));
  setButtonText(QWizard::FinishButton, tr("&Finish"));
  setButtonText(QWizard::CustomButton1, tr("&Refresh"));
  // Native window title must be empty to avoid showing up in some taskbars/etc
  // when using frameless, but we set it anyway for accessibility.
  // The custom title bar handles the visual title.
  setWindowTitle("");
}

BaseWizardPage *SetupWizard::getBasePage(int id) {
  if (id == -1)
    return nullptr;
  auto pagePtr = page(id);
  if (!pagePtr)
    return nullptr;
  return dynamic_cast<BaseWizardPage *>(pagePtr);
}

BaseWizardPage *SetupWizard::getCurrentBasePage() {
  return getBasePage(currentId());
}

void SetupWizard::pageChanged(int id) {
  auto basePagePtr = getBasePage(id);
  if (!basePagePtr) {
    return;
  }
  if (basePagePtr->wantsRefreshButton()) {
    setButtonLayout({QWizard::CustomButton1, QWizard::Stretch,
                     QWizard::BackButton, QWizard::NextButton,
                     QWizard::FinishButton});
    auto customButton = button(QWizard::CustomButton1);
    connect(customButton, &QAbstractButton::pressed, [&]() {
      auto basePagePtr = getCurrentBasePage();
      if (basePagePtr) {
        basePagePtr->refresh();
      }
    });
  } else {
    setButtonLayout({QWizard::Stretch, QWizard::BackButton, QWizard::NextButton,
                     QWizard::FinishButton});
  }
}

void SetupWizard::changeEvent(QEvent *event) {
  if (event->type() == QEvent::LanguageChange) {
    retranslate();
  }
  QWizard::changeEvent(event);
}

SetupWizard::~SetupWizard() {}
