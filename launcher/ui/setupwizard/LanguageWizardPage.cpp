#include "LanguageWizardPage.h"
#include <Application.h>
#include <translations/TranslationsModel.h>

#include <BuildConfig.h>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

LanguageWizardPage::LanguageWizardPage(QWidget *parent)
    : BaseWizardPage(parent) {
  setObjectName(QStringLiteral("languagePage"));
  
  auto layout = new QVBoxLayout(this);
  layout->setContentsMargins(50, 50, 50, 40);
  layout->setSpacing(20);

  // Logo
  auto logoLabel = new QLabel(this);
  QPixmap logo = APPLICATION->getThemedIcon("logo").pixmap(80, 80);
  logoLabel->setPixmap(logo);
  logoLabel->setAlignment(Qt::AlignCenter);
  layout->addWidget(logoLabel);

  // Welcome Text
  welcomeLabel = new QLabel(this);
  QFont welcomeFont = welcomeLabel->font();
  welcomeFont.setPointSize(24);
  welcomeFont.setWeight(QFont::Bold);
  welcomeLabel->setFont(welcomeFont);
  welcomeLabel->setAlignment(Qt::AlignCenter);
  layout->addWidget(welcomeLabel);

  subtitleLabel = new QLabel(this);
  QFont subtitleFont = subtitleLabel->font();
  subtitleFont.setPointSize(12);
  subtitleLabel->setFont(subtitleFont);
  subtitleLabel->setAlignment(Qt::AlignCenter);
  subtitleLabel->setStyleSheet("color: rgba(160, 160, 160, 255);");
  layout->addWidget(subtitleLabel);

  layout->addStretch(1);

  // Language Cards
  auto btnLayout = new QHBoxLayout();
  btnLayout->setSpacing(16);

  auto createLangButton = [this](const QString& displayText, const QString& key) {
      QPushButton* btn = new QPushButton(this);
      btn->setText(displayText);
      btn->setMinimumHeight(90);
      btn->setMinimumWidth(200);
      btn->setCursor(Qt::PointingHandCursor);
      btn->setStyleSheet(
          "QPushButton {"
          "  font-size: 16px;"
          "  font-weight: bold;"
          "  border-radius: 14px;"
          "  border: 2px solid rgba(128, 128, 128, 0.2);"
          "  background: rgba(128, 128, 128, 0.06);"
          "  padding: 16px;"
          "}"
          "QPushButton:hover {"
          "  background: rgba(128, 128, 128, 0.15);"
          "  border: 2px solid rgba(128, 128, 128, 0.4);"
          "}"
          "QPushButton:checked {"
          "  border: 2px solid #e67e22;"
          "  background: rgba(230, 126, 34, 0.12);"
          "}"
      );
      btn->setCheckable(true);
      btn->setAutoExclusive(true);
      connect(btn, &QPushButton::clicked, this, [this, key]() { languageClicked(key); });
      return btn;
  };

  btnEn = createLangButton(QString::fromUtf8("\xF0\x9F\x87\xAC\xF0\x9F\x87\xA7  English"), "en_US");
  btnUk = createLangButton(QString::fromUtf8("\xF0\x9F\x87\xBA\xF0\x9F\x87\xA6  \xD0\xA3\xD0\xBA\xD1\x80\xD0\xB0\xD1\x97\xD0\xBD\xD1\x81\xD1\x8C\xD0\xBA\xD0\xB0"), "uk_UA");

  btnLayout->addWidget(btnEn);
  btnLayout->addWidget(btnUk);
  
  layout->addLayout(btnLayout);
  layout->addStretch(2);

  // Set default
  auto settings = APPLICATION->settings();
  m_selectedLanguageKey = settings->get("Language").toString();
  if (m_selectedLanguageKey == "uk_UA") {
      btnUk->setChecked(true);
  } else {
      btnEn->setChecked(true);
      m_selectedLanguageKey = "en_US";
  }

  retranslate();
}

LanguageWizardPage::~LanguageWizardPage() {}

bool LanguageWizardPage::validatePage() {
  auto settings = APPLICATION->settings();
  settings->set("Language", m_selectedLanguageKey);
  return true;
}

void LanguageWizardPage::languageClicked(const QString &key) {
    m_selectedLanguageKey = key;
    auto translations = APPLICATION->translations();
    translations->selectLanguage(key);
    translations->updateLanguage(key);
}

void LanguageWizardPage::retranslate() {
  welcomeLabel->setText(tr("Welcome"));
  subtitleLabel->setText(tr("Select your language to continue"));
}
