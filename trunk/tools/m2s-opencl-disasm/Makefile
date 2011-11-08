M2S = ../../src/libgpudisasm
CC = gcc
CFLAGS = -Wall -I$(M2S) -I. -g
EXE = m2s-opencl-disasm

SRCS = \
	$(M2S)/gpudump.c \
	$(M2S)/gpudisasm.c \
	debug.c \
	misc.c \
	m2s-opencl-disasm.c
HDRS = \
	$(M2S)/gpudisasm.dat \
	$(M2S)/gpudisasm.h \
	debug.h \
	misc.h

opencl_kernel_disasm: $(SRCS) $(HDRS)
	$(CC) $(CFLAGS) $(SRCS) -o $(EXE)

clean:
	rm -f $(EXE) *.o
