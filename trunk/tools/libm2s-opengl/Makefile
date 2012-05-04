M2S_ROOT = ../..

AR = ar
CC = gcc
CFLAGS = -Wall -O3 -fPIC -m32 -I. -I$(M2S_ROOT)/src/lib/struct -I$(M2S_ROOT)/src/lib/mhandle \
	-I$(M2S_ROOT)/src/arch/evergreen/emu -I$(M2S_ROOT)/src/lib/misc \
	-I$(M2S_ROOT)/src/arch/evergreen/asm

OBJECTS = \
	bin-file.o \
	debug.o \
	elf-format.o \
	misc.o \
	\
	gl.o \
	m2s-opengl.o \

all: libm2s-opengl.so libm2s-opengl.a


#
# Dynamic and static library targets
#

libm2s-opengl.so: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -shared -o libm2s-opengl.so

libm2s-opengl.a: $(OBJECTS)
	$(AR) cru libm2s-opengl.a $(OBJECTS)

clean:
	rm -f $(OBJECTS) libm2s-opengl.so libm2s-opengl.a



#
# Auxiliary targets
#

bin-file.o: $(M2S_ROOT)/src/arch/evergreen/emu/bin-file.c
	$(CC) $(CFLAGS) -c $(M2S_ROOT)/src/arch/evergreen/emu/bin-file.c -o bin-file.o

debug.o: $(M2S_ROOT)/src/lib/struct/debug.c $(M2S_ROOT)/src/lib/struct/debug.h
	$(CC) $(CFLAGS) -c $(M2S_ROOT)/src/lib/struct/debug.c -o debug.o

elf-format.o: $(M2S_ROOT)/src/lib/struct/elf-format.c $(M2S_ROOT)/src/lib/struct/elf-format.h
	$(CC) $(CFLAGS) -c $(M2S_ROOT)/src/lib/struct/elf-format.c -o elf-format.o

list.o: $(M2S_ROOT)/src/lib/struct/list.c $(M2S_ROOT)/src/lib/struct/list.h
	$(CC) $(CFLAGS) -c $(M2S_ROOT)/src/lib/struct/list.c -o list.o

misc.o: $(M2S_ROOT)/src/lib/misc/misc.c $(M2S_ROOT)/src/lib/misc/misc.h
	$(CC) $(CFLAGS) -c $(M2S_ROOT)/src/lib/misc/misc.c -o misc.o

#
# M2S OpenCL Library targets
#

gl.o: gl.c m2s-opengl.h
	$(CC) $(CFLAGS) -c gl.c -o gl.o
