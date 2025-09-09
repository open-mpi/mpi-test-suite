# Documentation (beta :wink:) for the MPI-Testsuite

## Copyrights

Copyright (c) 2003-2018 High Performance Computing Center Stuttgart, University of Stuttgart.  All rights reserved.

Copyright (c) 2005-2009 The University of Tennessee and The University of Tennessee Research Foundation.  All rights reserved.

Copyright (c) 2007 The Trustees of Indiana University and Indiana University Research and Technology Corporation.  All rights reserved.

Copyright (c) 2009 Cisco Systems, Inc.  All rights reserved.

Additional copyrights may follow

---------------------------------------------------------------------------

## Overview

1. Introduction
1. Getting Started
    1. Prerequisites
    1. Compiling the MPI-Testsuite
    1. Running the MPI-Testsuite
    1. MPI-implementations already tested
    1. Tests failing on specific platforms
1. Extending the testsuite
    1. Adding new tests


## Introduction

This MPI-testsuite was initially developed for the use with PACX-MPI and
has been extended within the Open MPI project.

The main focus is on:

- High degree of code coverage through combinations of tests.
- Easy maintainability,
- Easy integration of new tests,
- Rich underlying functionality for flexible tests
  (convenience functions for datatypes, comms and checking),
- Only a single binary (for single, since expensive
  `MPI_Init`/`MPI_Finalize`) to make it as quick and easy as possible to
  run automatically.



## Getting Started

### Prerequisites

Required dependencies to build and run the MPI test suite:
- MPI library with C compiler support
- `make`

The following additional dependencies are required when building from the Git repository:
- `gengetopt` (used for the generation of the command line parser code)
- GNU Autoconf, Automake


### Compiling the MPI-Testsuite

The MPI-Testsuite uses a GNU Autotools-based build system.
To configure provide the MPI C compiler with:

```
$ ./configure CC=mpicc
```

After a successful `configure` run, one may compile with `make`:

```
$ make
```

### Running the MPI-Testsuite

The MPI-Testsuite may be run with an arbitrary number of processes.
It runs a variety of P2P and Collective tests with varying
datatypes and preset communicators. Each test specifies which kind of
datatypes -- e.g., struct datatypes and communicators, e.g., `MPI_COMM_SELF`,
intra-comms and the like -- it may run.

Per default, _ALL_ tests will be run with _ALL_ combinations of datatypes
and _ALL_ generated communicators! Only failed tests are shown afterwards.

Through command-line switches, the user may influence and reduce the
number of tests with the following commands, first the showing the help:

```
$ mpirun -np 1 ./mpi_test_suite -h

Usage: mpi_test_suite [OPTION]...

  -h, --help                   Print help and exit
  -V, --version                Print version and exit
  -t, --test=STRING            tests or test-classes  (default=`all')
  -c, --comm=STRING            communicators or commicator-classes
                                 (default=`all')
  -d, --datatype=STRING        datatypes of datatype-classes  (default=`all')
  -n, --num-values=STRING      number of values to communicate in tests
                                 (default=`1000')

All multiple test-/comm-/datatype-names and num-values must be comma-separated.
Names are not case-sensitive, due to spaces in names, propper quoting should be
used. The special name 'all' can be used to select all tests/comms/datatypes.
To exclude a test/comm/datatype prefix it with '^' but be aware, that the
selection will happen in order, so use 'all,^exclude'.

  -a, --atomic-io              enable atomicity for files in I/O for all tests
                                 that support it
  -j, --num-threads=INT        number of additional threads to execute the
                                 tests  (default=`0')
  -r, --report=STRING          level of detail for test report  (possible
                                 values="summary", "run", "full"
                                 default=`summary')
  -x, --execution-mode=STRING  level of correctness testing  (possible
                                 values="disabled", "strict", "relaxed"
                                 default=`relaxed')
  -v, --verbose                enable verbose output for debugging purpose
  -l, --list                   list all available tests, communicators,
                                 datatypes and corresponding classes
```

The following command lists all available tests/test-classes, comms/comm-classes and
type/type-classes:

```
$ mpirun -np 1 ./mpi_test_suite -l

Showing all the individual tests, comms and types would be too long, however,
all of the tests, comms and types are grouped in classes, all of which are
listed with (first the class, followed by number, then a free distinct name):

Num Tests : 100
Environment test:0 Status
Environment test:1 Request_Null
P2P test:2 Ring
...
P2P test:27 Alltoall with topo comm
Collective test:28 Bcast
...
Collective test:46 Alltoall
One-sided test:47 get_fence
...
One-sided test:58 Ring with Put
IO test:59 file simple
...
IO test:99 io_with_hole2
Test-Class:0 Unspecified
Test-Class:1 Environment
Test-Class:2 P2P
Test-Class:3 Collective
Test-Class:4 One-sided
Test-Class:5 Dynamic
Test-Class:6 IO
Test-Class:7 Threaded
Communicator:0 MPI_COMM_WORLD
...
Communicator:12 Intracomm merged of the Halved Intercomm
Communicator-Class:0 COMM_SELF
Communicator-Class:1 COMM_NULL
Communicator-Class:2 INTRA_COMM
Communicator-Class:3 INTER_COMM
Communicator-Class:4 CART_COMM
Communicator-Class:5 TOPO_COMM
Datatype:0 MPI_CHAR
...
Datatype:30 function MPI_Type_dup() n/a
Datatype-Class:0 STANDARD_C_INT_TYPES
Datatype-Class:1 STANDARD_C_FLOAT_TYPES
Datatype-Class:2 STANDARD_C_TYPES
Datatype-Class:3 STRUCT_C_TYPES
Datatype-Class:4 ALL_C_TYPES
Report:0 Summary
Report:1 Run
Report:2 Full
Test mode:0 disabled
Test mode:1 strict
Test mode:2 relaxed
```

For example, the command:

```
$ mpirun -np 32 ./mpi_test_suite -t "Ring\ Bsend,Collective" -c COMM_SELF,INTER_COMM -d MPI_CHAR
```

will run the test called `Ring Bsend` and _ALL_ test-cases belonging to
test-class `Collective` with the communicator `MPI_COMM_SELF` and
_ALL_ communicators which are belonging to communicator-class `INTER_COMM`
(e.g., the communicator called "Zero-and-Rest Intercommunicator") *but only* with
the datatype `MPI_CHAR`.
(However, you may also just specify the corresponding number :wink:).

If no options are specified, _ALL_ tests are run with all applicable
communicators and all applicable datatypes. Of course, if a test is not
applicable for a certain combination (e.g., `Ring` doesn't support
`MPI_COMM_NULL`) it is not being run.


### MPI-implementations already tested

We have run the testsuite successfully on

* Linux/IA32 using MPIch-1.2.7
* Linux/IA32 using LAM-7.0
* Linux/IA32 using PACX-MPI-5.0 on top of MPIch-1.2.5
* Linux/EM64t using MPIch-1.2.4
* NEC SX6 using MPIsx
* NEC SX8 using MPIsx
* Hitachi
* SGI Origin


### Tests failing on specific platforms

On some platforms, we found that some tests fail:

With MPIch-1.2.5 and the ch_shmem-device, we trigger a bug in `Ring Ssend`, which seems to be known:

* P2P tests Alltoall - Issend (9/14), comm Zero-and-Rest Intercommunicator (8/10), type MPI_CHAR (1/18)
* [0] MPI Internal Aborting program Nested call to GetSendPkt
* [0] Nested call to GetSendPkt

The source of MPIch-1.2.5 `mpid/ch_shmem/shmempriv.c`, line 590 explains here:

```c
/* There is an implementation bug in the flow control
   code that can cause MPID_DeviceCheck to call a
   routine that calls this routine. When that happens,
   we'll quickly drop into the same code, so we prefer
   to abort.  The test is here because if we find
   a free packet, it is ok to enter this routine,
   just not ok to enter and then call DeviceCheck */
if (nest_count++ > 0) {
    MPID_Abort( 0, 1, "MPI Internal",
            "Nested call to GetSendPkt" );
}
```


## Extending the testsuite


### Adding new tests

In order to integrate a new test, the programmer should

1. Write three functions in a new file with a descriptive name, like:
    ```
    tst_p2p_simple_ring_bsend.c
    ```
    containing:
    * `tst_p2p_simple_ring_bsend_init`
    * `tst_p2p_simple_ring_bsend_run`
    * `tst_p2p_simple_ring_bsend_cleanup`
    where all names start with `tst_`,
    and the kind of communication calls tested (actually a `TST_CLASS`)
    and the communication pattern (here a simple ring using `MPI_Bsend`).
1. Add the new file to the list of sources for the `mpi_test_suite` target in `Makefile.am`
   and update the build system with
   ```
   $ autoreconf -vif
   ```
1. Add the new functions to `mpi_test_suite.h` in a sorted manner
1. Add the test-description to the `tst_tests-array` in `tst_test.c`:
    ```c
    struct tst_test {
        int class;
        char * description;
        int run_with_comm;
        tst_int64 run_with_type;
        int needs_sync;
        int (*tst_init_func) (const struct tst_env * env);
        int (*tst_run_func) (const struct tst_env * env);
        int (*tst_cleanup_func) (const struct tst_env * env);
    };
    ```
    describes with
    * class:            Which kind of test is being run
                        (one of TST_CLASS_ENV, TST_CLASS_P2P, TST_CLASS_COLL, TST_CLASS_THREADED),
    * description:      A short notion of what is being done,
    * run_with_comm:    An OR-ed list of which communicators may be used within the test
                        (several of TST_MPI_COMM_SELF, TST_MPI_COMM_NULL, TST_MPI_INTRA_COMM,
                        TST_MPI_INTER_COMM, TST_MPI_CART_COMM, TST_MPI_TOPO_COMM).
    * run_with_type:    Similar to the above, a OR-ed list of type being one/several of:
                        TST_MPI_CHAR, TST_MPI_UNSIGNED_CHAR, ..., TST_MPI_INT, ... or even
                        sets like TST_MPI_STANDARD_C_INT_TYPES or TST_MPI_STRUCT_FORTRAN_TYPES
    * needs_sync:       If non-zero a MPI_Barrier is done before and after the test, if the
                        test's communication may not intermingle with other (or own) test due
                        to MPI_ANY_SOURCE or MPI_ANY_TAG or alike.
    * tst_init_func:    The initialization function run once before the test to allocate arrays.
    * tst_run_func:     The test-function itself, run once or many times!
    * tst_cleanup_func: The clean-up function run once after the test to release arrays and buffers.

For example the Buffered-Send test using a Ring-topology of communication is declared like this:
```c
{TST_CLASS_P2P, "Ring Bsend",
 TST_MPI_INTRA_COMM,
 TST_MPI_ALL_C_TYPES,
 TST_NONE,
 &tst_p2p_simple_ring_bsend_init, &tst_p2p_simple_ring_bsend_run, &tst_p2p_simple_ring_bsend_cleanup},
```

These functions are passed a struct `tst_env`, which contains:
```c
struct tst_env {
    int comm;
    int type;
    int test;
    int values_num;
};
```

An identifier for the communicator, type, test and the number of values to be communicated.
The respective MPI-objects may be retrieved with the corresponding functions, e.g.:
```c
mpi_comm = tst_comm_getcomm (env->comm);
```

The tests must adhere to the following rules:

1. They must run with an arbitrary number of processes!
    Even if it means, that some processes are just hanging in the `MPI_Barrier` if it `needs_sync`!
1. They _MUST_ adhere to their specification in `tst_tests.c`!
1. They should run with as many communicators as possible (like declaring `run_with_comm`:
    `(TST_MPI_COMM_SELF | TST_MPI_COMM_NULL | TST_MPI_INTRA_COMM | TST_MPI_INTER_COMM)` or more :smile:.
1. They should run with as many types as possible (see above).
1. Every call to MPI-functions should be wrapped with the `MPI_CHECK` macro, which
    checks for the return value... This doesn't buy much, but anyway.
1. Every communicated data should be initialized with the `tst_type_setstandardarray`
    and checked with `tst_type_checkstandardarray`!
