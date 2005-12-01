#include <stdio.h>
#include <unistd.h>
#ifdef HAVE_GETOPT_H
# include <getopt.h>
#endif
#include "mpi.h"
#include "mpi_test_suite.h"

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

int tst_tag_ub = 32766;


/*
 * Global functions, which don't fit into another category.
 */
int tst_hash_value (const struct tst_env * env)
{
  return (env->comm * 65521 + /* Smallest prime smaller than 2^16 */
          env->type * 32749  + /* Smallest prime smaller than 2^16 */
          env->test) % tst_tag_ub;
}

static int usage (void)
{
  fprintf (stderr, "Usage: mpi_test_suite [-t test] [-c comm] [-d datatype] [-n num_values] [-r report] [-v] [-l] [-h]\n"
           "test:\t\tone (or more) tests or test-classes (see -l) and\n"
           "comm:\t\tone (or more) communicator or communicator-classes (see -l)\n"
           "datatype:\tone (or more) datatype or datatype-classes (see -l)\n"
           "num_values:\thow many elements to communicate (default:%d)\n"
           "report:\t\tlevel of detail for tests being run, see -l (default:SUMMARY)\n"
           "\n"
           "All multiple test/comm/datatype-declarations must be separated by commas\n"
           "The option -l/--list lists all available tests, communicators and datatypes\n"
           "and corresponding classes.\n"
           "\n"
           "The option -v, verbose mode is turned on\n"
           "The option -h shows this help\n",
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
  int flag;
  int tst_global_size;
  int num_comms;
  int num_types;
  int num_tests;
  int num_values = NUM_VALUES;
  struct tst_env tst_env;
  int * tst_test_array;
  int tst_test_array_max;
  int * tst_comm_array;
  int tst_comm_array_max;
  int * tst_type_array;
  int tst_type_array_max;
  int * val;


  MPI_Init (&argc, &argv);
  MPI_Comm_rank (MPI_COMM_WORLD, &tst_global_rank);
  MPI_Comm_size (MPI_COMM_WORLD, &tst_global_size);

  if (NULL != getenv ("MPI_TEST_SUITE_SLEEP"))
    {
      printf ("(Rank:%d) pid:%ld Going to sleep\n", tst_global_rank, getpid());
      sleep (30);
    }

  MPI_CHECK (MPI_Attr_get (MPI_COMM_WORLD, MPI_TAG_UB, &val, &flag));
  if (!flag)
    ERROR (EINVAL, "Couldn't retrieve MPI_TAG_UB attribute");

  tst_tag_ub = *val;

  DEBUG (printf ("(Rank:%d) MPI_TAG_UB:%d\n",
                 tst_global_rank, tst_tag_ub));

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

      c = getopt (argc, argv, "t:c:d:n:r:x:lvh");

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
            memset (tst_test_array, 0, sizeof (int)*num_tests);
            num_tests = 0;

            str = strtok (optarg, ",");
            while (str)
              {
                tst_test_select (str, tst_test_array, &num_tests, tst_test_array_max);
                str = strtok (NULL, ",");
              }
            break;
          }

        case 'c':
          {
            char * str;
            /*
             * At first, reset the tst_comm_array
             */
            memset (tst_comm_array, 0, sizeof (int)*num_comms);
            num_comms = 0;

            str = strtok (optarg, ",");
            while (str)
              {
                tst_comm_select (str, tst_comm_array, &num_comms, tst_comm_array_max);
                str = strtok (NULL, ",");
              }

            break;
          }
        case 'd':
          {
            char * str;
            /*
             * At first, reset the tst_comm_array
             */
            memset (tst_type_array, 0, sizeof (int)*num_types);
            num_types = 0;

            str = strtok (optarg, ",");
            while (str)
              {
                tst_type_select (str, tst_type_array, &num_types, tst_type_array_max);
                str = strtok (NULL, ",");
              }
            break;
          }
        case 'n':
          num_values = atoi (optarg);
          break;
        case 'l':
          if (!tst_global_rank)
            {
              tst_test_list ();
              tst_comm_list ();
              tst_type_list ();
              for (i = 0; i < TST_REPORT_MAX; i++)
                printf ("Report:%d %s\n", i, tst_reports[i]);
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

  /*
   * For every test included in the tst_*_array, check if runnable and run!
   */
  DEBUG (printf ("num_tests:%d num_comms:%d num_types:%d\n",
                 num_tests, num_comms, num_types));
  for (i = 0; i < num_tests; i++)
    for (j = 0; j < num_comms; j++)
      for (k = 0; k < num_types; k++)
        {
          tst_env.comm = tst_comm_array[j];
          tst_env.type = tst_type_array[k];
          tst_env.test = tst_test_array[i];
          tst_env.values_num = num_values;

          if (!tst_test_check_run (&tst_env))
            {
              DEBUG(printf ("Not running tst_env.test:%d\n", tst_env.test));
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
          tst_test_init_func (&tst_env);
          tst_test_run_func (&tst_env);
          tst_test_cleanup_func (&tst_env);
          if (tst_test_check_sync (&tst_env))
            MPI_Barrier (MPI_COMM_WORLD);
        }
  if (tst_global_rank == 0 && tst_report == TST_REPORT_SUMMARY)
    tst_test_print_failed ();
  MPI_Finalize ();
  return 0;
}
