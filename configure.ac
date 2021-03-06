#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])
AC_INIT([xio], [0.9], [yp.fangdong@gmail.com])
AC_CONFIG_SRCDIR([src])
AC_CONFIG_HEADERS(config.h)
AM_INIT_AUTOMAKE([0.9 foreign subdir-objects tar-ustar dist-zip])

m4_ifdef([AM_SILENT_RULES], [
    AM_SILENT_RULES([yes])
])


# Checks for programs.
AC_PROG_LIBTOOL
AC_PROG_CXX
AC_PROG_CC_C99
AM_PROG_CC_C_O
AC_PROG_INSTALL
AC_PROG_MAKE_SET

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lpthread':
AC_CHECK_LIB([pthread], [pthread_create])
# FIXME: Replace `main' with a function in `-luuid':
AC_CHECK_LIB([uuid], [uuid_copy])

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h fcntl.h inttypes.h malloc.h netdb.h netinet/in.h stdint.h stdlib.h string.h sys/socket.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_INT8_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([getcwd gettimeofday memmove memset socket strchr strdup strndup strrchr strstr])

#  If --enable-lua is specified, building xio modules for lua.
AC_ARG_ENABLE([lua],
    AS_HELP_STRING([--enable-lua], [xio extension for lua])
)
AM_CONDITIONAL([LUA], [test "$enable_lua" = "yes"])
AS_IF([test "$enable_lua" = "yes"], [
    AC_CHECK_LIB([lua], [lua_pushnumber])
])

AS_IF([test $GCC = yes], [
    AC_DEFINE([HAVE_GCC], [], [gcc compiler support])
])


#  If --enable-debug is specified, set the HAVE_DEBUG macro.
AC_ARG_ENABLE([debug],
    [AS_HELP_STRING([--enable-debug], [enable debugging information [default=yes]])]
)

if test "$enable_debug" = "yes"; then
    if test "$GCC" = "yes"; then
        CFLAGS="-g -O0 "
    fi
    AC_DEFINE([HAVE_DEBUG], [], [enable debug compile mode])
fi


AC_CONFIG_FILES([Makefile])
AC_OUTPUT
