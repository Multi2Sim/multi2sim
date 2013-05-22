SUBDIRS = \
	amd \
	cl2llvm \
	llvm2si \
	si2bin

# The entire OpenCL compiler can only be built if g++ is installed on the
# system. This is done with conditional compilation based on HAVE_CXX
if HAVE_CXX
bin_PROGRAMS = $(top_builddir)/bin/m2s-clcc
endif


__top_builddir__bin_m2s_clcc_LINK = $(CXX) -o $@
__top_builddir__bin_m2s_clcc_SOURCES = \
	\
	clcc.c \
	clcc.h \
	\
	define.c \
	define.h

INCLUDES = @M2S_INCLUDES@

LDADD = \
	$(top_builddir)/tools/clcc/amd/libamd.a \
	$(top_builddir)/tools/clcc/cl2llvm/libcl2llvm.a \
	$(top_builddir)/tools/clcc/llvm2si/libllvm2si.a \
	$(top_builddir)/tools/clcc/si2bin/libsi2bin.a \
	$(top_builddir)/src/arch/southern-islands/asm/libasm.a \
	$(top_builddir)/src/lib/util/libutil.a \
	$(top_builddir)/src/lib/mhandle/libmhandle.a

if HAVE_OPENCL
LDADD += -lOpenCL \
	-L$(AMDAPPSDKROOT)/lib/x86 \
	-L$(AMDAPPSDKROOT)/lib/x86_64
endif

if HAVE_FLEX_BISON_AND_LLVM
LDADD += $(LLVM_LIBS) $(LLVM_LDFLAGS)
endif

