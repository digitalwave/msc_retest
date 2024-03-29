#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([mscretest], [0.5], [airween@gmail.com])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([macros])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

# Checks for header files.

AC_CHECK_HEADERS([pcre.h], [], [AC_MSG_ERROR([unable to find header pcre.h], 1)])
AC_CHECK_HEADERS([time.h], [], [AC_MSG_ERROR([unable to find header time.h], 1)])
AC_CHECK_HEADERS([math.h], [], [AC_MSG_ERROR([unable to find header math.h], 1)])

# Checks for libraries.
AC_CHECK_LIB([pcre], [pcre_compile], [], AC_MSG_ERROR([libpcre is not installed.], 1))
AC_CHECK_LIB([rt], [clock_gettime], [], AC_MSG_ERROR([librt is not installed.], 1))
AC_CHECK_LIB([m], [powl], [], AC_MSG_ERROR([libm is not installed.], 1))

AX_CHECK_PCRE2([16,32], [], [])
AS_VAR_IF(pcre2_cv_libpcre2, [], [pcre2_cv_libpcre2="no"])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T

# Checks for library functions.

AC_CONFIG_FILES([Makefile src/Makefile])
AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])
AC_OUTPUT

echo \
"----------------------------------------------------------------------

 ${PACKAGE_NAME} Version ${PACKAGE_VERSION} configuration:

 OS Type        ${os_type}
 Prefix         ${prefix}
 Preprocessor   ${CPP} ${CPPFLAGS}
 C Compiler     ${CC} ${CFLAGS}
 PCRE2 support  ${pcre2_cv_libpcre2}
-----------------------------------------------------------------------"
