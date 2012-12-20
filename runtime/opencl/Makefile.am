if HAVE_M32_FLAG

lib_LTLIBRARIES = $(top_builddir)/lib/libm2s-opencl.la

__top_builddir__lib_libm2s_opencl_la_LDFLAGS = -version-info 0:0:0

__top_builddir__lib_libm2s_opencl_la_SOURCES = \
	\
	clcpu-device.c \
	clcpu-device.h \
	\
	command-queue.c \
	\
	context.c \
	context.h \
	\
	debug.c \
	debug.h \
	\
	device.c \
	device.h \
	\
	elf-format.c \
	elf-format.h \
	\
	event.c \
	\
	clrt.c \
	clrt.h \
	\
	mkfibers.c \
	program.c \
	\
	clcpu.h \
	clcpu-execute.c \
	\
	kernel.c \
	mem.c \
	\
	object.c \
	object.h \
	\
	sampler.c \
	\
	clcpu-program.c \
	clcpu-program.h \
	\
	elf-loader.c \
	\
	list.c \
	list.h \
	\
	mhandle.c \
	mhandle.h \
	\
	platform.c \
	\
	thread-list.c \
	thread-list.h \
	\
	fibers.h


#AM_CFLAGS = -Wall -O2 -fPIC -m32 -DMHANDLE
AM_CFLAGS = -m32

endif

