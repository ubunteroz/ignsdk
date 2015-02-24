#-------------------------------------------------
#
# Project created by QtCreator 2013-02-03T23:13:36
#
#-------------------------------------------------

QT       += network core webkitwidgets sql printsupport

TARGET = ignsdk
TEMPLATE = app
CONFIG += qt c++11 silent
OBJECTS_DIR = build/
MOC_DIR = build/
RCC_DIR = build/
DESTDIR = bin/

SOURCES += src/main.cpp \
    src/ignsdk.cpp \
    src/filesystem.cpp \
    src/download.cpp \
    src/sql.cpp \
    src/system.cpp \
    src/movedrag.cpp \
    src/network.cpp

HEADERS  += src/ignsdk.h \
    src/filesystem.h \
    src/download.h \
    src/sql.h \
    src/system.h \
    src/movedrag.h \
    src/network.h \
    src/version.h

RESOURCES += \
    src/ignsdk.qrc

macx{
ICON += icon/ignsdk-logo.icns
}

OTHER_FILES += \
    bar-descriptor.xml
