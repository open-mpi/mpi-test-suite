/**
 * \todo Multiple outputs at the same time
 * \todo Rank specific output
 * \todo Macros for different output types
 */

#define _TST_OUTPUT_C_SRC
#include "tst_output.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <mpi.h>

#include "mpi_test_suite.h"


/****************************************************************************/
/**                                                                        **/
/**                     GLOBAL VARIABLES                                   **/
/**                                                                        **/
/****************************************************************************/

static int tst_output_global_rank;

/* Corresponding strings to values in enum tst_report_types. */
const char * tst_reports[] = {
  "Summary",
  "Run",
  "Full",
  "Max"
};

#ifdef HAVE_MPI2_THREADS
extern int tst_thread_running (void);
#endif

/****************************************************************************/
/**                                                                        **/
/**                     EXPORTED FUNCTIONS                                 **/
/**                                                                        **/
/****************************************************************************/

int tst_output_set_level(tst_output_stream * output, tst_report_types level) {
#ifdef HAVE_MPI2_THREADS
  if (tst_thread_running() && tst_thread_get_num() != 0) {
    return 0;
  }
#endif
  output->level = level;
  return level;
}


tst_output_types tst_output_init(tst_output_stream * output, int rank,
    tst_report_types level, tst_output_types type, ...) {

  va_list arglist;
  char * fname;
  int freelen;

  MPI_Comm_rank (MPI_COMM_WORLD, &tst_output_global_rank);

#ifdef HAVE_MPI2_THREADS
  {
    if (tst_thread_running()) {
      if (tst_thread_get_num() != 0) {
        return output->type;
      }
    }
  }
#endif

  /* Now do the initialisation for the different stream types */
  switch (type) {
    case TST_OUTPUT_TYPE_STDERR:
      output->streamptr = stderr;
      break;
    case TST_OUTPUT_TYPE_STDOUT:
      output->streamptr = stdout;
      break;
    case TST_OUTPUT_TYPE_LOGFILE:
      va_start(arglist, type);
      fname = va_arg (arglist, char *);
      snprintf(output->filename, TST_OUTPUT_FILENAME_MAX, "R%d_%s",
              tst_output_global_rank, fname);
      va_end (arglist);
      output->streamptr = fopen(output->filename, "w+");
      if (output->streamptr == NULL) {
        fprintf (stderr, "Error opening stream: Could not open output file.");
        return 0;
      }
      break;
    default:
      fprintf (stderr, "Error opening stream: Unknown stream type (%i).", type);
      return TST_OUTPUT_TYPE_NONE;
      break;
  }

  /* set the rest of the info of the stream */
  output->type = type;
  output->level = level;
  output->rank = rank;
  output->isopen = 1;

  return output->type;
}

int tst_output_close(tst_output_stream * output) {
#ifdef HAVE_MPI2_THREADS
  {
    if (tst_thread_running()) {
      if (tst_thread_get_num() != 0) {
        return 0;
      }
    }
  }
#endif
  if (output->isopen) {
    switch (output->type) {
      case TST_OUTPUT_TYPE_LOGFILE:
        strcpy (output->filename,"");
        fclose (output->streamptr);
        break;
      default:
        break;
    }
    output->streamptr = NULL;
    output->type = TST_OUTPUT_TYPE_NONE;
    output->level = 0;
    output->isopen = 0;
  }
  else {
    return 0;
  }

  return 1;
}

int tst_output_printf(tst_output_stream * output,
    tst_report_types error_level, char * format, ...) {
  int count;
  va_list arglist;

#ifdef HAVE_MPI2_THREADS
  {
    if (tst_thread_running()) {
      if (tst_thread_get_num() != 0) {
        return 0;
      }
    }
  }
#endif

  if ((output->isopen == 1) && (output->rank == tst_output_global_rank) && (error_level <= output->level)) {
    va_start(arglist, format);
    count = vfprintf (output->streamptr, format, arglist);
    fflush (output->streamptr);
    va_end(arglist);
  }
  else {
    count = 0;
  }

  return count;
}

