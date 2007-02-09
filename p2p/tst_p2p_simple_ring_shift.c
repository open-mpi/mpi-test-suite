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
#include "tst_output.h"

#undef DEBUG
#define DEBUG(x)

/*
 * XXX
static char * send_buffer = NULL;
static char * recv_buffer = NULL;
static int * send_to = NULL;
static int * recv_from = NULL;
 */

int tst_p2p_simple_ring_shift_init (struct tst_env * env)
{
  int comm_rank;
  int i;
  int dim_num;
  MPI_Comm comm;

  comm = tst_comm_getcomm (env->comm);
  MPI_CHECK (MPI_Comm_rank (comm, &comm_rank));
  tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "(Rank:%d) env->comm:%d env->type:%d env->values_num:%d\n",
                 tst_global_rank, env->comm, env->type, env->values_num);

  env->send_buffer = tst_type_allocvalues (env->type, env->values_num);
  tst_type_setstandardarray (env->type, env->values_num,
                             env->send_buffer, comm_rank);

  env->recv_buffer = tst_type_allocvalues (env->type, env->values_num);

  /*
   * Now, initialize the border and whatnot
   */
  MPI_CHECK(MPI_Cartdim_get(comm, &dim_num));
  if ((env->send_to = (int *)malloc(sizeof(int) * dim_num)) == NULL)
    ERROR (errno, "malloc");
  if ((env->recv_from = (int *)malloc(sizeof(int) * dim_num)) == NULL)
    ERROR (errno, "malloc");

  for(i=0; i < dim_num; i++)
    MPI_CHECK(MPI_Cart_shift(comm, i, 1, &(env->send_to[i]), &(env->recv_from[i])));

  return 0;
}

int tst_p2p_simple_ring_shift_run (struct tst_env * env)
{
  int i;
  MPI_Comm comm;
  MPI_Datatype type;
  MPI_Status status;
  int recv_menge;
  int dim_num;

  comm = tst_comm_getcomm (env->comm);
  type = tst_type_getdatatype (env->type);

  MPI_CHECK(MPI_Cartdim_get(comm, &dim_num));

  for(i=0; i < dim_num; i++)
    {
      MPI_CHECK (MPI_Sendrecv (env->send_buffer, env->values_num, type, env->send_to[i], env->tag,
                               env->recv_buffer, env->values_num, type, env->recv_from[i], env->tag, comm, &status));

      if(status.MPI_SOURCE != env->recv_from[i] ||
         (env->recv_from[i] != MPI_PROC_NULL && status.MPI_TAG != env->tag) ||
         (env->recv_from[i] == MPI_PROC_NULL && status.MPI_TAG != MPI_ANY_TAG))
        {
          ERROR(EINVAL, "Error in status");
        }

      if (tst_mode == TST_MODE_STRICT)
        {
          MPI_Get_count(&status, type, &recv_menge);
          if(recv_menge != env->values_num && env->recv_from[i] != MPI_PROC_NULL )
              ERROR(EINVAL, "Error in Menge");
        }

      if(env->recv_from[i] != MPI_PROC_NULL)
          tst_test_checkstandardarray (env, env->recv_buffer, env->recv_from[i]);
    }

  return 0;
}

int tst_p2p_simple_ring_shift_cleanup (struct tst_env * env)
{
  tst_type_freevalues (env->type, env->send_buffer, env->values_num);
  tst_type_freevalues (env->type, env->recv_buffer, env->values_num);
  free(env->send_to);
  free(env->recv_from);
  return 0;
}
