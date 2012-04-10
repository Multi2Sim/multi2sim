lib_LIBRARIES = libgpuvisual.a

libgpuvisual_a_SOURCES = \
	gpuvisual.h

if HAVE_GTK
libgpuvisual_a_SOURCES += \
	block-dia.c \
	gpu.c \
	gpu-layout.c \
	gpu-trace.c \
	gpuvisual.c \
	gpuvisual-private.h \
	info-popup.c \
	list-layout.c \
	timing-dia.c

AM_CFLAGS = @GTK_CFLAGS@

else

libgpuvisual_a_SOURCES += \
	gpuvisual-nogtk.c

endif

INCLUDES = @M2S_INCLUDES@
