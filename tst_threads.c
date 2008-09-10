#include "config.h"
#ifdef HAVE_PTHREAD_H
#  include <pthread.h>
#endif

#include "mpi_test_suite.h"

#undef DEBUG
#define DEBUG(x)

static int num_threads; /* Number of available threads - 1 */
static int working;
static pthread_mutex_t working_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t working_cond = PTHREAD_COND_INITIALIZER;
static void * tst_global_buffer;
static int tst_global_buffer_size;

typedef enum {
  TST_THREAD_CMD_NULL = 0,
  TST_THREAD_CMD_INIT,
  TST_THREAD_CMD_RUN,
  TST_THREAD_CMD_CLEANUP,
  TST_THREAD_CMD_FINALIZE
} tst_thread_cmd_t;

static tst_thread_cmd_t cmd = TST_THREAD_CMD_NULL;
static int cmd_count = 0;
static pthread_mutex_t cmd_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cmd_cond = PTHREAD_COND_INITIALIZER;


typedef enum {
  TST_THREAD_STATE_IDLE = 0,
  TST_THREAD_STATE_CALLING_INIT,
  TST_THREAD_STATE_CALLING_RUN,
  TST_THREAD_STATE_CALLING_CLEANUP,
  TST_THREAD_STATE_FINALIZE
} tst_thread_state_t;


struct tst_thread_env_t {
  int thread_num;
  tst_thread_state_t state;
  pthread_t tid;
  struct tst_env env;
  int (*tst_init_func) (const struct tst_env * env);
  int (*tst_run_func) (const struct tst_env * env);
  int (*tst_cleanup_func) (const struct tst_env * env);
};

typedef enum {
  TST_THREAD_SIGNAL_STATE_WAIT = 0,
  TST_THREAD_SIGNAL_STATE_GOON
} tst_thread_signal_state;
/*
 * Local declarations
 */
/* saving thread_ids for global access, also available in main via struct thread_env */
static pthread_t * tst_thread_tid_array;

/* arrays for the signal api */
static pthread_cond_t  * tst_thread_signal_cond_array;
static pthread_mutex_t * tst_thread_signal_mutex_array;
static tst_thread_signal_state * tst_thread_signal_states_array;
int tst_thread_signals_max = 0;         /* number of maximal usable tags */
int tst_thread_signals_count = 0;       /* number of waiting threads */

/* thread overlapping request handling */
static MPI_Request * tst_thread_requests_array;
int tst_thread_requests_max = 0;

/*
 * Function: worker_done
 */
static inline void worker_done(void)
{
  pthread_mutex_lock (&working_mutex);
  working--;
  if (0 == working)
    pthread_cond_signal (&working_cond);
  pthread_mutex_unlock (&working_mutex);
}

static inline void worker_barrier(void)
{
  pthread_mutex_lock (&working_mutex);
  while (working > 0)
    {
      DEBUG (printf ("(Rank:%d) worker_barrier still working:%d\n",
                    tst_global_rank, working));
      pthread_cond_wait (&working_cond, &working_mutex);
    }
  pthread_mutex_unlock (&working_mutex);
}


static void * tst_thread_dispatcher (void * arg)
{
  struct tst_thread_env_t * thread_env = (struct tst_thread_env_t*) arg;
  tst_thread_cmd_t local_cmd = TST_THREAD_CMD_NULL;
  int local_cmd_count = 0;


  DEBUG (printf ("(Rank:%d; Thread:%d) tst_thread_dispatcher started.\n",
                 tst_global_rank, thread_env->thread_num));

  /*
   * Wait for tests to be dispatched and scheduled.
   */
  while (TST_THREAD_CMD_FINALIZE != local_cmd)
    {
      const struct tst_env * env;

      pthread_mutex_lock (&cmd_mutex);
      while (local_cmd_count == cmd_count ||
             thread_env->tst_init_func == NULL ||
             thread_env->tst_run_func == NULL ||
             thread_env->tst_cleanup_func == NULL)
        pthread_cond_wait (&cmd_cond, &cmd_mutex);
      local_cmd = cmd;
      local_cmd_count = cmd_count;
      pthread_mutex_unlock (&cmd_mutex);

      /*
       * The env and init, run and cleanup functions are setup alright.
       */
      env = &(thread_env->env);

      DEBUG(printf ("(Rank:%d; Thread:%d) tst_thread_dispatcher cmd:%d func:[%s] comm:[%s] type:[%s] values_num:%d\n",
                    tst_global_rank, thread_env->thread_num, local_cmd,
                    tst_test_getdescription (env->test),
                    tst_test_getdescription (env->comm),
                    tst_test_getdescription (env->type),
                    env->values_num));

      switch (local_cmd) {
        case TST_THREAD_CMD_INIT:
          thread_env->state = TST_THREAD_STATE_CALLING_INIT;
          DEBUG(printf ("(Rank:%d; Thread:%d) tst_thread_dispatcher calling init\n",
                        tst_global_rank, thread_env->thread_num));
          thread_env->tst_init_func (env);

          worker_done();
          break;

        case TST_THREAD_CMD_RUN:
          thread_env->state = TST_THREAD_STATE_CALLING_RUN;
          DEBUG(printf ("(Rank:%d; Thread:%d) tst_thread_dispatcher calling run\n",
                        tst_global_rank, thread_env->thread_num));
          thread_env->tst_run_func (env);

          worker_done();
          break;

        case TST_THREAD_CMD_CLEANUP:
          thread_env->state = TST_THREAD_STATE_CALLING_CLEANUP;
          DEBUG(printf ("(Rank:%d; Thread:%d) tst_thread_dispatcher calling cleanup\n",
                        tst_global_rank, thread_env->thread_num));
          thread_env->tst_cleanup_func (env);

          worker_done();
          break;

        case TST_THREAD_CMD_FINALIZE:
          break;

        default:
          ERROR(EINVAL, "Unhandled cmd");
      }
      thread_env->state = TST_THREAD_STATE_IDLE;
    }
  return NULL;
}


/*
 * Initialize the thread-framework
 */
int tst_thread_init (int max_threads, struct tst_thread_env_t *** thread_env)
{
  int i;
  struct tst_thread_env_t ** env;

  if (max_threads <= 0 || thread_env == NULL)
    ERROR (EINVAL, "Called with faulty arguments");

  num_threads = max_threads - 1;
  /*
   * Without the pthread_mutex_lock, as no threads are started, yet
   */
  working = 0;

  DEBUG (printf ("(Rank:%d) tst_thread_init: max_threads:%d sizeof (struct tst_thread_env_t):%d\n",
                 tst_global_rank, max_threads, sizeof (struct tst_thread_env_t)));

  tst_thread_tid_array = malloc ( max_threads * sizeof (pthread_t));
  memset (tst_thread_tid_array, 0, max_threads * sizeof (pthread_t));
  env = malloc ( max_threads * sizeof (struct tst_thread_env_t *));
  memset (env, 0, max_threads * sizeof (struct tst_thread_env_t*));

  /*
   * Master thread also doing some work
   */
  tst_thread_tid_array[0] = pthread_self();

  for (i = 0; i < num_threads; i++)
    {
      int ret;
      env[i] = malloc (sizeof (struct tst_thread_env_t));
      memset (env[i], 0, sizeof (struct tst_thread_env_t));
      env[i]->thread_num = i;
      env[i]->state = TST_THREAD_STATE_IDLE;
      ret = pthread_create (&(env[i]->tid), NULL, tst_thread_dispatcher, env[i]);
      if (ret != 0)
        ERROR (errno, "tst_thread_init: pthread_create");
      tst_thread_tid_array[i + 1] = env[i]->tid;
    }


  *thread_env = env;
  return 0;
}

/*
 * Shut down the thread-framework
 */
int tst_thread_cleanup (struct tst_thread_env_t ** thread_env)
{
  int i;
  pthread_mutex_lock (&cmd_mutex);
  cmd = TST_THREAD_CMD_FINALIZE;
  cmd_count++;
  pthread_cond_broadcast (&cmd_cond);
  pthread_mutex_unlock (&cmd_mutex);

  /*
   * Before freeing everything, wait for the dispatcher threads to finish.
   */
  for (i = 0; i < num_threads; i++)
    pthread_join (thread_env[i]->tid, NULL);

  free (tst_thread_tid_array);
  tst_thread_tid_array = NULL;

  free (*thread_env);
  *thread_env = NULL;
  return 0;
}

/*
 * Returns the thread ID of the current thread
 */
inline int tst_thread_get_num (void)
{
  int i = 0;
  pthread_t self = pthread_self();
  DEBUG (printf ("Searching for thread_id %p\n", self));

  for (i = 0; i <= num_threads; i++) {
     DEBUG (printf ("Comparing with thread_id %p ....", tst_thread_tid_array[i]));
     if ( pthread_equal (tst_thread_tid_array[i], self) ) {
	DEBUG (printf ("Found :-) Return %d\n", i));
	return i;
     }
     else
	DEBUG (printf ("Not equal\n"));
  }
  ERROR (EINVAL, "Thread ID could not be determined");
  return -1;
}

/*
 * Assign all running threads the same test-environment
 */
int tst_thread_assign_reset (struct tst_thread_env_t ** thread_env)
{
  int i;
  for (i = 0; i < num_threads; i++)
    {
      memset (&(thread_env[i]->env), 0, sizeof (struct tst_env));
      thread_env[i]->tst_init_func = NULL;
      thread_env[i]->tst_run_func = NULL;
      thread_env[i]->tst_cleanup_func = NULL;
    }
  pthread_mutex_lock (&working_mutex);
  working = 0;
  pthread_mutex_unlock (&working_mutex);

  return 0;
}

/*
 * Assign all running threads the same test-environment
 */
int tst_thread_assign_all (struct tst_env * env, struct tst_thread_env_t ** thread_env)
{
  int i;
  for (i = 0; i < num_threads; i++)
    {
      memcpy (&(thread_env[i]->env), env, sizeof (struct tst_env));
      thread_env[i]->tst_init_func = tst_test_get_init_func(env);
      thread_env[i]->tst_run_func = tst_test_get_run_func(env);
      thread_env[i]->tst_cleanup_func = tst_test_get_cleanup_func(env);
    }
  pthread_mutex_lock (&working_mutex);
  working = num_threads;
  pthread_mutex_unlock (&working_mutex);
  return 0;
}

/*
 * Assign one particular running threads a test-environment
 */
int tst_thread_assign_one (struct tst_env * env, int thread_number, struct tst_thread_env_t ** thread_env)
{
  if (thread_number < 0 || thread_number >= num_threads)
    ERROR (EINVAL, "tst_thread_assign_one: Assignment not possible -- not enough threads");

  if (thread_env[thread_number]->tst_init_func != NULL ||
      thread_env[thread_number]->tst_run_func != NULL ||
      thread_env[thread_number]->tst_cleanup_func != NULL)
    ERROR (EINVAL, "tst_thread_assign_one: Assignment to this thread already done");

  memcpy (&(thread_env[thread_number]->env), env, sizeof (struct tst_env));
  thread_env[thread_number]->tst_init_func = tst_test_get_init_func(env);
  thread_env[thread_number]->tst_run_func = tst_test_get_run_func(env);
  thread_env[thread_number]->tst_cleanup_func = tst_test_get_cleanup_func(env);

  pthread_mutex_lock (&working_mutex);
  working++;
  pthread_mutex_unlock (&working_mutex);

  return 0;
}


int tst_thread_execute_init (struct tst_env * env)
{
  pthread_mutex_lock (&cmd_mutex);
  cmd = TST_THREAD_CMD_INIT;
  cmd_count++;
  pthread_cond_broadcast (&cmd_cond);
  pthread_mutex_unlock (&cmd_mutex);

  tst_test_init_func (env);

  worker_barrier();
  return 0;
}


int tst_thread_execute_run (struct tst_env * env)
{
  pthread_mutex_lock (&cmd_mutex);
  cmd = TST_THREAD_CMD_RUN;
  cmd_count++;
  working = num_threads;
  pthread_cond_broadcast (&cmd_cond);
  pthread_mutex_unlock (&cmd_mutex);

  tst_test_run_func (env);

  worker_barrier();
  return 0;
}

int tst_thread_execute_cleanup (struct tst_env * env)
{
  pthread_mutex_lock (&cmd_mutex);
  cmd = TST_THREAD_CMD_CLEANUP;
  cmd_count++;
  working = num_threads;
  pthread_cond_broadcast (&cmd_cond);
  pthread_mutex_unlock (&cmd_mutex);

  tst_test_cleanup_func (env);

  worker_barrier();
  return 0;
}


inline int tst_thread_num_threads (void)
{
  return num_threads + 1; /* +1 because the main thread also is doing some work */
}


inline int tst_thread_running (void)
{
  return (num_threads > 0);
}



/*
 * initialize num signals
 */
int tst_thread_signal_init (int num)
{
  int i;

  if ( (tst_thread_signal_cond_array = malloc (num * sizeof (pthread_cond_t))) == NULL)
    ERROR (errno, "malloc");
  if ( (tst_thread_signal_mutex_array = malloc (num * sizeof (pthread_mutex_t))) == NULL )
    ERROR (errno, "malloc");
  if ( (tst_thread_signal_states_array = malloc (num * sizeof (tst_thread_signal_state))) == NULL )
    ERROR (errno, "malloc");

  for (i = 0; i < num; i++)
  {
    pthread_cond_init (&tst_thread_signal_cond_array[i], NULL);
    pthread_mutex_init (&tst_thread_signal_mutex_array[i], NULL);
    tst_thread_signal_states_array[i] = TST_THREAD_SIGNAL_STATE_WAIT;
  }

  tst_thread_signals_max   = num;
  tst_thread_signals_count = 0;
  return 0;
}

/*
 * cleanup the signal environment
 */
int tst_thread_signal_cleanup (void)
{
  int i;
  if (tst_thread_signals_count != 0)
    ERROR (EINVAL, "tst_thread_signals already in use");
  for (i = 0; i < tst_thread_signals_max; i++)
  {
    pthread_mutex_destroy (&tst_thread_signal_mutex_array[i]);
    pthread_cond_destroy  (&tst_thread_signal_cond_array[i]);
  }
  free (tst_thread_signal_cond_array);
  free (tst_thread_signal_mutex_array);
  free (tst_thread_signal_states_array);
  tst_thread_signal_cond_array  = NULL;
  tst_thread_signal_mutex_array = NULL;
  tst_thread_signals_max = 0;
  return 0;
}

/*
 * wait until signal with tag occures from another tread
 * tag can be an integer between 0 and tst_thread_signals_max
 */
int tst_thread_signal_wait (int tag)
{
  pthread_cond_t  * signal_cond;
  pthread_mutex_t * signal_mutex;

  if ((tag < 0) || (tag >= tst_thread_signals_max))
    ERROR (EINVAL, "tag was outside range");
  signal_cond  = &tst_thread_signal_cond_array[tag];
  signal_mutex = &tst_thread_signal_mutex_array[tag];

  pthread_mutex_lock (signal_mutex);
  tst_thread_signals_count++;
  while (tst_thread_signal_states_array[tag] == TST_THREAD_SIGNAL_STATE_WAIT)
  {
    pthread_cond_wait (signal_cond, signal_mutex);
  }
  tst_thread_signals_count--;
  pthread_mutex_unlock (signal_mutex);

  return 0;
}

/*
 * send tag to threads waiting in tst_thread_signal_wait tag
 * tag can be an integer between 0 and tst_thread_signals_max
 */
int tst_thread_signal_send (int tag)
{
  if ((tag < 0) || (tag >= tst_thread_signals_max))
    ERROR (EINVAL, "tag was outside range");

  pthread_mutex_lock (&tst_thread_signal_mutex_array[tag]);
  tst_thread_signal_states_array[tag] = TST_THREAD_SIGNAL_STATE_GOON;
  pthread_cond_signal (&tst_thread_signal_cond_array[tag]);
  pthread_mutex_unlock (&tst_thread_signal_mutex_array[tag]);
  return 0;
}

/*
 * allocate num global requests
 */
MPI_Request * tst_thread_alloc_global_requests (int num)
{
  if ( (tst_thread_requests_array = malloc (num * sizeof (MPI_Request)) ) == NULL)
    ERROR (errno, "malloc");
  return tst_thread_requests_array;
}

/*
 * return the adress of the global request array
 */
MPI_Request * tst_thread_get_global_request (int num)
{
  if ((num < 0) || (num > tst_thread_requests_max))
    ERROR (EINVAL, "reuest number outside allowed range");
  return &tst_thread_requests_array[num];
}

/*
 * cleanup the global request environment
 */
int tst_thread_free_global_requests (void)
{
  tst_thread_requests_max = 0;
  free (tst_thread_requests_array);
  return 0;
}

/*
 * set the pointer for the global buffer
 */
void * tst_thread_global_buffer_init (int size)
{
  if (NULL == (tst_global_buffer = malloc(size)))
    ERROR (errno,"malloc");
  tst_global_buffer_size = size;
  return tst_global_buffer;
}

/*
 * Cleanup global buffer
 */
int tst_thread_global_buffer_cleanup (void)
{
  free(tst_global_buffer);
  tst_global_buffer_size = 0;
  tst_global_buffer = NULL;
  return ((0 == tst_global_buffer_size) && (NULL == tst_global_buffer));
}

/*
 * returns pointer onto the global buffer
 */
void * tst_thread_get_global_buffer ()
{
  return tst_global_buffer;
}

/*
 * returns size of the global buffer
 */
int tst_thread_get_global_buffer_size ()
{
  return tst_global_buffer_size;
}

