/*
 * File: tst_p2p_many_to_one_isend_cancel.c
 *
 * Functionality:
 *  Simple point-to-point many-to-one test, with everyone in the comm sending blocking to
 *  process zero, which receives with MPI_ANY_SOURCE.
 *  Works with intra- and inter- communicators and up to now with any C (standard and
 *  struct) type.
 *  All the started communication is then cancelled.
 *
 * Author: Rainer Keller
 *
 * Date: Dec 28th 2005
 */
#include "mpi.h"
#include "mpi_test_suite.h"

#undef DEBUG
#define DEBUG(x)

#define ROOT 0

static char * send_buffer = NULL;
static char ** recv_buffer = NULL;
static MPI_Request * recv_reqs = NULL;
static MPI_Status * recv_statuses = NULL;
static int * cancelled = NULL;

int tst_p2p_many_to_one_isend_cancel_init (const struct tst_env * env)
{
  int comm_rank;
  int comm_size;
  MPI_Comm comm;

  DEBUG (printf ("(Rank:%d) env->comm:%d env->type:%d env->values_num:%d\n",
                 tst_global_rank, env->comm, env->type, env->values_num));

  /*
   * Now, initialize the buffer
   */
  comm = tst_comm_getcomm (env->comm);
  MPI_CHECK (MPI_Comm_rank (comm, &comm_rank));
  MPI_CHECK (MPI_Comm_size (comm, &comm_size));

  send_buffer = tst_type_allocvalues (env->type, env->values_num);
  tst_type_setstandardarray (env->type, env->values_num, send_buffer, comm_rank);

  if (ROOT == comm_rank)
    {
      int rank;
      recv_buffer = malloc (sizeof(char *) * comm_size);
      if (NULL == recv_buffer)
        ERROR (ENOMEM, "malloc");

      for (rank = 0; rank < comm_size; rank++)
        {
          recv_buffer[rank] = tst_type_allocvalues (env->type, env->values_num);
          if (NULL == recv_buffer[rank])
            ERROR (ENOMEM, "malloc");
        }

      recv_reqs = malloc (sizeof(MPI_Request) * comm_size);
      if (NULL == recv_reqs)
        ERROR (ENOMEM, "malloc");
      for (rank = 0; rank < comm_size; rank++)
        recv_reqs[rank] = MPI_REQUEST_NULL;

      recv_statuses = malloc (sizeof(MPI_Status) * comm_size);
      if (NULL == recv_statuses)
        ERROR (ENOMEM, "malloc");

      cancelled = malloc (sizeof(int) * comm_size);
      if (NULL == cancelled)
        ERROR (ENOMEM, "malloc");
      memset (cancelled, 0, sizeof (int) * comm_size);
    }

  return 0;
}

int tst_p2p_many_to_one_isend_cancel_run (const struct tst_env * env)
{
  int comm_size;
  int comm_rank;
  int hash_value;
  int recv_count;
  MPI_Comm comm;
  MPI_Datatype type;
  MPI_Request send_request;
  int send_cancelled;
  MPI_Status status;

  comm = tst_comm_getcomm (env->comm);
  type = tst_type_getdatatype (env->type);

  if (tst_comm_getcommclass (env->comm) == TST_MPI_INTRA_COMM)
    MPI_CHECK (MPI_Comm_size (comm, &comm_size));
  else
    MPI_CHECK (MPI_Comm_remote_size (comm, &comm_size));

  MPI_CHECK (MPI_Comm_rank (comm, &comm_rank));

  hash_value = tst_hash_value (env);
  DEBUG (printf ("(Rank:%d) comm:%d type:%d test:%d hash_value:%d comm_size:%d comm_rank:%d\n",
                 tst_global_rank,
                 env->comm, env->type, env->test, hash_value,
                 comm_size, comm_rank));

  /*
  ** Even for intercommunicator, only process zero within
  ** (any intra/inter-com<municator) MPI_COMM_WORLD will receive data!
  */

  if (ROOT == comm_rank)
    {
      int rank;

      for (rank = 0; rank < comm_size; rank++)
        {
          MPI_CHECK (MPI_Irecv (recv_buffer[rank], env->values_num, type, rank,
                                hash_value, comm, &recv_reqs[rank]));
          if (recv_reqs[rank] == MPI_REQUEST_NULL)
            ERROR (EINVAL, "recv_reqs[rank] == MPI_REQUEST_NULL");
        }
    }

  /*
   * Yes, the ROOT also sends to himself, cancelling that, too!
   * If that communication is
   *       Cancelled: Good, the wait must complete and send_cancelled=1 -> Irecv is cancelled later
   *   Not Cancelled: Not bad either, then the communication must finish in the Wait.
   * There's no deadlock possible.
   * Either way, the Test_cancelled must give sensible information.
   *
   * For all other processes: The matching Recv has been posted, the communication may finish
   * if the Isend may not be cancelled.
   * Again either way, every process of comm will enter the MPI_Gather.
   */
  MPI_CHECK (MPI_Isend (send_buffer, env->values_num, type, ROOT, hash_value, comm, &send_request));
  MPI_CHECK (MPI_Cancel (&send_request));
  if (MPI_REQUEST_NULL == send_request)
    ERROR (EINVAL, "send_request == MPI_REQUEST_NULL");
  MPI_CHECK (MPI_Wait (&send_request, &status));
  send_cancelled = 0;
  MPI_CHECK (MPI_Test_cancelled (&status, &send_cancelled));
  DEBUG (printf ("(Rank:%d) send_cancelled:%d\n",
                 comm_rank, send_cancelled));
  /*
   * Now collect the cancel information from all the processes
   */
  MPI_CHECK (MPI_Gather (&send_cancelled, 1, MPI_INT, cancelled, 1, MPI_INT, ROOT, comm));

  if (ROOT == comm_rank)
    {
      int rank;
      for (rank = 0; rank < comm_size; rank++)
        DEBUG (printf ("(Rank:%d) cancelled[%d]:%d\n",
                       comm_rank, rank, cancelled[rank]));

      for (rank = 0; rank < comm_size; rank++)
        {
          /*
           * If the Isend has been cancelled on the processes, Cancel the Irecv as well
           */
          if (cancelled[rank])
            {
              DEBUG (printf ("(Rank:%d) cancelled[%d]:%d Going to cancel MPI_Irecv\n",
                             comm_rank, rank, cancelled[rank]));

              MPI_CHECK (MPI_Cancel (&recv_reqs[rank]));
              if (MPI_REQUEST_NULL == recv_reqs[rank])
                ERROR (EINVAL, "recv_reqs[rank] == MPI_REQUEST_NULL");
/* #define HAVE_MPI_LAM */
#ifdef HAVE_MPI_LAM
              /*
               * LAM hangs without this MPI_Request_free, as it doesn't finish the MPI_Waitall of cancelled requests
               */
              MPI_Request_free (&recv_reqs[rank]);
              if (MPI_REQUEST_NULL != recv_reqs[rank])
                ERROR (EINVAL, "recv_reqs[rank] != MPI_REQUEST_NULL");
#endif
            }
          else
            DEBUG (printf ("(Rank:%d) cancelled[%d]:%d\n",
                           comm_rank, rank, cancelled[rank]));
         }
      DEBUG (printf ("(Rank:%d) Before MPI_Waitall\n",
                     comm_rank));

      MPI_CHECK (MPI_Waitall(comm_size, recv_reqs, recv_statuses));

      for (rank = 0; rank < comm_size; rank++)
        {
          int flag;
          status = recv_statuses[rank];
#ifdef HAVE_MPI_LAM
          /*
           * With LAM, we have freed the request, the status is unitialized
           */
          if (cancelled[rank])
            continue;
#endif
          if (status.MPI_TAG != hash_value ||
              status.MPI_SOURCE < 0 ||
              status.MPI_SOURCE > comm_size)
            {
              printf ("(Rank:%d) comm_size:%d hash_value:%d MPI_SOURCE:%d MPI_TAG:%d\n",
                      comm_rank, comm_size, hash_value, status.MPI_SOURCE, status.MPI_TAG);
            }

          if (tst_mode == TST_MODE_STRICT)
            {
               MPI_CHECK(MPI_Get_count(&status, type, &recv_count));
               if(recv_count != env->values_num)
                  ERROR(EINVAL, "Error in Count");
            }
          MPI_CHECK (MPI_Test_cancelled (&recv_statuses[rank], &flag));
          if (cancelled[rank] && !flag)
            ERROR (EINVAL, "cancellation info and MPI_Test_cancelled differ");

          DEBUG (printf ("(Rank:%d) going to check array from source:%d\n",
                         comm_rank, status.MPI_SOURCE));

          if (!flag)
            tst_test_checkstandardarray (env, recv_buffer[rank], status.MPI_SOURCE);
        }
    }

  return 0;
}

int tst_p2p_many_to_one_isend_cancel_cleanup (const struct tst_env * env)
{
  int comm_rank;
  int comm_size;
  int rank;
  MPI_Comm comm;

  comm = tst_comm_getcomm (env->comm);

  if (tst_comm_getcommclass (env->comm) == TST_MPI_INTRA_COMM)
    MPI_CHECK (MPI_Comm_size (comm, &comm_size));
  else
    MPI_CHECK (MPI_Comm_remote_size (comm, &comm_size));
  MPI_CHECK (MPI_Comm_rank (comm, &comm_rank));

  if (ROOT == comm_rank)
    {
      for (rank = 0; rank < comm_size; rank++)
        tst_type_freevalues (env->type, recv_buffer[rank], env->values_num);
      free (recv_buffer);
      free (recv_reqs);
      free (recv_statuses);
      free (cancelled);
    }
  tst_type_freevalues (env->type, send_buffer, env->values_num);
  return 0;
}
