M2S_ROOT = ../..

AR = ar
CC = gcc
CFLAGS = -Wall -O3 -fPIC -m32 -I. -I$(M2S_ROOT)/src/lib/struct -I$(M2S_ROOT)/src/lib/mhandle \
	-I$(M2S_ROOT)/src/lib/misc

OBJECTS = \
	debug.o \
	list.o \
	opengl.o 

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

debug.o: $(M2S_ROOT)/src/lib/struct/debug.c $(M2S_ROOT)/src/lib/struct/debug.h
	$(CC) $(CFLAGS) -c $(M2S_ROOT)/src/lib/struct/debug.c -o debug.o

list.o: $(M2S_ROOT)/src/lib/struct/list.c $(M2S_ROOT)/src/lib/struct/list.h
	$(CC) $(CFLAGS) -c $(M2S_ROOT)/src/lib/struct/list.c -o list.o



#
# M2S OpenGL Library targets
#

opengl.o: opengl.c m2s-opengl.h
	$(CC) $(CFLAGS) -c opengl.c -o opengl.o

