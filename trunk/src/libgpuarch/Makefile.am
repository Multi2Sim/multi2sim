lib_LIBRARIES = libgpuarch.a
libgpuarch_a_SOURCES = \
	device.c \
	gpuarch.h \
	gpuarch.c \
	gpupipeline.c

AM_CFLAGS = -Wall -fno-strict-aliasing -DMHANDLE

INCLUDES = -I$(top_srcdir)/src/libstruct \
	-I$(top_srcdir)/src/libmhandle \
	-I$(top_srcdir)/src/libopt \
	-I$(top_srcdir)/src/libmisc \
	-I$(top_srcdir)/src/libgpudisasm \
	-I$(top_srcdir)/src/libgpukernel

