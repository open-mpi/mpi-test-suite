/*
 * File: tst_env_status_check.c
 *
 * Functionality:
 *   Checks, that any predefined datatype can be cloned and freed.
 *
 * Author: Rainer Keller
 *
 * Date: Sep. 2., 2009
 */
#include <mpi.h>
#include "mpi_test_suite.h"
#include "tst_output.h"


#define LOCAL_CHECK(func_string,var,op,expected) \
  if (var op expected) { \
    if (tst_report == TST_REPORT_FULL) \
      printf ("(Rank:%d) Failed in " func_string " " #var " " #op " expected:%d but got:%d\n", \
              tst_global_rank, expected, var); \
    tst_test_recordfailure (env); \
  }


int tst_env_type_dup_init (struct tst_env * env)
{
  tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "(Rank:%d) env->comm:%d env->type:%d env->values_num:%d\n",
                     tst_global_rank, env->comm, env->type, env->values_num);
  return 0;
}

int tst_env_type_dup_run (struct tst_env * env)
{
  MPI_Datatype type, check_type;
  MPI_Aint lb_extent, check_lb_extent;
  MPI_Aint extent, check_extent;
  MPI_Aint lb_true_extent, check_lb_true_extent;
  MPI_Aint true_extent, check_true_extent;

  check_type = tst_type_getdatatype (env->type);
  memset (&type, 0xFF, sizeof (type));

  MPI_CHECK (MPI_Type_get_extent (check_type, &check_lb_extent, &check_extent));
  MPI_CHECK (MPI_Type_get_true_extent (check_type, &check_lb_true_extent, &check_true_extent));

  /* Now on to the real thing! */
  MPI_CHECK (MPI_Type_dup (check_type, &type));
  /* Now here might be nice other checks... */
  /* E.g. dissecting the DDT using */
  MPI_Type_get_extent (type, &lb_extent, &extent);

  MPI_Type_get_true_extent (type, &lb_true_extent, &true_extent);

  LOCAL_CHECK ("MPI_Type_get_extent", (int)extent, !=, (int)check_extent);
  LOCAL_CHECK ("MPI_Type_get_true_extent", (int)true_extent, !=, (int)check_true_extent);

  MPI_CHECK (MPI_Type_free (&type));
  return 0;
}

int tst_env_type_dup_cleanup (struct tst_env * env)
{
  return 0;
}

