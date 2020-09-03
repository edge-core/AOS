/*-----------------------------------------------------------------------------
 * FILE NAME: poe_task.h
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *    Definitions for the POE task.
 *
 * NOTES:
 *    None.
 *
 * History:                                                               
 *    12/14/2007 - Daniel Chen, Created
 *    12/03/2008 - Eugene Yu, porting POE to Linux platform
 *
 * Copyright(C)      Accton Corporation, 2008
 *-----------------------------------------------------------------------------
 */


#ifndef POE_TASK_H
#define POE_TASK_H

#include "sys_type.h"


#define POE_TASK_MAX_MSGQ_LEN        48
#define POE_TASK_MSG_TYPE_HIGHPOWER    1
#define POE_TASK_MSG_TYPE_RECEIVELLDP  2

/* msg size must be 16 byte */
typedef struct
{
    UI32_T lport;
    UI16_T status_powerValue;
    UI16_T state_control_portValue;
    UI16_T ttl;
    UI8_T status_powerType:2;
    UI8_T status_powerSource:2;
    UI8_T status_powerPriority:4;
    UI8_T state_control_powerType:2;
    UI8_T state_control_powerSource:2;
    UI8_T state_control_powerPriority:4;
    UI8_T state_control_acknowledge;
    UI8_T force_high_power_state;  /* 1: enable, 0: disable */
    UI8_T type;                      /* 1: high_power, 2:receive_lldp */
    UI8_T reserve;
}POE_TASK_Msg_T;


/* ------------------------------------------------------------------------
 * FUNCTION NAME - POE_TASK_CreateTasks
 * ------------------------------------------------------------------------
 * FUNCTION : Create POE main task.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void POE_TASK_CreateTasks(void) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_TASK_InitiateSystemResources
 *-------------------------------------------------------------------------
 * FUNCTION: Init the POE System_Resources
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * Note    : None
 *-------------------------------------------------------------------------
 */
void POE_TASK_InitiateSystemResources() ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_TASK_EnterMasterMode
 *-------------------------------------------------------------------------
 * PURPOSE  : Enter Master mode calling by stkctrl.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void POE_TASK_EnterMasterMode(void) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_TASK_EnterSlaveMode
 *-------------------------------------------------------------------------
 * PURPOSE  : Enter Slave mode calling by stkctrl.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void POE_TASK_EnterSlaveMode(void) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_TASK_SetTransitionMode
 *-------------------------------------------------------------------------
 * PURPOSE  : Sending enter transition event to task calling by stkctrl.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * Note     : None
 *-------------------------------------------------------------------------
 */
void POE_TASK_SetTransitionMode() ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_TASK_EnterTransitionMode
 *-------------------------------------------------------------------------
 * PURPOSE  : Sending enter transition event to task calling by stkctrl.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * Note     : None
 *-------------------------------------------------------------------------
 */
void POE_TASK_EnterTransitionMode() ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_TASK_ProvisionComplete
 *-------------------------------------------------------------------------
 * PURPOSE  : This function calling by stkctrl will tell POE that provision is completed.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * Note     : None
 *-------------------------------------------------------------------------
 */
void POE_TASK_ProvisionComplete() ;


#endif /* POE_TASK_H */


