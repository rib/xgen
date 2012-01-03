lib_LTLIBRARIES = libxgen-@XGEN_MAJOR_VERSION@.@XGEN_MINOR_VERSION@.la

libxgen_@XGEN_MAJOR_VERSION@_@XGEN_MINOR_VERSION@_la_SOURCES = xgen.c
#libxgen_@XGEN_MAJOR_VERSION@_@XGEN_MINOR_VERSION@_la_LDADD =
libxgen_@XGEN_MAJOR_VERSION@_@XGEN_MINOR_VERSION@_la_LDFLAGS = \
	@XGEN_DEP_LIBS@ \
  	-version-info $(XGEN_LT_CURRENT):$(XGEN_LT_REVISION):$(XGEN_LT_AGE) \
	-export-dynamic \
	-export-symbols-regex "^xgen_.*"
libxgen_@XGEN_MAJOR_VERSION@_@XGEN_MINOR_VERSION@_la_CFLAGS = \
	@EXTRA_CFLAGS@ \
	@XGEN_DEP_CFLAGS@
libxgen_@XGEN_MAJOR_VERSION@_@XGEN_MINOR_VERSION@_la_CPPFLAGS = \
	@EXTRA_CPPFLAGS@ \
	-I$(top_srcdir)/ \
	-I$(top_srcdir)/src \
	-I$(top_builddir)/src \
	-DXCBPROTO_XCBINCLUDEDIR=\"$(XCBPROTO_XCBINCLUDEDIR)\"

#EXTRA_DIST =
#CLEANFILES =
#DISTCLEANFILES =

xgenincludedir = \
	$(includedir)/xgen-$(XGEN_MAJOR_VERSION).$(XGEN_MINOR_VERSION)
#xgeninternalincludedir = \
#	$(includedir)/xgen-$(XGEN_MAJOR_VERSION).$(XGEN_MINOR_VERSION)/xgen

xgeninclude_HEADERS = xgen.h
#xgeninternalinclude_HEADERS =

