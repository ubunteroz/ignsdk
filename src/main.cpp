/*ibnu.yahya@toroo.org*/
#include <QtWidgets/QApplication>
#include "ignsdk.h"
#include <QtWebKitWidgets/QWebView>
#include <QFileDialog>
#include <iostream>
#include "version.h"
#include <QCommandLineParser>

using namespace std;

int main(int argc, char *argv[]){
    QApplication app(argc, argv);
    ign ignsdk;

    QString url = NULL;
    bool file = false;
    QString optional;

    QCommandLineParser cmd_parser;
    cmd_parser.setApplicationDescription("IGOS Nusantara Software Development Kit");

    QCommandLineOption cmd_project(QStringList() << "p" << "project", "Specify project directory", "directory");
    cmd_parser.addOption(cmd_project);
    QCommandLineOption cmd_file(QStringList() << "f" << "file", "Load specific HTML file instead of index.html", "file");
    cmd_parser.addOption(cmd_file);
    QCommandLineOption cmd_dev(QStringList() << "d" << "development", "Activate development mode");
    cmd_parser.addOption(cmd_dev);
    QCommandLineOption cmd_remote(QStringList() << "r" << "remote", "Activate remote debugging", "port");
    cmd_parser.addOption(cmd_remote);
    QCommandLineOption cmd_version(QStringList() << "v" << "version", "Show version");
    cmd_parser.addOption(cmd_version);
    cmd_parser.addHelpOption();

    cmd_parser.process(app);

    if (cmd_parser.isSet(cmd_version)){
        printf("IGNSDK version %s (%s). Compiled on %s %s. Maintained by %s.\n", IGNSDK_VERSION, IGNSDK_CODENAME, __DATE__, __TIME__, IGNSDK_MAINTAINER);
        exit(0);
    }

    url = cmd_parser.value(cmd_project);

    if (cmd_parser.isSet(cmd_remote)){
        ignsdk.setDevRemote(cmd_parser.value(cmd_remote).toInt());
    }

    if (cmd_parser.isSet(cmd_file)){
        if (cmd_parser.isSet(cmd_project)){
            file = true;
            optional = cmd_parser.value(cmd_file);
        } else {
            qDebug() << "Error: Project directory must be specified.";
            exit(1);
        }
    }

    if (cmd_parser.isSet(cmd_dev)){
        ignsdk.setDev(true);
    }

    QString opt = url;
    QString path = url;

    if (!opt.isEmpty()){
        ignsdk.pathApp = opt;
        if (QFile::exists(path + "/icons/app.png")){
            app.setWindowIcon(QIcon(path + "/icons/app.png"));
        } else {
            app.setWindowIcon(QIcon(":/ignsdk.png"));
        }

        if (file){
            opt += "/";
            opt += optional;
        } else {
            opt += "/index.html";
        }

        if (QFile::exists(opt)){
            ignsdk.config(url);
            ignsdk.render(opt);
            ignsdk.show();
        } else {
            qDebug() << "Error:" << opt << "is not exist.";
            exit(1);
        }

    } else {
        QWidget *widget = new QWidget();
        QFileDialog *fileDialog = new QFileDialog(widget, "Select project directory...");
#ifdef Linux
        QTreeView *tree = fileDialog->findChild <QTreeView*>();
        tree->setRootIsDecorated(true);
        tree->setItemsExpandable(true);
#endif
        fileDialog->setFileMode(QFileDialog::Directory);
        fileDialog->setOption(QFileDialog::ShowDirsOnly);
        fileDialog->setViewMode(QFileDialog::Detail);
        int result = fileDialog->exec();
        QString directory;

        if (result){
            directory = fileDialog->selectedFiles()[0];
            ignsdk.pathApp = directory;

            if (QFile::exists(directory + "/index.html"))
            {
                if (QFile::exists(directory + "/icons/app.png")){
                    app.setWindowIcon(QIcon(directory + "/icons/app.png"));
                } else {
                    app.setWindowIcon(QIcon(":/ignsdk.png"));
                }
                ignsdk.config(directory);
                ignsdk.render(directory + "/index.html");
                ignsdk.show();
            } else {
                qDebug() << "Error:" << (directory + "/index.html") << "is not exist.";
                exit(1);
            }
        } else {
            exit(1);
        }
    }

    return app.exec();
}
