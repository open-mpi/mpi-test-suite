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
#include "tst_output.h"

#undef DEBUG
#define DEBUG(x)


int tst_p2p_direct_partner_intercomm_init (struct tst_env * env)
{
  int comm_rank;
  MPI_Comm comm;

  tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "(Rank:%d) env->comm:%d env->type:%d env->values_num:%d\n",
                 tst_global_rank, env->comm, env->type, env->values_num);

  env->send_buffer = tst_type_allocvalues (env->type, env->values_num);
  env->recv_buffer = tst_type_allocvalues (env->type, env->values_num);

  /*
   * Now, initialize the send_buffer
   */
  comm = tst_comm_getcomm (env->comm);
  MPI_CHECK (MPI_Comm_rank (comm, &comm_rank));

  tst_type_setstandardarray (env->type, env->values_num, env->send_buffer, comm_rank);

  return 0;
}

int tst_p2p_direct_partner_intercomm_run (struct tst_env * env)
{
  int comm_size;
  int comm_rank;
  int send_to;
  int recv_from;
  int remote_size;
  int recv_count;
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

  tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "(Rank:%d) comm_rank:%d comm_size:%d remote_size:%d "
                 "send_to:%d recv_from:%d\n",
                 tst_global_rank, comm_rank, comm_size, remote_size,
                 send_to, recv_from);

  /*
   * In case of an Inter communicator, we need to send, then recv!!!!
   * Otherwise (for processes > 0) both will wait in MPI_Recv
   */
  MPI_CHECK (MPI_Sendrecv (env->send_buffer, env->values_num, type, send_to, env->tag,
                           env->recv_buffer, env->values_num, type, recv_from, env->tag, comm, &status));

  if (status.MPI_SOURCE != recv_from ||
      (recv_from != MPI_PROC_NULL && status.MPI_TAG != env->tag) ||
      (recv_from == MPI_PROC_NULL && status.MPI_TAG != MPI_ANY_TAG))
    ERROR (EINVAL, "Error in status");
  if (tst_mode == TST_MODE_STRICT)
    {
       MPI_CHECK(MPI_Get_count(&status, type, &recv_count));
       if(recv_count != env->values_num)
          ERROR(EINVAL, "Error in Count");
    }

  if (recv_from != MPI_PROC_NULL)
    tst_test_checkstandardarray (env, env->recv_buffer, recv_from);

  return 0;
}

int tst_p2p_direct_partner_intercomm_cleanup (struct tst_env * env)
{
  tst_type_freevalues (env->type, env->send_buffer, env->values_num);
  tst_type_freevalues (env->type, env->recv_buffer, env->values_num);
  return 0;
}
