noinst_PROGRAMS = test-gx
# rendertest

test_gx_SOURCES = \
	test-gx-main.c \
	test-gx-common.c \
	test-gx-common.h \
	\
	test-connection.c \
	test-reply.c \
	test-async-reply.c \
	test-cookie-life-cycle.c \
	test-gerrors.c \
	test-screen-info.c

#rendertest_SOURCES = rendertest.c

# For convenience, this provides a way to easily run individual unit tests:
.PHONY: wrappers
wrappers: test-gx
	for i in `./test-gx -l`; \
	do \
		ln -sf $(top_srcdir)/tests/wrapper.sh "`basename $$i`_wrap.sh"; \
	done
# NB: BUILT_SOURCES here a misnomer. We aren't building source, just inserting
# a phony rule that will generate symlink scripts for running individual tests
BUILT_SOURCES = wrappers

test_gx_CFLAGS = \
	-I$(top_srcdir)/ \
	-I$(top_srcdir)/gx \
	-I$(top_builddir)/gx \
	@EXTRA_CFLAGS@ \
	@GX_DEP_CFLAGS@
test_gx_LDADD = @GX_DEP_LIBS@ $(top_builddir)/gx/libgx-@GX_MAJOR_VERSION@.@GX_MINOR_VERSION@.la

#rendertest_CFLAGS = \
#	-I$(top_srcdir)/ \
#	-I$(top_srcdir)/gx \
#	-I$(top_builddir)/gx \
#	@EXTRA_CFLAGS@ \
#	@GX_DEP_CFLAGS@
#rendertest_LDADD = @GX_DEP_LIBS@ $(top_builddir)/gx/libgx-@GX_MAJOR_VERSION@.@GX_MINOR_VERSION@.la

.PHONY: test test-report
test:
	gtester -o=test-gx-results.xml ./test-gx

test-report:
	gtester -o=test-gx-results.xml -k ./test-gx \
	  && gtester-report test-gx-results.xml > test-gx-results.html \
	  && gnome-open ./test-gx-results.html

full-report:
	gtester -o=test-gx-results.xml -k -m=slow ./test-gx \
	  && gtester-report test-gx-results.xml > test-gx-results.html \
	  && gnome-open ./test-gx-results.html

EXTRA_DIST = ADDING_NEW_TESTS

clean-local:
	rm -f *_wrap.sh

