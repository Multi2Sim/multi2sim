lib_LIBRARIES = libemu.a

libemu_a_SOURCES = \
	\
	x86-emu.h \
	\
	isa.c \
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
	regs.c \
	signal.c \
	syscall.c \
	syscall.dat \
	uinst.c

if HAVE_GLUT_H

libemu_a_SOURCES += \
	glut.c \
	glut.dat

endif

libemu_a_SOURCES += \
	opengl.c \
	opengl.dat


INCLUDES = @M2S_INCLUDES@

