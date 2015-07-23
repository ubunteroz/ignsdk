#-------------------------------------------------
#
# Project created by QtCreator 2013-02-03T23:13:36
#
#-------------------------------------------------

QT       += network core webkitwidgets sql printsupport serialport

TARGET = ignsdk
TEMPLATE = app
CONFIG += qt c++11 silent

DEFINES *= _FORTIFY_SOURCE=2
QMAKE_CFLAGS_RELEASE -= -O2
QMAKE_CFLAGS_RELEASE += -O3 -Wformat -Wformat-security -fstack-protector
QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -O3 -Wformat -Wformat-security -fstack-protector
QMAKE_LFLAGS_RELEASE -= -Wl,-O1
QMAKE_LFLAGS_RELEASE += -Wl,-O3 -Wl,-z,relro -Wl,-z,now

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
    src/process.cpp \
    src/serial.cpp \
    src/vfs.cpp \
    src/vfsprotocol.cpp

HEADERS  += src/ignsdk.h \
    src/filesystem.h \
    src/download.h \
    src/sql.h \
    src/system.h \
    src/movedrag.h \
    src/network.h \
    src/json.h \
    src/process.h \
    src/serial.h \
    src/vfs.h \
    src/vfsprotocol.h \
    src/version.h

# TTVFS
miniz.commands = $$QMAKE_CC $$QMAKE_CFLAGS_RELEASE -fPIE -c -o build/miniz.o src/external/ttvfs/ttvfs/miniz.c
QMAKE_EXTRA_TARGETS += miniz
PRE_TARGETDEPS += miniz
LIBS += build/miniz.o

SOURCES += src/external/ttvfs/ttvfs/VFSAtomic.cpp \
    src/external/ttvfs/ttvfs/VFSBase.cpp \
    src/external/ttvfs/ttvfs/VFSDir.cpp \
    src/external/ttvfs/ttvfs/VFSFile.cpp \
    src/external/ttvfs/ttvfs/VFSFileFuncs.cpp \
    src/external/ttvfs/ttvfs/VFSHelper.cpp \
    src/external/ttvfs/ttvfs/VFSLoader.cpp \
    src/external/ttvfs/ttvfs/VFSSystemPaths.cpp \
    src/external/ttvfs/ttvfs/VFSTools.cpp \
    src/external/ttvfs/ttvfs/VFSDirZip.cpp \
    src/external/ttvfs/ttvfs/VFSFileZip.cpp \
    src/external/ttvfs/ttvfs/VFSZipArchiveLoader.cpp

HEADERS += src/external/ttvfs/ttvfs/VFS.h \
    src/external/ttvfs/ttvfs/VFSArchiveLoader.h \
    src/external/ttvfs/ttvfs/VFSAtomic.h \
    src/external/ttvfs/ttvfs/VFSBase.h \
    src/external/ttvfs/ttvfs/VFSDefines.h \
    src/external/ttvfs/ttvfs/VFSDir.h \
    src/external/ttvfs/ttvfs/VFSFile.h \
    src/external/ttvfs/ttvfs/VFSFileFuncs.h \
    src/external/ttvfs/ttvfs/VFSHashmap.h \
    src/external/ttvfs/ttvfs/VFSHelper.h \
    src/external/ttvfs/ttvfs/VFSInternal.h \
    src/external/ttvfs/ttvfs/VFSLoader.h \
    src/external/ttvfs/ttvfs/VFSSelfRefCounter.h \
    src/external/ttvfs/ttvfs/VFSSystemPaths.h \
    src/external/ttvfs/ttvfs/VFSTools.h \
    src/external/ttvfs/ttvfs/VFSDirZip.h \
    src/external/ttvfs/ttvfs/VFSFileZip.h \
    src/external/ttvfs/ttvfs/VFSZipArchiveLoader.h

# LVPA
lvpa.commands = cd src/external/liblvpa/ \
    && cd build \
    && cmake --clean-first \
    -DLVPA_ENABLE_ZLIB=TRUE -DLVPA_USE_INTERNAL_ZLIB=TRUE \
    -DLVPA_ENABLE_LZMA=TRUE -DLVPA_USE_INTERNAL_LZMA=TRUE \
    -DLVPA_ENABLE_LZO=TRUE -DLVPA_USE_INTERNAL_LZO=TRUE \
    -DLVPA_ENABLE_LZF=TRUE -DLVPA_USE_INTERNAL_LZF=TRUE \
    -DLVPA_ENABLE_LZHAM=TRUE -DLVPA_USE_INTERNAL_LZHAM=TRUE \
    -DLVPA_BUILD_TTVFS_BINDINGS=TRUE -DLVPA_BUILD_TESTPROG=FALSE \
    -DTTVFS_INCLUDE_DIRS:PATH=../../ttvfs/ttvfs/ \
    -DCMAKE_CXX_FLAGS_RELEASE="-O3 -fPIE -fstack-protector" \
    -DCMAKE_C_FLAGS_RELEASE="-O3 -fPIE -fstack-protector" \
    .. \
    && make
QMAKE_EXTRA_TARGETS += lvpa
PRE_TARGETDEPS += lvpa
LIBS+=src/external/liblvpa/build/lvpa_ttvfs/liblvpa_ttvfs.a \
    src/external/liblvpa/build/lvpa/liblvpa.a \
    src/external/liblvpa/build/lzf/liblzf.a \
    src/external/liblvpa/build/lzham/liblzham.a \
    src/external/liblvpa/build/lzma/liblzma.a \
    src/external/liblvpa/build/lzo/liblzo.a \
    src/external/liblvpa/build/zlib/libzlib.a
INCLUDEPATH+=src/external/ttvfs/ttvfs/

RESOURCES += \
    src/ignsdk.qrc

macx{
ICON += icon/ignsdk-logo.icns
}

OTHER_FILES += \
    bar-descriptor.xml
