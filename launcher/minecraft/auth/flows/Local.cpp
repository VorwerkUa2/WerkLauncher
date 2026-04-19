#include "Local.h"
#include "minecraft/auth/steps/LocalSetupStep.h"
#include "minecraft/auth/steps/YggdrasilStep.h"

LocalLogin::LocalLogin(AccountData *data, const QString &username,
                       QObject *parent)
    : AuthFlow(data, parent) {
  m_steps.append(new LocalSetupStep(m_data, username));
  if (!m_data->syncUrl.isEmpty()) {
    m_steps.append(new YggdrasilLoginStep(m_data));
  }
}

LocalRefresh::LocalRefresh(AccountData *data, QObject *parent)
    : AuthFlow(data, parent) {
  m_steps.append(new LocalSetupStep(m_data, m_data->profileName()));
  if (!m_data->syncUrl.isEmpty()) {
    m_steps.append(new YggdrasilLoginStep(m_data));
  }
}
