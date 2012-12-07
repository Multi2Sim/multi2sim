M2S_ROOT = ../..
M2S_LIB = $(M2S_ROOT)/src/lib

AR = ar
CC = gcc
CFLAGS = -Wall -O3 -fPIC -m32 -I. \
	-I$(M2S_ROOT)/src

OBJECTS = \
	debug.o \
	mhandle.o \
	misc.o \
	string.o \
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

debug.o: $(M2S_LIB)/util/debug.c $(M2S_LIB)/util/debug.h
	$(CC) $(CFLAGS) -c $(M2S_LIB)/util/debug.c -o debug.o

list.o: $(M2S_LIB)/util/list.c $(M2S_LIB)/util/list.h
	$(CC) $(CFLAGS) -c $(M2S_LIB)/util/list.c -o list.o

mhandle.o: $(M2S_LIB)/mhandle/mhandle.c $(M2S_LIB)/mhandle/mhandle.h
	$(CC) $(CFLAGS) -c $(M2S_LIB)/mhandle/mhandle.c -o mhandle.o

misc.o: $(M2S_LIB)/util/misc.c $(M2S_LIB)/util/misc.h
	$(CC) $(CFLAGS) -c $(M2S_LIB)/util/misc.c -o misc.o

string.o: $(M2S_LIB)/util/string.c $(M2S_LIB)/util/string.h
	$(CC) $(CFLAGS) -c $(M2S_LIB)/util/string.c -o string.o



#
# M2S GLUT Library targets
#

glut.o: glut.c m2s-glut.h 
	$(CC) $(CFLAGS) -c glut.c -o glut.o

window.o: window.c m2s-glut.h
	$(CC) $(CFLAGS) -c window.c -o window.o
