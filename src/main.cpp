/*ibnu.yahya@toroo.org*/
#include "ignsdk.h"
#include "version.h"
#include <QCommandLineParser>
#include <QFileDialog>
#include <QtWebKitWidgets/QWebView>
#include <QtWidgets/QApplication>
#include <iostream>

using namespace std;

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);
  ign ignsdk;

  QString url = NULL;
  bool file = false;
  QString optional;

  QCommandLineParser cmd_parser;
  cmd_parser.setApplicationDescription(
      QStringLiteral("IGOS Nusantara Software Development Kit"));

  QCommandLineOption cmd_project(QStringList() << QStringLiteral("p")
                                               << QStringLiteral("project"),
                                 QStringLiteral("Specify project directory"), QStringLiteral("directory"));
  cmd_parser.addOption(cmd_project);
  QCommandLineOption cmd_file(QStringList() << QStringLiteral("f")
                                            << QStringLiteral("file"),
                              QStringLiteral("Load specific HTML file instead of index.html"),
                              QStringLiteral("file"));
  cmd_parser.addOption(cmd_file);
  QCommandLineOption cmd_dev(QStringList() << QStringLiteral("d")
                                           << QStringLiteral("development"),
                             QStringLiteral("Activate development mode"));
  cmd_parser.addOption(cmd_dev);
  QCommandLineOption cmd_remote(QStringList() << QStringLiteral("r")
                                              << QStringLiteral("remote"),
                                QStringLiteral("Activate remote debugging"), QStringLiteral("port"));
  cmd_parser.addOption(cmd_remote);
  QCommandLineOption cmd_version(QStringList() << QStringLiteral("v")
                                               << QStringLiteral("version"),
                                 QStringLiteral("Show version"));
  cmd_parser.addOption(cmd_version);
  cmd_parser.addHelpOption();

  cmd_parser.process(app);

  if (cmd_parser.isSet(cmd_version)) {
    printf("\033[1m\033[37mIGNSDK version %s (%s)\033[0m\nCompiled on %s "
           "%s\nMaintained by \033[1m\033[37m%s\033[0m <%s>\n",
           IGNSDK_VERSION, IGNSDK_CODENAME, __DATE__, __TIME__,
           IGNSDK_MAINTAINER, IGNSDK_EMAIL);
    return 0;
  }

  url = cmd_parser.value(cmd_project);

  if (cmd_parser.isSet(cmd_remote)) {
    ignsdk.setDevRemote(cmd_parser.value(cmd_remote).toInt());
  }

  if (cmd_parser.isSet(cmd_file)) {
    if (cmd_parser.isSet(cmd_project)) {
      file = true;
      optional = cmd_parser.value(cmd_file);
    } else {
      qDebug() << "Error: Project directory must be specified.";
      return 1;
    }
  }

  if (cmd_parser.isSet(cmd_dev)) {
    ignsdk.setDev(true);
  }

  QString opt = url;
  QString path = url;

  if (!opt.isEmpty()) {
    ignsdk.pathApp = opt;
    if (QFile::exists(path + "/icons/app.png")) {
      app.setWindowIcon(QIcon(path + "/icons/app.png"));
    } else {
      app.setWindowIcon(QIcon(":/ignsdk.png"));
    }

    if (file) {
      opt += QLatin1String("/");
      opt += optional;
    } else {
      opt += QLatin1String("/index.html");
    }

    if (QFile::exists(opt)) {
      ignsdk.config(url);
      ignsdk.render(opt);
      ignsdk.show();
    } else {
      qDebug() << "Error:" << opt << "is not exist.";
      return 1;
    }

  } else {
    QWidget *widget = new QWidget();
    QFileDialog *fileDialog =
        new QFileDialog(widget, QStringLiteral("Select project directory..."));
#ifdef Linux
    QTreeView *tree = fileDialog->findChild<QTreeView *>();
    tree->setRootIsDecorated(true);
    tree->setItemsExpandable(true);
#endif
    fileDialog->setFileMode(QFileDialog::Directory);
    fileDialog->setOption(QFileDialog::ShowDirsOnly);
    fileDialog->setViewMode(QFileDialog::Detail);
    int result = fileDialog->exec();
    QString directory;

    if (result) {
      directory = fileDialog->selectedFiles()[0];
      ignsdk.pathApp = directory;

      if (QFile::exists(directory + "/index.html")) {
        if (QFile::exists(directory + "/icons/app.png")) {
          app.setWindowIcon(QIcon(directory + "/icons/app.png"));
        } else {
          app.setWindowIcon(QIcon(":/ignsdk.png"));
        }
        ignsdk.config(directory);
        ignsdk.render(directory + "/index.html");
        ignsdk.show();
      } else {
        qDebug() << "Error:" << (directory + "/index.html") << "is not exist.";
        return 1;
      }
    } else {
      return 1;
    }
  }

  return app.exec();
}
