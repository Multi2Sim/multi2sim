CC = gcc
CFLAGS = -Wall -O2 -Wfatal-errors
LDFLAGS = -lncurses

SRCS = \
	m2s-pipeline.c

#HDRS = \
#	debug.h \
#	elfanalyze.h \
#	misc.h

EXE = m2s-pipeline

$(EXE): $(SRCS) $(HDRS)
ifeq ($(findstring ncurses.h, $(wildcard /usr/include/*.h)), )
	@echo >&2
	@echo "warning: 'ncurses' library might not be present." >&2
	@echo "    The '/usr/include/ncurses.h' file was not found in your system. The" >&2
	@echo "    installation of the 'ncurses' library is a requirement to build the" >&2
	@echo "    'm2s-pipeline' tool. If the compilation fails, please install the development" >&2
	@echo "    version of this package available for your Linux distribution." >&2
	@echo "    Hint: a Debian-based distribution should include the 'ncurses' library in" >&2
	@echo "    package 'libncurses-dev' or similar." >&2
	@echo >&2
endif
	$(CC) $(CFLAGS) $(SRCS) -o $(EXE) $(LDFLAGS)

clean:
	rm -f $(EXE)
