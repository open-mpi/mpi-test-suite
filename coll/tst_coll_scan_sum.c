/*
 * File: tst_coll_scan.c
 *
 * Functionality:
 *  Simple collective Scan test-program.
 *  Works with intra- communicators, MPI_COMM_SELF and up to now with any C (standard and struct) type.
 *
 * Author: Rainer Keller
 *
 * Date: Nov 2nd 2005
 */
#include "mpi.h"
#include "mpi_test_suite.h"

#undef DEBUG
#define DEBUG(x)

static char * send_buffer = NULL;
static char * recv_buffer = NULL;
static char * check_buffer = NULL;
static int sum = 0;

int tst_coll_scan_sum_init (const struct tst_env * env)
{
  const int type_size = tst_type_gettypesize (env->type);
  int i;
  MPI_Comm comm;
  int comm_rank;

  DEBUG (printf ("(Rank:%d) env->comm:%d env->type:%d env->values_num:%d\n",
                 tst_global_rank, env->comm, env->type, env->values_num));

  comm = tst_comm_getcomm (env->comm);
  MPI_CHECK (MPI_Comm_rank (comm, &comm_rank));

  send_buffer = tst_type_allocvalues (env->type, env->values_num);
  recv_buffer = tst_type_allocvalues (env->type, env->values_num);
  check_buffer = tst_type_allocvalues (env->type, env->values_num);

  /*
   * For each process, set send_buffer to 0, 1, 2...
   * As we sum up over all processes up to i==comm_rank inclusive,
   * the check_buffer is:
   *   proc0: 0, 1, 2...
   *   proc1: 0, 2, 4...
   *   proc2: 0, 3, 6...
   */
  for (i = 0; i < env->values_num; i++)
    {
      tst_type_setvalue (env->type, &send_buffer[i*type_size],
                         TST_TYPE_SET_VALUE, i);
      tst_type_setvalue (env->type, &check_buffer[i*type_size],
                         TST_TYPE_SET_VALUE, i*(comm_rank+1));
    }

  return 0;
}

int tst_coll_scan_sum_run (const struct tst_env * env)
{
  const int type_size = tst_type_gettypesize (env->type);
  int comm_rank;
  int comm_size;
  int i;
  MPI_Comm comm;
  MPI_Datatype type;
  int errors = 0;

  comm = tst_comm_getcomm (env->comm);
  type = tst_type_getdatatype (env->type);

  MPI_CHECK (MPI_Comm_rank (comm, &comm_rank));
  MPI_CHECK (MPI_Comm_size (comm, &comm_size));

  DEBUG (printf ("(Rank:%d) comm_size:%d comm_rank:%d. Going to Scan.\n",
                 tst_global_rank, comm_size, comm_rank));

  MPI_CHECK (MPI_Scan (send_buffer, recv_buffer, env->values_num, type,
                       MPI_SUM, comm));

  for (i = 0; i < env->values_num; i++)
    if (0 != tst_type_cmpvalue(env->type,
                               &recv_buffer[i*type_size], &check_buffer[i*type_size]))
      {
        if (tst_report >= TST_REPORT_FULL)
          {
            printf ("(Rank:%d) Error at pos:%d\n", tst_global_rank, i);
            tst_type_hexdump ("Expected cmp_value", &check_buffer[i*type_size], type_size);
            tst_type_hexdump ("Received buffer", &recv_buffer[i*type_size], type_size);
          }
        errors++;
      }
  if (errors)
    tst_test_recordfailure (env);

  return 0;
}

int tst_coll_scan_sum_cleanup (const struct tst_env * env)
{
  tst_type_freevalues (env->type, send_buffer, env->values_num);
  tst_type_freevalues (env->type, recv_buffer, env->values_num);
  tst_type_freevalues (env->type, check_buffer, env->values_num);
  return 0;
}
