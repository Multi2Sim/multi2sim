lib_LIBRARIES = libglut.a

if HAVE_OPENGL
libglut_a_SOURCES = \
	\
	frame-buffer.c \
	frame-buffer.h \
	\
	glut.c \
	glut.dat \
	glut.h

else

libglut_a_SOURCES = \
	\
	glut-missing.c \
	glut.h

endif


INCLUDES = @M2S_INCLUDES@

