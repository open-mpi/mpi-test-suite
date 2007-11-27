#include "config.h"
#include <stdio.h>
#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif
#ifdef HAVE_GETOPT_H
#  include <getopt.h>
#endif
#include "mpi.h"
#include "mpi_test_suite.h"

#include "tst_output.h"

#undef DEBUG
#define DEBUG(x)

#define NUM_VALUES 1000

/*
 * Global variables
 */
int tst_global_rank = 0;
int tst_global_size = 0;
int tst_verbose = 0;
tst_report_types tst_report = TST_REPORT_SUMMARY;
tst_mode_types tst_mode = TST_MODE_RELAXED;

/*
 * Declaration of output_relevant data
 * Do not remove or change the following without modifying tst_output.h!
 */
tst_output_stream tst_output;

/*
 * This should correspond to the enum tst_report
 */
const char * tst_reports[] = {
  "Summary",
  "Run",
  "Full"
};

const char * tst_modes[] = {
  "disabled",
  "strict",
  "relaxed"
};

static int tst_tag_ub = 32766;


/*
 * Global functions, which don't fit into another category.
 */
int tst_hash_value (const struct tst_env * env)
{
  return (env->comm * 65521 + /* Smallest prime smaller than 2^16 */
          env->type * 32749  + /* Smallest prime smaller than 2^16 */
          env->test) % tst_tag_ub;
}

static int tst_array_compress (int * list, const int list_max, int * list_num)
{
  int i;
  int current_pos;
  /*
   * Compress array, by removing all -1 entries.
   */
  for (current_pos = i = 0; i < list_max; i++)
    {
      list[current_pos] = list[i];
      tst_output_printf (DEBUG_LOG, TST_REPORT_FULL, "(Rank:%d) tst_array_compress list[%d]=list[%d]:%d\n",
                         tst_global_rank, current_pos, i, list[current_pos]);
      if (list[i] != -1)
        current_pos++;
    }
  (*list_num) = current_pos;
  return 0;
}



static int usage (void)
{
  fprintf (stderr, "Usage: mpi_test_suite [-h] [-v] [-l] [-t test] [-c comm] [-d datatype]\n"
           "       [-n num_values] [-r report] [-x execution_mode] [-j num_threads]\n"
           "test:\t\tone (or more) tests or test-classes (see -l) and\n"
           "comm:\t\tone (or more) communicator or communicator-classes (see -l)\n"
           "datatype:\tone (or more) datatype or datatype-classes (see -l)\n"
           "num_values:\tone (or more) numbers of values to communicate (default:%d)\n"
           "report:\t\tlevel of detail for tests being run, see -l (default:SUMMARY)\n"
           "execution_mode:\tlevel of correctness testing, tests to run and internal tests, see -l (default:RELAXED)\n"
           "num_threads:\tnumber of threads to execute the tests (default:no threads)\n"
           "\n"
           "All multiple test/comm/datatype-names must be comma-separated.\n"
           "Names are not case-sensisitve, due to spaces in names, proper quoting should be used.\n"
           "\n"
           "-h:\t\tShow this help\n"
           "-v:\t\tTurn on verbose mode for debugging output\n"
           "-l/--list:\tList all available tests, communicators, datatypes and\n"
           "\t\tcorresponding classes.\n"
           " \n",
           NUM_VALUES);
  MPI_Abort (MPI_COMM_WORLD, 0);
  exit (0);
  return 0;
}


int main (int argc, char * argv[])
{
  int i;
  int j;
  int k;
  int l;
  int flag;
  int tst_global_size;
  int num_comms;
  int num_types;
  int num_tests;
  int num_values = 1;
  int num_threads = 0;
  struct tst_env tst_env;
  int * tst_test_array;
  int tst_test_array_max;
  int * tst_comm_array;
  int tst_comm_array_max;
  int * tst_type_array;
  int tst_type_array_max;
  int tst_value_array[32] = {NUM_VALUES, 0, };
  int tst_value_array_max = 32;
  int * val;
  int tst_thread_level_provided;


  /*
   * Unfortunately, this has to be called before the actual MPI_Init
   */
  tst_profiling_init (10000, 50);

#ifdef HAVE_MPI2_THREADS
  struct tst_thread_env_t ** tst_thread_env;

  MPI_Init_thread (&argc, &argv, MPI_THREAD_MULTIPLE, &tst_thread_level_provided);

  if (tst_thread_level_provided != MPI_THREAD_MULTIPLE)
    { 
      printf ("Thread level support:%d unequal MPI_THREAD_MULTIPLE\n", tst_thread_level_provided);
      tst_thread_level_provided = MPI_THREAD_SINGLE;
    }

#else
  tst_thread_level_provided = 0;   /* MPI_THREAD_SINGLE would not work, if not MPI-2, as not defined */
  num_threads = 0;                 /* silence the compiler */
  MPI_Init (&argc, &argv);
#endif
  MPI_Comm_rank (MPI_COMM_WORLD, &tst_global_rank);
  MPI_Comm_size (MPI_COMM_WORLD, &tst_global_size);

  if (NULL != getenv ("MPI_TEST_SUITE_SLEEP"))
    {
      char hostname[256];
      gethostname (hostname, 256);
      hostname[255] = '\0';
      printf ("(Rank:%d) host:%s pid:%ld Going to sleep\n",
              tst_global_rank, hostname, (long int)getpid());
      sleep (30);
    }

  tst_output_init ( DEBUG_LOG, TST_OUTPUT_RANK_MASTER, TST_REPORT_MAX, TST_OUTPUT_TYPE_LOGFILE, "tst.log");

#ifndef HAVE_MPI2_THREADS
  tst_output_printf (DEBUG_LOG, TST_REPORT_FULL, "Testsuite was compiled without MPI2_THREADS");
#endif
  /*
   * Output example:
   * tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "Hello from rank %d\n", tst_global_rank);
   */

  MPI_CHECK (MPI_Attr_get (MPI_COMM_WORLD, MPI_TAG_UB, &val, &flag));
  if (!flag)
    ERROR (EINVAL, "Couldn't retrieve MPI_TAG_UB attribute");

  tst_tag_ub = *val;

  tst_output_printf (DEBUG_LOG, TST_REPORT_FULL, "(Rank:%d) MPI_TAG_UB:%d\n",
                    tst_global_rank, tst_tag_ub);

  tst_comm_init (&num_comms);
  tst_type_init (&num_types);

  /* tst_reduce_fn_init (&num_reduce_fns); */
  tst_test_init (&num_tests);

  /*
   * Schedule every test to be run.
   */
  if ((tst_test_array = malloc (sizeof (int) * num_tests)) == NULL)
    ERROR (errno, "Could not allocate memory");

  for (i = 0; i < num_tests; i++)
    tst_test_array[i] = i;
  tst_test_array_max = num_tests;

  /*
   * Schedule every communicator to be run.
   */
  if ((tst_comm_array = malloc (sizeof (int) * num_comms)) == NULL)
    ERROR (errno, "Could not allocate memory");

  for (i = 0; i < num_comms; i++)
    tst_comm_array[i] = i;
  tst_comm_array_max = num_comms;


  /*
   * Schedule every datatype to be run.
   */
  if ((tst_type_array = malloc (sizeof (int) * num_types)) == NULL)
    ERROR (errno, "Could not allocate memory");

  for (i = 0; i < num_types; i++)
    tst_type_array[i] = i;
  tst_type_array_max = num_types;

  /*
   * Scan command-line arguments
   * Here, we may exit -- this means all command-line arguments should be available to all processes!
   */
  while (1)
    {
      int c;
      /*
      int option_index = 0;
      const static struct option long_options[] = {
        {"test", 1, NULL, 0},
        {"communicator", 1, NULL, 0},
        {"datatype", 1, NULL, 0},
        {"list", 0, NULL, 0},
        {"verbose", 0, NULL, 0},
        {"help", 0, NULL, 0},
        {NULL, 0, NULL, 0}
      };
      */

      c = getopt (argc, argv, "t:c:d:n:r:x:j:lvh");

      if (c == -1)
        break;

      switch (c)
        {
        case 't':
          {
            char * str;
            /*
             * At first, reset the tst_test_array
             */
            for (i = 0; i < tst_test_array_max; i++)
              tst_test_array[i] = -1;
            num_tests = 0;

            str = strtok (optarg, ",");
            while (str)
              {
                /*
                 * In case we find the magic word all, we reset the list as above.
                 * In case we find a '!', deselect the test (test-class)
                 */
                /* Need to Check how to test for 'All' but not getting in the way of sthing like 'Alltoall with' */
                if (!strncasecmp ("All", str, strlen("All")) &&
                    (str[strlen("All")+1]==','))
                  {
                    for (i = 0; i < tst_test_array_max; i++)
                      tst_test_array[i] = i;
                    num_tests = tst_test_array_max;
                  }
                else if ('!' == str[0])
                  {
                    char tmp_str[TST_DESCRIPTION_LEN+1];

                    INTERNAL_CHECK (if (strlen(str) > TST_DESCRIPTION_LEN) ERROR (EINVAL, "Name of test too long for negation"));

                    strncpy (tmp_str, &(str[1]), TST_DESCRIPTION_LEN);
                    tst_test_deselect (tmp_str, tst_test_array, tst_test_array_max, &num_tests);
                  }
                else
                  tst_test_select (str, tst_test_array, tst_test_array_max, &num_tests);
                str = strtok (NULL, ",");
              }
            tst_array_compress (tst_test_array, tst_test_array_max, &num_tests);
            if (tst_global_rank == 0)
              {
                for (i = 0; i < num_tests; i++)
                  {
                    printf ("(Rank:0) tst_test_array[%d]:%s\n",
                            i, tst_test_getdescription(tst_test_array[i]));
                  }
              }
            break;
          }

        case 'c':
          {
            char * str;
            /*
             * At first, reset the tst_comm_array
             */
            for (i = 0; i < tst_comm_array_max; i++)
              tst_comm_array[i] = -1;
            num_comms = 0;

            str = strtok (optarg, ",");
            while (str)
              {
                /*
                 * In case we find the magic word all, we reset the list as above.
                 * In case we find a '!', deselect the communicator (communicators of a comm-class)
                 */
                if (!strncasecmp ("All", str, strlen("All")))
                  {
                    for (i = 0; i < tst_comm_array_max; i++)
                      tst_comm_array[i] = i;
                    num_comms = tst_comm_array_max;
                  }
                else if ('!' == str[0])
                  {
                    char tmp_str[TST_DESCRIPTION_LEN+1];

                    INTERNAL_CHECK (if (strlen(str) > TST_DESCRIPTION_LEN) ERROR (EINVAL, "Name of comm too long for negation"));

                    strncpy (tmp_str, &(str[1]), TST_DESCRIPTION_LEN);
                    tst_comm_deselect (tmp_str, tst_comm_array, tst_comm_array_max, &num_comms);
                  }
                else
                  tst_comm_select (str, tst_comm_array, tst_comm_array_max, &num_comms);
                str = strtok (NULL, ",");
              }
            tst_array_compress (tst_comm_array, tst_comm_array_max, &num_comms);
            break;
          }
        case 'd':
          {
            char * str;
            /*
             * At first, reset the tst_comm_array
             */
            for (i = 0; i < tst_type_array_max; i++)
              tst_type_array[i] = -1;
            num_types = 0;

            str = strtok (optarg, ",");
            while (str)
              {
                /*
                 * In case we find the magic word all, we reset the list as above.
                 * In case we find a '!', deselect the type (types of a type-class)
                 */
                if (!strncasecmp ("All", str, strlen("All")))
                  {
                    for (i = 0; i < tst_type_array_max; i++)
                      tst_type_array[i] = i;
                    num_types = tst_type_array_max;
                  }
                else if ('!' == str[0])
                  {
                    char tmp_str[TST_DESCRIPTION_LEN+1];

                    INTERNAL_CHECK (if (strlen(str) > TST_DESCRIPTION_LEN) ERROR (EINVAL, "Name of type too long for negation"));

                    strncpy (tmp_str, &(str[1]), TST_DESCRIPTION_LEN);
                    tst_type_deselect (tmp_str, tst_type_array, tst_type_array_max, &num_types);
                  }
                else
                  tst_type_select (str, tst_type_array, tst_type_array_max, &num_types);
                str = strtok (NULL, ",");
              }
            tst_array_compress (tst_type_array, tst_type_array_max, &num_types);
            break;
          }
        case 'n':
          {
            char * str;
            memset (tst_value_array, 0, sizeof (int)* tst_value_array_max);
            num_values = 0;
            str = strtok (optarg, ",");
            while (str)
              {
                tst_value_array[num_values++] = atoi (str);

                if (num_values >= tst_value_array_max)
                  ERROR (EINVAL, "Too many values specified");

                str = strtok (NULL, ",");
              }
            break;
          }
        case 'l':
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
          break;
        case 'r':
          for (tst_report=TST_REPORT_SUMMARY; tst_report < TST_REPORT_MAX; tst_report++)
            if (0 == strcasecmp (optarg, tst_reports[tst_report]))
              break;
          if (tst_report == TST_REPORT_MAX)
            {
              printf ("Unknown report type selected:%s\n",
                      optarg);
              usage ();
            }
          break;
        case 'x':
          for (tst_mode = TST_MODE_DISABLED; tst_mode < TST_MODE_MAX; tst_mode++)
            if (0 == strcasecmp (optarg, tst_modes[tst_mode]))
              break;
          if (tst_mode == TST_MODE_MAX)
            {
              printf ("Unknown test mode selected:%s\n",
                      optarg);
              usage ();
            }
          break;
        case 'j':
#ifdef HAVE_MPI2_THREADS
          if (tst_thread_level_provided != MPI_THREAD_MULTIPLE)
            printf ("Threads are not enabled by the MPI-Implementation\n");
          else
            num_threads = atoi (optarg);
#else
          printf ("Threads are not enabled by configure\n");
#endif
          break;
        case 'v':
          tst_verbose = 1;
          break;
        case '?':
        case 'h':
          if (!tst_global_rank)
            usage();
          break;
        default:
          if (!tst_global_rank)
            {
              printf ("UNKNOWN flag c:%c\n", c);
              usage ();
            }
        }
    }

#ifdef HAVE_MPI2_THREADS
  tst_thread_init (num_threads, &tst_thread_env);
#endif

  /*
   * For every test included in the tst_*_array, check if runnable and run!
   */
  tst_output_printf (DEBUG_LOG, TST_REPORT_FULL, "num_tests:%d num_comms:%d num_types:%d\n",
                     num_tests, num_comms, num_types);

#if 1
  for (i = 0; i < num_tests; i++)
    for (j = 0; j < num_comms; j++)
      for (k = 0; k < num_types; k++)
        for (l = 0; l < num_values; l++)
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
                      tst_test_getclass (tst_env.test),
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

#else

            /*
            * Before setting the mandatory first fields needed, reset.
            * Every test should clean up after itself.
            */

            if (num_tests != 1 || num_comms != 2)
              printf ("(Rank:%d) Error expected num_tests:%d to be 1 and num_comms:%d to be 2\n",
                      tst_global_rank, num_tests, num_comms);


            i = 0; /* test */
            j = 0; /* communicator */
            k = 0; /* type */
            l = 0; /* values_num */
            memset (&tst_env, 0, sizeof (tst_env));
            tst_env.test        = tst_test_array[i];
            tst_env.values_num  = tst_value_array[l];
            tst_env.type        = tst_type_array[k];
            tst_env.tag         = (i+j+k+l) % tst_tag_ub;
            tst_env.comm        = tst_comm_array[j];

            /*
             * Assign the first test
             */
            tst_thread_assign_reset (tst_thread_env);
            tst_thread_assign_one (&tst_env, 0, tst_thread_env);

            if (tst_global_rank == 0 && tst_report > TST_REPORT_SUMMARY)
              printf ("%s tests %s (%d/%d), comm %s (%d/%d), type %s (%d/%d) AND ",
                      tst_test_getclass (tst_test_array[i]),
                      tst_test_getdescription (tst_test_array[i]), tst_test_array[i+1], num_tests,
                      tst_comm_getdescription (tst_comm_array[j]), tst_comm_array[j+1], num_comms,
                      tst_type_getdescription (tst_type_array[k]), tst_type_array[k+1], num_types);

            i = 0; /* test */
            j = 1; /* communicator */
            k = 0; /* type */
            l = 0; /* values_num */
            memset (&tst_env, 0, sizeof (tst_env));
            tst_env.test        = tst_test_array[i];
            tst_env.values_num  = tst_value_array[l];
            tst_env.type        = tst_type_array[k];
            tst_env.tag         = (i+j+k+l) % tst_tag_ub;
            tst_env.comm        = tst_comm_array[j];

            /*
             * Assign the second test
             */
            tst_thread_assign_one (&tst_env, 1, tst_thread_env);

            if (tst_global_rank == 0 && tst_report > TST_REPORT_SUMMARY)
              printf (" on Thread 2: %s tests %s (%d/%d), comm %s (%d/%d), type %s (%d/%d)\n",
                      tst_test_getclass (tst_test_array[i]),
                      tst_test_getdescription (tst_test_array[i]), tst_test_array[i+1], num_tests,
                      tst_comm_getdescription (tst_comm_array[j]), tst_comm_array[j+1], num_comms,
                      tst_type_getdescription (tst_type_array[k]), tst_type_array[k+1], num_types);

#ifdef HAVE_MPI2_THREADS
            tst_thread_execute_init (&tst_env);
            tst_thread_execute_run (&tst_env);
            tst_thread_execute_cleanup (&tst_env);
#endif /* HAVE_MPI2_THREADS */
#endif /* 1 */
  if (tst_global_rank == 0)
    tst_test_print_failed ();

/*
 * XXX Disable for Thread Checker test, as we free twice???
 */
/*
  tst_comm_cleanup ();
  tst_type_cleanup ();
  tst_test_cleanup ();
  tst_profiling_cleanup ();
  free (tst_comm_array);
  free (tst_type_array);
  free (tst_test_array);
*/

  tst_output_printf (DEBUG_LOG, TST_REPORT_FULL, "(Rank:%d) Going to MPI_Finalize\n",
                     tst_global_rank);

  tst_output_close (DEBUG_LOG);

  MPI_Finalize ();

  return 0;
}
