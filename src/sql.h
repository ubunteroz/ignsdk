#ifndef SQL_H
#define SQL_H

#include <QObject>
#include <QtSql>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>

class ignsql: public QObject {
    Q_OBJECT

public:
    explicit ignsql(QObject *parent = 0);
    QSqlDatabase db;

signals:

public slots:
    bool driver(const QString &driver, QString connection);
    QVariant query(const QString &query);
};

#endif // SQL_H
