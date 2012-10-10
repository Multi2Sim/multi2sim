M2S_ROOT = ../..

AR = ar
CC = gcc
CFLAGS = -Wall -O3 -fPIC -m32 -I. -Icuda_inc -I$(M2S_ROOT)/src \
	-I$(M2S_ROOT)/src/lib/mhandle -I$(M2S_ROOT)/src/lib/misc -I$(M2S_ROOT)/src/lib/struct

CUDA_OBJECTS = \
	debug.o \
	misc.o \
	linked-list.o \
	list.o \
	cuda.o \
	cudart.o

all: libm2s-cuda.so libm2s-cuda.a


#
# Dynamic and static library targets
#

libm2s-cuda.so: $(CUDA_OBJECTS)
	$(CC) $(CFLAGS) $(CUDA_OBJECTS) -shared -o libm2s-cuda.so

libm2s-cuda.a: $(CUDA_OBJECTS)
	$(AR) cru libm2s-cuda.a $(CUDA_OBJECTS)

clean:
	rm -f $(CUDA_OBJECTS) libm2s-cuda.so libm2s-cuda.a



#
# Auxiliary targets
#

debug.o: $(M2S_ROOT)/src/lib/struct/debug.c $(M2S_ROOT)/src/lib/struct/debug.h
	$(CC) $(CFLAGS) -c $(M2S_ROOT)/src/lib/struct/debug.c -o debug.o

misc.o: $(M2S_ROOT)/src/lib/misc/misc.c $(M2S_ROOT)/src/lib/misc/misc.h
	$(CC) $(CFLAGS) -c $(M2S_ROOT)/src/lib/misc/misc.c -o misc.o

linked-list.o: $(M2S_ROOT)/src/lib/struct/linked-list.c $(M2S_ROOT)/src/lib/struct/linked-list.h
	$(CC) $(CFLAGS) -c $(M2S_ROOT)/src/lib/struct/linked-list.c -o linked-list.o

list.o: $(M2S_ROOT)/src/lib/struct/list.c $(M2S_ROOT)/src/lib/struct/list.h
	$(CC) $(CFLAGS) -c $(M2S_ROOT)/src/lib/struct/list.c -o list.o



#
# M2S CUDA Driver Library targets
#

cuda.o: cuda.c m2s-cuda.h
	$(CC) $(CFLAGS) -c cuda.c -o cuda.o

cudart.o: cudart.c m2s-cuda.h
	$(CC) $(CFLAGS) -c cudart.c -o cudart.o

