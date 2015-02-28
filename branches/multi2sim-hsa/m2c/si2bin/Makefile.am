# Define these even if Flex/Bison are not present. Automake does not support
# conditional definition of 'AM_YFLAGS'.
AM_YFLAGS = -d -p si2bin_yy
AM_LFLAGS = -Psi2bin_yy -olex.yy.c

lib_LIBRARIES = libsi2bin.a

# This rule is needed for Automake 1.12 and up. These versions generate a
# file name Parser.hh instead of Parser.h. To make compilation work under any
# version of Automake, we make sure that any .hh file is first converted into
# a .h file with the same name.
.hh.h:
	cp $< $@

BUILT_SOURCES = Parser.h

libsi2bin_a_SOURCES = \
	\
	Argument.cc \
	Argument.h \
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
	Instruction.cc \
	Instruction.h \
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

AM_CPPFLAGS = @M2S_INCLUDES@

