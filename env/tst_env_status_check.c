/*
 * File: tst_env_status_check.c
 *
 * Functionality:
 *   Checks, that the entry of MPI_ERROR is left untouched in the MPI_Status
 *
 * Author: Rainer Keller
 *
 * Date: Dec. 1., 2005
 */
#include "mpi.h"
#include "mpi_test_suite.h"
#include "tst_output.h"

#undef DEBUG
#define DEBUG(x)

#define LOCAL_CHECK(func_string,var,op,expected) \
  if (var op expected) { \
    if (tst_report == TST_REPORT_FULL) \
      printf ("(Rank:%d) Failed in " func_string " " #var " " #op " expected:%d but got:%d\n", \
              tst_global_rank, expected, var); \
    tst_test_recordfailure (env); \
  }


int tst_env_status_check_init (struct tst_env * env)
{
  tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "(Rank:%d) env->comm:%d env->type:%d env->values_num:%d\n",
                 tst_global_rank, env->comm, env->type, env->values_num);
  return 0;
}

int tst_env_status_check_run (struct tst_env * env)
{
  MPI_Status status;
  int cancelled, get_count, get_elements;

  memset (&status, 0xFF, sizeof (status));
  status.MPI_ERROR = 4711;

  MPI_CHECK (MPI_Recv (NULL, 0, MPI_CHAR, MPI_PROC_NULL, 4711, MPI_COMM_WORLD, &status));

  MPI_CHECK (MPI_Test_cancelled (&status, &cancelled));
  MPI_CHECK (MPI_Get_count (&status, MPI_CHAR, &get_count));
  MPI_CHECK (MPI_Get_elements (&status, MPI_CHAR, &get_elements));

  /*
   * If interpreting MPI-1.2, sec. 3.2.5, p. 22 (changing status.MPI_ERROR for single-completion
   * wait/test calls) in the same sense for MPI_Recv, status.MPI_ERROR should not be changed,
   *
   * Nevertheless, we are not that anal.
   */
  LOCAL_CHECK ("MPI_Recv", (status.MPI_ERROR != MPI_SUCCESS) && status.MPI_ERROR, !=, 4711);
  LOCAL_CHECK ("MPI_Recv", status.MPI_SOURCE, !=, MPI_PROC_NULL);
  LOCAL_CHECK ("MPI_Recv", status.MPI_TAG, !=, MPI_ANY_TAG);

  LOCAL_CHECK ("MPI_Recv", cancelled, !=, 0);
  LOCAL_CHECK ("MPI_Recv", get_count, !=, 0);
  LOCAL_CHECK ("MPI_Recv", get_elements, !=, 0);

  return 0;
}

int tst_env_status_check_cleanup (struct tst_env * env)
{
  return 0;
}
