lib_LIBRARIES = libemu.a

libemu_a_SOURCES = \
	\
	isa.c \
	checkpoint.c \
	clrt.c \
	clrt.dat \
	context.c \
	\
	emu.c \
	emu.h \
	\
	file.c \
	loader.c \
	machine.c \
	machine-ctrl.c \
	machine-fp.c \
	machine-rot.c \
	machine-std.c \
	machine-str.c \
	machine-xmm.c \
	regs.c \
	signal.c \
	syscall.c \
	syscall.dat \
	uinst.c

if HAVE_GLUT_H
libemu_a_SOURCES += \
	glut.c \
	glut.dat \
	glut-frame-buffer.c \
	opengl.c \
	opengl.dat \
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
	opengl-viewport.c \
	opengl-viewport.h
else
libemu_a_SOURCES += \
	glut-missing.c \
	opengl-missing.c
endif

INCLUDES = @M2S_INCLUDES@

