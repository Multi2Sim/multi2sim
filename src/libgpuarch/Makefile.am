lib_LIBRARIES = libgpuarch.a
libgpuarch_a_SOURCES = \
	coalesce.c \
	gpuarch.h \
	gpuarch.c \
	gpucache.c \
	\
	stg_schedule.c \
	stg_fetch.c \
	stg_decode.c \
	stg_read.c \
	stg_execute.c \
	stg_write.c

if DEBUG
AM_CFLAGS = -Wall -fno-strict-aliasing -DMHANDLE -g -O0
else
AM_CFLAGS = -fno-strict-aliasing -O3
endif

INCLUDES = -I$(top_srcdir)/src/libstruct \
	-I$(top_srcdir)/src/libmhandle \
	-I$(top_srcdir)/src/libopt \
	-I$(top_srcdir)/src/libmisc \
	-I$(top_srcdir)/src/libgpudisasm \
	-I$(top_srcdir)/src/libgpukernel \
	-I$(top_srcdir)/src/libnetwork \
	-I$(top_srcdir)/src/libesim

