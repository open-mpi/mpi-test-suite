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

static char * send_buffer = NULL;
static char * recv_buffer = NULL;
static int dim_num;
static int * send_to = NULL;
static int * recv_from = NULL;

int tst_p2p_simple_ring_shift_init (const struct tst_env * env)
{
  int comm_rank;
  int i;
  MPI_Comm comm;

  comm = tst_comm_getcomm (env->comm);
  MPI_CHECK (MPI_Comm_rank (comm, &comm_rank));
  DEBUG (printf ("(Rank:%d) env->comm:%d env->type:%d env->values_num:%d\n",
                 tst_global_rank, env->comm, env->type, env->values_num));

  send_buffer = tst_type_allocvalues (env->type, env->values_num);
  tst_type_setstandardarray (env->type, env->values_num,
                             send_buffer, comm_rank);

  recv_buffer = tst_type_allocvalues (env->type, env->values_num);

  /*
   * Now, initialize the border and whatnot
   */
  MPI_CHECK(MPI_Cartdim_get(comm, &dim_num));
  if ((send_to = (int *)malloc(sizeof(int) * dim_num)) == NULL)
    ERROR (errno, "malloc");
  if ((recv_from = (int *)malloc(sizeof(int) * dim_num)) == NULL)
    ERROR (errno, "malloc");

  for(i=0; i<dim_num; i++)
    MPI_CHECK(MPI_Cart_shift(comm, i, 1, &(send_to[i]), &(recv_from[i])));

  return 0;
}

int tst_p2p_simple_ring_shift_run (const struct tst_env * env)
{
  int i;
  MPI_Comm comm;
  MPI_Datatype type;
  MPI_Status status;
  int recv_menge;

  comm = tst_comm_getcomm (env->comm);
  type = tst_type_getdatatype (env->type);

  for(i=0; i < dim_num; i++)
    {
      MPI_CHECK (MPI_Sendrecv (send_buffer, env->values_num, type, send_to[i], 4711,
                               recv_buffer, env->values_num, type, recv_from[i], 4711, comm, &status
));

  if(status.MPI_SOURCE != recv_from[i] ||
      (recv_from[i] != MPI_PROC_NULL && status.MPI_TAG != 4711) ||
     (recv_from[i] == MPI_PROC_NULL && status.MPI_TAG != MPI_ANY_TAG))
      ERROR(EINVAL, "Error in status");
  if (tst_mode == TST_MODE_STRICT)
    {
      MPI_Get_count(&status, type, &recv_menge);
      if(recv_menge != env->values_num && recv_from[i] != MPI_PROC_NULL )
          ERROR(EINVAL, "Error in Menge");
    }

      if(recv_from[i] != MPI_PROC_NULL)
          tst_test_checkstandardarray (env, recv_buffer, recv_from[i]);
    }

  return 0;
}

int tst_p2p_simple_ring_shift_cleanup (const struct tst_env * env)
{
  tst_type_freevalues (env->type, send_buffer, env->values_num);
  tst_type_freevalues (env->type, recv_buffer, env->values_num);
  free(send_to);
  free(recv_from);
  return 0;
}
