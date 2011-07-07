lib_LIBRARIES = libgpudisasm.a
libgpudisasm_a_SOURCES = \
	gpudisasm.c \
	gpudisasm.dat \
	gpudisasm.h \
	gpudump.c

if DEBUG
AM_CFLAGS = -Wall -fno-strict-aliasing -DMHANDLE -g -O0
else
AM_CFLAGS = -fno-strict-aliasing -O3
endif


INCLUDES = -I$(top_srcdir)/src/libstruct \
	-I$(top_srcdir)/src/libmhandle \
	-I$(top_srcdir)/src/libmisc


