/*-----------------------------------------------------------------------------
 * FILE NAME: LACP_POM.C
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *    This file implements the APIs for LACP OM IPC.
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
#include "sys_bld.h"
#include "sys_module.h"
#include "sys_type.h"
#include "sysfun.h"
#include "lacp_om.h"
#include "lacp_pom.h"


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
 * FUNCTION NAME - LACP_POM_InitiateProcessResource
 *-----------------------------------------------------------------------------
 * PURPOSE : Initiate resource for LACP_POM in the calling process.
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
BOOL_T LACP_POM_InitiateProcessResource(void)
{
    /* get the ipc message queues for LACP OM
     */
    if (SYSFUN_GetMsgQ(SYS_BLD_L2_L4_PROC_OM_IPCMSGQ_KEY,
            SYSFUN_MSGQ_BIDIRECTIONAL, &ipcmsgq_handle) != SYSFUN_OK)
    {
        printf("\n%s(): L_IPCMSGQ_GetMsgQ fail.\n", __FUNCTION__);
        return FALSE;
    }

    return TRUE;
} /* End of LACP_POM_InitiateProcessResource */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LACP_POM_GetRunningDot3adLacpPortEnabled
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
UI32_T LACP_POM_GetRunningDot3adLacpPortEnabled(UI32_T ifindex, UI32_T *lacp_state)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LACP_OM_GET_MSG_SIZE(arg_grp1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LACP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LACP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LACP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LACP_OM_IPC_GETRUNNINGDOT3ADLACPPORTENABLED;
    msg_p->data.arg_grp1.arg1 = ifindex;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    *lacp_state = msg_p->data.arg_grp1.arg2;

    return msg_p->type.ret_ui32;
} /* End of LACP_POM_GetRunningDot3adLacpPortEnabled */

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
UI32_T  LACP_POM_GetRunningDot3adAggPortPartnerAdminSystemId(UI16_T port_index, UI8_T *system_id)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T msg_size = LACP_OM_GET_MSG_SIZE(arg_grp2);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    LACP_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_LACP;
    msgbuf_p->msg_size = msg_size;

    msg_p = (LACP_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = LACP_OM_IPC_GETRUNNINGDOT3ADAGGPORTPARTNERADMINSYSTEMID;
    msg_p->data.arg_grp2.arg1 = port_index;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    *system_id = msg_p->data.arg_grp2.arg2;

    return msg_p->type.ret_ui32;
} /* End of LACP_POM_GetRunningDot3adAggPortPartnerAdminSystemId */


/* LOCAL SUBPROGRAM BODIES
 */
