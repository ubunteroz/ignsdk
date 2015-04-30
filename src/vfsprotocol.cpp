#include <QtNetwork>
#include <QTimer>
#include "vfsprotocol.h"

NetworkAccessManager::NetworkAccessManager(QNetworkAccessManager *manager, QObject *parent):
QNetworkAccessManager(parent){
    setCache(manager->cache());
    setCookieJar(manager->cookieJar());
    setProxy(manager->proxy());
    setProxyFactory(manager->proxyFactory());
}

QNetworkReply *NetworkAccessManager::createRequest(
    QNetworkAccessManager::Operation operation, const QNetworkRequest &request,
    QIODevice *device)
{
    if (operation == GetOperation && request.url().scheme() == "vfs"){
        qDebug() << "DEBUG: Requesting" << request.url().toString() << "from VFS...";
        return new VfsReply(request.url());
    } else {
        return QNetworkAccessManager::createRequest(operation, request, device);
    }
}

VfsReply::VfsReply(const QUrl &url): QNetworkReply(){
    offset = 0;

    QString path = url.toString(QUrl::RemoveQuery | QUrl::RemoveFragment | QUrl::StripTrailingSlash | QUrl::NormalizePathSegments);
    QString path_fixed = path.mid(6);

    open(QNetworkReply::ReadOnly | QNetworkReply::Unbuffered);
    content = vfs.readFile(path_fixed);
    setHeader(QNetworkRequest::ContentLengthHeader, QVariant(content.size()));
    QTimer::singleShot(0, this, SIGNAL(metaDataChanged()));
    QTimer::singleShot(0, this, SIGNAL(readyRead()));
    QTimer::singleShot(0, this, SIGNAL(finished()));
}

void VfsReply::abort(){}

qint64 VfsReply::bytesAvailable() const
{
    return content.size() - offset + QIODevice::bytesAvailable();
}

bool VfsReply::isSequential() const
{
    return true;
}

qint64 VfsReply::readData(char *data, qint64 maxSize){
if (offset < content.size()) {
    qint64 number = qMin(maxSize, content.size() - offset);
    memcpy(data, content.constData() + offset, number);
    offset += number;
    return number;
} else
    return -1;
}

VfsView::VfsView()
{
    QNetworkAccessManager *oldManager = page()->networkAccessManager();
    NetworkAccessManager *newManager = new NetworkAccessManager(oldManager, this);
    page()->setNetworkAccessManager(newManager);
}