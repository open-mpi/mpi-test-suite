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
#include "tst_output.h"

#undef DEBUG
#define DEBUG(x)


int tst_coll_bcast_init (struct tst_env * env)
{
  tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "(Rank:%d) env->comm:%d env->type:%d env->values_num:%d\n",
                 tst_global_rank, env->comm, env->type, env->values_num);

  env->send_buffer = tst_type_allocvalues (env->type, env->values_num);

  return 0;
}

int tst_coll_bcast_run (struct tst_env * env)
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

  tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "(Rank:%d) comm_size:%d comm_rank:%d\n",
                 tst_global_rank, comm_size, comm_rank);

  for (i=0; i < comm_size; i++)
    {
      int root;
      tst_type_setstandardarray (env->type, env->values_num, env->send_buffer, i);

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

      tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "(Rank:%d) Going to Bcast with root:%d\n",
                     tst_global_rank, root);

      MPI_CHECK (MPI_Bcast (env->send_buffer, env->values_num, type, root, comm));
      tst_test_checkstandardarray (env, env->send_buffer, i);
    }

  return 0;
}

int tst_coll_bcast_cleanup (struct tst_env * env)
{
  tst_type_freevalues (env->type, env->send_buffer, env->values_num);
  return 0;
}
