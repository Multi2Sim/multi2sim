lib_LIBRARIES = libcpukernel.a

libcpukernel_a_SOURCES = \
	cpukernel.h \
	isa.c \
	context.c \
	cpukernel.c \
	elf.c \
	fs.c \
	loader.c \
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

if DEBUG
AM_CFLAGS = -Wall -fno-strict-aliasing -DMHANDLE -g -O0
else
AM_CFLAGS = -fno-strict-aliasing -O3
endif

INCLUDES = -I$(top_srcdir)/src/libstruct \
	-I$(top_srcdir)/src/libmhandle \
	-I$(top_srcdir)/src/libopt \
	-I$(top_srcdir)/src/libmisc \
	-I$(top_srcdir)/src/libcpudisasm \
	-I$(top_srcdir)/src/libgpukernel \
	-I$(top_srcdir)/src/libgpudisasm

