#ifndef TST_THREADS_H_
#define TST_THREADS_H_

#include <mpi.h>
#include <pthread.h>

#include "mpi_test_suite.h"


#define TST_THREAD_MASTER -1


typedef enum {
  TST_THREAD_CMD_NULL = 0,
  TST_THREAD_CMD_INIT,
  TST_THREAD_CMD_RUN,
  TST_THREAD_CMD_CLEANUP,
  TST_THREAD_CMD_FINALIZE
} tst_thread_cmd_t;

typedef enum {
  TST_THREAD_STATE_IDLE = 0,
  TST_THREAD_STATE_CALLING_INIT,
  TST_THREAD_STATE_CALLING_RUN,
  TST_THREAD_STATE_CALLING_CLEANUP,
  TST_THREAD_STATE_FINALIZE
} tst_thread_state_t;

typedef enum {
  TST_THREAD_SIGNAL_STATE_WAIT = 0,
  TST_THREAD_SIGNAL_STATE_GOON
} tst_thread_signal_state;


struct tst_thread_env_t {
  int thread_num;
  tst_thread_state_t state;
  pthread_t tid;
  struct tst_env env;
  int (*tst_init_func) (const struct tst_env * env);
  int (*tst_run_func) (const struct tst_env * env);
  int (*tst_cleanup_func) (const struct tst_env * env);
};

int tst_thread_init(int max_threads, struct tst_thread_env_t ***thread_env);
int tst_thread_cleanup(struct tst_thread_env_t **thread_env);
int tst_thread_assign_reset(struct tst_thread_env_t **thread_env);
int tst_thread_assign_all(struct tst_env *env, struct tst_thread_env_t **thread_env);
int tst_thread_assign_one(struct tst_env *env, int thread_number, struct tst_thread_env_t **thread_env);
int tst_thread_execute_init(struct tst_env *env);
int tst_thread_execute_run(struct tst_env *env);
int tst_thread_execute_cleanup(struct tst_env *env);

int tst_thread_get_num();
int tst_thread_running();
int tst_thread_num_threads();

int tst_thread_signal_init(int num);
int tst_thread_signal_cleanup();
int tst_thread_signal_wait(int tag);
int tst_thread_signal_send(int tag);

void *tst_thread_global_buffer_init(int size);
int tst_thread_global_buffer_cleanup();
void *tst_thread_get_global_buffer();
int tst_thread_get_global_buffer_size();

MPI_Request *tst_thread_alloc_global_requests(int num);
MPI_Request *tst_thread_get_global_request(int num);
int tst_thread_free_global_requests();

#endif  /* TST_THREADS_H_ */
