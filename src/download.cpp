//ibnu.yahya@toroo.org

#include "download.h"
#include <QCoreApplication>
#include <QUrl>
#include <QtNetwork/QNetworkRequest>
#include <QFile>
#include <QDebug>
#include <QDir>

QtDownload::QtDownload(): QObject(0){
    QObject::connect(&manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(downloadFinished(QNetworkReply*)));
}

QtDownload::~QtDownload(){}

void QtDownload::setTarget(const QString &target){
    this->target = target;
}

void QtDownload::save(const QString &destination){
    this->saveto = destination;
}

void QtDownload::downloadFinished(QNetworkReply *data){
    QUrl file = this->target;
    QFileInfo fileInfo(file.path());
    QString fileName = fileInfo.fileName();
    QString home = this->saveto;
    home += "/" + fileName;
    QFile localFile(home);

    if (!localFile.open(QIODevice::WriteOnly))
        return;

    const QByteArray sdata = data->readAll();
    localFile.write(sdata);
    localFile.close();

    emit done();
}

void QtDownload::download(){
    QUrl url = QUrl::fromEncoded(this->target.toLocal8Bit());
    QNetworkRequest request(url);
    QObject::connect(manager.get(request), SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(downloadProgress(qint64, qint64)));
}

void QtDownload::downloadProgress(qint64 received, qint64 total){
    emit download_signal(received, total);
}
