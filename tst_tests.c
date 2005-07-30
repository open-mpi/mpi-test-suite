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
  tst_int64 run_with_type;
  int needs_sync;
  int (*tst_init_func) (const struct tst_env * env);
  int (*tst_run_func) (const struct tst_env * env);
  int (*tst_cleanup_func) (const struct tst_env * env);
};

static struct tst_test tst_tests[] = {
  /*
   * Here come the P2P-tests
   */
  {TST_CLASS_P2P, "Ring",
   TST_MPI_COMM_SELF | TST_MPI_INTRA_COMM,
   TST_MPI_ALL_C_TYPES,
   TST_SYNC,
   &tst_p2p_simple_ring_init, &tst_p2p_simple_ring_run, &tst_p2p_simple_ring_cleanup},

  {TST_CLASS_P2P, "Ring Isend",
   TST_MPI_COMM_SELF | TST_MPI_INTRA_COMM,
   TST_MPI_ALL_C_TYPES,
   TST_SYNC,
   &tst_p2p_simple_ring_isend_init, &tst_p2p_simple_ring_isend_run, &tst_p2p_simple_ring_isend_cleanup},

  {TST_CLASS_P2P, "Ring Bsend",
   TST_MPI_INTRA_COMM,
   TST_MPI_ALL_C_TYPES,
   TST_NONE,
   &tst_p2p_simple_ring_bsend_init, &tst_p2p_simple_ring_bsend_run, &tst_p2p_simple_ring_bsend_cleanup},

  {TST_CLASS_P2P, "Ring Ssend",
   TST_MPI_INTRA_COMM,
   TST_MPI_ALL_C_TYPES,
   TST_NONE,
   &tst_p2p_simple_ring_ssend_init, &tst_p2p_simple_ring_ssend_run, &tst_p2p_simple_ring_ssend_cleanup},

  {TST_CLASS_P2P, "Direct Partner Intercomm",
   TST_MPI_INTER_COMM,
   TST_MPI_ALL_C_TYPES,
   TST_SYNC,
   &tst_p2p_direct_partner_intercomm_init,
   &tst_p2p_direct_partner_intercomm_run,
   &tst_p2p_direct_partner_intercomm_cleanup},

  {TST_CLASS_P2P, "Many-to-one",
   TST_MPI_INTRA_COMM | TST_MPI_INTER_COMM,
   TST_MPI_ALL_C_TYPES,
   TST_NONE,            /* No synchronization needed, done with hash */
   &tst_p2p_many_to_one_init, &tst_p2p_many_to_one_run, &tst_p2p_many_to_one_cleanup},

  {TST_CLASS_P2P, "Many-to-one with MPI_Probe (MPI_ANY_SOURCE)",
   TST_MPI_INTRA_COMM | TST_MPI_INTER_COMM,
   TST_MPI_ALL_C_TYPES,
   TST_SYNC,            /* No synchronization needed, done with hash */
   &tst_p2p_many_to_one_probe_anysource_init, &tst_p2p_many_to_one_probe_anysource_run, &tst_p2p_many_to_one_probe_anysource_cleanup},

  {TST_CLASS_P2P, "Many-to-one with MPI_Iprobe (MPI_ANY_SOURCE)",
   TST_MPI_INTRA_COMM | TST_MPI_INTER_COMM,
   TST_MPI_ALL_C_TYPES,
   TST_SYNC,            /* Receiving with MPI_ANY_SOURCE and MPI_ANY_TAG */
   &tst_p2p_many_to_one_iprobe_anysource_init, &tst_p2p_many_to_one_iprobe_anysource_run, &tst_p2p_many_to_one_iprobe_anysource_cleanup},

  {TST_CLASS_P2P, "Alltoall",
   TST_MPI_INTRA_COMM | TST_MPI_INTER_COMM,
   TST_MPI_ALL_C_TYPES,
   TST_NONE,            /* No synchronization needed, done with hash */
   &tst_p2p_alltoall_init, &tst_p2p_alltoall_run, &tst_p2p_alltoall_cleanup},

  {TST_CLASS_P2P, "Alltoall - Irsend",
   TST_MPI_INTRA_COMM /* | TST_MPI_INTER_COMM */,
   TST_MPI_ALL_C_TYPES,
   TST_NONE,            /* No synchronization needed */
   &tst_p2p_alltoall_irsend_init, &tst_p2p_alltoall_irsend_run, &tst_p2p_alltoall_irsend_cleanup},

  {TST_CLASS_P2P, "Alltoall - Issend",
   TST_MPI_INTRA_COMM | TST_MPI_INTER_COMM,
   TST_MPI_ALL_C_TYPES,
   TST_NONE,            /* No synchronization needed -- everyone waits */
   &tst_p2p_alltoall_issend_init, &tst_p2p_alltoall_issend_run, &tst_p2p_alltoall_issend_cleanup},

  {TST_CLASS_P2P, "Alltoall with MPI_Probe (MPI_ANY_SOURCE)",
   TST_MPI_INTRA_COMM,
   TST_MPI_ALL_C_TYPES,
   TST_SYNC,            /* Probing for MPI_ANY_SOURCE and MPI_ANY_TAG */
   &tst_p2p_alltoall_probe_anysource_init, &tst_p2p_alltoall_probe_anysource_run, &tst_p2p_alltoall_probe_anysource_cleanup},

  /*
   * Here come the collective tests
   *
   * XXX should allow TST_MPI_INTER_COMM depending on whether the underlying
   * MPI supports it!
   */
  {TST_CLASS_COLL, "Bcast",
   TST_MPI_COMM_SELF | TST_MPI_INTRA_COMM /* | TST_MPI_INTER_COMM */,
   TST_MPI_ALL_C_TYPES,
   TST_NONE,            /* No synchronization needed */
   &tst_coll_bcast_init, &tst_coll_bcast_run, &tst_coll_bcast_cleanup},

  /*
   * XXX should allow TST_MPI_INTER_COMM depending on whether the underlying
   * MPI supports it!
   */
  {TST_CLASS_COLL, "Gather",
   TST_MPI_COMM_SELF | TST_MPI_INTRA_COMM /* | TST_MPI_INTER_COMM */,
   TST_MPI_ALL_C_TYPES,
   TST_NONE,            /* No synchronization needed */
   &tst_coll_gather_init, &tst_coll_gather_run, &tst_coll_gather_cleanup},

  /*
   * XXX should allow TST_MPI_INTER_COMM depending on whether the underlying
   * MPI supports it!
   */
  {TST_CLASS_COLL, "Scatter",
   TST_MPI_COMM_SELF | TST_MPI_INTRA_COMM /* | TST_MPI_INTER_COMM */,
   TST_MPI_ALL_C_TYPES,
   TST_NONE,            /* No synchronization needed */
   &tst_coll_scatter_init, &tst_coll_scatter_run, &tst_coll_scatter_cleanup},

  /*
   * XXX should allow TST_MPI_INTER_COMM depending on whether the underlying
   * MPI supports it!
   */
  {TST_CLASS_COLL, "Scatterv",
   TST_MPI_COMM_SELF | TST_MPI_INTRA_COMM /* | TST_MPI_INTER_COMM */,
   TST_MPI_ALL_C_TYPES,
   TST_NONE,            /* No synchronization needed */
   &tst_coll_scatterv_init, &tst_coll_scatterv_run, &tst_coll_scatterv_cleanup},

  {TST_CLASS_COLL, "Scatterv_stride",
   TST_MPI_COMM_SELF | TST_MPI_INTRA_COMM /* | TST_MPI_INTER_COMM */,
   TST_MPI_ALL_C_TYPES,
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
   TST_NONE,            /* No synchronization needed */
   &tst_coll_allreduce_init, &tst_coll_allreduce_run, &tst_coll_allreduce_cleanup},

  /*
   * XXX should allow TST_MPI_INTER_COMM depending on whether the underlying
   * MPI supports it!
   */
  {TST_CLASS_COLL, "Alltoall",
   TST_MPI_COMM_SELF | TST_MPI_INTRA_COMM /* | TST_MPI_INTER_COMM */,
   TST_MPI_ALL_C_TYPES,
   TST_NONE,            /* No synchronization needed */
   &tst_coll_alltoall_init, &tst_coll_alltoall_run, &tst_coll_alltoall_cleanup}
};

int tst_test_init (int * num_tests)
{  
  *num_tests = TST_TESTS_NUM;
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
      (tst_comm_getcommclass (env->comm) & tst_tests[env->test].run_with_comm) == 0 ||
      (tst_type_gettypeclass (env->type) & tst_tests[env->test].run_with_type) == 0)
    {
      DEBUG (printf ("(Rank:%d) env->comm:%d getcommclass:%d test is run_with_comm:%d "
                     "gettypeclass:%d run_with_type:%d\n",
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
