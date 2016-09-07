/*
 * File: tst_threaded_comm_dup.c
 *
 * Functionality: 
 *      Generates as many copies of the communicator as threads exist
 *      using MPI_Comm_dup. Then each thread executes MPI_Bcast on its
 *      copy of the communicator.
 *
 * Author: Christoph Niethammer
 *
 * Date: Feb 26th 2007
 */
#include "mpi.h"
#include "mpi_test_suite.h"
#include "tst_output.h"

#ifdef HAVE_PTHREAD_H
#  include <pthread.h>
#endif

#undef DEBUG
#define DEBUG(x)

static MPI_Comm * new_comms;

int tst_threaded_comm_dup_init (struct tst_env * env)
{
  int comm_rank;
  int comm_size;
  MPI_Comm comm;
  int num_threads;
  int thread_num;
  int i;

  comm = tst_comm_getcomm (env->comm);
  MPI_CHECK (MPI_Comm_rank (comm, &comm_rank));
  MPI_CHECK (MPI_Comm_size (comm, &comm_size));

  num_threads = tst_thread_num_threads();
  thread_num = tst_thread_get_num();

  tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "(Rank:%d) env->comm:%d env->type:%d env->values_num:%d\n",
                 tst_global_rank, env->comm, env->type, env->values_num);

  env->send_buffer = tst_type_allocvalues (env->type, env->values_num);
  /* 
   * initialise copies of communicators 
   */
  if (thread_num == 0) {
    if (NULL == (new_comms = malloc (num_threads * sizeof (MPI_Comm)))) 
      ERROR (errno, "malloc");
    for (i = 0; i < num_threads; i++) 
      MPI_CHECK (MPI_Comm_dup (comm, &new_comms[i]));
    tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "(Rank:%d) initialised copies of communicators\n",
	tst_global_rank);
  }

  return 0;
}

int tst_threaded_comm_dup_run (struct tst_env * env)
{
  int comm_size;
  int comm_rank;
  MPI_Comm comm;
  MPI_Datatype type;

  int thread_num;

  int i;

  comm = tst_comm_getcomm (env->comm);
  type = tst_type_getdatatype (env->type);
  MPI_CHECK (MPI_Comm_size (comm, &comm_size));
  MPI_CHECK (MPI_Comm_rank (comm, &comm_rank));

  thread_num = tst_thread_get_num();


  for (i = 0; i < comm_size; i++)
  {
    int root;
    int tag;

    /* thread specific tag */
    tag = 100 * comm_rank + thread_num;
    if (tst_comm_getcommclass (env->comm) == TST_MPI_INTRA_COMM)
      root = i;
#if 1
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
#endif

    tst_type_setstandardarray (env->type, env->values_num, env->send_buffer, tag);
    tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "(Rank:%d) Going to Bcast with root:%d\n",
                              tst_global_rank, root);
    //printf ("Before Bcast (Rank %d, Thread %d\n", tst_global_rank, thread_num);
    MPI_CHECK (MPI_Bcast (env->send_buffer, env->values_num, type, root, new_comms[thread_num]));
    //printf ("After Bcast (Rank %d, Thread %d\n", tst_global_rank, thread_num);
    tst_test_checkstandardarray (env, env->send_buffer, tag);
  }

  return 0;
}

int tst_threaded_comm_dup_cleanup (struct tst_env * env)
{
  int thread_num;

  thread_num = tst_thread_get_num ();
  tst_type_freevalues (env->type, env->send_buffer, env->values_num);
  if (thread_num == 0)
    free (new_comms);

  return 0;
}
