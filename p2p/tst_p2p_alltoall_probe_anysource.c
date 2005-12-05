/*
 * File: tst_p2p_alltoall_probe_anysource.c
 *
 * Functionality:
 *   With this test process n sends comm_size-1 messages to processes
 *   n+1, ..., comm_size-1, 0, 1, ..., n-1 in a non-blocking manner.
 *   In the send loop, values are already checked for.
 *   After an MPI_Waitall all MPI_Isends should have finished.
 *   So they are waited for in a last loop.
 *
 * Author: Rainer Keller
 *
 * Date: September 9th 2003
 */
#include <stdlib.h>
#include "mpi.h"
#include "mpi_test_suite.h"

#undef DEBUG
#define DEBUG(x)

static char ** buffer_send = NULL;
static char ** buffer_recv = NULL;
static int * received = NULL;
static int received_num;
static MPI_Request * requests = NULL;
static MPI_Status * statuses = NULL;
static char * mpi_buffer = NULL;
static int mpi_buffer_size = 0;

int tst_p2p_alltoall_probe_anysource_init (const struct tst_env * env)
{
  int comm_rank;
  int comm_size;
  MPI_Comm comm;
  int i;

  DEBUG (printf ("(Rank:%d) env->comm:%d env->type:%d env->values_num:%d\n",
                 tst_global_rank, env->comm, env->type, env->values_num));

  /*
   * Now, initialize the buffer
   */
  comm = tst_comm_getcomm (env->comm);
  MPI_CHECK (MPI_Comm_size (comm, &comm_size));
  MPI_CHECK (MPI_Comm_rank (comm, &comm_rank));

  if ((buffer_send = malloc (comm_size * sizeof (char *))) == NULL)
    ERROR (errno, "malloc");

  if ((buffer_recv = malloc (comm_size * sizeof (char *))) == NULL)
    ERROR (errno, "malloc");

  if ((requests = malloc (comm_size * sizeof (MPI_Request))) == NULL)
    ERROR (errno, "malloc");

  if ((statuses = malloc (comm_size * sizeof (MPI_Status))) == NULL)
    ERROR (errno, "malloc");

  if ((received = malloc (comm_size * sizeof (int))) == NULL)
    ERROR (errno, "malloc");

  for (i=0; i < comm_size; i++)
    {
      buffer_send[i] = tst_type_allocvalues (env->type, env->values_num);
      tst_type_setstandardarray (env->type, env->values_num, buffer_send[i], comm_rank + i);

      buffer_recv[i] = tst_type_allocvalues (env->type, env->values_num);

      requests[i] = MPI_REQUEST_NULL;
      memset (&(statuses[i]), 0, sizeof (MPI_Status));
    }

  mpi_buffer_size = tst_type_gettypesize (env->type) * env->values_num * (comm_size - 1) + MPI_BUFFER_OVERHEAD;
  if ((mpi_buffer = malloc (mpi_buffer_size)) == NULL)
    ERROR (errno, "malloc");

  MPI_CHECK (MPI_Buffer_attach (mpi_buffer, mpi_buffer_size));

  return 0;
}

int tst_p2p_alltoall_probe_anysource_run (const struct tst_env * env)
{
  int comm_size;
  int comm_rank;
  int source;
  int dest;
  int tag;
  int recv_count;
  MPI_Comm comm;
  MPI_Datatype type;
  MPI_Status status;

  comm = tst_comm_getcomm (env->comm);
  type = tst_type_getdatatype (env->type);

  if (tst_comm_getcommclass (env->comm) == TST_MPI_INTRA_COMM)
    MPI_CHECK (MPI_Comm_size (comm, &comm_size));
  else
    MPI_CHECK (MPI_Comm_remote_size (comm, &comm_size));

  received_num = comm_size - 1;

  MPI_CHECK (MPI_Comm_rank (comm, &comm_rank));

  DEBUG (printf ("(Rank:%d) comm_size:%d comm_rank:%d\n",
                 tst_global_rank, comm_size, comm_rank));

  /*
   * We mix sending and receiving of messages and mark the already received messages.
   * We however do not send to ourselves.
   */
  for (dest = (comm_rank+1) % comm_size; dest != comm_rank; dest = (dest + 1)%comm_size)
    {
      int flag;

      DEBUG (printf ("(Rank:%d) Going to MPI_Ibsend to dest:%d\n",
                     comm_rank, dest));

      MPI_CHECK (MPI_Ibsend (buffer_send[dest], env->values_num, type, dest, comm_rank, comm, &requests[dest]));
      MPI_CHECK (MPI_Iprobe (MPI_ANY_SOURCE, MPI_ANY_TAG, comm, &flag, &status));
      if (flag)
        {
          DEBUG (printf ("(Rank:%d) Early finish from source:%d received_num:%d\n",
                         comm_rank, status.MPI_SOURCE, received_num));
          if (status.MPI_SOURCE < 0 ||
              status.MPI_SOURCE >= comm_size ||
              status.MPI_SOURCE == comm_rank ||
              status.MPI_TAG != status.MPI_SOURCE)
            ERROR (EINVAL, "Error in status after MPI_Iprobe");

          source = status.MPI_SOURCE;
          tag = status.MPI_TAG;

          MPI_CHECK (MPI_Recv (buffer_recv[source], env->values_num, type, source, tag, comm, &status));
          if (source != tag ||
              status.MPI_SOURCE != source ||
              status.MPI_TAG != tag)
            ERROR (EINVAL, "Error in status after MPI_Recv");
          if (tst_mode == TST_MODE_STRICT)
            {
              MPI_CHECK(MPI_Get_count(&status, type, &recv_count));
              if(recv_count != env->values_num)
                ERROR(EINVAL, "Error in Count");
            }
          tst_test_checkstandardarray (env, buffer_recv[source], source + comm_rank);

          received_num--;
        }
    }

  MPI_CHECK (MPI_Waitall (comm_size, requests, statuses));

  while (received_num > 0)
    {
      MPI_CHECK (MPI_Probe (MPI_ANY_SOURCE, MPI_ANY_TAG, comm, &status));

      if (status.MPI_SOURCE < 0 ||
          status.MPI_SOURCE >= comm_size ||
          status.MPI_SOURCE == comm_rank ||
          status.MPI_TAG != status.MPI_SOURCE)
        ERROR (EINVAL, "Error in status after MPI_Probe");

      source = status.MPI_SOURCE;
      tag = status.MPI_TAG;

      MPI_CHECK (MPI_Recv (buffer_recv[source], env->values_num, type, source, tag, comm, &status));
      if (source != tag ||
          status.MPI_SOURCE != source ||
          status.MPI_TAG != tag)
        ERROR (EINVAL, "Error in status after MPI_Recv");
      if (tst_mode == TST_MODE_STRICT)
        {
          MPI_CHECK(MPI_Get_count(&status, type, &recv_count));
          if(recv_count != env->values_num)
              ERROR(EINVAL, "Error in Count");
        }

      tst_test_checkstandardarray (env, buffer_recv[source], source + comm_rank);

      received_num--;
    }

  return 0;
}

int tst_p2p_alltoall_probe_anysource_cleanup (const struct tst_env * env)
{
  MPI_Comm comm;
  int comm_size;
  int i;

  MPI_CHECK (MPI_Buffer_detach (&mpi_buffer, &mpi_buffer_size));
  free (mpi_buffer);
  mpi_buffer = NULL;
  mpi_buffer_size = 0;

  comm = tst_comm_getcomm (env->comm);
  MPI_CHECK (MPI_Comm_size (comm, &comm_size));

  for (i=0; i < comm_size; i++)
    {
      tst_type_freevalues (env->type, buffer_recv[i], env->values_num);
      tst_type_freevalues (env->type, buffer_send[i], env->values_num);
    }

  free (buffer_send);
  free (buffer_recv);
  free (received);
  free (requests);
  free (statuses);

  return 0;
}
