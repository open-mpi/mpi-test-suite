#include "config.h"
#ifdef HAVE_MPI2_ONE_SIDED

#include <mpi.h>
#include "mpi_test_suite.h"

#undef DEBUG
#define DEBUG(x)

static char * send_buffer = NULL;
static char * recv_buffer = NULL;
static MPI_Group *group_guest = NULL;
static MPI_Group host;

int tst_accumulate_with_post_min_init (struct tst_env * env)
{
  MPI_Comm comm;
  int comm_rank;
  int comm_size;
  int excl_num[1];
  int incl_num[1];
  int i;
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

  recv_buffer = tst_type_allocvalues (env->type, env->values_num);
  tst_type_setstandardarray (env->type, env->values_num,
                             recv_buffer, comm_rank);

  if ((group_guest = malloc (sizeof (MPI_Group) * comm_size)) == NULL)
      ERROR (errno, "malloc");

  MPI_Comm_group(comm, &world_group);
  excl_num[0] = comm_rank;
  MPI_Group_excl(world_group, 1, excl_num, &host);

  for (i=0; i < comm_size; i++)
    {
      incl_num[0] = i;
      MPI_Group_incl(world_group, 1, incl_num, &(group_guest[i]));
    }


  return 0;
}

int tst_accumulate_with_post_min_run (struct tst_env * env)
{
  int i;
  int comm_size;
  int type_size;
  int comm_rank;
  MPI_Comm comm;
  MPI_Datatype type;
  MPI_Win window;

  comm = tst_comm_getcomm (env->comm);
  type = tst_type_getdatatype (env->type);
  type_size = tst_type_gettypesize (env->type);

  if (tst_comm_getcommclass (env->comm) == TST_MPI_INTRA_COMM)
    MPI_CHECK (MPI_Comm_size (comm, &comm_size));
  else
    MPI_CHECK (MPI_Comm_remote_size (comm, &comm_size));

  MPI_CHECK (MPI_Comm_rank (comm, &comm_rank));

  tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "(Rank:%d) comm_size:%d comm_rank:%d\n",
                 tst_global_rank, comm_size, comm_rank);

  MPI_CHECK (MPI_Win_create(recv_buffer, env->values_num*type_size, type_size,
                            MPI_INFO_NULL, comm, &window));

  for(i=0; i<comm_size; i++)
    {
      if(comm_rank ==i )
        {
          MPI_Win_post(host, 0, window);
          MPI_Win_wait(window);
        } else
        {
          MPI_Win_start(group_guest[i], 0, window);
          MPI_Accumulate(send_buffer, env->values_num, type,
                         i,0 ,env->values_num,type,
                         MPI_MIN, window);
          MPI_Win_complete(window);
        }
    }

  MPI_Win_free(&window);

  tst_test_checkstandardarray (env, recv_buffer,  0 );

  return 0;
}

int tst_accumulate_with_post_min_cleanup (struct tst_env * env)
{
  MPI_Comm comm;
  int i;
  int comm_size;
  comm = tst_comm_getcomm (env->comm);

  if (tst_comm_getcommclass (env->comm) == TST_MPI_INTER_COMM)
    MPI_CHECK (MPI_Comm_remote_size (comm, &comm_size));
  else
    MPI_CHECK (MPI_Comm_size (comm, &comm_size));

  for (i = 0; i < comm_size; i++)
    {
      MPI_Group_free(&(group_guest[i]));
    }
  MPI_Group_free(&host);
  free(group_guest);

  tst_type_freevalues (env->type, send_buffer, env->values_num);
  tst_type_freevalues (env->type, recv_buffer, env->values_num);

  return 0;
}

#endif /* HAVE_MPI2_ONE_SIDED */
