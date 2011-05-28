lib_LIBRARIES = libcpuarch.a

libcpuarch_a_SOURCES = \
	cpuarch.h \
	\
	stg_fetch.c \
	stg_decode.c \
	stg_dispatch.c \
	stg_issue.c \
	stg_writeback.c \
	stg_commit.c \
	\
	bpred.c \
	cpuarch.c \
	fu.c \
	queues.c \
	recover.c \
	rf.c \
	rob.c \
	sched.c \
	tcache.c \
	uop.c \
	uop1.dat \
	uop2.dat

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

