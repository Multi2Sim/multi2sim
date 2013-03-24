lib_LIBRARIES = libcommon.a

if HAVE_GTK
libcommon_a_SOURCES = \
	\
	cycle-bar.c \
	cycle-bar.h \
	\
	led.c \
	led.h \
	\
	list.c \
	list.h \
	\
	popup.c \
	popup.h \
	\
	state.c \
	state.h \
	\
	trace.c \
	trace.h \
	\
	visual.c \
	visual.h

AM_CFLAGS = @GTK_CFLAGS@

else

libcommon_a_SOURCES = \
	\
	visual-missing.c \
	visual.h

endif

INCLUDES = @M2S_INCLUDES@
