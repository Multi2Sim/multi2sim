CC = gcc
CFLAGS = -Wall -O2
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
	@echo "warning: 'ncurses' library might not be present.\n" \
		"\tThe '/usr/include/ncurses.h' file was not found in your system. The\n" \
		"\tinstallation of the 'ncurses' library is a requirement to build the\n" \
		"\t'm2s-pipeline' tool. If the compilation fails, please install the development\n" \
		"\tversion of this package available for your Linux distribution.\n" \
		"\tHint: a Debian-based distribution should include the 'ncurses' library in\n" \
		"\tpackage 'libncurses-dev' or similar.\n" > &2
	@exit 1
endif
	$(CC) $(CFLAGS) $(SRCS) -o $(EXE) $(LDFLAGS)

clean:
	rm -f $(EXE)
