#ifndef __MPI_TESTSUITE_H__
#define __MPI_TESTSUITE_H__

#include "config.h"

#include <stdio.h>
#include <errno.h>
#include <unistd.h>

#ifdef HAVE_STDLIB_H
#  include <stdlib.h>
#endif
#ifdef HAVE_STRING_H
#  include <string.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#  include <sys/types.h>
#endif

#include <mpi.h>
#include "tst_output.h"

/****************************************************************************/
/**                                                                        **/
/**                     DEFINITIONS AND MACROS                             **/
/**                                                                        **/
/****************************************************************************/

#define DEFAULT_INIT_BYTE 0xa5

#define WARNING(x) do { \
  if (!tst_global_rank) \
    x; \
  } while (0)

#define INTERNAL_CHECK(x) do { \
  x;\
  } while (0)


#define ERROR(e,s) do {                                           \
    int __local_error = (e);                                            \
    fprintf (stderr, "(%s:%d) ERROR: %s; %s(%d)\n",               \
             __FILE__, __LINE__, (s), strerror(__local_error), __local_error);\
    tst_output_printf (DEBUG_LOG, TST_REPORT_SUMMARY, "(%s:%d) ERROR: %s; %s(%d)\n", \
             __FILE__, __LINE__, (s), strerror(__local_error), __local_error);\
    tst_output_close (DEBUG_LOG);                                 \
    exit (__local_error);                                               \
  } while(0)

#define MPI_CHECK(x)                                                           \
  do {                                                                         \
    int __ret = (x);                                                           \
    if (MPI_SUCCESS != __ret) {                                                \
      char err_string[MPI_MAX_ERROR_STRING];                                   \
      int err_string_len = 0;                                                  \
      MPI_Error_string(__ret, err_string, &err_string_len);                    \
      fprintf(stderr, "(%s:%d) ERROR: MPI call returned error code %d (%s)",   \
              __FILE__, __LINE__, __ret, err_string);                          \
    }                                                                          \
  } while (0)

#if SIZEOF_INT == 8
#  define tst_int64 int
#  define tst_uint64 unsigned int
#elif SIZEOF_LONG == 8
#  define tst_int64 long
#  define tst_uint64 unsigned long
#elif SIZEOF_LONG_LONG == 8
#  define tst_int64 long long
#  define tst_uint64 unsigned long long
#else
#  error "No 8-Byte integer found"
#endif

#define TST_DESCRIPTION_LEN 48

#define TST_SUCESS 0
#define TST_ERROR -1

/*
 * Global definitions for the io tests
 */
#define TST_FILE_NAME "tst_io_test_file"
#define ATOM_MODE 0
#define NO_ATOM_MODE 1

/*
 * Definitions of the internal representation for the mpi communicators
 */
#define TST_MPI_COMM_SELF   1 /* Though MPI_COMM_SELF is an intra-communicator, specify it's own class*/
#define TST_MPI_COMM_NULL   2
#define TST_MPI_INTRA_COMM  4
#define TST_MPI_INTER_COMM  8
#define TST_MPI_CART_COMM  16 /* Same for these */
#define TST_MPI_TOPO_COMM  32 /* Same for these */
#define TST_MPI_SHARED_COMM 64 /* Same for these */

/*
 * Definition of the internal representations for the test classes
 */
#define TST_CLASS_UNSPEC      0
#define TST_CLASS_ENV         1
#define TST_CLASS_P2P         2
#define TST_CLASS_COLL        4
#define TST_CLASS_ONE_SIDED   8
#define TST_CLASS_DYNAMIC    16
#define TST_CLASS_IO         32
#define TST_CLASS_THREADED   64

#define ROOT 0

/*
 * The internal translation of the used datatypes
 */
#define TST_MPI_CHAR              ((tst_uint64)0x1 << 0)
#define TST_MPI_UNSIGNED_CHAR     ((tst_uint64)0x1 << 1)
#define TST_MPI_SIGNED_CHAR       ((tst_uint64)0x1 << 2)
#define TST_MPI_BYTE              ((tst_uint64)0x1 << 3)
#define TST_MPI_SHORT             ((tst_uint64)0x1 << 4)
#define TST_MPI_UNSIGNED_SHORT    ((tst_uint64)0x1 << 5)
#define TST_MPI_INT               ((tst_uint64)0x1 << 6)
#define TST_MPI_UNSIGNED          ((tst_uint64)0x1 << 7)
#define TST_MPI_LONG              ((tst_uint64)0x1 << 8)
#define TST_MPI_UNSIGNED_LONG     ((tst_uint64)0x1 << 9)
#define TST_MPI_FLOAT             ((tst_uint64)0x1 << 10)
#define TST_MPI_DOUBLE            ((tst_uint64)0x1 << 11)
#define TST_MPI_LONG_DOUBLE       ((tst_uint64)0x1 << 12)
#define TST_MPI_LONG_LONG         ((tst_uint64)0x1 << 13)
#define TST_MPI_PACKED            ((tst_uint64)0x1 << 14)
/*#define TST_MPI_LB                ((tst_uint64)0x1 << 15)*/
/*#define TST_MPI_UB                ((tst_uint64)0x1 << 16)*/
#define TST_MPI_FLOAT_INT         ((tst_uint64)0x1 << 15)
#define TST_MPI_DOUBLE_INT        ((tst_uint64)0x1 << 16)
#define TST_MPI_LONG_INT          ((tst_uint64)0x1 << 17)
#define TST_MPI_SHORT_INT         ((tst_uint64)0x1 << 18)
#define TST_MPI_2INT              ((tst_uint64)0x1 << 19)
#define TST_MPI_LONG_DOUBLE_INT   ((tst_uint64)0x1 << 20)
#define TST_MPI_COMPLEX           ((tst_uint64)0x1 << 21)
#define TST_MPI_DOUBLE_COMPLEX    ((tst_uint64)0x1 << 22)
#define TST_MPI_LOGICAL           ((tst_uint64)0x1 << 23)
#define TST_MPI_REAL              ((tst_uint64)0x1 << 24)
#define TST_MPI_DOUBLE_PRECISION  ((tst_uint64)0x1 << 25)
#define TST_MPI_INTEGER           ((tst_uint64)0x1 << 26)
#define TST_MPI_2INTEGER          ((tst_uint64)0x1 << 27)
#define TST_MPI_2COMPLEX          ((tst_uint64)0x1 << 28)
#define TST_MPI_2DOUBLE_COMPLEX   ((tst_uint64)0x1 << 29)
#define TST_MPI_2REAL             ((tst_uint64)0x1 << 30)
#define TST_MPI_2DOUBLE_PRECISION ((tst_uint64)0x1 << 31)
#define TST_MPI_CHARACTER         ((tst_uint64)0x1 << 32)
#define TST_MPI_INT_CONTI         ((tst_uint64)0x1 << 33)
#define TST_MPI_INT_VECTOR        ((tst_uint64)0x1 << 34)
#define TST_MPI_INT_HVECTOR       ((tst_uint64)0x1 << 35)
#define TST_MPI_INT_INDEXED       ((tst_uint64)0x1 << 36)
#define TST_MPI_INT_HINDEXED      ((tst_uint64)0x1 << 37)
#define TST_MPI_INT_STRUCT        ((tst_uint64)0x1 << 38)
#define TST_MPI_TYPE_MIX          ((tst_uint64)0x1 << 39)
#define TST_MPI_TYPE_MIX_ARRAY    ((tst_uint64)0x1 << 40)
#define TST_MPI_TYPE_MIX_LB_UB    ((tst_uint64)0x1 << 41)

#ifdef HAVE_MPI2
  /*
   * Include the TST_MPI_SIGNED_CHAR
   */
#define TST_MPI_STANDARD_C_INT_TYPES \
  (TST_MPI_CHAR | \
   TST_MPI_UNSIGNED_CHAR | \
   TST_MPI_SIGNED_CHAR | \
   TST_MPI_BYTE | \
   TST_MPI_SHORT | \
   TST_MPI_UNSIGNED_SHORT | \
   TST_MPI_INT | \
   TST_MPI_UNSIGNED | \
   TST_MPI_LONG | \
   TST_MPI_UNSIGNED_LONG | \
   TST_MPI_LONG_LONG)
#else
#define TST_MPI_STANDARD_C_INT_TYPES \
  (TST_MPI_CHAR | \
   TST_MPI_UNSIGNED_CHAR | \
   TST_MPI_BYTE | \
   TST_MPI_SHORT | \
   TST_MPI_UNSIGNED_SHORT | \
   TST_MPI_INT | \
   TST_MPI_UNSIGNED | \
   TST_MPI_LONG | \
   TST_MPI_UNSIGNED_LONG | \
   TST_MPI_LONG_LONG)
#endif


#define TST_MPI_STANDARD_C_FLOAT_TYPES \
  (TST_MPI_FLOAT | \
   TST_MPI_DOUBLE | \
   TST_MPI_LONG_DOUBLE)

#define TST_MPI_STANDARD_C_TYPES \
  (TST_MPI_STANDARD_C_INT_TYPES | \
   TST_MPI_STANDARD_C_FLOAT_TYPES)

#define TST_MPI_STRUCT_C_TYPES \
  (TST_MPI_FLOAT_INT | \
   TST_MPI_DOUBLE_INT | \
   TST_MPI_LONG_INT | \
   TST_MPI_SHORT_INT | \
   TST_MPI_2INT | \
   TST_MPI_LONG_DOUBLE_INT)

#define TST_MPI_ALL_C_TYPES \
  (TST_MPI_STANDARD_C_TYPES | \
   TST_MPI_STRUCT_C_TYPES | \
   TST_MPI_DERIVED_C_INT_TYPES | \
   TST_MPI_DERIVED_MIXED_TYPES)


#define TST_MPI_DERIVED_C_INT_TYPES \
  (TST_MPI_INT_CONTI | \
   TST_MPI_INT_VECTOR | \
   TST_MPI_INT_HVECTOR| \
   TST_MPI_INT_INDEXED| \
   TST_MPI_INT_HINDEXED | \
   TST_MPI_INT_STRUCT)

#define TST_MPI_DERIVED_MIXED_TYPES \
  (TST_MPI_TYPE_MIX | \
   TST_MPI_TYPE_MIX_ARRAY | \
   TST_MPI_TYPE_MIX_LB_UB)


#define TST_MPI_STANDARD_FORTRAN_INT_TYPES TST_MPI_INTEGER

#define TST_MPI_STANDARD_FORTRAN_FLOAT_TYPES \
   (TST_MPI_REAL | \
    TST_MPI_DOUBLE_PRECISION)

#define TST_MPI_STANDARD_FORTRAN_COMPLEX_TYPES \
   (TST_MPI_COMPLEX | \
    TST_MPI_DOUBLE_COMPLEX)

#define TST_MPI_STANDARD_FORTRAN_TYPES \
  (TST_MPI_CHARACTER | \
   TST_MPI_STANDARD_FORTRAN_INT_TYPES | \
   TST_MPI_STANDARD_FORTRAN_FLOAT_TYPES | \
   TST_MPI_STANDARD_FORTRAN_COMPLEX_TYPES | \
   TST_MPI_LOGICAL)

#define TST_MPI_STRUCT_FORTRAN_TYPES \
  (TST_MPI_2INTEGER | \
   TST_MPI_2COMPLEX | \
   TST_MPI_2DOUBLE_COMPLEX | \
   TST_MPI_2REAL | \
   TST_MPI_2DOUBLE_PRECISION)

#define TST_MPI_ALL_FORTRAN_TYPES \
  (TST_MPI_STANDARD_FORTRAN_TYPES | \
   TST_MPI_STRUCT_FORTRAN_TYPES)


#define TST_TYPE_SET_ZERO  0
#define TST_TYPE_SET_MAX   1
#define TST_TYPE_SET_MIN   2
#define TST_TYPE_SET_VALUE 3

/*
 * Synchronization
 */
#define TST_NONE 0
#define TST_SYNC 1


/****************************************************************************/
/**                                                                        **/
/**                     TYPEDEFS AND STRUCTURES                            **/
/**                                                                        **/
/****************************************************************************/

struct tst_env {
  int test;
  int values_num;
  int type;
  int tag;
  int comm;
  char * send_buffer;
  char * send_pack_buffer;
  char * recv_buffer;
  char * recv_pack_buffer;
  char ** send_buffer_array;
  char ** recv_buffer_array;
  MPI_Request * req_buffer;
  MPI_Status * status_buffer;
  int * neighbors;
  char * mpi_buffer;
  int mpi_buffer_size;
  char * check_buffer;
  int * cancelled;
  MPI_Datatype extra_type_send;
  int position;
  int * send_to;
  int * recv_from;
  int * send_counts;
  int * send_displs;
  char * read_buffer;
};

struct tst_mpi_float_int {
  float a;
  int b;
};


struct tst_mpi_double_int {
  double a;
  int b;
};

struct tst_mpi_long_int {
  long a;
  int b;
};

struct tst_mpi_short_int {
  short a;
  int b;
};

struct tst_mpi_2int {
  int a;
  int b;
};

struct tst_mpi_long_double_int {
  long double a;
  int b;
};

struct tst_mpi_type_mix {
    char a;
    short b;
    int c;
    long d;
    float e;
    double f;
    struct tst_mpi_float_int g;
    struct tst_mpi_double_int h;
    struct tst_mpi_long_int i;
    struct tst_mpi_short_int j;
    struct tst_mpi_2int k;
};

#define TST_MPI_TYPE_MIX_ARRAY_NUM 10
struct tst_mpi_type_mix_array {
    char a[TST_MPI_TYPE_MIX_ARRAY_NUM];
    short b[TST_MPI_TYPE_MIX_ARRAY_NUM];
    int c[TST_MPI_TYPE_MIX_ARRAY_NUM];
    long d[TST_MPI_TYPE_MIX_ARRAY_NUM];
    float e[TST_MPI_TYPE_MIX_ARRAY_NUM];
    double f[TST_MPI_TYPE_MIX_ARRAY_NUM];
};

/****************************************************************************/
/**                                                                        **/
/**                     EXPORTED VARIABLES                                 **/
/**                                                                        **/
/****************************************************************************/

#ifndef _MPI_TEST_SUITE_C_SRC
#endif

extern int tst_global_rank;
extern int tst_global_size;
extern int tst_atomic;

extern const char * tst_reports[];
extern tst_report_types tst_report;

extern const char * tst_modes[];
typedef enum {
  TST_MODE_DISABLED=0,      /* Just disable this test for the moment (or read: Run the disabled!) */
  TST_MODE_STRICT,          /* Check for anal stuff in MPI-standard */
  TST_MODE_RELAXED,         /* Leave out tests for strict conformance to MPI standard */
  TST_MODE_MAX
} tst_mode_types;
extern tst_mode_types tst_mode;


struct tst_thread_env_t; /* Just a forward declaration */

#define TST_TESTS_NUM_FAILED_MAX 1000

/****************************************************************************/
/**                                                                        **/
/**                     EXPORTED FUNCTIONS                                 **/
/**                                                                        **/
/****************************************************************************/
extern int tst_comm_cleanup (void);
extern MPI_Comm tst_comm_getcomm (int comm);
extern int tst_comm_getcommclass (int comm);
extern int tst_comm_getcommsize (int comm);
extern const char * tst_comm_getdescription (int comm);
extern void tst_comm_list (void);
extern int tst_comm_select (const char * comm_string,
                            int * comm_list, const int comm_list_max, int * comm_list_num);
extern int tst_comm_deselect (const char * comm_string,
                            int * comm_list, const int comm_list_max, int * comm_list_num);

extern int tst_test_init (int * num_tests);
extern int tst_test_cleanup (void);
extern const char * tst_test_getclass (int i);
extern const char * tst_test_getclass_string(int i);
extern const char * tst_test_getdescription (int i);
extern int tst_test_getmode (int i);
extern int tst_test_init_func (struct tst_env * env);
extern int tst_test_run_func (struct tst_env * env);
extern int tst_test_cleanup_func (struct tst_env * env);
void *  tst_test_get_init_func (struct tst_env * env);
void *  tst_test_get_run_func (struct tst_env * env);
void *  tst_test_get_cleanup_func (struct tst_env * env);
extern int tst_test_check_run (struct tst_env * env);
extern int tst_test_check_sync (struct tst_env * env);
extern void tst_test_list (void);
extern int tst_test_select (const char * test_string,
                            int * test_list, const int test_list_max, int * test_list_num);
extern int tst_test_deselect (const char * test_string,
                              int * test_list, const int test_list_max, int * test_list_num);

extern int tst_test_checkstandardarray (const struct tst_env * env,
                                        char * buffer,
                                        int comm_rank);
extern int tst_test_is_empty_status (MPI_Status * status);
extern int tst_test_recordfailure (const struct tst_env * env);
extern int tst_test_print_failed (void);
extern int tst_test_get_failed_num (void);


extern int tst_type_init (int * num_types);
extern int tst_type_cleanup (void);
extern MPI_Datatype tst_type_getdatatype (int type);
extern tst_uint64 tst_type_gettypeclass (int type);
extern const char * tst_type_getdescription (int type);
extern int tst_type_gettypesize (int type);
/*
 * XXX Niethammer: removed because it was declared static in tst_types.c and nowhere other used.
 * extern int tst_type_gettypelb (int type);
 * */
extern void tst_type_hexdump (const char * text, const char * data, int num);
extern int tst_type_setvalue (int type, char * buffer, int mode, long long direct_value);
extern int tst_type_cmpvalue (int type, const char * buffer1, const char * buffer2);
extern char * tst_type_allocvalues (const int type, const int values_num);
extern int tst_type_freevalues (const int type, char * buffer, const int values_num);
extern int tst_type_checkstandardarray (int type, int values_num, char * buffer, int comm_rank);
extern int tst_type_setstandardarray (int type, int values_num, char * buffer, int comm_rank);
extern int tst_type_getstandardarray_size (int type, int values_num, MPI_Aint * size);
extern void tst_type_list (void);
extern int tst_type_select (const char * type_string,
                            int * type_list, const int type_list_max, int * type_list_num);
extern int tst_type_deselect (const char * type_string,
                            int * type_list, const int type_list_max, int * type_list_num);

extern int tst_type_compare(const MPI_Datatype type1, const MPI_Datatype type2);
extern int tst_hash_value (const struct tst_env * env);


/*
 * Test functions
 * First the environment functions
 */
extern int tst_env_status_check_init (struct tst_env * env);
extern int tst_env_status_check_run (struct tst_env * env);
extern int tst_env_status_check_cleanup (struct tst_env * env);

extern int tst_env_request_null_init (struct tst_env * env);
extern int tst_env_request_null_run (struct tst_env * env);
extern int tst_env_request_null_cleanup (struct tst_env * env);

extern int tst_env_type_dup_init (struct tst_env * env);
extern int tst_env_type_dup_run (struct tst_env * env);
extern int tst_env_type_dup_cleanup (struct tst_env * env);

extern int tst_env_get_version_init (struct tst_env * env);
extern int tst_env_get_version_run (struct tst_env * env);
extern int tst_env_get_version_cleanup (struct tst_env * env);

/*
 * Following all p2p-functions
 */
extern int tst_p2p_alltoall_init (struct tst_env * env);
extern int tst_p2p_alltoall_run (struct tst_env * env);
extern int tst_p2p_alltoall_cleanup (struct tst_env * env);

extern int tst_p2p_alltoall_persistent_init (struct tst_env * env);
extern int tst_p2p_alltoall_persistent_run (struct tst_env * env);
extern int tst_p2p_alltoall_persistent_cleanup (struct tst_env * env);

extern int tst_p2p_alltoall_xisend_init (struct tst_env * env);
extern int tst_p2p_alltoall_xisend_run (struct tst_env * env);
extern int tst_p2p_alltoall_xisend_cleanup (struct tst_env * env);

extern int tst_p2p_alltoall_irsend_init (struct tst_env * env);
extern int tst_p2p_alltoall_irsend_run (struct tst_env * env);
extern int tst_p2p_alltoall_irsend_cleanup (struct tst_env * env);

extern int tst_p2p_alltoall_issend_init (struct tst_env * env);
extern int tst_p2p_alltoall_issend_run (struct tst_env * env);
extern int tst_p2p_alltoall_issend_cleanup (struct tst_env * env);

extern int tst_p2p_alltoall_probe_anysource_init (struct tst_env * env);
extern int tst_p2p_alltoall_probe_anysource_run (struct tst_env * env);
extern int tst_p2p_alltoall_probe_anysource_cleanup (struct tst_env * env);

extern int tst_p2p_alltoall_sendrecv_init (struct tst_env * env);
extern int tst_p2p_alltoall_sendrecv_run (struct tst_env * env);
extern int tst_p2p_alltoall_sendrecv_cleanup (struct tst_env * env);

extern int tst_p2p_alltoall_graph_init (struct tst_env * env);
extern int tst_p2p_alltoall_graph_run (struct tst_env * env);
extern int tst_p2p_alltoall_graph_cleanup (struct tst_env * env);

extern int tst_p2p_direct_partner_intercomm_init (struct tst_env * env);
extern int tst_p2p_direct_partner_intercomm_run (struct tst_env * env);
extern int tst_p2p_direct_partner_intercomm_cleanup (struct tst_env * env);

extern int tst_p2p_many_to_one_init (struct tst_env * env);
extern int tst_p2p_many_to_one_run (struct tst_env * env);
extern int tst_p2p_many_to_one_cleanup (struct tst_env * env);

extern int tst_p2p_many_to_one_probe_anysource_init (struct tst_env * env);
extern int tst_p2p_many_to_one_probe_anysource_run (struct tst_env * env);
extern int tst_p2p_many_to_one_probe_anysource_cleanup (struct tst_env * env);

extern int tst_p2p_many_to_one_iprobe_anysource_init (struct tst_env * env);
extern int tst_p2p_many_to_one_iprobe_anysource_run (struct tst_env * env);
extern int tst_p2p_many_to_one_iprobe_anysource_cleanup (struct tst_env * env);

extern int tst_p2p_simple_ring_init (struct tst_env * env);
extern int tst_p2p_simple_ring_run (struct tst_env * env);
extern int tst_p2p_simple_ring_cleanup (struct tst_env * env);

extern int tst_p2p_simple_ring_bottom_init (struct tst_env * env);
extern int tst_p2p_simple_ring_bottom_run (struct tst_env * env);
extern int tst_p2p_simple_ring_bottom_cleanup (struct tst_env * env);

extern int tst_p2p_simple_ring_pack_init (struct tst_env * env);
extern int tst_p2p_simple_ring_pack_run (struct tst_env * env);
extern int tst_p2p_simple_ring_pack_cleanup (struct tst_env * env);

extern int tst_p2p_simple_ring_bsend_init (struct tst_env * env);
extern int tst_p2p_simple_ring_bsend_run (struct tst_env * env);
extern int tst_p2p_simple_ring_bsend_cleanup (struct tst_env * env);

extern int tst_p2p_simple_ring_isend_init (struct tst_env * env);
extern int tst_p2p_simple_ring_isend_run (struct tst_env * env);
extern int tst_p2p_simple_ring_isend_cleanup (struct tst_env * env);

extern int tst_p2p_simple_ring_ibsend_init (struct tst_env * env);
extern int tst_p2p_simple_ring_ibsend_run (struct tst_env * env);
extern int tst_p2p_simple_ring_ibsend_cleanup (struct tst_env * env);

extern int tst_p2p_simple_ring_irsend_init (struct tst_env * env);
extern int tst_p2p_simple_ring_irsend_run (struct tst_env * env);
extern int tst_p2p_simple_ring_irsend_cleanup (struct tst_env * env);

extern int tst_p2p_simple_ring_issend_init (struct tst_env * env);
extern int tst_p2p_simple_ring_issend_run (struct tst_env * env);
extern int tst_p2p_simple_ring_issend_cleanup (struct tst_env * env);

extern int tst_p2p_simple_ring_rsend_init (struct tst_env * env);
extern int tst_p2p_simple_ring_rsend_run (struct tst_env * env);
extern int tst_p2p_simple_ring_rsend_cleanup (struct tst_env * env);

extern int tst_p2p_simple_ring_ssend_init (struct tst_env * env);
extern int tst_p2p_simple_ring_ssend_run (struct tst_env * env);
extern int tst_p2p_simple_ring_ssend_cleanup (struct tst_env * env);

extern int tst_p2p_simple_ring_sendrecv_init (struct tst_env * env);
extern int tst_p2p_simple_ring_sendrecv_run (struct tst_env * env);
extern int tst_p2p_simple_ring_sendrecv_cleanup (struct tst_env * env);

extern int tst_p2p_simple_ring_shift_init (struct tst_env * env);
extern int tst_p2p_simple_ring_shift_run (struct tst_env * env);
extern int tst_p2p_simple_ring_shift_cleanup (struct tst_env * env);

extern int tst_p2p_simple_ring_xsend_init (struct tst_env * env);
extern int tst_p2p_simple_ring_xsend_run (struct tst_env * env);
extern int tst_p2p_simple_ring_xsend_cleanup (struct tst_env * env);

extern int tst_p2p_many_to_one_isend_cancel_init (struct tst_env * env);
extern int tst_p2p_many_to_one_isend_cancel_run (struct tst_env * env);
extern int tst_p2p_many_to_one_isend_cancel_cleanup (struct tst_env * env);

extern int tst_p2p_simple_ring_persistent_init (struct tst_env * env);
extern int tst_p2p_simple_ring_persistent_run (struct tst_env * env);
extern int tst_p2p_simple_ring_persistent_cleanup (struct tst_env * env);

extern int tst_coll_bcast_init (struct tst_env * env);
extern int tst_coll_bcast_run (struct tst_env * env);
extern int tst_coll_bcast_cleanup (struct tst_env * env);

extern int tst_coll_gather_init (struct tst_env * env);
extern int tst_coll_gather_run (struct tst_env * env);
extern int tst_coll_gather_cleanup (struct tst_env * env);

extern int tst_coll_allgather_init (struct tst_env * env);
extern int tst_coll_allgather_run (struct tst_env * env);
extern int tst_coll_allgather_cleanup (struct tst_env * env);

extern int tst_coll_allgather_in_place_init (struct tst_env * env);
extern int tst_coll_allgather_in_place_run (struct tst_env * env);
extern int tst_coll_allgather_in_place_cleanup (struct tst_env * env);

extern int tst_coll_reduce_max_cleanup (struct tst_env * env);
extern int tst_coll_reduce_max_run (struct tst_env * env);
extern int tst_coll_reduce_max_init (struct tst_env * env);

extern int tst_coll_reduce_min_cleanup (struct tst_env * env);
extern int tst_coll_reduce_min_run (struct tst_env * env);
extern int tst_coll_reduce_min_init (struct tst_env * env);

extern int tst_coll_reduce_in_place_max_cleanup (struct tst_env * env);
extern int tst_coll_reduce_in_place_max_run (struct tst_env * env);
extern int tst_coll_reduce_in_place_max_init (struct tst_env * env);

extern int tst_coll_reduce_in_place_min_cleanup (struct tst_env * env);
extern int tst_coll_reduce_in_place_min_run (struct tst_env * env);
extern int tst_coll_reduce_in_place_min_init (struct tst_env * env);

extern int tst_coll_scan_sum_init (struct tst_env * env);
extern int tst_coll_scan_sum_run (struct tst_env * env);
extern int tst_coll_scan_sum_cleanup (struct tst_env * env);

extern int tst_coll_scatter_init (struct tst_env * env);
extern int tst_coll_scatter_run (struct tst_env * env);
extern int tst_coll_scatter_cleanup (struct tst_env * env);

extern int tst_coll_scatterv_init (struct tst_env * env);
extern int tst_coll_scatterv_run (struct tst_env * env);
extern int tst_coll_scatterv_cleanup (struct tst_env * env);

extern int tst_coll_scatterv_stride_init (struct tst_env * env);
extern int tst_coll_scatterv_stride_run (struct tst_env * env);
extern int tst_coll_scatterv_stride_cleanup (struct tst_env * env);

extern int tst_coll_allreduce_max_init (struct tst_env * env);
extern int tst_coll_allreduce_max_run (struct tst_env * env);
extern int tst_coll_allreduce_max_cleanup (struct tst_env * env);

extern int tst_coll_allreduce_min_init (struct tst_env * env);
extern int tst_coll_allreduce_min_run (struct tst_env * env);
extern int tst_coll_allreduce_min_cleanup (struct tst_env * env);

extern int tst_coll_allreduce_sum_init (struct tst_env * env);
extern int tst_coll_allreduce_sum_run (struct tst_env * env);
extern int tst_coll_allreduce_sum_cleanup (struct tst_env * env);

extern int tst_coll_allreduce_quadsum_init (struct tst_env * env);
extern int tst_coll_allreduce_quadsum_run (struct tst_env * env);
extern int tst_coll_allreduce_quadsum_cleanup (struct tst_env * env);

extern int tst_coll_alltoall_init (struct tst_env * env);
extern int tst_coll_alltoall_run (struct tst_env * env);
extern int tst_coll_alltoall_cleanup (struct tst_env * env);

extern int tst_establish_communication_init (struct tst_env * env);
extern int tst_establish_communication_run (struct tst_env * env);
extern int tst_establish_communication_cleanup (struct tst_env * env);

extern int tst_comm_spawn_init (struct tst_env * env);
extern int tst_comm_spawn_run (struct tst_env * env);
extern int tst_comm_spawn_cleanup (struct tst_env * env);

extern int tst_comm_spawn_multiple_init (struct tst_env * env);
extern int tst_comm_spawn_multiple_run (struct tst_env * env);
extern int tst_comm_spawn_multiple_cleanup (struct tst_env * env);

extern int tst_get_with_fence_alltoall_init (struct tst_env * env);
extern int tst_get_with_fence_alltoall_run (struct tst_env * env);
extern int tst_get_with_fence_alltoall_cleanup (struct tst_env * env);

extern int tst_put_with_fence_alltoall_init (struct tst_env * env);
extern int tst_put_with_fence_alltoall_run (struct tst_env * env);
extern int tst_put_with_fence_alltoall_cleanup (struct tst_env * env);

extern int tst_accumulate_with_fence_sum_init (struct tst_env * env);
extern int tst_accumulate_with_fence_sum_run (struct tst_env * env);
extern int tst_accumulate_with_fence_sum_cleanup (struct tst_env * env);

extern int tst_get_with_post_alltoall_init (struct tst_env * env);
extern int tst_get_with_post_alltoall_run (struct tst_env * env);
extern int tst_get_with_post_alltoall_cleanup (struct tst_env * env);

extern int tst_put_with_post_alltoall_init (struct tst_env * env);
extern int tst_put_with_post_alltoall_run (struct tst_env * env);
extern int tst_put_with_post_alltoall_cleanup (struct tst_env * env);

extern int tst_accumulate_with_post_min_init (struct tst_env * env);
extern int tst_accumulate_with_post_min_run (struct tst_env * env);
extern int tst_accumulate_with_post_min_cleanup (struct tst_env * env);

extern int tst_get_with_lock_alltoall_init (struct tst_env * env);
extern int tst_get_with_lock_alltoall_run (struct tst_env * env);
extern int tst_get_with_lock_alltoall_cleanup (struct tst_env * env);

extern int tst_put_with_lock_alltoall_init (struct tst_env * env);
extern int tst_put_with_lock_alltoall_run (struct tst_env * env);
extern int tst_put_with_lock_alltoall_cleanup (struct tst_env * env);

extern int tst_accumulate_with_lock_max_init (struct tst_env * env);
extern int tst_accumulate_with_lock_max_run (struct tst_env * env);
extern int tst_accumulate_with_lock_max_cleanup (struct tst_env * env);

extern int tst_file_alloc (int type, const int values_num, const int comm_size,
                             char file_name[100], const MPI_Comm comm);
extern int tst_file_check (int type, const int values_num, const int comm_size,
                             char file_name[100], const MPI_Comm comm);
extern int tst_file_simple_init (struct tst_env * env);
extern int tst_file_simple_run (struct tst_env * env);
extern int tst_file_simple_cleanup (struct tst_env * env);

extern int tst_file_read_at_init (struct tst_env * env);
extern int tst_file_read_at_run (struct tst_env * env);
extern int tst_file_read_at_cleanup (struct tst_env * env);

extern int tst_file_write_at_init (struct tst_env * env);
extern int tst_file_write_at_run (struct tst_env * env);
extern int tst_file_write_at_cleanup (struct tst_env * env);

extern int tst_file_iread_at_init (struct tst_env * env);
extern int tst_file_iread_at_run (struct tst_env * env);
extern int tst_file_iread_at_cleanup (struct tst_env * env);

extern int tst_file_iwrite_at_init (struct tst_env * env);
extern int tst_file_iwrite_at_run (struct tst_env * env);
extern int tst_file_iwrite_at_cleanup (struct tst_env * env);

extern int tst_file_read_at_all_init (struct tst_env * env);
extern int tst_file_read_at_all_run (struct tst_env * env);
extern int tst_file_read_at_all_cleanup (struct tst_env * env);

extern int tst_file_write_at_all_init (struct tst_env * env);
extern int tst_file_write_at_all_run (struct tst_env * env);
extern int tst_file_write_at_all_cleanup (struct tst_env * env);

extern int tst_file_read_at_all_begin_init (struct tst_env * env);
extern int tst_file_read_at_all_begin_run (struct tst_env * env);
extern int tst_file_read_at_all_begin_cleanup (struct tst_env * env);

extern int tst_file_write_at_all_begin_init (struct tst_env * env);
extern int tst_file_write_at_all_begin_run (struct tst_env * env);
extern int tst_file_write_at_all_begin_cleanup (struct tst_env * env);

extern int tst_file_read_init (struct tst_env * env);
extern int tst_file_read_run (struct tst_env * env);
extern int tst_file_read_cleanup (struct tst_env * env);

extern int tst_file_write_init (struct tst_env * env);
extern int tst_file_write_run (struct tst_env * env);
extern int tst_file_write_cleanup (struct tst_env * env);

extern int tst_file_iread_init (struct tst_env * env);
extern int tst_file_iread_run (struct tst_env * env);
extern int tst_file_iread_cleanup (struct tst_env * env);

extern int tst_file_iwrite_init (struct tst_env * env);
extern int tst_file_iwrite_run (struct tst_env * env);
extern int tst_file_iwrite_cleanup (struct tst_env * env);

extern int tst_file_read_all_init (struct tst_env * env);
extern int tst_file_read_all_run (struct tst_env * env);
extern int tst_file_read_all_cleanup (struct tst_env * env);

extern int tst_file_write_all_init (struct tst_env * env);
extern int tst_file_write_all_run (struct tst_env * env);
extern int tst_file_write_all_cleanup (struct tst_env * env);

extern int tst_file_read_all_begin_init (struct tst_env * env);
extern int tst_file_read_all_begin_run (struct tst_env * env);
extern int tst_file_read_all_begin_cleanup (struct tst_env * env);

extern int tst_file_write_all_begin_init (struct tst_env * env);
extern int tst_file_write_all_begin_run (struct tst_env * env);
extern int tst_file_write_all_begin_cleanup (struct tst_env * env);

extern int tst_file_read_shared_init (struct tst_env * env);
extern int tst_file_read_shared_run (struct tst_env * env);
extern int tst_file_read_shared_cleanup (struct tst_env * env);

extern int tst_file_write_shared_init (struct tst_env * env);
extern int tst_file_write_shared_run (struct tst_env * env);
extern int tst_file_write_shared_cleanup (struct tst_env * env);

extern int tst_file_iread_shared_init (struct tst_env * env);
extern int tst_file_iread_shared_run (struct tst_env * env);
extern int tst_file_iread_shared_cleanup (struct tst_env * env);

extern int tst_file_iwrite_shared_init (struct tst_env * env);
extern int tst_file_iwrite_shared_run (struct tst_env * env);
extern int tst_file_iwrite_shared_cleanup (struct tst_env * env);

extern int tst_file_read_ordered_init (struct tst_env * env);
extern int tst_file_read_ordered_run (struct tst_env * env);
extern int tst_file_read_ordered_cleanup (struct tst_env * env);

extern int tst_file_write_ordered_init (struct tst_env * env);
extern int tst_file_write_ordered_run (struct tst_env * env);
extern int tst_file_write_ordered_cleanup (struct tst_env * env);

extern int tst_file_read_ordered_begin_init (struct tst_env * env);
extern int tst_file_read_ordered_begin_run (struct tst_env * env);
extern int tst_file_read_ordered_begin_cleanup (struct tst_env * env);

extern int tst_file_write_ordered_begin_init (struct tst_env * env);
extern int tst_file_write_ordered_begin_run (struct tst_env * env);
extern int tst_file_write_ordered_begin_cleanup (struct tst_env * env);

extern int tst_file_read_darray_init (struct tst_env * env);
extern int tst_file_read_darray_run (struct tst_env * env);
extern int tst_file_read_darray_cleanup (struct tst_env * env);

extern int tst_file_write_darray_init (struct tst_env * env);
extern int tst_file_write_darray_run (struct tst_env * env);
extern int tst_file_write_darray_cleanup (struct tst_env * env);

extern int tst_file_read_subarray_init (struct tst_env * env);
extern int tst_file_read_subarray_run (struct tst_env * env);
extern int tst_file_read_subarray_cleanup (struct tst_env * env);

extern int tst_file_write_subarray_init (struct tst_env * env);
extern int tst_file_write_subarray_run (struct tst_env * env);
extern int tst_file_write_subarray_cleanup (struct tst_env * env);

extern int tst_file_io_with_hole_init (struct tst_env * env);
extern int tst_file_io_with_hole_run (struct tst_env * env);
extern int tst_file_io_with_hole_cleanup (struct tst_env * env);

extern int tst_file_io_with_arrange_init (struct tst_env * env);
extern int tst_file_io_with_arrange_run (struct tst_env * env);
extern int tst_file_io_with_arrange_cleanup (struct tst_env * env);

extern int tst_file_read_convert_init (struct tst_env * env);
extern int tst_file_read_convert_run (struct tst_env * env);
extern int tst_file_read_convert_cleanup (struct tst_env * env);

extern int tst_file_io_atomic_init (struct tst_env * env);
extern int tst_file_io_atomic_run (struct tst_env * env);
extern int tst_file_io_atomic_cleanup (struct tst_env * env);

extern int tst_file_io_sync_init (struct tst_env * env);
extern int tst_file_io_sync_run (struct tst_env * env);
extern int tst_file_io_sync_cleanup (struct tst_env * env);

extern int tst_file_asyncio_atomic_init (struct tst_env * env);
extern int tst_file_asyncio_atomic_run (struct tst_env * env);
extern int tst_file_asyncio_atomic_cleanup (struct tst_env * env);

extern int tst_file_append_mode_init (struct tst_env * env);
extern int tst_file_append_mode_run (struct tst_env * env);
extern int tst_file_append_mode_cleanup (struct tst_env * env);

extern int tst_file_io_commself_init (struct tst_env * env);
extern int tst_file_io_commself_run (struct tst_env * env);
extern int tst_file_io_commself_cleanup (struct tst_env * env);

extern int tst_file_sequential_mode_init (struct tst_env * env);
extern int tst_file_sequential_mode_run (struct tst_env * env);
extern int tst_file_sequential_mode_cleanup (struct tst_env * env);

extern int tst_file_set_size_init (struct tst_env * env);
extern int tst_file_set_size_run (struct tst_env * env);
extern int tst_file_set_size_cleanup (struct tst_env * env);

extern int tst_file_preallocate_init (struct tst_env * env);
extern int tst_file_preallocate_run (struct tst_env * env);
extern int tst_file_preallocate_cleanup (struct tst_env * env);

extern int tst_file_io_with_hole2_init (struct tst_env * env);
extern int tst_file_io_with_hole2_run (struct tst_env * env);
extern int tst_file_io_with_hole2_cleanup (struct tst_env * env);

extern int tst_coll_allreduce_init (struct tst_env * env);
extern int tst_coll_allreduce_run (struct tst_env * env);
extern int tst_coll_allreduce_cleanup (struct tst_env * env);

extern int tst_coll_allreduce_in_place_init (struct tst_env * env);
extern int tst_coll_allreduce_in_place_run (struct tst_env * env);
extern int tst_coll_allreduce_in_place_cleanup (struct tst_env * env);

#ifdef HAVE_MPI2_ONE_SIDED
extern int tst_one_sided_simple_ring_get_init (struct tst_env * env);
extern int tst_one_sided_simple_ring_get_run (struct tst_env * env);
extern int tst_one_sided_simple_ring_get_cleanup (struct tst_env * env);

extern int tst_one_sided_simple_ring_get_post_init (struct tst_env * env);
extern int tst_one_sided_simple_ring_get_post_run (struct tst_env * env);
extern int tst_one_sided_simple_ring_get_post_cleanup (struct tst_env * env);

extern int tst_one_sided_simple_ring_put_init (struct tst_env * env);
extern int tst_one_sided_simple_ring_put_run (struct tst_env * env);
extern int tst_one_sided_simple_ring_put_cleanup (struct tst_env * env);
#endif

#ifdef HAVE_MPI2_THREADS
extern int tst_threaded_ring_init (struct tst_env * env);
extern int tst_threaded_ring_run (struct tst_env * env);
extern int tst_threaded_ring_cleanup (struct tst_env * env);

extern int tst_threaded_ring_isend_init (struct tst_env * env);
extern int tst_threaded_ring_isend_run (struct tst_env * env);
extern int tst_threaded_ring_isend_cleanup (struct tst_env * env);

extern int tst_threaded_ring_bsend_init (struct tst_env * env);
extern int tst_threaded_ring_bsend_run (struct tst_env * env);
extern int tst_threaded_ring_bsend_cleanup (struct tst_env * env);

extern int tst_threaded_ring_persistent_init (struct tst_env * env);
extern int tst_threaded_ring_persistent_run (struct tst_env * env);
extern int tst_threaded_ring_persistent_cleanup (struct tst_env * env);

extern int tst_threaded_comm_dup_init (struct tst_env * env);
extern int tst_threaded_comm_dup_run (struct tst_env * env);
extern int tst_threaded_comm_dup_cleanup (struct tst_env * env);

extern int tst_threaded_ring_partitioned_init (struct tst_env * env);
extern int tst_threaded_ring_partitioned_run (struct tst_env * env);
extern int tst_threaded_ring_partitioned_cleanup (struct tst_env * env);

extern int tst_threaded_ring_partitioned_many_to_one_init (struct tst_env * env);
extern int tst_threaded_ring_partitioned_many_to_one_run (struct tst_env * env);
extern int tst_threaded_ring_partitioned_many_to_one_cleanup (struct tst_env * env);

#endif

#endif /* __MPI_TESTSUITE_H__ */
