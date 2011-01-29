CC = gcc
CFLAGS = -Wall -I. -I$(ATISTREAMSDKROOT)/include -g
LDFLAGS = -L$(ATISTREAMSDKROOT)/lib/x86 -lOpenCL -ldl -laticalrt -laticalcl

SRCS = \
	debug.c \
	elfanalyze.c \
	misc.c \
	m2s-opencl-kc.c

HDRS = \
	debug.h \
	elfanalyze.h \
	misc.h

EXE = m2s-opencl-kc

$(EXE): $(SRCS) $(HDRS)
ifndef ATISTREAMSDKROOT
	@echo "error: ATI SDK not installed.\n" \
		"\tThe 'm2s-opencl-kc' tool has been included in the set of Multi2Sim tools\n" \
		"\tto compile OpenCL kernels. It requires the AMD SDK to be installed in\n" \
		"\tyour system. Please, visit www.amd.com/stream to download it." >&2
	@exit 1
else
	$(CC) $(CFLAGS) $(SRCS) -o $(EXE) $(LDFLAGS)
endif

clean:
	rm -f $(EXE)
