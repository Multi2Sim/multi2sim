SUBDIRS = \
	libmhandle \
	libstruct \
	libopt \
	libmisc \
	libm2skernel \
	libdisasm \
	libesim \
	libnetwork \
	libcachesystem \
	libgpukernel \
	libgpudisasm

bin_PROGRAMS = \
	m2s-fast \
	m2s-objdump \
	m2s

m2s_fast_SOURCES = m2s-fast.c

m2s_objdump_SOURCES = m2s-objdump.c

m2s_SOURCES = \
	bpred.c \
	commit.c \
	decode.c \
	dispatch.c \
	fetch.c \
	fu.c \
	issue.c \
	m2s.c \
	m2s.h \
	processor.c \
	queues.c \
	recover.c \
	rf.c \
	rob.c \
	sched.c \
	tcache.c \
	uop.c \
	uop1.dat \
	uop2.dat \
	writeback.c

INCLUDES = -I$(top_srcdir)/src/libmhandle \
	-I$(top_srcdir)/src/libstruct \
	-I$(top_srcdir)/src/libopt \
	-I$(top_srcdir)/src/libmisc \
	-I$(top_srcdir)/src/libm2skernel \
	-I$(top_srcdir)/src/libdisasm \
	-I$(top_srcdir)/src/libesim \
	-I$(top_srcdir)/src/libnetwork \
	-I$(top_srcdir)/src/libcachesystem \
	-I$(top_srcdir)/src/libgpukernel \
	-I$(top_srcdir)/src/libgpudisasm

LDADD = \
	$(top_builddir)/src/libm2skernel/libm2skernel.a \
	$(top_builddir)/src/libdisasm/libdisasm.a \
        $(top_builddir)/src/libopt/libopt.a \
	$(top_builddir)/src/libmisc/libmisc.a \
	$(top_builddir)/src/libcachesystem/libcachesystem.a \
	$(top_builddir)/src/libgpukernel/libgpukernel.a \
	$(top_builddir)/src/libgpudisasm/libgpudisasm.a \
	$(top_builddir)/src/libnetwork/libnetwork.a \
	$(top_builddir)/src/libesim/libesim.a \
	$(top_builddir)/src/libstruct/libstruct.a \
	$(top_builddir)/src/libmhandle/libmhandle.a \
	-lm -lpthread

AM_CFLAGS = -Wall -fno-strict-aliasing
AM_LDFLAGS = #-static

