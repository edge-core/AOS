#ifndef _DRIVER_GROUP_OPERATION_H
#define _DRIVER_GROUP_OPERATION_H
/* INCLUDE FILE DECLARATIONS
 */

/*------------------------------------------------------------------------------
 * ROUTINE NAME : DRIVER_GROUP_SetTransitionMode
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will invoke all set transition mode function in CSCGroup1.
 *
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    None.
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
void DRIVER_GROUP_SetTransitionMode(void);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : DRIVER_GROUP_EnterTransitionMode
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will invoke all enter transition mode function in CSCGroup1.
 *
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    None.
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
void DRIVER_GROUP_EnterTransitionMode(void);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : DRIVER_GROUP_EnterMasterMode
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will invoke all set master mode function in CSCGroup1.
 *
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    None.
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
void DRIVER_GROUP_EnterMasterMode(void);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : DRIVER_GROUP_EnterSlaveMode
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will invoke all enter slave mode function in CSCGroup1.
 *
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    None.
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
void DRIVER_GROUP_EnterSlaveMode(void);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : DRIVER_GROUP_ProvisionComplete
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will invoke all set provision complete mode function in CSCGroup1.
 *
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    None.
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
void DRIVER_GROUP_ProvisionComplete(UI32_T unit);

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)

void DRIVER_GROUP_HandleHotInsertion(UI8_T unit_id, UI32_T starting_port_ifindex, UI32_T number_of_port);

void DRIVER_GROUP_HandleHotRemoval(UI8_T unit_id, UI32_T starting_port_ifindex, UI32_T number_of_port);

#endif

#endif
