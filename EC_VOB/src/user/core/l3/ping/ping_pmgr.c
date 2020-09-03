/* Module Name: PING_PMGR.C
 * Purpose:
 *    Implements the APIs for IPCs with PING MGR.
 *
 * Notes:
 *    None.
 * History:
 *      Date        --  Modifier,   Reason
 *      2007/11/01  --  Timon,      Create
 *
 * Copyright(C)      Accton Corporation, 2007
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <string.h>
#include "sys_bld.h"
#include "sys_module.h"
#include "sys_type.h"
#include "l_mm.h"
#include "sysfun.h"
#include "ping_mgr.h"
#include "ping_pmgr.h"
#include "ping_type.h"
#include "l_inet.h" /* for L_INET_AddrType_T */

/* NAMING CONSTANT DECLARATIONS
 */

/* trace id definition when using L_MM
 */
enum
{
    PING_TYPE_TRACEID_PMGR_SETCTLENTRY
};


/* MACRO FUNCTION DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */


/* LOCAL SUBPROGRAM DECLARATIONS
 */


/* STATIC VARIABLE DEFINITIONS
 */
static SYSFUN_MsgQ_T ping_ipcmsgq_handle;


/* EXPORTED SUBPROGRAM BODIES
 */

/* FUNCTION NAME: PING_PMGR_InitiateProcessResources
 * PURPOSE:
 *          Initiate resources for PING_PMGR in the calling process.
 * INPUT:
 *          None.
 * OUTPUT:
 *          None.
 * RETURN:
 *          TRUE    -- Success
 *          FALSE   -- Fail
 * NOTES:
 *          None.
 */
BOOL_T PING_PMGR_InitiateProcessResources(void)
{
    /* get the ipc message queues for PING MGR
     */
    if (SYSFUN_GetMsgQ(SYS_BLD_APP_PROTOCOL_GROUP_IPCMSGQ_KEY,
            SYSFUN_MSGQ_BIDIRECTIONAL, &ping_ipcmsgq_handle) != SYSFUN_OK)
    {
        printf("\n%s(): L_IPCMSGQ_GetMsgQ fail.\n", __FUNCTION__);
        return FALSE;
    }

    return TRUE;
} /* End of PING_PMGR_InitiateProcessResources */

/* FUNCTION NAME : PING_PMGR_SetCtlEntry
 * PURPOSE:
 *          To create / destroy contrl entry
 * INPUT:
 *          ctrl_entry_p    -- the specific control entry.
 * OUTPUT:
 *          None
 * RETURN:
 *          PING_TYPE_OK
 *          PING_TYPE_INVALID_ARG
 *          PING_TYPE_NO_MORE_WORKSPACE
 *          PING_TYPE_FAIL
 * NOTES:
 *          1. Currently, only CAG and Destroy is supported.
 *             Operation for CAW and NotInService is not allowed.
 *          2. Currentlt, we only permit the setting of row_status, target_address and admin_status
 *          3. key: ping_ctl_owner_index, ping_ctl_test_name.
 */
UI32_T PING_PMGR_SetCtlEntry(PING_TYPE_PingCtlEntry_T *ctrl_entry_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = PING_MGR_GET_MSG_SIZE(ctrl_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    PING_MGR_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_PING;
    msgbuf_p->msg_size = msg_size;

    msg_p = (PING_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = PING_MGR_IPCCMD_SETCTLENTRY;

    memcpy(&(msg_p->data.ctrl_entry), ctrl_entry_p, sizeof(PING_TYPE_PingCtlEntry_T));

    if (SYSFUN_SendRequestMsg(ping_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return PING_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;
}

/* FUNCTION NAME: PING_PMGR_SetCtlAdminStatus
 * PURPOSE:
 *          To enable or disable ping control entry
 * INPUT:
 *          ctl_entry_p         -- the specific control entry.
 *          ctrl_admin_status   -- the admin status of the to enable or disable the ping.
 * OUTPUT:
 *          None
 * RETURN:
 *          PING_TYPE_OK
 *          PING_TYPE_INVALID_ARG
 *          PING_TYPE_NO_MORE_WORKSPACE
 *          PING_TYPE_FAIL
 * NOTES:
 *          1.This API is only used by "set by filed", not "set by record".
 */
UI32_T PING_PMGR_SetCtlAdminStatus(PING_TYPE_PingCtlEntry_T *ctl_entry_p , UI32_T ctrl_admin_status)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = PING_MGR_GET_MSG_SIZE(ctrl_entry_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    PING_MGR_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_PING;
    msgbuf_p->msg_size = msg_size;

    msg_p = (PING_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = PING_MGR_IPCCMD_SETCTLADMINSTATUS;

    memcpy(&(msg_p->data.ctrl_entry_ui32.ctrl_entry), ctl_entry_p, sizeof(PING_TYPE_PingCtlEntry_T));
    msg_p->data.ctrl_entry_ui32.ui32 = ctrl_admin_status;

   if (SYSFUN_SendRequestMsg(ping_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return PING_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;
}

/* FUNCTION NAME: PING_PMGR_SetCtlTargetAddress
 * PURPOSE:
 *          To set the target address field for ping control entry
 * INPUT:
 *          ctl_entry_p     -- the pointer of specific control entry.
 *          taget_addr      -- the target address of the remote host.
 * OUTPUT:
 *          None
 * RETURN:
 *          PING_TYPE_OK
 *          PING_TYPE_INVALID_ARG
 *          PING_TYPE_FAIL
 * NOTES:
 *          1. We can not change the target address for the specified index when admin_status is enabled.
 *          2. Currently we do not support the domain name query of the target address.
 */
UI32_T PING_PMGR_SetCtlTargetAddress(PING_TYPE_PingCtlEntry_T *ctl_entry_p, L_INET_AddrIp_T* target_addr_p)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = PING_MGR_GET_MSG_SIZE(ctrl_entry_addr);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    PING_MGR_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_PING;
    msgbuf_p->msg_size = msg_size;

    msg_p = (PING_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = PING_MGR_IPCCMD_SETCTLTARGETADDRESS;

    memcpy(&(msg_p->data.ctrl_entry_addr.ctrl_entry), ctl_entry_p, sizeof(PING_TYPE_PingCtlEntry_T));
    memcpy(&(msg_p->data.ctrl_entry_addr.addr), target_addr_p, sizeof(L_INET_AddrIp_T));
   if (SYSFUN_SendRequestMsg(ping_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return PING_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;
}

/* FUNCTION NAME: PING_PMGR_SetCtlDataSize
 * PURPOSE:
 *          To set the data size field for ping control entry
 * INPUT:
 *          ctl_entry_p     -- the pointer of specific control entry.
 *          data_size       -- the size of data portion in ICMP pkt.
 * OUTPUT:
 *          None
 * RETURN:
 *          PING_TYPE_OK
 *          PING_TYPE_INVALID_ARG
 *          PING_TYPE_FAIL
 * NOTES:
 *          1. We can not change the data size for the specified index when admin_status is enabled.
 *          2. SNMP range: 0..65507, CLI range: 32-512.
 */
UI32_T PING_PMGR_SetCtlDataSize(PING_TYPE_PingCtlEntry_T *ctl_entry_p, UI32_T data_size)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = PING_MGR_GET_MSG_SIZE(ctrl_entry_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    PING_MGR_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_PING;
    msgbuf_p->msg_size = msg_size;

    msg_p = (PING_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = PING_MGR_IPCCMD_SETCTLDATASIZE;

    memcpy(&(msg_p->data.ctrl_entry_ui32.ctrl_entry), ctl_entry_p, sizeof(PING_TYPE_PingCtlEntry_T));
    msg_p->data.ctrl_entry_ui32.ui32 = data_size;

   if (SYSFUN_SendRequestMsg(ping_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return PING_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;
}

/* FUNCTION NAME: PING_PMGR_SetCtlProbeCount
 * PURPOSE:
 *          To set the probe count field for ping control entry
 * INPUT:
 *          ctl_entry_p     -- the pointer of specific control entry.
 *          probe_count     -- the number of ping packet
 * OUTPUT:
 *          None
 * RETURN:
 *          PING_TYPE_OK
 *          PING_TYPE_INVALID_ARG
 *          PING_TYPE_FAIL
 * NOTES:
 *          1. We can not change the probe count for the specified index when admin_status is enabled.
 *          2. SNMP range: 1-15, CLI range: 1-16. So 0 is not allowed.
 */
UI32_T PING_PMGR_SetCtlProbeCount(PING_TYPE_PingCtlEntry_T *ctl_entry_p, UI32_T probe_count)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = PING_MGR_GET_MSG_SIZE(ctrl_entry_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    PING_MGR_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_PING;
    msgbuf_p->msg_size = msg_size;

    msg_p = (PING_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = PING_MGR_IPCCMD_SETCTLPROBECOUNT;

    memcpy(&(msg_p->data.ctrl_entry_ui32.ctrl_entry), ctl_entry_p, sizeof(PING_TYPE_PingCtlEntry_T));
    msg_p->data.ctrl_entry_ui32.ui32 = probe_count;

   if (SYSFUN_SendRequestMsg(ping_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return PING_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;
}

/* FUNCTION NAME: PING_PMGR_SetCtlRowStatus
 * PURPOSE:
 *          To set row status field for ping control entry
 * INPUT:
 *          ctl_entry_p     -- the  specific control entry.
 *          ctrl_row_status -- the row status of the specified control entry.
 * OUTPUT:
 *          None
 * RETURN:
 *          PING_TYPE_OK
 *          PING_TYPE_INVALID_ARG
 *          PING_TYPE_NO_MORE_WORKSPACE
 *          PING_TYPE_FAIL
 * NOTES:
 *          1. The PingCtlEntry should not be modified. So we create a local entry for local use.
 */
UI32_T  PING_PMGR_SetCtlRowStatus(PING_TYPE_PingCtlEntry_T *ctl_entry_p, UI32_T ctrl_row_status)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = PING_MGR_GET_MSG_SIZE(ctrl_entry_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    PING_MGR_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_PING;
    msgbuf_p->msg_size = msg_size;

    msg_p = (PING_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = PING_MGR_IPCCMD_SETCTLROWSTATUS;

    memcpy(&(msg_p->data.ctrl_entry_ui32.ctrl_entry), ctl_entry_p, sizeof(PING_TYPE_PingCtlEntry_T));
    msg_p->data.ctrl_entry_ui32.ui32 = ctrl_row_status;

   if (SYSFUN_SendRequestMsg(ping_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return PING_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;

}

/* FUNCTION NAME: PING_PMGR_SetCtlEntryByField
 * PURPOSE:
 *          Set only the field of the entry.
 * INPUT:
 *          ctl_entry_p -- the pointer of the specified ctl entry.
 *          field       -- field.
 * OUTPUT:
 *          None.
 * RETURN:
 *          PING_TYPE_OK
 *          PING_TYPE_FAIL
 *          PING_TYPE_INVALID_ARG
 * NOTES:
 *          1. set only the field of the entry.
 */
UI32_T PING_PMGR_SetCtlEntryByField(PING_TYPE_PingCtlEntry_T *ctl_entry_p, PING_TYPE_CtlEntryField_T field)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = PING_MGR_GET_MSG_SIZE(ctrl_entry_field);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    PING_MGR_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_PING;
    msgbuf_p->msg_size = msg_size;

    msg_p = (PING_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = PING_MGR_IPCCMD_SETCTLENTRYBYFIELD;

    memset(&msg_p->data.ctrl_entry_field, 0, sizeof(msg_p->data.ctrl_entry_field));
    memcpy(&(msg_p->data.ctrl_entry_field.ctrl_entry), ctl_entry_p, sizeof(PING_TYPE_PingCtlEntry_T));
    msg_p->data.ctrl_entry_field.field = field;

   if (SYSFUN_SendRequestMsg(ping_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return PING_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;

}

