#pragma once

#include "minecraft/auth/AuthStep.h"

class YggdrasilProfileStep : public AuthStep {
  Q_OBJECT
public:
  explicit YggdrasilProfileStep(AccountData *data);
  virtual ~YggdrasilProfileStep() noexcept;

  void perform() override;
  QString describe() override;

private slots:
  void onRequestDone(QNetworkReply::NetworkError error, QByteArray data,
                     QList<QNetworkReply::RawHeaderPair> headers);
};
