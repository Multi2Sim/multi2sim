lib_LIBRARIES = libcpukernel.a

libcpukernel_a_SOURCES = \
	cpukernel.h \
	isa.c \
	context.c \
	cpukernel.c \
	file.c \
	loader.c \
	machine.c \
	machine-ctrl.c \
	machine-fp.c \
	machine-rot.c \
	machine-std.c \
	machine-str.c \
	machine-xmm.c \
	memory.c \
	regs.c \
	signal.c \
	spec-mem.c \
	syscall.c \
	syscall.dat \
	uinst.c

INCLUDES = -I$(top_srcdir)/src/libstruct \
	-I$(top_srcdir)/src/libmhandle \
	-I$(top_srcdir)/src/libesim \
	-I$(top_srcdir)/src/libmisc \
	-I$(top_srcdir)/src/libcpudisasm \
	-I$(top_srcdir)/src/libgpukernel \
	-I$(top_srcdir)/src/libgpudisasm

