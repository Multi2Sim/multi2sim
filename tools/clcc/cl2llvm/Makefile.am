# Define these even if Flex/Bison are not present. Automake does not support
# conditional definition of 'AM_YFLAGS'.
AM_YFLAGS = -d -p cl2llvm_yy
AM_LFLAGS = -P cl2llvm_yy -o lex.yy.c

lib_LIBRARIES = libcl2llvm.a

if HAVE_FLEX_BISON_AND_LLVM

BUILT_SOURCES = parser.h

libcl2llvm_a_SOURCES = \
	arg.h\
	arg.c\
	\
	cl2llvm.c \
	cl2llvm.h \
	\
	declarator-list.h\
	declarator-list.c\
	\
	function.h\
	function.c\
	\
	init.c\
	init.h\
	\
	symbol.c \
	symbol.h \
	\
	type.c\
	type.h\
	\
	val.c\
	val.h\
	\
	lex.l \
	parser.y

AM_CFLAGS = $(LLVM_CFLAGS)

else

libcl2llvm_a_SOURCES = \
	cl2llvm-missing.c \
	cl2llvm.h

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


