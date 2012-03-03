lib_LIBRARIES = libmemsystem.a

libmemsystem_a_SOURCES = \
	cache.c \
	config.c \
	cpu-coherence.c \
	directory.c \
	gpu-coherence.c \
	mem-system.c \
	mem-system.h \
	mmu.c \
	module.c

# FIXME: remove libgpuarch and libgpukernel

INCLUDES = -I$(top_srcdir)/src/libstruct \
	-I$(top_srcdir)/src/libcpuarch \
	-I$(top_srcdir)/src/libcpukernel \
	-I$(top_srcdir)/src/libcpudisasm \
	\
	-I$(top_srcdir)/src/libgpuarch \
	-I$(top_srcdir)/src/libgpukernel \
	-I$(top_srcdir)/src/libgpudisasm \
	\
	-I$(top_srcdir)/src/libmhandle \
	-I$(top_srcdir)/src/libnetwork \
	-I$(top_srcdir)/src/libopt \
	-I$(top_srcdir)/src/libesim \
	-I$(top_srcdir)/src/libmisc

