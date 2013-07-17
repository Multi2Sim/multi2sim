lib_LIBRARIES = libevergreen.a

libevergreen_a_SOURCES =

if HAVE_GTK
libevergreen_a_SOURCES += \
	\
	compute-unit.c \
	compute-unit.h \
	\
	gpu.c \
	gpu.h \
	\
	inst.c \
	inst.h \
	\
	panel.c \
	panel.h \
	\
	time-dia.c \
	time-dia.h \
	\
	work-group.c \
	work-group.h

AM_CFLAGS = @GTK_CFLAGS@

endif

INCLUDES = @M2S_INCLUDES@
