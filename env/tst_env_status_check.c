/*
 * File: tst_env_status_check.c
 *
 * Functionality:
 *   Checks, that the entry of MPI_ERROR is left untouched in the MPI_Status
 *
 * Author: Rainer Keller
 *
 * Date: Dec. 1., 2005
 */
#include <mpi.h>
#include "mpi_test_suite.h"
#include "tst_output.h"


#define LOCAL_CHECK(func_string,var,op,expected) \
  if (var op expected) { \
    if (tst_report == TST_REPORT_FULL) \
      printf ("(Rank:%d) Failed in " func_string " (Line:%d) " #var " " #op " expected:%d but got:%d\n", \
              tst_global_rank, __LINE__, expected, var); \
    tst_test_recordfailure (env); \
  }


int tst_env_status_check_init (struct tst_env * env)
{
  tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "(Rank:%d) env->comm:%d env->type:%d env->values_num:%d\n",
                 tst_global_rank, env->comm, env->type, env->values_num);
  return 0;
}

int tst_env_status_check_run (struct tst_env * env)
{
  MPI_Comm comm;
  MPI_Status status;
  MPI_Request request;
  int cancelled, get_count, get_elements;
  int flag;

  comm = tst_comm_getcomm (env->comm);

  memset (&status, DEFAULT_INIT_BYTE, sizeof (status));
  status.MPI_ERROR = 4711;

  MPI_CHECK (MPI_Recv (NULL, 0, MPI_CHAR, MPI_PROC_NULL, 4711, comm, &status));

  MPI_CHECK (MPI_Test_cancelled (&status, &cancelled));
  MPI_CHECK (MPI_Get_count (&status, MPI_CHAR, &get_count));
  MPI_CHECK (MPI_Get_elements (&status, MPI_CHAR, &get_elements));

  /*
   * If interpreting MPI-1.2, sec. 3.2.5, p. 22 (changing status.MPI_ERROR for single-completion
   * wait/test calls) in the same sense for MPI_Recv, status.MPI_ERROR should not be changed,
   *
   * Nevertheless, we are not that anal.
   */
  LOCAL_CHECK ("MPI_Recv", (status.MPI_ERROR != MPI_SUCCESS) && status.MPI_ERROR, !=, 4711);
  LOCAL_CHECK ("MPI_Recv", status.MPI_SOURCE, !=, MPI_PROC_NULL);
  LOCAL_CHECK ("MPI_Recv", status.MPI_TAG, !=, MPI_ANY_TAG);

  LOCAL_CHECK ("MPI_Recv", cancelled, !=, 0);
  LOCAL_CHECK ("MPI_Recv", get_count, !=, 0);
  LOCAL_CHECK ("MPI_Recv", get_elements, !=, 0);


  /*
   * Do the same with MPI_Irecv + MPI_Request_get_status, expecting to have request pass that through the status
   */
  memset (&status, DEFAULT_INIT_BYTE, sizeof (status));
  status.MPI_ERROR = 4711;

  MPI_CHECK (MPI_Irecv (NULL, 0, MPI_CHAR, MPI_PROC_NULL, 4711, comm, &request));

  MPI_CHECK (MPI_Request_get_status (request, &flag, &status));
  MPI_CHECK (MPI_Test_cancelled (&status, &cancelled));
  MPI_CHECK (MPI_Get_count (&status, MPI_CHAR, &get_count));
  MPI_CHECK (MPI_Get_elements (&status, MPI_CHAR, &get_elements));

  /*
   * If interpreting MPI-1.2, sec. 3.2.5, p. 22 (changing status.MPI_ERROR for single-completion
   * wait/test calls) in the same sense for MPI_Recv, status.MPI_ERROR should not be changed,
   *
   * Nevertheless, we are not that anal.
   */
  LOCAL_CHECK ("MPI_Irecv / MPI_Request_get_status", flag, !=, 1);
  LOCAL_CHECK ("MPI_Irecv / MPI_Request_get_status", (status.MPI_ERROR != MPI_SUCCESS) && status.MPI_ERROR, !=, 4711);
  LOCAL_CHECK ("MPI_Irecv / MPI_Request_get_status", status.MPI_SOURCE, !=, MPI_PROC_NULL);
  LOCAL_CHECK ("MPI_Irecv / MPI_Request_get_status", status.MPI_TAG, !=, MPI_ANY_TAG);

  LOCAL_CHECK ("MPI_Irecv / MPI_Request_get_status", cancelled, !=, 0);
  LOCAL_CHECK ("MPI_Irecv / MPI_Request_get_status", get_count, !=, 0);
  LOCAL_CHECK ("MPI_Irecv / MPI_Request_get_status", get_elements, !=, 0);

  MPI_CHECK (MPI_Request_free (&request));
  LOCAL_CHECK ("MPI_Irecv / MPI_Request_get_status / MPI_Request_free", request, !=, MPI_REQUEST_NULL);


  /*
   * Do the same with MPI_Irecv + MPI_Test, expecting to have request pass that through the status
   */
  memset (&status, DEFAULT_INIT_BYTE, sizeof (status));
  status.MPI_ERROR = 4711;

  MPI_CHECK (MPI_Irecv (NULL, 0, MPI_CHAR, MPI_PROC_NULL, 4711, comm, &request));

  MPI_CHECK (MPI_Test (&request, &flag, &status));
  MPI_CHECK (MPI_Test_cancelled (&status, &cancelled));
  MPI_CHECK (MPI_Get_count (&status, MPI_CHAR, &get_count));
  MPI_CHECK (MPI_Get_elements (&status, MPI_CHAR, &get_elements));

  /*
   * If interpreting MPI-1.2, sec. 3.2.5, p. 22 (changing status.MPI_ERROR for single-completion
   * wait/test calls) in the same sense for MPI_Recv, status.MPI_ERROR should not be changed,
   *
   * Nevertheless, we are not that anal.
   */
  LOCAL_CHECK ("MPI_Irecv / MPI_Test", flag, !=, 1);
  LOCAL_CHECK ("MPI_Irecv / MPI_Test", (status.MPI_ERROR != MPI_SUCCESS) && status.MPI_ERROR, !=, 4711);
  LOCAL_CHECK ("MPI_Irecv / MPI_Test", status.MPI_SOURCE, !=, MPI_PROC_NULL);
  LOCAL_CHECK ("MPI_Irecv / MPI_Test", status.MPI_TAG, !=, MPI_ANY_TAG);

  LOCAL_CHECK ("MPI_Irecv / MPI_Test", cancelled, !=, 0);
  LOCAL_CHECK ("MPI_Irecv / MPI_Test", get_count, !=, 0);
  LOCAL_CHECK ("MPI_Irecv / MPI_Test", get_elements, !=, 0);


  /*
   * Do the same with MPI_Irecv + MPI_Testall, expecting to have request pass that through the status
   */
  memset (&status, DEFAULT_INIT_BYTE, sizeof (status));
  status.MPI_ERROR = 4711;

  MPI_CHECK (MPI_Irecv (NULL, 0, MPI_CHAR, MPI_PROC_NULL, 4711, comm, &request));

  MPI_CHECK (MPI_Testall (1, &request, &flag, &status));
  MPI_CHECK (MPI_Test_cancelled (&status, &cancelled));
  MPI_CHECK (MPI_Get_count (&status, MPI_CHAR, &get_count));
  MPI_CHECK (MPI_Get_elements (&status, MPI_CHAR, &get_elements));

  /*
   * If interpreting MPI-1.2, sec. 3.2.5, p. 22 (changing status.MPI_ERROR for single-completion
   * wait/test calls) in the same sense for MPI_Recv, status.MPI_ERROR should not be changed,
   *
   * Nevertheless, we are not that anal.
   */
  LOCAL_CHECK ("MPI_Irecv / MPI_Testall", flag, !=, 1);
  LOCAL_CHECK ("MPI_Irecv / MPI_Testall", (status.MPI_ERROR != MPI_SUCCESS) && status.MPI_ERROR, !=, 4711);
  LOCAL_CHECK ("MPI_Irecv / MPI_Testall", status.MPI_SOURCE, !=, MPI_PROC_NULL);
  LOCAL_CHECK ("MPI_Irecv / MPI_Testall", status.MPI_TAG, !=, MPI_ANY_TAG);

  LOCAL_CHECK ("MPI_Irecv / MPI_Testall", cancelled, !=, 0);
  LOCAL_CHECK ("MPI_Irecv / MPI_Testall", get_count, !=, 0);
  LOCAL_CHECK ("MPI_Irecv / MPI_Testall", get_elements, !=, 0);


  /********************* Persistent communication (MPI_PROC_NULL) **********************************/
  /********************* As these are never started, expect an empty status *******************************/
  /*
   * Do the same for MPI_Send_init using MPI_Request_get_status:
   */
  memset (&status, DEFAULT_INIT_BYTE, sizeof (status));
  status.MPI_ERROR = 4711;

  MPI_CHECK (MPI_Send_init (&flag, 1, MPI_INT, MPI_PROC_NULL, 4711, comm, &request));

  MPI_CHECK (MPI_Request_get_status (request, &flag, &status));
  MPI_CHECK (MPI_Test_cancelled (&status, &cancelled));
  MPI_CHECK (MPI_Get_count (&status, MPI_CHAR, &get_count));
  MPI_CHECK (MPI_Get_elements (&status, MPI_CHAR, &get_elements));

  LOCAL_CHECK ("MPI_Send_init / MPI_Request_get_status", flag, !=, 1);
  LOCAL_CHECK ("MPI_Send_init / MPI_Request_get_status", (status.MPI_ERROR != MPI_SUCCESS) && status.MPI_ERROR, !=, 4711);
  LOCAL_CHECK ("MPI_Send_init / MPI_Request_get_status", status.MPI_SOURCE, !=, MPI_ANY_SOURCE);
  LOCAL_CHECK ("MPI_Send_init / MPI_Request_get_status", status.MPI_TAG, !=, MPI_ANY_TAG);

  LOCAL_CHECK ("MPI_Send_init / MPI_Request_get_status", cancelled, !=, 0);
  LOCAL_CHECK ("MPI_Send_init / MPI_Request_get_status", get_count, !=, 0);
  LOCAL_CHECK ("MPI_Send_init / MPI_Request_get_status", get_elements, !=, 0);

  MPI_CHECK (MPI_Request_free (&request));
  LOCAL_CHECK ("MPI_Send_init / MPI_Request_get_status / MPI_Request_free", request, !=, MPI_REQUEST_NULL);


  /*
   * Do the same for MPI_Send_init using MPI_Test:
   */
  memset (&status, DEFAULT_INIT_BYTE, sizeof (status));
  status.MPI_ERROR = 4711;

  MPI_CHECK (MPI_Send_init (&flag, 1, MPI_INT, MPI_PROC_NULL, 4711, comm, &request));

  MPI_CHECK (MPI_Test (&request, &flag, &status));
  MPI_CHECK (MPI_Test_cancelled (&status, &cancelled));
  MPI_CHECK (MPI_Get_count (&status, MPI_CHAR, &get_count));
  MPI_CHECK (MPI_Get_elements (&status, MPI_CHAR, &get_elements));

  LOCAL_CHECK ("MPI_Send_init / MPI_Test", flag, !=, 1);
  LOCAL_CHECK ("MPI_Send_init / MPI_Test", (status.MPI_ERROR != MPI_SUCCESS) && status.MPI_ERROR, !=, 4711);
  LOCAL_CHECK ("MPI_Send_init / MPI_Test", status.MPI_SOURCE, !=, MPI_ANY_SOURCE);
  LOCAL_CHECK ("MPI_Send_init / MPI_Test", status.MPI_TAG, !=, MPI_ANY_TAG);

  LOCAL_CHECK ("MPI_Send_init / MPI_Test", cancelled, !=, 0);
  LOCAL_CHECK ("MPI_Send_init / MPI_Test", get_count, !=, 0);
  LOCAL_CHECK ("MPI_Send_init / MPI_Test", get_elements, !=, 0);


  /*
   * Do the same for MPI_Send_init using MPI_Test:
   */
  memset (&status, DEFAULT_INIT_BYTE, sizeof (status));
  status.MPI_ERROR = 4711;

  MPI_CHECK (MPI_Send_init (&flag, 1, MPI_INT, MPI_PROC_NULL, 4711, comm, &request));

  MPI_CHECK (MPI_Testall (1, &request, &flag, &status));
  MPI_CHECK (MPI_Test_cancelled (&status, &cancelled));
  MPI_CHECK (MPI_Get_count (&status, MPI_CHAR, &get_count));
  MPI_CHECK (MPI_Get_elements (&status, MPI_CHAR, &get_elements));

  LOCAL_CHECK ("MPI_Send_init / MPI_Testall", flag, !=, 1);
  LOCAL_CHECK ("MPI_Send_init / MPI_Testall", (status.MPI_ERROR != MPI_SUCCESS) && status.MPI_ERROR, !=, 4711);
  LOCAL_CHECK ("MPI_Send_init / MPI_Testall", status.MPI_SOURCE, !=, MPI_ANY_SOURCE);
  LOCAL_CHECK ("MPI_Send_init / MPI_Testall", status.MPI_TAG, !=, MPI_ANY_TAG);

  LOCAL_CHECK ("MPI_Send_init / MPI_Testall", cancelled, !=, 0);
  LOCAL_CHECK ("MPI_Send_init / MPI_Testall", get_count, !=, 0);
  LOCAL_CHECK ("MPI_Send_init / MPI_Testall", get_elements, !=, 0);



  /*
   * Do the same for MPI_Recv_init using MPI_Request_get_status:
   */
  memset (&status, DEFAULT_INIT_BYTE, sizeof (status));
  status.MPI_ERROR = 4711;

  MPI_CHECK (MPI_Recv_init (&flag, 1, MPI_INT, MPI_PROC_NULL, 4711, comm, &request));

  MPI_CHECK (MPI_Request_get_status (request, &flag, &status));
  MPI_CHECK (MPI_Test_cancelled (&status, &cancelled));
  MPI_CHECK (MPI_Get_count (&status, MPI_CHAR, &get_count));
  MPI_CHECK (MPI_Get_elements (&status, MPI_CHAR, &get_elements));

  LOCAL_CHECK ("MPI_Recv_init / MPI_Request_get_status", flag, !=, 1);
  LOCAL_CHECK ("MPI_Recv_init / MPI_Request_get_status", (status.MPI_ERROR != MPI_SUCCESS) && status.MPI_ERROR, !=, 4711);
  LOCAL_CHECK ("MPI_Recv_init / MPI_Request_get_status", status.MPI_SOURCE, !=, MPI_ANY_SOURCE);
  LOCAL_CHECK ("MPI_Recv_init / MPI_Request_get_status", status.MPI_TAG, !=, MPI_ANY_TAG);

  LOCAL_CHECK ("MPI_Recv_init / MPI_Request_get_status", cancelled, !=, 0);
  LOCAL_CHECK ("MPI_Recv_init / MPI_Request_get_status", get_count, !=, 0);
  LOCAL_CHECK ("MPI_Recv_init / MPI_Request_get_status", get_elements, !=, 0);

  MPI_CHECK (MPI_Request_free (&request));
  LOCAL_CHECK ("MPI_Recv_init / MPI_Request_get_status / MPI_Request_free", request, !=, MPI_REQUEST_NULL);


  /*
   * Do the same for MPI_Recv_init using MPI_Test:
   */
  memset (&status, DEFAULT_INIT_BYTE, sizeof (status));
  status.MPI_ERROR = 4711;

  MPI_CHECK (MPI_Recv_init (&flag, 1, MPI_INT, MPI_PROC_NULL, 4711, comm, &request));

  MPI_CHECK (MPI_Test (&request, &flag, &status));
  MPI_CHECK (MPI_Test_cancelled (&status, &cancelled));
  MPI_CHECK (MPI_Get_count (&status, MPI_CHAR, &get_count));
  MPI_CHECK (MPI_Get_elements (&status, MPI_CHAR, &get_elements));

  LOCAL_CHECK ("MPI_Recv_init / MPI_Test", flag, !=, 1);
  LOCAL_CHECK ("MPI_Recv_init / MPI_Test", (status.MPI_ERROR != MPI_SUCCESS) && status.MPI_ERROR, !=, 4711);
  LOCAL_CHECK ("MPI_Recv_init / MPI_Test", status.MPI_SOURCE, !=, MPI_ANY_SOURCE);
  LOCAL_CHECK ("MPI_Recv_init / MPI_Test", status.MPI_TAG, !=, MPI_ANY_TAG);

  LOCAL_CHECK ("MPI_Recv_init / MPI_Test", cancelled, !=, 0);
  LOCAL_CHECK ("MPI_Recv_init / MPI_Test", get_count, !=, 0);
  LOCAL_CHECK ("MPI_Recv_init / MPI_Test", get_elements, !=, 0);


  /*
   * Do the same for MPI_Recv_init using MPI_Test:
   */
  memset (&status, DEFAULT_INIT_BYTE, sizeof (status));
  status.MPI_ERROR = 4711;

  MPI_CHECK (MPI_Recv_init (&flag, 1, MPI_INT, MPI_PROC_NULL, 4711, comm, &request));

  MPI_CHECK (MPI_Testall (1, &request, &flag, &status));
  MPI_CHECK (MPI_Test_cancelled (&status, &cancelled));
  MPI_CHECK (MPI_Get_count (&status, MPI_CHAR, &get_count));
  MPI_CHECK (MPI_Get_elements (&status, MPI_CHAR, &get_elements));

  LOCAL_CHECK ("MPI_Recv_init / MPI_Testall", flag, !=, 1);
  LOCAL_CHECK ("MPI_Recv_init / MPI_Testall", (status.MPI_ERROR != MPI_SUCCESS) && status.MPI_ERROR, !=, 4711);
  LOCAL_CHECK ("MPI_Recv_init / MPI_Testall", status.MPI_SOURCE, !=, MPI_ANY_SOURCE);
  LOCAL_CHECK ("MPI_Recv_init / MPI_Testall", status.MPI_TAG, !=, MPI_ANY_TAG);

  LOCAL_CHECK ("MPI_Recv_init / MPI_Testall", cancelled, !=, 0);
  LOCAL_CHECK ("MPI_Recv_init / MPI_Testall", get_count, !=, 0);
  LOCAL_CHECK ("MPI_Recv_init / MPI_Testall", get_elements, !=, 0);



  /********************* Persistent communication (specific source/dest) **********************************/
  /*
   * Do the same for MPI_Send_init with SPECIFIC destination (0):
   * Have MPI_Request_get_status on this inactive request, but here with a specific source / destination
   * Persisent, INactive Request, expect an empty status.
   */
  memset (&status, DEFAULT_INIT_BYTE, sizeof (status));
  status.MPI_ERROR = 4711;

  MPI_CHECK (MPI_Send_init (&flag, 1, MPI_INT, 0, 4711, comm, &request));

  MPI_CHECK (MPI_Request_get_status (request, &flag, &status));
  MPI_CHECK (MPI_Test_cancelled (&status, &cancelled));
  MPI_CHECK (MPI_Get_count (&status, MPI_CHAR, &get_count));
  MPI_CHECK (MPI_Get_elements (&status, MPI_CHAR, &get_elements));

  LOCAL_CHECK ("MPI_Send_init / MPI_Request_get_status", flag, !=, 1);
  LOCAL_CHECK ("MPI_Send_init / MPI_Request_get_status", (status.MPI_ERROR != MPI_SUCCESS) && status.MPI_ERROR, !=, 4711);
  LOCAL_CHECK ("MPI_Send_init / MPI_Request_get_status", status.MPI_SOURCE, !=, MPI_ANY_SOURCE);
  LOCAL_CHECK ("MPI_Send_init / MPI_Request_get_status", status.MPI_TAG, !=, MPI_ANY_TAG);

  LOCAL_CHECK ("MPI_Send_init / MPI_Request_get_status", cancelled, !=, 0);
  LOCAL_CHECK ("MPI_Send_init / MPI_Request_get_status", get_count, !=, 0);
  LOCAL_CHECK ("MPI_Send_init / MPI_Request_get_status", get_elements, !=, 0);

  MPI_CHECK (MPI_Request_free (&request));
  LOCAL_CHECK ("MPI_Send_init / MPI_Request_get_status / MPI_Request_free", request, !=, MPI_REQUEST_NULL);


  /*
   * Do the same for MPI_Recv_init with SPECIFIC destination (0):
   * Have MPI_Request_get_status on this inactive request
   */
  memset (&status, DEFAULT_INIT_BYTE, sizeof (status));
  status.MPI_ERROR = 4711;

  MPI_CHECK (MPI_Recv_init (&flag, 1, MPI_INT, 0, 4711, comm, &request));

  MPI_CHECK (MPI_Request_get_status (request, &flag, &status));
  MPI_CHECK (MPI_Test_cancelled (&status, &cancelled));
  MPI_CHECK (MPI_Get_count (&status, MPI_CHAR, &get_count));
  MPI_CHECK (MPI_Get_elements (&status, MPI_CHAR, &get_elements));

  LOCAL_CHECK ("MPI_Recv_init / MPI_Request_get_status", flag, !=, 1);
  LOCAL_CHECK ("MPI_Recv_init / MPI_Request_get_status", (status.MPI_ERROR != MPI_SUCCESS) && status.MPI_ERROR, !=, 4711);
  LOCAL_CHECK ("MPI_Recv_init / MPI_Request_get_status", status.MPI_SOURCE, !=, MPI_ANY_SOURCE);
  LOCAL_CHECK ("MPI_Recv_init / MPI_Request_get_status", status.MPI_TAG, !=, MPI_ANY_TAG);

  LOCAL_CHECK ("MPI_Recv_init / MPI_Request_get_status", cancelled, !=, 0);
  LOCAL_CHECK ("MPI_Recv_init / MPI_Request_get_status", get_count, !=, 0);
  LOCAL_CHECK ("MPI_Recv_init / MPI_Request_get_status", get_elements, !=, 0);

  MPI_CHECK (MPI_Request_free (&request));
  LOCAL_CHECK ("MPI_Recv_init / MPI_Request_get_status (2) / MPI_Request_free", request, !=, MPI_REQUEST_NULL);

  return 0;
}

int tst_env_status_check_cleanup (struct tst_env * env)
{
  return 0;
}
