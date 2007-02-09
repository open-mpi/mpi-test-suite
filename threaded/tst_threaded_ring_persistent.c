/*
 * File: tst_threaded_ring_persistent.c
 *
 * Functionality:
 *  Simple point-to-point ring-communication test using MPI_Send_init, MPI_Recv_init and MPI_Start, MPI_Wait.
 *  Works with intra-communicators and up to now with any C (standard and struct) type.
 *  Does not work with MPI_COMM_SELF.
 *
 * Author: Christoph Niethammer
 *
 * Date: Jan 31th 2007
 */
#include "mpi.h"
#include "mpi_test_suite.h"
#include "tst_output.h"

#undef DEBUG
#define DEBUG(x)

/*
 * Set this to 1 if you want to use MPI_Startall instead seperate MPI_Starts
 * Does 
 */
#define STARTALL 0
/*
 * Set this to 1 if you want to use MPI_Waitall instead seperate MPI_Waits
 */
#define WAITALL  1

int tst_threaded_ring_persistent_init (struct tst_env * env)
{
  int comm_rank;
  MPI_Comm comm;
  MPI_Datatype type;
  int thread_num;
  int num_threads;
  int comm_size;
  int send_to;
  int recv_from;
  int thread_tag_to;
  int thread_tag_from;
  
  tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "(Rank:%d) env->comm:%d env->type:%d env->values_num:%d\n",
                 tst_global_rank, env->comm, env->type, env->values_num);

  env->send_buffer = tst_type_allocvalues (env->type, env->values_num);
  env->recv_buffer = tst_type_allocvalues (env->type, env->values_num);
  comm = tst_comm_getcomm (env->comm);
  MPI_CHECK (MPI_Comm_rank (comm, &comm_rank));
  type = tst_type_getdatatype (env->type);
  thread_num  = tst_thread_get_num ();
  num_threads = tst_thread_num_threads ();
  
  /*
   * Now, initialize the send_buffer
   */
  tst_type_setstandardarray (env->type, env->values_num, env->send_buffer, comm_rank);

  if ( (env->req_buffer = (MPI_Request *) malloc (sizeof (MPI_Request) * 2)) == NULL )
    ERROR (errno, "malloc");
  if ( (env->status_buffer = malloc (sizeof (MPI_Status) * 2)) == NULL )
    ERROR (errno, "malloc");


  /*
   * Now initialise communication
   */
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
    ERROR (EINVAL, "tst_threaded_ring_persistent cannot run with this kind of communicator");

  thread_tag_to = env->tag + (thread_num + 1) % num_threads;
  thread_tag_from = env->tag + thread_num;


  tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "Adresses of allocated requests:\tRequest 0: %p\tRequest 1: %p, MPI_REQUEST_NULL: %p\n", &(env->req_buffer[0]), &(env->req_buffer[1]), MPI_REQUEST_NULL);
  tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "Before init:\tRequest 0: %p\tRequest 1: %p, MPI_REQUEST_NULL: %p\n", env->req_buffer[0], env->req_buffer[1], MPI_REQUEST_NULL);
 
  MPI_CHECK (MPI_Send_init (env->send_buffer, env->values_num, type, send_to, thread_tag_to, comm, &(env->req_buffer[0])));
  MPI_CHECK (MPI_Recv_init (env->recv_buffer, env->values_num, type, recv_from, thread_tag_from, comm,  &(env->req_buffer[1])));
  tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "After init:\tRequest 0: %p\tRequest 1: %p, MPI_REQUEST_NULL: %p\n", env->req_buffer[0], env->req_buffer[1], MPI_REQUEST_NULL);

  return 0;
}

int tst_threaded_ring_persistent_run (struct tst_env * env)
{
  int comm_size;
  int comm_rank;
  int send_to;
  int recv_from;
  int recv_count;
  MPI_Comm comm;
  MPI_Datatype type;

  int thread_tag;

  
  comm = tst_comm_getcomm (env->comm);
  type = tst_type_getdatatype (env->type);
  thread_tag = env->tag + tst_thread_get_num ();
      
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
    ERROR (EINVAL, "tst_threaded_ring_persistent cannot run with this kind of communicator");
  /*
   * Now start communikation and wait until communication ends
   */

  tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "Before start:\tRequest 0: %p\tRequest 1: %p, MPI_REQUEST_NULL: %p\n", env->req_buffer[0], env->req_buffer[1], MPI_REQUEST_NULL);

#if STARTALL
  MPI_CHECK (MPI_Startall (2, env->req_buffer));
#else
  MPI_CHECK (MPI_Start (&(env->req_buffer[0])));
  MPI_CHECK (MPI_Start (&(env->req_buffer[1])));
#endif
  
  tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "After start:\tRequest 0: %p\tRequest 1: %p, MPI_REQUEST_NULL: %p\n", env->req_buffer[0], env->req_buffer[1], MPI_REQUEST_NULL);
  tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "Before wait:\tRequest 0: %p\tRequest 1: %p, MPI_REQUEST_NULL: %p\n", env->req_buffer[0], env->req_buffer[1], MPI_REQUEST_NULL);
#if WAITALL
  MPI_CHECK (MPI_Waitall (2, env->req_buffer, env->status_buffer));
#else
  MPI_CHECK (MPI_Wait (&(env->req_buffer[0]), &(env->status_buffer[0])));
  MPI_CHECK (MPI_Wait (&(env->req_buffer[1]), &(env->status_buffer[1])));
#endif
  tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "After wait:\tRequest 0: %p\tRequest 1: %p, MPI_REQUEST_NULL: %p\n", env->req_buffer[0], env->req_buffer[1], MPI_REQUEST_NULL);
  
  /*
   * Now verify the sent data
   */
  tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "(Rank:%d) comm_rank:%d comm_size:%d "
                 "send_to:%d recv_from:%d env->tag:%d\n",
                 tst_global_rank, comm_rank, comm_size,
                 send_to, recv_from, env->tag);

  if (env->status_buffer[1].MPI_SOURCE != recv_from)
    {
     tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "source was %d instead %d\n",
                        env->status_buffer[1].MPI_SOURCE, recv_from);
      ERROR (EINVAL, "Error in status");
    }
  if (recv_from != MPI_PROC_NULL)
    {
      if (env->status_buffer[1].MPI_TAG != thread_tag)
        {
          tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "tag was %d instead %d (, MPI_ANY_TAG: %d)\n",
                        env->status_buffer[1].MPI_TAG, thread_tag,
                        MPI_ANY_TAG);
          ERROR (EINVAL, "Error in status");
        }
      if (tst_mode == TST_MODE_STRICT)
        {
          MPI_CHECK(MPI_Get_count(&(env->status_buffer[1]), type, &recv_count));
          if (recv_count != env->values_num)
            ERROR(EINVAL, "Error in count");
        }
      tst_test_checkstandardarray (env, env->recv_buffer, recv_from);
    }
  if ((env->req_buffer[0] == MPI_REQUEST_NULL) ||
        (env->req_buffer[1] == MPI_REQUEST_NULL))
    {
      tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "Request 0: %p\tRequest 1: %p (, MPI_REQUEST_NULL: %p)\n", env->req_buffer[0], env->req_buffer[1], MPI_REQUEST_NULL);
      ERROR (EINVAL, "Error in request");
    }
  return 0;
}

int tst_threaded_ring_persistent_cleanup (struct tst_env * env)
{
  int i;
  
  for (i = 0; i < 2; i++)
    MPI_CHECK (MPI_Request_free (&(env->req_buffer[i])));
  
  tst_type_freevalues (env->type, env->send_buffer, env->values_num);
  tst_type_freevalues (env->type, env->recv_buffer, env->values_num);

  free (env->req_buffer);
  free (env->status_buffer);
  return 0;
}
