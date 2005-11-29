/*
 * File: tst_p2p_simple_ring.c
 *
 * Functionality:
 *  Simple point-to-point ring-communication test using MPI_Send and MPI_Recv starting
 *  with process zero.
 *  Works with intra-communicators and up to now with any C (standard and struct) type.
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

int tst_p2p_simple_ring_init (const struct tst_env * env)
{
  int comm_rank;
  MPI_Comm comm;

  DEBUG (printf ("(Rank:%d) env->comm:%d env->type:%d env->values_num:%d\n",
                 tst_global_rank, env->comm, env->type, env->values_num));

  send_buffer = tst_type_allocvalues (env->type, env->values_num);
  recv_buffer = tst_type_allocvalues (env->type, env->values_num);

  /*
   * Now, initialize the send_buffer
   */
  comm = tst_comm_getcomm (env->comm);
  MPI_CHECK (MPI_Comm_rank (comm, &comm_rank));

  tst_type_setstandardarray (env->type, env->values_num, send_buffer, comm_rank);

  return 0;
}

int tst_p2p_simple_ring_run (const struct tst_env * env)
{
  int comm_size;
  int comm_rank;
  int send_to;
  int recv_from;
  MPI_Comm comm;
  MPI_Datatype type;
  MPI_Status status;

  comm = tst_comm_getcomm (env->comm);
  type = tst_type_getdatatype (env->type);

  if (tst_comm_getcommclass (env->comm) & TST_MPI_COMM_SELF)
    {
      comm_size = 1;
      comm_rank = 0;
      send_to = MPI_PROC_NULL;
      recv_from = MPI_PROC_NULL;
    }
  else if (tst_comm_getcommclass (env->comm) & TST_MPI_INTRA_COMM)
    {
      MPI_CHECK (MPI_Comm_rank (comm, &comm_rank));
      MPI_CHECK (MPI_Comm_size (comm, &comm_size));

      if (comm_size > 1)
        {
          send_to = (comm_rank + 1) % comm_size;
          recv_from = (comm_rank + comm_size - 1) % comm_size;
        }
      else
        {
          send_to = MPI_PROC_NULL;
          recv_from = MPI_PROC_NULL;
        }
    }
  else
    ERROR (EINVAL, "tst_p2p_simple_ring cannot run with this kind of communicator");

  DEBUG (printf ("(Rank:%d) comm_rank:%d comm_size:%d "
                 "send_to:%d recv_from:%d 4711:%d\n",
                 tst_global_rank, comm_rank, comm_size,
                 send_to, recv_from, 4711));

  if (comm_rank == 0)
    {
      MPI_CHECK (MPI_Send (send_buffer, env->values_num, type, send_to, 4711, comm));
      MPI_CHECK (MPI_Recv (recv_buffer, env->values_num, type, recv_from, 4711, comm, &status));
    }
  else
    {
      MPI_CHECK (MPI_Recv (recv_buffer, env->values_num, type, recv_from, 4711, comm, &status));
      MPI_CHECK (MPI_Send (send_buffer, env->values_num, type, send_to, 4711, comm));
    }

  if (status.MPI_SOURCE != recv_from ||
      (recv_from != MPI_PROC_NULL && status.MPI_TAG != 4711) ||
      (recv_from == MPI_PROC_NULL && status.MPI_TAG != MPI_ANY_TAG))
    ERROR (EINVAL, "Error in status");

  if (recv_from != MPI_PROC_NULL)
    tst_test_checkstandardarray (env, recv_buffer, recv_from);

  return 0;
}

int tst_p2p_simple_ring_cleanup (const struct tst_env * env)
{
  tst_type_freevalues (env->type, send_buffer, env->values_num);
  tst_type_freevalues (env->type, recv_buffer, env->values_num);
  return 0;
}
