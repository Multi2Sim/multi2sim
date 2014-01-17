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
	ctree.h \
	\
	BasicBlock.cc \
	BasicBlock.h \
	\
	Node.cc \
	Node.h \
	\
	Tree.cc \
	Tree.h \
	\
	Wrapper.cc \
	Wrapper.h

AM_CFLAGS = $(LLVM_CFLAGS)
AM_CXXFLAGS = $(LLVM_CXXFLAGS)

else

libcommon_a_SOURCES =

endif

INCLUDES = @M2S_INCLUDES@

