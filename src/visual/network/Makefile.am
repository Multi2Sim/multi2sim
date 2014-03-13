lib_LIBRARIES = libnetwork.a

libnetwork_a_SOURCES =

if HAVE_GTK
libnetwork_a_SOURCES += \
	\
	buffer.c \
	buffer.h \
	\
	net-system.c \
	net-system.h \
	\
	net.c \
	net.h \
	\
	node.c \
	node.h 

AM_CFLAGS = @GTK_CFLAGS@

endif

INCLUDES = @M2S_INCLUDES@
