lib_LIBRARIES = libm2skernel.a
libm2skernel_a_SOURCES = m2skernel.h \
	isa.c \
	context.c \
	elf.c \
	fs.c \
	loader.c \
	m2skernel.c \
	machine.c \
	machine-ctrl.c \
	machine-fp.c \
	machine-rot.c \
	machine-std.c \
	machine-str.c \
	memory.c \
	regs.c \
	signal.c \
	syscall.c \
	syscall.dat
AM_CFLAGS = -Wall -fno-strict-aliasing

INCLUDES = -I$(top_srcdir)/src/libstruct \
	-I$(top_srcdir)/src/libmhandle \
	-I$(top_srcdir)/src/libopt \
	-I$(top_srcdir)/src/libmisc \
	-I$(top_srcdir)/src/libdisasm \
	-I$(top_srcdir)/src/libgpukernel

