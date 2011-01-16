#!/bin/bash


# Check for $ATISTREAMSDKROOT
if [ -z "$ATISTREAMSDKROOT" ]; then
	echo "error: environment variable 'ATISTREAMSDKROOT' not defined. Is the ATI SDK installed?" >&2
	exit 1
fi
if [ ! -e "$ATISTREAMSDKROOT/include/CL/cl.h" ]; then
	echo "error: cannot find \$ATISTREAMSDKROOT/include/CL/cl.h" >&2
	exit 1
fi


# Compile

CC="gcc"
CFLAGS="-Wall -I$ATISTREAMSDKROOT/include -g -fPIC"

$CC $CFLAGS -c guest_opencl.c -o guest_opencl.o
$CC $CFLAGS guest_opencl.o -shared -o libguest_opencl.so

