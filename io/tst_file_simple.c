/*
 * File: tst_file_simple.c
 * Functionality:
 *  simple test opening and closing a file using MPI_File_open and 
 *  MPI_File_close
 * Author: Christoph Niethammer
 *
 * Date: Aug 11rd 2008
 * Copyright (c) 2009 Cisco Systems, Inc.  All rights reserved.
 */
#include "config.h"

#include <mpi.h>
#include "mpi_test_suite.h"



#ifdef HAVE_MPI2_IO
static char file_name[100];
#endif

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
    MPI_CHECK (MPI_Bcast(file_name, 100, MPI_CHAR, ROOT, comm));
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
  MPI_File file;

  comm = tst_comm_getcomm (env->comm);
  MPI_CHECK(MPI_Comm_rank(comm, &comm_rank));

  MPI_CHECK (MPI_File_open(comm, file_name, MPI_MODE_CREATE | MPI_MODE_RDWR, MPI_INFO_NULL, &file));
  MPI_CHECK (MPI_File_close(&file));
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
