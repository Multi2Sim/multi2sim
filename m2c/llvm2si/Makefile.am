lib_LIBRARIES = libllvm2si.a

if HAVE_FLEX_BISON_AND_LLVM

libllvm2si_a_SOURCES = \
	\
	basic-block.c \
	basic-block.h \
	\
	BasicBlock.cc \
	BasicBlock.h \
	\
	Builder.cc \
	Builder.h \
	\
	function.c \
	function.h \
	\
	Function.cc \
	Function.h \
	\
	llvm2si.c \
	llvm2si.h \
	\
	phi.c \
	phi.h \
	\
	Phi.cc \
	Phi.h \
	\
	symbol.c \
	symbol.h \
	\
	Symbol.cc \
	Symbol.h \
	\
	symbol-table.c \
	symbol-table.h \
	\
	Wrapper.cc \
	Wrapper.h

AM_CFLAGS = $(LLVM_CFLAGS)
AM_CXXFLAGS = $(LLVM_CXXFLAGS)

else

libllvm2si_a_SOURCES = \
	llvm2si-missing.c \
	llvm2si.h

endif

INCLUDES = @M2S_INCLUDES@

