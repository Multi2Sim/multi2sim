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
	@echo >&2
	@echo "error: AMD's APP software kit not installed or not detected." >&2
	@echo "    The 'm2s-opencl-kc' tool has been included in the set of Multi2Sim tools" >&2
	@echo "    to compile OpenCL kernels. It requires AMD's APP SDK to be installed in" >&2
	@echo "    your system. Make sure that the environment variable $$ATISTREAMSDKROOT" >&2
	@echo "    is set to the directory where the SDK was installed." >&2
	@echo "    Please, visit www.amd.com/stream to download it." >&2
	@echo >&2
	@exit 1
else
	$(CC) $(CFLAGS) $(SRCS) -o $(EXE) $(LDFLAGS)
endif

clean:
	rm -f $(EXE)
