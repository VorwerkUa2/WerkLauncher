#include "DarkTheme.h"
#include "Application.h"
#include "settings/SettingsObject.h"
#include <QColor>
#include <QObject>
#include <QPalette>

QString DarkTheme::id() { return "dark"; }
bool DarkTheme::isDark() { return true; }

QString DarkTheme::name() { return QObject::tr("Modern Black"); }

bool DarkTheme::hasColorScheme() { return true; }

QPalette DarkTheme::colorScheme() {
  QPalette darkPalette;
  QColor windowColor(26, 26, 27);
  QColor baseColor(37, 37, 38);
  QString customAccent =
      APPLICATION->settings()->get("CustomAccentColor").toString();
  QColor accentColor(customAccent);

  darkPalette.setColor(QPalette::Window, windowColor);
  darkPalette.setColor(QPalette::WindowText, Qt::white);
  darkPalette.setColor(QPalette::Base, baseColor);
  darkPalette.setColor(QPalette::AlternateBase, windowColor);
  darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
  darkPalette.setColor(QPalette::ToolTipText, Qt::white);
  darkPalette.setColor(QPalette::Text, Qt::white);
  darkPalette.setColor(QPalette::Button, windowColor);
  darkPalette.setColor(QPalette::ButtonText, Qt::white);
  darkPalette.setColor(QPalette::BrightText, accentColor);
  darkPalette.setColor(QPalette::Link, accentColor);
  darkPalette.setColor(QPalette::Highlight, accentColor);
  darkPalette.setColor(QPalette::HighlightedText, Qt::black);
  return fadeInactive(darkPalette, fadeAmount(), fadeColor());
}

double DarkTheme::fadeAmount() { return 0.5; }

QColor DarkTheme::fadeColor() { return QColor(26, 26, 27); }

bool DarkTheme::hasStyleSheet() { return true; }

QString DarkTheme::appStyleSheet() {
  QString customAccent =
      APPLICATION->settings()->get("CustomAccentColor").toString();
  QString qss =
      "QWidget { background: transparent; border: none; font-family: "
      "'Segoe "
      "UI', 'SF Pro Display', 'Helvetica Neue', 'Ubuntu', sans-serif; }"
      "QWizard#SetupWizard { background-color: #1a1a1b; border: 1px solid #333333; }"
      "QMainWindow { background-color: %1; }"
      "QDialog, QDockWidget { background-color: "
      "#1a1a1b; }"
      "QDockWidget::title { background-color: #1a1a1b; "
      "color: "
      "#ffffff; "
      "padding: 4px; }"
      "QStatusBar { background-color: #1a1a1b; color: "
      "#ffffff; }"
      "QMenuBar { background-color: #252526; color: #ffffff; "
      "padding: 2px; }"
      "QToolBar { background-color: #1a1a1b; "
      "border-bottom: 1px solid #2a2a2b; "
      "padding: 4px; "
      "margin: 0px; spacing: 4px; border-radius: 8px; }"
      "QToolBar::separator { width: 1px; background-color: #2a2a2b; "
      "margin: 4px 8px; }"
      "QMenuBar::item { background: transparent; padding: 4px 10px; "
      "margin: 2px; border-radius: 4px; }"
      "QMenuBar::item:selected { background-color: #333333; color: "
      "#ffae00; "
      "}"
      "QMenuBar::item:pressed { padding: 5px 9px 3px 11px; }"
      "QMenu { background-color: #1a1a1b; border: 1px solid "
      "#2a2a2b; "
      "border-radius: 8px; padding: 4px; }"
      "QMenu::item { padding: 4px 18px 4px 8px; border-radius: 4px; "
      "color: #ffffff; margin: 1px 2px; }"
      "QMenu::item:selected { background-color: #ffae00; color: #000000; }"
      "QMenu::item:pressed { padding: 5px 17px 3px 9px; }"
      "QToolTip { color: #000000; background-color: #ffae00; border: 1px "
      "solid #ffae00; border-radius: 6px; padding: 4px; }"
      "QPushButton, QToolButton { background-color: #252526; "
      "border: 1px solid #333333; "
      "border-radius: 6px; padding: 6px 14px; color: #ffffff; }"
      "QPushButton:hover, QToolButton:hover { background-color: #333333; "
      "border: 1px solid #ffae00; color: #ffffff; }"
      "QToolButton { spacing: 48px; }"
      "QToolButton:checked, QToolButton:on, QToolButton:open { background-color: #ffae00; "
      "border: 1px solid #ffae00; color: #000000; }"
      "QPushButton:pressed, QToolButton:pressed { background-color: "
      "#ffae00; border: 1px solid #ffae00; color: #000000; }"
      "QPushButton:disabled, QToolButton:disabled { background-color: "
      "#1e1e1f; border: 1px solid #2a2a2b; color: #555555; }"
      "QLineEdit, QTextEdit, QPlainTextEdit { background-color: "
      "#252526; "
      "border: 1px solid #333333; border-radius: 8px; padding: 4px; color: "
      "#ffffff; selection-background-color: #ffae00; }"
      "QLineEdit:focus { border: 1px solid #ffae00; }"
      "QComboBox { background-color: #252526; border: 1px "
      "solid #333333; "
      "border-radius: 8px; padding: 4px; color: #ffffff; }"
      "QComboBox::drop-down { border: none; width: 24px; }"
      "QComboBox::down-arrow { image: none; border-left: 4px solid "
      "transparent; "
      "border-right: 4px solid transparent; border-top: 4px solid #ffae00; "
      "width: 0; height: 0; margin-right: 8px; }"
      "QComboBox QAbstractItemView { background-color: #1a1a1b; "
      "border: 1px "
      "solid #333333; selection-background-color: #ffae00; "
      "selection-color: #000000; outline: none; }"
      "QComboBox QAbstractItemView::item { padding: 8px; color: #ffffff; "
      "}"
      "QComboBox QAbstractItemView::item:selected { background-color: "
      "#ffae00; color: #000000; }"
      "QListView, QTreeView, QTableView, InstanceView { background-color: "
      "%2; "
      "border: 1px solid #2a2a2b; border-radius: 8px; "
      "outline: none; }"
      "QListView::indicator, QTreeView::indicator { width: 14px; height: "
      "14px; "
      "background-color: #252526; border: 1px solid #333333; "
      "border-radius: "
      "4px; }"
      "QListView::indicator:checked, QTreeView::indicator:checked { "
      "background-color: #ffae00; "
      "border: 2px solid #ffffff; }"
      "QListView::item { padding: 8px; border-radius: 8px; color: "
      "#ffffff; }"
      "QListView::item:hover { background-color: #252526; "
      "border: 1px solid #ffae00; }"
      "QListView::item:pressed { background-color: #333333; padding: 9px "
      "7px 7px 9px; }"
      "QListView::item:selected { "
      "background-color: #333333; color: #ffae00; border: 1px solid "
      "#ffae00; "
      "border-radius: 8px; }"
      "QScrollBar:vertical { border: none; background: transparent; width: "
      "8px; margin: 2px; }"
      "QScrollBar:horizontal { border: none; background: transparent; "
      "height: 8px; margin: 2px; }"
      "QScrollBar::handle:vertical, QScrollBar::handle:horizontal { "
      "background: #3a3a3c; min-width: 20px; "
      "min-height: 20px; "
      "border-radius: 4px; }"
      "QScrollBar::handle:vertical:hover, "
      "QScrollBar::handle:horizontal:hover { "
      "background: #ffae00; }"
      "QScrollBar::add-line, QScrollBar::sub-line { border: none; "
      "background: none; }"
      "QScrollBar::add-page, QScrollBar::sub-page { background: none; }"
      "QHeaderView::section { background-color: #252526; "
      "padding: 4px; "
      "border: 1px solid #333333; color: #ffffff; }"
      "QTabWidget::pane { border: 1px solid #333333; top: -1px; "
      "border-radius: 8px; }"
      "QTabBar::tab { background-color: #1a1a1b; border: 1px "
      "solid #333333; "
      "border-radius: 8px; padding: 6px 12px; margin: 2px; color: #ffffff; "
      "}"
      "QTabBar::tab:hover { border: 1px solid #ffae00; color: #ffae00; }"
      "QTabBar::tab:selected { background-color: #ffae00; color: #000000; "
      "border: 1px solid #ffae00; }"
      "QTabBar::tab:disabled { color: #555555; border: 1px solid #2a2a2b; "
      "background-color: #1e1e1f; }"
      "QTabBar::tab:pressed { padding: 7px 11px 5px 13px; }"
      "QComboBox:disabled { background-color: #1e1e1f; border: 1px solid "
      "#2a2a2b; color: #555555; }"
      "QLineEdit:disabled, QTextEdit:disabled, QPlainTextEdit:disabled { "
      "background-color: #1e1e1f; border: 1px solid #2a2a2b; "
      "color: #555555; }"
      "QCheckBox:disabled, QRadioButton:disabled { color: #555555; }"
      "QProgressBar { background-color: #252526; border: 1px "
      "solid #333333; "
      "border-radius: 4px; text-align: center; color: #ffffff; }"
      "QProgressBar::chunk { background-color: #ffae00; border-radius: "
      "3px; "
      "}"
      "QCheckBox, QRadioButton { color: #ffffff; spacing: 8px; }"
      "QCheckBox::indicator, QRadioButton::indicator { width: 18px; "
      "height: "
      "18px; "
      "background-color: #252526; border: 1px solid #333333; "
      "border-radius: "
      "4px; }"
      "QCheckBox::indicator:hover, QRadioButton::indicator:hover { "
      "border: "
      "1px solid #ffae00; }"
      "QCheckBox::indicator:checked, QCheckBox::indicator:indeterminate, "
      "QRadioButton::indicator:checked { background-color: #ffae00; "
      "border: "
      "2px solid #ffffff; }"
      "QToolBar#instanceToolBar QToolButton { min-width: 154px; "
      "max-width: "
      "154px; padding: 6px; }"
      "QToolBar#mainToolBar QToolButton { "
      "   min-width: 48px; max-width: 48px; "
      "   min-height: 48px; max-height: 48px; "
      "   padding: 0px; margin: 2px; border-radius: 8px; "
      "   background: transparent; border: 1px solid transparent; "
      "}"
      "QToolBar#mainToolBar QToolButton:hover { "
      "   background-color: #333333; border: 1px solid #ffae00; "
      "}"
      "QToolBar#mainToolBar QToolButton:pressed, QToolBar#mainToolBar QToolButton:checked, QToolBar#mainToolBar QToolButton:on { "
      "   background-color: #ffae00; border: 1px solid #ffae00; color: #000000; "
      "}"
      "QToolBar#mainToolBar QToolButton::menu-indicator { image: none; width: 0px; }"
      "QGroupBox { border: 1px solid #333333; border-radius: 8px; "
      "margin-top: 2ex; padding-top: 2ex; font-weight: bold; "
      "color: #ffffff; }"
      "QGroupBox QPushButton { min-width: 130px; }"
      "QGroupBox::title { subcontrol-origin: margin; subcontrol-position: "
      "top left; padding: 0 4px; color: #ffae00; }"
      "QStatusBar { border-top: 1px solid #333333; }"
      "QStatusBar QLabel#playtimeLabel { padding-right: 20px; "
      "padding-bottom: 6px; }";
  bool hasBg = !APPLICATION->settings()->get("CustomBackgroundImage").toString().isEmpty();
  qss = qss.replace("%1", hasBg ? "transparent" : "#1a1a1b");
  qss = qss.replace("%2", hasBg ? "transparent" : "#141415");
  return qss.replace("#ffae00", customAccent);
}
