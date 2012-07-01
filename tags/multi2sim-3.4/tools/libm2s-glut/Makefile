M2S_ROOT = ../..

AR = ar
CC = gcc
CFLAGS = -Wall -O3 -fPIC -m32 -I. -I$(M2S_ROOT)/src/lib/struct -I$(M2S_ROOT)/src/lib/mhandle \
	-I$(M2S_ROOT)/src/lib/misc

OBJECTS = \
	debug.o \
	misc.o \
	list.o \
	glut.o \
	window.o

all: libm2s-glut.so libm2s-glut.a


#
# Dynamic and static library targets
#

libm2s-glut.so: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -shared -o libm2s-glut.so

libm2s-glut.a: $(OBJECTS)
	$(AR) cru libm2s-glut.a $(OBJECTS)

clean:
	rm -f $(OBJECTS) libm2s-glut.so libm2s-glut.a



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

glut.o: glut.c m2s-glut.h
	$(CC) $(CFLAGS) -c glut.c -o glut.o

window.o: window.c m2s-glut.h
	$(CC) $(CFLAGS) -c window.c -o window.o
