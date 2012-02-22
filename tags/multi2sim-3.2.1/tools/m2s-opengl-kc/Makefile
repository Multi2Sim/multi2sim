CC = gcc
CFLAGS = -Wall -I -g
LDFLAGS = -lGLEW -lGLU -lGL -lglut
INCLUDE_DIRS = -I./include
LIBRARY_DIRS = -L./lib

SRCS = \
	debug.c \
	misc.c \
	m2s-opengl-kc.c

HDRS = \
	debug.h \
	misc.h

EXE = m2s-opengl-kc

all: $(EXE)

$(EXE): $(SRCS) $(HDRS)
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) $(LIBRARY_DIRS) $(SRCS) -o $(EXE) $(LDFLAGS)

clean:
	rm -f $(EXE)
