/*
 * File: tst_coll_allgather_in_place.c
 *
 * Functionality:
 *  Simple collective Allgather test-program with MPI_IN_PLACE.
 *  Works with intra- communicators, MPI_COMM_SELF and up to now with any C (standard and struct) type.
 *
 * Author: Rainer Keller, Jelena Pjesivac-Grbovic
 *
 * Date: Jan 16th 2007
 */
#include <stdint.h>
#include <mpi.h>
#include "mpi_test_suite.h"

#undef DEBUG
#define DEBUG(x)

/*
 * XXX
static char * recv_buffer = NULL;
 */

int tst_coll_allgather_in_place_init (struct tst_env * env)
{
  int comm_rank;
  int comm_size;
  char * local_buffer;
  MPI_Comm comm;

  tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "(Rank:%d) env->comm:%d env->type:%d env->values_num:%d\n",
                 tst_global_rank, env->comm, env->type, env->values_num);

  comm = tst_comm_getcomm (env->comm);
  MPI_CHECK (MPI_Comm_size (comm, &comm_size));
  MPI_CHECK (MPI_Comm_rank (comm, &comm_rank));

  env->recv_buffer = tst_type_allocvalues (env->type, comm_size * env->values_num);
  local_buffer = env->recv_buffer + (comm_rank * env->values_num) * tst_type_gettypesize (env->type);

  tst_type_setstandardarray (env->type, env->values_num, local_buffer, comm_rank);

  return 0;
}

int tst_coll_allgather_in_place_run (struct tst_env * env)
{
  int comm_rank;
  int comm_size;
  int i;
  MPI_Comm comm;
  MPI_Datatype type;
  const int type_size = tst_type_gettypesize (env->type);

  comm = tst_comm_getcomm (env->comm);
  type = tst_type_getdatatype (env->type);
  
  MPI_CHECK (MPI_Comm_rank (comm, &comm_rank));
  MPI_CHECK (MPI_Comm_size (comm, &comm_size));

  tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "(Rank:%d) comm_size:%d comm_rank:%d\n",
                 tst_global_rank, comm_size, comm_rank);

  tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "(Rank:%d) Going to Allgather\n",
                 tst_global_rank);
  MPI_CHECK (MPI_Allgather (MPI_IN_PLACE, 0, MPI_DATATYPE_NULL, 
                            env->recv_buffer, env->values_num, type,
                            comm));

  for (i = 0; i < comm_size; i++)
    tst_type_checkstandardarray (env->type, env->values_num, &(env->recv_buffer[i * env->values_num * type_size]), i);

  return 0;
}

int tst_coll_allgather_in_place_cleanup (struct tst_env * env)
{
  int comm_size;
  MPI_Comm comm;
  comm = tst_comm_getcomm (env->comm);
  MPI_CHECK (MPI_Comm_size (comm, &comm_size));
  tst_type_freevalues (env->type, env->recv_buffer, comm_size * env->values_num);
  return 0;
}
