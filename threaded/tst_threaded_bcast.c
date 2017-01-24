/*
 * File: tst_p2p_simple_ring.c
 *
 * Functionality:
 *  Simultanious broadcast on different Threads and communicators.
 *
 * Author: Christoph Niethammer
 *
 * Date: Jan 18th 2007
 */
#include <mpi.h>
#include "mpi_test_suite.h"
#include "tst_output.h"

#ifdef HAVE_PTHREAD_H
#  include <pthread.h>
#endif


int debug_wait = 1;

int tst_threaded_ring_init (struct tst_env * env)
{
  // while (DebugWait) ;
  int comm_rank;
  int thread_num;
  MPI_Comm comm;
  MPI_Group 

  tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "(Rank:%d) env->comm:%d env->type:%d env->values_num:%d\n",
                 tst_global_rank, env->comm, env->type, env->values_num);

  env->send_buffer = tst_type_allocvalues (env->type, env->values_num);
  env->recv_buffer = tst_type_allocvalues (env->type, env->values_num);

  comm = tst_comm_getcomm (env->comm);
  MPI_CHECK (MPI_Comm_rank (comm, &comm_rank));
  thread_num = tst_thread_get_num();

  /* initialize the send_buffer */
  tst_type_setstandardarray (env->type, env->values_num, env->send_buffer, comm_rank);

  return 0;
}

int tst_threaded_ring_run (struct tst_env * env)
{
  int comm_size;
  int comm_rank;
  int send_to;
  int recv_from;
  int recv_count;
  MPI_Comm comm;
  MPI_Datatype type;
  MPI_Status status;
  int thread_num;
  int num_threads = tst_thread_num_threads();
  int thread_tag_to;
  int thread_tag_from;

  thread_num = tst_thread_get_num();
  comm = tst_comm_getcomm (env->comm);
  type = tst_type_getdatatype (env->type);
  /*
   * Calculate the tags to specify the number of the thread to send to and the thread
   * number to recieve from.
   * The tags are the sum of the constant env->tag and a threadspecific number.
   */
  thread_tag_to = env->tag + (thread_num + 1) % num_threads;    /* number of next thread */
  thread_tag_from = env->tag + (thread_num);                    /* already knows it's number */

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
    ERROR (EINVAL, "tst_threaded_ring cannot run with this kind of communicator");

  tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "(Rank:%d) comm_rank:%d comm_size:%d send_to:%d recv_from:%d\n",
                 tst_global_rank, comm_rank, comm_size, send_to, recv_from);

  if (0 == thread_num) {
    MPI_CHECK (MPI_Send (env->send_buffer, env->values_num, type, send_to, thread_tag_to, comm));
    MPI_CHECK (MPI_Recv (env->recv_buffer, env->values_num, type, recv_from, thread_tag_from, comm, &status));
  } else {
    MPI_CHECK (MPI_Recv (env->recv_buffer, env->values_num, type, recv_from, thread_tag_from, comm, &status));
    MPI_CHECK (MPI_Send (env->send_buffer, env->values_num, type, send_to, thread_tag_to, comm));
  }

  if (status.MPI_SOURCE != recv_from ||
      (recv_from != MPI_PROC_NULL && status.MPI_TAG != thread_tag_from) ||
      (recv_from == MPI_PROC_NULL && status.MPI_TAG != MPI_ANY_TAG))
    ERROR (EINVAL, "Error in status");

  if (recv_from != MPI_PROC_NULL)
    {
      if (tst_mode == TST_MODE_STRICT)
        {
          MPI_CHECK(MPI_Get_count(&status, type, &recv_count));
          if (recv_count != env->values_num)
            ERROR(EINVAL, "Error in count");
        }
      tst_test_checkstandardarray (env, env->recv_buffer, recv_from);
    }

  return 0;
}

int tst_threaded_ring_cleanup (struct tst_env * env)
{
  tst_type_freevalues (env->type, env->send_buffer, env->values_num);
  tst_type_freevalues (env->type, env->recv_buffer, env->values_num);

  return 0;
}
