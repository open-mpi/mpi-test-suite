/*
 * File: tst_coll_gather.c
 *
 * Functionality:
 *  Simple collective Gather test-program.
 *  Works with intra- communicators, MPI_COMM_SELF and up to now with any C (standard and struct) type.
 *
 * Author: Rainer Keller
 *
 * Date: Aug 8th 2003
 */
#include "mpi.h"
#include "mpi_test_suite.h"
#include "tst_output.h"

#undef DEBUG
#define DEBUG(x)

/*
 * XXX
static char * send_buffer = NULL;
static char * recv_buffer = NULL;
 */

int tst_coll_gather_init (struct tst_env * env)
{
  int comm_size;
  MPI_Comm comm;
  tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "(Rank:%d) env->comm:%d env->type:%d env->values_num:%d\n",
                 tst_global_rank, env->comm, env->type, env->values_num);

  comm = tst_comm_getcomm (env->comm);
  MPI_CHECK (MPI_Comm_size (comm, &comm_size));

  env->send_buffer = tst_type_allocvalues (env->type, env->values_num);
  env->recv_buffer = tst_type_allocvalues (env->type, comm_size * env->values_num);

  return 0;
}

int tst_coll_gather_run (struct tst_env * env)
{
  int comm_rank;
  int comm_size;
  int i;
  MPI_Comm comm;
  MPI_Datatype type;

  comm = tst_comm_getcomm (env->comm);
  type = tst_type_getdatatype (env->type);

  MPI_CHECK (MPI_Comm_rank (comm, &comm_rank));
  MPI_CHECK (MPI_Comm_size (comm, &comm_size));

  tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "(Rank:%d) comm_size:%d comm_rank:%d\n",
                 tst_global_rank, comm_size, comm_rank);

  tst_type_setstandardarray (env->type, env->values_num, env->send_buffer, comm_rank);
  for (i=0; i < comm_size; i++)
    {
      const int type_size = tst_type_gettypesize (env->type);
      int j;

      tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "(Rank:%d) Going to Gather with root:%d\n",
                     tst_global_rank, i);

      MPI_CHECK (MPI_Gather (env->send_buffer, env->values_num, type, 
                             env->recv_buffer, env->values_num, type,
                             i, comm));
      if (comm_rank == i)
        for (j = 0; j < comm_size; j++)
          tst_test_checkstandardarray (env, &(env->recv_buffer[j * env->values_num * type_size]), j);
    }

  return 0;
}

int tst_coll_gather_cleanup (struct tst_env * env)
{
  tst_type_freevalues (env->type, env->send_buffer, env->values_num);
  tst_type_freevalues (env->type, env->recv_buffer, env->values_num);
  return 0;
}
