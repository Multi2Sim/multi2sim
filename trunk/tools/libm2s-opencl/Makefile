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
	command-queue.o \
	context.o \
	device.o \
	event.o \
	kernel.o \
	list.o \
	m2s-opencl.o \
	mem.o \
	platform.o \
	program.o \
	sampler.o

all: libm2s-opencl.so libm2s-opencl.a


#
# Dynamic and static library targets
#

libm2s-opencl.so: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -shared -o libm2s-opencl.so

libm2s-opencl.a: $(OBJECTS)
	$(AR) cru libm2s-opencl.a $(OBJECTS)

clean:
	rm -f $(OBJECTS) libm2s-opencl.so libm2s-opencl.a



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

command-queue.o: command-queue.c m2s-opencl.h
	$(CC) $(CFLAGS) -c command-queue.c -o command-queue.o

context.o: context.c m2s-opencl.h
	$(CC) $(CFLAGS) -c context.c -o context.o

device.o: device.c m2s-opencl.h
	$(CC) $(CFLAGS) -c device.c -o device.o

event.o: event.c m2s-opencl.h
	$(CC) $(CFLAGS) -c event.c -o event.o

kernel.o: kernel.c m2s-opencl.h
	$(CC) $(CFLAGS) -c kernel.c -o kernel.o

m2s-opencl.o: m2s-opencl.c m2s-opencl.h
	$(CC) $(CFLAGS) -c m2s-opencl.c -o m2s-opencl.o

mem.o: mem.c m2s-opencl.h
	$(CC) $(CFLAGS) -c mem.c -o mem.o
	
platform.o: platform.c m2s-opencl.h
	$(CC) $(CFLAGS) -c platform.c -o platform.o

program.o: program.c m2s-opencl.h
	$(CC) $(CFLAGS) -c program.c -o program.o

sampler.o: sampler.c m2s-opencl.h
	$(CC) $(CFLAGS) -c sampler.c -o sampler.o

