#ifndef __MPI_TESTSUITE_H__
#define __MPI_TESTSUITE_H__
#include "config.h"
#include <stdio.h>
#ifdef HAVE_STDLIB_H
#  include <stdlib.h>
#endif
#ifdef HAVE_STRING_H
#  include <string.h>
#endif
#include <errno.h>
#ifdef HAVE_SYS_TYPES_H
#  include <sys/types.h>
#endif

#include "mpi.h"

/*
 * Global macros and defines
 */
#define DEBUG(x) x

#define WARNING(x) do { \
  if (!tst_global_rank) \
    x; \
  } while (0)

#define INTERNAL_CHECK(x) do { \
  } while (0)

#define ERROR(e,s) do {                               \
    fprintf (stderr, "(%s:%d) ERROR: %s; %s(%d)\n",   \
             __FILE__, __LINE__, (s), strerror(e), e);\
    exit (e);                                         \
  } while(0)

#define MPI_CHECK(x) do {                   \
    int __ret;                              \
    if ((__ret = (x)) != MPI_SUCCESS)       \
      ERROR (__ret, "MPI returned error");  \
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

/*
 * This probably should be found out another way...
 */
#define MPI_BUFFER_OVERHEAD 1024

#define TST_DESCRIPTION_LEN 48


#define TST_MPI_COMM_SELF   1 /* Though MPI_COMM_SELF is an intra-communicator, specify it's own class*/
#define TST_MPI_COMM_NULL   2
#define TST_MPI_INTRA_COMM  4
#define TST_MPI_INTER_COMM  8
#define TST_MPI_CART_COMM  16 /* Same for these */
#define TST_MPI_TOPO_COMM  32 /* Same for these */

#define TST_CLASS_ENV         1
#define TST_CLASS_P2P         2
#define TST_CLASS_COLL        4

/*
 * The internal translation of the used datatypes
 */
#define TST_MPI_CHAR              ((tst_uint64)0x1 << 0)
#define TST_MPI_UNSIGNED_CHAR     ((tst_uint64)0x1 << 1)
#define TST_MPI_BYTE              ((tst_uint64)0x1 << 2)
#define TST_MPI_SHORT             ((tst_uint64)0x1 << 3)
#define TST_MPI_UNSIGNED_SHORT    ((tst_uint64)0x1 << 4)
#define TST_MPI_INT               ((tst_uint64)0x1 << 5)
#define TST_MPI_UNSIGNED          ((tst_uint64)0x1 << 6)
#define TST_MPI_LONG              ((tst_uint64)0x1 << 7)
#define TST_MPI_UNSIGNED_LONG     ((tst_uint64)0x1 << 8)
#define TST_MPI_FLOAT             ((tst_uint64)0x1 << 9)
#define TST_MPI_DOUBLE            ((tst_uint64)0x1 << 10)
#define TST_MPI_LONG_DOUBLE       ((tst_uint64)0x1 << 11)
#define TST_MPI_LONG_LONG         ((tst_uint64)0x1 << 12)
#define TST_MPI_PACKED            ((tst_uint64)0x1 << 13)
/*#define TST_MPI_LB                ((tst_uint64)0x1 << 14)*/
/*#define TST_MPI_UB                ((tst_uint64)0x1 << 15)*/
#define TST_MPI_FLOAT_INT         ((tst_uint64)0x1 << 14)
#define TST_MPI_DOUBLE_INT        ((tst_uint64)0x1 << 15)
#define TST_MPI_LONG_INT          ((tst_uint64)0x1 << 16)
#define TST_MPI_SHORT_INT         ((tst_uint64)0x1 << 17)
#define TST_MPI_2INT              ((tst_uint64)0x1 << 18)
#define TST_MPI_LONG_DOUBLE_INT   ((tst_uint64)0x1 << 19)
#define TST_MPI_COMPLEX           ((tst_uint64)0x1 << 20)
#define TST_MPI_DOUBLE_COMPLEX    ((tst_uint64)0x1 << 21)
#define TST_MPI_LOGICAL           ((tst_uint64)0x1 << 22)
#define TST_MPI_REAL              ((tst_uint64)0x1 << 23)
#define TST_MPI_DOUBLE_PRECISION  ((tst_uint64)0x1 << 24)
#define TST_MPI_INTEGER           ((tst_uint64)0x1 << 25)
#define TST_MPI_2INTEGER          ((tst_uint64)0x1 << 26)
#define TST_MPI_2COMPLEX          ((tst_uint64)0x1 << 27)
#define TST_MPI_2DOUBLE_COMPLEX   ((tst_uint64)0x1 << 28)
#define TST_MPI_2REAL             ((tst_uint64)0x1 << 29)
#define TST_MPI_2DOUBLE_PRECISION ((tst_uint64)0x1 << 30)
#define TST_MPI_CHARACTER         ((tst_uint64)0x1 << 31)
#define TST_MPI_INT_CONTI         ((tst_uint64)0x1 << 32)
#define TST_MPI_INT_VECTOR        ((tst_uint64)0x1 << 33)
#define TST_MPI_INT_HVECTOR       ((tst_uint64)0x1 << 34)
#define TST_MPI_INT_INDEXED       ((tst_uint64)0x1 << 35)
#define TST_MPI_INT_HINDEXED      ((tst_uint64)0x1 << 36)
#define TST_MPI_INT_STRUCT        ((tst_uint64)0x1 << 37)
#define TST_MPI_TYPE_MIX          ((tst_uint64)0x1 << 38)
#define TST_MPI_TYPE_MIX_ARRAY    ((tst_uint64)0x1 << 39)


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
   TST_MPI_TYPE_MIX_ARRAY)


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
 * Synchronisation
 */
#define TST_NONE 0
#define TST_SYNC 1

/*
 * Global structures and variables
 */

#if SIZEOF_INT == 8
#  define tst_int64 int
#elif SIZEOF_LONG == 8
#  define tst_int64 long
#elif SIZEOF_LONG_LONG == 8
#  define tst_int64 long long
#else
#  error "No 8-Byte integer found"
#endif

struct tst_env {
  int comm;
  int type;
  int test;
  int values_num;
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

extern int tst_global_rank;
extern int tst_verbose;


/*
 * Global function definitions
 */

extern int tst_comm_init (int * num_comms);
extern MPI_Comm tst_comm_getcomm (int comm);
extern int tst_comm_getcommclass (int comm);
extern const char * tst_comm_getdescription (int comm);
extern void tst_comm_list (void);
extern int tst_comm_select (const char * comm_string,
                            int * comm_list, int * comm_list_num, const int comm_list_max);

extern int tst_test_init (int * num_tests);
extern const char * tst_test_getclass (int i);
extern const char * tst_test_getdescription (int i);
extern int tst_test_init_func (struct tst_env * env);
extern int tst_test_run_func (struct tst_env * env);
extern int tst_test_cleanup_func (struct tst_env * env);
extern int tst_test_check_run (struct tst_env * env);
extern int tst_test_check_sync (struct tst_env * env);
extern void tst_test_list (void);
extern int tst_test_select (const char * test_string,
                            int * test_list, int * test_list_num, const int test_list_max);

extern int tst_type_init (int * num_types);
extern MPI_Datatype tst_type_getdatatype (int type);
extern tst_uint64 tst_type_gettypeclass (int type);
extern const char * tst_type_getdescription (int type);
extern int tst_type_gettypesize (int type);
extern void tst_type_hexdump (const char * text, const char * data, int num);
extern int tst_type_setvalue (int type, char * buffer, int mode, long long direct_value);
extern int tst_type_cmpvalue (int type, const char * buffer1, const char * buffer2);
extern char * tst_type_allocvalues (const int type, const int values_num);
extern int tst_type_freevalues (const int type, char * buffer, const int values_num);
extern int tst_type_checkstandardarray (int type, int values_num, char * buffer, int comm_rank);
extern int tst_type_setstandardarray (int type, int values_num, char * buffer, int comm_rank);
extern void tst_type_list (void);
extern int tst_type_select (const char * type_string,
                            int * type_list, int * type_list_num, const int type_list_max);


extern int tst_hash_value (const struct tst_env * env);

/*
 * Test functions
 */
extern int tst_p2p_alltoall_init (const struct tst_env * env);
extern int tst_p2p_alltoall_run (const struct tst_env * env);
extern int tst_p2p_alltoall_cleanup (const struct tst_env * env);

extern int tst_p2p_alltoall_irsend_init (const struct tst_env * env);
extern int tst_p2p_alltoall_irsend_run (const struct tst_env * env);
extern int tst_p2p_alltoall_irsend_cleanup (const struct tst_env * env);

extern int tst_p2p_alltoall_issend_init (const struct tst_env * env);
extern int tst_p2p_alltoall_issend_run (const struct tst_env * env);
extern int tst_p2p_alltoall_issend_cleanup (const struct tst_env * env);

extern int tst_p2p_alltoall_probe_anysource_init (const struct tst_env * env);
extern int tst_p2p_alltoall_probe_anysource_run (const struct tst_env * env);
extern int tst_p2p_alltoall_probe_anysource_cleanup (const struct tst_env * env);

extern int tst_p2p_alltoall_sendrecv_init (const struct tst_env * env);
extern int tst_p2p_alltoall_sendrecv_run (const struct tst_env * env);
extern int tst_p2p_alltoall_sendrecv(const struct tst_env * env);

extern int tst_p2p_direct_partner_intercomm_init (const struct tst_env * env);
extern int tst_p2p_direct_partner_intercomm_run (const struct tst_env * env);
extern int tst_p2p_direct_partner_intercomm_cleanup (const struct tst_env * env);

extern int tst_p2p_many_to_one_init (const struct tst_env * env);
extern int tst_p2p_many_to_one_run (const struct tst_env * env);
extern int tst_p2p_many_to_one_cleanup (const struct tst_env * env);

extern int tst_p2p_many_to_one_probe_anysource_init (const struct tst_env * env);
extern int tst_p2p_many_to_one_probe_anysource_run (const struct tst_env * env);
extern int tst_p2p_many_to_one_probe_anysource_cleanup (const struct tst_env * env);

extern int tst_p2p_many_to_one_iprobe_anysource_init (const struct tst_env * env);
extern int tst_p2p_many_to_one_iprobe_anysource_run (const struct tst_env * env);
extern int tst_p2p_many_to_one_iprobe_anysource_cleanup (const struct tst_env * env);

extern int tst_p2p_simple_ring_init (const struct tst_env * env);
extern int tst_p2p_simple_ring_run (const struct tst_env * env);
extern int tst_p2p_simple_ring_cleanup (const struct tst_env * env);

extern int tst_p2p_simple_ring_bsend_init (const struct tst_env * env);
extern int tst_p2p_simple_ring_bsend_run (const struct tst_env * env);
extern int tst_p2p_simple_ring_bsend_cleanup (const struct tst_env * env);

extern int tst_p2p_simple_ring_isend_init (const struct tst_env * env);
extern int tst_p2p_simple_ring_isend_run (const struct tst_env * env);
extern int tst_p2p_simple_ring_isend_cleanup (const struct tst_env * env);

extern int tst_p2p_simple_ring_ssend_init (const struct tst_env * env);
extern int tst_p2p_simple_ring_ssend_run (const struct tst_env * env);
extern int tst_p2p_simple_ring_ssend_cleanup (const struct tst_env * env);

extern int tst_p2p_simple_ring_sendrecv_init (const struct tst_env * env);
extern int tst_p2p_simple_ring_sendrecv_run (const struct tst_env * env);
extern int tst_p2p_simple_ring_sendrecv_cleanup (const struct tst_env * env);

extern int tst_coll_bcast_init (const struct tst_env * env);
extern int tst_coll_bcast_run (const struct tst_env * env);
extern int tst_coll_bcast_cleanup (const struct tst_env * env);

extern int tst_coll_gather_init (const struct tst_env * env);
extern int tst_coll_gather_run (const struct tst_env * env);
extern int tst_coll_gather_cleanup (const struct tst_env * env);

extern int tst_coll_scatter_init (const struct tst_env * env);
extern int tst_coll_scatter_run (const struct tst_env * env);
extern int tst_coll_scatter_cleanup (const struct tst_env * env);

extern int tst_coll_scatterv_init (const struct tst_env * env);
extern int tst_coll_scatterv_run (const struct tst_env * env);
extern int tst_coll_scatterv_cleanup (const struct tst_env * env);

extern int tst_coll_scatterv_stride_init (const struct tst_env * env);
extern int tst_coll_scatterv_stride_run (const struct tst_env * env);
extern int tst_coll_scatterv_stride_cleanup (const struct tst_env * env);

extern int tst_coll_allreduce_init (const struct tst_env * env);
extern int tst_coll_allreduce_run (const struct tst_env * env);
extern int tst_coll_allreduce_cleanup (const struct tst_env * env);

extern int tst_coll_alltoall_init (const struct tst_env * env);
extern int tst_coll_alltoall_run (const struct tst_env * env);
extern int tst_coll_alltoall_cleanup (const struct tst_env * env);

#endif /* __MPI_TESTSUITE_H__ */
