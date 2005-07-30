/*
 * File: tst_coll_scatter.c
 *
 * Functionality:
 *  Simple collective Scatterv test-program.
 *  Works with intra- communicators, MPI_COMM_SELF and up to now with any C (standard and struct) type.
 *
 * Communication: Assumption, root = 0;
 *   0 -> 0 no value
 *   0 -> 1 1 value transmitted
 *   0 -> 2 2 values transmitted
 *   ...
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
static int * send_counts = NULL;
static int * send_displs = NULL;

int tst_coll_scatterv_init (const struct tst_env * env)
{
  int comm_size;
  int comm_rank;
  MPI_Comm comm;
  DEBUG (printf ("(Rank:%d) env->comm:%d env->type:%d env->values_num:%d\n",
                 tst_global_rank, env->comm, env->type, env->values_num));

  comm = tst_comm_getcomm (env->comm);
  MPI_CHECK (MPI_Comm_size (comm, &comm_size));
  MPI_CHECK (MPI_Comm_rank (comm, &comm_rank));
 /*
  * With the above communication pattern, we need in total (comm_size-1)*comm_size/2 elements.
  * See tst_coll_scatterv_stride.c testcase.
  */
  send_buffer = tst_type_allocvalues (env->type, ((comm_size-1)*comm_size / 2)+1);
  recv_buffer = tst_type_allocvalues (env->type, comm_rank+1);

  send_counts = malloc (comm_size * sizeof (int));
  send_displs = malloc (comm_size * sizeof (int));

  return 0;
}

int tst_coll_scatterv_run (const struct tst_env * env)
{
  const int type_size = tst_type_gettypesize (env->type);
  int comm_rank;
  int comm_size;
  int root;
  MPI_Comm comm;
  MPI_Datatype type;
  MPI_Aint stride;

  comm = tst_comm_getcomm (env->comm);
  type = tst_type_getdatatype (env->type);

  MPI_CHECK (MPI_Comm_rank (comm, &comm_rank));
  MPI_CHECK (MPI_Comm_size (comm, &comm_size));

  MPI_CHECK (MPI_Type_extent (type, &stride));

  DEBUG (printf ("(Rank:%d) comm_size:%d comm_rank:%d extent:%d type_size:%d\n",
                 tst_global_rank, comm_size, comm_rank, stride, type_size));


  for (root=0; root < comm_size; root++)
    {
      if (comm_rank == root)
        {
          int j;
          int displs = 0;
          int pos = 0;

          send_counts[0] = 0;
          send_displs[0] = 0;

          for (j = 1; j < comm_size; j++)
            {
              tst_type_setstandardarray (env->type, j, &(send_buffer[pos]), j);
              send_counts[j] = j;
              send_displs[j] = displs;
              pos += j*type_size;
              displs += j;
            }
        }

      DEBUG (printf ("(Rank:%d) Going to Scatterv with root:%d\n",
                     tst_global_rank, root));

      MPI_Scatterv (send_buffer, send_counts, send_displs, type,
                               recv_buffer, comm_rank, type,
                               root, comm);

      tst_type_checkstandardarray (env->type, comm_rank, recv_buffer, comm_rank);
    }

  return 0;
}

int tst_coll_scatterv_cleanup (const struct tst_env * env)
{
  free (send_buffer);
  free (recv_buffer);
  free (send_counts);
  free (send_displs);
  return 0;
}
