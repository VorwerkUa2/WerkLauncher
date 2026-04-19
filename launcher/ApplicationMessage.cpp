#include "ApplicationMessage.h"

#include <QCborMap>
#include <QCborValue>
#include <QJsonDocument>
#include <QJsonObject>

void ApplicationMessage::parse(const QByteArray &data) {
  QJsonDocument doc;
  if (data.startsWith("\xbf")) { // CBOR map
    doc = QJsonDocument::fromVariant(QCborValue::fromCbor(data).toVariant());
  } else {
    QJsonParseError error;
    doc = QJsonDocument::fromJson(data, &error);
  }
  auto root = doc.object();

  command = root.value("command").toString();
  args.clear();

  auto parsedArgs = root.value("args").toObject();
  for (auto iter = parsedArgs.begin(); iter != parsedArgs.end(); iter++) {
    args[iter.key()] = iter.value().toString();
  }
}

QByteArray ApplicationMessage::serialize() {
  QJsonObject root;
  root.insert("command", command);
  QJsonObject outArgs;
  for (auto iter = args.begin(); iter != args.end(); iter++) {
    outArgs[iter.key()] = iter.value();
  }
  root.insert("args", outArgs);

  QJsonDocument out;
  out.setObject(root);
  return QCborValue::fromVariant(out.toVariant()).toCbor();
}
