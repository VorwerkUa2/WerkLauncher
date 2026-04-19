/*
 * Copyright 2022-2023 arthomnix
 *
 * This source is subject to the Microsoft Public License (MS-PL).
 * Please see the COPYING.md file for more information.
 */

// Windows COM headers must come before Qt headers, because Qt may include
// <windows.h> with WIN32_LEAN_AND_MEAN which strips COM definitions.
// objbase.h must come first to define the 'interface' macro.
// Use _WIN32 (compiler-defined) instead of Q_OS_WIN (Qt-defined, not yet
// available).
#ifdef _WIN32
#include <objbase.h>
#include <objidl.h>
#include <shlguid.h>
#include <shobjidl.h>
#include <windows.h>
#endif

#include "Application.h"
#include "CreateShortcutDialog.h"
#include "icons/IconList.h"
#include "minecraft/MinecraftInstance.h"
#include "minecraft/PackProfile.h"
#include "minecraft/VersionFilterData.h"
#include "minecraft/WorldList.h"
#include "minecraft/auth/AccountList.h"
#include "ui/widgets/CustomTitleBar.h"
#include "ui_CreateShortcutDialog.h"
#include <BuildConfig.h>
#include <QFileDialog>
#include <QMessageBox>

CreateShortcutDialog::CreateShortcutDialog(QWidget *parent,
                                           InstancePtr instance)
    : QDialog(parent), ui(new Ui::CreateShortcutDialog), m_instance(instance) {
  setWindowFlags(Qt::FramelessWindowHint | windowFlags());
  ui->setupUi(this);

  auto titleBar = new CustomTitleBar(this);
  titleBar->setTitle(tr("Create Shortcut"));
  ui->verticalLayout_2->setContentsMargins(0, 0, 0, 0);
  ui->verticalLayout_2->setSpacing(0);
  ui->verticalLayout_2->insertWidget(0, titleBar);

  QStringList accountNameList;
  auto accounts = APPLICATION->accounts();

  for (int i = 0; i < accounts->count(); i++) {
    auto entry = accounts->at(i);
    if (!entry.isAccount) {
      continue;
    }
    accountNameList.append(entry.account->profileName());
  }

  ui->profileComboBox->addItems(accountNameList);

  if (accounts->defaultAccount()) {
    ui->profileComboBox->setCurrentText(
        accounts->defaultAccount()->profileName());
  }

  // TODO: check if version is affected by crashing when joining servers on
  // launch, ideally in meta

  bool instanceSupportsQuickPlay = false;

  auto mcInstance = std::dynamic_pointer_cast<MinecraftInstance>(instance);
  if (mcInstance) {
    mcInstance->getPackProfile()->reload(Net::Mode::Online);

    if (mcInstance->getPackProfile()
            ->getComponent("net.minecraft")
            ->getReleaseDateTime() >= g_VersionFilterData.quickPlayBeginsDate) {
      instanceSupportsQuickPlay = true;
      mcInstance->worldList()->update();
      for (const auto &world : mcInstance->worldList()->allWorlds()) {
        ui->joinSingleplayer->addItem(world.folderName());
      }
    }
  }

  if (!instanceSupportsQuickPlay) {
    ui->joinServerRadioButton->setChecked(true);
    ui->joinSingleplayerRadioButton->setVisible(false);
    ui->joinSingleplayer->setVisible(false);
  }

  // Macs don't have any concept of a desktop shortcut, so force-enable the
  // option to generate a shell script instead
#if defined(Q_OS_UNIX) && !defined(Q_OS_LINUX)
  ui->createScriptCheckBox->setEnabled(false);
  ui->createScriptCheckBox->setChecked(true);
#endif

  connect(ui->joinWorldCheckBox, &QCheckBox::toggled, this,
          &CreateShortcutDialog::updateDialogState);

  updateDialogState();
}

CreateShortcutDialog::~CreateShortcutDialog() { delete ui; }

void CreateShortcutDialog::on_shortcutPathBrowse_clicked() {
  QString linkExtension;
#if defined(Q_OS_UNIX) && !defined(Q_OS_MAC)
  linkExtension = ui->createScriptCheckBox->isChecked() ? "sh" : "desktop";
#endif
#ifdef Q_OS_MAC
  linkExtension = "command";
#endif
#ifdef Q_OS_WIN
  linkExtension = ui->createScriptCheckBox->isChecked() ? "bat" : "lnk";
#endif
  QFileDialog fileDialog(
      this, tr("Select shortcut path"),
      QStandardPaths::writableLocation(QStandardPaths::DesktopLocation));
  fileDialog.setDefaultSuffix(linkExtension);
  fileDialog.setAcceptMode(QFileDialog::AcceptSave);
  fileDialog.setFileMode(QFileDialog::AnyFile);
  fileDialog.selectFile(m_instance->name() + " - " +
                        BuildConfig.LAUNCHER_DISPLAYNAME + "." + linkExtension);
  if (fileDialog.exec()) {
    ui->shortcutPath->setText(fileDialog.selectedFiles().at(0));
  }
  updateDialogState();
}

void CreateShortcutDialog::accept() {
  createShortcut();
  QDialog::accept();
}

void CreateShortcutDialog::updateDialogState() {
  ui->buttonBox->button(QDialogButtonBox::StandardButton::Ok)
      ->setEnabled(!ui->shortcutPath->text().isEmpty() &&
                   (!ui->joinWorldCheckBox->isChecked() ||
                    (ui->joinServerRadioButton->isChecked() &&
                     !ui->joinServer->text().isEmpty()) ||
                    (ui->joinSingleplayerRadioButton->isChecked() &&
                     !ui->joinSingleplayer->currentText().isEmpty())) &&
                   (!ui->offlineUsernameCheckBox->isChecked() ||
                    !ui->offlineUsername->text().isEmpty()) &&
                   (!ui->useProfileCheckBox->isChecked() ||
                    !ui->profileComboBox->currentText().isEmpty()));
  ui->joinServer->setEnabled(ui->joinWorldCheckBox->isChecked() &&
                             ui->joinServerRadioButton->isChecked());
  ui->joinSingleplayer->setEnabled(
      ui->joinWorldCheckBox->isChecked() &&
      ui->joinSingleplayerRadioButton->isChecked());
  ui->offlineUsername->setEnabled(ui->launchOfflineCheckBox->isChecked() &&
                                  ui->offlineUsernameCheckBox->isChecked());
  if (!ui->launchOfflineCheckBox->isChecked()) {
    ui->offlineUsernameCheckBox->setChecked(false);
  }
}

QString CreateShortcutDialog::getLaunchCommand(bool escapeQuotesTwice) {
  return "\"" +
         QDir::toNativeSeparators(QCoreApplication::applicationFilePath())
             .replace('"', escapeQuotesTwice ? "\\\\\"" : "\\\"") +
         "\"" + getLaunchArgs(escapeQuotesTwice);
}

QString CreateShortcutDialog::getLaunchArgs(bool escapeQuotesTwice) {
  return " -d \"" +
         QDir::toNativeSeparators(QDir::currentPath())
             .replace('"', escapeQuotesTwice ? "\\\\\"" : "\\\"") +
         "\"" + " -l \"" + m_instance->id() + "\"" +
         (ui->joinServerRadioButton->isChecked()
              ? " -s \"" + ui->joinServer->text() + "\""
              : "") +
         (ui->joinSingleplayerRadioButton->isChecked()
              ? " -w \"" + ui->joinSingleplayer->currentText() + "\""
              : "") +
         (ui->useProfileCheckBox->isChecked()
              ? " -a \"" + ui->profileComboBox->currentText() + "\""
              : "") +
         (ui->launchOfflineCheckBox->isChecked() ? " -o" : "") +
         (ui->offlineUsernameCheckBox->isChecked()
              ? " -n \"" + ui->offlineUsername->text() + "\""
              : "");
}

void CreateShortcutDialog::createShortcut() {
#ifdef Q_OS_WIN
  if (ui->createScriptCheckBox
          ->isChecked()) // on windows, creating .lnk shortcuts requires
                         // specific win32 api stuff rather than just writing a
                         // text file
  {
#endif
    QString shortcutText;
#ifdef Q_OS_UNIX
    // Unix shell script
    if (ui->createScriptCheckBox->isChecked()) {
      shortcutText = "#!/bin/sh\n"
                     // FIXME: is there a way to use the launcher script instead
                     // of the raw binary here?
                     "cd \"" +
                     QDir::currentPath().replace('"', "\\\"") + "\"\n" +
                     getLaunchCommand() + " &\n";
    } else
    // freedesktop.org desktop entry
    {
      // save the launcher icon to a file so we can use it in the shortcut
      if (!QFileInfo::exists(QDir::currentPath() +
                             "/icons/shortcut-icon.png")) {
        QPixmap iconPixmap = QIcon(":/logo.svg").pixmap(64, 64);
        iconPixmap.save(QDir::currentPath() + "/icons/shortcut-icon.png");
      }

      shortcutText =
          "[Desktop Entry]\n"
          "Type=Application\n"
          "Name=" +
          m_instance->name() + " - " + BuildConfig.LAUNCHER_DISPLAYNAME + "\n" +
          "Exec=" + getLaunchCommand(true) + "\n" +
          "Path=" + QDir::currentPath() + "\n" + "Icon=" + QDir::currentPath() +
          "/icons/shortcut-icon.png\n";
    }
#endif
#ifdef Q_OS_WIN
    // Windows batch script implementation
    shortcutText = "@ECHO OFF\r\n"
                   "CD \"" +
                   QDir::toNativeSeparators(QDir::currentPath()) +
                   "\"\r\n"
                   "START /B \"\" " +
                   getLaunchCommand() + "\r\n";
#endif
    QFile shortcutFile(ui->shortcutPath->text());
    if (shortcutFile.open(QIODevice::WriteOnly)) {
      QTextStream stream(&shortcutFile);
      stream << shortcutText;
      shortcutFile.setPermissions(QFile::ReadOwner | QFile::ReadGroup |
                                  QFile::ReadOther | QFile::WriteOwner |
                                  QFile::ExeOwner | QFile::ExeGroup);
      shortcutFile.close();
    }
#ifdef Q_OS_WIN
  } else {
    if (!QFileInfo::exists(QDir::currentPath() + "/icons/shortcut-icon.ico")) {
      QPixmap iconPixmap = QIcon(":/logo.svg").pixmap(64, 64);
      iconPixmap.save(QDir::currentPath() + "/icons/shortcut-icon.ico");
    }

    createWindowsLink(
        QDir::toNativeSeparators(QCoreApplication::applicationFilePath())
            .toStdWString()
            .c_str(),
        QDir::toNativeSeparators(QDir::currentPath()).toStdWString().c_str(),
        getLaunchArgs().toStdWString().c_str(),
        ui->shortcutPath->text().toStdWString().c_str(),
        (m_instance->name() + " - " + BuildConfig.LAUNCHER_DISPLAYNAME)
            .toStdWString()
            .c_str(),
        QDir::toNativeSeparators(QDir::currentPath() +
                                 "/icons/shortcut-icon.ico")
            .toStdWString()
            .c_str());
  }
#endif
}

#ifdef Q_OS_WIN
void CreateShortcutDialog::createWindowsLink(LPCWSTR target, LPCWSTR workingDir,
                                             LPCWSTR args, LPCWSTR filename,
                                             LPCWSTR desc, LPCWSTR iconPath) {
  HRESULT result;
  IShellLink *link;

  CoInitialize(nullptr);
  result = CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER,
                            IID_IShellLink, (LPVOID *)&link);
  if (SUCCEEDED(result)) {
    IPersistFile *file;

    link->SetPath(target);
    link->SetWorkingDirectory(workingDir);
    link->SetArguments(args);
    link->SetDescription(desc);
    link->SetIconLocation(iconPath, 0);

    result = link->QueryInterface(IID_IPersistFile, (LPVOID *)&file);

    if (SUCCEEDED(result)) {
      file->Save(filename, TRUE);
      file->Release();
    }
    link->Release();
  }
  CoUninitialize();
}
#endif
