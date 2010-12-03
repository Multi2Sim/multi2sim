lib_LIBRARIES = libcachesystem.a
libcachesystem_a_SOURCES = cachesystem.h \
	cache.c \
	cachesystem.c \
	directory.c \
	mmu.c \
	moesi.c
AM_CFLAGS = -Wall -fno-strict-aliasing
INCLUDES = -I$(top_srcdir)/src/libstruct \
	-I$(top_srcdir)/src/libmhandle \
	-I$(top_srcdir)/src/libnetwork \
	-I$(top_srcdir)/src/libopt \
	-I$(top_srcdir)/src/libesim \
	-I$(top_srcdir)/src/libmisc

