#include "config.h"
#ifdef HAVE_STRING_H
#  include <string.h>
#endif
#ifdef HAVE_STRINGS_H
#  include <strings.h>
#endif
#include "mpi.h"
#include "mpi_test_suite.h"

#define CHECK_ARG(i,ret) do {         \
  if ((i) < 0 || (i) > TST_TESTS_NUM) \
    return (ret);                     \
} while (0)

#undef DEBUG
#define DEBUG(x)

#define TST_TESTS_NUM (sizeof (tst_tests) / sizeof (tst_tests[0]))
#define TST_TEST_CLASS_NUM (sizeof (tst_test_class_strings) / sizeof (tst_test_class_strings[0]))


static const char * const tst_test_class_strings [] =
  {
    "Environment",
    "P2P",
    "Collective"
  };

struct tst_test {
  int class;
  const char * description;
  int run_with_comm;
  tst_uint64 run_with_type;
  int mode;
  int needs_sync;
  int (*tst_init_func) (const struct tst_env * env);
  int (*tst_run_func) (const struct tst_env * env);
  int (*tst_cleanup_func) (const struct tst_env * env);
};

static struct tst_test tst_tests[] = {
  /*
   * Here come the ENV-tests
   */
  {TST_CLASS_ENV, "Status",
   TST_MPI_COMM_SELF,
   TST_MPI_CHAR,
   TST_MODE_STRICT,
   TST_SYNC,
   &tst_env_status_check_init, &tst_env_status_check_run, &tst_env_status_check_cleanup},

  {TST_CLASS_ENV, "Request_Null",
   TST_MPI_COMM_SELF,
   TST_MPI_CHAR,
   TST_MODE_STRICT,
   TST_SYNC,
   &tst_env_request_null_init, &tst_env_request_null_run, &tst_env_request_null_cleanup},

  /*
   * Here come the P2P-tests
   */
  {TST_CLASS_P2P, "Ring",
   TST_MPI_COMM_SELF | TST_MPI_INTRA_COMM,
   TST_MPI_ALL_C_TYPES,
   TST_MODE_RELAXED,
   TST_SYNC,
   &tst_p2p_simple_ring_init, &tst_p2p_simple_ring_run, &tst_p2p_simple_ring_cleanup},

  {TST_CLASS_P2P, "Ring Send Bottom",
   TST_MPI_COMM_SELF | TST_MPI_INTRA_COMM,
   TST_MPI_ALL_C_TYPES,
   TST_MODE_RELAXED,
   TST_SYNC,
   &tst_p2p_simple_ring_bottom_init, &tst_p2p_simple_ring_bottom_run, &tst_p2p_simple_ring_bottom_cleanup},

  {TST_CLASS_P2P, "Ring Send Pack",
   TST_MPI_COMM_SELF | TST_MPI_INTRA_COMM,
   TST_MPI_ALL_C_TYPES,
   TST_MODE_RELAXED,
   TST_SYNC,
   &tst_p2p_simple_ring_pack_init, &tst_p2p_simple_ring_pack_run, &tst_p2p_simple_ring_pack_cleanup},

  {TST_CLASS_P2P, "Ring Isend",
   TST_MPI_COMM_SELF | TST_MPI_INTRA_COMM,
   TST_MPI_ALL_C_TYPES,
   TST_MODE_RELAXED,
   TST_SYNC,
   &tst_p2p_simple_ring_isend_init, &tst_p2p_simple_ring_isend_run, &tst_p2p_simple_ring_isend_cleanup},

  {TST_CLASS_P2P, "Ring Bsend",
   TST_MPI_INTRA_COMM,
   TST_MPI_ALL_C_TYPES,
   TST_MODE_RELAXED,
   TST_NONE,
   &tst_p2p_simple_ring_bsend_init, &tst_p2p_simple_ring_bsend_run, &tst_p2p_simple_ring_bsend_cleanup},

  {TST_CLASS_P2P, "Ring Ssend",
   TST_MPI_INTRA_COMM,
   TST_MPI_ALL_C_TYPES,
   TST_MODE_RELAXED,
   TST_NONE,
   &tst_p2p_simple_ring_ssend_init, &tst_p2p_simple_ring_ssend_run, &tst_p2p_simple_ring_ssend_cleanup},

  {TST_CLASS_P2P, "Ring Sendrecv",
   TST_MPI_COMM_SELF | TST_MPI_INTRA_COMM,
   TST_MPI_ALL_C_TYPES,
   TST_MODE_RELAXED,
   TST_NONE,
   &tst_p2p_simple_ring_sendrecv_init, &tst_p2p_simple_ring_sendrecv_run, &tst_p2p_simple_ring_sendrecv_cleanup},

  {TST_CLASS_P2P, "Ring same value",
   TST_MPI_COMM_SELF | TST_MPI_INTRA_COMM,
   TST_MPI_ALL_C_TYPES,
   TST_MODE_RELAXED,
   TST_SYNC,
   &tst_p2p_simple_ring_xsend_init, &tst_p2p_simple_ring_xsend_run, &tst_p2p_simple_ring_xsend_cleanup},

  {TST_CLASS_P2P, "Direct Partner Intercomm",
   TST_MPI_INTER_COMM,
   TST_MPI_ALL_C_TYPES,
   TST_MODE_RELAXED,
   TST_SYNC,
   &tst_p2p_direct_partner_intercomm_init,
   &tst_p2p_direct_partner_intercomm_run,
   &tst_p2p_direct_partner_intercomm_cleanup},

  {TST_CLASS_P2P, "Many-to-one",
   TST_MPI_INTRA_COMM | TST_MPI_INTER_COMM,
   TST_MPI_ALL_C_TYPES,
   TST_MODE_RELAXED,
   TST_NONE,            /* No synchronization needed, done with hash */
   &tst_p2p_many_to_one_init, &tst_p2p_many_to_one_run, &tst_p2p_many_to_one_cleanup},

  {TST_CLASS_P2P, "Many-to-one with MPI_Probe (MPI_ANY_SOURCE)",
   TST_MPI_INTRA_COMM | TST_MPI_INTER_COMM,
   TST_MPI_ALL_C_TYPES,
   TST_MODE_RELAXED,
   TST_SYNC,            /* No synchronization needed, done with hash */
   &tst_p2p_many_to_one_probe_anysource_init, &tst_p2p_many_to_one_probe_anysource_run, &tst_p2p_many_to_one_probe_anysource_cleanup},

  {TST_CLASS_P2P, "Many-to-one with MPI_Iprobe (MPI_ANY_SOURCE)",
   TST_MPI_INTRA_COMM | TST_MPI_INTER_COMM,
   TST_MPI_ALL_C_TYPES,
   TST_MODE_RELAXED,
   TST_SYNC,            /* Receiving with MPI_ANY_SOURCE and MPI_ANY_TAG */
   &tst_p2p_many_to_one_iprobe_anysource_init, &tst_p2p_many_to_one_iprobe_anysource_run, &tst_p2p_many_to_one_iprobe_anysource_cleanup},

  {TST_CLASS_P2P, "Alltoall",
   TST_MPI_INTRA_COMM | TST_MPI_INTER_COMM,
   TST_MPI_ALL_C_TYPES,
   TST_MODE_RELAXED,
   TST_NONE,            /* No synchronization needed, done with hash */
   &tst_p2p_alltoall_init, &tst_p2p_alltoall_run, &tst_p2p_alltoall_cleanup},

  {TST_CLASS_P2P, "Alltoall - Persistent",
   TST_MPI_INTRA_COMM | TST_MPI_INTER_COMM,
   TST_MPI_ALL_C_TYPES,
   TST_MODE_RELAXED,
   TST_NONE,            /* No synchronization needed, done with hash */
   &tst_p2p_alltoall_persistent_init, &tst_p2p_alltoall_persistent_run, &tst_p2p_alltoall_persistent_cleanup},

  {TST_CLASS_P2P, "Alltoall - xIsend",
   TST_MPI_INTRA_COMM | TST_MPI_INTER_COMM,
   TST_MPI_ALL_C_TYPES,
   TST_MODE_RELAXED,
   TST_NONE,            /* No synchronization needed, done with hash */
   &tst_p2p_alltoall_xisend_init, &tst_p2p_alltoall_xisend_run, &tst_p2p_alltoall_xisend_cleanup},


  {TST_CLASS_P2P, "Alltoall - Irsend",
   TST_MPI_INTRA_COMM /* | TST_MPI_INTER_COMM */,
   TST_MPI_ALL_C_TYPES,
   TST_MODE_RELAXED,
   TST_NONE,            /* No synchronization needed */
   &tst_p2p_alltoall_irsend_init, &tst_p2p_alltoall_irsend_run, &tst_p2p_alltoall_irsend_cleanup},

  {TST_CLASS_P2P, "Alltoall - Issend",
   TST_MPI_INTRA_COMM | TST_MPI_INTER_COMM,
   TST_MPI_ALL_C_TYPES,
   TST_MODE_RELAXED,
   TST_NONE,            /* No synchronization needed -- everyone waits */
   &tst_p2p_alltoall_issend_init, &tst_p2p_alltoall_issend_run, &tst_p2p_alltoall_issend_cleanup},

  {TST_CLASS_P2P, "Alltoall with MPI_Probe (MPI_ANY_SOURCE)",
   TST_MPI_INTRA_COMM,
   TST_MPI_ALL_C_TYPES,
   TST_MODE_RELAXED,
   TST_SYNC,            /* Probing for MPI_ANY_SOURCE and MPI_ANY_TAG */
   &tst_p2p_alltoall_probe_anysource_init, &tst_p2p_alltoall_probe_anysource_run, &tst_p2p_alltoall_probe_anysource_cleanup},

  {TST_CLASS_P2P, "Ring Send with cart comm",
   TST_MPI_CART_COMM,
   TST_MPI_ALL_C_TYPES,
   TST_MODE_RELAXED,
   TST_SYNC,
   &tst_p2p_simple_ring_shift_init, &tst_p2p_simple_ring_shift_run, &tst_p2p_simple_ring_shift_cleanup},

  {TST_CLASS_P2P, "Alltoall on topo comm",
   TST_MPI_TOPO_COMM,
   TST_MPI_ALL_C_TYPES,
   TST_MODE_RELAXED,
   TST_SYNC,
   &tst_p2p_alltoall_graph_init, &tst_p2p_alltoall_graph_run, &tst_p2p_alltoall_graph_cleanup},


  /*
   * Here come the collective tests
   *
   * XXX should allow TST_MPI_INTER_COMM depending on whether the underlying
   * MPI supports it!
   */
  {TST_CLASS_COLL, "Bcast",
   TST_MPI_COMM_SELF | TST_MPI_INTRA_COMM /* | TST_MPI_INTER_COMM */,
   TST_MPI_ALL_C_TYPES,
   TST_MODE_RELAXED,
   TST_NONE,            /* No synchronization needed */
   &tst_coll_bcast_init, &tst_coll_bcast_run, &tst_coll_bcast_cleanup},

  /*
   * XXX should allow TST_MPI_INTER_COMM depending on whether the underlying
   * MPI supports it!
   */
  {TST_CLASS_COLL, "Gather",
   TST_MPI_COMM_SELF | TST_MPI_INTRA_COMM /* | TST_MPI_INTER_COMM */,
   TST_MPI_ALL_C_TYPES,
   TST_MODE_RELAXED,
   TST_NONE,            /* No synchronization needed */
   &tst_coll_gather_init, &tst_coll_gather_run, &tst_coll_gather_cleanup},

  /*
   * XXX should allow TST_MPI_INTER_COMM depending on whether the underlying
   * MPI supports it!
   */
  {TST_CLASS_COLL, "Allgather",
   TST_MPI_COMM_SELF | TST_MPI_INTRA_COMM /* | TST_MPI_INTER_COMM */,
   TST_MPI_ALL_C_TYPES,
   TST_MODE_RELAXED,
   TST_NONE,            /* No synchronization needed */
   &tst_coll_allgather_init, &tst_coll_allgather_run, &tst_coll_allgather_cleanup},

  /*
   * XXX should allow TST_MPI_INTER_COMM depending on whether the underlying
   * MPI supports it!
   */
  {TST_CLASS_COLL, "Scan sum",
   TST_MPI_COMM_SELF | TST_MPI_INTRA_COMM /* | TST_MPI_INTER_COMM */,
   (TST_MPI_STANDARD_C_INT_TYPES |
     TST_MPI_STANDARD_C_FLOAT_TYPES |
     TST_MPI_STANDARD_FORTRAN_COMPLEX_TYPES) &
   ~(TST_MPI_CHAR | TST_MPI_UNSIGNED_CHAR | TST_MPI_BYTE),
   TST_MODE_RELAXED,
   TST_SYNC,            /* No synchronization needed */
   &tst_coll_scan_sum_init, &tst_coll_scan_sum_run, &tst_coll_scan_sum_cleanup},

  /*
   * XXX should allow TST_MPI_INTER_COMM depending on whether the underlying
   * MPI supports it!
   */
  {TST_CLASS_COLL, "Scatter",
   TST_MPI_COMM_SELF | TST_MPI_INTRA_COMM /* | TST_MPI_INTER_COMM */,
   TST_MPI_ALL_C_TYPES,
   TST_MODE_RELAXED,
   TST_NONE,            /* No synchronization needed */
   &tst_coll_scatter_init, &tst_coll_scatter_run, &tst_coll_scatter_cleanup},

  /*
   * XXX should allow TST_MPI_INTER_COMM depending on whether the underlying
   * MPI supports it!
   */
  {TST_CLASS_COLL, "Scatterv",
   TST_MPI_COMM_SELF | TST_MPI_INTRA_COMM /* | TST_MPI_INTER_COMM */,
   TST_MPI_ALL_C_TYPES,
   TST_MODE_RELAXED,
   TST_NONE,            /* No synchronization needed */
   &tst_coll_scatterv_init, &tst_coll_scatterv_run, &tst_coll_scatterv_cleanup},

  /*
   * XXX should allow TST_MPI_INTER_COMM depending on whether the underlying
   * MPI supports it!
   */
  {TST_CLASS_COLL, "Scatterv_stride",
   TST_MPI_COMM_SELF | TST_MPI_INTRA_COMM /* | TST_MPI_INTER_COMM */,
   TST_MPI_ALL_C_TYPES,
   TST_MODE_RELAXED,
   TST_NONE,            /* No synchronization needed */
   &tst_coll_scatterv_stride_init, &tst_coll_scatterv_stride_run, &tst_coll_scatterv_stride_cleanup},

  /*
   * The Allreduce call is not valid for MPI_MIN/MPI_MAX and
   * the datatypes MPI_Char, MPI_UNSIGNED_CHAR and MPI_Byte and
   * the struct datatypes
   *
   * XXX should allow TST_MPI_INTER_COMM depending on whether the underlying
   * MPI supports it!
   */
  {TST_CLASS_COLL, "Allreduce MIN/MAX",
   TST_MPI_COMM_SELF | TST_MPI_INTRA_COMM /* | TST_MPI_INTER_COMM */,
   (TST_MPI_STANDARD_C_TYPES | TST_MPI_STANDARD_FORTRAN_INT_TYPES | TST_MPI_STANDARD_FORTRAN_FLOAT_TYPES) &
   ~(TST_MPI_CHAR | TST_MPI_UNSIGNED_CHAR | TST_MPI_BYTE),
   TST_MODE_RELAXED,
   TST_NONE,            /* No synchronization needed */
   &tst_coll_allreduce_init, &tst_coll_allreduce_run, &tst_coll_allreduce_cleanup},

  /*
   * XXX should allow TST_MPI_INTER_COMM depending on whether the underlying
   * MPI supports it!
   */
  {TST_CLASS_COLL, "Alltoall",
   TST_MPI_COMM_SELF | TST_MPI_INTRA_COMM /* | TST_MPI_INTER_COMM */,
   TST_MPI_ALL_C_TYPES,
   TST_MODE_RELAXED,
   TST_NONE,            /* No synchronization needed */
   &tst_coll_alltoall_init, &tst_coll_alltoall_run, &tst_coll_alltoall_cleanup}
};

static struct tst_env * tst_tests_failed = NULL;
static int tst_tests_failed_num = 0;


int tst_test_init (int * num_tests)
{
  *num_tests = TST_TESTS_NUM;

  if ((tst_tests_failed = malloc (sizeof (struct tst_env) * TST_TESTS_NUM_FAILED_MAX)) == NULL)
    ERROR (errno, "Could not allocate memory");

  return 0;
}


const char * tst_test_getclass (int i)
{
  CHECK_ARG (i, NULL);

  INTERNAL_CHECK
    (
     if (tst_tests[i].class != TST_CLASS_ENV &&
         tst_tests[i].class != TST_CLASS_P2P &&
         tst_tests[i].class != TST_CLASS_COLL)
     ERROR (EINVAL, "Class of test is unknown");
     );
  /*
  printf ("tst_test_getclass: i:%d class:%d ffs():%d string:%s\n",
          i, tst_tests[i].class, ffs(tst_tests[i].class)-1,
          tst_test_class_strings[ffs (tst_tests[i].class)-1]);
  */
  return tst_test_class_strings[ffs (tst_tests[i].class) - 1];
}

const char * tst_test_getdescription (int i)
{
  CHECK_ARG (i, NULL);

  return tst_tests[i].description;
}

int tst_test_getmode (int i)
{
  CHECK_ARG (i, -1);

  return tst_tests[i].mode;
}

int tst_test_init_func (struct tst_env * env)
{
  CHECK_ARG (env->test, -1);

  return tst_tests[env->test].tst_init_func (env);
}


int tst_test_run_func (struct tst_env * env)
{
  CHECK_ARG (env->test, -1);

  return tst_tests[env->test].tst_run_func (env);
}

int tst_test_cleanup_func (struct tst_env * env)
{
  CHECK_ARG (env->test, -1);

  return tst_tests[env->test].tst_cleanup_func (env);
}


int tst_test_check_run (struct tst_env * env)
{
  /*
   * Return 0 if this test shouldn't be run with the current communicator type
   */
  if (env->test < 0 ||
      env->test > TST_TESTS_NUM ||
      tst_test_getmode (env->test) < tst_mode ||
      (tst_comm_getcommclass (env->comm) & tst_tests[env->test].run_with_comm) == (tst_uint64)0 ||
      (tst_type_gettypeclass (env->type) & tst_tests[env->test].run_with_type) == (tst_uint64)0)
    {
      DEBUG (printf ("(Rank:%d) env->comm:%d getcommclass:%d test is run_with_comm:%d "
                     "gettypeclass:%lld run_with_type:%d\n",
                     tst_global_rank, env->comm, tst_comm_getcommclass (env->comm),
                     tst_tests[env->test].run_with_comm,
                     tst_type_gettypeclass (env->type), tst_tests[env->test].run_with_type));
      return 0;
    }
  else
    return 1;
}

int tst_test_check_sync (struct tst_env * env)
{
  CHECK_ARG (env->test, -1);
  return tst_tests[env->test].needs_sync;
}


void tst_test_list (void)
{
  int i;
  for (i = 0; i < TST_TESTS_NUM; i++)
    printf ("%s test:%d %s\n",
            tst_test_getclass (i), i, tst_tests[i].description);

  for (i = 0; i < TST_TEST_CLASS_NUM; i++)
    printf ("Test-Class:%d %s\n",
            i, tst_test_class_strings[i]);
}


static int tst_test_search (const int search_test, const int * test_list, const int test_list_num)
{
  int k;

  for (k = 0; k < test_list_num; k++)
    if (test_list[k] == search_test)
      break;
  return (k == test_list_num) ? 0 : 1;
}


int tst_test_select (const char * test_string, int * test_list, int * test_list_num, const int test_list_max)
{
  int i;

  if (test_string == NULL || test_list == NULL || test_list_num == NULL)
    ERROR (EINVAL, "Passed a NULL parameter");

  for (i = 0; i < TST_TEST_CLASS_NUM; i++)
    {
      /*
       * In case we match a complete class of tests, include every one!
       */
      if (!strcasecmp (test_string, tst_test_class_strings[i]))
        {
          int j;
          DEBUG (printf ("test_string:%s matched with tst_test_class_strings[%d]:%s\n",
                         test_string, i, tst_test_class_strings[i]));
          for (j = 0; j < TST_TESTS_NUM; j++)
            {
              /*
               * First search for this test in the test_list -- if already in, continue!
               */
              if (tst_test_search (j, test_list, *test_list_num))
                {
                  WARNING (printf ("Test:%s selected through class:%s was already "
                                   "included in list -- not including\n",
                                   tst_tests[j].description,
                                   tst_test_class_strings[i]));
                  continue;
                }
              if (tst_tests[j].class & (1 << i))
                {
                  DEBUG (printf ("test_string:%s test j:%d i:%d with class:%d matches, test_list_num:%d\n",
                                 test_string, j, (1 << i), tst_tests[j].class, *test_list_num));
                  test_list[*test_list_num] = j;
                  (*test_list_num)++;
                  if (*test_list_num == test_list_max)
                    ERROR (EINVAL, "Too many user selected tests");
                }
            }
          return 0;
        }
    }

  /*
   * In case we didn't match a complete class of tests, test for every single one...
   */
  for (i = 0; i < TST_TESTS_NUM; i++)
    {
      if (!strcasecmp (test_string, tst_tests[i].description))
        {
          if (tst_test_search (i, test_list, *test_list_num))
            {
              WARNING (printf ("Test:%s was already included in list -- not including\n",
                               tst_tests[i].description));
              return 0;
            }

          test_list[*test_list_num] = i;
          (*test_list_num)++;
          if (*test_list_num == test_list_max)
            ERROR (EINVAL, "Too many user selected tests");

          DEBUG (printf ("test_string:%s matched with test_list_num:%d\n",
                         test_string, *test_list_num));

          return 0;
        }
    }

  {
    char buffer[128];
    sprintf (buffer, "Test %s not recognized",
             test_string);
    ERROR (EINVAL, buffer);
  }
  return 0;
}

int tst_test_recordfailure (const struct tst_env * env)
{
  int i;
  /*
   * First make sure, that this combination is not already
   * in the failed-list
   */
/*
  for (i = 0; i < tst_tests_failed_num; i++)
    if (tst_tests_failed[i].comm == env->comm &&
        tst_tests_failed[i].type == env->type &&
        tst_tests_failed[i].test == env->test &&
        tst_tests_failed[i].values_num == env->values_num)
      break;
  if (i == tst_tests_failed_num)
    {
      if (tst_report >= TST_REPORT_FULL)
        printf ("ERROR test:%s (%d), comm %s (%d), type %s (%d)\n",
                tst_test_getdescription (env->test), env->test+1,
                tst_comm_getdescription (env->comm), env->comm+1,
                tst_type_getdescription (env->type), env->type+1);

      tst_tests_failed[tst_tests_failed_num].comm = env->comm;
      tst_tests_failed[tst_tests_failed_num].type = env->type;
      tst_tests_failed[tst_tests_failed_num].test = env->test;
      tst_tests_failed[tst_tests_failed_num].values_num= env->values_num;
      tst_tests_failed_num++;

      if (tst_tests_failed_num == TST_TESTS_NUM_FAILED_MAX)
        ERROR (EINVAL, "Maximum Error limit reached");
    }
*/
  return 0;
}

int tst_test_print_failed (void)
{
  int i;
  printf ("Number of failed tests:%d summary of failed tests:\n",
          tst_tests_failed_num);
  for (i = 0; i < tst_tests_failed_num; i++)
    {
      const int test = tst_tests_failed[i].test;
      const int comm = tst_tests_failed[i].comm;
      const int type = tst_tests_failed[i].type;
      const int values_num= tst_tests_failed[i].values_num;

      printf ("ERROR test:%s (%d), comm %s (%d), type %s (%d) num of values:%d\n",
              tst_test_getdescription (test), test+1,
              tst_comm_getdescription (comm), comm+1,
              tst_type_getdescription (type), type+1, values_num);
    }
  return 0;
}

int tst_test_checkstandardarray (const struct tst_env * env, char * buffer, int comm_rank)
{
  int ret;
  ret = tst_type_checkstandardarray (env->type, env->values_num, buffer, comm_rank);
  if (0 != ret)
    tst_test_recordfailure (env);
  return ret;
}
