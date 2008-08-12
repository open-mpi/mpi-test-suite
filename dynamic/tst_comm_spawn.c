/*
 * File: tst_comm_spawn.c
 *
 * Functionality:
 *  test the dynamic process mangement Function
 *  MPI_Comm_spawn
 * Author: Rainer Keller and Sheng Feng
 *
 * Date: jan 27th 2007
 */
#include "config.h"

#include "mpi.h"
#include "mpi_test_suite.h"

#undef DEBUG
#define DEBUG(x)

#define BUFFER_NUM 1000


int tst_comm_spawn_init (struct tst_env * env)
{
#ifdef HAVE_MPI2_DYNAMIC
  tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "(Rank:%d) env->comm:%d env->type:%d env->values_num:%d\n",
                 tst_global_rank, env->comm, env->type, env->values_num);
  if((env->recv_buffer = (int *)malloc( BUFFER_NUM * sizeof(int) )) == NULL)
    ERROR(errno, "malloc");
#endif
  return 0;
}

int tst_comm_spawn_run (struct tst_env * env)
{
#ifdef HAVE_MPI2_DYNAMIC
  MPI_Comm comm;
  MPI_Comm client;
  MPI_Status status;
  MPI_Datatype type;
  int error,rank,i;
  char err[128];
  const int type_size = tst_type_gettypesize (env->type);

  comm = tst_comm_getcomm (env->comm);
  type = tst_type_getdatatype (env->type);

  MPI_CHECK (MPI_Comm_rank(MPI_COMM_WORLD, &rank));
  MPI_CHECK (MPI_Comm_spawn("./client1", MPI_ARGV_NULL, 1,
			    MPI_INFO_NULL,0, comm, &client, &error));
  MPI_CHECK (MPI_Recv(env->recv_buffer, BUFFER_NUM, MPI_INT, MPI_ANY_SOURCE, 99,
		      client, &status));
  for(i = 0 ; i< BUFFER_NUM; i++)
    {
      int cmp = i;
      if(env->recv_buffer[i] != i)
        {
	  snprintf (err, sizeof (err), "(Rank:%d) Error at i:%d expected ", tst_global_rank, i);
	  tst_type_hexdump (err, (char *)(&cmp), type_size);
	  snprintf (err, sizeof (err), "(Rank:%d) Error at i:%d but recv ", tst_global_rank, i);
	  tst_type_hexdump (err, (char *)(&(env->recv_buffer[i])), type_size);
	}
    }
  MPI_Comm_free(&client);
#endif
  return 0;
}


int tst_comm_spawn_cleanup (struct tst_env * env)
{
#ifdef HAVE_MPI2_DYNAMIC
  free(env->recv_buffer);
#endif
  return 0;
}
/* HAVE_MPI2_DYNAMIC */
