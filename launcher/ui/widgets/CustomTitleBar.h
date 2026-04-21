#pragma once

#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPoint>
#include <QPushButton>
#include <QTimer>
#include <QWidget>

class CustomTitleBar : public QWidget {
  Q_OBJECT
public:
  explicit CustomTitleBar(QWidget *parent = nullptr);
  void setTitle(const QString &title);
  void updateMaximizeIcon();
  void setMaximizedState(bool maximized);

protected:
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mouseDoubleClickEvent(QMouseEvent *event) override;
  void changeEvent(QEvent *event) override;
  void paintEvent(QPaintEvent *event) override;

private slots:
  void applyMaximizeIcon();

private:
  void setupUi();
  void updateStyles();

  QLabel *m_iconLabel;
  QLabel *m_titleLabel;
  QPushButton *m_minButton;
  QPushButton *m_maxButton;
  QPushButton *m_closeButton;

  QTimer m_maxIconTimer;
  bool m_isMaximized = false;
  QPoint m_dragPos;
};
