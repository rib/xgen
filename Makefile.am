SUBDIRS = tools gx tests

if BUILD_GTK_DOC
SUBDIRS += doc
endif

pcfiles = gx-@GX_MAJOR_VERSION@.@GX_MINOR_VERSION@.pc

pkgconfig_DATA = $(pcfiles)
pkgconfigdir   = $(libdir)/pkgconfig


#SUBDIRS += po

#EXTRA_DIST = \
#	intltool-extract.in \
#	intltool-merge.in \
#	intltool-update.in \
#	config.rpath \
#	m4/ChangeLog


#DISTCLEANFILES = \
#        intltool-extract \
#        intltool-merge \
#        intltool-update

