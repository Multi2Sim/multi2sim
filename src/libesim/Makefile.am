lib_LIBRARIES = libesim.a

libesim_a_SOURCES = esim.c esim.h

if DEBUG
AM_CFLAGS = -Wall -fno-strict-aliasing -DMHANDLE -g -O0
else
AM_CFLAGS = -fno-strict-aliasing -O3
endif

INCLUDES = -I$(top_srcdir)/src/libstruct -I$(top_srcdir)/src/libmhandle

