
#include <mpi.h>
#include "mpi_test_suite.h"


static char * send_buffer = NULL;
static char ** recv_buffer = NULL;
static MPI_Group *group_single = NULL;


int tst_put_with_post_alltoall_init (struct tst_env * env)
{
  MPI_Comm comm;
  int comm_rank;
  int comm_size;
  int i;
  int incl_num[1];
  MPI_Group world_group;
  
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
    
  send_buffer = tst_type_allocvalues (env->type, env->values_num);
  tst_type_setstandardarray (env->type, env->values_num,
			     send_buffer, comm_rank);
    
    
  if ((recv_buffer = malloc (sizeof (char *) * comm_size)) == NULL)
    ERROR (errno, "malloc");
    
  for (i=0; i < comm_size; i++)
    {
      recv_buffer[i] = tst_type_allocvalues (env->type, env->values_num);
    }
  tst_type_setstandardarray (env->type, env->values_num,
			     recv_buffer[comm_rank], comm_rank);
    
  if ((group_single = malloc (sizeof (MPI_Group) * comm_size)) == NULL)
    ERROR (errno, "malloc");
  
  MPI_Comm_group(comm, &world_group);
  
    
  for (i=0; i < comm_size; i++)
    {
      incl_num[0] = i;
      MPI_Group_incl(world_group, 1, incl_num, &(group_single[i]));
    }
  MPI_Group_free(&world_group);
  return 0;
}

int tst_put_with_post_alltoall_run (struct tst_env * env)
{
  int i,j;
  int comm_size;
  int type_size;
  int comm_rank;
  int rank;
  MPI_Comm comm;
  MPI_Datatype type;
  
  
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
  
  for(i=0; i<comm_size; i++)
    {
      MPI_Win window;
      MPI_CHECK (MPI_Win_create(recv_buffer[i], env->values_num*type_size,
				type_size, MPI_INFO_NULL, comm, &window));
      
      
      if(comm_rank ==i )
        {
	  for(j=0; j<comm_size; j++)
	    {
	      if(j == comm_rank)
		  continue;
	      MPI_Win_start(group_single[j], 0, window);
	      MPI_Put(send_buffer, env->values_num, type, j,0 ,
		      env->values_num,type,   window);
	      MPI_Win_complete(window);
	    }
	} else
        {
	  MPI_Win_post(group_single[i], 0, window);
	  MPI_Win_wait(window);
	}
      
      
      MPI_Win_free(&window);
    }
  
  for (rank = 0; rank < comm_size; rank++)
    {
      tst_test_checkstandardarray (env, recv_buffer[rank],  rank );
    }
  return 0;
}

int tst_put_with_post_alltoall_cleanup (struct tst_env * env)
{
  MPI_Comm comm;
  int i;
  int comm_size;
  comm = tst_comm_getcomm (env->comm);
    
  tst_type_freevalues (env->type, send_buffer, env->values_num);
  
  if (tst_comm_getcommclass (env->comm) == TST_MPI_INTER_COMM)
    MPI_CHECK (MPI_Comm_remote_size (comm, &comm_size));
  else
    MPI_CHECK (MPI_Comm_size (comm, &comm_size));
  
  for (i = 0; i < comm_size; i++)
    {
      tst_type_freevalues (env->type, recv_buffer[i], env->values_num);
      MPI_Group_free(&(group_single[i]));
    }
  
  free (recv_buffer);
  free(group_single);
  return 0;
}

