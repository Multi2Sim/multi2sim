if HAVE_OPENCL

bin_PROGRAMS = $(top_builddir)/bin/m2s-clcc

__top_builddir__bin_m2s_clcc_SOURCES = \
	clcc.c

INCLUDES = @M2S_INCLUDES@

LDADD = \
	-L$(AMDAPPSDKROOT)/lib/x86 \
	-L$(AMDAPPSDKROOT)/lib/x86_64 \
	$(top_builddir)/src/lib/util/libutil.a \
	$(top_builddir)/src/lib/mhandle/libmhandle.a \
	-lOpenCL

AM_CFLAGS = -I$(AMDAPPSDKROOT)/include
	

endif

