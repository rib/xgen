noinst_PROGRAMS = test-xgen
# rendertest

test_xgen_SOURCES = \
	test-xgen-main.c \
	test-xgen-common.c \
	test-xgen-common.h

#rendertest_SOURCES = rendertest.c

# For convenience, this provides a way to easily run individual unit tests:
.PHONY: wrappers
wrappers: test-xgen
	for i in `./test-xgen -l`; \
	do \
		ln -sf $(top_srcdir)/tests/wrapper.sh "`basename $$i`_wrap.sh"; \
	done
# NB: BUILT_SOURCES here a misnomer. We aren't building source, just inserting
# a phony rule that will generate symlink scripts for running individual tests
BUILT_SOURCES = wrappers

test_xgen_CFLAGS = \
	-I$(top_srcdir)/ \
	-I$(top_srcdir)/xgen \
	-I$(top_builddir)/xgen \
	@EXTRA_CFLAGS@ \
	@XGEN_DEP_CFLAGS@
test_xgen_LDADD = @XGEN_DEP_LIBS@ $(top_builddir)/xgen/libxgen-@XGEN_MAJOR_VERSION@.@XGEN_MINOR_VERSION@.la

#rendertest_CFLAGS = \
#	-I$(top_srcdir)/ \
#	-I$(top_srcdir)/xgen \
#	-I$(top_builddir)/xgen \
#	@EXTRA_CFLAGS@ \
#	@XGEN_DEP_CFLAGS@
#rendertest_LDADD = @XGEN_DEP_LIBS@ $(top_builddir)/xgen/libxgen-@XGEN_MAJOR_VERSION@.@XGEN_MINOR_VERSION@.la

.PHONY: test test-report
test:
	gtester -o=test-xgen-results.xml ./test-xgen

test-report:
	gtester -o=test-xgen-results.xml -k ./test-xgen \
	  && gtester-report test-xgen-results.xml > test-xgen-results.html \
	  && gnome-open ./test-xgen-results.html

full-report:
	gtester -o=test-xgen-results.xml -k -m=slow ./test-xgen \
	  && gtester-report test-xgen-results.xml > test-xgen-results.html \
	  && gnome-open ./test-xgen-results.html

EXTRA_DIST = ADDING_NEW_TESTS

clean-local:
	rm -f *_wrap.sh

