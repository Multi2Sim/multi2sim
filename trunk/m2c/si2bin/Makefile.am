# Define these even if Flex/Bison are not present. Automake does not support
# conditional definition of 'AM_YFLAGS'.
AM_YFLAGS = -d -p si2bin_yy
AM_LFLAGS = -Psi2bin_yy -olex.yy.c

lib_LIBRARIES = libsi2bin.a

if HAVE_FLEX_BISON

# This rule is needed for Automake 1.12 and up. These versions generate a
# file name Parser.hh instead of Parser.h. To make compilation work under any
# version of Automake, we make sure that any .hh file is first converted into
# a .h file with the same name.
.hh.h:
	mv $< $@

BUILT_SOURCES = Parser.h

libsi2bin_a_SOURCES = \
	\
	Arg.cc \
	Arg.h \
	\
	Binary.cc \
	Binary.h \
	\
	Config.cc \
	Config.h \
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
	si2bin.c \
	si2bin.h \
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
	Wrapper.cc \
	Wrapper.h \
	\
	Lex.ll \
	Parser.yy

LDADD = \
	$(top_builddir)/src/arch/southern-islands/asm/libasm.a \
	$(top_builddir)/src/lib/util/libutil.a \
	$(top_builddir)/src/lib/mhandle/libmhandle.a

else

libsi2bin_a_SOURCES = \
	Config-missing.cc \
	Config.h \
	si2bin-missing.c \
	si2bin.h

endif

INCLUDES = @M2S_INCLUDES@

