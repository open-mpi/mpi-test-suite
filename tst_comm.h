#ifndef TST_COMM_H_
#define TST_COMM_H_

#include <mpi.h>


/** \brief register communicators
 *
 * \return number of registered communicators
 */
int tst_comms_register() ;

/** \brief initialize communicators
 *
 * \return number of initialized communicators
 */
int tst_comms_init();

/** \brief return thread private MPI communicator for given communicator
 *
 * \param[in]  comm  id of the communicator
 * \return a thread private MPI communicator
 */
MPI_Comm tst_comm_getcomm (int commId);

/** \brief Get master thread's MPI communicator for given communicator
 *
 * \param[in]  comm  id of the communicator
 * \return master thread's MPI communicator
 */
MPI_Comm tst_comm_getmastercomm(int commId);


#endif  /* TST_COMM_H_ */
