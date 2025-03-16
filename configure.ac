#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([msc_retest], [1.0], [airween@gmail.com])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([macros])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

PCRE2_LDFLAGS="-lpcre2-8"
PCRE_OLD_LDFLAGS=""

# Checks for header files.
AC_DEFINE([PCRE2_CODE_UNIT_WIDTH], [8], [Define the PCRE2 code unit width])
AC_CHECK_HEADERS([pcre2.h], [], [AC_MSG_ERROR([unable to find header pcre2.h], 1)])
PKG_CHECK_MODULES([PCRE2], [libpcre2-8], [AC_SUBST([LIBPCRE2_LIB], [-lpcre2-8])],
    [AC_MSG_ERROR([PCRE2 library is missing])])
AX_CHECK_PCRE2([16,32], [], [])
AS_VAR_IF(pcre2_cv_libpcre2, [], [pcre2_cv_libpcre2="no"])

old_pcre_header_exists="no"
old_pcre_lib_exists="no"
AC_CHECK_HEADERS(
    [pcre.h],
    [
        AC_DEFINE([HAVE_OLD_PCRE_HEADER], [1], [Mark that system has old pcre header])
        old_pcre_header_exists="yes"
    ],
    [])
AC_CHECK_LIB(
    [pcre],
    [pcre_compile],
    [
        AC_DEFINE([HAVE_OLD_PCRE_LIB], [1], [Mark that system has old pcre libraries])
        old_pcre_lib_exists="yes"
        PCRE_OLD_LDFLAGS="-lpcre"
    ],
    [])
AC_CHECK_HEADERS([time.h], [], [AC_MSG_ERROR([unable to find header time.h], 1)])
AC_CHECK_HEADERS([math.h], [], [AC_MSG_ERROR([unable to find header math.h], 1)])

dnl Check if we want to use old PCRE engine or not
AC_MSG_CHECKING([whether to use old PCRE engine])
AC_ARG_WITH([old-pcre],
    [AS_HELP_STRING([--with-old-pcre], [Enable old pcre (default is enabled)])],
    [case "$withval" in
        yes) wantoldpcre=yes ;;
        no)  wantoldpcre=no ;;
        *)   AC_MSG_ERROR([Invalid value for --with-feature: $withval]) ;;
    esac],
    [wantoldpcre="notset"])


old_pcre_msg=""
if test "$old_pcre_header_exists" = "yes"; then
    if test "$old_pcre_lib_exists" = "yes"; then
        if test "$wantoldpcre" = "yes" || test "$wantoldpcre" = "notset"; then
            AC_DEFINE([WITH_OLD_PCRE], [1], [Enable old pcre])
            old_pcre_msg="yes"
            LIBS="-lpcre $LIBS"
        else
            old_pcre_msg="exists but not wanted"
        fi
    fi
else
    old_pcre_msg="PCRE3 doesn't exist"
fi

if test "$wantoldpcre" = "yes"; then
    if test "$old_pcre_header_exists" = "no"; then
        AC_MSG_ERROR(["Can't find PCRE3 headers"])
    fi
    if test "$old_pcre_lib_exists" = "no"; then
        AC_MSG_ERROR(["Can't find PCRE3 library"])
    fi
fi

# Checks for libraries.
AC_CHECK_LIB([rt], [clock_gettime], [], AC_MSG_ERROR([librt is not installed.], 1))
AC_CHECK_LIB([m], [powl], [], AC_MSG_ERROR([libm is not installed.], 1))

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T

AC_SUBST(PCRE2_LDFLAGS)
AC_SUBST(PCRE_OLD_LDFLAGS)

AC_CHECK_PROG([CPPCHECK], [cppcheck], [cppcheck], [no])
if test "x$CPPCHECK" = "xno"; then
   AC_MSG_WARN([Cppcheck is not available, skipping static analysis.])
fi

# Checks for library functions.

AC_CONFIG_FILES([Makefile src/Makefile])
AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])
AC_OUTPUT

echo \
"----------------------------------------------------------------------

 ${PACKAGE_NAME} Version ${PACKAGE_VERSION} configuration:

 OS Type             ${os_type}
 Prefix              ${prefix}
 Preprocessor        ${CPP} ${CPPFLAGS}
 C Compiler          ${CC} ${CFLAGS}
 CPPCHECK            ${CPPCHECK}
 PCRE2 support       ${pcre2_cv_libpcre2}
 PCRE (OLD) support  ${old_pcre_msg}
-----------------------------------------------------------------------"
