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
	Frm2binBinary.c \
	Frm2binBinary.h \
	\
	Frm2binBinaryGlobalInfo.c \
	Frm2binBinaryGlobalInfo.h \
	\
	Frm2binBinaryGlobalInfoItem.c \
	Frm2binBinaryGlobalInfoItem.h \
	\
	Frm2binBinaryKernel.c \
	Frm2binBinaryKernel.h \
	\
	Frm2binBinaryKernelInfo.c \
	Frm2binBinaryKernelInfo.h \
	\
	Id.cc \
	Id.h \
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
	Symbol.cc \
	Symbol.h \
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

AM_CPPFLAGS = @M2S_INCLUDES@


# The two rules below overwrite the default rules generated to compile sources
# 'lex.c' and 'parser.c'. Some flex and bison versions generate code that
# produce warnings, so we need to avoid the use of flag '-Werror' in this case.

CFLAGS_NO_WERROR = `echo "$(CFLAGS) -DMHANDLE_ALLOW_NON_X_FUNCTIONS" | $(SED) "s/\-Werror\>//g"`

parser.o: CFLAGS := $(CFLAGS_NO_WERROR)

lex.o: CFLAGS := $(CFLAGS_NO_WERROR)
