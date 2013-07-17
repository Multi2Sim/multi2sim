lib_LIBRARIES = libutil.a

libutil_a_SOURCES = \
	\
	bin-config.c \
	bin-config.h \
	\
	bit-map.c \
	bit-map.h \
	\
	buffer.c \
	buffer.h \
	\
	class.c \
	class.h \
	\
	config.c \
	config.h \
	\
	debug.c \
	debug.h \
	\
	elf-encode.c \
	elf-encode.h \
	\
	elf-format.c \
	elf-format.h \
	\
	file.c \
	file.h \
	\
	hash-table.c \
	hash-table.h \
	\
	heap.c \
	heap.h \
	\
	list.c \
	list.h \
	\
	linked-list.c \
	linked-list.h \
	\
	misc.c \
	misc.h \
	\
	matrix.c \
	matrix.h \
	\
	repos.c \
	repos.h \
	\
	string.c \
	string.h \
	\
	timer.c \
	timer.h

INCLUDES = @M2S_INCLUDES@

# For 'misc.c', we need the data directory to find distribution files.
AM_CFLAGS = -DPACKAGE_DATA_DIR='"$(pkgdatadir)"'

