lib_LIBRARIES = libevergreen.a

libevergreen_a_SOURCES =

if HAVE_GTK
libevergreen_a_SOURCES += \
	\
	visual-evergreen.h \
	\
	compute-unit.c \
	gpu.c \
	panel.c \
	work-group.c

AM_CFLAGS = @GTK_CFLAGS@

endif

INCLUDES = @M2S_INCLUDES@
