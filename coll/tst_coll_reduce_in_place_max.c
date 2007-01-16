/*
 * File: tst_coll_reduce_in_place_max.c
 *
 * Functionality:
 *  Simple collective Reduce test-program with MPI_IN_PLACE as send buffer.
 *  Works with intra- and inter- communicators, MPI_COMM_SELF and up to now with any C (standard and struct) type,
 *    except MPI_CHAR, MPI_UNSIGNED_CHAR and MPI_BYTE, as defined by the MPI-1.2 standard.
 *    As of MPI-2 (p77), we also accept MPI_UNSIGNED_CHAR and MPI_SIGNED_CHAR.
 *
 * Author: Rainer Keller, Jelena Pjesivac-Grbovic
 *
 * Date: Jan 16th 2007
 */
#include "mpi.h"
#include "mpi_test_suite.h"

#undef DEBUG
#define DEBUG(x)

static char * send_buffer = NULL;
static char * recv_buffer = NULL;

#define ROOT 0

int tst_coll_reduce_in_place_max_init (const struct tst_env * env)
{
  int comm_rank;
  MPI_Comm comm;

  DEBUG (printf ("(Rank:%d) env->comm:%d env->type:%d env->values_num:%d\n",
                 tst_global_rank, env->comm, env->type, env->values_num));

  comm = tst_comm_getcomm (env->comm);
  MPI_CHECK (MPI_Comm_rank (comm, &comm_rank));

  send_buffer = tst_type_allocvalues (env->type, env->values_num);
  tst_type_setstandardarray (env->type, env->values_num, send_buffer, comm_rank);

  /*
   * MPIch2-1.0.3 and MPI/SX checks even on NON-Root proceeses, whether recv_buffer is set!
   */
#if !defined(HAVE_MPICH2) && !defined(HAVE_MPISX)
  if (ROOT == comm_rank)
#endif
    recv_buffer = tst_type_allocvalues (env->type, env->values_num);


  return 0;
}

int tst_coll_reduce_in_place_max_run (const struct tst_env * env)
{
  int comm_rank;
  int comm_size;
  MPI_Comm comm;
  MPI_Datatype type;

  comm = tst_comm_getcomm (env->comm);
  type = tst_type_getdatatype (env->type);

  MPI_CHECK (MPI_Comm_rank (comm, &comm_rank));
  MPI_CHECK (MPI_Comm_size (comm, &comm_size));

  DEBUG (printf ("(Rank:%d) comm_size:%d comm_rank:%d\n",
                 tst_global_rank, comm_size, comm_rank));

  DEBUG (printf ("(Rank:%d) Going to Reduce\n",
                 tst_global_rank));

  if (ROOT == comm_rank) {
     tst_type_setstandardarray (env->type, env->values_num, recv_buffer, comm_rank);
     MPI_CHECK (MPI_Reduce (MPI_IN_PLACE, recv_buffer, env->values_num, type, MPI_MAX, ROOT, comm));
  } else {
     MPI_CHECK (MPI_Reduce (send_buffer, recv_buffer, env->values_num, type, MPI_MAX, ROOT, comm));
  }

  if (ROOT == comm_rank) {
    tst_test_checkstandardarray (env, recv_buffer, (comm_size - 1));
  }

  return 0;
}

int tst_coll_reduce_in_place_max_cleanup (const struct tst_env * env)
{
  int comm_rank;
  MPI_Comm comm;

  comm = tst_comm_getcomm (env->comm);
  MPI_CHECK (MPI_Comm_rank (comm, &comm_rank));

  tst_type_freevalues (env->type, send_buffer, env->values_num);
#if !defined(HAVE_MPICH2) && !defined(HAVE_MPISX)
  if (ROOT == comm_rank)
#endif
  tst_type_freevalues (env->type, recv_buffer, env->values_num);

  return 0;
}
