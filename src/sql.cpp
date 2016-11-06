#include "sql.h"

ignsql::ignsql(QObject *parent) : QObject(parent), jsonParse(0) {}

bool ignsql::driver(const QString &driver, QString connection) {
  if (driver == "mysql") {
    QStringList details = connection.split(":");
    this->db = QSqlDatabase::addDatabase("QMYSQL");
    this->db.setHostName(details.value(0));
    this->db.setUserName(details.value(1));
    this->db.setPassword(details.value(2));
    this->db.setDatabaseName(details.value(3));
    return this->db.open();
  } else if (driver == "sqlite2") {
    this->db = QSqlDatabase::addDatabase("QSQLITE2");
    this->db.setDatabaseName(connection);
    return this->db.open();
  } else if (driver == "sqlite") {
    this->db = QSqlDatabase::addDatabase("QSQLITE");
    this->db.setDatabaseName(connection);
    return this->db.open();
  } else {
    qDebug() << "Error: Invalid database driver specified. Available drivers: "
                "mysql, sqlite, sqlite2.";
    return false;
  }
}

bool ignsql::driver(const QVariant &config) {
  QVariantMap configuration = jsonParse->jsonParser(config).toVariantMap();
  QString driver = configuration["driver"].toString();
  QString hostname = configuration["hostname"].toString();
  QString username = configuration["username"].toString();
  QString password = configuration["password"].toString();
  QString database = configuration["db"].toString();
  database = configuration["database"].toString();

  if (driver == "mysql") {
    this->db = QSqlDatabase::addDatabase("QMYSQL");
    this->db.setHostName(hostname);
    this->db.setUserName(username);
    this->db.setPassword(password);
    this->db.setDatabaseName(database);
    return this->db.open();
  } else if (driver == "sqlite2") {
    this->db = QSqlDatabase::addDatabase("QSQLITE2");
    this->db.setDatabaseName(database);
    return this->db.open();
  } else if (driver == "sqlite") {
    this->db = QSqlDatabase::addDatabase("QSQLITE");
    this->db.setDatabaseName(database);
    return this->db.open();
  } else {
    qDebug() << "Error: Invalid database driver specified. Available drivers: "
                "mysql, sqlite, sqlite2.";
    return false;
  }
}

QVariant ignsql::query(const QString &query) {
  bool status;
  int size;
  QVariantList dataRec;
  QVariantMap map;
  QVariantMap contentMap;
  QSqlQuery sqlQuery(this->db);

  sqlQuery.prepare(query);

  if (sqlQuery.exec()) {
    status = true;
  } else {
    status = false;
    contentMap.insert("error", sqlQuery.lastError().text());
  }

  contentMap.insert("status", status);

  QSqlRecord data = sqlQuery.record();

  while (sqlQuery.next()) {
    for (int index = 0; index < data.count(); index++) {
      QString key = data.fieldName(index);
      QVariant value = sqlQuery.value(index);
      map.insert(key, value);
    }

    dataRec << map;
  }

  contentMap.insert("content", dataRec);

  if (sqlQuery.size() > 0) {
    size = sqlQuery.size();
    contentMap.insert("size", size);
  }

  QJsonDocument json_enc = QJsonDocument::fromVariant(contentMap);
  return json_enc.toVariant();
}
