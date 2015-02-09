# The entire Multi2C compiler is only built if libraries LLVM, Flex, and Bison
# are present in the system.
if HAVE_FLEX_BISON_AND_LLVM

SUBDIRS = \
	amd \
	cl2llvm \
	common \
	frm2bin \
	gl \
	llvm2si \
	preprocessor \
	si2bin

bin_PROGRAMS = $(top_builddir)/bin/m2c

__top_builddir__bin_m2c_LINK = $(CXX) -o $@
__top_builddir__bin_m2c_SOURCES = \
	\
	m2c.cc \
	\
	Wrapper.cc \
	Wrapper.h


LDADD = \
	$(top_builddir)/m2c/amd/libamd.a \
	$(top_builddir)/m2c/cl2llvm/libcl2llvm.a \
	$(top_builddir)/m2c/frm2bin/libfrm2bin.a \
	$(top_builddir)/m2c/gl/libgl.a \
	$(top_builddir)/m2c/llvm2si/libllvm2si.a \
	$(top_builddir)/m2c/llvm2si/passes/libpasses.a \
	$(top_builddir)/m2c/preprocessor/libpreprocessor.a \
	$(top_builddir)/m2c/si2bin/libsi2bin.a \
	$(top_builddir)/m2c/common/libcommon.a \
	$(top_builddir)/src/arch/fermi/asm/libasm.a \
	$(top_builddir)/src/arch/southern-islands/asm/libasm.a \
	$(top_builddir)/src/arch/common/libcommon.a \
	$(top_builddir)/src/lib/class/libclass.a \
	$(top_builddir)/src/lib/util/libutil.a \
	$(top_builddir)/src/lib/cpp/libcpp.a \
	$(top_builddir)/src/lib/mhandle/libmhandle.a

# Reset flags
AM_CPPFLAGS = @M2S_INCLUDES@ $(LLVM_CXXFLAGS)
AM_LDFLAGS = ""
LDADD += $(LLVM_LIBS) $(LLVM_LDFLAGS)

if ENABLE_DEBUG
AM_LDFLAGS += -rdynamic
endif

if HAVE_OPENCL
LDADD += -lOpenCL \
	-L$(AMDAPPSDKROOT)/lib/x86 \
	-L$(AMDAPPSDKROOT)/lib/x86_64
endif

if HAVE_OPENGL
LDADD += -lX11 -lGL 
endif


endif  # HAVE_FLEX_BISON_AND_LLVM

