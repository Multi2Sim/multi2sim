M2S_ROOT = ../..

AR = ar
CC = gcc
CFLAGS = -Wall -O3 -fPIC -m32 -I. -I$(M2S_ROOT)/src/lib/struct -I$(M2S_ROOT)/src/lib/mhandle \
	-I$(M2S_ROOT)/src/lib/misc -I/usr/local/cuda/include

OBJECTS = \
	debug.o \
	misc.o \
	list.o \
	cuda.o \

all: libm2s-cuda.so libm2s-cuda.a


#
# Dynamic and static library targets
#

libm2s-cuda.so: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -shared -o libm2s-cuda.so

libm2s-cuda.a: $(OBJECTS)
	$(AR) cru libm2s-cuda.a $(OBJECTS)

clean:
	rm -f $(OBJECTS) libm2s-cuda.so libm2s-cuda.a



#
# Auxiliary targets
#

debug.o: $(M2S_ROOT)/src/lib/struct/debug.c $(M2S_ROOT)/src/lib/struct/debug.h
	$(CC) $(CFLAGS) -c $(M2S_ROOT)/src/lib/struct/debug.c -o debug.o

misc.o: $(M2S_ROOT)/src/lib/misc/misc.c $(M2S_ROOT)/src/lib/misc/misc.h
	$(CC) $(CFLAGS) -c $(M2S_ROOT)/src/lib/misc/misc.c -o misc.o

list.o: $(M2S_ROOT)/src/lib/struct/list.c $(M2S_ROOT)/src/lib/struct/list.h
	$(CC) $(CFLAGS) -c $(M2S_ROOT)/src/lib/struct/list.c -o list.o



#
# M2S GLUT Library targets
#

cuda.o: cuda.c m2s-cuda.h
	$(CC) $(CFLAGS) -c cuda.c -o cuda.o

