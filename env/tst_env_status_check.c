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

#undef DEBUG
#define DEBUG(x)

int tst_env_status_check_init (const struct tst_env * env)
{
  DEBUG (printf ("(Rank:%d) env->comm:%d env->type:%d env->values_num:%d\n",
                 tst_global_rank, env->comm, env->type, env->values_num));
  return 0;
}

int tst_env_status_check_run (const struct tst_env * env)
{
  MPI_Status status;

  memset (&status, 0, sizeof (status));
  status.MPI_ERROR = 4711;

  MPI_CHECK (MPI_Recv (NULL, 0, MPI_CHAR, MPI_PROC_NULL, 4711, MPI_COMM_WORLD, &status));

  /*
   * The above call should not change the value of MPI_ERROR (see MPI-1.2, sec. 3.2.5,
   * p. 22).
   * Only functions working on multiple requests/statuse may
   * change MPI_ERROR and set MPI_ERR_IN_STATUS (see MPI-1.2 sec. 3.7.5)
   */
  if (status.MPI_ERROR != 4711 ||
      status.MPI_SOURCE != MPI_PROC_NULL ||
      status.MPI_TAG != MPI_ANY_TAG)
    {
      if (tst_report >= TST_REPORT_FULL)
        {
          printf ("(Rank:%d) error:%d source:%d tag:%d\n"
                  "(Rank:%d) But expected error:%d source:%d tag:%d\n",
                  tst_global_rank, status.MPI_ERROR, status.MPI_SOURCE, status.MPI_TAG,
                  tst_global_rank, 4711, MPI_ANY_SOURCE, MPI_ANY_TAG);
        }
      tst_test_recordfailure (env);
    }
  return 0;
}

int tst_env_status_check_cleanup (const struct tst_env * env)
{
  return 0;
}
