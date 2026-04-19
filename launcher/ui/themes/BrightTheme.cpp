#include "BrightTheme.h"
#include "Application.h"
#include "settings/SettingsObject.h"
#include <QColor>
#include <QObject>
#include <QPalette>

QString BrightTheme::id() { return "bright"; }
bool BrightTheme::isDark() { return false; }

QString BrightTheme::name() { return QObject::tr("Modern White"); }

bool BrightTheme::hasColorScheme() { return true; }

QPalette BrightTheme::colorScheme() {
  QPalette brightPalette;
  QColor windowColor(245, 245, 247);
  QColor baseColor(255, 255, 255);
  QString customAccent =
      APPLICATION->settings()->get("CustomAccentColor").toString();
  QColor accentColor(customAccent);

  brightPalette.setColor(QPalette::Window, windowColor);
  brightPalette.setColor(QPalette::WindowText, QColor(26, 26, 27));
  brightPalette.setColor(QPalette::Base, baseColor);
  brightPalette.setColor(QPalette::AlternateBase, windowColor);
  brightPalette.setColor(QPalette::ToolTipBase, QColor(26, 26, 27));
  brightPalette.setColor(QPalette::ToolTipText, Qt::white);
  brightPalette.setColor(QPalette::Text, QColor(26, 26, 27));
  brightPalette.setColor(QPalette::Button, windowColor);
  brightPalette.setColor(QPalette::ButtonText, QColor(26, 26, 27));
  brightPalette.setColor(QPalette::BrightText, accentColor);
  brightPalette.setColor(QPalette::Link, accentColor);
  brightPalette.setColor(QPalette::Highlight, accentColor);
  brightPalette.setColor(QPalette::HighlightedText, Qt::white);
  return fadeInactive(brightPalette, fadeAmount(), fadeColor());
}

double BrightTheme::fadeAmount() { return 0.5; }

QColor BrightTheme::fadeColor() { return QColor(245, 245, 247); }

bool BrightTheme::hasStyleSheet() { return true; }

QString BrightTheme::appStyleSheet() {
  QString customAccent =
      APPLICATION->settings()->get("CustomAccentColor").toString();
  QString qss =
      "QWidget { background: transparent; border: none; font-family: 'Segoe "
      "UI', 'SF Pro Display', 'Helvetica Neue', 'Ubuntu', sans-serif; }"
      "QMainWindow, QDialog, QWizard, QDockWidget { background-color: "
      "#f5f5f7; }"
      "QDockWidget::title { background-color: #f5f5f7; color: "
      "#1a1a1b; "
      "padding: 4px; }"
      "QStatusBar { background-color: #f5f5f7; color: "
      "#1a1a1b; }"
      "QMenuBar { background-color: #ffffff; color: "
      "#1a1a1b; padding: 2px; }"
      "QToolBar { background-color: #ffffff; "
      "border-bottom: 1px solid #e5e5ea; "
      "padding: 4px; "
      "margin: 0px; spacing: 4px; border-radius: 8px; }"
      "QToolBar::separator { width: 1px; background-color: #e5e5ea; "
      "margin: 4px 8px; }"
      "QMenuBar::item { background: transparent; padding: 4px 10px; "
      "margin: 2px; border-radius: 4px; }"
      "QMenuBar::item:selected { background-color: #e5e5ea; color: #ffae00; "
      "}"
      "QMenuBar::item:pressed { padding: 5px 9px 3px 11px; }"
      "QMenu { background-color: #ffffff; border: 1px "
      "solid #e5e5ea; "
      "border-radius: 8px; padding: 4px; }"
      "QMenu::item { padding: 4px 18px 4px 8px; border-radius: 4px; "
      "color: #1a1a1b; margin: 1px 2px; }"
      "QMenu::item:selected { background-color: #ffae00; color: #ffffff; }"
      "QMenu::item:pressed { padding: 5px 17px 3px 9px; }"
      "QToolTip { color: #ffffff; background-color: #ffae00; border: 1px "
      "solid #ffae00; border-radius: 6px; padding: 4px; }"
      "QPushButton, QToolButton { background-color: #ffffff; "
      "border: 1px solid #d1d1d6; "
      "border-radius: 8px; padding: 6px 14px; color: #1a1a1b; }"
      "QPushButton:hover, QToolButton:hover { background-color: #f2f2f7; "
      "border: 1px solid "
      "#ffae00; color: #ffae00; }"
      "QToolButton { spacing: 48px; }"
      "QToolButton:on, QToolButton:open { background-color: #f2f2f7; "
      "border: 1px solid #d1d1d6; }"
      "QPushButton:pressed, QToolButton:pressed { background-color: "
      "#e5e5ea; border: 1px solid #ffae00; padding: 7px 13px 5px 15px; }"
      "QPushButton:disabled, QToolButton:disabled { background-color: "
      "#f0f0f2; border: 1px solid #e5e5ea; color: #8e8e93; }"
      "QLineEdit, QTextEdit, QPlainTextEdit { background-color: "
      "rgba(255, 255, 255, 220);"
      "border: 1px solid #d1d1d6; border-radius: 8px; padding: 4px; color: "
      "#1a1a1b; selection-background-color: #ffae00; }"
      "QLineEdit:disabled, QTextEdit:disabled, QPlainTextEdit:disabled { "
      "background-color: #f0f0f2; border: 1px solid #e5e5ea; color: "
      "#8e8e93; }"
      "QLineEdit:focus { border: 1px solid #ffae00; }"
      "QComboBox { background-color: #ffffff; border: 1px "
      "solid #d1d1d6; "
      "border-radius: 8px; padding: 4px; color: #1a1a1b; }"
      "QComboBox:disabled { background-color: #f0f0f2; border: 1px solid "
      "#e5e5ea; color: #8e8e93; }"
      "QComboBox::drop-down { border: none; width: 24px; }"
      "QComboBox::down-arrow { image: none; border-left: 4px solid "
      "transparent; "
      "border-right: 4px solid transparent; border-top: 4px solid #ffae00; "
      "width: 0; height: 0; margin-right: 8px; }"
      "QComboBox QAbstractItemView { background-color: #ffffff; "
      "border: 1px "
      "solid #d1d1d6; selection-background-color: #ffae00; "
      "selection-color: #ffffff; outline: none; }"
      "QComboBox QAbstractItemView::item { padding: 8px; color: #1a1a1b; "
      "}"
      "QComboBox QAbstractItemView::item:selected { background-color: "
      "#ffae00; color: #ffffff; }"
      "QListView, QTreeView, QTableView, InstanceView { background-color: "
      "#ffffff; "
      "border: 1px solid #e5e5ea; border-radius: 8px; outline: "
      "none; }"
      "QListView::indicator, QTreeView::indicator { width: 18px; height: "
      "18px; "
      "background-color: #ffffff; border: 1px solid #d1d1d6; border-radius: "
      "6px; }"
      "QListView::indicator:checked, QTreeView::indicator:checked { "
      "background-color: #ffae00; "
      "border: 2px solid #333333; }"
      "QListView::item { padding: 8px; border-radius: 8px; color: #1a1a1b; }"
      "QListView::item:hover { background-color: #f2f2f7; "
      "border: 1px solid #ffae00; }"
      "QListView::item:pressed { background-color: #e5e5ea; padding: 9px "
      "7px 7px 9px; }"
      "QListView::item:selected { "
      "background-color: #e5e5ea; color: #ffae00; border: 1px solid "
      "#ffae00; "
      "border-radius: 8px; }"
      "QListView::item:disabled { color: #8e8e93; background-color: "
      "transparent; }"
      "QScrollBar:vertical { border: none; background: transparent; width: "
      "8px; margin: 2px; }"
      "QScrollBar:horizontal { border: none; background: transparent; "
      "height: 8px; margin: 2px; }"
      "QScrollBar::handle:vertical, QScrollBar::handle:horizontal { "
      "background: #d1d1d6; min-width: 20px; min-height: 20px; "
      "border-radius: 4px; }"
      "QScrollBar::handle:vertical:hover, "
      "QScrollBar::handle:horizontal:hover { "
      "background: #ffae00; }"
      "QScrollBar::add-line, QScrollBar::sub-line { border: none; "
      "background: none; }"
      "QScrollBar::add-page, QScrollBar::sub-page { background: none; }"
      "QHeaderView::section { background-color: #f2f2f7; "
      "padding: 4px; "
      "border: 1px solid #d1d1d6; color: #1a1a1b; }"
      "QTabWidget::pane { border: 1px solid #d1d1d6; top: -1px; "
      "border-radius: 8px; }"
      "QTabBar::tab { background-color: #ffffff; border: "
      "1px solid #d1d1d6; "
      "border-radius: 8px; padding: 6px 12px; margin: 2px; color: #1a1a1b; }"
      "QTabBar::tab:hover { border: 1px solid #ffae00; color: #ffae00; }"
      "QTabBar::tab:selected { background-color: #ffae00; color: #ffffff; "
      "border: 1px solid #ffae00; }"
      "QTabBar::tab:disabled { color: #8e8e93; background-color: #f0f0f2; "
      "border: 1px solid #e5e5ea; }"
      "QTabBar::tab:pressed { padding: 7px 11px 5px 13px; }"
      "QProgressBar { background-color: #ffffff; border: "
      "1px solid #d1d1d6; "
      "border-radius: 6px; text-align: center; color: #1a1a1b; }"
      "QProgressBar::chunk { background-color: #ffae00; border-radius: 4px; "
      "}"
      "QCheckBox, QRadioButton { color: #1a1a1b; spacing: 8px; }"
      "QCheckBox:disabled, QRadioButton:disabled { color: #8e8e93; }"
      "QCheckBox::indicator, QRadioButton::indicator { width: 18px; height: "
      "18px; "
      "background-color: #ffffff; border: 1px solid #d1d1d6; border-radius: "
      "6px; }"
      "QCheckBox::indicator:hover, QRadioButton::indicator:hover { border: "
      "1px solid #ffae00; }"
      "QCheckBox::indicator:checked, QCheckBox::indicator:indeterminate, "
      "QRadioButton::indicator:checked { background-color: #ffae00; border: "
      "2px solid #333333; }"
      "QToolBar#instanceToolBar QToolButton { min-width: 154px; max-width: "
      "154px; padding: 6px; }"
      "QGroupBox { border: 1px solid #d1d1d6; border-radius: 8px; "
      "margin-top: 2ex; padding-top: 2ex; font-weight: bold; "
      "color: #1a1a1b; }"
      "QGroupBox QPushButton { min-width: 130px; }"
      "QGroupBox::title { subcontrol-origin: margin; subcontrol-position: "
      "top left; padding: 0 4px; color: #ffae00; }"
      "QStatusBar { border-top: 1px solid #d1d1d6; }"
      "QStatusBar QLabel#playtimeLabel { padding-right: 20px; "
      "padding-bottom: 6px; }";
  return qss.replace("#ffae00", customAccent);
}
