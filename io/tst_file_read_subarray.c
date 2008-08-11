/*
 * File: tst_file_read_subarray.c
 * Functionality:
 * test the function MPI_file_read_ordered
 * Author: Rainer Keller utnd Sheng Feng
 *
 * Date: Jan 3rd 2007
 */
#include "config.h"
#ifdef HAVE_MPI2_IO

#include "mpi.h"
#include "mpi_test_suite.h"

#undef DEBUG
#define DEBUG(x)

#define TST_ATOM_TRUE 1

static char * read_buffer = NULL;
static MPI_Datatype filetype;
static char file_name[100];

int tst_file_read_subarray_init (struct tst_env * env)
{
  MPI_Comm comm;
  int comm_rank;
  int comm_size;
  MPI_Datatype type;
  type = tst_type_getdatatype (env ->type);

  int sizes[1] ;
  int subsizes[1];
  int starts[1];

  comm = tst_comm_getcomm (env->comm);
  type = tst_type_getdatatype (env ->type);
  MPI_CHECK (MPI_Comm_rank (comm, &comm_rank));
  if(comm_rank == ROOT)
    {
      memset (file_name, 0, sizeof (char)*100);
      sprintf(file_name, "%s%ld", TST_FILE_NAME, (long)getpid());
    }
  MPI_CHECK (MPI_Bcast(file_name,100,MPI_CHAR,ROOT,comm));
  if (tst_comm_getcommclass (env->comm) == TST_MPI_INTER_COMM)
    MPI_CHECK (MPI_Comm_remote_size (comm, &comm_size));
  else
    MPI_CHECK (MPI_Comm_size (comm, &comm_size));

  sizes[0] =env->values_num * comm_size;
  subsizes[0] = env->values_num;
  starts[0] = comm_rank *subsizes[0];
  MPI_CHECK (MPI_Type_create_subarray(1, sizes, subsizes, starts,
                           MPI_ORDER_C, type, &filetype));
   MPI_CHECK (MPI_Type_commit(&filetype));
  read_buffer = tst_type_allocvalues (env->type, env->values_num);
  tst_file_alloc(env->type, env->values_num, comm_size, file_name, comm);
  return 0;

}


int tst_file_read_subarray_run (struct tst_env * env)
{
  int comm_rank,global_size;
  MPI_Comm comm;
  MPI_Datatype type;
  MPI_File file;
  MPI_Status stat;
  MPI_Datatype filetype_check, datatype_check;
  MPI_Offset off_position, off_view;
  char datarep[MPI_MAX_DATAREP_STRING];

  comm = tst_comm_getcomm (env->comm);
  type = tst_type_getdatatype (env ->type);
  MPI_CHECK(MPI_Comm_rank(comm, &comm_rank));

  MPI_CHECK (MPI_Comm_size (comm, &global_size));

  MPI_CHECK (MPI_File_open(comm, file_name, MPI_MODE_RDONLY, MPI_INFO_NULL, &file));
  MPI_CHECK (MPI_File_set_view(file, 0, type, filetype, "native", MPI_INFO_NULL));
  MPI_CHECK (MPI_File_read(file,read_buffer,env->values_num, type,&stat));
  MPI_CHECK (MPI_File_get_position(file, &off_position));

  MPI_CHECK (MPI_File_get_view(file, &off_view, &datatype_check, &filetype_check, datarep));

  MPI_CHECK (MPI_File_close(&file));

  if(off_position != env->values_num )
    ERROR (EINVAL, "Error in position after MPI_File_write");
  if(off_view != 0)
    ERROR (EINVAL, "Error in view position after MPI_File_write");
  if(tst_type_compare(datatype_check, type) != TST_SUCESS)
    ERROR (EINVAL, "Error in datatype");
  tst_test_checkstandardarray(env, read_buffer, comm_rank);

  return 0;

}

int tst_file_read_subarray_cleanup (struct tst_env * env)
{
  MPI_Comm comm;
  int comm_rank;
  comm = tst_comm_getcomm (env->comm);
  MPI_CHECK(MPI_Comm_rank(comm, &comm_rank));
  if(comm_rank == ROOT)
    MPI_File_delete(file_name, MPI_INFO_NULL);
  tst_type_freevalues (env->type, read_buffer, env->values_num);
  MPI_CHECK (MPI_Type_free(&filetype));
  return 0;
}

#endif
