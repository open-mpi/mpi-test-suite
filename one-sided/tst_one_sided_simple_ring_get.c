/*
 * File: tst_one_sided_simple.c
 *
 * Functionality:
 *  Simple one-sided Test-program.
 *
 * Author: Rainer Keller
 *
 */
#include <mpi.h>
#include "mpi_test_suite.h"
#include "tst_output.h"


#ifdef HAVE_MPI2_ONE_SIDED

/* XXX CN to be removed 
static char * send_buffer = NULL;
static char * recv_buffer = NULL;
*/

/* XXX the following could maybe put into the env */
static MPI_Aint send_buffer_size = 0;
static MPI_Win send_win = MPI_WIN_NULL;

int tst_one_sided_simple_ring_get_init (struct tst_env * env)
{
  int comm_rank;
  MPI_Comm comm;
  int type_size;

  tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "(Rank:%d) env->comm:%d env->type:%d env->values_num:%d\n",
                 tst_global_rank, env->comm, env->type, env->values_num);

  comm = tst_comm_getcomm (env->comm);
  MPI_CHECK (MPI_Comm_rank (comm, &comm_rank));

  env->send_buffer = tst_type_allocvalues (env->type, env->values_num);
  tst_type_setstandardarray (env->type, env->values_num, env->send_buffer, comm_rank);
  tst_type_getstandardarray_size (env->type, env->values_num, &send_buffer_size);

  env->recv_buffer = tst_type_allocvalues (env->type, env->values_num);
  type_size = tst_type_gettypesize (env->type);

  /*
   * Create a window for the send and the receive buffer
   */
  tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "(Rank:%d) Going to create window\n",
                 tst_global_rank);
  MPI_Win_create (env->send_buffer, send_buffer_size, type_size,
                  MPI_INFO_NULL, comm, &send_win);

  return 0;
}

int tst_one_sided_simple_ring_get_run (struct tst_env * env)
{
  int comm_rank;
  int comm_size;
  MPI_Comm comm;
  MPI_Datatype type;
  int get_from;

  comm = tst_comm_getcomm (env->comm);
  type = tst_type_getdatatype (env->type);

  MPI_CHECK (MPI_Comm_rank (comm, &comm_rank));
  MPI_CHECK (MPI_Comm_size (comm, &comm_size));

  tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "(Rank:%d) comm_size:%d comm_rank:%d\n",
                 tst_global_rank, comm_size, comm_rank);

  get_from = (comm_rank + comm_size - 1) % comm_size;
  tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "(Rank:%d) Going to MPI_from from get_from:%d\n",
                 tst_global_rank, get_from);

  /*
   * All processes call MPI_Put
   */
  MPI_Win_fence (MPI_MODE_NOPRECEDE, send_win);
  MPI_Get (env->recv_buffer, env->values_num, type,
           get_from, 0, env->values_num, type, send_win);

  MPI_Win_fence (MPI_MODE_NOSTORE | MPI_MODE_NOSUCCEED, send_win);

  tst_test_checkstandardarray (env, env->recv_buffer, (comm_rank + comm_size - 1) % comm_size);

  return 0;
}

int tst_one_sided_simple_ring_get_cleanup (struct tst_env * env)
{
  int comm_rank;
  MPI_Comm comm;

  comm = tst_comm_getcomm (env->comm);
  MPI_CHECK (MPI_Comm_rank (comm, &comm_rank));

  //if (tst_global_rank == 0 && tst_thread_get_num() == 0)
  	MPI_Win_free (&send_win);

  tst_type_freevalues (env->type, env->send_buffer, env->values_num);
  tst_type_freevalues (env->type, env->recv_buffer, env->values_num);

  return 0;
}

#endif
