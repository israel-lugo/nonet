#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([nonet], [0.1.0], [israel.lugo@lugosys.com])
AC_CONFIG_SRCDIR([src/nonet.c])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC

# Automake initialization
AM_INIT_AUTOMAKE([foreign])
AC_PROG_MAKE_SET

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_TYPE_UID_T

# Checks for library functions.
AC_CHECK_FUNCS([strerror strtol])

AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT
