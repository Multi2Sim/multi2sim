#!/bin/bash

# Error message
function error() {
	echo -e "\tCompilation of the guest OpenCL library has failed. This library is\n" \
		"\tused by Multi2Sim to provide support for 32-bit OpenCL programs. If\n" \
		"\tyour host machine is 64-bit, please make sure that the packages needed\n" \
		"\tto compile in 32-bit are installed. A 'gcc -m32' command line should\n" \
		"\tbe supported then. If this is not your problem, please email\n" \
		"\t'development@multi2sim.org' and report this error.\n" \
		>&2
	exit 1
}

# Message
echo "Executing script to generate guest OpenCL library code (CC='$CC', AR='$AR')"

# Compile object file
CFLAGS="-Wall -m32 -g -fPIC"
$CC $CFLAGS -c guest_opencl.c -o guest_opencl.o || error
echo "  * guest_opencl.o:      OpenCL implementation object file generated"

# Compile dynamic library
$CC $CFLAGS guest_opencl.o -shared -o libguest_opencl.so || error
echo "  * libguest_opencl.so:  Dynamic version of the guest OpenCL library generated"

# Compile static library
$AR cru libguest_opencl.a guest_opencl.o || error
echo "  * libguest_opencl.a:   Static version of the guest OpenCL library generated"

