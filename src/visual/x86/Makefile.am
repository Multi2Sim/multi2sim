lib_LIBRARIES = libx86.a

libx86_a_SOURCES =

if HAVE_GTK
libx86_a_SOURCES += \
	visual-x86.h \
	\
	context.c \
	core.c \
	cpu.c \
	panel.c

AM_CFLAGS = @GTK_CFLAGS@

endif

INCLUDES = @M2S_INCLUDES@
