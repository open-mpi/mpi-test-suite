/*
 * File: tst_one_sided_simple.c
 *
 * Functionality:
 *  Simple one-sided Test-program.
 *
 * Author: Rainer Keller
 *
 */
#include "mpi.h"
#include "mpi_test_suite.h"

#undef DEBUG
#define DEBUG(x)

#ifdef HAVE_MPI2_ONE_SIDED

static char * send_buffer = NULL;
static char * recv_buffer = NULL;
static MPI_Aint send_buffer_size = 0;
static MPI_Win send_win = MPI_WIN_NULL;

int tst_one_sided_simple_ring_get_init (const struct tst_env * env)
{
  int comm_rank;
  MPI_Comm comm;
  int type_size;

  DEBUG (printf ("(Rank:%d) env->comm:%d env->type:%d env->values_num:%d\n",
                 tst_global_rank, env->comm, env->type, env->values_num));

  comm = tst_comm_getcomm (env->comm);
  MPI_CHECK (MPI_Comm_rank (comm, &comm_rank));

  send_buffer = tst_type_allocvalues (env->type, env->values_num);
  tst_type_setstandardarray (env->type, env->values_num, send_buffer, comm_rank);
  tst_type_getstandardarray_size (env->type, env->values_num, &send_buffer_size);

  recv_buffer = tst_type_allocvalues (env->type, env->values_num);
  type_size = tst_type_gettypesize (env->type);

  /*
   * Create a window for the send and the receive buffer
   */
  DEBUG (printf ("(Rank:%d) Going to create window\n",
                 tst_global_rank));
  MPI_Win_create (send_buffer, send_buffer_size, type_size,
                  MPI_INFO_NULL, comm, &send_win);

  return 0;
}

int tst_one_sided_simple_ring_get_run (const struct tst_env * env)
{
  int comm_rank;
  int comm_size;
  MPI_Comm comm;
  MPI_Datatype type;
  int type_size;
  int get_from;

  comm = tst_comm_getcomm (env->comm);
  type = tst_type_getdatatype (env->type);
  type_size = tst_type_gettypesize (env->type);

  MPI_CHECK (MPI_Comm_rank (comm, &comm_rank));
  MPI_CHECK (MPI_Comm_size (comm, &comm_size));

  DEBUG (printf ("(Rank:%d) comm_size:%d comm_rank:%d\n",
                 tst_global_rank, comm_size, comm_rank));

  get_from = (comm_rank + comm_size - 1) % comm_size;
  DEBUG (printf ("(Rank:%d) Going to MPI_from from get_from:%d\n",
                 tst_global_rank, get_from));

  /*
   * All processes call MPI_Put
   */
  MPI_Win_fence (MPI_MODE_NOPRECEDE, send_win);
  MPI_Get (recv_buffer, env->values_num, type,
           get_from, 0, env->values_num, type, send_win);

  MPI_Win_fence (MPI_MODE_NOSTORE | MPI_MODE_NOSUCCEED, send_win);

  tst_test_checkstandardarray (env, recv_buffer, (comm_rank + comm_size - 1) % comm_size);

  return 0;
}

int tst_one_sided_simple_ring_get_cleanup (const struct tst_env * env)
{
  int comm_rank;
  MPI_Comm comm;

  comm = tst_comm_getcomm (env->comm);
  MPI_CHECK (MPI_Comm_rank (comm, &comm_rank));

  MPI_Win_free (&send_win);

  tst_type_freevalues (env->type, send_buffer, env->values_num);
  tst_type_freevalues (env->type, recv_buffer, env->values_num);

  return 0;
}

#endif
