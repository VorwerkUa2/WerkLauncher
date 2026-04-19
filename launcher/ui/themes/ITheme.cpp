#include "ITheme.h"
#include "Application.h"
#include "rainbow.h"
#include <QApplication>
#include <QDir>
#include <QStyle>
#include <QStyleFactory>

void ITheme::apply(bool initial) {
  qDebug() << "Applying theme" << id();
  if (QApplication::style()->objectName() != qtTheme()) {
    qDebug() << "Setting style to" << qtTheme();
    QApplication::setStyle(QStyleFactory::create(qtTheme()));
  }

  if (hasColorScheme()) {
    qDebug() << "Setting palette";
    QApplication::setPalette(colorScheme());
  }
  if (hasStyleSheet()) {
    qDebug() << "Setting stylesheet";
    APPLICATION->setStyleSheet(appStyleSheet());
  } else {
    qDebug() << "Clearing stylesheet";
    APPLICATION->setStyleSheet(QString());
  }
  qDebug() << "Setting search paths";
  QDir::setSearchPaths("theme", searchPaths());
  qDebug() << "Theme applied";
}

QPalette ITheme::fadeInactive(QPalette in, qreal bias, QColor color) {
  auto blend = [&in, bias, color](QPalette::ColorRole role) {
    QColor from = in.color(QPalette::Active, role);
    QColor blended = Rainbow::mix(from, color, bias);
    in.setColor(QPalette::Disabled, role, blended);
  };
  blend(QPalette::Window);
  blend(QPalette::WindowText);
  blend(QPalette::Base);
  blend(QPalette::AlternateBase);
  blend(QPalette::ToolTipBase);
  blend(QPalette::ToolTipText);
  blend(QPalette::Text);
  blend(QPalette::Button);
  blend(QPalette::ButtonText);
  blend(QPalette::BrightText);
  blend(QPalette::Link);
  blend(QPalette::Highlight);
  blend(QPalette::HighlightedText);
  return in;
}
