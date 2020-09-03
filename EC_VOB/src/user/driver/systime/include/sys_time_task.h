/* MODULE NAME:  sys_time_task.h
 * PURPOSE:
 * PURPOSE:
 *   Implementation of SYS_TIME task.
 *
 * NOTES:
 *   In order to keep the information of the accumulated system up time on a
 *   device and provides this information to ALU(Accton License) to verify that
 *   whether the license had been used for the time period more than the period
 *   offered by the license.
 *
 *   SYS_TIME task is responsible to count the system up time periodically and
 *   write the information of the accumulated system up time to a non-volatile
 *   storage device to ensure the information can be kept among every power
 *   cycle on the device. This vital information will be kept through the
 *   data storage service provided by FS.
 *
 *   Only needs to create SYS_TIME task when SYS_CPNT_SYS_TIME_ACCUMULATED_SYSTEM_UP_TIME
 *   is defined as TRUE.
 *
 * HISTORY
 *    9/8/2015 - Charlie Chen, Created
 *
 * Copyright(C)      Accton Corporation, 2015
 */
#ifndef SYS_TIME_TASK_H
#define SYS_TIME_TASK_H

/* INCLUDE FILE DECLARATIONS
 */

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME: SYS_TIME_TASK_CreateTask
 *-----------------------------------------------------------------------------
 * PURPOSE: Create task in SYS_TIME
 *-----------------------------------------------------------------------------
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 *-----------------------------------------------------------------------------
 * NOTES:
 */
void SYS_TIME_TASK_CreateTask(void);

#endif    /* End of #ifndef SYS_TIME_TASK_H */

