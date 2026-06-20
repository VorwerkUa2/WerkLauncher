#include "ToastNotification.h"
#include <QApplication>
#include <QEnterEvent>
#include <QPainter>
#include <QScreen>
#include <QStyle>
#include <xdgicon.h>

ToastNotification::ToastNotification(QWidget *parent, const QString &message,
                                     Type type)
    : QWidget(parent) {
  setWindowFlags(Qt::FramelessWindowHint | Qt::ToolTip |
                 Qt::NoDropShadowWindowHint);
  setAttribute(Qt::WA_TranslucentBackground);
  setAttribute(Qt::WA_DeleteOnClose);

  setupUi(message, type);

  m_timer = new QTimer(this);
  m_timer->setSingleShot(true);
  connect(m_timer, &QTimer::timeout, this, &ToastNotification::animateOut);
  m_timer->start(3500);
}

void ToastNotification::setupUi(const QString &message, Type type) {
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins(15, 10, 15, 10);
  layout->setSpacing(10);

  m_iconLabel = new QLabel(this);
  m_textLabel = new QLabel(message, this);
  m_textLabel->setStyleSheet(
      "color: white; font-weight: 500; font-size: 13px;");

  QString iconName;
  switch (type) {
  case Success:
    m_accentColor = QColor(46, 204, 113);
    iconName = "emblem-success";
    break;
  case Warning:
    m_accentColor = QColor(241, 194, 50);
    iconName = "emblem-important";
    break;
  case Error:
    m_accentColor = QColor(231, 76, 60);
    iconName = "emblem-unavailable";
    break;
  case Info:
  default:
    m_accentColor = QColor(52, 152, 219);
    iconName = "dialog-information";
    break;
  }

  m_bgColor = QColor(35, 35, 35, 200);

  m_iconLabel->setPixmap(XdgIcon::fromTheme(iconName).pixmap(20, 20));

  layout->addWidget(m_iconLabel);
  layout->addWidget(m_textLabel);

  adjustSize();
}

void ToastNotification::show(QWidget *parent, const QString &message,
                             Type type) {
  auto toast = new ToastNotification(parent, message, type);
  toast->animateIn();
}

void ToastNotification::animateIn() {
  QWidget *p = parentWidget();
  if (!p)
    return;

  // Position at bottom center of parent
  int x = p->geometry().x() + (p->width() - width()) / 2;
  int y = p->geometry().y() + p->height() - height() - 50;

  move(x, y + 20);
  setWindowOpacity(0);
  auto posAnim = new QPropertyAnimation(this, "pos");
  posAnim->setDuration(300);
  posAnim->setStartValue(pos());
  posAnim->setEndValue(QPoint(x, y));
  posAnim->setEasingCurve(QEasingCurve::OutCubic);
  posAnim->start(QAbstractAnimation::DeleteWhenStopped);

  auto opacityAnim = new QPropertyAnimation(this, "windowOpacity");
  opacityAnim->setDuration(300);
  opacityAnim->setStartValue(0);
  opacityAnim->setEndValue(1);
  opacityAnim->start(QAbstractAnimation::DeleteWhenStopped);
}

void ToastNotification::animateOut() {
  auto posAnim = new QPropertyAnimation(this, "pos");
  posAnim->setDuration(300);
  posAnim->setStartValue(pos());
  posAnim->setEndValue(pos() + QPoint(0, 20));
  posAnim->setEasingCurve(QEasingCurve::InCubic);
  posAnim->start(QAbstractAnimation::DeleteWhenStopped);

  auto opacityAnim = new QPropertyAnimation(this, "windowOpacity");
  opacityAnim->setDuration(300);
  opacityAnim->setStartValue(windowOpacity());
  opacityAnim->setEndValue(0);
  connect(opacityAnim, &QPropertyAnimation::finished, this,
          &ToastNotification::close);
  opacityAnim->start(QAbstractAnimation::DeleteWhenStopped);
}

void ToastNotification::paintEvent(QPaintEvent *event) {
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);

  // Subtle glow border
  QColor glowColor = m_accentColor;
  glowColor.setAlpha(35);
  painter.setBrush(Qt::NoBrush);
  painter.setPen(QPen(glowColor, 2));
  painter.drawRoundedRect(rect().adjusted(1, 1, -1, -1), 12, 12);

  // Draw background
  painter.setBrush(m_bgColor);
  painter.setPen(Qt::NoPen);
  painter.drawRoundedRect(rect().adjusted(2, 2, -2, -2), 11, 11);

  // Draw accent border on the left
  painter.setBrush(m_accentColor);
  painter.drawRoundedRect(QRect(2, 2, 4, height() - 4), 11, 11);
  painter.drawRect(
      QRect(4, 2, 2, height() - 4)); // Flatten right side of accent
}

void ToastNotification::enterEvent(QEnterEvent *event) {
  m_timer->stop();
  QWidget::enterEvent(event);
}

void ToastNotification::leaveEvent(QEvent *event) {
  m_timer->start(1000);
  QWidget::leaveEvent(event);
}
