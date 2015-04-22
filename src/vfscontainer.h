#ifndef VFSCONT_H
#define VFSCONT_H

#include <QObject>
#include <QDebug>
#include "external/ttvfs/ttvfs/VFS.h"
#include "external/ttvfs/ttvfs/VFSZipArchiveLoader.h"
#include "external/liblvpa/lvpa_ttvfs/VFSLVPAArchiveLoader.h"

class vfscontainer {
	private:
		static ttvfs::VFSHelper vfsHelper;
		static bool isVFSInitialized;
		static void _dirCallback(ttvfs::VFSDir *vd, void *);
		static void _fileCallback(ttvfs::VFSFile *vf, void *);
		static QByteArray _readFile(QString vpath, int offset, int length);

	public:
		explicit vfscontainer();
		static bool mount(QString path, QString mountpoint);
		static bool umount(QString path, QString mountpoint);
		static QByteArray readFile(QString vpath, int offset = 0, int length = 0);
		static QString readFile64(QString vpath, int offset = 0, int length = 0);
		static int fileSize(QString vpath);
		static void list(QString vpath);
		static QString hello();
};

#endif