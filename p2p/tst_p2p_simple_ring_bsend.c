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

static char * mpi_buffer = NULL;
static int mpi_buffer_size = 0;



int tst_p2p_simple_ring_bsend_init (const struct tst_env * env)
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

  mpi_buffer_size = tst_type_gettypesize (env->type) * env->values_num + MPI_BUFFER_OVERHEAD;
  if ((mpi_buffer = malloc (mpi_buffer_size)) == NULL)
    ERROR (errno, "malloc");

  MPI_CHECK (MPI_Buffer_attach (mpi_buffer, mpi_buffer_size));

  return 0;
}

int tst_p2p_simple_ring_bsend_run (const struct tst_env * env)
{
  int comm_size;
  int comm_rank;
  int send_to;
  int recv_from;
  int recv_count;
  MPI_Comm comm;
  MPI_Datatype type;
  MPI_Status status;

  comm = tst_comm_getcomm (env->comm);
  type = tst_type_getdatatype (env->type);

  MPI_CHECK (MPI_Comm_rank (comm, &comm_rank));
  MPI_CHECK (MPI_Comm_size (comm, &comm_size));
  send_to = (comm_rank + 1) % comm_size;
  recv_from = (comm_rank + comm_size - 1) % comm_size;

  DEBUG (printf ("(Rank:%d) comm_rank:%d comm_size:%d send_to:%d recv_from:%d\n",
                 tst_global_rank, comm_rank, comm_size, send_to, recv_from));

  if (comm_rank == 0)
    {
      MPI_CHECK (MPI_Bsend (send_buffer, env->values_num, type, send_to, 4711, comm));
      MPI_CHECK (MPI_Recv (recv_buffer, env->values_num, type, recv_from, 4711, comm, &status));
    }
  else
    {
      MPI_CHECK (MPI_Recv (recv_buffer, env->values_num, type, recv_from, 4711, comm, &status));
      MPI_CHECK (MPI_Bsend (send_buffer, env->values_num, type, send_to, 4711, comm));
    }

  if (status.MPI_SOURCE != recv_from ||
      status.MPI_TAG != 4711)
    ERROR (EINVAL, "Error in status");
  if (tst_mode == TST_MODE_STRICT)
    {
      MPI_CHECK(MPI_Get_count(&status, type, &recv_count));
      if(recv_count != env->values_num)
         ERROR(EINVAL, "Error in Count");
    }

  tst_test_checkstandardarray (env, recv_buffer, recv_from);

  return 0;
}

int tst_p2p_simple_ring_bsend_cleanup (const struct tst_env * env)
{
  MPI_CHECK (MPI_Buffer_detach (&mpi_buffer, &mpi_buffer_size));
  free (mpi_buffer);
  mpi_buffer = NULL;
  mpi_buffer_size = 0;

  tst_type_freevalues (env->type, send_buffer, env->values_num);
  tst_type_freevalues (env->type, recv_buffer, env->values_num);
  return 0;
}
