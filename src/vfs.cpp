#include "vfs.h"

ignvfs::ignvfs(QObject *parent): QObject(parent){}

bool ignvfs::mount(QString path, QString mountpoint){
	return vfs.mount(path, mountpoint);
}

bool ignvfs::umount(QString path, QString mountpoint){
	return vfs.umount(path, mountpoint);
}

QByteArray ignvfs::readFile(QString vpath, int offset, int length){
	return vfs.readFile(vpath, offset, length);
}

QString ignvfs::readFile64(QString vpath, int offset, int length){
	return vfs.readFile64(vpath, offset, length);
}

int ignvfs::fileSize(QString vpath){
	return vfs.fileSize(vpath);
}

QString ignvfs::hello(){
	return vfs.hello();
}