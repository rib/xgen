noinst_PROGRAMS = \
	test-connection \
	test-reply \
	test-async-reply \
	test-events \
	test-cookie-life-cycle

INCLUDES = -I$(top_srcdir)/ -I$(top_srcdir)/gx -I$(top_builddir)/gx
LDADD = @GX_DEP_LIBS@ $(top_builddir)/gx/libgx-@GX_MAJOR_VERSION@.@GX_MINOR_VERSION@.la
AM_CFLAGS = @EXTRA_CFLAGS@ @GX_DEP_CFLAGS@
#AM_LDFLAGS =

test_connection_SOURCES = test-connection.c
test_reply_SOURCES = test-reply.c
test_async_reply_SOURCES = test-async-reply.c
test_events_SOURCES = test-events.c
test_cookie_life_cycle_SOURCES = test-cookie-life-cycle.c

