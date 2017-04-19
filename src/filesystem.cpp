// ibnu.yahya@toroo.org
#include "filesystem.h"
#include <QDebug>

ignfilesystem::ignfilesystem(QObject *parent) : QObject(parent), jsonParse(0) {}

bool ignfilesystem::fileRemove(const QString &path) {
  QFile file(path);
  return file.remove();
}

bool ignfilesystem::fileWrite(const QString &path, const QString &data) {
  QFile file(path);
  if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    QTextStream out(&file);
    out << data;
    return true;
  } else {
    return false;
  }
  file.close();
}

QString ignfilesystem::fileRead(const QString &path) {
  QFile file(path);
  if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    QTextStream out(&file);
    QString data = out.readAll();
    file.close();
    return data;
  } else {
    return NULL;
    qDebug() << "Err : File not found";
  }
}

QString ignfilesystem::appPath() { return QApplication::applicationDirPath(); }

QString ignfilesystem::homePath() {
  QString home = QDir::homePath();
  return home;
}

bool ignfilesystem::dir(const QString &option, const QString &path) {
  QDir directory;
  if (option == QLatin1String("create")) {
    return directory.mkdir(path);
  } else if (option == QLatin1String("remove")) {
    return directory.rmdir(path);
  } else {
    return false;
  }
}

bool ignfilesystem::dirCreate(const QString &path) {
  QDir directory;
  return directory.mkdir(path);
}

bool ignfilesystem::dirRemove(const QString &path) {
  QDir directory;
  return directory.rmdir(path);
}

bool ignfilesystem::isExist(const QString &path) { return QFile::exists(path); }

bool ignfilesystem::isDirectory(const QString &path) {
  return QFileInfo(path).isDir();
}

bool ignfilesystem::isFile(const QString &path) {
  return QFileInfo(path).isFile();
}

bool ignfilesystem::isAbsolute(const QString &path) {
  return QFileInfo(path).isAbsolute();
}

bool ignfilesystem::isExecutable(const QString &path) {
  return QFileInfo(path).isExecutable();
}

bool ignfilesystem::isSymlink(const QString &path) {
  return QFileInfo(path).isSymLink();
}

bool ignfilesystem::isReadable(const QString &path) {
  return QFileInfo(path).isReadable();
}

bool ignfilesystem::isWritable(const QString &path) {
  return QFileInfo(path).isWritable();
}

bool ignfilesystem::copy(const QString &source, const QString &destination) {
  if (QFile::exists(destination)) {
    QFile::remove(destination);
  }

  return QFile::copy(source, destination);
}

QString ignfilesystem::openFileDialog() {
  QFileDialog *fileDialog = new QFileDialog;
  int result = fileDialog->exec();

  if (result) {
    QString directory = fileDialog->selectedFiles()[0];
    return directory;
  } else {
    return NULL;
  }
}

QString ignfilesystem::openDirDialog() {
  QFileDialog *fileDialog = new QFileDialog;
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
    return directory;
  } else {
    return NULL;
  }
}

QString ignfilesystem::saveFileDialog() {
  QFileDialog *fileDialog = new QFileDialog;
  QString directory = fileDialog->getSaveFileName();
  return directory;
}

QString ignfilesystem::saveFileDialog(const QVariant &config) {
  QVariantMap configuration = jsonParse->jsonParser(config).toVariantMap();
  QString title = QStringLiteral("Save File"), path = this->homePath(), extension = QLatin1String("");

  if (configuration[QStringLiteral("title")].toString() != QLatin1String("")) {
    title = configuration[QStringLiteral("title")].toString();
  }

  if (configuration[QStringLiteral("path")].toString() != QLatin1String("")) {
    path = configuration[QStringLiteral("path")].toString();
  }

  if (configuration[QStringLiteral("type")].toString() != QLatin1String("")) {
    extension = configuration[QStringLiteral("type")].toString();
  }

  QFileDialog *fileDialog = new QFileDialog;
  QWidget *widget = new QWidget();
  QString savePath =
      fileDialog->getSaveFileName(widget, title, path, extension);
  return savePath;
}

QStringList ignfilesystem::list(const QString &path) {
  QDirIterator dirIt(path, QDirIterator::Subdirectories);
  QStringList list;

  while (dirIt.hasNext()) {
    dirIt.next();
    list.push_front(dirIt.filePath());
  }

  return list;
}

QVariant ignfilesystem::info(const QString &path) {
  QVariantMap map;
  QFileInfo info(path);
  QVariant size = info.size();
  QVariant absoluteFilePath = info.absoluteFilePath();
  QVariant baseName = info.baseName();
  QVariant isSymlink = info.isSymLink();
  QVariant isAbsolute = info.isAbsolute();
  QVariant isBundle = info.isBundle();
  QVariant isDir = info.isDir();
  QVariant isExecutable = info.isExecutable();
  QVariant isFile = info.isFile();
  QVariant isHidden = info.isHidden();
  QVariant isReadable = info.isReadable();
  QVariant isRelative = info.isRelative();
  QVariant isRoot = info.isRoot();
  QVariant isWritable = info.isWritable();
  QVariant bundleName = info.bundleName();
  QVariant exists = info.exists();
  QVariant fileName = info.fileName();
  QVariant filePath = info.filePath();
  QVariant group = info.group();
  QVariant lastModified = info.lastModified();
  QVariant lastRead = info.lastRead();
  map.insert(QStringLiteral("size"), size);
  map.insert(QStringLiteral("absoluteFilePath"), absoluteFilePath);
  map.insert(QStringLiteral("baseName"), baseName);
  map.insert(QStringLiteral("isSymlink"), isSymlink);
  map.insert(QStringLiteral("isAbsolute"), isAbsolute);
  map.insert(QStringLiteral("isBundle"), isBundle);
  map.insert(QStringLiteral("isDir"), isDir);
  map.insert(QStringLiteral("isExecutable"), isExecutable);
  map.insert(QStringLiteral("isFile"), isFile);
  map.insert(QStringLiteral("isHidden"), isHidden);
  map.insert(QStringLiteral("isReadable"), isReadable);
  map.insert(QStringLiteral("isRelative"), isRelative);
  map.insert(QStringLiteral("isRoot"), isRoot);
  map.insert(QStringLiteral("isWritable"), isWritable);
  map.insert(QStringLiteral("filePath"), filePath);
  map.insert(QStringLiteral("bundleName"), bundleName);
  map.insert(QStringLiteral("exists"), exists);
  map.insert(QStringLiteral("fileName"), fileName);
  map.insert(QStringLiteral("group"), group);
  map.insert(QStringLiteral("lastModified"), lastModified);
  map.insert(QStringLiteral("lastRead"), lastRead);
  QJsonDocument json_enc = QJsonDocument::fromVariant(map);
  return json_enc.toVariant();
}
