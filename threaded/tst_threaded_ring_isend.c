/*
 * File: tst_threaded_ring.c
 *
 * Functionality:
 *  Simple point-to-point ring-communication test using MPI_Send and MPI_Recv starting
 *  with process zero.
 *  Works with intra-communicators and up to now with any C (standard and struct) type.
 *
 * Author: Christoph Niethammer
 *
 * Date: Nov 17th 2003
 */
#include <mpi.h>
#include "mpi_test_suite.h"
#include "tst_output.h"


int tst_threaded_ring_isend_init (struct tst_env * env)
{
  int comm_rank;
  MPI_Comm comm;
  int thread_num = tst_thread_get_num(); 

  tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "(Rank:%d) env->comm:%d env->type:%d env->values_num:%d\n",
                 tst_global_rank, env->comm, env->type, env->values_num);

  env->send_buffer = tst_type_allocvalues (env->type, env->values_num);
  env->recv_buffer = tst_type_allocvalues (env->type, env->values_num);


  /*
   * Now, initialize the send_buffer
   */
  comm = tst_comm_getcomm (env->comm);
  MPI_CHECK (MPI_Comm_rank (comm, &comm_rank));

  if ( (env->status_buffer = malloc (sizeof (MPI_Status) * 2)) == NULL )
    ERROR (errno, "malloc");

  if (0 == thread_num) 
  {
    tst_thread_alloc_global_requests (2);
    tst_thread_signal_init (2);
  }

  tst_type_setstandardarray (env->type, env->values_num, env->send_buffer, comm_rank);

  return 0;
}

int tst_threaded_ring_isend_run (struct tst_env * env)
{
  int comm_size;
  int comm_rank;
  int send_to;
  int recv_from;
  int recv_count;
  MPI_Comm comm;
  MPI_Datatype type;
  MPI_Status * statuses;
  MPI_Request * requests;

  int num_threads;
  int thread_num;

  num_threads = tst_thread_num_threads();
  thread_num = tst_thread_get_num(); 

  statuses = env->status_buffer;
  requests = tst_thread_get_global_request (0);
  
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

      send_to = (comm_rank + 1) % comm_size;
      recv_from = (comm_rank + comm_size - 1) % comm_size;
    }
  else
    ERROR (EINVAL, "tst_threaded_ring_isend cannot run with this kind of communicator");

  if (0 == thread_num)
    {
      MPI_CHECK (MPI_Irecv (env->recv_buffer, env->values_num, type, recv_from,
            env->tag, comm, &requests[0]));
      MPI_CHECK (MPI_Isend (env->send_buffer, env->values_num, type, send_to,
            env->tag, comm, &requests[1]));
      /*
       * TODO: Signal
       *       Wait on signal
       */
      /*
       * Wake up thread 1 to finish MPI communication
       */
      tst_thread_signal_send (0);
      /*
       * Now wait itself until thread 1 finished communication
       */
      tst_thread_signal_wait (1);
    }
  else if ((num_threads > 0) && (1 == thread_num) )
    {
      /*
       * Wait for thread 0 to initialize the communication
       */
      tst_thread_signal_wait (0);
      MPI_Waitall(2, requests, statuses);
      /*
       * Inform thread 0 that communication has finished 
       */
      tst_thread_signal_send (1);
    }

  tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "(Rank:%d) comm_rank:%d comm_size:%d "
                 "send_to:%d recv_from:%d env->tag:%d\n",
                 tst_global_rank, comm_rank, comm_size,
                 send_to, recv_from, env->tag);


  if (1 == thread_num)
  {
    if (requests[0] != MPI_REQUEST_NULL ||
        requests[1] != MPI_REQUEST_NULL)
      ERROR (EINVAL, "Error in requests");

    if (statuses[0].MPI_SOURCE != recv_from ||
        (recv_from != MPI_PROC_NULL && statuses[0].MPI_TAG != env->tag) ||
        (recv_from == MPI_PROC_NULL && statuses[0].MPI_TAG != MPI_ANY_TAG))
      ERROR (EINVAL, "Error in statuses");
  }
  if (recv_from != MPI_PROC_NULL)
    {
      if (tst_mode == TST_MODE_STRICT)
        {
          MPI_CHECK(MPI_Get_count(&(statuses[1]), type, &recv_count));
          if (recv_count != env->values_num)
            ERROR(EINVAL, "Error in count");
        }
      tst_test_checkstandardarray (env, env->recv_buffer, recv_from);
    }
  return 0;
}

int tst_threaded_ring_isend_cleanup (struct tst_env * env)
{
  int thread_num = tst_thread_get_num(); 

  tst_type_freevalues (env->type, env->send_buffer, env->values_num);
  tst_type_freevalues (env->type, env->recv_buffer, env->values_num);

  if (0 == thread_num) 
  {
    free (env->status_buffer);
    free (env->req_buffer);
    tst_thread_signal_cleanup ();
    tst_thread_free_global_requests ();
  }
  return 0;
}
