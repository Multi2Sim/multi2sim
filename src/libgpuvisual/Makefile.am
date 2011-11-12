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
	misc.c \
	timing-dia.c

AM_CFLAGS = @GTK_CFLAGS@

else

libgpuvisual_a_SOURCES += \
	gpuvisual-nogtk.c

endif


INCLUDES = \
	-I$(top_srcdir)/src/libmhandle \
	-I$(top_srcdir)/src/libstruct \
	-I$(top_srcdir)/src/libgpuvisual

