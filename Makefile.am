SUBDIRS = src pyext
AUTOMAKE_OPTIONS = foreign subdir-objects
ACLOCAL_AMFLAGS = -I m4

include aminclude.am

MOSTLYCLEANFILES = $(DX_CLEANFILES)

EXTRA_DIST = $(DX_CONFIG)

doc: doxygen-doc

install-html:
	$(MAKE) doc
	@$(NORMAL_INSTALL)
	$(MKDIR_P) $(DESTDIR)$(docdir)
	$(INSTALL_DATA) $(DX_DOCDIR)/html/* $(DESTDIR)$(docdir)

uninstall-local:
	@$(NORMAL_UNINSTALL)
	echo "removing $(DESTDIR)$(docdir)"
	rm -fr $(DESTDIR)$(docdir)


