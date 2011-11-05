lib_LIBRARIES = libnetwork.a
libnetwork_a_SOURCES = \
	network.h \
	network.c

if DEBUG
AM_CFLAGS = -Wall -DMHANDLE -g -O0
else
AM_CFLAGS = -O3
endif

INCLUDES = -I$(top_srcdir)/src/libstruct \
	-I$(top_srcdir)/src/libmhandle \
	-I$(top_srcdir)/src/libesim \
	-I$(top_srcdir)/src/libopt \
	-I$(top_srcdir)/src/libesim \
	-I$(top_srcdir)/src/libmisc

