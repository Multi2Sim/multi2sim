lib_LIBRARIES = libcpuarch.a

libcpuarch_a_SOURCES = \
	cpuarch.h \
	\
	stg-fetch.c \
	stg-decode.c \
	stg-dispatch.c \
	stg-issue.c \
	stg-writeback.c \
	stg-commit.c \
	\
	bpred.c \
	cpuarch.c \
	fu.c \
	queues.c \
	recover.c \
	rf.c \
	rob.c \
	sched.c \
	trace-cache.c \
	uop.c

INCLUDES = -I$(top_srcdir)/src/libstruct \
	-I$(top_srcdir)/src/libmhandle \
	-I$(top_srcdir)/src/libopt \
	-I$(top_srcdir)/src/libmisc \
	-I$(top_srcdir)/src/libcpudisasm \
	-I$(top_srcdir)/src/libmemsystem \
	-I$(top_srcdir)/src/libnetwork \
	-I$(top_srcdir)/src/libesim \
	-I$(top_srcdir)/src/libcpukernel \
	-I$(top_srcdir)/src/libgpukernel \
	-I$(top_srcdir)/src/libgpudisasm

