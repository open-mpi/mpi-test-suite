/*
 * File: tst_env_get_version.c
 *
 * Functionality:
 *   Checks if MPI_Get_version returns a valid MPI version number combination.
 *
 * Author: Christoph Niethammer
 *
 * Date: April. 23., 2013
 */
#include <mpi.h>
#include "mpi_test_suite.h"
#include "tst_output.h"




int tst_env_get_version_init (struct tst_env * env)
{
  tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "(Rank:%d) env->comm:%d env->type:%d env->values_num:%d\n",
                     tst_global_rank, env->comm, env->type, env->values_num);
  return 0;
}

int tst_env_get_version_run (struct tst_env * env)
{
  int version = -1;
  int subversion = -1;

  MPI_CHECK (MPI_Get_version (&version, &subversion));
  tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "(Rank:%d) version: %d, subversion: %d\n", tst_global_rank, version, subversion);

  int valid_mpi_versions[][2] = { {3,1}, {3,0}, {2,2}, {2,1}, {2,0}, {1,2} };
  int i;
  for( i = 0; i < (sizeof(valid_mpi_versions)/sizeof(valid_mpi_versions[0])); i++) {
    if( valid_mpi_versions[i][0] == version && valid_mpi_versions[i][1] == subversion) {
      return 0;
    }
  }
  /* No valid version number combination found. */
  tst_test_recordfailure(env);

  return 1;
}

int tst_env_get_version_cleanup (struct tst_env * env)
{
  return 0;
}

