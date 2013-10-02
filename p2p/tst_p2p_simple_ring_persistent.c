/*
 * File: tst_p2p_simple_ring_persistent.c
 *
 * Functionality:
 *  Simple point-to-point ring-communication test using MPI_Send_init and MPI_Recv_init starting
 *  with process zero.
 *  Works with intra-communicators and up to now with any C (standard and struct) type.
 *
 * Author: Rainer Keller
 *
 * Date: Aug 8th 2008
 */
#include "mpi.h"
#include "mpi_test_suite.h"
#include "tst_output.h"

#undef DEBUG
#define DEBUG(x)


int tst_p2p_simple_ring_persistent_init (struct tst_env * env)
{
  MPI_Comm comm;
  MPI_Datatype type;
  int comm_size;
  int comm_rank;
  int send_to;
  int recv_from;

  tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "(Rank:%d) env->comm:%d env->type:%d env->values_num:%d\n",
                     tst_global_rank, env->comm, env->type, env->values_num);

  env->send_buffer = tst_type_allocvalues (env->type, env->values_num);
  env->recv_buffer = tst_type_allocvalues (env->type, env->values_num);

  comm = tst_comm_getcomm (env->comm);
  type = tst_type_getdatatype (env->type);
  MPI_CHECK (MPI_Comm_rank (comm, &comm_rank));
  MPI_CHECK (MPI_Comm_size (comm, &comm_size));

  /*
   * Now, initialize the send_buffer
   */
  tst_type_setstandardarray (env->type, env->values_num, env->send_buffer, comm_rank);

  if ( NULL == (env->req_buffer = (MPI_Request *) malloc (2 * sizeof (MPI_Request))))
    ERROR (errno, "malloc");

  /*
   * Now initialize communication
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
      send_to = (comm_rank + 1) % comm_size;
      recv_from = (comm_rank + comm_size - 1) % comm_size;
    }
  else
    ERROR (EINVAL, "tst_p2p_simple_ring_persistent cannot run with this kind of communicator");

  MPI_CHECK (MPI_Send_init (env->send_buffer, env->values_num, type, send_to, env->tag, comm, &(env->req_buffer[0])));
  MPI_CHECK (MPI_Recv_init (env->recv_buffer, env->values_num, type, recv_from, env->tag, comm, &(env->req_buffer[1])));

  return 0;
}

int tst_p2p_simple_ring_persistent_run (struct tst_env * env)
{
  int comm_size;
  int comm_rank;
  int send_to;
  int recv_from;
  int recv_count;
  MPI_Comm comm;
  MPI_Datatype type;
  MPI_Status statuses[2];

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
    ERROR (EINVAL, "tst_p2p_simple_ring_persistent cannot run with this kind of communicator");


  tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "(Rank:%d) comm_rank:%d comm_size:%d "
                 "send_to:%d recv_from:%d env->tag:%d\n",
                 tst_global_rank, comm_rank, comm_size,
                 send_to, recv_from, env->tag);
  /*
   * Now start communication and wait until communication ends
   */
  MPI_CHECK (MPI_Startall (2, env->req_buffer));
  MPI_CHECK (MPI_Waitall (2, env->req_buffer, statuses));

  /*
   * Now verify the sent data
   */
  if (env->req_buffer[0] == MPI_REQUEST_NULL ||
      env->req_buffer[1] == MPI_REQUEST_NULL)
    ERROR (EINVAL, "Error in requests");

  if (statuses[1].MPI_SOURCE != recv_from ||
      (recv_from != MPI_PROC_NULL && statuses[1].MPI_TAG != env->tag) ||
      (recv_from == MPI_PROC_NULL && statuses[1].MPI_TAG != MPI_ANY_TAG))
    {
      if (statuses[1].MPI_SOURCE == MPI_ANY_SOURCE && statuses[1].MPI_TAG == MPI_ANY_TAG) {
        tst_output_printf(DEBUG_LOG, TST_REPORT_MAX, "empty status detected\n");
      }
      else {
        if (statuses[1].MPI_SOURCE != recv_from)
          printf ("statuses[1].MPI_SOURCE:%d instead of %d (MPI_ANY_SOURCE:%d MPI_PROC_NULL:%d)\n",
                  statuses[1].MPI_SOURCE, recv_from, MPI_ANY_SOURCE, MPI_PROC_NULL);

        if (recv_from == MPI_PROC_NULL && statuses[1].MPI_TAG != MPI_ANY_TAG)
          printf ("MPI_PROC_NULL: statuses[1].tag:%d instead of MPI_ANY_TAG:%d\n",
                  statuses[1].MPI_TAG, MPI_ANY_TAG);

        ERROR (EINVAL, "Error in statuses");
      }
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

int tst_p2p_simple_ring_persistent_cleanup (struct tst_env * env)
{
  tst_type_freevalues (env->type, env->send_buffer, env->values_num);
  tst_type_freevalues (env->type, env->recv_buffer, env->values_num);

  free (env->req_buffer);
  return 0;
}
