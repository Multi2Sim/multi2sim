lib_LIBRARIES = libmemvisual.a

libmemvisual_a_SOURCES = \
	memvisual.h

if HAVE_GTK
libmemvisual_a_SOURCES += \
	info-popup.c \
	memvisual.c \
	memvisual-private.h \
	trace-file.c \
	vcache.c \
	vlist.c \
	vmod.c \
	vmod-panel.c

AM_CFLAGS = @GTK_CFLAGS@

else

libmemvisual_a_SOURCES += \
	memvisual-nogtk.c

endif

INCLUDES = \
	-I$(top_srcdir)/src/libmhandle \
	-I$(top_srcdir)/src/libstruct \
	-I$(top_srcdir)/src/libmemvisual \
	-I$(top_srcdir)/src/libmisc

