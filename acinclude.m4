dnl
dnl Get the format of Fortran names.
dnl May not be converted to AC_CACHE_CHECK, since we need the AC_DEFINE for the
dnl various Fortran naming conventions -- this can't be done within second arg to cache check.
dnl
dnl Dependancies: AC_PROG_F77
dnl
AC_DEFUN([AC_CHECK_FORTRAN_NAME_CONVENTION],[
  AC_REQUIRE([AC_PROG_GREP])
  AC_MSG_CHECKING(for Fortran naming convention)
  AC_LANG_PUSH([Fortran 77])
  AC_COMPILE_IFELSE([AC_LANG_SOURCE([
      subroutine mpir_init_fop()
      return
      end
  ])],
  [
  if nm conftest.$ac_objext | $GREP mpir_init_fop__ >/dev/null 2>&1 ; then
    ac_cv_fortran_name_convention="FORTRANDOUBLEUNDERSCORE"
    AWK_COMMAND_PACX='[{printf("#define %s %s__\n",$][2,tolower($][2))}]'
    AWK_COMMAND_PPACX='[{printf("#define %s p%s__\n",$][2,tolower($][2))}]'
    AC_DEFINE([FORTRANDOUBLEUNDERSCORE], 1, [Defined if name convention of Fortran uses "lowercase__"])
    AC_MSG_RESULT(Fortran externals are lower case and have two trailing underscores)
  elif nm conftest.$ac_objext | $GREP mpir_init_fop_ >/dev/null 2>&1 ; then
    # We do not set this in CFLAGS; it is a default case
    ac_cv_fortran_name_convention="FORTRANUNDERSCORE"
    AWK_COMMAND_PACX='[{printf("#define %s %s_\n",$][2,tolower($][2))}]'
    AWK_COMMAND_PPACX='[{printf("#define %s p%s_\n",$][2,tolower($][2))}]'
    AC_DEFINE([FORTRANUNDERSCORE], 1, [Defined if name convention of Fortran uses "lowercase_"])
    AC_MSG_RESULT(Fortran externals are lower case and have one trailing underscore)
  elif nm conftest.$ac_objext | $GREP MPIR_INIT_FOP >/dev/null 2>&1 ; then
    ac_cv_fortran_name_convention="FORTRANCAPS"
    AWK_COMMAND_PACX='[{printf("#define %s %s\n",$][2,toupper($][2))}]'
    AWK_COMMAND_PPACX='[{printf("#define %s P%s\n",$][2,toupper($][2))}]'
    AC_DEFINE([FORTRANCAPS], 1, [Defined if name convention of Fortran uses "UPPERCASE"])
    AC_MSG_RESULT(Fortran externals are uppercase)
  elif nm conftest.$ac_objext | $GREP mpir_init_fop >/dev/null 2>&1 ; then
    ac_cv_fortran_name_convention="FORTRANNOUNDERSCORE"
    AWK_COMMAND_PACX='[{printf("#define %s %s\n",$][2,tolower($][2))}]'
    AWK_COMMAND_PPACX='[{printf("#define %s p%s\n",$][2,tolower($][2))}]'
    AC_DEFINE([FORTRANNOUNDERSCORE], 1, [Defined if name convention of Fortran uses "lowercase"])
    AC_MSG_RESULT(Fortran externals are lower case)
  else
    # We do not set this in CFLAGS; it is a default case
    ac_cv_fortran_name_convention="FORTRANUNDERSCORE"
    AWK_COMMAND_PACX='[{printf("#define %s %s_\n",$][2,tolower($][2))}]'
    AWK_COMMAND_PPACX='[{printf("#define %s p%s_\n",$][2,tolower($][2))}]'
    AC_DEFINE([FORTRANUNDERSCORE], 1, [Defined if name convention of Fortran uses "lowercase_"])
    AC_MSG_RESULT(Unable to determine the form of Fortran external names)
    AC_MSG_RESULT(Choosing the default: lower case symbols without underscores)
  fi
  ],
  [AC_MSG_ERROR(Could not compile test-program to figure out Fortran-naming convention)])

  AC_LANG_POP([Fortran 77])
])




dnl
dnl Check for the Fortran MPI-Compiler
dnl
dnl Dependancies: AC_PROG_F77
dnl
AC_DEFUN([AC_PROG_MPIF], [
  AC_CACHE_CHECK([for the MPI Fortran compiler], [ac_cv_prog_mpif],
  [
    dnl This gets hard -- we are looking for a F77 compiler, which supports MPI!
    dnl We have to jump through a hoop, to get it running on all systems!
    AC_LANG_PUSH([Fortran 77])

    ac_progs="mpif90 mpif77 mpf90 mpf77"
    for ac_prog in $ac_progs ; do
      if test -x $mpi_dir/bin/$ac_prog$ac_exec_ext ; then

        ac_cv_prog_mpif=$mpi_dir/bin/$ac_prog$ac_exec_ext
        ac_compile="$ac_cv_prog_mpif -c conftest.$ac_ext >&AS_MESSAGE_LOG_FD"

        AC_COMPILE_IFELSE(AC_LANG_SOURCE([
        implicit none
        include 'mpif.h'
        integer ierror
        call MPI_INIT ( ierror )
        call MPI_FINALIZE ( ierror )
        end
        ]),[
          test -n "$ac_cv_prog_mpif" && break
        ])
      fi
    done

    if test -z "$ac_cv_prog_mpif" ; then
      dnl This is our last resort -- do it by hand
      dnl This might mean, that the test-program will not run on the cpu, configure runs on!
      ac_cv_prog_mpif="$F77 $FFLAGS -I${mpi_inc_dir}"
    fi
    ac_cv_mpif_compile="$ac_cv_prog_mpif -c conftest.$ac_ext >&AS_MESSAGE_LOG_FD"
    ac_cv_mpif_link="$ac_cv_prog_mpif -o conftest$ac_exeext conftest.$ac_ext $LIBS >&AS_MESSAGE_LOG_FD"
    AC_LANG_POP([Fortran 77])
  ])
])



dnl
dnl Check for the C MPI-Compiler
dnl
dnl Dependancies: AC_PROG_CC
dnl
AC_DEFUN([AC_PROG_MPICC], [
  AC_CACHE_CHECK([for the MPI C compiler], [ac_cv_prog_mpicc],
  [
    ac_cv_prog_mpicc=""
    if test -x $mpi_dir/bin/mpicc ; then
      ac_cv_prog_mpicc="$mpi_dir/bin/mpicc"
      ac_cv_mpicc_compile="$ac_cv_prog_mpicc -c conftest.$ac_ext >&AS_MESSAGE_LOG_FD"
      ac_cv_mpicc_link="$ac_cv_prog_mpicc -o conftest$ac_exeext conftest.$ac_ext $LIBS $LDFLAGS >&AS_MESSAGE_LOG_FD"
    elif test -x $mpi_dir/bin/mpcc ; then
      dnl SUN has got a stupid naming convention!
      dnl PLEASE notice the -l${lib_mpi} -- are they stupid or what ?
      ac_cv_prog_mpicc="$mpi_dir/bin/mpcc"
      ac_cv_mpicc_compile="$ac_cv_prog_mpicc -c conftest.$ac_ext >&AS_MESSAGE_LOG_FD"
      ac_cv_mpicc_link="$ac_cv_prog_mpicc -o conftest$ac_exeext conftest.$ac_ext -l${lib_mpi} $LIBS >&AS_MESSAGE_LOG_FD"
    else
      dnl This is our last resort -- do it by hand
      dnl This might mean, that the test-program will not run on the cpu, configure runs on!
      ac_cv_prog_mpicc="$CC"
      ac_cv_mpicc_compile="$CC $CFLAGS -I${mpi_inc_dir} -c conftest.$ac_ext -L${mpi_lib_dir} -l${lib_mpi} >&AS_MESSAGE_LOG_FD"
      ac_cv_mpicc_link="$CC $CFLAGS -I${mpi_inc_dir} -o conftest$ac_exeext conftest.$ac_ext -L${mpi_lib_dir} -l${lib_mpi} $LDFLAGS $SYS_LDFLAGS >&AS_MESSAGE_LOG_FD"
    fi
  ])
])


dnl
dnl The following two tests are very similar in structure.
dnl Both test for optional datatypes in the MPI standard
dnl
dnl We set several cache_values:
dnl   ac_cv_have_mpi_fortran_{MPI-DATATYPE} to 'yes' if we have the datatype
dnl   ac_cv_{MPI-DATATYPE} to ' '(space) if we have the datatype and
dnl                        to   'C' as comment, if we don't have it, used in Fortran header-files
dnl
dnl Dependancies: AC_PROG_MPIF
dnl
AC_DEFUN([AC_CHECK_FORTRAN_MPI_DATATYPE], [
  AC_CACHE_CHECK([whether MPI has (opt.) Fortran [$1]], [ac_cv_have_mpi_fortran_[$1]],
  [
    AC_LANG_PUSH([Fortran 77])
    dnl ac_ext=f
    dnl ac_compile=$ac_cv_mpif_compile
    dnl ac_link=$ac_cv_mpif_link

    AC_TRY_RUN([
        implicit none
        include 'mpif.h'
        open (unit=44, file='conftestval')
        write (44,*) ' ', $1
        close (44)
        end
    ], [
    dnl MPIch defines those Variables, but sets them to 0, if they are not available.
      VAL=`cat ./conftestval`
      rm -f conftestval
      if test $VAL = 0 ; then
        ac_cv_have_mpi_fortran_$1="no"
      else
        ac_cv_have_mpi_fortran_$1="yes"
      fi
    ], [ac_cv_have_mpi_fortran_$1="no"])

    AC_LANG_POP([Fortran 77])

    dnl OK, we specify a constant for each optional Fortran variable (MPI_INTEGER4),
    dnl which replaces the variable in src/pacx2/PACX_get_c_type.f.in
    dnl Next, we have to declare something, in order to get the variable type declared.
    if test "x$ac_cv_have_mpi_fortran_$1" = "xyes" ; then
      [$2]
      AC_CACHE_VAL([ac_cv_$1], [ac_cv_$1=" "])
    else
      [$3]
      AC_CACHE_VAL([ac_cv_$1], [ac_cv_$1="C"])
    fi
  ])
  AC_SUBST([ac_cv_$1])
])



dnl
dnl Test for optional C-datatypes in the MPI standard
dnl
dnl We set several cache_values:
dnl   ac_cv_have_mpi_c_{MPI-DATATYPE} to 'yes' if we have the datatype
dnl   ac_cv_{MPI-DATATYPE} to ' '(space) if we have the optional C-datatype and
dnl                           'C' as comment, if we don't have it, used in Fortran header-files
dnl
dnl Dependancies: AC_PROG_MPIC
dnl
AC_DEFUN([AC_CHECK_C_MPI_DATATYPE],
  [
  AC_CACHE_CHECK([whether MPI has (opt.) C [$1]], [ac_cv_have_mpi_c_[$1]],
  [
    AC_LANG_SAVE()
    ac_ext=c
    ac_compile=$ac_cv_mpicc_compile
    ac_link=$ac_cv_mpicc_link
    AC_TRY_RUN([
#     include <stdio.h>
#     include "mpi.h"
      int main() {
        FILE *f=fopen("conftestval","w");
        if (!f) return 1;
        fprintf(f, "%d\n", $1);
        return 0;
      }
    ], [
    dnl MPIch defines those Variables, but sets them to 0, if they are not available.
      VAL=`cat ./conftestval`
      rm -f conftestval
      if test $VAL = 0 ; then
        ac_cv_have_mpi_c_$1="no"
      else
        ac_cv_have_mpi_c_$1="yes"
      fi
    ], [ac_cv_have_mpi_c_$1="no"])
    if test "x$ac_cv_have_mpi_c_$1" = "xyes" ; then
      [$2]
      AC_CACHE_VAL([ac_cv_$1], [ac_cv_$1=" "])
    else
      [$3]
      AC_CACHE_VAL([ac_cv_$1], [ac_cv_$1="C"])
    fi
    AC_LANG_RESTORE()
  ])
  AC_SUBST([ac_cv_$1])
])


dnl
dnl Check for MPI2's one-sided communication
dnl
dnl This check for support is really lousy,
dnl just check, whether the test-case compiles.
dnl
AC_DEFUN([AC_CHECK_MPI2_ONE_SIDED],
  [
  AC_CACHE_CHECK([whether MPI supports one-sided communication], [ac_cv_have_mpi2_one_sided],
  [
    AC_LANG_SAVE()
    ac_ext=c
    ac_compile=$ac_cv_mpicc_compile
    ac_link=$ac_cv_mpicc_link
    AC_TRY_COMPILE([
#     include <stdio.h>
#     include "mpi.h"
],[
      int a;
      MPI_Win win;

      MPI_Win_create (&a, sizeof(a), 1, MPI_INFO_NULL, MPI_COMM_WORLD, &win);
      return 0;
    ], [ac_cv_have_mpi2_one_sided="yes"], [ac_cv_have_mpi2_one_sided="no"])
    AC_LANG_RESTORE()
  ])
  if test "x$ac_cv_have_mpi2_one_sided" = "xyes" ; then
    AC_DEFINE([HAVE_MPI2_ONE_SIDED], 1, [Define to support MPI2's one-sided communication])
  fi
])


dnl
dnl Check for MPI2's dynamic process creation
dnl
dnl This check for support is really lousy,
dnl just check, whether the test-case compiles.
dnl
AC_DEFUN([AC_CHECK_MPI2_DYNAMIC_PROCESSES],
  [
  AC_CACHE_CHECK([whether MPI supports dynamic process management], [ac_cv_have_mpi2_dynamic_processes],
  [
    AC_LANG_SAVE()
    ac_ext=c
    ac_compile=$ac_cv_mpicc_compile
    ac_link=$ac_cv_mpicc_link
    AC_TRY_COMPILE([
#     include <stdio.h>
#     include "mpi.h"
],[
      int a;
      MPI_Comm comm;

      MPI_Comm_get_parent (&comm);
      return 0;
    ], [ac_cv_have_mpi2_dynamic_processes="yes"], [ac_cv_have_mpi2_dynamic_processes="no"])
    AC_LANG_RESTORE()
  ])
  if test "x$ac_cv_have_mpi2_dynamic_processes" = "xyes" ; then
    AC_DEFINE([HAVE_MPI2_DYNAMIC_PROCESSES], 1, [Define to support MPI2's dynamic process management])
  fi
])

dnl
dnl Check for MPI2's io 
dnl
AC_DEFUN([AC_CHECK_MPI2_IO],[
  AC_CACHE_CHECK([whether MPI supports MPI2 io], [ac_cv_have_mpi2_io],
  [
    AC_LANG_SAVE()
    ac_ext=c
    ac_compile=$ac_cv_mpicc_compile
    ac_link=$ac_cv_mpicc_link

    AC_TRY_LINK([
#     include <stdio.h>
#     include "mpi.h"
],[
            MPI_File file;
            MPI_File_open(MPI_COMM_WORLD, "aaa", MPI_MODE_RDWR, MPI_INFO_NULL, &file);
          ],[ac_cv_have_mpi2_io="yes"], [ac_cv_have_mpi2_io="no"])
    AC_LANG_RESTORE()
  ])
  if test "x$ac_cv_have_mpi2_io" = "xyes" ; then
    AC_DEFINE([HAVE_MPI2_IO], 1, [Define to support MPI2's Parallel IO])
  fi
])


dnl
dnl Check for MPI2's thread initialization functions
dnl
dnl This check for support is really lousy,
dnl just check, whether the test-case compiles.
dnl
AC_DEFUN([AC_CHECK_MPI2_THREADS],
  [
  if test -x $mpi_dir/bin/ompi_info ; then
    AC_CACHE_CHECK([whether Open MPI supports threads], [ac_cv_have_mpi2_threads], [
        ac_cv_have_mpi2_threads=`$mpi_dir/bin/ompi_info --parseable | grep option:threads: | cut -d\  -f3 |cut -d, -f1`
    ])
  else
    AC_CACHE_CHECK([whether MPI supports thread initialization], [ac_cv_have_mpi2_threads],
    [
        AC_LANG_SAVE()
        ac_ext=c
        ac_compile=$ac_cv_mpicc_compile
        ac_link=$ac_cv_mpicc_link
        AC_TRY_COMPILE([
#     include <stdio.h>
#     include "mpi.h"
],[
      int provided;
      int my_argc = 0;
      char * my_argv[] = {NULL};
      MPI_Init_threads (&my_argc, &my_argv, MPI_THREAD_MULTIPLE, &provided);
      return 0;
    ], [ac_cv_have_mpi2_threads="yes"], [ac_cv_have_mpi2_threads="no"])
    AC_LANG_RESTORE()
  ])
  fi
  if test "x$ac_cv_have_mpi2_threads" = "xyes" ; then
    AC_DEFINE([HAVE_MPI2_THREADS], 1, [Define to support MPI2's thread initialization])
  fi
])


dnl
dnl Check for size of Fortran datatypes
dnl This is very tricky: We first compile a Fortran-object code, which contains a
dnl an array of two elements of the datatype in question and
dnl a call to a C-function, which takes the two elements as arguments.
dnl Fortran is "Call-By-Reference", therefore the C function finds the "size", better extent
dnl of the datatype in question.
dnl
dnl Dependancies: AC_PROG_CC, AC_PROG_F77, AC_CHECK_FORTRAN_NAME_CONVENTION
dnl
AC_DEFUN([AC_CHECK_SIZEOF_FORTRAN], [
  AC_CACHE_CHECK([size of Fortran [$1]], [ac_cv_sizeof_fortran_[]translit($1, [A-Z *], [a-z_p])],[
    AC_LANG_SAVE()
    AC_LANG([Fortran 77])
    AC_COMPILE_IFELSE([
      AC_LANG_SOURCE([
        subroutine f_size()
        $1 i(2)
        call c_size(i(1), i(2))
        end
      ])
    ],[
      AC_LANG([C])
dnl First, we have to rename the Fortran object file, which we need to link --
dnl Some compilers such as on the T3e may not be able to compile like:
dnl   cc -o conftest conftest.o conftest.c
      mv conftest.$ac_objext conftestf.$ac_objext
      old_LIBS="$LIBS"
      LIBS="conftestf.$ac_objext $LIBS $FLIBS"
      AC_TRY_RUN([
#       include <stdio.h>
#       if defined(FORTRANNOUNDERSCORE)
#         define C_SIZE c_size
#         define F_SIZE f_size
#       elif defined(FORTRANCAPS)
#         define C_SIZE C_SIZE
#         define F_SIZE F_SIZE
#       elif defined(FORTRANUNDERSCORE)
#         define C_SIZE c_size_
#         define F_SIZE f_size_
#       elif defined(FORTRANDOUBLEUNDERSCORE)
#         define C_SIZE c_size__
#         define F_SIZE f_size__
#       endif
        extern void F_SIZE(void);
        static long size_val;
        /* Called by Fortran */
        void C_SIZE (i1p, i2p)
        char *i1p, *i2p;
        {
          size_val = (i2p - i1p);
        }
        int main() {
          FILE *f=fopen ("conftestval","w");
          if (!f) return 1;
          /* Call the Fortran function */
          F_SIZE ();
          fprintf (f, "%ld\n", size_val);
          return 0;
        }
     ])
     LIBS="$old_LIBS"
     ac_cv_sizeof_fortran_[]translit($1, [A-Z *], [a-z_p])=`cat conftestval`
dnl Since we do renaming of the object file and reset the language,
dnl we need to manually delete the object file and the old conftest.f file.
     rm -f conftestval conftestf.$ac_objext conftest.f
    ])
    AC_LANG_RESTORE()
  ])
  AC_DEFINE_UNQUOTED(SIZEOF_FORTRAN_[]translit($1, [a-z *], [A-Z_p]),
                     [$ac_cv_sizeof_fortran_[]translit($1, [A-Z *], [a-z_p])],
                     [The size of a Fortran `$1', as computed by sizeof.])
])

dnl
dnl This test checks, whether the Fortran MPI types are provided through
dnl including "mpi.h" as well.
dnl
AC_DEFUN([AC_CHECK_FORTRAN_MPI_TYPES_IN_C],[
  AC_MSG_CHECKING(for Fortran MPI types in C)
  AC_LANG_SAVE()
  ac_ext=c
  ac_compile=$ac_cv_mpicc_compile
  ac_link=$ac_cv_mpicc_link

  AC_COMPILE_IFELSE([AC_LANG_SOURCE([
#   include "mpi.h"
  int main (int argc, char * argv[])
  {
    int size;
    MPI_Type_size (MPI_CHARACTER, &size);
    MPI_Type_size (MPI_INTEGER, &size);
    MPI_Type_size (MPI_LOGICAL, &size);
    MPI_Type_size (MPI_REAL, &size);
    MPI_Type_size (MPI_DOUBLE_PRECISION, &size);
    return 0;
  }
  ])],
  [
    AC_DEFINE([HAVE_MPI_FORTRAN_TYPES_IN_C], 1, [Defined if Fortran MPI_Types are available in C])
    AC_MSG_RESULT(yes)
  ],
  [
    AC_MSG_RESULT(no)
  ])
  AC_LANG_RESTORE()
])


dnl
dnl PAC_STRUCT_GET_SIZE(Name of variable, first variable, second variable, cache_name)
dnl Second version adapted to caching
dnl
AC_DEFUN([AC_CHECK_SIZEOF_MPI_STRUCT],
  [
  AC_CACHE_CHECK([size of reduction function variables $1],
                 [ac_cv_sizeof_struct_[]translit($1, [A-Z *], [a-z_p])],
  [
    AC_TRY_RUN([
#     include <stdio.h>
      int calc (void) {
        struct mytype {
         $2 a;
         $3 b;
        };
        return sizeof (struct mytype);
      }
      int main() {
         FILE *f=fopen("conftestval","w");
         if (!f) return 1;
         fprintf( f, "%d\n", calc());
         return 0;
      }], [
      ac_cv_sizeof_struct_[]translit($1, [A-Z *], [a-z_p])=`cat conftestval`
      rm -f conftestval
      ])
  ])
  AC_DEFINE_UNQUOTED(SIZEOF_STRUCT_[]translit($1, [a-z *], [A-Z_p]),
                     [$ac_cv_sizeof_struct_[]translit($1, [A-Z *], [a-z_p])],
                     [The size of a MPI structure `$1', as computed by sizeof.])
])


dnl
dnl Compiles and runs a simple program, including the "mpi.h"-header
dnl printing the MPI_VERSION and MPI_SUBVERSION as specified by the MPI-standard
dnl
dnl Depedancies: setting of $mpi_inc_dir, $mpi_lib_dir, $lib_mpi
dnl
AC_DEFUN([AC_CHECK_MPI_VERSION],
[
  AC_CACHE_CHECK([for version of MPI implementation], [ac_cv_mpi_version],
  [
    AC_LANG_PUSH(C)
    ac_compile=$ac_cv_mpicc_compile
    ac_link=$ac_cv_mpicc_link

    AC_TRY_RUN([
#     include <stdio.h>
#     include "mpi.h"
      int main () {
        FILE * f=fopen("conftestval", "w");
        if (!f) return 1;
#       ifdef MPI_VERSION
        fprintf (f, "%d.%d\n", MPI_VERSION, MPI_SUBVERSION);
#       else
        fprintf (f, "unknown\n");
#       endif
        return 0;
      }],
    [
      VAL=`cat ./conftestval`
      rm -f conftestval
      if test "x$VAL" = "x" ; then
        ac_cv_mpi_version="unknown"
      else
        ac_cv_mpi_version="$VAL"
        ac_cv_mpi_major_version=`echo $VAL | cut -f1 -d'.'`
        ac_cv_mpi_minor_version=`echo $VAL | cut -f2 -d'.'`
      fi
    ], [ac_cv_mpi_version="unknown", ac_cv_mpi_major_version="1", ac_cv_mpi_minor_version="0"])
    AC_LANG_POP(C)
  ])
])


dnl
dnl Check for the socklen_t type; should be in <sys/types.h> or <sys/socket.h>
dnl If not defined, set it to int.
dnl
dnl Dependancies: AC_CHECK_HEADER (sys/types.h)
dnl
AC_DEFUN([AC_TYPE_SOCKLEN_T], [
  dnl Since the old-style (autoconf 2.13) macro AC_CHECK_TYPE(type, replacement)
  dnl only checks in <sys/types.h> we have to be more awkward:
  dnl We have to check for socklen_t in <sys/types.h> -- if it is not defined
  dnl there, we also look in <sys/socket.h> (a more common place)
  AC_CHECK_TYPE(socklen_t,,
    AC_DEFINE(socklen_t, int, [Define to `int' if neither <sys/types.h> nor <sys/socket.h> define it.]),
    [
#     include <sys/types.h>
#     include <sys/socket.h>
  ])
])


dnl
dnl Check POSIX Thread version
dnl
dnl defines ac_cv_pthread_version to "final", "draft4", "draft5" or "unknown"
dnl  "unknown" implies that the version could not be detected
dnl  or that pthread.h does exist.
dnl  Existance of pthread.h should be tested separately.
dnl
AC_DEFUN([AC_CHECK_PTHREAD_VERSION],[
AC_CACHE_CHECK([POSIX thread version],[ac_cv_pthread_version],[
        AC_EGREP_CPP(pthread_version_final,[
#                include <pthread.h>
                /* this check could be improved */
#                ifdef PTHREAD_ONCE_INIT
                        pthread_version_final;
#                endif
        ], ac_pthread_final="yes", ac_pthread_final="no")

        AC_EGREP_CPP(pthread_version_draft4,[
#                include <pthread.h>
                /* this check could be improved */
#                ifdef pthread_once_init
                        pthread_version_draft4;
#                endif
        ], ac_pthread_draft4="yes", ac_pthread_draft4="no")

dnl Disable this test, cause we cant link this without error on Hitachi and other platforms.
dnl        AC_TRY_COMPILE([
dnl#                include <pthread.h>
dnl                ],
dnl                [
dnl                pthread_attr_create();
dnl                ],
dnl                ac_pthread_draft5="yes",
dnl                ac_pthread_draft5="no")
        ac_pthread_draft5="no"

        if test "x$ac_pthread_final" = "xyes"  -a "x$ac_pthread_draft4" = "xno"  -a "x$ac_pthread_draft5" = "xno"; then
                ac_cv_pthread_version="final"
        elif test "x$ac_pthread_final" = "xno" -a "x$ac_pthread_draft4" = "xyes" -a "x$ac_pthread_draft5" = "xno"; then
                ac_cv_pthread_version="draft4"
        elif test "x$ac_pthread_final" = "xno" -a "x$ac_pthread_draft4" = "xno"  -a "x$ac_pthread_draft5" = "xyes"; then
                ac_cv_pthread_version="draft5"
        else
                ac_cv_pthread_version="unknown"
        fi
dnl        echo "draft4:$ac_pthread_draft4  draft5:$ac_pthread_draft5  final:$ac_pthread_final"
])
])


dnl
dnl Check for threads in general
dnl   Substitutes the pthread_cflags && pthread_ldflags according to system
dnl
AC_DEFUN([AC_CHECK_PTHREAD],[
  old_CFLAGS=$CFLAGS
  old_LDFLAGS=$LDFLAGS
  old_LIBS=$LIBS

  case "$target" in
    *-ibm-aix*) dnl set the compiler to be cc_r, if it's not set yet!
        if test -z "${CC}" ; then
          AC_CHECK_PROGS(CC, cc_r xlc_r cc)
          if test "x${CC}" = "xcc" ; then
            AC_MSG_ERROR([--enable-tcp-threads requires cc_r (or equivalent compiler) on AIX])
          fi
        fi
        ;;
  esac

  dnl First check for the c_r library which is needed on some systems to link
  dnl the reentrant versions of NON-threadsafe functions
  dnl We could check for any function, but specifying a reentrant one makes it more safe.
  AC_CHECK_LIB(c_r, strerror_r,        [
        LIBS="-lc_r"
        LDFLAGS="-lc_r"
        ])

dnl  echo "CFLAGS:$CFLAGS   LDFLAGS:$LDFLAGS"

  AC_CHECK_HEADER(pthread.h)
  CFLAGS="-D_REENTRANT"

dnl  echo "CFLAGS:$CFLAGS   LDFLAGS:$LDFLAGS"
  AC_CHECK_LIB(pthread, pthread_create,
        LDFLAGS="-lpthread",
        [
        AC_CHECK_LIB(pthreads, pthread_create,
                LDFLAGS="$LDFLAGS -lpthreads",
                AC_MSG_ERROR(Can't find pthread-library))
        ])
dnl  echo "CFLAGS:$CFLAGS   LDFLAGS:$LDFLAGS"

  AC_CHECK_PTHREAD_VERSION

  dnl This is necessary for Hitachi, otherwise interface of
  dnl early drafts of pthread.h is used.
  if test "x$ac_cv_pthread_version" != "xfinal" ; then
    AC_CACHE_CHECK([whether -D_PTHREADS_D10 is needed], [ac_cv_pthreads_needs_variable],
      [
      AC_TRY_LINK([
#       include <pthread.h>
      ],
      [
        pthread_attr_t attribute;
        pthread_attr_init (&attribute);
      ],
      ac_cv_pthreads_needs_variable="no",
      [
        dnl Action if not found:
        CFLAGS="$CFLAGS -D_PTHREADS_D10"
        AC_TRY_LINK(
                [
#                include <pthread.h>
                ],
                [
                  pthread_attr_t attribute;
                  pthread_attr_init (&attribute);
                ],
                ac_cv_pthreads_needs_variable="yes",
                AC_MSG_ERROR(Can't link program with and without compile-variable _PTHREADS_D10)
        )
      ])
    ])
    if test "x$ac_cv_pthreads_needs_variable" = "xyes" ; then
      CFLAGS="$CFLAGS -D_PTHREADS_D10"
    fi
  fi
dnl  echo "CFLAGS:$CFLAGS   LDFLAGS:$LDFLAGS"
  ac_cv_pthread_cflags=$CFLAGS
  ac_cv_pthread_ldflags=$LDFLAGS
  CFLAGS=$old_CFLAGS
  LDFLAGS=$old_LDFLAGS
  LIBS=$LIBS
])


dnl
dnl This test has to be improved!!!!
dnl
AC_DEFUN([AC_CHECK_MPI_THREADSAFETY],[
  AC_MSG_CHECKING(for thread-safety of MPI)
  old_CFLAGS=$CFLAGS
  old_LDFLAGS=$LDFLAGS
  old_LIBS=$LIBS
  CFLAGS="-I$mpi_inc_dir"
  LDFLAGS="-L$mpi_lib_dir"
  LIBS="-lmpi"
dnl  echo "CFLAGS:$CFLAGS   LDFLAGS:$LDFLAGS  LIBS:$LIBS"
  AC_TRY_LINK(
        [
#include <stdio.h>
#include "mpi.h"
        ],
        [
          char * s, * argv[2] = {"conftest", NULL};
          int argc=1, provided;
          FILE * output;
          if (MPI_SUCCESS != MPI_Init_thread (&argc, (char***)&argv, MPI_THREAD_MULTIPLE, &provided))
            return -1;
            output = fopen ("output.txt", "w");
          switch (provided)
            {
            case MPI_THREAD_SINGLE:     s="MPI_THREAD_SINGLE"; break;
            case MPI_THREAD_FUNNELED:   s="MPI_THREAD_FUNNELED"; break;
            case MPI_THREAD_SERIALIZED: s="MPI_THREAD_SERIALIZED"; break;
            case MPI_THREAD_MULTIPLE:   s="MPI_THREAD_MULTIPLE"; break;
            default:                    s="none";
            }
          fprintf (output, "%s\n", s);
          fclose (output);
          MPI_Finalize ();
        ],
        AC_MSG_RESULT(yes),
        AC_MSG_RESULT(no)
        )
  CFLAGS=$old_CFLAGS
  LDFLAGS=$old_LDFLAGS
  LIBS=$old_LIBS
])


dnl
dnl
dnl
AC_DEFUN([AC_CHECK_PTHREAD_MUTEXATTR_SETKIND],[
  AC_REQUIRE([AC_CHECK_PTHREAD])
  dnl THIS NEEDS TO BE REWRITTEN TO TAKE ADVANTAGE OF THE
  AC_CHECK_LIB(pthread, pthread_mutexattr_setkind_np,
   [AC_DEFINE([HAVE_PTHREAD_MUTEXATTR_SETKIND_NP], 1, [Define if pthreads supports pthread_,itex_attr_setkind_np])],
   [
     AC_CHECK_LIB(pthreads, pthread_mutexattr_setkind_np,
       [AC_DEFINE([HAVE_PTHREAD_MUTEXATTR_SETKIND_NP], 1, [Define if pthreads supports pthread_,itex_attr_setkind_np])])
  ])
])


dnl
dnl
dnl
AC_DEFUN([AC_CHECK_PTHREAD_MUTEX_ERRORCHECK], [
  AC_REQUIRE([AC_CHECK_PTHREAD])
  AC_REQUIRE([AC_CHECK_PTHREAD_MUTEXATTR_SETKIND])
  AC_CACHE_CHECK([for PTHREAD_MUTEX_ERRORCHECK_NP], [ac_cv_have_mutex_errorcheck],
  [
    old_CFLAGS=$CFLAGS
    old_LDFLAGS=$LDFLAGS
    CFLAGS=$ac_cv_pthread_cflags
    LDFLAGS=$ac_cv_pthread_ldflags
dnl This should be replaced with AC_COMPILE_IFELSE
    AC_TRY_COMPILE([
#     include <pthread.h>
    ],
    [
      pthread_mutexattr_t mutex_attr;
      int ret;
      ret = pthread_mutexattr_init (&mutex_attr);
      ret = pthread_mutexattr_setkind_np (&mutex_attr, PTHREAD_MUTEX_ERRORCHECK_NP);
    ], [ac_cv_have_mutex_errorcheck="yes"], [ac_cv_have_mutex_errorcheck="no"])
    CFLAGS=$old_CFLAGS
    LDFLAGS=$old_LDFLAGS
  ])
  if test "x$ac_have_mutex_errorcheck" = "xyes" ; then
    AC_DEFINE([HAVE_PTHREAD_MUTEX_ERRORCHECK_NP], 1, [Define if pthreads support the mutex attribute pthread_mutex_errorcheck_np])
  fi
])



dnl
dnl AC_CHECK_TCP_SOLINGER
dnl
dnl Check for the TCP_SOLINGER Option
dnl
AC_DEFUN([AC_CHECK_TCP_SOLINGER],
[
  AC_CACHE_CHECK([for TCP SOLINGER socket option], [ac_cv_tcp_solinger],
  [
  AC_TRY_COMPILE(
  [
#   include <stdlib.h>
#   include <sys/types.h>
#   include <sys/socket.h>
  ],
  [
    int s;
    int ret;
    struct linger ling;
    if ((s = socket (PF_INET, SOCK_STREAM, 0)) == -1)
      return 1;

    ling.l_onoff = 1;
    ling.l_linger = 0;
    return setsockopt (s, SOL_SOCKET, SO_LINGER, &ling, sizeof(struct linger));
  ], [ac_cv_tcp_solinger="yes"], [ac_cv_tcp_solinger="no"])
  ])
  if test "x$ac_cv_tcp_solinger" = "xyes" ; then
    AC_DEFINE([PACX_TCP_SOLINGER], 1, [Define if TCP SO-linger state shall be used])
  fi
])


dnl
dnl AC_CHECK_TCP_WINSHIFT
dnl Only known on CrayT3e; must be used before calling connect.
dnl See http://archive.ncsa.uiuc.edu/People/vwelch/net_perf/tcp_windows.html
dnl
dnl Check for the TCP_WINSHIFT socket option.
dnl
AC_DEFUN([AC_CHECK_TCP_WINSHIFT],
[
  AC_CACHE_CHECK([for TCP WINSHIFT socket option], [ac_cv_have_tcp_winshift],
  [
  AC_TRY_COMPILE(
  [
#   include <sys/types.h>
#   include <sys/socket.h>
#   include <netinet/in.h>
#   include <netinet/tcp.h>
  ],
  [
    int s;
    int ret;
    int flag;
    if ((s = socket (PF_INET, SOCK_STREAM, 0)) == -1)
      return 1;
    flag = 1;
    return setsockopt (s, IPPROTO_TCP, TCP_WINSHIFT, &flag, sizeof(flag));
  ], [ac_cv_have_tcp_winshift="yes"], [ac_cv_have_tcp_winshift="no"])
  ])
  if test "x$ac_cv_have_tcp_winshift" = "xyes" ; then
    AC_DEFINE([PACX_TCP_WINSHIFT], 1, [Define if socket option TCP_WINSHIFT is available])
  fi
])


dnl
dnl AC_CHECK_TCP_RFC1323
dnl Mainly known on AIX servers.
dnl See http://archive.ncsa.uiuc.edu/People/vwelch/net_perf/tcp_windows.html
dnl
dnl Check for the TCP_RFC1323 socket option.
dnl
AC_DEFUN([AC_CHECK_TCP_RFC1323],
[
  AC_CACHE_CHECK([for TCP RFC1323 socket option], [ac_cv_have_tcp_rfc1323],
  [
  AC_TRY_COMPILE(
  [
#   include <sys/types.h>
#   include <sys/socket.h>
#   include <netinet/in.h>
#   include <netinet/tcp.h>
  ],
  [
    int s;
    int ret;
    int flag;
    if ((s = socket (PF_INET, SOCK_STREAM, 0)) == -1)
      return 1;
    flag = 1;
    return setsockopt (s, IPPROTO_TCP, TCP_RFC1323, &flag, sizeof(flag));
  ], [ac_cv_have_tcp_rfc1323="yes"], [ac_cv_have_tcp_rfc1323="no"])
  ])
  if test "x$ac_cv_have_tcp_rfc1323" = "xyes" ; then
    AC_DEFINE([PACX_TCP_RFC1323], 1, [Define if socket option TCP_RFC1323 is available])
  fi
])


dnl
dnl AC_CHECK_TCP_NODELAY ([cmds, if true], [cmds, if false])
dnl
dnl Check for the TCP_NODELAY Option
dnl
AC_DEFUN([AC_CHECK_TCP_NODELAY],
[
  AC_CACHE_CHECK([for TCP NODELAY socket option], [ac_cv_have_tcp_nodelay],
  [
  AC_TRY_COMPILE(
  [
#   include <sys/types.h>
#   include <sys/socket.h>
#   include <netinet/in.h>
#   include <netinet/tcp.h>
  ],
  [
    int s, flag = 1, flag_len = sizeof(flag);
    if ((s = socket (PF_INET, SOCK_STREAM, 0)) == -1)
      return 1;
    if (setsockopt (s, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag)) == -1)
      return 2;
    if (getsockopt (s, IPPROTO_TCP, TCP_NODELAY, &flag, &flag_len) == -1)
      return 3;
    if (flag_len != sizeof(flag))
      return 4;
    if (flag != 1)
      return 5;
    return 0;
  ], [ac_cv_have_tcp_nodelay="yes"], [ac_cv_have_tcp_nodelay="no"])
  ])
  if test "x$ac_cv_have_tcp_nodelay" = "xyes" ; then
    AC_DEFINE([PACX_TCP_NODELAY], 1, [Define if support for socket option TCP_NODELAY shall be compiled])
  fi
])


dnl
dnl
dnl
AC_DEFUN([AC_PROG_AR], [
  AC_ARG_VAR(AR, [Archiver to create libraries, useful for cross compilation])
  AC_CHECK_TOOL(AR, ar)
  AC_SUBST(AR)
])


dnl
dnl
dnl
AC_DEFUN([AC_CHECK_STDARG], [
  AC_CACHE_CHECK([for availability of stdargs], [ac_cv_stdarg], [
    AC_TRY_COMPILE([
#     include <stdarg.h>
      int foo (int bar, ...)
        {
          va_list ap;
          int n;

          va_start(ap, bar);
          n = va_arg (ap, int);
          va_end (ap);

          if (n != 4711)
            return 1;
          return 0;
        }
    ],
    [if (foo(0, 4711)) return 1;],
    ac_cv_stdarg="yes", ac_cv_stdarg="no")
  ])
])


dnl AC_DEFUN([AC_CHECK_LARGEFILE],[
dnl #define _LARGEFILE_SOURCE
dnl #define _FILE_OFFSET_BITS 64
dnl #include <unistd.h>
dnl #include <sys/types.h>
dnl #include <sys/stat.h>
dnl #include <assert.h>
dnl #include <stdio.h>
dnl
dnl int main( int, char **argv )
dnl {
dnl // check that off_t can hold 2^63 - 1 and perform basic operations...
dnl #define OFF_T_64 (((off_t) 1 << 62) - 1 + ((off_t) 1 << 62))
dnl     assert( OFF_T_64 % 2147483647 == 1 );
dnl
dnl     // stat breaks on SCO OpenServer
dnl     struct stat buf;
dnl     stat( argv[0], &buf );
dnl     assert( S_ISREG(buf.st_mode) );
dnl
dnl     FILE *file = fopen( argv[0], "r" );
dnl     off_t offset = ftello( file );
dnl     fseek( file, offset, SEEK_CUR );
dnl     fclose( file );
dnl     return 0;
dnl }
