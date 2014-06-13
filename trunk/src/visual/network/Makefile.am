lib_LIBRARIES = libnetwork.a

libnetwork_a_SOURCES =

if HAVE_GTK
libnetwork_a_SOURCES += \
	\
	buffer.c \
	buffer.h \
	\
	link.c \
	link.h \
	\
	net.c \
	net.h \
	\
	net-graph.c \
	net-graph.h \
	\
	net-message.c \
	net-message.h \
	\
	net-packet.c \
	net-packet.h \
	\
	net-system.c \
	net-system.h \
	\
	net-widget.c \
	net-widget.h \
	\
	node.c \
	node.h \
	\
	node-widget.c \
	node-widget.h \
	\
	panel.c \
	panel.h

AM_CFLAGS = @GTK_CFLAGS@

endif

AM_CPPFLAGS = @M2S_INCLUDES@
