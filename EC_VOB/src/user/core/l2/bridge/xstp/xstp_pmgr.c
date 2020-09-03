/*-----------------------------------------------------------------------------
 * FILE NAME: XSTP_PMGR.C
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *    This file implements the APIs for XSTP MGR IPC.
 *
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    2007/05/03     --- Timon, Create
 *
 * Copyright(C)      Accton Corporation, 2007
 *-----------------------------------------------------------------------------
 */


/* INCLUDE FILE DECLARATIONS
 */

#include <stdio.h>
#include <string.h>
#include "sys_bld.h"
#include "sys_type.h"
#include "sys_module.h"
#include "sysfun.h"
#include "xstp_mgr.h"
#include "xstp_pmgr.h"
#include "xstp_type.h"


/* NAMING CONSTANT DECLARATIONS
 */


/* MACRO FUNCTION DECLARATIONS
 */
#define XSTP_PMGR_FUNC_BEGIN(req_sz, rep_sz, cmd_id)            \
        const UI32_T        req_size = req_sz;                  \
        const UI32_T        rep_size = rep_sz;                  \
        UI8_T               ipc_buf[SYSFUN_SIZE_OF_MSG((req_sz>rep_size)?req_sz:rep_size)]; \
        SYSFUN_Msg_T        *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf; \
        XSTP_MGR_IpcMsg_T   *msg_p;                             \
                                                                \
        msgbuf_p->cmd = SYS_MODULE_XSTP;                        \
        msgbuf_p->msg_size = req_size;                          \
        msg_p = (XSTP_MGR_IpcMsg_T *)msgbuf_p->msg_buf;         \
        msg_p->type.cmd = cmd_id;


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
 * ROUTINE NAME : XSTP_PMGR_InitiateProcessResource
 *-----------------------------------------------------------------------------
 * PURPOSE : Initiate resource for XSTP_PMGR in the calling process.
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
BOOL_T XSTP_PMGR_InitiateProcessResource(void)
{
    /* get the ipc message queues for XSTP MGR
     */
    if (SYSFUN_GetMsgQ(SYS_BLD_STA_GROUP_IPCMSGQ_KEY,
            SYSFUN_MSGQ_BIDIRECTIONAL, &ipcmsgq_handle) != SYSFUN_OK)
    {
        printf("\r\n%s(): SYSFUN_GetMsgQ failed.\r\n", __FUNCTION__);
        return FALSE;
    }

    return TRUE;
} /* End of XSTP_PMGR_InitiateProcessResource */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_SetChangeStatePortListForbidden
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will set the flag which controls whether the
 *            XSTP_MGR_ChangeStatePortList is allowed to be added new
 *            element.
 * INPUT    : flag    -- TRUE:disallowed, FALSE:allowed
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
void    XSTP_PMGR_SetChangeStatePortListForbidden(BOOL_T flag)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_MGR_GET_MSG_SIZE(arg_bool);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_MGR_IPC_SETCHANGESTATEPORTLISTFORBIDDEN;
    msg_p->data.arg_bool = flag;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            XSTP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return;
    }

    return;
} /* End of XSTP_PMGR_SetChangeStatePortListForbidden */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_SetSystemSpanningTreeStatus
 * ----------------------------------------------------------------------------
 * PURPOSE : Provide other processes to use
 *           XSTP_MGR_SetSystemSpanningTreeStatus through IPC message queue.
 *
 * INPUT   : status -- the status value
 *                     VAL_xstpSystemStatus_enabled
 *                     VAL_xstpSystemStatus_disabled
 *
 * OUTPUT  : None.
 *
 * RETURN  : XSTP_TYPE_RETURN_OK                -- set successfully
 *           XSTP_TYPE_RETURN_ERROR             -- failed
 *           XSTP_TYPE_RETURN_MASTER_MODE_ERROR -- not master mode
 *
 * NOTES   : None.
 * ----------------------------------------------------------------------------
 */
UI32_T XSTP_PMGR_SetSystemSpanningTreeStatus(UI32_T status)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_MGR_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_MGR_IPC_SETSYSTEMSPANNINGTREESTATUS;
    msg_p->data.arg_ui32 = status;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            XSTP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return XSTP_TYPE_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
} /* End of XSTP_PMGR_SetSystemSpanningTreeStatus */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_SetSystemSpanningTreeVersion
 * ----------------------------------------------------------------------------
 * PURPOSE : Set the spanning tree mode.
 *
 * OUTPUT  : mode -- the mode value
 *                   VAL_dot1dStpVersion_stpCompatible(0)
 *                   VAL_dot1dStpVersion_rstp(2)
 *                   VAL_dot1dStpVersion_mstp(3)
 *
 * OUTPUT  : None.
 *
 * RETURN  : XSTP_TYPE_RETURN_OK                 -- set successfully
 *           XSTP_TYPE_RETURN_ERROR              -- failed
 *           XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 *           XSTP_TYPE_RETURN_ST_STATUS_DISABLED -- status is disabled
 *
 * NOTE    : Default -- SYS_DFLT_STP_PROTOCOL_TYPE
 *           Can't set mode when the status is disabled.
 *
 * REF     : draft-ietf-bridge-rstpmib-02/dot1dStp 16
 * ----------------------------------------------------------------------------
 */
UI32_T XSTP_PMGR_SetSystemSpanningTreeVersion(UI32_T mode)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_MGR_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_MGR_IPC_SETSYSTEMSPANNINGTREEVERSION;
    msg_p->data.arg_ui32 = mode;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            XSTP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return XSTP_TYPE_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
} /* End of XSTP_PMGR_SetSystemSpanningTreeVersion */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_SetForwardDelay
 * ----------------------------------------------------------------------------
 * PURPOSE : Provide other processes to use XSTP_IMGR_SetForwardDelay
 *           through IPC message queue.
 *
 * INPUT   : forward_delay -- the forward_delay value
 *
 * OUTPUT  : None.
 *
 * RETURN  : XSTP_TYPE_RETURN_OK                -- set successfully
 *           XSTP_TYPE_RETURN_ERROR             -- failed
 *           XSTP_TYPE_RETURN_MASTER_MODE_ERROR -- not master mode
 *           XSTP_TYPE_RETURN_INDEX_OOR         -- forward_delay out of range
 *
 * NOTES   : 1. Time unit is 1/100 sec
 *           2. Range
 *              -- XSTP_TYPE_MIN_FORWARD_DELAY
 *              -- XSTP_TYPE_MAX_FORWARD_DELAY
 *           3. Default
 *              -- XSTP_TYPE_DEFAULT_FORWARD_DELAY
 *
 * REF     : RFC-1493/dot1dStp 14
 * ----------------------------------------------------------------------------
 */
UI32_T XSTP_PMGR_SetForwardDelay(UI32_T forward_delay)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_MGR_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_MGR_IPC_SETFORWARDDELAY;
    msg_p->data.arg_ui32 = forward_delay;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            XSTP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return XSTP_TYPE_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
} /* End of XSTP_PMGR_SetForwardDelay */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_SetHelloTime
 * ----------------------------------------------------------------------------
 * PURPOSE : Set the hello_time information.
 *
 * INPUT   : hello_time -- the hello_time value
 *
 * OUTPUT  : None.
 *
 * RETURN  : XSTP_TYPE_RETURN_OK                -- set successfully
 *           XSTP_TYPE_RETURN_ERROR             -- failed
 *           XSTP_TYPE_RETURN_MASTER_MODE_ERROR -- not master mode
 *           XSTP_TYPE_RETURN_INDEX_OOR         -- hello_time out of range
 *
 * NOTES   : 1. Time unit is 1/100 sec
 *           2. Range
 *              -- XSTP_TYPE_MIN_HELLO_TIME
 *              -- XSTP_TYPE_MAX_HELLO_TIME
 *           3. Default
 *              -- XSTP_TYPE_DEFAULT_HELLO_TIME
 *
 * REF     : RFC-1493/dot1dStp 13
 * ----------------------------------------------------------------------------
 */
UI32_T XSTP_PMGR_SetHelloTime(UI32_T hello_time)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_MGR_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_MGR_IPC_SETHELLOTIME;
    msg_p->data.arg_ui32 = hello_time;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            XSTP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return XSTP_TYPE_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
} /* End of XSTP_PMGR_SetHelloTime */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_SetMaxAge
 * ----------------------------------------------------------------------------
 * PURPOSE : Set the max_age information.
 *
 * INPUT   : max_age -- the max_age value
 *
 * OUTPUT  : None.
 *
 * RETURN  : XSTP_TYPE_RETURN_OK                -- set successfully
 *           XSTP_TYPE_RETURN_ERROR             -- failed
 *           XSTP_TYPE_RETURN_MASTER_MODE_ERROR -- not master mode
 *           XSTP_TYPE_RETURN_INDEX_OOR         -- max_age out of range
 *
 * NOTES   : 1. Time unit is 1/100 sec
 *           2. Range
 *              -- XSTP_TYPE_MIN_MAXAGE
 *              -- XSTP_TYPE_MAX_MAXAGE
 *           3. Default
 *              -- XSTP_TYPE_DEFAULT_MAX_AGE
 *
 * REF     : RFC-1493/dot1dStp 12
 * ----------------------------------------------------------------------------
 */
UI32_T XSTP_PMGR_SetMaxAge(UI32_T max_age)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_MGR_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_MGR_IPC_SETMAXAGE;
    msg_p->data.arg_ui32 = max_age;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            XSTP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return XSTP_TYPE_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
} /* End of XSTP_PMGR_SetMaxAge */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_SetPathCostMethod
 * ----------------------------------------------------------------------------
 * PURPOSE : Set the default path cost calculation method.
 *
 * INPUT   : pathcost_method -- the method value
 *                              VAL_dot1dStpPathCostDefault_stp8021d1998(1)
 *                              VAL_dot1dStpPathCostDefault_stp8021t2001(2)
 *
 * OUTPUT  : None.
 *
 * RETURN  : XSTP_TYPE_RETURN_OK                -- set successfully
 *           XSTP_TYPE_RETURN_ERROR             -- failed
 *           XSTP_TYPE_RETURN_MASTER_MODE_ERROR -- not master mode
 *
 * NOTES   : 1. Long
 *              -- 32-bit based values for default port path costs.
 *              -- VAL_dot1dStpPathCostDefault_stp8021t2001
 *           2. Short
 *              -- 16-bit based values for default port path costs.
 *              -- VAL_dot1dStpPathCostDefault_stp8021d1998
 *           3. Default
 *              -- The long method.
 *
 * REF     : draft-ietf-bridge-rstpmib-02/dot1dStp 18
 * ----------------------------------------------------------------------------
 */
UI32_T XSTP_PMGR_SetPathCostMethod(UI32_T pathcost_method)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_MGR_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_MGR_IPC_SETPATHCOSTMETHOD;
    msg_p->data.arg_ui32 = pathcost_method;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            XSTP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return XSTP_TYPE_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
} /* End of XSTP_PMGR_SetPathCostMethod */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_SetTransmissionLimit
 * ----------------------------------------------------------------------------
 * PURPOSE : Set the transmission limit count vlaue.
 *
 * INPUT   : tx_hold_count -- the TXHoldCount value
 *
 * OUTPUT  : None.
 *
 * RETURN  : XSTP_TYPE_RETURN_OK                -- set successfully
 *           XSTP_TYPE_RETURN_ERROR             -- failed
 *           XSTP_TYPE_RETURN_MASTER_MODE_ERROR -- not master mode
 *           XSTP_TYPE_RETURN_INDEX_OOR         -- tx_hold_count out of range
 *
 * NOTES   : 1. The value used by the Port Transmit state machine to
 *              limit the maximum transmission rate.
 *           2. Range
 *              -- XSTP_TYPE_MIN_TX_HOLD_COUNT
 *              -- XSTP_TYPE_MAX_TX_HOLD_COUNT
 *           3. Default
 *              -- XSTP_TYPE_DEFAULT_TX_HOLD_COUNT
 *
 * REF     : draft-ietf-bridge-rstpmib-02/dot1dStp 17
 * ----------------------------------------------------------------------------
 */
UI32_T XSTP_PMGR_SetTransmissionLimit(UI32_T tx_hold_count)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_MGR_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_MGR_IPC_SETTRANSMISSIONLIMIT;
    msg_p->data.arg_ui32 = tx_hold_count;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            XSTP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return XSTP_TYPE_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
} /* End of XSTP_PMGR_SetTransmissionLimit */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_SetSystemGlobalPriority
 * ----------------------------------------------------------------------------
 * PURPOSE : Set the global priority value when the switch is MST mode.
 *
 * INPUT   : priority -- the priority value
 *
 * OUTPUT  : None.
 *
 * RETURN  : XSTP_TYPE_RETURN_OK                -- set successfully
 *           XSTP_TYPE_RETURN_ERROR             -- failed
 *           XSTP_TYPE_RETURN_MASTER_MODE_ERROR -- not master mode
 *           XSTP_TYPE_RETURN_INDEX_OOR         -- priority out of range
 *
 * NOTES   : 1. When mode is STP or RSTP
 *              -- Only set the priority value for mstid = 0.
 *           2. When mode is MSTP
 *              -- Set the priority value for all MST instances.
 *           3. Range : 0 ~ 61440 (in steps of 4096)
 *                      XSTP_TYPE_MIN_BRIDGE_PRIORITY
 *                      XSTP_TYPE_MAX_BRIDGE_PRIORITY_RSTP_MSTP
 *           4. DEFAULT : 32768
 *              -- XSTP_TYPE_DEFAULT_BRIDGE_PRIORITY
 *
 * REF     : RFC-1493/dot1dStp 2
 * ----------------------------------------------------------------------------
 */
UI32_T XSTP_PMGR_SetSystemGlobalPriority(UI32_T priority)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_MGR_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_MGR_IPC_SETSYSTEMGLOBALPRIORITY;
    msg_p->data.arg_ui32 = priority;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            XSTP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return XSTP_TYPE_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
} /* End of XSTP_PMGR_SetSystemGlobalPriority */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_SetSystemBridgePriority
 * ----------------------------------------------------------------------------
 * PURPOSE : Set the bridge priority value.
 *
 * INPUT   : priority -- the priority value
 *
 * OUTPUT  : None.
 *
 * RETURN  : XSTP_TYPE_RETURN_OK                -- set successfully
 *           XSTP_TYPE_RETURN_ERROR             -- failed
 *           XSTP_TYPE_RETURN_MASTER_MODE_ERROR -- not master mode
 *           XSTP_TYPE_RETURN_INDEX_OOR         -- priority out of range
 *
 * NOTES   : 1. Only set the priority value for mstid = 0.
 *           2. Range : 0 ~ 61440 (in steps of 4096)
 *                      XSTP_TYPE_MIN_BRIDGE_PRIORITY
 *                      XSTP_TYPE_MAX_BRIDGE_PRIORITY_RSTP_MSTP
 *           3. DEFAULT : 32768
 *              -- XSTP_TYPE_DEFAULT_BRIDGE_PRIORITY
 *
 * REF     : RFC-1493/dot1dStp 2
 * ----------------------------------------------------------------------------
 */
UI32_T XSTP_PMGR_SetSystemBridgePriority(UI32_T priority)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_MGR_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_MGR_IPC_SETSYSTEMBRIDGEPRIORITY;
    msg_p->data.arg_ui32 = priority;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            XSTP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return XSTP_TYPE_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
} /* End of XSTP_PMGR_SetSystemBridgePriority */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_SetPortPathCost
 * ----------------------------------------------------------------------------
 * PURPOSE : Set the path_cost of the port.
 *
 * INPUT   : lport     -- lport number
 *           path_cost -- the path_cost value
 *
 * OUTPUT  : None.
 *
 * RETURN  : XSTP_TYPE_RETURN_OK                -- set successfully
 *           XSTP_TYPE_RETURN_ERROR             -- failed
 *           XSTP_TYPE_RETURN_MASTER_MODE_ERROR -- not master mode
 *           XSTP_TYPE_RETURN_PORTNO_OOR        -- port number out of range
 *
 * NOTES   : 1. When mode is STP or RSTP
 *              -- For mstid = 0
 *                 Set the path cost value to the specified port.
 *           2. When mode is MSTP
 *              -- For all instances
 *                 Set the path cost value to the specified port.
 *           3. In long mode (VAL_dot1dStpPathCostDefault_stp8021t2001)
 *              -- Range : 0 ~ 200000000
 *                 XSTP_TYPE_MIN_PORT_PATH_COST
 *                 XSTP_TYPE_MAX_PORT_PATH_COST_32
 *           4. In short mode (VAL_dot1dStpPathCostDefault_stp8021d1998)
 *              -- Range : 0 ~ 65535
 *                 XSTP_TYPE_MIN_PORT_PATH_COST
 *                 XSTP_TYPE_MAX_PORT_PATH_COST_16
 *
 * REF     : RFC-1493/dot1dStpPortEntry 5
 * ----------------------------------------------------------------------------
 */
UI32_T XSTP_PMGR_SetPortPathCost(UI32_T lport, UI32_T path_cost)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_MGR_GET_MSG_SIZE(arg_grp1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_MGR_IPC_SETPORTPATHCOST;
    msg_p->data.arg_grp1.arg1 = lport;
    msg_p->data.arg_grp1.arg2 = path_cost;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            XSTP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return XSTP_TYPE_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
} /* End of XSTP_PMGR_SetPortPathCost */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_SetPortPriority
 * ----------------------------------------------------------------------------
 * PURPOSE : Set the port priority value.
 *
 * INPUT   : lport    -- lport number
 *           mstid    -- instance value
 *           priority -- the priority value
 *
 * OUTPUT  : None.
 *
 * RETURN  : XSTP_TYPE_RETURN_OK                -- set successfully
 *           XSTP_TYPE_RETURN_ERROR             -- failed
 *           XSTP_TYPE_RETURN_MASTER_MODE_ERROR -- not master mode
 *           XSTP_TYPE_RETURN_PORTNO_OOR        -- lport number out of range
 *           XSTP_TYPE_RETURN_INDEX_OOR         -- priority out of range
 *
 * NOTES   : 1. When mode is STP or RSTP
 *              -- For mstid = 0
 *                 Set the priority value to the specified port.
 *           2. When mode is MSTP
 *              -- For all instances
 *                 Set the priority value to the specified port.
 *           3. Range : 0 ~ 240 (in steps of 16)
 *                      XSTP_TYPE_MIN_PORT_PRIORITY
 *                      XSTP_TYPE_MAX_PORT_PRIORITY_RSTP_MSTP
 *           4. Default : 128
 *              -- XSTP_TYPE_DEFAULT_PORT_PRIORITY
 *
 * REF     : RFC-1493/dot1dStpPortEntry 2
 * ----------------------------------------------------------------------------
 */
UI32_T XSTP_PMGR_SetPortPriority(UI32_T lport, UI32_T priority)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_MGR_GET_MSG_SIZE(arg_grp1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_MGR_IPC_SETPORTPRIORITY;
    msg_p->data.arg_grp1.arg1 = lport;
    msg_p->data.arg_grp1.arg2 = priority;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            XSTP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return XSTP_TYPE_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
} /* End of XSTP_PMGR_SetPortPriority */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_SetPortLinkTypeMode
 * ----------------------------------------------------------------------------
 * PURPOSE : Set a link type for a port when mode is RSTP or MSTP.
 *
 * INPUT   : lport -- lport number
 *           mode  -- the status value
 *                    VAL_dot1dStpPortAdminPointToPoint_forceTrue(0)
 *                    VAL_dot1dStpPortAdminPointToPoint_forceFalse(1)
 *                    VAL_dot1dStpPortAdminPointToPoint_auto(2)
 *
 * OUTPUT  : None.
 *
 * RETURN  : XSTP_TYPE_RETURN_OK              -- set successfully
 *           XSTP_TYPE_RETURN_ERROR           -- failed
 *           XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 *           XSTP_TYPE_RETURN_INDEX_OOR       -- mstid out of range
 *           XSTP_TYPE_RETURN_PORTNO_OOR      -- port number out of range
 *
 * NOTES   : Default value
 *           -- VAL_dot1dStpPortAdminPointToPoint_auto
 *
 * REF     : draft-ietf-bridge-rstpmib-02/dot1dStpExtPortEntry 4
 * ----------------------------------------------------------------------------
 */
UI32_T XSTP_PMGR_SetPortLinkTypeMode(UI32_T lport, UI32_T mode)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_MGR_GET_MSG_SIZE(arg_grp1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_MGR_IPC_SETPORTLINKTYPEMODE;
    msg_p->data.arg_grp1.arg1 = lport;
    msg_p->data.arg_grp1.arg2 = mode;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            XSTP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return XSTP_TYPE_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
} /* End of XSTP_PMGR_SetPortLinkTypeMode */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_SetPortProtocolMigration
 * ----------------------------------------------------------------------------
 * PURPOSE : Set protocol_migration status for a port.
 *
 * INPUT   : lport -- lport number
 *           mode  -- the mode value
 *                    VAL_dot1dStpPortProtocolMigration_true
 *                    VAL_dot1dStpPortProtocolMigration_false
 *
 * OUTPUT  : None.
 *
 * RETURN  : XSTP_TYPE_RETURN_OK                -- set successfully
 *           XSTP_TYPE_RETURN_ERROR             -- failed
 *           XSTP_TYPE_RETURN_MASTER_MODE_ERROR -- not master mode
 *           XSTP_TYPE_RETURN_INDEX_OOR         -- mstid out of range
 *           XSTP_TYPE_RETURN_PORTNO_OOR        -- port number out of range
 *
 * NOTES   : Default value
 *           -- FALSE
 *              XSTP_TYPE_DEFAULT_PORT_PROTOCOL_MIGRATION_STATUS
 *
 * REF     : draft-ietf-bridge-rstpmib-02/dot1dStpExtPortEntry 1
 * ----------------------------------------------------------------------------
 */
UI32_T XSTP_PMGR_SetPortProtocolMigration(UI32_T lport, UI32_T mode)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_MGR_GET_MSG_SIZE(arg_grp1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_MGR_IPC_SETPORTPROTOCOLMIGRATION;
    msg_p->data.arg_grp1.arg1 = lport;
    msg_p->data.arg_grp1.arg2 = mode;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            XSTP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return XSTP_TYPE_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
} /* End of XSTP_PMGR_SetPortProtocolMigration */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_SetPortAdminEdgePort
 * ----------------------------------------------------------------------------
 * PURPOSE : Set edge_port status for a port.
 *
 * INPUT   : lport -- lport number
 *           mode  -- the mode value
 *                    VAL_dot1dStpPortAdminEdgePort_true
 *                    VAL_dot1dStpPortAdminEdgePort_false
 *
 * OUTPUT  : None.
 *
 * RETURN  : XSTP_TYPE_RETURN_OK                -- set successfully
 *           XSTP_TYPE_RETURN_ERROR             -- failed
 *           XSTP_TYPE_RETURN_MASTER_MODE_ERROR -- not master mode
 *           XSTP_TYPE_RETURN_INDEX_OOR         -- mstid out of range
 *           XSTP_TYPE_RETURN_PORTNO_OOR        -- port number out of range
 *
 * NOTES   : Default value
 *           -- FALSE
 *              XSTP_TYPE_DEFAULT_PORT_ADMIN_EDGE_PORT
 *
 * REF     : draft-ietf-bridge-rstpmib-02/dot1dStpExtPortEntry 1
 * ----------------------------------------------------------------------------
 */
UI32_T XSTP_PMGR_SetPortAdminEdgePort(UI32_T lport, UI32_T mode)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_MGR_GET_MSG_SIZE(arg_grp1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_MGR_IPC_SETPORTADMINEDGEPORT;
    msg_p->data.arg_grp1.arg1 = lport;
    msg_p->data.arg_grp1.arg2 = mode;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            XSTP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return XSTP_TYPE_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
} /* End of XSTP_PMGR_SetPortAdminEdgePort */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_SetMstPriority
 * ----------------------------------------------------------------------------
 * PURPOSE : Set the priority of the specified MST instance
 *           when the switch is MST mode.
 *
 * INPUT   : priority -- the priority value
 *           mstid    -- instance value
 *
 * OUTPUT  : None.
 *
 * RETURN  : XSTP_TYPE_RETURN_OK                -- set successfully
 *           XSTP_TYPE_RETURN_ERROR             -- failed
 *           XSTP_TYPE_RETURN_MASTER_MODE_ERROR -- not master mode
 *           XSTP_TYPE_RETURN_INDEX_OOR         -- mstid and priority
 *                                                 out of range
 *
 * NOTES   : 1. Range : 0 ~ 61440 (in steps of 4096)
 *              -- XSTP_TYPE_MIN_BRIDGE_PRIORITY
 *              -- XSTP_TYPE_MAX_BRIDGE_PRIORITY_RSTP_MSTP
 *           2. DEFAULT : 32768
 *              -- XSTP_TYPE_DEFAULT_BRIDGE_PRIORITY
 * ----------------------------------------------------------------------------
 */
UI32_T XSTP_PMGR_SetMstPriority(UI32_T mstid, UI32_T priority)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_MGR_GET_MSG_SIZE(arg_grp1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_MGR_IPC_SETMSTPRIORITY;
    msg_p->data.arg_grp1.arg1 = mstid;
    msg_p->data.arg_grp1.arg2 = priority;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            XSTP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return XSTP_TYPE_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
} /* End of XSTP_PMGR_SetMstPriority */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_SetMstPortPriority
 * ----------------------------------------------------------------------------
 * PURPOSE : Set the port priority for the specified spanning tree
 *           when mode is MSTP.
 *
 * INPUT   : lport    -- lport number
 *           mstid    -- instance value
 *           priority -- the priority value
 *
 * OUTPUT  : None.
 *
 * RETURN  : XSTP_TYPE_RETURN_OK                -- set successfully
 *           XSTP_TYPE_RETURN_ERROR             -- failed
 *           XSTP_TYPE_RETURN_MASTER_MODE_ERROR -- not master mode
 *           XSTP_TYPE_RETURN_PORTNO_OOR        -- lport number out of range
 *           XSTP_TYPE_RETURN_INDEX_OOR         -- mstid and priority
 *                                                 out of range
 *
 * NOTES   : 1. Range : 0 ~ 240 (in steps of 16)
 *              -- XSTP_TYPE_MIN_PORT_PRIORITY
 *              -- XSTP_TYPE_MAX_PORT_PRIORITY_RSTP_MSTP
 *           2. Default : 128
 *              -- XSTP_TYPE_DEFAULT_PORT_PRIORITY
 * ----------------------------------------------------------------------------
 */
UI32_T XSTP_PMGR_SetMstPortPriority(UI32_T lport,
                                    UI32_T mstid,
                                    UI32_T priority)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_MGR_GET_MSG_SIZE(arg_grp2);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_MGR_IPC_SETMSTPORTPRIORITY;
    msg_p->data.arg_grp2.arg1 = lport;
    msg_p->data.arg_grp2.arg2 = mstid;
    msg_p->data.arg_grp2.arg3 = priority;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            XSTP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return XSTP_TYPE_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
} /* End of XSTP_PMGR_SetMstPriority */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_SetMstPortPathCost
 * ----------------------------------------------------------------------------
 * PURPOSE : Set the path_cost of the port for specified spanning tree
 *           when mode is MSTP.
 * INPUT   : lport     -- lport number
 *           mstid     -- instance value
 *           path_cost -- the path_cost value
 *
 * OUTPUT  : None.
 *
 * RETURN  : XSTP_TYPE_RETURN_OK                -- set successfully
 *           XSTP_TYPE_RETURN_ERROR             -- failed
 *           XSTP_TYPE_RETURN_MASTER_MODE_ERROR -- not master mode
 *           XSTP_TYPE_RETURNPORTNO_OOR         -- port number out of range
 *           XSTP_TYPE_RETURNINDEX_OOR          -- mstid and priority
 *                                                 out of range
 *
 * NOTES   : 1. In long mode (VAL_dot1dStpPathCostDefault_stp8021t2001)
 *              -- Range : 0 ~ 200000000
 *                 XSTP_TYPE_MIN_PORT_PATH_COST
 *                 XSTP_TYPE_MAX_PORT_PATH_COST_32
 *           2. In short mode (VAL_dot1dStpPathCostDefault_stp8021d1998)
 *              -- Range : 0 ~ 65535
 *                 XSTP_TYPE_MIN_PORT_PATH_COST
 *                 XSTP_TYPE_MAX_PORT_PATH_COST_16
 * ----------------------------------------------------------------------------
 */
UI32_T XSTP_PMGR_SetMstPortPathCost(UI32_T lport,
                                    UI32_T mstid,
                                    UI32_T path_cost)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_MGR_GET_MSG_SIZE(arg_grp2);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_MGR_IPC_SETMSTPORTPATHCOST;
    msg_p->data.arg_grp2.arg1 = lport;
    msg_p->data.arg_grp2.arg2 = mstid;
    msg_p->data.arg_grp2.arg3 = path_cost;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            XSTP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return XSTP_TYPE_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
} /* End of XSTP_PMGR_SetMstPortPathCost */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_AttachVlanListToMstConfigTable
 * ----------------------------------------------------------------------------
 * PURPOSE : Attach the vlan(s) to the new instance.
 *
 * INPUT   : mstid     -- instance value
 *           range     -- range value
 *                        XSTP_MSTP_SET_INSTANCE_VLANS_MAPPED_1K
 *                        XSTP_MSTP_SET_INSTANCE_VLANS_MAPPED_2K
 *                        XSTP_MSTP_SET_INSTANCE_VLANS_MAPPED_3K
 *                        XSTP_MSTP_SET_INSTANCE_VLANS_MAPPED_4K
 *           vlan_list -- pointer of vlan_list
 *
 * OUTPUT  : None.
 *
 * RETURN  : XSTP_TYPE_RETURN_OK                 -- set successfully
 *           XSTP_TYPE_RETURN_ERROR              -- failed
 *           XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 *           XSTP_TYPE_RETURN_ST_STATUS_DISABLED -- status is disabled
 *           XSTP_TYPE_RETURN_INDEX_OOR          -- mstid out of range
 *           XSTP_TYPE_RETURN_INDEX_NEX          -- vlan not existed
 *
 * NOTES   : 1. All vlans will join MST instance 0 by default.
 *           2. This API will automatically move this VLAN from old instance
 *              to new instance.
 * ----------------------------------------------------------------------------
 */
UI32_T XSTP_PMGR_AttachVlanListToMstConfigTable(UI32_T mstid,
                                                UI32_T range,
                                                UI8_T *vlan_list)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_MGR_GET_MSG_SIZE(arg_grp3);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_MGR_IPC_ATTACHVLANLISTTOMSTCONFIGTABLE;
    msg_p->data.arg_grp3.arg1 = mstid;
    msg_p->data.arg_grp3.arg2 = range;
    memcpy(msg_p->data.arg_grp3.arg3, vlan_list,
        sizeof(msg_p->data.arg_grp3.arg3));

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            XSTP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return XSTP_TYPE_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
} /* End of XSTP_PMGR_AttachVlanListToMstConfigTable */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_SetVlanToMstConfigTable
 * ----------------------------------------------------------------------------
 * PURPOSE : Map vlan to an instance for mst configuration table.
 *
 * INPUT   : mstid -- instance value
 *           vlan  -- vlan value
 *
 * OUTPUT  : None.
 *
 * RETURN  : XSTP_TYPE_RETURN_OK                 -- set successfully
 *           XSTP_TYPE_RETURN_ERROR              -- failed
 *           XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 *           XSTP_TYPE_RETURN_ST_STATUS_DISABLED -- status is disabled
 *           XSTP_TYPE_RETURN_INDEX_OOR          -- mstid out of range
 *           XSTP_TYPE_RETURN_INDEX_NEX          -- vlan not existed
 *
 * NOTES   : 1. All vlans will join MST instance 0 by default.
 *           2. This function is for the vlan which is attached to the
 *              MST instance 0.
 * ----------------------------------------------------------------------------
 */
UI32_T XSTP_PMGR_SetVlanToMstConfigTable(UI32_T mstid, UI32_T vlan)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_MGR_GET_MSG_SIZE(arg_grp1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_MGR_IPC_SETVLANTOMSTCONFIGTABLE;
    msg_p->data.arg_grp1.arg1 = mstid;
    msg_p->data.arg_grp1.arg2 = vlan;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            XSTP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return XSTP_TYPE_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
} /* End of XSTP_PMGR_SetVlanToMstConfigTable */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_AttachVlanToMstConfigTable
 * ----------------------------------------------------------------------------
 * PURPOSE : Attach the vlan to the new instance.
 *
 * INPUT   : mstid -- instance value
 *           vlan  -- vlan value
 *
 * OUTPUT  : None.
 *
 * RETURN  : XSTP_TYPE_RETURN_OK                 -- set successfully
 *           XSTP_TYPE_RETURN_ERROR              -- failed
 *           XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 *           XSTP_TYPE_RETURN_ST_STATUS_DISABLED -- status is disabled
 *           XSTP_TYPE_RETURN_INDEX_OOR          -- mstid out of range
 *           XSTP_TYPE_RETURN_INDEX_NEX          -- vlan not existed
 *
 * NOTES   : 1. All vlans will join MST instance 0 by default.
 *           2. This API will automatically move this VLAN from old instance
 *              to new instance.
 * ----------------------------------------------------------------------------
 */
UI32_T XSTP_PMGR_AttachVlanToMstConfigTable(UI32_T mstid, UI32_T vlan)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_MGR_GET_MSG_SIZE(arg_grp1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_MGR_IPC_ATTACHVLANTOMSTCONFIGTABLE;
    msg_p->data.arg_grp1.arg1 = mstid;
    msg_p->data.arg_grp1.arg2 = vlan;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            XSTP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return XSTP_TYPE_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
} /* End of XSTP_PMGR_AttachVlanToMstConfigTable */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_RemoveVlanFromMstConfigTable
 * ----------------------------------------------------------------------------
 * PURPOSE : Remove vlan from an instance for mst configuration table.
 *
 * INPUT   : mstid -- instance value
 *           vlan  -- vlan value
 *
 * OUTPUT  : None.
 *
 * RETURN  : XSTP_TYPE_RETURN_OK                -- set successfully
 *           XSTP_TYPE_RETURN_ERROR             -- failed
 *           XSTP_TYPE_RETURN_MASTER_MODE_ERROR -- not master mode
 *           XSTP_TYPE_RETURN_INDEX_OOR         -- mstid out of range
 *           XSTP_TYPE_RETURN_INDEX_NEX         -- vlan not existed
 *
 * NOTES   : None.
 * ----------------------------------------------------------------------------
 */
UI32_T XSTP_PMGR_RemoveVlanFromMstConfigTable(UI32_T mstid, UI32_T vlan)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_MGR_GET_MSG_SIZE(arg_grp1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_MGR_IPC_REMOVEVLANFROMMSTCONFIGTABLE;
    msg_p->data.arg_grp1.arg1 = mstid;
    msg_p->data.arg_grp1.arg2 = vlan;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            XSTP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return XSTP_TYPE_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
} /* End of XSTP_PMGR_RemoveVlanFromMstConfigTable */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_SetMstpConfigurationName
 * ----------------------------------------------------------------------------
 * PURPOSE : Set MSTP configurstion name.
 *
 * INPUT   : config_name -- pointer of the config_name
 *
 * OUTPUT  : None.
 *
 * RETURN  : XSTP_TYPE_RETURN_OK                -- set successfully
 *           XSTP_TYPE_RETURN_ERROR             -- failed
 *           XSTP_TYPE_RETURN_MASTER_MODE_ERROR -- not master mode
 *
 * NOTES   : Default : the bridage address
 * ----------------------------------------------------------------------------
 */
UI32_T XSTP_PMGR_SetMstpConfigurationName(UI8_T *config_name)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_MGR_GET_MSG_SIZE(arg_ar1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_MGR_IPC_SETMSTCONFIGNAME;
    memcpy(msg_p->data.arg_ar1, config_name, sizeof(msg_p->data.arg_ar1));

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            XSTP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return XSTP_TYPE_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
} /* End of XSTP_PMGR_SetMstpConfigurationName */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_SetMstpRevisionLevel
 * ----------------------------------------------------------------------------
 * PURPOSE : Set MSTP revision level value.
 *
 * INPUT   : revision -- revision value
 *
 * OUTPUT  : None.
 *
 * RETURN  : XSTP_TYPE_RETURN_OK                -- set successfully
 *           XSTP_TYPE_RETURN_MASTER_MODE_ERROR -- not master mode
 *           XSTP_TYPE_RETURN_ERROR             -- failed
 *
 * NOTES   : Default : 0
 *           -- XSTP_TYPE_DEFAULT_CONFIG_REVISION
 * ----------------------------------------------------------------------------
 */
UI32_T XSTP_PMGR_SetMstpRevisionLevel(UI32_T revision)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_MGR_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_MGR_IPC_SETMSTPREVISIONLEVEL;
    msg_p->data.arg_ui32 = revision;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            XSTP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return XSTP_TYPE_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
} /* End of XSTP_PMGR_SetMstpRevisionLevel */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_SetMstpMaxHop
 * ----------------------------------------------------------------------------
 * PURPOSE :  Set MSTP Max_Hop count.
 *
 * INPUT   :  hop_count -- max_hop value
 *
 * OUTPUT  :  None.
 *
 * RETURN  :  XSTP_TYPE_RETURN_OK                -- set successfully
 *            XSTP_TYPE_RETURN_ERROR             -- failed
 *            XSTP_TYPE_RETURN_MASTER_MODE_ERROR -- not master mode
 *            XSTP_TYPE_RETURN_INDEX_OOR         -- hop_count out of range
 *
 * NOTES   :  Range   : 1 ~ 40
 *            -- XSTP_TYPE_MSTP_MIN_MAXHOP
 *            -- XSTP_TYPE_MSTP_MAX_MAXHOP
 *            Default : 20
 *            -- XSTP_TYPE_DEFAULT_BRIDGE_MAXHOP
 * ----------------------------------------------------------------------------
 */
UI32_T XSTP_PMGR_SetMstpMaxHop(UI32_T hop_count)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_MGR_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_MGR_IPC_SETMSTPMAXHOP;
    msg_p->data.arg_ui32 = hop_count;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            XSTP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return XSTP_TYPE_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
} /* End of XSTP_PMGR_SetMstpMaxHop */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_RestartStateMachine
 *-----------------------------------------------------------------------------
 * PURPOSE : If restart_state_machine flag is TRUE, retart state machine.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETUEN  : None.
 *
 * NOTES   : Call the function when user leave the spa mst config mode.
 * ----------------------------------------------------------------------------
 */
void XSTP_PMGR_RestartStateMachine(void)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(XSTP_MGR_IPCMSG_TYPE_SIZE)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = XSTP_MGR_IPCMSG_TYPE_SIZE;

    msg_p = (XSTP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_MGR_IPC_RESTARTSTATEMACHINE;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            XSTP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return;
    }

    return;
} /* End of XSTP_PMGR_RestartStateMachine */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_SetMstPortAutoPathCost
 * ----------------------------------------------------------------------------
 * PURPOSE : Restore the default internal path_cost of the port for a instance.
 *
 * INPUT   : lport -- lport number
 *           mstid -- instance value
 *
 * OUTPUT  : None.
 *
 * RETURN  : XSTP_TYPE_RETURN_OK                -- set successfully
 *           XSTP_TYPE_RETURN_ERROR             -- failed
 *           XSTP_TYPE_RETURN_MASTER_MODE_ERROR -- not master mode
 *           XSTP_TYPE_RETURN_PORTNO_OOR        -- port number out of range
 *
 * NOTES   : internal_port_path_cost
 * ----------------------------------------------------------------------------
 */
UI32_T XSTP_PMGR_SetMstPortAutoPathCost(UI32_T lport, UI32_T mstid)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_MGR_GET_MSG_SIZE(arg_grp1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_MGR_IPC_SETMSTPORTAUTOPATHCOST;
    msg_p->data.arg_grp1.arg1 = lport;
    msg_p->data.arg_grp1.arg2 = mstid;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            XSTP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return XSTP_TYPE_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
} /* End of XSTP_PMGR_SetMstPortAutoPathCost */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_SetPortAdminPathCostAgent
 * ----------------------------------------------------------------------------
 * PURPOSE : Set the admin path_cost of the port.
 *
 * INPUT   : lport     -- lport number
 *           path_cost -- the path_cost value
 *
 * OUTPUT  : None.
 *
 * RETURN  : XSTP_TYPE_RETURN_OK                -- set successfully
 *           XSTP_TYPE_RETURN_ERROR             -- failed
 *           XSTP_TYPE_RETURN_MASTER_MODE_ERROR -- not master mode
 *           XSTP_TYPE_RETURN_PORTNO_OOR        -- port number out of range
 *
 * NOTES   : Writing a value of '0' assigns the automatically calculated
 *           default Path Cost value to the port.
 * ----------------------------------------------------------------------------
 */
UI32_T XSTP_PMGR_SetPortAdminPathCostAgent(UI32_T lport, UI32_T path_cost)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_MGR_GET_MSG_SIZE(arg_grp1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_MGR_IPC_SETPORTADMINPATHCOSTAGENT;
    msg_p->data.arg_grp1.arg1 = lport;
    msg_p->data.arg_grp1.arg2 = path_cost;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            XSTP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return XSTP_TYPE_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
} /* End of XSTP_PMGR_SetPortAdminPathCostAgent */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_SetPortAutoPathCost
 * ----------------------------------------------------------------------------
 * PURPOSE : Restore the default path_cost of the port.
 *
 * INPUT   : lport -- lport number
 *
 * OUTPUT  : None.
 *
 * RETURN  : XSTP_TYPE_RETURN_OK                -- set successfully
 *           XSTP_TYPE_RETURN_ERROR             -- failed
 *           XSTP_TYPE_RETURN_MASTER_MODE_ERROR -- not master mode
 *           XSTP_TYPE_RETURN_PORTNO_OOR        -- port number out of range
 *
 * NOTES   : external_port_path_cost
 * ----------------------------------------------------------------------------
 */
UI32_T XSTP_PMGR_SetPortAutoPathCost(UI32_T lport)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_MGR_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_MGR_IPC_SETPORTAUTOPATHCOST;
    msg_p->data.arg_ui32 = lport;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            XSTP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return XSTP_TYPE_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
} /* End of XSTP_PMGR_SetPortAutoPathCost */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_SetMstPortAdminPathCostAgent
 * ----------------------------------------------------------------------------
 * PURPOSE : Set the admin path_cost of the port for specified spanning tree
 *           when mode is MSTP.
 *
 * INPUT   : lport     -- lport number
 *           mstid     -- instance value
 *           path_cost -- the path_cost value
 *
 * OUTPUT  : None.
 *
 * RETURN  : XSTP_TYPE_RETURN_OK                -- set successfully
 *           XSTP_TYPE_RETURN_ERROR             -- failed
 *           XSTP_TYPE_RETURN_MASTER_MODE_ERROR -- not master mode
 *           XSTP_TYPE_RETURNPORTNO_OOR         -- port number out of range
 *           XSTP_TYPE_RETURNINDEX_OOR          -- mstid and priority
 *                                                 out of range
 *
 * NOTES   : Writing a value of '0' assigns the automatically calculated
             default Path Cost value to the port.
 * ----------------------------------------------------------------------------
 */
UI32_T XSTP_PMGR_SetMstPortAdminPathCostAgent(UI32_T lport,
                                              UI32_T mstid,
                                              UI32_T path_cost)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_MGR_GET_MSG_SIZE(arg_grp2);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_MGR_IPC_SETMSTPORTADMINPATHCOSTAGENT;
    msg_p->data.arg_grp2.arg1 = lport;
    msg_p->data.arg_grp2.arg2 = mstid;
    msg_p->data.arg_grp2.arg3 = path_cost;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            XSTP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return XSTP_TYPE_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
} /* End of XSTP_PMGR_SetMstPortAdminPathCostAgent */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_SetPortSpanningTreeStatus
 * ----------------------------------------------------------------------------
 * PURPOSE : Set the spanning tree status for the specified port.
 *
 * INPUT   : lport  -- lport number
 *           status -- the status value
 *
 * OUTPUT  : None.
 *
 * RETURN  : XSTP_TYPE_RETURN_OK                 -- set successfully
 *           XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 *           XSTP_TYPE_RETURN_ERROR              -- failed
 *           XSTP_TYPE_RETURN_ST_STATUS_DISABLED
 *
 * NOTES   : None.
 * ----------------------------------------------------------------------------
 */
UI32_T XSTP_PMGR_SetPortSpanningTreeStatus(UI32_T lport, UI32_T status)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_MGR_GET_MSG_SIZE(arg_grp1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_MGR_IPC_SETPORTSPANNINGTREESTATUS;
    msg_p->data.arg_grp1.arg1 = lport;
    msg_p->data.arg_grp1.arg2 = status;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            XSTP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return XSTP_TYPE_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
} /* End of XSTP_PMGR_SetPortAdminPathCostAgent */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_GetRunningMstPortPathCost
 * ----------------------------------------------------------------------------
 * PURPOSE : Get the path_cost of the port.
 *
 * INPUT   : lport -- lport number
 *           mstid -- instance value
 *
 * OUTPUT  : path_cost -- pointer of the path_cost value
 *
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL
 *
 * NOTES   : 1. This function shall only be invoked by CLI to save the
 *              "running configuration" to local or remote files.
 *           2. Since only non-default configuration will be saved, this
 *              function shall return non-default value.
 *
 * REF     : RFC-1493/dot1dStpPortEntry 5
 * ----------------------------------------------------------------------------
 */
UI32_T XSTP_PMGR_GetRunningMstPortPathCost(UI32_T lport,
                                           UI32_T mstid,
                                           UI32_T *path_cost)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_MGR_GET_MSG_SIZE(arg_grp2);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_MGR_IPC_GETRUNNINGMSTPORTPATHCOST;
    msg_p->data.arg_grp2.arg1 = lport;
    msg_p->data.arg_grp2.arg2 = mstid;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    *path_cost = msg_p->data.arg_grp2.arg3;

    return msg_p->type.ret_ui32;
} /* End of XSTP_PMGR_GetRunningMstPortPathCost */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_GetMstPortPathCost
 * ----------------------------------------------------------------------------
 * PURPOSE : Get the path_cost of the port.
 *
 * INPUT   : lport -- lport number
 *           mstid -- instance value
 *
 * OUTPUT  : path_cost -- pointer of the path_cost value
 *
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL
 *
 * NOTES   : TRUE/FALSE
 *
 * NOTES   : For SNMP.
 *
 * REF     : RFC-1493/dot1dStpPortEntry 5
 * ----------------------------------------------------------------------------
 */
BOOL_T XSTP_PMGR_GetMstPortPathCost(UI32_T lport,
                                    UI32_T mstid,
                                    UI32_T *path_cost)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_MGR_GET_MSG_SIZE(arg_grp2);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_MGR_IPC_GETMSTPORTPATHCOST;
    msg_p->data.arg_grp2.arg1 = lport;
    msg_p->data.arg_grp2.arg2 = mstid;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *path_cost = msg_p->data.arg_grp2.arg3;

    return msg_p->type.ret_bool;
} /* End of XSTP_PMGR_GetMstPortPathCost */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_GetLportDefaultPathCost
 * ----------------------------------------------------------------------------
 * PURPOSE : Get the specified lport path cost.
 *
 * INPUT   : lport -- the lport number
 *
 * OUTPUT  : path_cost -- pointer of the path cost value
 *
 * RETURN  : XSTP_TYPE_RETURN_OK    -- OK
 *           XSTP_TYPE_RETURN_ERROR -- failed
 *
 * NOTE    : This value is calculated from specification.
 *-----------------------------------------------------------------------------
 */
UI32_T XSTP_PMGR_GetLportDefaultPathCost(UI32_T lport, UI32_T *path_cost)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_MGR_GET_MSG_SIZE(arg_grp1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_MGR_IPC_GETLPORTDEFAULTPATHCOST;
    msg_p->data.arg_grp1.arg1 = lport;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return XSTP_TYPE_RETURN_ERROR;
    }

    *path_cost = msg_p->data.arg_grp1.arg2;

    return msg_p->type.ret_ui32;
} /* End of XSTP_PMGR_GetLportDefaultPathCost */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_GetDot1dMstEntry
 * ----------------------------------------------------------------------------
 * PURPOSE : This funtion returns true if the specified spanning tree entry
 *           info can be successfully retrieved. Otherwise, false is returned.
 *
 * INPUT   : mstid -- instance value
 *
 * OUTPUT  : entry -- the specified mst entry info
 *
 * RETURN  : TRUE/FALSE
 *
 * NOTES   : None.
 * ----------------------------------------------------------------------------
 */
BOOL_T XSTP_PMGR_GetDot1dMstEntry(UI32_T mstid,
                                  XSTP_MGR_Dot1dStpEntry_T *entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_MGR_GET_MSG_SIZE(arg_grp4);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_MGR_IPC_GETDOT1DMSTENTRY;
    msg_p->data.arg_grp4.arg1 = mstid;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *entry = msg_p->data.arg_grp4.arg2;

    return msg_p->type.ret_bool;
} /* End of XSTP_PMGR_GetDot1dMstEntry */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_GetNextDot1dMstEntry
 * ----------------------------------------------------------------------------
 * PURPOSE : This funtion returns true if the next spanning tree entry info
 *           can be successfully retrieved. Otherwise, false is returned.
 *
 * INPUT   : mstid -- instance value
 *
 * OUTPUT  : mstid -- instance value
 *           entry -- the specified mst entry info
 *
 * RETURN  : TRUE/FALSE
 *
 * NOTES   : For SNMP.
 * ----------------------------------------------------------------------------
 */
BOOL_T XSTP_PMGR_GetNextDot1dMstEntry(UI32_T *mstid,
                                      XSTP_MGR_Dot1dStpEntry_T *entry)
{
    const UI32_T msg_size = XSTP_MGR_GET_MSG_SIZE(arg_grp4);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_MGR_IPC_GETNEXTDOT1DMSTENTRY;
    msg_p->data.arg_grp4.arg1 = *mstid;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *mstid = msg_p->data.arg_grp4.arg1;
    *entry = msg_p->data.arg_grp4.arg2;

    return msg_p->type.ret_bool;
} /* End of XSTP_PMGR_GetNextDot1dMstEntry */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_GetDot1dBaseEntry
 * ----------------------------------------------------------------------------
 * PURPOSE : This funtion returns true if the specified base entry
 *           info can be successfully retrieved. Otherwise, false is
 *           returned.
 *
 * INPUT   : None.
 *
 * OUTPUT  : base_entry -- base entry info
 *
 * RETURN  : TRUE/FALSE
 *
 * NOTES   : None.
 * ----------------------------------------------------------------------------
 */
BOOL_T XSTP_PMGR_GetDot1dBaseEntry(XSTP_MGR_Dot1dBaseEntry_T *base_entry)
{
    const UI32_T msg_size = XSTP_MGR_GET_MSG_SIZE(arg_base_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_MGR_IPC_GETDOT1DBASEENTRY;
    msg_p->data.arg_base_entry = *base_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *base_entry = msg_p->data.arg_base_entry;

    return msg_p->type.ret_bool;
} /* End of XSTP_PMGR_GetDot1dBaseEntry */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_GetDot1dBasePortEntry
 * ----------------------------------------------------------------------------
 * PURPOSE : This funtion returns true if the specified base port entry
 *           info can be successfully retrieved. Otherwise, false is
 *           returned.
 *
 * INPUT   : base_port_entry->dot1d_base_port
 *                                       -- key to specify a unique base
 *                                          entry
 *
 * OUTPUT  : base_port_entry             -- base entry info of specified
 *                                          key
 *
 * RETURN  : TRUE/FALSE
 *
 * NOTES   : None.
 * ----------------------------------------------------------------------------
 */
BOOL_T XSTP_PMGR_GetDot1dBasePortEntry(XSTP_MGR_Dot1dBasePortEntry_T *base_port_entry)
{
    const UI32_T msg_size = XSTP_MGR_GET_MSG_SIZE(arg_base_port_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_MGR_IPC_GETDOT1DBASEPORTENTRY;
    msg_p->data.arg_base_port_entry = *base_port_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *base_port_entry = msg_p->data.arg_base_port_entry;

    return msg_p->type.ret_bool;
} /* End of XSTP_PMGR_GetDot1dBasePortEntry */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME -- XSTP_PMGR_GetNextDot1dBasePortEntry
 * ----------------------------------------------------------------------------
 * PURPOSE : This funtion returns true if the next available base port
 *           entry info can be successfully retrieved. Otherwise, false
 *           is returned.
 *
 * INPUT   : base_port_entry->dot1d_base_port
 *                                       -- key to specify a unique base
 *                                          entry
 *
 * OUTPUT  : base_port_entry             -- xstp entry info of specified
 *                                          key
 *
 * RETURN  : TRUE/FALSE
 *
 * NOTES   : If next available stp entry is available, the
 *           base_port_entry->dot1d_base_port will be updated and the
 *           entry info will be retrieved from the table.
 * ----------------------------------------------------------------------------
 */
BOOL_T XSTP_PMGR_GetNextDot1dBasePortEntry(XSTP_MGR_Dot1dBasePortEntry_T *base_port_entry)
{
    const UI32_T msg_size = XSTP_MGR_GET_MSG_SIZE(arg_base_port_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_MGR_IPC_GETNEXTDOT1DBASEPORTENTRY;
    msg_p->data.arg_base_port_entry = *base_port_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *base_port_entry = msg_p->data.arg_base_port_entry;

    return msg_p->type.ret_bool;
} /* End of XSTP_PMGR_GetNextDot1dBasePortEntry */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_GetRunningMstpConfigurationName
 * ----------------------------------------------------------------------------
 * PURPOSE : Get the MSTP configurstion name.
 *
 * INPUT   : None.
 *
 * OUTPUT  : config_name -- pointer of the config_name
 *
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL
 *
 * NOTES   : 1. This function shall only be invoked by CLI to save the
 *              "running configuration" to local or remote files.
 *           2. Since only non-default configuration will be saved, this
 *              function shall return non-default value.
 * ----------------------------------------------------------------------------
 */
UI32_T XSTP_PMGR_GetRunningMstpConfigurationName(UI8_T *config_name)
{
    const UI32_T msg_size = XSTP_MGR_GET_MSG_SIZE(arg_ar1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = XSTP_MGR_IPCMSG_TYPE_SIZE;

    msg_p = (XSTP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_MGR_IPC_GETRUNNINGMSTPCONFIGURATIONNAME;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    memcpy(config_name, msg_p->data.arg_ar1, sizeof(msg_p->data.arg_ar1));

    return msg_p->type.ret_ui32;
} /* End of XSTP_PMGR_GetRunningMstpConfigurationName */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_GetMstpConfigurationName
 * ----------------------------------------------------------------------------
 * PURPOSE : Get the MSTP configurstion name.
 *
 * INPUT   : None.
 *
 * OUTPUT  : config_name -- pointer of the config_name
 *
 * RETURN  : TRUE/FALSE
 *
 * NOTES   : For SNMP.
 * ----------------------------------------------------------------------------
 */
BOOL_T XSTP_PMGR_GetMstpConfigurationName(UI8_T *config_name)
{
    const UI32_T msg_size = XSTP_MGR_GET_MSG_SIZE(arg_ar1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = XSTP_MGR_IPCMSG_TYPE_SIZE;

    msg_p = (XSTP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_MGR_IPC_GETMSTPCONFIGURATIONNAME;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    memcpy(config_name, msg_p->data.arg_ar1, sizeof(msg_p->data.arg_ar1));

    return msg_p->type.ret_bool;
} /* End of XSTP_PMGR_GetMstpConfigurationName */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_VlanIsMstMember
 * ----------------------------------------------------------------------------
 * PURPOSE : This funcion returns true if the vlan is in the
 *           specified spanning tree. Otherwise, returns false.
 *
 * INPUT   : mstid -- the instance id
 *           vlan  -- the vlan value
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE/FALSE
 *
 * NOTES   : None.
 * ----------------------------------------------------------------------------
 */
BOOL_T XSTP_PMGR_VlanIsMstMember(UI32_T mstid, UI32_T vlan)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_MGR_GET_MSG_SIZE(arg_grp1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_MGR_IPC_VLANISMSTMEMBER;
    msg_p->data.arg_grp1.arg1 = mstid;
    msg_p->data.arg_grp1.arg2 = vlan;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            XSTP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
} /* End of XSTP_PMGR_VlanIsMstMember */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_GetPortStateByVlan
 * ----------------------------------------------------------------------------
 * PURPOSE : Get the port state in a specified vlan.
 *
 * INPUT   : vid   -- vlan id
 *           lport -- lport number
 *
 * OUTPUT  : state -- the pointer of state value
 *                    XSTP_TYPE_PORT_STATE_DISCARDING
 *                    XSTP_TYPE_PORT_STATE_LEARNING
 *                    XSTP_TYPE_PORT_STATE_FORWARDING
 *
 * RETURN  : TRUE/FALSE
 *
 * NOTES   : None.
 * ----------------------------------------------------------------------------
 */
BOOL_T XSTP_PMGR_GetPortStateByVlan(UI32_T vid,
                                    UI32_T lport,
                                    UI32_T *state)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_MGR_GET_MSG_SIZE(arg_grp2);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_MGR_IPC_GETPORTSTATEBYVLAN;
    msg_p->data.arg_grp2.arg1 = vid;
    msg_p->data.arg_grp2.arg2 = lport;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *state = msg_p->data.arg_grp2.arg3;

    return msg_p->type.ret_bool;
} /* End of XSTP_PMGR_GetPortStateByVlan */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_GetMappedInstanceByVlan
 * ----------------------------------------------------------------------------
 * PURPOSE : Get mapped instance by a specified vlan
 *
 * INPUT   : vid   -- vlan id
 *           mstid -- instance value pointer
 *
 * OUTPUT  : mstid -- instance value pointer
 *
 * RETURN  : TRUE if OK, or FALSE if at the end of the port list
 *
 * NOTE    : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T  XSTP_PMGR_GetMappedInstanceByVlan(UI32_T vid, UI32_T *mstid)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_MGR_GET_MSG_SIZE(arg_grp1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_MGR_IPC_GETMAPPEDINSTANCEBYVLAN;
    msg_p->data.arg_grp1.arg1 = vid;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *mstid = msg_p->data.arg_grp1.arg2;

    return msg_p->type.ret_bool;
} /* End of XSTP_PMGR_GetMappedInstanceByVlan */

#ifdef  XSTP_TYPE_PROTOCOL_MSTP

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_GetRunningPortPathCost
 * ----------------------------------------------------------------------------
 * PURPOSE : Get the external path_cost of the port for CIST.
 *
 * INPUT   : lport         -- lport number
 *
 * OUTPUT  : ext_path_cost -- pointer of the path_cost value
 *
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL
 *
 * NOTES   : 1. This function shall only be invoked by CLI to save the
 *              "running configuration" to local or remote files.
 *           2. Since only non-default configuration will be saved, this
 *              function shall return non-default value.
 *
 * REF     : RFC-1493/dot1dStpPortEntry 5
 * ----------------------------------------------------------------------------
 */
UI32_T XSTP_PMGR_GetRunningPortPathCost(UI32_T lport, UI32_T *ext_path_cost)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_MGR_GET_MSG_SIZE(arg_grp1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_MGR_IPC_GETRUNNINGPORTPATHCOST;
    msg_p->data.arg_grp1.arg1 = lport;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    *ext_path_cost = msg_p->data.arg_grp1.arg2;

    return msg_p->type.ret_ui32;
} /* End of XSTP_PMGR_GetRunningPortPathCost */

#endif /* XSTP_TYPE_PROTOCOL_MSTP */

#if (SYS_CPNT_STP_ROOT_GUARD == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_SetPortRootGuardStatus
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set root guard status for the specified port.
 * INPUT    :   UI32_T lport            -- lport number
 *              UI32_T status           -- the status value
 *                                         XSTP_TYPE_PORT_ROOT_GUARD_ENABLED
 *                                         XSTP_TYPE_PORT_ROOT_GUARD_DISABLED
 * OUTPUT   :   None
 * RETURN   :   XSTP_TYPE_RETURN_OK
 *              XSTP_TYPE_RETURN_ERROR
 * NOTE     :   Only designated port can be enabled root guard function now.
 * ------------------------------------------------------------------------
 */
UI32_T  XSTP_PMGR_SetPortRootGuardStatus(UI32_T lport, UI32_T status)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_MGR_GET_MSG_SIZE(arg_grp1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_MGR_IPC_SETPORTROOTGUARDSTATUS;
    msg_p->data.arg_grp1.arg1 = lport;
    msg_p->data.arg_grp1.arg2 = status;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            XSTP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return XSTP_TYPE_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
} /* End of XSTP_PMGR_SetPortRootGuardStatus */
#endif /* #if (SYS_CPNT_STP_ROOT_GUARD == TRUE) */

#if (SYS_CPNT_STP_BPDU_GUARD == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_SetPortBpduGuardStatus
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the BPDU guard status for the specified port.
 * INPUT    :   UI32_T lport            -- lport number
 *              UI32_T status           -- the status value
 * OUTPUT   :   None
 * RETURN   :   XSTP_TYPE_RETURN_OK
 *              XSTP_TYPE_RETURN_ERROR
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_PMGR_SetPortBpduGuardStatus(UI32_T lport, UI32_T status)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_MGR_GET_MSG_SIZE(arg_grp1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_MGR_IPC_SETPORTBPDUGUARDSTATUS;
    msg_p->data.arg_grp1.arg1 = lport;
    msg_p->data.arg_grp1.arg2 = status;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            XSTP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return XSTP_TYPE_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
} /* End of XSTP_PMGR_SetPortBpduGuardStatus */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_SetPortBPDUGuardAutoRecovery
 * ------------------------------------------------------------------------
 * PURPOSE :  Set BPDU guard auto recovery status for the specified port.
 * INPUT   :  lport  -- the logical port number
 *            status -- the status value
 * OUTPUT  :  None
 * RETURN  :  XSTP_TYPE_RETURN_CODE_E
 * NOTE    :  None
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_PMGR_SetPortBPDUGuardAutoRecovery(UI32_T lport, UI32_T status)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_MGR_GET_MSG_SIZE(arg_grp1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_MGR_IPC_SETPORTBPDUGUARDAUTORECOVERY;
    msg_p->data.arg_grp1.arg1 = lport;
    msg_p->data.arg_grp1.arg2 = status;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            XSTP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return XSTP_TYPE_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
} /* End of XSTP_PMGR_SetPortBPDUGuardAutoRecovery */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_SetPortBPDUGuardAutoRecoveryInterval
 * ------------------------------------------------------------------------
 * PURPOSE :  Set BPDU guard auto recovery status for the specified port.
 * INPUT   :  lport    -- the logical port number
 *            interval -- the interval value
 * OUTPUT  :  None
 * RETURN  :  XSTP_TYPE_RETURN_CODE_E
 * NOTE    :  None
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_PMGR_SetPortBPDUGuardAutoRecoveryInterval(UI32_T lport, UI32_T interval)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_MGR_GET_MSG_SIZE(arg_grp1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_MGR_IPC_SETPORTBPDUGUARDAUTORECOVERYINTERVAL;
    msg_p->data.arg_grp1.arg1 = lport;
    msg_p->data.arg_grp1.arg2 = interval;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            XSTP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return XSTP_TYPE_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
} /* End of XSTP_PMGR_SetPortBPDUGuardAutoRecoveryInterval */
#endif /* #if (SYS_CPNT_STP_BPDU_GUARD == TRUE) */

#if (SYS_CPNT_STP_BPDU_FILTER == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_SetPortBpduFilterStatus
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the BPDU filter status for the specified port.
 * INPUT    :   UI32_T lport            -- lport number
 *              UI32_T status           -- the status value
 * OUTPUT   :   None
 * RETURN   :   XSTP_TYPE_RETURN_OK
 *              XSTP_TYPE_RETURN_ERROR
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_PMGR_SetPortBpduFilterStatus(UI32_T lport, UI32_T status)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_MGR_GET_MSG_SIZE(arg_grp1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_MGR_IPC_SETPORTBPDUFILTERSTATUS;
    msg_p->data.arg_grp1.arg1 = lport;
    msg_p->data.arg_grp1.arg2 = status;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            XSTP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return XSTP_TYPE_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
} /* End of XSTP_PMGR_SetPortBpduFilterStatus */
#endif /* #if (SYS_CPNT_STP_BPDU_FILTER == TRUE) */

#if (SYS_CPNT_STP_COMPATIBLE_WITH_CISCO_PRESTANDARD == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_SetCiscoPrestandardCompatibility
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the cisco prestandard compatibility status
 * INPUT    :   status    -- the status value
 * OUTPUT   :   None
 * RETURN   :   XSTP_TYPE_RETURN_OK                 -- set successfully
 *              XSTP_TYPE_RETURN_ERROR              -- failed
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_PMGR_SetCiscoPrestandardCompatibility(UI32_T status)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_MGR_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_MGR_IPC_SETCISCOPRESTANDARDCOMPATIBILITY;
    msg_p->data.arg_ui32 = status;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            XSTP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return XSTP_TYPE_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
} /* End of XSTP_PMGR_SetCiscoPrestandardCompatibility */
#endif /* End of #if (SYS_CPNT_STP_COMPATIBLE_WITH_CISCO_PRESTANDARD == TRUE) */


#if (SYS_CPNT_XSTP_TC_PROP_STOP == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_SetPortTcPropStop
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the port don't propagate TC
 * INPUT    :   UI32_T lport            -- lport number
 *              BOOL_T status           -- the status value
 * OUTPUT   :   None
 * RETURN   :   XSTP_TYPE_RETURN_OK
 *              XSTP_TYPE_RETURN_ERROR
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_PMGR_SetPortTcPropStop(UI32_T lport, BOOL_T status)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = XSTP_MGR_GET_MSG_SIZE(arg_grp1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    XSTP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_XSTP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (XSTP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = XSTP_MGR_IPC_SETPORTTCPROPSTOP;
    msg_p->data.arg_grp1.arg1 = lport;
    msg_p->data.arg_grp1.arg2 = status;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            XSTP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return XSTP_TYPE_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
} /* End of XSTP_PMGR_SetPortFlooding */
#endif
#if (SYS_CPNT_EAPS == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_SetEthRingPortRole
 * ------------------------------------------------------------------------
 * PURPOSE  :   To set the port role for eth ring protocol.
 * INPUT    :   lport     -- lport number (1-based)
 *              port_role -- port role to set
 * OUTPUT   :   None
 * RETURN   :   XSTP_TYPE_RETURN_OK                 -- set successfully
 *              XSTP_TYPE_RETURN_ERROR              -- failed
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_PMGR_SetEthRingPortRole(
    UI32_T  lport,
    UI32_T  port_role)
{
    XSTP_PMGR_FUNC_BEGIN(
        XSTP_MGR_GET_MSG_SIZE(arg_grp1),
        XSTP_MGR_IPCMSG_TYPE_SIZE,
        XSTP_MGR_IPC_SETETHRINGPORTROLE);

    {
        BOOL_T  ret = FALSE;

        msg_p->data.arg_grp1.arg1 = lport;
        msg_p->data.arg_grp1.arg2 = port_role;

        if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                rep_size, msgbuf_p) == SYSFUN_OK)
        {
            ret = msg_p->type.ret_bool;
        }

        return ret;
    }
}

#endif /* End of #if (SYS_CPNT_EAPS == TRUE) */

#if(SYS_CPNT_XSTP_TC_PROP_GROUP == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_SetTcPropGroupPortList
 * ------------------------------------------------------------------------
 * PURPOSE  :   To add/remove the ports to/from a group.
 * INPUT    :   is_add       -- add or remove
 *              group_id     -- group ID
 *              portbitmap   -- ports
 * OUTPUT   :   None
 * RETURN   :   XSTP_TYPE_RETURN_OK                 -- set successfully
 *              XSTP_TYPE_RETURN_ERROR              -- failed
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_PMGR_SetTcPropGroupPortList(BOOL_T is_add,
                                UI32_T group_id,
                                UI8_T portbitmap[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST])

{
    XSTP_PMGR_FUNC_BEGIN(
        XSTP_MGR_GET_MSG_SIZE(arg_grp5),
        XSTP_MGR_IPCMSG_TYPE_SIZE,
        XSTP_MGR_IPC_SETTCPROPGROUPPORTLIST);

    msg_p->data.arg_grp5.arg1 = is_add;
    msg_p->data.arg_grp5.arg2 = group_id;
    memcpy(msg_p->data.arg_grp5.arg3, portbitmap,
        sizeof(msg_p->data.arg_grp5.arg3));

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            XSTP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return XSTP_TYPE_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_DelTcPropGroup
 * ------------------------------------------------------------------------
 * PURPOSE  :   To delete a group.
 * INPUT    :   group_id     -- group ID
 * OUTPUT   :   None
 * RETURN   :   XSTP_TYPE_RETURN_OK                 -- set successfully
 *              XSTP_TYPE_RETURN_ERROR              -- failed
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_PMGR_DelTcPropGroup(UI32_T group_id)
{
    XSTP_PMGR_FUNC_BEGIN(
        XSTP_MGR_GET_MSG_SIZE(arg_ui32),
        XSTP_MGR_IPCMSG_TYPE_SIZE,
        XSTP_MGR_IPC_DELTCPROPGROUP);

    msg_p->data.arg_ui32 = group_id;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            XSTP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return XSTP_TYPE_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
}
#endif /*#if(SYS_CPNT_XSTP_TC_PROP_GROUP == TRUE)*/

/* LOCAL SUBPROGRAM BODIES
 */
