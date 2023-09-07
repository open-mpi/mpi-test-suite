/*
 * File: tst_threaded_ring_partitioned.c
 *
 * Functionality:
 *  Sends data through a ring using partitioned communication.
 *  Each thread corresponds to a partition of the send/receive buffers.
 *
 * Author: Axel Schneewind
 *
 * Date: July 19th 2023
 */
#include <mpi.h>
#include "mpi_test_suite.h"
#include "tst_threads.h"
#include "tst_output.h"
#include "tst_comm.h"

#include <pthread.h>

#define TST_RANK_MASTER 0

static pthread_barrier_t thread_barrier;

static int ratio_send_to_receive = 1;

int tst_threaded_ring_partitioned_init(struct tst_env *env)
{
  int comm_rank;
  MPI_Comm comm = tst_comm_getmastercomm(env->comm);
  MPI_CHECK(MPI_Comm_rank(comm, &comm_rank));

  int thread_num = tst_thread_get_num();
  int num_worker_threads = tst_thread_num_threads();

  tst_output_printf(DEBUG_LOG, TST_REPORT_MAX, "(Rank:%d, Thread:%d) env->comm:%d env->type:%d env->values_num:%d\n",
            tst_global_rank, thread_num, env->comm, env->type, env->values_num);

  // each partition contains env->values_num values
  MPI_Aint type_extent = tst_type_gettypesize(env->type);
  size_t buffer_size = num_worker_threads * env->values_num * type_extent;

  if (thread_num == TST_THREAD_MASTER)
  {
    // one request for sending, one for receiving
    tst_thread_alloc_global_requests(2);

    // barrier syncs master and worker threads
    pthread_barrier_init(&thread_barrier, NULL, num_worker_threads + 1);

    // global buffer holds send and recv buffer
    tst_thread_global_buffer_init(2 * buffer_size);
  }

  // wait until buffer is initialized by master thread (busy wait as thread barrier is not ready here)
  while (tst_thread_get_global_buffer_size() != 2 * buffer_size)
    usleep(2000);

  // first half of shared buffer is send and second half is receive buffer
  env->send_buffer = tst_thread_get_global_buffer();
  env->recv_buffer = (char *)tst_thread_get_global_buffer() + buffer_size;

  env->req_buffer = tst_thread_get_global_request(0);

  // master thread of master rank initializes array values
  if (comm_rank == TST_RANK_MASTER && thread_num == TST_THREAD_MASTER)
    tst_type_setstandardarray(env->type, num_worker_threads * env->values_num, env->send_buffer, comm_rank);

  return 0;
}


// busy wait until partition arrived, using exponential backoff with initial backoff time given.
// returns 1 if the partition has arrived and 0 if waiting was interupted
static int wait_for_partition(MPI_Request *recv_request, int partition_num, useconds_t backoff_time)
{
  int flag = 0;
  do
  {
    MPI_CHECK(MPI_Parrived(*recv_request, partition_num, &flag));
  } while (flag == 0 && usleep((backoff_time = (backoff_time * 3) / 2)) == 0);

  return flag;
}

int tst_threaded_ring_partitioned_run(struct tst_env *env)
{
  int comm_size;
  int comm_rank;
  int send_to;
  int recv_from;

  // for measuring time
  double time_init;

  // only allow intra comm
  MPI_Comm comm = tst_comm_getmastercomm(env->comm);
  if (tst_comm_getcommclass(env->comm) & TST_MPI_INTRA_COMM)
  {
    MPI_CHECK(MPI_Comm_rank(comm, &comm_rank));
    MPI_CHECK(MPI_Comm_size(comm, &comm_size));

    send_to = (comm_rank + 1) % comm_size;
    recv_from = (comm_rank + comm_size - 1) % comm_size;
  }
  else if (tst_comm_getcommclass(env->comm) & TST_MPI_COMM_SELF)
  {
    MPI_CHECK(MPI_Comm_rank(comm, &comm_rank));
    MPI_CHECK(MPI_Comm_size(comm, &comm_size));

    send_to = comm_rank;
    recv_from = comm_rank;
  }
  else
    ERROR(EINVAL, "tst_threaded_ring_partitioned cannot run with this kind of communicator");

  MPI_Datatype type = tst_type_getdatatype(env->type);
  MPI_Aint type_extent = tst_type_gettypesize(env->type);

  MPI_Request *send_request = &env->req_buffer[0];
  MPI_Request *recv_request = &env->req_buffer[1];

  int num_threads = 1 + tst_thread_num_threads(); /* we have to add 1 for the master thread */
  int num_worker_threads = tst_thread_num_threads();
  int thread_num = tst_thread_get_num();

  MPI_CHECK(MPI_Comm_rank(comm, &comm_rank));
  MPI_CHECK(MPI_Comm_size(comm, &comm_size));

  tst_output_printf(DEBUG_LOG, TST_REPORT_MAX, "(Rank:%d, Thread:%d) comm_rank:%d comm_size:%d "
                         "send_to:%d recv_from:%d env->tag:%d\n",
            tst_global_rank, thread_num, comm_rank, comm_size,
            send_to, recv_from, env->tag);

  // number of partitions and values per partition
  int num_send_partitions = num_worker_threads;
  int num_recv_partitions = num_send_partitions / ratio_send_to_receive;
  int partition_size = env->values_num; // number of elements per send partition

  // partition numbers for this thread
  int send_partition_num = thread_num;
  int recv_partition_num = (thread_num % ratio_send_to_receive == 0) ? thread_num / ratio_send_to_receive : -1;

  // init send and recv and start both
  if (thread_num == TST_THREAD_MASTER)
  {
    tst_output_printf(DEBUG_LOG, TST_REPORT_MAX,"(Rank:%i, Thread:%i) initializing send to %i and recv from %i with %i partitions of size %i*%i bytes\n",
              comm_rank, thread_num,
              send_to, recv_from, num_send_partitions, partition_size, type_extent);

    MPI_CHECK(MPI_Psend_init(env->send_buffer, num_send_partitions, partition_size, type, send_to,
                 0, comm, MPI_INFO_NULL, send_request));
    MPI_CHECK(MPI_Precv_init(env->recv_buffer, num_recv_partitions, partition_size * ratio_send_to_receive, type, recv_from,
                 0, comm, MPI_INFO_NULL, recv_request));

    MPI_CHECK(MPI_Startall(2, env->req_buffer));

    // wait for all ranks to become ready
    MPI_CHECK(MPI_Barrier(MPI_COMM_WORLD));
  }

  pthread_barrier_wait(&thread_barrier);

  if (comm_rank == TST_RANK_MASTER)
  {
    if (thread_num == TST_THREAD_MASTER)
      time_init = MPI_Wtime();

    if (send_partition_num >= 0 && send_partition_num < num_send_partitions)
    {
      // allow this partition to be sent
      MPI_CHECK(MPI_Pready(send_partition_num, *send_request));
    }

    if (recv_partition_num >= 0 && recv_partition_num < num_recv_partitions)
    {
      wait_for_partition(recv_request, recv_partition_num, 512);
    }
  }
  else
  {
    if (send_partition_num >= 0 && send_partition_num < num_send_partitions)
    {
	  if (recv_partition_num >= 0 && recv_partition_num < num_recv_partitions) {
	  	wait_for_partition(recv_request, recv_partition_num, 128);
	  }

      // simply copy data from input to output buffer
      int begin_index = partition_size * send_partition_num * type_extent;
      int size = partition_size * type_extent;
      memcpy(&env->send_buffer[begin_index], &env->recv_buffer[begin_index], size);

      // allow sending of this partition
      MPI_CHECK(MPI_Pready(send_partition_num, *send_request));
    }
  }

  // wait until sends and recvs are done
  if (thread_num == TST_THREAD_MASTER)
  {
    MPI_CHECK(MPI_Waitall(2, env->req_buffer, env->status_buffer));

    if (comm_rank == TST_RANK_MASTER)
    {
      double time_final = MPI_Wtime();

      // print timing
      tst_output_printf(DEBUG_LOG, TST_REPORT_MAX, "(Rank:%d) Sending through ring took %fs\n", comm_rank, time_final - time_init);
    }
    else
      tst_output_printf(DEBUG_LOG, TST_REPORT_MAX, "(Rank:%d) done\n", comm_rank);
  }

  pthread_barrier_wait(&thread_barrier);

  // check that data was transmitted correctly
  if (thread_num == TST_THREAD_MASTER)
    return tst_test_checkstandardarray(env, env->recv_buffer, TST_RANK_MASTER);
  else
    return 0;
}

int tst_threaded_ring_partitioned_cleanup(struct tst_env *env)
{
  int thread_num = tst_thread_get_num();
  int num_worker_threads = tst_thread_num_threads();

  if (thread_num == TST_THREAD_MASTER)
  {
    tst_thread_free_global_requests();

    tst_thread_global_buffer_cleanup();

    pthread_barrier_destroy(&thread_barrier);
  }

  return 0;
}
