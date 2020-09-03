/*-----------------------------------------------------------------------------
 * FILE NAME: poe_engine.h
 *-----------------------------------------------------------------------------
 * PURPOSE: 
 *    Definitions for the POE engine (dot3at state machine)
 * 
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    12/14/2007 - Daniel Chen, Created
 *    12/03/2008 - Eugene Yu, porting POE to Linux platform
 *
 * Copyright(C)      Accton Corporation, 2008
 *-----------------------------------------------------------------------------
 */
 

#ifndef POE_ENGINE_H
#define POE_ENGINE_H

#include "sys_type.h"

#define POE_ENGINE_TLV_POWER_PRIORITY_UNKNOW     0x00
#define POE_ENGINE_TLV_POWER_PRIORITY_CRITICAL   0x01
#define POE_ENGINE_TLV_POWER_PRIORITY_HIGH       0x02
#define POE_ENGINE_TLV_POWER_PRIORITY_LOW        0x03

#define POE_ENGINE_TLV_POWER_SOURCE_PSE_UNKNOW   0x00
#define POE_ENGINE_TLV_POWER_SOURCE_PSE_PRIMARY  0x10
#define POE_ENGINE_TLV_POWER_SOURCE_PSE_BACKUP   0x20
#define POE_ENGINE_TLV_POWER_SOURCE_PSE_RESERVED 0x30

#define POE_ENGINE_TLV_POWER_SOURCE_PD_UNKNOW    0x00
#define POE_ENGINE_TLV_POWER_SOURCE_PD_PSE       0x10
#define POE_ENGINE_TLV_POWER_SOURCE_PD_LOCAL     0x20
#define POE_ENGINE_TLV_POWER_SOURCE_PD_PSE_LOCAL 0x30

#define POE_ENGINE_TLV_POWER_TYPE_TYPE2_PSE      0x00
#define POE_ENGINE_TLV_POWER_TYPE_TYPE2_PD       0x40
#define POE_ENGINE_TLV_POWER_TYPE_TYPE1_PSE      0x80
#define POE_ENGINE_TLV_POWER_TYPE_TYPE1_PD       0xc0

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_ENGINE_InitialSystemResource
 *-------------------------------------------------------------------------
 * FUNCTION: Init the POE State Machine System_Resources
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * Note    : None
 *-------------------------------------------------------------------------
 */
void POE_ENGINE_InitialSystemResource() ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_ENGINE_LocalSystemChange
 *-------------------------------------------------------------------------
 * PURPOSE  : This function is called when the local system change.
 * INPUT    : unit - unit number
 *            port - port number
 * OUTPUT   : None
 * RETURN   : None
 * Note     : None
 *-------------------------------------------------------------------------
 */
BOOL_T POE_ENGINE_LocalSystemChange(UI32_T unit, UI32_T port);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_ENGINE_ProcessTimerEvent
 *-------------------------------------------------------------------------
 * PURPOSE  : This function is called when the period timer event occur. (1 seconed)
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * Note     : None
 *-------------------------------------------------------------------------
 */
BOOL_T POE_ENGINE_ProcessTimerEvent(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_ENGINE_RefreshTimer
 *-------------------------------------------------------------------------
 * PURPOSE  : This function is used to refresh timer
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * Note     : This function should be called when receive the power control frame
 *-------------------------------------------------------------------------
 */
BOOL_T POE_ENGINE_RefreshTimer(UI32_T unit, UI32_T port);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_ENGINE_ProcessLLDPInfo
 *-------------------------------------------------------------------------
 * PURPOSE  : This function handle the state machine while receiving
 *            LLDP frame from Powerd Device
 * INPUT    : lport - port number
 *            info - the information passed by LLDP
 * OUTPUT   : None
 * RETURN   : None
 * Note     : None
 *-------------------------------------------------------------------------
 */
BOOL_T POE_ENGINE_ProcessLLDPInfo(UI32_T unit, UI32_T port, POE_TYPE_Dot3atPowerInfo_T *info);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_ENGINE_ProcessPortStateChange
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will reset the state machine when port state
 *            changed
 * OUTPUT   : None
 * RETURN   : None
 * Note     : None
 *-------------------------------------------------------------------------
 */
BOOL_T POE_ENGINE_ProcessPortStateChange(UI32_T unit, UI32_T port);


/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_ENGINE_GetPortDot3atPowerInfo
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get all infomation for LLDP tlvs of PoE, For LLDPDU tx
 * INPUT    : group_index, port_index
 * OUTPUT   : None
 * RETURN   : None
 * Note     :
 *-------------------------------------------------------------------------
 */
BOOL_T POE_ENGINE_GetPortDot3atPowerInfo(UI32_T unit, UI32_T port, POE_TYPE_Dot3atPowerInfo_T *info);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_ENGINE_DumpDot3atInfo
 *-------------------------------------------------------------------------
 * PURPOSE  : Dump all state variable
 * INPUT    : port: 0 - all port, else logical port
 * OUTPUT   : None
 * RETURN   : None
 * Note     :
 *-------------------------------------------------------------------------
 */
BOOL_T POE_ENGINE_DumpDot3atInfo(UI32_T unit, UI32_T port);


/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_ENGINE_ENABLE_STATE_MECHINE
 *-------------------------------------------------------------------------
 * PURPOSE  : This function is used to enable state machine
 * INPUT    : lport
 * OUTPUT   : None
 * RETURN   : None
 * Note     : only called by poe_task.c
 *-------------------------------------------------------------------------
 */
BOOL_T POE_ENGINE_ENABLE_STATE_MECHINE(UI32_T unit, UI32_T port);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_ENGINE_DISABLE_STATE_MECHINE
 *-------------------------------------------------------------------------
 * PURPOSE  : This function is used to disable state machine
 * INPUT    : lport
 * OUTPUT   : None
 * RETURN   : None
 * Note     : only called by poe_task.c
 *-------------------------------------------------------------------------
 */
BOOL_T POE_ENGINE_DISABLE_STATE_MECHINE(UI32_T unit, UI32_T port);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_ENGINE_DISABLE_PORT_DOT3AT
 *-------------------------------------------------------------------------
 * PURPOSE  : This function is used to disable state machine
 * INPUT    : lport
 * OUTPUT   : None
 * RETURN   : None
 * Note     : only called by poe_task.c
 *-------------------------------------------------------------------------
 */
void POE_ENGINE_DISABLE_PORT_DOT3AT(UI32_T unit, UI32_T port);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_ENGINE_ENABLE_PORT_DOT3AT
 *-------------------------------------------------------------------------
 * PURPOSE  : This function is used to enable state machine
 * INPUT    : lport
 * OUTPUT   : None
 * RETURN   : None
 * Note     : only called by poe_task.c
 *-------------------------------------------------------------------------
 */
void POE_ENGINE_ENABLE_PORT_DOT3AT(UI32_T unit, UI32_T port);


#endif /* POE_ENGINE_H */


