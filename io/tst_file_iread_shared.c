/*
 * File: tst_file_iread_shared.c
 * Functionality:
 *  test the Function MPI_File_iread_shared
 * Author: Rainer Keller und Sheng Feng
 *
 * Date: Jan 3rd 2007
 * Copyright (c) 2009 Cisco Systems, Inc.  All rights reserved.
 */

#include "config.h"

#include <mpi.h>
#include "mpi_test_suite.h"


#define TST_ATOM_TRUE 1

#ifdef HAVE_MPI2_IO
static char * read_buffer = NULL;
static char file_name[100];
#endif

int tst_file_iread_shared_init (struct tst_env * env)
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

  MPI_CHECK(MPI_Comm_rank(comm, &comm_rank));
  if(comm_rank == ROOT)
    {
      memset (file_name, 0, sizeof (char)*100);
      sprintf(file_name, "%s%ld", TST_FILE_NAME, (long)getpid());
    }
  MPI_CHECK (MPI_Bcast(file_name,100,MPI_CHAR,ROOT,comm));

  read_buffer = tst_type_allocvalues (env->type, env->values_num);
  tst_file_alloc(env->type, env->values_num, comm_size, file_name, comm);

#endif
  return 0;

}


int tst_file_iread_shared_run (struct tst_env * env)
{
#ifdef HAVE_MPI2_IO
  int comm_rank;
  int comm_size;
  int i;
  MPI_Comm comm;
  MPI_Datatype type;
  MPI_File file;
  MPI_Status stat;
  MPI_Request req;
  MPI_Datatype filetype_check, datatype_check;
  MPI_Offset off, off_shared;
  char datarep[MPI_MAX_DATAREP_STRING];

  comm = tst_comm_getcomm (env->comm);
  type = tst_type_getdatatype (env ->type);
  MPI_CHECK(MPI_Comm_rank(comm, &comm_rank));
  if (tst_comm_getcommclass (env->comm) == TST_MPI_INTER_COMM)
    MPI_CHECK (MPI_Comm_remote_size (comm, &comm_size));
  else
  MPI_CHECK (MPI_Comm_size (comm, &comm_size));

  MPI_CHECK(MPI_File_open(comm, file_name, MPI_MODE_RDONLY, MPI_INFO_NULL, &file));
  if(tst_atomic == TST_ATOM_TRUE)
     MPI_CHECK (MPI_File_set_atomicity(file, TST_ATOM_TRUE));
  MPI_CHECK(MPI_File_set_view(file, 0, type, type, "native", MPI_INFO_NULL));
  for (i = 0; i < comm_size; i++ )
    {
      if (comm_rank == i)
        {
           MPI_CHECK(MPI_File_iread_shared(file, read_buffer,env->values_num, type,&req));
           MPI_CHECK(MPI_Wait(&req, &stat));
           MPI_CHECK(MPI_File_get_position_shared(file, &off_shared));
           MPI_CHECK(MPI_File_get_view(file, &off, &datatype_check, &filetype_check, datarep));
        }
       MPI_CHECK(MPI_Barrier(comm));
    }

  MPI_File_close(&file);
  if(off_shared != env->values_num * (comm_rank+1))
     ERROR (EINVAL, "Error in shared position after MPI_File_iread_shared");
  if(off != 0)
     ERROR (EINVAL, "Error in view position after MPI_File_iread_shared");
  if(tst_type_compare(datatype_check, type) != TST_SUCESS)
     ERROR (EINVAL, "Error in datatype after MPI_File_iread_shared");
  if(tst_type_compare(filetype_check , type) != TST_SUCESS)
     ERROR (EINVAL, "Error in filetype after MPI_File_iread_shared");

  tst_test_checkstandardarray(env, read_buffer, comm_rank);

#endif
  return 0;

}

int tst_file_iread_shared_cleanup (struct tst_env * env)
{
#ifdef HAVE_MPI2_IO
  MPI_Comm comm;
  int comm_rank;
  comm = tst_comm_getcomm (env->comm);
  MPI_CHECK(MPI_Comm_rank(comm, &comm_rank));
  if(comm_rank == ROOT)
    MPI_File_delete(file_name, MPI_INFO_NULL);
  tst_type_freevalues (env->type, read_buffer, env->values_num);
#endif
   return 0;
}
