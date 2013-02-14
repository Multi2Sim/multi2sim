# Define these even if Flex/Bison are not present. Automake does not support
# conditional definition of 'AM_YFLAGS'.
AM_YFLAGS = -d

# Only if Flex and Bison are present
if HAVE_FLEX_BISON

bin_PROGRAMS = $(top_builddir)/bin/m2s-si-as

BUILT_SOURCES = parser.h

__top_builddir__bin_m2s_si_as_SOURCES = \
	\
	dis-inst.c \
	dis-inst.h \
	\
	id.c \
	id.h \
	\
	arg.c \
	arg.h \
	\
	label.c \
	label.h \
	\
	main.c \
	\
	stream.c \
	stream.h \
	\
	task.c \
	task.h \
	\
	\
	lex.l \
	parser.y

INCLUDES = @M2S_INCLUDES@

LDADD = \
	$(top_builddir)/src/arch/southern-islands/asm/libasm.a \
	$(top_builddir)/src/lib/util/libutil.a \
	$(top_builddir)/src/lib/mhandle/libmhandle.a

endif

