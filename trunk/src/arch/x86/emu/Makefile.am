lib_LIBRARIES = libemu.a

libemu_a_SOURCES = \
	\
	checkpoint.c \
	checkpoint.h \
	\
	clrt.c \
	clrt.dat \
	clrt.h \
	\
	context.c \
	context.h \
	\
	emu.c \
	emu.h \
	\
	file-desc.c \
	file-desc.h \
	\
	isa.c \
	isa.h \
	\
	loader.c \
	loader.h \
	\
	machine.c \
	machine.h \
	machine-ctrl.c \
	machine-fp.c \
	machine-rot.c \
	machine-std.c \
	machine-str.c \
	machine-xmm.c \
	\
	regs.c \
	regs.h \
	\
	signal.c \
	signal.h \
	\
	syscall.c \
	syscall.dat \
	syscall.h \
	\
	uinst.c \
	uinst.h

if HAVE_GLUT_H
libemu_a_SOURCES += \
	\
	glut.c \
	glut.dat \
	glut.h \
	\
	glut-frame-buffer.c \
	glut-frame-buffer.h \
	\
	opengl.c \
	opengl.dat \
	opengl.h \
	\
	opengl-context.c \
	opengl-context.h \
	\
	opengl-matrix.c \
	opengl-matrix.h \
	\
	opengl-matrix-stack.c \
	opengl-matrix-stack.h \
	\
	opengl-buffers.c \
	opengl-buffers.h \
	\
	opengl-vertex.c \
	opengl-vertex.h \
	\
	opengl-color-channel.c \
	opengl-color-channel.h \
	\
	opengl-light.c \
	opengl-light.h \
	\
	opengl-span.c \
	opengl-span.h \
	\
	opengl-edge.c \
	opengl-edge.h \
	\
	opengl-rast-config.h \
	\
	opengl-rast-line.c \
	opengl-rast-line.h \
	\
	opengl-rast-triangle.c \
	opengl-rast-triangle.h \
	\
	opengl-viewport.c \
	opengl-viewport.h
else
libemu_a_SOURCES += \
	\
	glut.h \
	glut-missing.c \
	\
	opengl.h \
	opengl-missing.c
endif

INCLUDES = @M2S_INCLUDES@

