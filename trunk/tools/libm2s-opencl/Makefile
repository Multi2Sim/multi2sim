AR = ar
CC = gcc
CFLAGS = -Wall -fPIC -m32 -I.

all: libm2s-opencl.so libm2s-opencl.a

libm2s-opencl.so: m2s-opencl.o
	$(CC) m2s-opencl.o -shared -o libm2s-opencl.so

libm2s-opencl.a: m2s-opencl.o
	$(AR) cru libm2s-opencl.a m2s-opencl.o

m2s-opencl.o: m2s-opencl.c
	$(CC) $(CFLAGS) -c m2s-opencl.c -o m2s-opencl.o

clean:
	rm -f m2s-opencl.o libm2s-opencl.so libm2s-opencl.a

