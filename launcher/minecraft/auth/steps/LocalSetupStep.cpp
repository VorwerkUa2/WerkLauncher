#include "LocalSetupStep.h"
#include <QRegularExpression>
#include <QUuid>


LocalSetupStep::LocalSetupStep(AccountData *data, const QString &username)
    : AuthStep(data), m_username(username) {}

LocalSetupStep::~LocalSetupStep() noexcept = default;

QString LocalSetupStep::describe() { return tr("Setting up local profile..."); }

void LocalSetupStep::perform() {
  m_data->type = AccountType::Local;
  m_data->accountState = AccountState::Offline;

  // Fill in MSA stub data
  m_data->msaToken.token = "offline";
  m_data->msaToken.refresh_token = "offline";
  m_data->msaToken.validity = Katabasis::Validity::Certain;
  m_data->validity_ = Katabasis::Validity::Certain;

  // Fill in Minecraft Profile stuff
  if (m_data->minecraftProfile.id.isEmpty()) {
    m_data->minecraftProfile.id =
        QUuid::createUuid().toString().remove(QRegularExpression("[{}-]"));
  }
  m_data->minecraftProfile.name = m_username;

  // We own minecraft "locally"
  m_data->minecraftEntitlement.ownsMinecraft = true;
  m_data->minecraftEntitlement.canPlayMinecraft = true;

  emit finished(AccountTaskState::STATE_WORKING, "");
}
