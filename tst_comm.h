#ifndef TST_COMM_H_
#define TST_COMM_H_

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

#endif  /* TST_COMM_H_ */
