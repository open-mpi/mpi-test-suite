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

#include "tst_output.h"
#include "mpi_test_suite.h"

/****************************************************************************/
/**                                                                        **/
/**                     PROTOTYPES OF LOCAL FUNCTIONS                      **/
/**                                                                        **/
/****************************************************************************/

/* Functions to parse output for LaTeX/HTML files 
 *
 * Parameters:
 *      char * src[]    source
 *      char * dest[]   destination
 * 
 * Results:
 *      Sucess:         Value unequal 0
 *      Fail:           0
 */
/*
int tst_output_parse_html (char * src[], char * dest[]);
int tst_output_parse_latex (char * src[], char * dest[]) ;
*/

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
int str_replace (char * search, char * replace, char * string, char * result[]);


/* Function to mask special chars for latex in a string
 *
 * Parameters:
 *      char * string   string to escape
 *      char * result[] place to store pointer of the result string
 * Result:
 *      Success:        Number of replacements
 *      Fail:           -1
 */
int latex_special_chars (char * string, char * result[]);


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
      if (tst_thread_get_num () != 0) {
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
    switch (output->type) {
      case TST_OUTPUT_TYPE_HTML: {
        char * msg;
        char * tmpstrptr;
        
        msg = malloc (sizeof(char) * 2048);
        count = vsnprintf (msg, 2047, format, arglist);
        /* Output was truncated */
        if (count >= 2047)
          return -1;
        str_replace ("rank", "<B>rank</B>$", msg, &tmpstrptr);
        free (msg);
        str_replace ("Rank", "<B>Rank</B>", tmpstrptr, &msg);
        free (tmpstrptr);
        str_replace ("ERROR", "<B style=\"color:red;\">ERROR</B>", msg, &tmpstrptr);
        free (msg);
        str_replace ("Error", "<B style=\"color:red;\">Error</B>", tmpstrptr, &msg);
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
        if (count >= 2047)
          return -1;
        latex_special_chars (msg, &tmpstrptr);
        str_replace ("<", "$<$", tmpstrptr, &msg);
        free (tmpstrptr);
        str_replace (">", "$>$", msg, &tmpstrptr);
        free (msg);
        str_replace ("Rank", "\\textbf{Rank}", tmpstrptr, &msg);
        free (tmpstrptr);
        str_replace ("ERROR", "\\textbf{ERROR}", msg, &tmpstrptr);
        free (msg);
        str_replace ("Error", "\\textbf{Error}", tmpstrptr, &msg);
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

int latex_special_chars (char * string, char * result[])
{
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

int str_replace (char * search, char * replace, char * string, char * result[])
{
  int count = 0, replace_count = 0;
  int oldlength = 0;
  int newlength = 0 ;
  int searchlength = 0;
  int n;
  char * strptr = NULL;
  char * oldstrptr = NULL;

  oldlength = strlen (string);
  searchlength = strlen (search);

  /* 
   * Calculate number of neccessary replacements 
   */
  for (strptr = string; strptr; strptr++)
  {
    strptr = strstr (strptr, search);
    if (strptr == NULL)
      break;
    count++; 
  }
  /* 
   * Calculate new neccessary stinglength and allocate the neccessary memory
   */
  newlength = oldlength + count * (strlen(replace) - strlen(search));
  *result = (char *) malloc ( (newlength + 1) * sizeof (char) );
  memset (*result, 0, newlength + 1);

  /*
   * Generate the new String
   */
  for (strptr = string; ; )
  {
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

  /*
   * Safty check 
   */
  if (count != replace_count) {
    fprintf (stderr, "Error: count (%d) != replace_count (%d)!\n", count, replace_count);
    exit (-1);
  }
  return count;
}

/****************************************************************************/
/**                                                                        **/
/**                     EOF                                                **/
/**                                                                        **/
/****************************************************************************/
