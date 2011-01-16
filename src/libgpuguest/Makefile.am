lib_LIBRARIES = libgpuguest.a

libgpuguest_a_SOURCES = \
	gpuguest.c \
	gpuguest.h

libgpuguest_a_LIBADD = \
	libguest_opencl_so.o

AM_CFLAGS = -Wall -fno-strict-aliasing

EXTRA_DIST = guest_code/libguest_opencl.so

libguest_opencl_so.o: guest_code/libguest_opencl.so
	$(LD) -r -b binary -o libguest_opencl_so.o guest_code/libguest_opencl.so

INCLUDES = -I$(top_srcdir)/src/libstruct \
	-I$(top_srcdir)/src/libmhandle \
	-I$(top_srcdir)/src/libmisc

