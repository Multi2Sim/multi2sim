lib_LIBRARIES = libgpukernel.a
libgpukernel_a_SOURCES = \
	cal-abi.c \
	gpuisa.c \
	gpukernel.c \
	gpukernel.h \
	gpukernel-local.h \
	gpumachine.c \
	opencl.dat \
	opencl.c \
	opencl-obj.c

AM_CFLAGS = -Wall -fno-strict-aliasing -DMHANDLE

INCLUDES = -I$(top_srcdir)/src/libstruct \
	-I$(top_srcdir)/src/libmhandle \
	-I$(top_srcdir)/src/libopt \
	-I$(top_srcdir)/src/libmisc \
	-I$(top_srcdir)/src/libdisasm \
	-I$(top_srcdir)/src/libgpudisasm \
	-I$(top_srcdir)/src/libgpukernel \
	-I$(top_srcdir)/src/libm2skernel


