lib_LIBRARIES = libglu.a

if HAVE_OPENGL
libglu_a_SOURCES = \
	glu.c \
	glu.dat \
	glu.h

else

libglu_a_SOURCES = \
	\
	glu-missing.c \
	glu.h

endif


INCLUDES = @M2S_INCLUDES@

