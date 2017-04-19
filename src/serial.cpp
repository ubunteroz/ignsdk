#include "serial.h"

ignserial::ignserial(QObject *parent) : QObject(parent) {}

QVariant ignserial::info() {
  QList<QSerialPortInfo> serialPortList = QSerialPortInfo::availablePorts();
  const QString blank = QString::null;
  QString description, manufacturer, serialNumber;
  bool busy;
  QVariantMap output, info;
  QVariantList list;

  output.insert(QStringLiteral("count"), serialPortList.count());

  if (serialPortList.count() > 0) {
    foreach (const QSerialPortInfo &serial, serialPortList) {
      description =
          (!serial.description().isEmpty() ? serial.description() : blank);
      manufacturer =
          (!serial.manufacturer().isEmpty() ? serial.manufacturer() : blank);
#if QT_VERSION >= 0x050400
      serialNumber =
          (!serial.serialNumber().isEmpty() ? serial.serialNumber() : blank);
#else
      serialNumber = blank;
#endif
      busy = serial.isBusy();
      info.insert(QStringLiteral("port"), serial.portName());
      info.insert(QStringLiteral("location"), serial.systemLocation());
      info.insert(QStringLiteral("vendor"),
                  (serial.hasVendorIdentifier()
                       ? QByteArray::number(serial.hasVendorIdentifier(), 16)
                       : blank));
      info.insert(QStringLiteral("product"),
                  (serial.hasProductIdentifier()
                       ? QByteArray::number(serial.hasProductIdentifier(), 16)
                       : blank));
      info.insert(QStringLiteral("isBusy"), busy);
      info.insert(QStringLiteral("description"), description);
      info.insert(QStringLiteral("manufacturer"), manufacturer);
      info.insert(QStringLiteral("serialNumber"), serialNumber);
      list << info;
    }
    output.insert(QStringLiteral("device"), list);
  }

  QJsonDocument json_enc = QJsonDocument::fromVariant(output);
  return json_enc.toVariant();
}

void ignserial::Read(const QVariant &config) {
  QVariantMap configuration = jsonParse->jsonParser(config).toVariantMap();
  QString port = configuration[QStringLiteral("port")].toString();
  int brt =
      (configuration[QStringLiteral("baudRate")].toInt() ? configuration[QStringLiteral("baudRate")].toInt()
                                         : QSerialPort::Baud9600);

  if (!port.isEmpty()) {
    serialPort.setPortName(port);
  } else {
    qDebug() << "Port name is null";
  }

  serialPort.setBaudRate(brt);

  if (serialPort.open(QIODevice::ReadOnly)) {
    connect(&serialPort, &QIODevice::readyRead, this, &ignserial::readOut);
    connect(&serialPort, SIGNAL(error(QSerialPort::SerialPortError)), this,
            SLOT(readOut()));
  } else {
    qDebug() << "Cannot read from device";
  }
}

void ignserial::readOut() {
  QByteArray readData;
  readData.append(serialPort.readAll());
  QTextStream readStream(readData);
  QString readOutData(readStream.readAll());
  emit out(readOutData, serialPort.errorString());
}
