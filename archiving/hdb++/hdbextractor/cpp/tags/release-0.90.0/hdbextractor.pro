#-------------------------------------------------
#
# Project created by QtCreator 2014-03-06T10:53:55
#
#-------------------------------------------------

QT       -= core gui

INSTALL_ROOT = /runtime
LIB_DIR = $${INSTALL_ROOT}/lib
INC_DIR = $${INSTALL_ROOT}/include/hdbextractor
DOC_DIR = $${INSTALL_ROOT}/share/doc/hdbextractor

CONFIG += debug

TARGET = hdbextractor++

VERSION_HEX = 0x010000
VERSION = 1.0.0
VER_MAJ = 1
VER_MIN = 0
VER_FIX = 0

unix:INCLUDEPATH += mysql/src db/src db src ../src db/src/helpers utils utils/private

DEFINES += HDBEXTRACTOR_VERSION_STR=\"\\\"$${VERSION}\\\"\" \
    HDBEXTRACTOR_VERSION=$${VERSION_HEX} \
    VER_MAJ=$${VER_MAJ} \
    VER_MIN=$${VER_MIN} \
    VER_FIX=$${VER_FIX} \
    HDBEXTRACTORLIB_PRINTINFO

TEMPLATE = lib

DEFINES += HDBEXTRACTOR_LIBRARY

SOURCES += src/hdbextractor.cpp \
    src/hdbextractorprivate.cpp \
    mysql/src/mysqlconnection.cpp \
    db/src/row.cpp \
    mysql/src/mysqlresult.cpp \
    mysql/src/mysqlrow.cpp \
    db/src/result.cpp \
    db/src/xvariant.cpp \
    hdb/src/mysqlhdbschema.cpp \
    db/src/xvariantlist.cpp \
    hdbpp/src/mysqlhdbppschema.cpp \
    src/configurationparser.cpp \
    src/queryconfiguration.cpp \
    db/src/configurabledbschema.cpp \
    db/src/helpers/configurabledbschemahelper.cpp \
    utils/datasiever.cpp \
    utils/private/datasieverprivate.cpp \
    utils/xvariantprinter.cpp \
    utils/datasieverprogresslistener.cpp \
    db/src/timeinterval.cpp \
    db/src/datetimeutils.cpp \
    db/src/xvariantprivate.cpp

HEADERS += src/hdbextractor.h\
    src/hdbextractorprivate.h \
    src/hdbxmacros.h \
    db/src/connection.h \
    mysql/src/mysqlconnection.h \
    db/src/row.h \
    mysql/src/mysqlresult.h \
    db/src/result.h \
    mysql/src/mysqlrow.h \
    db/src/xvariant.h \
    db/src/dbschema.h \
    db/src/dbschemaprivate.h \
    hdb/src/mysqlhdbschema.h \
    db/src/xvariantlist.h \
    db/src/xvariantprivate.h \
    db/src/resultlistenerinterface.h \
    src/hdbextractorlistener.h \
    hdbpp/src/mysqlhdbppschema.h \
    src/configurationparser.h \
    src/queryconfiguration.h \
    db/src/configurabledbschema.h \
    db/src/helpers/configurabledbschemahelper.h \
    utils/datasiever.h \
    utils/private/datasieverprivate.h \
    utils/xvariantprinter.h \
    utils/datasieverprogresslistener.h \
    db/src/timeinterval.h \
    db/src/datetimeutils.h


lib.path = $${INSTALL_ROOT}/lib

LIBTARGET = lib$${TARGET}

lib.files = $${LIBTARGET}.so.$${VERSION}
lib.commands =  ln \
    -sf \
    $${LIBTARGET}.so.$${VERSION} \
    $${LIB_DIR}/$${LIBTARGET}.so.$${VER_MAJ} \
    && \
    ln \
    -sf \
    $${LIBTARGET}.so.$${VER_MAJ} \
    $${LIB_DIR}/$${LIBTARGET}.so

inc.files = $${HEADERS}
inc.path = $${INC_DIR}


doc.commands = doxygen \
    Doxyfile;
doc.files = doc/
doc.path = $${DOC_DIR}

LIBS += -lmysqlclient -lpthread

INSTALLS += lib inc doc

