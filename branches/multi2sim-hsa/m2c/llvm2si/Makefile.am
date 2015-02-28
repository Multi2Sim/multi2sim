SUBDIRS = \
	passes

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
	Module.cc \
	Module.h \
	\
	Symbol.cc \
	Symbol.h

AM_CFLAGS = $(LLVM_CFLAGS)
AM_CXXFLAGS = $(LLVM_CXXFLAGS)
AM_CPPFLAGS = @M2S_INCLUDES@

