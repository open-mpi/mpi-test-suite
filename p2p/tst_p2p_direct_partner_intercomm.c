/*
 * File: tst_p2p_direct_partner_intercomm.c
 *
 * Functionality:
 *   Very simple test intercomm-sending...
 *   A process n sends it's direct partner n within the Intercomm.
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

int tst_p2p_direct_partner_intercomm_init (const struct tst_env * env)
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

int tst_p2p_direct_partner_intercomm_run (const struct tst_env * env)
{
  int comm_size;
  int comm_rank;
  int send_to;
  int recv_from;
  int remote_size;
  MPI_Comm comm;
  MPI_Datatype type;
  MPI_Status status;

  comm = tst_comm_getcomm (env->comm);
  type = tst_type_getdatatype (env->type);

  /*
   * In case of Intercommunicator, we just send to our 
   * equivalent process in the remote group -- NO ring communication.
   */
  MPI_CHECK (MPI_Comm_rank (comm, &comm_rank));
  MPI_CHECK (MPI_Comm_size (comm, &comm_size));
  MPI_CHECK (MPI_Comm_remote_size (comm, &remote_size));
      
  if (comm_rank < remote_size)
    {
      send_to = comm_rank;
      recv_from = comm_rank;
    }
  else 
    {
      send_to = MPI_PROC_NULL;
      recv_from = MPI_PROC_NULL;
    }

  DEBUG (printf ("(Rank:%d) comm_rank:%d comm_size:%d remote_size:%d "
		 "send_to:%d recv_from:%d\n",
                 tst_global_rank, comm_rank, comm_size, remote_size,
		 send_to, recv_from));

  /*
   * In case of an Inter communicator, we need to send, then recv!!!!
   * Otherwise (for processes > 0) both will wait in MPI_Recv
   */
  MPI_CHECK (MPI_Sendrecv (send_buffer, env->values_num, type, send_to, 4711,
			   recv_buffer, env->values_num, type, recv_from, 4711, comm, &status));

  if (status.MPI_SOURCE != recv_from ||
      (recv_from != MPI_PROC_NULL && status.MPI_TAG != 4711) ||
      (recv_from == MPI_PROC_NULL && status.MPI_TAG != MPI_ANY_TAG))
    ERROR (EINVAL, "Error in status");

  if (recv_from != MPI_PROC_NULL)
    tst_type_checkstandardarray (env->type, env->values_num, recv_buffer, recv_from);

  return 0;
}

int tst_p2p_direct_partner_intercomm_cleanup (const struct tst_env * env)
{
  free (send_buffer);
  free (recv_buffer);
  return 0;
}
