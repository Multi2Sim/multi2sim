lib_LIBRARIES = libvisual.a

libvisual_a_SOURCES = \
	visual.h

if HAVE_GTK
libvisual_a_SOURCES += \
	cycle-bar.c \
	info-popup.c \
	visual.c \
	visual-private.h \
	state-file.c \
	trace-file.c \
	vcache.c \
	\
	visual-mem-system.c \
	visual-mod.c \
	visual-mod-access.c \
	visual-net.c \
	\
	visual-mem-system-widget.c \
	visual-mod-widget.c \
	\
	vlist.c \
	vmod.c \
	vmod-access.c \
	vmod-panel.c \
	vnet.c

AM_CFLAGS = @GTK_CFLAGS@

else

libvisual_a_SOURCES += \
	visual-nogtk.c

endif

INCLUDES = \
	-I$(top_srcdir)/src/libmhandle \
	-I$(top_srcdir)/src/libstruct \
	-I$(top_srcdir)/src/libvisual \
	-I$(top_srcdir)/src/libmisc

