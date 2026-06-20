#include "SidePanel.h"
#include "Application.h"
#include "MMCTime.h"
#include "icons/IconList.h"
#include <QPainter>
#include <QStyleOption>
#include <QTimer>
#include <xdgicon.h>

SidePanel::SidePanel(QWidget *parent) : QWidget(parent) {
  setFixedWidth(250);
  setupUi();

  m_anim = new QPropertyAnimation(this, "maximumWidth");
  m_anim->setDuration(300);
  m_anim->setEasingCurve(QEasingCurve::InOutCubic);
}

void SidePanel::updateStyles() {
  if (!m_nameLabel)
    return;

  QColor textColor = palette().color(QPalette::WindowText);
  QColor dimColor = palette().color(QPalette::WindowText);
  dimColor.setAlphaF(0.6f);
  QColor faintColor = palette().color(QPalette::WindowText);
  faintColor.setAlphaF(0.45f);

  QColor btnBg = palette().color(QPalette::Button);
  QColor btnHover = palette().color(QPalette::Highlight);
  QColor accentColor = palette().color(QPalette::Highlight);

  m_nameLabel->setStyleSheet(
      QString("font-size: 18px; font-weight: bold; color: %1;")
          .arg(textColor.name()));
  m_versionLabel->setStyleSheet(
      QString("color: rgba(%1, %2, %3, 160); font-size: 13px;")
          .arg(dimColor.red())
          .arg(dimColor.green())
          .arg(dimColor.blue()));
  m_playTimeLabel->setStyleSheet(
      QString("color: rgba(%1, %2, %3, 115); font-size: 12px;")
          .arg(faintColor.red())
          .arg(faintColor.green())
          .arg(faintColor.blue()));

  auto btnStyle =
      QString(
          "QPushButton { background: rgb(%1, %2, %3); color: %4; "
          "border: none; padding: 10px; border-radius: 6px; text-align: left; "
          "padding-left: 15px; } "
          "QPushButton:hover { background: rgba(%5, %6, %7, 60); "
          "color: %8; border: none; } "
          "QPushButton:pressed { background: %8; color: %9; border: none; } "
          "QPushButton#playBtn { background: %8; font-weight: bold; "
          "text-align: center; padding-left: 10px; color: %9; border: none; } "
          "QPushButton#playBtn:hover { background: rgba(%5, %6, %7, 200); border: none; }")
          .arg(btnBg.red())
          .arg(btnBg.green())
          .arg(btnBg.blue())
          .arg(textColor.name())
          .arg(accentColor.red())
          .arg(accentColor.green())
          .arg(accentColor.blue())
          .arg(accentColor.name())
          .arg(palette().color(QPalette::HighlightedText).name());

  m_playButton->setStyleSheet(btnStyle);
  m_settingsButton->setStyleSheet(btnStyle);
  m_logButton->setStyleSheet(btnStyle);
}

void SidePanel::setupUi() {
  auto layout = new QVBoxLayout(this);
  layout->setContentsMargins(20, 20, 20, 20);
  layout->setSpacing(15);

  m_iconLabel = new QLabel(this);
  m_iconLabel->setAlignment(Qt::AlignCenter);
  m_iconLabel->setFixedSize(64, 64);

  m_nameLabel = new QLabel(this);
  m_nameLabel->setWordWrap(true);
  m_nameLabel->setAlignment(Qt::AlignCenter);

  m_versionLabel = new QLabel(this);
  m_versionLabel->setAlignment(Qt::AlignCenter);

  m_playTimeLabel = new QLabel(this);
  m_playTimeLabel->setAlignment(Qt::AlignCenter);

  layout->addSpacing(20);
  layout->addWidget(m_iconLabel, 0, Qt::AlignCenter);
  layout->addWidget(m_nameLabel);
  layout->addWidget(m_versionLabel);
  layout->addWidget(m_playTimeLabel);
  layout->addSpacing(20);

  m_playButton = new QPushButton(tr("Launch Game"), this);
  m_playButton->setObjectName("playBtn");
  m_playButton->setIcon(XdgIcon::fromTheme("media-playback-start"));

  m_settingsButton = new QPushButton(tr("Instance Settings"), this);
  m_settingsButton->setIcon(XdgIcon::fromTheme("configure"));

  m_logButton = new QPushButton(tr("View Logs"), this);
  m_logButton->setIcon(XdgIcon::fromTheme("utilities-log-viewer"));

  updateStyles();

  layout->addWidget(m_playButton);
  layout->addWidget(m_settingsButton);
  layout->addWidget(m_logButton);
  layout->addStretch();

  connect(m_playButton, &QPushButton::clicked, [this]() {
    if (m_instance)
      APPLICATION->launch(m_instance);
  });
  connect(m_settingsButton, &QPushButton::clicked, [this]() {
    if (m_instance)
      APPLICATION->showInstanceWindow(m_instance);
  });
  connect(m_logButton, &QPushButton::clicked, [this]() {
    if (m_instance)
      APPLICATION->showInstanceWindow(m_instance, "log");
  });
}

void SidePanel::setInstance(InstancePtr inst) {
  m_instance = inst;
  if (!inst)
    return;

  m_iconLabel->setPixmap(
      APPLICATION->icons()->getIcon(inst->iconKey()).pixmap(64, 64));
  m_nameLabel->setText(inst->name());

  QString version = inst->getManagedPackVersionName();
  if (version.isEmpty()) {
    version = inst->instanceType();
  }
  m_versionLabel->setText(version);

  int time = inst->totalTimePlayed();
  m_playTimeLabel->setText(tr("Played: %1").arg(Time::prettifyDuration(time)));
}

void SidePanel::toggle(bool visible) {
  m_anim->stop();
  if (visible) {
    show();
    m_anim->setStartValue(0);
    m_anim->setEndValue(250);
  } else {
    m_anim->setStartValue(width());
    m_anim->setEndValue(0);
    connect(m_anim, &QPropertyAnimation::finished, this, &SidePanel::hide);
  }
  m_anim->start();
}

void SidePanel::changeEvent(QEvent *event) {
  if (event->type() == QEvent::PaletteChange) {
    QTimer::singleShot(0, this, &SidePanel::updateStyles);
  }
  QWidget::changeEvent(event);
}

void SidePanel::paintEvent(QPaintEvent *event) {
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);

  // Solid background from palette
  QColor bg = palette().color(QPalette::Window);
  painter.fillRect(rect(), bg);

  // Subtle accent border on the left edge
  QColor accent = palette().color(QPalette::Highlight);
  accent.setAlpha(60);
  painter.setPen(QPen(accent, 2));
  painter.drawLine(0, 0, 0, height());
}
