SUBDIRS = xgen tests

if BUILD_GTK_DOC
SUBDIRS += doc
endif

pcfiles = xgen-@XGEN_MAJOR_VERSION@.@XGEN_MINOR_VERSION@.pc

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

