lib_LIBRARIES = libllvm2si.a

if HAVE_LLVM

libllvm2si_a_SOURCES = \
	\
	llvm2si.c \
	llvm2si.h

AM_CFLAGS = $(LLVM_CFLAGS)

else

libllvm2si_a_SOURCES = \
	llvm2si-missing.c \
	llvm2si.h

endif

INCLUDES = @M2S_INCLUDES@

