lib_LTLIBRARIES = libhdbextractor++.la

ACLOCAL_AMFLAGS = ${ACLOCAL_FLAGS}

AUTOMAKE_OPTIONS = foreign subdir-objects

libhdbextractor___la_SOURCES = hdbextractor.cpp \
    hdbextractorprivate.cpp \
    mysql/mysqlconnection.cpp \
    db/row.cpp \
    mysql/mysqlresult.cpp \
    mysql/mysqlrow.cpp \
    db/result.cpp \
    db/xvariant.cpp \
    hdb/mysqlhdbschema.cpp \
    db/xvariantlist.cpp \
    hdbpp/mysqlhdbppschema.cpp \
    configurationparser.cpp \
    hdbxsettings.cpp \
    db/configurabledbschema.cpp \
    db/helpers/configurabledbschemahelper.cpp \
    utils/datasiever.cpp \
    utils/private/datasieverprivate.cpp \
    utils/xvariantprinter.cpp \
    utils/datasieverprogresslistener.cpp \
    db/timeinterval.cpp \
    db/datetimeutils.cpp \
    db/xvariantprivate.cpp \
    hdbextractor.h \
    hdbextractorlistener.h \
    db/resultlistenerinterface.h \
    hdbextractorprivate.h \
    hdbxmacros.h \
    mysql/mysqlconnection.h \
    db/connection.h \
    db/dbschema.h \
    db/row.h \
    mysql/mysqlresult.h \
    mysql/mysqlrow.h \
    db/result.h \
    db/xvariant.h \
    hdb/mysqlhdbschema.h \
    db/dbschemaprivate.h \
    utils/private/datasieverprivate.h \
    db/xvariantlist.h \
    hdbpp/mysqlhdbppschema.h \
    configurationparser.h \
    hdbxsettings.h \
    db/configurabledbschema.h \
    db/helpers/configurabledbschemahelper.h \
    utils/datasiever.h \
    db/xvariantprivate.h \
    utils/xvariantprinter.h \
    utils/datasieverprogresslistener.h \
    db/timeinterval.h \
    db/datetimeutils.h \ \
    db/xvariantdatainfo.cpp \
    sharedpointer.cpp \
    test/xvariantstatstest.cpp

# Header files that must be installed are specified by the HEADERS family of variables.
# Headers can be installed in
# includedir, oldincludedir, pkgincludedir or any other directory you may have defined
#
include_HEADERS = hdbextractor.h \
hdbextractorlistener.h \
db/resultlistenerinterface.h \
hdbxmacros.h \
mysql/mysqlconnection.h \
db/connection.h \
db/row.h \
mysql/mysqlresult.h \
mysql/mysqlrow.h \
db/result.h \
db/xvariant.h \
hdb/mysqlhdbschema.h \
db/xvariantlist.h \
hdbpp/mysqlhdbppschema.h \
configurationparser.h \
hdbxsettings.h \
db/configurabledbschema.h \
db/helpers/configurabledbschemahelper.h \
utils/datasiever.h \
utils/xvariantprinter.h \
utils/datasieverprogresslistener.h \
db/timeinterval.h \
db/datetimeutils.h \
sharedpointer.h \
test/xvariantstatstest.h \
db/xvariantdatainfo.h 


libhdbextractor___la_CXXFLAGS = -I$(top_srcdir)/src -I$(top_srcdir)/src/db \
    -I$(top_srcdir)/src/mysql -I$(top_srcdir)/src/hdbpp \
    -I$(top_srcdir)/src/utils -I$(top_srcdir)/src/utils/private \
    -I$(top_srcdir)/src/.. -I$(top_srcdir)/src/db/helpers

# libhdbextractor___la_LDFLAGS = -version-info $(HDBX_VERSION)

# Use revision instead of version info
libhdbextractor___la_LDFLAGS = -release $(HDBX_REVISION)
