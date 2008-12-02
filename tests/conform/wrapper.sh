#!/bin/sh

if ! test -f ./test-xgen; then
    echo "You need to build test-xgen before running a wrapper script"
    exit 1
fi

UNIT_TEST_WRAP=`basename $0`
UNIT_TEST=`echo $UNIT_TEST_WRAP|sed 's/_wrap.sh//g'`
UNIT_TEST_PATH=`./test-xgen -l |grep $UNIT_TEST`

echo "Running: gtester -p $UNIT_TEST_PATH ./test-xgen"
echo ""
gtester -p $UNIT_TEST_PATH ./test-xgen

echo ""
echo "NOTE: For debugging purposes, you can run this single test as follows:"
echo "$ env LD_LIBRARY_PATH=../xgen/.libs \\"
echo "     gdb --eval-command=\"b $UNIT_TEST\" --args \\"
echo "        .libs/test-xgen -p $UNIT_TEST_PATH"

