#include "serial.h"

ignserial::ignserial(QObject *parent): QObject(parent){}

QVariant ignserial::info(){
    QList<QSerialPortInfo> serialPortList = QSerialPortInfo::availablePorts();
    const QString blank = QString::null;
    QString description, manufacturer, serialNumber;
    bool busy;
    QVariantMap output, info;
    QVariantList list;

    output.insert("count",serialPortList.count());

    if (serialPortList.count() > 0){
        foreach (const QSerialPortInfo &serial, serialPortList){
            description = (!serial.description().isEmpty() ? serial.description() : blank);
            manufacturer = (!serial.manufacturer().isEmpty() ? serial.manufacturer() : blank);
            serialNumber = (!serial.serialNumber().isEmpty() ? serial.serialNumber() : blank);
            busy = serial.isBusy();
            info.insert("port", serial.portName());
            info.insert("location", serial.systemLocation());
            info.insert("vendor", (serial.hasVendorIdentifier() ? QByteArray::number(serial.hasVendorIdentifier(), 16) : blank));
            info.insert("product", (serial.hasProductIdentifier() ? QByteArray::number(serial.hasProductIdentifier(), 16) : blank));
            info.insert("isBusy", busy);
            info.insert("description", description);
            info.insert("manufacturer", manufacturer);
            info.insert("serialNumber", serialNumber);
            list << info;
        }
        output.insert("device", list);
    }

    QJsonDocument json_enc = QJsonDocument::fromVariant(output);
    return json_enc.toVariant();
}

void ignserial::Read(const QVariant &config){
    QVariantMap configuration = jsonParse->jsonParser(config).toVariantMap();
    QString port = configuration["port"].toString();
    int brt = (configuration["baudRate"].toInt() ? configuration["baudRate"].toInt() : QSerialPort::Baud9600);

    if (!port.isEmpty()){
        serialPort.setPortName(port);
    } else {
        qDebug() << "Port name is null";
    }

    serialPort.setBaudRate(brt);

    if (serialPort.open(QIODevice::ReadOnly)){
        connect(&serialPort, SIGNAL(readyRead()), this, SLOT(readOut()));
        connect(&serialPort, SIGNAL(error(QSerialPort::SerialPortError)), this, SLOT(readOut()));
    } else {
        qDebug() << "Cannot read from device";
    }
}

void ignserial::readOut(){
    QByteArray readData;
    readData.append(serialPort.readAll());
    QTextStream readStream(readData);
    QString readOutData(readStream.readAll());
    emit out(readOutData, serialPort.errorString());
}