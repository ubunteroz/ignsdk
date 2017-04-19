#include "sql.h"

ignsql::ignsql(QObject *parent) : QObject(parent), jsonParse(0) {}

bool ignsql::driver(const QString &driver, QString connection) {
  if (driver == QLatin1String("mysql")) {
    QStringList details = connection.split(QStringLiteral(":"));
    this->db = QSqlDatabase::addDatabase(QStringLiteral("QMYSQL"));
    this->db.setHostName(details.value(0));
    this->db.setUserName(details.value(1));
    this->db.setPassword(details.value(2));
    this->db.setDatabaseName(details.value(3));
    return this->db.open();
  } else if (driver == QLatin1String("sqlite2")) {
    this->db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE2"));
    this->db.setDatabaseName(connection);
    return this->db.open();
  } else if (driver == QLatin1String("sqlite")) {
    this->db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"));
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
  QString driver = configuration[QStringLiteral("driver")].toString();
  QString hostname = configuration[QStringLiteral("hostname")].toString();
  QString username = configuration[QStringLiteral("username")].toString();
  QString password = configuration[QStringLiteral("password")].toString();
  QString database = configuration[QStringLiteral("db")].toString();
  database = configuration[QStringLiteral("database")].toString();

  if (driver == QLatin1String("mysql")) {
    this->db = QSqlDatabase::addDatabase(QStringLiteral("QMYSQL"));
    this->db.setHostName(hostname);
    this->db.setUserName(username);
    this->db.setPassword(password);
    this->db.setDatabaseName(database);
    return this->db.open();
  } else if (driver == QLatin1String("sqlite2")) {
    this->db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE2"));
    this->db.setDatabaseName(database);
    return this->db.open();
  } else if (driver == QLatin1String("sqlite")) {
    this->db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"));
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
    contentMap.insert(QStringLiteral("error"), sqlQuery.lastError().text());
  }

  contentMap.insert(QStringLiteral("status"), status);

  QSqlRecord data = sqlQuery.record();

  while (sqlQuery.next()) {
    for (int index = 0; index < data.count(); index++) {
      QString key = data.fieldName(index);
      QVariant value = sqlQuery.value(index);
      map.insert(key, value);
    }

    dataRec << map;
  }

  contentMap.insert(QStringLiteral("content"), dataRec);

  if (sqlQuery.size() > 0) {
    size = sqlQuery.size();
    contentMap.insert(QStringLiteral("size"), size);
  }

  QJsonDocument json_enc = QJsonDocument::fromVariant(contentMap);
  return json_enc.toVariant();
}
