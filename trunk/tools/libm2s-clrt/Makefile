M2S_ROOT = ../..

AR = ar
CC = gcc
CFLAGS = -Wall -Werror -O3 -fPIC -m32 -I. \
	-I$(M2S_ROOT)/src/lib/struct \
	-I$(M2S_ROOT)/src/lib/mhandle \
	-I$(M2S_ROOT)/src/lib/misc -g
ASM = nasm
ASMFLAGS = -f elf32


LIB_NAME = libm2s-clrt
HEADER_NAME = m2s-clrt.h

OBJECTS = \
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
	m2s-clrt.o \
	mem.o \
	object.o \
	platform.o \
	program.o \
	sampler.o \
	elf-loader.o \
	fibers.o \
	mkfibers.o \
	workgroupdata.o

all: $(LIB_NAME).so $(LIB_NAME).a


#
# Dynamic and static library targets
#

$(LIB_NAME).so: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -shared -o $(LIB_NAME).so

$(LIB_NAME).a: $(OBJECTS)
	$(AR) cru $(LIB_NAME).a $(OBJECTS)

clean:
	rm -f $(OBJECTS) $(LIB_NAME).so $(LIB_NAME).a



#
# Auxiliary targets
#

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

command-queue.o: command-queue.c $(HEADER_NAME)
	$(CC) $(CFLAGS) -c command-queue.c -o command-queue.o

context.o: context.c $(HEADER_NAME)
	$(CC) $(CFLAGS) -c context.c -o context.o

device.o: device.c $(HEADER_NAME)
	$(CC) $(CFLAGS) -c device.c -o device.o

event.o: event.c $(HEADER_NAME)
	$(CC) $(CFLAGS) -c event.c -o event.o

kernel.o: kernel.c $(HEADER_NAME)
	$(CC) $(CFLAGS) -c kernel.c -o kernel.o

m2s-clrt.o: m2s-clrt.c $(HEADER_NAME)
	$(CC) $(CFLAGS) -c m2s-clrt.c -o m2s-clrt.o

mem.o: mem.c $(HEADER_NAME)
	$(CC) $(CFLAGS) -c mem.c -o mem.o

object.o: object.c $(HEADER_NAME)
	$(CC) $(CFLAGS) -c object.c -o object.o
	
platform.o: platform.c $(HEADER_NAME)
	$(CC) $(CFLAGS) -c platform.c -o platform.o

program.o: program.c $(HEADER_NAME)
	$(CC) $(CFLAGS) -c program.c -o program.o

sampler.o: sampler.c $(HEADER_NAME)
	$(CC) $(CFLAGS) -c sampler.c -o sampler.o

elf-loader.o: elf-loader.c $(HEADER_NAME)
	$(CC) $(CFLAGS) -c elf-loader.c -o elf-loader.o

fibers.o: fibers.s
	$(ASM) $(ASMFLAGS) fibers.s -o fibers.o

mkfibers.o: mkfibers.c
	$(CC) $(CFLAGS) -c mkfibers.c -o mkfibers.o

workgroupdata.o: workgroupdata.s
	$(ASM) $(ASMFLAGS) workgroupdata.s -o workgroupdata.o

