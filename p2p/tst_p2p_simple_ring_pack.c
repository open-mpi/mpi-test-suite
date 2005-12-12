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
static char * send_pack_buffer = NULL;
static char * recv_buffer = NULL;
static char * recv_pack_buffer = NULL;
static int position = 0;

int tst_p2p_simple_ring_pack_init (const struct tst_env * env)
{
  int comm_rank;
  MPI_Comm comm;
  MPI_Datatype type;
  int size;

  DEBUG (printf ("(Rank:%d) env->comm:%d env->type:%d env->values_num:%d\n",
                 tst_global_rank, env->comm, env->type, env->values_num));
  /*
   * Now, initialize the send_buffer
   */
  send_buffer = tst_type_allocvalues (env->type, env->values_num);
  comm = tst_comm_getcomm (env->comm);
  MPI_CHECK (MPI_Comm_rank (comm, &comm_rank));
  tst_type_setstandardarray (env->type, env->values_num, send_buffer, comm_rank);
  type = tst_type_getdatatype (env->type);
  MPI_Pack_size(env->values_num, type, comm, &size);
  send_pack_buffer = (char *)malloc(size);
  /*
   * Now, pack the send_buffer in send_pack_buffer
   */
  position = 0;
  MPI_Pack(send_buffer, env->values_num, type, send_pack_buffer, size, &position, comm);

  recv_buffer = tst_type_allocvalues (env->type, env->values_num);
  recv_pack_buffer = (char *)malloc(size);

  return 0;
}

int tst_p2p_simple_ring_pack_run (const struct tst_env * env)
{
  int comm_size;
  int comm_rank;
  int send_to;
  int recv_from;
  int recv_count;
  int size;
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
      MPI_CHECK (MPI_Send (send_pack_buffer, position, MPI_PACKED, send_to, 4711, comm));
      MPI_CHECK (MPI_Recv (recv_pack_buffer, position, MPI_PACKED, recv_from, 4711, comm, &status));

    }
  else
    {
       MPI_CHECK (MPI_Recv (recv_pack_buffer, position, MPI_PACKED, recv_from, 4711, comm, &status));
       MPI_CHECK (MPI_Send (send_pack_buffer, position, MPI_PACKED, send_to, 4711, comm));
    }

  if (status.MPI_SOURCE != recv_from ||
      (recv_from != MPI_PROC_NULL && status.MPI_TAG != 4711) ||
      (recv_from == MPI_PROC_NULL && status.MPI_TAG != MPI_ANY_TAG))
    ERROR (EINVAL, "Error in status");



  if (recv_from != MPI_PROC_NULL)
    {
      int unpack_position = 0;
      MPI_Pack_size(env->values_num, type, comm, &size);
      MPI_Unpack(recv_pack_buffer, size, &unpack_position, recv_buffer, env->values_num, type, comm );
      if (tst_mode == TST_MODE_STRICT)
        {
          MPI_CHECK(MPI_Get_count(&status, type, &recv_count));
          if(recv_count != env->values_num)
            ERROR(EINVAL, "Error in Count");
        }
      tst_test_checkstandardarray (env, recv_buffer, recv_from);
    }
  return 0;
}

int tst_p2p_simple_ring_pack_cleanup (const struct tst_env * env)
{
  tst_type_freevalues (env->type, send_buffer, env->values_num);
  tst_type_freevalues (env->type, recv_buffer, env->values_num);
  free (send_pack_buffer);
  free (recv_pack_buffer);
  return 0;
}
