/*
 * File: tst_coll_allreduce.c
 *
 * Functionality:
 *  Simple collective Allreduce test-program.
 *  Works with intra- and inter- communicators, MPI_COMM_SELF and up to now with any C (standard and struct) type.
 *
 * Author: Rainer Keller
 *
 * Date: Aug 8th 2003
 */
#include <mpi.h>
#include "mpi_test_suite.h"

#undef DEBUG
#define DEBUG(x)


int tst_coll_allreduce_sum_init (struct tst_env * env)
{
  tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "(Rank:%d) env->comm:%d env->type:%d env->values_num:%d\n",
                 tst_global_rank, env->comm, env->type, env->values_num);

  env->send_buffer = tst_type_allocvalues (env->type, env->values_num);
  env->recv_buffer = tst_type_allocvalues (env->type, env->values_num);

  return 0;
}

int tst_coll_allreduce_sum_run (struct tst_env * env)
{
  int comm_rank;
  int comm_size;
  MPI_Comm comm;
  MPI_Datatype type;

  comm = tst_comm_getcomm (env->comm);
  type = tst_type_getdatatype (env->type);

  MPI_CHECK (MPI_Comm_rank (comm, &comm_rank));
  MPI_CHECK (MPI_Comm_size (comm, &comm_size));

  tst_type_setstandardarray (env->type, env->values_num, env->send_buffer, comm_rank);

  tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "(Rank:%d) comm_size:%d comm_rank:%d\n",
                 tst_global_rank, comm_size, comm_rank);

  tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "(Rank:%d) Going to Allreduce\n",
                 tst_global_rank);

  MPI_CHECK (MPI_Allreduce (env->send_buffer, env->recv_buffer, env->values_num, type, MPI_SUM, comm));

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
	
	if (tst_type_cmpvalue (env->type, cmp_value, &(env->recv_buffer[i*type_size])))
          {
          /* struct tst_mpi_float_int * tmp = (struct tst_mpi_float_int*) &(buffer[i*type_size]); */
          /* ((struct tst_mpi_float_int*)cmp_value)->a, */
            if (tst_report >= TST_REPORT_FULL)
              {
                snprintf (err, sizeof (err), "(Rank:%d) Error at i:%d expected ", tst_global_rank, i);
		tst_type_hexdump (err, cmp_value, type_size);

		snprintf (err, sizeof (err), "(Rank:%d) Error at i:%d but received ", tst_global_rank, i);
		tst_type_hexdump (err, &(env->recv_buffer[i*type_size]), type_size);
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

int tst_coll_allreduce_sum_cleanup (struct tst_env * env)
{
  tst_type_freevalues (env->type, env->send_buffer, env->values_num);
  tst_type_freevalues (env->type, env->recv_buffer, env->values_num);
  return 0;
}
