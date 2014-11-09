#ifndef IGN_ROCKSDB
#define IGN_ROCKSDB

#include <QObject>
#include <string>
#include "rocksdb/db.h"
#include "rocksdb/options.h"

class ignrocksdb : public QObject {
	Q_OBJECT
public:
	explicit ignrocksdb(QObject* parent = 0);
public slots:
	void init(QString wal_dir);
	void close();
	bool put(QString key, QString value);
	void bput(QString key, QString value);
	void bdel(QString key);
	bool bwrite();
	QString get(QString key);
	void del(QString key);
};

#endif