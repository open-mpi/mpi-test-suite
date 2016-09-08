/*
 * File: tst_env_cart_communicator.c
 *
 * Functionality:
 *   Create several cartesian communicators with known layout and
 *   test setup.
 *
 * Author: Rainer Keller
 *
 * Date: Dec. 1., 2005
 */
#include "mpi.h"
#include "mpi_test_suite.h"
#include "tst_output.h"

#undef DEBUG
#define DEBUG(x)


int tst_env_cart_communicator_init (struct tst_env * env)
{
  tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "(Rank:%d) env->comm:%d env->type:%d env->values_num:%d\n",
                 tst_global_rank, env->comm, env->type, env->values_num);
  return 0;
}

int tst_env_cart_communicator_run (struct tst_env * env)
{
  int dims[2];
  int periods[2];
  MPI_Comm comm_cart;
  int rank;
  int rank_cart;
  int newrank;

  /*
   * Create a 1-dimensional communicator and check that ranks are
   * correct and that the mapping works as expected.
   * The cartesian communicator is set to have periodic boundaries,
   * NO reordering is allowed.
   */
  MPI_CHECK (MPI_Comm_rank (MPI_COMM_WORLD, &rank));
  MPI_CHECK (MPI_Comm_size (MPI_COMM_WORLD, &(dims[0])));
  periods[0] = 1;
  MPI_CHECK (MPI_Cart_create (MPI_COMM_WORLD, 1, dims, periods, 0, &comm_cart));

  MPI_CHECK (MPI_Comm_rank (comm_cart, &rank_cart));
  if (rank_cart != rank)
    {
      if (tst_report == TST_REPORT_FULL)
        {
          printf ("(Rank:%d) tst_env_cart_communicator_run: Expected rank:%d rank_cart:%d\n",
                  rank, rank, rank_cart);
        }
      tst_test_record_failure (env);
    }

  MPI_CHECK (MPI_Cart_map (MPI_COMM_WORLD, 1, dims, periods, &newrank));
  if (newrank XXX)
  /*
   * Create a 1-dimensional communicaotr which does *not* contain the last process
   * to check, whether MPI_Cart_map returns MPI_UNDEFINED for this rank.
   */
  return 0;
}

int tst_env_cart_communicator_cleanup (struct tst_env * env)
{
  return 0;
}
