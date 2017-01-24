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
#include "config.h"
#ifdef HAVE_STDLIB_H
#  include <stdlib.h>
#endif
#include <mpi.h>
#include "mpi_test_suite.h"
#include "tst_output.h"


int tst_p2p_alltoall_probe_anysource_init (struct tst_env * env)
{
  int comm_rank;
  int comm_size;
  int num_threads = 1;
  MPI_Comm comm;
  int i;

  tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "(Rank:%d) env->comm:%d env->type:%d env->values_num:%d\n",
                     tst_global_rank, env->comm, env->type, env->values_num);

#ifdef HAVE_MPI2_THREADS
  num_threads = tst_thread_num_threads();
#endif

  /*
   * Now, initialize the buffer
   */
  comm = tst_comm_getcomm (env->comm);
  if (tst_comm_getcommclass (env->comm) & TST_MPI_INTRA_COMM)
    MPI_CHECK (MPI_Comm_size (comm, &comm_size));
  else
    MPI_CHECK (MPI_Comm_remote_size (comm, &comm_size));

  MPI_CHECK (MPI_Comm_rank (comm, &comm_rank));

  if ((env->send_buffer_array = malloc (comm_size * sizeof (char *))) == NULL)
    ERROR (errno, "malloc");

  if ((env->recv_buffer_array = malloc (comm_size * sizeof (char *))) == NULL)
    ERROR (errno, "malloc");

  if ((env->req_buffer = malloc (comm_size * sizeof (MPI_Request))) == NULL)
    ERROR (errno, "malloc");

  if ((env->status_buffer = malloc (comm_size * sizeof (MPI_Status))) == NULL)
    ERROR (errno, "malloc");

  for (i=0; i < comm_size; i++)
    {
      env->send_buffer_array[i] = tst_type_allocvalues (env->type, env->values_num);
      tst_type_setstandardarray (env->type, env->values_num, env->send_buffer_array[i], comm_rank + i);

      env->recv_buffer_array[i] = tst_type_allocvalues (env->type, env->values_num);

      env->req_buffer[i] = MPI_REQUEST_NULL;
      memset (&(env->status_buffer[i]), 0, sizeof (MPI_Status));
    }

  env->mpi_buffer_size = num_threads * (tst_type_gettypesize (env->type) * env->values_num * (comm_size - 1) + comm_size * MPI_BSEND_OVERHEAD);
  if ((env->mpi_buffer = malloc (env->mpi_buffer_size)) == NULL)
    ERROR (errno, "malloc");
#ifdef HAVE_MPI2_THREADS
  if ( tst_thread_get_num () == 0 ) {
#endif
  MPI_CHECK (MPI_Buffer_attach (env->mpi_buffer, env->mpi_buffer_size));
#ifdef HAVE_MPI2_THREADS
  }
#endif

  return 0;
}

int tst_p2p_alltoall_probe_anysource_run (struct tst_env * env)
{
  int comm_size;
  int comm_rank;
  int source;
  int dest;
  int tag;
  int recv_count;
  int received_num;
  MPI_Comm comm;
  MPI_Datatype type;
  MPI_Status status;

  comm = tst_comm_getcomm (env->comm);
  type = tst_type_getdatatype (env->type);

  if (tst_comm_getcommclass (env->comm) & TST_MPI_INTRA_COMM)
    MPI_CHECK (MPI_Comm_size (comm, &comm_size));
  else
    MPI_CHECK (MPI_Comm_remote_size (comm, &comm_size));

  received_num = comm_size - 1;

  MPI_CHECK (MPI_Comm_rank (comm, &comm_rank));

  tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "(Rank:%d) comm_size:%d comm_rank:%d\n",
                 tst_global_rank, comm_size, comm_rank);

  /*
   * We mix sending and receiving of messages and mark the already received messages.
   * We however do not send to ourselves.
   */
  for (dest = (comm_rank+1) % comm_size; dest != comm_rank; dest = (dest + 1)%comm_size)
    {
      int flag;

      tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "(Rank:%d) Going to MPI_Ibsend to dest:%d\n",
                     comm_rank, dest);

      MPI_CHECK (MPI_Ibsend (env->send_buffer_array[dest], env->values_num, type, dest, comm_rank, comm, &env->req_buffer[dest]));
      MPI_CHECK (MPI_Iprobe (MPI_ANY_SOURCE, MPI_ANY_TAG, comm, &flag, &status));
      if (flag)
        {
          tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "(Rank:%d) Early finish from source:%d received_num:%d\n",
                             comm_rank, status.MPI_SOURCE, received_num);
          if (status.MPI_SOURCE < 0 ||
              status.MPI_SOURCE >= comm_size ||
              status.MPI_SOURCE == comm_rank ||
              status.MPI_TAG != status.MPI_SOURCE)
            ERROR (EINVAL, "Error in status after MPI_Iprobe");

          source = status.MPI_SOURCE;
          tag = status.MPI_TAG;

          MPI_CHECK (MPI_Recv (env->recv_buffer_array[source], env->values_num, type, source, tag, comm, &status));
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
          tst_test_checkstandardarray (env, env->recv_buffer_array[source], source + comm_rank);

          received_num--;
        }
    }

  MPI_CHECK (MPI_Waitall (comm_size, env->req_buffer, env->status_buffer));

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

      MPI_CHECK (MPI_Recv (env->recv_buffer_array[source], env->values_num, type, source, tag, comm, &status));
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

      tst_test_checkstandardarray (env, env->recv_buffer_array[source], source + comm_rank);

      received_num--;
    }

  return 0;
}

int tst_p2p_alltoall_probe_anysource_cleanup (struct tst_env * env)
{
  MPI_Comm comm;
  int comm_size;
  int i;

#ifdef HAVE_MPI2_THREADS
  if ( tst_thread_get_num () == 0 ) {
#endif
  MPI_CHECK (MPI_Buffer_detach (&env->mpi_buffer, &env->mpi_buffer_size));
#ifdef HAVE_MPI2_THREADS
  }
#endif
  free (env->mpi_buffer);
  env->mpi_buffer = NULL;
  env->mpi_buffer_size = 0;

  comm = tst_comm_getcomm (env->comm);
  if (tst_comm_getcommclass (env->comm) & TST_MPI_INTRA_COMM)
    MPI_CHECK (MPI_Comm_size (comm, &comm_size));
  else
    MPI_CHECK (MPI_Comm_remote_size (comm, &comm_size));


  for (i=0; i < comm_size; i++)
    {
      tst_type_freevalues (env->type, env->recv_buffer_array[i], env->values_num);
      tst_type_freevalues (env->type, env->send_buffer_array[i], env->values_num);
    }

  free (env->send_buffer_array);
  free (env->recv_buffer_array);
  free (env->req_buffer);
  free (env->status_buffer);

  return 0;
}
