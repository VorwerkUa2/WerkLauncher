#include "InstanceSettingsPage.h"
#include "ui_InstanceSettingsPage.h"

#include <QFileDialog>
#include <QDialog>
#include <QMessageBox>

#include <sys.h>

#include "ui/dialogs/VersionSelectDialog.h"
#include "ui/widgets/CustomCommands.h"

#include "JavaCommon.h"
#include "Application.h"

#include "java/JavaInstallList.h"
#include "FileSystem.h"

#include "minecraft/MinecraftInstance.h"
#include "minecraft/PackProfile.h"
#include "minecraft/VersionFilterData.h"
#include "minecraft/WorldList.h"

InstanceSettingsPage::InstanceSettingsPage(BaseInstance *inst, QWidget *parent)
    : QWidget(parent), ui(new Ui::InstanceSettingsPage), m_instance(inst)
{
    m_settings = inst->settings();
    ui->setupUi(this);
    connect(ui->openGlobalJavaSettingsButton, &QCommandLinkButton::clicked, this, &InstanceSettingsPage::globalSettingsButtonClicked);
    connect(APPLICATION, &Application::globalSettingsAboutToOpen, this, &InstanceSettingsPage::applySettings);
    connect(APPLICATION, &Application::globalSettingsClosed, this, &InstanceSettingsPage::loadSettings);

    bool supportsQuickPlay = false;
    auto *mcInst = dynamic_cast<MinecraftInstance *>(inst);
    if (mcInst)
    {
        auto minecraftComponent = mcInst->getPackProfile()->getComponent("net.minecraft");
        if(minecraftComponent && minecraftComponent->getReleaseDateTime() >= g_VersionFilterData.quickPlayBeginsDate)
        {
            supportsQuickPlay = true;
        }
    }

    if(supportsQuickPlay)
    {
        mcInst->worldList()->update();
        for (const auto &world : mcInst->worldList()->allWorlds())
        {
            ui->worldsComboBox->addItem(world.folderName());
        }
    }
    else
    {
        ui->worldRadioButton->setVisible(false);
        ui->worldsComboBox->setVisible(false);
        ui->serverAddressRadioButton->setChecked(true);
        connect(ui->quickPlayGroupBox, &QGroupBox::toggled, ui->serverJoinAddress, &QLineEdit::setEnabled);
    }

    loadSettings();
}

bool InstanceSettingsPage::shouldDisplay() const
{
    return !m_instance->isRunning();
}

InstanceSettingsPage::~InstanceSettingsPage()
{
    delete ui;
}

void InstanceSettingsPage::globalSettingsButtonClicked(bool)
{
    switch(ui->settingsTabs->currentIndex()) {
        case 0:
            APPLICATION->ShowGlobalSettings(this, "minecraft-settings");
            return;
        case 1:
            APPLICATION->ShowGlobalSettings(this, "custom-commands");
            return;
    }
}

bool InstanceSettingsPage::apply()
{
    applySettings();
    return true;
}

void InstanceSettingsPage::applySettings()
{
    SettingsObject::Lock lock(m_settings);

    // Console
    bool console = ui->consoleSettingsBox->isChecked();
    m_settings->set("OverrideConsole", console);
    if (console)
    {
        m_settings->set("ShowConsole", ui->showConsoleCheck->isChecked());
        m_settings->set("AutoCloseConsole", ui->autoCloseConsoleCheck->isChecked());
        m_settings->set("ShowConsoleOnError", ui->showConsoleErrorCheck->isChecked());
    }
    else
    {
        m_settings->reset("ShowConsole");
        m_settings->reset("AutoCloseConsole");
        m_settings->reset("ShowConsoleOnError");
    }

    // Window Size
    bool window = ui->windowSizeGroupBox->isChecked();
    m_settings->set("OverrideWindow", window);
    if (window)
    {
        m_settings->set("LaunchMaximized", ui->maximizedCheckBox->isChecked());
        m_settings->set("MinecraftWinWidth", ui->windowWidthSpinBox->value());
        m_settings->set("MinecraftWinHeight", ui->windowHeightSpinBox->value());
    }
    else
    {
        m_settings->reset("LaunchMaximized");
        m_settings->reset("MinecraftWinWidth");
        m_settings->reset("MinecraftWinHeight");
    }

    // Custom Commands
    bool custcmd = ui->customCommands->checked();
    m_settings->set("OverrideCommands", custcmd);
    if (custcmd)
    {
        m_settings->set("PreLaunchCommand", ui->customCommands->prelaunchCommand());
        m_settings->set("WrapperCommand", ui->customCommands->wrapperCommand());
        m_settings->set("PostExitCommand", ui->customCommands->postexitCommand());
    }
    else
    {
        m_settings->reset("PreLaunchCommand");
        m_settings->reset("WrapperCommand");
        m_settings->reset("PostExitCommand");
    }

    // Workarounds
    bool workarounds = ui->nativeWorkaroundsGroupBox->isChecked();
    m_settings->set("OverrideNativeWorkarounds", workarounds);
    if(workarounds)
    {
        m_settings->set("UseNativeOpenAL", ui->useNativeOpenALCheck->isChecked());
        m_settings->set("UseNativeGLFW", ui->useNativeGLFWCheck->isChecked());
    }
    else
    {
        m_settings->reset("UseNativeOpenAL");
        m_settings->reset("UseNativeGLFW");
    }

    // Game time
    bool gameTime = ui->gameTimeGroupBox->isChecked();
    m_settings->set("OverrideGameTime", gameTime);
    if (gameTime)
    {
        m_settings->set("ShowGameTime", ui->showGameTime->isChecked());
        m_settings->set("RecordGameTime", ui->recordGameTime->isChecked());
    }
    else
    {
        m_settings->reset("ShowGameTime");
        m_settings->reset("RecordGameTime");
    }

    // Join server on launch
    bool joinWorldOnLaunch = ui->quickPlayGroupBox->isChecked();
    m_settings->set("JoinWorldOnLaunch", joinWorldOnLaunch);

    bool joinServerOnLaunch = ui->serverAddressRadioButton->isChecked();
    m_settings->set("JoinServerOnLaunch", joinServerOnLaunch);

    if (joinServerOnLaunch)
    {
        m_settings->set("JoinServerOnLaunchAddress", ui->serverJoinAddress->text());
    }
    else
    {
        m_settings->reset("JoinServerOnLaunchAddress");
    }

    bool joinSingleplayerWorldOnLaunch = ui->worldRadioButton->isChecked();
    m_settings->set("JoinSingleplayerWorldOnLaunch", joinSingleplayerWorldOnLaunch);

    if (joinSingleplayerWorldOnLaunch)
    {
        m_settings->set("JoinSingleplayerWorldOnLaunchName", ui->worldsComboBox->currentText());
    }
    else
    {
        m_settings->reset("JoinSingleplayerWorldOnLaunchName");
    }
}

void InstanceSettingsPage::loadSettings()
{
    // Console
    ui->consoleSettingsBox->setChecked(m_settings->get("OverrideConsole").toBool());
    ui->showConsoleCheck->setChecked(m_settings->get("ShowConsole").toBool());
    ui->autoCloseConsoleCheck->setChecked(m_settings->get("AutoCloseConsole").toBool());
    ui->showConsoleErrorCheck->setChecked(m_settings->get("ShowConsoleOnError").toBool());

    // Window Size
    ui->windowSizeGroupBox->setChecked(m_settings->get("OverrideWindow").toBool());
    ui->maximizedCheckBox->setChecked(m_settings->get("LaunchMaximized").toBool());
    ui->windowWidthSpinBox->setValue(m_settings->get("MinecraftWinWidth").toInt());
    ui->windowHeightSpinBox->setValue(m_settings->get("MinecraftWinHeight").toInt());

    // Custom commands
    ui->customCommands->initialize(
        true,
        m_settings->get("OverrideCommands").toBool(),
        m_settings->get("PreLaunchCommand").toString(),
        m_settings->get("WrapperCommand").toString(),
        m_settings->get("PostExitCommand").toString()
    );

    // Workarounds
    ui->nativeWorkaroundsGroupBox->setChecked(m_settings->get("OverrideNativeWorkarounds").toBool());
    ui->useNativeGLFWCheck->setChecked(m_settings->get("UseNativeGLFW").toBool());
    ui->useNativeOpenALCheck->setChecked(m_settings->get("UseNativeOpenAL").toBool());

    // Miscellanous
    ui->gameTimeGroupBox->setChecked(m_settings->get("OverrideGameTime").toBool());
    ui->showGameTime->setChecked(m_settings->get("ShowGameTime").toBool());
    ui->recordGameTime->setChecked(m_settings->get("RecordGameTime").toBool());

    if (!m_settings->contains("JoinWorldOnLaunch"))
    {
        ui->quickPlayGroupBox->setChecked(m_settings->get("JoinServerOnLaunch").toBool());
        ui->serverAddressRadioButton->setChecked(m_settings->get("JoinServerOnLaunch").toBool());
        ui->worldRadioButton->setChecked(false);
    }
    else
    {
        ui->quickPlayGroupBox->setChecked(m_settings->get("JoinWorldOnLaunch").toBool());
        ui->serverAddressRadioButton->setChecked(m_settings->get("JoinServerOnLaunch").toBool());
        ui->serverJoinAddress->setEnabled(m_settings->get("JoinServerOnLaunch").toBool());
        ui->serverJoinAddress->setText(m_settings->get("JoinServerOnLaunchAddress").toString());
        ui->worldRadioButton->setChecked(m_settings->get("JoinSingleplayerWorldOnLaunch").toBool());
        ui->worldsComboBox->setEnabled(m_settings->get("JoinSingleplayerWorldOnLaunch").toBool());
        ui->worldsComboBox->setCurrentText(m_settings->get("JoinSingleplayerWorldOnLaunchName").toString());
    }

}

void InstanceSettingsPage::on_serverAddressRadioButton_toggled(bool checked)
{
    ui->serverJoinAddress->setEnabled(checked);
}

void InstanceSettingsPage::on_worldRadioButton_toggled(bool checked)
{
    ui->worldsComboBox->setEnabled(checked);
}

