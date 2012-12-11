AC_INIT([multi2sim], [4.0.1])
AC_CONFIG_SRCDIR([src/m2s.c])
AM_INIT_AUTOMAKE([nostdinc])
AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_RANLIB

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

AC_CHECK_LIB([m], [sin])
AC_CHECK_LIB([pthread], [pthread_create])




##################
# Test flag '-m32'
##################

saved_flags="$CFLAGS"
CFLAGS="-m32"
AC_MSG_CHECKING([for -m32 compiler flag])
AC_LINK_IFELSE(
	[
		AC_LANG_PROGRAM(
			[[#include <stdio.h>]],
			[[int main() { printf("hello world\n"); return 0; }]]
		)
	],
	[
		AC_DEFINE([HAVE_M32_FLAG], [1])
		AC_MSG_RESULT(yes)
	],
	[
		AC_MSG_RESULT(no)
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

		sudo apt-get install lib32gcc1]
		)
	]
)
CFLAGS="$saved_flags"
AC_SUBST(HAVE_M32_FLAG)




#############
# Test OpenCL
#############

# Run an OpenCL test program
AC_MSG_CHECKING([for AMD OpenCL libraries])
have_opencl=no
if test "x$AMDAPPSDKROOT" != "x"
then
	saved_cflags="$CFLAGS"
	saved_ldflags="$LDFLAGS"
	CFLAGS="-I$AMDAPPSDKROOT/include"
	LDFLAGS="-L$AMDAPPSDKROOT/lib/x86 -L$AMDAPPSDKROOT/lib/x86_64 -lOpenCL"
	AC_LINK_IFELSE(
	[
		AC_LANG_PROGRAM(
			[[#include <CL/cl.h>]],
			[[int main() {
				cl_int ret;
				cl_platform_id platform;
				cl_uint num_platforms;
				ret = clGetPlatformIDs(1, &platform, &num_platforms);
			        return CL_SUCCESS;
			}]]
		)
	],
	[
		have_opencl=yes
	])
	CFLAGS="$saved_cflags"
	LDFLAGS="$saved_ldflags"
fi

# Warning if not
AC_MSG_RESULT($have_opencl)
if test "x$have_opencl" = "xno"
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





###########
# Test ZLib
###########

have_zlib=yes
AC_CHECK_HEADER([zlib.h], [AC_DEFINE([HAVE_ZLIB_H])], [have_zlib=no])
AC_CHECK_LIB([z], [inflate], [], [have_zlib=no])
if test x$have_zlib == xno
then
	AC_MSG_ERROR([
	
	The 'zlib' library development package is missing in your system.
	Please install the package and then retry. On Debian-based Linux
	distributions, you can run the following command:

		sudo apt-get install zlib1g-dev]
	)
fi




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

	The 'freeglut3' library development explicitly disabled. Some features in
	Multi2Sim will not be accessible.]
	)

else

	have_opengl=yes
	AC_CHECK_HEADER([GL/glut.h], [], [have_opengl=no])
	AC_CHECK_HEADER([GL/gl.h], [], [have_opengl=no])
	AC_CHECK_HEADER([GL/glu.h], [], [have_opengl=no])
	AC_CHECK_HEADER([GL/glew.h], [], [have_opengl=no])
	AC_CHECK_LIB([glut], [glutInit], [], [have_opengl=no])
	AC_CHECK_LIB([GL], [glBegin], [], [have_opengl=no])
	AC_CHECK_LIB([GLU], [gluOrtho2D], [], [have_opengl=no])
	AC_CHECK_LIB([GLEW], [glewInit], [], [have_opengl=no])

	if test "x$have_opengl" = "xno"
	then
		AC_MSG_WARN([
	
	The OpenGL/glut/glew libraries are not installed in your system. These
	libraries are needed to compile the simulation modules for OpenGL guest
	programs, and to build the OpenGL shader compiler in 'tools/glcc'.
	
	In Ubuntu, you can install the package by using the following command:

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
-I\$(top_srcdir)/src \
\
"

AC_SUBST(M2S_INCLUDES)




###########
# Makefiles
###########

AC_CONFIG_FILES([Makefile
                 src/Makefile

		 src/arch/Makefile
		 src/driver/Makefile
		 src/lib/Makefile
		 src/mem-system/Makefile
		 src/network/Makefile
		 src/visual/Makefile

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

		 src/arch/southern-islands/Makefile
		 src/arch/southern-islands/asm/Makefile
		 src/arch/southern-islands/emu/Makefile
		 src/arch/southern-islands/timing/Makefile

		 src/arch/x86/Makefile
		 src/arch/x86/asm/Makefile
		 src/arch/x86/emu/Makefile
		 src/arch/x86/timing/Makefile

		 src/driver/x86/Makefile
		 src/driver/x86/glut/Makefile

		 src/visual/common/Makefile
		 src/visual/evergreen/Makefile
		 src/visual/memory/Makefile
		 src/visual/southern-islands/Makefile
		 src/visual/x86/Makefile

		 src/lib/esim/Makefile
                 src/lib/mhandle/Makefile
                 src/lib/util/Makefile

		 tools/Makefile
		 tools/clcc/Makefile
		 tools/glcc/Makefile
		 ])
AC_OUTPUT

