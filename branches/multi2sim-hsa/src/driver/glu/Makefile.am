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


AM_CPPFLAGS = @M2S_INCLUDES@

