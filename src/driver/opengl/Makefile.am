lib_LIBRARIES = libopengl.a

if HAVE_OPENGL
libopengl_a_SOURCES = \
	\
	buffers.c \
	buffers.h \
	\
	color-channel.c \
	color-channel.h \
	\
	context.c \
	context.h \
	\
	edge.c \
	edge.h \
	\
	light.c \
	light.h \
	\
	material.c \
	material.h \
	\
	matrix.c \
	matrix.h \
	\
	matrix-stack.c \
	matrix-stack.h \
	\
	opengl.c \
	opengl.dat \
	opengl.h \
	\
	rasterizer.c \
	rasterizer.h \
	\
	span.c \
	span.h \
	\
	vector.c \
	vector.h \
	\
	vertex.c \
	vertex.h \
	\
	viewport.c \
	viewport.h

else

libopengl_a_SOURCES = \
	\
	opengl.h \
	opengl-missing.c

endif

INCLUDES = @M2S_INCLUDES@

