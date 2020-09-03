/*-----------------------------------------------------------------------------
 * FILE NAME: LLDP_PMGR.C
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *    This file implements the APIs for LLDP MGR IPC.
 *
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    2007/07/07     --- Timon, Create
 *
 * Copyright(C)      Accton Corporation, 2007
 *-----------------------------------------------------------------------------
 */


/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>   /*to remove warning, because the memcpy function can't be recognized,so here we add string.h and stdio.h head files*/
#include <string.h>  /*to remove warning, because the memcpy function can't be recognized,so here we add string.h and stdio.h head files*/
#include "sys_bld.h"
#include "sys_module.h"
#include "sys_type.h"
#include "sysfun.h"
#include "lldp_mgr.h"
#include "lldp_pmgr.h"
#include "lldp_type.h"


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
 * ROUTINE NAME : LLDP_PMGR_InitiateProcessResources
 *-----------------------------------------------------------------------------
 * PURPOSE : Initiate resources for LLDP_PMGR in the calling process.
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
BOOL_T LLDP_PMGR_InitiateProcessResources(void)
{
    /* get the handle of ipc message queues for LLDP MGR
     */
    if (SYSFUN_GetMsgQ(SYS_BLD_GVRP_GROUP_IPCMSGQ_KEY,
            SYSFUN_MSGQ_BIDIRECTIONAL, &ipcmsgq_handle) != SYSFUN_OK)
    {
        printf("\n%s(): SYSFUN_GetMsgQ failed.\n", __FUNCTION__);
        return FALSE;
    }

    return TRUE;
} /* End of LLDP_PMGR_InitiateProcessResources */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_SetSysAdminStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : Set LLDP global admin status
 * INPUT    : admin_status
 * OUTPUT   : None
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_PMGR_SetSysAdminStatus(UI32_T admin_status)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_SETSYSADMINSTATUS;
    msg_p->data.arg_ui32 = admin_status;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            LLDP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
} /* End of LLDP_PMGR_SetSysAdminStatus */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_SetMsgTxInterval
 *-------------------------------------------------------------------------
 * PURPOSE  : Set msg_tx_interval to determine interval at which LLDP frames are transmitted
 * INPUT    : UI32_T msg_tx_interval    --  time value
 * OUTPUT   : None
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 10.5.3.3, IEEE Std 802.1AB-2005.
 *            Range: 5~32768
 *            Default value: 30 seconds.
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_PMGR_SetMsgTxInterval(UI32_T msg_tx_interval)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_SETMSGTXINTERVAL;
    msg_p->data.arg_ui32 = msg_tx_interval;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            LLDP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
} /* End of LLDP_PMGR_SetMsgTxInterval */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_SetMsgTxHoldMul
 *-------------------------------------------------------------------------
 * PURPOSE  : Set msg_hold time multiplier to determine the actual TTL value used in an LLDPDU.
 * INPUT    : UI32_T msg_tx_hold_multiplier      --  a multiplier on the msgTxInterval
 * OUTPUT   : None
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 10.5.3.3, IEEE Std 802.1AB-2005.
 *            Range: 2~10
 *            Default value: 4.
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_PMGR_SetMsgTxHoldMul(UI32_T msg_tx_hold_mul)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_SETMSGTXHOLDMUL;
    msg_p->data.arg_ui32 = msg_tx_hold_mul;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            LLDP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
} /* End of LLDP_PMGR_SetMsgTxHoldMul */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_SetReinitDelay
 *-------------------------------------------------------------------------
 * PURPOSE  : Set the amount of delay from when adminStatus becomes ¡§disabled¡¦
 *            until re-initialization will be attempted.
 * INPUT    : UI32_T reinit_delay    --  time value
 * OUTPUT   : None
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 10.5.3.3, IEEE Std 802.1AB-2005.
 *            Range: 1~10
 *            Default value: 2 seconds.
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_PMGR_SetReinitDelay(UI32_T reinit_delay)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_SETREINITDELAY;
    msg_p->data.arg_ui32 = reinit_delay;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            LLDP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
} /* End of LLDP_PMGR_SetReinitDelay */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_SetTxDelay
 *-------------------------------------------------------------------------
 * PURPOSE  : Set the minimum delay between successive LLDP frame transmissions.
 * INPUT    : UI32_T tx_delay    --  time value
 * OUTPUT   : None
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 10.5.3.3, IEEE Std 802.1AB-2005.
 *            Range: 1~8192
 *            Default value: 2 seconds.
 *            The recommended value is set by the following formula:
 *              1 <= lldpTxDelay <= (0.25 * lldpMessageTxInterval)
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_PMGR_SetTxDelay(UI32_T tx_delay)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_SETTXDELAY;
    msg_p->data.arg_ui32 = tx_delay;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            LLDP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
} /* End of LLDP_PMGR_SetTxDelay */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_SetNotificationInterval
 *-------------------------------------------------------------------------
 * PURPOSE  : Set this value to control the transmission of LLDP notifications.
 * INPUT    : UI32_T interval_time    --  time value
 * OUTPUT   : None
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 12.2, IEEE Std 802.1AB-2005.
 *            Range: 5~3600
 *            Default value: 5 seconds.
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_PMGR_SetNotificationInterval(UI32_T notify_interval_time)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_SETNOTIFICATIONINTERVAL;
    msg_p->data.arg_ui32 = notify_interval_time;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            LLDP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
} /* End of LLDP_PMGR_SetNotificationInterval */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_SetPortConfigAdminStatus
 *-------------------------------------------------------------------------
 * PURPOSE  :  Set admin_status to control whether or not a local LLDP agent is
 *             enabled(transmit and receive, transmit only, or receive only)
 *             or is disabled.
 * INPUT    : UI32_T lport            -- lport number
 *            UI32_T admin_status     -- status vaule:
 *                                       txOnly(1),
 *                                       rxOnly(2),
 *                                       txAndRx(3),
 *                                       disabled(4)
 * OUTPUT   : None
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 10.5.1, IEEE Std 802.1AB-2005.
 *            Default value: txAndRx(3).
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_PMGR_SetPortConfigAdminStatus(UI32_T lport, UI32_T admin_status)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_grp_ui32_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_SETPORTCONFIGADMINSTATUS;
    msg_p->data.arg_grp_ui32_ui32.arg_ui32_1 = lport;
    msg_p->data.arg_grp_ui32_ui32.arg_ui32_2 = admin_status;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            LLDP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
} /* End of LLDP_PMGR_SetPortConfigAdminStatus */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_SetPortAdminDisable
 *-------------------------------------------------------------------------
 * PURPOSE  :  port is admin shutdown
 * INPUT    : UI32_T lport            -- lport number
 *            UI32_T admin_status     -- status vaule:
 *                                       txOnly(1),
 *                                       rxOnly(2),
 *                                       txAndRx(3),
 *                                       disabled(4)
 * OUTPUT   : None
 * RETURN   : when port shutdown it need call this first so that lldp can send out packet before shutdown
 *-------------------------------------------------------------------------
 */
void LLDP_PMGR_SetPortAdminDisable(UI32_T lport)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_SETPORTADMINDISABLE;
    msg_p->data.arg_ui32 = lport;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            LLDP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return;
    }

    return ;
} /* End of LLDP_PMGR_SetPortConfigAdminStatus */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_SetPortConfigNotificationEnable
 *-------------------------------------------------------------------------
 * PURPOSE  : The value true(1) means that notifications are enabled;
 *            The value false(2) means that they are not.
 * INPUT    : UI32_T lport            -- lport number
 *            UI32_T status           -- status vaule:
 *                                       true(1),
 *                                       false(2),
 * OUTPUT   : None
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 12.2, IEEE Std 802.1AB-2005.
 *            Default value: false(2).
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_PMGR_SetPortConfigNotificationEnable(UI32_T lport, BOOL_T status)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_grp_ui32_bool);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_SETPORTCONFIGNOTIFICATIONENABLE;
    msg_p->data.arg_grp_ui32_bool.arg_ui32 = lport;
    msg_p->data.arg_grp_ui32_bool.arg_bool = status;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            LLDP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
} /* End of LLDP_PMGR_SetPortConfigNotificationEnable */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_SetPortOptionalTlvStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : Set optional TLVs status that may be included in LLDPDU.
 * INPUT    : UI32_T lport            -- lport number
 *            UI8_T tlv_status        -- bitmap:
 *                                       BIT_0: Port Description TLV,
 *                                       BIT_1: System Name TLV,
 *                                       BIT_2: System Description TLV,
 *                                       BIT_3: System Capabilities TLV,
 * OUTPUT   : None
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 10.2.1.1, IEEE Std 802.1AB-2005.
 *            Default value: no bit on (empty set).
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_PMGR_SetPortOptionalTlvStatus(UI32_T port, UI8_T tlv_status)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_grp_ui32_ui8);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_SETPORTOPTIONALTLVSTATUS;
    msg_p->data.arg_grp_ui32_ui8.arg_ui32 = port;
    msg_p->data.arg_grp_ui32_ui8.arg_ui8 = tlv_status;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            LLDP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
} /* End of LLDP_PMGR_SetPortOptionalTlvStatus */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_SetConfigManAddrTlv
 *-------------------------------------------------------------------------
 * PURPOSE  : Set optional TLVs status that may be included in LLDPDU.
 * INPUT    : lport, transfer_enable
 * OUTPUT   : None
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_PMGR_SetConfigManAddrTlv(UI32_T lport, BOOL_T transfer_enable)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_grp_ui32_bool);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_SETCONFIGMANADDRTLV;
    msg_p->data.arg_grp_ui32_bool.arg_ui32 = lport;
    msg_p->data.arg_grp_ui32_bool.arg_bool = transfer_enable;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            LLDP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
} /* End of LLDP_PMGR_SetConfigManAddrTlv */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_SetConfigAllPortManAddrTlv
 *-------------------------------------------------------------------------
 * PURPOSE  : Set optional TLVs status that may be included in LLDPDU.
 * INPUT    : lport, transfer_enable
 * OUTPUT   : None
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_PMGR_SetConfigAllPortManAddrTlv(UI8_T *lport)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_grp_ui8);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_SETCONFIGALLPORTMANADDRTLV;
    memcpy(msg_p->data.arg_grp_ui8.arg_ui8,lport,sizeof(msg_p->data.arg_grp_ui8.arg_ui8));
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            LLDP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
}
/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetPortConfigEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get configuration entry info. for specified port.
 * INPUT    : LLDP_MGR_PortConfigEntry_T  *port_config_entry
 * OUTPUT   : LLDP_MGR_PortConfigEntry_T  *port_config_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 12.2, IEEE Std 802.1AB-2005.
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetPortConfigEntry(LLDP_MGR_PortConfigEntry_T *port_config_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_port_config_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_GETPORTCONFIGENTRY;
    msg_p->data.arg_port_config_entry = *port_config_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    *port_config_entry = msg_p->data.arg_port_config_entry;

    return msg_p->type.ret_ui32;
} /* End of LLDP_PMGR_GetPortConfigEntry */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetNextPortConfigEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This funtion returns true if the next configuration entry info.
 *            can be successfully retrieved. Otherwise, false.
 * INPUT    : LLDP_MGR_PortConfigEntry_T  *port_config_entry
 * OUTPUT   : LLDP_MGR_PortConfigEntry_T  *port_config_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 12.2, IEEE Std 802.1AB-2005.
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetNextPortConfigEntry(LLDP_MGR_PortConfigEntry_T *port_config_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_port_config_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_GETNEXTPORTCONFIGENTRY;
    msg_p->data.arg_port_config_entry = *port_config_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    *port_config_entry = msg_p->data.arg_port_config_entry;

    return msg_p->type.ret_ui32;
} /* End of LLDP_PMGR_GetNextPortConfigEntry */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetConfigManAddrTlv
 *-------------------------------------------------------------------------
 * PURPOSE  : This funtion returns true if the config man addr entry info.
 *            can be successfully retrieved. Otherwise, false.
 * INPUT    : None
 * OUTPUT   : enable
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 12.2, IEEE Std 802.1AB-2005.
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetConfigManAddrTlv(UI32_T lport, UI8_T *enabled)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_grp_ui32_ui8);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_GETCONFIGMANADDRTLV;
    msg_p->data.arg_grp_ui32_ui8.arg_ui32 = lport;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    *enabled = msg_p->data.arg_grp_ui32_ui8.arg_ui8;

    return msg_p->type.ret_ui32;
} /* End of LLDP_PMGR_GetConfigManAddrTlv */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetSysStatisticsEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get statistics info.
 * INPUT    : LLDP_MGR_Statistics_T *statistics_entry
 * OUTPUT   : LLDP_MGR_Statistics_T *statistics_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 12.2, IEEE Std 802.1AB-2005.
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetSysStatisticsEntry(LLDP_MGR_Statistics_T *statistics_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_statistics);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = LLDP_MGR_IPCMSG_TYPE_SIZE;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_GETSYSSTATISTICSENTRY;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    *statistics_entry = msg_p->data.arg_statistics;

    return msg_p->type.ret_ui32;
} /* End of LLDP_PMGR_GetSysStatisticsEntry */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetPortTxStatisticsEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get tx statistics info. for specified port.
 * INPUT    : LLDP_MGR_PortTxStatistics_T *port_statistics_entry
 * OUTPUT   : LLDP_MGR_PortTxStatistics_T *port_statistics_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 10.5.2.1, IEEE Std 802.1AB-2005.
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetPortTxStatisticsEntry(LLDP_MGR_PortTxStatistics_T *port_statistics_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_port_tx_statistics);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_GETPORTTXSTATISTICSENTRY;
    msg_p->data.arg_port_tx_statistics = *port_statistics_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    *port_statistics_entry = msg_p->data.arg_port_tx_statistics;

    return msg_p->type.ret_ui32;
} /* End of LLDP_PMGR_GetPortTxStatisticsEntry */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetNextPortTxStatisticsEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This funtion returns true if the next statistics info.
 *            can be successfully retrieved. Otherwise, false.
 * INPUT    : LLDP_MGR_PortStatistics_T *port_statistics_entry
 * OUTPUT   : LLDP_MGR_PortStatistics_T *port_statistics_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 10.5.2.1, IEEE Std 802.1AB-2005.
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetNextPortTxStatisticsEntry(LLDP_MGR_PortTxStatistics_T *port_statistics_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_port_tx_statistics);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_GETNEXTPORTTXSTATISTICSENTRY;
    msg_p->data.arg_port_tx_statistics = *port_statistics_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    *port_statistics_entry = msg_p->data.arg_port_tx_statistics;

    return msg_p->type.ret_ui32;
} /* End of LLDP_PMGR_GetNextPortTxStatisticsEntry */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetPortRxStatisticsEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get rx statistics info. for specified port.
 * INPUT    : LLDP_MGR_PortRxStatistics_T *port_statistics_entry
 * OUTPUT   : LLDP_MGR_PortRxStatistics_T *port_statistics_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 10.5.2.2, IEEE Std 802.1AB-2005.
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetPortRxStatisticsEntry(LLDP_MGR_PortRxStatistics_T *port_statistics_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_port_rx_statistics);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_GETPORTRXSTATISTICSENTRY;
    msg_p->data.arg_port_rx_statistics = *port_statistics_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    *port_statistics_entry = msg_p->data.arg_port_rx_statistics;

    return msg_p->type.ret_ui32;
} /* End of LLDP_PMGR_GetPortRxStatisticsEntry */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetNextPortRxStatisticsEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This funtion returns true if the next statistics info.
 *            can be successfully retrieved. Otherwise, false.
 * INPUT    : LLDP_MGR_PortRxStatistics_T *port_statistics_entry
 * OUTPUT   : LLDP_MGR_PortRxStatistics_T *port_statistics_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 10.5.2.2, IEEE Std 802.1AB-2005.
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetNextPortRxStatisticsEntry(LLDP_MGR_PortRxStatistics_T *port_statistics_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_port_rx_statistics);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_GETNEXTPORTRXSTATISTICSENTRY;
    msg_p->data.arg_port_rx_statistics = *port_statistics_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    *port_statistics_entry = msg_p->data.arg_port_rx_statistics;

    return msg_p->type.ret_ui32;
} /* End of LLDP_PMGR_GetNextPortRxStatisticsEntry */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetLocalSystemData
 *-------------------------------------------------------------------------
 * PURPOSE  : Get related TLV info. in the local system.
 * INPUT    : LLDP_MGR_LocalSystemData_T *system_entry
 * OUTPUT   : LLDP_MGR_LocalSystemData_T *system_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 9.5.2/9.5.6/9.5.7/9.5.8, IEEE Std 802.1AB-2005.
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetLocalSystemData(LLDP_MGR_LocalSystemData_T *system_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_local_system_data);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = LLDP_MGR_IPCMSG_TYPE_SIZE;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_GETLOCALSYSTEMDATA;
    msg_p->data.arg_local_system_data = *system_entry;
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    *system_entry = msg_p->data.arg_local_system_data;

    return msg_p->type.ret_ui32;
} /* End of LLDP_PMGR_GetLocalSystemData */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetLocalPortData
 *-------------------------------------------------------------------------
 * PURPOSE  : Get statistics info. in the local system for specified port.
 * INPUT    : LLDP_MGR_LocalPortData_T *port_entry
 * OUTPUT   : LLDP_MGR_LocalPortData_T *port_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 9.5.3/9.5.5, IEEE Std 802.1AB-2005.
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetLocalPortData(LLDP_MGR_LocalPortData_T *port_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_local_port_data);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_GETLOCALPORTDATA;
    msg_p->data.arg_local_port_data = *port_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    *port_entry = msg_p->data.arg_local_port_data;

    return msg_p->type.ret_ui32;
} /* End of LLDP_PMGR_GetLocalPortData */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetNextLocalPortData
 *-------------------------------------------------------------------------
 * PURPOSE  : This funtion returns true if the next statistics info.
 *            can be successfully retrieved. Otherwise, false.
 * INPUT    : LLDP_MGR_LocalPortData_T *port_entry
 * OUTPUT   : LLDP_MGR_LocalPortData_T *port_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 9.5.3/9.5.5, IEEE Std 802.1AB-2005.
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetNextLocalPortData(LLDP_MGR_LocalPortData_T *port_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_local_port_data);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_GETNEXTLOCALPORTDATA;
    msg_p->data.arg_local_port_data = *port_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    *port_entry = msg_p->data.arg_local_port_data;

    return msg_p->type.ret_ui32;
} /* End of LLDP_PMGR_GetNextLocalPortData */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetLocalManagementAddressTlvEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get management address TLV info. in the local system.
 * INPUT    : None
 * OUTPUT   : LLDP_MGR_LocalManagementAddrEntry_T *man_addr_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 9.5.9, IEEE Std 802.1AB-2005.
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetLocalManagementAddressTlvEntry(LLDP_MGR_LocalManagementAddrEntry_T *man_addr_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_local_mgmt_addr_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_GETLOCALMANAGEMENTADDRESSTLVENTRY;
    msg_p->data.arg_local_mgmt_addr_entry = *man_addr_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    *man_addr_entry = msg_p->data.arg_local_mgmt_addr_entry;

    return msg_p->type.ret_ui32;
} /* End of LLDP_PMGR_GetLocalManagementAddressTlvEntry */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetLocalManagementAddress
 *-------------------------------------------------------------------------
 * PURPOSE  : Get management address TLV info. in the local system.
 * INPUT    : None
 * OUTPUT   : LLDP_MGR_LocalManagementAddrEntry_T *man_addr_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 9.5.9, IEEE Std 802.1AB-2005.
 *            This function is for WE
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetLocalManagementAddress(LLDP_MGR_LocalManagementAddrEntry_T *man_addr_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_local_mgmt_addr_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_GETLOCALMANAGEMENTADDRESS;

    msg_p->data.arg_local_mgmt_addr_entry = *man_addr_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    *man_addr_entry = msg_p->data.arg_local_mgmt_addr_entry;

    return msg_p->type.ret_ui32;
} /* End of LLDP_PMGR_GetLocalManagementAddress */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetNextLocalManagementAddress
 *-------------------------------------------------------------------------
 * PURPOSE  : Get management address TLV info. in the local system.
 * INPUT    : None
 * OUTPUT   : LLDP_MGR_LocalManagementAddrEntry_T *man_addr_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 9.5.9, IEEE Std 802.1AB-2005.
 *            This function is for WE
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetNextLocalManagementAddress(LLDP_MGR_LocalManagementAddrEntry_T *man_addr_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_local_mgmt_addr_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_GETNEXTLOCALMANAGEMENTADDRESS;
    msg_p->data.arg_local_mgmt_addr_entry = *man_addr_entry;
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    *man_addr_entry = msg_p->data.arg_local_mgmt_addr_entry;

    return msg_p->type.ret_ui32;
}
/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetNextLocalManagementAddressTlvEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get management address TLV info. in the local system.
 * INPUT    : LLDP_MGR_LocalManagementAddrEntry_T *man_addr_entry
 * OUTPUT   : LLDP_MGR_LocalManagementAddrEntry_T *man_addr_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 9.5.9, IEEE Std 802.1AB-2005.
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetNextLocalManagementAddressTlvEntry(LLDP_MGR_LocalManagementAddrEntry_T *man_addr_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_local_mgmt_addr_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_GETNEXTLOCALMANAGEMENTADDRESSTLVENTRY;
    msg_p->data.arg_local_mgmt_addr_entry = *man_addr_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    *man_addr_entry = msg_p->data.arg_local_mgmt_addr_entry;

    return msg_p->type.ret_ui32;
} /* End of LLDP_PMGR_GetNextLocalManagementAddressTlvEntry */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetNextRemoteSystemData
 *-------------------------------------------------------------------------
 * PURPOSE  : Get related TLV info. in the remote system.
 * INPUT    : LLDP_MGR_RemoteSystemData_T *system_entry
 * OUTPUT   : LLDP_MGR_RemoteSystemData_T *system_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 9.5.2.2/9.5.2.3/9.5.3.3/9.5.5.2/
 *            9.5.6.2/9.5.7.2/9.5.8.1/9.5.8.2, IEEE Std 802.1AB-2005.
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetNextRemoteSystemData(LLDP_MGR_RemoteSystemData_T *system_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_remote_system_data);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_GETNEXTREMOTESYSTEMDATA;
    msg_p->data.arg_remote_system_data = *system_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    *system_entry = msg_p->data.arg_remote_system_data;

    return msg_p->type.ret_ui32;
} /* End of LLDP_PMGR_GetNextRemoteSystemData */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetNextRemoteSystemDataByPort
 *-------------------------------------------------------------------------
 * PURPOSE  : Get related TLV info. in the remote system.
 * INPUT    : LLDP_MGR_RemoteSystemData_T *system_entry
 * OUTPUT   : LLDP_MGR_RemoteSystemData_T *system_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : If system_entry->index = 0, the return system entry will be
 *            the 1st data of the port
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetNextRemoteSystemDataByPort(LLDP_MGR_RemoteSystemData_T *system_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_remote_system_data);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_GETNEXTREMOTESYSTEMDATABYPORT;
    msg_p->data.arg_remote_system_data = *system_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    *system_entry = msg_p->data.arg_remote_system_data;

    return msg_p->type.ret_ui32;
} /* End of LLDP_PMGR_GetNextRemoteSystemDataByPort */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetNextRemoteSystemDataByIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : Get related TLV info. in the remote system.
 * INPUT    : LLDP_MGR_RemoteSystemData_T *system_entry
 * OUTPUT   : LLDP_MGR_RemoteSystemData_T *system_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetNextRemoteSystemDataByIndex(LLDP_MGR_RemoteSystemData_T *system_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_remote_system_data);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_GETNEXTREMOTESYSTEMDATABYINDEX;
    msg_p->data.arg_remote_system_data = *system_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    *system_entry = msg_p->data.arg_remote_system_data;

    return msg_p->type.ret_ui32;
} /* End of LLDP_PMGR_GetNextRemoteSystemDataByIndex */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetRemoteManagementAddressTlvEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get management address TLV info. in the remote system.
 * INPUT    : LLDP_MGR_RemoteManagementAddrEntry_T *man_addr_entry
 * OUTPUT   : LLDP_MGR_RemoteManagementAddrEntry_T *man_addr_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 9.5.9, IEEE Std 802.1AB-2005.
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetRemoteManagementAddressTlvEntry(LLDP_MGR_RemoteManagementAddrEntry_T *man_addr_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_remote_mgmt_addr_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_GETREMOTEMANAGEMENTADDRESSTLVENTRY;
    msg_p->data.arg_remote_mgmt_addr_entry = *man_addr_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    *man_addr_entry = msg_p->data.arg_remote_mgmt_addr_entry;

    return msg_p->type.ret_ui32;
} /* End of LLDP_PMGR_GetRemoteManagementAddressTlvEntry */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetNextRemoteManagementAddressTlvEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get management address TLV info. in the remote system.
 * INPUT    : LLDP_MGR_RemoteManagementAddrEntry_T *man_addr_entry
 * OUTPUT   : LLDP_MGR_RemoteManagementAddrEntry_T *man_addr_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 9.5.9, IEEE Std 802.1AB-2005.
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetNextRemoteManagementAddressTlvEntry(LLDP_MGR_RemoteManagementAddrEntry_T *man_addr_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_remote_mgmt_addr_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_GETNEXTREMOTEMANAGEMENTADDRESSTLVENTRY;
    msg_p->data.arg_remote_mgmt_addr_entry = *man_addr_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    *man_addr_entry = msg_p->data.arg_remote_mgmt_addr_entry;

    return msg_p->type.ret_ui32;
} /* End of LLDP_PMGR_GetNextRemoteManagementAddressTlvEntry */

#if (LLDP_TYPE_EXT_802DOT1 == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetXdot1ConfigEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.1 extension configuraiton
 * INPUT    : xdot1_config_entry
 * OUTPUT   : xdot1_config_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetXdot1ConfigEntry(LLDP_MGR_Xdot1ConfigEntry_T *xdot1_config_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xdot1_config_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_GETXDOT1CONFIGENTRY;
    msg_p->data.arg_xdot1_config_entry = *xdot1_config_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    *xdot1_config_entry = msg_p->data.arg_xdot1_config_entry;

    return msg_p->type.ret_ui32;
} /* End of LLDP_PMGR_GetXdot1ConfigEntry */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetNextXdot1ConfigEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.1 extension configuraiton
 * INPUT    : xdot1_config_entry
 * OUTPUT   : xdot1_config_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetNextXdot1ConfigEntry(LLDP_MGR_Xdot1ConfigEntry_T *xdot1_config_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xdot1_config_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_GETNEXTXDOT1CONFIGENTRY;
    msg_p->data.arg_xdot1_config_entry = *xdot1_config_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    *xdot1_config_entry = msg_p->data.arg_xdot1_config_entry;

    return msg_p->type.ret_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_SetXdot1ConfigEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Set 802.1 extension configuraiton
 * INPUT    : xdot1_config_entry
 * OUTPUT   : None
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_SetXdot1ConfigEntry(LLDP_MGR_Xdot1ConfigEntry_T *xdot1_config_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xdot1_config_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_SETXDOT1CONFIGENTRY;
    msg_p->data.arg_xdot1_config_entry = *xdot1_config_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            LLDP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
} /* End of LLDP_PMGR_SetXdot1ConfigEntry */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_SetXdot1ConfigPortVlanTxEnable
 *-------------------------------------------------------------------------
 * PURPOSE  : Set 802.1 extension configuraiton
 * INPUT    : xdot1_config_entry
 * OUTPUT   : None
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_PMGR_SetXdot1ConfigPortVlanTxEnable(UI32_T lport, UI32_T port_vlan_tx_enable)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_grp_ui32_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_SETXDOT1CONFIGPORTVLANTXENABLE;
    msg_p->data.arg_grp_ui32_ui32.arg_ui32_1 = lport;
    msg_p->data.arg_grp_ui32_ui32.arg_ui32_2 = port_vlan_tx_enable;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            LLDP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_SetXdot1ConfigProtoVlanTxEnable
 *-------------------------------------------------------------------------
 * PURPOSE  : Set 802.1 extension configuraiton
 * INPUT    : xdot1_config_entry
 * OUTPUT   : None
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_PMGR_SetXdot1ConfigProtoVlanTxEnable(UI32_T lport, UI32_T proto_vlan_tx_enable)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_grp_ui32_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_SETXDOT1CONFIGPROTOVLANTXENABLE;
    msg_p->data.arg_grp_ui32_ui32.arg_ui32_1 = lport;
    msg_p->data.arg_grp_ui32_ui32.arg_ui32_2 = proto_vlan_tx_enable;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            LLDP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_SetXdot1ConfigVlanNameTxEnable
 *-------------------------------------------------------------------------
 * PURPOSE  : Set 802.1 extension configuraiton
 * INPUT    : xdot1_config_entry
 * OUTPUT   : None
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_PMGR_SetXdot1ConfigVlanNameTxEnable(UI32_T lport, UI32_T vlan_name_tx_enable)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_grp_ui32_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_SETXDOT1CONFIGVLANNAMETXENABLE;
    msg_p->data.arg_grp_ui32_ui32.arg_ui32_1 = lport;
    msg_p->data.arg_grp_ui32_ui32.arg_ui32_2 = vlan_name_tx_enable;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            LLDP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_SetXdot1ConfigProtocolTxEnable
 *-------------------------------------------------------------------------
 * PURPOSE  : Set 802.1 extension configuraiton
 * INPUT    : xdot1_config_entry
 * OUTPUT   : None
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_PMGR_SetXdot1ConfigProtocolTxEnable(UI32_T lport, UI32_T protocol_tx_enable)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_grp_ui32_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_SETXDOT1CONFIGPROTOCOLTXENABLE;
    msg_p->data.arg_grp_ui32_ui32.arg_ui32_1 = lport;
    msg_p->data.arg_grp_ui32_ui32.arg_ui32_2 = protocol_tx_enable;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            LLDP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetXdot1LocEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.1 extension local entry
 * INPUT    : xdot1_loc_entry
 * OUTPUT   : xdot1_loc_entry
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetXdot1LocEntry(LLDP_MGR_Xdot1LocEntry_T *xdot1_loc_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xdot1_loc_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_GETXDOT1LOCENTRY;
    msg_p->data.arg_xdot1_loc_entry = *xdot1_loc_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    *xdot1_loc_entry = msg_p->data.arg_xdot1_loc_entry;

    return msg_p->type.ret_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetNextXdot1LocEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.1 extension local entry
 * INPUT    : xdot1_loc_entry
 * OUTPUT   : xdot1_loc_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetNextXdot1LocEntry(LLDP_MGR_Xdot1LocEntry_T *xdot1_loc_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xdot1_loc_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_GETNEXTXDOT1LOCENTRY;
    msg_p->data.arg_xdot1_loc_entry = *xdot1_loc_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    *xdot1_loc_entry = msg_p->data.arg_xdot1_loc_entry;

    return msg_p->type.ret_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetXdot1LocProtoVlanEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.1 extension local protocol vlan entry
 * INPUT    : xdot1_loc_proto_vlan_entry
 * OUTPUT   : xdot1_loc_proto_vlan_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetXdot1LocProtoVlanEntry(LLDP_MGR_Xdot1LocProtoVlanEntry_T *xdot1_loc_proto_vlan_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xdot1_loc_proto_vlan_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_GETXDOT1LOCPROTOVLANENTRY;
    msg_p->data.arg_xdot1_loc_proto_vlan_entry = *xdot1_loc_proto_vlan_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    *xdot1_loc_proto_vlan_entry = msg_p->data.arg_xdot1_loc_proto_vlan_entry;

    return msg_p->type.ret_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetNextXdot1LocProtoVlanEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.1 extension local protocol vlan entry
 * INPUT    : xdot1_loc_proto_vlan_entry
 * OUTPUT   : xdot1_loc_proto_vlan_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetNextXdot1LocProtoVlanEntry(LLDP_MGR_Xdot1LocProtoVlanEntry_T *xdot1_loc_proto_vlan_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xdot1_loc_proto_vlan_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_GETNEXTXDOT1LOCPROTOVLANENTRY;
    msg_p->data.arg_xdot1_loc_proto_vlan_entry = *xdot1_loc_proto_vlan_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    *xdot1_loc_proto_vlan_entry = msg_p->data.arg_xdot1_loc_proto_vlan_entry;

    return msg_p->type.ret_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetXdot1LocVlanNameEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.1 extension local vlan name entry
 * INPUT    : xdot1_loc_vlan_name_entry
 * OUTPUT   : xdot1_loc_vlan_name_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetXdot1LocVlanNameEntry(LLDP_MGR_Xdot1LocVlanNameEntry_T *xdot1_loc_vlan_name_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xdot1_loc_vlan_name_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_GETXDOT1LOCVLANNAMEENTRY;
    msg_p->data.arg_xdot1_loc_vlan_name_entry = *xdot1_loc_vlan_name_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    *xdot1_loc_vlan_name_entry = msg_p->data.arg_xdot1_loc_vlan_name_entry;

    return msg_p->type.ret_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetNextXdot1LocVlanNameEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.1 extension local vlan name entry
 * INPUT    : xdot1_loc_vlan_name_entry
 * OUTPUT   : xdot1_loc_vlan_name_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetNextXdot1LocVlanNameEntry(LLDP_MGR_Xdot1LocVlanNameEntry_T *xdot1_loc_vlan_name_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xdot1_loc_vlan_name_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_GETNEXTXDOT1LOCVLANNAMEENTRY;
    msg_p->data.arg_xdot1_loc_vlan_name_entry = *xdot1_loc_vlan_name_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    *xdot1_loc_vlan_name_entry = msg_p->data.arg_xdot1_loc_vlan_name_entry;

    return msg_p->type.ret_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetXdot1LocProtocolEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.1 extension protocol entry
 * INPUT    : xdot1_loc_protocol_entry
 * OUTPUT   : xdot1_loc_protocol_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetXdot1LocProtocolEntry(LLDP_MGR_Xdot1LocProtocolEntry_T *xdot1_loc_protocol_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xdot1_loc_protocol_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_GETXDOT1LOCPROTOCOLENTRY;
    msg_p->data.arg_xdot1_loc_protocol_entry = *xdot1_loc_protocol_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    *xdot1_loc_protocol_entry = msg_p->data.arg_xdot1_loc_protocol_entry;

    return msg_p->type.ret_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetNextXdot1LocProtocolEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.1 extension protocol entry
 * INPUT    : xdot1_loc_protocol_entry
 * OUTPUT   : xdot1_loc_protocol_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetNextXdot1LocProtocolEntry(LLDP_MGR_Xdot1LocProtocolEntry_T *xdot1_loc_protocol_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xdot1_loc_protocol_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_GETNEXTXDOT1LOCPROTOCOLENTRY;
    msg_p->data.arg_xdot1_loc_protocol_entry = *xdot1_loc_protocol_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    *xdot1_loc_protocol_entry = msg_p->data.arg_xdot1_loc_protocol_entry;

    return msg_p->type.ret_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetNextXdot1RemEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.1 extension remote entry
 * INPUT    : xdot1_rem_entry
 * OUTPUT   : xdot1_rem_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetNextXdot1RemEntry(LLDP_MGR_Xdot1RemEntry_T *xdot1_rem_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xdot1_rem_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_GETNEXTXDOT1REMENTRY;
    msg_p->data.arg_xdot1_rem_entry = *xdot1_rem_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    *xdot1_rem_entry = msg_p->data.arg_xdot1_rem_entry;

    return msg_p->type.ret_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetNextXdot1RemEntryByIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.1 extension remote entry
 * INPUT    : xdot1_rem_entry
 * OUTPUT   : xdot1_rem_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : for WEB/CLI
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetNextXdot1RemEntryByIndex(LLDP_MGR_Xdot1RemEntry_T *xdot1_rem_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xdot1_rem_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_GETNEXTXDOT1REMENTRYBYINDEX;
    msg_p->data.arg_xdot1_rem_entry = *xdot1_rem_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    *xdot1_rem_entry = msg_p->data.arg_xdot1_rem_entry;

    return msg_p->type.ret_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetNextXdot1RemProtoVlanEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.1 extension remote protocol vlan entry
 * INPUT    : xdot1_rem_proto_vlan_entry
 * OUTPUT   : xdot1_rem_proto_vlan_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetNextXdot1RemProtoVlanEntry(LLDP_MGR_Xdot1RemProtoVlanEntry_T *xdot1_rem_proto_vlan_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xdot1_rem_proto_vlan_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_GETNEXTXDOT1REMPROTOVLANENTRY;
    msg_p->data.arg_xdot1_rem_proto_vlan_entry = *xdot1_rem_proto_vlan_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    *xdot1_rem_proto_vlan_entry = msg_p->data.arg_xdot1_rem_proto_vlan_entry;

    return msg_p->type.ret_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetXdot1RemProtoVlanEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.1 extension remote vlan name entry
 * INPUT    : xdot1_rem_vlan_name_entry
 * OUTPUT   : xdot1_rem_vlan_name_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetNextXdot1RemVlanNameEntry(LLDP_MGR_Xdot1RemVlanNameEntry_T *xdot1_rem_vlan_name_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xdot1_rem_vlan_name_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_GETNEXTXDOT1REMVLANNAMEENTRY;
    msg_p->data.arg_xdot1_rem_vlan_name_entry = *xdot1_rem_vlan_name_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    *xdot1_rem_vlan_name_entry = msg_p->data.arg_xdot1_rem_vlan_name_entry;

    return msg_p->type.ret_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetNextXdot1RemProtocolEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.1 extension remote protocol entry
 * INPUT    : xdot1_rem_protocol_entry
 * OUTPUT   : xdot1_rem_protocol_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetNextXdot1RemProtocolEntry(LLDP_MGR_Xdot1RemProtocolEntry_T *xdot1_rem_protocol_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xdot1_rem_protocol_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_GETNEXTXDOT1REMPROTOCOLENTRY;
    msg_p->data.arg_xdot1_rem_protocol_entry = *xdot1_rem_protocol_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    *xdot1_rem_protocol_entry = msg_p->data.arg_xdot1_rem_protocol_entry;

    return msg_p->type.ret_ui32;
}
#endif /* #if (LLDP_TYPE_EXT_802DOT1 == TRUE) */

#if (LLDP_TYPE_EXT_802DOT3 == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetXdot3PortConfigEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.3 extension port config entry
 * INPUT    : xdot3_port_config_entry
 * OUTPUT   : xdot3_port_config_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetXdot3PortConfigEntry(LLDP_MGR_Xdot3PortConfigEntry_T *xdot3_port_config_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xdot3_port_config_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_GETXDOT3PORTCONFIGENTRY;
    msg_p->data.arg_xdot3_port_config_entry = *xdot3_port_config_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    *xdot3_port_config_entry = msg_p->data.arg_xdot3_port_config_entry;

    return msg_p->type.ret_ui32;
} /* End of LLDP_PMGR_GetXdot3PortConfigEntry */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetNextXdot3PortConfigEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.3 extension port config entry
 * INPUT    : xdot3_port_config_entry
 * OUTPUT   : xdot3_port_config_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetNextXdot3PortConfigEntry(LLDP_MGR_Xdot3PortConfigEntry_T *xdot3_port_config_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xdot3_port_config_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_GETNEXTXDOT3PORTCONFIGENTRY;
    msg_p->data.arg_xdot3_port_config_entry = *xdot3_port_config_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    *xdot3_port_config_entry = msg_p->data.arg_xdot3_port_config_entry;

    return msg_p->type.ret_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_SetXdot3PortConfigEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Set 802.3 extension port config entry
 * INPUT    : xdot3_port_config_entry
 * OUTPUT   : None
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : for snmp
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_SetXdot3PortConfigEntry(UI32_T lport, UI8_T status)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_grp_ui32_ui8);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_SETXDOT3PORTCONFIGENTRY;
    msg_p->data.arg_grp_ui32_ui8.arg_ui32 = lport;
    msg_p->data.arg_grp_ui32_ui8.arg_ui8 = status;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            LLDP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_SetXdot3PortConfig
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.3 extension port config entry
 * INPUT    : xdot3_port_config_entry
 * OUTPUT   : xdot3_port_config_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : for WEB/CLI
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_SetXdot3PortConfig(LLDP_MGR_Xdot3PortConfigEntry_T *xdot3_port_config_entry_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xdot3_port_config_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_SETXDOT3PORTCONFIG;
    msg_p->data.arg_xdot3_port_config_entry = *xdot3_port_config_entry_p;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            LLDP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
} /* End of LLDP_PMGR_SetXdot3PortConfig */
UI32_T  LLDP_PMGR_GetXdot3LocPortEntry(LLDP_MGR_Xdot3LocPortEntry_T *xdot3_port_local_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xdot3_local_port_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_GETXDOT3LOCPORTENTRY;
    msg_p->data.arg_xdot3_local_port_entry = *xdot3_port_local_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    *xdot3_port_local_entry = msg_p->data.arg_xdot3_local_port_entry;

    return msg_p->type.ret_ui32;
}
UI32_T  LLDP_PMGR_GetNextXdot3LocPortEntry(LLDP_MGR_Xdot3LocPortEntry_T *xdot3_port_local_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xdot3_local_port_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_GETNEXTXDOT3LOCPORTENTRY;
    msg_p->data.arg_xdot3_local_port_entry = *xdot3_port_local_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    *xdot3_port_local_entry = msg_p->data.arg_xdot3_local_port_entry;

    return msg_p->type.ret_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetXdot3LocPowerEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.3 extension local power entry
 * INPUT    : xdot3_loc_power_entry
 * OUTPUT   : xdot3_loc_power_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetXdot3LocPowerEntry(LLDP_MGR_Xdot3LocPowerEntry_T *xdot3_loc_power_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xdot3_loc_power_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_GETXDOT3LOCPOWERENTRY;
    msg_p->data.arg_xdot3_loc_power_entry = *xdot3_loc_power_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    *xdot3_loc_power_entry = msg_p->data.arg_xdot3_loc_power_entry;

    return msg_p->type.ret_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetNextXdot3LocPowerEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.3 extension local power entry
 * INPUT    : xdot3_loc_power_entry
 * OUTPUT   : xdot3_loc_power_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetNextXdot3LocPowerEntry(LLDP_MGR_Xdot3LocPowerEntry_T *xdot3_loc_power_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xdot3_loc_power_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_GETNEXTXDOT3LOCPOWERENTRY;
    msg_p->data.arg_xdot3_loc_power_entry = *xdot3_loc_power_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    *xdot3_loc_power_entry = msg_p->data.arg_xdot3_loc_power_entry;

    return msg_p->type.ret_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetXdot3LocLinkAggEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.3 extension local link aggregation entry
 * INPUT    : xdot3_loc_link_agg_entry
 * OUTPUT   : xdot3_loc_link_agg_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetXdot3LocLinkAggEntry(LLDP_MGR_Xdot3LocLinkAggEntry_T *xdot3_loc_link_agg_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xdot3_loc_link_agg_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_GETXDOT3LOCLINKAGGENTRY;
    msg_p->data.arg_xdot3_loc_link_agg_entry = *xdot3_loc_link_agg_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    *xdot3_loc_link_agg_entry = msg_p->data.arg_xdot3_loc_link_agg_entry;

    return msg_p->type.ret_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetNextXdot3LocLinkAggEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.3 extension local link aggregation entry
 * INPUT    : xdot3_loc_link_agg_entry
 * OUTPUT   : xdot3_loc_link_agg_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetNextXdot3LocLinkAggEntry(LLDP_MGR_Xdot3LocLinkAggEntry_T *xdot3_loc_link_agg_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xdot3_loc_link_agg_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_GETNEXTXDOT3LOCLINKAGGENTRY;
    msg_p->data.arg_xdot3_loc_link_agg_entry = *xdot3_loc_link_agg_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    *xdot3_loc_link_agg_entry = msg_p->data.arg_xdot3_loc_link_agg_entry;

    return msg_p->type.ret_ui32;
}

UI32_T  LLDP_PMGR_GetXdot3LocMaxFrameSizeEntry(LLDP_MGR_Xdot3LocMaxFrameSizeEntry_T *xdot3_loc_maxframe_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xdot3_loc_maxframe_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_GETXDOT3LOCMAXFRAMESIZEENTRY;
    msg_p->data.arg_xdot3_loc_maxframe_entry = *xdot3_loc_maxframe_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    *xdot3_loc_maxframe_entry = msg_p->data.arg_xdot3_loc_maxframe_entry;

    return msg_p->type.ret_ui32;
}

UI32_T  LLDP_PMGR_GetNextXdot3LocMaxFrameSizeEntry(LLDP_MGR_Xdot3LocMaxFrameSizeEntry_T *xdot3_loc_maxframe_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xdot3_loc_maxframe_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_GETNEXTXDOT3LOCMAXFRAMESIZEENTRY;
    msg_p->data.arg_xdot3_loc_maxframe_entry = *xdot3_loc_maxframe_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    *xdot3_loc_maxframe_entry = msg_p->data.arg_xdot3_loc_maxframe_entry;

    return msg_p->type.ret_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetXdot3RemPortEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.3 extension remote port entry
 * INPUT    : xdot3_rem_port_entry
 * OUTPUT   : xdot3_rem_port_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : for snmp
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetXdot3RemPortEntry(LLDP_MGR_Xdot3RemPortEntry_T *xdot3_rem_port_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xdot3_rem_port_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_GETXDOT3REMPORTENTRY;
    msg_p->data.arg_xdot3_rem_port_entry = *xdot3_rem_port_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    *xdot3_rem_port_entry = msg_p->data.arg_xdot3_rem_port_entry;

    return msg_p->type.ret_ui32;

}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetNextXdot3RemPortEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.3 extension remote port entry
 * INPUT    : xdot3_rem_port_entry
 * OUTPUT   : xdot3_rem_port_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : for snmp
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetNextXdot3RemPortEntry(LLDP_MGR_Xdot3RemPortEntry_T *xdot3_rem_port_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xdot3_rem_port_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_GETNEXTXDOT3REMPORTENTRY;
    msg_p->data.arg_xdot3_rem_port_entry = *xdot3_rem_port_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    *xdot3_rem_port_entry = msg_p->data.arg_xdot3_rem_port_entry;

    return msg_p->type.ret_ui32;

}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetNextXdot3RemPowerEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.3 extension remote power entry
 * INPUT    : xdot3_rem_power_entry
 * OUTPUT   : xdot3_rem_power_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetNextXdot3RemPowerEntry(LLDP_MGR_Xdot3RemPowerEntry_T *xdot3_rem_power_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xdot3_rem_power_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_GETNEXTXDOT3REMPOWERENTRY;
    msg_p->data.arg_xdot3_rem_power_entry = *xdot3_rem_power_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    *xdot3_rem_power_entry = msg_p->data.arg_xdot3_rem_power_entry;

    return msg_p->type.ret_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetXdot3RemLinkAggEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.3 extension remote link aggregation entry
 * INPUT    : xdot3_rem_link_agg_entry
 * OUTPUT   : xdot3_rem_link_agg_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetXdot3RemLinkAggEntry(LLDP_MGR_Xdot3RemLinkAggEntry_T *xdot3_rem_link_agg_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xdot3_rem_link_agg_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_GETXDOT3REMLINKAGGENTRY;
    msg_p->data.arg_xdot3_rem_link_agg_entry = *xdot3_rem_link_agg_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    *xdot3_rem_link_agg_entry = msg_p->data.arg_xdot3_rem_link_agg_entry;

    return msg_p->type.ret_ui32;

}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetNextXdot3RemLinkAggEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.3 extension remote link aggregation entry
 * INPUT    : xdot3_rem_link_agg_entry
 * OUTPUT   : xdot3_rem_link_agg_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetNextXdot3RemLinkAggEntry(LLDP_MGR_Xdot3RemLinkAggEntry_T *xdot3_rem_link_agg_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xdot3_rem_link_agg_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_GETNEXTXDOT3REMLINKAGGENTRY;
    msg_p->data.arg_xdot3_rem_link_agg_entry = *xdot3_rem_link_agg_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    *xdot3_rem_link_agg_entry = msg_p->data.arg_xdot3_rem_link_agg_entry;

    return msg_p->type.ret_ui32;

}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetXdot3RemMaxFrameSizeEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.3 extension remote maximum frame size entry
 * INPUT    : xdot3_rem_max_frame_size_entry
 * OUTPUT   : xdot3_rem_max_frame_size_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetXdot3RemMaxFrameSizeEntry(LLDP_MGR_Xdot3RemMaxFrameSizeEntry_T *xdot3_rem_max_frame_size_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xdot3_rem_max_frame_size_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_GETXDOT3REMMAXFRAMESIZEENTRY;
    msg_p->data.arg_xdot3_rem_max_frame_size_entry = *xdot3_rem_max_frame_size_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    *xdot3_rem_max_frame_size_entry = msg_p->data.arg_xdot3_rem_max_frame_size_entry;

    return msg_p->type.ret_ui32;

}
/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetNextXdot3RemMaxFrameSizeEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.3 extension remote maximum frame size entry
 * INPUT    : xdot3_rem_max_frame_size_entry
 * OUTPUT   : xdot3_rem_max_frame_size_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetNextXdot3RemMaxFrameSizeEntry(LLDP_MGR_Xdot3RemMaxFrameSizeEntry_T *xdot3_rem_max_frame_size_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xdot3_rem_max_frame_size_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_GETNEXTXDOT3REMMAXFRAMESIZEENTRY;
    msg_p->data.arg_xdot3_rem_max_frame_size_entry = *xdot3_rem_max_frame_size_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    *xdot3_rem_max_frame_size_entry = msg_p->data.arg_xdot3_rem_max_frame_size_entry;

    return msg_p->type.ret_ui32;

}
#endif /* #if (LLDP_TYPE_EXT_802DOT3 == TRUE) */

#if (SYS_CPNT_ADD == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_IsTelephoneMac
 *-------------------------------------------------------------------------
 * PURPOSE  : To determine if the mac address is belonged to a telephone.
 * INPUT    : lport, device_mac, device_mac_len
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_PMGR_IsTelephoneMac(UI32_T lport, UI8_T  *device_mac, UI32_T device_mac_len)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_grp_ui32_ui8_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_ISTELEPHONEMAC;
    msg_p->data.arg_grp_ui32_ui8_ui32.arg_ui32_1 = lport;
    memcpy(msg_p->data.arg_grp_ui32_ui8_ui32.arg_ui8, device_mac, device_mac_len);
    msg_p->data.arg_grp_ui32_ui8_ui32.arg_ui32_2 = device_mac_len;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            LLDP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_IsTelephoneNetworkAddr
 *-------------------------------------------------------------------------
 * PURPOSE  : To determine if the netowrk address is belonged to a telephone.
 * INPUT    : lport, device_network_addr_subtype, device_network_addr, device_network_addr_len
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_PMGR_IsTelephoneNetworkAddr(UI32_T lport, UI8_T device_network_addr_subtype, UI8_T  *device_network_addr, UI32_T device_network_addr_len)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_grp_ui32_ui8_ui8_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_ISTELEPHONENETWORKADDR;
    msg_p->data.arg_grp_ui32_ui8_ui8_ui32.arg_ui32_1 = lport;
    msg_p->data.arg_grp_ui32_ui8_ui8_ui32.arg_ui8_1 = device_network_addr_subtype;
    memcpy(msg_p->data.arg_grp_ui32_ui8_ui8_ui32.arg_ui8_2, device_network_addr, device_network_addr_len);
    msg_p->data.arg_grp_ui32_ui8_ui8_ui32.arg_ui32_2 = device_network_addr_len;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            LLDP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    return msg_p->type.ret_bool;
}
#endif /* #if (SYS_CPNT_ADD == TRUE) */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_NotifySysNameChanged
 *-------------------------------------------------------------------------
 * PURPOSE  : When there is something changed, this function will be called.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void LLDP_PMGR_NotifySysNameChanged(void)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(LLDP_MGR_IPCMSG_TYPE_SIZE)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = LLDP_MGR_IPCMSG_TYPE_SIZE;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_NOTIFYSYSNAMECHANGED;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            LLDP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return;
    }

    return;
} /* End of LLDP_PMGR_NotifySysNameChanged */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_NotifyRifChanged
 *-------------------------------------------------------------------------
 * PURPOSE  : When there is something changed, this function will be called.
 * INPUT    : vid_ifindex -- the specified vlan;
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void LLDP_PMGR_NotifyRifChanged(UI32_T vid_ifindex)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_NOTIFYRIFCHANGED;
    msg_p->data.arg_ui32 = vid_ifindex;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            LLDP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return;
    }

    return;
} /* End of LLDP_PMGR_NotifyRifChanged */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_NotifyRoutingChanged
 *-------------------------------------------------------------------------
 * PURPOSE  : When there is something changed, this function will be called.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void LLDP_PMGR_NotifyRoutingChanged(void)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(LLDP_MGR_IPCMSG_TYPE_SIZE)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = LLDP_MGR_IPCMSG_TYPE_SIZE;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_NOTIFYROUTINGCHANGED;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            LLDP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return;
    }

    return;
} /* End of LLDP_PMGR_NotifyRoutingChanged */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_NotifyPseTableChanged
 *-------------------------------------------------------------------------
 * PURPOSE  : When there is something changed, this function will be called.
 * INPUT    : lport -- the specified port
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : lport can be 0, which means changes for all lports
 *-------------------------------------------------------------------------
 */
void LLDP_PMGR_NotifyPseTableChanged(UI32_T lport)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_NOTIFYPSETABLECHANGED;
    msg_p->data.arg_ui32 = lport;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            LLDP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return;
    }

    return;
} /* End of LLDP_PMGR_NotifyPseTableChanged */
#if (LLDP_TYPE_MED == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_SetXMedFastStartRepeatCount
 *-------------------------------------------------------------------------
 * PURPOSE  : Set LLDP-MED fast start repeat count
 * INPUT    : repeat_count
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T LLDP_PMGR_SetXMedFastStartRepeatCount(UI32_T  repeat_count)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_SETXMEDFASTSTARTREPEATCOUNT;
    msg_p->data.arg_ui32 = repeat_count;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            LLDP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetXMedPortConfigEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED port configuration entry
 * INPUT    : None
 * OUTPUT   : port_config_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_PMGR_GetXMedPortConfigEntry(LLDP_MGR_XMedPortConfigEntry_T *port_config_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xmed_port_config_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_GETXMEDPORTCONFIGENTRY;
    msg_p->data.arg_xmed_port_config_entry = *port_config_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *port_config_entry = msg_p->data.arg_xmed_port_config_entry;

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetNextXMedPortConfigEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED port configuration entry
 * INPUT    : None
 * OUTPUT   : port_config_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_PMGR_GetNextXMedPortConfigEntry(LLDP_MGR_XMedPortConfigEntry_T *port_config_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xmed_port_config_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_GETNEXTXMEDPORTCONFIGENTRY;
    msg_p->data.arg_xmed_port_config_entry = *port_config_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *port_config_entry = msg_p->data.arg_xmed_port_config_entry;

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_SetXMedPortConfigTlvsTx
 *-------------------------------------------------------------------------
 * PURPOSE  : Set LLDP-MED port transfer tlvs
 * INPUT    : lport, tlvs_tx_enabled
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_PMGR_SetXMedPortConfigTlvsTx(UI32_T lport, UI16_T tlvs_tx_enabled)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_grp_ui32_ui16);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_SETXMEDPORTCONFIGTLVSTX;
    msg_p->data.arg_grp_ui32_ui16.arg_ui32 = lport;
    msg_p->data.arg_grp_ui32_ui16.arg_ui16 = tlvs_tx_enabled;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            LLDP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetRunningXMedPortConfigTlvsTx
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED port configuration entry
 * INPUT    : lport
 * OUTPUT   : tlvs_tx_enabled
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     : CLI
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetRunningXMedPortConfigTlvsTx(UI32_T lport, UI16_T *tlvs_tx_enabled)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_grp_ui32_ui16);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_GETRUNNINGXMEDPORTCONFIGTLVSTX;
    msg_p->data.arg_grp_ui32_ui16.arg_ui32 = lport;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    *tlvs_tx_enabled = msg_p->data.arg_grp_ui32_ui16.arg_ui16;

    return msg_p->type.ret_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_SetXMedPortConfigNotifEnabled
 *-------------------------------------------------------------------------
 * PURPOSE  : Set LLDP-MED port notification
 * INPUT    : notif_enabled
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_PMGR_SetXMedPortConfigNotifEnabled(UI32_T lport, UI8_T notif_enabled)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_grp_ui32_ui8);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_SETXMEDPORTCONFIGNOTIFENABLED;
    msg_p->data.arg_grp_ui32_ui8.arg_ui32 = lport;
    msg_p->data.arg_grp_ui32_ui8.arg_ui8 = notif_enabled;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            LLDP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetRunningXMedPortNotification
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED port configuration entry
 * INPUT    : lport
 * OUTPUT   : enabled
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     : CLI
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetRunningXMedPortNotification(UI32_T lport, BOOL_T *enabled)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_grp_ui32_ui8);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_GETRUNNINGXMEDPORTNOTIFICATION;
    msg_p->data.arg_grp_ui32_ui8.arg_ui32 = lport;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    *enabled = msg_p->data.arg_grp_ui32_ui8.arg_ui8;

    return msg_p->type.ret_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetXMedLocMediaPolicyEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED media policy entry
 * INPUT    : None
 * OUTPUT   : loc_med_policy_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_PMGR_GetXMedLocMediaPolicyEntry(LLDP_MGR_XMedLocMediaPolicyEntry_T *loc_med_policy_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xmed_loc_med_policy_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_GETXMEDLOCMEDIAPOLICYENTRY;
    msg_p->data.arg_xmed_loc_med_policy_entry = *loc_med_policy_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *loc_med_policy_entry = msg_p->data.arg_xmed_loc_med_policy_entry;

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetNextXMedLocMediaPolicyEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED media policy entry
 * INPUT    : None
 * OUTPUT   : loc_med_policy_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_PMGR_GetNextXMedLocMediaPolicyEntry(LLDP_MGR_XMedLocMediaPolicyEntry_T *loc_med_policy_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xmed_loc_med_policy_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_GETNEXTXMEDLOCMEDIAPOLICYENTRY;
    msg_p->data.arg_xmed_loc_med_policy_entry = *loc_med_policy_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *loc_med_policy_entry = msg_p->data.arg_xmed_loc_med_policy_entry;

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetXMedLocLocationEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED local location entry
 * INPUT    : None
 * OUTPUT   : loc_location_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : This API is only used by SNMP
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_PMGR_GetXMedLocLocationEntry(LLDP_MGR_XMedLocLocationEntry_T *loc_location_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xmed_loc_location_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_GETXMEDLOCLOCATIONENTRY;
    msg_p->data.arg_xmed_loc_location_entry = *loc_location_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *loc_location_entry = msg_p->data.arg_xmed_loc_location_entry;

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetNextXMedLocLocationEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED local location entry
 * INPUT    : None
 * OUTPUT   : loc_location_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : This API is only used by SNMP
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_PMGR_GetNextXMedLocLocationEntry(LLDP_MGR_XMedLocLocationEntry_T *loc_location_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xmed_loc_location_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_GETNEXTXMEDLOCLOCATIONENTRY;
    msg_p->data.arg_xmed_loc_location_entry = *loc_location_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *loc_location_entry = msg_p->data.arg_xmed_loc_location_entry;

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_SetXMedLocLocationEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Set LLDP-MED local location entry
 * INPUT    : loc_location_entry
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : This API is only used by SNMP.
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_PMGR_SetXMedLocLocationEntry(LLDP_MGR_XMedLocLocationEntry_T *loc_location_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xmed_loc_location_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_SETXMEDLOCLOCATIONENTRY;
    msg_p->data.arg_xmed_loc_location_entry = *loc_location_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            LLDP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetXMedLocLocationStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED local location status
 * INPUT    : lport
 *            location_type
 * OUTPUT   : status_p
 * RETURN   : TRUE/FALSE
 * NOTE     : This API is used for CLI/WEB.
 *-------------------------------------------------------------------------
 */
BOOL_T LLDP_PMGR_GetXMedLocLocationStatus(UI32_T lport, UI32_T location_type, BOOL_T *status_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_grp_xmed_loc_status);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_GETXMEDLOCLOCATIONSTATUS;
    msg_p->data.arg_grp_xmed_loc_status.arg_lport = lport;
    msg_p->data.arg_grp_xmed_loc_status.arg_location_type = location_type;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *status_p = msg_p->data.arg_grp_xmed_loc_status.arg_status;

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_SetXMedLocLocationStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : Set LLDP-MED local location status
 * INPUT    : lport
 *            location_type
 *            status
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : This API is used for CLI/WEB.
 *-------------------------------------------------------------------------
 */
BOOL_T LLDP_PMGR_SetXMedLocLocationStatus(UI32_T lport, UI32_T location_type, BOOL_T status)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_grp_xmed_loc_status);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_SETXMEDLOCLOCATIONSTATUS;
    msg_p->data.arg_grp_xmed_loc_status.arg_lport = lport;
    msg_p->data.arg_grp_xmed_loc_status.arg_location_type = location_type;
    msg_p->data.arg_grp_xmed_loc_status.arg_status = status;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            LLDP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetRunningXMedLocLocationStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED local location status
 * INPUT    : lport
 *            location_type
 * OUTPUT   : status_p
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_PMGR_GetRunningXMedLocLocationStatus(UI32_T lport, UI32_T location_type, BOOL_T *status_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_grp_xmed_loc_status);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_GETRUNNINGXMEDLOCLOCATIONSTATUS;
    msg_p->data.arg_grp_xmed_loc_status.arg_lport = lport;
    msg_p->data.arg_grp_xmed_loc_status.arg_location_type = location_type;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    *status_p = msg_p->data.arg_grp_xmed_loc_status.arg_status;

    return msg_p->type.ret_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetXMedLocLocationCivicAddrCoutryCode
 *-------------------------------------------------------------------------
 * PURPOSE  : Get civic addr country code.
 * INPUT    : lport
 * OUTPUT   : country_code.
 * RETURN   : TRUE/FALSE
 * NOTE     : This API is used for CLI/WEB.
 *-------------------------------------------------------------------------
 */
BOOL_T LLDP_PMGR_GetXMedLocLocationCivicAddrCoutryCode(UI32_T lport, UI8_T *country_code)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_grp_xmed_loc_country_code);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_GETXMEDLOCLOCATIONCIVICADDRCOUTRYCODE;
    msg_p->data.arg_grp_xmed_loc_country_code.arg_lport = lport;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    memcpy(country_code, msg_p->data.arg_grp_xmed_loc_country_code.arg_country_code, 2);

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_SetXMedLocLocationCivicAddrCoutryCode
 *-------------------------------------------------------------------------
 * PURPOSE  : Set civic addr country code.
 * INPUT    : lport, country_code
 * OUTPUT   : None.
 * RETURN   : TRUE/FALSE
 * NOTE     : This API is used for CLI/WEB.
 *-------------------------------------------------------------------------
 */
BOOL_T LLDP_PMGR_SetXMedLocLocationCivicAddrCoutryCode(UI32_T lport, UI8_T *country_code)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_grp_xmed_loc_country_code);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_SETXMEDLOCLOCATIONCIVICADDRCOUTRYCODE;
    msg_p->data.arg_grp_xmed_loc_country_code.arg_lport = lport;
    memcpy(msg_p->data.arg_grp_xmed_loc_country_code.arg_country_code, country_code, 2);

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            LLDP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetRunningXMedLocLocationCivicAddrCoutryCode
 *-------------------------------------------------------------------------
 * PURPOSE  : Get civic addr country code.
 * INPUT    : lport
 * OUTPUT   : country_code.
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     : This API is used for CLI/WEB.
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_PMGR_GetRunningXMedLocLocationCivicAddrCoutryCode(UI32_T lport, UI8_T *country_code)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_grp_xmed_loc_country_code);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_GETRUNNINGXMEDLOCLOCATIONCIVICADDRCOUTRYCODE;
    msg_p->data.arg_grp_xmed_loc_country_code.arg_lport = lport;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    memcpy(country_code, msg_p->data.arg_grp_xmed_loc_country_code.arg_country_code, 2);

    return msg_p->type.ret_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetXMedLocLocationCivicAddrWhat
 *-------------------------------------------------------------------------
 * PURPOSE  : Get civic addr what value.
 * INPUT    : lport, what
 * OUTPUT   : None.
 * RETURN   : TRUE/FALSE
 * NOTE     : This API is used for CLI/WEB.
 *-------------------------------------------------------------------------
 */
BOOL_T LLDP_PMGR_GetXMedLocLocationCivicAddrWhat(UI32_T lport, UI8_T *what)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_grp_ui32_ui8);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_GETXMEDLOCLOCATIONCIVICADDRWHAT;
    msg_p->data.arg_grp_ui32_ui8.arg_ui32 = lport;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *what = msg_p->data.arg_grp_ui32_ui8.arg_ui8;

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_SetXMedLocLocationCivicAddrWhat
 *-------------------------------------------------------------------------
 * PURPOSE  : Set civic addr what value.
 * INPUT    : lport, what
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : This API is used for CLI/WEB.
 *-------------------------------------------------------------------------
 */
BOOL_T LLDP_PMGR_SetXMedLocLocationCivicAddrWhat(UI32_T lport, UI8_T what)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_grp_ui32_ui8);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_SETXMEDLOCLOCATIONCIVICADDRWHAT;
    msg_p->data.arg_grp_ui32_ui8.arg_ui32 = lport;
    msg_p->data.arg_grp_ui32_ui8.arg_ui8 = what;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            LLDP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetRunningXMedLocLocationCivicAddrWhat
 *-------------------------------------------------------------------------
 * PURPOSE  : Get civic addr what value.
 * INPUT    : lport
 * OUTPUT   : what
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     : This API is used for CLI/WEB.
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_PMGR_GetRunningXMedLocLocationCivicAddrWhat(UI32_T lport, UI8_T *what)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_grp_ui32_ui8);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_GETRUNNINGXMEDLOCLOCATIONCIVICADDRWHAT;
    msg_p->data.arg_grp_ui32_ui8.arg_ui32 = lport;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    *what = msg_p->data.arg_grp_ui32_ui8.arg_ui8;

    return msg_p->type.ret_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_Get1stXMedLocLocationCivicAddrCaEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get civic addr country code.
 * INPUT    : lport
 * OUTPUT   : ca_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : This API is used for CLI/WEB.
 *-------------------------------------------------------------------------
 */
BOOL_T LLDP_PMGR_Get1stXMedLocLocationCivicAddrCaEntry(UI32_T lport, LLDP_MGR_XMedLocationCivicAddrCaEntry_T *ca_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_grp_xmed_ca_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_GET1STXMEDLOCLOCATIONCIVICADDRCAENTRY;
    msg_p->data.arg_grp_xmed_ca_entry.arg_lport = lport;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *ca_entry = msg_p->data.arg_grp_xmed_ca_entry.arg_ca_entry;

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetNextXMedLocLocationCivicAddrCaEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get civic addr country code.
 * INPUT    : lport
 * OUTPUT   : ca_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : This API is used for CLI/WEB.
 *-------------------------------------------------------------------------
 */
BOOL_T LLDP_PMGR_GetNextXMedLocLocationCivicAddrCaEntry(UI32_T lport, LLDP_MGR_XMedLocationCivicAddrCaEntry_T *ca_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_grp_xmed_ca_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_GETNEXTXMEDLOCLOCATIONCIVICADDRCAENTRY;
    msg_p->data.arg_grp_xmed_ca_entry.arg_lport = lport;
    msg_p->data.arg_grp_xmed_ca_entry.arg_ca_entry = *ca_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *ca_entry = msg_p->data.arg_grp_xmed_ca_entry.arg_ca_entry;

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_SetXMedLocLocationCivicAddrCaEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Set civic addr tlv.
 * INPUT    : lport, ca_tlv, set_or_unset
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : This API is used for CLI/WEB.
 *-------------------------------------------------------------------------
 */
BOOL_T LLDP_PMGR_SetXMedLocLocationCivicAddrCaEntry(UI32_T lport, LLDP_MGR_XMedLocationCivicAddrCaEntry_T *ca_entry, BOOL_T set_or_unset)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_grp_xmed_ca_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_SETXMEDLOCLOCATIONCIVICADDRCAENTRY;
    msg_p->data.arg_grp_xmed_ca_entry.arg_lport = lport;
    msg_p->data.arg_grp_xmed_ca_entry.arg_ca_entry = *ca_entry;
    msg_p->data.arg_grp_xmed_ca_entry.arg_set_or_unset = set_or_unset;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            LLDP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetXMedLocXPoePsePortEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED local port poe/pse entry
 * INPUT    : None
 * OUTPUT   : loc_poe_pse_port_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_PMGR_GetXMedLocXPoePsePortEntry(LLDP_MGR_XMedLocXPoePsePortEntry_T *loc_poe_pse_port_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xmed_loc_poe_pse_port_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_GETXMEDLOCXPOEPSEPORTENTRY;
    msg_p->data.arg_xmed_loc_poe_pse_port_entry = *loc_poe_pse_port_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *loc_poe_pse_port_entry = msg_p->data.arg_xmed_loc_poe_pse_port_entry;

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetNextXMedLocXPoePsePortEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED local port poe/pse entry
 * INPUT    : None
 * OUTPUT   : loc_poe_pse_port_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_PMGR_GetNextXMedLocXPoePsePortEntry(LLDP_MGR_XMedLocXPoePsePortEntry_T *loc_poe_pse_port_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xmed_loc_poe_pse_port_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_GETNEXTXMEDLOCXPOEPSEPORTENTRY;
    msg_p->data.arg_xmed_loc_poe_pse_port_entry = *loc_poe_pse_port_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *loc_poe_pse_port_entry = msg_p->data.arg_xmed_loc_poe_pse_port_entry;

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetXMedLocXPoeEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED local poe entry
 * INPUT    : None
 * OUTPUT   : loc_poe_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_PMGR_GetXMedLocXPoeEntry(LLDP_MGR_XMedLocXPoeEntry_T *loc_poe_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xmed_loc_poe_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = LLDP_MGR_IPCMSG_TYPE_SIZE;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_GETXMEDLOCXPOEENTRY;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *loc_poe_entry = msg_p->data.arg_xmed_loc_poe_entry;

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetXMedLocInventoryEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED local inventory entry
 * INPUT    : None
 * OUTPUT   : loc_inventory_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_PMGR_GetXMedLocInventoryEntry(LLDP_MGR_XMedLocInventory_T *loc_inventory_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xmed_loc_inventory_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = LLDP_MGR_IPCMSG_TYPE_SIZE;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_GETXMEDLOCINVENTORYENTRY;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *loc_inventory_entry = msg_p->data.arg_xmed_loc_inventory_entry;

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetNextXMedRemCapEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED remote device capability entry
 * INPUT    : None
 * OUTPUT   : rem_cap_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_PMGR_GetNextXMedRemCapEntry(LLDP_MGR_XMedRemCapEntry_T *rem_cap_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xmed_rem_cap_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_GETNEXTXMEDREMCAPENTRY;
    msg_p->data.arg_xmed_rem_cap_entry = *rem_cap_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *rem_cap_entry = msg_p->data.arg_xmed_rem_cap_entry;

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetNextXMedRemMediaPolicyEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED remote device media policy entry
 * INPUT    : None
 * OUTPUT   : rem_med_policy_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_PMGR_GetNextXMedRemMediaPolicyEntry(LLDP_MGR_XMedRemMediaPolicyEntry_T *rem_med_policy_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xmed_rem_med_policy_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_GETNEXTXMEDREMMEDIAPOLICYENTRY;
    msg_p->data.arg_xmed_rem_med_policy_entry = *rem_med_policy_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *rem_med_policy_entry = msg_p->data.arg_xmed_rem_med_policy_entry;

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetXMedRemMediaPolicyEntryByIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED remote device media policy entry by lport, index and app_type
 *            (i.e. without timemark)
 * INPUT    : None
 * OUTPUT   : rem_med_policy_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : This API is used only for WEB/CLI.
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_PMGR_GetXMedRemMediaPolicyEntryByIndex(LLDP_MGR_XMedRemMediaPolicyEntry_T *rem_med_policy_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xmed_rem_med_policy_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_GETXMEDREMMEDIAPOLICYENTRYBYINDEX;
    msg_p->data.arg_xmed_rem_med_policy_entry = *rem_med_policy_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *rem_med_policy_entry = msg_p->data.arg_xmed_rem_med_policy_entry;

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetNextXMedRemInventoryEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED remote device inventory entry
 * INPUT    : None
 * OUTPUT   : rem_inventory_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_PMGR_GetNextXMedRemInventoryEntry(LLDP_MGR_XMedRemInventoryEntry_T *rem_inventory_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xmed_rem_inventory_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_GETNEXTXMEDREMINVENTORYENTRY;
    msg_p->data.arg_xmed_rem_inventory_entry = *rem_inventory_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *rem_inventory_entry = msg_p->data.arg_xmed_rem_inventory_entry;

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetNextXMedRemLocationEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED remote device location entry
 * INPUT    : None
 * OUTPUT   : rem_location_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_PMGR_GetNextXMedRemLocationEntry(LLDP_MGR_XMedRemLocationEntry_T *rem_location_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xmed_rem_location_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_GETNEXTXMEDREMLOCATIONENTRY;
    msg_p->data.arg_xmed_rem_location_entry = *rem_location_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *rem_location_entry = msg_p->data.arg_xmed_rem_location_entry;

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetNextXMedRemLocationEntryByIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED remote device location entry
 * INPUT    : None
 * OUTPUT   : rem_location_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : This API is used onlu for WEB/CLI.
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_PMGR_GetNextXMedRemLocationEntryByIndex(LLDP_MGR_XMedRemLocationEntry_T *rem_location_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xmed_rem_location_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_GETNEXTXMEDREMLOCATIONENTRYBYINDEX;
    msg_p->data.arg_xmed_rem_location_entry = *rem_location_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *rem_location_entry = msg_p->data.arg_xmed_rem_location_entry;

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetXMedRemLocationCivicAddrCountryCode
 *-------------------------------------------------------------------------
 * PURPOSE  : Set civic addr country code.
 * INPUT    : rem_loc_port_num
 *            rem_index
 * OUTPUT   : country_code
 * RETURN   : TRUE/FALSE
 * NOTE     : This API is used for CLI/WEB.
 *-------------------------------------------------------------------------
 */
BOOL_T LLDP_PMGR_GetXMedRemLocationCivicAddrCountryCode(UI32_T rem_loc_port_num,
                                                        UI32_T rem_index,
                                                        UI8_T *country_code)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_grp_xmed_rem_country_code);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_GETXMEDREMLOCATIONCIVICADDRCOUTRYCODE;
    msg_p->data.arg_grp_xmed_rem_country_code.arg_rem_loc_port_num = rem_loc_port_num;
    msg_p->data.arg_grp_xmed_rem_country_code.arg_rem_index = rem_index;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    memcpy(country_code, msg_p->data.arg_grp_xmed_rem_country_code.arg_country_code, 2);

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetXMedRemLocationCivicAddrWhat
 *-------------------------------------------------------------------------
 * PURPOSE  : Get civic addr what value.
 * INPUT    : rem_loc_port_num
 *            rem_index
 * OUTPUT   : what
 * RETURN   : TRUE/FALSE
 * NOTE     : This API is used for CLI/WEB.
 *-------------------------------------------------------------------------
 */
BOOL_T LLDP_PMGR_GetXMedRemLocationCivicAddrWhat(UI32_T rem_loc_port_num,
                                                 UI32_T rem_index,
                                                 UI8_T *what)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_grp_xmed_rem_what);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_GETXMEDREMLOCATIONCIVICADDRWHAT;
    msg_p->data.arg_grp_xmed_rem_what.arg_rem_loc_port_num = rem_loc_port_num;
    msg_p->data.arg_grp_xmed_rem_what.arg_rem_index = rem_index;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *what = msg_p->data.arg_grp_xmed_rem_what.arg_what;

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_Get1stXMedRemLocationCivicAddrCaEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get first civic addr entry.
 * INPUT    : rem_loc_port_num
 *            rem_index
 * OUTPUT   : ca_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : This API is used for CLI/WEB.
 *-------------------------------------------------------------------------
 */
BOOL_T LLDP_PMGR_Get1stXMedRemLocationCivicAddrCaEntry(UI32_T rem_loc_port_num,
                                                       UI32_T rem_index,
                                                       LLDP_MGR_XMedLocationCivicAddrCaEntry_T *ca_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_grp_xmed_rem_ca_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_GET1STXMEDREMLOCATIONCIVICADDRCAENTRY;
    msg_p->data.arg_grp_xmed_rem_ca_entry.arg_rem_loc_port_num = rem_loc_port_num;
    msg_p->data.arg_grp_xmed_rem_ca_entry.arg_rem_index = rem_index;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *ca_entry = msg_p->data.arg_grp_xmed_rem_ca_entry.arg_ca_entry;

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetNextXMedRemLocationCivicAddrCaEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get next civic addr entry.
 * INPUT    : rem_loc_port_num
 *            rem_index
 * OUTPUT   : ca_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : This API is used for CLI/WEB.
 *-------------------------------------------------------------------------
 */
BOOL_T LLDP_PMGR_GetNextXMedRemLocationCivicAddrCaEntry(UI32_T rem_loc_port_num,
                                                        UI32_T rem_index,
                                                        LLDP_MGR_XMedLocationCivicAddrCaEntry_T *ca_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_grp_xmed_rem_ca_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_GETNEXTXMEDREMLOCATIONCIVICADDRCAENTRY;
    msg_p->data.arg_grp_xmed_rem_ca_entry.arg_rem_loc_port_num = rem_loc_port_num;
    msg_p->data.arg_grp_xmed_rem_ca_entry.arg_rem_index = rem_index;
    msg_p->data.arg_grp_xmed_rem_ca_entry.arg_ca_entry = *ca_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *ca_entry = msg_p->data.arg_grp_xmed_rem_ca_entry.arg_ca_entry;

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetXMedRemLocationElin
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED remote device location elin entry
 * INPUT    : None
 * OUTPUT   : rem_loc_elin_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : This API are used for CLI/WEB.
 *-------------------------------------------------------------------------
 */
BOOL_T LLDP_PMGR_GetXMedRemLocationElin(UI32_T rem_loc_port_num,
                                        UI32_T rem_index,
                                        LLDP_MGR_XMedLocationElin_T *rem_loc_elin_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_grp_xmed_rem_loc_elin_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_GETXMEDREMLOCATIONELIN;
    msg_p->data.arg_grp_xmed_rem_loc_elin_entry.arg_rem_loc_port_num = rem_loc_port_num;
    msg_p->data.arg_grp_xmed_rem_loc_elin_entry.arg_rem_index = rem_index;
    msg_p->data.arg_grp_xmed_rem_loc_elin_entry.arg_rem_loc_elin_entry = *rem_loc_elin_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *rem_loc_elin_entry = msg_p->data.arg_grp_xmed_rem_loc_elin_entry.arg_rem_loc_elin_entry;

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetNextXMedRemPoeEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED remote device poe entry
 * INPUT    : None
 * OUTPUT   : rem_poe_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_PMGR_GetNextXMedRemPoeEntry(LLDP_MGR_XMedRemPoeEntry_T *rem_poe_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xmed_rem_poe_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_GETNEXTXMEDREMPOEENTRY;
    msg_p->data.arg_xmed_rem_poe_entry = *rem_poe_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *rem_poe_entry = msg_p->data.arg_xmed_rem_poe_entry;

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetNextXMedRemPoePseEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED remote device poe pse entry
 * INPUT    : None
 * OUTPUT   : rem_pse_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_PMGR_GetNextXMedRemPoePseEntry(LLDP_MGR_XMedRemPoePseEntry_T *rem_pse_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xmed_rem_pse_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_GETNEXTXMEDREMPOEPSEENTRY;
    msg_p->data.arg_xmed_rem_pse_entry = *rem_pse_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *rem_pse_entry = msg_p->data.arg_xmed_rem_pse_entry;

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetNextXMedRemPoePdEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED remote device poe pd entry
 * INPUT    : None
 * OUTPUT   : rem_pd_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_PMGR_GetNextXMedRemPoePdEntry(LLDP_MGR_XMedRemPoePdEntry_T *rem_pd_entry)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xmed_rem_pd_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_GETNEXTXMEDREMPOEPDENTRY;
    msg_p->data.arg_xmed_rem_pd_entry = *rem_pd_entry;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *rem_pd_entry = msg_p->data.arg_xmed_rem_pd_entry;

    return msg_p->type.ret_bool;
}
#endif

#if (LLDP_TYPE_DCBX == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_SetXdcbxPortConfig
 *-------------------------------------------------------------------------
 * PURPOSE  : Set DCBX port config
 * INPUT    : lport, xdcbx_port_config_entry_p
 * OUTPUT   : None
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_SetXdcbxPortConfig(LLDP_TYPE_XdcbxPortConfigEntry_T *xdcbx_port_config_entry_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xdcbx_port_config_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_SETXDCBXPORTCONFIGENTRY;
     memcpy(&msg_p->data.arg_xdcbx_port_config_entry, xdcbx_port_config_entry_p, sizeof(LLDP_TYPE_XdcbxPortConfigEntry_T));

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    return msg_p->type.ret_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetXdcbxPortConfig
 *-------------------------------------------------------------------------
 * PURPOSE  : Get DCBX port config
 * INPUT    : lport
 * OUTPUT   : xdcbx_port_config_entry_p
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetXdcbxPortConfig(LLDP_TYPE_XdcbxPortConfigEntry_T *xdcbx_port_config_entry_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xdcbx_port_config_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_GETXDCBXPORTCONFIGENTRY;
     memcpy(&msg_p->data.arg_xdcbx_port_config_entry, xdcbx_port_config_entry_p, sizeof(LLDP_TYPE_XdcbxPortConfigEntry_T));

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

     memcpy(xdcbx_port_config_entry_p, &msg_p->data.arg_xdcbx_port_config_entry, sizeof(LLDP_TYPE_XdcbxPortConfigEntry_T));
    return msg_p->type.ret_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetRunningXdcbxPortConfig
 *-------------------------------------------------------------------------
 * PURPOSE  : Get DCBX port config
 * INPUT    : lport
 * OUTPUT   : xdcbx_port_config_entry_p
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetRunningXdcbxPortConfig(LLDP_TYPE_XdcbxPortConfigEntry_T *xdcbx_port_config_entry_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xdcbx_port_config_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_GETRUNNINGXDCBXPORTCONFIGENTRY;
     memcpy(&msg_p->data.arg_xdcbx_port_config_entry, xdcbx_port_config_entry_p, sizeof(LLDP_TYPE_XdcbxPortConfigEntry_T));

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

     memcpy(xdcbx_port_config_entry_p, &msg_p->data.arg_xdcbx_port_config_entry, sizeof(LLDP_TYPE_XdcbxPortConfigEntry_T));
    return msg_p->type.ret_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetDcbxEtsRemoteData
 *-------------------------------------------------------------------------
 * PURPOSE  : Get DCBX ETS remote data
 * INPUT    : lport
 * OUTPUT   : rem_ets_entry_p
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetDcbxEtsRemoteData(LLDP_TYPE_DcbxRemEtsEntry_T *rem_ets_entry_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xdcbx_ets_rem_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_GETXDCBXETSREMOTEENTRY;
     memcpy(&msg_p->data.arg_xdcbx_ets_rem_entry, rem_ets_entry_p, sizeof(LLDP_TYPE_DcbxRemEtsEntry_T));

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

     memcpy(rem_ets_entry_p, &msg_p->data.arg_xdcbx_ets_rem_entry, sizeof(LLDP_TYPE_DcbxRemEtsEntry_T));
    return msg_p->type.ret_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetDcbxPfcRemoteData
 *-------------------------------------------------------------------------
 * PURPOSE  : Get DCBX PFC remote data
 * INPUT    : lport
 * OUTPUT   : rem_pfc_entry_p
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetDcbxPfcRemoteData(LLDP_TYPE_DcbxRemPfcEntry_T *rem_pfc_entry_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xdcbx_pfc_rem_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_GETXDCBXPFCREMOTEENTRY;
     memcpy(&msg_p->data.arg_xdcbx_pfc_rem_entry, rem_pfc_entry_p, sizeof(LLDP_TYPE_DcbxRemPfcEntry_T));

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

     memcpy(rem_pfc_entry_p, &msg_p->data.arg_xdcbx_pfc_rem_entry, sizeof(LLDP_TYPE_DcbxRemPfcEntry_T));
    return msg_p->type.ret_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetNextDcbxAppPriorityRemoteData
 *-------------------------------------------------------------------------
 * PURPOSE  : Get next DCBX Application priority remote data
 * INPUT    : lport
 * OUTPUT   : rem_app_pri_entry_p
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetNextDcbxAppPriorityRemoteData(LLDP_TYPE_DcbxRemAppPriEntry_T *rem_app_pri_entry_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xdcbx_app_pri_rem_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_GETNEXTXDCBXAPPPRIREMOTEENTRY;
     memcpy(&msg_p->data.arg_xdcbx_app_pri_rem_entry, rem_app_pri_entry_p, sizeof(LLDP_TYPE_DcbxRemAppPriEntry_T));

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

     memcpy(rem_app_pri_entry_p, &msg_p->data.arg_xdcbx_app_pri_rem_entry, sizeof(LLDP_TYPE_DcbxRemAppPriEntry_T));
    return msg_p->type.ret_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_NotifyEtsPfcCfgChanged
 *-------------------------------------------------------------------------
 * PURPOSE  : When ets/pfc cfg is changed, this function will be called.
 * INPUT    : lport       -- the specified port
 *            is_ets_chgd -- TRUE if ets is changed
 *            is_pfc_chgd -- TRUE if pfc is changed
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void LLDP_PMGR_NotifyEtsPfcCfgChanged(
    UI32_T  lport,
    BOOL_T  is_ets_chgd,
    BOOL_T  is_pfc_chgd)
{
    const UI32_T msg_size = LLDP_MGR_GET_MSG_SIZE(arg_grp_ui32_bool_bool);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LLDP_MGR_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LLDP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LLDP_MGR_IPC_NOTIFYETSPFCCFGCHANGED;
    msg_p->data.arg_grp_ui32_bool_bool.arg_ui32   = lport;
    msg_p->data.arg_grp_ui32_bool_bool.arg_bool_1 = is_ets_chgd;
    msg_p->data.arg_grp_ui32_bool_bool.arg_bool_2 = is_pfc_chgd;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return;
    }
}
#endif /* End of #if (LLDP_TYPE_DCBX == TRUE) */

