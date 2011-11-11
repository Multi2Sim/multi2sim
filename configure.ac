AC_INIT
AM_INIT_AUTOMAKE(multi2sim, 3.1.1)
: ${CFLAGS=""}
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

AC_CHECK_LIB([m], [sin])
AC_CHECK_LIB([pthread], [pthread_create], [LIBPTHREAD=true], [], [])

# Check for GTK
#AM_PATH_GTK_2_0([2.10.0], have_gtk=yes, have_gtk=no))



# ---------------------------------------------------------------------
# Debug and profile
# ---------------------------------------------------------------------
AC_ARG_ENABLE(debug,
[  --enable-debug    Turn on debugging],
[case "${enableval}" in
yes) debug=true ;;
no)  debug=false ;;
*) AC_MSG_ERROR(bad value ${enableval} for --enable-debug) ;;
esac],[debug=false])
AM_CONDITIONAL(DEBUG, test x$debug = xtrue)


AC_CONFIG_FILES([Makefile
                 src/Makefile
		 src/libcpuarch/Makefile
		 src/libnetwork/Makefile
		 src/libgpuarch/Makefile
		 src/libgpukernel/Makefile
		 src/libgpudisasm/Makefile
		 src/libcachesystem/Makefile
                 src/libcpukernel/Makefile
		 src/libcpudisasm/Makefile
		 src/libesim/Makefile
                 src/libmhandle/Makefile
                 src/libstruct/Makefile
		 src/libmisc/Makefile
		 ])
AC_OUTPUT
