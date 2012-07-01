lib_LIBRARIES = libcommon.a

libcommon_a_SOURCES = \
	visual-common.h

if HAVE_GTK
libcommon_a_SOURCES += \
	cycle-bar.c \
	led.c \
	list.c \
	popup.c \
	state.c \
	trace.c \
	visual.c

AM_CFLAGS = @GTK_CFLAGS@

else

libcommon_a_SOURCES += \
	visual-nogtk.c

endif

INCLUDES = @M2S_INCLUDES@
