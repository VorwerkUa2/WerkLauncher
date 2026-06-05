#pragma once

#include "BaseWizardPage.h"

#include <QString>
class QPushButton;
class QLabel;

class LanguageWizardPage : public BaseWizardPage {
  Q_OBJECT
public:
  explicit LanguageWizardPage(QWidget *parent = Q_NULLPTR);

  virtual ~LanguageWizardPage();

  bool validatePage() override;

protected:
  void retranslate() override;
  void changeEvent(QEvent *event) override;

private slots:
  void languageClicked(const QString &key);

private:
  QString m_selectedLanguageKey;
  QLabel *welcomeLabel = nullptr;
  QLabel *subtitleLabel = nullptr;
  QPushButton *btnEn = nullptr;
  QPushButton *btnUk = nullptr;
};
