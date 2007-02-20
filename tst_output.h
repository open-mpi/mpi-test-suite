/*
******************************* C HEADER FILE *******************************
**                                                                         **
** project   : MPI Testsuite                                               **
** filename  : TPL.H                                                       **
** date      : June 11, 2006                                               **
** Revised by  : Christoph Niethammer                                      **
**                                                                         **
*****************************************************************************
*/

#ifndef _TST_OUTPUT_INCLUDED
#define _TST_OUTPUT_INCLUDED

/****************************************************************************/
/**                                                                        **/
/**                     MODULES USED                                       **/
/**                                                                        **/
/****************************************************************************/

#include "config.h"

#ifdef HAVE_STDIO_H
#  include <stdio.h>
#endif
#ifdef HAVE_STDARG_H
#  include <stdarg.h>
#endif



/****************************************************************************/
/**                                                                        **/
/**                     DEFINITIONS AND MACROS                             **/
/**                                                                        **/
/****************************************************************************/

#define DEBUG_REPORT_TYPE     TST_REPORT_MAX
#define DEBUG_LOG_TYPE        TST_OUTPUT_TYPE_LOGFILE
#define DEBUG_LOG_FILENAME    "tst.log"
#define DEBUG_LOG             (&tst_output)

#define TST_OUTPUT_RANK_MASTER	(-1)
#define TST_OUTPUT_RANK_SELF	(tst_global_rank)

/****************************************************************************/
/**                                                                        **/
/**                     TYPEDEFS AND STRUCTURES                            **/
/**                                                                        **/
/****************************************************************************/

typedef enum {
  TST_OUTPUT_TYPE_NONE = 0,			/* No Output opened */
  TST_OUTPUT_TYPE_STDERR,			/* Output on stderr */
  TST_OUTPUT_TYPE_STDOUT,			/* Output on stdout */
  TST_OUTPUT_TYPE_LOGFILE,			/* Output into logfile */
  TST_OUTPUT_TYPE_LATEX,			/* Output into latex file */
  TST_OUTPUT_TYPE_HTML			/* Output into html file */
} tst_output_types;

typedef enum {
  TST_REPORT_SUMMARY=0,     /* No output, except for failed tests at the end of the run */
  TST_REPORT_RUN,           /* Output every test that runs, plus the previous */
  TST_REPORT_FULL,          /* Full output, including the hexdump of wrong memory */
  TST_REPORT_MAX
} tst_report_types;



typedef struct {
  FILE * streamptr;		/* Pointer on the stream */
  tst_output_types type;		
  char filename[256];		/* Filename if stream writes to file */
  tst_report_types level;		
  int rank;			/* Thread responsible for output */
  int isopen;			/* 1 if open */
} tst_output_stream;

/****************************************************************************/
/**                                                                        **/
/**                     EXPORTED VARIABLES                                 **/
/**                                                                        **/
/****************************************************************************/

#ifndef _TST_OUTPUT_C_SRC
extern tst_output_stream tst_output;
#endif

/****************************************************************************/
/**                                                                        **/
/**                     EXPORTED FUNCTIONS                                 **/
/**                                                                        **/
/****************************************************************************/


/* Functin to set the amount of information to be written into the output.
 *
 * Parameters:
 * 	tst_report_types level	Level describing the amount of information
 * 				Can be one of the following
 * 				
 * 	  TST_REPORT_SUMMARY, 	No output, except for failed tests at the end 
 * 				of the run 
 * 	  TST_REPORT_RUN,	Output every test that runs, plus the previous 
 * 	  TST_REPORT_FULL,	Full output, including the hexdump of wrong 
 * 				memory 
 * 	  TST_REPORT_MAX
 * 
 * Results:
 * 	Success:		Vaule unequal 0
 *	Fail:			0
 */
int tst_set_output_level (tst_output_stream * output, tst_report_types level);


/*
 * Initialise the output dependent on the set parameters
 *
 * Parameters:
 * 	tst_output_stream * output	Pointer on the ouput structure holding
 * 					the information
 * 	int rank		Rank of thread responsible for the 
 * 				output. TST_OUTPUT_RANK_MASTER
 * 	tst_report_types level	Output level up to which the output will be
 * 				performed
 * 	tst_output_types type	Output type of the stream
 * 	 ...			Filename if the output will be written to a 
 * 	 			file 
 * 					
 * Success:		output type of the opened output
 * Fail:		TST_OUTPUT_TYPE_NONE
 */
tst_output_types tst_output_init (tst_output_stream * output, int rank,
    tst_report_types level, tst_output_types type, ...);

/* Closes an opened output.
 *
 * Parameters:
 * 	tst_output_stream * output	Pointer onto the stream to be closed
 *
 * Success:		1
 * Fail:		0
 */
int tst_output_close (tst_output_stream * output);

/*
 * Replacement for printf / fprintf. Prints the string format with the given
 * replacements ... (like printf) if the error level is included in the 
 * output level of the output_stream.
 *
 * Parameters:
 * 	tst_output_stream * output	Pointer onto the stream to write on
 * 	tst_report_types error_level	The error level of this output
 * 	char * format			Format string (see printf)
 * 	...				Parameters for replace in format
 *
 * Results:
 * 
 * Success:	Number of written characters.
 * Fail:	0
 */
int tst_output_printf (tst_output_stream * output, 
    tst_report_types error_level, char * format, ...);

#endif

