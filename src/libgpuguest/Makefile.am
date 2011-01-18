lib_LIBRARIES = libgpuguest.a

libgpuguest_a_SOURCES = \
	gpuguest.c \
	gpuguest.h

libgpuguest_a_LIBADD = \
	libguest_opencl_so.o

AM_CFLAGS = -Wall -fno-strict-aliasing

libguest_opencl_so.o: guest_code/guest_opencl.c
	cd guest_code && CC=$(CC) AR=$(AR) $(SHELL) guest_opencl_make.sh
	$(LD) -r -b binary -o libguest_opencl_so.o guest_code/libguest_opencl.so

INCLUDES = -I$(top_srcdir)/src/libstruct \
	-I$(top_srcdir)/src/libmhandle \
	-I$(top_srcdir)/src/libmisc

EXTRA_DIST = \
	guest_code/guest_opencl_make.sh \
	guest_code/guest_opencl.c

CLEANFILES = \
	guest_code/libguest_opencl.a \
	guest_code/libguest_opencl.so \
	guest_code/guest_opencl.o

