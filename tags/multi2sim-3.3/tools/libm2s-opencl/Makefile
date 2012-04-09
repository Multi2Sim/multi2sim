M2S_ROOT = ../..

AR = ar
CC = gcc
CFLAGS = -Wall -O3 -fPIC -m32 -I. -I$(M2S_ROOT)/src/libstruct -I$(M2S_ROOT)/src/libmhandle \
	-I$(M2S_ROOT)/src/libgpukernel -I$(M2S_ROOT)/src/libmisc -I$(M2S_ROOT)/src/libgpudisasm

OBJECTS = \
	amd-bin.o \
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

amd-bin.o: $(M2S_ROOT)/src/libgpukernel/amd-bin.c
	$(CC) $(CFLAGS) -c $(M2S_ROOT)/src/libgpukernel/amd-bin.c -o amd-bin.o

debug.o: $(M2S_ROOT)/src/libstruct/debug.c $(M2S_ROOT)/src/libstruct/debug.h
	$(CC) $(CFLAGS) -c $(M2S_ROOT)/src/libstruct/debug.c -o debug.o

elf-format.o: $(M2S_ROOT)/src/libstruct/elf-format.c $(M2S_ROOT)/src/libstruct/elf-format.h
	$(CC) $(CFLAGS) -c $(M2S_ROOT)/src/libstruct/elf-format.c -o elf-format.o

list.o: $(M2S_ROOT)/src/libstruct/list.c $(M2S_ROOT)/src/libstruct/list.h
	$(CC) $(CFLAGS) -c $(M2S_ROOT)/src/libstruct/list.c -o list.o

misc.o: $(M2S_ROOT)/src/libmisc/misc.c $(M2S_ROOT)/src/libmisc/misc.h
	$(CC) $(CFLAGS) -c $(M2S_ROOT)/src/libmisc/misc.c -o misc.o



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

