namespace {

QString profileInUseFilter(const QString &profileName,

                           const QString &accountName, bool used) {

  QString displayString;

  if (profileName.size() == 0) {

    displayString = QObject::tr("No profile (%1)").arg(accountName);

  } else {

    displayString = profileName;
  }

  if (used) {

    return QObject::tr("%1 (in use)").arg(displayString);

  } else {

    return displayString;
  }
}
}

void MainWindow::repopulateAccountsMenu() {

  accountMenu->clear();

  auto accounts = APPLICATION->accounts();

  MinecraftAccountPtr defaultAccount = accounts->defaultAccount();

  QString active_profileId = "";

  if (defaultAccount) {

    // this can be called before accountMenuButton exists

    if (accountMenuButton) {

      auto profileLabel = profileInUseFilter(defaultAccount->profileName(),

                                             defaultAccount->gamerTag(),

                                             defaultAccount->isInUse());

      accountMenuButton->setToolTip(profileLabel);
    }
  }

  if (accounts->count() <= 0) {

    QAction *action = new QAction(tr("No accounts added!"), this);

    action->setEnabled(false);

    accountMenu->addAction(action);

  } else {

    // TODO: Nicer way to iterate?

    for (int i = 0; i < accounts->count(); i++) {

      auto entry = accounts->at(i);

      if (!entry.isAccount) {

        continue;
      }

      auto account = entry.account;

      auto profileLabel = profileInUseFilter(

          account->profileName(), account->gamerTag(), account->isInUse());

      QAction *action = new QAction(profileLabel, this);

      action->setData(i);

      action->setCheckable(true);

      if (defaultAccount == account) {

        action->setChecked(true);
      }

      auto face = account->getFace();

      if (!face.isNull()) {

        action->setIcon(face);

      } else {

        action->setIcon(APPLICATION->getThemedIcon("noaccount"));
      }

      accountMenu->addAction(action);

      connect(action, &QAction::triggered, this, &MainWindow::changeActiveAccount);
    }
  }

  accountMenu->addSeparator();

  QAction *action = new QAction(tr("No Default Account"), this);

  action->setCheckable(true);

  action->setIcon(APPLICATION->getThemedIcon("noaccount"));

  action->setData(-1);

  if (!defaultAccount) {

    action->setChecked(true);
  }

  accountMenu->addAction(action);

  connect(action, &QAction::triggered, this, &MainWindow::changeActiveAccount);

  accountMenu->addSeparator();

  accountMenu->addAction(ui->actionManageAccounts);
}
void MainWindow::changeActiveAccount() {

  QAction *sAction = qobject_cast<QAction *>(sender());

  // Profile's associated Mojang username

  if (sAction->data().typeId() != QMetaType::Int)

    return;

  QVariant data = sAction->data();

  bool valid = false;

  int index = data.toInt(&valid);

  if (!valid) {

    index = -1;
  }

  auto accounts = APPLICATION->accounts();

  accounts->setDefaultAccount(index == -1 ? nullptr

                                          : accounts->at(index).account);

  defaultAccountChanged();
}
void MainWindow::defaultAccountChanged() {

  repopulateAccountsMenu();

  MinecraftAccountPtr account = APPLICATION->accounts()->defaultAccount();

  if (!account) {

    accountMenuButton->setIcon(APPLICATION->getThemedIcon("accounts"));

    accountMenuButton->setToolTip(tr("Accounts"));

    return;
  }

  ToastNotification::show(this,

                          tr("Account changed: %1").arg(account->gamerTag()),

                          ToastNotification::Success);

  auto profileLabel = profileInUseFilter(

      account->profileName(), account->gamerTag(), account->isInUse());

  accountMenuButton->setToolTip(profileLabel);

  auto face = account->getFace();

  if (face.isNull()) {

    accountMenuButton->setIcon(APPLICATION->getThemedIcon("accounts"));

  } else {

    accountMenuButton->setIcon(face);
  }
}
