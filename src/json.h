#ifndef IGNJSON_H
#define IGNJSON_H

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QObject>

class ignjson : public QObject {
  Q_OBJECT

public:
  explicit ignjson(QObject *parent = 0);
  QJsonObject jsonParser(const QVariant &config) {
    QJsonParseError *parseError = new QJsonParseError();
    QJsonDocument json = QJsonDocument::fromVariant(config);

    if (parseError->error != 0) {
      qDebug() << parseError->errorString();
      exit(1);
    }

    return json.object();
  }
};

#endif
