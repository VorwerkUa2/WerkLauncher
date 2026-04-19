#pragma once
#include "tasks/Task.h"
#include <QBuffer>
#include <QNetworkReply>
#include <memory>

class PasteUpload : public Task {
  Q_OBJECT
public:
  PasteUpload(QWidget *window, QString text, QString key = "public");
  virtual ~PasteUpload();

  QString pasteLink() { return m_pasteLink; }
  QString pasteID() { return m_pasteID; }
  int maxSize() {
    // 2MB for paste.ee - public
    if (m_key == "public")
      return 1024 * 1024 * 2;
    // 12MB for paste.ee - with actual key
    return 1024 * 1024 * 12;
  }
  bool validateText();
  bool abort() override;

protected:
  virtual void executeTask();

private:
  bool parseResult(QJsonDocument doc);
  QString m_error;
  QWidget *m_window;
  QString m_pasteID;
  QString m_pasteLink;
  QString m_key;
  QByteArray m_jsonContent;
  QNetworkReply *m_reply = nullptr;
public slots:
  void downloadError(QNetworkReply::NetworkError);
  void downloadFinished();
};
