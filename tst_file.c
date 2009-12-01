#include <stdio.h>
#include <unistd.h>
#ifdef HAVE_GETOPT_H
# include <getopt.h>
#endif
#include "mpi.h"
#include "mpi_test_suite.h"

#undef DEBUG
#define DEBUG(x)




int tst_file_alloc (int type, const int values_num, const int comm_size,
                    char file_name[100], const MPI_Comm comm)
{
#if HAVE_MPI2_IO
  MPI_File file;
  MPI_Datatype datatype;
  int i, comm_rank;
  char *write_buffer = NULL;
  MPI_Status stat;

  datatype = tst_type_getdatatype (type);
  MPI_CHECK( MPI_Comm_rank(comm, &comm_rank));
  MPI_CHECK( MPI_File_open(comm, file_name,
                           MPI_MODE_CREATE | MPI_MODE_RDWR, MPI_INFO_NULL, &file));
  if (file == MPI_FILE_NULL) {
      printf("got FILE_NULL! (quitting gracefully)\n");
      MPI_Finalize();
      exit(1);
  }

  if(comm_rank == ROOT)
    {
      write_buffer = tst_type_allocvalues (type, values_num);
      for(i = 0; i< comm_size; i++)
        {
          tst_type_setstandardarray (type, values_num, write_buffer, i);
          MPI_CHECK( MPI_File_write(file, write_buffer, values_num,  datatype, &stat));
        }
      tst_type_freevalues (type, write_buffer, values_num);
    }
  MPI_File_close(&file);

#endif

  return 0;
}

int tst_file_check (int type, const int values_num, const int comm_size,
                    char file_name[100], const MPI_Comm comm)
{
#if HAVE_MPI2_IO
  MPI_File file;
  MPI_Datatype datatype;
  int i, comm_rank;
  char *cmp_buffer = NULL;
  MPI_Status stat;

  datatype = tst_type_getdatatype (type);
  MPI_CHECK(MPI_Comm_rank(comm, &comm_rank));
  MPI_CHECK(MPI_File_open(comm, file_name,
                          MPI_MODE_RDWR ,
                          MPI_INFO_NULL, &file));

  if (file == MPI_FILE_NULL) {
      printf(" FILE IS LOSE! (quitting gracefully)\n");
      MPI_Finalize();
      exit(1);
  }

  if(comm_rank == ROOT)
    {
      cmp_buffer = tst_type_allocvalues (type, values_num);
      for(i = 0; i< comm_size; i++)
        {
          MPI_File_read(file, cmp_buffer, values_num, datatype, &stat);
          tst_type_checkstandardarray (type, values_num, cmp_buffer, i);
        }
      tst_type_freevalues (type, cmp_buffer, values_num);
    }
  MPI_File_close(&file);
#endif

  return 0;
}
