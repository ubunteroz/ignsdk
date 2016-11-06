#ifndef NETWORK_H
#define NETWORK_H

#include <QEventLoop>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QNetworkAccessManager>
#include <QNetworkInterface>
#include <QNetworkProxy>
#include <QNetworkReply>
#include <QObject>
#include <QUrl>
#include <QUrlQuery>

class ignnetwork : public QObject {
  Q_OBJECT

public:
  explicit ignnetwork(QObject *parent = 0);

public slots:
  QString myIP();
  void setProxy(const QVariant &config);
  QString get(const QString &url);
};

#endif // NETWORK_H
