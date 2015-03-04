#-------------------------------------------------
#
# Project created by QtCreator 2013-02-03T23:13:36
#
#-------------------------------------------------

QT       += network core webkitwidgets sql printsupport

TARGET = ignsdk
TEMPLATE = app
CONFIG += qt c++11 silent

DEFINES *= _FORTIFY_SOURCE=2
QMAKE_CFLAGS_RELEASE -= -O2
QMAKE_CFLAGS_RELEASE += -O3 -Wformat -Wformat-security -fstack-protector
QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -O3 -Wformat -Wformat-security -fstack-protector
QMAKE_LFLAGS_RELEASE -= -Wl,-O1
QMAKE_LFLAGS_RELEASE += -Wl,-O3 -Wl,-z,relro -Wl,-z,now -pie

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
    src/network.cpp \
    src/process.cpp

HEADERS  += src/ignsdk.h \
    src/filesystem.h \
    src/download.h \
    src/sql.h \
    src/system.h \
    src/movedrag.h \
    src/network.h \
    src/json.h \
    src/process.h \
    src/version.h

RESOURCES += \
    src/ignsdk.qrc

macx{
ICON += icon/ignsdk-logo.icns
}

OTHER_FILES += \
    bar-descriptor.xml
