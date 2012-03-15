SUBDIRS = \
	libstruct \
	libmisc \
	libcpukernel \
	libcpudisasm \
	libesim \
	libcpuarch \
	libgpuarch \
	libmemsystem \
	libmemvisual \
	libnetwork \
	libgpukernel \
	libmhandle \
	libgpudisasm \
	libgpuvisual

bin_PROGRAMS = m2s

m2s_SOURCES = m2s.c

INCLUDES = -I$(top_srcdir)/src/libmhandle \
	-I$(top_srcdir)/src/libstruct \
	-I$(top_srcdir)/src/libmisc \
	-I$(top_srcdir)/src/libcpukernel \
	-I$(top_srcdir)/src/libcpudisasm \
	-I$(top_srcdir)/src/libesim \
	-I$(top_srcdir)/src/libcpuarch \
	-I$(top_srcdir)/src/libmemsystem \
	-I$(top_srcdir)/src/libnetwork \
	-I$(top_srcdir)/src/libgpuarch \
	-I$(top_srcdir)/src/libgpukernel \
	-I$(top_srcdir)/src/libgpudisasm \
	-I$(top_srcdir)/src/libgpuvisual \
	-I$(top_srcdir)/src/libmemvisual

LDADD = \
	$(top_builddir)/src/libmemvisual/libmemvisual.a \
	$(top_builddir)/src/libgpuvisual/libgpuvisual.a \
	$(top_builddir)/src/libgpukernel/libgpukernel.a \
	$(top_builddir)/src/libcpukernel/libcpukernel.a \
	$(top_builddir)/src/libgpuarch/libgpuarch.a \
	$(top_builddir)/src/libcpuarch/libcpuarch.a \
	$(top_builddir)/src/libgpudisasm/libgpudisasm.a \
	$(top_builddir)/src/libcpudisasm/libcpudisasm.a \
	$(top_builddir)/src/libmemsystem/libmemsystem.a \
	$(top_builddir)/src/libnetwork/libnetwork.a \
	$(top_builddir)/src/libesim/libesim.a \
	$(top_builddir)/src/libstruct/libstruct.a \
	$(top_builddir)/src/libmhandle/libmhandle.a \
	$(top_builddir)/src/libmisc/libmisc.a \
	-lm -lpthread -lz

if HAVE_GTK
LDADD += @GTK_LIBS@
endif
