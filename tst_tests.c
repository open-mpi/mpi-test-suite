/*
 * Copyright (c) 2009 Cisco Systems, Inc.  All rights reserved.
 */

#include "config.h"
#ifdef HAVE_STRING_H
#  include <string.h>
#endif
#ifdef HAVE_STRINGS_H
#  include <strings.h>
#endif
#include <mpi.h>
#include "mpi_test_suite.h"

#define CHECK_ARG(i,ret) do {         \
  if ((i) < 0 || (i) > TST_TESTS_NUM) \
    return (ret);                     \
} while (0)


/*
 * Do not count the last test with UNSPEC-Class
 */
#define TST_TESTS_NUM (int)(sizeof (tst_tests) / sizeof (tst_tests[0]) -1)
#define TST_TEST_CLASS_NUM (int)(sizeof (tst_test_class_strings) / sizeof (tst_test_class_strings[0]))

/*
 * This string has to be kept up-to-date with the internal representations for
 * the test classes in mpi_test_suite.h
 * Also edit the internal check in tst_get_class below!
 */
static const char * const tst_test_class_strings [] =
  {
    "Unspecified",
    "Environment",
    "P2P",
    "Collective",
    "One-sided",
    "Dynamic",
    "IO",
    "Threaded"
  };

struct tst_test {
  int class;
  const char * description;
  int run_with_comm;
  int min_comm_size;
  tst_uint64 run_with_type;
  int mode;
  int needs_sync;
  int (*tst_init_func) (struct tst_env * env);
  int (*tst_run_func) (struct tst_env * env);
  int (*tst_cleanup_func) (struct tst_env * env);
};

static struct tst_test tst_tests[] = {
  /*
   * Here come the ENV-tests
   */
/* 0 */
  {TST_CLASS_ENV, "Status",
   TST_MPI_COMM_SELF,
   1,
   TST_MPI_CHAR,
   TST_MODE_STRICT,
   TST_SYNC,
   &tst_env_status_check_init, &tst_env_status_check_run, &tst_env_status_check_cleanup},


  {TST_CLASS_ENV, "Request_Null",
   TST_MPI_COMM_SELF,
   1,
   TST_MPI_CHAR,
   TST_MODE_STRICT,
   TST_SYNC,
   &tst_env_request_null_init, &tst_env_request_null_run, &tst_env_request_null_cleanup},

  {TST_CLASS_ENV, "Type_dup",
   TST_MPI_COMM_SELF,
   1,
   TST_MPI_ALL_C_TYPES,
   TST_MODE_RELAXED,
   TST_SYNC,
   &tst_env_type_dup_init, &tst_env_type_dup_run, &tst_env_type_dup_cleanup},

  {TST_CLASS_ENV, "Get_version",
   TST_MPI_COMM_SELF,
   1,
   TST_MPI_ALL_C_TYPES,
   TST_MODE_RELAXED,
   TST_SYNC,
   &tst_env_get_version_init, &tst_env_get_version_run, &tst_env_get_version_cleanup},


  /*
   * Here come the P2P-tests
   */
  {TST_CLASS_P2P, "Ring",
   TST_MPI_COMM_SELF | TST_MPI_INTRA_COMM,
   1,
   TST_MPI_ALL_C_TYPES,
   TST_MODE_RELAXED,
   TST_SYNC,
   &tst_p2p_simple_ring_init, &tst_p2p_simple_ring_run, &tst_p2p_simple_ring_cleanup},


  {TST_CLASS_P2P, "Ring Send Bottom",
   TST_MPI_COMM_SELF | TST_MPI_INTRA_COMM,
   1,
   TST_MPI_ALL_C_TYPES,
   TST_MODE_RELAXED,
   TST_SYNC,
   &tst_p2p_simple_ring_bottom_init, &tst_p2p_simple_ring_bottom_run, &tst_p2p_simple_ring_bottom_cleanup},


  {TST_CLASS_P2P, "Ring Send Pack",
   TST_MPI_COMM_SELF | TST_MPI_INTRA_COMM,
   1,
   TST_MPI_ALL_C_TYPES,
   TST_MODE_RELAXED,
   TST_SYNC,
   &tst_p2p_simple_ring_pack_init, &tst_p2p_simple_ring_pack_run, &tst_p2p_simple_ring_pack_cleanup},


  {TST_CLASS_P2P, "Ring Isend",
   TST_MPI_COMM_SELF | TST_MPI_INTRA_COMM,
   1,
   TST_MPI_ALL_C_TYPES,
   TST_MODE_RELAXED,
   TST_SYNC,
   &tst_p2p_simple_ring_isend_init, &tst_p2p_simple_ring_isend_run, &tst_p2p_simple_ring_isend_cleanup},


  {TST_CLASS_P2P, "Ring Ibsend",
   TST_MPI_COMM_SELF | TST_MPI_INTRA_COMM,
   1,
   TST_MPI_ALL_C_TYPES,
   TST_MODE_RELAXED,
   TST_SYNC,
   &tst_p2p_simple_ring_ibsend_init, &tst_p2p_simple_ring_ibsend_run, &tst_p2p_simple_ring_ibsend_cleanup},


  {TST_CLASS_P2P, "Ring Irsend",
   TST_MPI_COMM_SELF | TST_MPI_INTRA_COMM,
   1,
   TST_MPI_ALL_C_TYPES,
   TST_MODE_RELAXED,
   TST_SYNC,
   &tst_p2p_simple_ring_irsend_init, &tst_p2p_simple_ring_irsend_run, &tst_p2p_simple_ring_irsend_cleanup},


  {TST_CLASS_P2P, "Ring Issend",
   TST_MPI_COMM_SELF | TST_MPI_INTRA_COMM,
   1,
   TST_MPI_ALL_C_TYPES,
   TST_MODE_RELAXED,
   TST_SYNC,
   &tst_p2p_simple_ring_issend_init, &tst_p2p_simple_ring_issend_run, &tst_p2p_simple_ring_issend_cleanup},


  {TST_CLASS_P2P, "Ring Bsend",
   TST_MPI_COMM_SELF | TST_MPI_INTRA_COMM,
   1,
   TST_MPI_ALL_C_TYPES,
   TST_MODE_RELAXED,
   TST_NONE,
   &tst_p2p_simple_ring_bsend_init, &tst_p2p_simple_ring_bsend_run, &tst_p2p_simple_ring_bsend_cleanup},


  {TST_CLASS_P2P, "Ring Rsend",
   TST_MPI_COMM_SELF | TST_MPI_INTRA_COMM,
   1,
   TST_MPI_ALL_C_TYPES,
   TST_MODE_RELAXED,
   TST_SYNC,
   &tst_p2p_simple_ring_rsend_init, &tst_p2p_simple_ring_rsend_run, &tst_p2p_simple_ring_rsend_cleanup},


  {TST_CLASS_P2P, "Ring Ssend",
   TST_MPI_COMM_SELF | TST_MPI_INTRA_COMM,
   1,
   TST_MPI_ALL_C_TYPES,
   TST_MODE_RELAXED,
   TST_NONE,
   &tst_p2p_simple_ring_ssend_init, &tst_p2p_simple_ring_ssend_run, &tst_p2p_simple_ring_ssend_cleanup},


  {TST_CLASS_P2P, "Ring Sendrecv",
   TST_MPI_COMM_SELF | TST_MPI_INTRA_COMM,
   1,
   TST_MPI_ALL_C_TYPES,
   TST_MODE_RELAXED,
   TST_NONE,
   &tst_p2p_simple_ring_sendrecv_init, &tst_p2p_simple_ring_sendrecv_run, &tst_p2p_simple_ring_sendrecv_cleanup},


  {TST_CLASS_P2P, "Ring same value",
   TST_MPI_COMM_SELF | TST_MPI_INTRA_COMM,
   1,
   TST_MPI_ALL_C_TYPES,
   TST_MODE_RELAXED,
   TST_SYNC,
   &tst_p2p_simple_ring_xsend_init, &tst_p2p_simple_ring_xsend_run, &tst_p2p_simple_ring_xsend_cleanup},


  {TST_CLASS_P2P, "Ring Persistent",
   TST_MPI_COMM_SELF | TST_MPI_INTRA_COMM,
   1,
   TST_MPI_ALL_C_TYPES,
   TST_MODE_RELAXED,
   TST_SYNC,
   &tst_p2p_simple_ring_persistent_init, &tst_p2p_simple_ring_persistent_run, &tst_p2p_simple_ring_persistent_cleanup},


  {TST_CLASS_P2P, "Direct Partner Intercomm",
   TST_MPI_INTER_COMM,
   1,
   TST_MPI_ALL_C_TYPES,
   TST_MODE_RELAXED,
   TST_SYNC,
   &tst_p2p_direct_partner_intercomm_init,
   &tst_p2p_direct_partner_intercomm_run,
   &tst_p2p_direct_partner_intercomm_cleanup},


  {TST_CLASS_P2P, "Many-to-one",
   TST_MPI_INTRA_COMM | TST_MPI_INTER_COMM,
   1,
   TST_MPI_ALL_C_TYPES,
   TST_MODE_RELAXED,
   TST_NONE,            /* No synchronization needed, done with hash */
   &tst_p2p_many_to_one_init, &tst_p2p_many_to_one_run, &tst_p2p_many_to_one_cleanup},


  {TST_CLASS_P2P, "Many-to-one with MPI_Probe (MPI_ANY_SOURCE)",
   TST_MPI_INTRA_COMM | TST_MPI_INTER_COMM,
   1,
   TST_MPI_ALL_C_TYPES,
   TST_MODE_RELAXED,
   TST_SYNC,            /* No synchronization needed, done with hash */
   &tst_p2p_many_to_one_probe_anysource_init, &tst_p2p_many_to_one_probe_anysource_run, &tst_p2p_many_to_one_probe_anysource_cleanup},


  {TST_CLASS_P2P, "Many-to-one with MPI_Iprobe (MPI_ANY_SOURCE)",
   TST_MPI_INTRA_COMM | TST_MPI_INTER_COMM,
   1,
   TST_MPI_ALL_C_TYPES,
   TST_MODE_RELAXED,
   TST_SYNC,            /* Receiving with MPI_ANY_SOURCE and MPI_ANY_TAG */
   &tst_p2p_many_to_one_iprobe_anysource_init, &tst_p2p_many_to_one_iprobe_anysource_run, &tst_p2p_many_to_one_iprobe_anysource_cleanup},


   /*
    * LAM core-dumps with sig-segv when running this test with
    * "MPI_COMM_WORLD,Reversed MPI_COMM_WORLD"
    */
#ifndef HAVE_MPI_LAM
  {TST_CLASS_P2P, "Many-to-one with Isend and Cancellation",
   TST_MPI_INTRA_COMM, /* XXX We use a MPI_Gather insided the tests -- no intercomm allowed! | TST_MPI_INTER_COMM,*/
   1,
   TST_MPI_ALL_C_TYPES,
   TST_MODE_RELAXED,
   TST_SYNC,            /* Receiving with MPI_ANY_SOURCE and MPI_ANY_TAG */
   &tst_p2p_many_to_one_isend_cancel_init, &tst_p2p_many_to_one_isend_cancel_run, &tst_p2p_many_to_one_isend_cancel_cleanup},
#endif


  {TST_CLASS_P2P, "Alltoall",
   TST_MPI_INTRA_COMM | TST_MPI_INTER_COMM,
   1,
   TST_MPI_ALL_C_TYPES,
   TST_MODE_RELAXED,
   TST_NONE,            /* No synchronization needed, done with hash */
   &tst_p2p_alltoall_init, &tst_p2p_alltoall_run, &tst_p2p_alltoall_cleanup},


#ifndef HAVE_MPI_LAM
  {TST_CLASS_P2P, "Alltoall Persistent",
   TST_MPI_INTRA_COMM,
   1,
   TST_MPI_ALL_C_TYPES,
   TST_MODE_RELAXED,
   TST_NONE,            /* No synchronization needed, done with hash */
   &tst_p2p_alltoall_persistent_init, &tst_p2p_alltoall_persistent_run, &tst_p2p_alltoall_persistent_cleanup},
#endif


  {TST_CLASS_P2P, "Alltoall xIsend",
   TST_MPI_INTRA_COMM | TST_MPI_INTER_COMM,
   1,
   TST_MPI_ALL_C_TYPES,
   TST_MODE_RELAXED,
   TST_NONE,            /* No synchronization needed, done with hash */
   &tst_p2p_alltoall_xisend_init, &tst_p2p_alltoall_xisend_run, &tst_p2p_alltoall_xisend_cleanup},


  {TST_CLASS_P2P, "Alltoall Irsend",
   TST_MPI_INTRA_COMM /* | TST_MPI_INTER_COMM */,
   1,
   TST_MPI_ALL_C_TYPES,
   TST_MODE_RELAXED,
   TST_NONE,            /* No synchronization needed */
   &tst_p2p_alltoall_irsend_init, &tst_p2p_alltoall_irsend_run, &tst_p2p_alltoall_irsend_cleanup},


  {TST_CLASS_P2P, "Alltoall Issend",
   TST_MPI_INTRA_COMM | TST_MPI_INTER_COMM,
   1,
   TST_MPI_ALL_C_TYPES,
   TST_MODE_RELAXED,
   TST_NONE,            /* No synchronization needed -- everyone waits */
   &tst_p2p_alltoall_issend_init, &tst_p2p_alltoall_issend_run, &tst_p2p_alltoall_issend_cleanup},


  {TST_CLASS_P2P, "Alltoall with MPI_Probe (MPI_ANY_SOURCE)",
   TST_MPI_INTRA_COMM,
   1,
   TST_MPI_ALL_C_TYPES,
   TST_MODE_RELAXED,
   TST_SYNC,            /* Probing for MPI_ANY_SOURCE and MPI_ANY_TAG */
   &tst_p2p_alltoall_probe_anysource_init, &tst_p2p_alltoall_probe_anysource_run, &tst_p2p_alltoall_probe_anysource_cleanup},


  {TST_CLASS_P2P, "Ring Send with cart comm",
   TST_MPI_CART_COMM,
   1,
   TST_MPI_ALL_C_TYPES,
   TST_MODE_RELAXED,
   TST_SYNC,
   &tst_p2p_simple_ring_shift_init, &tst_p2p_simple_ring_shift_run, &tst_p2p_simple_ring_shift_cleanup},


  {TST_CLASS_P2P, "Alltoall with topo comm",
   TST_MPI_TOPO_COMM,
   1,
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
   1,
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
   1,
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
   1,
   TST_MPI_ALL_C_TYPES,
   TST_MODE_RELAXED,
   TST_NONE,            /* No synchronization needed */
   &tst_coll_allgather_init, &tst_coll_allgather_run, &tst_coll_allgather_cleanup},


  /*
   * XXX should allow TST_MPI_INTER_COMM depending on whether the underlying
   * MPI supports it!
   */
  {TST_CLASS_COLL, "Allgather with MPI_IN_PLACE",
   TST_MPI_COMM_SELF | TST_MPI_INTRA_COMM /* | TST_MPI_INTER_COMM */,
   1,
   TST_MPI_ALL_C_TYPES,
   TST_MODE_RELAXED,
   TST_NONE,            /* No synchronization needed */
   &tst_coll_allgather_in_place_init, &tst_coll_allgather_in_place_run, &tst_coll_allgather_in_place_cleanup},


  /*
   * XXX should allow TST_MPI_INTER_COMM depending on whether the underlying
   * MPI supports it!
   */
  {TST_CLASS_COLL, "Scan sum",
   TST_MPI_COMM_SELF | TST_MPI_INTRA_COMM /* | TST_MPI_INTER_COMM */,
   1,
   (TST_MPI_STANDARD_C_INT_TYPES |
     TST_MPI_STANDARD_C_FLOAT_TYPES |
     TST_MPI_STANDARD_FORTRAN_COMPLEX_TYPES) &
#ifdef HAVE_MPI2
   /*
    * MPI2 allows the usage of the MPI_SIGNED_CHAR and the MPI_UNSIGNED_CHAR types in reductions!
    * However, MPI_CHAR with no actual knowledge on the signed-ness is of course still not allowed.
    */
   /*
    * MPIch2 does not allow MPI_SIGNED_CHAR on collective Ops.
    */
#  ifdef HAVE_MPI_MPICH2
   ~(TST_MPI_CHAR | TST_MPI_SIGNED_CHAR | TST_MPI_BYTE),
#  else
   ~(TST_MPI_CHAR | TST_MPI_BYTE),
#  endif
#else
  ~(TST_MPI_CHAR | TST_MPI_UNSIGNED_CHAR | TST_MPI_BYTE),
#endif
   TST_MODE_RELAXED,
   TST_SYNC,            /* No synchronization needed */
   &tst_coll_scan_sum_init, &tst_coll_scan_sum_run, &tst_coll_scan_sum_cleanup},


  /*
   * XXX should allow TST_MPI_INTER_COMM depending on whether the underlying
   * MPI supports it!
   */
  {TST_CLASS_COLL, "Scatter",
   TST_MPI_COMM_SELF | TST_MPI_INTRA_COMM /* | TST_MPI_INTER_COMM */,
   1,
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
   1,
   TST_MPI_ALL_C_TYPES,
   TST_MODE_RELAXED,
   TST_NONE,            /* No synchronization needed */
   &tst_coll_scatterv_init, &tst_coll_scatterv_run, &tst_coll_scatterv_cleanup},


  /*
   * XXX should allow TST_MPI_INTER_COMM depending on whether the underlying
   * MPI supports it!
   */
  {TST_CLASS_COLL, "Scatterv with stride",
   TST_MPI_COMM_SELF | TST_MPI_INTRA_COMM /* | TST_MPI_INTER_COMM */,
   1,
   TST_MPI_ALL_C_TYPES,
   TST_MODE_RELAXED,
   TST_NONE,            /* No synchronization needed */
   &tst_coll_scatterv_stride_init, &tst_coll_scatterv_stride_run, &tst_coll_scatterv_stride_cleanup},


  /*
   * XXX should allow TST_MPI_INTER_COMM depending on whether the underlying
   * MPI supports it!
   */
  {TST_CLASS_COLL, "Reduce Min",
   TST_MPI_COMM_SELF | TST_MPI_INTRA_COMM /* | TST_MPI_INTER_COMM */,
   1,
   (TST_MPI_STANDARD_C_TYPES | TST_MPI_STANDARD_FORTRAN_INT_TYPES | TST_MPI_STANDARD_FORTRAN_FLOAT_TYPES) &
#ifdef HAVE_MPI2
   /*
    * MPI2 allows the usage of the MPI_SIGNED_CHAR and the MPI_UNSIGNED_CHAR types in reductions!
    * However, MPI_CHAR with no actual knowledge on the signed-ness is of course still not allowed.
    */
   /*
    * MPIch2 does not allow MPI_SIGNED_CHAR on collective Ops.
    */
#  ifdef HAVE_MPI_MPICH2
   ~(TST_MPI_CHAR | TST_MPI_SIGNED_CHAR | TST_MPI_BYTE),
#  else
   ~(TST_MPI_CHAR | TST_MPI_BYTE),
#  endif
#else
   ~(TST_MPI_CHAR | TST_MPI_SIGNED_CHAR | TST_MPI_UNSIGNED_CHAR | TST_MPI_BYTE),
#endif
   TST_MODE_RELAXED,
   TST_NONE,            /* No synchronization needed */
   &tst_coll_reduce_min_init, &tst_coll_reduce_min_run, &tst_coll_reduce_min_cleanup},


  /*
   * XXX should allow TST_MPI_INTER_COMM depending on whether the underlying
   * MPI supports it!
   */
  {TST_CLASS_COLL, "Reduce Max",
   TST_MPI_COMM_SELF | TST_MPI_INTRA_COMM /* | TST_MPI_INTER_COMM */,
   1,
   (TST_MPI_STANDARD_C_TYPES | TST_MPI_STANDARD_FORTRAN_INT_TYPES | TST_MPI_STANDARD_FORTRAN_FLOAT_TYPES) &
#ifdef HAVE_MPI2
   /*
    * MPI2 allows the usage of the MPI_SIGNED_CHAR and the MPI_UNSIGNED_CHAR types in reductions!
    * However, MPI_CHAR with no actual knowledge on the signed-ness is of course still not allowed.
    */
   /*
    * MPIch2 does not allow MPI_SIGNED_CHAR on collective Ops.
    */
#  ifdef HAVE_MPI_MPICH2
   ~(TST_MPI_CHAR | TST_MPI_SIGNED_CHAR | TST_MPI_BYTE),
#  else
   ~(TST_MPI_CHAR | TST_MPI_BYTE),
#  endif
#else
   ~(TST_MPI_CHAR | TST_MPI_SIGNED_CHAR | TST_MPI_UNSIGNED_CHAR | TST_MPI_BYTE),
#endif

   TST_MODE_RELAXED,
   TST_NONE,            /* No synchronization needed */
   &tst_coll_reduce_max_init, &tst_coll_reduce_max_run, &tst_coll_reduce_max_cleanup},


  /*
   * XXX should allow TST_MPI_INTER_COMM depending on whether the underlying
   * MPI supports it!
   */
  {TST_CLASS_COLL, "Reduce Min with MPI_IN_PLACE",
   TST_MPI_COMM_SELF | TST_MPI_INTRA_COMM /* | TST_MPI_INTER_COMM */,
   1,
   (TST_MPI_STANDARD_C_TYPES | TST_MPI_STANDARD_FORTRAN_INT_TYPES | TST_MPI_STANDARD_FORTRAN_FLOAT_TYPES) &
#ifdef HAVE_MPI2
   /*
    * MPI2 allows the usage of the MPI_SIGNED_CHAR and the MPI_UNSIGNED_CHAR types in reductions!
    * However, MPI_CHAR with no actual knowledge on the signed-ness is of course still not allowed.
    */
   /*
    * MPIch2 does not allow MPI_SIGNED_CHAR on collective Ops.
    */
#  ifdef HAVE_MPI_MPICH2
   ~(TST_MPI_CHAR | TST_MPI_SIGNED_CHAR | TST_MPI_BYTE),
#  else
   ~(TST_MPI_CHAR | TST_MPI_BYTE),
#  endif
#else
   ~(TST_MPI_CHAR | TST_MPI_SIGNED_CHAR | TST_MPI_UNSIGNED_CHAR | TST_MPI_BYTE),
#endif
   TST_MODE_RELAXED,
   TST_NONE,            /* No synchronization needed */
   &tst_coll_reduce_in_place_min_init, &tst_coll_reduce_in_place_min_run, &tst_coll_reduce_in_place_min_cleanup},


  {TST_CLASS_COLL, "Reduce Max with MPI_IN_PLACE",
   TST_MPI_COMM_SELF | TST_MPI_INTRA_COMM /* | TST_MPI_INTER_COMM */,
   1,
   (TST_MPI_STANDARD_C_TYPES | TST_MPI_STANDARD_FORTRAN_INT_TYPES | TST_MPI_STANDARD_FORTRAN_FLOAT_TYPES) &
#ifdef HAVE_MPI2
   /*
    * MPI2 allows the usage of the MPI_SIGNED_CHAR and the MPI_UNSIGNED_CHAR types in reductions!
    * However, MPI_CHAR with no actual knowledge on the signed-ness is of course still not allowed.
    */
   /*
    * MPIch2 does not allow MPI_SIGNED_CHAR on collective Ops.
    */
#  ifdef HAVE_MPI_MPICH2
   ~(TST_MPI_CHAR | TST_MPI_SIGNED_CHAR | TST_MPI_BYTE),
#  else
   ~(TST_MPI_CHAR | TST_MPI_BYTE),
#  endif
#else
   ~(TST_MPI_CHAR | TST_MPI_SIGNED_CHAR | TST_MPI_UNSIGNED_CHAR | TST_MPI_BYTE),
#endif
   TST_MODE_RELAXED,
   TST_NONE,            /* No synchronization needed */
   &tst_coll_reduce_in_place_max_init, &tst_coll_reduce_in_place_max_run, &tst_coll_reduce_in_place_max_cleanup},


  {TST_CLASS_COLL, "Allreduce Min",
   TST_MPI_COMM_SELF | TST_MPI_INTRA_COMM /* | TST_MPI_INTER_COMM */,
   1,
   (TST_MPI_STANDARD_C_TYPES | TST_MPI_STANDARD_FORTRAN_INT_TYPES | TST_MPI_STANDARD_FORTRAN_FLOAT_TYPES) &
#ifdef HAVE_MPI2
   /*
    * MPI2 allows the usage of the MPI_SIGNED_CHAR and the MPI_UNSIGNED_CHAR types in reductions!
    * However, MPI_CHAR with no actual knowledge on the signed-ness is of course still not allowed.
    */
   /*
    * MPIch2 does not allow MPI_SIGNED_CHAR on collective Ops.
    */
#  ifdef HAVE_MPI_MPICH2
   ~(TST_MPI_CHAR | TST_MPI_SIGNED_CHAR | TST_MPI_BYTE),
#  else
   ~(TST_MPI_CHAR | TST_MPI_BYTE),
#  endif
#else
   ~(TST_MPI_CHAR | TST_MPI_SIGNED_CHAR | TST_MPI_UNSIGNED_CHAR | TST_MPI_BYTE),
#endif
   TST_MODE_RELAXED,
   TST_NONE,            /* No synchronization needed */
   &tst_coll_allreduce_min_init, &tst_coll_allreduce_min_run, &tst_coll_allreduce_min_cleanup},


  /*
   * The Allreduce call is not valid for MPI_MIN/MPI_MAX and
   * the datatypes MPI_Char, MPI_UNSIGNED_CHAR and MPI_Byte and
   * the struct datatypes
   *
   * XXX should allow TST_MPI_INTER_COMM depending on whether the underlying
   * MPI supports it!
   */
  {TST_CLASS_COLL, "Allreduce Max",
   TST_MPI_COMM_SELF | TST_MPI_INTRA_COMM /* | TST_MPI_INTER_COMM */,
   1,
   (TST_MPI_STANDARD_C_TYPES | TST_MPI_STANDARD_FORTRAN_INT_TYPES | TST_MPI_STANDARD_FORTRAN_FLOAT_TYPES) &
#ifdef HAVE_MPI2
   /*
    * MPI2 allows the usage of the MPI_SIGNED_CHAR and the MPI_UNSIGNED_CHAR types in reductions!
    * However, MPI_CHAR with no actual knowledge on the signed-ness is of course still not allowed.
    */
   /*
    * MPIch2 does not allow MPI_SIGNED_CHAR on collective Ops.
    */
#  ifdef HAVE_MPI_MPICH2
   ~(TST_MPI_CHAR | TST_MPI_SIGNED_CHAR | TST_MPI_BYTE),
#  else
   ~(TST_MPI_CHAR | TST_MPI_BYTE),
#  endif
#else
   ~(TST_MPI_CHAR | TST_MPI_SIGNED_CHAR | TST_MPI_UNSIGNED_CHAR | TST_MPI_BYTE),
#endif
   TST_MODE_RELAXED,
   TST_NONE,            /* No synchronization needed */
   &tst_coll_allreduce_max_init, &tst_coll_allreduce_max_run, &tst_coll_allreduce_max_cleanup},


  {TST_CLASS_COLL, "Allreduce Min/Max",
   TST_MPI_COMM_SELF | TST_MPI_INTRA_COMM /* | TST_MPI_INTER_COMM */,
   1,
   (TST_MPI_STANDARD_C_TYPES | TST_MPI_STANDARD_FORTRAN_INT_TYPES | TST_MPI_STANDARD_FORTRAN_FLOAT_TYPES) &
#ifdef HAVE_MPI2
   /*
    * MPI2 allows the usage of the MPI_SIGNED_CHAR and the MPI_UNSIGNED_CHAR types in reductions!
    * However, MPI_CHAR with no actual knowledge on the signed-ness is of course still not allowed.
    */
   /*
    * MPIch2 does not allow MPI_SIGNED_CHAR on collective Ops.
    */
#  ifdef HAVE_MPI_MPICH2
   ~(TST_MPI_CHAR | TST_MPI_SIGNED_CHAR | TST_MPI_BYTE),
#  else
   ~(TST_MPI_CHAR | TST_MPI_BYTE),
#  endif
#else
   ~(TST_MPI_CHAR | TST_MPI_SIGNED_CHAR | TST_MPI_UNSIGNED_CHAR | TST_MPI_BYTE),
#endif
   TST_MODE_RELAXED,
   TST_NONE,            /* No synchronization needed */
   &tst_coll_allreduce_init, &tst_coll_allreduce_run, &tst_coll_allreduce_cleanup},


  /*
   * MPI_IN_PLACE allreduce
   * XXX should allow TST_MPI_INTER_COMM depending on whether the underlying
   * MPI supports it!
   */
  {TST_CLASS_COLL, "Allreduce Min/Max with MPI_IN_PLACE",
   TST_MPI_COMM_SELF | TST_MPI_INTRA_COMM /* | TST_MPI_INTER_COMM */,
   1,
   (TST_MPI_STANDARD_C_TYPES | TST_MPI_STANDARD_FORTRAN_INT_TYPES | TST_MPI_STANDARD_FORTRAN_FLOAT_TYPES) &
#ifdef HAVE_MPI2
   /*
    * MPI2 allows the usage of the MPI_SIGNED_CHAR and the MPI_UNSIGNED_CHAR types in reductions!
    * However, MPI_CHAR with no actual knowledge on the signed-ness is of course still not allowed.
    */
   /*
    * MPIch2 does not allow MPI_SIGNED_CHAR on collective Ops.
    */
#  ifdef HAVE_MPICH2
   ~(TST_MPI_CHAR | TST_MPI_SIGNED_CHAR | TST_MPI_BYTE),
#  else
   ~(TST_MPI_CHAR | TST_MPI_BYTE),
#  endif
#else
   ~(TST_MPI_CHAR | TST_MPI_SIGNED_CHAR | TST_MPI_UNSIGNED_CHAR | TST_MPI_BYTE),
#endif
   TST_MODE_RELAXED,
   TST_NONE,            /* No synchronization needed */
   &tst_coll_allreduce_in_place_init, &tst_coll_allreduce_in_place_run, &tst_coll_allreduce_in_place_cleanup},


  {TST_CLASS_COLL, "Allreduce Sum",
   TST_MPI_COMM_SELF | TST_MPI_INTRA_COMM /* | TST_MPI_INTER_COMM */,
   1,
   (TST_MPI_STANDARD_C_FLOAT_TYPES) &
   ~(TST_MPI_FLOAT | TST_MPI_LONG_DOUBLE),
   TST_MODE_RELAXED,
   TST_NONE,            /* No synchronization needed */
   &tst_coll_allreduce_sum_init, &tst_coll_allreduce_sum_run, &tst_coll_allreduce_sum_cleanup},


#if 0 /* quadsum operation is not associative so the test is wrong. */
  {TST_CLASS_COLL, "Allreduce Quadsum",
   TST_MPI_INTRA_COMM /* | TST_MPI_INTER_COMM */,
   1,
   TST_MPI_LONG,
   TST_MODE_RELAXED,
   TST_NONE,            /* No synchronization needed */
   &tst_coll_allreduce_quadsum_init, &tst_coll_allreduce_quadsum_run, &tst_coll_allreduce_quadsum_cleanup},
#endif

  /*
   * XXX should allow TST_MPI_INTER_COMM depending on whether the underlying
   * MPI supports it!
   */
  {TST_CLASS_COLL, "Alltoall",
   TST_MPI_COMM_SELF | TST_MPI_INTRA_COMM /* | TST_MPI_INTER_COMM */,
   1,
   TST_MPI_ALL_C_TYPES,
   TST_MODE_RELAXED,
   TST_NONE,            /* No synchronization needed */
   &tst_coll_alltoall_init, &tst_coll_alltoall_run, &tst_coll_alltoall_cleanup},


  /*
   * Here follow the MPI2 Dynamic process management test
   */
#if HAVE_MPI2_DYNAMIC
  {TST_CLASS_DYNAMIC, "establish_communication_alltoall",
   TST_MPI_INTRA_COMM /* | TST_MPI_INTER_COMM */,
   1,
   TST_MPI_ALL_C_TYPES,
   TST_MODE_RELAXED,
   /* XXX CN Need sync or not? */
   TST_SYNC,            /* No synchronization needed */
   &tst_establish_communication_init, &tst_establish_communication_run, &tst_establish_communication_cleanup },


  {TST_CLASS_DYNAMIC, "comm_spawn",
   TST_MPI_INTRA_COMM, /* | TST_MPI_INTER_COMM, */
   1,
   TST_MPI_INT,
   TST_MODE_RELAXED,
   TST_SYNC,            /* No synchronization needed */
   &tst_comm_spawn_init, &tst_comm_spawn_run, &tst_comm_spawn_cleanup },

  {TST_CLASS_DYNAMIC, "comm_spawn_multiple",
   TST_MPI_INTRA_COMM, /* | TST_MPI_INTER_COMM, */
   1,
   TST_MPI_INT,
   TST_MODE_RELAXED,
   TST_SYNC,            /* No synchronization needed */
   &tst_comm_spawn_multiple_init, &tst_comm_spawn_multiple_run, &tst_comm_spawn_multiple_cleanup },
#endif /* HAVE_MPI2_DYNAMIC */


#ifdef HAVE_MPI2_ONE_SIDED
  {TST_CLASS_ONE_SIDED, "get_fence",
   TST_MPI_INTRA_COMM /* | TST_MPI_INTER_COMM */,
   1,
   TST_MPI_ALL_C_TYPES,
   TST_MODE_RELAXED,
   TST_SYNC,            /* No synchronization needed */
   &tst_get_with_fence_alltoall_init, &tst_get_with_fence_alltoall_run, &tst_get_with_fence_alltoall_cleanup },


  {TST_CLASS_ONE_SIDED, "put_fence",
   TST_MPI_INTRA_COMM /* | TST_MPI_INTER_COMM */,
   1,
   TST_MPI_ALL_C_TYPES,
   TST_MODE_RELAXED,
   TST_SYNC,            /* No synchronization needed */
   &tst_put_with_fence_alltoall_init, &tst_put_with_fence_alltoall_run, &tst_put_with_fence_alltoall_cleanup },


  {TST_CLASS_ONE_SIDED, "accumulate_fence",
   TST_MPI_INTRA_COMM /* | TST_MPI_INTER_COMM */,
   1,
   (TST_MPI_STANDARD_C_FLOAT_TYPES) &
   ~(TST_MPI_FLOAT | TST_MPI_LONG_DOUBLE),
   TST_MODE_RELAXED,
   TST_SYNC,            /* No synchronization needed */
   &tst_accumulate_with_fence_sum_init, &tst_accumulate_with_fence_sum_run, &tst_accumulate_with_fence_sum_cleanup },


  {TST_CLASS_ONE_SIDED, "get_post",
   TST_MPI_INTRA_COMM /* | TST_MPI_INTER_COMM */,
   1,
   TST_MPI_ALL_C_TYPES,
   TST_MODE_RELAXED,
   TST_SYNC,            /* No synchronization needed */
   &tst_get_with_post_alltoall_init, &tst_get_with_post_alltoall_run, &tst_get_with_post_alltoall_cleanup },


  {TST_CLASS_ONE_SIDED, "put_post",
   TST_MPI_INTRA_COMM /* | TST_MPI_INTER_COMM */,
   1,
   TST_MPI_ALL_C_TYPES,
   TST_MODE_RELAXED,
   TST_SYNC,            /* No synchronization needed */
   &tst_put_with_post_alltoall_init, &tst_put_with_post_alltoall_run, &tst_put_with_post_alltoall_cleanup },


  {TST_CLASS_ONE_SIDED, "accumulate_post_min",
   TST_MPI_INTRA_COMM /* | TST_MPI_INTER_COMM */,
   1,
   (TST_MPI_STANDARD_C_TYPES | TST_MPI_STANDARD_FORTRAN_INT_TYPES | TST_MPI_STANDARD_FORTRAN_FLOAT_TYPES) &
   ~(TST_MPI_CHAR | TST_MPI_UNSIGNED_CHAR | TST_MPI_BYTE),
   TST_MODE_RELAXED,
   TST_SYNC,            /* No synchronization needed */
   &tst_accumulate_with_post_min_init, &tst_accumulate_with_post_min_run, &tst_accumulate_with_post_min_cleanup },


  {TST_CLASS_ONE_SIDED, "get_lock",
   TST_MPI_INTRA_COMM /* | TST_MPI_INTER_COMM */,
   1,
   TST_MPI_ALL_C_TYPES,
   TST_MODE_RELAXED,
   TST_SYNC,            /* No synchronization needed */
   &tst_get_with_lock_alltoall_init, &tst_get_with_lock_alltoall_run, &tst_get_with_lock_alltoall_cleanup },


  {TST_CLASS_ONE_SIDED, "put_lock",
   TST_MPI_INTRA_COMM /* | TST_MPI_INTER_COMM */,
   1,
   TST_MPI_ALL_C_TYPES,
   TST_MODE_RELAXED,
   TST_SYNC,            /* No synchronization needed */
   &tst_put_with_lock_alltoall_init, &tst_put_with_lock_alltoall_run, &tst_put_with_lock_alltoall_cleanup },


  {TST_CLASS_ONE_SIDED, "accumulate_lock",
   TST_MPI_INTRA_COMM /* | TST_MPI_INTER_COMM */,
   1,
   (TST_MPI_STANDARD_C_TYPES | TST_MPI_STANDARD_FORTRAN_INT_TYPES | TST_MPI_STANDARD_FORTRAN_FLOAT_TYPES) &
   ~(TST_MPI_CHAR | TST_MPI_UNSIGNED_CHAR | TST_MPI_BYTE),
   TST_MODE_RELAXED,
   TST_SYNC,            /* No synchronization needed */
   &tst_accumulate_with_lock_max_init, &tst_accumulate_with_lock_max_run, &tst_accumulate_with_lock_max_cleanup },


  {TST_CLASS_ONE_SIDED, "Ring with Get",
   TST_MPI_COMM_SELF | TST_MPI_INTRA_COMM, /* XXX possible with MPI_COMM_SELF?? */
   1,
   TST_MPI_STANDARD_C_TYPES, /* Fails with TST_MPI_ALL_C_TYPES, as the struct-datatypes are not supported */
   TST_MODE_RELAXED,
   TST_NONE,            /* No synchronization needed */
   &tst_one_sided_simple_ring_get_init, &tst_one_sided_simple_ring_get_run, &tst_one_sided_simple_ring_get_cleanup},


   /*
    * MPICH2-1.0.3 hangs in this test on the
    *  Odd/Even split MPI_COMM_WORLD (ONLY on *this* one, all other intra-comms work)
    * OpenMPI-v9189 also has problems
    */
#if !defined(HAVE_MPI_MPICH2) && !defined(HAVE_MPI_NECSX) && !defined(HAVE_MPI_OPENMPI)
  {TST_CLASS_ONE_SIDED, "Ring with Get using Post",
   TST_MPI_INTRA_COMM, /* XXX possible with MPI_COMM_SELF?? */
   2,
   TST_MPI_STANDARD_C_TYPES, /* Fails with TST_MPI_ALL_C_TYPES, as the struct-datatypes are not supported */
   TST_MODE_RELAXED,
   TST_NONE,            /* No synchronization needed */
   &tst_one_sided_simple_ring_get_post_init, &tst_one_sided_simple_ring_get_post_run, &tst_one_sided_simple_ring_get_post_cleanup},
#endif


  {TST_CLASS_ONE_SIDED, "Ring with Put",
   TST_MPI_COMM_SELF | TST_MPI_INTRA_COMM, /* XXX possible with MPI_COMM_SELF?? */
   1,
   TST_MPI_STANDARD_C_TYPES, /* Fails with TST_MPI_ALL_C_TYPES, as the struct-datatypes are not supported */
   TST_MODE_RELAXED,
   TST_NONE,            /* No synchronization needed */
   &tst_one_sided_simple_ring_put_init, &tst_one_sided_simple_ring_put_run, &tst_one_sided_simple_ring_put_cleanup},
#endif /* HAVE_MPI2_ONE_SIDED */


#ifdef HAVE_MPI2_IO
  /*
   * Here come the io tests
   */
  {TST_CLASS_IO, "file simple",
   TST_MPI_INTRA_COMM /* | TST_MPI_INTER_COMM */,
   1,
   /*TST_MPI_STANDARD_C_TYPES,*/
   TST_MPI_ALL_C_TYPES&
   ~(TST_MPI_TYPE_MIX_LB_UB),
   TST_MODE_RELAXED,
   TST_SYNC,            /* Needs sync due to bcast of filename in init */
   &tst_file_simple_init, &tst_file_simple_run, &tst_file_simple_cleanup },


  {TST_CLASS_IO, "read_at",
   TST_MPI_INTRA_COMM /* | TST_MPI_INTER_COMM */,
   1,
   /*TST_MPI_STANDARD_C_TYPES,*/
   TST_MPI_ALL_C_TYPES&
   ~(TST_MPI_TYPE_MIX_LB_UB),
   TST_MODE_RELAXED,
   TST_SYNC,            /* Needs sync due to bcast of filename in init */
   &tst_file_read_at_init, &tst_file_read_at_run, &tst_file_read_at_cleanup },


  {TST_CLASS_IO, "write_at",
   TST_MPI_INTRA_COMM /* | TST_MPI_INTER_COMM */,
   1,
   /*TST_MPI_STANDARD_C_TYPES,*/
   TST_MPI_ALL_C_TYPES&
   ~(TST_MPI_TYPE_MIX_LB_UB),
   TST_MODE_RELAXED,
   TST_SYNC,            /* Needs sync due to bcast of filename in init */
   &tst_file_write_at_init, &tst_file_write_at_run, &tst_file_write_at_cleanup },


  {TST_CLASS_IO, "read_at_all",
   TST_MPI_INTRA_COMM /* | TST_MPI_INTER_COMM */,
   1,
   /*TST_MPI_STANDARD_C_TYPES,*/
   TST_MPI_ALL_C_TYPES&
   ~(TST_MPI_TYPE_MIX_LB_UB),
   TST_MODE_RELAXED,
   TST_SYNC,            /* Needs sync due to bcast of filename in init */
   &tst_file_read_at_all_init, &tst_file_read_at_all_run, &tst_file_read_at_all_cleanup },


  {TST_CLASS_IO, "write_at_all",
   TST_MPI_INTRA_COMM /* | TST_MPI_INTER_COMM */,
   1,
   /*TST_MPI_STANDARD_C_TYPES,*/
   TST_MPI_ALL_C_TYPES&
   ~(TST_MPI_TYPE_MIX_LB_UB),
   TST_MODE_RELAXED,
   TST_SYNC,            /* Needs sync due to bcast of filename in init */
   &tst_file_write_at_all_init, &tst_file_write_at_all_run, &tst_file_write_at_all_cleanup },


  {TST_CLASS_IO, "iread_at",
   TST_MPI_INTRA_COMM /* | TST_MPI_INTER_COMM */,
   1,
   /*TST_MPI_STANDARD_C_TYPES,*/
   TST_MPI_ALL_C_TYPES&
   ~(TST_MPI_TYPE_MIX_LB_UB),
   TST_MODE_RELAXED,
   TST_SYNC,            /* Needs sync due to bcast of filename in init */
   &tst_file_iread_at_init, &tst_file_iread_at_run, &tst_file_iread_at_cleanup },


  {TST_CLASS_IO, "iwrite_at",
   TST_MPI_INTRA_COMM /* | TST_MPI_INTER_COMM */,
   1,
   /*TST_MPI_STANDARD_C_TYPES,*/
   TST_MPI_ALL_C_TYPES&
   ~(TST_MPI_TYPE_MIX_LB_UB),
   TST_MODE_RELAXED,
   TST_SYNC,            /* Needs sync due to bcast of filename in init */
   &tst_file_iwrite_at_init, &tst_file_iwrite_at_run, &tst_file_iwrite_at_cleanup },


  {TST_CLASS_IO, "read_at_all_begin",
   TST_MPI_INTRA_COMM /* | TST_MPI_INTER_COMM */,
   1,
   /*TST_MPI_STANDARD_C_TYPES,*/
   TST_MPI_ALL_C_TYPES&
   ~(TST_MPI_TYPE_MIX_LB_UB),
   TST_MODE_RELAXED,
   TST_SYNC,            /* Needs sync due to bcast of filename in init */
   &tst_file_read_at_all_begin_init, &tst_file_read_at_all_begin_run, &tst_file_read_at_all_begin_cleanup },


  {TST_CLASS_IO, "write_at_all_begin",
   TST_MPI_INTRA_COMM /* | TST_MPI_INTER_COMM */,
   1,
   /*TST_MPI_STANDARD_C_TYPES,*/
   TST_MPI_ALL_C_TYPES&
   ~(TST_MPI_TYPE_MIX_LB_UB),
   TST_MODE_RELAXED,
   TST_SYNC,            /* Needs sync due to bcast of filename in init */
   &tst_file_write_at_all_begin_init, &tst_file_write_at_all_begin_run, &tst_file_write_at_all_begin_cleanup },


  {TST_CLASS_IO, "read",
   TST_MPI_INTRA_COMM /* | TST_MPI_INTER_COMM */,
   1,
   /*TST_MPI_STANDARD_C_TYPES,*/
   TST_MPI_ALL_C_TYPES&
   ~(TST_MPI_TYPE_MIX_LB_UB),
   TST_MODE_RELAXED,
   TST_SYNC,            /* Needs sync due to bcast of filename in init */
   &tst_file_read_init, &tst_file_read_run, &tst_file_read_cleanup },


  {TST_CLASS_IO, "write",
   TST_MPI_INTRA_COMM /* | TST_MPI_INTER_COMM */,
   1,
   /*TST_MPI_STANDARD_C_TYPES,*/
   TST_MPI_ALL_C_TYPES&
   ~(TST_MPI_TYPE_MIX_LB_UB),
   TST_MODE_RELAXED,
   TST_SYNC,            /* Needs sync due to bcast of filename in init */
   &tst_file_write_init, &tst_file_write_run, &tst_file_write_cleanup },


  {TST_CLASS_IO, "read_all",
   TST_MPI_INTRA_COMM /* | TST_MPI_INTER_COMM */,
   1,
   /*TST_MPI_STANDARD_C_TYPES,*/
   TST_MPI_ALL_C_TYPES&
   ~(TST_MPI_TYPE_MIX_LB_UB),
   TST_MODE_RELAXED,
   TST_SYNC,            /* Needs sync due to bcast of filename in init */
   &tst_file_read_all_init, &tst_file_read_all_run, &tst_file_read_all_cleanup },


  {TST_CLASS_IO, "write_all",
   TST_MPI_INTRA_COMM /* | TST_MPI_INTER_COMM */,
   1,
   /*TST_MPI_STANDARD_C_TYPES,*/
   TST_MPI_ALL_C_TYPES&
   ~(TST_MPI_TYPE_MIX_LB_UB),
   TST_MODE_RELAXED,
   TST_SYNC,            /* Needs sync due to bcast of filename in init */
   &tst_file_write_all_init, &tst_file_write_all_run, &tst_file_write_all_cleanup },


  {TST_CLASS_IO, "iread",
   TST_MPI_INTRA_COMM /* | TST_MPI_INTER_COMM */,
   1,
   /*TST_MPI_STANDARD_C_TYPES,*/
   TST_MPI_ALL_C_TYPES&
   ~(TST_MPI_TYPE_MIX_LB_UB),
   TST_MODE_RELAXED,
   TST_SYNC,            /* Needs sync due to bcast of filename in init */
   &tst_file_iread_init, &tst_file_iread_run, &tst_file_iread_cleanup },


  {TST_CLASS_IO, "iwrite",
   TST_MPI_INTRA_COMM /* | TST_MPI_INTER_COMM */,
   1,
   /*TST_MPI_STANDARD_C_TYPES,*/
   TST_MPI_ALL_C_TYPES&
   ~(TST_MPI_TYPE_MIX_LB_UB),
   TST_MODE_RELAXED,
   TST_SYNC,            /* Needs sync due to bcast of filename in init */
   &tst_file_iwrite_init, &tst_file_iwrite_run, &tst_file_iwrite_cleanup },


  {TST_CLASS_IO, "read_all_begin",
   TST_MPI_INTRA_COMM /* | TST_MPI_INTER_COMM */,
   1,
   /*TST_MPI_STANDARD_C_TYPES,*/
   TST_MPI_ALL_C_TYPES&
   ~(TST_MPI_TYPE_MIX_LB_UB),
   TST_MODE_RELAXED,
   TST_SYNC,            /* Needs sync due to bcast of filename in init */
   &tst_file_read_all_begin_init, &tst_file_read_all_begin_run, &tst_file_read_all_begin_cleanup },


  {TST_CLASS_IO, "write_all_begin",
   TST_MPI_INTRA_COMM /* | TST_MPI_INTER_COMM */,
   1,
   /*TST_MPI_STANDARD_C_TYPES,*/
   TST_MPI_ALL_C_TYPES&
   ~(TST_MPI_TYPE_MIX_LB_UB),
   TST_MODE_RELAXED,
   TST_SYNC,            /* Needs sync due to bcast of filename in init */
   &tst_file_write_all_begin_init, &tst_file_write_all_begin_run, &tst_file_write_all_begin_cleanup },


  {TST_CLASS_IO, "read_shared",
   TST_MPI_INTRA_COMM /* | TST_MPI_INTER_COMM */,
   1,
   /*TST_MPI_STANDARD_C_TYPES,*/
   TST_MPI_ALL_C_TYPES&
   ~(TST_MPI_TYPE_MIX_LB_UB),
   TST_MODE_RELAXED,
   TST_SYNC,            /* Needs sync due to bcast of filename in init */
   &tst_file_read_shared_init, &tst_file_read_shared_run, &tst_file_read_shared_cleanup },


  {TST_CLASS_IO, "write_shared",
   TST_MPI_INTRA_COMM /* | TST_MPI_INTER_COMM */,
   1,
   /*TST_MPI_STANDARD_C_TYPES,*/
   TST_MPI_ALL_C_TYPES&
   ~(TST_MPI_TYPE_MIX_LB_UB),
   TST_MODE_RELAXED,
   TST_SYNC,            /* Needs sync due to bcast of filename in init */
   &tst_file_write_shared_init, &tst_file_write_shared_run, &tst_file_write_shared_cleanup },


  {TST_CLASS_IO, "read_ordered",
   TST_MPI_INTRA_COMM /* | TST_MPI_INTER_COMM */,
   1,
   /*TST_MPI_STANDARD_C_TYPES,*/
   TST_MPI_ALL_C_TYPES&
   ~(TST_MPI_TYPE_MIX_LB_UB),
   TST_MODE_RELAXED,
   TST_SYNC,            /* Needs sync due to bcast of filename in init */
   &tst_file_read_ordered_init, &tst_file_read_ordered_run, &tst_file_read_ordered_cleanup },


  {TST_CLASS_IO, "write_ordered",
   TST_MPI_INTRA_COMM /* | TST_MPI_INTER_COMM */,
   1,
   /*TST_MPI_STANDARD_C_TYPES,*/
   TST_MPI_ALL_C_TYPES&
   ~(TST_MPI_TYPE_MIX_LB_UB),
   TST_MODE_RELAXED,
   TST_SYNC,            /* Needs sync due to bcast of filename in init */
   &tst_file_write_ordered_init, &tst_file_write_ordered_run, &tst_file_write_ordered_cleanup },


  {TST_CLASS_IO, "iread_shared",
   TST_MPI_INTRA_COMM /* | TST_MPI_INTER_COMM */,
   1,
   /*TST_MPI_STANDARD_C_TYPES,*/
   TST_MPI_ALL_C_TYPES&
   ~(TST_MPI_TYPE_MIX_LB_UB),
   TST_MODE_RELAXED,
   TST_SYNC,            /* Needs sync due to bcast of filename in init */
   &tst_file_iread_shared_init, &tst_file_iread_shared_run, &tst_file_iread_shared_cleanup },


  {TST_CLASS_IO, "iwrite_shared",
   TST_MPI_INTRA_COMM /* | TST_MPI_INTER_COMM */,
   1,
    /*TST_MPI_STANDARD_C_TYPES,*/
   TST_MPI_ALL_C_TYPES&
   ~(TST_MPI_TYPE_MIX_LB_UB),
   TST_MODE_RELAXED,
   TST_SYNC,            /* Needs sync due to bcast of filename in init */
   &tst_file_iwrite_shared_init, &tst_file_iwrite_shared_run, &tst_file_iwrite_shared_cleanup },


  {TST_CLASS_IO, "read_ordered_begin",
   TST_MPI_INTRA_COMM /* | TST_MPI_INTER_COMM */,
   1,
   /*TST_MPI_STANDARD_C_TYPES,*/
   TST_MPI_ALL_C_TYPES&
   ~(TST_MPI_TYPE_MIX_LB_UB),
   TST_MODE_RELAXED,
   TST_SYNC,            /* Needs sync due to bcast of filename in init */
   &tst_file_read_ordered_begin_init, &tst_file_read_ordered_begin_run, &tst_file_read_ordered_begin_cleanup },


  {TST_CLASS_IO, "write_ordered_begin",
   TST_MPI_INTRA_COMM /* | TST_MPI_INTER_COMM */,
   1,
   /*TST_MPI_STANDARD_C_TYPES,*/
   TST_MPI_ALL_C_TYPES&
   ~(TST_MPI_TYPE_MIX_LB_UB),
   TST_MODE_RELAXED,
   TST_SYNC,            /* Needs sync due to bcast of filename in init */
   &tst_file_write_ordered_begin_init, &tst_file_write_ordered_begin_run, &tst_file_write_ordered_begin_cleanup },


  {TST_CLASS_IO, "read_darray",
   TST_MPI_INTRA_COMM /* | TST_MPI_INTER_COMM */,
   1,
   /*TST_MPI_STANDARD_C_TYPES,*/
   TST_MPI_ALL_C_TYPES&
   ~(TST_MPI_TYPE_MIX_LB_UB),
   TST_MODE_RELAXED,
   TST_SYNC,            /* Needs sync due to bcast of filename in init */
   &tst_file_read_darray_init, &tst_file_read_darray_run, &tst_file_read_darray_cleanup },


  {TST_CLASS_IO, "write_darray",
   TST_MPI_INTRA_COMM /* | TST_MPI_INTER_COMM */,
   1,
   /*TST_MPI_STANDARD_C_TYPES,*/
   TST_MPI_ALL_C_TYPES&
   ~(TST_MPI_TYPE_MIX_LB_UB),
   TST_MODE_RELAXED,
   TST_SYNC,            /* Needs sync due to bcast of filename in init */
   &tst_file_write_darray_init, &tst_file_write_darray_run, &tst_file_write_darray_cleanup },


  {TST_CLASS_IO, "read_subarray",
   TST_MPI_INTRA_COMM /* | TST_MPI_INTER_COMM */,
   1,
   /*TST_MPI_STANDARD_C_TYPES,*/
   TST_MPI_ALL_C_TYPES&
   ~(TST_MPI_TYPE_MIX_LB_UB),
   TST_MODE_RELAXED,
   TST_SYNC,            /* Needs sync due to bcast of filename in init */
   &tst_file_read_subarray_init, &tst_file_read_subarray_run, &tst_file_read_subarray_cleanup },


  {TST_CLASS_IO, "write_subarray",
   TST_MPI_INTRA_COMM /* | TST_MPI_INTER_COMM */,
   1,
   /*TST_MPI_STANDARD_C_TYPES,*/
   TST_MPI_ALL_C_TYPES&
   ~(TST_MPI_TYPE_MIX_LB_UB),
   TST_MODE_RELAXED,
   TST_SYNC,            /* Needs sync due to bcast of filename in init */
   &tst_file_write_subarray_init, &tst_file_write_subarray_run, &tst_file_write_subarray_cleanup },


  {TST_CLASS_IO, "io_with_hole",
   TST_MPI_INTRA_COMM /* | TST_MPI_INTER_COMM */,
   1,
   /*TST_MPI_STANDARD_C_TYPES,*/
   TST_MPI_ALL_C_TYPES&
   ~(TST_MPI_TYPE_MIX_LB_UB),
   TST_MODE_RELAXED,
   TST_SYNC,            /* Needs sync due to bcast of filename in init */
   &tst_file_io_with_hole_init, &tst_file_io_with_hole_run, &tst_file_io_with_hole_cleanup },


  {TST_CLASS_IO, "io_with_arrange",
   TST_MPI_INTRA_COMM /* | TST_MPI_INTER_COMM */,
   1,
   /*TST_MPI_STANDARD_C_TYPES,*/
   TST_MPI_ALL_C_TYPES&
   ~(TST_MPI_TYPE_MIX_LB_UB),
   TST_MODE_RELAXED,
   TST_SYNC,            /* Needs sync due to bcast of filename in init */
   &tst_file_io_with_arrange_init, &tst_file_io_with_arrange_run, &tst_file_io_with_arrange_cleanup },


  {TST_CLASS_IO, "read_convert",
   TST_MPI_INTRA_COMM /* | TST_MPI_INTER_COMM */,
   1,
   /*TST_MPI_STANDARD_C_TYPES,*/
   TST_MPI_ALL_C_TYPES&
   ~(TST_MPI_TYPE_MIX_LB_UB),
   TST_MODE_RELAXED,
   TST_SYNC,            /* Needs sync due to bcast of filename in init */
   &tst_file_read_convert_init, &tst_file_read_convert_run, &tst_file_read_convert_cleanup },


  {TST_CLASS_IO, "io_atomic",
   TST_MPI_INTRA_COMM /* | TST_MPI_INTER_COMM */,
   1,
   /*TST_MPI_STANDARD_C_TYPES,*/
   TST_MPI_ALL_C_TYPES&
   ~(TST_MPI_TYPE_MIX_LB_UB),
   TST_MODE_RELAXED,
   TST_SYNC,            /* Needs sync due to bcast of filename in init */
   &tst_file_io_atomic_init, &tst_file_io_atomic_run, &tst_file_io_atomic_cleanup },


  {TST_CLASS_IO, "io_sync",
   TST_MPI_INTRA_COMM /* | TST_MPI_INTER_COMM */,
   1,
   /*TST_MPI_STANDARD_C_TYPES,*/
   TST_MPI_ALL_C_TYPES&
   ~(TST_MPI_TYPE_MIX_LB_UB),
   TST_MODE_RELAXED,
   TST_SYNC,            /* Needs sync due to bcast of filename in init */
   &tst_file_io_sync_init, &tst_file_io_sync_run, &tst_file_io_sync_cleanup },


  {TST_CLASS_IO, "asyncio_atomic",
   TST_MPI_INTRA_COMM /* | TST_MPI_INTER_COMM */,
   1,
   TST_MPI_ALL_C_TYPES&
   ~(TST_MPI_TYPE_MIX_LB_UB),
   TST_MODE_RELAXED,
   TST_SYNC,            /* Needs sync due to bcast of filename in init */
   &tst_file_asyncio_atomic_init, &tst_file_asyncio_atomic_run, &tst_file_asyncio_atomic_cleanup },


  {TST_CLASS_IO, "append_mode",
   TST_MPI_INTRA_COMM /* | TST_MPI_INTER_COMM */,
   1,
   TST_MPI_ALL_C_TYPES&
   ~(TST_MPI_TYPE_MIX_LB_UB),
   TST_MODE_RELAXED,
   TST_SYNC,            /* Needs sync due to bcast of filename in init */
   &tst_file_append_mode_init, &tst_file_append_mode_run, &tst_file_append_mode_cleanup },


  {TST_CLASS_IO, "io_commself",
   TST_MPI_INTRA_COMM /* | TST_MPI_INTER_COMM */,
   1,
   /*TST_MPI_STANDARD_C_TYPES,*/
   TST_MPI_ALL_C_TYPES&
   ~(TST_MPI_TYPE_MIX_LB_UB),
   TST_MODE_RELAXED,
   TST_SYNC,            /* Needs sync due to bcast of filename in init */
   &tst_file_io_commself_init, &tst_file_io_commself_run, &tst_file_io_commself_cleanup },


  {TST_CLASS_IO, "sequential_mode",
   TST_MPI_INTRA_COMM /* | TST_MPI_INTER_COMM */,
   1,
   /*TST_MPI_STANDARD_C_TYPES,*/
   TST_MPI_ALL_C_TYPES&
   ~(TST_MPI_TYPE_MIX_LB_UB),
   TST_MODE_RELAXED,
   TST_SYNC,            /* Needs sync due to bcast of filename in init */
   &tst_file_sequential_mode_init, &tst_file_sequential_mode_run, &tst_file_sequential_mode_cleanup },


  {TST_CLASS_IO, "set_size",
   TST_MPI_INTRA_COMM /* | TST_MPI_INTER_COMM */,
   1,
   /*TST_MPI_STANDARD_C_TYPES,*/
   TST_MPI_ALL_C_TYPES&
   ~(TST_MPI_TYPE_MIX_LB_UB),
   TST_MODE_RELAXED,
   TST_SYNC,            /* Needs sync due to bcast of filename in init */
   &tst_file_set_size_init, &tst_file_set_size_run, &tst_file_set_size_cleanup },


  {TST_CLASS_IO, "preallocate",
   TST_MPI_INTRA_COMM /* | TST_MPI_INTER_COMM */,
   1,
   /*TST_MPI_STANDARD_C_TYPES,*/
   TST_MPI_ALL_C_TYPES&
   ~(TST_MPI_TYPE_MIX_LB_UB),
   TST_MODE_RELAXED,
   TST_SYNC,            /* Needs sync due to bcast of filename in init */
   &tst_file_preallocate_init, &tst_file_preallocate_run, &tst_file_preallocate_cleanup },


  {TST_CLASS_IO, "io_with_hole2",
   TST_MPI_INTRA_COMM /* | TST_MPI_INTER_COMM */,
   1,
   /*TST_MPI_STANDARD_C_TYPES,*/
   TST_MPI_ALL_C_TYPES&
   ~(TST_MPI_TYPE_MIX_LB_UB),
   TST_MODE_RELAXED,
   TST_SYNC,            /* Needs sync due to bcast of filename in init */
   &tst_file_io_with_hole2_init, &tst_file_io_with_hole2_run, &tst_file_io_with_hole2_cleanup },
#endif /* HAVE_MPI2_IO */

  /*
   * Here come the real threaded tests
   */
#ifdef HAVE_MPI2_THREADS
  {TST_CLASS_THREADED, "Threaded ring",
   TST_MPI_COMM_SELF | TST_MPI_INTRA_COMM,
   1,
   TST_MPI_ALL_C_TYPES,
   TST_MODE_RELAXED,
   TST_NONE,
   &tst_threaded_ring_init, &tst_threaded_ring_run, &tst_threaded_ring_cleanup},


  {TST_CLASS_THREADED, "Threaded ring isend",
   TST_MPI_COMM_SELF | TST_MPI_INTRA_COMM,
   1,
   TST_MPI_ALL_C_TYPES,
   TST_MODE_RELAXED,
   TST_NONE,
   &tst_threaded_ring_isend_init, &tst_threaded_ring_isend_run, &tst_threaded_ring_isend_cleanup},


  {TST_CLASS_THREADED, "Threaded ring bsend",
   TST_MPI_COMM_SELF | TST_MPI_INTRA_COMM,
   1,
   TST_MPI_ALL_C_TYPES,
   TST_MODE_RELAXED,
   TST_NONE,
   &tst_threaded_ring_bsend_init, &tst_threaded_ring_bsend_run, &tst_threaded_ring_bsend_cleanup},


  {TST_CLASS_THREADED, "Threaded ring persistent",
   TST_MPI_COMM_SELF |  TST_MPI_INTRA_COMM,
   1,
   TST_MPI_ALL_C_TYPES,
   TST_MODE_RELAXED,
   TST_NONE,
   &tst_threaded_ring_persistent_init, &tst_threaded_ring_persistent_run, &tst_threaded_ring_persistent_cleanup},


  {TST_CLASS_THREADED, "Threaded bcast on duplicated comms",
   TST_MPI_COMM_SELF | TST_MPI_INTRA_COMM,
   1,
   TST_MPI_ALL_C_TYPES,
   TST_MODE_RELAXED,
   TST_NONE,
   &tst_threaded_comm_dup_init, &tst_threaded_comm_dup_run, &tst_threaded_comm_dup_cleanup},


  {TST_CLASS_THREADED, "Threaded ring partitioned",
   TST_MPI_COMM_SELF | TST_MPI_INTRA_COMM,
   1,
   TST_MPI_ALL_C_TYPES,
   TST_MODE_RELAXED,
   TST_NONE,
   &tst_threaded_ring_partitioned_init, &tst_threaded_ring_partitioned_run, &tst_threaded_ring_partitioned_cleanup},

  {TST_CLASS_THREADED, "Threaded ring partitioned with less receive partitions",
   TST_MPI_COMM_SELF | TST_MPI_INTRA_COMM,
   1,
   TST_MPI_ALL_C_TYPES,
   TST_MODE_RELAXED,
   TST_NONE,
   &tst_threaded_ring_partitioned_many_to_one_init, &tst_threaded_ring_partitioned_many_to_one_run, &tst_threaded_ring_partitioned_many_to_one_cleanup},
#endif

  {TST_CLASS_UNSPEC, "None",
   0,
   0,
   0,
   0,
   0,
   NULL, NULL, NULL}
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

int tst_test_cleanup (void)
{
  free (tst_tests_failed);

  return 0;
}

const char *tst_test_getclass_string(int i) {
  CHECK_ARG (i, NULL);

  /* XXX CN Can this internal check be implemented another way, because it can
   * easily be forgetten when adding a new calss ...
   */
  INTERNAL_CHECK
    (
     if (tst_tests[i].class != TST_CLASS_ENV &&
         tst_tests[i].class != TST_CLASS_P2P &&
         tst_tests[i].class != TST_CLASS_COLL &&
         tst_tests[i].class != TST_CLASS_ONE_SIDED &&
         tst_tests[i].class != TST_CLASS_DYNAMIC &&
         tst_tests[i].class != TST_CLASS_IO &&
         tst_tests[i].class != TST_CLASS_THREADED)
       ERROR (EINVAL, "Class of test is unknown");
     );
  /*
  printf ("tst_test_getclass_string: i:%d class:%d ffs():%d string:%s\n",
          i, tst_tests[i].class, ffs(tst_tests[i].class)-1,
          tst_test_class_strings[ffs (tst_tests[i].class)-1]);
  */
  return tst_test_class_strings[ffs (tst_tests[i].class)];
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


void *  tst_test_get_init_func (struct tst_env * env)
{
  CHECK_ARG (env->test, NULL);
  return tst_tests[env->test].tst_init_func;
}

void * tst_test_get_run_func (struct tst_env * env)
{
  CHECK_ARG (env->test, NULL);
  return tst_tests[env->test].tst_run_func;
}

void * tst_test_get_cleanup_func (struct tst_env * env)
{
  CHECK_ARG (env->test, NULL);
  return tst_tests[env->test].tst_cleanup_func;
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
      (tst_comm_getcommsize (env->comm) < tst_tests[env->test].min_comm_size) ||
      (tst_type_gettypeclass (env->type) & tst_tests[env->test].run_with_type) == (tst_uint64)0)
    {
      tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "(Rank:%d) env->comm:%d getcommclass:%d test is run_with_comm:%d "
                     "comm_size:%d min_comm_size:%d gettypeclass:%lld run_with_type:%d\n",
                     tst_global_rank, env->comm, tst_comm_getcommclass (env->comm),
                     tst_tests[env->test].run_with_comm,
                     tst_comm_getcommsize(env->comm), tst_tests[env->test].min_comm_size,
                     tst_type_gettypeclass (env->type), tst_tests[env->test].run_with_type);
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

  printf ("Num Tests : %d\n", TST_TESTS_NUM);
  for (i = 0; i < TST_TESTS_NUM; i++)
    printf ("%s test:%d %s\n",
            tst_test_getclass_string (i), i, tst_tests[i].description);

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
  return (k == test_list_num) ? -1 : k;
}

int tst_test_select(const char *test_string, int *test_list,
                    const int test_list_max, int *test_list_num) {
  int i;

  if (test_string == NULL || test_list == NULL || test_list_num == NULL) {
    ERROR(EINVAL, "Passed a NULL parameter");
  }

  /*
   * In case we match a complete class of tests, include every one!
   */
  for (i = 0; i < TST_TEST_CLASS_NUM; i++) {
    if (0 == strcasecmp(test_string, tst_test_class_strings[i])) {
      int j;
      int tst_class = i - 1;
      tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "test_string:%s matched with tst_test_class_strings[%d]:%s\n",
                 test_string, i, tst_test_class_strings[i]);
      for (j = 0; j < TST_TESTS_NUM; j++) {
        /*
         * First search for this test in the test_list -- if already in,
         * continue!
         */
        if (tst_tests[j].class & (1 << tst_class)) {
          if (-1 != tst_test_search(j, test_list, *test_list_num)) {
            tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "Test:%s selected through class:%s (already included)\n",
                         tst_tests[j].description,
                         tst_test_class_strings[tst_class]);
            continue;
          }
          test_list[*test_list_num] = j;
          (*test_list_num)++;
          tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "Test:%s selected through class:%s\n",
                       tst_tests[j].description,
                       tst_test_class_strings[tst_class]);
          if (*test_list_num >= test_list_max) {
            ERROR(EINVAL, "Too many user selected tests");
          }
        }
      }
      return 0;
    }
  }

  /*
   * In case we didn't match a complete class of tests, test for every single one...
   */
  for (i = 0; i < TST_TESTS_NUM; i++) {
    if (0 == strcasecmp(test_string, tst_tests[i].description)) {
      tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "test_string:%s matched with tst_tests[%d]:%s\n",
                   test_string, i, tst_tests[i].description);
      if (-1 != tst_test_search(i, test_list, *test_list_num)) {
        tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "Test:%s selected (already included)\n",
                     tst_tests[i].description);
        return 0;
      }
      test_list[*test_list_num] = i;
      (*test_list_num)++;
      if (*test_list_num >= test_list_max) {
        ERROR(EINVAL, "Too many user selected tests");
      }
      tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "Test:%s selected\n", tst_tests[i].description);
      return 0;
    }
  }

  {
    char buffer[128];
    sprintf(buffer, "Test %s not recognized", test_string);
    ERROR (EINVAL, buffer);
  }
  return 0;
}

int tst_test_deselect(const char *test_string, int *test_list,
                      const int test_list_max, int *test_list_num) {
  int i;

  if (test_string == NULL || test_list == NULL || test_list_num == NULL) {
    ERROR(EINVAL, "Passed a NULL parameter");
  }

  for (i = 0; i < TST_TEST_CLASS_NUM; i++) {
    /*
     * In case we match a complete class of tests, exclude every one!
     */
    if (0 == strcasecmp(test_string, tst_test_class_strings[i])) {
      int j;
      int tst_class = i - 1;
      tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "test_string:%s matched with tst_test_class_strings[%d]:%s\n",
                 test_string, i, tst_test_class_strings[i]);
      for (j = 0; j < TST_TESTS_NUM; j++) {
        int ret;
        /*
         * Search for this test in the test_list --
         * if it belongs to this class and is already included, deselect
         */
        if (((ret = tst_test_search(j, test_list, test_list_max)) != -1) &&
            tst_tests[j].class & (1 << tst_class)) {
          tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "test_string:%s test j:%d (1 << tst_class):%d with "
                     "class:%d matches for deselect, test_list_num:%d\n",
                     test_string, j, (1 << tst_class), tst_tests[j].class,
                     *test_list_num);
          test_list[ret] = -1;
          (*test_list_num)--;
          if (*test_list_num < 0)
            ERROR(EINVAL, "Negative selected tests: This should not happen");
        }
      }
      return 0;
    }
  }

  /*
   * In case we didn't match a complete class of tests, test for every single one...
   */
  for (i = 0; i < TST_TESTS_NUM; i++) {
    if (0 == strcasecmp(test_string, tst_tests[i].description)) {
      int ret;
      if ((ret = tst_test_search(i, test_list, *test_list_num)) == -1) {
        tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "Test:%s was not included in list -- not excluding\n",
                       tst_tests[i].description);
        return 0;
      }

      test_list[ret] = -1;
      (*test_list_num)--;
      if (*test_list_num < 0) {
        ERROR(EINVAL, "Negative selected tests: This should not happen");
      }
      tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "test_string:%s matched with test_list_num:%d excluding\n",
                   test_string, *test_list_num);

      return 0;
    }
  }

  {
    char buffer[128];
    sprintf(buffer, "Test %s not recognized", test_string);
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
  return 0;
}

int tst_test_get_failed_num (void)
{
  return tst_tests_failed_num;
}

int tst_test_print_failed (void)
{
  int i;
  printf ("Number of failed tests: %d\n", tst_tests_failed_num);
  if (tst_tests_failed_num > 0) {
    printf ("Summary of failed tests:\n");
    for (i = 0; i < tst_tests_failed_num; i++) {
        const int test = tst_tests_failed[i].test;
        const int comm = tst_tests_failed[i].comm;
        const int type = tst_tests_failed[i].type;
        const int values_num= tst_tests_failed[i].values_num;

        printf ("ERROR class:%s test:%s (%d), comm %s (%d), type %s (%d) number of values:%d\n",
                tst_test_getclass_string (test),
                tst_test_getdescription (test), test+1,
                tst_comm_getdescription (comm), comm+1,
                tst_type_getdescription (type), type+1, values_num);
      }
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

int tst_test_is_empty_status (MPI_Status * status)
{
    if (status->MPI_SOURCE == MPI_ANY_SOURCE &&
        status->MPI_TAG == MPI_ANY_TAG &&
        status->MPI_ERROR == MPI_SUCCESS)
      return 1;
    else
      return 0;
}
