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
  if (option == "create") {
    return directory.mkdir(path);
  } else if (option == "remove") {
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
  QString title = "Save File", path = this->homePath(), extension = "";

  if (configuration["title"].toString() != "") {
    title = configuration["title"].toString();
  }

  if (configuration["path"].toString() != "") {
    path = configuration["path"].toString();
  }

  if (configuration["type"].toString() != "") {
    extension = configuration["type"].toString();
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
  map.insert("size", size);
  map.insert("absoluteFilePath", absoluteFilePath);
  map.insert("baseName", baseName);
  map.insert("isSymlink", isSymlink);
  map.insert("isAbsolute", isAbsolute);
  map.insert("isBundle", isBundle);
  map.insert("isDir", isDir);
  map.insert("isExecutable", isExecutable);
  map.insert("isFile", isFile);
  map.insert("isHidden", isHidden);
  map.insert("isReadable", isReadable);
  map.insert("isRelative", isRelative);
  map.insert("isRoot", isRoot);
  map.insert("isWritable", isWritable);
  map.insert("filePath", filePath);
  map.insert("bundleName", bundleName);
  map.insert("exists", exists);
  map.insert("fileName", fileName);
  map.insert("group", group);
  map.insert("lastModified", lastModified);
  map.insert("lastRead", lastRead);
  QJsonDocument json_enc = QJsonDocument::fromVariant(map);
  return json_enc.toVariant();
}
