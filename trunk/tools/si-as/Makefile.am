# Define these even if Flex/Bison are not present. Automake does not support
# conditional definition of 'AM_YFLAGS'.
AM_YFLAGS = -d

# Only if Flex and Bison are present
if HAVE_FLEX_BISON

bin_PROGRAMS = $(top_builddir)/bin/m2s-si-as

BUILT_SOURCES = parser.h

__top_builddir__bin_m2s_si_as_SOURCES = \
	\
	arg.c \
	arg.h \
	\
	dis-inst.c \
	dis-inst.h \
	\
	dis-inst-info.c \
	dis-inst-info.h \
	\
	id.c \
	id.h \
	\
	main.c \
	main.h \
	\
	stream.c \
	stream.h \
	\
	symbol.c \
	symbol.h \
	\
	task.c \
	task.h \
	\
	token.c \
	token.h \
	\
	lex.l \
	parser.y

INCLUDES = @M2S_INCLUDES@

LDADD = \
	$(top_builddir)/src/arch/southern-islands/asm/libasm.a \
	$(top_builddir)/src/lib/util/libutil.a \
	$(top_builddir)/src/lib/mhandle/libmhandle.a

endif

