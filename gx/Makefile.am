lib_LTLIBRARIES = libgx-@GX_MAJOR_VERSION@.@GX_MINOR_VERSION@.la

EXTENSION_XML = xproto.xml
# FIXME
#EXTENSION_XML += bigreq.xml
#EXTENSION_XML += shape.xml
#EXTENSION_XML += render.xml
## depends on shape + render
#EXTENSION_XML += xfixes.xml
## depends on fixes
#EXTENSION_XML += composite.xml
#EXTENSION_XML += damage.xml
#EXTENSION_XML += dpms.xml
##EXTENSION_XML += glx.xml
##EXTENSION_XML += randr.xml
#EXTENSION_XML += record.xml
#EXTENSION_XML += res.xml
##EXTENSION_XML += screensaver.xml
#EXTENSION_XML += shm.xml
##EXTENSION_XML += sync.xml
#EXTENSION_XML += xc_misc.xml
#EXTENSION_XML += xevie.xml
#EXTENSION_XML += xf86dri.xml
#EXTENSION_XML += xinerama.xml
##EXTENSION_XML += xprint.xml
##EXTENSION_XML += xtest.xml
##EXTENSION_XML += xvmc.xml
##EXTENSION_XML += xv.xml


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
	gx-mask-value-item.c \
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

gxincludedir = \
	$(includedir)/gx-$(GX_MAJOR_VERSION).$(GX_MINOR_VERSION)
gxinternalincludedir = \
	$(includedir)/gx-$(GX_MAJOR_VERSION).$(GX_MINOR_VERSION)/gx

gxinclude_HEADERS = gx.h
gxinternalinclude_HEADERS = \
	gx-types.h \
	gx-mask-value-item.h \
	gx-connection.h \
	gx-connection-gen.h \
	gx-drawable.h \
	gx-drawable-gen.h \
	gx-pixmap.h \
	gx-pixmap-gen.h \
	gx-window.h \
	gx-window-gen.h \
	gx-cookie.h \
	gx-screen.h \
	gx-gcontext.h \
	gx-gcontext-gen.h

$(EXTENSION_XML):
	$(LN_S) -f $(XCBPROTO_XCBINCLUDEDIR)/$@ $@

