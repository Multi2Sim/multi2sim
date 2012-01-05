AR = ar
CC = gcc
CFLAGS = -Wall -O3 -fPIC -m32 -I. -I../../src/libstruct -I../../src/libmhandle

OBJECTS = \
	command-queue.o \
	context.o \
	debug.o \
	device.o \
	elf-format.o \
	event.o \
	kernel.o \
	m2s-opencl.o \
	mem.o \
	platform.o \
	program.o \
	sampler.o

all: libm2s-opencl.so libm2s-opencl.a

libm2s-opencl.so: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -shared -o libm2s-opencl.so

libm2s-opencl.a: $(OBJECTS)
	$(AR) cru libm2s-opencl.a $(OBJECTS)

command-queue.o: command-queue.c m2s-opencl.h
	$(CC) $(CFLAGS) -c command-queue.c -o command-queue.o

context.o: context.c m2s-opencl.h
	$(CC) $(CFLAGS) -c context.c -o context.o

debug.o: ../../src/libstruct/debug.c ../../src/libstruct/debug.h
	$(CC) $(CFLAGS) -c ../../src/libstruct/debug.c -o debug.o

device.o: device.c m2s-opencl.h
	$(CC) $(CFLAGS) -c device.c -o device.o

elf-format.o: ../../src/libstruct/elf-format.c ../../src/libstruct/elf-format.h
	$(CC) $(CFLAGS) -c ../../src/libstruct/elf-format.c -o elf-format.o

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

clean:
	rm -f $(OBJECTS) libm2s-opencl.so libm2s-opencl.a

