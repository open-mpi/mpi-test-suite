/*
 * File: tst_establish_communication.c
 * Functionality:
 *  Check the communication with port function
 + with MPI_Port_open , MPI_Publish_name
 * MPI_Comm_connect and MPI_Comm_accept
 * Author: Rainer Keller und Sheng Feng
 *
 * Date: Jan 3rd 2007
 */
#include "config.h"

#include <mpi.h>
#include "mpi_test_suite.h"

#include "tst_output.h"

#undef DEBUG
#define DEBUG(x)

#ifdef HAVE_MPI2_DYNAMIC
static char * send_buffer = NULL;
static char ** recv_buffer = NULL;
static MPI_Comm *root_comm;
static MPI_Comm *client_comm ;
static MPI_Comm *comm_array;
static MPI_Status *status_buffer=NULL;
static char **port_name =NULL ;
static char *port_name_tmp =NULL;
static char **publish_name=NULL;
static char *publish_name_tmp=NULL;
#endif

int tst_establish_communication_init (struct tst_env * env)
{
#ifdef HAVE_MPI2_DYNAMIC
  MPI_Comm comm;
  int comm_rank;
  int comm_size;
  int excl_num = 1;
  int excl[1];
  int i;
  MPI_Group world_group, self_group;

  tst_output_printf (DEBUG_LOG, TST_REPORT_MAX,
                     "(Rank:%d) env->comm:%d env->type:%d env->values_num:%d\n",
                     tst_global_rank, env->comm, env->type, env->values_num);
  /*
   * Now, initialize the buffer
   */
  comm = tst_comm_getcomm (env->comm);
  MPI_CHECK (MPI_Comm_rank (comm, &comm_rank));

  if (tst_comm_getcommclass (env->comm) == TST_MPI_INTER_COMM)
    MPI_CHECK (MPI_Comm_remote_size (comm, &comm_size));
  else
    MPI_CHECK (MPI_Comm_size (comm, &comm_size));

  send_buffer = tst_type_allocvalues (env->type, env->values_num);
  tst_type_setstandardarray (env->type, env->values_num,
                             send_buffer, comm_rank);

  if ((recv_buffer = malloc (sizeof (char *) * comm_size)) == NULL)
    ERROR (errno, "malloc");
  if ((status_buffer = malloc (sizeof (MPI_Status) * comm_size)) == NULL)
    ERROR(errno, "malloc");

  for (i=0; i < comm_size; i++)
    {
      recv_buffer[i] = tst_type_allocvalues (env->type, env->values_num);
    }

  /*
   * Now, initialize the publishname for each process
   */
  publish_name_tmp = calloc(comm_size*10 , sizeof(char));
  publish_name = calloc(comm_size , sizeof(char *));
  for (i = 0 ; i< comm_size; i++)
    publish_name[i] = &(publish_name_tmp[i * 10]);

  sprintf( publish_name[comm_rank], "rank%d", tst_global_rank);

  MPI_CHECK (MPI_Allgather( publish_name[comm_rank], 10, MPI_CHAR,publish_name_tmp, 10, MPI_CHAR, comm ));
  /*
   * Now, initialize the buffer for portname for each process
   */
  port_name_tmp = calloc(comm_size*MPI_MAX_PORT_NAME , sizeof(char));
  port_name = calloc(comm_size , sizeof(char *));
  for (i = 0 ; i< comm_size; i++)
    port_name[i] = &(port_name_tmp[i * MPI_MAX_PORT_NAME]);

  comm_array = calloc(comm_size, sizeof(MPI_Comm));
  for(i = 0; i<comm_size; i++)
    {
      excl[0] = i;
      MPI_CHECK (MPI_Comm_group (comm, &world_group));
      MPI_CHECK (MPI_Group_excl (world_group, excl_num, excl, &self_group));
      MPI_CHECK (MPI_Comm_create (comm, self_group, &(comm_array[i])));
    }

  client_comm= calloc (comm_size , sizeof(MPI_Comm));
  root_comm= calloc (comm_size , sizeof(MPI_Comm));

  /*
   * Now, initialize the server und client communicator
   */

  for(i = 0; i<comm_size; i++)
    {
      if(i == comm_rank)
        {
          MPI_CHECK (MPI_Open_port(MPI_INFO_NULL, port_name[comm_rank]));
          MPI_CHECK (MPI_Publish_name(publish_name[comm_rank], MPI_INFO_NULL, port_name[comm_rank]));
          MPI_CHECK (MPI_Barrier(comm));
          MPI_CHECK (MPI_Comm_accept(port_name[comm_rank], MPI_INFO_NULL, 0, MPI_COMM_SELF, &(client_comm[comm_rank])));
        }

      if(i != comm_rank)
        {
          MPI_CHECK (MPI_Barrier(comm));
          MPI_CHECK (MPI_Lookup_name(publish_name[i], MPI_INFO_NULL, port_name[i]));
          MPI_CHECK (MPI_Comm_connect(port_name[i], MPI_INFO_NULL, 0, comm_array[i], &(root_comm[i])));
        }
    }

#endif /* HAVE_MPI2_DYNAMIC */

  return 0;
}


int tst_establish_communication_run (struct tst_env * env)
{
#ifdef HAVE_MPI2_DYNAMIC
  int comm_size;
  int comm_rank;
  int i,j,k;
  MPI_Comm comm;
  MPI_Datatype type;
  comm = tst_comm_getcomm (env->comm);
  type = tst_type_getdatatype (env->type);

  if (tst_comm_getcommclass (env->comm) == TST_MPI_INTRA_COMM)
    MPI_CHECK (MPI_Comm_size (comm, &comm_size));
  else
    MPI_CHECK (MPI_Comm_remote_size (comm, &comm_size));

  MPI_CHECK (MPI_Comm_rank (comm, &comm_rank));

  tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "(Rank:%d) comm_size:%d comm_rank:%d\n",
                 tst_global_rank, comm_size, comm_rank);

  for(i = 0; i<comm_size; i++)
    {
      if(i != comm_rank)
        {
          MPI_CHECK (MPI_Send(send_buffer, env->values_num , type, 0, comm_rank, root_comm[i]));
        }
      if(i == comm_rank)
        {
          for(j= k= 0; k<comm_size; k++)
            {
              if(k==comm_rank){
                continue;
              }
	      MPI_CHECK (MPI_Recv(recv_buffer[k], env->values_num, type, j, k, client_comm[comm_rank], &(status_buffer[k])));
              j++;

            }
        }
    }

  for(j= i = 0; i<comm_size; i++)
    {
      if(i==comm_rank){
        continue;
      }
      if (status_buffer[i].MPI_SOURCE != j  ||
          status_buffer[i].MPI_TAG != i)
        {
          tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "(Rank:%d) rank:%d comm_size:%d should be:%d "
                         "MPI_SOURCE:%d MPI_TAG:%d\n",
                         comm_rank, rank, comm_size, i,
                         status_buffer[i].MPI_SOURCE,
                         status_buffer[i].MPI_TAG);
          ERROR (EINVAL, "Error in communication");
        }


      tst_type_checkstandardarray (env->type, env->values_num,
                                   recv_buffer[i], i);
      j++;

    }
#endif /* HAVE_MPI2_DYNAMIC */

  return 0;
}


int tst_establish_communication_cleanup (struct tst_env * env)
{
#ifdef HAVE_MPI2_DYNAMIC
  MPI_Comm comm;
  int i;
  int comm_size, comm_rank;

  comm = tst_comm_getcomm (env->comm);

  tst_type_freevalues (env->type, send_buffer, env->values_num);

  if (tst_comm_getcommclass (env->comm) == TST_MPI_INTER_COMM)
    MPI_CHECK (MPI_Comm_remote_size (comm, &comm_size));
  else
    MPI_CHECK (MPI_Comm_size (comm, &comm_size));
  MPI_CHECK (MPI_Comm_rank (comm, &comm_rank));

  for(i = 0; i<comm_size; i++)
    {
      if(i == comm_rank)
        {
          MPI_CHECK (MPI_Comm_disconnect( &(client_comm[comm_rank])));
          MPI_CHECK (MPI_Unpublish_name(publish_name[comm_rank], MPI_INFO_NULL, port_name[comm_rank]));
          MPI_CHECK (MPI_Close_port(port_name[comm_rank]));
        }

      if(i != comm_rank)
        {
          MPI_CHECK (MPI_Comm_disconnect( &(root_comm[i])));
        }
    }



  for (i = 0; i < comm_size; i++)
    tst_type_freevalues (env->type, recv_buffer[i], env->values_num);
  free (recv_buffer);
  free (root_comm);
  free (client_comm);
  free (status_buffer);
  free (publish_name);
  free (publish_name_tmp);
  free (port_name);
  free (port_name_tmp);
  free (comm_array);
#endif
  return 0;
}
