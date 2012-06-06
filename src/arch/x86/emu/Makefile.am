lib_LIBRARIES = libemu.a

libemu_a_SOURCES = \
	\
	x86-emu.h \
	\
	isa.c \
	clrt.c \
	clrt.dat \
	context.c \
	emu.c \
	file.c \
	loader.c \
	machine.c \
	machine-ctrl.c \
	machine-fp.c \
	machine-rot.c \
	machine-std.c \
	machine-str.c \
	machine-xmm.c \
	opengl.c \
	opengl.dat \
	regs.c \
	signal.c \
	syscall.c \
	syscall.dat \
	uinst.c

if HAVE_GLUT_H
libemu_a_SOURCES += \
	glut.c \
	glut.dat \
	glut-frame-buffer.c
else
libemu_a_SOURCES += \
	glut-missing.c
endif

INCLUDES = @M2S_INCLUDES@

