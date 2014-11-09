#include "ignrocksdb.h"

ignrocksdb::ignrocksdb(QObject* parent):
	QObject(parent){}

rocksdb::DB* rdbDB;
rocksdb::Options rdbOptions;
rocksdb::WriteBatch rdbWriteBatch;
bool isRdbOpen = false;

void rdbOpen(QString wal_dir){
	rocksdb::Status status = rdbDB->Open(rdbOptions, wal_dir.toStdString(), &rdbDB);
	if (status.ok())
		isRdbOpen = true;
	else
		isRdbOpen = false;
}

void ignrocksdb::init(QString wal_dir){
	if (!isRdbOpen){
		rdbOptions.IncreaseParallelism();
		rdbOptions.OptimizeLevelStyleCompaction();
		rdbOptions.compression = rocksdb::kLZ4Compression;
		rdbOptions.create_if_missing = true;
		rdbOptions.keep_log_file_num = 1;
		rdbOpen(wal_dir);
	} else {
		ignrocksdb::close();
		rdbOpen(wal_dir);
	}
}

void ignrocksdb::close(){
	if (isRdbOpen){
		delete rdbDB;
		isRdbOpen = false;
	}
}

bool ignrocksdb::put(QString key, QString value){
	rocksdb::Status status = rdbDB->Put(rocksdb::WriteOptions(), key.toStdString(), value.toStdString());
	return status.ok();
}

void ignrocksdb::bput(QString key, QString value){
	rdbWriteBatch.Put(key.toStdString(), value.toStdString());
}

void ignrocksdb::bdel(QString key){
	rdbWriteBatch.Delete(key.toStdString());
}

bool ignrocksdb::bwrite(){
	rocksdb::Status status = rdbDB->Write(rocksdb::WriteOptions(), &rdbWriteBatch);
	return status.ok();
}

QString ignrocksdb::get(QString key){
	std::string value;
	rdbDB->Get(rocksdb::ReadOptions(), key.toStdString(), &value);
	QString value_ = QString::fromUtf8(value.c_str());
	return value_;
}

void ignrocksdb::del(QString key){
	rdbDB->Delete(rocksdb::WriteOptions(), key.toStdString());
}