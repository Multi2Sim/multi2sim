lib_LIBRARIES = libllvm2si.a

libllvm2si_a_SOURCES = \
	\
	BasicBlock.cc \
	BasicBlock.h \
	\
	Context.cc \
	Context.h \
	\
	Function.cc \
	Function.h \
	\
	InterferenceGraph.cc \
	InterferenceGraph.h \
	\
	llvm2si.c \
	llvm2si.h \
	\
	Symbol.cc \
	Symbol.h \
	\
	Wrapper.cc \
	Wrapper.h

AM_CFLAGS = $(LLVM_CFLAGS)
AM_CXXFLAGS = $(LLVM_CXXFLAGS)
AM_CPPFLAGS = @M2S_INCLUDES@

