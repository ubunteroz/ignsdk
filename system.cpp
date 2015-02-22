#include "system.h"
#include <QDebug>

ignsystem::ignsystem(QObject *parent): QObject(parent){}

QString ignsystem::cliOut(const QString& command){
    QProcess process;
    process.setProcessChannelMode(QProcess::MergedChannels);
    process.start(command);
    int pid = process.pid();
    qDebug() << "Executing process with PID" << pid;
    process.waitForFinished(-1);
    return process.readAllStandardOutput();
}

QString ignsystem::hash(const QString &data, QString hash_func){
    bool isValid = true;
    QByteArray hash;
    QByteArray byteArray = data.toUtf8();

    if (hash_func == "md4"){
        hash = QCryptographicHash::hash(byteArray, QCryptographicHash::Md4);
    } else if (hash_func == "md5"){
        hash = QCryptographicHash::hash(byteArray, QCryptographicHash::Md5);
    } else if (hash_func == "sha1"){
        hash = QCryptographicHash::hash(byteArray, QCryptographicHash::Sha1);
    } else if (hash_func == "sha224"){
        hash = QCryptographicHash::hash(byteArray, QCryptographicHash::Sha224);
    } else if (hash_func == "sha256"){
        hash = QCryptographicHash::hash(byteArray, QCryptographicHash::Sha256);
    } else if (hash_func == "sha384"){
        hash = QCryptographicHash::hash(byteArray, QCryptographicHash::Sha384);
    } else if (hash_func == "sha512"){
        hash = QCryptographicHash::hash(byteArray, QCryptographicHash::Sha512);
    } else if (hash_func == "sha3-224"){
        hash = QCryptographicHash::hash(byteArray, QCryptographicHash::Sha3_224);
    } else if (hash_func == "sha3-256"){
        hash = QCryptographicHash::hash(byteArray, QCryptographicHash::Sha3_256);
    } else if (hash_func == "sha3-384"){
        hash = QCryptographicHash::hash(byteArray, QCryptographicHash::Sha3_384);
    } else if (hash_func == "sha3-512"){
        hash = QCryptographicHash::hash(byteArray, QCryptographicHash::Sha3_512);
    } else {
        isValid = false;
    }

    if (isValid){
        return hash.toHex();
    } else {
        qDebug() << "Unknown hash algorithm:" << hash_func;
        return NULL;
    }
}

void ignsystem::desktopService(const QString &link){
    QDesktopServices::openUrl(QUrl(link));
}

void ignsystem::exec(const QString &command){
    process = new QProcess(this);
    process->setReadChannelMode(QProcess::MergedChannels);
    process->start(command);
    connect(process, SIGNAL(readyReadStandardOutput()), this, SLOT( _out()));
}

void ignsystem::_out(){
    emit out(process->readAllStandardOutput());
}

void ignsystem::kill(){
    process->kill();
}