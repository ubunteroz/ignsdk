//ibnu.yahya@toroo.org
#ifndef IGNSDK_H
#define IGNSDK_H

#include "filesystem.h"
#include "download.h"
#include "sql.h"
#include "system.h"
#include "network.h"
#include <QObject>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QWidget>
#include <QtWidgets/QLayout>
#include <QtWidgets/QMessageBox>
#include <QtWebKitWidgets/QtWebKitWidgets>
#include <QtNetwork/QNetworkInterface>
#include <QSize>
#include <QVariant>
#include <QPixmap>
#include <QtWidgets/QSplashScreen>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>
#include <QNetworkProxy>
#include <QFileSystemWatcher>
#include <QThread>
#include <QPrinter>
#include <QPrintDialog>

class ign: public QObject {
    Q_OBJECT

private:
    QWebView web;
    QWebFrame *frame;
    bool fullscreen;
    QtDownload *dl;
    ignsql *m_sql;
    ignsystem *m_system;
    ignfilesystem *m_filesystem;
    ignnetwork *m_network;
    QPoint offset;
    bool mMoving;

public:
    ign(QObject *parent = 0);
    void render(QString w);
    void show();
    void widgetNoFrame();
    void widgetTransparent();
    void liveCode();
    QString pathLive;
    QString pathApp;
    QString version;
    bool debugging;
    QFileSystemWatcher live;
    QPrinter printer;

signals:
    void downloadProgress(qint64 received, qint64 total);

private slots:
    void fileChanged(const QString& path){
        qDebug() << "Changes detected on" << path;
        QThread::msleep(50);
        this->web.page()->triggerAction(QWebPage::Reload,true);
    }

public slots:
    void ignJS();
    void setUrl(const QString& url);

    // Message box
    QString showMessageBox(const QString &title, const QString &message, const QString &button);

    // Print support
    void print();

    // Development mode
    void setDev(bool v);
    void setDevRemote(int port);

    // Action
    void quit();
    void back();
    void forward();
    void reload();
    void stop();
    void cut();
    void copy();
    void paste();
    void undo();
    void redo();

    // Window manipulation
    void widgetSizeMax(int w, int h);
    void widgetSizeMin(int w, int h);
    void widgetSize(int w, int h);
    void widgetNoTaskbar();
    void getToggleFullScreen();
    void getFullScreen(bool screen);
    void showMaximized();
    void showMinimized();

    // Load executable file in bin/
    QString loadBin(const QString &script);

    // Load app configuration
    void config(QString path);

    // Websecurity
    void websecurity(bool c);

    // Network
    void saveFile(const QByteArray &data, QString filename, QString path);
    void download(QString data, QString path);
    void download_signal(qint64 received, qint64 total);

    // Evaluate external JS
    void include(QString path);

    // Filesystem
    QObject *filesystem();

    // SQL
    QObject *sql();

    // System
    QObject *system();
    QObject *sys();

    // Network
    QObject *network();
    QObject *net();

    // IGNSDK Version
    QString sdkVersion();
};

#endif // IGN_H
