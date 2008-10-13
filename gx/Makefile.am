lib_LTLIBRARIES = libgx-@GX_MAJOR_VERSION@.@GX_MINOR_VERSION@.la

EXTENSION_XML = xproto.xml
# FIXME
#EXTENSION_XML += bigreq.xml
if BUILD_SHAPE
EXTENSION_XML += shape.xml
endif
if BUILD_RENDER
EXTENSION_XML += render.xml
endif
if BUILD_XFIXES
# depends on shape + render
EXTENSION_XML += xfixes.xml
endif
# depends on fixes
if BUILD_COMPOSITE
EXTENSION_XML += composite.xml
endif
if BUILD_DAMAGE
EXTENSION_XML += damage.xml
endif
if BUILD_DPMS
EXTENSION_XML += dpms.xml
endif
##EXTENSION_XML += glx.xml
if BUILD_RANDR
EXTENSION_XML += randr.xml
endif
if BUILD_RECORD
EXTENSION_XML += record.xml
endif
if BUILD_RES
EXTENSION_XML += res.xml
endif
if BUILD_SCREENSAVER
EXTENSION_XML += screensaver.xml
endif
if BUILD_SHM
EXTENSION_XML += shm.xml
endif
if BUILD_SYNC
EXTENSION_XML += sync.xml
endif
#EXTENSION_XML += xc_misc.xml
if BUILD_XEVIE
EXTENSION_XML += xevie.xml
endif
#EXTENSION_XML += xf86dri.xml
#EXTENSION_XML += xinerama.xml
#EXTENSION_XML += xprint.xml
#EXTENSION_XML += xtest.xml
#EXTENSION_XML += xv.xml
#Depends on xv
#EXTENSION_XML += xvmc.xml

# Generate this list via:
# find ./ -iname 'gx-*-gen.*' |cut -d'/' -f2|xargs printf '\t%s \\\n'
GENERATED_CODE = \
        gx-window-gen.c \
        gx-protocol-error-codes-gen.h \
        gx-gcontext-gen.c \
        gx-xcb-dependencies-gen.h \
        gx-connection-gen.c \
        gx-protocol-error-details-gen.h \
        gx-pixmap-gen.c \
        gx-connection-gen.h \
        gx-drawable-gen.c \
        gx-pixmap-gen.h \
        gx-drawable-gen.h \
        gx-window-gen.h \
        gx-gcontext-gen.h \
        gx-tests-gen.c \
        gx-cookie-gen.h

$(GENERATED_CODE): $(EXTENSION_XML) $(top_builddir)/tools/gx-gen
	echo "generating $@"
	$(top_builddir)/tools/gx-gen $(EXTENSION_XML)
	indent $(GENERATED_CODE)
BUILT_SOURCES = $(GENERATED_CODE)

#.PHONY: generated-code
#BUILT_SOURCES=generated-code
##BUILT_SOURCES=gx-connection-gen.c
##gx-connection-gen.c: $(top_builddir)/tools/gx-gen
#generated-code: $(EXTENSION_XML)
#	$(top_builddir)/tools/gx-gen $(EXTENSION_XML)
##	$(top_srcdir)/tools/post_process.sh

#libgx_@GX_MAJOR_VERSION@_@GX_MINOR_VERSION@_la_SOURCES = \
#gx-connection.c gx-drawable.c gx-pixmap.c gx-window.c
libgx_@GX_MAJOR_VERSION@_@GX_MINOR_VERSION@_la_SOURCES = \
	gx-main.c \
	gx-mask-value-item.c \
	gx-connection.c \
	gx-connection.h \
	gx-drawable.c \
	gx-drawable.h \
	gx-window.c \
	gx-window.h \
	gx-pixmap.c \
	gx-pixmap.h \
	gx-gcontext.c \
	gx-gcontext.h \
	gx-screen.c \
	gx-screen.h \
	gx-cookie.c \
	gx-cookie.h \
	gx-protocol-error.c \
	gx-protocol-error.h

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
	gx-main.h \
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

