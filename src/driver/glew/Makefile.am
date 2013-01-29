lib_LIBRARIES = libglew.a

if HAVE_OPENGL
libglew_a_SOURCES = \
	glew.c \
	glew.dat \
	glew.h

else

libglew_a_SOURCES = \
	\
	glew-missing.c \
	glew.h

endif


INCLUDES = @M2S_INCLUDES@

