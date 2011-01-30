AR = ar
CC = gcc
CFLAGS = -Wall -fPIC -m32 -I$(ATISTREAMSDKROOT)/include

all: libm2s-opencl.so libm2s-opencl.a

libm2s-opencl.so: m2s-opencl.o
	$(CC) m2s-opencl.o -shared -o libm2s-opencl.so

libm2s-opencl.a: m2s-opencl.o
	$(AR) cru libm2s-opencl.a m2s-opencl.o

m2s-opencl.o: m2s-opencl.c
ifndef ATISTREAMSDKROOT
	@echo "error: ATI SDK not installed.\n" \
		"\tThe 'libm2s-opencl' library has been included in the set of Multi2Sim tools\n" \
		"\tto compile OpenCL kernels. It requires the AMD SDK to be installed in\n" \
		"\tyour system. Please, visit www.amd.com/stream to download it." >&2
	@exit 1
else
	$(CC) $(CFLAGS) -c m2s-opencl.c -o m2s-opencl.o
endif

clean:
	rm -f m2s-opencl.o libm2s-opencl.so libm2s-opencl.a

