#ifndef VFSPROTOCOL_H
#define VFSPROTOCOL_H

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QtWebKitWidgets/QtWebKitWidgets>
#include "vfscontainer.h"

class NetworkAccessManager;

class NetworkAccessManager : public QNetworkAccessManager
{
    Q_OBJECT

public:
    NetworkAccessManager(QNetworkAccessManager *oldManager, QObject *parent = 0);

protected:    
    QNetworkReply *createRequest(Operation operation, const QNetworkRequest &request, QIODevice *device);
};

class VfsReply: public QNetworkReply{
    Q_OBJECT

public:
    VfsReply(const QUrl &url);
    void abort();
    qint64 bytesAvailable() const;
    bool isSequential() const;

protected:
	qint64 readData(char *data, qint64 maxSize);

private:
	vfscontainer vfs;
	qint64 offset;
	QByteArray content;
};    

class VfsView: public QWebView{
    Q_OBJECT

public:
    VfsView();
};

#endif