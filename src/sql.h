#ifndef SQL_H
#define SQL_H

#include "json.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QObject>
#include <QtSql>

class ignsql : public QObject {
  Q_OBJECT

public:
  explicit ignsql(QObject *parent = 0);
  QSqlDatabase db;
  ignjson *jsonParse;

signals:

public slots:
  bool driver(const QString &driver, QString connection);
  bool driver(const QVariant &config);
  QVariant query(const QString &query);
};

#endif // SQL_H
