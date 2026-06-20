#include "CustomTitleBar.h"
#include "Application.h"
#include "BuildConfig.h"
#include <QApplication>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QStyle>
#include <QTimer>
#include <QWindow>
#include <xdgicon.h>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

CustomTitleBar::CustomTitleBar(QWidget *parent) : QWidget(parent) {
  setFixedHeight(38);
  setupUi();

  // Debounce timer for maximize icon updates during rapid state changes (e.g., Aero Snap)
  m_maxIconTimer.setSingleShot(true);
  m_maxIconTimer.setInterval(100);
  connect(&m_maxIconTimer, &QTimer::timeout, this,
          &CustomTitleBar::applyMaximizeIcon);
}

void CustomTitleBar::setupUi() {
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins(10, 0, 0, 0);
  layout->setSpacing(0);

  m_iconLabel = new QLabel(this);
  m_iconLabel->setPixmap(APPLICATION->getThemedIcon("logo").pixmap(16, 16));

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
    if (auto win = window()) {
#ifdef Q_OS_WIN
      PostMessage((HWND)win->winId(), WM_SYSCOMMAND, SC_MINIMIZE, 0);
#else
      win->showMinimized();
#endif
    }
  });
  connect(m_maxButton, &QPushButton::clicked, [this]() {
    if (auto win = window()) {
      bool isMax = false;
#ifdef Q_OS_WIN
      isMax = IsZoomed((HWND)win->winId());
#else
      isMax = win->isMaximized();
#endif
      if (isMax) {
        setMaximizedState(false);
        win->showNormal();
      } else {
        setMaximizedState(true);
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

  auto btnStyle = QString("QPushButton { border: none; background: transparent; } "
                          "QPushButton:hover { background: %1; } "
                          "QPushButton:pressed { background: rgba(255, 255, 255, 0.1); }")
                      .arg(hoverBg);

  m_minButton->setStyleSheet(btnStyle);
  m_maxButton->setStyleSheet(btnStyle);
  m_closeButton->setStyleSheet(btnStyle);

  m_minButton->setFixedSize(48, 38);
  m_maxButton->setFixedSize(48, 38);
  m_closeButton->setFixedSize(48, 38);

  // Draw static icons for minimize and close (they never change shape)
  auto color = palette().color(QPalette::WindowText);
  color.setAlpha(120);
  auto drawIcon = [&](QPushButton *btn, auto drawFunc) {
    QPixmap pix(48, 38);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(color, 1.8, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    drawFunc(p);
    p.end();
    btn->setIcon(QIcon(pix));
    btn->setIconSize(QSize(48, 38));
  };

  drawIcon(m_minButton, [](QPainter &p) { p.drawLine(19, 19, 29, 19); });
  drawIcon(m_closeButton, [](QPainter &p) {
    p.drawLine(19, 14, 29, 24);
    p.drawLine(29, 14, 19, 24);
  });

  updateMaximizeIcon();
  
  // Refresh the logo in case the accent color changed
  m_iconLabel->setPixmap(APPLICATION->getThemedIcon("logo").pixmap(16, 16));
}

void CustomTitleBar::updateMaximizeIcon() {
  // Restart the debounce timer — only the final state will be drawn
  m_maxIconTimer.start();
}

void CustomTitleBar::setMaximizedState(bool maximized) {
  m_isMaximized = maximized;
}

void CustomTitleBar::applyMaximizeIcon() {
  if (!window())
    return;

  // Final check of the state after debounce
  // m_isMaximized is set by MainWindow during native WM_SIZE events
  auto color = palette().color(QPalette::WindowText);
  color.setAlpha(120);

  QPixmap pix(48, 38);
  pix.fill(Qt::transparent);
  QPainter p(&pix);
  p.setRenderHint(QPainter::Antialiasing);
  p.setPen(QPen(color, 1.8, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

  if (m_isMaximized) {
    // Restore icon (double square)
    p.drawRect(21, 14, 8, 8);
    p.drawRect(19, 16, 8, 8);
  } else {
    // Maximize icon (single square)
    p.drawRect(19, 13, 11, 11);
  }
  p.end();
  m_maxButton->setIcon(QIcon(pix));
  m_maxButton->setIconSize(QSize(48, 38));
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
      if (auto handle = win->windowHandle()) {
        handle->startSystemMove();
        event->accept();
        return;
      }
    }
  }
  QWidget::mousePressEvent(event);
}

void CustomTitleBar::mouseMoveEvent(QMouseEvent *event) {
  // Logic removed: native dragging is now handled by HTCAPTION in MainWindow (Windows)
  // or startSystemMove() in mousePressEvent (cross-platform).
  QWidget::mouseMoveEvent(event);
}

void CustomTitleBar::mouseDoubleClickEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    m_maxButton->click();
  }
}

void CustomTitleBar::paintEvent(QPaintEvent *event) {
  QPainter painter(this);
  bool isDark = palette().color(QPalette::WindowText).lightness() >= 128;
  QColor bg = isDark ? QColor(26, 26, 27) : QColor(245, 245, 247);
  painter.fillRect(rect(), bg);
}
