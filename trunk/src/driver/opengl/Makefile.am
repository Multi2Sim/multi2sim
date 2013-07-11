lib_LIBRARIES = libopengl.a

if HAVE_OPENGL
libopengl_a_SOURCES = \
	\
	opengl.c \
	opengl.dat \
	opengl.h \
	\
	si-shader.c \
	si-shader.h \
	\
	si-program.c \
	si-program.h 

else

libopengl_a_SOURCES = \
	\
	opengl.h \
	opengl-missing.c

endif

INCLUDES = @M2S_INCLUDES@

