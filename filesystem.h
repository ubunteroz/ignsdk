//ibnu.yahya@toroo.org
#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <QObject>
#include <QSize>
#include <QtWidgets/QApplication>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QFileDialog>
#include <QTreeView>
#include <QDirIterator>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>
#include <QDateTime>

class ignfilesystem: public QObject{
    Q_OBJECT

public:
    ignfilesystem(QObject *parent = 0);

signals:

public slots:
    QString appPath();
    QString homePath();

    bool fileRemove(const QString &path);
    bool fileWrite(const QString &path, const QString &data);
    QString fileRead(const QString &path);

    bool dir(const QString &option, const QString &path);
    bool dirCreate(const QString &path);
    bool dirRemove(const QString &path);
    
    bool isExist(const QString &path);
    bool isDirectory(const QString &path);
    bool isFile(const QString &path);
    bool isAbsolute(const QString &path);
    bool isExecutable(const QString &path);
    bool isReadable(const QString &path);
    bool isWritable(const QString &path);
    bool isSymlink(const QString &path);

    bool copy(const QString &source, const QString &destination);

    QString openFileDialog();
    QString openDirDialog();
    QString saveFileDialog();

    QStringList list(const QString &path);
    QVariant info(const QString &path);
};

#endif // FILESYSTEM_H