#include "ignrocksdb.h"

ignrocksdb::ignrocksdb(QObject* parent):
	QObject(parent){}

rocksdb::DB* rdbDB;
rocksdb::Options rdbOptions;

void ignrocksdb::init(){
	printf("Initializing Facebook RocksDB...\n");
	rdbOptions.IncreaseParallelism();
	rdbOptions.OptimizeLevelStyleCompaction();
	rdbOptions.create_if_missing = true;
	rdbDB->Open(rdbOptions, "/tmp/ignsdk.rdb", &rdbDB);
}

void ignrocksdb::close(){
	printf("Closing RocksDB database...\n");
	delete rdbDB;
}

bool ignrocksdb::put(QString key, QString value){
	rocksdb::Status status = rdbDB->Put(rocksdb::WriteOptions(), key.toStdString(), value.toStdString());
	printf("Put \"%s\":\"%s\"\n", key.toStdString().c_str(), value.toStdString().c_str());
	return status.ok();
}

QString ignrocksdb::get(QString key){
	std::string value;
	rdbDB->Get(rocksdb::ReadOptions(), key.toStdString(), &value);
	QString value_ = QString::fromUtf8(value.c_str());
	printf("Get \"%s\" = %s\n", key.toStdString().c_str(), value.c_str());
	return value_;
}

void ignrocksdb::del(QString key){
	rdbDB->Delete(rocksdb::WriteOptions(), key.toStdString());
	printf("Delete key \"%s\"\n", key.toStdString().c_str());
}