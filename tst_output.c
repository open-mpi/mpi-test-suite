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



/* Function to replace substrings in a string
 *
 * Parameters:
 *      char * search   string to search for
 *      char * replace  replacement for searchstring
 *      char * string   pointer on string to search through
 *      char * result[] place to store pointer of the result string
 *
 * Results:
 *      Success:        number of replacements
 */
static int tst_output_str_replace(char *search, char *replace, char *string, char **result);


/* Function to mask special chars for latex in a string
 *
 * Parameters:
 *      char * string   string to escape
 *      char * result[] place to store pointer of the result string
 * Result:
 *      Success:        Number of replacements
 *      Fail:           -1
 */
static int tst_output_latex_special_chars(char *string, char **result);


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
extern int num_threads;
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

  /* Check if stream type uses a file and get the name of this file */
  if (type == TST_OUTPUT_TYPE_LOGFILE || type == TST_OUTPUT_TYPE_HTML ||
      type == TST_OUTPUT_TYPE_LATEX) {
    va_start(arglist, type);
    fname = va_arg (arglist, char *);
    sprintf (output->filename,"R%d_", tst_output_global_rank);
    freelen = 255 - strlen(output->filename);
    strncat (output->filename, fname, freelen);
    va_end (arglist);
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
      output->streamptr = fopen(output->filename, "w+");
      if (output->streamptr == NULL) {
        fprintf (stderr, "Error opening stream: Could not open output file.");
        return 0;
      }
      break;
    case TST_OUTPUT_TYPE_HTML:
      output->streamptr = fopen(output->filename, "w+");
      if (output->streamptr == NULL) {
        fprintf (stderr, "Error opening stream: Could not open output file.");
        return 0;
      }
      fprintf (output->streamptr, "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">\n"
          "<HTML>\n"
          "<TITLE>OpenMPI-Test-Suite - Logfile</TITLE>\n"
          "</HEAD>\n"
          "<BODY>\n");
      break;
    case TST_OUTPUT_TYPE_LATEX:
      output->streamptr = fopen(output->filename, "w+");
      if (output->streamptr == NULL) {
        fprintf (stderr, "Error opening stream: Could not open output file.");
        return 0;
      }
      fprintf (output->streamptr, "\\documentclass{article}\n"
          "\\title{OpenMPI-Test-Suite - Logfile}\n"
          "\\begin{document}\n");
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
      case TST_OUTPUT_TYPE_HTML:
        fprintf (output->streamptr, "</BODY>\n</HTML>\n");
        strcpy (output->filename,"");
        fclose (output->streamptr);
        break;
      case TST_OUTPUT_TYPE_LATEX:
        fprintf (output->streamptr, "\\end{document}\n");
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
    switch (output->type) {
      case TST_OUTPUT_TYPE_HTML: {
        char * msg;
        char * tmpstrptr;

        msg = malloc (sizeof(char) * 2048);
        count = vsnprintf (msg, 2047, format, arglist);
        /* Output was truncated */
        if (count >= 2047) {
          free(msg);
          return -1;
        }
        tst_output_str_replace ("rank", "<B>rank</B>$", msg, &tmpstrptr);
        free (msg);
        tst_output_str_replace ("Rank", "<B>Rank</B>", tmpstrptr, &msg);
        free (tmpstrptr);
        tst_output_str_replace ("ERROR", "<B style=\"color:red;\">ERROR</B>", msg, &tmpstrptr);
        free (msg);
        tst_output_str_replace ("Error", "<B style=\"color:red;\">Error</B>", tmpstrptr, &msg);
        free (tmpstrptr);
        count = fprintf (output->streamptr, "<P>\n%s</P>\n", msg);
        fflush (output->streamptr);
        break;
        }
      case TST_OUTPUT_TYPE_LATEX: {
        char * msg;
        char * tmpstrptr;

        msg = malloc (sizeof(char) * 2048);
        count = vsnprintf (msg, 2047, format, arglist);
        /* Output was truncated */
        if (count >= 2047) {
          free(msg);
          return -1;
        }
        tst_output_latex_special_chars(msg, &tmpstrptr);
        tst_output_str_replace ("<", "$<$", tmpstrptr, &msg);
        free (tmpstrptr);
        tst_output_str_replace (">", "$>$", msg, &tmpstrptr);
        free (msg);
        tst_output_str_replace ("Rank", "\\textbf{Rank}", tmpstrptr, &msg);
        free (tmpstrptr);
        tst_output_str_replace ("ERROR", "\\textbf{ERROR}", msg, &tmpstrptr);
        free (msg);
        tst_output_str_replace ("Error", "\\textbf{Error}", tmpstrptr, &msg);
        free (tmpstrptr);

        count = fprintf (output->streamptr, "%s", msg);
        fprintf (output->streamptr, "\n\n");
        fflush (output->streamptr);
        break;
        }
      default:
        count = vfprintf (output->streamptr, format, arglist);
        fflush (output->streamptr);
        break;
    }
    va_end(arglist);
  }
  else {
    count = 0;
  }

  return count;
}

/****************************************************************************/
/**                                                                        **/
/**                     LOCAL FUNCTIONS                                    **/
/**                                                                        **/
/****************************************************************************/

int tst_output_latex_special_chars(char *string, char **result) {
  int count = 0;
  int newlength = 0;
  char * strptr = NULL;
  char * resultptr;

  /* calculate number of special chars */
  strptr = string;
  while (*strptr != '\0'){
    switch (*strptr) {
      case '\\':
      case '&':
      case '$':
      case '%':
      case '#':
      case '_':
      case '{':
      case '}':
        count++;
      break;
    }
    strptr++;
  }
  /* allocate memory for the result */
  newlength = strlen (string) + count;
  resultptr = malloc(sizeof (char) * (newlength + 1));
  memset (resultptr, 0, newlength + 1);
  *result = resultptr;
  /* do special char masking */
  strptr = string;
  while (*strptr != '\0'){
    switch (*strptr) {
      case '\\':
      case '&':
      case '$':
      case '%':
      case '#':
      case '_':
      case '{':
      case '}':
        *resultptr = '\\';
        resultptr++;
        *resultptr = *strptr;
        resultptr++;
      break;
      default:
        *resultptr = *strptr;
        resultptr++;
      break;
    }
    strptr++;
  }
  *resultptr = '\0';

  return count;
}

int tst_output_str_replace(char *search, char *replace, char *string, char **result) {
  int count = 0, replace_count = 0;
  int oldlength = 0;
  int newlength = 0 ;
  int searchlength = 0;
  int n;
  char * strptr = NULL;
  char * oldstrptr = NULL;

  oldlength = strlen(string);
  searchlength = strlen (search);

  /* Calculate number of replacements to be preformed */
  for (strptr = strstr(string, search); strptr != NULL; strptr = strstr(++strptr, search)) {
    count++;
  }
  /* Calculate length of the resulting string and allocate the necessary memory */
  newlength = oldlength + count * (strlen(replace) - strlen(search));
  *result = (char *) malloc ( (newlength + 1) * sizeof (char) );
  memset (*result, 0, newlength + 1);

  /* Perform replacements and store result to new string */
  for (strptr = string; ; ) {
    oldstrptr = strptr;
    strptr = strstr (strptr, search);
    /* all replacements done? */
    if (strptr == NULL) {
      strcat (*result, oldstrptr);
      break;
    }
    /* Append all up to place of discovery */
    n = strptr - oldstrptr;
    strncat (*result, oldstrptr, n);
    /* Append replace string and move forward in the string */
    strcat (*result, replace);
    strptr += searchlength;
    replace_count++;
  }

  /* Safety check */
  if (count != replace_count) {
    fprintf (stderr, "Error: count (%d) != replace_count (%d)!\n", count, replace_count);
    exit (-1);
  }
  return count;
}
