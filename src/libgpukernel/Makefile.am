lib_LIBRARIES = libgpukernel.a
libgpukernel_a_SOURCES = \
	gpukernel.c \
	gpukernel.h \
	gpukernel-local.h \
	opencl.dat \
	opencl.c \
	opencl-obj.c

AM_CFLAGS = -Wall -fno-strict-aliasing

INCLUDES = -I$(top_srcdir)/src/libstruct \
	-I$(top_srcdir)/src/libmhandle \
	-I$(top_srcdir)/src/libopt \
	-I$(top_srcdir)/src/libmisc \
	-I$(top_srcdir)/src/libdisasm \
	-I$(top_srcdir)/src/libgpukernel \
	-I$(top_srcdir)/src/libm2skernel


