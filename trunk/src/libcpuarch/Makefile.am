lib_LIBRARIES = libcpuarch.a

libcpuarch_a_SOURCES = \
	cpuarch.h \
	bpred.c \
	commit.c \
	decode.c \
	dispatch.c \
	fetch.c \
	fu.c \
	issue.c \
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

AM_CFLAGS = -Wall -fno-strict-aliasing

INCLUDES = -I$(top_srcdir)/src/libstruct \
	-I$(top_srcdir)/src/libmhandle \
	-I$(top_srcdir)/src/libopt \
	-I$(top_srcdir)/src/libmisc \
	-I$(top_srcdir)/src/libcpudisasm \
	-I$(top_srcdir)/src/libcachesystem \
	-I$(top_srcdir)/src/libnetwork \
	-I$(top_srcdir)/src/libesim \
	-I$(top_srcdir)/src/libcpukernel \
	-I$(top_srcdir)/src/libgpukernel \
	-I$(top_srcdir)/src/libgpudisasm

