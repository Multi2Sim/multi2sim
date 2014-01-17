if HAVE_M32_FLAG

lib_LTLIBRARIES = $(top_builddir)/lib/libm2s-opencl.la

__top_builddir__lib_libm2s_opencl_la_LDFLAGS = -version-info 1:0:0 -lpthread -ldl -lrt

__top_builddir__lib_libm2s_opencl_la_SOURCES = \
	\
	command.c \
	command.h \
	\
	command-queue.c \
	command-queue.h \
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
	event.h \
	\
	evg-device.c \
	evg-device.h \
	\
	evg-kernel.c \
	evg-kernel.h \
	\
	evg-program.c \
	evg-program.h \
	\
	list.c \
	list.h \
	\
	mhandle.c \
	mhandle.h \
	\
	misc.c \
	misc.h \
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
	opencl.c \
	opencl.h \
	\
	platform.c \
	platform.h \
	\
	sampler.c \
	sampler.h \
	\
	si-device.c \
	si-device.h \
	\
	si-kernel.c \
	si-kernel.h \
	\
	si-program.c \
	si-program.h \
	\
	string.c \
	string.h \
	\
	thread-list.c \
	thread-list.h \
	\
	x86-device.c \
	x86-device.h \
	\
	x86-kernel.c \
	x86-kernel.h \
	\
	x86-program.c \
	x86-program.h \
	\
	union-device.c \
	union-device.h \
	\
	union-kernel.c \
	union-kernel.h \
	\
	union-program.c \
	union-program.h \
	\
	partition-strategy.c \
	partition-strategy.h \
	\
	partition-util.c \
	partition-util.h \
	\
	partition-util-time.c \
	partition-util-time.h \
	\
	even-partition-strategy.c \
	even-partition-strategy.h \
	\
	relative-runtime-partition-strategy.c \
	relative-runtime-partition-strategy.h \
	\
	x86-device-wi.s \
	\
	throughput-overhead-partition-strategy.c \
	throughput-overhead-partition-strategy.h \
	\
	block-by-block-partition-strategy.c \
	block-by-block-partition-strategy.h \
	\
	numa-partition-strategy.c \
	numa-partition-strategy.h \
	\
	first-done-partition-strategy.c \
	first-done-partition-strategy.h \
	\
	same-latency-partition-strategy.c \
	same-latency-partition-strategy.h \
	\
	partition-issue-log.c \
	partition-issue-log.h

AM_CFLAGS = -m32 -D_GNU_SOURCE
AM_CCASFLAGS = -m32

endif

