#include "CustomTitleBar.h"
#include "Application.h"
#include "BuildConfig.h"
#include <QApplication>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QStyle>
#include <QTimer>
#include <XdgIcon.h>

CustomTitleBar::CustomTitleBar(QWidget *parent) : QWidget(parent) {
  setFixedHeight(32);
  setupUi();
}

void CustomTitleBar::setupUi() {
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins(10, 0, 0, 0);
  layout->setSpacing(0);

  m_iconLabel = new QLabel(this);
  m_iconLabel->setPixmap(XdgIcon::fromTheme("multimc").pixmap(16, 16));

  m_titleLabel = new QLabel(BuildConfig.LAUNCHER_NAME, this);

  layout->addWidget(m_iconLabel);
  layout->addWidget(m_titleLabel);
  layout->addStretch();

  m_minButton = new QPushButton(this);
  m_maxButton = new QPushButton(this);
  m_closeButton = new QPushButton(this);
  m_closeButton->setObjectName("closeBtn");

  updateStyles();

  layout->addWidget(m_minButton);
  layout->addWidget(m_maxButton);
  layout->addWidget(m_closeButton);

  connect(m_minButton, &QPushButton::clicked, [this]() {
    if (auto win = window())
      win->showMinimized();
  });
  connect(m_maxButton, &QPushButton::clicked, [this]() {
    if (auto win = window()) {
      if (win->isMaximized()) {
        win->showNormal();
      } else {
        win->showMaximized();
      }
      updateMaximizeIcon();
    }
  });
  connect(m_closeButton, &QPushButton::clicked, [this]() {
    if (auto win = window()) {
      QTimer::singleShot(0, win, &QWidget::close);
    }
  });
}

void CustomTitleBar::updateStyles() {
  bool isDark = palette().color(QPalette::WindowText).lightness() >= 128;
  QString textColor = palette().color(QPalette::WindowText).name();
  QString hoverBg = isDark ? "rgba(255, 255, 255, 0.1)" : "rgba(0, 0, 0, 0.05)";

  m_titleLabel->setStyleSheet(
      QString("color: %1; font-size: 12px; font-weight: 500; margin-left: 5px;")
          .arg(textColor));

  auto btnStyle = QString("QPushButton { border: none; background: "
                          "transparent; width: 32px; height: 32px; } "
                          "QPushButton:hover { background: %1; }")
                      .arg(hoverBg);

  m_minButton->setStyleSheet(btnStyle);
  m_maxButton->setStyleSheet(btnStyle);
  m_closeButton->setStyleSheet(btnStyle);

  updateMaximizeIcon();
}

void CustomTitleBar::updateMaximizeIcon() {
  bool maximized = window() ? window()->isMaximized() : false;
  auto color = palette().color(QPalette::WindowText);

  auto setIcon = [&](QPushButton *btn, const QString &type,
                     bool isMax = false) {
    QPixmap pix(32, 32);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(color, 1.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

    if (type == "min") {
      p.drawLine(10, 16, 22, 16);
    } else if (type == "max") {
      if (isMax) {
        p.drawRect(12, 10, 8, 8);
        p.drawRect(10, 12, 8, 8); // Double square look for restore
      } else {
        p.drawRect(10, 10, 12, 12);
      }
    } else if (type == "close") {
      p.drawLine(10, 10, 22, 22);
      p.drawLine(22, 10, 10, 22);
    }
    btn->setIcon(QIcon(pix));
    btn->setIconSize(QSize(32, 32));
  };

  setIcon(m_minButton, "min");
  setIcon(m_maxButton, "max", maximized);
  setIcon(m_closeButton, "close");
}

void CustomTitleBar::setTitle(const QString &title) {
  m_titleLabel->setText(title);
}

void CustomTitleBar::changeEvent(QEvent *event) {
  if (event->type() == QEvent::PaletteChange) {
    QTimer::singleShot(0, this, &CustomTitleBar::updateStyles);
  }
  QWidget::changeEvent(event);
}

void CustomTitleBar::mousePressEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    if (auto win = window()) {
      m_dragPos =
          event->globalPosition().toPoint() - win->frameGeometry().topLeft();
      event->accept();
    }
  }
}

void CustomTitleBar::mouseMoveEvent(QMouseEvent *event) {
  if (event->buttons() & Qt::LeftButton) {
    if (auto win = window()) {
      if (win->isMaximized()) {
        int oldWidth = win->width();
        win->showNormal();
        int newWidth = win->width();
        m_dragPos.setX(newWidth * ((float)m_dragPos.x() / oldWidth));
        updateMaximizeIcon();
      }
      win->move(event->globalPosition().toPoint() - m_dragPos);
      event->accept();
    }
  }
}

void CustomTitleBar::mouseDoubleClickEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    m_maxButton->click();
    event->accept();
  }
}

void CustomTitleBar::paintEvent(QPaintEvent *event) {
  QPainter painter(this);
  bool isDark = palette().color(QPalette::WindowText).lightness() >= 128;
  QColor bg = isDark ? QColor(26, 26, 27) : QColor(245, 245, 247);
  painter.fillRect(rect(), bg);
}
