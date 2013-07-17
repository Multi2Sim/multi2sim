SUBDIRS = \
	amd \
	cl2llvm \
	common \
	frm2bin \
	gl \
	llvm2si \
	si2bin

# The entire OpenCL compiler can only be built if g++ is installed on the
# system. This is done with conditional compilation based on HAVE_CXX
if HAVE_CXX
bin_PROGRAMS = $(top_builddir)/bin/m2c
endif


__top_builddir__bin_m2c_LINK = $(CXX) -o $@
__top_builddir__bin_m2c_SOURCES = \
	m2c.c

INCLUDES = @M2S_INCLUDES@

LDADD = \
	$(top_builddir)/m2c/amd/libamd.a \
	$(top_builddir)/m2c/cl2llvm/libcl2llvm.a \
	$(top_builddir)/m2c/frm2bin/libfrm2bin.a \
	$(top_builddir)/m2c/gl/libgl.a \
	$(top_builddir)/m2c/llvm2si/libllvm2si.a \
	$(top_builddir)/m2c/si2bin/libsi2bin.a \
	$(top_builddir)/m2c/common/libcommon.a \
	$(top_builddir)/src/arch/fermi/asm/libasm.a \
	$(top_builddir)/src/arch/southern-islands/asm/libasm.a \
	$(top_builddir)/src/lib/util/libutil.a \
	$(top_builddir)/src/lib/mhandle/libmhandle.a

if HAVE_LLVM
AM_CFLAGS = $(LLVM_CFLAGS)
endif

if HAVE_OPENCL
LDADD += -lOpenCL \
	-L$(AMDAPPSDKROOT)/lib/x86 \
	-L$(AMDAPPSDKROOT)/lib/x86_64
endif

if HAVE_OPENGL
LDADD += -lX11 -lGL 
endif

if HAVE_FLEX_BISON_AND_LLVM
LDADD += $(LLVM_LIBS) $(LLVM_LDFLAGS)
endif

