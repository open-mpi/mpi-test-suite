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
#include "mpi.h"
#include "mpi_test_suite.h"
#include "tst_output.h"

#undef DEBUG
#define DEBUG(x)
#define ROOT 0

/*
 * XXX
static char * send_buffer = NULL;
static char ** recv_buffer_array = NULL;
static MPI_Status * status_buffer = NULL;
static MPI_Request * req_buffer = NULL;
 */

int tst_p2p_alltoall_persistent_init (struct tst_env * env)
{
  MPI_Comm comm;
  MPI_Datatype type;
  int comm_rank;
  int comm_size;
  int i;
  int current_req;
  int rank;

  tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "(Rank:%d) env->comm:%d env->type:%d env->values_num:%d\n",
                 tst_global_rank, env->comm, env->type, env->values_num);

  /*
   * Now, initialize the buffer
   */
  comm = tst_comm_getcomm (env->comm);
  MPI_CHECK (MPI_Comm_rank (comm, &comm_rank));

  type = tst_type_getdatatype (env->type);

  MPI_CHECK (MPI_Comm_size (comm, &comm_size));

  env->send_buffer = tst_type_allocvalues (env->type, env->values_num);
  tst_type_setstandardarray (env->type, env->values_num,
                             env->send_buffer, comm_rank);

  if ((env->status_buffer = malloc (sizeof (MPI_Status) * 2*(comm_size-1))) == NULL)
    ERROR (errno, "malloc");

  if ((env->recv_buffer_array = malloc (sizeof (char *) * comm_size)) == NULL)
    ERROR (errno, "malloc");

  if ((env->req_buffer = malloc (sizeof (MPI_Request) * 2*comm_size)) == NULL)
    ERROR (errno, "malloc");

  for (i=0; i < comm_size; i++)
    {
      env->recv_buffer_array[i] = tst_type_allocvalues (env->type, env->values_num);
    }

  for (current_req = rank = 0; rank < comm_size; rank++)
    {
      env->req_buffer[current_req] = MPI_REQUEST_NULL;

      if (rank == comm_rank)
        {
          for (i=0; i < comm_size; i++)
            {
              if (i == comm_rank)
                continue;
              MPI_CHECK (MPI_Send_init (env->send_buffer, env->values_num, type,
                                        i, env->tag, comm,
                                        &env->req_buffer[current_req]));

              if (env->req_buffer[current_req] == MPI_REQUEST_NULL)
                ERROR (EINVAL, "Error in request after MPI_Send_init");
              current_req++;
            }
        }
      else
        {
          MPI_CHECK (MPI_Recv_init (env->recv_buffer_array[rank], env->values_num, type,
                                    rank, env->tag, comm,
                                    &env->req_buffer[current_req]));

          if (env->req_buffer[current_req] == MPI_REQUEST_NULL)
            ERROR (EINVAL, "Error in request after MPI_Recv_init");
          current_req++;
        }
    }

  return 0;
}

int tst_p2p_alltoall_persistent_run (struct tst_env * env)
{
  int comm_size;
  int comm_rank;
  int rank;
  int recv_count;
  MPI_Comm comm;
  MPI_Datatype type;
  int current_req;
  int flag;

  comm = tst_comm_getcomm (env->comm);
  type = tst_type_getdatatype (env->type);

  MPI_CHECK (MPI_Comm_size (comm, &comm_size));
  MPI_CHECK (MPI_Comm_rank (comm, &comm_rank));

  tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "(Rank:%d) comm_size:%d comm_rank:%d\n",
                 tst_global_rank, comm_size, comm_rank);

  MPI_CHECK (MPI_Startall (2*(comm_size-1), env->req_buffer));
  MPI_CHECK (MPI_Waitall (2*(comm_size-1), env->req_buffer, env->status_buffer));

  for (current_req = rank = 0; rank < comm_size; rank++)
    {
      if (rank == comm_rank)
        {
          int i;
          for (i=0; i < comm_size; i++)
            {
              if (i == comm_rank)
                continue;

              MPI_CHECK (MPI_Test_cancelled (&env->status_buffer[current_req++], &flag));
              if (flag)
                ERROR (EINVAL, "Error in communication");
            }
        }
      else
        {
          MPI_CHECK (MPI_Test_cancelled (&env->status_buffer[current_req], &flag));
          if (flag)
            ERROR (EINVAL, "Error in communication");

          if (env->status_buffer[current_req].MPI_SOURCE != rank ||
              env->status_buffer[current_req].MPI_TAG != env->tag)
            ERROR (EINVAL, "Error in status");
          if (tst_mode == TST_MODE_STRICT)
            {
              MPI_CHECK(MPI_Get_count(&(env->status_buffer[current_req]), type, &recv_count));
              if(recv_count != env->values_num)
                ERROR(EINVAL, "Error in count");
            }
          current_req++;
          tst_test_checkstandardarray (env, env->recv_buffer_array[rank], rank);
        }
    }
  return 0;
}

int tst_p2p_alltoall_persistent_cleanup (struct tst_env * env)
{
  MPI_Comm comm;
  int i;
  int comm_size;

  comm = tst_comm_getcomm (env->comm);
  MPI_CHECK (MPI_Comm_size (comm, &comm_size));

  tst_type_freevalues (env->type, env->send_buffer, env->values_num);

  for (i = 0; i < 2*(comm_size-1); i++)
    {
      MPI_CHECK (MPI_Request_free (&env->req_buffer[i]));
      if (env->req_buffer[i] != MPI_REQUEST_NULL)
        ERROR (EINVAL, "Request != MPI_REQUEST_NULL");
    }

  for (i = 0; i < comm_size; i++)
    tst_type_freevalues (env->type, env->recv_buffer_array[i], env->values_num);

  free (env->recv_buffer_array);
  free (env->status_buffer);
  free (env->req_buffer);
  return 0;
}
