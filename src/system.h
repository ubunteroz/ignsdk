#ifndef SYSTEM_H
#define SYSTEM_H

#include <QObject>
#include <QtGlobal>
#include <QProcess>
#include <QCryptographicHash>
#include <QDesktopServices>
#include <QUrl>
#include <QFile>
#include <QFileInfo>
#include <QTextDocument>
#include <QPrinter>
#include <QPrintDialog>
#include <QPrintPreviewDialog>
#include "json.h"

class ignsystem: public QObject{
    Q_OBJECT

public:
    explicit ignsystem(QObject *parent = 0);
    QProcess *process;
    ignjson *jsonParse;

public slots:
    QString cliOut(const QString &command);
    int exec(const QString &command);
    QString hash(const QString &data, QString hash_func);
    QString hashFile(const QString &path, QString hash_algo);
    void desktopService(const QString &link);
    void _out();
    void kill();
    bool print(const QVariant &config);

signals:
    void out(const QString &link);
};

#endif // SYSTEM_H
