/*-----------------------------------------------------------------------------
 * FILE NAME: LACP_PMGR.C
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *    This file implements the APIs for LACP MGR IPC.
 *
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    2007/06/21     --- Timon, Create
 *
 * Copyright(C)      Accton Corporation, 2007
 *-----------------------------------------------------------------------------
 */


/* INCLUDE FILE DECLARATIONS
 */

#include <stdio.h>
#include <string.h>
#include "sys_bld.h"
#include "sys_module.h"
#include "sys_type.h"
#include "sysfun.h"
#include "lacp_mgr.h"
#include "lacp_pmgr.h"


/* NAMING CONSTANT DECLARATIONS
 */


/* MACRO FUNCTION DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */


/* LOCAL SUBPROGRAM DECLARATIONS
 */


/* STATIC VARIABLE DEFINITIONS
 */

static SYSFUN_MsgQ_T ipcmsgq_handle;


/* EXPORTED SUBPROGRAM BODIES
 */

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : LACP_PMGR_InitiateProcessResource
 *-----------------------------------------------------------------------------
 * PURPOSE : Initiate resource for LACP_PMGR in the calling process.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  --  Sucess
 *           FALSE --  Error
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T LACP_PMGR_InitiateProcessResource(void)
{
    /* get the ipc message queues for LACP MGR
     */
    if (SYSFUN_GetMsgQ(SYS_BLD_LACP_GROUP_IPCMSGQ_KEY,
            SYSFUN_MSGQ_BIDIRECTIONAL, &ipcmsgq_handle) != SYSFUN_OK)
    {
        printf("\r\n%s(): SYSFUN_GetMsgQ failed.\r\n", __FUNCTION__);
        return FALSE;
    }

    return TRUE;
} /* End of LACP_PMGR_InitiateProcessResource */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LACP_PMGR_GetDot3adAggEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified Agg entry
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   agg_entry->dot3ad_agg_index
 * OUTPUT   :   agg_entry                  -- aggregator entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
BOOL_T LACP_PMGR_GetDot3adAggEntry(LACP_MGR_Dot3adAggEntry_T *agg_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LACP_MGR_GET_MSG_SIZE(arg_agg_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LACP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LACP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LACP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LACP_MGR_IPC_GETDOT3ADAGGENTRY;
    msg_p->data.arg_agg_entry = *agg_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *agg_entry = msg_p->data.arg_agg_entry;

    return msg_p->type.ret_bool;
} /* End of LACP_PMGR_GetDot3adAggEntry */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LACP_PMGR_GetNextDot3adAggEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the next aggregator index of
 *              the specified Agg entry info can be successfully retrieved.
 *              Otherwise, false is returned.
 * INPUT    :   agg_entry->dot3ad_agg_index
 * OUTPUT   :   agg_entry                  -- aggregator entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
BOOL_T LACP_PMGR_GetNextDot3adAggEntry(LACP_MGR_Dot3adAggEntry_T *agg_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LACP_MGR_GET_MSG_SIZE(arg_agg_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LACP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LACP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LACP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LACP_MGR_IPC_GETNEXTDOT3ADAGGENTRY;
    msg_p->data.arg_agg_entry = *agg_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *agg_entry = msg_p->data.arg_agg_entry;

    return msg_p->type.ret_bool;
} /* End of LACP_PMGR_GetNextDot3adAggEntry */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LACP_PMGR_GetDot3adAggPortListEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified port list entry
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   None
 * OUTPUT   :   base_entry                  -- base entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
BOOL_T LACP_PMGR_GetDot3adAggPortListEntry(LACP_MGR_Dot3adAggPortListEntry_T *agg_port_list_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LACP_MGR_GET_MSG_SIZE(arg_agg_port_list_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LACP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LACP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LACP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LACP_MGR_IPC_GETDOT3ADAGGPORTLISTENTRY;
    msg_p->data.arg_agg_port_list_entry = *agg_port_list_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *agg_port_list_entry = msg_p->data.arg_agg_port_list_entry;

    return msg_p->type.ret_bool;
} /* End of LACP_PMGR_GetDot3adAggPortListEntry */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LACP_PMGR_GetNextDot3adAggPortListEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the next aggregator index of
 *              the specified Agg entry info can be successfully retrieved.
 *              Otherwise, false is returned.
 * INPUT    :   None
 * OUTPUT   :   base_entry                  -- base entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
BOOL_T LACP_PMGR_GetNextDot3adAggPortListEntry(LACP_MGR_Dot3adAggPortListEntry_T *agg_port_list_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LACP_MGR_GET_MSG_SIZE(arg_agg_port_list_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LACP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LACP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LACP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LACP_MGR_IPC_GETNEXTDOT3ADAGGPORTLISTENTRY;
    msg_p->data.arg_agg_port_list_entry = *agg_port_list_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *agg_port_list_entry = msg_p->data.arg_agg_port_list_entry;

    return msg_p->type.ret_bool;
} /* End of LACP_PMGR_GetNextDot3adAggPortListEntry */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LACP_PMGR_GetDot3adAggPortEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified port entry
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   None
 * OUTPUT   :   base_entry                  -- base entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
BOOL_T LACP_PMGR_GetDot3adAggPortEntry(LACP_MGR_Dot3adAggPortEntry_T *agg_port_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LACP_MGR_GET_MSG_SIZE(arg_agg_port_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LACP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LACP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LACP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LACP_MGR_IPC_GETDOT3ADAGGPORTENTRY;
    msg_p->data.arg_agg_port_entry = *agg_port_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *agg_port_entry = msg_p->data.arg_agg_port_entry;

    return msg_p->type.ret_bool;
} /* End of LACP_PMGR_GetDot3adAggPortEntry */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LACP_PMGR_GetNextDot3adAggPortEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the next port entry
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   None
 * OUTPUT   :   agg_port_entry
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
BOOL_T LACP_PMGR_GetNextDot3adAggPortEntry(LACP_MGR_Dot3adAggPortEntry_T *agg_port_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LACP_MGR_GET_MSG_SIZE(arg_agg_port_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LACP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LACP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LACP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LACP_MGR_IPC_GETNEXTDOT3ADAGGPORTENTRY;
    msg_p->data.arg_agg_port_entry = *agg_port_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *agg_port_entry = msg_p->data.arg_agg_port_entry;

    return msg_p->type.ret_bool;
} /* End of LACP_PMGR_GetNextDot3adAggPortEntry */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LACP_PMGR_GetDot3adAggPortStatsEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified port stats entry
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   None
 * OUTPUT   :   base_entry                  -- base entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
BOOL_T LACP_PMGR_GetDot3adAggPortStatsEntry(LACP_MGR_Dot3adAggPortStatsEntry_T *agg_port_stats_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LACP_MGR_GET_MSG_SIZE(arg_agg_port_stats_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LACP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LACP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LACP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LACP_MGR_IPC_GETDOT3ADAGGPORTSTATSENTRY;
    msg_p->data.arg_agg_port_stats_entry = *agg_port_stats_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *agg_port_stats_entry = msg_p->data.arg_agg_port_stats_entry;

    return msg_p->type.ret_bool;
} /* End of LACP_PMGR_GetDot3adAggPortStatsEntry */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LACP_PMGR_GetNextDot3adAggPortStatsEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified port stats entry
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   None
 * OUTPUT   :   base_entry                  -- base entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
BOOL_T LACP_PMGR_GetNextDot3adAggPortStatsEntry(LACP_MGR_Dot3adAggPortStatsEntry_T *agg_port_stats_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LACP_MGR_GET_MSG_SIZE(arg_agg_port_stats_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LACP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LACP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LACP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LACP_MGR_IPC_GETNEXTDOT3ADAGGPORTSTATSENTRY;
    msg_p->data.arg_agg_port_stats_entry = *agg_port_stats_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *agg_port_stats_entry = msg_p->data.arg_agg_port_stats_entry;

    return msg_p->type.ret_bool;
} /* End of LACP_PMGR_GetNextDot3adAggPortStatsEntry */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LACP_PMGR_GetDot3adAggPortDebugEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified port debug entry
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   None
 * OUTPUT   :   base_entry                  -- base entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
BOOL_T LACP_PMGR_GetDot3adAggPortDebugEntry(LACP_MGR_Dot3adAggPortDebugEntry_T *agg_port_debug_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LACP_MGR_GET_MSG_SIZE(arg_agg_port_debug_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LACP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LACP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LACP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LACP_MGR_IPC_GETDOT3ADAGGPORTDEBUGENTRY;
    msg_p->data.arg_agg_port_debug_entry = *agg_port_debug_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *agg_port_debug_entry = msg_p->data.arg_agg_port_debug_entry;

    return msg_p->type.ret_bool;
} /* End of LACP_PMGR_GetDot3adAggPortDebugEntry */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LACP_PMGR_GetNextDot3adAggPortDebugEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified port debug entry
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   None
 * OUTPUT   :   base_entry                  -- base entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
BOOL_T LACP_PMGR_GetNextDot3adAggPortDebugEntry(LACP_MGR_Dot3adAggPortDebugEntry_T *agg_port_debug_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LACP_MGR_GET_MSG_SIZE(arg_agg_port_debug_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LACP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LACP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LACP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LACP_MGR_IPC_GETNEXTDOT3ADAGGPORTDEBUGENTRY;
    msg_p->data.arg_agg_port_debug_entry = *agg_port_debug_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *agg_port_debug_entry = msg_p->data.arg_agg_port_debug_entry;

    return msg_p->type.ret_bool;
} /* End of LACP_PMGR_GetNextDot3adAggPortDebugEntry */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LACP_MGR_GetDot3adLagMibObjects
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified lag mib objects
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   None
 * OUTPUT   :   base_entry                  -- base entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
BOOL_T LACP_PMGR_GetDot3adLagMibObjects(LACP_MGR_LagMibObjects_T *lag_mib_objects)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LACP_MGR_GET_MSG_SIZE(arg_lag_mib_objects);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LACP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LACP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LACP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LACP_MGR_IPC_GETDOT3ADLAGMIBOBJECTS;
    msg_p->data.arg_lag_mib_objects = *lag_mib_objects;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *lag_mib_objects = msg_p->data.arg_lag_mib_objects;

    return msg_p->type.ret_bool;
} /* End of LACP_PMGR_GetDot3adLagMibObjects */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LACP_PMGR_SetDot3adLacpPortEnabled
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set LACP on port to be enable or disable.
 * INPUT    :   UI32_T lacp_state         -- VAL_lacpPortStatus_enabled or VAL_lacpPortStatus_disabled (defined in Leaf_es3626a.h)
 * OUTPUT   :   None
 * RETURN   :   LACP_RETURN_OK           -- set successfully
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_PMGR_SetDot3adLacpPortEnabled(UI32_T ifindex, UI32_T lacp_state)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LACP_MGR_GET_MSG_SIZE(arg_grp1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LACP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LACP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LACP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LACP_MGR_IPC_SETDOT3ADLACPPORTENABLED;
    msg_p->data.arg_grp1.arg1 = ifindex;
    msg_p->data.arg_grp1.arg2 = lacp_state;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            LACP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return LACP_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
} /* End of LACP_PMGR_SetDot3adLacpPortEnabled */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LACP_PMGR_GetDot3adLacpPortEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified port entry
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   None
 * OUTPUT   :   lacp_port_entry -- port entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
BOOL_T LACP_PMGR_GetDot3adLacpPortEntry(LACP_MGR_Dot3adLacpPortEntry_T *lacp_port_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LACP_MGR_GET_MSG_SIZE(arg_lacp_port_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LACP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LACP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LACP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LACP_MGR_IPC_GETDOT3ADLACPPORTENTRY;
    msg_p->data.arg_lacp_port_entry = *lacp_port_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *lacp_port_entry = msg_p->data.arg_lacp_port_entry;

    return msg_p->type.ret_bool;
} /* End of LACP_PMGR_GetDot3adLacpPortEntry */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LACP_PMGR_GetNextDot3adLacpPortEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the next lacp port entry
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   None
 * OUTPUT   :   lacp_port_entry -- port entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
BOOL_T LACP_PMGR_GetNextDot3adLacpPortEntry(LACP_MGR_Dot3adLacpPortEntry_T *lacp_port_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LACP_MGR_GET_MSG_SIZE(arg_lacp_port_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LACP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LACP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LACP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LACP_MGR_IPC_GETNEXTDOT3ADLACPPORTENTRY;
    msg_p->data.arg_lacp_port_entry = *lacp_port_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *lacp_port_entry = msg_p->data.arg_lacp_port_entry;

    return msg_p->type.ret_bool;
} /* End of LACP_PMGR_GetNextDot3adLacpPortEntry */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_PMGR_SetDot3adAggActorSystemPriority
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the dot3ad_agg_actor_system_priority information.
 * INPUT    :   UI16_T  agg_index   -- the dot3ad_agg_index
 *              UI16_T  priority    -- the dot3ad_agg_actor_system_priority value
 * OUTPUT   :   None
 * RETURN   :   LACP_RETURN_SUCCESS/LACP_RETURN_ERROR
 * NOTE     :   None
 * REF      :   None
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_PMGR_SetDot3adAggActorSystemPriority(UI16_T agg_index, UI16_T priority)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LACP_MGR_GET_MSG_SIZE(arg_grp2);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LACP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LACP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LACP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LACP_MGR_IPC_SETDOT3ADAGGACTORSYSTEMPRIORITY;
    msg_p->data.arg_grp2.arg1 = agg_index;
    msg_p->data.arg_grp2.arg2 = priority;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            LACP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return LACP_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
} /* End of LACP_PMGR_SetDot3adAggActorSystemPriority */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_PMGR_SetDot3adAggActorAdminKey
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the dot3ad_agg_actor_admin_key information.
 * INPUT    :   UI16_T  agg_index   -- the dot3ad_agg_index
 *              UI16_T  admin_key   -- the dot3ad_agg_actor_admin_key value
 * OUTPUT   :   None
 * RETURN   :   LACP_RETURN_SUCCESS/LACP_RETURN_ERROR
 * NOTE     :   None
 * REF      :   None
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_PMGR_SetDot3adAggActorAdminKey(UI16_T agg_index, UI16_T admin_key)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LACP_MGR_GET_MSG_SIZE(arg_grp2);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LACP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LACP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LACP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LACP_MGR_IPC_SETDOT3ADAGGACTORADMINKEY;
    msg_p->data.arg_grp2.arg1 = agg_index;
    msg_p->data.arg_grp2.arg2 = admin_key;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            LACP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return LACP_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
} /* End of LACP_PMGR_SetDot3adAggActorAdminKey */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_PMGR_SetDefaultDot3adAggActorAdminKey
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the dot3ad_agg_actor_admin_key information to default.
 * INPUT    :   UI16_T  agg_index   -- the dot3ad_agg_index
 * OUTPUT   :   None
 * RETURN   :   LACP_RETURN_SUCCESS/LACP_RETURN_ERROR
 * NOTE     :   None
 * REF      :   None
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_PMGR_SetDefaultDot3adAggActorAdminKey(UI16_T agg_index)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LACP_MGR_GET_MSG_SIZE(arg_ui16);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LACP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LACP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LACP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LACP_MGR_IPC_SETDEFAULTDOT3ADAGGACTORADMINKEY;
    msg_p->data.arg_ui16 = agg_index;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            LACP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return LACP_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
} /* End of LACP_PMGR_SetDefaultDot3adAggActorAdminKey */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_PMGR_GetRunningDot3adAggActorAdminKey
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the dot3ad_agg_actor_admin_key information.
 * INPUT    :   UI16_T  agg_index   -- the dot3ad_agg_index
 * OUTPUT   :   UI16_T  *admin_key  -- the dot3ad_agg_actor_admin_key value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_PMGR_GetRunningDot3adAggActorAdminKey(UI16_T agg_index, UI16_T *admin_key)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LACP_MGR_GET_MSG_SIZE(arg_grp2);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LACP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LACP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LACP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LACP_MGR_IPC_GETRUNNINGDOT3ADAGGACTORADMINKEY;
    msg_p->data.arg_grp2.arg1 = agg_index;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    *admin_key = msg_p->data.arg_grp2.arg2;

    return msg_p->type.ret_ui32;
} /* End of LACP_PMGR_GetRunningDot3adAggActorAdminKey */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_PMGR_SetDot3adAggCollectorMaxDelay
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the dot3ad_agg_collector_max_delay information.
 * INPUT    :   UI16_T  agg_index   -- the dot3ad_agg_index
 *              UI32_T  max_delay   -- the dot3ad_agg_collector_max_delay value
 * OUTPUT   :   None
 * RETURN   :   LACP_RETURN_SUCCESS/LACP_RETURN_ERROR
 * NOTE     :   None
 * REF      :   None
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_PMGR_SetDot3adAggCollectorMaxDelay(UI16_T agg_index, UI32_T max_delay)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LACP_MGR_GET_MSG_SIZE(arg_grp3);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LACP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LACP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LACP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LACP_MGR_IPC_SETDOT3ADAGGCOLLECTORMAXDELAY;
    msg_p->data.arg_grp3.arg1 = agg_index;
    msg_p->data.arg_grp3.arg2 = max_delay;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            LACP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return LACP_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
} /* End of LACP_PMGR_SetDot3adAggCollectorMaxDelay */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_PMGR_SetDot3adAggPortActorSystemPriority
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the dot3ad_agg_port_actor_system_priority information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 *              UI16_T  priority    -- the dot3ad_agg_port_actor_system_priority value
 * OUTPUT   :   None
 * RETURN   :   LACP_RETURN_SUCCESS/LACP_RETURN_ERROR
 * NOTE     :   None
 * REF      :   None
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_PMGR_SetDot3adAggPortActorSystemPriority(UI16_T port_index, UI16_T priority)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LACP_MGR_GET_MSG_SIZE(arg_grp2);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LACP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LACP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LACP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LACP_MGR_IPC_SETDOT3ADAGGPORTACTORSYSTEMPRIORITY;
    msg_p->data.arg_grp2.arg1 = port_index;
    msg_p->data.arg_grp2.arg2 = priority;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            LACP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return LACP_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
} /* End of LACP_PMGR_SetDot3adAggPortActorSystemPriority */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_PMGR_GetRunningDot3adAggPortActorSystemPriority
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the dot3ad_agg_port_actor_system_priority information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 * OUTPUT   :   UI16_T  *priority   -- the dot3ad_agg_port_actor_system_priority value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_PMGR_GetRunningDot3adAggPortActorSystemPriority(UI16_T port_index, UI16_T *priority)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LACP_MGR_GET_MSG_SIZE(arg_grp2);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LACP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LACP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LACP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LACP_MGR_IPC_GETRUNNINGDOT3ADAGGPORTACTORSYSTEMPRIORITY;
    msg_p->data.arg_grp2.arg1 = port_index;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    *priority = msg_p->data.arg_grp2.arg2;

    return msg_p->type.ret_ui32;
} /* End of LACP_PMGR_GetRunningDot3adAggPortActorSystemPriority */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_PMGR_SetDot3adAggPortActorAdminKey
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the dot3ad_agg_port_actor_admin_key information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 *              UI16_T  admin_key   -- the dot3ad_agg_port_actor_admin_key value
 * OUTPUT   :   None
 * RETURN   :   LACP_RETURN_SUCCESS/LACP_RETURN_ERROR
 * NOTE     :   None
 * REF      :   None
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_PMGR_SetDot3adAggPortActorAdminKey(UI16_T port_index, UI16_T admin_key)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LACP_MGR_GET_MSG_SIZE(arg_grp2);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LACP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LACP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LACP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LACP_MGR_IPC_SETDOT3ADAGGPORTACTORADMINKEY;
    msg_p->data.arg_grp2.arg1 = port_index;
    msg_p->data.arg_grp2.arg2 = admin_key;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            LACP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return LACP_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
} /* End of LACP_PMGR_SetDot3adAggPortActorAdminKey */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_PMGR_GetRunningDot3adAggPortActorAdminKey
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the dot3ad_agg_port_actor_admin_key information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 * OUTPUT   :   UI16_T  *admin_key  -- the dot3ad_agg_port_actor_admin_key value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_PMGR_GetRunningDot3adAggPortActorAdminKey(UI16_T port_index, UI16_T *admin_key)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LACP_MGR_GET_MSG_SIZE(arg_grp2);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LACP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LACP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LACP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LACP_MGR_IPC_GETRUNNINGDOT3ADAGGPORTACTORADMINKEY;
    msg_p->data.arg_grp2.arg1 = port_index;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    *admin_key = msg_p->data.arg_grp2.arg2;

    return msg_p->type.ret_ui32;
} /* End of LACP_PMGR_GetRunningDot3adAggPortActorAdminKey */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_PMGR_SetDefaultDot3adAggPortActorAdminKey
 * ------------------------------------------------------------------------
 * PURPOSE  :   Restore the dot3ad_agg_port_actor_admin_key to default value.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 * OUTPUT   :   None
 * RETURN   :   LACP_RETURN_SUCCESS/LACP_RETURN_ERROR
 * NOTE     :   None
 * REF      :   None
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_PMGR_SetDefaultDot3adAggPortActorAdminKey(UI16_T port_index)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LACP_MGR_GET_MSG_SIZE(arg_ui16);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LACP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LACP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LACP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LACP_MGR_IPC_SETDEFAULTDOT3ADAGGPORTACTORADMINKEY;
    msg_p->data.arg_ui16 = port_index;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            LACP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return LACP_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
} /* End of LACP_PMGR_SetDefaultDot3adAggPortActorAdminKey */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_PMGR_SetDot3adAggPortActorOperKey
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the dot3ad_agg_port_actor_oper_key information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 *              UI16_T  oper_key    -- the dot3ad_agg_port_actor_oper_key value
 * OUTPUT   :   None
 * RETURN   :   LACP_RETURN_SUCCESS/LACP_RETURN_ERROR
 * NOTE     :   None
 * REF      :   None
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_PMGR_SetDot3adAggPortActorOperKey(UI16_T port_index, UI16_T oper_key)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LACP_MGR_GET_MSG_SIZE(arg_grp2);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LACP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LACP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LACP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LACP_MGR_IPC_SETDOT3ADAGGPORTACTOROPERKEY;
    msg_p->data.arg_grp2.arg1 = port_index;
    msg_p->data.arg_grp2.arg2 = oper_key;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            LACP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return LACP_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
} /* End of LACP_PMGR_SetDot3adAggPortActorOperKey */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_PMGR_SetDot3adAggPortPartnerAdminSystemPriority
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the dot3ad_agg_port_partner_admin_system_priority information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 *              UI16_T  priority    -- the dot3ad_agg_port_partner_admin_system_priority value
 * OUTPUT   :   None
 * RETURN   :   LACP_RETURN_SUCCESS/LACP_RETURN_ERROR
 * NOTE     :   None
 * REF      :   None
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_PMGR_SetDot3adAggPortPartnerAdminSystemPriority(UI16_T port_index, UI16_T priority)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LACP_MGR_GET_MSG_SIZE(arg_grp2);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LACP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LACP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LACP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LACP_MGR_IPC_SETDOT3ADAGGPORTPARTNERADMINSYSTEMPRIORITY;
    msg_p->data.arg_grp2.arg1 = port_index;
    msg_p->data.arg_grp2.arg2 = priority;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            LACP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return LACP_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
} /* End of LACP_PMGR_SetDot3adAggPortPartnerAdminSystemPriority */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_PMGR_GetRunningDot3adAggPortPartnerAdminSystemPriority
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the dot3ad_agg_port_partner_admin_system_priority information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 * OUTPUT   :   UI16_T  *priority   -- the dot3ad_agg_port_partner_admin_system_priority value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_PMGR_GetRunningDot3adAggPortPartnerAdminSystemPriority(UI16_T port_index, UI16_T *priority)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LACP_MGR_GET_MSG_SIZE(arg_grp2);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LACP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LACP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LACP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LACP_MGR_IPC_GETRUNNINGDOT3ADAGGPORTPARTNERADMINSYSTEMPRIORITY;
    msg_p->data.arg_grp2.arg1 = port_index;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    *priority = msg_p->data.arg_grp2.arg2;

    return msg_p->type.ret_ui32;
} /* End of LACP_PMGR_GetRunningDot3adAggPortPartnerAdminSystemPriority */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_PMGR_SetDot3adAggPortPartnerAdminSystemId
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the dot3ad_agg_port_partner_admin_system_id information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 *              UI8_T   *system_id  -- the dot3ad_agg_port_partner_admin_system_id value
 * OUTPUT   :   None
 * RETURN   :   LACP_RETURN_SUCCESS/LACP_RETURN_ERROR
 * NOTE     :   None
 * REF      :   None
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_PMGR_SetDot3adAggPortPartnerAdminSystemId(UI16_T port_index, UI8_T *system_id)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LACP_MGR_GET_MSG_SIZE(arg_grp4);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LACP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LACP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LACP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LACP_MGR_IPC_SETDOT3ADAGGPORTPARTNERADMINSYSTEMID;
    msg_p->data.arg_grp4.arg1 = port_index;
    memcpy(msg_p->data.arg_grp4.arg2, system_id, sizeof(msg_p->data.arg_grp4.arg2));

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            LACP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return LACP_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
} /* End of LACP_PMGR_SetDot3adAggPortPartnerAdminSystemId */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_PMGR_SetDot3adAggPortPartnerAdminKey
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the dot3ad_agg_port_partner_admin_key information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 *              UI16_T  admin_key   -- the dot3ad_agg_port_partner_admin_key value
 * OUTPUT   :   None
 * RETURN   :   LACP_RETURN_SUCCESS/LACP_RETURN_ERROR
 * NOTE     :   None
 * REF      :   None
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_PMGR_SetDot3adAggPortPartnerAdminKey(UI16_T port_index, UI16_T admin_key)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LACP_MGR_GET_MSG_SIZE(arg_grp2);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LACP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LACP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LACP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LACP_MGR_IPC_SETDOT3ADAGGPORTPARTNERADMINKEY;
    msg_p->data.arg_grp2.arg1 = port_index;
    msg_p->data.arg_grp2.arg2 = admin_key;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            LACP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return LACP_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
} /* End of LACP_PMGR_SetDot3adAggPortPartnerAdminKey */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_PMGR_GetRunningDot3adAggPortPartnerAdminKey
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the dot3ad_agg_port_partner_admin_key information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 * OUTPUT   :   UI16_T  *admin_key  -- the dot3ad_agg_port_partner_admin_key value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_PMGR_GetRunningDot3adAggPortPartnerAdminKey(UI16_T port_index, UI16_T *admin_key)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LACP_MGR_GET_MSG_SIZE(arg_grp2);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LACP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LACP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LACP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LACP_MGR_IPC_GETRUNNINGDOT3ADAGGPORTPARTNERADMINKEY;
    msg_p->data.arg_grp2.arg1 = port_index;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    *admin_key = msg_p->data.arg_grp2.arg2;

    return msg_p->type.ret_ui32;
} /* End of LACP_PMGR_GetRunningDot3adAggPortPartnerAdminKey */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_PMGR_SetDot3adAggPortActorPortPriority
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the dot3ad_agg_port_actor_port_priority information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 *              UI16_T  priority    -- the dot3ad_agg_port_actor_port_priority value
 * OUTPUT   :   None
 * RETURN   :   LACP_RETURN_SUCCESS/LACP_RETURN_ERROR
 * NOTE     :   None
 * REF      :   None
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_PMGR_SetDot3adAggPortActorPortPriority(UI16_T port_index, UI16_T priority)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LACP_MGR_GET_MSG_SIZE(arg_grp2);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LACP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LACP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LACP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LACP_MGR_IPC_SETDOT3ADAGGPORTACTORPORTPRIORITY;
    msg_p->data.arg_grp2.arg1 = port_index;
    msg_p->data.arg_grp2.arg2 = priority;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            LACP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return LACP_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
} /* End of LACP_PMGR_SetDot3adAggPortActorPortPriority */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_PMGR_GetRunningDot3adAggPortActorPortPriority
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the dot3ad_agg_port_actor_port_priority information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 * OUTPUT   :   UI16_T  *priority   -- the dot3ad_agg_port_actor_port_priority value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_PMGR_GetRunningDot3adAggPortActorPortPriority(UI16_T port_index, UI16_T *priority)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LACP_MGR_GET_MSG_SIZE(arg_grp2);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LACP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LACP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LACP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LACP_MGR_IPC_GETRUNNINGDOT3ADAGGPORTACTORPORTPRIORITY;
    msg_p->data.arg_grp2.arg1 = port_index;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    *priority = msg_p->data.arg_grp2.arg2;

    return msg_p->type.ret_ui32;
} /* End of LACP_PMGR_GetRunningDot3adAggPortActorPortPriority */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_PMGR_SetDot3adAggPortPartnerAdminPort
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the dot3ad_agg_port_partner_admin_port information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 *              UI16_T  admin_port  -- the dot3ad_agg_port_partner_admin_port value
 * OUTPUT   :   None
 * RETURN   :   LACP_RETURN_SUCCESS/LACP_RETURN_ERROR
 * NOTE     :   None
 * REF      :   None
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_PMGR_SetDot3adAggPortPartnerAdminPort(UI16_T port_index, UI16_T admin_port)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LACP_MGR_GET_MSG_SIZE(arg_grp2);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LACP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LACP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LACP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LACP_MGR_IPC_SETDOT3ADAGGPORTPARTNERADMINPORT;
    msg_p->data.arg_grp2.arg1 = port_index;
    msg_p->data.arg_grp2.arg2 = admin_port;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            LACP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return LACP_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
} /* End of LACP_PMGR_SetDot3adAggPortPartnerAdminPort */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_PMGR_SetDot3adAggPortPartnerAdminPortPriority
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the dot3ad_agg_port_partner_admin_port_priority information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 *              UI16_T  priority    -- the dot3ad_agg_port_partner_admin_port_priority value
 * OUTPUT   :   None
 * RETURN   :   LACP_RETURN_SUCCESS/LACP_RETURN_ERROR
 * NOTE     :   None
 * REF      :   None
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_PMGR_SetDot3adAggPortPartnerAdminPortPriority(UI16_T port_index, UI16_T priority)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LACP_MGR_GET_MSG_SIZE(arg_grp2);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LACP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LACP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LACP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LACP_MGR_IPC_SETDOT3ADAGGPORTPARTNERADMINPORTPRIORITY;
    msg_p->data.arg_grp2.arg1 = port_index;
    msg_p->data.arg_grp2.arg2 = priority;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            LACP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return LACP_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
} /* End of LACP_PMGR_SetDot3adAggPortPartnerAdminPortPriority */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_PMGR_GetRunningDot3adAggPortPartnerAdminPortPriority
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the dot3ad_agg_port_partner_admin_port_priority information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 * OUTPUT   :   UI16_T  *priority   -- the dot3ad_agg_port_partner_admin_port_priority value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_PMGR_GetRunningDot3adAggPortPartnerAdminPortPriority(UI16_T port_index, UI16_T *priority)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LACP_MGR_GET_MSG_SIZE(arg_grp2);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LACP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LACP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LACP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LACP_MGR_IPC_GETRUNNINGDOT3ADAGGPORTPARTNERADMINPORTPRIORITY;
    msg_p->data.arg_grp2.arg1 = port_index;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    *priority = msg_p->data.arg_grp2.arg2;

    return msg_p->type.ret_ui32;
} /* End of LACP_PMGR_GetRunningDot3adAggPortPartnerAdminPortPriority */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_PMGR_SetDot3adAggPortActorAdminState
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the dot3ad_agg_port_actor_admin_state information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 *              UI8_T   admin_state -- the dot3ad_agg_port_actor_admin_state value
 * OUTPUT   :   None
 * RETURN   :   LACP_RETURN_SUCCESS/LACP_RETURN_ERROR
 * NOTE     :   None
 * REF      :   None
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_PMGR_SetDot3adAggPortActorAdminState(UI16_T port_index, UI8_T admin_state)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LACP_MGR_GET_MSG_SIZE(arg_grp5);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LACP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LACP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LACP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LACP_MGR_IPC_SETDOT3ADAGGPORTACTORADMINSTATE;
    msg_p->data.arg_grp5.arg1 = port_index;
    msg_p->data.arg_grp5.arg2 = admin_state;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            LACP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return LACP_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
} /* End of LACP_PMGR_SetDot3adAggPortActorAdminState */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_PMGR_SetDot3adAggPortPartnerAdminState
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the dot3ad_agg_port_partner_admin_state information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 *              UI8_T   admin_state -- the dot3ad_agg_port_partner_admin_state value
 * OUTPUT   :   None
 * RETURN   :   LACP_RETURN_SUCCESS/LACP_RETURN_ERROR
 * NOTE     :   None
 * REF      :   None
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_PMGR_SetDot3adAggPortPartnerAdminState(UI16_T port_index, UI8_T admin_state)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LACP_MGR_GET_MSG_SIZE(arg_grp5);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LACP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LACP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LACP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LACP_MGR_IPC_SETDOT3ADAGGPORTPARTNERADMINSTATE;
    msg_p->data.arg_grp5.arg1 = port_index;
    msg_p->data.arg_grp5.arg2 = admin_state;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            LACP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return LACP_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
} /* End of LACP_PMGR_SetDot3adAggPortPartnerAdminState */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_PMGR_SetDot3adAggPortActorLACP_Timeout
 * ------------------------------------------------------------------------
 * PURPOSE  :   set the port with long term or short term timeout
 * INPUT    :   UI32_T  unit  -- the unit number
 *              UI32_T  port  -- ther port number
 *              UI32_T  timeout -- long or short term timeout
 * RETURN   :   LACP_RETURN_SUCCESS - success
 *              LACP_RETURN_ERROR - fail
 * NOTES    :
 * ------------------------------------------------------------------------
 */
UI32_T LACP_PMGR_SetDot3adAggPortActorLACP_Timeout(UI32_T unit, UI32_T port, UI32_T timeout)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LACP_MGR_GET_MSG_SIZE(arg_grp6);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LACP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LACP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LACP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LACP_MGR_IPC_SETDOT3ADAGGPORTACTORLACP_TIMEOUT;
    msg_p->data.arg_grp6.arg1 = unit;
    msg_p->data.arg_grp6.arg2 = port;
    msg_p->data.arg_grp6.arg3 = timeout;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            LACP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return LACP_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
} /* End of LACP_PMGR_SetDot3adAggPortActorLACP_Timeout */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_PMGR_GetDot3adAggPortActorLACP_Timeout
 * ------------------------------------------------------------------------
 * PURPOSE  :   get the port with long term or short term timeout
 * INPUT    :   UI32_T  unit  -- the unit number
 *              UI32_T  port  -- ther port number
 *              UI32_T  timeout -- long or short term timeout
 * RETURN   :   LACP_RETURN_SUCCESS - success
 *              LACP_RETURN_ERROR - fail
 * NOTES    :
 * ------------------------------------------------------------------------
 */
UI32_T LACP_PMGR_GetDot3adAggPortActorLACP_Timeout(UI32_T unit, UI32_T port, UI32_T *timeout)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LACP_MGR_GET_MSG_SIZE(arg_grp6);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LACP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LACP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LACP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LACP_MGR_IPC_GETDOT3ADAGGPORTACTORLACP_TIMEOUT;
    msg_p->data.arg_grp6.arg1 = unit;
    msg_p->data.arg_grp6.arg2 = port;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return LACP_RETURN_ERROR;
    }

    *timeout = msg_p->data.arg_grp6.arg3;

    return msg_p->type.ret_ui32;
} /* End of LACP_PMGR_GetDot3adAggPortActorLACP_Timeout */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_PMGR_GetRunningDot3adAggPortActorLACP_Timeout
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the dot3ad_agg_port_actor_port_lacp_timeout information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 * OUTPUT   :   UI16_T  *priority   -- the dot3ad_agg_port_actor_port_priority value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_PMGR_GetRunningDot3adAggPortActorLACP_Timeout(UI16_T port_index, UI32_T *timeout)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LACP_MGR_GET_MSG_SIZE(arg_grp3);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LACP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LACP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LACP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LACP_MGR_IPC_GETRUNNINGDOT3ADAGGPORTACTORLACP_TIMEOUT;
    msg_p->data.arg_grp3.arg1 = port_index;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    *timeout = msg_p->data.arg_grp3.arg2;

    return msg_p->type.ret_ui32;
} /* End of LACP_PMGR_GetRunningDot3adAggPortActorLACP_Timeout */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_PMGR_SetDot3adAggActorLACP_Timeout
 * ------------------------------------------------------------------------
 * PURPOSE  :   set the lacp port channel with long term or short term timeout.
 * INPUT    :   UI16_T  agg_index   -- the dot3ad_agg_index
 *              UI32_T  timeout   -- long or short term timeout
 * OUTPUT   :   None
 * RETURN   :   LACP_RETURN_SUCCESS/LACP_RETURN_ERROR
 * NOTE     :   None
 * REF      :   None
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_PMGR_SetDot3adAggActorLACP_Timeout(UI16_T agg_index, UI32_T timeout)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LACP_MGR_GET_MSG_SIZE(arg_grp3);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LACP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LACP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LACP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LACP_MGR_IPC_SETDOT3ADAGGACTORLACP_TIMEOUT;
    msg_p->data.arg_grp3.arg1 = agg_index;
    msg_p->data.arg_grp3.arg2 = timeout;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            LACP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return LACP_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
} /* End of LACP_PMGR_SetDot3adAggActorLACP_Timeout */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_PMGR_GetDot3adAggActorLACP_Timeout
 * ------------------------------------------------------------------------
 * PURPOSE  :   get the lacp port channel with long term or short term timeout
 * INPUT    :   UI16_T  agg_index   -- the dot3ad_agg_index
 *              UI32_T  *timeout -- long or short term timeout
 * RETURN   :   LACP_RETURN_SUCCESS - success
 *              LACP_RETURN_ERROR - fail
 * NOTES    :
 * ------------------------------------------------------------------------
 */
UI32_T LACP_PMGR_GetDot3adAggActorLACP_Timeout(UI16_T agg_index, UI32_T *timeout)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LACP_MGR_GET_MSG_SIZE(arg_grp3);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LACP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LACP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LACP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LACP_MGR_IPC_GETDOT3ADAGGACTORLACP_TIMEOUT;
    msg_p->data.arg_grp3.arg1 = agg_index;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return LACP_RETURN_ERROR;
    }

    *timeout = msg_p->data.arg_grp3.arg2;

    return msg_p->type.ret_ui32;
} /* End of LACP_PMGR_GetDot3adAggActorLACP_Timeout */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_PMGR_GetRunningDot3adAggActorLACP_Timeout
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the dot3ad_agg_port_actor_lacp_timeout information.
 * INPUT    :   UI16_T  agg_index  -- the dot3ad_agg_index
 * OUTPUT   :   UI16_T  *timeout   -- long or short term timeout
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_PMGR_GetRunningDot3adAggActorLACP_Timeout(UI16_T agg_index, UI32_T *timeout)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LACP_MGR_GET_MSG_SIZE(arg_grp3);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LACP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LACP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LACP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LACP_MGR_IPC_GETRUNNINGDOT3ADAGGACTORLACP_TIMEOUT;
    msg_p->data.arg_grp3.arg1 = agg_index;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    *timeout = msg_p->data.arg_grp3.arg2;

    return msg_p->type.ret_ui32;
} /* End of LACP_PMGR_GetRunningDot3adAggActorLACP_Timeout */

