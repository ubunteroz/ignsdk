#include "vfscontainer.h"

vfscontainer::vfscontainer(){
	if (!isVFSInitialized){
		qDebug() << "DEBUG: Initializing VFS...";
		vfsHelper.LoadFileSysRoot(false);
		vfsHelper.Prepare();
		vfsHelper.AddArchiveLoader(new ttvfs::VFSZipArchiveLoader);
		vfsHelper.AddArchiveLoader(new ttvfs::VFSLVPAArchiveLoader);
		isVFSInitialized = true;
	}
}

ttvfs::VFSHelper vfscontainer::vfsHelper;

bool vfscontainer::isVFSInitialized = false;

void vfscontainer::_fileCallback(ttvfs::VFSFile *vf, void *){
	std::cout << vf->name() << " --> " << vf->fullname() << std::endl;
}

void vfscontainer::_dirCallback(ttvfs::VFSDir *vd, void *){
	std::cout << vd->name() << " --> " << vd->fullname() << std::endl;
}

bool vfscontainer::mount(QString path, QString mountpoint){
	QByteArray archive_ba = path.toUtf8();
	QByteArray vdir_ba = mountpoint.toUtf8();
	char *archive = archive_ba.data();
	char *vdir = vdir_ba.data();
	printf("DEBUG: Mounting '%s' on '%s'...\n", archive, vdir);
	return vfsHelper.AddArchive(archive, false, vdir);
}

bool vfscontainer::umount(QString path, QString mountpoint){
	QByteArray archive_ba = path.toUtf8();
	QByteArray vdir_ba = mountpoint.toUtf8();
	char *archive = archive_ba.data();
	char *vdir = vdir_ba.data();
	printf("DEBUG: Unmounting '%s' from '%s'...\n", archive, vdir);
	return vfsHelper.Unmount(archive, vdir);
}

QByteArray vfscontainer::_readFile(QString vpath, int offset, int length){
	QByteArray data;
	QByteArray filepath_ba = vpath.toUtf8();
	char *filepath = filepath_ba.data();
	ttvfs::VFSFile *vf = vfsHelper.GetFile(filepath);
	if ((offset < 0) || (length < 0)){
		printf("DEBUG: Error: Invalid offset/length value, returning NULL\n");
		return NULL;
	}
	if ((offset > vf->size()) || (offset == vf->size())){
		printf("DEBUG: Warning: Offset %i is beyond/at the end of file size %i, starting at 0\n", offset, vf->size());
		offset = 0;
	}
	if (length == 0){
		length = vf->size() - offset;
	}
	int seek_end = offset + length;
	if (seek_end > vf->size()){
		printf("DEBUG: Warning: Seeking beyond file size, stopping at %i\n", vf->size());
		length = vf->size() - offset;
	}
	printf("DEBUG: Reading '%s', offset %i, length %i bytes, file size %i bytes\n", filepath, offset, length, vf->size());
	if (vf){
		vf->open("rb");
		data = QByteArray::fromRawData((const char *)vf->getBuf(), vf->size()).mid(offset, length);
		QDataStream in(&data, QIODevice::ReadOnly);
		vf->close();
	}

	return data;
}

QByteArray vfscontainer::readFile(QString vpath, int offset, int length){
	return _readFile(vpath, offset, length);
}

QString vfscontainer::readFile64(QString vpath, int offset, int length){
	QByteArray data = _readFile(vpath, offset, length);
	return data.toBase64();
}

int vfscontainer::fileSize(QString vpath){
	QByteArray byteArray;
	QByteArray filepath_ba = vpath.toUtf8();
	char *filepath = filepath_ba.data();
	ttvfs::VFSFile *vf = vfsHelper.GetFile(filepath);
	vf = vfsHelper.GetFile(filepath);
	return (const int)vf->size();
}

void vfscontainer::list(QString vpath){
	QByteArray vdir_ba = vpath.toUtf8();
	char *vdir = vdir_ba.data();
	ttvfs::VFSDir *vd = vfsHelper.GetDir(vdir);
	vd->forEachDir(_dirCallback);
	vd->forEachFile(_fileCallback);
}

QString vfscontainer::hello(){
	return "Hello from VFS!";
}