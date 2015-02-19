#-------------------------------------------------
#
# Project created by QtCreator 2013-02-03T23:13:36
#
#-------------------------------------------------

QT       += network core webkitwidgets sql printsupport

TARGET = ignsdk
TEMPLATE = app
CONFIG += qt c++11 silent

SOURCES += main.cpp \
    ignsdk.cpp \
    filesystem.cpp \
    download.cpp \
    sql.cpp \
    system.cpp \
    movedrag.cpp \
    network.cpp

HEADERS  += ignsdk.h \
    filesystem.h \
    download.h \
    sql.h \
    system.h \
    movedrag.h \
    network.h

RESOURCES += \
    ignsdk.qrc

macx{
ICON += icon/ignsdk-logo.icns
}

OTHER_FILES += \
    bar-descriptor.xml
