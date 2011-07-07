SUBDIRS = \
	libmhandle \
	libstruct \
	libmisc \
	libcpukernel \
	libcpudisasm \
	libesim \
	libcpuarch \
	libgpuarch \
	libnetwork \
	libcachesystem \
	libgpukernel \
	libgpudisasm

bin_PROGRAMS = m2s

m2s_SOURCES = m2s.c

INCLUDES = -I$(top_srcdir)/src/libmhandle \
	-I$(top_srcdir)/src/libstruct \
	-I$(top_srcdir)/src/libmisc \
	-I$(top_srcdir)/src/libcpukernel \
	-I$(top_srcdir)/src/libcpudisasm \
	-I$(top_srcdir)/src/libesim \
	-I$(top_srcdir)/src/libcpuarch \
	-I$(top_srcdir)/src/libnetwork \
	-I$(top_srcdir)/src/libcachesystem \
	-I$(top_srcdir)/src/libgpuarch \
	-I$(top_srcdir)/src/libgpukernel \
	-I$(top_srcdir)/src/libgpudisasm

LDADD = \
	$(top_builddir)/src/libgpukernel/libgpukernel.a \
	$(top_builddir)/src/libcpukernel/libcpukernel.a \
	$(top_builddir)/src/libgpuarch/libgpuarch.a \
	$(top_builddir)/src/libcpuarch/libcpuarch.a \
	$(top_builddir)/src/libcachesystem/libcachesystem.a \
	$(top_builddir)/src/libgpudisasm/libgpudisasm.a \
	$(top_builddir)/src/libcpudisasm/libcpudisasm.a \
	$(top_builddir)/src/libnetwork/libnetwork.a \
	$(top_builddir)/src/libesim/libesim.a \
	$(top_builddir)/src/libstruct/libstruct.a \
	$(top_builddir)/src/libmhandle/libmhandle.a \
	$(top_builddir)/src/libmisc/libmisc.a \
	-lm -lpthread

if DEBUG
AM_CFLAGS = -Wall -fno-strict-aliasing -DMHANDLE -g -O0
else
AM_CFLAGS = -fno-strict-aliasing -O3
endif

