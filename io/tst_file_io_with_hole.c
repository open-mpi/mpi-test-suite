/*
 * File: tst_file_io_with_hole.c
 * Functionality:
 *  a new data type with loch is allocated.
 *  and test MPI_INFO
 * Author: Rainer Keller und Sheng Feng
 *
 * Date: Jan 3rd 2007
 * Copyright (c) 2009 Cisco Systems, Inc.  All rights reserved.
 */
#include "config.h"

#include "mpi.h"
#include "mpi_test_suite.h"

#undef DEBUG
#define DEBUG(x)
#define TST_ATOM_TRUE 1

#ifdef HAVE_MPI2_IO
static char * read_buffer = NULL;
static char * write_buffer = NULL;
static MPI_Datatype filetype;
static char datarep[MPI_MAX_DATAREP_STRING];
static char file_name[100];
#endif

int tst_file_io_with_hole_init (struct tst_env * env)
{
#ifdef HAVE_MPI2_IO
  MPI_Comm comm;
  int comm_size, comm_rank;
  int i;
  MPI_File file;
  MPI_Datatype type;
  MPI_Aint extent;
  MPI_Aint filetype_extent;
  MPI_Status stat;
  MPI_Datatype filetype_check, datatype_check;
  MPI_Offset off_view, off_position;

  MPI_Info info;

  MPI_CHECK (MPI_Info_create(&info));
  MPI_CHECK (MPI_Info_set(info, "cb_buffer_size", "8388608"));
  MPI_CHECK (MPI_Info_set(info, "ind_rd_buffer_size", "1048576"));
  MPI_CHECK (MPI_Info_set(info, "ind_wr_buffer_size", "262144"));
  MPI_CHECK (MPI_Info_set(info, "romio_cb_read", "enable"));
  MPI_CHECK (MPI_Info_set(info, "romio_cb_write", "enable"));
  MPI_CHECK (MPI_Info_set(info, "romio_ds_read", "enable"));
  MPI_CHECK (MPI_Info_set(info, "romio_ds_write", "enable"));

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

  read_buffer = tst_type_allocvalues (env->type, env->values_num);
  write_buffer = tst_type_allocvalues (env->type, env->values_num);

  MPI_CHECK (MPI_File_open(comm, file_name,
                          MPI_MODE_CREATE | MPI_MODE_RDWR, info, &file));
  MPI_CHECK (MPI_File_set_info(file , info));
  if(tst_atomic == TST_ATOM_TRUE)
     MPI_CHECK (MPI_File_set_atomicity(file, TST_ATOM_TRUE));

  if (file == MPI_FILE_NULL) {
        printf("Rank %d got FILE_NULL! (quitting gracefully)\n", comm_rank);
        MPI_Finalize();
        exit(1);
  }
  MPI_CHECK (MPI_File_get_type_extent(file , type, &extent));
  {
    int block_mix[3];
    MPI_Aint disp_array[3]={
        0,
        0,
        2*extent
    };
    MPI_Datatype mix_type[3] = {
        MPI_LB,
        type,
        MPI_UB
    };
    for(i=0 ; i <  3; i++) block_mix[i]=1;
    MPI_CHECK (MPI_Type_struct(3, block_mix, disp_array, mix_type, &filetype));
    MPI_CHECK (MPI_Type_commit(&filetype));
  }
  MPI_CHECK (MPI_File_get_type_extent(file , filetype, &filetype_extent));

  MPI_CHECK ( MPI_File_set_view(file,comm_rank*filetype_extent*env->values_num,type, filetype,                                       "native", MPI_INFO_NULL));
  tst_type_setstandardarray (env->type, env->values_num,
                             write_buffer, comm_rank);
  MPI_CHECK (MPI_File_write_all(file, write_buffer, env->values_num, type, &stat));
  MPI_CHECK (MPI_File_get_position(file, &off_position));
  MPI_File_get_view(file, &off_view, &datatype_check, &filetype_check, datarep);
  MPI_File_close(&file);
  if(off_position != env->values_num)
    ERROR (EINVAL, "Error in position");
  if(off_view != comm_rank*env->values_num*filetype_extent)
    ERROR (EINVAL, "Error in view point");
  if(tst_type_compare(datatype_check, type) != TST_SUCESS)
    ERROR (EINVAL, "Error in datatype");

#endif
  return 0;

}


int tst_file_io_with_hole_run (struct tst_env * env)
{
#ifdef HAVE_MPI2_IO
  int comm_rank;
  MPI_Comm comm;
  MPI_Datatype type;
  MPI_File file;
  MPI_Aint filetype_extent;
  MPI_Status stat;
  MPI_Datatype filetype_check, datatype_check;
  MPI_Offset off_view, off_position;
  int i, nkeys=0, flag, errs = 0;
  MPI_Info info, info_check;
  char key[MPI_MAX_INFO_KEY], value[MPI_MAX_INFO_VAL];

  MPI_CHECK (MPI_Info_create(&info));
  MPI_CHECK (MPI_Info_set(info, "cb_buffer_size", "8388608"));
  MPI_CHECK (MPI_Info_set(info, "ind_rd_buffer_size", "1048576"));
  MPI_CHECK (MPI_Info_set(info, "ind_wr_buffer_size", "262144"));
  MPI_CHECK (MPI_Info_set(info, "romio_cb_read", "enable"));
  MPI_CHECK (MPI_Info_set(info, "romio_cb_write", "enable"));
  MPI_CHECK (MPI_Info_set(info, "romio_ds_read", "enable"));
  MPI_CHECK (MPI_Info_set(info, "romio_ds_write", "enable"));

  comm = tst_comm_getcomm (env->comm);
  type = tst_type_getdatatype (env ->type);
  MPI_CHECK(MPI_Comm_rank(comm, &comm_rank));
  MPI_CHECK (MPI_File_open(comm, file_name, MPI_MODE_RDWR, info, &file));
  MPI_CHECK (MPI_File_set_info(file , info));
  if(tst_atomic == TST_ATOM_TRUE)
     MPI_CHECK (MPI_File_set_atomicity(file, TST_ATOM_TRUE));

  MPI_CHECK (MPI_File_get_type_extent(file , filetype, &filetype_extent));
  MPI_CHECK (MPI_File_set_view(file,comm_rank*filetype_extent*env->values_num,type, filetype, "native", MPI_INFO_NULL));
  MPI_CHECK ( MPI_File_read_all(file,read_buffer,env->values_num, type,&stat));
  MPI_CHECK ( MPI_File_get_position(file, &off_position));
  MPI_CHECK ( MPI_File_get_view(file, &off_view, &datatype_check, &filetype_check, datarep));
  MPI_CHECK ( MPI_File_get_info(file, &info_check));
  MPI_CHECK (MPI_File_close(&file));

  if(off_position != env->values_num)
    ERROR (EINVAL, "Error in position");
  if(off_view != comm_rank*env->values_num*filetype_extent)
    ERROR (EINVAL, "Error in view point");
  if(tst_type_compare(datatype_check, type) != TST_SUCESS)
    ERROR (EINVAL, "Error in datatype");

 for (i=0; i<nkeys; i++)
    {
      MPI_CHECK (MPI_Info_get_nthkey(info_check, i, key));
      MPI_CHECK (MPI_Info_get(info_check, key, MPI_MAX_INFO_VAL-1, value, &flag));
      if (!strcmp("cb_buffer_size", key))
        {
          if (atoi(value) != 8388608)
            errs++;
        }
      else if (!strcmp("ind_rd_buffer_size", key))
        {
          if (atoi(value) != 1048576)
            errs++;
        }
      else if (!strcmp("ind_wr_buffer_size", key))
        {
          if (atoi(value) != 262144)
            errs++;
        }
      else if (!strcmp("romio_cb_read", key))
        {
          if(strcmp("enable", value))
            errs++;
        }
      else if (!strcmp("romio_cb_write", key))
        {
          if(strcmp("enable", value))
            errs++;
        }
      else if (!strcmp("romio_ds_read", key))
        {
          if(strcmp("enable", value))
            errs++;
        }
      else if (!strcmp("romio_ds_write", key))
        {
          if(strcmp("enable", value))
            errs++;
        }
    }
  if (errs != 0)
    ERROR (EINVAL, "Error in info check");
  tst_test_checkstandardarray(env, read_buffer, comm_rank);
#endif
  return 0;

}

int tst_file_io_with_hole_cleanup (struct tst_env * env)
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

