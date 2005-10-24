/*
 * File: tst_p2p_alltoall.c
 *
 * Functionality:
 *  This test features sending from every processes (the same array)
 *  to every one else, sending with sendrecv between pairs of processes.
 *  Works with intra- and inter- communicators and up to now with
 *  any C (standard and struct) type.
 *
 * Author: Rainer Keller
 *
 * Date: Sep 8th 2005
 */
#include "mpi.h"
#include "mpi_test_suite.h"

#undef DEBUG
#define DEBUG(x)

static char * send_buffer = NULL;
static char ** recv_buffer = NULL;

int tst_p2p_alltoall_sendrecv_init (const struct tst_env * env)
{
  MPI_Comm comm;
  int comm_rank;
  int comm_size;
  int i;

  DEBUG (printf ("(Rank:%d) env->comm:%d env->type:%d env->values_num:%d\n",
                 tst_global_rank, env->comm, env->type, env->values_num));

  /*
   * Now, initialize the buffer
   */
  comm = tst_comm_getcomm (env->comm);
  MPI_CHECK (MPI_Comm_rank (comm, &comm_rank));

  if (tst_comm_getcommclass (env->comm) == TST_MPI_INTER_COMM)
    MPI_CHECK (MPI_Comm_remote_size (comm, &comm_size));
  else
    MPI_CHECK (MPI_Comm_size (comm, &comm_size));

  send_buffer = tst_type_allocvalues (env->type, env->values_num);
  tst_type_setstandardarray (env->type, env->values_num,
                             send_buffer, comm_rank);

  if ((recv_buffer = malloc (sizeof (char *) * comm_size)) == NULL)
    ERROR (errno, "malloc");

  for (i=0; i < comm_size; i++)
    {
      recv_buffer[i] = tst_type_allocvalues (env->type, env->values_num);
    }

  return 0;
}

int tst_p2p_alltoall_sendrecv_run (const struct tst_env * env)
{
  int comm_size;
  int comm_rank;
  int partner;
  MPI_Comm comm;
  MPI_Datatype type;

  comm = tst_comm_getcomm (env->comm);
  type = tst_type_getdatatype (env->type);

  if (tst_comm_getcommclass (env->comm) == TST_MPI_INTRA_COMM)
    MPI_CHECK (MPI_Comm_size (comm, &comm_size));
  else
    MPI_CHECK (MPI_Comm_remote_size (comm, &comm_size));

  MPI_CHECK (MPI_Comm_rank (comm, &comm_rank));

  DEBUG (printf ("(Rank:%d) comm_size:%d comm_rank:%d\n",
                 tst_global_rank, comm_size, comm_rank));

  /*
   * We sendrecv in the following order:
   *  0 <-> n-1
   *  1 <-> n-2
   * n/2<->n/2+1
   */
  if (comm_rank < comm_size/2)
    {
      DEBUG (printf ("(Rank:%d) LOWER HALF comm_size:%d partner:%d-%d\n",
                     comm_rank, comm_size, comm_size-1, comm_size/2));

      for (partner = comm_size-1; partner > comm_size/2; partner--)
        {
          MPI_Status status;
          MPI_CHECK (MPI_Sendrecv (send_buffer, env->values_num, type, partner, 4711,
                                recv_buffer[partner], env->values_num, type, partner, 4711,
                                comm, &status));
          if (status.MPI_SOURCE != partner ||
              status.MPI_TAG != 4711)
            {
              DEBUG (printf ("(Rank:%d) comm_size:%d should be:%d "
                             "MPI_SOURCE:%d MPI_TAG:%d\n",
                             comm_rank, comm_size, partner,
                             status.MPI_SOURCE,
                             status.MPI_TAG));
              ERROR (EINVAL, "Error in communication");
            }
          tst_type_checkstandardarray (env->type, env->values_num,
                                 recv_buffer[partner], comm_size - partner - 1);
        }
    }
  else
    {
      DEBUG (printf ("(Rank:%d) UPPER HALF comm_size:%d partner:%d-%d\n",
                     comm_rank, comm_size, comm_size-1, comm_size/2));

      for (partner = 0; partner < comm_size/2; partner++)
        {
          MPI_Status status;
          MPI_CHECK (MPI_Sendrecv (send_buffer, env->values_num, type, partner, 4711,
                                recv_buffer[partner], env->values_num, type, partner, 4711,
                                comm, &status));
          if (status.MPI_SOURCE != partner ||
              status.MPI_TAG != 4711)
            {
              DEBUG (printf ("(Rank:%d) rank:%d comm_size:%d should be:%d "
                             "MPI_SOURCE:%d MPI_TAG:%d\n",
                             comm_rank, rank, comm_size, partner,
                             status.MPI_SOURCE,
                             status.MPI_TAG));
              ERROR (EINVAL, "Error in communication");
            }
          tst_type_checkstandardarray (env->type, env->values_num,
                                 recv_buffer[partner], comm_size - partner - 1);
        }
    }
  return 0;
}

int tst_p2p_alltoall_sendrecv_cleanup (const struct tst_env * env)
{
  MPI_Comm comm;
  int i;
  int comm_size;


  free (send_buffer);

  comm = tst_comm_getcomm (env->comm);

  if (tst_comm_getcommclass (env->comm) == TST_MPI_INTER_COMM)
    MPI_CHECK (MPI_Comm_remote_size (comm, &comm_size));
  else
    MPI_CHECK (MPI_Comm_size (comm, &comm_size));

  for (i = 0; i < comm_size; i++)
    free (recv_buffer[i]);

  free (recv_buffer);

  return 0;
}
