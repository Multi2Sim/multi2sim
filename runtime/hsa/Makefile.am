if HAVE_M32_FLAG

lib_LTLIBRARIES = $(top_builddir)/lib/libm2s-hsa.la

__top_builddir__lib_libm2s_hsa_la_LDFLAGS = -version-info 1:0:0
		

__top_builddir__lib_libm2s_hsa_la_SOURCES = \
		debug.h \
		debug.c \
		\
		hsa.h \
		hsa.c \
		\
		hsa_agent.c \
		hsa_code_object.c \
		hsa_executable.c \
		hsa_finalize.c \
		hsa_isa.c \
		hsa_queue.c \
		hsa_region.c \
		hsa_signal.c \
		hsa_system.c 
	

AM_CFLAGS = -m32 -D_GNU_SOURCE
AM_CXXFLAGS = -m32

endif