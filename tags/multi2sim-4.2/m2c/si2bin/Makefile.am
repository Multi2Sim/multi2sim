# Define these even if Flex/Bison are not present. Automake does not support
# conditional definition of 'AM_YFLAGS'.
AM_YFLAGS = -d -p si2bin_yy
AM_LFLAGS = -P si2bin_yy -o lex.yy.c

lib_LIBRARIES = libsi2bin.a

if HAVE_FLEX_BISON

BUILT_SOURCES = parser.h

libsi2bin_a_SOURCES = \
	\
	arg.c \
	arg.h \
	\
	data.h \
	data.c \
	\
	inner-bin.c \
	inner-bin.h \
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
	metadata.c\
	metadata.h\
	\
	outer-bin.c \
	outer-bin.h \
	\
	si2bin.c \
	si2bin.h \
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

LDADD = \
	$(top_builddir)/src/arch/southern-islands/asm/libasm.a \
	$(top_builddir)/src/lib/util/libutil.a \
	$(top_builddir)/src/lib/mhandle/libmhandle.a

else

libsi2bin_a_SOURCES = \
	si2bin-missing.c \
	si2bin.h

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


