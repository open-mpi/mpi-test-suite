/*
 * File: tst_coll_bcast.c
 *
 * Functionality:
 *  Simple collective Bcast test-program.
 *  Works with intra- and inter- communicators, MPI_COMM_SELF and up to now with any C (standard and struct) type.
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

int tst_coll_bcast_init (const struct tst_env * env)
{
  DEBUG (printf ("(Rank:%d) env->comm:%d env->type:%d env->values_num:%d\n",
                 tst_global_rank, env->comm, env->type, env->values_num));

  buffer = tst_type_allocvalues (env->type, env->values_num);

  return 0;
}

int tst_coll_bcast_run (const struct tst_env * env)
{
  int comm_rank;
  int comm_size;
  int i;
  MPI_Comm comm;
  MPI_Datatype type;

  comm = tst_comm_getcomm (env->comm);
  type = tst_type_getdatatype (env->type);

  MPI_CHECK (MPI_Comm_rank (comm, &comm_rank));
  MPI_CHECK (MPI_Comm_size (comm, &comm_size));

  DEBUG (printf ("(Rank:%d) comm_size:%d comm_rank:%d\n",
                 tst_global_rank, comm_size, comm_rank));

  for (i=0; i < comm_size; i++)
    {
      int root;
      tst_type_setstandardarray (env->type, env->values_num, buffer, i);

      if (tst_comm_getcommclass (env->comm) == TST_MPI_INTRA_COMM)
        root = i;
#ifdef HAVE_MPI_EXTENDED_COLLECTIVES
      else if (tst_comm_getcommclass (env->comm) == TST_MPI_INTER_COMM)
        {
          /*
           * This is bogus -- all other processes on the remote group should
           * specify a correct group.
           */
          if (i == comm_rank)
            root = MPI_ROOT;
          else
            root = MPI_PROC_NULL;
        }
#else
      root = 0; /* Just to satisfy gcc -- this shouldn't get called with TST_MPI_INTER_COMM,
                   if we don't HAVE_MPI_EXTENDED_COLLECTIVES */
#endif /* HAVE_MPI_EXTENDED_COLLECTIVES */

      DEBUG (printf ("(Rank:%d) Going to Bcast with root:%d\n",
                     tst_global_rank, root));

      MPI_CHECK (MPI_Bcast (buffer, env->values_num, type, root, comm));
      tst_test_checkstandardarray (env, buffer, i);
    }

  return 0;
}

int tst_coll_bcast_cleanup (const struct tst_env * env)
{
  tst_type_freevalues (env->type, buffer, env->values_num);
  return 0;
}
