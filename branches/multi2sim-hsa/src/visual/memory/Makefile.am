lib_LIBRARIES = libmemory.a

libmemory_a_SOURCES =

if HAVE_GTK
libmemory_a_SOURCES += \
	\
	mem-system.c \
	mem-system.h \
	\
	mod-access.c \
	mod-access.h \
	\
	mod.c \
	mod.h \
	\
	mod-widget.c \
	mod-widget.h \
	\
	panel.c \
	panel.h

AM_CFLAGS = @GTK_CFLAGS@

endif

AM_CPPFLAGS = @M2S_INCLUDES@
