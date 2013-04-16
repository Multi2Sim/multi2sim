SUBDIRS = \
	amd

bin_PROGRAMS = $(top_builddir)/bin/m2s-clcc

__top_builddir__bin_m2s_clcc_SOURCES = \
	\
	clcc.c \
	clcc.h

INCLUDES = @M2S_INCLUDES@

LDADD = \
	$(top_builddir)/tools/clcc/amd/libamd.a \
	$(top_builddir)/src/lib/util/libutil.a \
	$(top_builddir)/src/lib/mhandle/libmhandle.a

if HAVE_OPENCL
LDADD += -lOpenCL \
	-L$(AMDAPPSDKROOT)/lib/x86 \
	-L$(AMDAPPSDKROOT)/lib/x86_64
endif

