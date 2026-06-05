#include "LaunchController.h"
#include "Application.h"
#include "BuildConfig.h"
#include "discord/DiscordRPC.h"
#include "minecraft/auth/AccountList.h"

#include "ui/InstanceWindow.h"
#include "ui/MainWindow.h"
#include "ui/dialogs/CustomMessageBox.h"
#include "ui/dialogs/OfflineNameDialog.h"
#include "ui/dialogs/ProfileSelectDialog.h"
#include "ui/dialogs/ProgressDialog.h"

#include <QHostAddress>
#include <QHostInfo>
#include <QInputDialog>
#include <QLineEdit>
#include <QList>
#include <QPushButton>
#include <QStringList>

#include "BuildConfig.h"
#include "JavaCommon.h"
#include "launch/steps/TextPrint.h"
#include "minecraft/auth/AccountTask.h"
#include "tasks/Task.h"
#include "ui/dialogs/AccountsDialog.h"

LaunchController::LaunchController(QObject *parent) : Task(parent) {}

void LaunchController::executeTask() {
  if (!m_instance) {
    emitFailed(tr("No instance specified!"));
    return;
  }

  if (!JavaCommon::checkJVMArgs(
          m_instance->settings()->get("JvmArgs").toString(), m_parentWidget)) {
    emitFailed(tr("Invalid Java arguments specified. Please fix this first."));
    return;
  }

  login();
}

void LaunchController::decideAccount() {
  if (m_accountToUse) {
    return;
  }

  // Find an account to use.
  auto accounts = APPLICATION->accounts();
  if (!accounts->anyAccount()) {
    // Tell the user they need to log in at least one account in order to play.
    auto reply =
        CustomMessageBox::selectable(m_parentWidget, tr("No Accounts"),
                                     tr("In order to play Minecraft, you must "
                                        "have at least one Mojang or Minecraft "
                                        "account logged in."
                                        "Would you like to open the account "
                                        "manager to add an account now?"),
                                     QMessageBox::Information,
                                     QMessageBox::Yes | QMessageBox::No)
            ->exec();

    if (reply == QMessageBox::Yes) {
      // Open the account manager.
      APPLICATION->ShowAccountsDialog(m_parentWidget);
    }
  }

  // Check again, as the user might have added an account now.
  if (!accounts->anyAccount()) {
    return;
  }

  m_accountToUse = accounts->defaultAccount();
  if (!m_accountToUse) {
    // If no default account is set, ask the user which one to use.
    ProfileSelectDialog selectDialog(tr("Which account would you like to use?"),
                                     ProfileSelectDialog::GlobalDefaultCheckbox,
                                     m_parentWidget);

    if (selectDialog.exec() == QDialog::Accepted) {
      // Launch the instance with the selected account.
      m_accountToUse = selectDialog.selectedAccount();

      // If the user said to use the account as default, do that.
      if (selectDialog.useAsGlobalDefault() && m_accountToUse) {
        accounts->setDefaultAccount(m_accountToUse);
      }
    }
  }
}

void LaunchController::login() {
  decideAccount();

  // if no account is selected, we bail
  if (!m_accountToUse) {
    emitFailed(tr("No account selected for launch."));
    return;
  }

  // we loop until the user succeeds in logging in or gives up
  bool tryagain = true;

  while (tryagain) {
    m_session = std::make_shared<AuthSession>();
    m_session->wants_online = m_userWantsOnline;
    m_accountToUse->fillSession(m_session);
    switch (m_accountToUse->accountState()) {
    case AccountState::Offline: {
      m_session->wants_online = false;
      // NOTE: fallthrough is intentional
    }
    case AccountState::Online: {
      if (!m_session->wants_online) {
        QString usedname;
        if (m_offlineName.isEmpty()) {
          // we ask the user for a player name
          QString lastOfflinePlayerName =
              APPLICATION->settings()->get("LastOfflinePlayerName").toString();
          usedname = lastOfflinePlayerName.isEmpty() ? m_session->player_name
                                                     : lastOfflinePlayerName;
          OfflineNameDialog dialog(usedname, m_parentWidget);
          int result = dialog.exec();
          if (result == OfflineNameDialog::Accepted) {
            usedname = dialog.textValue();
            APPLICATION->settings()->set("LastOfflinePlayerName", usedname);
          } else if (result == OfflineNameDialog::Rejected) {
            tryagain = false;
            break;
          } else if (result == OfflineNameDialog::OnlineRequested) {
            m_userWantsOnline = true;
            if (m_accountToUse->accountState() == AccountState::Offline) {
              auto task = m_accountToUse->refresh();
              if (task) {
                task->start();
              }
            }
            break;
          }
        } else {
          usedname = m_offlineName;
        }

        m_session->MakeOffline(usedname);
        // offline flavored game from here :3
      }
      if (m_accountToUse->ownsMinecraft()) {
        if (!m_accountToUse->hasProfile()) {
          QMessageBox box(m_parentWidget);
          box.setWindowTitle(tr("Account doesn't have a Minecraft profile"));
          box.setText(tr("The account doesn't have a Minecraft profile "
                         "yet.\nYou need to create a profile first to "
                         "play.\n\nDo you want to open the Accounts dialog?"));
          box.setIcon(QMessageBox::Warning);
          auto accountsButton = box.addButton(tr("Open Accounts"),
                                              QMessageBox::ButtonRole::YesRole);
          box.addButton(tr("Cancel"), QMessageBox::ButtonRole::NoRole);
          box.setDefaultButton(accountsButton);

          box.exec();
          if (box.clickedButton() == accountsButton) {
            APPLICATION->ShowAccountsDialog(m_parentWidget);
            emitFailed(tr("Please create a profile in the Accounts dialog and try launching again."));
            return;
          } else {
            emitFailed(
                tr("Launch cancelled - account does not own Minecraft."));
            return;
          }
        }
        // we own Minecraft, there is a profile, it's all ready to go!
        launchInstance();
        return;
      } else {
        // play demo ?
        QMessageBox box(m_parentWidget);
        box.setWindowTitle(tr("Play demo?"));
        box.setText(
            tr("This account does not own Minecraft.\nYou need to purchase the "
               "game first to play it.\n\nDo you want to play the demo?"));
        box.setIcon(QMessageBox::Warning);
        auto demoButton =
            box.addButton(tr("Play Demo"), QMessageBox::ButtonRole::YesRole);
        auto cancelButton =
            box.addButton(tr("Cancel"), QMessageBox::ButtonRole::NoRole);
        box.setDefaultButton(cancelButton);

        box.exec();
        if (box.clickedButton() == demoButton) {
          // play demo here
          m_session->MakeDemo();
          launchInstance();
        } else {
          emitFailed(tr("Launch cancelled - account does not own Minecraft."));
        }
      }
      return;
    }
    case AccountState::Errored:
      // This means some sort of soft error that we can fix with a refresh ...
      // so let's refresh.
    case AccountState::Unchecked: {
      m_accountToUse->refresh();
      // NOTE: fallthrough intentional
    }
    case AccountState::Working: {
      // refresh is in progress, we need to wait for it to finish to proceed.
      ProgressDialog progDialog(m_parentWidget);
      auto task = m_accountToUse->currentTask();
      progDialog.execWithTask(task.get());
      continue;
    }
    // FIXME: this is missing - the meaning is that the account is queued for
    // refresh and we should wait for that
    /*
    case AccountState::Queued: {
        return;
    }
    */
    case AccountState::Expired: {
      auto errorString =
          tr("The account has expired and needs to be logged into manually. "
             "Press OK to open the accounts window.");
      auto button = QMessageBox::warning(
          m_parentWidget, tr("Account refresh failed"), errorString,
          QMessageBox::StandardButton::Ok | QMessageBox::StandardButton::Cancel,
          QMessageBox::StandardButton::Ok);
      if (button == QMessageBox::StandardButton::Ok) {
        auto accounts = APPLICATION->accounts();
        accounts->removeAccount(m_accountToUse->internalId());
        APPLICATION->ShowAccountsDialog(m_parentWidget);
        emitFailed(tr("The account has expired. Please log in again."));
        return;
      } else {
        emitFailed(errorString);
        return;
      }
    }
    case AccountState::Gone: {
      auto errorString = tr("The account no longer exists on the servers. It "
                            "may have been migrated, in which case please add "
                            "the new account you migrated this one to.");
      QMessageBox::warning(m_parentWidget, tr("Account gone"), errorString,
                           QMessageBox::StandardButton::Ok,
                           QMessageBox::StandardButton::Ok);
      emitFailed(errorString);
      return;
    }
    case AccountState::MustMigrate: {
      auto errorString =
          tr("The account must be migrated to a Microsoft account.");
      QMessageBox::warning(m_parentWidget, tr("Account requires migration"),
                           errorString, QMessageBox::StandardButton::Ok,
                           QMessageBox::StandardButton::Ok);
      emitFailed(errorString);
      return;
    }
    }
  }
  emitFailed(tr("Failed to launch."));
}

void LaunchController::launchInstance() {
  Q_ASSERT_X(m_instance != NULL, "launchInstance", "instance is NULL");
  if (!m_session) {
    m_session = std::make_shared<AuthSession>();
    m_accountToUse->fillSession(m_session);
  }
  Q_ASSERT_X(m_session.get() != nullptr, "launchInstance", "session is NULL");

  if (!m_instance->reloadSettings()) {
    QMessageBox::critical(m_parentWidget, tr("Error!"),
                          tr("Couldn't load the instance profile."));
    emitFailed(tr("Couldn't load the instance profile."));
    return;
  }

  m_launcher = m_instance->createLaunchTask(m_session, m_quickPlayTarget);
  if (!m_launcher) {
    emitFailed(tr("Couldn't instantiate a launcher."));
    return;
  }

  auto console = qobject_cast<InstanceWindow *>(m_parentWidget);
  auto showConsole = m_instance->settings()->get("ShowConsole").toBool();
  if (!console && showConsole) {
    APPLICATION->showInstanceWindow(m_instance);
  }
  connect(m_launcher.get(), &LaunchTask::readyForLaunch, this,
          &LaunchController::readyForLaunch);
  connect(m_launcher.get(), &LaunchTask::succeeded, this,
          &LaunchController::onSucceeded);
  connect(m_launcher.get(), &LaunchTask::failed, this,
          &LaunchController::onFailed);
  connect(m_launcher.get(), &LaunchTask::requestProgress, this,
          &LaunchController::onProgressRequested);

  // Prepend Online and Auth Status
  QString online_mode;

  auto finishLaunch = [this, online_mode](const QString &resolved_servers) {
    if (!resolved_servers.isEmpty()) {
      m_launcher->prependStep(new TextPrint(m_launcher.get(), resolved_servers,
                                            MessageLevel::Launcher));
    }
    m_launcher->prependStep(new TextPrint(
        m_launcher.get(), "Launched instance in " + online_mode + " mode\n",
        MessageLevel::Launcher));
    m_launcher->prependStep(new TextPrint(
        m_launcher.get(),
        BuildConfig.LAUNCHER_NAME +
            " version: " + BuildConfig.printableVersionString() + "\n\n",
        MessageLevel::Launcher));
    m_launcher->start();

    if (APPLICATION->discordRPC()) {
      APPLICATION->discordRPC()->setActivity("Playing Minecraft",
                                             m_instance->name());
    }
  };

  if (m_session->wants_online) {
    online_mode = "online";
    QStringList servers = {QUrl(BuildConfig.SESSION_BASE).host(),
                           QUrl(BuildConfig.TEXTURE_BASE).host(),
                           QUrl(BuildConfig.API_BASE).host()};

    struct DnsState {
      QString resolved_servers;
      int remaining;
    };
    auto state = std::make_shared<DnsState>();
    state->remaining = servers.size();

    for (const QString &server : servers) {
      QHostInfo::lookupHost(
          server, this,
          [state, server, finishLaunch](const QHostInfo &host_info) {
            state->resolved_servers += server + " resolves to:\n    [";
            if (!host_info.addresses().isEmpty()) {
              auto addrs = host_info.addresses();
              for (int i = 0; i < addrs.size(); i++) {
                state->resolved_servers += addrs[i].toString();
                if (i < addrs.size() - 1) {
                  state->resolved_servers += ", ";
                }
              }
            } else {
              state->resolved_servers += "N/A";
            }
            state->resolved_servers += "]\n\n";

            state->remaining--;
            if (state->remaining == 0) {
              finishLaunch(state->resolved_servers);
            }
          });
    }
  } else {
    online_mode = "offline";
    finishLaunch("");
  }
}

void LaunchController::readyForLaunch() {
  if (!m_profiler) {
    m_launcher->proceed();
    return;
  }

  QString error;
  if (!m_profiler->check(&error)) {
    m_launcher->abort();
    QMessageBox::critical(m_parentWidget, tr("Error!"),
                          tr("Couldn't start profiler: %1").arg(error));
    emitFailed("Profiler startup failed!");
    return;
  }
  BaseProfiler *profilerInstance =
      m_profiler->createProfiler(m_launcher->instance(), this);

  connect(
      profilerInstance, &BaseProfiler::readyToLaunch,
      [this](const QString &message) {
        QMessageBox msg;
        msg.setText(
            tr("The game launch is delayed until you press the "
               "button. This is the right time to setup the profiler, as the "
               "profiler server is running now.\n\n%1")
                .arg(message));
        msg.setWindowTitle(tr("Waiting."));
        msg.setIcon(QMessageBox::Information);
        msg.addButton(tr("Launch"), QMessageBox::AcceptRole);
        msg.setModal(true);
        msg.exec();
        m_launcher->proceed();
      });
  connect(profilerInstance, &BaseProfiler::abortLaunch,
          [this](const QString &message) {
            QMessageBox msg;
            msg.setText(tr("Couldn't start the profiler: %1").arg(message));
            msg.setWindowTitle(tr("Error"));
            msg.setIcon(QMessageBox::Critical);
            msg.addButton(QMessageBox::Ok);
            msg.setModal(true);
            msg.exec();
            m_launcher->abort();
            emitFailed("Profiler startup failed!");
          });
  profilerInstance->beginProfiling(m_launcher);
}

void LaunchController::onSucceeded() {
  // Reset Discord Rich Presence
  if (APPLICATION->discordRPC()) {
    APPLICATION->discordRPC()->setActivity("In the launcher",
                                           "Browsing instances", false);
  }
  emitSucceeded();
}

void LaunchController::onFailed(QString reason) {
  // Reset Discord Rich Presence
  if (APPLICATION->discordRPC()) {
    APPLICATION->discordRPC()->setActivity("In the launcher",
                                           "Browsing instances", false);
  }
  if (m_instance->settings()->get("ShowConsoleOnError").toBool()) {
    APPLICATION->showInstanceWindow(m_instance, "console");
  }
  emitFailed(reason);
}

void LaunchController::onProgressRequested(Task *task) {
  ProgressDialog progDialog(m_parentWidget);
  progDialog.setSkipButton(true, tr("Abort"));
  m_launcher->proceed();
  progDialog.execWithTask(task);
}

bool LaunchController::abort() {
  if (!m_launcher) {
    return true;
  }
  if (!m_launcher->canAbort()) {
    return false;
  }
  auto response = CustomMessageBox::selectable(
                      m_parentWidget, tr("Kill Minecraft?"),
                      tr("This can cause the instance to get corrupted and "
                         "should only be used if Minecraft "
                         "is frozen for some reason"),
                      QMessageBox::Question, QMessageBox::Yes | QMessageBox::No,
                      QMessageBox::Yes)
                      ->exec();
  if (response == QMessageBox::Yes) {
    return m_launcher->abort();
  }
  return false;
}
