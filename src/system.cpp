#include "system.h"
#include <QDebug>

ignsystem::ignsystem(QObject *parent): QObject(parent), jsonParse(0){}

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

QString ignsystem::hashFile(const QString &path, QString hash_algo){
    bool isValid = true;
    QCryptographicHash::Algorithm algo;

    if (hash_algo == "md4"){
        algo = QCryptographicHash::Md4;
    } else if (hash_algo == "md5"){
        algo = QCryptographicHash::Md5;
    } else if (hash_algo == "sha1"){
        algo = QCryptographicHash::Sha1;
    } else if (hash_algo == "sha224"){
        algo = QCryptographicHash::Sha224;
    } else if (hash_algo == "sha256"){
        algo = QCryptographicHash::Sha256;
    } else if (hash_algo == "sha384"){
        algo = QCryptographicHash::Sha384;
    } else if (hash_algo == "sha512"){
        algo = QCryptographicHash::Sha512;
    } else if (hash_algo == "sha3-224"){
        algo = QCryptographicHash::Sha3_224;
    } else if (hash_algo == "sha3-256"){
        algo = QCryptographicHash::Sha3_256;
    } else if (hash_algo == "sha3-384"){
        algo = QCryptographicHash::Sha3_384;
    } else if (hash_algo == "sha3-512"){
        algo = QCryptographicHash::Sha3_512;
    } else {
        isValid = false;
    }

    if (isValid && QFile::exists(path) && QFileInfo(path).isFile()){
        QCryptographicHash crypto(algo);
        QFile file(path);
        file.open(QFile::ReadOnly);

        while (!file.atEnd()){
            crypto.addData(file.read(512));
        }

        file.close();
        QByteArray hash = crypto.result();
        return hash.toHex();
    } else {
        return NULL;
    }
}

void ignsystem::desktopService(const QString &link){
    QDesktopServices::openUrl(QUrl(link));
}

int ignsystem::exec(const QString &command){
    process = new QProcess(this);
    process->setReadChannelMode(QProcess::MergedChannels);
    process->start(command);
    connect(process, SIGNAL(readyReadStandardOutput()), this, SLOT( _out()));
    return process->pid();
}

void ignsystem::_out(){
    emit out(process->readAllStandardOutput());
}

void ignsystem::kill(){
    process->kill();
}

bool ignsystem::print(const QVariant &config){
    QVariantMap configuration = jsonParse->jsonParser(config).toVariantMap();
    QPrinter printer;
    QTextDocument *document = new QTextDocument();
    QPrintDialog *printDialog = new QPrintDialog(&printer);

    QString type = configuration["type"].toString();
    QString content = configuration["content"].toString();
    QString output = configuration["out"].toString();

    if (type == "html"){
        document->setHtml(content);
    } else {
        document->setPlainText(content);
    }

    if (output == "pdf"){
        printer.setOutputFormat(QPrinter::PdfFormat);
    }

    if (printDialog->exec() == QDialog::Accepted){
        document->print(&printer);
        delete document;
        return true;
    } else {
        return false;
    }
}