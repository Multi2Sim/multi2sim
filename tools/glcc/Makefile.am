if HAVE_OPENGL

bin_PROGRAMS = $(top_builddir)/bin/m2s-glcc

__top_builddir__bin_m2s_glcc_SOURCES = \
	glcc.c

INCLUDES = @M2S_INCLUDES@

LDADD = \
	$(top_builddir)/src/lib/util/libutil.a \
	$(top_builddir)/src/lib/mhandle/libmhandle.a \
	-lX11 -lGL 

endif

