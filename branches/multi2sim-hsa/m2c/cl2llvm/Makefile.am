# Define these even if Flex/Bison are not present. Automake does not support
# conditional definition of 'AM_YFLAGS'.
AM_YFLAGS = -d -p cl2llvm_yy
AM_LFLAGS = -Pcl2llvm_yy -olex.yy.c

lib_LIBRARIES = libcl2llvm.a

BUILT_SOURCES = parser.h

libcl2llvm_a_SOURCES = \
	\
	arg.h \
	arg.c \
	\
	built-in-funcs.h \
	built-in-funcs.c \
	\
	cl2llvm.c \
	cl2llvm.h \
	\
	Context.cc \
	Context.h \
	\
	declarator-list.h \
	declarator-list.c \
	\
	enum-types.h \
	enum-types.c \
	\
	format.h \
	format.c \
	\
	for-blocks.c \
	for-blocks.h \
	\
	function.h \
	function.c \
	\
	init.c \
	init.h \
	\
	ret-error.c \
	ret-error.h \
	\
	symbol.c \
	symbol.h \
	\
	Type.cc \
	Type.h \
	\
	val.c \
	val.h \
	\
	vector-ops.c \
	vector-ops.h \
	\
	while-blocks.c \
	while-blocks.h \
	\
	Wrapper.c \
	Wrapper.h \
	\
	lex.l \
	parser.y

AM_CFLAGS = $(LLVM_CFLAGS)
AM_CXXFLAGS = $(LLVM_CXXFLAGS)
AM_CPPFLAGS = @M2S_INCLUDES@


# The two rules below overwrite the default rules generated to compile sources
# 'lex.c' and 'parser.c'. Some flex and bison versions generate code that
# produce warnings, so we need to avoid the use of flag '-Werror' in this case.

CFLAGS_NO_WERROR = `echo "$(CFLAGS)" | $(SED) "s/\-Werror\>//g"`

parser.o: CFLAGS := $(CFLAGS_NO_WERROR)

lex.o: CFLAGS := $(CFLAGS_NO_WERROR)

