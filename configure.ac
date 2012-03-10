AC_INIT
AM_INIT_AUTOMAKE(multi2sim, 3.2.1)
AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_RANLIB

# Checks for header files.
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([fcntl.h float.h stddef.h stdint.h stdlib.h string.h sys/ioctl.h sys/socket.h sys/time.h unistd.h utime.h])
AC_CHECK_HEADER([elf.h], [AC_DEFINE([HAVE_ELF_H])], [AC_MSG_ERROR([cannot find header elf.h])])
AC_CHECK_HEADER([zlib.h], [AC_DEFINE([HAVE_ZLIB_H])], [AC_MSG_ERROR([cannot find header zlib.h])])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_INT8_T
AC_TYPE_OFF_T
AC_TYPE_SIZE_T
AC_CHECK_MEMBERS([struct stat.st_blksize])
AC_STRUCT_ST_BLOCKS
AC_CHECK_MEMBERS([struct stat.st_rdev])
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T


# Checks for library functions.
AC_FUNC_CHOWN
AC_FUNC_ERROR_AT_LINE
AC_FUNC_LSTAT_FOLLOWS_SLASHED_SYMLINK
AC_FUNC_MALLOC
AC_FUNC_MMAP
AC_FUNC_REALLOC
AC_CHECK_FUNCS([alarm bzero ftruncate getcwd gettimeofday memset mkdir munmap select socket strcasecmp strchr strdup strncasecmp strtol utime])

AC_CHECK_LIB([z], [inflate])
AC_CHECK_LIB([m], [sin])
AC_CHECK_LIB([pthread], [pthread_create], [LIBPTHREAD=true], [], [])

# Check for GTK
have_gtk=no
have_gtk_test=yes

ifdef([AM_PATH_GTK_3_0], [AM_PATH_GTK_3_0([3.0.0], have_gtk=yes, )], [have_gtk_test=no] )

if test x$have_gtk_test == xno; then
AC_MSG_WARN([
	
	The 'autoconf' tool could not interpret the macro checking for the
	presence of the GTK development library (libgtk-3-dev) in your system.
	Multi2Sim will be permanently compiled without support for GTK.
	Before releasing a version with this configuration (running 'make dist'),
	please install this package and rerun 'autoconf'. This will guarantee
	that the conditional inclusion of GTK will depend on the user configuration.
])
fi

if test x$have_gtk == xno; then
AC_MSG_WARN([

	Multi2Sim will be compiled without support for GTK graphical applications.
	Some features are disabled under this configuration, such as the GPU pipeline
	debugger. To enable it, please install the development packages for GTK on
	your system (e.g., package 'libgtk-3-dev' in Debian-based distributions),
	and run './configure' again.
])
fi

AM_CONDITIONAL(HAVE_GTK, test x$have_gtk = xyes)


DATADIR=`eval echo $datadir`
AC_SUBST([DATADIR])
PKGDATADIR="$datadir/multi2sim"
AC_SUBST([PKGDATADIR])


# ---------------------------------------------------------------------
# Debug and profile
# ---------------------------------------------------------------------
debug=false
CFLAGS="-O3 -std=gnu99 -DNDEBUG"
AC_ARG_ENABLE(debug,
[  --enable-debug    Turn on debugging],
[
	case "${enableval}" in
	yes)
		debug=true
		CFLAGS="-Wall -Werror -DMHANDLE -g -O0 -std=gnu99"
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


AC_CONFIG_FILES([Makefile
                 src/Makefile
		 src/libcpuarch/Makefile
		 src/libnetwork/Makefile
		 src/libgpuarch/Makefile
		 src/libgpukernel/Makefile
		 src/libgpudisasm/Makefile
		 src/libgpuvisual/Makefile
		 src/libmemsystem/Makefile
                 src/libcpukernel/Makefile
		 src/libcpudisasm/Makefile
		 src/libesim/Makefile
                 src/libmhandle/Makefile
                 src/libstruct/Makefile
		 src/libmisc/Makefile
		 ])
AC_OUTPUT

