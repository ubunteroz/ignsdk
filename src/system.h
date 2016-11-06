#ifndef SYSTEM_H
#define SYSTEM_H

#include "json.h"
#include "process.h"
#include "serial.h"
#include <QCryptographicHash>
#include <QDesktopServices>
#include <QFile>
#include <QFileInfo>
#include <QObject>
#include <QPrintDialog>
#include <QPrintPreviewDialog>
#include <QPrinter>
#include <QProcess>
#include <QTextDocument>
#include <QUrl>
#include <QtGlobal>

class ignsystem : public QObject {
  Q_OBJECT

public:
  explicit ignsystem(QObject *parent = 0);
  ignjson *jsonParse;
  ignprocess *process;
  ignserial *m_serial;

public slots:
  QString cliOut(const QString &command);
  QObject *exec(const QString &command);
  QVariant serial();
  QObject *serial(const QVariant &config);
  QString hash(const QString &data, QString hash_func);
  QString hashFile(const QString &path, QString hash_algo);
  void desktopService(const QString &link);
  bool print(const QVariant &config);

signals:
};

#endif // SYSTEM_H
