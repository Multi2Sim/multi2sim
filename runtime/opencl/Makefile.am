if HAVE_M32_FLAG

lib_LTLIBRARIES = $(top_builddir)/lib/libm2s-opencl.la

__top_builddir__lib_libm2s_opencl_la_LDFLAGS = -version-info 0:0:0

__top_builddir__lib_libm2s_opencl_la_SOURCES = \
	\
	clcpu-device.c \
	clcpu-device.h \
	\
	command-queue.c \
	command-queue.h \
	\
	clcpu.h \
	\
	clcpu-execute.c \
	\
	clcpu-program.c \
	clcpu-program.h \
	\
	clrt.c \
	clrt.h \
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
	elf-loader.c \
	\
	event.c \
	event.h \
	\
	fibers.h \
	\
	list.c \
	list.h \
	\
	mhandle.c \
	mhandle.h \
	\
	mkfibers.c \
	\
	program.c \
	program.h \
	\
	kernel.c \
	kernel.h \
	\
	mem.c \
	mem.h \
	\
	object.c \
	object.h \
	\
	sampler.c \
	sampler.h \
	\
	platform.c \
	platform.h \
	\
	thread-list.c \
	thread-list.h


#AM_CFLAGS = -Wall -O2 -fPIC -m32 -DMHANDLE
AM_CFLAGS = -m32

endif

