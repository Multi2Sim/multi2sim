lib_LIBRARIES = libgl.a

if HAVE_OPENGL

libgl_a_SOURCES = \
	\
	gl.c \
	gl.h

else

libgl_a_SOURCES = \
	\
	gl-missing.c \
	gl.h

endif

AM_CPPFLAGS = @M2S_INCLUDES@

