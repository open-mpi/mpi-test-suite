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

#undef DEBUG
#define DEBUG(x)

static char * send_buffer = NULL;
static char ** recv_buffer = NULL;
static MPI_Status * status_buffer = NULL;
static int * neighbors = NULL;

int tst_p2p_alltoall_graph_init (const struct tst_env * env)
{
  MPI_Comm comm;
  int comm_rank;
  int neighbors_num;
  int i;

  DEBUG (printf ("(Rank:%d) env->comm:%d env->type:%d env->values_num:%d\n",
                 tst_global_rank, env->comm, env->type, env->values_num));

  /*
   * Now, initialize the buffer
   */
  comm = tst_comm_getcomm (env->comm);
  MPI_CHECK (MPI_Comm_rank (comm, &comm_rank));
  MPI_CHECK (MPI_Graph_neighbors_count (comm, comm_rank, &neighbors_num));
  send_buffer = tst_type_allocvalues (env->type, env->values_num);
  tst_type_setstandardarray (env->type, env->values_num,
                             send_buffer, comm_rank);

  if ((status_buffer = (MPI_Status *)malloc (sizeof (MPI_Status) * neighbors_num)) == NULL)
    ERROR (errno, "malloc");

  if ((recv_buffer = (char **)malloc (sizeof (char *) * neighbors_num)) == NULL)
    ERROR (errno, "malloc");

  if((neighbors = (int *)malloc(sizeof (int) * neighbors_num))==NULL)
    ERROR(errno, "malloc");

  for (i=0; i < neighbors_num; i++)
    {
      recv_buffer[i] = tst_type_allocvalues (env->type, env->values_num);
    }

  return 0;
}

int tst_p2p_alltoall_graph_run (const struct tst_env * env)
{
  int neighbors_num;
  int comm_rank;
  int rank;
  int recv_count;
  MPI_Comm comm;
  MPI_Datatype type;

  comm = tst_comm_getcomm (env->comm);
  type = tst_type_getdatatype (env->type);
  MPI_CHECK (MPI_Comm_rank(comm, &comm_rank));
  MPI_CHECK (MPI_Graph_neighbors_count (comm, comm_rank, &neighbors_num));
  MPI_CHECK (MPI_Graph_neighbors(comm, comm_rank, neighbors_num, neighbors));
  DEBUG (printf ("(Rank:%d) comm_size:%d comm_rank:%d\n",
                 tst_global_rank, comm_size, comm_rank));

  /*
    * Communicate with neighbors with MPI_Sendrecv
    */
  for (rank = 0; rank < neighbors_num; rank++)
    {
      MPI_CHECK (MPI_Sendrecv(send_buffer, env->values_num, type, neighbors[rank], 4711,
                              recv_buffer[rank], env->values_num, type, neighbors[rank], 4711,
                              comm, &(status_buffer[rank])));
    }

  for (rank = 0; rank < neighbors_num; rank++)
    {
      if (status_buffer[rank].MPI_SOURCE != neighbors[rank]  ||
          status_buffer[rank].MPI_TAG != 4711)
        {
          DEBUG (printf ("(Rank:%d) rank:%d neighbors_num :%d should be:%d "
                         "MPI_SOURCE:%d MPI_TAG:%d\n",
                         comm_rank, rank, comm_size, comm_size - rank-1,
                         status_buffer[rank].MPI_SOURCE,
                         status_buffer[rank].MPI_TAG));
          ERROR (EINVAL, "Error in communication");
        }
      if (tst_mode == TST_MODE_STRICT)
        {
          MPI_CHECK(MPI_Get_count(&(status_buffer[rank]), type, &recv_count));
          if (recv_count != env->values_num)
              ERROR(EINVAL, "Error in Count");
        }
      tst_test_checkstandardarray (env, recv_buffer[rank], neighbors[rank]);
    }

  return 0;
}

int tst_p2p_alltoall_graph_cleanup (const struct tst_env * env)
{
  MPI_Comm comm;
  int i;
  int neighbors_num;
  int comm_rank;
  comm = tst_comm_getcomm (env->comm);
  tst_type_freevalues (env->type, send_buffer, env->values_num);
  MPI_CHECK (MPI_Comm_rank(comm, &comm_rank));
  MPI_CHECK (MPI_Graph_neighbors_count (comm, comm_rank, &neighbors_num));
  for (i = 0; i < neighbors_num; i++)
      tst_type_freevalues (env->type, recv_buffer[i], env->values_num);

  free (recv_buffer);
  free (status_buffer);
  free (neighbors);
  return 0;
}
