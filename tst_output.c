/*
******************************* C SOURCE FILE *******************************
**                                                                         **
** project   : MPI Testsuite                                               **
** filename  : TST_OUTPUT.C                                                **
** version   : 1                                                           **
** date      : June 11, 2006                                               **
** Revised by: Christoph Niethammer                                        **
**                                                                         **
*****************************************************************************
Todo:
- Multiple outputs at the same time
- Rankspecific output
- Macros for different output types
*/

#define _TST_OUTPUT_C_SRC

/****************************************************************************/
/**                                                                        **/
/**                     MODULES USED                                       **/
/**                                                                        **/
/****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <mpi.h>

/* #include "tst_output.h" */
#include "tst_output.h"
#include "mpi_test_suite.h"


/****************************************************************************/
/**                                                                        **/
/**                     GLOBAL VARIABLES                                   **/
/**                                                                        **/
/****************************************************************************/

static int tst_output_global_rank;

#ifdef HAVE_MPI2_THREADS
extern int num_threads;
extern int tst_thread_running (void);
#endif

/****************************************************************************/
/**                                                                        **/
/**                     EXPORTED FUNCTIONS                                 **/
/**                                                                        **/
/****************************************************************************/

int tst_set_output_level (tst_output_stream * output, tst_report_types level)
{
#ifdef HAVE_MPI2_THREADS
    {
        if (tst_thread_running ()) {
            if (tst_thread_get_num() != 0) {
                return 0;
            }
        }
    }
#endif
    output->level = level;
    return (output->level == level);
}


tst_output_types tst_output_init (tst_output_stream * output, int rank,
                                  tst_report_types level, tst_output_types type, ...)
{

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

    /* Checking if stream is already opened */
#if 0
    if (output->isopen) {
        fprintf (stderr, "Error opening stream: Stream allready opened.");
        return output->type;
    }
#endif
    /* Check if stream type uses a file and get the name of this file */
    if (type == TST_OUTPUT_TYPE_LOGFILE || type == TST_OUTPUT_TYPE_HTML ||
            type == TST_OUTPUT_TYPE_LATEX) {
        va_start(arglist, type);
        fname = va_arg (arglist, char *);
        sprintf (output->filename,"R%d_", tst_output_global_rank);
        freelen = 255 - strlen(output->filename);
        strncat (output->filename, fname, freelen);
    }

    /* Now do the initialisation for the different stream types */
    switch (type) {
        case TST_OUTPUT_TYPE_STDERR:
            output->streamptr = stderr;
            break;
        case TST_OUTPUT_TYPE_STDOUT:
            output->streamptr = stdout;
            break;
        case TST_OUTPUT_TYPE_LOGFILE:
        case TST_OUTPUT_TYPE_HTML:
        case TST_OUTPUT_TYPE_LATEX:
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

int tst_output_close (tst_output_stream * output)
{
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
            case TST_OUTPUT_TYPE_HTML:
            case TST_OUTPUT_TYPE_LATEX:
                strcpy(output->filename,"");
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

int tst_output_printf (tst_output_stream * output,
                       tst_report_types error_level, char * format, ...)
{
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

