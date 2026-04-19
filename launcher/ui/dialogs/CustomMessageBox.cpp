#include "CustomMessageBox.h"
#include <QApplication>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QStyle>
#include <QVBoxLayout>

#include "Application.h"
#include "ui/widgets/CustomTitleBar.h"

ThemedMessageBox::ThemedMessageBox(QWidget *parent, const QString &title,
                                   const QString &text, QMessageBox::Icon icon,
                                   QMessageBox::StandardButtons buttons,
                                   QMessageBox::StandardButton defaultButton)
    : QDialog(parent), m_buttons(buttons) {
  setWindowFlags(Qt::FramelessWindowHint | windowFlags());

  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);

  auto titleBar = new CustomTitleBar(this);
  titleBar->setTitle(title);
  layout->addWidget(titleBar);

  QWidget *contentArea = new QWidget(this);
  QVBoxLayout *contentLayout = new QVBoxLayout(contentArea);
  contentLayout->setContentsMargins(20, 20, 20, 20);
  contentLayout->setSpacing(20);

  QHBoxLayout *textLayout = new QHBoxLayout();
  textLayout->setSpacing(20);

  if (icon != QMessageBox::NoIcon) {
    QLabel *iconLabel = new QLabel(this);
    QString iconName;
    switch (icon) {
    case QMessageBox::Information:
      iconName = "status-good";
      break;
    case QMessageBox::Warning:
      iconName = "status-yellow";
      break;
    case QMessageBox::Critical:
      iconName = "status-bad";
      break;
    case QMessageBox::Question:
      iconName = "help";
      break;
    default:
      iconName = "status-good";
      break;
    }
    iconLabel->setPixmap(APPLICATION->getThemedIcon(iconName).pixmap(48, 48));
    iconLabel->setAlignment(Qt::AlignTop);
    textLayout->addWidget(iconLabel);
  }

  QLabel *messageLabel = new QLabel(text, this);
  messageLabel->setWordWrap(true);
  messageLabel->setTextInteractionFlags(Qt::TextSelectableByMouse |
                                        Qt::TextBrowserInteraction);
  textLayout->addWidget(messageLabel, 1);

  contentLayout->addLayout(textLayout);

  QDialogButtonBox *buttonBox = new QDialogButtonBox(
      static_cast<QDialogButtonBox::StandardButtons>(static_cast<int>(buttons)),
      this);
  if (defaultButton != QMessageBox::NoButton) {
    if (auto btn =
            buttonBox->button(static_cast<QDialogButtonBox::StandardButton>(
                static_cast<int>(defaultButton)))) {
      btn->setDefault(true);
    }
  }
  connect(buttonBox, &QDialogButtonBox::clicked, this,
          &ThemedMessageBox::onButtonClicked);

  contentLayout->addWidget(buttonBox);

  layout->addWidget(contentArea);
}

void ThemedMessageBox::onButtonClicked(QAbstractButton *button) {
  m_selectedButton = (QMessageBox::StandardButton)((QDialogButtonBox *)sender())
                         ->standardButton(button);
  done(m_selectedButton);
}

namespace CustomMessageBox {
QMessageBox *selectable(QWidget *parent, const QString &title,
                        const QString &text, QMessageBox::Icon icon,
                        QMessageBox::StandardButtons buttons,
                        QMessageBox::StandardButton defaultButton) {
  QMessageBox *messageBox = new QMessageBox(parent);
  messageBox->setWindowTitle(title);
  messageBox->setText(text);
  messageBox->setStandardButtons(buttons);
  messageBox->setDefaultButton(defaultButton);
  messageBox->setTextInteractionFlags(Qt::TextSelectableByMouse);
  messageBox->setIcon(icon);
  messageBox->setTextInteractionFlags(Qt::TextBrowserInteraction);

  return messageBox;
}

QMessageBox::StandardButton
question(QWidget *parent, const QString &title, const QString &text,
         QMessageBox::StandardButtons buttons,
         QMessageBox::StandardButton defaultButton) {
  ThemedMessageBox dlg(parent, title, text, QMessageBox::Question, buttons,
                       defaultButton);
  dlg.exec();
  return dlg.selectedButton();
}

QMessageBox::StandardButton
information(QWidget *parent, const QString &title, const QString &text,
            QMessageBox::StandardButtons buttons,
            QMessageBox::StandardButton defaultButton) {
  ThemedMessageBox dlg(parent, title, text, QMessageBox::Information, buttons,
                       defaultButton);
  dlg.exec();
  return dlg.selectedButton();
}

QMessageBox::StandardButton warning(QWidget *parent, const QString &title,
                                    const QString &text,
                                    QMessageBox::StandardButtons buttons,
                                    QMessageBox::StandardButton defaultButton) {
  ThemedMessageBox dlg(parent, title, text, QMessageBox::Warning, buttons,
                       defaultButton);
  dlg.exec();
  return dlg.selectedButton();
}

QMessageBox::StandardButton
critical(QWidget *parent, const QString &title, const QString &text,
         QMessageBox::StandardButtons buttons,
         QMessageBox::StandardButton defaultButton) {
  ThemedMessageBox dlg(parent, title, text, QMessageBox::Critical, buttons,
                       defaultButton);
  dlg.exec();
  return dlg.selectedButton();
}
} // namespace CustomMessageBox
