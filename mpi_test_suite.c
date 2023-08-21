#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <mpi.h>

#include "mpi_test_suite.h"
#include "tst_comm.h"
#include "tst_threads.h"
#include "tst_output.h"
#include "compile_info.h"

#include "cmdline.h"


/****************************************************************************/
/**                                                                        **/
/**                     GLOBAL VARIABLES                                   **/
/**                                                                        **/
/****************************************************************************/
int tst_global_rank = 0;
int tst_global_size = 0;
int tst_atomic = 0;
tst_report_types tst_report = TST_REPORT_RUN;
tst_mode_types tst_mode = TST_MODE_RELAXED;
/*
 * Declaration of output relevant data
 * Do not remove or change the following without modifying tst_output.h!
 */
tst_output_stream tst_output;


/****************************************************************************/
/**                                                                        **/
/**                     EXPORTED VARIABLES                                 **/
/**                                                                        **/
/****************************************************************************/


const char * tst_modes[] = {
  "disabled",
  "strict",
  "relaxed"
};

/* Upper limit for the tag defined in the MPI-2.1 p.28 and MPI-2.1 8.1.2. */
static int tst_tag_ub = 32767;


/****************************************************************************/
/**                                                                        **/
/**                     LOCAL FUNCTIONS                                    **/
/**                                                                        **/
/****************************************************************************/

int tst_hash_value (const struct tst_env * env)
{
  return (env->comm * 65521 + /* Smallest prime smaller than 2^16 */
          env->type * 32749  + /* Smallest prime smaller than 2^16 */
          env->test) % tst_tag_ub;
}

/** \brief Compress array, by removing all -1 entries
 *
 * \param[in,out]  list      array list to be compressed
 * \param[in]      list_max  number of elements in list
 * \param[out]     list_num  number of elements in list after compression
 *
 * \return number of elements after compression
 */
static int tst_array_compress(int *list, const int list_max, int *list_num) {
  int i;
  int current_pos = 0;
  for (i = 0; i < list_max; i++) {
    if (list[i] != -1) {
      list[current_pos] = list[i];
      current_pos++;
    }
  }
  (*list_num) = current_pos;
  return current_pos;
}




int main (int argc, char * argv[])
{
  struct gengetopt_args_info args_info;
  if (cmdline_parser (argc, argv, &args_info) != 0) {
    MPI_Abort(MPI_COMM_WORLD, 1);
    return 1;
  }


  int i, j, k, l;
  int flag;

  int num_comms = 0;
  int num_types = 0;
  int num_tests = 0;
  int num_num_values = 1;
  struct tst_env tst_env;
  int * tst_test_array;
  int tst_test_array_max;
  int * tst_comm_array;
  int tst_comm_array_max;
  int * tst_type_array;
  int tst_type_array_max;
  int tst_value_array[32];
  int tst_value_array_max = 32;
  int * val;
  double time_start, time_stop;
#ifdef HAVE_MPI2_THREADS
  int num_threads = 0;
  int tst_thread_level_provided;
#endif


#ifdef HAVE_MPI2_THREADS
  struct tst_thread_env_t ** tst_thread_env;

  MPI_Init_thread (&argc, &argv, MPI_THREAD_MULTIPLE, &tst_thread_level_provided);

  if (tst_thread_level_provided != MPI_THREAD_MULTIPLE)
    {
      /*
       * XXX LOG CN Should be modified for logfile support. Maybe end Program?
       */
      printf ("Thread level support:%d unequal MPI_THREAD_MULTIPLE\n", tst_thread_level_provided);
      tst_thread_level_provided = MPI_THREAD_SINGLE;
    }

#else
  MPI_Init (&argc, &argv);
#endif

  time_start = MPI_Wtime ();

  MPI_Comm_rank (MPI_COMM_WORLD, &tst_global_rank);
  MPI_Comm_size (MPI_COMM_WORLD, &tst_global_size);

  tst_output_init (DEBUG_LOG, TST_OUTPUT_RANK_SELF, TST_REPORT_MAX, TST_OUTPUT_TYPE_LOGFILE, "tst.log");

  char info_str[MAX_INFO_STRING_LENGTH];
  get_compiler_info(info_str);
  tst_output_printf(DEBUG_LOG, TST_REPORT_RUN, "Compiler used was %s\n", info_str);
  get_mpi_info(info_str);
  tst_output_printf(DEBUG_LOG, TST_REPORT_FULL, "MPI version used was %s\n", info_str);
  get_compile_time(info_str);
  tst_output_printf(DEBUG_LOG, TST_REPORT_FULL, "Compiled at %s\n", info_str);
  get_timestamp(info_str);
  tst_output_printf(DEBUG_LOG, TST_REPORT_FULL, "Started at %s\n", info_str);
#ifndef HAVE_MPI2_THREADS
  tst_output_printf(DEBUG_LOG, TST_REPORT_FULL, "Testsuite was compiled without MPI2_THREADS");
#endif

  #if MPI_VERSION < 2
  MPI_CHECK (MPI_Attr_get (MPI_COMM_WORLD, MPI_TAG_UB, &val, &flag));
  #else
  MPI_CHECK (MPI_Comm_get_attr (MPI_COMM_WORLD, MPI_TAG_UB, &val, &flag));
  #endif
  if (!flag)
    ERROR (EINVAL, "Couldn't retrieve MPI_TAG_UB attribute");

  tst_tag_ub = *val;
  tst_output_printf (DEBUG_LOG, TST_REPORT_FULL, "MPI_TAG_UB is %d\n", tst_tag_ub);

  /* XXX CN This check should be implemented better ...
   */
  /*
   * Checking if the upper boundary for tag values is at least 32767 as required by MPI-1.1.
   * (see MPI-1.1 section 7.1.1.1 Tag values or MPI-3.1 section 8.1.2)
   */
  if (tst_tag_ub < 32767) {
    printf ("Error: MPI_TAG_UB was below 32767.\n");
  }

  tst_output_printf (DEBUG_LOG, TST_REPORT_FULL, "(Rank:%d) MPI_TAG_UB:%d\n",
                    tst_global_rank, tst_tag_ub);

  /* XXX CN Maybe rename these functions to tst_get_num_comms/types/tests ?  */
  tst_comm_array_max = tst_comms_register();
  tst_type_init(&tst_type_array_max);
  tst_test_init(&tst_test_array_max);

  if (tst_comm_array_max < 1 || tst_type_array_max < 1 || tst_test_array_max < 1) {
    ERROR(EINVAL, "Nothing to execute");
  }

  if ((tst_test_array = malloc (sizeof (int) * tst_test_array_max)) == NULL)
    ERROR (errno, "Could not allocate memory");
  /* deselect all tests, will be selected from command line option and its default value */
  for (i = 0; i < tst_test_array_max; i++) {
    tst_test_array[i] = -1;
  }

  if ((tst_comm_array = malloc (sizeof (int) * tst_comm_array_max)) == NULL)
    ERROR (errno, "Could not allocate memory");
  /* deselect all comms, will be selected from command line option and its default value */
  for (i = 0; i < tst_comm_array_max; i++) {
    tst_comm_array[i] = -1;
  }

  if ((tst_type_array = malloc (sizeof (int) * tst_type_array_max)) == NULL)
    ERROR (errno, "Could not allocate memory");
  /* deselect all datatypes, will be selected from command line option and its default value */
  for (i = 0; i < tst_type_array_max; i++) {
    tst_type_array[i] = -1;
  }
  for (i = 0; i < tst_value_array_max; i++) {
    tst_value_array[i] = 0;
  }


  /* just list tests, comms, ... and exit */
  if(args_info.list_given) {
    if (!tst_global_rank)
    {
      tst_test_list ();
      tst_comm_list ();
      tst_type_list ();
      for (i = 0; i < TST_REPORT_MAX; i++)
        printf ("Report:%d %s\n", i, tst_reports[i]);
      for (i = 0; i < TST_MODE_MAX; i++)
        printf ("Test mode:%d %s\n", i, tst_modes[i]);
    }
    MPI_Finalize ();
    exit (0);
  }

  char *str;
  /*
   * select tests
   */
  num_tests = 0;
  str = strtok (args_info.test_arg, ",");
  while (str) {
    /*
     * In case we find the magic word all, we select all the list
     * In case we find a '^', deselect the following test or test-class
     */
    if (0 == strcasecmp("all", str)) {
      /* str returned by strtok terminated by '\0'. So this will
       * match only 'all' but not getting in the way of something
       * like 'Alltoall with' */
      for (i = 0; i < tst_test_array_max; i++) {
        tst_test_array[i] = i;
        num_tests++;
      }
    }
    else if ('^' == str[0]) {
      char tmp_str[TST_DESCRIPTION_LEN+1];
      INTERNAL_CHECK (if (strlen(str) > TST_DESCRIPTION_LEN) ERROR (EINVAL, "Name of test too long for negation"));
      strncpy (tmp_str, &(str[1]), TST_DESCRIPTION_LEN);
      tst_test_deselect (tmp_str, tst_test_array, tst_test_array_max, &num_tests);
    }
    else if ('0' <= str[0] && '9' >= str[0]) {
      int tmp_test = atoi (str);
      if (0 > tmp_test || tst_test_array_max <= tmp_test)
        ERROR (EINVAL, "Specified test number out of range");
      tst_test_array[num_tests++] = tmp_test;
    }
    else {
      tst_test_select (str, tst_test_array, tst_test_array_max, &num_tests);
    }
    str = strtok (NULL, ",");
  }
  tst_array_compress (tst_test_array, tst_test_array_max, &num_tests);
  if (tst_global_rank == 0) {
    for (i = 0; i < num_tests; i++) {
      printf ("(Rank:0) tst_test_array[%d]:%s\n",
          i, tst_test_getdescription(tst_test_array[i]));
    }
  }


  /*
   * select communicators
   */
  num_comms = 0;
  str = strtok (args_info.comm_arg, ",");
  while (str) {
    /*
     * In case we find the magic word all, we select all the list
     * In case we find a '^', deselect the communicator or comm-class
     */
    if (0 == strcasecmp("all", str)) {
      for (i = 0; i < tst_comm_array_max; i++) {
        tst_comm_array[i] = i;
        num_comms++;
      }
    }
    else if ('^' == str[0]) {
      char tmp_str[TST_DESCRIPTION_LEN+1];
      INTERNAL_CHECK (if (strlen(str) > TST_DESCRIPTION_LEN) ERROR (EINVAL, "Name of comm too long for negation"));
      strncpy (tmp_str, &(str[1]), TST_DESCRIPTION_LEN);
      tst_comm_deselect (tmp_str, tst_comm_array, tst_comm_array_max, &num_comms);
    }
    else if ('0' <= str[0] && '9' >= str[0]) {
      int tmp_test_comm = atoi (str);
      if (0 > tmp_test_comm || tst_comm_array_max <= tmp_test_comm)
        ERROR (EINVAL, "Specified communicator number out of range");
      tst_comm_array[num_comms++] = tmp_test_comm;
    }
    else {
      tst_comm_select (str, tst_comm_array, tst_comm_array_max, &num_comms);
    }
    str = strtok (NULL, ",");
  }
  tst_array_compress (tst_comm_array, tst_comm_array_max, &num_comms);

  /*
   * select datatypes
   */
  num_types = 0;
  str = strtok (args_info.datatype_arg, ",");
  while (str) {
    /*
     * In case we find the magic word all, we select all the list
     * In case we find a '^', deselect the type or type type-class
     */
    if (0 == strcasecmp("all", str)) {
      for (i = 0; i < tst_type_array_max; i++) {
        tst_type_array[i] = i;
        num_types++;
      }
    }
    else if ('^' == str[0]) {
      char tmp_str[TST_DESCRIPTION_LEN+1];
      INTERNAL_CHECK (if (strlen(str) > TST_DESCRIPTION_LEN) ERROR (EINVAL, "Name of type too long for negation"));
      strncpy (tmp_str, &(str[1]), TST_DESCRIPTION_LEN);
      tst_type_deselect (tmp_str, tst_type_array, tst_type_array_max, &num_types);
    }
    else if ('0' <= str[0] && '9' >= str[0]) {
      int tmp_test_type = atoi (str);
      if (0 > tmp_test_type || tst_type_array_max <= tmp_test_type)
        ERROR (EINVAL, "Specified type number out of range");
      tst_type_array[num_types++] = tmp_test_type;
    }
    else {
      tst_type_select (str, tst_type_array, tst_type_array_max, &num_types);
    }
    str = strtok (NULL, ",");
  }
  tst_array_compress (tst_type_array, tst_type_array_max, &num_types);

  /*
   * fill list of number of values
   */
  num_num_values = 0;
  str = strtok (args_info.num_values_arg, ",");
  while (str) {
    tst_value_array[num_num_values++] = atoi (str);
    if (num_num_values >= tst_value_array_max)
      ERROR (EINVAL, "Too many values specified");
    str = strtok (NULL, ",");
  }

  for (tst_report = TST_REPORT_SUMMARY; tst_report < TST_REPORT_MAX; tst_report++) {
    if (0 == strcasecmp (args_info.report_arg, tst_reports[tst_report])) {
      break;
    }
  }

  for (tst_mode = TST_MODE_DISABLED; tst_mode < TST_MODE_MAX; tst_mode++) {
    if (0 == strcasecmp (args_info.execution_mode_arg, tst_modes[tst_mode])) {
      break;
    }
  }

  if(args_info.num_threads_arg > 0) {
#ifdef HAVE_MPI2_THREADS
    if (tst_thread_level_provided != MPI_THREAD_MULTIPLE) {
      printf ("Error: The provided thread level from the MPI-Implementation is not sufficient to run with threads.\n");
      MPI_Abort(MPI_COMM_WORLD, 1);
    }
    else {
      num_threads = args_info.num_threads_arg;
    }
#else
    printf ("Error: Threads are not enabled by configure\n");
    MPI_Abort(MPI_COMM_WORLD, 1);
#endif
  }

  if(args_info.atomic_io_given) {
      tst_atomic = 1;
  }

#ifdef HAVE_MPI2_THREADS
  if (num_threads <= 0) {
    printf ("Error: Number of threads must be greater than 0 (given %d)\n", num_threads);
    MPI_Abort(MPI_COMM_WORLD, 1);
  }
  tst_thread_init (num_threads, &tst_thread_env);
#endif

  num_comms = tst_comms_init();
  /*
   * For every test included in the tst_*_array, check if runnable and run!
   */
  tst_output_printf (DEBUG_LOG, TST_REPORT_FULL, "num_tests:%d num_comms:%d num_types:%d\n",
                     num_tests, num_comms, num_types);

  for (i = 0; i < num_tests; i++)
    for (j = 0; j < num_comms; j++)
      for (k = 0; k < num_types; k++)
        for (l = 0; l < num_num_values; l++)
          {
            /*
             * Before setting the mandatory first fields needed, reset.
             * Every test should clean up after itself.
             */
            memset (&tst_env, 0, sizeof (tst_env));
            tst_env.test        = tst_test_array[i];
            tst_env.values_num  = tst_value_array[l];
            tst_env.type        = tst_type_array[k];
            tst_env.tag         = (i+j+k+l) % tst_tag_ub;
            tst_env.comm        = tst_comm_array[j];
            double time_curr = MPI_Wtime ();
            tst_output_printf (DEBUG_LOG, TST_REPORT_FULL, "(Rank:%d) starting test_env.test:%d at time %f\n",
                     tst_global_rank, tst_env.test, time_curr - time_start);

            if (!tst_test_check_run (&tst_env))
              {
                tst_output_printf (DEBUG_LOG, TST_REPORT_FULL, "Not running tst_env.test:%d\n", tst_env.test);
                continue;
              }

            fflush (stderr);
            fflush (stdout);
            if (tst_test_check_sync (&tst_env))
              MPI_Barrier (MPI_COMM_WORLD);

            if (tst_global_rank == 0 && tst_report > TST_REPORT_SUMMARY)
              printf ("%s tests %s (%d/%d), comm %s (%d/%d), type %s (%d/%d)\n",
                      tst_test_getclass_string (tst_env.test),
                      tst_test_getdescription (tst_env.test), tst_env.test+1, num_tests,
                      tst_comm_getdescription (tst_env.comm), tst_env.comm+1, num_comms,
                      tst_type_getdescription (tst_env.type), tst_env.type+1, num_types);
#ifdef HAVE_MPI2_THREADS
            if (num_threads > 0)
              {
                tst_thread_assign_all (&tst_env, tst_thread_env);
                tst_thread_execute_init (&tst_env);
                tst_thread_execute_run (&tst_env);
                tst_thread_execute_cleanup (&tst_env);
              }
            else
#else
              {
                tst_test_init_func (&tst_env);
                tst_test_run_func (&tst_env);
                tst_test_cleanup_func (&tst_env);
              }
#endif
            if (tst_test_check_sync (&tst_env))
              MPI_Barrier (MPI_COMM_WORLD);
          }

  if (tst_global_rank == 0) {
    tst_test_print_failed ();
  }

  time_stop = MPI_Wtime ();
  tst_output_printf (DEBUG_LOG, TST_REPORT_FULL, "(Rank:%d) Overall time taken:%lf\n",
                     tst_global_rank, time_stop - time_start);
  tst_output_printf (DEBUG_LOG, TST_REPORT_FULL, "(Rank:%d) Going to MPI_Finalize\n",
                     tst_global_rank);
  tst_output_close (DEBUG_LOG);

  MPI_Finalize ();

  return (tst_test_get_failed_num () > 0 ? -1 : 0);
}

/****************************************************************************/
/**                                                                        **/
/**                               EOF                                      **/
/**                                                                        **/
/****************************************************************************/
