dnl Copyright (c) 2009      Cisco Systems, Inc.  All rights reserved.
dnl Copyright (c) 2020      High-Performance Computing Center Stuttgart,
dnl                         University of Stuttgart. All rights reserved.

dnl Process this file with autoconf to produce a configure script.
AC_INIT([mpi_test_suite], [1.0])
AM_INIT_AUTOMAKE([-Wall -Werror foreign no-dependencies subdir-objects])
AC_PREREQ(2.59)

AC_CONFIG_SRCDIR([mpi_test_suite.c])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIRS([m4])

AC_GNU_SOURCE
AC_PROG_CPP
AX_PROG_CC_MPI(,,[
    AC_MSG_FAILURE([MPI compiler required.])
])

AC_ARG_ENABLE(mpi2,
  AS_HELP_STRING([--enable-mpi2], [Check for MPI2 features [[default=yes]]]),
  [test "$enableval" = "no" && ENABLE_MPI2=no], ENABLE_MPI2=yes)

AC_ARG_ENABLE(mpi2-oneside,
  AS_HELP_STRING([--enable-mpi2-oneside], [build checks for MPI2 onesided Communication [[default=yes]]]),
  [test "$enableval" = "no" && ENABLE_MPI2_ONESIDED=no], ENABLE_MPI2_ONESIDED=yes)

AC_ARG_ENABLE(mpi2-io,
  AS_HELP_STRING([--enable-mpi2-io], [build checks for MPI2 dynamic process management [[default=yes]]]),
  [test "$enableval" = "no" && ENABLE_MPI2_IO=no], ENABLE_MPI2_IO=yes)


dnl Checks for header files and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_C_RESTRICT
AC_HEADER_STDC

dnl The second argument is IGNORED!!!!!
AC_CHECK_SIZEOF(char, 1)
AC_CHECK_SIZEOF(short, 2)
AC_CHECK_SIZEOF(int, 4)
AC_CHECK_SIZEOF(long, 4)
AC_CHECK_SIZEOF(long long, 8)
AC_CHECK_SIZEOF(unsigned short, 2)
AC_CHECK_SIZEOF(unsigned int, 4)
AC_CHECK_SIZEOF(unsigned long, 8)
AC_CHECK_SIZEOF(unsigned long long, 8)
AC_CHECK_SIZEOF(float, 4)
AC_CHECK_SIZEOF(double, 8)
AC_CHECK_SIZEOF(long double, 8)

dnl Check for headers
dnl Need to check for sys/types.h since AC_TYPE_PID_T depends on it later!
AC_CHECK_HEADERS([float.h getopt.h limits.h stdlib.h unistd.h sys/time.h sys/types.h values.h pthread.h])

dnl Check for sizes of different types and Endian-ness
dnl AC_C_LONG_DOUBLE
dnl AC_C_BIGENDIAN


dnl Checks for library functions.
dnl AC_FUNC_MEMCMP
AC_FUNC_MALLOC
AC_FUNC_VPRINTF
dnl AC_CHECK_FUNCS([kill memset snprintf strcasecmp strerror strstr setlinebuf gethostname select socket poll vsprintf vsnprintf])
AC_CHECK_FUNCS([gethostname memset strcasecmp strerror strstr])


AC_CONFIG_FILES([Makefile])

AS_MKDIR_P([coll])
AS_MKDIR_P([dynamic])
AS_MKDIR_P([env])
AS_MKDIR_P([io])
AS_MKDIR_P([one-sided])
AS_MKDIR_P([p2p])
AS_MKDIR_P([threaded])

AC_OUTPUT
