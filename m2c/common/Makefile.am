lib_LIBRARIES = libcommon.a

libcommon_a_SOURCES = \
	\
	basic-block.c \
	basic-block.h \
	\
	cnode.c \
	cnode.h \
	\
	ctree.c \
	ctree.h

if HAVE_LLVM
AM_CFLAGS = $(LLVM_CFLAGS)
endif

INCLUDES = @M2S_INCLUDES@

