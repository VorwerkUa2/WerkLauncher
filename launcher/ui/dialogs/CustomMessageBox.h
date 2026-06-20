/* Copyright 2013-2021 MultiMC Contributors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#pragma once

#include <QDialog>
#include <QMessageBox>
#include <QPushButton>

class ThemedMessageBox : public QDialog {
  Q_OBJECT
public:
  ThemedMessageBox(
      QWidget *parent, const QString &title, const QString &text,
      QMessageBox::Icon icon = QMessageBox::NoIcon,
      QMessageBox::StandardButtons buttons = QMessageBox::Ok,
      QMessageBox::StandardButton defaultButton = QMessageBox::NoButton);
  virtual ~ThemedMessageBox() {}

  QMessageBox::StandardButton selectedButton() const {
    return m_selectedButton;
  }

private slots:
  void onButtonClicked(QAbstractButton *button);

private:
  QMessageBox::StandardButton m_selectedButton = QMessageBox::NoButton;
  QMessageBox::StandardButtons m_buttons;
};

namespace CustomMessageBox {
QMessageBox *
selectable(QWidget *parent, const QString &title, const QString &text,
           QMessageBox::Icon icon = QMessageBox::NoIcon,
           QMessageBox::StandardButtons buttons = QMessageBox::Ok,
           QMessageBox::StandardButton defaultButton = QMessageBox::NoButton);

QMessageBox::StandardButton question(
    QWidget *parent, const QString &title, const QString &text,
    QMessageBox::StandardButtons buttons = QMessageBox::Yes | QMessageBox::No,
    QMessageBox::StandardButton defaultButton = QMessageBox::NoButton);

QMessageBox::StandardButton
information(QWidget *parent, const QString &title, const QString &text,
            QMessageBox::StandardButtons buttons = QMessageBox::Ok,
            QMessageBox::StandardButton defaultButton = QMessageBox::NoButton);

QMessageBox::StandardButton
warning(QWidget *parent, const QString &title, const QString &text,
        QMessageBox::StandardButtons buttons = QMessageBox::Ok,
        QMessageBox::StandardButton defaultButton = QMessageBox::NoButton);

QMessageBox::StandardButton
critical(QWidget *parent, const QString &title, const QString &text,
         QMessageBox::StandardButtons buttons = QMessageBox::Ok,
         QMessageBox::StandardButton defaultButton = QMessageBox::NoButton);
} // namespace CustomMessageBox
