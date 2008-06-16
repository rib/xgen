noinst_PROGRAMS = test-connection test-events

INCLUDES = -I$(top_srcdir)/ -I$(top_srcdir)/gx -I$(top_builddir)/gx
LDADD = @GX_DEP_LIBS@ $(top_builddir)/gx/libgx-@GX_MAJOR_VERSION@.@GX_MINOR_VERSION@.la
AM_CFLAGS = @EXTRA_CFLAGS@ @GX_DEP_CFLAGS@
#AM_LDFLAGS =

test_connection_SOURCES = test-connection.c
test_events_SOURCES = test-events.c

