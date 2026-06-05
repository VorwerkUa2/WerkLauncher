void MainWindow::on_actionCopyInstance_triggered() {
  if (!m_selectedInstance)
    return;
  CopyInstanceDialog copyInstDlg(m_selectedInstance, this);
  if (!copyInstDlg.exec())
    return;
  auto copyTask =
      new InstanceCopyTask(m_selectedInstance, copyInstDlg.shouldCopySaves(),
                           copyInstDlg.shouldKeepPlaytime());
  copyTask->setName(copyInstDlg.instName());
  copyTask->setGroup(copyInstDlg.instGroup());
  copyTask->setIcon(copyInstDlg.iconKey());
  unique_qobject_ptr<Task> task(
      APPLICATION->instances()->wrapInstanceTask(copyTask));
  runModalTask(task.get());
}

void MainWindow::on_actionAddInstance_triggered() { addInstance(); }

void MainWindow::droppedURLs(QList<QUrl> urls) {
  for (auto &url : urls) {
    ToastNotification::show(this, tr("Processing dropped content..."),
                            ToastNotification::Info);
    if (url.isLocalFile()) {
      addInstance(url.toLocalFile());
    } else {
      addInstance(url.toString());
    }
    // Only process one dropped file...
    break;
  }
}

void MainWindow::on_actionTELEGRAM_triggered() {
  DesktopServices::openUrl(QUrl(BuildConfig.TELEGRAM_URL));
}

void MainWindow::on_actionPatreon_triggered() {
  DesktopServices::openUrl(QUrl(BuildConfig.PATREON_URL));
}

void MainWindow::on_actionMoreNews_triggered() {
  DesktopServices::openUrl(QUrl(BuildConfig.NEWS_URL));
}

void MainWindow::on_actionLaunchInstanceOffline_triggered() {
  if (m_selectedInstance) {
    APPLICATION->launch(m_selectedInstance, false);
  }
}

void MainWindow::on_actionDISCORD_triggered() {
  DesktopServices::openUrl(QUrl(BuildConfig.DISCORD_URL));
}

void MainWindow::on_actionChangeInstIcon_triggered() {
  if (!m_selectedInstance)
    return;
  IconPickerDialog dlg(this);
  dlg.execWithSelection(m_selectedInstance->iconKey());
  if (dlg.result() == QDialog::Accepted) {
    m_selectedInstance->setIconKey(dlg.selectedIconKey);
    auto icon = APPLICATION->icons()->getIcon(dlg.selectedIconKey);
    ui->actionChangeInstIcon->setIcon(icon);
    ui->changeIconButton->setIcon(icon);
  }
}

void MainWindow::on_actionChangeInstGroup_triggered() {
  if (!m_selectedInstance)
    return;
  bool ok = false;
  InstanceId instId = m_selectedInstance->id();
  QString name(APPLICATION->instances()->getInstanceGroup(instId));
  auto groups = APPLICATION->instances()->getGroups();
  groups.insert(0, "");
  groups.sort(Qt::CaseInsensitive);
  int foo = groups.indexOf(name);
  name = QInputDialog::getItem(this, tr("Group name"),
                               tr("Enter a new group name."), groups, foo, true,
                               &ok);
  name = name.simplified();
  if (ok) {
    APPLICATION->instances()->setInstanceGroup(instId, name);
  }
}

void MainWindow::on_actionViewInstanceFolder_triggered() {
  DesktopServices::openDirectory(
      APPLICATION->settings()->get("InstanceDir").toString());
}

void MainWindow::on_actionViewCentralModsFolder_triggered() {
  DesktopServices::openDirectory(
      APPLICATION->settings()->get("CentralModsDir").toString());
}

void MainWindow::on_actionConfig_Folder_triggered() {
  if (m_selectedInstance) {
    QString str = m_selectedInstance->instanceConfigFolder();
    DesktopServices::openDirectory(QDir(str).absolutePath());
  }
}

void MainWindow::on_actionSettings_triggered() {
  APPLICATION->ShowGlobalSettings(this, "global-settings");
}

void MainWindow::on_actionInstanceSettings_triggered() {
  APPLICATION->showInstanceWindow(m_selectedInstance, "settings");
}

void MainWindow::on_actionEditInstNotes_triggered() {
  APPLICATION->showInstanceWindow(m_selectedInstance, "notes");
}

void MainWindow::on_actionWorlds_triggered() {
  APPLICATION->showInstanceWindow(m_selectedInstance, "worlds");
}

void MainWindow::on_actionMods_triggered() {
  APPLICATION->showInstanceWindow(m_selectedInstance, "mods");
}

void MainWindow::on_actionEditInstance_triggered() {
  APPLICATION->showInstanceWindow(m_selectedInstance);
}

void MainWindow::on_actionScreenshots_triggered() {
  APPLICATION->showInstanceWindow(m_selectedInstance, "screenshots");
}

void MainWindow::on_actionManageAccounts_triggered() {
  APPLICATION->ShowAccountsDialog(this);
}

void MainWindow::on_actionReportBug_triggered() {
  DesktopServices::openUrl(QUrl(BuildConfig.BUG_TRACKER_URL));
}

void MainWindow::on_actionAbout_triggered() {
  AboutDialog dialog(this);
  dialog.exec();
}

void MainWindow::on_actionDeleteInstance_triggered() {
  if (!m_selectedInstance) {
    return;
  }
  auto id = m_selectedInstance->id();
  auto response = CustomMessageBox::selectable(
                      this, tr("CAREFUL!"),
                      tr("About to delete: %1\nThis is permanent and will "
                         "completely delete the instance.\n\nAre you sure?")
                          .arg(m_selectedInstance->name()),
                      QMessageBox::Warning, QMessageBox::Yes | QMessageBox::No,
                      QMessageBox::No)
                      ->exec();
  if (response == QMessageBox::Yes) {
    APPLICATION->instances()->deleteInstance(id);
  }
}

void MainWindow::on_actionExportInstance_triggered() {
  if (m_selectedInstance) {
    ExportInstanceDialog dlg(m_selectedInstance, this);
    dlg.exec();
  }
}

void MainWindow::on_actionRenameInstance_triggered() {
  if (m_selectedInstance) {
    view->edit(view->currentIndex());
  }
}

void MainWindow::on_actionViewSelectedInstFolder_triggered() {
  if (m_selectedInstance) {
    QString str = m_selectedInstance->instanceRoot();
    DesktopServices::openDirectory(QDir(str).absolutePath());
  }
}

void MainWindow::on_actionViewSelectedMCFolder_triggered() {
  if (m_selectedInstance) {
    QString str = m_selectedInstance->gameRoot();
    if (!FS::ensureFilePathExists(str)) {
      // TODO: report error
      return;
    }
    DesktopServices::openDirectory(QDir(str).absolutePath());
  }
}

void MainWindow::on_actionViewSelectedModsFolder_triggered() {
  if (m_selectedInstance) {
    QString str = m_selectedInstance->modsRoot();
    if (!FS::ensureFilePathExists(str)) {
      // TODO: report error
      return;
    }
    DesktopServices::openDirectory(QDir(str).absolutePath());
  }
}

void MainWindow::on_actionLaunchInstance_triggered() {
  if (!m_selectedInstance) {
    return;
  }
  if (m_selectedInstance->isRunning()) {
    APPLICATION->kill(m_selectedInstance);
  } else {
    APPLICATION->launch(m_selectedInstance);
  }
}

void MainWindow::on_actionCreateShortcut_triggered() {
  if (m_selectedInstance) {
    CreateShortcutDialog(this, m_selectedInstance).exec();
  }
}

void MainWindow::on_actionREDDIT_triggered() {
  DesktopServices::openUrl(QUrl(BuildConfig.SUBREDDIT_URL));
}

