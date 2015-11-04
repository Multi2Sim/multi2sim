SUBDIRS = \
	lib \
	arch \
	dram \
	memory \
	network \
	visual

bin_PROGRAMS = $(top_builddir)/bin/m2s

__top_builddir__bin_m2s_SOURCES = \
	\
	m2s.cc

AM_LIBTOOLFLAGS = --preserve-dup-deps
AM_CPPFLAGS = @M2S_INCLUDES@

LDADD = \
	$(top_builddir)/src/arch/x86/timing/libtiming.a \
	$(top_builddir)/src/arch/x86/emulator/libemulator.a \
	$(top_builddir)/src/arch/x86/disassembler/libdisassembler.a \
	\
	$(top_builddir)/src/arch/arm/timing/libtiming.a \
	$(top_builddir)/src/arch/arm/emulator/libemulator.a \
	$(top_builddir)/src/arch/arm/disassembler/libdisassembler.a \
	\
	$(top_builddir)/src/arch/hsa/disassembler/libdisassembler.a \
	$(top_builddir)/src/arch/hsa/driver/libdriver.a \
	$(top_builddir)/src/arch/hsa/emulator/libemulator.a \
	\
	$(top_builddir)/src/arch/kepler/driver/libdriver.a \
	$(top_builddir)/src/arch/kepler/emulator/libemulator.a \
	$(top_builddir)/src/arch/kepler/disassembler/libdisassembler.a \
	\
	$(top_builddir)/src/arch/mips/timing/libtiming.a \
	$(top_builddir)/src/arch/mips/emulator/libemulator.a \
	$(top_builddir)/src/arch/mips/disassembler/libdisassembler.a \
	\
	$(top_builddir)/src/arch/southern-islands/timing/libtiming.a \
	$(top_builddir)/src/arch/southern-islands/driver/libdriver.a \
	$(top_builddir)/src/arch/southern-islands/emulator/libemulator.a \
	$(top_builddir)/src/arch/southern-islands/disassembler/libdisassembler.a \
	\
	$(top_builddir)/src/arch/common/libcommon.a \
	\
	$(top_builddir)/src/dram/libdram.a \
	$(top_builddir)/src/memory/libmemory.a \
	$(top_builddir)/src/network/libnetwork.a \
	\
	$(top_builddir)/src/visual/common/libcommon.a \
	$(top_builddir)/src/visual/evergreen/libevergreen.a \
	$(top_builddir)/src/visual/memory/libmemory.a \
	$(top_builddir)/src/visual/network/libnetwork.a \
	$(top_builddir)/src/visual/southern-islands/libsouthernislands.a \
	$(top_builddir)/src/visual/x86/libx86.a \
	$(top_builddir)/src/visual/common/libcommon.a \
	\
	$(top_builddir)/src/lib/esim/libesim.a \
	$(top_builddir)/src/lib/util/libutil.a \
	$(top_builddir)/src/lib/mhandle/libmhandle.a \
	$(top_builddir)/src/lib/cpp/libcpp.a

if ENABLE_DEBUG
AM_LDFLAGS = -rdynamic
endif

if HAVE_GTK
AM_CFLAGS = @GTK_CFLAGS@
LDADD += @GTK_LIBS@
endif

LDADD += -lpthread -lz -lm -lstdc++

