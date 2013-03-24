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
	elf-format.c \
	elf-format.h \
	\
	list.c \
	list.h \
	\
	mhandle.c \
	mhandle.h \
	\
	context.c \
	context.h \
	\
	device.c \
	device.h \
	\
	function.c \
	function.h \
	\
	function-arg.c \
	function-arg.h \
	\
	memory.c \
	memory.h \
	\
	module.c \
	module.h \
	\
	stream.c \
	stream.h

AM_CFLAGS = -m32

endif

