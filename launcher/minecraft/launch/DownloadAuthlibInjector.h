#pragma once

#include <launch/LaunchStep.h>
#include <minecraft/auth/AuthSession.h>
#include <net/NetJob.h>

class DownloadAuthlibInjector : public LaunchStep {
  Q_OBJECT
public:
  explicit DownloadAuthlibInjector(LaunchTask *parent, AuthSessionPtr session);
  virtual ~DownloadAuthlibInjector() {}

  void executeTask() override;
  bool canAbort() const override { return true; }
  bool abort() override;

private slots:
  void downloadSucceeded();
  void downloadFailed(QString reason);

private:
  AuthSessionPtr m_session;
  NetJob::Ptr m_job;
};
