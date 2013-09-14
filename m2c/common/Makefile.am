lib_LIBRARIES = libcommon.a

if HAVE_FLEX_BISON_AND_LLVM

libcommon_a_SOURCES = \
	\
	basic-block.c \
	basic-block.h \
	\
	node.c \
	node.h \
	\
	ctree.c \
	ctree.h

AM_CFLAGS = $(LLVM_CFLAGS)

else

libcommon_a_SOURCES =

endif

INCLUDES = @M2S_INCLUDES@

