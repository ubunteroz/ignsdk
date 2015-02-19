//ibnu.yahya@toroo.org
#ifndef DOWNLOAD_H
#define DOWNLOAD_H

#include <QObject>
#include <QString>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>

class QtDownload: public QObject{
    Q_OBJECT

public:
    explicit QtDownload();
    ~QtDownload();

    void setTarget(const QString &target);
    void save(const QString &destination);

private:
    QNetworkAccessManager manager;
    QString target;
    QString saveto;

signals:
    void done();
    void download_signal(qint64 received, qint64 total);
    
public slots:
    void download();
    void downloadFinished(QNetworkReply *data);
    void downloadProgress(qint64 received, qint64 total);
};

#endif // DOWNLOAD_H