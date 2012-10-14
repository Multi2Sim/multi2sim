SUBDIRS = \
	arch \
	lib \
	mem-system \
	network \
	visual

bin_PROGRAMS = m2s

m2s_SOURCES = m2s.c

INCLUDES = @M2S_INCLUDES@

LDADD = \
	$(top_builddir)/src/arch/arm/timing/libtiming.a \
	$(top_builddir)/src/arch/arm/emu/libemu.a \
	$(top_builddir)/src/arch/arm/asm/libasm.a \
	\
	$(top_builddir)/src/arch/evergreen/timing/libtiming.a \
	$(top_builddir)/src/arch/evergreen/emu/libemu.a \
	$(top_builddir)/src/arch/evergreen/asm/libasm.a \
	\
	$(top_builddir)/src/arch/southern-islands/asm/libasm.a \
	\
	$(top_builddir)/src/arch/x86/timing/libtiming.a \
	$(top_builddir)/src/arch/x86/emu/libemu.a \
	$(top_builddir)/src/arch/x86/asm/libasm.a \
	\
	$(top_builddir)/src/arch/southern-islands/timing/libtiming.a \
	$(top_builddir)/src/arch/southern-islands/emu/libemu.a \
	$(top_builddir)/src/arch/southern-islands/asm/libasm.a \
	\
	$(top_builddir)/src/mem-system/libmemsystem.a \
	$(top_builddir)/src/network/libnetwork.a \
	\
	$(top_builddir)/src/visual/common/libcommon.a \
	$(top_builddir)/src/visual/evergreen/libevergreen.a \
	$(top_builddir)/src/visual/memory/libmemory.a \
	$(top_builddir)/src/visual/southern-islands/libsouthernislands.a \
	$(top_builddir)/src/visual/x86/libx86.a \
	$(top_builddir)/src/visual/common/libcommon.a \
	\
	$(top_builddir)/src/arch/fermi/timing/libtiming.a \
	$(top_builddir)/src/arch/fermi/emu/libemu.a \
	$(top_builddir)/src/arch/fermi/asm/libasm.a \
	\
	$(top_builddir)/src/lib/mhandle/libmhandle.a \
	$(top_builddir)/src/lib/esim/libesim.a \
	$(top_builddir)/src/lib/util/libutil.a

if HAVE_GTK
AM_CFLAGS = @GTK_CFLAGS@
LDADD += @GTK_LIBS@
endif

