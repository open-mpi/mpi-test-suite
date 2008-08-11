/*
 * File: tst_p2p_alltoall_issend.c
 *
 * Functionality:
 *  This test is derived from tst_p2p_alltoall.c
 *  The sending here is done with an issend.
 *
 *
 * Author: Rainer Keller
 *
 * Date: Oct 14th 2003
 */
#include "mpi.h"
#include "mpi_test_suite.h"
#include "tst_output.h"

#undef DEBUG
#define DEBUG(x)


int tst_p2p_alltoall_issend_init (struct tst_env * env)
{
  MPI_Comm comm;
  int comm_rank;
  int comm_size;
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

  env->send_buffer = tst_type_allocvalues (env->type, env->values_num);
  tst_type_setstandardarray (env->type, env->values_num,
                             env->send_buffer, comm_rank);

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

int tst_p2p_alltoall_issend_run (struct tst_env * env)
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
      MPI_CHECK (MPI_Issend (env->send_buffer, env->values_num, type,
                             rank, env->tag, comm,
                             &(env->req_buffer[2*rank + 0])));

      if (env->req_buffer[2*rank + 0] == MPI_REQUEST_NULL)
        ERROR (EINVAL, "Error in request after MPI_Issend");

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
           MPI_CHECK(MPI_Get_count(&(env->status_buffer[2*rank+1]), type, &recv_count));
           if(recv_count != env->values_num)
              ERROR(EINVAL, "Error in Count");
        }
      tst_test_checkstandardarray (env,
                                   env->recv_buffer_array[rank], comm_size - rank - 1);
    }
  return 0;
}

int tst_p2p_alltoall_issend_cleanup (struct tst_env * env)
{
  MPI_Comm comm;
  int i;
  int comm_size;

  tst_type_freevalues (env->type, env->send_buffer, env->values_num);

  comm = tst_comm_getcomm (env->comm);

  if (tst_comm_getcommclass (env->comm) & TST_MPI_INTRA_COMM)
    MPI_CHECK (MPI_Comm_size (comm, &comm_size));
  else
    MPI_CHECK (MPI_Comm_remote_size (comm, &comm_size));

  for (i = 0; i < comm_size; i++)
    tst_type_freevalues (env->type, env->recv_buffer_array[i], env->values_num);

  free (env->recv_buffer_array);
  free (env->req_buffer);
  free (env->status_buffer);

  return 0;
}
