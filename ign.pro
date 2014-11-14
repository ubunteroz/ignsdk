#-------------------------------------------------
#
# Project created by QtCreator 2013-02-03T23:13:36
#
#-------------------------------------------------

QT       += network core webkitwidgets sql

TARGET = ignsdk
TEMPLATE = app
CONFIG += qt c++11

# RocksDB
INCLUDEPATH += external/rocksdb/include/
rocksdb.commands = make -C external/rocksdb/ static_lib
QMAKE_EXTRA_TARGETS += rocksdb
PRE_TARGETDEPS += rocksdb
LIBS += external/rocksdb/librocksdb.a -lz -lbz2 -lsnappy

SOURCES += main.cpp\
        ign.cpp \
    fs.cpp \
    igndownload.cpp \
    ignsql.cpp \
    ignsystem.cpp \
    ignmovedrag.cpp \
    ignnetwork.cpp \
    ignrocksdb.cpp

HEADERS  += ign.h \
    fs.h \
    igndownload.h \
    ignsql.h \
    ignsystem.h \
    ignmovedrag.h \
    ignnetwork.h \
    ignrocksdb.h

RESOURCES += \
    ign.qrc

macx{
ICON += icon/ignsdk-logo.icns
}

OTHER_FILES += \
    bar-descriptor.xml
