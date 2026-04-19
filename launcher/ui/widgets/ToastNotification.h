#pragma once
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QLabel>
#include <QPropertyAnimation>
#include <QTimer>
#include <QWidget>

class QEnterEvent;

class ToastNotification : public QWidget {
  Q_OBJECT
  Q_PROPERTY(qreal opacity READ windowOpacity WRITE setWindowOpacity)

public:
  enum Type { Info, Success, Warning, Error };
  static void show(QWidget *parent, const QString &message, Type type = Info);

protected:
  void paintEvent(QPaintEvent *event) override;
  void enterEvent(QEnterEvent *event) override;
  void leaveEvent(QEvent *event) override;

private:
  ToastNotification(QWidget *parent, const QString &message, Type type);
  void setupUi(const QString &message, Type type);
  void animateIn();
  void animateOut();

  QLabel *m_iconLabel;
  QLabel *m_textLabel;
  QTimer *m_timer;
  QColor m_bgColor;
  QColor m_accentColor;
};
