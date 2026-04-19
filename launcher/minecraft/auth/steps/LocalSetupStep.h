#pragma once
#include "minecraft/auth/AuthStep.h"

class LocalSetupStep : public AuthStep {
  Q_OBJECT
public:
  explicit LocalSetupStep(AccountData *data, const QString &username);
  virtual ~LocalSetupStep() noexcept;

  void perform() override;
  QString describe() override;

private:
  QString m_username;
};
