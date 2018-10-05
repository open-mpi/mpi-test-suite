/** \todo Check: If the communicator's size is one, are we also an TST_MPI_COMM_SELF ? */

#include "config.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <limits.h>

#include <mpi.h>
#include "mpi_test_suite.h"
#include "tst_threads.h"


#define COMM_NUM 32


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
#if MPI_VERSION >= 3
    , "SHARED_COMM"
#endif
  };


struct comm {
  MPI_Comm mpi_comm;                       /* The actual MPI communicator */
  MPI_Comm *mpi_thread_comms;              /* List of duplicate MPI communicators used for threads */
  char description [TST_DESCRIPTION_LEN];  /* The communicator's description */
  int class;                               /* Class of communicator */
  int size;                                /* Size of this communicator */
  int * mapping;                           /* Our mapping of the communicator */
  int other_size;                          /* In case of intra-comms, the size of the other communicator */
  int * other_mapping;                     /* In case of intra-comms, the mapping of the other communicator */
};

static int num_registered_comms = 0;

static struct comm comms[COMM_NUM];


int tst_comm_register(char *description, MPI_Comm mpi_comm, int class, int size, int *mapping, int other_size, int *other_mapping) {
  assert(num_registered_comms < COMM_NUM);
  int i;
  int num_threads = tst_thread_num_threads();
  comms[num_registered_comms].mpi_comm = mpi_comm;
  comms[num_registered_comms].mpi_thread_comms = (MPI_Comm *) malloc(num_threads * sizeof(MPI_Comm));
  for (i = 0; i < num_threads; i++) {
    if(mpi_comm != MPI_COMM_NULL) {
      MPI_CHECK (MPI_Comm_dup(mpi_comm, &comms[num_registered_comms].mpi_thread_comms[i]));
    }
    else {
      comms[num_registered_comms].mpi_thread_comms[i] = MPI_COMM_NULL;
    }
  }
  strncpy(comms[num_registered_comms].description, description, TST_DESCRIPTION_LEN);
  comms[num_registered_comms].class = class;
  comms[num_registered_comms].size = size;
  comms[num_registered_comms].mapping = mapping;
  comms[num_registered_comms].other_size = other_size;
  comms[num_registered_comms].other_mapping = other_mapping;
  num_registered_comms++;
  return 0;
}


int tst_comm_register_comm_world() {
  int comm_size = 1;
  MPI_CHECK (MPI_Comm_size(MPI_COMM_WORLD, &comm_size));
  int i;
  int *mapping = (int *) malloc(comm_size * sizeof(int));
  if (NULL == mapping) {
    ERROR (errno, "malloc");
  }
  for (i = 0; i < comm_size; i++) {
    mapping[i] = i;
  }
  tst_comm_register("MPI_COMM_WORLD", MPI_COMM_WORLD, TST_MPI_INTRA_COMM, comm_size, mapping, 0, NULL);
  return 0;
}

int tst_comm_register_comm_null() {
  tst_comm_register("MPI_COMM_NULL", MPI_COMM_NULL, TST_MPI_COMM_NULL, 0, NULL, 0, NULL);
  return 0;
}

int tst_comm_register_comm_self() {
  tst_comm_register("MPI_COMM_SELF", MPI_COMM_SELF, TST_MPI_COMM_SELF, 1, NULL, 0, NULL);
  return 0;
}

int tst_comm_register_duplicate_comm_world() {
  MPI_Comm comm;
  int comm_size = 1;
  int i;
  MPI_CHECK (MPI_Comm_dup (MPI_COMM_WORLD, &comm));
  MPI_CHECK (MPI_Comm_size (comm, &comm_size));
  int *mapping = (int *) malloc(comm_size * sizeof(int));
  if (NULL == mapping) {
    ERROR (errno, "malloc");
  }
  for (i = 0; i < comm_size; i++) {
    mapping[i] = i;
  }

  INTERNAL_CHECK (
    int tmp_rank; int comm_rank;
    int tmp_size; int comm_size;
    MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &comm_rank);
    MPI_Comm_size(comm, &tmp_size);
    MPI_Comm_rank(comm, &tmp_rank);
    if (tmp_size != comm_size || tmp_rank != comm_rank)
      ERROR (EINVAL, "CHECK for Reversed MPI_COMM_WORLD failed");
  );

  tst_comm_register("Duplicated MPI_COMM_WORLD", comm, TST_MPI_INTRA_COMM, comm_size, mapping, 0, NULL);
  return 0;
}

int tst_comm_register_reversed_comm_world() {
  MPI_Comm comm;
  int comm_size = 1;
  int i;
  MPI_Group tmp_group, tmp_group2;

  MPI_CHECK (MPI_Comm_size(MPI_COMM_WORLD, &comm_size));
  int *mapping = (int *) malloc(comm_size * sizeof(int));
  if (NULL == mapping) {
    ERROR (errno, "malloc");
  }
  for (i = 1; i <= comm_size; i++) {
    mapping[i - 1] = comm_size - i;
  }
  MPI_CHECK (MPI_Comm_group(MPI_COMM_WORLD, &tmp_group));
  MPI_CHECK (MPI_Group_incl(tmp_group, comm_size, mapping, &tmp_group2));
  MPI_CHECK (MPI_Comm_create(MPI_COMM_WORLD, tmp_group2, &comm));
  MPI_CHECK (MPI_Group_free(&tmp_group));
  MPI_CHECK (MPI_Group_free(&tmp_group2));
  MPI_CHECK (MPI_Comm_size(comm, &comm_size));

  INTERNAL_CHECK (
    int tmp_rank; int comm_rank;
    int tmp_size; int comm_size;
    MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &comm_rank);
    MPI_Comm_size(comm, &tmp_size);
    MPI_Comm_rank(comm, &tmp_rank);
    if (tmp_size != comm_size || tmp_rank != comm_size - comm_rank-1)
      ERROR (EINVAL, "CHECK for Reversed MPI_COMM_WORLD failed");
  );

  tst_comm_register("Reversed MPI_COMM_WORLD", comm, TST_MPI_INTRA_COMM, comm_size, mapping, 0, NULL);
  return 0;
}


int tst_comm_register_halved_comm_world() {
  MPI_Comm comm;
  int comm_size = 1;
  int i;
  int world_size = -1;
  int world_rank = -1;

  MPI_CHECK (MPI_Comm_size(MPI_COMM_WORLD, &world_size));
  MPI_CHECK (MPI_Comm_rank(MPI_COMM_WORLD, &world_rank));
  comm_size = world_size / 2;
  int *mapping = (int *) malloc(comm_size * sizeof(int));
  if (NULL == mapping) {
    ERROR (errno, "malloc");
  }
  /** \todo Check mapping definition */
  for (i = 0; i < comm_size; i++) {
    mapping[i] = i;
  }
  MPI_CHECK (MPI_Comm_split (MPI_COMM_WORLD, world_rank >= comm_size,
                               world_rank, &comm));
  /** \todo WATCH OUT, ONE process may contain MPI_COMM_NULL */
  MPI_CHECK (MPI_Comm_size(comm, &comm_size));

  tst_comm_register("Halved MPI_COMM_WORLD", comm, TST_MPI_INTRA_COMM, comm_size, mapping, 0, NULL);
  return 0;
}


int tst_comm_register_2D_cart_comm() {
  int comm_size;
  MPI_CHECK (MPI_Comm_size(MPI_COMM_WORLD, &comm_size));
  if (comm_size > 1) {
    MPI_Comm comm;
    int i;
    int dims[2] = {0, 0};
    int periods[2] = {1, 1};
    int *mapping = (int *) malloc(comm_size * sizeof(int));
    if (NULL == mapping) {
      ERROR (errno, "malloc");
    }
    /** \todo Check mapping definition */
    for (i = 0; i < comm_size; i++) {
      mapping[i] = i;
    }
    MPI_CHECK (MPI_Dims_create(comm_size, 2, dims));
    MPI_CHECK (MPI_Cart_create(MPI_COMM_WORLD, 2, dims, periods, 1, &comm));

    tst_comm_register("2D Cart_comm", comm, TST_MPI_CART_COMM, comm_size, mapping, 0, NULL);
  }
  return 0;
}


int tst_comm_register_3D_cart_comm() {
  int comm_size;
  MPI_CHECK (MPI_Comm_size(MPI_COMM_WORLD, &comm_size));
  if (comm_size > 1) {
    MPI_Comm comm;
    int i;
    int dims[3] = {0, 0, 0};         /* Set to zero in order to receive value */
    int periods[3] = {0, 0, 0};
    int *mapping = (int *) malloc(comm_size * sizeof(int));
    if (NULL == mapping) {
      ERROR (errno, "malloc");
    }
    /** \todo Check mapping definition */
    for (i = 0; i < comm_size; i++) {
      mapping[i] = i;
    }
    MPI_CHECK(MPI_Dims_create(comm_size, 3, dims));
    MPI_CHECK(MPI_Cart_create(MPI_COMM_WORLD, 3, dims, periods, 1, &comm));

    tst_comm_register("3D Cart_comm", comm, TST_MPI_CART_COMM, comm_size, mapping, 0, NULL);
  }
  return 0;
}

int tst_comm_register_odd_even_split() {
  MPI_Comm comm;
  int i;
  int comm_size = 1;
  int world_size = -1;
  int world_rank = -1;

  MPI_CHECK (MPI_Comm_size(MPI_COMM_WORLD, &world_size));
  MPI_CHECK (MPI_Comm_rank(MPI_COMM_WORLD, &world_rank));

  MPI_CHECK (MPI_Comm_split(MPI_COMM_WORLD, world_rank % 2, world_rank, &comm));
  MPI_CHECK (MPI_Comm_size(comm, &comm_size));

  int *mapping = (int *) malloc(comm_size * sizeof(int));
  if (NULL == mapping) {
    ERROR (errno, "malloc");
  }
  for (i = 0; i < comm_size; i++) {
    mapping[i] = i*2 + (world_rank % 2);
  }

  tst_comm_register("Odd/Even split MPI_COMM_WORLD", comm, TST_MPI_INTRA_COMM, comm_size, mapping, 0, NULL);
  return 0;
}

int tst_comm_register_fully_connected_topology() {
  MPI_Comm comm;
  int i;
  int comm_size = 1;

  MPI_CHECK (MPI_Comm_size(MPI_COMM_WORLD, &comm_size));
  int *mapping = (int *) malloc(comm_size * sizeof(int));
  if (NULL == mapping) {
    ERROR (errno, "malloc");
  }
  for (i = 0; i < comm_size; i++) {
    mapping[i] = i;
  }
  int *index=NULL;
  int *edges=NULL;
  int j, num;
  /*allocate index*/
  if((index = (int*)malloc(sizeof(int) * comm_size))==NULL)
    ERROR(errno, "malloc");
  for (i=0; i < comm_size; i++)
    index[i] = (i+1)*(comm_size-1);
  /*allocate edges*/
  if((edges = (int*)malloc(sizeof(int) * comm_size * (comm_size - 1)))==NULL)
    ERROR(errno,"malloc");
  num=0;
  for(i=0; i < comm_size; i++) {
    for(j=0; j < comm_size;j++) {
      if(j==i) {
        continue;
      }
      edges[num] = j;
      num++;
    }
  }
  MPI_CHECK (MPI_Graph_create(MPI_COMM_WORLD, comm_size, index, edges, 1, &comm));

  free(index);
  free(edges);

  tst_comm_register("Full-connected Topology", comm, TST_MPI_TOPO_COMM, comm_size, mapping, 0, NULL);
  return 0;
}


/*
 * Create a halved inter-communicator with all processes < comm_size/2 on one side
 * and all the others on the other side!
 */
int tst_comm_register_halved_inter_comm() {
  int world_size;
  MPI_CHECK (MPI_Comm_size(MPI_COMM_WORLD, &world_size));

  if (world_size > 1) {
    int i;
    MPI_Comm comm;
    MPI_Comm tmp_comm;
    int world_rank;
    MPI_CHECK (MPI_Comm_rank(MPI_COMM_WORLD, &world_rank));
    MPI_CHECK (MPI_Comm_split(MPI_COMM_WORLD, world_rank >= world_size / 2, world_rank, &tmp_comm));

    int comm_size;
    MPI_CHECK (MPI_Comm_size(MPI_COMM_WORLD, &comm_size));
    int other_size = world_size - comm_size;

    int *mapping;
    if ((mapping = malloc (comm_size * sizeof(int))) == NULL)
      ERROR (errno, "malloc");
    for (i = 0; i < comm_size; i++)
      mapping[i] = i;

    int *other_mapping;
    if ((other_mapping = malloc (other_size * sizeof(int))) == NULL)
      ERROR (errno, "malloc");
    for (i = 0; i < other_size; i++)
      other_mapping[i] = comm_size + i;

    /*
     * The MPI-standard doesn't require the remote_leader to be the same on all processes.
     * Here, we specify for process zero the correct value, all others
     * (including the process world_size/2 gets the value 0 -- which is correct for him, but
     * not the others).
     *
     * More correct would be: (comm_rank < comm_size/2 ? comm_size/2 : 0).
     */
    MPI_CHECK (MPI_Intercomm_create(tmp_comm,
                                    0,
                                    MPI_COMM_WORLD,
                                    (world_rank == 0) ? world_size / 2 : 0,
                                    num_registered_comms,
                                    &comm));

    MPI_CHECK (MPI_Comm_free (&tmp_comm));

    tst_comm_register("Halved Inter_communicator", comm, TST_MPI_INTER_COMM, comm_size, mapping, other_size, other_mapping);
  }
  return 0;
}

int tst_comm_register_merged_inter_comm() {
  int world_size;
  MPI_CHECK (MPI_Comm_size(MPI_COMM_WORLD, &world_size));

  if (world_size > 1) {
    /* Create an Intra-communicator merged out of the "Halved Inter_communicator" communicator */
    int i;
    MPI_Comm comm;
    int halved_inter_comm_Id;
    for(halved_inter_comm_Id = 0; halved_inter_comm_Id <num_registered_comms; halved_inter_comm_Id++) {
      if (strcmp("Halved Inter_communicator", comms[halved_inter_comm_Id].description) == 0) {
        break;
      }
    }

    int comm_size;
    MPI_CHECK (MPI_Intercomm_merge(comms[halved_inter_comm_Id].mpi_comm, 0, &comm));
    MPI_CHECK (MPI_Comm_size(comm, &comm_size));

    int *mapping;
    if ((mapping = malloc (comm_size * sizeof(int))) == NULL)
      ERROR (errno, "malloc");
    for (i = 0; i < comm_size; i++)
      mapping[i] = i;

    tst_comm_register("Intracomm merged of the Halved Inter_communicator", comm, TST_MPI_INTRA_COMM, comm_size, mapping, 0, NULL);
  }
  return 0;
}


int tst_comm_register_split_type_shared() {
#if MPI_VERSION >= 3
  int i;
  MPI_Comm comm;
  int comm_size;
  int world_rank;
  MPI_CHECK(MPI_Comm_rank(MPI_COMM_WORLD, &world_rank));
  MPI_CHECK (MPI_Comm_split_type(MPI_COMM_WORLD, MPI_COMM_TYPE_SHARED, world_rank, MPI_INFO_NULL, &comm));
  MPI_CHECK(MPI_Comm_size(comm, &comm_size));
  int *mapping;
  if ((mapping = malloc (comm_size * sizeof(int))) == NULL) {
    ERROR (errno, "malloc");
  }
  for (i = 0; i < comm_size; i++) {
    mapping[i] = i;
  }

  tst_comm_register("MPI_COMM_TYPE_SHARED comm", comm, TST_MPI_SHARED_COMM | TST_MPI_INTRA_COMM, comm_size, mapping, 0, NULL);
#endif
  return 0;
}


int tst_comms_init(int * num_comms) {

  tst_comm_register_comm_world();
  tst_comm_register_comm_null();
  tst_comm_register_comm_self();
  tst_comm_register_duplicate_comm_world();
  tst_comm_register_reversed_comm_world();
  tst_comm_register_halved_comm_world();
  tst_comm_register_2D_cart_comm();
  tst_comm_register_3D_cart_comm();
  tst_comm_register_odd_even_split();
  tst_comm_register_fully_connected_topology();
  tst_comm_register_halved_inter_comm();
  tst_comm_register_merged_inter_comm();
  tst_comm_register_split_type_shared();

  *num_comms = num_registered_comms;
  return 0;

  int count_comms = num_registered_comms;
  int comm_size;
  int comm_rank;
  MPI_Comm tmp_comm = MPI_COMM_NULL;
  int i;

  MPI_Comm_size (MPI_COMM_WORLD, &comm_size);
  MPI_Comm_rank (MPI_COMM_WORLD, &comm_rank);


  /*
   * Create a inter-communicator with process zero on one side and all the others
   * on the other side!
   */
#if !defined(HAVE_MPI_PACX) /* HMM; TEST FAILED WITH PACX AND PACXTRACE */
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
#endif /* !HAVE_MPI_PACX */




#ifdef HAVE_MPI_CLUSTER_SIZE
    {
      int cluster_size;
      int flag;
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
              tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "(Rank:%d) cluster_color:%d tmp_array[%d]:%d\n",
                            tst_global_rank, cluster_color, i, tmp_array[i]);
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
    }
#endif

  *num_comms = count_comms;
  return 0;
}


int tst_comm_cleanup () {
  int i;
  for (i = 0; i < num_registered_comms; i++) {
    if (NULL == ((void*)comms[i].mpi_comm) || MPI_COMM_NULL == comms[i].mpi_comm)
      continue;

    free(comms[i].mapping);
    free(comms[i].other_mapping);
    MPI_Comm_free(&comms[i].mpi_comm);
    int j;
    for (j = 0; j < tst_thread_num_threads(); j++) {
      MPI_Comm_free(&comms[i].mpi_thread_comms[j]);
    }
  }
  return 0;
}


MPI_Comm tst_comm_getcomm (int i) {

  CHECK_ARG (i, MPI_COMM_NULL);

  if (tst_thread_get_num() > 0) {
    int threadId = tst_thread_get_num();
    return comms[i].mpi_thread_comms[threadId - 1];
  }
  else {
    return comms[i].mpi_comm;
  }
}

int tst_comm_getcommsize (int i)
{
  int size;
  CHECK_ARG (i, -1);
  /* XXX Niethammer: Some log output calls this also with MPI_COMM_NULL. */
  if(MPI_COMM_NULL == comms[i].mpi_comm) {
    return 0;
  }
  MPI_Comm_size (comms[i].mpi_comm, &size);
  return size;
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


void tst_comm_list() {
  int i;
  for (i = 0; i < num_registered_comms; i++) {
    printf ("Communicator:%d %s\n", i, comms[i].description);
  }
  for (i = 0; i < TST_COMMS_CLASS_NUM; i++) {
    printf ("Communicator-Class:%d %s\n", i, tst_comms_class_strings[i]);
  }
}


static int tst_comm_search (const int search_test, const int * test_list, const int test_list_num)
{
  int k;

  for (k = 0; k < test_list_num; k++)
    if (test_list[k] == search_test)
      break;
  return (k == test_list_num) ? -1 : k;
}

int tst_comm_select (const char * comm_string, int * comm_list, const int comm_list_max, int * comm_list_num) {
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
          tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "comm_string:%s matched with tst_comms_class_strings[%d]:%s\n",
                         comm_string, i, tst_comms_class_strings[i]);
          for (j = 0; j < TST_COMMS_NUM; j++)
            {
              /*
               * First search for this test in the comm_list -- if already in, continue!
               */
              if (-1 != tst_comm_search (j, comm_list, *comm_list_num))
                {
                  WARNING (printf ("Comm:%s selected through class:%s was already "
                                   "included in list -- not including\n",
                                   comms[j].description,
                                   tst_comms_class_strings[i]));
                  continue;
                }

              if (comms[j].class & (1 << i))
                {
                  tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "comm_string:%s test j:%d i:%d with class:%d matches, comm_list_num:%d\n",
                                 comm_string, j, (1 << i), comms[j].class, *comm_list_num);
                  comm_list[*comm_list_num] = j;
                  (*comm_list_num)++;
                  if (*comm_list_num == comm_list_max)
                    ERROR (EINVAL, "Too many user selected tests");
                }
            }
          return 0;
        }
    }

  for (i = 0; i < num_registered_comms; i++)
    {
      if (!strcasecmp (comm_string, comms[i].description))
        {
          if (-1 != tst_comm_search (i, comm_list, *comm_list_num))
            {
              WARNING (printf ("Comm:%s was already included in list -- not including\n",
                               comms[i].description));
              return 0;
            }
          comm_list[*comm_list_num] = i;
          (*comm_list_num)++;
          if (*comm_list_num == comm_list_max)
            ERROR (EINVAL, "Too many user selected tests");

          tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "comm_string:%s matched with comm_list_num:%d\n",
                         comm_string, *comm_list_num);

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

int tst_comm_deselect (const char * comm_string, int * comm_list, const int comm_list_max, int * comm_list_num)
{
  int i;

  if (comm_string == NULL || comm_list == NULL || comm_list_num == NULL)
    ERROR (EINVAL, "Passed a NULL parameter");

  for (i = 0; i < TST_COMMS_CLASS_NUM; i++)
    {
      /*
       * In case we match a complete class of comms, exclude every one!
       */
      if (!strcasecmp (comm_string, tst_comms_class_strings[i]))
        {
          int j;
          tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "comm_string:%s matched with tst_comms_class_strings[%d]:%s\n",
                         comm_string, i, tst_comms_class_strings[i]);
          for (j = 0; j < TST_COMMS_NUM; j++)
            {
              int ret;
              /*
               * Search for this test in the test_list --
               * if it belongs to this class and is already included, deselect
               */
              if (((ret = tst_comm_search (j, comm_list, *comm_list_num)) != 1) &&
                  comms[j].class & (1 << i))
                {
                  tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "comm_string:%s test j:%d i:%d with class:%d matches for deselect, comm_list_num:%d\n",
                                 comm_string, j, (1 << i), comms[j].class, *comm_list_num);
                  comm_list[ret] = -1;
                  (*comm_list_num)--;
                  if (*comm_list_num < 0)
                    ERROR (EINVAL, "Negative selected comms: This should not happen");
                }
            }
          return 0;
        }
    }

  for (i = 0; i < TST_COMMS_NUM; i++)
    {
      if (!strcasecmp (comm_string, comms[i].description))
        {
          int ret;
          if ((ret = tst_comm_search (i, comm_list, *comm_list_num)) == -1)
            {
              WARNING (printf ("Comm:%s was not included in list -- not excluding\n",
                               comms[i].description));
              return 0;
            }
          comm_list[ret] = -1;
          (*comm_list_num)--;
          if (*comm_list_num < 0)
            ERROR (EINVAL, "Negative selected tests: This should not happen");

          tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "comm_string:%s matched with comm_list_num:%d excluding\n",
                         comm_string, *comm_list_num);

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
