/*
 * File: tst_coll_allreduce.c
 *
 * Functionality:
 *  Simple collective Allreduce test-program.
 *  Works with intra- and inter- communicators, MPI_COMM_SELF and up to now with any C (standard and struct) type.
 *
 * Author: Rainer Keller
 *
 * Date: Aug 8th 2003
 */
#include "mpi.h"
#include "mpi_test_suite.h"

#undef DEBUG
#define DEBUG(x)

static char * send_buffer = NULL;
static char * recv_buffer = NULL;


int tst_coll_allreduce_init (const struct tst_env * env)
{
  DEBUG (printf ("(Rank:%d) env->comm:%d env->type:%d env->values_num:%d\n",
                 tst_global_rank, env->comm, env->type, env->values_num));

  send_buffer = tst_type_allocvalues (env->type, env->values_num);
  recv_buffer = tst_type_allocvalues (env->type, env->values_num);

  return 0;
}

int tst_coll_allreduce_run (const struct tst_env * env)
{
  int comm_rank;
  int comm_size;
  MPI_Comm comm;
  MPI_Datatype type;

  comm = tst_comm_getcomm (env->comm);
  type = tst_type_getdatatype (env->type);

  MPI_CHECK (MPI_Comm_rank (comm, &comm_rank));
  MPI_CHECK (MPI_Comm_size (comm, &comm_size));

  tst_type_setstandardarray (env->type, env->values_num, send_buffer, comm_rank);

  DEBUG (printf ("(Rank:%d) comm_size:%d comm_rank:%d\n",
                 tst_global_rank, comm_size, comm_rank));

  DEBUG (printf ("(Rank:%d) Going to Allreduce\n",
                 tst_global_rank));

  MPI_CHECK (MPI_Allreduce (send_buffer, recv_buffer, env->values_num, type, MPI_MAX, comm));

  tst_test_checkstandardarray (env, recv_buffer, (comm_size - 1));

  return 0;
}

int tst_coll_allreduce_cleanup (const struct tst_env * env)
{
  tst_type_freevalues (env->type, send_buffer, env->values_num);
  tst_type_freevalues (env->type, recv_buffer, env->values_num);
  return 0;
}
