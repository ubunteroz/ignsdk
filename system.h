#ifndef SYSTEM_H
#define SYSTEM_H

#include <QObject>
#include <QtGlobal>
#include <QProcess>
#include <QCryptographicHash>
#include <QDesktopServices>
#include <QUrl>

class ignsystem: public QObject{
    Q_OBJECT

public:
    explicit ignsystem(QObject *parent = 0);
    QProcess *process;

public slots:
    QString cliOut(const QString &command);
    void exec(const QString &command);
    QString hash(const QString &data, QString hash_func);
    void desktopService(const QString &link);
    void _out();
    void kill();

signals:
    void out(const QString &link);
};

#endif // SYSTEM_H
