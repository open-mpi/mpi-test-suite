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
#include <mpi.h>
#include "mpi_test_suite.h"
#include "tst_output.h"

#undef DEBUG
#define DEBUG(x)


int tst_p2p_many_to_one_init (struct tst_env * env)
{
  int comm_rank;
  MPI_Comm comm;

  tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "(Rank:%d) env->comm:%d env->type:%d env->values_num:%d\n",
                     tst_global_rank, env->comm, env->type, env->values_num);

  env->send_buffer = tst_type_allocvalues (env->type, env->values_num);

  /*
   * Now, initialize the send_buffer
   */
  comm = tst_comm_getcomm (env->comm);
  MPI_CHECK (MPI_Comm_rank (comm, &comm_rank));

  tst_type_setstandardarray (env->type, env->values_num, env->send_buffer, comm_rank);
  return 0;
}

int tst_p2p_many_to_one_run (struct tst_env * env)
{
  int comm_size;
  int comm_rank;
  int hash_value;
  int recv_count;
  MPI_Comm comm;
  MPI_Datatype type;
  MPI_Status status;

  comm = tst_comm_getcomm (env->comm);
  type = tst_type_getdatatype (env->type);

  if (tst_comm_getcommclass (env->comm) & TST_MPI_INTRA_COMM)
    MPI_CHECK (MPI_Comm_size (comm, &comm_size));
  else
    MPI_CHECK (MPI_Comm_remote_size (comm, &comm_size));

  MPI_CHECK (MPI_Comm_rank (comm, &comm_rank));

  hash_value = tst_hash_value (env);
  tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "(Rank:%d) comm:%d type:%d test:%d hash_value:%d comm_size:%d comm_rank:%d\n",
                 tst_global_rank,
                 env->comm, env->type, env->test, hash_value,
                 comm_size, comm_rank);

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
          MPI_CHECK (MPI_Recv (env->send_buffer, env->values_num, type, MPI_ANY_SOURCE, hash_value, comm, &status));
          if (status.MPI_TAG != hash_value ||
              status.MPI_SOURCE <= 0 ||
              status.MPI_SOURCE > comm_size-1)
            ERROR (EINVAL, "Error in status");
          tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "(Rank:%d) going to check array from source:%d\n",
                             comm_rank, status.MPI_SOURCE);
          if (tst_mode == TST_MODE_STRICT)
            {
               MPI_CHECK(MPI_Get_count(&status, type, &recv_count));
               if(recv_count != env->values_num)
                  ERROR(EINVAL, "Error in Count");
            }
          tst_test_checkstandardarray (env, env->send_buffer, status.MPI_SOURCE);
        }
    }
  else
    MPI_CHECK (MPI_Send (env->send_buffer, env->values_num, type, 0, hash_value, comm));

  return 0;
}

int tst_p2p_many_to_one_cleanup (struct tst_env * env)
{
  tst_type_freevalues (env->type, env->send_buffer, env->values_num);
  return 0;
}
