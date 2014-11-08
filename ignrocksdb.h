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
	void init();
	void close();
	bool put(QString key, QString value);
	QString get(QString key);
	void del(QString key);
};

#endif