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
#include <mpi.h>
#include "mpi_test_suite.h"
#include "tst_output.h"


int tst_p2p_simple_ring_xsend_init (struct tst_env * env)
{
  int comm_rank;
  MPI_Comm comm;
  MPI_Datatype type;

  tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "(Rank:%d) env->comm:%d env->type:%d env->values_num:%d\n",
                 tst_global_rank, env->comm, env->type, env->values_num);

  env->send_buffer = tst_type_allocvalues (env->type, 1);
  env->check_buffer = tst_type_allocvalues (env->type, 1);
  env->recv_buffer = tst_type_allocvalues (env->type, env->values_num);

  /*
   * Now, initialize the send_buffer
   */
  comm = tst_comm_getcomm (env->comm);
  MPI_CHECK (MPI_Comm_rank (comm, &comm_rank));

  tst_type_setvalue (env->type, env->send_buffer, TST_TYPE_SET_VALUE, comm_rank);

  /*
   * Create the derived datatype to send multiple entries of this type
   */
  type = tst_type_getdatatype (env->type);
  MPI_Type_hvector (env->values_num, 1, 0, type, &env->extra_type_send);
  MPI_Type_commit (&env->extra_type_send);
  return 0;
}

int tst_p2p_simple_ring_xsend_run (struct tst_env * env)
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

  tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "(Rank:%d) comm_rank:%d comm_size:%d "
                 "send_to:%d recv_from:%d env->tag:%d\n",
                 tst_global_rank, comm_rank, comm_size,
                 send_to, recv_from, env->tag);

  if (comm_rank == 0)
    {
      MPI_CHECK (MPI_Send (env->send_buffer, 1, env->extra_type_send, send_to, env->tag, comm));
      MPI_CHECK (MPI_Recv (env->recv_buffer, env->values_num, type, recv_from, env->tag, comm, &status));
    }
  else
    {
      MPI_CHECK (MPI_Recv (env->recv_buffer, env->values_num, type, recv_from, env->tag, comm, &status));
      MPI_CHECK (MPI_Send (env->send_buffer, 1, env->extra_type_send, send_to, env->tag, comm));
    }

  if (status.MPI_SOURCE != recv_from ||
      (recv_from != MPI_PROC_NULL && status.MPI_TAG != env->tag) ||
      (recv_from == MPI_PROC_NULL && status.MPI_TAG != MPI_ANY_TAG))
    ERROR (EINVAL, "Error in status");
  if (tst_mode == TST_MODE_STRICT)
    {
      MPI_CHECK(MPI_Get_count(&status, type, &recv_count));
      if(recv_count != env->values_num && recv_from != MPI_PROC_NULL)
        {
          printf ("(Rank:%d) recv_count:%d env->values_num:%d recv_from:%d\n",
                  tst_global_rank, recv_count, env->values_num, recv_from);
          ERROR(EINVAL, "Error in Count");
        }
    }
  if (recv_from != MPI_PROC_NULL)
    {
      const int type_size = tst_type_gettypesize (env->type);
      int i;
      int errors=0;

      tst_type_setvalue (env->type, env->check_buffer, TST_TYPE_SET_VALUE, recv_from);

      for (i = 0; i < env->values_num; i++)
        if (0 != tst_type_cmpvalue (env->type, env->check_buffer, &env->recv_buffer[i*type_size]))
          {
            if (tst_report >= TST_REPORT_FULL)
              {
                tst_type_hexdump ("Expected cmp_value", env->check_buffer, type_size);
                tst_type_hexdump ("Received buffer", &(env->recv_buffer[i*type_size]), type_size);
              }
            errors++;
          }
      if (errors)
        tst_test_recordfailure (env);
    }

  return 0;
}

int tst_p2p_simple_ring_xsend_cleanup (struct tst_env * env)
{
  tst_type_freevalues (env->type, env->send_buffer, 1);
  tst_type_freevalues (env->type, env->check_buffer, 1);
  tst_type_freevalues (env->type, env->recv_buffer, env->values_num);
  MPI_Type_free (&env->extra_type_send);
  return 0;
}
