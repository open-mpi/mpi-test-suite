/*
 * File: tst_file_read_at_all.c
 * Functionality:
 *  test the Function MPI_File_read_at_all
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

int tst_file_read_at_all_init (struct tst_env * env)
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
  MPI_CHECK (MPI_Bcast(file_name,100,MPI_CHAR,ROOT,comm));
  read_buffer = tst_type_allocvalues (env->type, env->values_num);
  tst_file_alloc(env->type, env->values_num, comm_size, file_name, comm);
#endif
  return 0;

}


int tst_file_read_at_all_run (struct tst_env * env)
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

  comm = tst_comm_getcomm (env->comm);
  type = tst_type_getdatatype (env ->type);
  MPI_CHECK(MPI_Comm_rank(comm, &comm_rank));

  MPI_CHECK (MPI_File_open(comm, file_name, MPI_MODE_RDONLY, MPI_INFO_NULL, &file));
  MPI_CHECK (MPI_File_set_view(file, 0, type, type, "native", MPI_INFO_NULL));
  MPI_CHECK (MPI_File_read_at_all(file, comm_rank*env->values_num,
                                  read_buffer,env->values_num, type,&stat));
  MPI_CHECK (MPI_File_get_position(file, &off_position));
  MPI_CHECK ( MPI_File_get_view(file, &off_view, &datatype_check, &filetype_check, datarep));
  MPI_CHECK ( MPI_File_close(&file));

  if(off_position != 0)
    ERROR (EINVAL, "Error in position");
  if(off_view != 0)
    ERROR (EINVAL, "Error in view point");
  if(tst_type_compare(datatype_check, type) != TST_SUCESS)
    ERROR (EINVAL, "Error in datatype");
  if(tst_type_compare(filetype_check , type) != TST_SUCESS)
    ERROR (EINVAL, "Error in filetype");

  tst_test_checkstandardarray(env, read_buffer, comm_rank);
#endif




  return 0;

}

int tst_file_read_at_all_cleanup (struct tst_env * env)
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


