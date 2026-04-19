#pragma once

#include <QDialog>
#include <QDialogButtonBox>
#include <QLineEdit>


class GroupInputDialog : public QDialog {
  Q_OBJECT
public:
  explicit GroupInputDialog(const QString &title, const QString &label,
                            const QString &initialText = QString(),
                            QWidget *parent = nullptr);

  QString textValue() const;

private slots:
  void textChanged(const QString &text);

private:
  QLineEdit *m_lineEdit;
  QDialogButtonBox *m_buttonBox;
};
