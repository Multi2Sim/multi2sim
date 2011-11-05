lib_LIBRARIES = libstruct.a

libstruct_a_SOURCES = buffer.c buffer.h chrono.c chrono.h config.c config.h \
	hash.c hash.h heap.c heap.h list.c list.h lnlist.c lnlist.h \
	repos.c repos.h debug.c debug.h

if DEBUG
AM_CFLAGS = -Wall -fno-strict-aliasing -DMHANDLE -g -O0
else
AM_CFLAGS = -fno-strict-aliasing -O3
endif

INCLUDES = -I$(top_srcdir)/src/libmhandle

