
#ifndef		_DHCPSNP_TASK_H
#define		_DHCPSNP_TASK_H

/* INCLUDE FILE DECLARATIONS
 */
#include "dhcpsnp_type.h"

/* NAME CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME : DHCPSNP_TASK_Init
 * PURPOSE:
 *        Init system resource required by DHCPSNP_TASK; including message queue, memory.
 *
 * INPUT:
 *        None.
 *
 * OUTPUT:
 *        None.
 *
 * RETURN:
 *        None.
 *
 * NOTES:
 *        None.
 */
void DHCPSNP_TASK_Init (void);

/* FUNCTION NAME : DHCPSNP_TASK_CreateTask
 * PURPOSE:
 *        Create DHCPSNP Task.
 *
 * INPUT:
 *        None.
 *
 * OUTPUT:
 *        None.
 *
 * RETURN:
 *        None.
 *
 * NOTES:
 *        None.
 */
void DHCPSNP_TASK_CreateTask (void);

/* FUNCTION NAME : DHCPSNP_TASK_SetTransitionMode
 * PURPOSE:
 *        Sending enter transition event to task calling by stkctrl.
 *
 * INPUT:
 *        None.
 *
 * OUTPUT:
 *        None.
 *
 * RETURN:
 *        None.
 *
 * NOTES:
 *        None.
 */
void DHCPSNP_TASK_SetTransitionMode(void);

/* FUNCTION NAME : DHCPSNP_TASK_EnterTransitionMode
 * PURPOSE:
 *        Sending enter transition event to task calling by stkctrl.
 *
 * INPUT:
 *        None.
 *
 * OUTPUT:
 *        None.
 *
 * RETURN:
 *        None.
 *
 * NOTES:
 *        None.
 */
void DHCPSNP_TASK_EnterTransitionMode(void);

/* FUNCTION NAME : DHCPSNP_TASK_EnterMasterMode
 * PURPOSE:
 *        Enter Master mode calling by stkctrl.
 *
 * INPUT:
 *        None.
 *
 * OUTPUT:
 *        None.
 *
 * RETURN:
 *        None.
 *
 * NOTES:
 *        None.
 */
void DHCPSNP_TASK_EnterMasterMode(void);

/* FUNCTION NAME : DHCPSNP_TASK_EnterSlaveMode
 * PURPOSE:
 *        Enter Slave mode calling by stkctrl.
 *
 * INPUT:
 *        None.
 *
 * OUTPUT:
 *        None.
 *
 * RETURN:
 *        None.
 *
 * NOTES:
 *        None.
 */
void DHCPSNP_TASK_EnterSlaveMode(void);

/* FUNCTION NAME : DHCPSNP_TASK_ProvisionComplete
 * PURPOSE:
 *        This function will call set DHCPSNP Task to provision complete
 *
 * INPUT:
 *        None.
 *
 * OUTPUT:
 *        None.
 *
 * RETURN:
 *        None.
 *
 * NOTES:
 *        None.
 */
void DHCPSNP_TASK_ProvisionComplete(void);
#endif

