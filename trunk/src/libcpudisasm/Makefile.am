lib_LIBRARIES = libcpudisasm.a

libcpudisasm_a_SOURCES = \
	cpudisasm.h \
	cpudisasm.c \
	machine.dat

if DEBUG
AM_CFLAGS = -Wall -fno-strict-aliasing -DMHANDLE -g -O0
else
AM_CFLAGS = -fno-strict-aliasing -O3
endif

INCLUDES = -I$(top_srcdir)/src/libstruct \
	-I$(top_srcdir)/src/libmhandle

