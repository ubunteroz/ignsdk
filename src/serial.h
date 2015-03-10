#ifndef IGNSERIAL_H
#define IGNSERIAL_H

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTextStream>
#include <QList>
#include <QTimer>
#include "json.h"

class ignserial: public QObject
{
    Q_OBJECT

public:
    explicit ignserial(QObject *parent = 0);
    ignjson *jsonParse;
    QSerialPort serialPort;
    QTimer timer;

public slots:
    QVariant info();
    void Read(const QVariant &config);
    void readOut();

signals:
	void out(const QString &output, const QString &error);
};

#endif // IGNSERIAL_H