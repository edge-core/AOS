/*-----------------------------------------------------------------------------
 * Module   : psec_pmgr.c
 *-----------------------------------------------------------------------------
 * PURPOSE  : Provide APIs to access port security control functions.
 *-----------------------------------------------------------------------------
 * NOTES    :
 *
 *-----------------------------------------------------------------------------
 * HISTORY  : 06/29/2007 - Wakka Tu, Create
 *
 *-----------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2007
 *-----------------------------------------------------------------------------
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <string.h>
#include "sys_type.h"
#include "sys_module.h"
#include "sysfun.h"
#include "psec_mgr.h"
#include "psec_pmgr.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void PSEC_PMGR_SendMsg(UI32_T cmd, SYSFUN_Msg_T *msg_p, UI32_T req_size, UI32_T res_size, UI32_T ret_val);

/* STATIC VARIABLE DECLARATIONS
 */
static SYSFUN_MsgQ_T ipcmsgq_handle;

/* MACRO FUNCTIONS DECLARACTION
 */

/* EXPORTED SUBPROGRAM BODIES
 */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - PSEC_PMGR_InitiateProcessResources
 *-------------------------------------------------------------------------
 * PURPOSE : Initiate resource used in the calling process.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE  - success
 *           FALSE - failure
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
BOOL_T PSEC_PMGR_InitiateProcessResources(void)
{
    if (SYSFUN_GetMsgQ(PSEC_MGR_IPCMSG_KEY, SYSFUN_MSGQ_BIDIRECTIONAL, &ipcmsgq_handle) != SYSFUN_OK)
    {
        printf("\r\n%s(): SYSFUN_GetMsgQ fail.", __FUNCTION__);
        return FALSE;
    }

    return TRUE;
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - PSEC_PMGR_GetPortSecurityStatus
 * ------------------------------------------------------------------------|
 * FUNCTION : The function will get the port security status
 * INPUT    : ifindex : the logical port
 * OUTPUT   : port_security_status
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 * ------------------------------------------------------------------------*/
BOOL_T PSEC_PMGR_GetPortSecurityStatus( UI32_T ifindex, UI32_T  *port_security_status)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(PSEC_MGR_GET_MSGBUFSIZE(PSEC_MGR_IPCMsg_GetStatus_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    PSEC_MGR_IPCMsg_GetStatus_T *data_p;

    if (port_security_status == NULL)
        return FALSE;

    data_p = PSEC_MGR_MSG_DATA(msg_p);
    data_p->ifindex = ifindex;

    PSEC_PMGR_SendMsg(PSEC_MGR_IPC_CMD_GET_PORT_SECURITY_STATUS,
                      msg_p,
                      PSEC_MGR_GET_MSGBUFSIZE(PSEC_MGR_IPCMsg_GetStatus_T),
                      PSEC_MGR_GET_MSGBUFSIZE(PSEC_MGR_IPCMsg_GetStatus_T),
                      (UI32_T)FALSE);

    *port_security_status = data_p->status;

    return (BOOL_T)PSEC_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - PSEC_PMGR_GetPortSecurityPortState
 * ------------------------------------------------------------------------|
 * FUNCTION : The function will get the port state
 * INPUT    : ifindex : the logical port
 * OUTPUT   : action_active
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 * ------------------------------------------------------------------------*/
BOOL_T PSEC_PMGR_GetPortSecurityActionActive( UI32_T ifindex, UI32_T  *action_active)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(PSEC_MGR_GET_MSGBUFSIZE(PSEC_MGR_IPCMsg_GetStatus_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    PSEC_MGR_IPCMsg_GetStatus_T *data_p;

    if (action_active== NULL)
        return FALSE;

    data_p = PSEC_MGR_MSG_DATA(msg_p);
    data_p->ifindex = ifindex;

    PSEC_PMGR_SendMsg(PSEC_MGR_IPC_CMD_GET_PORT_SECURITY_ACTION_ACTIVE,
                      msg_p,
                      PSEC_MGR_GET_MSGBUFSIZE(PSEC_MGR_IPCMsg_GetStatus_T),
                      PSEC_MGR_GET_MSGBUFSIZE(PSEC_MGR_IPCMsg_GetStatus_T),
                      (UI32_T)FALSE);

    *action_active = data_p->status;

    return (BOOL_T)PSEC_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - PSEC_PMGR_GetPortSecurityLastIntrusionMac
 * ------------------------------------------------------------------------|
 * FUNCTION : The function will get the last intrusion mac
 * INPUT    : ifindex : the logical port
 * OUTPUT   : mac addrress
 * RETURN   : TRUE/FALSE
 * ------------------------------------------------------------------------*/
BOOL_T PSEC_PMGR_GetPortSecurityLastIntrusionMac( UI32_T ifindex, UI8_T  *mac)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(PSEC_MGR_GET_MSGBUFSIZE(PSEC_MGR_IPCMsg_GetMac_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    PSEC_MGR_IPCMsg_GetMac_T *data_p;

    if (mac == NULL)
        return FALSE;

    data_p = PSEC_MGR_MSG_DATA(msg_p);
    data_p->ifindex = ifindex;

    PSEC_PMGR_SendMsg(PSEC_MGR_IPC_CMD_GET_PORT_SECURITY_LAST_INTRUSION_MAC,
                      msg_p,
                      PSEC_MGR_GET_MSGBUFSIZE(PSEC_MGR_IPCMsg_GetMac_T),
                      PSEC_MGR_GET_MSGBUFSIZE(PSEC_MGR_IPCMsg_GetMac_T),
                      (UI32_T)FALSE);

    memcpy(mac, data_p->mac, SYS_ADPT_MAC_ADDR_LEN);

    return (BOOL_T)PSEC_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - PSEC_PMGR_GetPortSecurityLastIntrusionTime
 * ------------------------------------------------------------------------|
 * FUNCTION : The function will get the last intrusion time
 * INPUT    : ifindex : the logical port
 * OUTPUT   : last intrusion time
 * RETURN   : TRUE/FALSE
 * ------------------------------------------------------------------------*/
BOOL_T PSEC_PMGR_GetPortSecurityLastIntrusionTime( UI32_T ifindex, UI32_T  *seconds)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(PSEC_MGR_GET_MSGBUFSIZE(PSEC_MGR_IPCMsg_GetStatus_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    PSEC_MGR_IPCMsg_GetStatus_T *data_p;

    if (seconds == NULL)
        return FALSE;

    data_p = PSEC_MGR_MSG_DATA(msg_p);
    data_p->ifindex = ifindex;

    PSEC_PMGR_SendMsg(PSEC_MGR_IPC_CMD_GET_PORT_SECURITY_LAST_INTRUSION_TIME,
                      msg_p,
                      PSEC_MGR_GET_MSGBUFSIZE(PSEC_MGR_IPCMsg_GetStatus_T),
                      PSEC_MGR_GET_MSGBUFSIZE(PSEC_MGR_IPCMsg_GetStatus_T),
                      (UI32_T)FALSE);

    *seconds = data_p->status;

    return (BOOL_T)PSEC_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - PSEC_PMGR_GetPortSecurityActionStatus
 * ------------------------------------------------------------------------|
 * FUNCTION : The function will get the action status of port security
 * INPUT    : ifindex : the logical port
 * OUTPUT   : action_status
 * RETURN   : TRUE/FALSE
 * NOTE     : none(1)/trap(2)/shutdown(3)/trapAndShutdown(4)
 * ------------------------------------------------------------------------*/
BOOL_T PSEC_PMGR_GetPortSecurityActionStatus( UI32_T ifindex, UI32_T  *action_status)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(PSEC_MGR_GET_MSGBUFSIZE(PSEC_MGR_IPCMsg_GetStatus_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    PSEC_MGR_IPCMsg_GetStatus_T *data_p;

    if (action_status == NULL)
        return FALSE;

    data_p = PSEC_MGR_MSG_DATA(msg_p);
    data_p->ifindex = ifindex;

    PSEC_PMGR_SendMsg(PSEC_MGR_IPC_CMD_GET_PORT_SECURITY_ACTION_STATUS,
                      msg_p,
                      PSEC_MGR_GET_MSGBUFSIZE(PSEC_MGR_IPCMsg_GetStatus_T),
                      PSEC_MGR_GET_MSGBUFSIZE(PSEC_MGR_IPCMsg_GetStatus_T),
                      (UI32_T)FALSE);

    *action_status = data_p->status;

    return (BOOL_T)PSEC_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - PSEC_PMGR_GetRunningPortSecurityStatus
 * ------------------------------------------------------------------------|
 * FUNCTION : The function will get the port security status for running config
 * INPUT    : ifindex : the logical port
 * OUTPUT   : port_security_status
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 * ------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T PSEC_PMGR_GetRunningPortSecurityStatus( UI32_T ifindex,
                                                                  UI32_T *port_security_status)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(PSEC_MGR_GET_MSGBUFSIZE(PSEC_MGR_IPCMsg_GetStatus_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    PSEC_MGR_IPCMsg_GetStatus_T *data_p;

    if (port_security_status == NULL)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    data_p = PSEC_MGR_MSG_DATA(msg_p);
    data_p->ifindex = ifindex;

    PSEC_PMGR_SendMsg(PSEC_MGR_IPC_CMD_GET_RUNNING_PORT_SECURITY_STATUS,
                      msg_p,
                      PSEC_MGR_GET_MSGBUFSIZE(PSEC_MGR_IPCMsg_GetStatus_T),
                      PSEC_MGR_GET_MSGBUFSIZE(PSEC_MGR_IPCMsg_GetStatus_T),
                      SYS_TYPE_GET_RUNNING_CFG_FAIL);

    *port_security_status = data_p->status;

    return PSEC_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - PSEC_PMGR_GetRunningPortSecurityActionStatus
 * ------------------------------------------------------------------------|
 * FUNCTION : The function will get the port security action status
 *            for running config
 * INPUT    : ifindex : the logical port
 * OUTPUT   : action_status
 * RETURN   : TRUE/FALSE
 * NOTE     : none(1)/trap(2)/shutdown(3)/trapAndShutdown(4)
 * ------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T PSEC_PMGR_GetRunningPortSecurityActionStatus( UI32_T ifindex,
                                                                        UI32_T *action_status)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(PSEC_MGR_GET_MSGBUFSIZE(PSEC_MGR_IPCMsg_GetStatus_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    PSEC_MGR_IPCMsg_GetStatus_T *data_p;

    if (action_status == NULL)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    data_p = PSEC_MGR_MSG_DATA(msg_p);
    data_p->ifindex = ifindex;

    PSEC_PMGR_SendMsg(PSEC_MGR_IPC_CMD_GET_RUNNING_PORT_SECURITY_ACTION_STATUS,
                      msg_p,
                      PSEC_MGR_GET_MSGBUFSIZE(PSEC_MGR_IPCMsg_GetStatus_T),
                      PSEC_MGR_GET_MSGBUFSIZE(PSEC_MGR_IPCMsg_GetStatus_T),
                      SYS_TYPE_GET_RUNNING_CFG_FAIL);

    *action_status = data_p->status;

    return PSEC_MGR_MSG_RETVAL(msg_p);
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - PSEC_PMGR_SetPortSecurityMacCount
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set port auto learning mac counts
 * INPUT   : ifindex        -- which port to
 *           mac_count      -- mac learning count
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T PSEC_PMGR_SetPortSecurityMacCount(UI32_T ifindex, UI32_T mac_count)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(PSEC_MGR_GET_MSGBUFSIZE(PSEC_MGR_IPCMsg_SetPortSecurityMacCount_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    PSEC_MGR_IPCMsg_SetPortSecurityMacCount_T *data_p;

    data_p = PSEC_MGR_MSG_DATA(msg_p);
    data_p->ifindex = ifindex;
    data_p->mac_count = mac_count;

    PSEC_PMGR_SendMsg(PSEC_MGR_IPC_CMD_SET_PORT_SECURITY_MAC_COUNT,
                      msg_p,
                      PSEC_MGR_GET_MSGBUFSIZE(PSEC_MGR_IPCMsg_SetPortSecurityMacCount_T),
                      PSEC_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      (UI32_T)FALSE);

    return (BOOL_T)PSEC_MGR_MSG_RETVAL(msg_p);
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - PSEC_PMGR_SetPortSecurityMacCountOperation
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set port auto learning mac counts
 * INPUT   : ifindex        -- which port to
 *           mac_count      -- mac learning count
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T PSEC_PMGR_SetPortSecurityMacCountOperation(UI32_T ifindex, UI32_T mac_count)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(PSEC_MGR_GET_MSGBUFSIZE(PSEC_MGR_IPCMsg_SetPortSecurityMacCount_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    PSEC_MGR_IPCMsg_SetPortSecurityMacCount_T *data_p;

    data_p = PSEC_MGR_MSG_DATA(msg_p);
    data_p->ifindex = ifindex;
    data_p->mac_count = mac_count;

    PSEC_PMGR_SendMsg(PSEC_MGR_IPC_CMD_SET_PORT_SECURITY_MAC_COUNT_OPERATION,
                      msg_p,
                      PSEC_MGR_GET_MSGBUFSIZE(PSEC_MGR_IPCMsg_SetPortSecurityMacCount_T),
                      PSEC_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      (UI32_T)FALSE);

    return (BOOL_T)PSEC_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - PSEC_PMGR_GetRunningPortSecurityMacCount
 * ------------------------------------------------------------------------|
 * FUNCTION : The function will get the port security max mac count
 *            for running config
 * INPUT    : ifindex : the logical port
 * OUTPUT   : mac_count
 * RETURN   : SYS_TYPE_Get_Running_Cfg_T
 * NOTE     : none
 * ------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T PSEC_PMGR_GetRunningPortSecurityMacCount( UI32_T ifindex,
                                                                    UI32_T *mac_count)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(PSEC_MGR_GET_MSGBUFSIZE(PSEC_MGR_IPCMsg_GetPortSecurityMacCount_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    PSEC_MGR_IPCMsg_GetPortSecurityMacCount_T *data_p;

    if (mac_count == NULL)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    data_p = PSEC_MGR_MSG_DATA(msg_p);
    data_p->ifindex = ifindex;

    PSEC_PMGR_SendMsg(PSEC_MGR_IPC_CMD_GET_RUNNING_PORT_SECURITY_MAC_COUNT,
                      msg_p,
                      PSEC_MGR_GET_MSGBUFSIZE(PSEC_MGR_IPCMsg_GetPortSecurityMacCount_T),
                      PSEC_MGR_GET_MSGBUFSIZE(PSEC_MGR_IPCMsg_GetPortSecurityMacCount_T),
                      SYS_TYPE_GET_RUNNING_CFG_FAIL);

    *mac_count = data_p->mac_count;

    return PSEC_MGR_MSG_RETVAL(msg_p);
}

/*---------------------------------------------------------------------- */
/* ( PortSecurityMgt 1 )--VS2524 */
/*
 *      INDEX       { portSecPortIndex }
 *      portSecPortEntry ::= SEQUENCE
 *      {
 *          portSecPortIndex      INTEGER,
            portSecPortStatus     INTEGER
 *      }
 */
 /*------------------------------------------------------------------------
 * ROUTINE NAME - PSEC_PMGR_GetPortSecurityEntry
 *------------------------------------------------------------------------
 * FUNCTION: This function will get the port security management entry
 * INPUT   : portsec_entry->portsec_port_index - interface index
 * OUTPUT  : portsec_entry                     - port security entry
 * RETURN  : TRUE/FALSE
 * NOTE    : VS2524 MIB/PortSecurityMgt 1
 *------------------------------------------------------------------------*/
BOOL_T PSEC_PMGR_GetPortSecurityEntry (PSEC_MGR_PortSecurityEntry_T *portsec_entry)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(PSEC_MGR_GET_MSGBUFSIZE(PSEC_MGR_IPCMsg_PortSecurityEntry_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    PSEC_MGR_IPCMsg_PortSecurityEntry_T *data_p;

    if (portsec_entry == NULL)
        return FALSE;

    data_p = PSEC_MGR_MSG_DATA(msg_p);
    memcpy(&data_p->portsec_entry, portsec_entry, sizeof(data_p->portsec_entry));

    PSEC_PMGR_SendMsg(PSEC_MGR_IPC_CMD_GET_PORT_SECURITY_ENTRY,
                      msg_p,
                      PSEC_MGR_GET_MSGBUFSIZE(PSEC_MGR_IPCMsg_PortSecurityEntry_T),
                      PSEC_MGR_GET_MSGBUFSIZE(PSEC_MGR_IPCMsg_PortSecurityEntry_T),
                      (UI32_T)FALSE);

    memcpy(portsec_entry, &data_p->portsec_entry, sizeof(data_p->portsec_entry));

    return (BOOL_T)PSEC_MGR_MSG_RETVAL(msg_p);
}


/*------------------------------------------------------------------------
 * ROUTINE NAME - PSEC_PMGR_GetNextPortSecurityEntry
 *------------------------------------------------------------------------
 * FUNCTION: This function will get the next port security management entry
 * INPUT   : portsec_entry->portsec_port_index - interface index
 * OUTPUT  : portsec_entry                     - port security entry
 * RETURN  : TRUE/FALSE
 * NOTE    : VS2524 MIB/PortSecurityMgt 1
 *------------------------------------------------------------------------*/
BOOL_T PSEC_PMGR_GetNextPortSecurityEntry (PSEC_MGR_PortSecurityEntry_T *portsec_entry)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(PSEC_MGR_GET_MSGBUFSIZE(PSEC_MGR_IPCMsg_PortSecurityEntry_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    PSEC_MGR_IPCMsg_PortSecurityEntry_T *data_p;

    if (portsec_entry == NULL)
        return FALSE;

    data_p = PSEC_MGR_MSG_DATA(msg_p);
    memcpy(&data_p->portsec_entry, portsec_entry, sizeof(data_p->portsec_entry));

    PSEC_PMGR_SendMsg(PSEC_MGR_IPC_CMD_GET_NEXT_PORT_SECURITY_ENTRY,
                      msg_p,
                      PSEC_MGR_GET_MSGBUFSIZE(PSEC_MGR_IPCMsg_PortSecurityEntry_T),
                      PSEC_MGR_GET_MSGBUFSIZE(PSEC_MGR_IPCMsg_PortSecurityEntry_T),
                      (UI32_T)FALSE);

    memcpy(portsec_entry, &data_p->portsec_entry, sizeof(data_p->portsec_entry));

    return (BOOL_T)PSEC_MGR_MSG_RETVAL(msg_p);
}


/*------------------------------------------------------------------------
 * ROUTINE NAME - PSEC_PMGR_SetPortSecurityStatus
 *------------------------------------------------------------------------
 * FUNCTION: This function will Set the port security status
 * INPUT   : ifindex                - interface index
 *           portsec_status         - VAL_portSecPortStatus_enabled
 *                                    VAL_portSecPortStatus_disabled
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    :
 *------------------------------------------------------------------------*/
BOOL_T PSEC_PMGR_SetPortSecurityStatus (UI32_T ifindex, UI32_T portsec_status)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(PSEC_MGR_GET_MSGBUFSIZE(PSEC_MGR_IPCMsg_SetStatus_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    PSEC_MGR_IPCMsg_SetStatus_T *data_p;

    data_p = PSEC_MGR_MSG_DATA(msg_p);
    data_p->ifindex = ifindex;
    data_p->status = portsec_status;

    PSEC_PMGR_SendMsg(PSEC_MGR_IPC_CMD_SET_PORT_SECURITY_STATUS,
                      msg_p,
                      PSEC_MGR_GET_MSGBUFSIZE(PSEC_MGR_IPCMsg_SetStatus_T),
                      PSEC_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      (UI32_T)FALSE);

    return (BOOL_T)PSEC_MGR_MSG_RETVAL(msg_p);
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - PSEC_PMGR_SetPortSecurityPortState
 *------------------------------------------------------------------------
 * FUNCTION: This function will Set the port security port state
 * INPUT   : ifindex                - interface index
 *           action_active         - TRUE/FALSE
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    :
 *------------------------------------------------------------------------*/
BOOL_T PSEC_PMGR_SetPortSecurityActionActive (UI32_T ifindex, UI32_T action_active)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(PSEC_MGR_GET_MSGBUFSIZE(PSEC_MGR_IPCMsg_SetStatus_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    PSEC_MGR_IPCMsg_SetStatus_T *data_p;

    data_p = PSEC_MGR_MSG_DATA(msg_p);
    data_p->ifindex = ifindex;
    data_p->status = action_active;

    PSEC_PMGR_SendMsg(PSEC_MGR_IPC_CMD_SET_PORT_SECURITY_ACTION_ACTIVE,
                      msg_p,
                      PSEC_MGR_GET_MSGBUFSIZE(PSEC_MGR_IPCMsg_SetStatus_T),
                      PSEC_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      (UI32_T)FALSE);

    return (BOOL_T)PSEC_MGR_MSG_RETVAL(msg_p);
}


/*------------------------------------------------------------------------
 * ROUTINE NAME - PSEC_PMGR_SetPortSecurityStatusOperation
 *------------------------------------------------------------------------
 * FUNCTION: This function will Set the port security status
 * INPUT   : ifindex                - interface index
 *           portsec_status         - VAL_portSecPortStatus_enabled
 *                                    VAL_portSecPortStatus_disabled
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    :
 *------------------------------------------------------------------------*/
BOOL_T PSEC_PMGR_SetPortSecurityStatusOperation(UI32_T ifindex, UI32_T portsec_status)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(PSEC_MGR_GET_MSGBUFSIZE(PSEC_MGR_IPCMsg_SetStatus_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    PSEC_MGR_IPCMsg_SetStatus_T *data_p;

    data_p = PSEC_MGR_MSG_DATA(msg_p);
    data_p->ifindex = ifindex;
    data_p->status = portsec_status;

    PSEC_PMGR_SendMsg(PSEC_MGR_IPC_CMD_SET_PORT_SECURITY_STATUS_OPERATION,
                      msg_p,
                      PSEC_MGR_GET_MSGBUFSIZE(PSEC_MGR_IPCMsg_SetStatus_T),
                      PSEC_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      (UI32_T)FALSE);

    return (BOOL_T)PSEC_MGR_MSG_RETVAL(msg_p);
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - PSEC_PMGR_SetPortSecurityActionStatus
 *------------------------------------------------------------------------
 * FUNCTION: This function will Set the port security action status
 * INPUT   : ifindex                - interface index
 *           action_status - VAL_portSecAction_none(1)
 *                           VAL_portSecAction_trap(2)
 *                           VAL_portSecAction_shutdown(3)
 *                           VAL_portSecAction_trapAndShutdown(4)
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    :
 *------------------------------------------------------------------------*/
BOOL_T PSEC_PMGR_SetPortSecurityActionStatus (UI32_T ifindex, UI32_T action_status)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(PSEC_MGR_GET_MSGBUFSIZE(PSEC_MGR_IPCMsg_SetStatus_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    PSEC_MGR_IPCMsg_SetStatus_T *data_p;

    data_p = PSEC_MGR_MSG_DATA(msg_p);
    data_p->ifindex = ifindex;
    data_p->status = action_status;

    PSEC_PMGR_SendMsg(PSEC_MGR_IPC_CMD_SET_PORT_SECURITY_ACTION_STATUS,
                      msg_p,
                      PSEC_MGR_GET_MSGBUFSIZE(PSEC_MGR_IPCMsg_SetStatus_T),
                      PSEC_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      (UI32_T)FALSE);

    return (BOOL_T)PSEC_MGR_MSG_RETVAL(msg_p);
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - PSEC_PMGR_SetPortSecurityLearningStatus
 *------------------------------------------------------------------------
 * FUNCTION: This function will Set the port security learning status
 * INPUT   : ifindex                - interface index
 *           portsec_status         - VAL_portSecLearningStatus_enabled
 *                                    VAL_portSecLearningStatus_disabled
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    :
 *------------------------------------------------------------------------*/
BOOL_T PSEC_PMGR_SetPortSecurityLearningStatus (UI32_T ifindex, UI32_T learning_status)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(PSEC_MGR_GET_MSGBUFSIZE(PSEC_MGR_IPCMsg_SetStatus_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    PSEC_MGR_IPCMsg_SetStatus_T *data_p;

    data_p = PSEC_MGR_MSG_DATA(msg_p);
    data_p->ifindex = ifindex;
    data_p->status = learning_status;

    PSEC_PMGR_SendMsg(PSEC_MGR_IPC_CMD_SET_PORT_SECURITY_LEARNING_STATUS,
                      msg_p,
                      PSEC_MGR_GET_MSGBUFSIZE(PSEC_MGR_IPCMsg_SetStatus_T),
                      PSEC_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      (UI32_T)FALSE);

    return (BOOL_T)PSEC_MGR_MSG_RETVAL(msg_p);
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - PSEC_PMGR_GetPortSecurityLearningStatus
 *------------------------------------------------------------------------
 * FUNCTION: This function will Get the port security learning status
 * INPUT   : ifindex                - interface index
 * OUTPUT  : learning_status        - VAL_portSecLearningStatus_enabled
 *                                    VAL_portSecLearningStatus_disabled
 * RETURN  : TRUE/FALSE
 * NOTE    :
 *------------------------------------------------------------------------*/
BOOL_T PSEC_PMGR_GetPortSecurityLearningStatus (UI32_T ifindex, UI32_T *learning_status)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(PSEC_MGR_GET_MSGBUFSIZE(PSEC_MGR_IPCMsg_GetStatus_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    PSEC_MGR_IPCMsg_GetStatus_T *data_p;

    if (learning_status == NULL)
        return FALSE;

    data_p = PSEC_MGR_MSG_DATA(msg_p);
    data_p->ifindex = ifindex;

    PSEC_PMGR_SendMsg(PSEC_MGR_IPC_CMD_GET_PORT_SECURITY_LEARNING_STATUS,
                      msg_p,
                      PSEC_MGR_GET_MSGBUFSIZE(PSEC_MGR_IPCMsg_GetStatus_T),
                      PSEC_MGR_GET_MSGBUFSIZE(PSEC_MGR_IPCMsg_GetStatus_T),
                      (UI32_T)FALSE);

    *learning_status = data_p->status;

    return (BOOL_T)PSEC_MGR_MSG_RETVAL(msg_p);
}

/* Function - PSEC_PMGR_GetPortSecurityMacCount
 * Purpose  - This function will get port auto learning mac counts
 * Input    - ifindex        -- which port to
 * Output   - mac_count      -- mac learning count
 * Return  : TRUE: Successfully, FALSE: If not available
 * Note     -
 */
BOOL_T PSEC_PMGR_GetPortSecurityMacCount( UI32_T ifindex, UI32_T * mac_count )
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(PSEC_MGR_GET_MSGBUFSIZE(PSEC_MGR_IPCMsg_GetPortSecurityMacCount_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    PSEC_MGR_IPCMsg_GetPortSecurityMacCount_T *data_p;

    if (mac_count == NULL)
        return FALSE;

    data_p = PSEC_MGR_MSG_DATA(msg_p);
    data_p->ifindex = ifindex;

    PSEC_PMGR_SendMsg(PSEC_MGR_IPC_CMD_GET_PORT_SECURITY_MAC_COUNT,
                      msg_p,
                      PSEC_MGR_GET_MSGBUFSIZE(PSEC_MGR_IPCMsg_GetPortSecurityMacCount_T),
                      PSEC_MGR_GET_MSGBUFSIZE(PSEC_MGR_IPCMsg_GetPortSecurityMacCount_T),
                      (UI32_T)FALSE);

    *mac_count = data_p->mac_count;

    return (BOOL_T)PSEC_MGR_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - PSEC_PMGR_GetMinNbrOfMaxMacCount
 * ---------------------------------------------------------------------
 * PURPOSE  : Get the minimum value that max-mac-count can be set to.
 * INPUT    : none
 * OUTPUT   : min_number
 * RETURN   : none
 * NOTES    : for 3com CLI & WEB
 * ---------------------------------------------------------------------
 */
void PSEC_PMGR_GetMinNbrOfMaxMacCount(UI32_T *min_number)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(PSEC_MGR_GET_MSGBUFSIZE(PSEC_MGR_IPCMsg_GetMinNbrOfMaxMacCount_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    PSEC_MGR_IPCMsg_GetMinNbrOfMaxMacCount_T *data_p;

    if (min_number == NULL)
        return;

    data_p = PSEC_MGR_MSG_DATA(msg_p);

    PSEC_PMGR_SendMsg(PSEC_MGR_IPC_CMD_GET_MIN_NBR_OF_MAX_MAC_COUNT,
                      msg_p,
                      PSEC_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      PSEC_MGR_GET_MSGBUFSIZE(PSEC_MGR_IPCMsg_GetMinNbrOfMaxMacCount_T),
                      (UI32_T)FALSE);

    *min_number = data_p->min_number;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - PSEC_PMGR_ConvertSecuredAddressIntoManual
 *------------------------------------------------------------------------
 * FUNCTION: To convert port security learnt secured MAC address into manual
 *           configured on the specified interface. If interface is
 *           PSEC_MGR_INTERFACE_INDEX_FOR_ALL, it will apply to all interface.
 * INPUT   : ifindex    -- ifindex
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 *------------------------------------------------------------------------
 */
BOOL_T
PSEC_PMGR_ConvertSecuredAddressIntoManual(
    UI32_T ifindex)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(PSEC_MGR_GET_MSGBUFSIZE(PSEC_MGR_IPCMsg_Interface_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    PSEC_MGR_IPCMsg_Interface_T *data_p;

    data_p = PSEC_MGR_MSG_DATA(msg_p);
    data_p->ifindex = ifindex;

    PSEC_PMGR_SendMsg(PSEC_MGR_IPC_CMD_CONVERT_SECURED_ADDRESS_INTO_MANUAL,
                      msg_p,
                      PSEC_MGR_GET_MSGBUFSIZE(PSEC_MGR_IPCMsg_Interface_T),
                      PSEC_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      (UI32_T)FALSE);

    return (BOOL_T)PSEC_MGR_MSG_RETVAL(msg_p);
}

/* LOCAL SUBPROGRAM BODIES
 */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - PSEC_PMGR_SendMsg
 *-------------------------------------------------------------------------
 * PURPOSE : To send an IPC message.
 * INPUT   : cmd       - the PSEC message command.
 *           msg_p     - the buffer of the IPC message.
 *           req_size  - the size of PSEC request message.
 *           res_size  - the size of PSEC response message.
 *           ret_val   - the return value to set when IPC is failed.
 * OUTPUT  : msg_p     - the response message.
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
static void PSEC_PMGR_SendMsg(UI32_T cmd, SYSFUN_Msg_T *msg_p, UI32_T req_size, UI32_T res_size, UI32_T ret_val)
{
    UI32_T ret;

    msg_p->cmd = SYS_MODULE_PSEC;
    msg_p->msg_size = req_size;

    PSEC_MGR_MSG_CMD(msg_p) = cmd;

    ret = SYSFUN_SendRequestMsg(ipcmsgq_handle,
                                msg_p,
                                SYSFUN_TIMEOUT_WAIT_FOREVER,
                                SYSFUN_SYSTEM_EVENT_IPCMSG,
                                res_size,
                                msg_p);

    /* If IPC is failed, set return value as ret_val.
     */
    if ((ret != SYSFUN_OK) || (PSEC_MGR_MSG_RETVAL(msg_p) == FALSE))
        PSEC_MGR_MSG_RETVAL(msg_p) = ret_val;
}

/* End of this file */

