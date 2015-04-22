#ifndef IGNVFS_H
#define IGNVFS_H

#include <QObject>
#include <QDebug>
#include "vfscontainer.h"

class ignvfs: public QObject {
	Q_OBJECT

private:
	vfscontainer vfs;

public:
	explicit ignvfs(QObject *parent = 0);

public slots:
	bool mount(QString path, QString mountpoint);
	bool umount(QString path, QString mountpoint);
	QByteArray readFile(QString vpath, int offset = 0, int length = 0);
	QString readFile64(QString vpath, int offset = 0, int length = 0);
	int fileSize(QString vpath);
	QString hello();
};

#endif