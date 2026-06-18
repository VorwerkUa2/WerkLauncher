template <typename T> class Translated {

public:
  Translated() {}

  Translated(QWidget *parent) { m_contained = new T(parent); }

  void setTooltipId(const char *tooltip) { m_tooltip = tooltip; }

  void setTextId(const char *text) { m_text = text; }

  operator T *() { return m_contained; }

  T *operator->() { return m_contained; }

  void retranslate() {

    if (m_text) {

      QString result;

      result = QApplication::translate("MainWindow", m_text);

      if (result.contains("%1")) {

        result = result.arg(BuildConfig.LAUNCHER_NAME);
      }

      m_contained->setText(result);
    }

    if (m_tooltip) {

      QString result;

      result = QApplication::translate("MainWindow", m_tooltip);

      if (result.contains("%1")) {

        result = result.arg(BuildConfig.LAUNCHER_NAME);
      }

      m_contained->setToolTip(result);
    }
  }

private:
  T *m_contained = nullptr;

  const char *m_text = nullptr;

  const char *m_tooltip = nullptr;
};

using TranslatedAction = Translated<QAction>;

using TranslatedToolButton = Translated<QToolButton>;

class TranslatedToolbar {

public:
  TranslatedToolbar() {}

  TranslatedToolbar(QWidget *parent) { m_contained = new QToolBar(parent); }

  void setWindowTitleId(const char *title) { m_title = title; }

  operator QToolBar *() { return m_contained; }

  QToolBar *operator->() { return m_contained; }

  void retranslate() {

    if (m_title) {

      m_contained->setWindowTitle(

          QApplication::translate("MainWindow", m_title));
    }
  }

private:
  QToolBar *m_contained = nullptr;

  const char *m_title = nullptr;
};

class MainWindow::Ui {

public:
  TranslatedAction actionBack;
  TranslatedAction actionAddInstance;

  // TranslatedAction actionRefresh;

  TranslatedAction actionCheckUpdate;

  TranslatedAction actionSettings;

  TranslatedAction actionMoreNews;

  TranslatedAction actionManageAccounts;

  TranslatedAction actionLaunchInstance;

  TranslatedAction actionRenameInstance;

  TranslatedAction actionChangeInstGroup;

  TranslatedAction actionChangeInstIcon;

  TranslatedAction actionEditInstNotes;

  TranslatedAction actionEditInstance;

  TranslatedAction actionWorlds;

  TranslatedAction actionMods;

  TranslatedAction actionViewSelectedInstFolder;

  TranslatedAction actionViewSelectedMCFolder;

  TranslatedAction actionViewSelectedModsFolder;

  TranslatedAction actionDeleteInstance;

  TranslatedAction actionConfig_Folder;

  TranslatedAction actionCopyInstance;

  TranslatedAction actionScreenshots;

  TranslatedAction actionExportInstance;

  TranslatedAction actionCreateShortcut;

  QVector<TranslatedAction *> all_actions;

  LabeledToolButton *renameButton = nullptr;

  LabeledToolButton *changeIconButton = nullptr;

  QMenu *foldersMenu = nullptr;

  TranslatedToolButton foldersMenuButton;

  TranslatedAction actionViewInstanceFolder;

  TranslatedAction actionViewCentralModsFolder;

  QMenu *helpMenu = nullptr;

  TranslatedToolButton helpMenuButton;

  TranslatedAction actionReportBug;

  TranslatedAction actionDISCORD;

  TranslatedAction actionTELEGRAM;

  TranslatedAction actionAbout;

  QVector<TranslatedToolButton *> all_toolbuttons;

  QWidget *centralWidget = nullptr;

  QHBoxLayout *horizontalLayout = nullptr;

  QStatusBar *statusBar = nullptr;

  TranslatedToolbar mainToolBar;



  TranslatedToolbar newsToolBar;

  QVector<TranslatedToolbar *> all_toolbars;

  bool m_kill = false;

  void updateLaunchAction() {

    if (m_kill) {

      actionLaunchInstance.setTextId(QT_TRANSLATE_NOOP("MainWindow", "Kill"));

      actionLaunchInstance.setTooltipId(

          QT_TRANSLATE_NOOP("MainWindow", "Kill the running instance"));

    } else {

      actionLaunchInstance.setTextId(QT_TRANSLATE_NOOP("MainWindow", "Launch"));

      actionLaunchInstance.setTooltipId(

          QT_TRANSLATE_NOOP("MainWindow", "Launch the selected instance."));
    }

    actionLaunchInstance.retranslate();
  }

  void setLaunchAction(bool kill) {

    m_kill = kill;

    updateLaunchAction();
  }

  void createMainToolbar(QMainWindow *MainWindow) {

    mainToolBar = TranslatedToolbar(MainWindow);

    mainToolBar->setObjectName(QStringLiteral("mainToolBar"));

    mainToolBar->setMovable(false);

    mainToolBar->setAllowedAreas(Qt::LeftToolBarArea);

    mainToolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);

    mainToolBar->setFloatable(false);



    mainToolBar.setWindowTitleId(

        QT_TRANSLATE_NOOP("MainWindow", "Main Toolbar"));

    actionAddInstance = TranslatedAction(MainWindow);

    actionAddInstance->setObjectName(QStringLiteral("actionAddInstance"));

    actionAddInstance->setIcon(APPLICATION->getThemedIcon("new"));

    actionAddInstance.setTextId(

        QT_TRANSLATE_NOOP("MainWindow", "Add Instance"));

    actionAddInstance.setTooltipId(

        QT_TRANSLATE_NOOP("MainWindow", "Add a new instance."));

    all_actions.append(&actionAddInstance);

    actionBack = TranslatedAction(MainWindow);
    actionBack->setObjectName(QStringLiteral("actionBack"));
    actionBack->setIcon(APPLICATION->getThemedIcon("back")); // assuming a back icon exists, fallback to home/up or standard text if none
    actionBack.setTextId(QT_TRANSLATE_NOOP("MainWindow", "Back"));
    actionBack.setTooltipId(QT_TRANSLATE_NOOP("MainWindow", "Go back to the instance list."));
    all_actions.append(&actionBack);
    mainToolBar->addAction(actionBack);
    actionBack->setVisible(false); // Only visible in sub-pages

    QWidget *topSpacer = new QWidget(MainWindow);
    topSpacer->setFixedSize(1, 72);
    mainToolBar->addWidget(topSpacer);

    mainToolBar->addAction(actionAddInstance);

    mainToolBar->addSeparator();

    foldersMenu = new QMenu(MainWindow);

    foldersMenu->setToolTipsVisible(true);

    actionViewInstanceFolder = TranslatedAction(MainWindow);

    actionViewInstanceFolder->setObjectName(

        QStringLiteral("actionViewInstanceFolder"));

    actionViewInstanceFolder->setIcon(APPLICATION->getThemedIcon("viewfolder"));

    actionViewInstanceFolder.setTextId(

        QT_TRANSLATE_NOOP("MainWindow", "View Instance Folder"));

    actionViewInstanceFolder.setTooltipId(QT_TRANSLATE_NOOP(

        "MainWindow", "Open the instance folder in a file browser."));

    all_actions.append(&actionViewInstanceFolder);

    foldersMenu->addAction(actionViewInstanceFolder);

    actionViewCentralModsFolder = TranslatedAction(MainWindow);

    actionViewCentralModsFolder->setObjectName(

        QStringLiteral("actionViewCentralModsFolder"));

    actionViewCentralModsFolder->setIcon(

        APPLICATION->getThemedIcon("centralmods"));

    actionViewCentralModsFolder.setTextId(

        QT_TRANSLATE_NOOP("MainWindow", "View Central Mods Folder"));

    actionViewCentralModsFolder.setTooltipId(QT_TRANSLATE_NOOP(

        "MainWindow", "Open the central mods folder in a file browser."));

    all_actions.append(&actionViewCentralModsFolder);

    foldersMenu->addAction(actionViewCentralModsFolder);

    foldersMenuButton = TranslatedToolButton(MainWindow);

    foldersMenuButton.setTextId(QT_TRANSLATE_NOOP("MainWindow", "Folders"));

    foldersMenuButton.setTooltipId(QT_TRANSLATE_NOOP(

        "MainWindow", "Open one of the folders shared between instances."));

    foldersMenuButton->setMenu(foldersMenu);

    foldersMenuButton->setPopupMode(QToolButton::InstantPopup);

    foldersMenuButton->setToolButtonStyle(Qt::ToolButtonIconOnly);

    foldersMenuButton->setIcon(APPLICATION->getThemedIcon("viewfolder"));

    foldersMenuButton->setFocusPolicy(Qt::NoFocus);

    all_toolbuttons.append(&foldersMenuButton);

    QWidgetAction *foldersButtonAction = new QWidgetAction(MainWindow);

    foldersButtonAction->setDefaultWidget(foldersMenuButton);

    mainToolBar->addAction(foldersButtonAction);

    actionSettings = TranslatedAction(MainWindow);

    actionSettings->setObjectName(QStringLiteral("actionSettings"));

    actionSettings->setIcon(APPLICATION->getThemedIcon("settings"));

    actionSettings->setMenuRole(QAction::PreferencesRole);

    actionSettings.setTextId(QT_TRANSLATE_NOOP("MainWindow", "Settings"));

    actionSettings.setTooltipId(

        QT_TRANSLATE_NOOP("MainWindow", "Change settings."));

    all_actions.append(&actionSettings);

    mainToolBar->addAction(actionSettings);

    helpMenu = new QMenu(MainWindow);

    helpMenu->setToolTipsVisible(true);

    if (!BuildConfig.BUG_TRACKER_URL.isEmpty()) {

      actionReportBug = TranslatedAction(MainWindow);

      actionReportBug->setObjectName(QStringLiteral("actionReportBug"));

      actionReportBug->setIcon(APPLICATION->getThemedIcon("bug"));

      actionReportBug.setTextId(

          QT_TRANSLATE_NOOP("MainWindow", "Report a Bug"));

      actionReportBug.setTooltipId(QT_TRANSLATE_NOOP(

          "MainWindow", "Open the bug tracker to report a bug with %1."));

      all_actions.append(&actionReportBug);

      helpMenu->addAction(actionReportBug);
    }

    if (!BuildConfig.DISCORD_URL.isEmpty()) {

      actionDISCORD = TranslatedAction(MainWindow);

      actionDISCORD->setObjectName(QStringLiteral("actionDISCORD"));

      actionDISCORD->setIcon(APPLICATION->getThemedIcon("discord"));

      actionDISCORD.setTextId(QT_TRANSLATE_NOOP("MainWindow", "Discord"));

      actionDISCORD.setTooltipId(

          QT_TRANSLATE_NOOP("MainWindow", "Open %1 discord voice chat."));

      all_actions.append(&actionDISCORD);

      helpMenu->addAction(actionDISCORD);
    }

    actionTELEGRAM = TranslatedAction(MainWindow);

    actionTELEGRAM->setObjectName(QStringLiteral("actionTELEGRAM"));

    actionTELEGRAM->setIcon(APPLICATION->getThemedIcon(

        "telegram")); // Need Telegram icon or generic help icon

    actionTELEGRAM.setTextId(QT_TRANSLATE_NOOP("MainWindow", "Telegram"));

    actionTELEGRAM.setTooltipId(

        QT_TRANSLATE_NOOP("MainWindow", "Open %1 telegram chat."));

    all_actions.append(&actionTELEGRAM);

    helpMenu->addAction(actionTELEGRAM);

    actionAbout = TranslatedAction(MainWindow);

    actionAbout->setObjectName(QStringLiteral("actionAbout"));

    actionAbout->setIcon(APPLICATION->getThemedIcon("about"));

    actionAbout->setMenuRole(QAction::AboutRole);

    actionAbout.setTextId(QT_TRANSLATE_NOOP("MainWindow", "About %1"));

    actionAbout.setTooltipId(

        QT_TRANSLATE_NOOP("MainWindow", "View information about %1."));

    all_actions.append(&actionAbout);

    helpMenu->addAction(actionAbout);

    helpMenuButton = TranslatedToolButton(MainWindow);

    helpMenuButton.setTextId(QT_TRANSLATE_NOOP("MainWindow", "Help"));

    helpMenuButton.setTooltipId(

        QT_TRANSLATE_NOOP("MainWindow", "Get help with %1 or Minecraft."));

    helpMenuButton->setMenu(helpMenu);

    helpMenuButton->setPopupMode(QToolButton::InstantPopup);

    helpMenuButton->setToolButtonStyle(Qt::ToolButtonIconOnly);

    helpMenuButton->setIcon(APPLICATION->getThemedIcon("help"));

    helpMenuButton->setFocusPolicy(Qt::NoFocus);

    all_toolbuttons.append(&helpMenuButton);

    QWidgetAction *helpButtonAction = new QWidgetAction(MainWindow);

    helpButtonAction->setDefaultWidget(helpMenuButton);

    mainToolBar->addAction(helpButtonAction);

    if (BuildConfig.UPDATER_ENABLED) {

      actionCheckUpdate = TranslatedAction(MainWindow);

      actionCheckUpdate->setObjectName(QStringLiteral("actionCheckUpdate"));

      actionCheckUpdate->setIcon(APPLICATION->getThemedIcon("checkupdate"));

      actionCheckUpdate.setTextId(QT_TRANSLATE_NOOP("MainWindow", "Update"));

      actionCheckUpdate.setTooltipId(

          QT_TRANSLATE_NOOP("MainWindow", "Check for new updates for %1."));

      all_actions.append(&actionCheckUpdate);

      mainToolBar->addAction(actionCheckUpdate);
    }

    mainToolBar->addSeparator();

    // profile menu and its actions

    actionManageAccounts = TranslatedAction(MainWindow);

    actionManageAccounts->setObjectName(QStringLiteral("actionManageAccounts"));

    actionManageAccounts.setTextId(

        QT_TRANSLATE_NOOP("MainWindow", "Manage Accounts"));

    // FIXME: no tooltip!

    actionManageAccounts->setCheckable(false);

    actionManageAccounts->setIcon(APPLICATION->getThemedIcon("accounts"));

    all_actions.append(&actionManageAccounts);

    all_toolbars.append(&mainToolBar);

    MainWindow->addToolBar(Qt::LeftToolBarArea, mainToolBar);
  }

  void createStatusBar(QMainWindow *MainWindow) {

    statusBar = new QStatusBar(MainWindow);

    statusBar->setObjectName(QStringLiteral("statusBar"));

    MainWindow->setStatusBar(statusBar);
  }

  void createInstanceActions(QMainWindow *MainWindow) {
    actionChangeInstIcon = TranslatedAction(MainWindow);
    actionChangeInstIcon->setObjectName(QStringLiteral("actionChangeInstIcon"));
    actionChangeInstIcon->setIcon(APPLICATION->getThemedIcon("logo"));
    actionChangeInstIcon->setIconVisibleInMenu(true);
    actionChangeInstIcon.setTextId(
        QT_TRANSLATE_NOOP("MainWindow", "Change Icon"));
    actionChangeInstIcon.setTooltipId(QT_TRANSLATE_NOOP(
        "MainWindow", "Change the selected instance's icon."));
    all_actions.append(&actionChangeInstIcon);

    actionRenameInstance = TranslatedAction(MainWindow);
    actionRenameInstance->setObjectName(QStringLiteral("actionRenameInstance"));
    actionRenameInstance.setTextId(QT_TRANSLATE_NOOP("MainWindow", "Rename"));
    actionRenameInstance.setTooltipId(
        QT_TRANSLATE_NOOP("MainWindow", "Rename the selected instance."));
    all_actions.append(&actionRenameInstance);

    actionChangeInstGroup = TranslatedAction(MainWindow);
    actionChangeInstGroup->setObjectName(
        QStringLiteral("actionChangeInstGroup"));
    actionChangeInstGroup.setTextId(
        QT_TRANSLATE_NOOP("MainWindow", "Change Group"));
    actionChangeInstGroup.setTooltipId(QT_TRANSLATE_NOOP(
        "MainWindow", "Change the selected instance's group."));
    all_actions.append(&actionChangeInstGroup);

    actionLaunchInstance = TranslatedAction(MainWindow);
    actionLaunchInstance->setObjectName(QStringLiteral("actionLaunchInstance"));
    all_actions.append(&actionLaunchInstance);

    actionEditInstance = TranslatedAction(MainWindow);
    actionEditInstance->setObjectName(QStringLiteral("actionEditInstance"));
    actionEditInstance.setTextId(
        QT_TRANSLATE_NOOP("MainWindow", "Edit Instance"));
    actionEditInstance.setTooltipId(QT_TRANSLATE_NOOP(
        "MainWindow", "Change the instance settings, mods and versions."));
    all_actions.append(&actionEditInstance);

    actionEditInstNotes = TranslatedAction(MainWindow);
    actionEditInstNotes->setObjectName(QStringLiteral("actionEditInstNotes"));
    actionEditInstNotes.setTextId(
        QT_TRANSLATE_NOOP("MainWindow", "Edit Notes"));
    actionEditInstNotes.setTooltipId(QT_TRANSLATE_NOOP(
        "MainWindow", "Edit the notes for the selected instance."));
    all_actions.append(&actionEditInstNotes);

    actionMods = TranslatedAction(MainWindow);
    actionMods->setObjectName(QStringLiteral("actionMods"));
    actionMods.setTextId(QT_TRANSLATE_NOOP("MainWindow", "View Mods"));
    actionMods.setTooltipId(
        QT_TRANSLATE_NOOP("MainWindow", "View the mods of this instance."));
    all_actions.append(&actionMods);

    actionWorlds = TranslatedAction(MainWindow);
    actionWorlds->setObjectName(QStringLiteral("actionWorlds"));
    actionWorlds.setTextId(QT_TRANSLATE_NOOP("MainWindow", "View Worlds"));
    actionWorlds.setTooltipId(
        QT_TRANSLATE_NOOP("MainWindow", "View the worlds of this instance."));
    all_actions.append(&actionWorlds);

    actionScreenshots = TranslatedAction(MainWindow);
    actionScreenshots->setObjectName(QStringLiteral("actionScreenshots"));
    actionScreenshots.setTextId(
        QT_TRANSLATE_NOOP("MainWindow", "Manage Screenshots"));
    actionScreenshots.setTooltipId(QT_TRANSLATE_NOOP(
        "MainWindow", "View and upload screenshots for this instance."));
    all_actions.append(&actionScreenshots);

    actionViewSelectedMCFolder = TranslatedAction(MainWindow);
    actionViewSelectedMCFolder->setObjectName(
        QStringLiteral("actionViewSelectedMCFolder"));
    actionViewSelectedMCFolder.setTextId(
        QT_TRANSLATE_NOOP("MainWindow", "Minecraft Folder"));
    actionViewSelectedMCFolder.setTooltipId(QT_TRANSLATE_NOOP(
        "MainWindow",
        "Open the selected instance's minecraft folder in a file browser."));
    all_actions.append(&actionViewSelectedMCFolder);

    actionConfig_Folder = TranslatedAction(MainWindow);
    actionConfig_Folder->setObjectName(QStringLiteral("actionConfig_Folder"));
    actionConfig_Folder.setTextId(
        QT_TRANSLATE_NOOP("MainWindow", "Config Folder"));
    actionConfig_Folder.setTooltipId(
        QT_TRANSLATE_NOOP("MainWindow", "Open the instance's config folder."));
    all_actions.append(&actionConfig_Folder);

    actionViewSelectedInstFolder = TranslatedAction(MainWindow);
    actionViewSelectedInstFolder->setObjectName(
        QStringLiteral("actionViewSelectedInstFolder"));
    actionViewSelectedInstFolder.setTextId(
        QT_TRANSLATE_NOOP("MainWindow", "Instance Folder"));
    actionViewSelectedInstFolder.setTooltipId(QT_TRANSLATE_NOOP(
        "MainWindow",
        "Open the selected instance's root folder in a file browser."));
    all_actions.append(&actionViewSelectedInstFolder);

    actionCreateShortcut = TranslatedAction(MainWindow);
    actionCreateShortcut->setObjectName(QStringLiteral("actionCreateShortcut"));
    actionCreateShortcut.setTextId(
        QT_TRANSLATE_NOOP("MainWindow", "Create Shortcut"));
    actionCreateShortcut.setTooltipId(QT_TRANSLATE_NOOP(
        "MainWindow", "Create a shortcut that launches the selected instance"));
    all_actions.append(&actionCreateShortcut);

    actionExportInstance = TranslatedAction(MainWindow);
    actionExportInstance->setObjectName(QStringLiteral("actionExportInstance"));
    actionExportInstance.setTextId(
        QT_TRANSLATE_NOOP("MainWindow", "Export Instance"));
    actionExportInstance.setTooltipId(QT_TRANSLATE_NOOP(
        "MainWindow", "Export the selected instance as a zip file."));
    all_actions.append(&actionExportInstance);

    actionDeleteInstance = TranslatedAction(MainWindow);
    actionDeleteInstance->setObjectName(QStringLiteral("actionDeleteInstance"));
    actionDeleteInstance.setTextId(QT_TRANSLATE_NOOP("MainWindow", "Delete"));
    actionDeleteInstance.setTooltipId(
        QT_TRANSLATE_NOOP("MainWindow", "Delete the selected instance."));
    all_actions.append(&actionDeleteInstance);

    actionCopyInstance = TranslatedAction(MainWindow);
    actionCopyInstance->setObjectName(QStringLiteral("actionCopyInstance"));
    actionCopyInstance->setIcon(APPLICATION->getThemedIcon("copy"));
    actionCopyInstance.setTextId(
        QT_TRANSLATE_NOOP("MainWindow", "Copy Instance"));
    actionCopyInstance.setTooltipId(
        QT_TRANSLATE_NOOP("MainWindow", "Copy the selected instance."));
    all_actions.append(&actionCopyInstance);
  }

  void setupUi(QMainWindow *MainWindow) {

    if (MainWindow->objectName().isEmpty()) {

      MainWindow->setObjectName(QStringLiteral("MainWindow"));
    }

    MainWindow->resize(800, 600);

    MainWindow->setWindowIcon(APPLICATION->getThemedIcon("logo"));

    MainWindow->setWindowTitle(BuildConfig.LAUNCHER_DISPLAYNAME);

#ifndef QT_NO_ACCESSIBILITY

    MainWindow->setAccessibleName(BuildConfig.LAUNCHER_NAME);

#endif

    createMainToolbar(MainWindow);

    centralWidget = new QWidget(MainWindow);

    centralWidget->setObjectName(QStringLiteral("centralWidget"));

    horizontalLayout = new QHBoxLayout(centralWidget);

    horizontalLayout->setSpacing(0);

    horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));

    horizontalLayout->setSizeConstraint(QLayout::SetDefaultConstraint);

    horizontalLayout->setContentsMargins(0, 0, 0, 0);

    centralWidget->setMinimumSize(1, 1);

    MainWindow->setCentralWidget(centralWidget);

    createStatusBar(MainWindow);

    qDebug() << "setupUi: createInstanceActions";
    createInstanceActions(MainWindow);

    retranslateUi(MainWindow);

    QMetaObject::connectSlotsByName(MainWindow);

  } // setupUi

  void retranslateUi(QMainWindow *MainWindow) {

    QString winTitle = tr("%1 - Version %2", "Launcher - Version X")

                           .arg(BuildConfig.LAUNCHER_DISPLAYNAME,

                                BuildConfig.printableVersionString());

    if (!BuildConfig.BUILD_PLATFORM.isEmpty()) {

      winTitle += tr(" on %1", "on platform, as in operating system")

                      .arg(BuildConfig.BUILD_PLATFORM);
    }

    MainWindow->setWindowTitle(winTitle);

    // all the actions

    for (auto *item : all_actions) {

      item->retranslate();
    }

    for (auto *item : all_toolbars) {

      item->retranslate();
    }

    for (auto *item : all_toolbuttons) {

      item->retranslate();
    }

    // submenu buttons

    foldersMenuButton->setText(tr("Folders"));

    helpMenuButton->setText(tr("Help"));

  } // retranslateUi
};
