AC_INIT([multi2sim], [5.0])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([src/m2s.cc])
AM_INIT_AUTOMAKE([nostdinc subdir-objects])
AM_PROG_AS
AC_PROG_CC([gcc])
AC_PROG_INSTALL

# Silent rules if supported
ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

# Libtool
LT_INIT
AC_PROG_LIBTOOL

# Checks for header files.
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([fcntl.h float.h stddef.h stdint.h stdlib.h string.h sys/ioctl.h sys/socket.h sys/time.h unistd.h utime.h])
AC_CHECK_HEADER([elf.h], [AC_DEFINE([HAVE_ELF_H])], [AC_MSG_ERROR([cannot find header elf.h])])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_OFF_T
AC_TYPE_SIZE_T
AC_CHECK_MEMBERS([struct stat.st_blksize])
AC_STRUCT_ST_BLOCKS
AC_CHECK_MEMBERS([struct stat.st_rdev])


# Checks for library functions.
AC_FUNC_CHOWN
AC_FUNC_ERROR_AT_LINE
AC_FUNC_LSTAT_FOLLOWS_SLASHED_SYMLINK
AC_FUNC_MALLOC
AC_FUNC_MMAP
AC_FUNC_REALLOC
AC_CHECK_FUNCS([alarm bzero ftruncate getcwd gettimeofday memset mkdir munmap select socket strcasecmp strchr strdup strncasecmp strtol utime])





#############################
# Note on tests for libraries
#############################

# We avoid using macro AC_CHECK_LIB, because it automatically updates variable
# LIBS, which is later used by any linker in the source tree. This is a problem
# when compiling runtimes with the -m32 flags. If LIBS are added, and the system
# has only the 64-bit versions, compilation fails.
# Thus, we perform tests manually by using AC_LINK_IFELSE and AC_LANG_PROGRAM.
# Then libraries are added only when generating the 'm2s' executable using
# LDFLAGS.



################################
# Check presence of C++ compiler
################################

AC_PROG_CXX([g++])
if test "$CXX" = no || test "$CXX:$GXX" = "g++:"
then
	AC_MSG_ERROR([

	The g++ compiler is not installed in your machine. Please install a
	valid C++11 compiler and retry. In Ubuntu, the following command is
	suggested:

		sudo apt-get install g++]
	)
fi



###############################################################################
# Test of GCC version >= 4.8 (support for C++11). Even though GCC 4.7 already
# supports C++11, some standard library features are not available, which are
# required in Multi2Sim. An example is functions std::map::emplace() or
# std::unordered_map::emplace().
###############################################################################

# Check for option -dumpversion
AC_MSG_CHECKING([for option -dumpversion])
$CXX -dumpversion >/dev/null 2>&1
if test $? -ne 0
then
	AC_MSG_ERROR([

	The version of the GCC compiler could not be determined, since option
	-dumpversion is not available. Please install GCC 4.8 or later in your
	system to compile Multi2Sim.]
	)
fi
AC_MSG_RESULT(yes)

# Get GCC version
AC_MSG_CHECKING([for GCC version])
cxx_version=`$CXX -dumpversion`
AC_MSG_RESULT($cxx_version)

# Check version
cxx_major_version=`echo $cxx_version | cut -d. -f1`
cxx_minor_version=`echo $cxx_version | cut -d. -f2`
if test \( $cxx_major_version -lt 4 \) -o \( $cxx_major_version -eq 4 -a $cxx_minor_version -lt 8 \)
then
	AC_MSG_ERROR([

	The version of the GCC compiler must be 4.8 or higher to support the
	C++11 code introduced in Multi2Sim. Please install a more recent version
	of GCC and recompile.]
	)
fi




###############
# Debug/Profile
###############

debug=false
CFLAGS="-msse2 -O3 -std=gnu99 -DNDEBUG"
CXXFLAGS="-msse2 -std=gnu++0x -O3 -DNDEBUG"
AC_ARG_ENABLE(debug,
[  --enable-debug    Turn on debugging],
[
	case "${enableval}" in
	yes)
		debug=true
		
		# Add flag '-Wno-error=deprecated-declarations' only if it is supported.
		# This flag prevents warnings caused by newer GTK versions that would cause with -Werror
		CXXFLAGS="-msse2 -std=gnu++0x -Wall -Werror -g -O0"
		saved_flags="-msse2 -Wall -Werror -DMHANDLE -g -O0 -std=gnu99"
		CFLAGS="-Wno-error=deprecated-declarations"
		AC_MSG_CHECKING([whether CC supports -Wno-error=deprecated-declarations])
		AC_COMPILE_IFELSE([AC_LANG_PROGRAM([])],
				[ AC_MSG_RESULT([yes]) ]
				[ saved_flags="$saved_flags -Wno-error=deprecated-declarations" ],
				[ AC_MSG_RESULT([no]) ]
			)
		CFLAGS="$saved_flags"
		enable_debug="yes"
		AC_SUBST([ENABLE_DEBUG], 1)
		;;
	no)
		enable_debug="no"
		;;
	*)
		AC_MSG_ERROR(bad value ${enableval} for --enable-debug)
		;;
	esac
],
[])
AM_CONDITIONAL(ENABLE_DEBUG, test x$enable_debug = xyes)

profile=false
AC_ARG_ENABLE(profile,
[  --enable-profile    Turn on profiling],
[
	case "${enableval}" in
	yes)
		CFLAGS+=" -pg"
		CXXFLAGS+=" -pg"
		profile=true
		;;
	no)
		;;
	*)
		AC_MSG_ERROR(bad value ${enableval} for --enable-profile)
		;;
	esac
],
[])

AC_ARG_ENABLE(no-strict-aliasing,
[  --enable-no-strict-aliasing    Allow for no strict aliasing],
[
	case "${enableval}" in
	yes)
		CFLAGS+=" -fno-strict-aliasing"
		;;
	no)
		;;
	*)
		AC_MSG_ERROR(bad value ${enableval} for --enable-no-strict-aliasing)
		;;
	esac
],
[])
AC_SUBST([CFLAGS])




################
# Test for 'sed'
################

AC_MSG_CHECKING([for sed])
SED=`which sed`
if test $? == 0
then
	AC_MSG_RESULT($SED)
	AC_SUBST(SED)
else
	AC_MSG_RESULT(no)
	AC_MSG_ERROR([

	Program 'sed' could not be found in your system. Please install this
	command-line tool and retry compilation.]
	)
fi


###############
# Test for libm
###############

AC_CHECK_HEADER([math.h], [], [AC_MSG_ERROR([cannot find header math.h])])
AC_MSG_CHECKING([for -lm])

# Use new LDFLAGS
saved_libs="$LIBS"
LIBS="-lm"

# Try program
AC_LINK_IFELSE(
	[
		AC_LANG_PROGRAM([[#include <math.h>]], [[return sin(1.0);]])
	],
	[
		AC_MSG_RESULT(yes)
	],
	[
		AC_MSG_RESULT(no)
		AC_MSG_ERROR([
	
	The math library (libmath) is not present in your system. Please make
	sure that your linker supports the -lm flag.]
		)
	]
)

# Restore original LDFLAGS
LIBS="$saved_libs"




#####################
# Test for libpthread
#####################

AC_CHECK_HEADER([pthread.h], [], [AC_MSG_ERROR([cannot find header pthread.h])])
AC_MSG_CHECKING([for -lpthread])

# Use temporary LDFLAGS
saved_libs="$LIBS"
LIBS="-lpthread"

# Try
AC_LINK_IFELSE(
	[
		AC_LANG_PROGRAM([[int pthread_create();]], [[return pthread_create();]])
	],
	[
		AC_MSG_RESULT(yes)
	],
	[
		AC_MSG_RESULT(no)
		AC_MSG_ERROR([

	The POSIX threads library (libpthread) is not present in your system.
	Please make sure that your linked supports the -lpthread flag.]
		)
	]
)

# Restore original LIBS
LIBS="$saved_libs"




###########
# Test ZLib
###########

have_zlib=yes
AC_CHECK_HEADER([zlib.h], [], [have_zlib=no])
AC_MSG_CHECKING([for -lz])

# Use temporary LIBS
saved_libs="$LIBS"
LIBS="-lz"

# Try them
AC_LINK_IFELSE(
	[
		AC_LANG_PROGRAM([[int inflate();]], [[return inflate();]])
	],
	[],
	[
		have_zlib=no
	]
)

# Restore original LIBS
LIBS="$saved_libs"

# Result
AC_MSG_RESULT($have_zlib)
if test x$have_zlib == xno
then
	AC_MSG_ERROR([
	
	The 'zlib' library development package is missing in your system.
	Please install the package and then retry. In Ubuntu, the following
	command is suggested:

		sudo apt-get install zlib1g-dev]
	)
fi




###################
# Test SSE4 Support
###################

have_sse4=no
AC_MSG_CHECKING([for SSE4 instructions support])

# Compile a test program with SSE4 assembly
AC_LINK_IFELSE(
	[ AC_LANG_PROGRAM([[]], [[asm volatile( "pcmpeqq %xmm0, %xmm1\n\t");]])	],
	[ have_sse4=yes ],
	[]
)

# Result
AC_MSG_RESULT($have_sse4)
if test x$have_sse4 == xno
then
	AC_MSG_WARN([

	Your CPU does not have support for x86 SSE4 instructions. Emulation of
	these instructions will not be supported. The module in the x86 emulator
	dealing with SSE4 instruction will not be compiled.]
	)
else
	AC_DEFINE(HAVE_SSE4, [1])
fi




###############################
# Test for __sync_XXX built-ins
###############################

saved_flags="$CFLAGS"
CFLAGS="-Wall -Werror"
AC_MSG_CHECKING([for __sync_XXX built-in functions])
have_sync_builtins="no"
AC_LINK_IFELSE(
	[
		AC_LANG_PROGRAM(
			[[ ]],
			[[
				int a = 0, b = 0;
				__sync_fetch_and_add(&a, b);
				return 0;
			]]
		)
	],
	[
		have_sync_builtins="yes"
	]
)
AC_MSG_RESULT($have_sync_builtins)
CFLAGS="$saved_flags"

AM_CONDITIONAL(HAVE_SYNC_BUILTINS, test x$have_sync_builtins = xyes)
if test x$have_sync_builtins = xyes
then
	AC_DEFINE(HAVE_SYNC_BUILTINS, [1])
fi




##################
# Test flag '-m32'
##################

saved_flags="$CFLAGS"
CFLAGS="-m32"
AC_MSG_CHECKING([for -m32 compiler flag])
have_m32_flag="no"
AC_LINK_IFELSE(
	[
		AC_LANG_PROGRAM(
			[[#include <stdio.h>]],
			[[ printf("hello world\n"); ]]
		)
	],
	[
		have_m32_flag="yes"
	]
)
AC_MSG_RESULT($have_m32_flag)
CFLAGS="$saved_flags"

# Show warning
if test "x$have_m32_flag" = "xno"
then
	AC_MSG_WARN([

	Your compiler does not support 32-bit compilation using the '-m32' flag.
	This mode is needed to compile the Multi2Sim runtime libraries in
	directory 'runtime', which you can later link with your OpenCL, OpenGL,
	GLUT, ... guest programs.

	This feature is not needed if you are just intending to run pre-compiled
	benchmarks. You need it only to compile your custom benchmarks that use
	any of the Multi2Sim runtimes.

	If you want to support this feature, please install the 32-bit gcc
	libraries. For example, this can be done in Ubuntu 12.04 by running
	command:

		sudo apt-get install lib32gcc1 gcc-multilib]
	)
fi

# Conditional for Makefile.am
AM_CONDITIONAL(HAVE_M32_FLAG, test x$have_m32_flag = xyes)




#####################
# Set GTest Variables
#####################
GTEST_VERSION="1.7.0"
GTEST_CPPFLAGS="-DGTEST_HAS_PTHREAD=1"
GTEST_CXXFLAGS=""
GTEST_LDFLAGS=
GTEST_LIBS="-lpthread -pthread"
AC_SUBST([GTEST_VERSION])
AC_SUBST([GTEST_CPPFLAGS])
AC_SUBST([GTEST_CXXFLAGS])
AC_SUBST([GTEST_LDFLAGS])
AC_SUBST([GTEST_LIBS])
	


##########
# Test GTK
##########

have_gtk=no
have_gtk_test=yes

# Check if the user explicitly disabled GTK
AC_ARG_ENABLE(gtk,
	[  --enable-gtk={yes|no}	Enable support for GTK (default=yes)],
	[], [ enable_gtk=yes ])

# Check if test for GTK is available
ifdef([AM_PATH_GTK_3_0], [AM_PATH_GTK_3_0([3.0.0], have_gtk=yes, )], [have_gtk_test=no] )
if test x$have_gtk_test = xno
then
	AC_MSG_WARN([
	
	The 'autoconf' tool could not interpret the macro checking for the
	presence of the GTK development library (libgtk-3-dev) in your system.
	Multi2Sim will be permanently compiled without support for GTK.
	Before releasing a version with this configuration (running 'make dist'),
	please install this package and rerun 'autoconf'. This will guarantee
	that the conditional inclusion of GTK will depend on the user configuration.]
	)
fi

# Check if GTK is available
if test x$have_gtk = xno
then
	AC_MSG_WARN([

	Multi2Sim will be compiled without support for GTK graphical applications.
	Some features are disabled under this configuration, such as the GPU pipeline
	debugger. To enable it, please install the development packages for GTK on
	your system (e.g., package 'libgtk-3-dev' in Debian-based distributions),
	and run './configure' again.]
	)
fi

# Check if GTK is enabled
if test x$enable_gtk = xno
then
	AC_MSG_WARN([

	GTK is present in your system, but you have explicitly disabled support
	for it. The GTK-based subset of Multi2Sim will not be available in the
	generated executable.]
	)
fi

# Define macro for Automake and gcc
AM_CONDITIONAL(HAVE_GTK, test x$have_gtk = xyes -a x$enable_gtk = xyes)
if test x$have_gtk = xyes -a x$enable_gtk = xyes
then
	AC_DEFINE(HAVE_GTK, [1])
fi



#
# Data directory
#

DATADIR=`eval echo $datadir`
AC_SUBST([DATADIR])
PKGDATADIR="$datadir/multi2sim"
AC_SUBST([PKGDATADIR])




##########
# Includes
##########

M2S_INCLUDES=" \
\
-I\$(top_srcdir) \
-I\$(top_srcdir)/src \
\
"

AC_SUBST(M2S_INCLUDES)




###########
# Makefiles
###########

AC_CONFIG_FILES([
		Makefile
		 
		runtime/Makefile
		runtime/cuda/Makefile
		runtime/hsa/Makefile
		runtime/opencl/Makefile
		runtime/include/Makefile

		src/Makefile

		src/arch/Makefile
		src/lib/Makefile
		src/memory/Makefile
		src/network/Makefile
		src/dram/Makefile

		src/visual/Makefile
		src/visual/common/Makefile
		src/visual/memory/Makefile
		src/visual/network/Makefile
		src/visual/evergreen/Makefile
		src/visual/southern-islands/Makefile
		src/visual/x86/Makefile

		src/arch/common/Makefile

		src/arch/arm/Makefile
		src/arch/arm/disassembler/Makefile
		src/arch/arm/emulator/Makefile
		src/arch/arm/timing/Makefile

		src/arch/hsa/Makefile
		src/arch/hsa/disassembler/Makefile
		src/arch/hsa/driver/Makefile
		src/arch/hsa/emulator/Makefile

		src/arch/kepler/Makefile
		src/arch/kepler/disassembler/Makefile
		src/arch/kepler/driver/Makefile
		src/arch/kepler/emulator/Makefile

		src/arch/mips/Makefile
		src/arch/mips/disassembler/Makefile
		src/arch/mips/emulator/Makefile
		src/arch/mips/timing/Makefile

		src/arch/southern-islands/Makefile
		src/arch/southern-islands/disassembler/Makefile
		src/arch/southern-islands/driver/Makefile
		src/arch/southern-islands/emulator/Makefile
		src/arch/southern-islands/timing/Makefile

		src/arch/x86/Makefile
		src/arch/x86/disassembler/Makefile
		src/arch/x86/emulator/Makefile
		src/arch/x86/timing/Makefile

		src/lib/cpp/Makefile
		src/lib/esim/Makefile
		src/lib/gtest/Makefile
		src/lib/mhandle/Makefile
		src/lib/util/Makefile

		tests/Makefile
		])
AC_OUTPUT

AC_CONFIG_MACRO_DIR([m4])
