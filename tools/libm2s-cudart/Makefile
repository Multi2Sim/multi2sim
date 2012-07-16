M2S_ROOT = ../..

AR = ar
CC = gcc
CFLAGS = -Wall -O3 -fPIC -m32 -I. -I$(M2S_ROOT)/src/lib/struct -I$(M2S_ROOT)/src/lib/mhandle \
	-I$(M2S_ROOT)/src/lib/misc -I/usr/local/cuda/include

CUDART_OBJECTS = \
	debug.o \
	misc.o \
	linked-list.o \
	list.o \
	cudart.o

all: libm2s-cudart.so libm2s-cudart.a


#
# Dynamic and static library targets
#

libm2s-cudart.so: $(CUDART_OBJECTS)
	$(CC) $(CFLAGS) $(CUDART_OBJECTS) -shared -o libm2s-cudart.so

libm2s-cudart.a: $(CUDART_OBJECTS)
	$(AR) cru libm2s-cudart.a $(CUDART_OBJECTS)

clean:
	rm -f $(CUDART_OBJECTS) libm2s-cudart.so libm2s-cudart.a



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
# M2S CUDA Runtime Library targets
#

cudart.o: cudart.c m2s-cudart.h
	$(CC) $(CFLAGS) -c cudart.c -o cudart.o

