/*
 * File: tst_file_write_at.c
 *
 * Functionality:
 *
 + MPI_file_write_at
 *
 * Author: Rainer Keller und Sheng Feng
 *
 * Date: Jan 3rd 2007
 */
#include "config.h"

#include "mpi.h"
#include "mpi_test_suite.h"

#undef DEBUG
#define DEBUG(x)

#define TST_ATOM_TRUE 1

static char * write_buffer = NULL;
static char file_name[100];

int tst_file_write_at_init (struct tst_env * env)
{
#ifdef HAVE_MPI2_IO
  MPI_Comm comm;
  int comm_rank;

  comm = tst_comm_getcomm (env->comm);
  MPI_CHECK (MPI_Comm_rank (comm, &comm_rank));
  if(comm_rank == ROOT)
    {
      memset (file_name, 0, sizeof (char)*100);
      sprintf(file_name, "%s%ld", TST_FILE_NAME, (long)getpid());
    }
  MPI_CHECK (MPI_Bcast(file_name,100,MPI_CHAR,ROOT,comm));
  write_buffer = tst_type_allocvalues (env->type, env->values_num);
  tst_type_setstandardarray(env->type, env->values_num, write_buffer, comm_rank);
#endif
  return 0;

}


int tst_file_write_at_run (struct tst_env * env)
{
#ifdef HAVE_MPI2_IO
  int comm_rank;
  int comm_size;
  MPI_Comm comm;
  MPI_Datatype type;
  MPI_File file =MPI_FILE_NULL;
  MPI_Status stat;
  char datarep[MPI_MAX_DATAREP_STRING];
  MPI_Datatype filetype_check, datatype_check;
  MPI_Offset off_position, off_view, off_position_byte;

  comm = tst_comm_getcomm (env->comm);
  type = tst_type_getdatatype (env ->type);
  MPI_CHECK(MPI_Comm_rank(comm, &comm_rank));
  if (tst_comm_getcommclass (env->comm) == TST_MPI_INTER_COMM)
    MPI_CHECK (MPI_Comm_remote_size (comm, &comm_size));
  else
  MPI_CHECK (MPI_Comm_size (comm, &comm_size));

  MPI_CHECK (MPI_File_open(comm, file_name,MPI_MODE_CREATE |MPI_MODE_WRONLY, MPI_INFO_NULL, &file));
  if (file == MPI_FILE_NULL) {
    printf("got FILE_NULL! (quitting gracefully)\n");
    MPI_Finalize();
    exit(1);
  }

  if(tst_atomic == TST_ATOM_TRUE)
    MPI_CHECK (MPI_File_set_atomicity(file, TST_ATOM_TRUE));

  MPI_CHECK (MPI_File_set_view(file, 0, type, type, "native", MPI_INFO_NULL));
  MPI_CHECK (MPI_File_write_at(file, comm_rank*env->values_num,
                               write_buffer,env->values_num, type,&stat));
  MPI_CHECK (MPI_File_get_position(file, &off_position));

  MPI_CHECK (MPI_File_get_view(file, &off_view, &datatype_check, &filetype_check, datarep));
  MPI_CHECK (MPI_File_get_byte_offset(file,0, &off_position_byte));
  MPI_CHECK (MPI_File_close(&file));
  if(off_position != 0)
    ERROR (EINVAL, "Error in position after MPI_File_write_at");
  if(off_view != 0)
    ERROR (EINVAL, "Error in view position after MPI_File_write_at");
  if(tst_type_compare(datatype_check, type) != TST_SUCESS)
    ERROR (EINVAL, "Error in datatype");
  if(tst_type_compare(filetype_check , type) != TST_SUCESS)
    ERROR (EINVAL, "Error in filetype");
  tst_file_check(env ->type, env ->values_num,comm_size,file_name, comm);

#endif

  return 0;

}

int tst_file_write_at_cleanup (struct tst_env * env)
{
#ifdef HAVE_MPI2_IO
  MPI_Comm comm;
  int comm_rank;
  comm = tst_comm_getcomm (env->comm);
  MPI_CHECK(MPI_Comm_rank(comm, &comm_rank));
  if(comm_rank == ROOT)
    MPI_File_delete(file_name, MPI_INFO_NULL);
  tst_type_freevalues (env->type, write_buffer, env->values_num);
#endif
  return 0;
}


