/*
 * File: tst_file_simple.c
 * Functionality:
 *  simple test opening and closing a file using MPI_File_open and 
 *  MPI_File_close
 * Author: Christoph Niethammer
 *
 * Date: Aug 11rd 2008
 */
#include "config.h"

#include "mpi.h"
#include "mpi_test_suite.h"

#undef DEBUG
#define DEBUG(x)


static char file_name[100];

int tst_file_simple_init (struct tst_env * env)
{
#ifdef HAVE_MPI2_IO
  MPI_Comm comm;
  int comm_size;
  int comm_rank;

  comm = tst_comm_getcomm (env->comm);
  if (tst_comm_getcommclass (env->comm) == TST_MPI_INTER_COMM)
    MPI_CHECK (MPI_Comm_remote_size (comm, &comm_size));
  else
    MPI_CHECK (MPI_Comm_size (comm, &comm_size));
  MPI_CHECK (MPI_Comm_rank (comm, &comm_rank));
  if(comm_rank == ROOT)
    {
      memset (file_name, 0, sizeof (char)*100);
      sprintf(file_name, "%s%ld", TST_FILE_NAME, (long)getpid());
    }
  if (0 == tst_thread_get_num())
	  MPI_CHECK (MPI_Bcast(file_name,100,MPI_CHAR,ROOT,comm));
  env->read_buffer = tst_type_allocvalues (env->type, env->values_num);
  tst_file_alloc(env->type, env->values_num, comm_size, file_name, comm);

#endif
  return 0;
}


int tst_file_simple_run (struct tst_env * env)
{
#ifdef HAVE_MPI2_IO
  int comm_rank;
  MPI_Comm comm;
  MPI_Datatype type;
  MPI_File file;
  MPI_Status stat;
  MPI_Datatype filetype_check, datatype_check;
  MPI_Offset off_view, off_position;
  char datarep[MPI_MAX_DATAREP_STRING];
int count = 1;

  comm = tst_comm_getcomm (env->comm);
  type = tst_type_getdatatype (env ->type);
  MPI_CHECK(MPI_Comm_rank(comm, &comm_rank));


printf("Started \"%s\"\n", "open"); fflush (stdout);
  MPI_CHECK (MPI_File_open(comm, file_name, MPI_MODE_CREATE | MPI_MODE_RDWR, MPI_INFO_NULL, &file));
printf("Ended \"%s\"\n", "open"); fflush (stdout);
#if 0
  MPI_CHECK (MPI_File_set_view(file, 0, type, type, "native", MPI_INFO_NULL));
  MPI_CHECK (MPI_File_seek(file, comm_rank*env->values_num, MPI_SEEK_SET ));
  MPI_CHECK (MPI_File_read(file,env->read_buffer,env->values_num, type,&stat));

  MPI_CHECK (MPI_File_get_position(file, &off_position));
  MPI_CHECK (MPI_File_get_view(file, &off_view, &datatype_check, &filetype_check, datarep));
printf("OK %s\n", "get view"); fflush (stdout);
#endif
printf("Started \"%s\"\n", "close"); fflush (stdout);
  MPI_CHECK (MPI_File_close(&file));
printf("Ended \"%s\"\n", "close"); fflush (stdout);

  MPI_Barrier(MPI_COMM_WORLD);

#if 0
  if(off_position != env->values_num * (comm_rank+1))
    ERROR (EINVAL, "Error in position");
  if(off_view != 0)
    ERROR (EINVAL, "Error in view point");
  if(tst_type_compare(datatype_check, type) != TST_SUCESS)
    ERROR (EINVAL, "Error in datatype");
  if(tst_type_compare(filetype_check , type) != TST_SUCESS)
    ERROR (EINVAL, "Error in filetype");

  tst_test_checkstandardarray(env, env->read_buffer, comm_rank);
#endif

#endif

  return 0;

}

int tst_file_simple_cleanup (struct tst_env * env)
{
#ifdef HAVE_MPI2_IO
  MPI_Comm comm;
  int comm_rank;
  comm = tst_comm_getcomm (env->comm);
  MPI_CHECK(MPI_Comm_rank(comm, &comm_rank));
  if(comm_rank == ROOT)
    MPI_File_delete(file_name, MPI_INFO_NULL);
  tst_type_freevalues (env->type, env->read_buffer, env->values_num);
#endif
return 0;
}


