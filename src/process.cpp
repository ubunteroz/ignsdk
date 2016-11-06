#include "process.h"

ignprocess::ignprocess(QObject *parent) : QObject(parent) {}

void ignprocess::exec(const QString &command) {
  process.setReadChannelMode(QProcess::MergedChannels);
  connect(&process, SIGNAL(readyReadStandardOutput()), this, SLOT(_out()));
  process.start(command);
}

void ignprocess::_out() { emit this->out(process.readAllStandardOutput()); }

void ignprocess::kill() { this->process.kill(); }
