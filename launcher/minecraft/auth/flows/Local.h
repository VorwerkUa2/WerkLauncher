#include "AuthFlow.h"

class LocalLogin : public AuthFlow {
  Q_OBJECT
public:
  explicit LocalLogin(AccountData *data, const QString &username,
                      QObject *parent = nullptr);
  virtual ~LocalLogin() = default;
};

class LocalRefresh : public AuthFlow {
  Q_OBJECT
public:
  explicit LocalRefresh(AccountData *data, QObject *parent = nullptr);
  virtual ~LocalRefresh() = default;
};
