#include "GroupInputDialog.h"
#include "ui/widgets/CustomTitleBar.h"

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>


GroupInputDialog::GroupInputDialog(const QString &title, const QString &label,
                                   const QString &initialText, QWidget *parent)
    : QDialog(parent) {
  setWindowFlags(Qt::FramelessWindowHint | windowFlags());
  setWindowTitle(title);

  m_lineEdit = new QLineEdit(this);
  m_lineEdit->setText(initialText);
  connect(m_lineEdit, &QLineEdit::textChanged, this,
          &GroupInputDialog::textChanged);

  QLabel *textLabel = new QLabel(label, this);
  textLabel->setBuddy(m_lineEdit);

  m_buttonBox = new QDialogButtonBox(
      QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
  connect(m_buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->setSpacing(0);

  auto titleBar = new CustomTitleBar(this);
  titleBar->setTitle(title);
  mainLayout->addWidget(titleBar);

  QVBoxLayout *contentLayout = new QVBoxLayout();
  contentLayout->setContentsMargins(10, 10, 10, 10);
  contentLayout->setSpacing(10);
  contentLayout->addWidget(textLabel);
  contentLayout->addWidget(m_lineEdit);
  contentLayout->addWidget(m_buttonBox);
  mainLayout->addLayout(contentLayout);

  textChanged(initialText);

  if (!initialText.isEmpty()) {
    m_lineEdit->selectAll();
  }
}

QString GroupInputDialog::textValue() const {
  return m_lineEdit->text().trimmed();
}

void GroupInputDialog::textChanged(const QString &text) {
  m_buttonBox->button(QDialogButtonBox::Ok)
      ->setEnabled(!text.trimmed().isEmpty());
}
