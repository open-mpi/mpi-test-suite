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

#include "config.h"
#ifdef HAVE_MPI2_ONE_SIDED

#include "mpi.h"
#include "mpi_test_suite.h"

#undef DEBUG
#define DEBUG(x)

static char * send_buffer = NULL;
static char * recv_buffer = NULL;

int tst_accumulate_with_fence_sum_init (struct tst_env * env)
{
  MPI_Comm comm;
  int comm_rank;
  int comm_size;

  DEBUG (printf ("(Rank:%d) env->comm:%d env->type:%d env->values_num:%d\n",
                 tst_global_rank, env->comm, env->type, env->values_num));

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

  return 0;
}

int tst_accumulate_with_fence_sum_run (struct tst_env * env)
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

  DEBUG (printf ("(Rank:%d) comm_size:%d comm_rank:%d\n",
                 tst_global_rank, comm_size, comm_rank));



  MPI_CHECK (MPI_Win_create(recv_buffer, env->values_num*type_size, type_size,
                            MPI_INFO_NULL, comm, &window));
  MPI_CHECK(MPI_Win_fence(0, window));
  for(i=0; i<comm_size; i++)
    {
      if (comm_rank == i)
        continue;
      MPI_Accumulate(send_buffer, env->values_num, type, i,0 ,env->values_num,type,
                     MPI_SUM, window);
    }
  MPI_CHECK(MPI_Win_fence(0, window));
  MPI_Win_free(&window);


  {
    int comm_size;
    MPI_Aint sum;
    const int type_size = tst_type_gettypesize (env->type);
    int errors = 0;
    char err[128];
    char * cmp_value;
    int i;


    MPI_Comm_size(comm, &comm_size);
    sum=(comm_size + 3) * comm_size /2;
    cmp_value = tst_type_allocvalues (env->type, 1);

    for (i = 2; i < env->values_num; i++)
      {
        tst_type_setvalue (env->type, cmp_value, TST_TYPE_SET_VALUE, sum + (i-2) * comm_size);

        if (tst_type_cmpvalue (env->type, cmp_value, &(recv_buffer[i*type_size])))
          {
            /* struct tst_mpi_float_int * tmp = (struct tst_mpi_float_int*) &(buffer[i*type_size]); */
            /* ((struct tst_mpi_float_int*)cmp_value)->a, */
            if (tst_report >= TST_REPORT_FULL)
                {
                snprintf (err, sizeof (err), "(Rank:%d) Error at i:%d expected ", tst_global_rank, i);
                tst_type_hexdump (err, cmp_value, type_size);

                snprintf (err, sizeof (err), "(Rank:%d) Error at i:%d but received ", tst_global_rank, i);
                tst_type_hexdump (err, &(recv_buffer[i*type_size]), type_size);
                }
            errors++;
          }
/*
      else
        printf ("(Rank:%d) Correct at i:%d cmp_value:%d buffer[%d]:%d\n",
                tst_global_rank, i,
                (char)*cmp_value,
                i*type_size,
                (char)buffer[i*type_size]);
*/
      }

    tst_type_freevalues (env->type, cmp_value, 1);
  }

  return 0;
}

int tst_accumulate_with_fence_sum_cleanup (struct tst_env * env)
{
  MPI_Comm comm;

  tst_type_freevalues (env->type, send_buffer, env->values_num);
  tst_type_freevalues (env->type, recv_buffer, env->values_num);

  return 0;
}
#endif /* HAVE_MPI2_ONE_SIDED */