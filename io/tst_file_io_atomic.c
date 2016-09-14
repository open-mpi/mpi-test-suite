/*
 * File: tst_file_io_atomic.c
 * Functionality:
 *  Check the function atomic mode
 + with MPI_Set_atomicity und MPI_file_write_at
 *
 * Author: Rainer Keller und Sheng Feng
 *
 * Date: Jan 3rd 2007
 * Copyright (c) 2009 Cisco Systems, Inc.  All rights reserved.
 */



#include "config.h"

#include <mpi.h>
#include "mpi_test_suite.h"

#undef DEBUG
#define DEBUG(x)

#define TST_ATOM_TRUE 1

#ifdef HAVE_MPI2_IO
static char * read_buffer = NULL;
static char * write_buffer = NULL;
static char file_name[100];
#endif

int tst_file_io_atomic_init (struct tst_env * env)
{
#ifdef HAVE_MPI2_IO
  int comm_rank;
  MPI_Comm comm;

  DEBUG (printf ("(Rank:%d) env->comm:%d env->type:%d env->values_num:%d\n",
                 tst_global_rank, env->comm, env->type, env->values_num));

  read_buffer = tst_type_allocvalues (env->type, env->values_num);
  write_buffer = tst_type_allocvalues (env->type, env->values_num);

   /*
   * Now, initialize the write_buffer
   */
  comm = tst_comm_getcomm (env->comm);
  MPI_CHECK (MPI_Comm_rank (comm, &comm_rank));
  if(comm_rank == ROOT)
    {
      memset (file_name, 0, sizeof (char)*100);
      sprintf(file_name, "%s%ld", TST_FILE_NAME, (long)getpid());
    }
  MPI_CHECK (MPI_Bcast(file_name,100,MPI_CHAR,ROOT,comm));
  tst_type_setstandardarray (env->type, env->values_num, write_buffer, comm_rank);

#endif
  return 0;
}

int tst_file_io_atomic_run (struct tst_env * env)
{
#ifdef HAVE_MPI2_IO
  int comm_rank, comm_size;
  int read_from;
  int check_atomic;
  MPI_Comm comm;
  MPI_Datatype type;
  MPI_File file;
  MPI_Status stat;
  MPI_Datatype filetype_check, datatype_check;
  MPI_Offset off_view;
  MPI_Offset position1, position2;
  char datarep[MPI_MAX_DATAREP_STRING];

  comm = tst_comm_getcomm (env->comm);
  type = tst_type_getdatatype (env ->type);
  MPI_CHECK(MPI_Comm_rank(comm, &comm_rank));
  if (tst_comm_getcommclass (env->comm) == TST_MPI_INTER_COMM)
    MPI_CHECK (MPI_Comm_remote_size (comm, &comm_size));
  else
    MPI_CHECK (MPI_Comm_size (comm, &comm_size));
  read_from = (comm_rank + 1) % comm_size;

  MPI_CHECK (MPI_File_open(comm, file_name,MPI_MODE_CREATE |MPI_MODE_RDWR, MPI_INFO_NULL, &file));
  if (file == MPI_FILE_NULL) {
    printf("got FILE_NULL! (quitting gracefully)\n");
    MPI_Finalize();
    exit(1);
  }
  MPI_CHECK ( MPI_File_set_view(file, 0, type, type, "native", MPI_INFO_NULL));
  MPI_CHECK ( MPI_File_set_atomicity(file, TST_ATOM_TRUE));
  MPI_CHECK ( MPI_File_write_at(file, env->values_num * comm_rank, write_buffer,
                               env->values_num, type, &stat));
  MPI_CHECK ( MPI_File_get_position(file, &position1));
  MPI_CHECK ( MPI_File_get_atomicity(file, &check_atomic));
  MPI_CHECK ( MPI_Barrier(comm));
  MPI_CHECK ( MPI_File_read_at(file, env->values_num * read_from, read_buffer,
                               env->values_num, type, &stat));
  MPI_CHECK ( MPI_File_get_position(file, &position2));
  MPI_CHECK (MPI_File_get_view(file, &off_view, &datatype_check, &filetype_check, datarep));
  MPI_CHECK (MPI_File_close(&file));

  if(position1 != 0)
    ERROR (EINVAL, "Error in position after MPI_File_write_at");
  if(position2 != 0)
    ERROR (EINVAL, "Error in position after MPI_File_write_at");
  if(off_view != 0)
    ERROR (EINVAL, "Error in view position after MPI_File_write_at");
  if(tst_type_compare(datatype_check, type) != TST_SUCESS)
    ERROR (EINVAL, "Error in datatype after MPI_File_write_at");
  if(tst_type_compare(filetype_check , type) != TST_SUCESS)
    ERROR (EINVAL, "Error in filetype after MPI_File_write_at");

  tst_test_checkstandardarray (env, read_buffer, read_from);
#endif
  return 0;
}

int tst_file_io_atomic_cleanup (struct tst_env * env)
{
#ifdef HAVE_MPI2_IO
  MPI_Comm comm;
  int comm_rank;
  comm = tst_comm_getcomm (env->comm);
  MPI_CHECK(MPI_Comm_rank(comm, &comm_rank));
  if(comm_rank == ROOT)
    MPI_File_delete(file_name, MPI_INFO_NULL);
  tst_type_freevalues (env->type, read_buffer, env->values_num);
  tst_type_freevalues (env->type, write_buffer, env->values_num);
#endif
  return 0;
}
