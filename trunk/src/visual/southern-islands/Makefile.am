lib_LIBRARIES = libsouthernislands.a

libsouthernislands_a_SOURCES = 
	
if HAVE_GTK
libsouthernislands_a_SOURCES += \
	\
	visual-southern-islands.h \
	\
	compute-unit.c \
	gpu.c \
	inst.c \
	panel.c \
	time-dia.c \
	work-group.c

AM_CFLAGS = @GTK_CFLAGS@

endif

INCLUDES = @M2S_INCLUDES@
