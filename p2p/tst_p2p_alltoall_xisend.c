/*
 * File: tst_p2p_alltoall.c
 *
 * Functionality:
 *  This test features sending from every processes (the same array)
 *  to every one else, sending non-blocking and then doing a waitall.
 *  Works with intra- and inter- communicators and up to now with
 *  any C (standard and struct) type.
 *
 * Author: Rainer Keller
 *
 * Date: Oct 14th 2003
 */
#include <mpi.h>
#include "mpi_test_suite.h"
#include "tst_output.h"

#undef DEBUG
#define DEBUG(x)


int tst_p2p_alltoall_xisend_init (struct tst_env * env)
{
  MPI_Comm comm;
  int comm_rank;
  int comm_size;
  MPI_Datatype type;
  int i;

  tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "(Rank:%d) env->comm:%d env->type:%d env->values_num:%d\n",
                     tst_global_rank, env->comm, env->type, env->values_num);
  /*
   * Now, initialize the buffer
   */
  comm = tst_comm_getcomm (env->comm);
  MPI_CHECK (MPI_Comm_rank (comm, &comm_rank));

  if (tst_comm_getcommclass (env->comm) & TST_MPI_INTRA_COMM)
    MPI_CHECK (MPI_Comm_size (comm, &comm_size));
  else
    MPI_CHECK (MPI_Comm_remote_size (comm, &comm_size));


  /*
   * Now, initialize the send_buffer
   */
  env->send_buffer = tst_type_allocvalues (env->type, 1);
  env->check_buffer = tst_type_allocvalues (env->type, 1);
  tst_type_setvalue (env->type, env->send_buffer, TST_TYPE_SET_VALUE, comm_rank);

  /*
   * Create the derived datatype to send multiple entries of this type
   */

  type = tst_type_getdatatype (env->type);
  MPI_Type_hvector (env->values_num, 1, 0, type, &env->extra_type_send);
  MPI_Type_commit (&env->extra_type_send);

  if ((env->req_buffer = malloc (2 * sizeof (MPI_Request) * comm_size)) == NULL)
    ERROR (errno, "malloc");

  if ((env->status_buffer = malloc (2 * sizeof (MPI_Status) * comm_size)) == NULL)
    ERROR (errno, "malloc");

  if ((env->recv_buffer_array = malloc (sizeof (char *) * comm_size)) == NULL)
    ERROR (errno, "malloc");

  for (i=0; i < comm_size; i++)
    {
      env->recv_buffer_array[i] = tst_type_allocvalues (env->type, env->values_num);
    }

  return 0;
}

int tst_p2p_alltoall_xisend_run (struct tst_env * env)
{
  int comm_size;
  int comm_rank;
  int rank;
  int recv_count;
  MPI_Comm comm;
  MPI_Datatype type;

  comm = tst_comm_getcomm (env->comm);
  type = tst_type_getdatatype (env->type);

  if (tst_comm_getcommclass (env->comm) & TST_MPI_INTRA_COMM)
    MPI_CHECK (MPI_Comm_size (comm, &comm_size));
  else
    MPI_CHECK (MPI_Comm_remote_size (comm, &comm_size));

  MPI_CHECK (MPI_Comm_rank (comm, &comm_rank));

  tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "(Rank:%d) comm_size:%d comm_rank:%d\n",
                 tst_global_rank, comm_size, comm_rank);

  /*
   * We send bottom-up, but receive top-down
   */
  for (rank = 0; rank < comm_size; rank++)
    {
      env->req_buffer[2*rank + 0] = MPI_REQUEST_NULL;
      MPI_CHECK (MPI_Isend (env->send_buffer, 1, env->extra_type_send,
                            rank, env->tag, comm,
                            &(env->req_buffer[2*rank + 0])));

      if (env->req_buffer[2*rank + 0] == MPI_REQUEST_NULL)
        ERROR (EINVAL, "Error in request after MPI_Isend");

      env->req_buffer[2*rank + 1] = MPI_REQUEST_NULL;
      MPI_CHECK (MPI_Irecv (env->recv_buffer_array[rank], env->values_num, type,
                            comm_size - rank - 1, env->tag, comm,
                            &(env->req_buffer[2*rank + 1])));

      if (env->req_buffer[2*rank + 1] == MPI_REQUEST_NULL)
        ERROR (EINVAL, "Error in request after MPI_Irecv");
    }

  MPI_CHECK (MPI_Waitall (comm_size*2, env->req_buffer, env->status_buffer));

  for (rank = 0; rank < comm_size; rank++)
    {
      if (env->req_buffer[2*rank + 0] != MPI_REQUEST_NULL ||
          env->req_buffer[2*rank + 1] != MPI_REQUEST_NULL)
        ERROR (EINVAL, "Requests are not reset to MPI_REQUEST_NULL");

      if (env->status_buffer[2*rank+1].MPI_SOURCE != comm_size - rank - 1  ||
          env->status_buffer[2*rank+1].MPI_TAG != env->tag)
        {
          tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "(Rank:%d) rank:%d comm_size:%d should be:%d "
                             "MPI_SOURCE:%d MPI_TAG:%d\n",
                             comm_rank, rank, comm_size, comm_size - rank-1,
                             env->status_buffer[2*rank+1].MPI_SOURCE,
                             env->status_buffer[2*rank+1].MPI_TAG);
          ERROR (EINVAL, "Error in communication");
        }
      if (tst_mode == TST_MODE_STRICT)
        {
          MPI_CHECK(MPI_Get_count(&env->status_buffer[2*rank+1], type, &recv_count));
          if(recv_count != env->values_num)
              ERROR(EINVAL, "Error in Count");
        }
      /*
       * We cannot check using tst_type_checkstandardarray
       * tst_type_checkstandardarray (env->type, env->values_num,
                                   env->recv_buffer_array[rank], comm_size - rank - 1);
       */

      {
        const int type_size = tst_type_gettypesize (env->type);
        int i;
        tst_type_setvalue (env->type, env->check_buffer, TST_TYPE_SET_VALUE, comm_size - rank - 1);
        for (i = 0; i < env->values_num; i++)
          if (0 != tst_type_cmpvalue (env->type, env->check_buffer, &((env->recv_buffer_array[rank])[i*type_size])))
            {
              if (tst_report >= TST_REPORT_FULL)
                {
                  tst_type_hexdump ("Expected cmp_value", env->check_buffer, type_size);
                  tst_type_hexdump ("Received buffer", &((env->recv_buffer_array[rank])[i*type_size]) , type_size);
                }
              tst_test_recordfailure (env);
            }
      }
    }
  return 0;
}

int tst_p2p_alltoall_xisend_cleanup (struct tst_env * env)
{
  MPI_Comm comm;
  int i;
  int comm_size;

  comm = tst_comm_getcomm (env->comm);

  tst_type_freevalues (env->type, env->send_buffer, 1);
  tst_type_freevalues (env->type, env->check_buffer, 1);

  if (tst_comm_getcommclass (env->comm) & TST_MPI_INTRA_COMM)
    MPI_CHECK (MPI_Comm_size (comm, &comm_size));
  else
    MPI_CHECK (MPI_Comm_remote_size (comm, &comm_size));

  for (i = 0; i < comm_size; i++)
    tst_type_freevalues (env->type, env->recv_buffer_array[i], env->values_num);

  free (env->recv_buffer_array);
  free (env->req_buffer);
  free (env->status_buffer);
  MPI_Type_free (&env->extra_type_send);

  return 0;
}
