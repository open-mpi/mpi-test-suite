/*
 * File: tst_one_sided_simple_ring_get_post.c
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
static MPI_Group group_from = MPI_GROUP_NULL;
static MPI_Group group_to = MPI_GROUP_NULL;

int tst_one_sided_simple_ring_get_post_init (const struct tst_env * env)
{
  int comm_rank;
  int comm_size;
  int ranks[2];
  int ranks_num;
  MPI_Comm comm;
  MPI_Group comm_group;
  int type_size;

  DEBUG (printf ("(Rank:%d) env->comm:%d env->type:%d env->values_num:%d\n",
                 tst_global_rank, env->comm, env->type, env->values_num));

  comm = tst_comm_getcomm (env->comm);
  MPI_CHECK (MPI_Comm_rank (comm, &comm_rank));
  MPI_CHECK (MPI_Comm_size (comm, &comm_size));

  send_buffer = tst_type_allocvalues (env->type, env->values_num);
  tst_type_setstandardarray (env->type, env->values_num, send_buffer, comm_rank);

  recv_buffer = tst_type_allocvalues (env->type, env->values_num);
  type_size = tst_type_gettypesize (env->type);
  tst_type_getstandardarray_size (env->type, env->values_num, &send_buffer_size);

  /*
   * Just include the process and it's predecessor to post/start/complete/wait
   */
  MPI_CHECK (MPI_Comm_group (comm, &comm_group));
  ranks_num = 1;
  ranks[0] = (comm_rank + comm_size - 1) % comm_size;
  MPI_CHECK (MPI_Group_incl (comm_group, ranks_num, ranks, &group_from));

  ranks_num = 1;
  ranks[0] = (comm_rank + 1) % comm_size;
  MPI_CHECK (MPI_Group_incl (comm_group, ranks_num, ranks, &group_to));

  /*
   * Create a window for the send and the receive buffer
   */
  DEBUG (printf ("(Rank:%d) Going to create window pid:%ld\n",
                 tst_global_rank, getpid()));
  MPI_CHECK (MPI_Win_create (send_buffer, send_buffer_size, type_size,
                             MPI_INFO_NULL, comm, &send_win));

  MPI_CHECK (MPI_Group_free (&comm_group));

  return 0;
}

int tst_one_sided_simple_ring_get_post_run (const struct tst_env * env)
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
  DEBUG (printf ("(Rank:%d) Going to MPI_Get from get_from:%d\n",
                 tst_global_rank, get_from));

  /*
   * All processes call MPI_Get
   */
  if (tst_global_rank % 2 == 0) {
    /*
     * We first GET, then post our window
     */
    MPI_CHECK (MPI_Win_start (group_from, 0, send_win));
    MPI_CHECK (MPI_Get (recv_buffer, env->values_num, type,
                        get_from, 0, env->values_num, type, send_win));
    MPI_CHECK (MPI_Win_complete (send_win));

    MPI_CHECK (MPI_Win_post (group_to, 0, send_win));
    MPI_CHECK (MPI_Win_wait (send_win));
  } else {
    /*
     * We first post our window, then GET
     */
    MPI_CHECK (MPI_Win_post (group_to, 0, send_win));
    MPI_CHECK (MPI_Win_wait (send_win));

    MPI_CHECK (MPI_Win_start (group_from, 0, send_win));
    MPI_CHECK (MPI_Get (recv_buffer, env->values_num, type,
                        get_from, 0, env->values_num, type, send_win));
    MPI_CHECK (MPI_Win_complete (send_win));
  }

  tst_test_checkstandardarray (env, recv_buffer, get_from);

  return 0;
}

int tst_one_sided_simple_ring_get_post_cleanup (const struct tst_env * env)
{
  int comm_rank;
  MPI_Comm comm;

  comm = tst_comm_getcomm (env->comm);
  MPI_CHECK (MPI_Comm_rank (comm, &comm_rank));

  MPI_CHECK (MPI_Win_free (&send_win));

  tst_type_freevalues (env->type, send_buffer, env->values_num);
  tst_type_freevalues (env->type, recv_buffer, env->values_num);

  return 0;
}

#endif
