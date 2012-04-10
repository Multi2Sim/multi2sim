SUBDIRS = \
	libcpukernel \
	libcpudisasm \
	libcpuarch \
	libgpuarch \
	libmemsystem \
	libvisual \
	libnetwork \
	libgpukernel \
	libgpudisasm \
	libgpuvisual \
	\
	lib

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
	-I$(top_srcdir)/src/libvisual

LDADD = \
	$(top_builddir)/src/libvisual/libvisual.a \
	$(top_builddir)/src/libgpuvisual/libgpuvisual.a \
	$(top_builddir)/src/libgpukernel/libgpukernel.a \
	$(top_builddir)/src/libcpukernel/libcpukernel.a \
	$(top_builddir)/src/libgpuarch/libgpuarch.a \
	$(top_builddir)/src/libcpuarch/libcpuarch.a \
	$(top_builddir)/src/libgpudisasm/libgpudisasm.a \
	$(top_builddir)/src/libcpudisasm/libcpudisasm.a \
	$(top_builddir)/src/libmemsystem/libmemsystem.a \
	$(top_builddir)/src/libnetwork/libnetwork.a \
	\
	$(top_builddir)/src/lib/mhandle/libmhandle.a \
	$(top_builddir)/src/lib/misc/libmisc.a \
	$(top_builddir)/src/lib/esim/libesim.a \
	$(top_builddir)/src/lib/struct/libstruct.a \
	\
	-lm -lpthread -lz

if HAVE_GTK
LDADD += @GTK_LIBS@
endif
