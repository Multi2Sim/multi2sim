lib_LIBRARIES = libopengl.a

if HAVE_OPENGL
libopengl_a_SOURCES = \
	\
	buffers.c \
	buffers.h \
	\
	buffer-obj.c \
	buffer-obj.h \
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
	mem.c \
	mem.h \
	\
	opengl.c \
	opengl.dat \
	opengl.h \
	\
	rasterizer.c \
	rasterizer.h \
	\
	si-shader.c \
	si-shader.h \
	\
	si-program.c \
	si-program.h \
	\
	shader.c \
	shader.h \
	\
	span.c \
	span.h \
	\
	transform-feedback.c \
	transform-feedback.h \
	\
	vector.c \
	vector.h \
	\
	vertex.c \
	vertex.h \
	\
	vertex-array.c \
	vertex-array.h \
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

