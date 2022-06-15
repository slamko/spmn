#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([spm], [0.0.2], [vaceslavsck@ukr.net])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_CONFIG_SRCDIR([include/def.h])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h stdint.h stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_TYPE_PID_T
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_REALLOC
AC_FUNC_STRNLEN
AC_CHECK_FUNCS([floor memset strerror strndup strstr])

AC_CONFIG_FILES([
    Makefile
    src/Makefile
    src/utils/Makefile
    src/commands/Makefile
])

AC_OUTPUT
