/*
 * File: tst_coll_alltoall.c
 *
 * Functionality:
 *  Simple collective Alltoall test-program.
 *  Works with intra-communicators, MPI_COMM_SELF and up to now with any C (standard and struct) type.
 *
 * Author: Rainer Keller
 *
 * Date: Jan 11th 2004
 */
#include "mpi.h"
#include "mpi_test_suite.h"
#include "tst_output.h"

#undef DEBUG
#define DEBUG(x)


static int tst_coll_alltoall_setarray (struct tst_env * env,
                                       char * send_buffer,
                                       int comm_rank, int comm_size);
static int tst_coll_alltoall_checkarray (struct tst_env * env,
                                         char * recv_buffer,
                                         int comm_rank, int comm_size);


int tst_coll_alltoall_init (struct tst_env * env)
{
  int comm_size;
  MPI_Comm comm;
  tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "(Rank:%d) env->comm:%d env->type:%d env->values_num:%d\n",
                 tst_global_rank, env->comm, env->type, env->values_num);

  /*
   * We have to allocate comm_size as many values!
   */
  comm = tst_comm_getcomm (env->comm);

  MPI_CHECK (MPI_Comm_size (comm, &comm_size));

  env->send_buffer = tst_type_allocvalues (env->type, env->values_num * comm_size);
  env->recv_buffer = tst_type_allocvalues (env->type, env->values_num * comm_size);

  return 0;
}

static int tst_coll_alltoall_setarray (struct tst_env * env,
                                       char * send_buffer,
                                       int comm_rank, int comm_size)
{
  const int type = env->type;
  const int values_num = env->values_num;
  const int type_size = tst_type_gettypesize (type);
  int j, k;

  for (k=0; k < comm_size; k++)
    for (j=0; j < values_num; j++)
      {
        if (j == 0)
          tst_type_setvalue (type, &(send_buffer[(k*values_num + j) * type_size]),
                            TST_TYPE_SET_MIN, 0);
        else if (j == 1)
          tst_type_setvalue (type, &(send_buffer[(k*values_num + j) * type_size]),
                            TST_TYPE_SET_MAX, 0);
        else
          tst_type_setvalue (type, &(send_buffer[(k*values_num + j) * type_size]),
                            TST_TYPE_SET_VALUE, (100000*comm_rank) + (1000*k) + j);
      }
  return 0;
}


static int tst_coll_alltoall_checkarray (struct tst_env * env,
                                         char * recv_buffer,
                                         int comm_rank, int comm_size)
{
  const int type = env->type;
  const int values_num = env->values_num;
  const int type_size = tst_type_gettypesize (type);
  char * cmp_value;
  int j, k;
  int errors = 0;

  cmp_value = tst_type_allocvalues (type, 1);

  for (k=0; k < comm_size; k++)
    for (j=0; j < values_num; j++)
      {
        if (j == 0)
          {
            tst_type_setvalue (type, cmp_value, TST_TYPE_SET_MIN, 0);
            if (tst_type_cmpvalue (type, &(recv_buffer[(k*values_num + j) * type_size]), cmp_value))
              {
                if (tst_report >= TST_REPORT_FULL)
                  {
                    printf ("(Rank:%d) Error in MIN cmp_value:%d buffer[%d]:%d\n",
                            tst_global_rank, (char)*cmp_value, (k*values_num + j) * type_size, 
                            recv_buffer[(k*values_num + j) * type_size]);
                   }
                errors++;
              }
          }
        else if (j == 1)
          {
            tst_type_setvalue (type, cmp_value, TST_TYPE_SET_MAX, 0);
            if (tst_type_cmpvalue (type, &(recv_buffer[(k*values_num + j) * type_size]), cmp_value))
              {
                if (tst_report >= TST_REPORT_FULL)
                  {
                    printf ("(Rank:%d) Error in MAX cmp_value:%d buffer[%d]:%d\n",
                            tst_global_rank, (char)*cmp_value, (k*values_num + j) * type_size, 
                            recv_buffer[(k*values_num + j) * type_size]);
                  }
                errors++;
              }
          }
        else
          {
            tst_type_setvalue (type, cmp_value, TST_TYPE_SET_VALUE, (100000*comm_rank) + (1000*k) + j);
            if (tst_type_cmpvalue (type, &(env->send_buffer[(k*values_num + j) * type_size]), cmp_value))
              {
                if (tst_report >= TST_REPORT_FULL)
                  {
                    printf ("(Rank:%d) Error in MAX cmp_value:%d buffer[%d]:%d\n",
                            tst_global_rank, (char)*cmp_value, (k*values_num + j) * type_size, 
                            recv_buffer[(k*values_num + j) * type_size]);
                  }
                errors++;
              }
          }
      }
  if (errors)
    tst_test_recordfailure (env);
  return 0;
}


int tst_coll_alltoall_run (struct tst_env * env)
{
  int comm_rank;
  int comm_size;
  MPI_Comm comm;
  MPI_Datatype type;

  comm = tst_comm_getcomm (env->comm);
  type = tst_type_getdatatype (env->type);

  MPI_CHECK (MPI_Comm_rank (comm, &comm_rank));
  MPI_CHECK (MPI_Comm_size (comm, &comm_size));

  tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "(Rank:%d) comm_size:%d comm_rank:%d\n",
                 tst_global_rank, comm_size, comm_rank);

  tst_coll_alltoall_setarray (env, env->send_buffer, comm_rank, comm_size);

  tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "(Rank:%d) Going to Alltoall\n",
                 tst_global_rank);

  MPI_CHECK (MPI_Alltoall (env->send_buffer, env->values_num, type,
                           env->recv_buffer, env->values_num, type, comm));

  tst_coll_alltoall_checkarray (env, env->recv_buffer, comm_rank, comm_size);

  return 0;
}

int tst_coll_alltoall_cleanup (struct tst_env * env)
{
  tst_type_freevalues (env->type, env->send_buffer, env->values_num);
  tst_type_freevalues (env->type, env->recv_buffer, env->values_num);
  return 0;
}
