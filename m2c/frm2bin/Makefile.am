# Define these even if Flex/Bison are not present. Automake does not support
# conditional definition of 'AM_YFLAGS'.
AM_YFLAGS = -d -p frm2bin_yy
AM_LFLAGS = -Pfrm2bin_yy -olex.yy.c

lib_LIBRARIES = libfrm2bin.a

if HAVE_FLEX_BISON

BUILT_SOURCES = parser.h

libfrm2bin_a_SOURCES = \
	\
	arg.c \
	arg.h \
	\
	id.c \
	id.h \
	\
	inst.c \
	inst.h \
	\
	inst-info.c \
	inst-info.h \
	\
	frm2bin.c \
	frm2bin.h \
	\
	symbol.c \
	symbol.h \
	\
	token.c \
	token.h \
	\
	inner-bin.c \
	inner-bin.h \
	\
	outer-bin.c \
	outer-bin.h \
	\
	lex.l \
	parser.y

LDADD = \
	$(top_builddir)/src/arch/fermi/asm/libasm.a \
	$(top_builddir)/src/lib/util/libutil.a \
	$(top_builddir)/src/lib/mhandle/libmhandle.a

else

libfrm2bin_a_SOURCES = \
	frm2bin-missing.c \
	frm2bin.h

endif

INCLUDES = @M2S_INCLUDES@


# The two rules below overwrite the default rules generated to compile sources
# 'lex.c' and 'parser.c'. Some flex and bison versions generate code that
# produce warnings, so we need to avoid the use of flag '-Werror' in this case.

CFLAGS_NO_WERROR = `echo "$(CFLAGS)" | $(SED) "s/\-Werror\>//g"`

lex.o: lex.c parser.h
	$(AM_V_CC) $(CC) -c -o lex.o lex.c $(DEFS) $(INCLUDES) $(CFLAGS_NO_WERROR) $(AM_CFLAGS)

parser.o: parser.c
	$(AM_V_CC) $(CC) -c -o parser.o parser.c $(DEFS) $(INCLUDES) $(CFLAGS_NO_WERROR) $(AM_CFLAGS)


