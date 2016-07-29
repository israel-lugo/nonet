#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.59])
AC_INIT([nonet], [0.1.0], [israel.lugo@lugosys.com])

# unique source file --- primitive safety check 
AC_CONFIG_SRCDIR([src/nonet.c])

# Checks for programs.
AC_PROG_CC

# Automake initialization
AM_INIT_AUTOMAKE([-Wall foreign])
AC_PROG_MAKE_SET

# Checks for libraries.

# Use pkg-config to look for check unit testing library. This sets CHECK_CFLAGS
# and CHECK_LIBS appropriately.
PKG_CHECK_MODULES([CHECK], [check])

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_TYPE_UID_T

# Checks for library functions.
AC_CHECK_FUNCS([strerror strtol])

# Output files
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile src/Makefile tests/Makefile])

AC_OUTPUT
