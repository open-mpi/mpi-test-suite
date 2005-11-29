/*
 * File: tst_coll_scatter_stride.c
 *
 * Functionality:
 *  Simple collective Scatterv test-program with a different stride - holes are being filled.
 *  Works with intra- communicators, MPI_COMM_SELF and up to now with any C (standard and struct) type.
 *
 * Communication: Assumption, root = 0;
 *   0 -> 0 no value but one hole
 *   0 -> 1 1 value transmitted then one hole
 *   0 -> 2 2 values transmitted then one hole
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

int tst_coll_scatterv_stride_init (const struct tst_env * env)
{
  int comm_size;
  int comm_rank;
  MPI_Comm comm;
  DEBUG (printf ("(Rank:%d) env->comm:%d env->type:%d env->values_num:%d\n",
                 tst_global_rank, env->comm, env->type, env->values_num));

  comm = tst_comm_getcomm (env->comm);
  MPI_CHECK (MPI_Comm_rank (comm, &comm_rank));
  MPI_CHECK (MPI_Comm_size (comm, &comm_size));

  /*
   * WITH the stride of one element, we have 1 element for proc 0, 2 elements for proc 1
   * See tst_coll_scatterv, where there is no stride of one additional Element.
   */
  send_buffer = tst_type_allocvalues (env->type, (comm_size * (comm_size + 1) /2) +1);
  recv_buffer = tst_type_allocvalues (env->type, comm_rank);

  send_counts = malloc (comm_size * sizeof (int));
  send_displs = malloc (comm_size * sizeof (int));

  return 0;
}

int tst_coll_scatterv_stride_run (const struct tst_env * env)
{
  const int type_size = tst_type_gettypesize (env->type);
  int comm_rank;
  int comm_size;
  int root;
  MPI_Comm comm;
  MPI_Datatype type;

  comm = tst_comm_getcomm (env->comm);
  type = tst_type_getdatatype (env->type);

  MPI_CHECK (MPI_Comm_rank (comm, &comm_rank));
  MPI_CHECK (MPI_Comm_size (comm, &comm_size));

  DEBUG (printf ("(Rank:%d) comm_size:%d comm_rank:%d\n",
                 tst_global_rank, comm_size, comm_rank));


  for (root=0; root < comm_size; root++)
    {
      if (comm_rank == root)
        {
          int j;
          int displs = 0;
          int pos = 0;
          /*
           * First write the first hole; it's not sent anyway.
           */
          send_counts [0] = 0;
          send_displs [0] = 0;
          tst_type_setvalue (env->type, send_buffer, TST_TYPE_SET_MIN, 0);

          for (j = 1; j < comm_size; j++)
            {
              tst_type_setstandardarray (env->type, j, &(send_buffer[pos]), j);
              send_counts[j] = j;
              /*
              * We have a dispacement of ONE element, BUT since we do not send to ourselve
              * we have to substract again.
              */
              send_displs[j] = displs;
              /*
              * Fill this element with something sensible, to check later again.
              */
              tst_type_setvalue (env->type, &(send_buffer[pos+j*type_size]), TST_TYPE_SET_MIN, 0);
              pos += (j+1)*type_size;
              displs += (j+1);
            }
        }
      DEBUG (printf ("(Rank:%d) Going to Scatter with root:%d\n",
                     tst_global_rank, root));

      MPI_CHECK (MPI_Scatterv (send_buffer, send_counts, send_displs, type,
                               recv_buffer, comm_rank, type,
                               root, comm));

      tst_test_checkstandardarray (env, recv_buffer, comm_rank);

      /*
       * Now, check the send_buffer for the values NOT send, whether they still are the same
       *
      if (root == comm_rank)
        {
          char * check_buffer;
          tst_type_allocvalues (env->type, 1);
          tst_type_setvalue (env->type, check_buffer, TST_TYPE_SET_MIN, 0);
          for (pos = j = 0; j < comm_size; j++, pos+=(j+1)*type_size)
           {
             tst_type_cmpvalue (env->type, 
           }
        }
       */
    }

  return 0;
}

int tst_coll_scatterv_stride_cleanup (const struct tst_env * env)
{
  tst_type_freevalues (env->type, send_buffer, env->values_num);
  tst_type_freevalues (env->type, recv_buffer, env->values_num);
  free (send_counts);
  free (send_displs);
  return 0;
}
