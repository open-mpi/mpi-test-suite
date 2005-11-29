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

static MPI_Datatype send_type;
static char * send_buffer = NULL;
static char * check_buffer = NULL;
static char * recv_buffer = NULL;

int tst_p2p_simple_ring_xsend_init (const struct tst_env * env)
{
  int comm_rank;
  MPI_Comm comm;
  MPI_Datatype type;

  DEBUG (printf ("(Rank:%d) env->comm:%d env->type:%d env->values_num:%d\n",
                 tst_global_rank, env->comm, env->type, env->values_num));

  send_buffer = tst_type_allocvalues (env->type, 1);
  check_buffer = tst_type_allocvalues (env->type, 1);
  recv_buffer = tst_type_allocvalues (env->type, env->values_num);

  /*
   * Now, initialize the send_buffer
   */
  comm = tst_comm_getcomm (env->comm);
  MPI_CHECK (MPI_Comm_rank (comm, &comm_rank));

  tst_type_setvalue (env->type, send_buffer, TST_TYPE_SET_VALUE, comm_rank);

  /*
   * Create the derived datatype to send multiple entries of this type
   */
  type = tst_type_getdatatype (env->type);
  MPI_Type_hvector (env->values_num, 1, 0, type, &send_type);
  MPI_Type_commit (&send_type);
  return 0;
}

int tst_p2p_simple_ring_xsend_run (const struct tst_env * env)
{
  int comm_size;
  int comm_rank;
  int send_to;
  int recv_from;
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

  DEBUG (printf ("(Rank:%d) comm_rank:%d comm_size:%d "
                 "send_to:%d recv_from:%d 4711:%d\n",
                 tst_global_rank, comm_rank, comm_size,
                 send_to, recv_from, 4711));

  if (comm_rank == 0)
    {
      MPI_CHECK (MPI_Send (send_buffer, 1, send_type, send_to, 4711, comm));
      MPI_CHECK (MPI_Recv (recv_buffer, env->values_num, type, recv_from, 4711, comm, &status));
    }
  else
    {
      MPI_CHECK (MPI_Recv (recv_buffer, env->values_num, type, recv_from, 4711, comm, &status));
      MPI_CHECK (MPI_Send (send_buffer, 1, send_type, send_to, 4711, comm));
    }

  if (status.MPI_SOURCE != recv_from ||
      (recv_from != MPI_PROC_NULL && status.MPI_TAG != 4711) ||
      (recv_from == MPI_PROC_NULL && status.MPI_TAG != MPI_ANY_TAG))
    ERROR (EINVAL, "Error in status");

  if (recv_from != MPI_PROC_NULL)
    {
      const int type_size = tst_type_gettypesize (env->type);
      int i;
      int errors=0;

      tst_type_setvalue (env->type, check_buffer, TST_TYPE_SET_VALUE, recv_from);

      for (i = 0; i < env->values_num; i++)
        if (0 != tst_type_cmpvalue (env->type, check_buffer, &recv_buffer[i*type_size]))
          {
            if (tst_report >= TST_REPORT_FULL)
              {
                tst_type_hexdump ("Expected cmp_value", check_buffer, type_size);
                tst_type_hexdump ("Received buffer", &(recv_buffer[i*type_size]), type_size);
              }
            errors++;
          }
      if (errors)
        tst_test_recordfailure (env);
    }

  return 0;
}

int tst_p2p_simple_ring_xsend_cleanup (const struct tst_env * env)
{
  tst_type_freevalues (env->type, send_buffer, 1);
  tst_type_freevalues (env->type, check_buffer, 1);
  tst_type_freevalues (env->type, recv_buffer, env->values_num);
  MPI_Type_free (&send_type);
  return 0;
}
