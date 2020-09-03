/*-----------------------------------------------------------------------------
 * Module Name: lacp_om.h
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *-----------------------------------------------------------------------------
 * NOTES:
 *
 *
 *-----------------------------------------------------------------------------
 * HISTORY:
 *    12/05/2001 - Lewis Kang, Created
 *
 *-----------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2001
 *-----------------------------------------------------------------------------
 */

#ifndef _LACP_OM_H
#define _LACP_OM_H


/* INCLUDE FILE DECLARATIONS
 */

#include "sys_type.h"
#include "sysfun.h"


/* NAMING CONSTANT DECLARATIONS
 */

#define LACP_OM_IPCMSG_TYPE_SIZE sizeof(union LACP_OM_IpcMsg_Type_U)

/* command used in IPC message
 */
enum
{
	LACP_OM_IPC_GETRUNNINGDOT3ADLACPPORTENABLED,
	LACP_OM_IPC_GETRUNNINGDOT3ADAGGPORTPARTNERADMINSYSTEMID
};


/* MACRO FUNCTION DECLARATIONS
 */

/* Macro function for computation of IPC msg_buf size based on field name
 * used in LACP_OM_IpcMsg_T.data
 */
#define LACP_OM_GET_MSG_SIZE(field_name)                        \
            (LACP_OM_IPCMSG_TYPE_SIZE +                         \
            sizeof(((LACP_OM_IpcMsg_T*)0)->data.field_name))


/* DATA TYPE DECLARATIONS
 */

/* IPC message structure
 */
typedef struct
{
	union LACP_OM_IpcMsg_Type_U
	{
		UI32_T cmd;
		UI32_T ret_ui32;
	} type;

	union
	{
        struct
        {
            UI32_T arg1;
	        UI32_T arg2;
        } arg_grp1;

		struct
		{
			UI16_T arg1;
			UI8_T  arg2;
		} arg_grp2;
	} data;
} LACP_OM_IpcMsg_T;


/*-----------------------------------------------------------------------------
 * FUNCTION NAME - LACP_OM_HandleIPCReqMsg
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the ipc request message for XSTP OM.
 *
 * INPUT   : msgbuf_p - input request ipc message buffer
 *
 * OUTPUT  : msgbuf_p - output response ipc message buffer
 *
 * RETURN  : TRUE  - there is a response required to be sent
 *           FALSE - there is no response required to be sent
 *
 * NOTES   : 1. The size of msgbuf_p->msg_buf must be large enough to carry
 *              any response messages.
 *-----------------------------------------------------------------------------
 */
BOOL_T LACP_OM_HandleIPCReqMsg(SYSFUN_Msg_T* msgbuf_p);

/*=============================================================================
 * Moved from lacp_mgr.h
 *=============================================================================
 */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LACP_OM_GetRunningDot3adLacpPortEnabled
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the LACP enabled value of a port.
 * INPUT    :   UI32_T lport            -- lport number
 * OUTPUT   :   UI32_T *lacp_state        -- pointer of the enable value
 *                                         VAL_LacpStuts_enable or VAL_LacpStuts_disable (defined in swctrl.h)
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T LACP_OM_GetRunningDot3adLacpPortEnabled(UI32_T ifindex, UI32_T *lacp_state);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_OM_GetRunningDot3adAggActorSystemPriority
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the dot3ad_agg_actor_system_priority information.
 * INPUT    :   UI16_T  agg_index   -- the dot3ad_agg_index
 * OUTPUT   :   UI16_T  *priority   -- the dot3ad_agg_actor_system_priority value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_OM_GetRunningDot3adAggActorSystemPriority(UI16_T agg_index, UI16_T *priority);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_OM_GetRunningDot3adAggCollectorMaxDelay
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the dot3ad_agg_collector_max_delay information.
 * INPUT    :   UI16_T  agg_index   -- the dot3ad_agg_index
 * OUTPUT   :   UI32_T  *max_delay  -- the dot3ad_agg_collector_max_delay value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_OM_GetRunningDot3adAggCollectorMaxDelay(UI16_T agg_index, UI32_T *max_delay);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_OM_GetRunningDot3adAggPortPartnerAdminSystemId
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the dot3ad_agg_port_partner_admin_system_id information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 * OUTPUT   :   UI8_T   *system_id  -- the dot3ad_agg_port_partner_admin_system_id value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_OM_GetRunningDot3adAggPortPartnerAdminSystemId(UI16_T port_index, UI8_T *system_id);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_OM_GetRunningDot3adAggPortPartnerAdminPort
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the dot3ad_agg_port_partner_admin_port information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 * OUTPUT   :   UI16_T  *admin_port -- the dot3ad_agg_port_partner_admin_port value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_OM_GetRunningDot3adAggPortPartnerAdminPort(UI16_T port_index, UI16_T *admin_port);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_OM_GetRunningDot3adAggPortActorAdminState
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the dot3ad_agg_port_actor_admin_state information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 * OUTPUT   :   UI8_T   *admin_state-- the dot3ad_agg_port_actor_admin_state value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_OM_GetRunningDot3adAggPortActorAdminState(UI16_T port_index, UI8_T *admin_state);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_OM_GetRunningDot3adAggPortPartnerAdminState
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the dot3ad_agg_port_partner_admin_state information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 * OUTPUT   :   UI8_T   *admin_state-- the dot3ad_agg_port_partner_admin_state value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_OM_GetRunningDot3adAggPortPartnerAdminState(UI16_T port_index, UI8_T *admin_state);


#endif //_LACP_OM_H
