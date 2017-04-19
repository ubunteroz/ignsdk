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
    return QStringLiteral("IP not found");
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

  if (proxyConfig[QStringLiteral("type")].toString() != QLatin1String("")) {
    QNetworkProxy proxy;
    QString proxyType = proxyConfig[QStringLiteral("type")].toString();

    if (proxyType == QLatin1String("http")) {
      proxy.setType(QNetworkProxy::HttpProxy);
    } else if (proxyType == QLatin1String("socks5")) {
      proxy.setType(QNetworkProxy::Socks5Proxy);
    } else if (proxyType == QLatin1String("ftp")) {
      proxy.setType(QNetworkProxy::FtpCachingProxy);
    } else if (proxyType == QLatin1String("httpCaching")) {
      proxy.setType(QNetworkProxy::HttpCachingProxy);
    } else {
      qDebug() << "Proxy type is not specified. Available options: http, "
                  "socks5, ftp, httpCaching.";
    }

    if (proxyConfig[QStringLiteral("url")].toString() != QLatin1String("")) {
      QString url = proxyConfig[QStringLiteral("url")].toString();
      QStringList proxyUrl = url.split(QStringLiteral(":"));
      proxy.setHostName(proxyUrl.at(0));
      proxy.setPort(proxyUrl.at(1).toInt());
    } else {
      qDebug() << "Proxy address is not specified.";
    }

    if (proxyConfig[QStringLiteral("username")].toString() != QLatin1String("")) {
      proxy.setUser(proxyConfig[QStringLiteral("username")].toString());
    }

    if (proxyConfig[QStringLiteral("password")].toString() != QLatin1String("")) {
      proxy.setPassword(proxyConfig[QStringLiteral("password")].toString());
    }

    QNetworkProxy::setApplicationProxy(proxy);
  }
}

QString ignnetwork::get(const QString &url) {
  QEventLoop eventLoop;
  QNetworkAccessManager manager;
  QObject::connect(&manager, &QNetworkAccessManager::finished, &eventLoop,
                   &QEventLoop::quit);
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
