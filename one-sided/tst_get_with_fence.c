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

#include <mpi.h>
#include "mpi_test_suite.h"



int tst_get_with_fence_alltoall_init (struct tst_env * env)
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

  if (tst_comm_getcommclass (env->comm) == TST_MPI_INTER_COMM)
    MPI_CHECK (MPI_Comm_remote_size (comm, &comm_size));
  else
    MPI_CHECK (MPI_Comm_size (comm, &comm_size));

  env->send_buffer = tst_type_allocvalues (env->type, env->values_num);
  tst_type_setstandardarray (env->type, env->values_num,
                             env->send_buffer, comm_rank);


  if ((env->recv_buffer_array = malloc (sizeof(char *) * comm_size)) == NULL)
    ERROR (errno, "malloc");

  for (i=0; i < comm_size; i++)
    {
      env->recv_buffer_array[i] = tst_type_allocvalues (env->type, env->values_num);
    }

  return 0;
}

int tst_get_with_fence_alltoall_run (struct tst_env * env)
{
  int i;
  int comm_size;
  int comm_rank;
  int type_size;
  int rank;
  MPI_Comm comm;
  MPI_Datatype type;
  MPI_Win window;

  comm = tst_comm_getcomm (env->comm);
  type = tst_type_getdatatype (env->type);
  type_size = tst_type_gettypesize (env->type);

  if (tst_comm_getcommclass (env->comm) == TST_MPI_INTER_COMM)
    MPI_CHECK (MPI_Comm_remote_size (comm, &comm_size));
  else
    MPI_CHECK (MPI_Comm_size (comm, &comm_size));

  MPI_CHECK (MPI_Comm_rank (comm, &comm_rank));

  tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "(Rank:%d) comm_size:%d comm_rank:%d\n",
                 tst_global_rank, comm_size, comm_rank);

  MPI_CHECK (MPI_Win_create(env->send_buffer, env->values_num*type_size, type_size,
			    MPI_INFO_NULL, comm, &window));  

  MPI_CHECK(MPI_Win_fence(0, window));
  for(i=0; i<comm_size; i++)
    {
  tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "(Rank:%d) comm_size:%d comm_rank:%d iter:%d\n",
                 tst_global_rank, comm_size, comm_rank, i);
	MPI_Get(env->recv_buffer_array[i], env->values_num, type, i,0,env->values_num,type,
		window);
    }
  MPI_CHECK(MPI_Win_fence(0, window));

  MPI_Win_free(&window);

  for (rank = 0; rank < comm_size; rank++)
    {
     
      tst_test_checkstandardarray (env, env->recv_buffer_array[rank],  rank );
    }
  return 0;
}

int tst_get_with_fence_alltoall_cleanup (struct tst_env * env)
{
  MPI_Comm comm;
  int i;
  int comm_size;
  comm = tst_comm_getcomm (env->comm);

  tst_type_freevalues (env->type, env->send_buffer, env->values_num);

  if (tst_comm_getcommclass (env->comm) == TST_MPI_INTER_COMM)
    MPI_CHECK (MPI_Comm_remote_size (comm, &comm_size));
  else
    MPI_CHECK (MPI_Comm_size (comm, &comm_size));

  for (i = 0; i < comm_size; i++)
    tst_type_freevalues (env->type, env->recv_buffer_array[i], env->values_num);

  free (env->recv_buffer);


  return 0;
}

