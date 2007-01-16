#include "config.h"
#ifdef HAVE_VALUES_H
#  include <values.h>
#endif
#ifdef HAVE_LIMITS_H
#  include <limits.h>
#endif
#ifdef HAVE_FLOAT_H
#  include <float.h>
#endif
#ifdef HAVE_STRING_H
#  include <string.h>
#endif
#ifdef HAVE_STRINGS_H
#  include <strings.h>
#endif
#include "mpi.h"
#include "mpi_test_suite.h"

#ifndef LLONG_MAX
#   define LLONG_MAX    9223372036854775807LL
#   define LLONG_MIN    (-LLONG_MAX - 1LL)
#endif

/*
#ifdef __USE_ISOC99
#error "ISOC99 defined"
#endif
*/

#undef DEBUG
#define DEBUG(x)

#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

#define MAX_TYPES 128                   /* One of the largest type_maaings is for MPI_TYPE_MIX_ARRAY */
#define TYPES_NUM_REPEAT   7
#define OVERHEAD 8
#define DEFAULT_INIT_BYTE 0xa5

#define CHECK_ARG(i,ret) do {           \
    if ((i) < 0 || (i) > TST_TYPES_NUM) \
      return (ret);                     \
  } while (0)

#define TST_TYPES_NUM (sizeof (types) / sizeof (types[0]))
#define TST_TYPES_CLASS_NUM (sizeof (tst_types_class_strings) / sizeof (tst_types_class_strings[0]))
#define SIZEOF_TYPE_MIX_LB_UB (sizeof(char)+sizeof(short)+sizeof(int)+sizeof(long)+sizeof(float)+sizeof(double))

/*
 * Local static Functions
 */
static int tst_type_gettypelb (int type);
static int tst_type_gettypemax_log (int type);
static int tst_type_search (const int search_test, const int * test_list, const int test_list_num);

/*
 * Local variables and declaration
 */
struct tst_type_class {
  char * class_string;
  tst_uint64 class;
};

static const struct tst_type_class tst_types_class_strings [] =
  {
    {"STANDARD_C_INT_TYPES", TST_MPI_STANDARD_C_INT_TYPES},
    {"STANDARD_C_FLOAT_TYPES", TST_MPI_STANDARD_C_FLOAT_TYPES},
    {"STANDARD_C_TYPES", TST_MPI_STANDARD_C_TYPES},
    {"STRUCT_C_TYPES", TST_MPI_STRUCT_C_TYPES},
    {"ALL_C_TYPES", TST_MPI_ALL_C_TYPES},
/*    {"STANDARD_FORTRAN_INT_TYPES", TST_MPI_STANDARD_FORTRAN_INT_TYPES},
    {"STANDARD_FORTRAN_FLOAT_TYPES", TST_MPI_STANDARD_FORTRAN_FLOAT_TYPES},
    {"STANDARD_FORTRAN_COMPLEX_TYPES", TST_MPI_STANDARD_FORTRAN_COMPLEX_TYPES},
    {"STANDARD_FORTRAN_TYPES", TST_MPI_STANDARD_FORTRAN_TYPES},
    {"STRUCT_FORTRAN_TYPES",  TST_MPI_STRUCT_FORTRAN_TYPES},
    {"ALL_FORTRAN_TYPES", TST_MPI_ALL_FORTRAN_TYPES} */
  };


struct type {
  MPI_Datatype mpi_datatype;
  char description [TST_DESCRIPTION_LEN];
  MPI_Aint lb;
  MPI_Aint ub;
  tst_uint64 type_class;
  int type_num;
  int type_mapping[MAX_TYPES];
};

static struct type types[32] = {
/* Standard C Types */
      {MPI_CHAR,              "MPI_CHAR",             0, sizeof (char), TST_MPI_CHAR, 1, {TST_MPI_CHAR}},
      {MPI_UNSIGNED_CHAR,     "MPI_UNSIGNED_CHAR",    0, sizeof (unsigned char), TST_MPI_UNSIGNED_CHAR, 1, {TST_MPI_UNSIGNED_CHAR}},
#ifdef HAVE_MPI2
      {MPI_SIGNED_CHAR,       "MPI_SIGNED_CHAR",      0, sizeof (signed char), TST_MPI_SIGNED_CHAR, 1, {TST_MPI_SIGNED_CHAR}},
#else
      {MPI_DATATYPE_NULL,     "MPI_SIGNED_CHAR n/a",  0, 0, 0, 0, {0}},
#endif
      {MPI_BYTE,              "MPI_BYTE",             0, sizeof (char), TST_MPI_BYTE, 1, {TST_MPI_BYTE}},
      {MPI_SHORT,             "MPI_SHORT",            0, sizeof (short), TST_MPI_SHORT, 1, {TST_MPI_SHORT}},
/*5*/ {MPI_UNSIGNED_SHORT,    "MPI_UNSIGNED_SHORT",   0, sizeof (unsigned short), TST_MPI_UNSIGNED_SHORT, 1, {TST_MPI_UNSIGNED_SHORT}},
      {MPI_INT,               "MPI_INT",              0, sizeof (int), TST_MPI_INT, 1, {TST_MPI_INT}},
      {MPI_UNSIGNED,          "MPI_UNSIGNED",         0, sizeof (unsigned int), TST_MPI_UNSIGNED, 1, {TST_MPI_UNSIGNED}},
      {MPI_LONG,              "MPI_LONG",             0, sizeof (long), TST_MPI_LONG, 1, {TST_MPI_LONG}},
      {MPI_UNSIGNED_LONG,     "MPI_UNSIGNED_LONG",    0, sizeof (unsigned long), TST_MPI_UNSIGNED_LONG, 1, {TST_MPI_UNSIGNED_LONG}},
/*10*/{MPI_FLOAT,             "MPI_FLOAT",            0, sizeof (float), TST_MPI_FLOAT, 1, {TST_MPI_FLOAT}},
      {MPI_DOUBLE,            "MPI_DOUBLE",           0, sizeof (double), TST_MPI_DOUBLE, 1, {TST_MPI_DOUBLE}},
#if defined(HAVE_LONG_DOUBLE) && defined(LDBL_MAX)
      {MPI_LONG_DOUBLE,       "MPI_LONG_DOUBLE",      0, sizeof (long double), TST_MPI_LONG_DOUBLE, 1, {TST_MPI_LONG_DOUBLE}},
#else
      {MPI_DATATYPE_NULL,     "MPI_LONG_DOUBLE n/a",  0, 0, 0, 0, {0}},
#endif
#if defined(HAVE_C_MPI_LONG_LONG_INT)
      {MPI_LONG_LONG,         "MPI_LONG_LONG",        0, sizeof (long long int), TST_MPI_LONG_LONG, 1, {TST_MPI_LONG_LONG}},
#else
      {MPI_DATATYPE_NULL,     "MPI_LONG_LONG n/a",    0, 0, 0, 0, {0}},
#endif /* HAVE_C_MPI_LONG_LONG_INT */
      /*  {MPI_PACKED,          "MPI_PACKED",           0, 0, 0, NULL},
          {MPI_LB,                "MPI_LB",               0, sizeof (), 1, {TST_}},
          {MPI_UB,                "MPI_UB",               0, sizeof (), 1, {TST_}},*/
      /* Composed C types */
      {MPI_FLOAT_INT,         "MPI_FLOAT_INT",        0, sizeof (struct tst_mpi_float_int), TST_MPI_FLOAT_INT, 1, {TST_MPI_FLOAT_INT}},
/*15*/{MPI_DOUBLE_INT,        "MPI_DOUBLE_INT",       0, sizeof (struct tst_mpi_double_int), TST_MPI_DOUBLE_INT, 1, {TST_MPI_DOUBLE_INT}},
      {MPI_LONG_INT,          "MPI_LONG_INT",         0, sizeof (struct tst_mpi_long_int), TST_MPI_LONG_INT, 1, {TST_MPI_LONG_INT}},
      {MPI_SHORT_INT,         "MPI_SHORT_INT",        0, sizeof (struct tst_mpi_short_int), TST_MPI_SHORT_INT, 1, {TST_MPI_SHORT_INT}},
      {MPI_2INT,              "MPI_2INT",             0, sizeof (struct tst_mpi_2int), TST_MPI_2INT, 1, {TST_MPI_2INT}},
#if defined(HAVE_LONG_DOUBLE) && defined (LDBL_MAX)
      {MPI_LONG_DOUBLE_INT,   "MPI_LONG_DOUBLE_INT",  0, sizeof (struct tst_mpi_long_double_int), TST_MPI_LONG_DOUBLE_INT, 1, {TST_MPI_LONG_DOUBLE_INT}},
#else
      {MPI_DATATYPE_NULL,     "MPI_LONG_DOUBLE_INT n/a", 0, 0, 0, 0, {0}},
#endif /* HAVE_MPI_LONG_DOUBLE */

#define PREDEFINED_DATATYPES 20
      /*
       * First of all a representation of 7 MPI_INTs with different MPI_Type-creation calls
       */
/*20*/{MPI_DATATYPE_NULL,     "MPI_CONTIGUOUS_INT",   0, 7*sizeof(int), TST_MPI_INT_CONTI, 7, {TST_MPI_INT}},
      {MPI_DATATYPE_NULL,     "MPI_VECTOR_INT",       0, 7*sizeof(int), TST_MPI_INT_VECTOR, 7, {TST_MPI_INT}},
      {MPI_DATATYPE_NULL,     "MPI_HVECTOR_INT",      0, 7*sizeof(int), TST_MPI_INT_HVECTOR, 7, {TST_MPI_INT}},
      {MPI_DATATYPE_NULL,     "MPI_INDEXED_INT",      0, 7*sizeof(int), TST_MPI_INT_INDEXED, 7, {TST_MPI_INT}},
      {MPI_DATATYPE_NULL,     "MPI_HINDEXED_INT",     0, 7*sizeof(int), TST_MPI_INT_HINDEXED, 7, {TST_MPI_INT}},
/*25*/{MPI_DATATYPE_NULL,     "MPI_STRUCT_INT",       0, 7*sizeof(int), TST_MPI_INT_STRUCT, 7, {TST_MPI_INT}},
      /*
       * Now, more complex derived MPI_Datattypes
       */
      {MPI_DATATYPE_NULL,     "MPI_TYPE_MIX",         0, sizeof(struct tst_mpi_type_mix), TST_MPI_TYPE_MIX, 11, {TST_MPI_INT}},
      {MPI_DATATYPE_NULL,     "MPI_TYPE_MIX_ARRAY",   0, sizeof(struct tst_mpi_type_mix_array), TST_MPI_TYPE_MIX_ARRAY, 6, {TST_MPI_INT}},
#ifndef HAVE_MPI_NECSX
      {MPI_DATATYPE_NULL,     "MPI_TYPE_MIX_LB_UB",   0, 0, TST_MPI_TYPE_MIX_LB_UB, 6, {TST_MPI_INT}},
#endif

      /*
       * Two examples for MPI_Type_dup usage on a predefined- and a derived datatype.
       */
#if defined(HAVE_MPI2)
      {MPI_DATATYPE_NULL,     "Dup MPI_CHAR",            0, sizeof (char), TST_MPI_CHAR, 1, {TST_MPI_CHAR}},
#ifndef HAVE_MPI_NECSX
/*30*/{MPI_DATATYPE_NULL,     "Dup MPI_TYPE_MIX_LB_UB",  0, 0, TST_MPI_TYPE_MIX_LB_UB, 6, {TST_MPI_INT}},
#endif

#else
      {MPI_DATATYPE_NULL,     "function MPI_Type_dup() n/a", 0, 0, 0, 0, {0}},
      {MPI_DATATYPE_NULL,     "function MPI_Type_dup() n/a", 0, 0, 0, 0, {0}},

#endif /* HAVE_MPI2 */

    /* Fortran Types */
    /*  {MPI_COMPLEX,           "MPI_COMPLEX",         0, sizeof (complex), 1, {TST_}},
      {MPI_DOUBLE_COMPLEX,    "MPI_DOUBLE_COMPLEX",    0, sizeof (), 1, {TST_}},
      {MPI_LOGICAL,           "MPI_LOGICAL",           0, sizeof (), 1, {TST_}},
      {MPI_REAL,              "MPI_REAL",              0, sizeof (), 1, {TST_}},
      {MPI_DOUBLE_PRECISION,  "MPI_DOUBLE_PRECISION",  0, sizeof (), 1, {TST_}},
      {MPI_INTEGER,           "MPI_INTEGER",           0, sizeof (), 1, {TST_}},
      {MPI_2INTEGER,          "MPI_2INTEGER",          0, sizeof (), 1, {TST_}},
      {MPI_2COMPLEX,          "MPI_2COMPLEX",          0, sizeof (), 1, {TST_}},
      {MPI_2DOUBLE_COMPLEX,   "MPI_2DOUBLE_COMPLEX",   0, sizeof (), 1, {TST_}},
      {MPI_2REAL,             "MPI_2REAL",             0, sizeof (), 1, {TST_}},
      {MPI_2DOUBLE_PRECISION, "MPI_2DOUBLE_PRECISION", 0, sizeof (), 1, {TST_}},
      {MPI_CHARACTER,         "MPI_CHARACTER",         0, sizeof (), 1, {TST_}}*/
      /*
      * The last element
      */
      {MPI_DATATYPE_NULL,     "",                     0, 0, 0, 0, {TST_MPI_INT}}
};

int tst_type_init (int * num_types)
{
  int i;
  int num = PREDEFINED_DATATYPES;

  {
    /*
     * MPI_CONTIGUOUS_INT
     */
    MPI_Type_contiguous (TYPES_NUM_REPEAT, MPI_INT, &(types[num].mpi_datatype));
    MPI_Type_commit (&(types[num].mpi_datatype));
    types[num].type_num = TYPES_NUM_REPEAT;
    for(i=0 ; i < TYPES_NUM_REPEAT; i++)
      types[num].type_mapping[i] = TST_MPI_INT;
    num++;
  }

  {
    /*
     * MPI_VECTOR_INT
     */
    MPI_Type_vector (TYPES_NUM_REPEAT, 1, 1, MPI_INT, &(types[num].mpi_datatype));
    MPI_Type_commit (&(types[num].mpi_datatype));
    types[num].type_num = TYPES_NUM_REPEAT;
    for(i=0; i < TYPES_NUM_REPEAT; i++)
      types[num].type_mapping[i] = TST_MPI_INT;
    num++;
  }

  {
    /*
     * MPI_HVECTOR_INT
     */
    MPI_Type_hvector (TYPES_NUM_REPEAT, 1, sizeof(int), MPI_INT, &(types[num].mpi_datatype));
    MPI_Type_commit (&(types[num].mpi_datatype));
    types[num].type_num = TYPES_NUM_REPEAT;
    for(i=0; i < TYPES_NUM_REPEAT; i++)
      types[num].type_mapping[i] = TST_MPI_INT;
    num++;
  }

  {
    /*
     * MPI_INDEXED_INT
     */
    int block[TYPES_NUM_REPEAT];
    int dis[TYPES_NUM_REPEAT];
    for(i=0; i < TYPES_NUM_REPEAT; i++) {
      block[i] = 1;
      dis[i] = i;
    }
    MPI_Type_indexed (TYPES_NUM_REPEAT, block, dis, MPI_INT, &(types[num].mpi_datatype));
    MPI_Type_commit (&(types[num].mpi_datatype));
    types[num].type_num = TYPES_NUM_REPEAT;
    for(i=0; i < TYPES_NUM_REPEAT; i++)
      types[num].type_mapping[i]=TST_MPI_INT;
    num++;
  }

  /*
   * Intel-MPI1.0 segfaults with this datatype
   */
#if !defined(HAVE_MPI_INTEL10) && !defined(HAVE_MPI_INTEL20)
  {
    /*
     * MPI_HINDEXED_INT
     */
    int block[TYPES_NUM_REPEAT];
    MPI_Aint dis[TYPES_NUM_REPEAT];
    for(i=0; i < TYPES_NUM_REPEAT; i++) {
      block[i] = 1;
      dis[i] = i*sizeof(int);
    }
    MPI_Type_hindexed (TYPES_NUM_REPEAT, block, dis, MPI_INT, &(types[num].mpi_datatype));
    MPI_Type_commit (&(types[num].mpi_datatype));
    types[num].type_num = TYPES_NUM_REPEAT;
    for(i=0 ; i < TYPES_NUM_REPEAT; i++)
      types[num].type_mapping[i]=TST_MPI_INT;
    num++;
  }
#else
  {
    types[num].mpi_datatype = MPI_DATATYPE_NULL;
    num++;
  }
#endif

  {
    /*
     * MPI_STRUCT_INT
     */
    int block_struct[1];
    MPI_Aint dis_struct[1];
    MPI_Datatype dtype[1];

    block_struct[0] = TYPES_NUM_REPEAT;
    dis_struct[0] = 0;
    dtype[0] = MPI_INT;

    MPI_Type_struct (1, block_struct, dis_struct, dtype, &(types[num].mpi_datatype));
    MPI_Type_commit (&(types[num].mpi_datatype));

    types[num].type_num = TYPES_NUM_REPEAT;
    for(i=0; i < TYPES_NUM_REPEAT; i++)
      types[num].type_mapping[i] = TST_MPI_INT;
    num++;
  }

#ifndef HAVE_MPI_INTEL20
  {
    /*
     * MPI_TYPE_MIX
     */
    int block_mix[11];
    MPI_Aint disp_mix[11];
    MPI_Datatype mix_type[11] = {MPI_CHAR, MPI_SHORT, MPI_INT, MPI_LONG,
                                 MPI_FLOAT, MPI_DOUBLE, MPI_FLOAT_INT,
                                 MPI_DOUBLE_INT, MPI_LONG_INT, MPI_SHORT_INT, MPI_2INT};
    MPI_Aint mix_base;
    struct tst_mpi_type_mix type_tmp;

    MPI_Address (&(type_tmp.a), disp_mix);
    MPI_Address (&(type_tmp.b), disp_mix+1);
    MPI_Address (&(type_tmp.c), disp_mix+2);
    MPI_Address (&(type_tmp.d), disp_mix+3);
    MPI_Address (&(type_tmp.e), disp_mix+4);
    MPI_Address (&(type_tmp.f), disp_mix+5);
    MPI_Address (&(type_tmp.g), disp_mix+6);
    MPI_Address (&(type_tmp.h), disp_mix+7);
    MPI_Address (&(type_tmp.i), disp_mix+8);
    MPI_Address (&(type_tmp.j), disp_mix+9);
    MPI_Address (&(type_tmp.k), disp_mix+10);
    mix_base = disp_mix[0];
    for(i=0; i < 11; i++) disp_mix[i] -= mix_base;
    for(i=0; i < 11; i++) block_mix[i] = 1;
    MPI_Type_struct (11, block_mix, disp_mix, mix_type, &(types[num].mpi_datatype));
    MPI_Type_commit (&(types[num].mpi_datatype));
    types[num].type_num = 11;
    types[num].type_mapping[0] = TST_MPI_CHAR;
    types[num].type_mapping[1] = TST_MPI_SHORT;
    types[num].type_mapping[2] = TST_MPI_INT;
    types[num].type_mapping[3] = TST_MPI_LONG;
    types[num].type_mapping[4] = TST_MPI_FLOAT;
    types[num].type_mapping[5] = TST_MPI_DOUBLE;
    types[num].type_mapping[6] = TST_MPI_FLOAT_INT;
    types[num].type_mapping[7] = TST_MPI_DOUBLE_INT;
    types[num].type_mapping[8] = TST_MPI_LONG_INT;
    types[num].type_mapping[9] = TST_MPI_SHORT_INT;
    types[num].type_mapping[10] = TST_MPI_2INT;
    num++;
  }
#else
  {
    types[num].mpi_datatype = MPI_DATATYPE_NULL;
    num++;
  }
#endif


  {
    /*
     * MPI_TYPE_MIX_ARRAY
     */
    int block_mix[6];
    MPI_Aint disp_mix[6];
    MPI_Datatype mix_type[6];
    MPI_Aint mix_base;
    struct tst_mpi_type_mix_array type_tmp_array;

    MPI_Type_contiguous (TST_MPI_TYPE_MIX_ARRAY_NUM, MPI_CHAR, &(mix_type[0]));
    MPI_Type_contiguous (TST_MPI_TYPE_MIX_ARRAY_NUM, MPI_SHORT, &(mix_type[1]));
    MPI_Type_contiguous (TST_MPI_TYPE_MIX_ARRAY_NUM, MPI_INT, &(mix_type[2]));
    MPI_Type_contiguous (TST_MPI_TYPE_MIX_ARRAY_NUM, MPI_LONG, &(mix_type[3]));
    MPI_Type_contiguous (TST_MPI_TYPE_MIX_ARRAY_NUM, MPI_FLOAT, &(mix_type[4]));
    MPI_Type_contiguous (TST_MPI_TYPE_MIX_ARRAY_NUM, MPI_DOUBLE, &(mix_type[5]));
/*
    MPI_Type_commit (&(mix_type[0]));
    MPI_Type_commit (&(mix_type[1]));
    MPI_Type_commit (&(mix_type[2]));
    MPI_Type_commit (&(mix_type[3]));
    MPI_Type_commit (&(mix_type[4]));
    MPI_Type_commit (&(mix_type[5]));
*/
    MPI_Address(&(type_tmp_array.a[0]), disp_mix);
    MPI_Address(&(type_tmp_array.b[0]), disp_mix+1);
    MPI_Address(&(type_tmp_array.c[0]), disp_mix+2);
    MPI_Address(&(type_tmp_array.d[0]), disp_mix+3);
    MPI_Address(&(type_tmp_array.e[0]), disp_mix+4);
    MPI_Address(&(type_tmp_array.f[0]), disp_mix+5);

    mix_base = disp_mix[0];
    for(i=0; i < 6; i++) disp_mix[i] -= mix_base;
    for(i=0; i < 6; i++) block_mix[i] = 1;
    MPI_Type_struct (6, block_mix, disp_mix, mix_type, &(types[num].mpi_datatype));

    MPI_Type_free (&(mix_type[0]));
    MPI_Type_free (&(mix_type[1]));
    MPI_Type_free (&(mix_type[2]));
    MPI_Type_free (&(mix_type[3]));
    MPI_Type_free (&(mix_type[4]));
    MPI_Type_free (&(mix_type[5]));

    MPI_Type_commit (&(types[num].mpi_datatype));

    types[num].type_num = 6 * TST_MPI_TYPE_MIX_ARRAY_NUM;
    for (i = 0; i < TST_MPI_TYPE_MIX_ARRAY_NUM; i++)
      {
        types[num].type_mapping[0*TST_MPI_TYPE_MIX_ARRAY_NUM+i] = TST_MPI_CHAR;
        types[num].type_mapping[1*TST_MPI_TYPE_MIX_ARRAY_NUM+i] = TST_MPI_SHORT;
        types[num].type_mapping[2*TST_MPI_TYPE_MIX_ARRAY_NUM+i] = TST_MPI_INT;
        types[num].type_mapping[3*TST_MPI_TYPE_MIX_ARRAY_NUM+i] = TST_MPI_LONG;
        types[num].type_mapping[4*TST_MPI_TYPE_MIX_ARRAY_NUM+i] = TST_MPI_FLOAT;
        types[num].type_mapping[5*TST_MPI_TYPE_MIX_ARRAY_NUM+i] = TST_MPI_DOUBLE;
      }
    num++;
  }
#ifndef HAVE_MPI_NECSX
  {
    /*
     * MPI_TYPE_MIX_LB_UB
     */
    int block_mix[8];
    MPI_Aint disp_array[8] = {
        -1*(sizeof(char)+sizeof(long)+sizeof(double)), /* Position of LB */
        -1*sizeof(char),                               /* Position of char */
        0,                                             /* Position of short */
        sizeof(short),                                 /* Position of int */
        -1*(sizeof(char)+sizeof(long)),                /* Position of long */
        sizeof(short)+sizeof(int),                     /* Position of float */
        -1*(sizeof(char)+sizeof(long)+sizeof(double)), /* Position of double */
        sizeof(short)+sizeof(int)+sizeof(float)        /* Position of UB */
      };
    MPI_Datatype mix_type[8] = {
        MPI_LB,
        MPI_CHAR,
        MPI_SHORT,
        MPI_INT,
        MPI_LONG,
        MPI_FLOAT,
        MPI_DOUBLE,
        MPI_UB };
    for(i=0 ; i <  8; i++) block_mix[i]=1;
    MPI_Type_struct(8, block_mix, disp_array, mix_type, &(types[num].mpi_datatype));
    MPI_Type_commit(&(types[num].mpi_datatype));
    types[num].type_num = 6;
    types[num].type_mapping[0] = TST_MPI_CHAR;
    types[num].type_mapping[1] = TST_MPI_SHORT;
    types[num].type_mapping[2] = TST_MPI_INT;
    types[num].type_mapping[3] = TST_MPI_LONG;
    types[num].type_mapping[4] = TST_MPI_FLOAT;
    types[num].type_mapping[5] = TST_MPI_DOUBLE;
    MPI_Type_lb ((types[num].mpi_datatype), &(types[num].lb));
    MPI_Type_ub ((types[num].mpi_datatype), &(types[num].ub));

    num++;
  }
#endif

#if defined(HAVE_MPI2)
  {
    /*
     * Dup MPI_CHAR
     * Just duplicate the Type -- everthing else is setup already.
     */
    if (strcmp (types[0].description, "MPI_CHAR"))
      ERROR (EINVAL, "Internal Error");
    MPI_Type_dup (types[0].mpi_datatype, &(types[num].mpi_datatype));
/*    types[num].lb = types[0].lb;
    types[num].ub = types[0].ub;
    types[num].type_class = types[0].type_class;
    types[num].type_num = types[0].type_num;
    types[num].type_mapping[0] = types[0].type_mapping[0];*/
    num++;
  }
#if !defined (HAVE_MPI_NECSX) && !defined(HAVE_MPI_OPENMPI)
  /*
   * OpenMPI fails here!!!
   */
  {
    /*
     * Dup MPI_TYPE_MIX_LB_UB
     */
    if (strcmp(types[num-2].description, "MPI_TYPE_MIX_LB_UB"))
      ERROR (EINVAL, "Internal Error");
    MPI_Type_dup (types[num-2].mpi_datatype, &(types[num].mpi_datatype));
    types[num].type_num = types[num-1].type_num;
    types[num].type_mapping[0] = types[num-1].type_mapping[0];
    types[num].type_mapping[1] = types[num-1].type_mapping[1];
    types[num].type_mapping[2] = types[num-1].type_mapping[2];
    types[num].type_mapping[3] = types[num-1].type_mapping[3];
    types[num].type_mapping[4] = types[num-1].type_mapping[4];
    types[num].type_mapping[5] = types[num-1].type_mapping[5];

    MPI_Type_lb ((types[num].mpi_datatype), &(types[num].lb));
    MPI_Type_ub ((types[num].mpi_datatype), &(types[num].ub));

    num++;
  }
#endif
#endif /* HAVE_MPI2 */

  *num_types = num;
  return 0;
}

int tst_type_cleanup (void)
{
  int i;
  for (i=PREDEFINED_DATATYPES; i < TST_TYPES_NUM; i++)
    {
      if (NULL == ((void*)types[i].mpi_datatype) || MPI_DATATYPE_NULL == types[i].mpi_datatype)
        continue;

      MPI_Type_free (&types[i].mpi_datatype);
    }
  return 0;
}

MPI_Datatype tst_type_getdatatype (int type)
{
  CHECK_ARG (type, MPI_DATATYPE_NULL);
  return types[type].mpi_datatype;
}

tst_uint64 tst_type_gettypeclass (int type)
{
  CHECK_ARG (type, -1);
  return types[type].type_class;
}

const char * tst_type_getdescription (int type)
{
  CHECK_ARG (type, NULL);
  return types[type].description;
}

int tst_type_gettypesize (int type)
{
  CHECK_ARG (type, -1);
  return (types[type].ub - types[type].lb);
}


void tst_type_hexdump (const char * text, const char * data, int num)
{
#define NUM_COL 16
   int i;

   printf ("(Rank:%d) %s [%d Bytes]\n(Rank:%d) [ 0-%d]:\t",
           tst_global_rank, text, num,
           tst_global_rank, NUM_COL-1);
   for (i = 0; i < num; i++)
     {
       if ((i % 7) == 7)
         printf ("  0x%.2x", data[i] & 0xFF);
       else
         printf (" 0x%.2x", data[i] & 0xFF);
       if ((i % NUM_COL) == NUM_COL-1 && i != num-1)
         printf ("\n(Rank:%d) [%d-%d]:\t",
                 tst_global_rank, i+1, MIN(i+NUM_COL+1, num-1));
     }
   printf("\n");
}


static int tst_type_gettypemax_log (int type)
{
  CHECK_ARG (type, -1);

  switch (types[type].type_class)
    {
      case TST_MPI_CHAR: return 1;
      case TST_MPI_UNSIGNED_CHAR: return 1;
#ifdef HAVE_MPI2
      case TST_MPI_SIGNED_CHAR: return 1;
#endif
      case TST_MPI_BYTE: return 1;
      case TST_MPI_SHORT: return SIZEOF_SHORT;
      case TST_MPI_UNSIGNED_SHORT: return SIZEOF_SHORT;
      case TST_MPI_INT: return SIZEOF_INT;
      case TST_MPI_UNSIGNED: return SIZEOF_INT;
      case TST_MPI_LONG: return SIZEOF_LONG;
      case TST_MPI_UNSIGNED_LONG: return SIZEOF_LONG;
      case TST_MPI_FLOAT: return SIZEOF_FLOAT;
      case TST_MPI_DOUBLE: return SIZEOF_DOUBLE;
#if defined(HAVE_LONG_DOUBLE) && defined (LDBL_MAX)
      case TST_MPI_LONG_DOUBLE: return SIZEOF_LONG_DOUBLE;
#endif
#if defined(HAVE_C_MPI_LONG_LONG_INT)
      case TST_MPI_LONG_LONG: return SIZEOF_LONG_LONG;
#endif /* HAVE_C_MPI_LONG_LONG_INT */
      case TST_MPI_PACKED: return 1;
        /*
          case TST_MPI_LB: return 0;
          case TST_MPI_UB: return 0;
        */
      case TST_MPI_FLOAT_INT: return SIZEOF_STRUCT_MPI_FLOAT_INT;
      case TST_MPI_DOUBLE_INT: return SIZEOF_STRUCT_MPI_DOUBLE_INT;
      case TST_MPI_LONG_INT: return SIZEOF_STRUCT_MPI_LONG_INT;
      case TST_MPI_SHORT_INT: return SIZEOF_STRUCT_MPI_SHORT_INT;
      case TST_MPI_2INT: return SIZEOF_STRUCT_MPI_2INT;
#if defined(HAVE_LONG_DOUBLE) && defined (LDBL_MAX)
      case TST_MPI_LONG_DOUBLE_INT: return SIZEOF_STRUCT_MPI_LONG_DOUBLE_INT;
#endif
      case TST_MPI_INT_CONTI: return (7*sizeof(int));
      case TST_MPI_INT_VECTOR: return (7*sizeof(int));
      case TST_MPI_INT_HVECTOR: return (7*sizeof(int));
      case TST_MPI_INT_INDEXED: return (7*sizeof(int));
      case TST_MPI_INT_HINDEXED: return (7*sizeof(int));
      case TST_MPI_INT_STRUCT: return (7*sizeof(int));
      case TST_MPI_TYPE_MIX: return sizeof(struct tst_mpi_type_mix);
      case TST_MPI_TYPE_MIX_ARRAY: return sizeof(struct tst_mpi_type_mix_array);
      case TST_MPI_TYPE_MIX_LB_UB: return SIZEOF_TYPE_MIX_LB_UB;
/*
      case TST_MPI_COMPLEX: return SIZEOF_FORTRAN_COMPLEX;
      case TST_MPI_DOUBLE_COMPLEX: return SIZEOF_FORTRAN_DOUBLE_COMPLEX;
      case TST_MPI_LOGICAL: return SIZEOF_FORTRAN_LOGICAL;
      case TST_MPI_REAL: return SIZEOF_FORTRAN_REAL;
      case TST_MPI_DOUBLE_PRECISION: return SIZEOF_FORTRAN_DOUBLE_PRECISION;
      case TST_MPI_INTEGER: return SIZEOF_FORTRAN_INTEGER;
      case TST_MPI_2INTEGER: return SIZEOF_FORTRAN_2INTEGER;
      case TST_MPI_2COMPLEX: return SIZEOF_FORTRAN_2COMPLEX;
      case TST_MPI_2DOUBLE_COMPLEX: return SIZEOF_FORTRAN_2DOUBLE_COMPEX;
      case TST_MPI_2REAL: return SIZEOF_FORTRAN_2REAL;
      case TST_MPI_2DOUBLE_PRECISION: return SIZEOF_FORTRAN_2DOUBLE_PRECISION;
*/
      default: return -1;
    }
}

char * tst_type_allocvalues (const int type, const int values_num)
{
  char * buffer;
  int type_size;

  CHECK_ARG (type, NULL);
  type_size = tst_type_gettypesize(type);

  buffer = malloc ((values_num+OVERHEAD) * type_size);
  if (buffer == NULL)
      ERROR (errno, "malloc");

  memset (buffer, DEFAULT_INIT_BYTE, (values_num+OVERHEAD) * type_size);
  buffer -= tst_type_gettypelb(type);

  return buffer;
}

int tst_type_freevalues (const int type, char * buffer, const int values_num)
{
  CHECK_ARG (type, -1);

  buffer += tst_type_gettypelb(type);
  free (buffer);

  return 0;
}

#ifndef UCHAR_MIN
#  define UCHAR_MIN 0
#endif
#ifndef USHRT_MIN
#  define USHRT_MIN 0
#endif
#ifndef UINT_MIN
#  define UINT_MIN 0
#endif
#ifndef ULONG_MIN
#  define ULONG_MIN 0
#endif
#ifndef ULLONG_MIN
#  define ULLONG_MIN 0
#endif

/*
#define TST_TYPE_VALUE_BOUNDED(tst_type,c_type,c_type_caps)                                      \
          if (direct_value > c_type_caps##_MAX) {                                                \
            *(c_type*)buffer = c_type_caps##_MAX;                                                \
          } else if (direct_value < c_type_caps##_MIN) {                                         \
            *(c_type*)buffer = c_type_caps##_MIN;                                                \
          } else {                                                                               \
            *(c_type*)buffer = direct_value; break;                                              \
          }                                                                                      \
*/

#define TST_TYPE_SET(tst_type,c_type,c_type_caps)                                                \
  case tst_type:                                                                                 \
    {                                                                                            \
      switch (type_set)                                                                          \
      {                                                                                          \
      case TST_TYPE_SET_ZERO: *(c_type*)buffer = 0; break;                                       \
      case TST_TYPE_SET_MAX: *(c_type*)buffer = c_type_caps##_MAX; break;                        \
      case TST_TYPE_SET_MIN: *(c_type*)buffer = c_type_caps##_MIN; break;                        \
      case TST_TYPE_SET_VALUE:                                                                   \
        if (direct_value > c_type_caps##_MAX) {                                                  \
          *(c_type*)buffer = c_type_caps##_MAX;                                                  \
        } else if (direct_value < c_type_caps##_MIN) {                                           \
          *(c_type*)buffer = c_type_caps##_MIN;                                                  \
        } else {                                                                                 \
          *(c_type*)buffer = direct_value;                                                       \
        }                                                                                        \
        break;                                                                                   \
      }                                                                                          \
      break;                                                                                     \
    }


#define TST_TYPE_SET_UNSIGNED(tst_type,c_type,c_type_caps)                                       \
  case tst_type:                                                                                 \
    {                                                                                            \
      switch (type_set)                                                                          \
      {                                                                                          \
      case TST_TYPE_SET_ZERO: *(c_type*)buffer = 0; break;                                       \
      case TST_TYPE_SET_MAX: *(c_type*)buffer = c_type_caps##_MAX; break;                        \
      case TST_TYPE_SET_MIN: *(c_type*)buffer = 0; break;                                        \
      case TST_TYPE_SET_VALUE:                                                                   \
        if (direct_value > c_type_caps##_MAX) {                                                  \
          *(c_type*)buffer = c_type_caps##_MAX;                                                  \
        } else if (direct_value < c_type_caps##_MIN) {                                           \
          *(c_type*)buffer = c_type_caps##_MIN;                                                  \
        } else {                                                                                 \
          *(c_type*)buffer = direct_value;                                                       \
        }                                                                                        \
        break;                                                                                   \
      }                                                                                          \
      break;                                                                                     \
    }


#define TST_TYPE_SET_STRUCT(tst_type,c_type,c_type_caps)                                         \
  case tst_type:                                                                                 \
    {                                                                                            \
      switch (type_set)                                                                          \
      {                                                                                          \
      case TST_TYPE_SET_ZERO:                                                                    \
        (*(c_type*)buffer).a = 0;                                                                \
        (*(c_type*)buffer).b = 0;                                                                \
        break;                                                                                   \
      case TST_TYPE_SET_MAX:                                                                     \
        (*(c_type*)buffer).a = c_type_caps##_MAX;                                                \
        (*(c_type*)buffer).b = INT_MAX;                                                          \
        break;                                                                                   \
      case TST_TYPE_SET_MIN:                                                                     \
        (*(c_type*)buffer).a = c_type_caps##_MIN;                                                \
        (*(c_type*)buffer).b = INT_MIN;                                                          \
        break;                                                                                   \
      case TST_TYPE_SET_VALUE:                                                                   \
        if (direct_value > c_type_caps##_MAX) {                                                  \
          (*(c_type*)buffer).a = c_type_caps##_MAX;                                              \
        } else if (direct_value < c_type_caps##_MIN) {                                           \
          (*(c_type*)buffer).a = c_type_caps##_MIN;                                              \
        } else {                                                                                 \
          (*(c_type*)buffer).a = direct_value;                                                   \
        }                                                                                        \
        if (direct_value > INT_MAX) {                                                            \
          (*(c_type*)buffer).b = INT_MAX;                                                        \
        } else if (direct_value < INT_MIN) {                                                     \
          (*(c_type*)buffer).b = INT_MIN;                                                        \
        } else {                                                                                 \
          (*(c_type*)buffer).b = direct_value;                                                   \
        }                                                                                        \
        break;                                                                                   \
      default:                                                                                   \
        return -1;                                                                               \
      }                                                                                          \
      break;                                                                                     \
    }


#define TST_TYPE_SET_CONTI(tst_type,c_type,c_type_caps)                                          \
  case tst_type:                                                                                 \
    {                                                                                            \
      int __i;                                                                                   \
      switch (type_set)                                                                          \
      {                                                                                          \
      case TST_TYPE_SET_ZERO:                                                                    \
        for(__i=0 ; __i<7 ; __i++)                                                               \
          ((c_type*)buffer)[__i] = 0;                                                            \
        break;                                                                                   \
      case TST_TYPE_SET_MAX:                                                                     \
        for(__i=0 ; __i<7 ; __i++)                                                               \
          ((c_type*)buffer)[__i] = c_type_caps##_MAX;                                            \
        break;                                                                                   \
      case TST_TYPE_SET_MIN:                                                                     \
        for(__i=0 ; __i<7 ; __i++)                                                               \
          ((c_type*)buffer)[__i] = c_type_caps##_MIN;                                            \
        break;                                                                                   \
      case TST_TYPE_SET_VALUE:                                                                   \
        for(__i=0 ; __i<7 ; __i++)                                                               \
          ((c_type*)buffer)[__i] =  direct_value;                                                \
        break;                                                                                   \
      }                                                                                          \
      break;                                                                                     \
    }


#define TST_TYPE_SET_STRUCT_MIX(tst_type,c_type,c_type_caps)                                     \
  case tst_type:                                                                                 \
    {                                                                                            \
      switch (type_set)                                                                          \
      {                                                                                          \
      case TST_TYPE_SET_ZERO:                                                                    \
        (*(c_type*)buffer).a = 0;                                                                \
        (*(c_type*)buffer).b = 0;                                                                \
        (*(c_type*)buffer).c = 0;                                                                \
        (*(c_type*)buffer).d = 0;                                                                \
        (*(c_type*)buffer).e = 0;                                                                \
        (*(c_type*)buffer).f = 0;                                                                \
        (*(c_type*)buffer).g.a = 0;                                                              \
        (*(c_type*)buffer).g.b = 0;                                                              \
        (*(c_type*)buffer).h.a = 0;                                                              \
        (*(c_type*)buffer).h.b = 0;                                                              \
        (*(c_type*)buffer).i.a = 0;                                                              \
        (*(c_type*)buffer).i.b = 0;                                                              \
        (*(c_type*)buffer).j.a = 0;                                                              \
        (*(c_type*)buffer).j.b = 0;                                                              \
        (*(c_type*)buffer).k.a = 0;                                                              \
        (*(c_type*)buffer).k.b = 0;                                                              \
        break;                                                                                   \
        case TST_TYPE_SET_MAX:                                                                   \
        (*(c_type*)buffer).a = CHAR_MAX;                                                         \
        (*(c_type*)buffer).b = SHRT_MAX;                                                         \
        (*(c_type*)buffer).c = INT_MAX;                                                          \
        (*(c_type*)buffer).d = LONG_MAX;                                                         \
        (*(c_type*)buffer).e = FLT_MAX;                                                          \
        (*(c_type*)buffer).f = DBL_MAX;                                                          \
        (*(c_type*)buffer).g.a = FLT_MAX;                                                        \
        (*(c_type*)buffer).g.b = INT_MAX;                                                        \
        (*(c_type*)buffer).h.a = DBL_MAX;                                                        \
        (*(c_type*)buffer).h.b = INT_MAX;                                                        \
        (*(c_type*)buffer).i.a = LONG_MAX;                                                       \
        (*(c_type*)buffer).i.b = INT_MAX;                                                        \
        (*(c_type*)buffer).j.a = SHRT_MAX;                                                       \
        (*(c_type*)buffer).j.b = INT_MAX;                                                        \
        (*(c_type*)buffer).k.a = INT_MAX;                                                        \
        (*(c_type*)buffer).k.b = INT_MAX;                                                        \
        break;                                                                                   \
        case TST_TYPE_SET_MIN:                                                                   \
        (*(c_type*)buffer).a = CHAR_MIN;                                                         \
        (*(c_type*)buffer).b = SHRT_MIN;                                                         \
        (*(c_type*)buffer).c = INT_MIN;                                                          \
        (*(c_type*)buffer).d = LONG_MIN;                                                         \
        (*(c_type*)buffer).e = FLT_MIN;                                                          \
        (*(c_type*)buffer).f = DBL_MIN;                                                          \
        (*(c_type*)buffer).g.a = FLT_MIN;                                                        \
        (*(c_type*)buffer).g.b = INT_MIN;                                                        \
        (*(c_type*)buffer).h.a = DBL_MIN;                                                        \
        (*(c_type*)buffer).h.b = INT_MIN;                                                        \
        (*(c_type*)buffer).i.a = LONG_MIN;                                                       \
        (*(c_type*)buffer).i.b = INT_MIN;                                                        \
        (*(c_type*)buffer).j.a = SHRT_MIN;                                                       \
        (*(c_type*)buffer).j.b = INT_MIN;                                                        \
        (*(c_type*)buffer).k.a = INT_MIN;                                                        \
        (*(c_type*)buffer).k.b = INT_MIN;                                                        \
        break;                                                                                   \
      case TST_TYPE_SET_VALUE:                                                                   \
        (*(c_type*)buffer).a = direct_value;                                                     \
        (*(c_type*)buffer).b = direct_value;                                                     \
        (*(c_type*)buffer).c = direct_value;                                                     \
        (*(c_type*)buffer).d = direct_value;                                                     \
        (*(c_type*)buffer).e = direct_value;                                                     \
        (*(c_type*)buffer).f = direct_value;                                                     \
        (*(c_type*)buffer).g.a = direct_value;                                                   \
        (*(c_type*)buffer).g.b = direct_value;                                                   \
        (*(c_type*)buffer).h.a = direct_value;                                                   \
        (*(c_type*)buffer).h.b = direct_value;                                                   \
        (*(c_type*)buffer).i.a = direct_value;                                                   \
        (*(c_type*)buffer).i.b = direct_value;                                                   \
        (*(c_type*)buffer).j.a = direct_value;                                                   \
        (*(c_type*)buffer).j.b = direct_value;                                                   \
        (*(c_type*)buffer).k.a = direct_value;                                                   \
        (*(c_type*)buffer).k.b = direct_value;                                                   \
        break;                                                                                   \
      default:                                                                                   \
      return -1;                                                                                 \
      }                                                                                          \
      break;                                                                                     \
    }


#define TST_TYPE_SET_STRUCT_MIX_ARRAY(tst_type,c_type,c_type_caps)                               \
  case tst_type:                                                                                 \
    {                                                                                            \
      switch (type_set)                                                                          \
      {                                                                                          \
      int __i;                                                                                   \
      case TST_TYPE_SET_ZERO:                                                                    \
      for (__i=0; __i < TST_MPI_TYPE_MIX_ARRAY_NUM; __i++) {                                     \
        ((*(c_type*)buffer).a)[__i] = 0;                                                         \
        ((*(c_type*)buffer).b)[__i] = 0;                                                         \
        ((*(c_type*)buffer).c)[__i] = 0;                                                         \
        ((*(c_type*)buffer).d)[__i] = 0;                                                         \
        ((*(c_type*)buffer).e)[__i] = 0;                                                         \
        ((*(c_type*)buffer).f)[__i] = 0;                                                         \
      }                                                                                          \
      break;                                                                                     \
      case TST_TYPE_SET_MAX:                                                                     \
      for(__i=0; __i < TST_MPI_TYPE_MIX_ARRAY_NUM; __i++){                                       \
        ((*(c_type*)buffer).a)[__i] = CHAR_MAX;                                                  \
        ((*(c_type*)buffer).b)[__i] = SHRT_MAX;                                                  \
        ((*(c_type*)buffer).c)[__i] = INT_MAX;                                                   \
        ((*(c_type*)buffer).d)[__i] = LONG_MAX;                                                  \
        ((*(c_type*)buffer).e)[__i] = FLT_MAX;                                                   \
        ((*(c_type*)buffer).f)[__i] = DBL_MAX;                                                   \
      }                                                                                          \
      break;                                                                                     \
      case TST_TYPE_SET_MIN:                                                                     \
      for(__i=0; __i < TST_MPI_TYPE_MIX_ARRAY_NUM; __i++) {                                      \
        ((*(c_type*)buffer).a)[__i] = CHAR_MIN;                                                  \
        ((*(c_type*)buffer).b)[__i] = SHRT_MIN;                                                  \
        ((*(c_type*)buffer).c)[__i] = INT_MIN;                                                   \
        ((*(c_type*)buffer).d)[__i] = LONG_MIN;                                                  \
        ((*(c_type*)buffer).e)[__i] = FLT_MIN;                                                   \
        ((*(c_type*)buffer).f)[__i] = DBL_MIN;                                                   \
      }                                                                                          \
      break;                                                                                     \
      case TST_TYPE_SET_VALUE:                                                                   \
      for(__i=0; __i < TST_MPI_TYPE_MIX_ARRAY_NUM; __i++) {                                      \
        ((*(c_type*)buffer).a)[__i] = direct_value;                                              \
        ((*(c_type*)buffer).b)[__i] = direct_value;                                              \
        ((*(c_type*)buffer).c)[__i] = direct_value;                                              \
        ((*(c_type*)buffer).d)[__i] = direct_value;                                              \
        ((*(c_type*)buffer).e)[__i] = direct_value;                                              \
        ((*(c_type*)buffer).f)[__i] = direct_value;                                              \
      }                                                                                          \
      break;                                                                                     \
      default:                                                                                   \
      return -1;                                                                                 \
      }                                                                                          \
      break;                                                                                     \
    }

#define TST_TYPE_SET_STRUCT_MIX_LB_UB(tst_type,c_type,c_type_caps)                               \
  case tst_type:                                                                                 \
    {                                                                                            \
      MPI_Aint __disp_array[8]={                                                                 \
        -1*(sizeof(char)+sizeof(long)+sizeof(double)), /* Position of LB */                      \
        -1*sizeof(char),                               /* Position of char */                    \
	0,                                             /* Position of short */                   \
	sizeof(short),                                 /* Position of int */                     \
	-1*(sizeof(char)+sizeof(long)),                /* Position of long */                    \
	sizeof(short)+sizeof(int),                     /* Position of float */                   \
	-1*(sizeof(char)+sizeof(long)+sizeof(double)), /* Position of double */                  \
	sizeof(short)+sizeof(int)+sizeof(float)        /* Position of UB */                      \
      };                                                                                         \
      switch (type_set)                                                                          \
      {                                                                                          \
      case TST_TYPE_SET_ZERO:                                                                    \
        buffer[__disp_array[1]]=0;                                                               \
	*((short*)(&buffer[__disp_array[2]]))= 0;                                                \
	*((int*)(&buffer[__disp_array[3]]))= 0;                                                  \
	*((long*)(&buffer[__disp_array[4]]))= 0;                                                 \
	*((float*)(&buffer[__disp_array[5]]))= 0;                                                \
	*((double*)(&buffer[__disp_array[6]]))= 0;                                               \
        break;                                                                                   \
      case TST_TYPE_SET_MAX:                                                                     \
        buffer[__disp_array[1]]=CHAR_MAX;                                                        \
	*((short*)(&buffer[__disp_array[2]]))= SHRT_MAX;                                         \
	*((int*)(&buffer[__disp_array[3]]))= INT_MAX;                                            \
	*((long*)(&buffer[__disp_array[4]]))= LONG_MAX;                                          \
	*((float*)(&buffer[__disp_array[5]]))= FLT_MAX;                                          \
	*((double*)(&buffer[__disp_array[6]]))= DBL_MAX;                                         \
        break;                                                                                   \
      case TST_TYPE_SET_MIN:                                                                     \
        buffer[__disp_array[1]]=CHAR_MIN;                                                        \
	*((short*)(&buffer[__disp_array[2]]))= SHRT_MIN;                                         \
	*((int*)(&buffer[__disp_array[3]]))= INT_MIN;                                            \
	*((long*)(&buffer[__disp_array[4]]))= LONG_MIN;                                          \
	*((float*)(&buffer[__disp_array[5]]))= FLT_MIN;                                          \
	*((double*)(&buffer[__disp_array[6]]))= DBL_MIN;                                         \
      break;                                                                                     \
      case TST_TYPE_SET_VALUE:                                                                   \
	buffer[__disp_array[1]]= direct_value;                                                   \
	*((short*)(&buffer[__disp_array[2]]))= direct_value;                                     \
	*((int*)(&buffer[__disp_array[3]]))= direct_value;                                       \
	*((long*)(&buffer[__disp_array[4]]))= direct_value;                                      \
	*((float*)(&buffer[__disp_array[5]]))= direct_value;                                     \
	*((double*)(&buffer[__disp_array[6]]))= direct_value;                                    \
      break;                                                                                     \
      default:                                                                                   \
      return -1;                                                                                 \
      }                                                                                          \
      break;                                                                                     \
    }


int tst_type_setvalue (int type, char * buffer, int type_set, long long direct_value)
{
  CHECK_ARG (type, -1);

  memset (buffer, DEFAULT_INIT_BYTE, tst_type_gettypesize (type));

  switch (types[type].type_class)
    {
      TST_TYPE_SET (TST_MPI_CHAR, char, CHAR);
      TST_TYPE_SET_UNSIGNED (TST_MPI_UNSIGNED_CHAR, unsigned char, UCHAR);
#ifdef HAVE_MPI2
      TST_TYPE_SET (TST_MPI_SIGNED_CHAR, signed char, SCHAR);
#endif
      TST_TYPE_SET (TST_MPI_BYTE, char, CHAR);
      TST_TYPE_SET (TST_MPI_SHORT, short, SHRT);
      TST_TYPE_SET_UNSIGNED (TST_MPI_UNSIGNED_SHORT, unsigned short, USHRT);
      TST_TYPE_SET (TST_MPI_INT, int, INT);
      TST_TYPE_SET_UNSIGNED (TST_MPI_UNSIGNED, unsigned int, UINT);
      TST_TYPE_SET (TST_MPI_LONG, long, LONG);
      TST_TYPE_SET_UNSIGNED (TST_MPI_UNSIGNED_LONG, unsigned long, ULONG);
      TST_TYPE_SET (TST_MPI_FLOAT, float, FLT);
      TST_TYPE_SET (TST_MPI_DOUBLE, double, DBL);
#if defined(HAVE_LONG_DOUBLE) && defined (LDBL_MAX)
      TST_TYPE_SET (TST_MPI_LONG_DOUBLE, long double, LDBL);
#endif
#if defined(HAVE_C_MPI_LONG_LONG_INT)
      TST_TYPE_SET (TST_MPI_LONG_LONG, long long, LLONG);
#endif /* HAVE_C_MPI_LONG_LONG_INT */
      TST_TYPE_SET (TST_MPI_PACKED, char, CHAR);
      /*
        TST_TYPE_SET (TST_MPI_LB, char, CHAR);
        TST_TYPE_SET (TST_MPI_UB, char, CHAR);
      */

      TST_TYPE_SET_STRUCT (TST_MPI_FLOAT_INT, struct tst_mpi_float_int, FLT);
      TST_TYPE_SET_STRUCT (TST_MPI_DOUBLE_INT, struct tst_mpi_double_int, DBL);
      TST_TYPE_SET_STRUCT (TST_MPI_LONG_INT, struct tst_mpi_long_int, LONG);
      TST_TYPE_SET_STRUCT (TST_MPI_SHORT_INT, struct tst_mpi_short_int, SHRT);
      TST_TYPE_SET_STRUCT (TST_MPI_2INT, struct tst_mpi_2int, INT);
#if defined(HAVE_LONG_DOUBLE) && defined (LDBL_MAX)
      TST_TYPE_SET_STRUCT (TST_MPI_LONG_DOUBLE_INT, struct tst_mpi_long_double_int, LDBL);
#endif
      TST_TYPE_SET_CONTI (TST_MPI_INT_CONTI, int, INT);
      TST_TYPE_SET_CONTI (TST_MPI_INT_VECTOR, int, INT);
      TST_TYPE_SET_CONTI (TST_MPI_INT_HVECTOR, int, INT);
      TST_TYPE_SET_CONTI (TST_MPI_INT_INDEXED, int, INT);
      TST_TYPE_SET_CONTI (TST_MPI_INT_HINDEXED, int, INT);
      TST_TYPE_SET_CONTI (TST_MPI_INT_STRUCT, int, INT);
      TST_TYPE_SET_STRUCT_MIX (TST_MPI_TYPE_MIX, struct tst_mpi_type_mix, NOT_USED);
      TST_TYPE_SET_STRUCT_MIX_ARRAY (TST_MPI_TYPE_MIX_ARRAY, struct tst_mpi_type_mix_array, NOT_USED);
      TST_TYPE_SET_STRUCT_MIX_LB_UB (TST_MPI_TYPE_MIX_LB_UB, NOT_USED, NOT_USED);

/*
      TST_TYPE_SET (TST_MPI_COMPLEX
      TST_TYPE_SET (TST_MPI_DOUBLE_COMPLEX
      TST_TYPE_SET (TST_MPI_LOGICAL
      TST_TYPE_SET (TST_MPI_REAL
      TST_TYPE_SET (TST_MPI_DOUBLE_PRECISION
      TST_TYPE_SET (TST_MPI_INTEGER
      TST_TYPE_SET (TST_MPI_2INTEGER
      TST_TYPE_SET (TST_MPI_2COMPLEX
      TST_TYPE_SET (TST_MPI_2DOUBLE_COMPLEX
      TST_TYPE_SET (TST_MPI_2REAL
      TST_TYPE_SET (TST_MPI_2DOUBLE_PRECISION
*/
      default: ERROR (EINVAL, "Unknown type in tst_type_setvalue");
    }
  /* tst_type_hexdump ("Setting:", buffer, tst_type_gettypesize (type)); */
  return 0;
}

int tst_type_setstandardarray (int type, int values_num, char * buffer, int comm_rank)
{
  const int type_size = tst_type_gettypesize (type);
  int i;
  CHECK_ARG (type, -1);

  if (values_num > 0)
    tst_type_setvalue (type, &(buffer[0*type_size]), TST_TYPE_SET_MIN, 0);
  if (values_num > 1)
    tst_type_setvalue (type, &(buffer[1*type_size]), TST_TYPE_SET_MAX, 0);

  for (i = 2; i < values_num; i++)
    tst_type_setvalue (type, &(buffer[i*type_size]), TST_TYPE_SET_VALUE, comm_rank + i);

  return 0;
}

int tst_type_getstandardarray_size (int type, int values_num, MPI_Aint * size)
{
  CHECK_ARG (type, -1);
  if (NULL == size)
    return -1;

  *size  = (values_num+OVERHEAD) * tst_type_gettypesize(type);

  return 0;
}

int tst_type_cmpvalue (int type, const char * buffer1, const char * buffer2)
{
  const char * buf1 = buffer1 + tst_type_gettypelb (type);
  const char * buf2 = buffer2 + tst_type_gettypelb (type);
  CHECK_ARG (type, -1);
  /*
  ret = memcmp (buffer1, buffer2, tst_type_gettypesize (type));
  if (ret && tst_type_gettypeclass (type) == TST_MPI_SHORT_INT)
    printf ("(Rank:%d) type_size:%d sizeof(struct):%d buffer1:%d,%d buffer2:%d,%d differs for TST_MPI_SHORT_INT\n",
            tst_global_rank, tst_type_gettypesize (type),
            sizeof (struct tst_mpi_short_int),
            ((struct tst_mpi_short_int*)buffer1)->a,
            ((struct tst_mpi_short_int*)buffer1)->b,
            ((struct tst_mpi_short_int*)buffer2)->a,
            ((struct tst_mpi_short_int*)buffer2)->b);
  */
  /*
  printf ("buffer1:%p buf1:%p buffer2:%p buf2:%p lb:%d\n",
          buffer1, buf1, buffer2, buf2, tst_type_gettypelb(type));
  */
  return memcmp (buf1, buf2, tst_type_gettypesize (type));
}

int tst_type_checkstandardarray (int type, int values_num, char * buffer, int comm_rank)
{
  const int type_size = tst_type_gettypesize (type);
  int errors = 0;
  char err[128];
  char * cmp_value;
  int i;

  CHECK_ARG (type, -1);

  cmp_value = tst_type_allocvalues (type, 1);
  tst_type_setvalue (type, cmp_value, TST_TYPE_SET_MIN, 0);

  if (values_num > 0 && tst_type_cmpvalue (type, &buffer[0*type_size], cmp_value))
    {
      if (tst_report >= TST_REPORT_FULL)
        {
          printf ("(Rank:%d) Error in MIN:\n",
                  tst_global_rank);
          tst_type_hexdump ("Expected cmp_value", cmp_value, type_size);
          tst_type_hexdump ("Received buffer", &(buffer[0*type_size]), type_size);
        }
      errors++;
    }

  tst_type_setvalue (type, cmp_value, TST_TYPE_SET_MAX, 0);
  if (values_num > 1 && tst_type_cmpvalue (type, &buffer[1*type_size], cmp_value))
    {
      if (tst_report >= TST_REPORT_FULL)
        {
          printf ("(Rank:%d) Error in MAX:\n",
                  tst_global_rank);
          tst_type_hexdump ("Expected cmp_value", cmp_value, type_size);
          tst_type_hexdump ("Received buffer", &(buffer[0*type_size]), type_size);
        }
      errors++;
    }

  for (i = 2; i < values_num; i++)
    {
      tst_type_setvalue (type, cmp_value, TST_TYPE_SET_VALUE, comm_rank + i);

      if (tst_type_cmpvalue (type, cmp_value, &(buffer[i*type_size])))
        {
          /* struct tst_mpi_float_int * tmp = (struct tst_mpi_float_int*) &(buffer[i*type_size]); */
          /* ((struct tst_mpi_float_int*)cmp_value)->a, */
          if (tst_report >= TST_REPORT_FULL)
            {
              snprintf (err, sizeof (err), "(Rank:%d) Error at i:%d expected ", tst_global_rank, i);
              tst_type_hexdump (err, cmp_value, type_size);

              snprintf (err, sizeof (err), "(Rank:%d) Error at i:%d but received ", tst_global_rank, i);
              tst_type_hexdump (err, &(buffer[i*type_size]), type_size);
            }
          errors++;
        }
/*
      else
        printf ("(Rank:%d) Correct at i:%d cmp_value:%d buffer[%d]:%d\n",
                tst_global_rank, i,
                (char)*cmp_value,
                i*type_size,
                (char)buffer[i*type_size]);
*/
    }

  tst_type_freevalues (type, cmp_value, 1);
  return errors;
}


static int tst_type_gettypelb (int type)
{
  CHECK_ARG (type, -1);

  return types[type].lb;
}

/*
 * Currently unused
static int tst_type_getvalue (int type, char * buffer1, char * buffer2)
{
  CHECK_ARG (type, -1);

  return 0;
}
*/


void tst_type_list (void)
{
  int i;
  for (i = 0; i < TST_TYPES_NUM; i++)
    {
      if (types[i].description[0] == '\0')
        break;
      printf ("Type:%d %s\n",
              i, types[i].description);
    }

  for (i = 0; i < TST_TYPES_CLASS_NUM; i++)
    printf ("Type-Class:%d %s\n",
            i, tst_types_class_strings[i].class_string);
}


static int tst_type_search (const int search_test, const int * test_list, const int test_list_num)
{
  int k;

  for (k = 0; k < test_list_num; k++)
    if (test_list[k] == search_test)
      break;
  return (k == test_list_num) ? 0 : 1;
}


int tst_type_select (const char * type_string,
                     int * type_list, int * type_list_num, const int type_list_max)
{
  int i;

  if (type_string == NULL || type_list == NULL || type_list_num == NULL)
    ERROR (EINVAL, "Passed a NULL parameter");

  for (i = 0; i < TST_TYPES_CLASS_NUM; i++)
    {
      /*
       * In case we match a complete class of types, include every one!
       */
      if (!strcasecmp (type_string, tst_types_class_strings[i].class_string))
        {
          int j;
          DEBUG (printf ("type_string:%s matched with tst_types_class_strings[%d]:%s\n",
                         type_string, i, tst_types_class_strings[i].class_string));
          for (j = 0; j < TST_TYPES_NUM; j++)
            {
              /*
               * First search for this test in the type_list -- if already in, continue!
               */
              if (tst_type_search (j, type_list, *type_list_num))
                {
                  WARNING (printf ("Type:%s selected through class:%s was already "
                                   "included in list -- not including\n",
                                   types[j].description,
                                   tst_types_class_strings[i].class_string));
                  continue;
                }
              if (types[j].type_class & tst_types_class_strings[i].class)
                {
                  DEBUG (printf ("type_string:%s test j:%d i:%d with class:%d matches, type_list_num:%d\n",
                                 type_string, j, (1 << i), types[j].type_class,
                                 *type_list_num));
                  type_list[*type_list_num] = j;
                  (*type_list_num)++;
                  if (*type_list_num == type_list_max)
                    ERROR (EINVAL, "Too many user selected types");
                }
            }
          return 0;
        }
    }

  /*
   * In case we didn't match a complete class of types, test for every single one...
   */
  for (i = 0; i < TST_TYPES_NUM; i++)
    {
      if (!strcasecmp (type_string, types[i].description))
        {
          if (tst_type_search (i, type_list, *type_list_num))
            {
              WARNING (printf ("Type:%s was already included in list -- not including\n",
                               types[i].description));
              return 0;
            }

          type_list[*type_list_num] = i;
          (*type_list_num)++;
          if (*type_list_num == type_list_max)
            ERROR (EINVAL, "Too many user selected types");

          DEBUG (printf ("type_string:%s matched with type_list_num:%d\n",
                         type_string, *type_list_num));
          return 0;
        }
    }

  {
    char buffer[128];
    sprintf (buffer, "Datatype %s not recognized",
             type_string);
    ERROR (EINVAL, buffer);
  }
  return -1;
}
