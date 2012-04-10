SUBDIRS = \
	libgpuarch \
	libmemsystem \
	libvisual \
	libnetwork \
	libgpukernel \
	libgpudisasm \
	libgpuvisual \
	\
	arch \
	lib

bin_PROGRAMS = m2s

m2s_SOURCES = m2s.c

INCLUDES = @M2S_INCLUDES@

LDADD = \
	$(top_builddir)/src/libvisual/libvisual.a \
	$(top_builddir)/src/libgpuvisual/libgpuvisual.a \
	$(top_builddir)/src/libgpukernel/libgpukernel.a \
	$(top_builddir)/src/libgpuarch/libgpuarch.a \
	$(top_builddir)/src/libgpudisasm/libgpudisasm.a \
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
