lib_LIBRARIES = libamd.a

if HAVE_OPENCL

libamd_a_SOURCES = \
	amd.c \
	amd.h

AM_CFLAGS = -I$(AMDAPPSDKROOT)/include

else

libamd_a_SOURCES = \
	amd-missing.c \
	amd.h

endif

INCLUDES = @M2S_INCLUDES@
