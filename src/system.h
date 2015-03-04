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
#include "process.h"

class ignsystem: public QObject{
    Q_OBJECT

public:
    explicit ignsystem(QObject *parent = 0);
    ignjson *jsonParse;
    ignprocess *process;

public slots:
    QString cliOut(const QString &command);
    QObject *exec(const QString &command);
    QString hash(const QString &data, QString hash_func);
    QString hashFile(const QString &path, QString hash_algo);
    void desktopService(const QString &link);
    bool print(const QVariant &config);

signals:
};

#endif // SYSTEM_H
