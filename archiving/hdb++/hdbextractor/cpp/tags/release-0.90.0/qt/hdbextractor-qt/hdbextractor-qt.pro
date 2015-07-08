#-------------------------------------------------
#
# Project created by QtCreator 2014-08-08T14:06:59
#
#-------------------------------------------------


INSTALL_ROOT = /runtime
LIB_DIR = $${INSTALL_ROOT}/lib
INC_DIR = $${INSTALL_ROOT}/include/hdbextractor/qt
DOC_DIR = $${INSTALL_ROOT}/share/doc/hdbextractor/qt

contains(QT_VERSION, ^5\\..*\\..*) {
    VER_SUFFIX = -qt5
} else {
    VER_SUFFIX =
}

CONFIG += debug

QT       -= gui

TARGET = hdbextractor-qt$${VER_SUFFIX}
TEMPLATE = lib

OBJECTS_DIR = obj

QMAKE_CLEAN += libhdbextractor-qt* core*

VERSION_HEX = 0x010000
VERSION = 1.0.0
VER_MAJ = 1
VER_MIN = 0
VER_FIX = 0

DEFINES += HDBEXTRACTORQT_LIBRARY

DEFINES -= QT_NO_DEBUG_OUTPUT

SOURCES += \
    src/qhdbextractorthread.cpp \
    src/qhdbxevent.cpp \
    src/qhdbxqueryevent.cpp \
    src/qhdbxconnectionevent.cpp \
    src/qhdbextractorproxy.cpp \
    src/qhdbnewdataevent.cpp \
    src/qhdbxutils.cpp \
    src/qhdbxerrorevent.cpp \
    src/qhdbxsourceslistqueryevent.cpp \
    src/qhdbsourceslistreadyevent.cpp \
    src/qhdbxerrorqueryevent.cpp \
    src/qhdbnewerrordataevent.cpp \
    src/qhdbxutilsprivate.cpp \
    src/qhdbdataboundaries.cpp

HEADERS +=\
        src/hdbextractor-qt_global.h \
    src/qhdbextractorthread.h \
    src/qhdbxevent.h \
    src/qhdbxqueryevent.h \
    src/qhdbxconnectionevent.h \
    src/qhdbextractorproxy.h \
    src/qhdbextractorproxyprivate.h \
    src/qhdbnewdataevent.h \
    src/qhdbxutils.h \
    src/qhdbxerrorevent.h \
    src/qhdbxsourceslistqueryevent.h \
    src/qhdbsourceslistreadyevent.h \
    src/qhdbxerrorqueryevent.h \
    src/qhdbnewerrordataevent.h \
    src/qhdbxutilsprivate.h \
    src/qhdbdataboundaries.h

INCLUDEPATH += src/ ../../src ../../db/src ../../hdb/src ../../mysql/src

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}

lib.path = $${INSTALL_ROOT}/lib

lib.files = lib$${TARGET}.so.$${VERSION}

lib.commands =  ln \
    -sf \
    lib$${TARGET}.so.$${VERSION} \
    $${LIB_DIR}/lib$${TARGET}.so.$${VER_MAJ} \
    && \
    ln \
    -sf \
    lib$${TARGET}.so.$${VER_MAJ} \
    $${LIB_DIR}/lib$${TARGET}.so

inc.files = $${HEADERS}
inc.path = $${INC_DIR}


doc.commands = doxygen \
    Doxyfile;
doc.files = doc/
doc.path = $${DOC_DIR}

LIBS += -L../..  -lmysqlclient -lpthread -lhdbextractor++

INSTALLS += lib inc doc

