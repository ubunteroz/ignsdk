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
    bool driver(const QString &drv, QString connect);
    QVariant query(const QString &qr);
};

#endif // SQL_H
