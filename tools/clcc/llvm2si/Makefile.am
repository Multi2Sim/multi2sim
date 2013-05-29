lib_LIBRARIES = libllvm2si.a

if HAVE_LLVM

libllvm2si_a_SOURCES = \
	\
	basic-block.c \
	basic-block.h \
	\
	llvm2si.c \
	llvm2si.h \
	\
	translate.cc \
	translate.h

AM_CFLAGS = $(LLVM_CFLAGS)
AM_CXXFLAGS = $(LLVM_CXXFLAGS)

else

libllvm2si_a_SOURCES = \
	llvm2si-missing.c \
	llvm2si.h

endif

INCLUDES = @M2S_INCLUDES@

