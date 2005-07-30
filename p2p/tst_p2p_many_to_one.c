/*
 * File: tst_p2p_many_to_one.c
 *
 * Functionality:
 *  Simple point-to-point many-to-one test, with everyone in the comm sending blocking to 
 *  process zero, which receives with MPI_ANY_SOURCE.
 *  Works with intra- and inter- communicators and up to now with any C (standard and struct) type.
 *
 * Author: Rainer Keller
 *
 * Date: Aug 8th 2003
 */
#include "mpi.h"
#include "mpi_test_suite.h"

#undef DEBUG
#define DEBUG(x)

static char * buffer = NULL;

int tst_p2p_many_to_one_init (const struct tst_env * env)
{
  int comm_rank;
  MPI_Comm comm;

  DEBUG (printf ("(Rank:%d) env->comm:%d env->type:%d env->values_num:%d\n",
                 tst_global_rank, env->comm, env->type, env->values_num));

  buffer = tst_type_allocvalues (env->type, env->values_num);

  /*
   * Now, initialize the buffer
   */
  comm = tst_comm_getcomm (env->comm);
  MPI_CHECK (MPI_Comm_rank (comm, &comm_rank));

  tst_type_setstandardarray (env->type, env->values_num, buffer, comm_rank);
  return 0;
}

int tst_p2p_many_to_one_run (const struct tst_env * env)
{
  int comm_size;
  int comm_rank;
  int hash_value;
  MPI_Comm comm;
  MPI_Datatype type;
  MPI_Status status;

  comm = tst_comm_getcomm (env->comm);
  type = tst_type_getdatatype (env->type);

  if (tst_comm_getcommclass (env->comm) == TST_MPI_INTRA_COMM)
    MPI_CHECK (MPI_Comm_size (comm, &comm_size));
  else
    MPI_CHECK (MPI_Comm_remote_size (comm, &comm_size));
  
  MPI_CHECK (MPI_Comm_rank (comm, &comm_rank));

  hash_value = tst_hash_value (env);
  DEBUG (printf ("(Rank:%d) comm:%d type:%d test:%d hash_value:%d comm_size:%d comm_rank:%d\n",
                 tst_global_rank,
		 env->comm, env->type, env->test, hash_value,
		 comm_size, comm_rank));

  /*
  ** Even for intercommunicator, only process zero within
  ** (any intra/inter-com<municator) MPI_COMM_WORLD will receive data!
  */
  
  if (comm_rank == 0)
    {
      int rank;

      for (rank = 0; rank < comm_size-1; rank++)
        {
          /*
	   * The source-argument MPI_ANY_SOURCE requires us to synchronize.
	   * Otherwise, if there isn't a outside synchronisation point (barrier)
	   * the MPI_Recv might match a send from another TEST-program!
	   */
          MPI_CHECK (MPI_Recv (buffer, env->values_num, type, MPI_ANY_SOURCE, hash_value, comm, &status));
          if (status.MPI_TAG != hash_value ||
	      status.MPI_SOURCE <= 0 ||
	      status.MPI_SOURCE > comm_size-1)
            ERROR (EINVAL, "Error in status");
          DEBUG (printf ("(Rank:%d) going to check array from source:%d\n",
			 comm_rank, status.MPI_SOURCE));
          tst_type_checkstandardarray (env->type, env->values_num, buffer, status.MPI_SOURCE);
        }
    }
  else
    MPI_CHECK (MPI_Send (buffer, env->values_num, type, 0, hash_value, comm));

  return 0;
}

int tst_p2p_many_to_one_cleanup (const struct tst_env * env)
{
  free (buffer);
  return 0;
}
