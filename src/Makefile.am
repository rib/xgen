
bin_PROGRAMS = gx

GX_SOURCES = gx.c

#gx_LDADD =
gx_LDFLAGS = -export-dynamic
gx_CFLAGS = @EXTRA_CFLAGS@
gx_CPPFLAGS = \
    -DGX_GLADEDIR=\""$(datadir)/gx/glade/"\" \
    -DGNOME_ICONDIR=\""$(datadir)/pixmaps"\" \
    -DSHARE_DIR=\"$(pkgdatadir)\" \
    -DGX_LOCALEDIR=\""$(gxlocaledir)"\" \
    -DGX_BIN_DIR=\""$(bindir)"\" \
    -I$(top_srcdir) \
    -I$(top_srcdir)/src \
    @EXTRA_CPPFLAGS@

gladedir = $(datadir)/gx/glade/
glade_DATA = \
    data/glade/gx.glade

#uidir = $(datadir)/gx/ui/
#ui_DATA = \
#    data/ui/toolbar-menu-ui.xml

gnomemenudir = $(datadir)/applications
gnomemenu_in_files = data/gx.desktop.in
gnomemenu_DATA = $(gnomemenu_premerge_files:.desktop.in=.desktop)

EXTRA_DIST = \
    $(glade_DATA) \
    $(gnomemenu_DATA)
#    $(ui_DATA)

#@INTLTOOL_DESKTOP_RULE@

CLEANFILES = \
	$(gnomemenu_DATA)

DISTCLEANFILES = \
	$(DISTCLEANFILES)

