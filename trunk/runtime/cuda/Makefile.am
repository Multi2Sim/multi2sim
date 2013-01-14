if HAVE_M32_FLAG

lib_LTLIBRARIES = $(top_builddir)/lib/libm2s-cuda.la

__top_builddir__lib_libm2s_cuda_la_LDFLAGS = -version-info 1:0:0

__top_builddir__lib_libm2s_cuda_la_SOURCES = \
	\
	api.c \
	api.h \
	api-rt.c \
	\
	debug.c \
	debug.h \
	\
	linked-list.c \
	linked-list.h \
	\
	mhandle.c \
	mhandle.h

AM_CFLAGS = -Wall -O2 -fPIC -m32

endif

