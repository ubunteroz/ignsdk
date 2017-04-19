#-------------------------------------------------
#
# Project created by QtCreator 2013-02-03T23:13:36
#
#-------------------------------------------------

QT += core gui widgets network webkitwidgets sql printsupport serialport

TARGET = ignsdk
TEMPLATE = app
CONFIG += silent c++11
GIT_VERSION = $$system(git --git-dir $$PWD/.git --work-tree $$PWD describe --always --long)
DEFINES += GIT_VERSION=\\\"$$GIT_VERSION\\\"

OBJECTS_DIR = build/
MOC_DIR = build/
RCC_DIR = build/
DESTDIR = bin/

SOURCES += src/main.cpp \
    src/ignsdk.cpp \
    src/filesystem.cpp \
    src/download.cpp \
    src/sql.cpp \
    src/serial.cpp \
    src/system.cpp \
    src/network.cpp \
    src/process.cpp

HEADERS  += src/ignsdk.h \
    src/filesystem.h \
    src/download.h \
    src/sql.h \
    src/serial.h \
    src/system.h \
    src/network.h \
    src/json.h \
    src/process.h \
    src/version.h

RESOURCES += src/ignsdk.qrc
