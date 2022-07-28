#ifndef TST_OUTPUT_H_
#define TST_OUTPUT_H_


#include <stdio.h>
#include <stdarg.h>

#include "config.h"


#define DEBUG_REPORT_TYPE       TST_REPORT_MAX
#define DEBUG_LOG_TYPE          TST_OUTPUT_TYPE_LOGFILE
#define DEBUG_LOG_FILENAME      "tst.log"
#define DEBUG_LOG               (&tst_output)

#define TST_OUTPUT_RANK_MASTER  0
#define TST_OUTPUT_RANK_SELF    (tst_global_rank)


typedef enum {
  TST_OUTPUT_TYPE_NONE = 0,  /**< No Output opened */
  TST_OUTPUT_TYPE_STDERR,    /**< Output on stderr */
  TST_OUTPUT_TYPE_STDOUT,    /**< Output on stdout */
  TST_OUTPUT_TYPE_LOGFILE,   /**< Output into logfile */
} tst_output_types;


typedef enum {
  TST_REPORT_SUMMARY = 0, /**< No output, except for final summary */
  TST_REPORT_RUN,         /**< Output every test that runs, plus the previous */
  TST_REPORT_FULL,        /**< Full output, including hexdump of wrong memory */
  TST_REPORT_MAX          /**< Output everything */
} tst_report_types;


#define TST_OUTPUT_FILENAME_MAX 256
typedef struct {
  FILE * streamptr;  /**< Pointer on the stream */
  tst_output_types type;
  char filename[TST_OUTPUT_FILENAME_MAX];  /**< Filename if stream writes to file */
  tst_report_types level;
  int rank;    /**< MPI rank responsible for output */
  int isopen;  /**< open status: 1 if open */
} tst_output_stream;


#ifndef _TST_OUTPUT_C_SRC
extern tst_output_stream tst_output;
#endif


/** \brief Function to set the maximal level of information to be outputted.
 *
 * \param[in] level  Level describing the amount of information to be outputted
 *
 * \return  Success: Value unequal 0, Fail: 0
 */
int tst_output_set_level(tst_output_stream *output, tst_report_types level);


/** \brief Initialise the output dependent on the set parameters.
 *
 * \param[in,out] output  Pointer to output structure holding all output relevant information
 * \param[in] rank   rank of MPI process responsible for the output TST_OUTPUT_RANK_MASTER
 * \param[in] level  Initial output level up to which the output will be performed
 * \param[in] type   Output type of the stream
 * \param[in] ...    Filename if the output will be written to a file
 *
 * \return  Success: output type of the opened output, Fail: TST_OUTPUT_TYPE_NONE
 */
tst_output_types tst_output_init(tst_output_stream *output, int rank,
    tst_report_types level, tst_output_types type, ...);

/** \brief Closes an opened output.
 *
 * \param[in,out]: output  Pointer to the stream to be closed
 *
 * \return  Success: 1, Fail: 0
 */
int tst_output_close(tst_output_stream *output);

/** \brief Replacement of printf for output
 *
 * Prints a formatted output like printf if the error level is lower than the
 * maximal output level of the output stream.
 *
 * \param[in] output      Pointer to the output stream to be used
 * \param[in] error_level The error level of this output
 * \param[in] format      Format string (see printf)
 * \param[in] ...         Parameters for replace in format
 *
 * \return  Success: Number of written characters, Fail: 0
 */
int tst_output_printf(tst_output_stream *output,
    tst_report_types error_level, char *format, ...);

#endif  /* TST_OUTPUT_H_ */
