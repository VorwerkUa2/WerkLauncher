/*
 * Copyright 2024 Petr Mrázek
 *
 * This source is subject to the Microsoft Permissive License (MS-PL).
 * Please see the COPYING.md file for more information.
 */

#include <QDialogButtonBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

#include "OfflineNameDialog.h"
#include "ui/widgets/CustomTitleBar.h"

OfflineNameDialog::OfflineNameDialog(QString current_name, QWidget *parent) {
  setWindowFlags(Qt::FramelessWindowHint | windowFlags());
  setWindowTitle(tr("Player name"));

  m_lineEdit = new QLineEdit(this);
  m_lineEdit->setText(current_name);
  connect(m_lineEdit, &QLineEdit::textChanged, this,
          &OfflineNameDialog::textChanged);

  QLabel *label = new QLabel(tr("Choose your offline mode player name."), this);
  label->setBuddy(m_lineEdit);
  label->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

  m_buttonBox = new QDialogButtonBox(
      QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
  QPushButton *retryButton =
      m_buttonBox->addButton(tr("Retry Online"), QDialogButtonBox::ActionRole);
  connect(m_buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
  connect(retryButton, &QPushButton::clicked, this,
          &OfflineNameDialog::retryOnline);

  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->setSpacing(0);

  auto titleBar = new CustomTitleBar(this);
  titleBar->setTitle(tr("Player name"));
  mainLayout->addWidget(titleBar);

  mainLayout->addWidget(label);
  mainLayout->addWidget(m_lineEdit);
  mainLayout->addWidget(m_buttonBox);

  textChanged(current_name);
}

void OfflineNameDialog::accept() {
  if (!m_acceptEnabled) {
    return;
  }
  QDialog::accept();
}

QString OfflineNameDialog::textValue() const { return m_lineEdit->text(); }

void OfflineNameDialog::textChanged(const QString &text) {
  m_acceptEnabled = !text.isEmpty();
  m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(m_acceptEnabled);
}

void OfflineNameDialog::retryOnline() { done(OnlineRequested); }
