/*
 * File: tst_file_io_sync.c
 * Functionality:
 *  allocate a new filetype with loch
 *
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
static char * write_buffer = NULL;
static MPI_Datatype filetype;
static char datarep[MPI_MAX_DATAREP_STRING];
static char file_name[100];
#endif

int tst_file_io_with_arrange_init (struct tst_env * env)
{
#ifdef HAVE_MPI2_IO
  MPI_Comm comm;
  int comm_size, comm_rank, global_size;
  int i;
  MPI_Offset off_view, off_position;
  MPI_Datatype filetype_check, datatype_check;
  MPI_File file;
  MPI_Datatype type;
  MPI_Aint extent;
  MPI_Status stat;
  comm = tst_comm_getcomm (env->comm);
  type = tst_type_getdatatype (env ->type);
  MPI_CHECK(MPI_Comm_rank(comm, &comm_rank));
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
  MPI_CHECK (MPI_Comm_size (MPI_COMM_WORLD, &global_size));
  read_buffer = tst_type_allocvalues (env->type, env->values_num);
  write_buffer = tst_type_allocvalues (env->type, env->values_num);
  MPI_CHECK( MPI_File_open(comm, file_name,
                          MPI_MODE_CREATE | MPI_MODE_RDWR, MPI_INFO_NULL, &file));
  if (file == MPI_FILE_NULL) {
    printf("Rank %d got FILE_NULL! (quitting gracefully)\n", comm_rank);
    MPI_Finalize();
    exit(1);
  }
  MPI_CHECK(MPI_File_get_type_extent(file , type, &extent));
  {
    int block_mix[3];
    MPI_Aint disp_array[3]={
        0,
        0,
        comm_size*extent
    };
    MPI_Datatype mix_type[3] = {
        MPI_LB,
        type,
        MPI_UB
    };
    for(i=0 ; i <  3; i++) block_mix[i]=1;
    MPI_CHECK(MPI_Type_struct(3, block_mix, disp_array, mix_type, &filetype));
    MPI_CHECK(MPI_Type_commit(&filetype));
  }
  MPI_CHECK(MPI_File_set_view(file,comm_rank*extent,type, filetype, "native", MPI_INFO_NULL));
  tst_type_setstandardarray (env->type, env->values_num,
                             write_buffer, comm_rank);
  MPI_CHECK(MPI_File_write(file, write_buffer, env->values_num, type, &stat));
  MPI_CHECK(MPI_File_get_position(file, &off_position));
  MPI_CHECK(MPI_File_get_view(file, &off_view, &datatype_check, &filetype_check, datarep));


  MPI_File_close(&file);
  if(global_size%2 == 0)
    {
      if(off_position != env->values_num)
        ERROR (EINVAL, "Error in position by init");
      if(off_view != comm_rank*extent)
        ERROR (EINVAL, "Error in view point");
      if(tst_type_compare(datatype_check, type) != TST_SUCESS)
        ERROR (EINVAL, "Error in datatype");
    }

#endif
  return 0;
}


int tst_file_io_with_arrange_run (struct tst_env * env)
{
#ifdef HAVE_MPI2_IO
  int comm_rank, global_size;;
  MPI_Comm comm;
  MPI_Datatype type;
  MPI_File file;
  MPI_Aint extent;
  MPI_Status stat;
  MPI_Offset off_position, off_view;
  MPI_Datatype filetype_check, datatype_check;
  comm = tst_comm_getcomm (env->comm);
  type = tst_type_getdatatype (env ->type);
  MPI_CHECK(MPI_Comm_rank(comm, &comm_rank));
  MPI_CHECK (MPI_Comm_size(MPI_COMM_WORLD, &global_size));
  MPI_CHECK(MPI_File_open(comm, file_name, MPI_MODE_RDWR, MPI_INFO_NULL, &file));
  MPI_CHECK(MPI_File_get_type_extent(file , type, &extent));
  MPI_CHECK(MPI_File_set_view(file,comm_rank*extent,type, filetype, "native", MPI_INFO_NULL));
  MPI_CHECK(MPI_File_read(file,read_buffer,env->values_num, type,&stat));
  MPI_CHECK(MPI_File_get_position(file, &off_position));

  MPI_CHECK(MPI_File_get_view(file, &off_view, &datatype_check, &filetype_check, datarep));

  MPI_CHECK(MPI_File_close(&file));

  if(off_position != env->values_num)
    ERROR (EINVAL, "Error in position by init");
  if(off_view != comm_rank*extent)
    ERROR (EINVAL, "Error in view point");
  if(tst_type_compare(datatype_check, type) != TST_SUCESS)
    ERROR (EINVAL, "Error in datatype");

  tst_test_checkstandardarray(env, read_buffer, comm_rank);
#endif
  return 0;
}


int tst_file_io_with_arrange_cleanup (struct tst_env * env)
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
  MPI_Type_free(&filetype);
#endif
   return 0;
}












