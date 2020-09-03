#ifndef UNIT_TEST
#include "mldsnp_pmgr.h"
#include "mldsnp_mgr.h"

#define MLDSNP_PMGR_DEBUG_ENABLE FALSE

#if (MLDSNP_PMGR_DEBUG_ENABLE==TRUE)

#define MLDSNP_PMGR_DEBUG_LINE() \
{  \
  printf("\r\n%s(%d)",__FUNCTION__,__LINE__); \
  fflush(stdout); \
}

#define MLDSNP_PMGR_DEBUG_MSG(a,b...)   \
{ \
  printf(a,##b); \
  fflush(stdout); \
}
#else
#define MLDSNP_PMGR_DEBUG_LINE()
#define MLDSNP_PMGR_DEBUG_MSG(a,...)
#endif

/* STATIC VARIABLE DECLARATIONS
 */
static SYSFUN_MsgQ_T mldsnp_pmgr_ipcmsgq_handle = 0;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_PMGR_SendMsg
 *-------------------------------------------------------------------------
 * PURPOSE : To send an IPC message.
 * INPUT   : cmd       - the MLDSNP message command.
 *           msg_p     - the buffer of the IPC message.
 *           req_size  - the size of MLDSNP request message.
 *           res_size  - the size of MLDSNP response message.
 *           ret_val   - the return value to set when IPC is failed.
 * OUTPUT  : msg_p     - the response message.
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
static void MLDSNP_PMGR_SendMsg(UI32_T cmd, SYSFUN_Msg_T *msg_p, UI32_T req_size, UI32_T res_size, UI32_T ret_val)
{
    UI32_T ret;

    msg_p->cmd = SYS_MODULE_MLDSNP;
    msg_p->msg_size = req_size;

    MLDSNP_MGR_MSG_CMD(msg_p) = cmd;

    ret = SYSFUN_SendRequestMsg(mldsnp_pmgr_ipcmsgq_handle,
                                msg_p,
                                SYSFUN_TIMEOUT_WAIT_FOREVER,
                                SYSFUN_SYSTEM_EVENT_IPCMSG,
                                res_size,
                                msg_p);

    /* If IPC is failed, set return value as ret_val.
     */
    if (ret != SYSFUN_OK)
    {
        printf("\r\n%s():Fail to IPC send/response message: %d", __FUNCTION__, (int)ret);
        MLDSNP_MGR_MSG_RETVAL(msg_p) = ret_val;
    }

    if (MLDSNP_MGR_MSG_RETVAL(msg_p) == MLDSNP_MGR_IPC_RESULT_FAIL)
        MLDSNP_MGR_MSG_RETVAL(msg_p) = ret_val;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : MLDSNP_PMGR_InitiateProcessResources
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Do initialization procedures for MLDSNP_PMGR.
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
void MLDSNP_PMGR_InitiateProcessResources(void)
{
    /* Given that L4 PMGR requests are handled in L4GROUP of L2_L4_PROC
     */
    MLDSNP_PMGR_DEBUG_LINE();
    if (SYSFUN_GetMsgQ(SYS_BLD_L2MCAST_GROUP_IPCMSGQ_KEY,
                       SYSFUN_MSGQ_BIDIRECTIONAL, &mldsnp_pmgr_ipcmsgq_handle) != SYSFUN_OK)
    {
        printf("%s(): SYSFUN_GetMsgQ fail.\n", __FUNCTION__);
    }
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_PMGR_AddPortStaticJoinGroup
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to set static port join group
 * INPUT   : vid        - the vlan id
 *          *gip_ap     - the group ip
 *          *sip_ap     - the source ip
 *          lport       - the static port list
 *          rec_type      - include or exclude
 * OUTPUT  : None
 * RETURN  : TRUE  - success
 *           FALSE - failure
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
UI32_T MLDSNP_PMGR_AddPortStaticJoinGroup(
    UI32_T vid,
    UI8_T *gip_ap,
    UI8_T *sip_ap,
    UI32_T lport,
    MLDSNP_TYPE_CurrentMode_T rec_type)
{
    const UI32_T msg_buf_size = (sizeof(((MLDSNP_MGR_IPCMsg_T *)0)->data.port_join_group)
                                 + MLDSNP_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    MLDSNP_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size, resp_size;
    MLDSNP_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size = msg_buf_size;
    resp_size = MLDSNP_MGR_MSGBUF_TYPE_SIZE;

    msg_p = (SYSFUN_Msg_T *) & space_msg;
    msg_p->cmd = SYS_MODULE_MLDSNP;
    msg_p->msg_size = req_size;

    msg_data_p = (MLDSNP_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = MLDSNP_MGR_IPCCMD_ADDPORTSTATICJOINGROUP;

    /*assign input*/
    msg_data_p->data.port_join_group.vid = vid;
    msg_data_p->data.port_join_group.lport = lport;
    memcpy(msg_data_p->data.port_join_group.gip_ap,
           gip_ap, sizeof(msg_data_p->data.port_join_group.gip_ap));
    memcpy(msg_data_p->data.port_join_group.sip_ap,
           sip_ap, sizeof(msg_data_p->data.port_join_group.sip_ap));
    msg_data_p->data.port_join_group.mode = rec_type;
    /*send ipc*/
    if (SYSFUN_SendRequestMsg(mldsnp_pmgr_ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
                              SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p) != SYSFUN_OK)
    {
        MLDSNP_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    MLDSNP_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_PMGR_AddStaticRouterPort
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to add the static router port
 * INPUT   : vid        - the vlan id
 *           lport      - the logical port
 * OUTPUT  : None
 * RETURN  : TRUE  - success
 *           FALSE - failure
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_PMGR_AddStaticRouterPort(
    UI32_T vid,
    UI32_T lport)
{
    const UI32_T msg_buf_size = (sizeof(((MLDSNP_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
                                 + MLDSNP_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    MLDSNP_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size, resp_size;
    MLDSNP_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size = msg_buf_size;
    resp_size = MLDSNP_MGR_MSGBUF_TYPE_SIZE;

    msg_p = (SYSFUN_Msg_T *) & space_msg;
    msg_p->cmd = SYS_MODULE_MLDSNP;
    msg_p->msg_size = req_size;

    msg_data_p = (MLDSNP_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = MLDSNP_MGR_IPCCMD_ADDSTATICROUTERPORT;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1 = vid;
    msg_data_p->data.u32a1_u32a2.u32_a2 = lport;

    /*send ipc*/
    if (SYSFUN_SendRequestMsg(mldsnp_pmgr_ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
                              SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p) != SYSFUN_OK)
    {
        MLDSNP_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    MLDSNP_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_PMGR_DeletePortStaticJoinGroup
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to delete static port from group
 * INPUT   : vid        - the vlan id
 *           *gip_ap    - the group ip
 *           *sip_ap    - the source ip
 *           lport      - the logical port
 * OUTPUT  : None
 * RETURN  : TRUE  - success
 *           FALSE - failure
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
UI32_T MLDSNP_PMGR_DeletePortStaticJoinGroup(
    UI32_T vid,
    UI8_T  *gip_ap,
    UI8_T  *sip_ap,
    UI32_T lport)
{
    const UI32_T msg_buf_size = (sizeof(((MLDSNP_MGR_IPCMsg_T *)0)->data.port_join_group)
                                 + MLDSNP_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    MLDSNP_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size, resp_size;
    MLDSNP_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size = msg_buf_size;
    resp_size = MLDSNP_MGR_MSGBUF_TYPE_SIZE;

    msg_p = (SYSFUN_Msg_T *) & space_msg;
    msg_p->cmd = SYS_MODULE_MLDSNP;
    msg_p->msg_size = req_size;

    msg_data_p = (MLDSNP_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = MLDSNP_MGR_IPCCMD_DELETEPORTSTATICJOINGROUP;

    /*assign input*/
    msg_data_p->data.port_join_group.vid = vid;
    msg_data_p->data.port_join_group.lport = lport;
    memcpy(msg_data_p->data.port_join_group.gip_ap,
           gip_ap, sizeof(msg_data_p->data.port_join_group.gip_ap));
    memcpy(msg_data_p->data.port_join_group.sip_ap,
           sip_ap, sizeof(msg_data_p->data.port_join_group.sip_ap));

    /*send ipc*/
    if (SYSFUN_SendRequestMsg(mldsnp_pmgr_ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
                              SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p) != SYSFUN_OK)
    {
        MLDSNP_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    MLDSNP_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_PMGR_DeleteStaticRouterPort
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to delete the static router port
 * INPUT   : vid        - the vlan id
 *           lport      - the logical port
 * OUTPUT  : None
 * RETURN  : TRUE  - success
 *           FALSE - failure
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_PMGR_DeleteStaticRouterPort(
    UI32_T vid,
    UI32_T lport)
{
    const UI32_T msg_buf_size = (sizeof(((MLDSNP_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
                                 + MLDSNP_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    MLDSNP_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size, resp_size;
    MLDSNP_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size = msg_buf_size;
    resp_size = MLDSNP_MGR_MSGBUF_TYPE_SIZE;

    msg_p = (SYSFUN_Msg_T *) & space_msg;
    msg_p->cmd = SYS_MODULE_MLDSNP;
    msg_p->msg_size = req_size;

    msg_data_p = (MLDSNP_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = MLDSNP_MGR_IPCCMD_DELETESTATICROUTERPORT;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1 = vid;
    msg_data_p->data.u32a1_u32a2.u32_a2 = lport;

    /*send ipc*/
    if (SYSFUN_SendRequestMsg(mldsnp_pmgr_ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
                              SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p) != SYSFUN_OK)
    {
        MLDSNP_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    MLDSNP_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_ui32;
}

/*------------------------------------------------------------------------------
* Function : MLDSNP_PMGR_SetImmediateLeaveStatus
*------------------------------------------------------------------------------
* Purpose: This function Set the immediate leave status
* INPUT  : vid                    - the vlan id
*          immediate_leave_status - the returned router port info
*
* OUTPUT : None
* RETURN : MLDSNP_TYEP_RETURN_FAIL
*          MLDSNP_TYEP_RETURN_SUCCESS
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_PMGR_SetImmediateLeaveStatus(
    UI32_T vid,
    UI32_T immediate_leave_status)
{
    const UI32_T msg_buf_size = (sizeof(((MLDSNP_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
                                 + MLDSNP_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    MLDSNP_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size, resp_size;
    MLDSNP_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size = msg_buf_size;
    resp_size = MLDSNP_MGR_MSGBUF_TYPE_SIZE;

    msg_p = (SYSFUN_Msg_T *) & space_msg;
    msg_p->cmd = SYS_MODULE_MLDSNP;
    msg_p->msg_size = req_size;

    msg_data_p = (MLDSNP_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = MLDSNP_MGR_IPCCMD_SETIMMEDIATELEAVESTATUS;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1 = vid;
    msg_data_p->data.u32a1_u32a2.u32_a2 = immediate_leave_status;

    /*send ipc*/
    if (SYSFUN_SendRequestMsg(mldsnp_pmgr_ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
                              SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p) != SYSFUN_OK)
    {
        MLDSNP_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    MLDSNP_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_ui32;
}

/*------------------------------------------------------------------------------
* Function : MLDSNP_PMGR_SetImmediateLeaveStatusByHostStatus
*------------------------------------------------------------------------------
* Purpose: This function Set the immediate leave status by-host-ip
* INPUT  : vid                    - the vlan id
*          immediate_leave_byhost_status - the immediate leave by host status
* OUTPUT : None
* RETURN : MLDSNP_TYEP_RETURN_FAIL
*          MLDSNP_TYEP_RETURN_SUCCESS
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_PMGR_SetImmediateLeaveByHostStatus(
    UI32_T vid,
    UI32_T immediate_leave_byhost_status)
{
    const UI32_T msg_buf_size = (sizeof(((MLDSNP_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
                                 + MLDSNP_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    MLDSNP_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size, resp_size;
    MLDSNP_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size = msg_buf_size;
    resp_size = MLDSNP_MGR_MSGBUF_TYPE_SIZE;

    msg_p = (SYSFUN_Msg_T *) & space_msg;
    msg_p->cmd = SYS_MODULE_MLDSNP;
    msg_p->msg_size = req_size;

    msg_data_p = (MLDSNP_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = MLDSNP_MGR_IPCCMD_SETIMMEDIATELEAVEBYHOSTSTATUS;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1 = vid;
    msg_data_p->data.u32a1_u32a2.u32_a2 = immediate_leave_byhost_status;

    /*send ipc*/
    if (SYSFUN_SendRequestMsg(mldsnp_pmgr_ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
                              SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p) != SYSFUN_OK)
    {
        MLDSNP_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    MLDSNP_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_ui32;
}


/*------------------------------------------------------------------------------
* Function : MLDSNP_PMGR_SetLastListenerQueryInterval
*------------------------------------------------------------------------------
* Purpose: This function set the last listener query interval
* INPUT  : interval  - the interval in second
* OUTPUT : None
* RETURN : MLDSNP_TYEP_RETURN_FAIL
*          MLDSNP_TYEP_RETURN_SUCCESS
* NOTES  :  7.8, 9.8
*------------------------------------------------------------------------------*/
UI32_T MLDSNP_PMGR_SetLastListenerQueryInterval(
    UI32_T interval)
{
    const UI32_T msg_buf_size = (sizeof(((MLDSNP_MGR_IPCMsg_T *)0)->data.ui32_v)
                                 + MLDSNP_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    MLDSNP_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size, resp_size;
    MLDSNP_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size = msg_buf_size;
    resp_size = MLDSNP_MGR_MSGBUF_TYPE_SIZE;

    msg_p = (SYSFUN_Msg_T *) & space_msg;
    msg_p->cmd = SYS_MODULE_MLDSNP;
    msg_p->msg_size = req_size;

    msg_data_p = (MLDSNP_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = MLDSNP_MGR_IPCCMD_SETLASTLISTENERQUERYINTERVAL;

    /*assign input*/
    msg_data_p->data.ui32_v = interval;

    /*send ipc*/
    if (SYSFUN_SendRequestMsg(mldsnp_pmgr_ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
                              SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p) != SYSFUN_OK)
    {
        MLDSNP_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    MLDSNP_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_ui32;
}

/*------------------------------------------------------------------------------
* Function : MLDSNP_PMGR_SetMldSnpVer
*------------------------------------------------------------------------------
* Purpose: This function set the mldsno version
* INPUT  : ver
* OUTPUT : None
* RETURN : MLDSNP_TYEP_RETURN_FAIL
*          MLDSNP_TYEP_RETURN_SUCCESS
* NOTES  :
*------------------------------------------------------------------------------*/
UI32_T MLDSNP_PMGR_SetMldSnpVer(
    UI32_T ver)
{
    const UI32_T msg_buf_size = (sizeof(((MLDSNP_MGR_IPCMsg_T *)0)->data.ui32_v)
                                 + MLDSNP_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    MLDSNP_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size, resp_size;
    MLDSNP_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size = msg_buf_size;
    resp_size = MLDSNP_MGR_MSGBUF_TYPE_SIZE;

    msg_p = (SYSFUN_Msg_T *) & space_msg;
    msg_p->cmd = SYS_MODULE_MLDSNP;
    msg_p->msg_size = req_size;

    msg_data_p = (MLDSNP_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = MLDSNP_MGR_IPCCMD_SETMLDSNPVER;

    /*assign input*/
    msg_data_p->data.ui32_v = ver;
    MLDSNP_PMGR_DEBUG_LINE();
    /*send ipc*/
    if (SYSFUN_SendRequestMsg(mldsnp_pmgr_ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
                              SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p) != SYSFUN_OK)
    {
        MLDSNP_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    MLDSNP_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_ui32;
}

/*------------------------------------------------------------------------------
* Function : MLDSNP_PMGR_SetMldStatus
*------------------------------------------------------------------------------
* Purpose: This function set the Mld status
* INPUT  : mldsnp_status - the mldsnp status
* OUTPUT : None
* RETURN : MLDSNP_TYEP_RETURN_FAIL
*          MLDSNP_TYEP_RETURN_SUCCESS
* NOTES  :
*------------------------------------------------------------------------------*/
UI32_T MLDSNP_PMGR_SetMldStatus(
    UI32_T mldsnp_status)
{
    const UI32_T msg_buf_size = (sizeof(((MLDSNP_MGR_IPCMsg_T *)0)->data.ui32_v)
                                 + MLDSNP_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    MLDSNP_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size, resp_size;
    MLDSNP_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size = msg_buf_size;
    resp_size = MLDSNP_MGR_MSGBUF_TYPE_SIZE;

    msg_p = (SYSFUN_Msg_T *) & space_msg;
    msg_p->cmd = SYS_MODULE_MLDSNP;
    msg_p->msg_size = req_size;

    msg_data_p = (MLDSNP_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = MLDSNP_MGR_IPCCMD_SETMLDSTATUS;

    /*assign input*/
    msg_data_p->data.ui32_v = mldsnp_status;

    /*send ipc*/
    if (SYSFUN_SendRequestMsg(mldsnp_pmgr_ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
                              SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p) != SYSFUN_OK)
    {
        MLDSNP_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    MLDSNP_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_ui32;
}

/*------------------------------------------------------------------------------
* Function : MLDSNP_PMGR_SetQuerierStatus
*------------------------------------------------------------------------------
* Purpose: This function set the querier status
* INPUT  : querier_status - the querier status
* OUTPUT : None
* RETURN : MLDSNP_TYPE_RETURN_FAIL
*          MLDSNP_TYPE_RETURN_SUCCESS
* NOTES  :
*------------------------------------------------------------------------------*/
UI32_T MLDSNP_PMGR_SetQuerierStatus(
    UI32_T querier_status)
{
    const UI32_T msg_buf_size = (sizeof(((MLDSNP_MGR_IPCMsg_T *)0)->data.ui32_v)
                                 + MLDSNP_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    MLDSNP_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size, resp_size;
    MLDSNP_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size = msg_buf_size;
    resp_size = MLDSNP_MGR_MSGBUF_TYPE_SIZE;

    msg_p = (SYSFUN_Msg_T *) & space_msg;
    msg_p->cmd = SYS_MODULE_MLDSNP;
    msg_p->msg_size = req_size;

    msg_data_p = (MLDSNP_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = MLDSNP_MGR_IPCCMD_SETQUERIERSTATUS;

    /*assign input*/
    msg_data_p->data.ui32_v = querier_status;

    /*send ipc*/
    if (SYSFUN_SendRequestMsg(mldsnp_pmgr_ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
                              SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p) != SYSFUN_OK)
    {
        MLDSNP_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    MLDSNP_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_ui32;
}

/*------------------------------------------------------------------------------
* Function : MLDSNP_PMGR_SetQueryInterval
*------------------------------------------------------------------------------
* Purpose: This funcgtion set the query interval
* INPUT  :  interval - the interval in seconds
* OUTPUT : None
* RETURN : MLDSNP_TYEP_RETURN_FAIL
*          MLDSNP_TYEP_RETURN_SUCCESS
* NOTES  :   7.2, 9.2
*------------------------------------------------------------------------------*/
UI32_T MLDSNP_PMGR_SetQueryInterval(
    UI32_T interval)
{
    const UI32_T msg_buf_size = (sizeof(((MLDSNP_MGR_IPCMsg_T *)0)->data.ui32_v)
                                 + MLDSNP_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    MLDSNP_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size, resp_size;
    MLDSNP_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size = msg_buf_size;
    resp_size = MLDSNP_MGR_MSGBUF_TYPE_SIZE;

    msg_p = (SYSFUN_Msg_T *) & space_msg;
    msg_p->cmd = SYS_MODULE_MLDSNP;
    msg_p->msg_size = req_size;

    msg_data_p = (MLDSNP_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = MLDSNP_MGR_IPCCMD_SETQUERYINTERVAL;

    /*assign input*/
    msg_data_p->data.ui32_v = interval;

    /*send ipc*/
    if (SYSFUN_SendRequestMsg(mldsnp_pmgr_ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
                              SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p) != SYSFUN_OK)
    {
        MLDSNP_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    MLDSNP_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_ui32;
}

/*------------------------------------------------------------------------------
* Function : MLDSNP_PMGR_SetQueryResponseInterval
*------------------------------------------------------------------------------
* Purpose: This function get the query response interval
* INPUT  : interval - the query repsonse interval in seconds
* OUTPUT : None
* RETURN : MLDSNP_TYEP_RETURN_FAIL
*          MLDSNP_TYEP_RETURN_SUCCESS
* NOTES  :   7.3, 9.3
*------------------------------------------------------------------------------*/
UI32_T MLDSNP_PMGR_SetQueryResponseInterval(
    UI32_T interval)
{
    const UI32_T msg_buf_size = (sizeof(((MLDSNP_MGR_IPCMsg_T *)0)->data.ui32_v)
                                 + MLDSNP_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    MLDSNP_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size, resp_size;
    MLDSNP_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size = msg_buf_size;
    resp_size = MLDSNP_MGR_MSGBUF_TYPE_SIZE;

    msg_p = (SYSFUN_Msg_T *) & space_msg;
    msg_p->cmd = SYS_MODULE_MLDSNP;
    msg_p->msg_size = req_size;

    msg_data_p = (MLDSNP_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = MLDSNP_MGR_IPCCMD_SETQUERYRESPONSEINTERVAL;

    /*assign input*/
    msg_data_p->data.ui32_v = interval;

    /*send ipc*/
    if (SYSFUN_SendRequestMsg(mldsnp_pmgr_ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
                              SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p) != SYSFUN_OK)
    {
        MLDSNP_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    MLDSNP_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_ui32;
}

/*------------------------------------------------------------------------------
* Function : MLDSNP_PMGR_SetRobustnessValue
*------------------------------------------------------------------------------
* Purpose: This function set the robust ess value
* INPUT  : value - the robustness value
* OUTPUT : None
* RETURN : MLDSNP_TYEP_RETURN_FAIL
*          MLDSNP_TYEP_RETURN_SUCCESS
* NOTES  :  7.1, 7.9, 9.1, 9.9
*------------------------------------------------------------------------------*/
UI32_T MLDSNP_PMGR_SetRobustnessValue(
    UI32_T value)
{
    const UI32_T msg_buf_size = (sizeof(((MLDSNP_MGR_IPCMsg_T *)0)->data.ui32_v)
                                 + MLDSNP_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    MLDSNP_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size, resp_size;
    MLDSNP_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size = msg_buf_size;
    resp_size = MLDSNP_MGR_MSGBUF_TYPE_SIZE;

    msg_p = (SYSFUN_Msg_T *) & space_msg;
    msg_p->cmd = SYS_MODULE_MLDSNP;
    msg_p->msg_size = req_size;

    msg_data_p = (MLDSNP_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = MLDSNP_MGR_IPCCMD_SETROBUSTNESSVALUE;

    /*assign input*/
    msg_data_p->data.ui32_v = value;

    /*send ipc*/
    if (SYSFUN_SendRequestMsg(mldsnp_pmgr_ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
                              SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p) != SYSFUN_OK)
    {
        MLDSNP_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    MLDSNP_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_ui32;
}

/*------------------------------------------------------------------------------
* Function : MLDSNP_PMGR_SetRouterExpireTime
*------------------------------------------------------------------------------
* Purpose: This function set the router expire time
* INPUT  : exp_time  - the expire time
* OUTPUT : None
* RETURN : MLDSNP_TYEP_RETURN_FAIL
*          MLDSNP_TYEP_RETURN_SUCCESS
* NOTES  :
*------------------------------------------------------------------------------*/
UI32_T MLDSNP_PMGR_SetRouterExpireTime(
    UI32_T exp_time)
{
    const UI32_T msg_buf_size = (sizeof(((MLDSNP_MGR_IPCMsg_T *)0)->data.ui32_v)
                                 + MLDSNP_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    MLDSNP_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size, resp_size;
    MLDSNP_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size = msg_buf_size;
    resp_size = MLDSNP_MGR_MSGBUF_TYPE_SIZE;

    msg_p = (SYSFUN_Msg_T *) & space_msg;
    msg_p->cmd = SYS_MODULE_MLDSNP;
    msg_p->msg_size = req_size;

    msg_data_p = (MLDSNP_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = MLDSNP_MGR_IPCCMD_SETROUTEREXPIRETIME;

    /*assign input*/
    msg_data_p->data.ui32_v = exp_time;

    /*send ipc*/
    if (SYSFUN_SendRequestMsg(mldsnp_pmgr_ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
                              SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p) != SYSFUN_OK)
    {
        MLDSNP_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    MLDSNP_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_ui32;
}

/*------------------------------------------------------------------------------
* Function : MLDSNP_PMGR_SetUnknownFloodBehavior
*------------------------------------------------------------------------------
* Purpose: This function Set the unknown multicast data flood behavior
* INPUT  :  flood_behavior - the returned router port info
*           vlan_id        - which vlan to configure
* OUTPUT : None
* RETURN : MLDSNP_TYEP_RETURN_FAIL
*          MLDSNP_TYEP_RETURN_SUCCESS
* NOTES  : vlan_id =0 means all vlan
*------------------------------------------------------------------------------*/
UI32_T MLDSNP_PMGR_SetUnknownFloodBehavior(UI32_T vlan_id,
    UI32_T flood_behavior)
{
    const UI32_T msg_buf_size = (sizeof(((MLDSNP_MGR_IPCMsg_T *)0)->data.u32a1_u32a2)
                                 + MLDSNP_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    MLDSNP_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size, resp_size;
    MLDSNP_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size = msg_buf_size;
    resp_size = MLDSNP_MGR_MSGBUF_TYPE_SIZE;

    msg_p = (SYSFUN_Msg_T *) & space_msg;
    msg_p->cmd = SYS_MODULE_MLDSNP;
    msg_p->msg_size = req_size;

    msg_data_p = (MLDSNP_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = MLDSNP_MGR_IPCCMD_SETUNKNOWNFLOODBEHAVIOR;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2.u32_a1 = vlan_id;
    msg_data_p->data.u32a1_u32a2.u32_a2 = flood_behavior;

    /*send ipc*/
    if (SYSFUN_SendRequestMsg(mldsnp_pmgr_ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
                              SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p) != SYSFUN_OK)
    {
        MLDSNP_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    MLDSNP_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_PMGR_SetPortListStaticJoinGroup
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to set static port join group
 * INPUT   : vid           - the vlan id
 *           *gip_ap       - the group ip
 *           *sip_ap       - the source ip
 *           *port_list_ap - the static port list
 * OUTPUT  : None
 * RETURN  : MLDSNP_TYPE_RETURN_SUCCESS  - success
 *           MLDSNP_TYPE_RETURN_FAIL - failure
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
UI32_T MLDSNP_PMGR_SetPortListStaticJoinGroup(
    UI32_T vid,
    UI8_T  *gip_ap,
    UI8_T  *sip_ap,
    UI8_T  *port_list_ap)
{
    const UI32_T msg_buf_size = (sizeof(((MLDSNP_MGR_IPCMsg_T *)0)->data.static_portlist)
                                 + MLDSNP_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    MLDSNP_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size, resp_size;
    MLDSNP_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size = msg_buf_size;
    resp_size = MLDSNP_MGR_MSGBUF_TYPE_SIZE;

    msg_p = (SYSFUN_Msg_T *) & space_msg;
    msg_p->cmd = SYS_MODULE_MLDSNP;
    msg_p->msg_size = req_size;

    msg_data_p = (MLDSNP_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = MLDSNP_MGR_IPCCMD_SETPORTLISTSTATICJOINGROUP;

    /*assign input*/
    msg_data_p->data.static_portlist.vid = vid;
    memcpy(msg_data_p->data.static_portlist.gip_ap,
           gip_ap, sizeof(msg_data_p->data.static_portlist.gip_ap));
    memcpy(msg_data_p->data.static_portlist.sip_ap,
           sip_ap, sizeof(msg_data_p->data.static_portlist.sip_ap));
    memcpy(msg_data_p->data.static_portlist.port_list,
           port_list_ap, sizeof(msg_data_p->data.static_portlist.port_list));

    /*send ipc*/
    if (SYSFUN_SendRequestMsg(mldsnp_pmgr_ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
                              SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p) != SYSFUN_OK)
    {
        MLDSNP_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    MLDSNP_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_PMGR_SetPortListStaticLeaveGroup
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to set static port join group
 * INPUT   : vid          - the vlan id
 *           *gip_ap      - the group ip
 *           *sip_ap      - the source ip
 *           *port_list_ap- the static port list
 * OUTPUT  : None
 * RETURN  : MLDSNP_TYPE_RETURN_SUCCESS  - success
 *           MLDSNP_TYPE_RETURN_FAIL - failure
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
UI32_T MLDSNP_PMGR_SetPortListStaticLeaveGroup(
    UI32_T vid,
    UI8_T  *gip_ap,
    UI8_T  *sip_ap,
    UI8_T  *port_list_ap)
{
    const UI32_T msg_buf_size = (sizeof(((MLDSNP_MGR_IPCMsg_T *)0)->data.static_portlist)
                                 + MLDSNP_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    MLDSNP_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size, resp_size;
    MLDSNP_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size = msg_buf_size;
    resp_size = MLDSNP_MGR_MSGBUF_TYPE_SIZE;

    msg_p = (SYSFUN_Msg_T *) & space_msg;
    msg_p->cmd = SYS_MODULE_MLDSNP;
    msg_p->msg_size = req_size;

    msg_data_p = (MLDSNP_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = MLDSNP_MGR_IPCCMD_SETPORTLISTSTATICLEAVEGROUP;

    /*assign input*/
    msg_data_p->data.static_portlist.vid = vid;
    memcpy(msg_data_p->data.static_portlist.gip_ap,
           gip_ap, sizeof(msg_data_p->data.static_portlist.gip_ap));
    memcpy(msg_data_p->data.static_portlist.sip_ap,
           sip_ap, sizeof(msg_data_p->data.static_portlist.sip_ap));
    memcpy(msg_data_p->data.static_portlist.port_list,
           port_list_ap, sizeof(msg_data_p->data.static_portlist.port_list));

    /*send ipc*/
    if (SYSFUN_SendRequestMsg(mldsnp_pmgr_ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
                              SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p) != SYSFUN_OK)
    {
        MLDSNP_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    MLDSNP_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_ui32;
}

/*------------------------------------------------------------------------------
* Function : MLDSNP_PMGR_DeleteStaticRouterPortlist
*------------------------------------------------------------------------------
* Purpose: This function set the static router port list
* INPUT  : vid           - the vlan id
*         *port_list_ap  - the static router port list
* OUTPUT : None
* RETURN  : MLDSNP_TYPE_RETURN_SUCCESS  - success
*           MLDSNP_TYPE_RETURN_FAIL     - failure
* NOTES  :SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST
*------------------------------------------------------------------------------*/
UI32_T MLDSNP_PMGR_DeleteStaticRouterPortlist(
    UI32_T vid,
    UI8_T  *port_list_ap)
{
    const UI32_T msg_buf_size = (sizeof(((MLDSNP_MGR_IPCMsg_T *)0)->data.router_portlist)
                                 + MLDSNP_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    MLDSNP_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size, resp_size;
    MLDSNP_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size = msg_buf_size;
    resp_size = MLDSNP_MGR_MSGBUF_TYPE_SIZE;

    msg_p = (SYSFUN_Msg_T *) & space_msg;
    msg_p->cmd = SYS_MODULE_MLDSNP;
    msg_p->msg_size = req_size;

    msg_data_p = (MLDSNP_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = MLDSNP_MGR_IPCCMD_DELETESTATICROUTERPORTLIST;

    /*assign input*/
    msg_data_p->data.router_portlist.vid = vid;
    memcpy(msg_data_p->data.router_portlist.port_list,
           port_list_ap, sizeof(msg_data_p->data.router_portlist.port_list));

    /*send ipc*/
    if (SYSFUN_SendRequestMsg(mldsnp_pmgr_ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
                              SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p) != SYSFUN_OK)
    {
        MLDSNP_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    MLDSNP_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_ui32;
}

/*------------------------------------------------------------------------------
* Function : MLDSNP_PMGR_SetStaticRouterPortlist
*------------------------------------------------------------------------------
* Purpose: This function set the static router port list
* INPUT  : *port_list_ap  - the static router port list
* OUTPUT : None
* RETURN  : MLDSNP_TYPE_RETURN_SUCCESS  - success
*           MLDSNP_TYPE_RETURN_FAIL     - failure
* NOTES  :SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST
*------------------------------------------------------------------------------*/
UI32_T MLDSNP_PMGR_SetStaticRouterPortlist(
    UI32_T vid,
    UI8_T  *port_list_ap)
{
    const UI32_T msg_buf_size = (sizeof(((MLDSNP_MGR_IPCMsg_T *)0)->data.router_portlist)
                                 + MLDSNP_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    MLDSNP_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size, resp_size;
    MLDSNP_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size = msg_buf_size;
    resp_size = MLDSNP_MGR_MSGBUF_TYPE_SIZE;

    msg_p = (SYSFUN_Msg_T *) & space_msg;
    msg_p->cmd = SYS_MODULE_MLDSNP;
    msg_p->msg_size = req_size;

    msg_data_p = (MLDSNP_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = MLDSNP_MGR_IPCCMD_SETSTATICROUTERPORTLIST;

    /*assign input*/
    msg_data_p->data.router_portlist.vid = vid;
    memcpy(msg_data_p->data.router_portlist.port_list,
           port_list_ap, sizeof(msg_data_p->data.router_portlist.port_list));

    /*send ipc*/
    if (SYSFUN_SendRequestMsg(mldsnp_pmgr_ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
                              SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p) != SYSFUN_OK)
    {
        MLDSNP_PMGR_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    MLDSNP_PMGR_DEBUG_LINE();
    return msg_data_p->type.result_ui32;
}


/*------------------------------------------------------------------------------
* Function : MLDSNP_PMGR_SetMRouteStatus
*------------------------------------------------------------------------------
* Purpose: This function set mroute status
* INPUT  : is_eanbled - mroute is enabled or not
* OUTPUT : None
* RETURN  : None
* NOTES  :
*------------------------------------------------------------------------------*/
void MLDSNP_PMGR_SetMRouteStatus(
    BOOL_T is_enabled)
{
    const UI32_T msg_buf_size = (sizeof(((MLDSNP_MGR_IPCMsg_T *)0)->data.bool_v)
                                 + MLDSNP_MGR_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    MLDSNP_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size, resp_size;
    MLDSNP_PMGR_DEBUG_LINE();

    /*assign size*/
    req_size = msg_buf_size;
    resp_size = MLDSNP_MGR_MSGBUF_TYPE_SIZE;

    msg_p = (SYSFUN_Msg_T *) & space_msg;
    msg_p->cmd = SYS_MODULE_MLDSNP;
    msg_p->msg_size = req_size;

    msg_data_p = (MLDSNP_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = MLDSNP_MGR_IPCCMD_SETMROUTESTATUS;

    /*assign input*/
    msg_data_p->data.bool_v = is_enabled;

    /*send ipc*/
    if (SYSFUN_SendRequestMsg(mldsnp_pmgr_ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_NOWAIT,
                              SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p) != SYSFUN_OK)
    {
        MLDSNP_PMGR_DEBUG_LINE();
        return;
    }

    /*assign output*/
    MLDSNP_PMGR_DEBUG_LINE();
    return ;
}

#if (SYS_CPNT_FILTER_THROOTTLE_MLDSNP == TRUE)
UI32_T MLDSNP_PMGR_SetMldFilter(UI32_T status)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(MLDSNP_MGR_GET_MSGBUFSIZE(MLDSNP_MGR_IPCMsg_MldSnoopFilter_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    MLDSNP_MGR_IPCMsg_MldSnoopFilter_T *data_p;

    data_p = MLDSNP_MGR_MSG_DATA(msg_p);
    data_p->mldsnp_FilterStatus = status;

    MLDSNP_PMGR_SendMsg(MLDSNP_MGR_IPCCMD_SET_MLD_FILTER_STATUS,
                        msg_p,
                        MLDSNP_MGR_GET_MSGBUFSIZE(MLDSNP_MGR_IPCMsg_MldSnoopFilter_T),
                        MLDSNP_MGR_GET_MSGBUFSIZE(MLDSNP_MGR_IPCMsg_MldSnoopFilter_T),
                        (UI32_T)FALSE);

    return MLDSNP_MGR_MSG_RETVAL(msg_p);
}

UI32_T MLDSNP_PMGR_CreateMLDProfileEntry(UI32_T profile_id)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(MLDSNP_MGR_GET_MSGBUFSIZE(MLDSNP_MGR_IPCMsg_MldSnoopProfile_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    MLDSNP_MGR_IPCMsg_MldSnoopProfile_T *data_p;

    data_p = MLDSNP_MGR_MSG_DATA(msg_p);
    data_p->mldsnp_Profile_id = profile_id;

    MLDSNP_PMGR_SendMsg(MLDSNP_MGR_IPCCMD_SET_MLD_CREATE_PRIFILE,
                        msg_p,
                        MLDSNP_MGR_GET_MSGBUFSIZE(MLDSNP_MGR_IPCMsg_MldSnoopProfile_T),
                        MLDSNP_MGR_GET_MSGBUFSIZE(MLDSNP_MGR_IPCMsg_MldSnoopProfile_T),
                        (UI32_T)FALSE);

    return MLDSNP_MGR_MSG_RETVAL(msg_p);
}

UI32_T MLDSNP_PMGR_DestroyMLDProfileEntry(UI32_T profile_id)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(MLDSNP_MGR_GET_MSGBUFSIZE(MLDSNP_MGR_IPCMsg_MldSnoopProfile_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    MLDSNP_MGR_IPCMsg_MldSnoopProfile_T *data_p;

    data_p = MLDSNP_MGR_MSG_DATA(msg_p);
    data_p->mldsnp_Profile_id = profile_id;

    MLDSNP_PMGR_SendMsg(MLDSNP_MGR_IPCCMD_SET_MLD_DESTROY_PRIFILE,
                        msg_p,
                        MLDSNP_MGR_GET_MSGBUFSIZE(MLDSNP_MGR_IPCMsg_MldSnoopProfile_T),
                        MLDSNP_MGR_GET_MSGBUFSIZE(MLDSNP_MGR_IPCMsg_MldSnoopProfile_T),
                        (UI32_T)FALSE);

    return MLDSNP_MGR_MSG_RETVAL(msg_p);
}

UI32_T MLDSNP_PMGR_SetMLDProfileAccessMode(UI32_T pid, UI32_T mode)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(MLDSNP_MGR_GET_MSGBUFSIZE(MLDSNP_MGR_IPCMsg_MldSnoopProfileMode_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    MLDSNP_MGR_IPCMsg_MldSnoopProfileMode_T *data_p;

    data_p = MLDSNP_MGR_MSG_DATA(msg_p);
    data_p->mldsnp_Profile_id = pid;
    data_p->profile_mode = mode;

    MLDSNP_PMGR_SendMsg(MLDSNP_MGR_IPCCMD_SET_MLD_PRIFILE_MODE,
                        msg_p,
                        MLDSNP_MGR_GET_MSGBUFSIZE(MLDSNP_MGR_IPCMsg_MldSnoopProfileMode_T),
                        MLDSNP_MGR_GET_MSGBUFSIZE(MLDSNP_MGR_IPCMsg_MldSnoopProfileMode_T),
                        (UI32_T)FALSE);

    return MLDSNP_MGR_MSG_RETVAL(msg_p);
}

UI32_T MLDSNP_PMGR_SetMLDThrottlingActionToPort(UI32_T port, UI32_T action)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(MLDSNP_MGR_GET_MSGBUFSIZE(MLDSNP_MGR_IPCMsg_MldSnoopMaxGroupAction_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    MLDSNP_MGR_IPCMsg_MldSnoopMaxGroupAction_T *data_p;

    data_p = MLDSNP_MGR_MSG_DATA(msg_p);
    data_p->port = port;
    data_p->action = action;

    MLDSNP_PMGR_SendMsg(MLDSNP_MGR_IPCCMD_SET_MLD_MAXGROUP_ACTION_PORT,
                        msg_p,
                        MLDSNP_MGR_GET_MSGBUFSIZE(MLDSNP_MGR_IPCMsg_MldSnoopMaxGroupAction_T),
                        MLDSNP_MGR_GET_MSGBUFSIZE(MLDSNP_MGR_IPCMsg_MldSnoopMaxGroupAction_T),
                        (UI32_T)FALSE);

    return MLDSNP_MGR_MSG_RETVAL(msg_p);
}

UI32_T MLDSNP_PMGR_AddMLDProfileGroup(UI32_T pid, UI8_T mip_begin[], UI8_T mip_end[])
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(MLDSNP_MGR_GET_MSGBUFSIZE(MLDSNP_MGR_IPCMsg_MldSnoopProfileRange_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    MLDSNP_MGR_IPCMsg_MldSnoopProfileRange_T *data_p;

    data_p = MLDSNP_MGR_MSG_DATA(msg_p);
    data_p->mldsnp_Profile_id = pid;
    memcpy(data_p->ip_begin, mip_begin, SYS_ADPT_IPV6_ADDR_LEN);
    memcpy(data_p->ip_end, mip_end, SYS_ADPT_IPV6_ADDR_LEN);

    MLDSNP_PMGR_SendMsg(MLDSNP_MGR_IPCCMD_ADD_MLD_PRIFILE_GROUP,
                        msg_p,
                        MLDSNP_MGR_GET_MSGBUFSIZE(MLDSNP_MGR_IPCMsg_MldSnoopProfileRange_T),
                        MLDSNP_MGR_GET_MSGBUFSIZE(MLDSNP_MGR_IPCMsg_MldSnoopProfileRange_T),
                        (UI32_T)FALSE);

    return MLDSNP_MGR_MSG_RETVAL(msg_p);
}

UI32_T MLDSNP_PMGR_DeleteMLDProfileGroup(UI32_T pid, UI8_T mip_begin[], UI8_T mip_end[])
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(MLDSNP_MGR_GET_MSGBUFSIZE(MLDSNP_MGR_IPCMsg_MldSnoopProfileRange_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    MLDSNP_MGR_IPCMsg_MldSnoopProfileRange_T *data_p;

    data_p = MLDSNP_MGR_MSG_DATA(msg_p);
    data_p->mldsnp_Profile_id = pid;
    memcpy(data_p->ip_begin, mip_begin, SYS_ADPT_IPV6_ADDR_LEN);
    memcpy(data_p->ip_end, mip_end, SYS_ADPT_IPV6_ADDR_LEN);

    MLDSNP_PMGR_SendMsg(MLDSNP_MGR_IPCCMD_DELETE_MLD_PRIFILE_GROUP,
                        msg_p,
                        MLDSNP_MGR_GET_MSGBUFSIZE(MLDSNP_MGR_IPCMsg_MldSnoopProfileRange_T),
                        MLDSNP_MGR_GET_MSGBUFSIZE(MLDSNP_MGR_IPCMsg_MldSnoopProfileRange_T),
                        (UI32_T)FALSE);

    return MLDSNP_MGR_MSG_RETVAL(msg_p);
}

UI32_T MLDSNP_PMGR_AddMLDProfileToPort(UI32_T ifindex, UI32_T pid)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(MLDSNP_MGR_GET_MSGBUFSIZE(MLDSNP_MGR_IPCMsg_MldSnoopProfilePort_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    MLDSNP_MGR_IPCMsg_MldSnoopProfilePort_T *data_p;

    data_p = MLDSNP_MGR_MSG_DATA(msg_p);
    data_p->mldsnp_Profile_id = pid;
    data_p->port = ifindex;

    MLDSNP_PMGR_SendMsg(MLDSNP_MGR_IPCCMD_ADD_MLD_PRIFILE_PORT,
                        msg_p,
                        MLDSNP_MGR_GET_MSGBUFSIZE(MLDSNP_MGR_IPCMsg_MldSnoopProfilePort_T),
                        MLDSNP_MGR_GET_MSGBUFSIZE(MLDSNP_MGR_IPCMsg_MldSnoopProfilePort_T),
                        (UI32_T)FALSE);

    return MLDSNP_MGR_MSG_RETVAL(msg_p);
}

UI32_T MLDSNP_PMGR_RemoveMLDProfileFromPort(UI32_T ifindex)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(MLDSNP_MGR_GET_MSGBUFSIZE(MLDSNP_MGR_IPCMsg_MldSnoopProfilePort_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    MLDSNP_MGR_IPCMsg_MldSnoopProfilePort_T *data_p;

    data_p = MLDSNP_MGR_MSG_DATA(msg_p);
    data_p->port = ifindex;

    MLDSNP_PMGR_SendMsg(MLDSNP_MGR_IPCCMD_REMOVE_MLD_PRIFILE_PORT,
                        msg_p,
                        MLDSNP_MGR_GET_MSGBUFSIZE(MLDSNP_MGR_IPCMsg_MldSnoopProfilePort_T),
                        MLDSNP_MGR_GET_MSGBUFSIZE(MLDSNP_MGR_IPCMsg_MldSnoopProfilePort_T),
                        (UI32_T)FALSE);

    return MLDSNP_MGR_MSG_RETVAL(msg_p);
}


UI32_T MLDSNP_PMGR_SetMLDThrottlingNumberToPort(UI32_T ifindex, UI32_T pid)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(MLDSNP_MGR_GET_MSGBUFSIZE(MLDSNP_MGR_IPCMsg_MldSnoopProfilePort_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    MLDSNP_MGR_IPCMsg_MldSnoopProfilePort_T *data_p;

    data_p = MLDSNP_MGR_MSG_DATA(msg_p);
    data_p->mldsnp_Profile_id = pid;
    data_p->port = ifindex;

    MLDSNP_PMGR_SendMsg(MLDSNP_MGR_IPCCMD_SET_MLD_PRIFILE_PORT,
                        msg_p,
                        MLDSNP_MGR_GET_MSGBUFSIZE(MLDSNP_MGR_IPCMsg_MldSnoopProfilePort_T),
                        MLDSNP_MGR_GET_MSGBUFSIZE(MLDSNP_MGR_IPCMsg_MldSnoopProfilePort_T),
                        (UI32_T)FALSE);

    return MLDSNP_MGR_MSG_RETVAL(msg_p);
}

UI32_T MLDSNP_PMGR_GetNextPortMLDProfileID(UI32_T *ifindex, UI32_T *pid)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(MLDSNP_MGR_GET_MSGBUFSIZE(MLDSNP_MGR_IPCMsg_MldSnoopProfilePort_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    MLDSNP_MGR_IPCMsg_MldSnoopProfilePort_T *data_p;

    data_p = MLDSNP_MGR_MSG_DATA(msg_p);
    data_p->mldsnp_Profile_id = *pid;
    data_p->port = *ifindex;

    MLDSNP_PMGR_SendMsg(MLDSNP_MGR_IPCCMD_GETNEXT_MLD_PRIFILE_PORT,
                        msg_p,
                        MLDSNP_MGR_GET_MSGBUFSIZE(MLDSNP_MGR_IPCMsg_MldSnoopProfilePort_T),
                        MLDSNP_MGR_GET_MSGBUFSIZE(MLDSNP_MGR_IPCMsg_MldSnoopProfilePort_T),
                        (UI32_T)FALSE);
    *pid = data_p->mldsnp_Profile_id;
    *ifindex = data_p->port;

    return MLDSNP_MGR_MSG_RETVAL(msg_p);
}
#endif

#if (SYS_CPNT_MLDSNP_MLD_REPORT_LIMIT_PER_SECOND_PER_PORT== TRUE || SYS_CPNT_MLDSNP_MLD_REPORT_LIMIT_PER_SECOND_PER_VLAN == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_PMGR_SetMldReportLimitPerSec
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to set igmp report limit value per second
 * INPUT   : ifindex       - which port or vlan to get
 *           limit_per_sec - per second limit value
 * OUTPUT  : None
 * RETURN  : TRUE  - success
 *           FALSE - fail
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_PMGR_SetMldReportLimitPerSec(UI32_T ifindex, UI16_T limit_per_sec)
{
    UI8_T msgbuf[SYSFUN_SIZE_OF_MSG(MLDSNP_MGR_GET_MSGBUFSIZE(MLDSNP_MGR_IPCMsg_GS2_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    MLDSNP_MGR_IPCMsg_GS2_T *data_p;

    data_p = MLDSNP_MGR_MSG_DATA(msg_p);
    data_p->value1 = ifindex;
    data_p->value2 = limit_per_sec;

    MLDSNP_PMGR_SendMsg(MLDSNP_MGR_IPCCMD_SET_MLD_REPORT_LIMIT_PER_SECOND,
                        msg_p,
                        MLDSNP_MGR_GET_MSGBUFSIZE(MLDSNP_MGR_IPCMsg_GS2_T),
                        MLDSNP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                        (UI32_T)FALSE);


    return MLDSNP_MGR_MSG_RETVAL(msg_p);
}/*End of MLDSNP_PMGR_SetMldReportLimitPerSec*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_PMGR_GetNextMldReportLimitPerSec
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to set mld report limit value per second
 * INPUT   : ifindex       - which port or vlan to get
 *           limit_per_sec - per second limit value
 * OUTPUT  : ifindex - next port or vlan
 *           limit_per_sec - per second limit value
 * RETURN  : TRUE  - success
 *           FALSE - fail
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_PMGR_GetNextMldReportLimitPerSec(UI32_T *ifindex, UI16_T  *limit_per_sec)
{
    UI8_T msgbuf[SYSFUN_SIZE_OF_MSG(MLDSNP_MGR_GET_MSGBUFSIZE(MLDSNP_MGR_IPCMsg_GS2_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    MLDSNP_MGR_IPCMsg_GS2_T *data_p;

    data_p = MLDSNP_MGR_MSG_DATA(msg_p);
    data_p->value1 = *ifindex;

    MLDSNP_PMGR_SendMsg(MLDSNP_MGR_IPCCMD_GET_NEXT_MLD_REPORT_LIMIT_PER_SECOND,
                        msg_p,
                        MLDSNP_MGR_GET_MSGBUFSIZE(MLDSNP_MGR_IPCMsg_GS2_T),
                        MLDSNP_MGR_GET_MSGBUFSIZE(MLDSNP_MGR_IPCMsg_GS2_T),
                        (UI32_T)FALSE);

    *ifindex = data_p->value1;
    *limit_per_sec = data_p->value2;

    return MLDSNP_MGR_MSG_RETVAL(msg_p);
}/*End of MLDSNP_PMGR_GetNextMldReportLimitPerSec*/

#endif

#if (SYS_CPNT_MLDSNP_QUERY_DROP == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_PMGR_SetQueryDropStatus
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to set query quard status
 * INPUT   : lport  - which port to get information
 *           status - the enable or diable status
 * OUTPUT  : None
 * RETURN  : MVR_MGR_OK  - success
 *           MVR_MGR_CAN_NOT_SET - failure
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_PMGR_SetQueryDropStatus(UI32_T lport, UI32_T status)
{
    UI8_T msgbuf[SYSFUN_SIZE_OF_MSG(MLDSNP_MGR_GET_MSGBUFSIZE(MLDSNP_MGR_IPCMsg_GS2_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    MLDSNP_MGR_IPCMsg_GS2_T *data_p;

    data_p = MLDSNP_MGR_MSG_DATA(msg_p);

    data_p->value1 = lport;
    data_p->value2 = status;

    MLDSNP_PMGR_SendMsg(MLDSNP_MGR_IPCCMD_SET_QUERY_GUARD_STATUS,
                        msg_p,
                        MLDSNP_MGR_GET_MSGBUFSIZE(MLDSNP_MGR_IPCMsg_GS2_T),
                        MLDSNP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                        (UI32_T)FALSE);


    return MLDSNP_MGR_MSG_RETVAL(msg_p);
}/*End of MLDSNP_PMGR_SetQueryDropStatus*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_PMGR_GetNextQueryDropStatus
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to get next port query guard status
 * INPUT   : lport  - which port to get information
 *           status - the enable or diable status
 * OUTPUT  : status  - the enabled or disable status
 * RETURN  : MVR_MGR_OK  - success
 *           MVR_MGR_CAN_NOT_GET - failure
 * NOTE    :
 *-------------------------------------------------------------------------
 */
UI32_T MLDSNP_PMGR_GetNextQueryDropStatus(UI32_T *lport, UI32_T  *status)
{
    UI8_T msgbuf[SYSFUN_SIZE_OF_MSG(MLDSNP_MGR_GET_MSGBUFSIZE(MLDSNP_MGR_IPCMsg_GS2_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    MLDSNP_MGR_IPCMsg_GS2_T *data_p;

    data_p = MLDSNP_MGR_MSG_DATA(msg_p);

    data_p->value1 = *lport;

    MLDSNP_PMGR_SendMsg(MLDSNP_MGR_IPCCMD_GETNEXT_QUERY_GUARD_STATUS,
                        msg_p,
                        MLDSNP_MGR_GET_MSGBUFSIZE(MLDSNP_MGR_IPCMsg_GS2_T),
                        MLDSNP_MGR_GET_MSGBUFSIZE(MLDSNP_MGR_IPCMsg_GS2_T),
                        (UI32_T)FALSE);

    *lport = data_p->value1;
    *status = data_p->value2;
    return MLDSNP_MGR_MSG_RETVAL(msg_p);
}/*End of MLDSNP_PMGR_GetNextQueryDropStatus*/
#endif

#if (SYS_CPNT_IPV6_MULTICAST_DATA_DROP== TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_PMGR_SetMulticastDataDropStatus
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to set multicast data guard status
 * INPUT   : lport  - which port to get information
 *           status - the enable or diable status
 * OUTPUT  : None
 * RETURN  : MVR_MGR_OK  - success
 *           MVR_MGR_CAN_NOT_SET - failure
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_PMGR_SetMulticastDataDropStatus(UI32_T lport, UI32_T status)
{
    UI8_T msgbuf[SYSFUN_SIZE_OF_MSG(MLDSNP_MGR_GET_MSGBUFSIZE(MLDSNP_MGR_IPCMsg_GS2_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    MLDSNP_MGR_IPCMsg_GS2_T *data_p;

    data_p = MLDSNP_MGR_MSG_DATA(msg_p);

    data_p->value1 = lport;
    data_p->value2 = status;

    MLDSNP_PMGR_SendMsg(MLDSNP_MGR_IPCCMD_SET_MULTICAST_DATA_DROP_STATUS,
                        msg_p,
                        MLDSNP_MGR_GET_MSGBUFSIZE(MLDSNP_MGR_IPCMsg_GS2_T),
                        MLDSNP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                        (UI32_T)FALSE);


    return MLDSNP_MGR_MSG_RETVAL(msg_p);
}/*End of MLDSNP_PMGR_SetMulticastDataDropStatus*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_PMGR_GetNextMulticastDataDropStatus
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to get multicast data guard
 * INPUT   : lport  - which port to get information
 *           status - the enable or diable status
 * OUTPUT  : status  - the enabled or disable status
 * RETURN  : MVR_MGR_OK  - success
 *           MVR_MGR_CAN_NOT_GET - failure
 * NOTE    :
 *-------------------------------------------------------------------------
 */
UI32_T MLDSNP_PMGR_GetNextMulticastDataDropStatus(UI32_T *lport, UI32_T  *status)
{
    UI8_T msgbuf[SYSFUN_SIZE_OF_MSG(MLDSNP_MGR_GET_MSGBUFSIZE(MLDSNP_MGR_IPCMsg_GS2_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    MLDSNP_MGR_IPCMsg_GS2_T *data_p;

    data_p = MLDSNP_MGR_MSG_DATA(msg_p);

    data_p->value1 = *lport;

    MLDSNP_PMGR_SendMsg(MLDSNP_MGR_IPCCMD_GETNEXT_MULTICAST_DATA_DROP_STATUS,
                        msg_p,
                        MLDSNP_MGR_GET_MSGBUFSIZE(MLDSNP_MGR_IPCMsg_GS2_T),
                        MLDSNP_MGR_GET_MSGBUFSIZE(MLDSNP_MGR_IPCMsg_GS2_T),
                        (UI32_T)FALSE);

    *lport = data_p->value1;
    *status = data_p->value2;
    return MLDSNP_MGR_MSG_RETVAL(msg_p);
}/*End of MLDSNP_PMGR_GetNextMulticastDataDropStatus*/
#endif

#if (SYS_CPNT_MLDSNP_PROXY == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_PMGR_SetMldSnoopProxyReporting
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to proxy reporting status
 * INPUT   : status - setting status
 * OUTPUT  : None
 * RETURN  : TRUE  - success
 *    FALSE - failure
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_PMGR_SetMldSnoopProxyReporting(UI32_T status)
{
    UI8_T    msgbuf[SYSFUN_SIZE_OF_MSG(MLDSNP_MGR_GET_MSGBUFSIZE(MLDSNP_MGR_IPCMsg_GS1_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    MLDSNP_MGR_IPCMsg_GS1_T *data_p;

    data_p = MLDSNP_MGR_MSG_DATA(msg_p);
    data_p->value1 = status;

    MLDSNP_PMGR_SendMsg(MLDSNP_MGR_IPCCMD_SET_PROXY_REPORTING,
                        msg_p,
                        MLDSNP_MGR_GET_MSGBUFSIZE(MLDSNP_MGR_IPCMsg_GS1_T),
                        MLDSNP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                        (UI32_T)FALSE);

    return MLDSNP_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_PMGR_SetUnsolicitedReportInterval
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to set unsolicited report interval.
 * INPUT   : Unsolicit_report_interval - setting value
 * OUTPUT  : None
 * RETURN  : TRUE  - success
 *    FALSE - failure
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_PMGR_SetUnsolicitedReportInterval(UI32_T Unsolicit_report_interval)
{
    UI8_T    msgbuf[SYSFUN_SIZE_OF_MSG(MLDSNP_MGR_GET_MSGBUFSIZE(MLDSNP_MGR_IPCMsg_GS1_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    MLDSNP_MGR_IPCMsg_GS1_T *data_p;

    data_p = MLDSNP_MGR_MSG_DATA(msg_p);
    data_p->value1 = Unsolicit_report_interval;

    MLDSNP_PMGR_SendMsg(MLDSNP_MGR_IPCCMD_SET_UNSOLICITEDREPORTINTERVAL,
                        msg_p,
                        MLDSNP_MGR_GET_MSGBUFSIZE(MLDSNP_MGR_IPCMsg_GS1_T),
                        MLDSNP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                        (UI32_T)FALSE);

    return MLDSNP_MGR_MSG_RETVAL(msg_p);
}


#endif

/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_PMGR_ClearMldSnoopingDynamicgGroup
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to clear mldsnp dynamic groups
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE  - success
 *    FALSE - failure
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
BOOL_T
MLDSNP_PMGR_ClearMldSnoopingDynamicgGroup()
{
    UI8_T msgbuf[SYSFUN_SIZE_OF_MSG(MLDSNP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA())];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;

    MLDSNP_PMGR_SendMsg(MLDSNP_MGR_IPCCMD_CLEAR_MLD_SNOOP_DYNAMIC_GROUP,
                        msg_p,
                        MLDSNP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                        MLDSNP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                        (UI32_T)FALSE);

    return (BOOL_T)MLDSNP_MGR_MSG_RETVAL(msg_p);
}


/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_PMGR_SetUnsolicitedReportInterval
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to clear interface's statistics
 * INPUT   : ifindex - vlan or lport ifindex
 * OUTPUT  : None
 * RETURN  : TRUE  - success
 *    FALSE - failure
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
BOOL_T
MLDSNP_PMGR_Clear_Ipv6_Mld_snooping_Statistics(UI32_T ifindex)
{
    UI8_T msgbuf[SYSFUN_SIZE_OF_MSG(MLDSNP_MGR_GET_MSGBUFSIZE(MLDSNP_MGR_IPCMsg_GS1_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    MLDSNP_MGR_IPCMsg_GS1_T *data_p;

    data_p = MLDSNP_MGR_MSG_DATA(msg_p);
    data_p->value1 = ifindex;

    MLDSNP_PMGR_SendMsg(MLDSNP_MGR_IPCCMD_CLEAR_IP_MLDSNOOPING_STATISTICS,
                        msg_p,
                        MLDSNP_MGR_GET_MSGBUFSIZE(MLDSNP_MGR_IPCMsg_GS1_T),
                        MLDSNP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                        (UI32_T)FALSE);

    return (BOOL_T)MLDSNP_MGR_MSG_RETVAL(msg_p);

}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_PMGR_GetInfStatistics
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to get interface statistics entry
 * INPUT   : inf_id       - interface id
 *           is_vlan      - TRUE to indicate the inf_id is VLAN id
 * OUTPUT  : statistics_p - pointer to content of the statistics entry
 *
 * RETUEN  : TRUE  - Success
 *           FALSE - Fail
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
BOOL_T
MLDSNP_PMGR_GetInfStatistics(
    UI32_T  inf_id,
    BOOL_T  is_vlan,
    MLDSNP_MGR_InfStat_T   *statistics_p)
{
    UI8_T msgbuf[SYSFUN_SIZE_OF_MSG(MLDSNP_MGR_GET_MSGBUFSIZE(MLDSNP_MGR_IPCMsg_InfStatistics))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    MLDSNP_MGR_IPCMsg_InfStatistics *data_p;

    data_p = MLDSNP_MGR_MSG_DATA(msg_p);
    data_p->inf_id  = inf_id;
    data_p->is_vlan = is_vlan;

    MLDSNP_PMGR_SendMsg(MLDSNP_MGR_IPCCMD_GET_INF_STATISTICS,
                        msg_p,
                        MLDSNP_MGR_GET_MSGBUFSIZE(MLDSNP_MGR_IPCMsg_InfStatistics),
                        MLDSNP_MGR_GET_MSGBUFSIZE(MLDSNP_MGR_IPCMsg_InfStatistics),
                        (UI32_T)FALSE);

    if (TRUE == MLDSNP_MGR_MSG_RETVAL(msg_p))
    {
        memcpy(statistics_p, &data_p->statistics, sizeof(data_p->statistics));
    }

    return (BOOL_T)MLDSNP_MGR_MSG_RETVAL(msg_p);
}

#endif /*#ifndef UNIT_TEST*/
