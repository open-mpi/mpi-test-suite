#include <stdlib.h>
#include <stdio.h>
#include "config.h"
#ifdef HAVE_STRING_H
#  include <string.h>
#endif
#ifdef HAVE_STRINGS_H
#  include <strings.h>
#endif

#include <limits.h>
#include "mpi.h"
#include "mpi_test_suite.h"


#define COMM_NUM 16

/*#define HAVE_MPI_CLUSTER_SIZE*/

#undef DEBUG
#define DEBUG(x)

#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif

#define CHECK_ARG(i, ret) do {        \
  if ((i) < 0 || (i) > TST_COMMS_NUM) \
    return (ret);                     \
} while (0)

#define TST_COMMS_NUM (sizeof (comms) / sizeof (comms[0]))
#define TST_COMMS_CLASS_NUM (sizeof (tst_comms_class_strings) / sizeof (tst_comms_class_strings[0]))

static const char * const tst_comms_class_strings [] =
  {
    "COMM_SELF",
    "COMM_NULL",
    "INTRA_COMM",
    "INTER_COMM",
    "CART_COMM",
    "TOPO_COMM"
  };


struct comm {
  MPI_Comm mpi_comm;                       /* The actual MPI communicator */
  char description [TST_DESCRIPTION_LEN];  /* The communicator's description */
  int class;                               /* Class of communicator */
  int size;                                /* Size of this communicator */
  int * mapping;                           /* Our mapping of the communicator */
  int other_size;                          /* In case of intra-comms, the size of the other communicator */
  int * other_mapping;                     /* In case of intra-comms, the mapping of the other communicator */
};

static struct comm comms[16] = {
  {MPI_COMM_WORLD, "MPI_COMM_WORLD", TST_MPI_INTRA_COMM, -1, NULL, 0, NULL},
  {MPI_COMM_NULL,  "MPI_COMM_NULL",  TST_MPI_COMM_NULL,   0, NULL, 0, NULL},
  {MPI_COMM_SELF,  "MPI_COMM_SELF",  TST_MPI_COMM_SELF,   1, NULL, 0, NULL},
  /* The Rest */
  {MPI_COMM_WORLD, "",               -1,                 -1, NULL, 0, NULL},
};

#define NUM_CONNS *num_conns;

int tst_comm_init (int * num_comms)
{
  int comm_size;
  int comm_rank;
  MPI_Group tmp_group, tmp_group2;
  MPI_Comm tmp_comm = MPI_COMM_NULL;
  int i;
  int count_comms;
  int INTERCOMM_TO_MERGE;
#ifdef HAVE_MPI_CLUSTER_SIZE
  int cluster_size;
  int flag;
#endif

  MPI_Comm_size (MPI_COMM_WORLD, &comm_size);
  MPI_Comm_rank (MPI_COMM_WORLD, &comm_rank);

  /*
   * Finish definition of MPI_COMM_WORLD
   */
  comms[0].size = comm_size;
  if ((comms[0].mapping = malloc (comm_size * sizeof(int))) == NULL)
    ERROR (errno, "malloc");
  for (i = 0; i < comm_size; i++)
    comms[0].mapping[i] = i;
  count_comms = 3;

  /*
   * Create a duplicate of MPI_COMM_WORLD
   * Might be used for communication concurrently to MPI_COMM_WORLD
   */
  {
    strncpy (comms[count_comms].description, "Duplicated MPI_COMM_WORLD", TST_DESCRIPTION_LEN);
    comms[count_comms].size = comm_size;
    if ((comms[count_comms].mapping = malloc (comm_size * sizeof(int))) == NULL)
      ERROR (errno, "malloc");
    for (i = 0; i < comm_size; i++)
      comms[count_comms].mapping[i] = i;

    DEBUG (printf ("\n"));
    MPI_CHECK (MPI_Comm_dup (MPI_COMM_WORLD, &comms[count_comms].mpi_comm));

    /*
     * Remark, that if the communicator's size is one, we are also an TST_MPI_COMM_SELF
     */
    if (comms[count_comms].size > 1)
      comms[count_comms].class = TST_MPI_INTRA_COMM;
    else
      comms[count_comms].class = TST_MPI_COMM_SELF;

    comms[count_comms].other_size = 0;
    comms[count_comms].other_mapping = NULL;

    INTERNAL_CHECK
      (
       int tmp_rank;
       int tmp_size;
       MPI_Comm_size (comms[count_comms].mpi_comm, &tmp_size);
       MPI_Comm_rank (comms[count_comms].mpi_comm, &tmp_rank);
       if (tmp_size != comm_size || tmp_rank != comm_rank)
         ERROR (EINVAL, "CHECK for Reversed MPI_COMM_WORLD failed");
       );

    if (++count_comms > COMM_NUM)
      ERROR (EINVAL, "Too many communicators, increase COMM_NUM");
  }

  /*
   * Create a communicator, which is reversed
   */
  {
    strncpy (comms[count_comms].description, "Reversed MPI_COMM_WORLD", TST_DESCRIPTION_LEN);
    comms[count_comms].size = comm_size;
    if ((comms[count_comms].mapping = malloc (comm_size * sizeof(int))) == NULL)
      ERROR (errno, "malloc");
    for (i = 1; i <= comm_size; i++)
      comms[count_comms].mapping[i-1] = comm_size - i;

    MPI_CHECK (MPI_Comm_group (MPI_COMM_WORLD, &tmp_group));
    MPI_CHECK (MPI_Group_incl (tmp_group, comm_size, comms[count_comms].mapping, &tmp_group2));
    MPI_CHECK (MPI_Comm_create (MPI_COMM_WORLD, tmp_group2, &comms[count_comms].mpi_comm));
    MPI_CHECK (MPI_Group_free (&tmp_group));
    MPI_CHECK (MPI_Group_free (&tmp_group2));

    /*
     * Remark, that if the communicator's size is one, we are also an TST_MPI_COMM_SELF
     */
    if (comms[count_comms].size > 1)
      comms[count_comms].class = TST_MPI_INTRA_COMM;
    else
      comms[count_comms].class = TST_MPI_COMM_SELF;
    comms[count_comms].other_size = 0;
    comms[count_comms].other_mapping = NULL;

    INTERNAL_CHECK (
                    int tmp_rank;
                    int tmp_size;
                    MPI_Comm_size (comms[count_comms].mpi_comm, &tmp_size);
                    MPI_Comm_rank (comms[count_comms].mpi_comm, &tmp_rank);
                    if (tmp_size != comm_size || tmp_rank != comm_size - comm_rank)
                    ERROR (EINVAL, "CHECK for Reversed MPI_COMM_WORLD failed");
                    );

    if (++count_comms > COMM_NUM)
      ERROR (EINVAL, "Too many communicators, increase COMM_NUM");
  }

  /*
   * Create a communicator, which is split in two halfs.
   * WATCH OUT, ONE process may contain MPI_COMM_NULL
   */
  {
    strncpy (comms[count_comms].description, "Halfed MPI_COMM_WORLD", TST_DESCRIPTION_LEN);
    comms[count_comms].size = comm_size / 2;
    if ((comms[count_comms].mapping = malloc (comms[count_comms].size * sizeof(int))) == NULL)
      ERROR (errno, "malloc");
    for (i = 0; i < comms[count_comms].size; i++)
      comms[count_comms].mapping[i] = i;

    MPI_CHECK (MPI_Comm_split (MPI_COMM_WORLD, comm_rank >= comm_size / 2,
                               comm_rank, &comms[count_comms].mpi_comm));

    /*
     * Remark, that if the communicator's size is one, we are also an TST_MPI_COMM_SELF
     */
    if (comms[count_comms].size > 1)
      comms[count_comms].class = TST_MPI_INTRA_COMM;
    else
      comms[count_comms].class = TST_MPI_COMM_SELF;

    comms[count_comms].other_size = 0;
    comms[count_comms].other_mapping = NULL;

    if (++count_comms > COMM_NUM)
      ERROR (EINVAL, "Too many communicators, increase COMM_NUM");
  }

  /*
   * Create a communicator which is all the even and one with all the odd processes
   * So comm_size is for an even MPI_COMM_WORLD comm_size / 2 and
   *   for odd MPI_COMM_WORLD: comm_size / 2 + 1 for all even processes!
   */
  {
    strncpy (comms[count_comms].description, "Odd/Even split MPI_COMM_WORLD", TST_DESCRIPTION_LEN);
    if (comm_size % 2)
      {
        /* Odd number of processes in MPI_COMM_WORLD */
        comms[count_comms].size = comm_size / 2 + (comm_rank+1) % 2;
      }
    else
      comms[count_comms].size = comm_size / 2;

    if ((comms[count_comms].mapping = malloc (comms[count_comms].size * sizeof(int))) == NULL)
      ERROR (errno, "malloc");

    for (i = 0; i < comms[count_comms].size; i++)
      comms[count_comms].mapping[i] = i*2 + (comm_rank % 2);

    MPI_CHECK (MPI_Comm_split (MPI_COMM_WORLD, comm_rank % 2, comm_rank, &comms[count_comms].mpi_comm));


    /*
     * Remark, that if the communicator's size is one, we are also an TST_MPI_COMM_SELF
     */
    if (comms[count_comms].size > 1)
      comms[count_comms].class = TST_MPI_INTRA_COMM;
    else
      comms[count_comms].class = TST_MPI_COMM_SELF;

    comms[count_comms].other_size = 0;
    comms[count_comms].other_mapping = NULL;
    if (++count_comms > COMM_NUM)
      ERROR (EINVAL, "Too many communicators, increase COMM_NUM");
  }

  /*
   * Create a inter-communicator with process zero on one side and all the others
   * on the other side!
   */
  if (comm_size > 1)
    {
      int rrank;
      if (tmp_comm != MPI_COMM_NULL)
        MPI_CHECK (MPI_Comm_free (&tmp_comm));
      strncpy (comms[count_comms].description, "Zero-and-Rest Intercommunicator", TST_DESCRIPTION_LEN);

      if (comm_rank == 0)
        {
          comms[count_comms].size = 1;
          comms[count_comms].other_size = comm_size-1;
        }
      else
        {
          comms[count_comms].size = comm_size-1;
          comms[count_comms].other_size = 1;
        }

      if ((comms[count_comms].mapping = malloc (comms[count_comms].size * sizeof(int))) == NULL)
        ERROR (errno, "malloc");

      if ((comms[count_comms].other_mapping = malloc (comms[count_comms].other_size * sizeof(int))) == NULL)
        ERROR (errno, "malloc");

      if (comm_rank == 0)
        {
          comms[count_comms].mapping[0] = 0;

          for (i = 0; i < comms[count_comms].other_size; i++)
            comms[count_comms].other_mapping[i] = i+1;
          rrank = 1;
        }
      else
        {
          for (i = 0; i < comms[count_comms].size; i++)
            comms[count_comms].mapping[i] = i+1;

          comms[count_comms].other_mapping[0] = 0;
          rrank = 0;
        }

      MPI_CHECK (MPI_Comm_split (MPI_COMM_WORLD, comm_rank > 0, comm_rank, &tmp_comm));
      MPI_CHECK (MPI_Intercomm_create (tmp_comm, 0, MPI_COMM_WORLD, rrank, count_comms,
                                      &comms[count_comms].mpi_comm));

      MPI_CHECK (MPI_Comm_free (&tmp_comm));
      comms[count_comms].class = TST_MPI_INTER_COMM;
      if (++count_comms > COMM_NUM)
        ERROR (EINVAL, "Too many communicators, increase COMM_NUM");
    }

  /*
   * Create a halfed inter-communicator with all processes < comm_size/2 on one side
   * and all the others on the other side!
   */
  if (comm_size > 1)
    {
      {
        INTERCOMM_TO_MERGE = count_comms;

        strncpy (comms[count_comms].description, "Halfed Intercommunicator", TST_DESCRIPTION_LEN);

        if (comm_rank > comm_size / 2)
          {
            comms[count_comms].size = comm_size / 2 + comm_size % 2;
            comms[count_comms].other_size = comm_size/2;
          }
        else
          {
            comms[count_comms].size = comm_size / 2;
            comms[count_comms].other_size = comm_size/2 + comm_size % 2;
          }

        if ((comms[count_comms].mapping = malloc (comms[count_comms].size * sizeof(int))) == NULL)
          ERROR (errno, "malloc");
        for (i = 0; i < comms[count_comms].size; i++)
          comms[count_comms].mapping[i] = i;

        if ((comms[count_comms].other_mapping = malloc (comms[count_comms].other_size * sizeof(int))) == NULL)
          ERROR (errno, "malloc");
        for (i = 0; i < comms[count_comms].other_size; i++)
          comms[count_comms].other_mapping[i] = comms[count_comms].size + i;

        MPI_CHECK (MPI_Comm_split (MPI_COMM_WORLD, comm_rank >= comm_size / 2, comm_rank, &tmp_comm));

        /*
        * The MPI-standard doesn't require the remote_leader to be the same on all processes.
        * Here, we specify for process zero the correct value, all others
        * (including the process comm_size/2 gets the value 0 -- which is correct for him, but
        * not the others).
        *
        * More correct would be: (comm_rank < comm_size/2 ? comm_size/2 : 0).
        */
        MPI_CHECK (MPI_Intercomm_create (tmp_comm,
                                        0,
                                        MPI_COMM_WORLD,
                                        (comm_rank == 0) ? comm_size / 2 : 0,
                                        count_comms,
                                        &comms[count_comms].mpi_comm));


        MPI_CHECK (MPI_Comm_free (&tmp_comm));

        comms[count_comms].class = TST_MPI_INTER_COMM;
        if (++count_comms > COMM_NUM)
          ERROR (EINVAL, "Too many communicators, increase COMM_NUM");
      }

      /*
      * Create an Intra-communicator merged out of the communicator created above...
      */
      {
        strncpy (comms[count_comms].description, "Intracomm merged of the Halfed Intercomm", TST_DESCRIPTION_LEN);
        comms[count_comms].size = comm_size;
        comms[count_comms].other_size = 0;
        comms[count_comms].other_mapping = NULL;

        if ((comms[count_comms].mapping = malloc (comms[count_comms].size * sizeof(int))) == NULL)
          ERROR (errno, "malloc");
        for (i = 0; i < comm_size; i++)
          comms[count_comms].mapping[i] = i;

        MPI_CHECK (MPI_Intercomm_merge (comms[INTERCOMM_TO_MERGE].mpi_comm,
                                        /*                                    (comm_rank > comm_size/2),*/
                                        0,
                                        &comms[count_comms].mpi_comm));
        /*
        * A merged intercommunicator must have at least two processes!
        */
        comms[count_comms].class = TST_MPI_INTRA_COMM;
        if (++count_comms > COMM_NUM)
          ERROR (EINVAL, "Too many communicators, increase COMM_NUM");
      }
    }
#ifdef HAVE_MPI_CLUSTER_SIZE
  /*
   * Create a communicator which is split onto the number of comps
   */
  MPI_Attr_get (MPI_COMM_WORLD, MPI_CLUSTER_SIZE, &cluster_size, &flag);
  cluster_size = *(int*)cluster_size;
  if (flag)
    {
      int * tmp_array;
      int j;
      int cluster_color;

      if (cluster_size < 1 || cluster_size > comm_size)
        ERROR (EINVAL, "Invalid value for cluster_size");

      strncpy (comms[count_comms].description, "Communicator split on Metacomputer boundaries", TST_DESCRIPTION_LEN);

      if ((tmp_array = malloc (comm_size * sizeof (int))) == NULL)
        ERROR (errno, "malloc");

      MPI_Attr_get (MPI_COMM_WORLD, MPI_CLUSTER_COLOR, &cluster_color, &flag);
      if (!flag)
        ERROR (EINVAL, "MPI defines MPI_CLUSTER_SIZE, but does not define MPI_CLUSTER_COLOR");

      cluster_color = *(int*)cluster_color;
      if (cluster_color < 0 || cluster_color > cluster_size)
        ERROR (EINVAL, "Invalid value for cluster_color");

      /*
       * Collect values for our mapping
       */
      MPI_Allgather (&cluster_color, 1, MPI_INT, tmp_array, 1, MPI_INT, MPI_COMM_WORLD);

      if ((comms[count_comms].mapping = malloc (comms[count_comms].size * sizeof(int))) == NULL)
        ERROR (errno, "malloc");
      /*
       * Count the number of processes on this side.
       */
      /*for (j = 0, i = 0; i < comm_size && tmp_array[i] < cluster_color; i++)*/
      comms[count_comms].size = 0;
      for (i = 0; i < comm_size; i++)
        if (tmp_array[i] == cluster_color)
          comms[count_comms].size++;

      if (comms[count_comms].size == 0)
        ERROR (EINVAL, "Couldn't determine cluster-color in tmp_array");

      if ((comms[count_comms].mapping = malloc (comms[count_comms].size * sizeof(int))) == NULL)
        ERROR (errno, "malloc");

      for (j = 0, i = 0 ; i < comm_size; i++)
        {
          if (tmp_array[i] == cluster_color)
            {
              comms[count_comms].mapping[j] = i;
              if (++j > comms[count_comms].size)
                ERROR (EINVAL, "Too many processes in this cluster_color");
            }
          DEBUG(printf ("(Rank:%d) cluster_color:%d tmp_array[%d]:%d\n",
                        tst_global_rank, cluster_color, i, tmp_array[i]));
        }

      /*
       * With all this information create and check the mpi_comm
       */
      MPI_Comm_split (MPI_COMM_WORLD, cluster_color, comm_rank, &comms[count_comms].mpi_comm);

      comms[count_comms].class = TST_MPI_INTRA_COMM;
      comms[count_comms].other_size = 0;

      INTERNAL_CHECK (
        int tmp_rank;
        int tmp_size;
        MPI_Comm_size (comms[count_comms].mpi_comm, &tmp_size);
        MPI_Comm_rank (comms[count_comms].mpi_comm, &tmp_rank);
        if (tmp_size != j)
          ERROR (EINVAL, "Invalid size for local communicator after cluster_color MPI_Comm_split");
      );
      free (tmp_array);

      if (++count_comms > COMM_NUM)
        ERROR (EINVAL, "Too many communicators, increase COMM_NUM");

      /*
       * Create a inter-communicator with all odd processes on one side and all the others
       * on the other side!
       */
    }
#endif

   *num_comms = count_comms;
  return 0;
}


MPI_Comm tst_comm_getcomm (int i)
{
  CHECK_ARG (i, MPI_COMM_NULL);

  return comms[i].mpi_comm;
}

int tst_comm_getcommclass (int i)
{
  CHECK_ARG (i, -1);

  return comms[i].class;
}

const char * tst_comm_getdescription (int i)
{
  CHECK_ARG (i, NULL);

  return comms[i].description;
}


void tst_comm_list (void)
{
  int i;
  for (i = 0; i < TST_COMMS_NUM; i++)
    {
      if (comms[i].description[0] == '\0')
        break;
      printf ("Communicator:%d %s\n",
              i, comms[i].description);
    }

  for (i = 0; i < TST_COMMS_CLASS_NUM; i++)
    printf ("Communicator-Class:%d %s\n",
            i, tst_comms_class_strings[i]);
}


static int tst_comm_search (const int search_test, const int * test_list, const int test_list_num)
{
  int k;

  for (k = 0; k < test_list_num; k++)
    if (test_list[k] == search_test)
      break;
  return (k == test_list_num) ? 0 : 1;
}

int tst_comm_select (const char * comm_string, int * comm_list, int * comm_list_num, const int comm_list_max)
{
  int i;

  if (comm_string == NULL || comm_list == NULL || comm_list_num == NULL)
    ERROR (EINVAL, "Passed a NULL parameter");

  for (i = 0; i < TST_COMMS_CLASS_NUM; i++)
    {
      /*
       * In case we match a complete class of tests, include every one!
       */
      if (!strcasecmp (comm_string, tst_comms_class_strings[i]))
        {
          int j;
          DEBUG (printf ("comm_string:%s matched with tst_comms_class_strings[%d]:%s\n",
                         comm_string, i, tst_comms_class_strings[i]));
          for (j = 0; j < TST_COMMS_NUM; j++)
            {
              /*
               * First search for this test in the comm_list -- if already in, continue!
               */
              if (tst_comm_search (j, comm_list, *comm_list_num))
                {
                  WARNING (printf ("Comm:%s selected through class:%s was already "
                                   "included in list -- not including\n",
                                   comms[j].description,
                                   tst_comms_class_strings[i]));
                  continue;
                }

              if (comms[j].class & (1 << i))
                {
                  DEBUG (printf ("comm_string:%s test j:%d i:%d with class:%d matches, comm_list_num:%d\n",
                                 comm_string, j, (1 << i), comms[j].class, *comm_list_num));
                  comm_list[*comm_list_num] = j;
                  (*comm_list_num)++;
                  if (*comm_list_num == comm_list_max)
                    ERROR (EINVAL, "Too many user selected tests");
                }
            }
          return 0;
        }
    }

  for (i = 0; i < TST_COMMS_NUM; i++)
    {
      if (!strcasecmp (comm_string, comms[i].description))
        {
          if (tst_comm_search (i, comm_list, *comm_list_num))
            {
              WARNING (printf ("Comm:%s was already included in list -- not including\n",
                               comms[i].description));
              return 0;
            }
          comm_list[*comm_list_num] = i;
          (*comm_list_num)++;
          if (*comm_list_num == comm_list_max)
            ERROR (EINVAL, "Too many user selected tests");

          DEBUG (printf ("comm_string:%s matched with comm_list_num:%d\n",
                         comm_string, *comm_list_num));

          return 0;
        }
    }

  {
    char buffer[128];
    sprintf (buffer, "Communicator %s not recognized",
             comm_string);
    ERROR (EINVAL, buffer);
  }
  return 0;
}
