if HAVE_M32_FLAG

lib_LTLIBRARIES = $(top_builddir)/lib/libm2s-glut.la

__top_builddir__lib_libm2s_glut_la_LDFLAGS = -version-info 0:0:0

__top_builddir__lib_libm2s_glut_la_SOURCES = \
	\
	api.c \
	api.h \
	\
	debug.c \
	debug.h \
	\
	list.c \
	list.h \
	\
	window.c \
	window.h \
	\
	include/GL/glut.h

AM_CFLAGS = -Wall -O2 -fPIC -m32

INCLUDES = -I./include -I../opengl/include

endif

