#ifndef IGNPROCESS_H
#define IGNPROCESS_H

#include <QObject>
#include <QProcess>

class ignprocess: public QObject{
    Q_OBJECT

public:
    explicit ignprocess(QObject *parent = 0);
    QProcess process;

signals:
    void out(const QString& link);

public slots:
    void exec(const QString& command);
    void _out();
    void kill();
};

#endif // IGNPROCESS_H