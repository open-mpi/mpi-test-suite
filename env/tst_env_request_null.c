/*
 * File: tst_env_request_null.c
 *
 * Functionality:
 *   Make sure, MPI_REQUEST_NULL is handled correctly in all the
 *   MPI_Test, MPI_Wait, and variants
 *
 * Author: Rainer Keller
 *
 * Date: Dec. 1., 2005
 */
#include <mpi.h>
#include "mpi_test_suite.h"
#include "tst_output.h"

#undef DEBUG
#define DEBUG(x)

static int tst_test_reset_statuses (int count, MPI_Status * statuses)
{
  int i;
  memset (statuses, DEFAULT_INIT_BYTE, sizeof (MPI_Status) * count);
  for (i = 0; i < count; i++)
    statuses[i].MPI_ERROR = 4711;
  return 0;
}

int tst_env_request_null_init (struct tst_env * env)
{
  tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "(Rank:%d) env->comm:%d env->type:%d env->values_num:%d\n",
                 tst_global_rank, env->comm, env->type, env->values_num);
  return 0;
}

int tst_env_request_null_run (struct tst_env * env)
{
  MPI_Request requests[2] = {MPI_REQUEST_NULL, MPI_REQUEST_NULL};
  MPI_Status statuses[2];
  int flags[2];
  int cancelled[2];
  int count;
  int indices[2];
  int get_count[2];
  int get_elements[2];

  /*
   * This test does not make sense to be run by more than one process.
   */
  if (tst_global_rank)
    return 0;

  /*
   * The below calls should not change the value of MPI_ERROR (see MPI-1.2,
   * sec. 3.2.5, p. 22), or at least should return an empty status.
   *
   * Only the MULTIPLE COMPLETION calls may change the MPI_ERROR field
   * however, it is not said, that they should change in case of
   * inactive Requests. So, we check MPI_ERROR for the two sensible
   * values either MPI_SUCCESS or 4711, and according to the MPI standard
   * with empty status (MPI-2.1, p52, l35):
   *   aka not cancelled, zero count, zero elements,
   *   MPI_TAG == MPI_ANY_TAG and
   *   MPI_SOURCE == MPI_ANY_SOURCE.
   */

#define LOCAL_CHECK(func_string,var,op,expected) \
  if (var op expected) { \
    if (tst_report == TST_REPORT_FULL) \
      printf ("(Rank:%d) Failed in " func_string " " #var " " #op " expected:%d but got:%d\n", \
              tst_global_rank, expected, var); \
    tst_test_recordfailure (env); \
  }

  /*
   * Check MPI_Wait
   */
  tst_test_reset_statuses (1, statuses);
  requests[0] = MPI_REQUEST_NULL;
  requests[1] = MPI_REQUEST_NULL;
  MPI_CHECK (MPI_Wait (&requests[0], &statuses[0]));

  MPI_CHECK (MPI_Test_cancelled (&statuses[0], &cancelled[0]));
  MPI_CHECK (MPI_Get_count (&statuses[0], MPI_CHAR, &get_count[0]));
  MPI_CHECK (MPI_Get_elements (&statuses[0], MPI_CHAR, &get_elements[0]));

  LOCAL_CHECK ("MPI_Wait", (statuses[0].MPI_ERROR != MPI_SUCCESS) && statuses[0].MPI_ERROR, !=, 4711);
  LOCAL_CHECK ("MPI_Wait", cancelled[0], !=, 0);
  LOCAL_CHECK ("MPI_Wait", get_count[0], !=, 0);
  LOCAL_CHECK ("MPI_Wait", get_elements[0], !=, 0);
  LOCAL_CHECK ("MPI_Wait", statuses[0].MPI_SOURCE, !=, MPI_ANY_SOURCE);
  LOCAL_CHECK ("MPI_Wait", statuses[0].MPI_TAG, !=, MPI_ANY_TAG);

  /*
   * Check MPI_Waitall
   */
  tst_test_reset_statuses (2, statuses);
  requests[0] = MPI_REQUEST_NULL;
  requests[1] = MPI_REQUEST_NULL;
  MPI_CHECK (MPI_Waitall (2, requests, statuses));

  MPI_CHECK (MPI_Test_cancelled (&statuses[0], &cancelled[0]));
  MPI_CHECK (MPI_Test_cancelled (&statuses[1], &cancelled[1]));
  MPI_CHECK (MPI_Get_count (&statuses[0], MPI_CHAR, &get_count[0]));
  MPI_CHECK (MPI_Get_count (&statuses[1], MPI_CHAR, &get_count[1]));
  MPI_CHECK (MPI_Get_elements (&statuses[0], MPI_CHAR, &get_elements[0]));
  MPI_CHECK (MPI_Get_elements (&statuses[1], MPI_CHAR, &get_elements[1]));


  LOCAL_CHECK ("MPI_Waitall", (statuses[0].MPI_ERROR != MPI_SUCCESS) && statuses[0].MPI_ERROR, !=, 4711);
  LOCAL_CHECK ("MPI_Waitall", (statuses[1].MPI_ERROR != MPI_SUCCESS) && statuses[1].MPI_ERROR, !=, 4711);
  LOCAL_CHECK ("MPI_Waitall", cancelled[0], !=, 0);
  LOCAL_CHECK ("MPI_Waitall", cancelled[1], !=, 0);
  LOCAL_CHECK ("MPI_Waitall", get_count[0], !=, 0);
  LOCAL_CHECK ("MPI_Waitall", get_count[1], !=, 0);
  LOCAL_CHECK ("MPI_Waitall", get_elements[0], !=, 0);
  LOCAL_CHECK ("MPI_Waitall", get_elements[1], !=, 0);
  LOCAL_CHECK ("MPI_Waitall", statuses[0].MPI_SOURCE, !=, MPI_ANY_SOURCE);
  LOCAL_CHECK ("MPI_Waitall", statuses[0].MPI_TAG, !=, MPI_ANY_TAG);
  LOCAL_CHECK ("MPI_Waitall", statuses[1].MPI_SOURCE, !=, MPI_ANY_SOURCE);
  LOCAL_CHECK ("MPI_Waitall", statuses[1].MPI_TAG, !=, MPI_ANY_TAG);


  /*
   * Check MPI_Waitany
   */
  tst_test_reset_statuses (2, statuses);
  requests[0] = MPI_REQUEST_NULL;
  requests[1] = MPI_REQUEST_NULL;
  /* For a change we return into statuses[1] */
  MPI_CHECK (MPI_Waitany (2, requests, &count, &statuses[1]));

  LOCAL_CHECK ("MPI_Waitany", count, !=, MPI_UNDEFINED);

  MPI_CHECK (MPI_Test_cancelled (&statuses[1], &cancelled[1]));
  MPI_CHECK (MPI_Get_count (&statuses[1], MPI_CHAR, &get_count[1]));
  MPI_CHECK (MPI_Get_elements (&statuses[1], MPI_CHAR, &get_elements[1]));

  /*
   * As said, only the MULTIPLE COMPLETION calls may change the MPI_ERROR field
   * however, it is not said, that they should change in case of inactive Requests.
   * So, we check for the two sensible values either MPI_SUCCESS or 4711
   */
  LOCAL_CHECK ("MPI_Waitany", (statuses[1].MPI_ERROR != MPI_SUCCESS) && statuses[1].MPI_ERROR, !=, 4711);

  LOCAL_CHECK ("MPI_Waitany", cancelled[1], !=, 0);
  LOCAL_CHECK ("MPI_Waitany", get_count[1], !=, 0);
  LOCAL_CHECK ("MPI_Waitany", get_elements[1], !=, 0);
  LOCAL_CHECK ("MPI_Waitany", statuses[1].MPI_SOURCE, !=, MPI_ANY_SOURCE);
  LOCAL_CHECK ("MPI_Waitany", statuses[1].MPI_TAG, !=, MPI_ANY_TAG);

  /*
   * Check MPI_Waitsome
   *
   * Only returns with count == MPI_UNDEFINED
   */
  tst_test_reset_statuses (2, statuses);
  requests[0] = MPI_REQUEST_NULL;
  requests[1] = MPI_REQUEST_NULL;
  MPI_CHECK (MPI_Waitsome (2, requests, &count, indices, statuses));

  LOCAL_CHECK ("MPI_Waitsome", count, !=, MPI_UNDEFINED);

  /*
   * Check MPI_Test
   */
  tst_test_reset_statuses (2, statuses);
  requests[0] = MPI_REQUEST_NULL;
  MPI_CHECK (MPI_Test (&requests[0], &flags[0], &statuses[0]));

  MPI_CHECK (MPI_Test_cancelled (&statuses[0], &cancelled[0]));
  MPI_CHECK (MPI_Get_count (&statuses[0], MPI_CHAR, &get_count[0]));
  MPI_CHECK (MPI_Get_elements (&statuses[0], MPI_CHAR, &get_elements[0]));

  LOCAL_CHECK ("MPI_Test", (statuses[0].MPI_ERROR != MPI_SUCCESS) && statuses[0].MPI_ERROR, !=, 4711);
  LOCAL_CHECK ("MPI_Test", flags[0], !=, 1);
  LOCAL_CHECK ("MPI_Test", cancelled[0], !=, 0);
  LOCAL_CHECK ("MPI_Test", get_count[0], !=, 0);
  LOCAL_CHECK ("MPI_Test", get_elements[0], !=, 0);
  LOCAL_CHECK ("MPI_Test", statuses[0].MPI_SOURCE, !=, MPI_ANY_SOURCE);
  LOCAL_CHECK ("MPI_Test", statuses[0].MPI_TAG, !=, MPI_ANY_TAG);

  /*
   * Check MPI_Testall
   * p48: Each status, that corresponds to inactive Request is set to empty.
   */
  tst_test_reset_statuses (2, statuses);
  requests[0] = MPI_REQUEST_NULL;
  requests[1] = MPI_REQUEST_NULL;
  MPI_CHECK (MPI_Testall (2, requests, &flags[0], statuses));

  MPI_CHECK (MPI_Test_cancelled (&statuses[0], &cancelled[0]));
  MPI_CHECK (MPI_Test_cancelled (&statuses[1], &cancelled[1]));
  MPI_CHECK (MPI_Get_count (&statuses[0], MPI_CHAR, &get_count[0]));
  MPI_CHECK (MPI_Get_count (&statuses[1], MPI_CHAR, &get_count[1]));
  MPI_CHECK (MPI_Get_elements (&statuses[0], MPI_CHAR, &get_elements[0]));
  MPI_CHECK (MPI_Get_elements (&statuses[1], MPI_CHAR, &get_elements[1]));

  LOCAL_CHECK ("MPI_Testall", (statuses[0].MPI_ERROR != MPI_SUCCESS) && statuses[0].MPI_ERROR, !=, 4711);
  LOCAL_CHECK ("MPI_Testall", (statuses[1].MPI_ERROR != MPI_SUCCESS) && statuses[1].MPI_ERROR, !=, 4711);
  LOCAL_CHECK ("MPI_Testall", flags[0], !=, 1);
  LOCAL_CHECK ("MPI_Testall", cancelled[0], !=, 0);
  LOCAL_CHECK ("MPI_Testall", cancelled[1], !=, 0);
  LOCAL_CHECK ("MPI_Testall", get_count[0], !=, 0);
  LOCAL_CHECK ("MPI_Testall", get_count[1], !=, 0);
  LOCAL_CHECK ("MPI_Testall", get_elements[0], !=, 0);
  LOCAL_CHECK ("MPI_Testall", get_elements[1], !=, 0);
  LOCAL_CHECK ("MPI_Testall", statuses[0].MPI_SOURCE, !=, MPI_ANY_SOURCE);
  LOCAL_CHECK ("MPI_Testall", statuses[0].MPI_TAG, !=, MPI_ANY_TAG);
  LOCAL_CHECK ("MPI_Testall", statuses[1].MPI_SOURCE, !=, MPI_ANY_SOURCE);
  LOCAL_CHECK ("MPI_Testall", statuses[1].MPI_TAG, !=, MPI_ANY_TAG);

  /*
   * Check MPI_Testany
   * Supposed to returned empty status!
   */
  tst_test_reset_statuses (2, statuses);
  requests[0] = MPI_REQUEST_NULL;
  requests[1] = MPI_REQUEST_NULL;
  /* For a change we return into statuses[1] */
  MPI_CHECK (MPI_Testany (2, requests, &indices[1], &flags[1], &statuses[1]));

  MPI_CHECK (MPI_Test_cancelled (&statuses[1], &cancelled[1]));
  MPI_CHECK (MPI_Get_count (&statuses[1], MPI_CHAR, &get_count[1]));
  MPI_CHECK (MPI_Get_elements (&statuses[1], MPI_CHAR, &get_elements[1]));

  LOCAL_CHECK ("MPI_Testany", (statuses[1].MPI_ERROR != MPI_SUCCESS) && statuses[1].MPI_ERROR , !=, 4711);
  LOCAL_CHECK ("MPI_Testany", flags[1], !=, 1);
  LOCAL_CHECK ("MPI_Testany", indices[1], !=, MPI_UNDEFINED);
  LOCAL_CHECK ("MPI_Testany", cancelled[1], !=, 0);
  LOCAL_CHECK ("MPI_Testany", get_count[1], !=, 0);
  LOCAL_CHECK ("MPI_Testany", get_elements[1], !=, 0);
  LOCAL_CHECK ("MPI_Testany", statuses[1].MPI_SOURCE, !=, MPI_ANY_SOURCE);
  LOCAL_CHECK ("MPI_Testany", statuses[1].MPI_TAG, !=, MPI_ANY_TAG);

  /*
   * Check MPI_Testsome
   *
   * Only returns with count == MPI_UNDEFINED
   */
  tst_test_reset_statuses (2, statuses);
  MPI_CHECK (MPI_Testsome (2, requests, &count, indices, statuses));

  LOCAL_CHECK ("MPI_Testsome", count, !=, MPI_UNDEFINED);

  /*
   * Check MPI_Probe
   *
   * Supposed to return MPI_SOURCE = MPI_PROC_NULL, MPI_TAG = MPI_ANY_TAG.
   */
  tst_test_reset_statuses (1, statuses);
  MPI_CHECK (MPI_Probe (MPI_PROC_NULL, 4711, MPI_COMM_WORLD, &statuses[0]));

  MPI_CHECK (MPI_Test_cancelled (&statuses[0], &cancelled[0]));
  MPI_CHECK (MPI_Get_count (&statuses[0], MPI_CHAR, &get_count[0]));
  MPI_CHECK (MPI_Get_elements (&statuses[0], MPI_CHAR, &get_elements[0]));

  LOCAL_CHECK ("MPI_Probe", (statuses[0].MPI_ERROR != MPI_SUCCESS) && statuses[0].MPI_ERROR, !=, 4711);
  LOCAL_CHECK ("MPI_Probe", cancelled[0], !=, 0);
  LOCAL_CHECK ("MPI_Probe", get_count[0], !=, 0);
  LOCAL_CHECK ("MPI_Probe", get_elements[0], !=, 0);
  LOCAL_CHECK ("MPI_Probe", statuses[0].MPI_SOURCE, !=, MPI_PROC_NULL);
  LOCAL_CHECK ("MPI_Probe", statuses[0].MPI_TAG, !=, MPI_ANY_TAG);

  /*
   * Check MPI_Iprobe
   * Supposed to return empty status!
   */
  tst_test_reset_statuses (1, statuses);
  MPI_CHECK (MPI_Iprobe (MPI_PROC_NULL, 4711, MPI_COMM_WORLD, &flags[0], &statuses[0]));

  MPI_CHECK (MPI_Test_cancelled (&statuses[0], &cancelled[0]));
  MPI_CHECK (MPI_Get_count (&statuses[0], MPI_CHAR, &get_count[0]));
  MPI_CHECK (MPI_Get_elements (&statuses[0], MPI_CHAR, &get_elements[0]));

  LOCAL_CHECK ("MPI_Iprobe", (statuses[0].MPI_ERROR != MPI_SUCCESS) && statuses[0].MPI_ERROR, !=, 4711);
  LOCAL_CHECK ("MPI_Iprobe", flags[0], !=, 1);
  LOCAL_CHECK ("MPI_Iprobe", cancelled[0], !=, 0);
  LOCAL_CHECK ("MPI_Iprobe", get_count[0], !=, 0);
  LOCAL_CHECK ("MPI_Iprobe", get_elements[0], !=, 0);
  LOCAL_CHECK ("MPI_Iprobe", statuses[0].MPI_SOURCE, !=, MPI_PROC_NULL);
  LOCAL_CHECK ("MPI_Iprobe", statuses[0].MPI_TAG, !=, MPI_ANY_TAG);

  return 0;
}

int tst_env_request_null_cleanup (struct tst_env * env)
{
  return 0;
}
