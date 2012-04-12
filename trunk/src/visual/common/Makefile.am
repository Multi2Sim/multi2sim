lib_LIBRARIES = libcommon.a

libcommon_a_SOURCES = \
	visual-common.h

if HAVE_GTK
libcommon_a_SOURCES += \
	cycle-bar.c \
	info-popup.c \
	trace-file.c \
	visual.c \
	visual-list.c \
	visual-state.c

AM_CFLAGS = @GTK_CFLAGS@

else

libcommon_a_SOURCES += \
	visual-nogtk.c

endif

INCLUDES = @M2S_INCLUDES@
