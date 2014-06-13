SUBDIRS = \
	southern-islands

lib_LIBRARIES = libopengl.a

if HAVE_OPENGL
libopengl_a_SOURCES = \
	\
	opengl.c \
	opengl.dat \
	opengl.h \
	\
	si-db.c \
	si-db.h \
	\
	si-sc.c \
	si-sc.h \
	\
	si-spi.c \
	si-spi.h \
	\
	si-pa.c \
	si-pa.h \
	\
	si-shader.c \
	si-shader.h \
	\
	si-program.c \
	si-program.h \
	\
	OpenGLDriver.cc \
	OpenGLDriver.h \
	\
	ABI.cc \
	ABI.dat \
	ABI.h

else

libopengl_a_SOURCES = \
	\
	opengl.h \
	opengl-missing.c

endif

AM_CPPFLAGS = @M2S_INCLUDES@
