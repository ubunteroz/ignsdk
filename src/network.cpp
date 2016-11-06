#include "network.h"

ignnetwork::ignnetwork(QObject *parent) : QObject(parent) {}

QString ignnetwork::myIP() {
  QString host;

  Q_FOREACH (QHostAddress address, QNetworkInterface::allAddresses()) {
    if (!address.isLoopback() &&
        (address.protocol() == QAbstractSocket::IPv4Protocol)) {
      host = address.toString();
      break;
    }
  }

  if (host.isEmpty()) {
    return "IP not found";
  } else {
    return host;
  }
}

void ignnetwork::setProxy(const QVariant &config) {
  QJsonParseError *parseError = new QJsonParseError();
  QJsonDocument json = QJsonDocument::fromVariant(config);

  if (parseError->error != 0) {
    qDebug() << parseError->errorString();
    exit(1);
  }

  QJsonObject jObject = json.object();
  QVariantMap proxyConfig = jObject.toVariantMap();

  if (proxyConfig["type"].toString() != "") {
    QNetworkProxy proxy;
    QString proxyType = proxyConfig["type"].toString();

    if (proxyType == "http") {
      proxy.setType(QNetworkProxy::HttpProxy);
    } else if (proxyType == "socks5") {
      proxy.setType(QNetworkProxy::Socks5Proxy);
    } else if (proxyType == "ftp") {
      proxy.setType(QNetworkProxy::FtpCachingProxy);
    } else if (proxyType == "httpCaching") {
      proxy.setType(QNetworkProxy::HttpCachingProxy);
    } else {
      qDebug() << "Proxy type is not specified. Available options: http, "
                  "socks5, ftp, httpCaching.";
    }

    if (proxyConfig["url"].toString() != "") {
      QString url = proxyConfig["url"].toString();
      QStringList proxyUrl = url.split(":");
      proxy.setHostName(proxyUrl.at(0));
      proxy.setPort(proxyUrl.at(1).toInt());
    } else {
      qDebug() << "Proxy address is not specified.";
    }

    if (proxyConfig["username"].toString() != "") {
      proxy.setUser(proxyConfig["username"].toString());
    }

    if (proxyConfig["password"].toString() != "") {
      proxy.setPassword(proxyConfig["password"].toString());
    }

    QNetworkProxy::setApplicationProxy(proxy);
  }
}

QString ignnetwork::get(const QString &url) {
  QEventLoop eventLoop;
  QNetworkAccessManager manager;
  QObject::connect(&manager, SIGNAL(finished(QNetworkReply *)), &eventLoop,
                   SLOT(quit()));
  QUrl uri(url);
  QNetworkRequest request(uri);
  QNetworkReply *reply = manager.get(request);
  eventLoop.exec();

  if (reply->error() == QNetworkReply::NoError) {
    return reply->readAll();
    delete reply;
  } else {
    return reply->errorString();
    delete reply;
  }
}
