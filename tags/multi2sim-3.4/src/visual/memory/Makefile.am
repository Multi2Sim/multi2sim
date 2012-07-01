lib_LIBRARIES = libmemory.a

libmemory_a_SOURCES =

if HAVE_GTK
libmemory_a_SOURCES += \
	\
	visual-memory.h	\
	\
	mem-system.c \
	mod-access.c \
	mod.c \
	mod-widget.c \
	net.c \
	panel.c

AM_CFLAGS = @GTK_CFLAGS@

endif

INCLUDES = @M2S_INCLUDES@
