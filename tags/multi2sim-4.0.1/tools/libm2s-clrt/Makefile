M2S_ROOT = ../..
M2S_LIB = $(M2S_ROOT)/src/lib

AR = ar
CC = gcc
DEBUG_CFLAGS = -Wall -Werror -O0 -g -DMHANDLE
CFLAGS = -fPIC -m32 -I./include
CFLAGS += $(DEBUG_CFLAGS)  ## Remove for no debugging
ASM = nasm
ASMFLAGS = -f elf32


LIB_NAME = libm2s-clrt
HEADER_NAME = m2s-clrt.h clrt-object.h thread-list.h device-interface.h
CLCPU_HEADERS = clcpu.h clcpu-device.h clcpu-program.h fibers.h

OBJS = \
	command-queue.o \
	context.o \
	debug.o \
	device.o \
	elf-format.o \
	elf-loader.o \
	event.o \
	kernel.o \
	list.o \
	fibers.o \
	m2s-clrt.o \
	workgroupdata.o \
	mem.o \
	mhandle.o \
	mkfibers.o \
	object.o \
	platform.o \
	program.o \
	sampler.o \
	thread-list.o \
	clcpu-device.o \
	clcpu-program.o \
	clcpu-execute.o


all: $(LIB_NAME).so $(LIB_NAME).a


#
# Dynamic and static library targets
#

$(LIB_NAME).so: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -shared -o $(LIB_NAME).so

$(LIB_NAME).a: $(OBJS)
	$(AR) cru $(LIB_NAME).a $(OBJS)

clean:
	rm -f *.o *.so *.a

depend:
	makedepend -- $(CFLAGS) -- $(OBJS:.o=.c)

.c.o:
	$(CC) $(CFLAGS) -c $*.c -o $@

.s.o:
	$(ASM) $(ASMFLAGS) $*.s -o $@




# DO NOT DELETE



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

mkfibers.o: mkfibers.c $(HEADER_NAME)
	$(CC) $(CFLAGS) -c mkfibers.c -o mkfibers.o

workgroupdata.o: workgroupdata.s
	$(ASM) $(ASMFLAGS) workgroupdata.s -o workgroupdata.o

thread-list.o: thread-list.c $(HEADER_NAME)
	$(CC) $(CFLAGS) -c thread-list.c -o thread-list.o

clcpu-device.o: clcpu-device.c $(HEADER_NAME) $(CLCPU_HEADERS)
	$(CC) $(CFLAGS) -c clcpu-device.c

clcpu-program.o: clcpu-program.c $(HEADER_NAME) $(CLCPU_HEADERS)
	$(CC) $(CFLAGS) -c clcpu-program.c

clcpu-execute.o: clcpu-execute.c $(HEADER_NAME) $(CLCPU_HEADERS)
	$(CC) $(CFLAGS) -c clcpu-execute.c
