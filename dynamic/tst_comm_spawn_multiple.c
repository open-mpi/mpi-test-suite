/*
 * File: tst_comm_spawn_multiple.c
 *
 * Functionality:
 *  test the dynamic process mangement Function
 *  MPI_Comm_spawn_multiple
 * Author: Rainer Keller and Sheng Feng
 *
 * Date: jan 27th 2007
 */
#include "config.h"

#include <mpi.h>
#include "mpi_test_suite.h"

#undef DEBUG
#define DEBUG(x)

#define BUFFER_NUM 1000

int tst_comm_spawn_multiple_init (struct tst_env * env)
{
#ifdef HAVE_MPI2_DYNAMIC
  tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, 
      		"(Rank:%d) env->comm:%d env->type:%d env->values_num:%d\n",
                tst_global_rank, env->comm, env->type, env->values_num);
  if((env->recv_buffer_array =  malloc ( 2 * BUFFER_NUM * sizeof(int) )) == NULL)
    ERROR(errno, "malloc");
#endif
  return 0;
}

int tst_comm_spawn_multiple_run (struct tst_env * env)
{
#ifdef HAVE_MPI2_DYNAMIC
  MPI_Comm comm;
  MPI_Comm client;
  MPI_Status status;
  MPI_Datatype type;
  int rank,i;
  char err[128];
  const int type_size = tst_type_gettypesize (env->type);
  char *commands[] = {"./client1", "./client2" };
  MPI_Info infos[2] = { MPI_INFO_NULL, MPI_INFO_NULL} ;

  int procs[2] = { 1, 1 };
  int error_code[2];


  comm = tst_comm_getcomm (env->comm);
  type = tst_type_getdatatype (env->type);

  MPI_CHECK (MPI_Comm_rank(MPI_COMM_WORLD, &rank));
  MPI_CHECK (MPI_Comm_spawn_multiple(2, commands, MPI_ARGVS_NULL, procs,
			    infos,0, comm, &client, error_code));
  MPI_CHECK (MPI_Recv(&(env->recv_buffer[0]), BUFFER_NUM, MPI_INT, 0, 99,
		      client, &status));
  MPI_CHECK (MPI_Recv(&(recv_buffer_array[BUFFER_NUM]), BUFFER_NUM, MPI_INT, 1, 299,
                      client, &status));
  for(i = 0 ; i< BUFFER_NUM; i++)
    {
      int cmp = i;
      if(env->recv_buffer_array[i] != i)
        {
	  snprintf (err, sizeof (err), "(Rank:%d) Error at i:%d expected ", tst_global_rank, i);
	  tst_type_hexdump (err, (char *)(&cmp), type_size);
	  snprintf (err, sizeof (err), "(Rank:%d) Error at i:%d but recv ", tst_global_rank, i);
	  tst_type_hexdump (err, (char *)(&(env->recv_buffer_array[i])), type_size);
	}
      if(env->recv_buffer_array[BUFFER_NUM + i] != BUFFER_NUM -1-i)
        {
          snprintf (err, sizeof (err), "(Rank:%d) Error at i:%d expected ", tst_global_rank, i);
          tst_type_hexdump (err, (char *)(&cmp), type_size);
          snprintf (err, sizeof (err), "(Rank:%d) Error at i:%d but recv ", tst_global_rank, i);
          tst_type_hexdump (err, (char *)(&(env->recv_buffer_array[BUFFER_NUM + i])), type_size);
        }
    }
  MPI_Comm_free(&client);
#endif
  return 0;
}


int tst_comm_spawn_multiple_cleanup (struct tst_env * env)
{
#ifdef HAVE_MPI2_DYNAMIC
  free(env->recv_buffer_array);
#endif
  return 0;
}
/* HAVE_MPI2_DYNAMIC */
