lib_LTLIBRARIES = libgx-@GX_MAJOR_VERSION@.@GX_MINOR_VERSION@.la

EXTENSION_XML = xproto.xml
EXTENSION_XML += shape.xml
EXTENSION_XML += render.xml
# depends on shape + render
EXTENSION_XML += xfixes.xml
# depends on fixes
EXTENSION_XML += composite.xml


.PHONY: generated-code
BUILT_SOURCES=generated-code
#BUILT_SOURCES=gx-connection-gen.c
#gx-connection-gen.c: $(top_builddir)/tools/gx-gen
generated-code: $(EXTENSION_XML)
	$(top_builddir)/tools/gx-gen $(EXTENSION_XML)
#	$(top_srcdir)/tools/post_process.sh

#libgx_@GX_MAJOR_VERSION@_@GX_MINOR_VERSION@_la_SOURCES = \
#gx-connection.c gx-drawable.c gx-pixmap.c gx-window.c
libgx_@GX_MAJOR_VERSION@_@GX_MINOR_VERSION@_la_SOURCES = \
	gx-connection.c \
	gx-connection.h \
	gx-drawable.c \
	gx-drawable.h \
	gx-window.c \
	gx-window.h \
	gx-gcontext.c \
	gx-gcontext.h \
	gx-screen.c \
	gx-screen.h

#libgx_@GX_MAJOR_VERSION@_@GX_MINOR_VERSION@_la_LDADD =
libgx_@GX_MAJOR_VERSION@_@GX_MINOR_VERSION@_la_LDFLAGS = \
	@GX_DEP_LIBS@ \
  	-version-info $(GX_LT_CURRENT):$(GX_LT_REVISION):$(GX_LT_AGE) \
	-export-dynamic \
	-export-symbols-regex "^gx_.*"
libgx_@GX_MAJOR_VERSION@_@GX_MINOR_VERSION@_la_CFLAGS = \
	@EXTRA_CFLAGS@ @GX_DEP_CFLAGS@
libgx_@GX_MAJOR_VERSION@_@GX_MINOR_VERSION@_la_CPPFLAGS = \
	@EXTRA_CPPFLAGS@ \
	-I$(top_srcdir)/ \
	-I$(top_srcdir)/src \
	-I$(top_builddir)/src


#EXTRA_DIST =

#CLEANFILES =

#DISTCLEANFILES =


$(EXTENSION_XML):
	$(LN_S) -f $(XCBPROTO_XCBINCLUDEDIR)/$@ $@

