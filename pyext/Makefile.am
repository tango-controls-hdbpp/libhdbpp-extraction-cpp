BUILT_SOURCES = swig_wrap.cpp
# SWIG_SOURCES = datasieverprogresslistener.i hdbextractor.i hdbxsettings.i resultlistenerinterface.i xvariantdatainfo.i xvariantlist.i datasiever.i datetimeutils.i hdbextractorlistener.i  mysqlconnection.i timeinterval.i xvariant.i xvariantprinter.i

SWIG_SOURCES =  hdbextractor_global.i 

INCLUDEDIRS = -I$(top_srcdir)/src -I$(top_srcdir)/src/db \
		      -I$(top_srcdir)/src/hdb -I$(top_srcdir)/src/hdbpp -I$(top_srcdir)/src/utils \
		      -I$(top_srcdir)/src/mysql -I$(top_srcdir)/src/db/helpers


pkgpython_PYTHON = hdbextractor.py 
pkgpyexec_LTLIBRARIES = _hdbextractor.la

_hdbextractor_la_SOURCES = swig_wrap.cpp $(SWIG_SOURCES)
_hdbextractor_la_CPPFLAGS = $(AX_SWIG_PYTHON_CPPFLAGS) $(INCLUDEDIRS)
_hdbextractor_la_LDFLAGS = -module -classic -DUSE_TCL_STUBS 
_hdbextractor_la_LIBADD = ../src/libhdbextractor++.la

swig_wrap.cpp : $(SWIG_SOURCES)
	$(SWIG) $(SWIG_PYTHON_OPT) $(AX_SWIG_PYTHON_OPT) $(INCLUDEDIRS) -o $@ $<

# hdbextractor.py: swig_wrap.cpp
#	$(MAKE) $(AM_MAKEFLAGS)  $(BUILT_SOURCES)
