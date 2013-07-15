AC_INIT([multi2sim], [4.2])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([src/m2s.c])
AM_INIT_AUTOMAKE([nostdinc])
AM_PROG_AS
AC_PROG_CC
AC_PROG_CXX
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
		AC_LANG_PROGRAM([[#include <math.h>]], [[double x = sin(1.0);]])
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

	This feature is not needed if you are just indenting to run pre-compiled
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




#############
# Test OpenCL
#############

# Check if the user explicitly disabled OpenCL
AC_ARG_ENABLE(opencl,
	[  --enable-opencl={yes|no}	Enable support for OpenCL (default=yes)],
	[], [ enable_opencl=yes ])

# Check if OpenCL is present
have_opencl=no
if test "$enable_opencl" == "no"
then
	AC_MSG_WARN([

	Support for OpenCL has been explicitly disabled by a configuration flag.
	Some features of Multi2Sim will be disabled.]
	)

elif test "x$AMDAPPSDKROOT" != "x"
then
	# Use temporary CFLAGS, LDFLAGS
	saved_cflags="$CFLAGS"
	saved_ldflags="$LDFLAGS"
	saved_libs="$LIBS"
	CFLAGS="-I$AMDAPPSDKROOT/include"
	LDFLAGS="-L$AMDAPPSDKROOT/lib/x86 -L$AMDAPPSDKROOT/lib/x86_64"
	LIBS="-lOpenCL"

	# Try program
	AC_LINK_IFELSE(
	[
		AC_LANG_PROGRAM(
			[[#include <CL/cl.h>]],
			[[
				cl_int ret;
				cl_platform_id platform;
				cl_uint num_platforms;
				ret = clGetPlatformIDs(1, &platform, &num_platforms);
			        return CL_SUCCESS;
			]]
		)
	],
	[
		have_opencl=yes
	])

	# Restore original CFLAGS, LDFLAGS, LIBS
	CFLAGS="$saved_cflags"
	LDFLAGS="$saved_ldflags"
	LIBS="$saved_libs"
fi

# Warning if not
if test "x$have_opencl" = "xno" -a "x$enable_opencl" = "xyes"
then
	AC_MSG_WARN([
	
	The AMD OpenCL SDK is not presently installed in your system, as
	detected by the absence of the AMDAPPSDKROOT environment variable. The
	headers and libraries provided by this package are needed to build the
	command-line OpenCL kernel compiler provided by Multi2Sim.

	This feature is only needed if you are intending to compile your own
	OpenCL kernels, but not if you are using pre-compiled kernels available
	on the benchmark suites. To enable this feature, you need to download
	and install the AMD Catalyst GPU driver, as well as the AMD SDK.]
	)
fi

# Conditional for Makefile.am
AM_CONDITIONAL(HAVE_OPENCL, test x$have_opencl = xyes)




#############
# Test OpenGL
#############

# Check if the user explicitly disabled OpenGL
AC_ARG_ENABLE(opengl,
	[  --enable-opengl={yes|no}	Enable support for OpenGL (default=yes)],
	[], [ enable_opengl=yes ])

# Check if OpenGL is present
if test "$enable_opengl" == "no"
then

	have_opengl=no
	AC_MSG_WARN([

	Support for OpenGL has been explicitly disabled by a configuration flag.
	Some features of Multi2Sim will be disabled.]
	)

else

	# Assume that we have OpenGL
	have_opengl=yes

	# Check headers
	AC_CHECK_HEADER([GL/glut.h], [], [have_opengl=no])
	AC_CHECK_HEADER([GL/gl.h], [], [have_opengl=no])
	AC_CHECK_HEADER([GL/glu.h], [], [have_opengl=no])
	AC_CHECK_HEADER([GL/glew.h], [], [have_opengl=no])

	# Linker flags for OpenGL
	OPENGL_LDFLAGS="-lGL -lGLU -lGLEW -lglut"
	AC_SUBST([OPENGL_LDFLAGS])

	# Export temporary LDFLAGS
	saved_libs="$LIBS"
	LIBS="$OPENGL_LDFLAGS"

	# Try link
	AC_MSG_CHECKING([for $OPENGL_LDFLAGS])
	AC_LINK_IFELSE(
		[
			AC_LANG_PROGRAM(
				[[
					#include <GL/glut.h>
					#include <GL/gl.h>
					#include <GL/glu.h>
				]],
				[[
					glutInit(NULL, NULL);
					glBegin(0);
					gluOrtho2D(0.0, 0.0, 0.0, 0.0);
				]]
			)
		],
		[
			AC_MSG_RESULT(yes)
		],
		[
			AC_MSG_RESULT(no)
			have_opengl=no
		]
	)

	# Restore original LIBS
	LIBS="$saved_libs"

	# Message if not present
	if test "x$have_opengl" = "xno"
	then
		AC_MSG_WARN([
	
	Your system lacks at least one of the graphics libraries required to
	compile some features of Multi2Sim. These libraries are OpenGL, GLU,
	GLUT, and GLEW. Please make sure that your linker supports flags

		$OPENGL_LDFLAGS

	In Ubuntu, the following command is suggested to install the missing
	libraries:

		sudo apt-get install freeglut3-dev libglew-dev]
	)

	fi
fi

# Define macro for Automake and gcc
AM_CONDITIONAL(HAVE_OPENGL, test x$have_opengl == xyes -a x$enable_opengl == xyes)
if test x$have_opengl = xyes -a x$enable_opengl = xyes
then
	AC_DEFINE(HAVE_OPENGL, [1])
fi


#####################
# Test Flex and Bison
#####################

# Message
have_flex=no
have_bison=no

# Check Flex
AC_PROG_LEX
if test "x$LEX" = xflex
then
	have_flex=yes
fi

# Check Bison
AC_PROG_YACC
if test "x$YACC" != xyacc
then
	have_bison=yes
fi

# Check if the user explicitly disabled Flex/Bison
AC_ARG_ENABLE(flex-bison,
	[  --enable-flex-bison={yes|no}	Enable support for Flex/Bison (default=yes)],
	[ ], [ enable_flex_bison=yes ])
if test x$enable_flex_bison = xno
then
	have_flex=no
	have_bison=no
fi

# Define macro for Automake and gcc
AM_CONDITIONAL(HAVE_FLEX_BISON, test x$have_flex = xyes -a x$have_bison = xyes)
if test x$have_flex = xyes -a x$have_bison = xyes
then
	AC_DEFINE(HAVE_FLEX_BISON, [1])
elif test x$enable_flex_bison = xno
then
	AC_MSG_WARN([

	Support for tools 'flex' and 'bison' has been explicitly disabled with
	an argument in the ./configure script. Modules in the simulator
	dependent on these tools will not be built.]
	)
else
	AC_MSG_WARN([

	Tools 'flex' and/or 'bison' are missing in your system. Some modules in
	Multi2Sim will not be built. In an Ubuntu distribution, you can install
	these packages by running command

		sudo apt-get install flex bison]
	)
fi



###########
# Test LLVM
###########

have_llvm=yes

# Command-line option to force LLVM absence
AC_ARG_ENABLE(llvm,
	[  --enable-llvm={yes|no}	Enable support for LLVM (default=yes)],
	[], [ enable_llvm=yes ])
if test x$enable_llvm = xno
then
	have_llvm=no
fi

# Check for 'llvm-config' command
LLVM_CONFIG="llvm-config-3.1"
if test x$have_llvm = xyes
then
	which $LLVM_CONFIG > /dev/null || \
		have_llvm=no
fi

# Check for 'g++'
have_cxx=yes
which $CXX > /dev/null || have_cxx=no
if test x$have_cxx = xno
then
	have_llvm=no
fi
AM_CONDITIONAL(HAVE_CXX, test x$have_cxx = xyes)

# Try to compile a program using LLVM
if test x$have_llvm = xyes
then
	# Use temporary CXXFLAGS, LDFLAGS
	saved_cxxflags="$CXXFLAGS"
	saved_ldflags="$LDFLAGS"
	saved_libs="$LIBS"
	CXXFLAGS="`$LLVM_CONFIG --cflags`"
	LDFLAGS=""
	LIBS="`$LLVM_CONFIG --libs` `$LLVM_CONFIG --ldflags`"

	# Try program
	AC_LANG_PUSH([C++])
	AC_LINK_IFELSE(
	[
		AC_LANG_PROGRAM(
			[[
				#include <llvm-c/Core.h>
				#include <llvm-c/Analysis.h>
				#include <llvm-c/ExecutionEngine.h>
				#include <llvm-c/Target.h>
				#include <llvm-c/Transforms/Scalar.h>
			]],
			[[
				LLVMLinkInJIT();
				LLVMInitializeNativeTarget();
				LLVMBuilderRef builder = LLVMCreateBuilder();
				LLVMModuleRef mod = LLVMModuleCreateWithName("hello_module");
				LLVMTypeRef func_str_count_args[] = { LLVMPointerType(LLVMInt8Type(), 0) };
				LLVMValueRef func_str_count = LLVMAddFunction(mod, "str_count",
					LLVMFunctionType(LLVMInt32Type(), func_str_count_args, 1, 0));
				LLVMSetFunctionCallConv(func_str_count, LLVMCCallConv);

				/* Half type added in LLVM 3.1 */
				LLVMHalfType();
			]]
		)
	],
	[],
	[
		have_llvm=no
	])
	AC_LANG_POP([C++])

	# Restore original CXXFLAGS, LDFLAGS, LIBS
	CXXFLAGS="$saved_cxxflags"
	LDFLAGS="$saved_ldflags"
	LIBS="$saved_libs"
fi

# Define macro for Automake and gcc
AM_CONDITIONAL(HAVE_LLVM, test x$have_llvm = xyes)
AM_CONDITIONAL(HAVE_FLEX_BISON_AND_LLVM, test x$have_flex = xyes -a x$have_bison = xyes -a x$have_llvm = xyes)
if test x$have_llvm = xyes
then
	AC_DEFINE(HAVE_LLVM, [1])

	# Use the LLVM flags suggested by 'llvm-config', but don't let them
	# interfere with our own flags. Get rid of their -O2, -NDEBUG, and -g.
	# We will decide if we want those with our --enable-debug.
	LLVM_CFLAGS=`$LLVM_CONFIG --cflags | sed "s/\(\-DNDEBUG\>\)\|\(\-g\>\)\|\(\-O2\>\)//g"`

	LLVM_CXXFLAGS=`$LLVM_CONFIG --cxxflags`
	LLVM_LDFLAGS=`$LLVM_CONFIG --ldflags`
	LLVM_LIBS=`$LLVM_CONFIG --libs`
	AC_SUBST([LLVM_CFLAGS])
	AC_SUBST([LLVM_CXXFLAGS])
	AC_SUBST([LLVM_LDFLAGS])
	AC_SUBST([LLVM_LIBS])
elif test x$enable_llvm = xno
then
	AC_MSG_WARN([

	Support for LLVM has been explicitly disabled with an argument in the
	./configure script. Modules in the simulator dependent on these tools
	will not be built.]
	)
elif test x$have_cxx = xno
then
	AC_MSG_WARN([

	The C++ compiler g++ is not installed in your system. This compiler is
	needed to build LLVM-based modules on Multi2Sim. Please install g++, and
	recompile. In an Ubuntu distribution, you can run
	
		sudo apt-get install g++]
	)
else
	AC_MSG_WARN([

	LLVM support is missing in your system, or you have an older version of
	this library. Some modules in Multi2Sim will not be built. In an Ubuntu
	distribution, you can install these package by running command

		sudo apt-get install llvm-3.1]
	)
fi



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




###################
# Debug and profile
###################

debug=false
CFLAGS="-msse2 -O3 -std=gnu99 -DNDEBUG"
AC_ARG_ENABLE(debug,
[  --enable-debug    Turn on debugging],
[
	case "${enableval}" in
	yes)
		debug=true
		
		# Add flag '-Wno-error=deprecated-declarations' only if it is supported.
		# This flag prevents warnings caused by newer GTK versions that would cause with -Werror
		saved_flags="-msse2 -Wall -Werror -DMHANDLE -g -O0 -std=gnu99"
		CFLAGS="-Wno-error=deprecated-declarations"
		AC_MSG_CHECKING([whether CC supports -Wno-error=deprecated-declarations])
		AC_COMPILE_IFELSE([AC_LANG_PROGRAM([])],
				[ AC_MSG_RESULT([yes]) ]
				[ saved_flags="$saved_flags -Wno-error=deprecated-declarations" ],
				[ AC_MSG_RESULT([no]) ]
			)
		CFLAGS="$saved_flags"
		;;
	no)
		;;
	*)
		AC_MSG_ERROR(bad value ${enableval} for --enable-debug)
		;;
	esac
],
[])
AC_ARG_ENABLE(profile,
[  --enable-profile    Turn on profiling],
[
	case "${enableval}" in
	yes)
		CFLAGS+=" -pg"
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
		runtime/glew/Makefile
		runtime/glu/Makefile
		runtime/glut/Makefile
		runtime/opencl/Makefile
		runtime/opencl-old/Makefile
		runtime/opengl/Makefile
		runtime/include/Makefile

                src/Makefile

		src/arch/Makefile
		src/driver/Makefile
		src/lib/Makefile
		src/mem-system/Makefile
		src/network/Makefile
		src/dram/Makefile

		src/visual/Makefile
		src/visual/common/Makefile
		src/visual/evergreen/Makefile
		src/visual/memory/Makefile
		src/visual/southern-islands/Makefile
		src/visual/x86/Makefile

		src/arch/common/Makefile

		src/arch/arm/Makefile
		src/arch/arm/asm/Makefile
		src/arch/arm/emu/Makefile
		src/arch/arm/timing/Makefile

		src/arch/evergreen/Makefile
		src/arch/evergreen/asm/Makefile
		src/arch/evergreen/emu/Makefile
		src/arch/evergreen/timing/Makefile

		src/arch/fermi/Makefile
		src/arch/fermi/asm/Makefile
		src/arch/fermi/emu/Makefile
		src/arch/fermi/timing/Makefile

		src/arch/kepler/Makefile
		src/arch/kepler/asm/Makefile

		src/arch/mips/Makefile
		src/arch/mips/asm/Makefile
		src/arch/mips/emu/Makefile
		src/arch/mips/timing/Makefile

		src/arch/southern-islands/Makefile
		src/arch/southern-islands/asm/Makefile
		src/arch/southern-islands/emu/Makefile
		src/arch/southern-islands/timing/Makefile

		src/arch/x86/Makefile
		src/arch/x86/asm/Makefile
		src/arch/x86/emu/Makefile
		src/arch/x86/timing/Makefile

		src/driver/cuda/Makefile
		src/driver/glu/Makefile
		src/driver/glut/Makefile
		src/driver/glew/Makefile
		src/driver/opencl/Makefile
		src/driver/opencl-old/Makefile
		src/driver/opencl-old/evergreen/Makefile
		src/driver/opengl/Makefile

		src/lib/esim/Makefile
		src/lib/mhandle/Makefile
		src/lib/util/Makefile

		m2c/Makefile
		m2c/amd/Makefile
		m2c/common/Makefile
		m2c/cl2llvm/Makefile
		m2c/frm2bin/Makefile
		m2c/gl/Makefile
		m2c/llvm2si/Makefile
		m2c/si2bin/Makefile
		])
AC_OUTPUT

AC_CONFIG_MACRO_DIR([m4])
