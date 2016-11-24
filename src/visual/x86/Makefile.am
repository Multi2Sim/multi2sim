lib_LIBRARIES = libx86.a

libx86_a_SOURCES =

if HAVE_GTK
libx86_a_SOURCES += \
	\
	context.c \
	context.h \
	\
	core.c \
	core.h \
	\
	cpu.c \
	cpu.h \
	\
	inst.c \
	inst.h \
	\
	panel.c \
	panel.h \
	\
	time-dia.c \
	time-dia.h

AM_CFLAGS = @GTK_CFLAGS@

endif

AM_CPPFLAGS = @M2S_INCLUDES@
