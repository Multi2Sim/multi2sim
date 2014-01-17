# Define these even if Flex/Bison are not present. Automake does not support
# conditional definition of 'AM_YFLAGS'.
AM_YFLAGS = -d -p si2bin_yy
AM_LFLAGS = -Psi2bin_yy -olex.yy.c

lib_LIBRARIES = libsi2bin.a

if HAVE_FLEX_BISON

#BUILT_SOURCES = parser.h
BUILT_SOURCES = Parser.h

libsi2bin_a_SOURCES = \
	\
	Arg.cc \
	Arg.h \
	\
	Binary.cc \
	Binary.h \
	\
	Context.cc \
	Context.h \
	\
	InternalBinary.cc \
	InternalBinary.h \
	\
	Inst.cc \
	Inst.h \
	\
	Metadata.cc \
	Metadata.h \
	\
	Symbol.cc \
	Symbol.h \
	\
	Task.cc \
	Task.h \
	\
	Token.cc \
	Token.h \
	\
	Lex.ll \
	Parser.yy

LDADD = \
	$(top_builddir)/src/arch/southern-islands/asm/libasm.a \
	$(top_builddir)/src/lib/util/libutil.a \
	$(top_builddir)/src/lib/mhandle/libmhandle.a

else

#libsi2bin_a_SOURCES = \
#	si2bin-missing.c \
#	si2bin.h

endif

INCLUDES = @M2S_INCLUDES@


# The two rules below overwrite the default rules generated to compile sources
# 'lex.c' and 'parser.c'. Some flex and bison versions generate code that
# produce warnings, so we need to avoid the use of flag '-Werror' in this case.

CFLAGS_NO_WERROR = `echo "$(CFLAGS)" | $(SED) "s/\-Werror\>//g"` -DMHANDLE_ALLOW_NON_X_FUNCTIONS

parser.o: CFLAGS := $(CFLAGS_NO_WERROR)

lex.o: CFLAGS := $(CFLAGS_NO_WERROR)


