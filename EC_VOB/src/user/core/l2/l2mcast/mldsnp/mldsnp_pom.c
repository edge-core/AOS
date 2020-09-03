/* MODULE NAME:  mldsnp_pom.c
 * PURPOSE: For accessing om through IPC
 *
 *
 * NOTES:
 *
 * REASON:
 * Description:
 * HISTORY
 *    12/3/2006 - Wind Lai, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */
#ifndef UNIT_TEST
#include "mldsnp_pom.h"


/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */
#define MLDSNP_POM_DEBUG_ENABLE FALSE

#if (MLDSNP_POM_DEBUG_ENABLE==TRUE)

#define MLDSNP_POM_DEBUG_LINE() \
{  \
  printf("%s(%d)\r\n",__FUNCTION__,__LINE__); \
  fflush(stdout); \
}

#define MLDSNP_POM_DEBUG_MSG(a,b...)   \
{ \
  printf(a,##b); \
  fflush(stdout); \
}
#else
#define MLDSNP_POM_DEBUG_LINE()
#define MLDSNP_POM_DEBUG_MSG(a,b...)
#endif

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */
static SYSFUN_MsgQ_T mldsnp_pom_ipcmsgq_handle = 0;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_POM_SendMsg
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
static void MLDSNP_POM_SendMsg(UI32_T cmd, SYSFUN_Msg_T *msg_p, UI32_T req_size, UI32_T res_size, UI32_T ret_val)
{
    UI32_T ret;

    msg_p->cmd = SYS_MODULE_MLDSNP;
    msg_p->msg_size = req_size;

    MLDSNP_OM_MSG_CMD(msg_p) = cmd;

    ret = SYSFUN_SendRequestMsg(mldsnp_pom_ipcmsgq_handle,
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
        MLDSNP_OM_MSG_RETVAL(msg_p) = ret_val;
    }

    if ((((MLDSNP_OM_IPCMsg_T *)(msg_p)->msg_buf)->type.result_i32) == MLDSNP_OM_IPC_RESULT_FAIL)
        MLDSNP_OM_MSG_RETVAL(msg_p) = ret_val;
}

/* EXPORTED SUBPROGRAM BODIES
 */
/*------------------------------------------------------------------------------
 * ROUTINE NAME : MLDSNP_POM_InitiateProcessResources
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Do initialization procedures for CSCA_POM.
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
void MLDSNP_POM_InitiateProcessResources(void)
{
    MLDSNP_POM_DEBUG_LINE();
    /* Given that MLDSNP is run in MGMT_PROC
     */
    if (SYSFUN_GetMsgQ(SYS_BLD_L2_L4_PROC_OM_IPCMSGQ_KEY,
                       SYSFUN_MSGQ_BIDIRECTIONAL,
                       &mldsnp_pom_ipcmsgq_handle) != SYSFUN_OK)
    {
        printf("%s(): SYSFUN_GetMsgQ fail.\n", __FUNCTION__);
    }
}/*End of MLDSNP_POM_InitiateProcessResources*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_POM_GetGlobalConf
*------------------------------------------------------------------------------
* Purpose: This function get the global configuration
* INPUT  : None
*
* OUTPUT : *mldsnp_global_conf_p - the global configuration
* RETURN : None
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_POM_GetGlobalConf(
    MLDSNP_OM_Cfg_T *mldsnp_global_conf_p)
{
    const UI32_T msg_buf_size = (sizeof(((MLDSNP_OM_IPCMsg_T *)NULL)->data.mldsnp_global_conf)
                                 + MLDSNP_OM_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    MLDSNP_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size, resp_size;

    MLDSNP_POM_DEBUG_LINE();

    /*assign size*/
    req_size = MLDSNP_OM_MSGBUF_TYPE_SIZE;
    resp_size = msg_buf_size;

    msg_p = (SYSFUN_Msg_T *) & space_msg;
    msg_p->cmd = SYS_MODULE_MLDSNP;
    msg_p->msg_size = req_size;

    msg_data_p = (MLDSNP_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = MLDSNP_OM_IPCCMD_GETGLOBALCONF;

    /*assign input*/

    /*send ipc*/
    if (SYSFUN_SendRequestMsg(mldsnp_pom_ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
                              SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p) != SYSFUN_OK)
    {
        MLDSNP_POM_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    memcpy(mldsnp_global_conf_p, &msg_data_p->data.mldsnp_global_conf, sizeof(MLDSNP_OM_Cfg_T));

    MLDSNP_POM_DEBUG_LINE();
    return msg_data_p->type.result_bool;
}/*End of MLDSNP_POM_GetGlobalConf*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_POM_GetGroupPortlist
*------------------------------------------------------------------------------
* Purpose: This function get the group's port list
* INPUT  : vid     - the vlan id
*          *gip_ap  - the group ip address
*          *sip_ap  - the source ip address
*
* OUTPUT : *group_info_p  - the group informaiton
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  : This function is only rovide to UI to access
*------------------------------------------------------------------------------
*/
BOOL_T MLDSNP_POM_GetGroupPortlist(
    UI16_T                vid,
    UI8_T                 *gip_ap,
    UI8_T                 *sip_ap,
    MLDSNP_OM_GroupInfo_T *group_info_p)
{
    const UI32_T msg_buf_size = (sizeof(((MLDSNP_OM_IPCMsg_T *)NULL)->data.group_portlist)
                                 + MLDSNP_OM_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    MLDSNP_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size, resp_size;
    MLDSNP_POM_DEBUG_LINE();

    /*assign size*/
    req_size = sizeof(msg_data_p->data.group_portlist)
               + MLDSNP_OM_MSGBUF_TYPE_SIZE;
    resp_size = msg_buf_size;

    msg_p = (SYSFUN_Msg_T *) & space_msg;
    msg_p->cmd = SYS_MODULE_MLDSNP;
    msg_p->msg_size = req_size;

    msg_data_p = (MLDSNP_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = MLDSNP_OM_IPCCMD_GETGROUPPORTLIST;

    /*assign input*/
    msg_data_p->data.group_portlist.vid = vid;
    memcpy(msg_data_p->data.group_portlist.gip_ar, gip_ap, MLDSNP_TYPE_IPV6_DST_IP_LEN);
    memcpy(msg_data_p->data.group_portlist.sip_ar, sip_ap, MLDSNP_TYPE_IPV6_SRC_IP_LEN);

    /*send ipc*/
    if (SYSFUN_SendRequestMsg(mldsnp_pom_ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
                              SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p) != SYSFUN_OK)
    {
        MLDSNP_POM_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    memcpy(group_info_p, &msg_data_p->data.group_portlist.scgroup_info, sizeof(MLDSNP_OM_GroupInfo_T));

    MLDSNP_POM_DEBUG_LINE();
    return msg_data_p->type.result_bool;
}/*End of MLDSNP_POM_GetGroupPortlist*/
#if 0
/*------------------------------------------------------------------------------
* Function : MLDSNP_POM_GetHisamEntryInfo
*------------------------------------------------------------------------------
* Purpose: This function get the info in hisam entry
* INPUT  : vid       - the vlan id
*          gip_ap    - the group ip
*          sip_ap    - the source ip
*          key_idx   - use which key to get
* OUTPUT : *entry_info   - the infomation store in hisam
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :  MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_POM_GetHisamEntryInfo(
    UI32_T                 vid,
    UI8_T                  *gip_ap,
    UI8_T                  *sip_ap,
    UI32_T                 key_idx,
    MLDSNP_OM_HisamEntry_T *entry_info)
{
    const UI32_T msg_buf_size = (sizeof(((MLDSNP_OM_IPCMsg_T *)NULL)->data.hisam_entry_info)
                                 + MLDSNP_OM_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    MLDSNP_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size, resp_size;
    MLDSNP_POM_DEBUG_LINE();

    /*assign size*/
    req_size = sizeof(msg_data_p->data.hisam_entry_info)
               + MLDSNP_OM_MSGBUF_TYPE_SIZE;
    resp_size = msg_buf_size;

    msg_p = (SYSFUN_Msg_T *) & space_msg;
    msg_p->cmd = SYS_MODULE_MLDSNP;
    msg_p->msg_size = req_size;

    msg_data_p = (MLDSNP_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = MLDSNP_OM_IPCCMD_GETHISAMENTRYINFO;

    /*assign input*/
    msg_data_p->data.hisam_entry_info.vid = vid;
    msg_data_p->data.hisam_entry_info.key_idx = key_idx;
    memcpy(msg_data_p->data.hisam_entry_info.gip_ar, gip_ap, MLDSNP_TYPE_IPV6_DST_IP_LEN);
    memcpy(msg_data_p->data.hisam_entry_info.sip_ar, sip_ap, MLDSNP_TYPE_IPV6_SRC_IP_LEN);

    /*send ipc*/
    if (SYSFUN_SendRequestMsg(mldsnp_pom_ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
                              SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p) != SYSFUN_OK)
    {
        MLDSNP_POM_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    memcpy(entry_info, msg_data_p->data.hisam_entry_info.entry_info, sizeof(MLDSNP_OM_HisamEntry_T));
    MLDSNP_POM_DEBUG_LINE();
    return msg_data_p->type.result_bool;
}/*End of MLDSNP_POM_GetHisamEntryInfo*/
#endif
/*------------------------------------------------------------------------------
* Function : MLDSNP_POM_GetImmediateLeaveStatus
*------------------------------------------------------------------------------
* Purpose: This function get the immediate leave status
* INPUT  : vid                       - the vlan id
*
* OUTPUT : immediate_leave_status_p - the immediate leave status
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_POM_GetImmediateLeaveStatus(
    UI16_T vid,
    MLDSNP_TYPE_ImmediateStatus_T *immediate_leave_status_p)
{
    const UI32_T msg_buf_size = (sizeof(((MLDSNP_OM_IPCMsg_T *)NULL)->data.immediate_leave)
                                 + MLDSNP_OM_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    MLDSNP_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size, resp_size;
    MLDSNP_POM_DEBUG_LINE();

    /*assign size*/
    req_size = sizeof(msg_data_p->data.immediate_leave)
               + MLDSNP_OM_MSGBUF_TYPE_SIZE;
    resp_size = msg_buf_size;

    msg_p = (SYSFUN_Msg_T *) & space_msg;
    msg_p->cmd = SYS_MODULE_MLDSNP;
    msg_p->msg_size = req_size;

    msg_data_p = (MLDSNP_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = MLDSNP_OM_IPCCMD_GETIMMEDIATELEAVESTATUS;

    /*assign input*/
    msg_data_p->data.immediate_leave.vid = vid;

    /*send ipc*/
    if (SYSFUN_SendRequestMsg(mldsnp_pom_ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
                              SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p) != SYSFUN_OK)
    {
        MLDSNP_POM_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    *immediate_leave_status_p = msg_data_p->data.immediate_leave.status;

    MLDSNP_POM_DEBUG_LINE();
    return msg_data_p->type.result_bool;
}/*End of MLDSNP_POM_GetImmediateLeaveStatus*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_POM_GetImmediateLeaveStatusByHostStatus
*------------------------------------------------------------------------------
* Purpose: This function get the immediate leave by-host-ip status
* INPUT  : vid                      - the vlan id
* OUTPUT : immediate_leave_status_p - the immediate leave by-host-ip status
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_POM_GetImmediateLeaveByHostStatus(
    UI16_T vid,
    MLDSNP_TYPE_ImmediateByHostStatus_T *immediate_leave_byhost_status_p)
{
    const UI32_T msg_buf_size = (sizeof(((MLDSNP_OM_IPCMsg_T *)NULL)->data.immediate_leave_byhost)
                                 + MLDSNP_OM_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    MLDSNP_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size, resp_size;
    MLDSNP_POM_DEBUG_LINE();

    /*assign size*/
    req_size = sizeof(msg_data_p->data.immediate_leave_byhost)
               + MLDSNP_OM_MSGBUF_TYPE_SIZE;
    resp_size = msg_buf_size;

    msg_p = (SYSFUN_Msg_T *) & space_msg;
    msg_p->cmd = SYS_MODULE_MLDSNP;
    msg_p->msg_size = req_size;

    msg_data_p = (MLDSNP_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = MLDSNP_OM_IPCCMD_GETIMMEDIATELEAVEBYHOSTSTATUS;

    /*assign input*/
    msg_data_p->data.immediate_leave_byhost.vid = vid;

    /*send ipc*/
    if (SYSFUN_SendRequestMsg(mldsnp_pom_ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
                              SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p) != SYSFUN_OK)
    {
        MLDSNP_POM_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    *immediate_leave_byhost_status_p = msg_data_p->data.immediate_leave_byhost.status;

    MLDSNP_POM_DEBUG_LINE();
    return msg_data_p->type.result_bool;
}/*End of MLDSNP_POM_GetImmediateLeaveByHostStatus*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_POM_GetNextImmediateLeaveStatus
*------------------------------------------------------------------------------
* Purpose: This function get the next vlan's immediate leave status
* INPUT  : *vid_p                       - the vlan id
*
* OUTPUT : immediate_leave_status_p - the immediate leave status
*         *vid_p                - the next vlan id
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_POM_GetNextImmediateLeaveStatus(
    UI16_T *vid_p,
    MLDSNP_TYPE_ImmediateStatus_T* immediate_leave_status_p)
{
    const UI32_T msg_buf_size = (sizeof(((MLDSNP_OM_IPCMsg_T *)NULL)->data.immediate_leave)
                                 + MLDSNP_OM_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    MLDSNP_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size, resp_size;
    MLDSNP_POM_DEBUG_LINE();

    /*assign size*/
    req_size = sizeof(msg_data_p->data.u32a1_u32a2.u32_a1)
               + MLDSNP_OM_MSGBUF_TYPE_SIZE;
    resp_size = msg_buf_size;

    msg_p = (SYSFUN_Msg_T *) & space_msg;
    msg_p->cmd = SYS_MODULE_MLDSNP;
    msg_p->msg_size = req_size;

    msg_data_p = (MLDSNP_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = MLDSNP_OM_IPCCMD_GETNEXTIMMEDIATELEAVESTATUS;

    /*assign input*/
    msg_data_p->data.immediate_leave.vid = *vid_p;

    /*send ipc*/
    if (SYSFUN_SendRequestMsg(mldsnp_pom_ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
                              SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p) != SYSFUN_OK)
    {
        MLDSNP_POM_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    *vid_p = msg_data_p->data.immediate_leave.vid;
    *immediate_leave_status_p = msg_data_p->data.immediate_leave.status;

    MLDSNP_POM_DEBUG_LINE();
    return msg_data_p->type.result_bool;
}/*End of MLDSNP_POM_GetNextImmediateLeaveStatus*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_POM_GetNextImmediateLeaveByHostStatus
*------------------------------------------------------------------------------
* Purpose: This function get the next vlan's immediate leave by-host-ip status
* INPUT  : *vid_p                   - the vlan id
* OUTPUT : immediate_leave_status_p - the immediate leave by-host-ip status
*          *vid_p                    - the next vlan id
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_POM_GetNextImmediateLeaveByHostStatus(
    UI16_T *vid_p,
    MLDSNP_TYPE_ImmediateByHostStatus_T* immediate_leave_byhost_status_p)
{
    const UI32_T msg_buf_size = (sizeof(((MLDSNP_OM_IPCMsg_T *)NULL)->data.immediate_leave_byhost)
                                 + MLDSNP_OM_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    MLDSNP_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size, resp_size;
    MLDSNP_POM_DEBUG_LINE();

    /*assign size*/
    req_size = sizeof(msg_data_p->data.u32a1_u32a2.u32_a1)
               + MLDSNP_OM_MSGBUF_TYPE_SIZE;
    resp_size = msg_buf_size;

    msg_p = (SYSFUN_Msg_T *) & space_msg;
    msg_p->cmd = SYS_MODULE_MLDSNP;
    msg_p->msg_size = req_size;

    msg_data_p = (MLDSNP_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = MLDSNP_OM_IPCCMD_GETNEXTIMMEDIATELEAVEBYHOSTSTATUS;

    /*assign input*/
    msg_data_p->data.immediate_leave_byhost.vid = *vid_p;

    /*send ipc*/
    if (SYSFUN_SendRequestMsg(mldsnp_pom_ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
                              SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p) != SYSFUN_OK)
    {
        MLDSNP_POM_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    *vid_p = msg_data_p->data.immediate_leave_byhost.vid;
    *immediate_leave_byhost_status_p = msg_data_p->data.immediate_leave_byhost.status;

    MLDSNP_POM_DEBUG_LINE();
    return msg_data_p->type.result_bool;
}/*End of MLDSNP_POM_GetNextImmediateLeaveStatus*/


/*------------------------------------------------------------------------------
* Function : MLDSNP_POM_GetLastListenerQueryInterval
*------------------------------------------------------------------------------
* Purpose: This function Get the last listener query interval
* INPUT  : None
* OUTPUT : *interval_p  - the interval in second
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :  7.8, 9.8
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_POM_GetLastListenerQueryInterval(
    UI16_T *interval_p)
{
    const UI32_T msg_buf_size = (sizeof(((MLDSNP_OM_IPCMsg_T *)NULL)->data.ui16_v)
                                 + MLDSNP_OM_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    MLDSNP_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size, resp_size;
    MLDSNP_POM_DEBUG_LINE();

    /*assign size*/
    req_size = MLDSNP_OM_MSGBUF_TYPE_SIZE;
    resp_size = msg_buf_size;

    msg_p = (SYSFUN_Msg_T *) & space_msg;
    msg_p->cmd = SYS_MODULE_MLDSNP;
    msg_p->msg_size = req_size;

    msg_data_p = (MLDSNP_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = MLDSNP_OM_IPCCMD_GETLASTLISTENERQUERYINTERVAL;

    /*assign input*/

    /*send ipc*/
    if (SYSFUN_SendRequestMsg(mldsnp_pom_ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
                              SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p) != SYSFUN_OK)
    {
        MLDSNP_POM_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    *interval_p = msg_data_p->data.ui16_v;

    MLDSNP_POM_DEBUG_LINE();
    return msg_data_p->type.result_bool;
}/*End of MLDSNP_POM_GetLastListenerQueryInterval*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_POM_GetListenerInterval
*------------------------------------------------------------------------------
* Purpose: This function get the listner interval
* INPUT  : None
* OUTPUT : *interval_p - the listerner interval in seconds
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :7.4 in RFC2710,  9.4 in RFC 3810
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_POM_GetListenerInterval(
    UI16_T *interval_p)
{
    const UI32_T msg_buf_size = (sizeof(((MLDSNP_OM_IPCMsg_T *)NULL)->data.ui16_v)
                                 + MLDSNP_OM_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    MLDSNP_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size, resp_size;
    MLDSNP_POM_DEBUG_LINE();

    /*assign size*/
    req_size = MLDSNP_OM_MSGBUF_TYPE_SIZE;
    resp_size = msg_buf_size;

    msg_p = (SYSFUN_Msg_T *) & space_msg;
    msg_p->cmd = SYS_MODULE_MLDSNP;
    msg_p->msg_size = req_size;

    msg_data_p = (MLDSNP_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = MLDSNP_OM_IPCCMD_GETLISTENERINTERVAL;

    /*assign input*/

    /*send ipc*/
    if (SYSFUN_SendRequestMsg(mldsnp_pom_ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
                              SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p) != SYSFUN_OK)
    {
        MLDSNP_POM_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    *interval_p = msg_data_p->data.ui16_v;

    MLDSNP_POM_DEBUG_LINE();
    return msg_data_p->type.result_bool;
}/*End of MLDSNP_POM_GetListenerInterval*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_POM_GetMldSnpVer
*------------------------------------------------------------------------------
* Purpose: This function get the mldsno version
* INPUT  : None
* OUTPUT : *ver_p - the version
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_POM_GetMldSnpVer(
    UI16_T *ver_p)
{
    const UI32_T msg_buf_size = (sizeof(((MLDSNP_OM_IPCMsg_T *)NULL)->data.ui16_v)
                                 + MLDSNP_OM_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    MLDSNP_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size, resp_size;
    MLDSNP_POM_DEBUG_LINE();

    /*assign size*/
    req_size = MLDSNP_OM_MSGBUF_TYPE_SIZE;
    resp_size = msg_buf_size;

    msg_p = (SYSFUN_Msg_T *) & space_msg;
    msg_p->cmd = SYS_MODULE_MLDSNP;
    msg_p->msg_size = req_size;

    msg_data_p = (MLDSNP_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = MLDSNP_OM_IPCCMD_GETMLDSNPVER;

    /*assign input*/

    /*send ipc*/
    if (SYSFUN_SendRequestMsg(mldsnp_pom_ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
                              SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p) != SYSFUN_OK)
    {
        MLDSNP_POM_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    *ver_p = msg_data_p->data.ui16_v;

    MLDSNP_POM_DEBUG_LINE();
    return msg_data_p->type.result_bool;
}/*End of MLDSNP_POM_GetMldSnpVer*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_POM_GetMldStatus
*------------------------------------------------------------------------------
* Purpose: This function get the Mld status
* INPUT  :   None
* OUTPUT : *mldsnp_status_p - the mldsnp status
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_POM_GetMldStatus(
    MLDSNP_TYPE_MLDSNP_STATUS_T *mldsnp_status_p)
{
    const UI32_T msg_buf_size = (sizeof(((MLDSNP_OM_IPCMsg_T *)NULL)->data.mldsnp_status)
                                 + MLDSNP_OM_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    MLDSNP_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size, resp_size;
    MLDSNP_POM_DEBUG_LINE();

    /*assign size*/
    req_size = MLDSNP_OM_MSGBUF_TYPE_SIZE;
    resp_size = msg_buf_size;

    msg_p = (SYSFUN_Msg_T *) & space_msg;
    msg_p->cmd = SYS_MODULE_MLDSNP;
    msg_p->msg_size = req_size;

    msg_data_p = (MLDSNP_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = MLDSNP_OM_IPCCMD_GETMLDSTATUS;

    /*assign input*/

    /*send ipc*/
    if (SYSFUN_SendRequestMsg(mldsnp_pom_ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
                              SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p) != SYSFUN_OK)
    {
        MLDSNP_POM_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    *mldsnp_status_p = msg_data_p->data.mldsnp_status;

    MLDSNP_POM_DEBUG_LINE();
    return msg_data_p->type.result_bool;
}/*End of MLDSNP_POM_GetMldStatus*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_POM_GetNextGroupPortlist
*------------------------------------------------------------------------------
* Purpose: This function get the next group's port list
* INPUT  : vid_p    - the vlan id
*          *gip_ap  - the group ip address
*          *sip_ap  - the source ip address
* OUTPUT :  vid_p   - the vlan id
*          *gip_ap  - the group ip address
*          *sip_ap  - the source ip address
*          *group_info_p  - the group informaiton
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  : This frunction only provide to user to access
*------------------------------------------------------------------------------
*/
BOOL_T MLDSNP_POM_GetNextGroupPortlist(
    UI16_T                *vid_p,
    UI8_T                 *gip_ap,
    UI8_T                 *sip_ap,
    MLDSNP_OM_GroupInfo_T *group_info_p)
{
    const UI32_T msg_buf_size = (sizeof(((MLDSNP_OM_IPCMsg_T *)NULL)->data.group_portlist)
                                 + MLDSNP_OM_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    MLDSNP_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size, resp_size;
    MLDSNP_POM_DEBUG_LINE();

    /*assign size*/
    req_size = sizeof(msg_data_p->data.group_portlist)
               + MLDSNP_OM_MSGBUF_TYPE_SIZE;
    resp_size = msg_buf_size;

    msg_p = (SYSFUN_Msg_T *) & space_msg;
    msg_p->cmd = SYS_MODULE_MLDSNP;
    msg_p->msg_size = req_size;

    msg_data_p = (MLDSNP_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = MLDSNP_OM_IPCCMD_GETNEXTGROUPPORTLIST;
    MLDSNP_POM_DEBUG_LINE();

    /*assign input*/
    msg_data_p->data.group_portlist.vid = *vid_p;
    memcpy(msg_data_p->data.group_portlist.gip_ar, gip_ap, MLDSNP_TYPE_IPV6_DST_IP_LEN);
    memcpy(msg_data_p->data.group_portlist.sip_ar, sip_ap, MLDSNP_TYPE_IPV6_SRC_IP_LEN);
    MLDSNP_POM_DEBUG_LINE();

    /*send ipc*/
    if (SYSFUN_SendRequestMsg(mldsnp_pom_ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
                              SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    /*assign output*/
    *vid_p = msg_data_p->data.group_portlist.vid;
    memcpy(gip_ap, msg_data_p->data.group_portlist.gip_ar, MLDSNP_TYPE_IPV6_DST_IP_LEN);
    memcpy(sip_ap, msg_data_p->data.group_portlist.sip_ar, MLDSNP_TYPE_IPV6_SRC_IP_LEN);
    memcpy(group_info_p, &msg_data_p->data.group_portlist.scgroup_info, sizeof(MLDSNP_OM_GroupInfo_T));


    MLDSNP_POM_DEBUG_LINE();
    return msg_data_p->type.result_bool;
}/*End of MLDSNP_POM_GetNextGroupPortlist*/
/*------------------------------------------------------------------------------
* Function : MLDSNP_POM_GetNextPortGroupSourceList
*------------------------------------------------------------------------------
* Purpose: This function get the port joined all group
* INPUT  : port_src_lst_p->vid    - the next vlan id
*          port_src_lst_p->port  - the port to get
* OUTPUT :  port_src_lst_p - the next vid, gip and source list
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  : This frunction only provide to user to access
*------------------------------------------------------------------------------
*/
BOOL_T MLDSNP_POM_GetNextPortGroupSourceList(
    MLDSNP_OM_PortSourceListInfo_T *port_source_list_info_p)
{
    const UI32_T msg_buf_size = (sizeof(((MLDSNP_OM_IPCMsg_T *)NULL)->data.port_source_list)
                                 + MLDSNP_OM_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    MLDSNP_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size, resp_size;
    MLDSNP_POM_DEBUG_LINE();

    /*assign size*/
    req_size = sizeof(msg_data_p->data.port_source_list)
               + MLDSNP_OM_MSGBUF_TYPE_SIZE;
    resp_size = msg_buf_size;

    msg_p = (SYSFUN_Msg_T *) & space_msg;
    msg_p->cmd = SYS_MODULE_MLDSNP;
    msg_p->msg_size = req_size;

    msg_data_p = (MLDSNP_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = MLDSNP_OM_IPCCMD_GETNEXTPORTGROUPSOURCELIST;
    MLDSNP_POM_DEBUG_LINE();

    /*assign input*/
    memcpy(&msg_data_p->data.port_source_list, port_source_list_info_p, sizeof(MLDSNP_OM_PortSourceListInfo_T));

    /*send ipc*/
    if (SYSFUN_SendRequestMsg(mldsnp_pom_ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
                              SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p) != SYSFUN_OK)
    {
        MLDSNP_POM_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    memcpy(port_source_list_info_p, &msg_data_p->data.port_source_list, sizeof(MLDSNP_OM_PortSourceListInfo_T));

    MLDSNP_POM_DEBUG_LINE();
    return msg_data_p->type.result_bool;
}
/*------------------------------------------------------------------------------
* Function : MLDSNP_POM_GetPortGroupSourceList
*------------------------------------------------------------------------------
* Purpose: This function get the port joined all group
* INPUT  : port_src_lst_p->vid    - the next vlan id
*          port_src_lst_p->port  - the port to get
* OUTPUT :  port_src_lst_p - the next vid, gip and source list
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  : This frunction only provide to user to access
*------------------------------------------------------------------------------
*/
BOOL_T MLDSNP_POM_GetPortGroupSourceList(
    MLDSNP_OM_PortSourceListInfo_T *port_source_list_info_p)
{
    const UI32_T msg_buf_size = (sizeof(((MLDSNP_OM_IPCMsg_T *)NULL)->data.port_source_list)
                                 + MLDSNP_OM_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    MLDSNP_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size, resp_size;
    MLDSNP_POM_DEBUG_LINE();

    /*assign size*/
    req_size = sizeof(msg_data_p->data.port_source_list)
               + MLDSNP_OM_MSGBUF_TYPE_SIZE;
    resp_size = msg_buf_size;

    msg_p = (SYSFUN_Msg_T *) & space_msg;
    msg_p->cmd = SYS_MODULE_MLDSNP;
    msg_p->msg_size = req_size;

    msg_data_p = (MLDSNP_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = MLDSNP_OM_IPCCMD_GETPORTGROUPSOURCELIST;
    MLDSNP_POM_DEBUG_LINE();

    /*assign input*/
    memcpy(&msg_data_p->data.port_source_list, port_source_list_info_p, sizeof(MLDSNP_OM_PortSourceListInfo_T));

    /*send ipc*/
    if (SYSFUN_SendRequestMsg(mldsnp_pom_ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
                              SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p) != SYSFUN_OK)
    {
        MLDSNP_POM_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    memcpy(port_source_list_info_p, &msg_data_p->data.port_source_list, sizeof(MLDSNP_OM_PortSourceListInfo_T));

    MLDSNP_POM_DEBUG_LINE();
    return msg_data_p->type.result_bool;
}

/*------------------------------------------------------------------------------
* Function : MLDSNP_POM_GetNextPortGroupSource
*------------------------------------------------------------------------------
* Purpose: This function get the port jonied (S,G)
* INPUT  : port_source_linfo_p->vid    - the next vlan id
*          port_source_linfo_p->port   - the port to get
*          port_source_linfo_p->gip_a  - current group
*          port_source_linfo_p->sip_a  - current group
* OUTPUT :  port_src_lst_p - the next vid, gip, sip
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  : This frunction only provide to user to access
*------------------------------------------------------------------------------
*/
BOOL_T MLDSNP_POM_GetNextPortGroupSource(
    MLDSNP_OM_PortSourceInfo_T *port_source_linfo_p)
{
    const UI32_T msg_buf_size = (sizeof(((MLDSNP_OM_IPCMsg_T *)NULL)->data.port_grp_src)
                                 + MLDSNP_OM_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    MLDSNP_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size, resp_size;
    MLDSNP_POM_DEBUG_LINE();

    /*assign size*/
    req_size = sizeof(msg_data_p->data.port_grp_src)
               + MLDSNP_OM_MSGBUF_TYPE_SIZE;
    resp_size = msg_buf_size;

    msg_p = (SYSFUN_Msg_T *) & space_msg;
    msg_p->cmd = SYS_MODULE_MLDSNP;
    msg_p->msg_size = req_size;

    msg_data_p = (MLDSNP_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = MLDSNP_OM_IPCCMD_GETNEXTPORTGROUPSOURCE;
    MLDSNP_POM_DEBUG_LINE();

    /*assign input*/
    memcpy(&msg_data_p->data.port_grp_src, port_source_linfo_p, sizeof(MLDSNP_OM_PortSourceInfo_T));

    /*send ipc*/
    if (SYSFUN_SendRequestMsg(mldsnp_pom_ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
                              SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p) != SYSFUN_OK)
    {
        MLDSNP_POM_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    memcpy(port_source_linfo_p, &msg_data_p->data.port_grp_src, sizeof(MLDSNP_OM_PortSourceInfo_T));

    MLDSNP_POM_DEBUG_LINE();
    return msg_data_p->type.result_bool;
}

/*------------------------------------------------------------------------------
* Function : MLDSNP_POM_GetNextPortGroupSourceHost
*------------------------------------------------------------------------------
* Purpose: This function get the port joined (S,G, host ip)
* INPUT  : port_source_linfo_p->vid    - the next vlan id
*          port_source_linfo_p->port   - the port to get
*          port_source_linfo_p->gip_a  - current group
*          port_source_linfo_p->sip_a  - current group
*          port_source_linfo_p->host_a  - current group
* OUTPUT :  port_src_lst_p - the next vid, gip, sip
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  : This frunction only provide to user to access
*------------------------------------------------------------------------------
*/
BOOL_T MLDSNP_POM_GetNextPortGroupSourceHost(
    MLDSNP_OM_PortHostInfo_T *port_host_p)
{
    const UI32_T msg_buf_size = (sizeof(((MLDSNP_OM_IPCMsg_T *)NULL)->data.port_host)
                                 + MLDSNP_OM_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    MLDSNP_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size, resp_size;
    MLDSNP_POM_DEBUG_LINE();

    /*assign size*/
    req_size = sizeof(msg_data_p->data.port_host)
               + MLDSNP_OM_MSGBUF_TYPE_SIZE;
    resp_size = msg_buf_size;

    msg_p = (SYSFUN_Msg_T *) & space_msg;
    msg_p->cmd = SYS_MODULE_MLDSNP;
    msg_p->msg_size = req_size;

    msg_data_p = (MLDSNP_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = MLDSNP_OM_IPCCMD_GETNEXTPORTGROUPSOURCEHOST;
    MLDSNP_POM_DEBUG_LINE();

    /*assign input*/
    memcpy(&msg_data_p->data.port_host, port_host_p, sizeof(MLDSNP_OM_PortHostInfo_T));

    /*send ipc*/
    if (SYSFUN_SendRequestMsg(mldsnp_pom_ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
                              SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p) != SYSFUN_OK)
    {
        MLDSNP_POM_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    memcpy(port_host_p, &msg_data_p->data.port_host, sizeof(MLDSNP_OM_PortHostInfo_T));

    MLDSNP_POM_DEBUG_LINE();
    return msg_data_p->type.result_bool;
}
/*------------------------------------------------------------------------------
* Function : MLDSNP_POM_GetNextRunningPortStaticGroup
*------------------------------------------------------------------------------
* Purpose: This function get the static join group port bit list
* INPUT  : *nxt_id   - current entry id
* OUTPUT : *nxt_id       - the next entry id
*           *vid    - the  vlan id
*          *gip_ap - the gruop ip
*          *sip_ap - the source ip
*          *port  -the port
*          *rec_type - ther record type
* RETUEN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   - not same as default
*           SYS_TYPE_GET_RUNNING_CFG_FAIL    - get failure
*           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE - same as default
* NOTES  :
*------------------------------------------------------------------------------*/
UI32_T MLDSNP_POM_GetNextRunningPortStaticGroup(
    UI16_T *nxt_id,
    UI16_T *vid,
    UI8_T  *gip_ap,
    UI8_T  *sip_ap,
    UI16_T  *port,
    MLDSNP_TYPE_RecordType_T *rec_type)
{
    const UI32_T msg_buf_size = (sizeof(((MLDSNP_OM_IPCMsg_T *)NULL)->data.running_join_static_group)
                                 + MLDSNP_OM_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    MLDSNP_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size, resp_size;
    MLDSNP_POM_DEBUG_LINE();

    /*assign size*/
    req_size = sizeof(msg_data_p->data.running_join_static_group)
               + MLDSNP_OM_MSGBUF_TYPE_SIZE;
    resp_size = msg_buf_size;

    msg_p = (SYSFUN_Msg_T *) & space_msg;
    msg_p->cmd = SYS_MODULE_MLDSNP;
    msg_p->msg_size = req_size;

    msg_data_p = (MLDSNP_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = MLDSNP_OM_IPCCMD_GETNEXTRUNNINGPORTJOINSTATICGROUP;

    /*assign input*/
    msg_data_p->data.running_join_static_group.id = *nxt_id;

    /*send ipc*/
    if (SYSFUN_SendRequestMsg(mldsnp_pom_ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
                              SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p) != SYSFUN_OK)
    {
        MLDSNP_POM_DEBUG_LINE();
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    /*assign output*/
    *nxt_id = msg_data_p->data.running_join_static_group.id;
    *vid = msg_data_p->data.running_join_static_group.vid;
    *port = msg_data_p->data.running_join_static_group.port;
    *rec_type = msg_data_p->data.running_join_static_group.rec_type;
    memcpy(gip_ap, msg_data_p->data.running_join_static_group.gip_ar, MLDSNP_TYPE_IPV6_DST_IP_LEN);
    memcpy(sip_ap, msg_data_p->data.running_join_static_group.sip_ar, MLDSNP_TYPE_IPV6_SRC_IP_LEN);

    MLDSNP_POM_DEBUG_LINE();
    return msg_data_p->type.result_ui32;
}/*End of MLDSNP_POM_GetNextRunningPortStaticGroup*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_POM_GetNextPortStaticGroup
*------------------------------------------------------------------------------
* Purpose: This function get the static join group port bit list
* INPUT  : *nxt_id   - current entry id
* OUTPUT : *nxt_id       - the next entry id
*           *vid    - the  vlan id
*          *gip_ap - the gruop ip
*          *sip_ap - the source ip
*          *port  -the port
*          *rec_type - ther record type
* RETUEN  : TRUE/FALSE
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_POM_GetNextPortStaticGroup(
    UI16_T *nxt_id,
    UI16_T *vid,
    UI8_T  *gip_ap,
    UI8_T  *sip_ap,
    UI16_T  *port,
    MLDSNP_TYPE_RecordType_T *rec_type)
{
    const UI32_T msg_buf_size = (sizeof(((MLDSNP_OM_IPCMsg_T *)NULL)->data.running_join_static_group)
                                 + MLDSNP_OM_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    MLDSNP_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size, resp_size;
    MLDSNP_POM_DEBUG_LINE();

    /*assign size*/
    req_size = sizeof(msg_data_p->data.running_join_static_group)
               + MLDSNP_OM_MSGBUF_TYPE_SIZE;
    resp_size = msg_buf_size;

    msg_p = (SYSFUN_Msg_T *) & space_msg;
    msg_p->cmd = SYS_MODULE_MLDSNP;
    msg_p->msg_size = req_size;

    msg_data_p = (MLDSNP_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = MLDSNP_OM_IPCCMD_GETNEXTPORTJOINSTATICGROUP;

    /*assign input*/
    msg_data_p->data.running_join_static_group.id = *nxt_id;

    /*send ipc*/
    if (SYSFUN_SendRequestMsg(mldsnp_pom_ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
                              SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p) != SYSFUN_OK)
    {
        MLDSNP_POM_DEBUG_LINE();
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    /*assign output*/
    *nxt_id = msg_data_p->data.running_join_static_group.id;
    *vid = msg_data_p->data.running_join_static_group.vid;
    *port = msg_data_p->data.running_join_static_group.port;
    *rec_type = msg_data_p->data.running_join_static_group.rec_type;
    memcpy(gip_ap, msg_data_p->data.running_join_static_group.gip_ar, MLDSNP_TYPE_IPV6_DST_IP_LEN);
    memcpy(sip_ap, msg_data_p->data.running_join_static_group.sip_ar, MLDSNP_TYPE_IPV6_SRC_IP_LEN);

    MLDSNP_POM_DEBUG_LINE();
    return msg_data_p->type.result_bool;
}/*End of MLDSNP_POM_GetNextRunningPortStaticGroup*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_POM_GetNextStaticGroupPortlist
*------------------------------------------------------------------------------
* Purpose: This function get the static join group port bit list
* INPUT  : *vid   - current vlan id
* OUTPUT : *vid    - the next vlan id
*          *gip_ap - the gruop ip
*          *sip_ap - the source ip
*          *group_info_p  - the group informaiton
* RETUEN  : TRUE/FALSE
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_POM_GetNextStaticGroupPortlist(
    UI16_T                *vid_p,
    UI8_T                 *gip_ap,
    UI8_T                 *sip_ap,
    UI8_T                 *static_portbitmap_p)
{
    const UI32_T msg_buf_size = (sizeof(((MLDSNP_OM_IPCMsg_T *)NULL)->data.group_portlist)
                                 + MLDSNP_OM_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    MLDSNP_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size, resp_size;
    MLDSNP_POM_DEBUG_LINE();

    /*assign size*/
    req_size = sizeof(msg_data_p->data.group_portlist)
               + MLDSNP_OM_MSGBUF_TYPE_SIZE;
    resp_size = msg_buf_size;

    msg_p = (SYSFUN_Msg_T *) & space_msg;
    msg_p->cmd = SYS_MODULE_MLDSNP;
    msg_p->msg_size = req_size;

    msg_data_p = (MLDSNP_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = MLDSNP_OM_IPCCMD_GETNEXTSTATICGROUPPORTLIST;
    MLDSNP_POM_DEBUG_LINE();

    /*assign input*/
    msg_data_p->data.group_portlist.vid = *vid_p;
    memcpy(msg_data_p->data.group_portlist.gip_ar, gip_ap, MLDSNP_TYPE_IPV6_DST_IP_LEN);
    memcpy(msg_data_p->data.group_portlist.sip_ar, sip_ap, MLDSNP_TYPE_IPV6_SRC_IP_LEN);
    MLDSNP_POM_DEBUG_LINE();

    /*send ipc*/
    if (SYSFUN_SendRequestMsg(mldsnp_pom_ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
                              SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    /*assign output*/
    *vid_p = msg_data_p->data.group_portlist.vid;
    memcpy(gip_ap, msg_data_p->data.group_portlist.gip_ar, MLDSNP_TYPE_IPV6_DST_IP_LEN);
    memcpy(sip_ap, msg_data_p->data.group_portlist.sip_ar, MLDSNP_TYPE_IPV6_SRC_IP_LEN);
    memcpy(static_portbitmap_p, &msg_data_p->data.group_portlist.scgroup_info.static_port_bitmap, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);


    MLDSNP_POM_DEBUG_LINE();
    return msg_data_p->type.result_bool;
}/*End of MLDSNP_POM_GetNextStaticGroupPortlist*/


/*------------------------------------------------------------------------------
* Function : MLDSNP_POM_GetStaticGroupPortlist
*------------------------------------------------------------------------------
* Purpose: This function get the static join group port bit list
* INPUT  : *vid    - the  vlan id
*          *gip_ap - the gruop ip
*          *sip_ap - the source ip
* OUTPUT : *group_info_p  - the group informaiton
* RETUEN  : TRUE/FALSE
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_POM_GetStaticGroupPortlist(
    UI16_T                vid,
    UI8_T                 *gip_ap,
    UI8_T                 *sip_ap,
    UI8_T                 *static_portbitmap_p)
{
    const UI32_T msg_buf_size = (sizeof(((MLDSNP_OM_IPCMsg_T *)NULL)->data.group_portlist)
                                 + MLDSNP_OM_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    MLDSNP_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size, resp_size;
    MLDSNP_POM_DEBUG_LINE();

    /*assign size*/
    req_size = sizeof(msg_data_p->data.group_portlist)
               + MLDSNP_OM_MSGBUF_TYPE_SIZE;
    resp_size = msg_buf_size;

    msg_p = (SYSFUN_Msg_T *) & space_msg;
    msg_p->cmd = SYS_MODULE_MLDSNP;
    msg_p->msg_size = req_size;

    msg_data_p = (MLDSNP_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = MLDSNP_OM_IPCCMD_GETSTATICGROUPPORTLIST;

    /*assign input*/
    msg_data_p->data.group_portlist.vid = vid;
    memcpy(msg_data_p->data.group_portlist.gip_ar, gip_ap, MLDSNP_TYPE_IPV6_DST_IP_LEN);
    memcpy(msg_data_p->data.group_portlist.sip_ar, sip_ap, MLDSNP_TYPE_IPV6_SRC_IP_LEN);

    /*send ipc*/
    if (SYSFUN_SendRequestMsg(mldsnp_pom_ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
                              SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p) != SYSFUN_OK)
    {
        MLDSNP_POM_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    memcpy(static_portbitmap_p, &msg_data_p->data.group_portlist.scgroup_info.static_port_bitmap, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);

    MLDSNP_POM_DEBUG_LINE();
    return msg_data_p->type.result_bool;
}/*End of MLDSNP_POM_GetStaticGroupPortlist*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_POM_GetOldVerQuerierPresentTimeOut
*------------------------------------------------------------------------------
* Purpose: This function get the old version querier present time out value
* INPUT  : vid - the vlan id

* OUTPUT : *time_out_p   - the time out in seconds
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :9.12 in RFC3180
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_POM_GetOldVerQuerierPresentTimeOut(
    UI16_T *time_out_p)
{
    const UI32_T msg_buf_size = (sizeof(((MLDSNP_OM_IPCMsg_T *)NULL)->data.ui16_v)
                                 + MLDSNP_OM_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    MLDSNP_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size, resp_size;
    MLDSNP_POM_DEBUG_LINE();

    /*assign size*/
    req_size = MLDSNP_OM_MSGBUF_TYPE_SIZE;
    resp_size = msg_buf_size;

    msg_p = (SYSFUN_Msg_T *) & space_msg;
    msg_p->cmd = SYS_MODULE_MLDSNP;
    msg_p->msg_size = req_size;

    msg_data_p = (MLDSNP_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = MLDSNP_OM_IPCCMD_GETOLDVERQUERIERPRESENTTIMEOUT;

    /*assign input*/

    /*send ipc*/
    if (SYSFUN_SendRequestMsg(mldsnp_pom_ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
                              SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p) != SYSFUN_OK)
    {
        MLDSNP_POM_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    *time_out_p = msg_data_p->data.ui16_v;

    MLDSNP_POM_DEBUG_LINE();
    return msg_data_p->type.result_bool;
}/*End of MLDSNP_OM_GetOldVerQuerierPresentTimeOut*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_POM_GetOtherQueryPresentInterval
*------------------------------------------------------------------------------
* Purpose: This function get the other querier present interval
* INPUT  : None
* OUTPUT : *interval_p   - the interval in seconds
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :  7.5, 9.5 robust * query_interval + query_rsponse_interval
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_POM_GetOtherQueryPresentInterval(
    UI32_T *interval_p)
{
    const UI32_T msg_buf_size = (sizeof(((MLDSNP_OM_IPCMsg_T *)NULL)->data.ui32_v)
                                 + MLDSNP_OM_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    MLDSNP_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size, resp_size;
    MLDSNP_POM_DEBUG_LINE();

    /*assign size*/
    req_size = MLDSNP_OM_MSGBUF_TYPE_SIZE;
    resp_size = msg_buf_size;

    msg_p = (SYSFUN_Msg_T *) & space_msg;
    msg_p->cmd = SYS_MODULE_MLDSNP;
    msg_p->msg_size = req_size;

    msg_data_p = (MLDSNP_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = MLDSNP_OM_IPCCMD_GETOTHERQUERYPRESENTINTERVAL;

    /*assign input*/

    /*send ipc*/
    if (SYSFUN_SendRequestMsg(mldsnp_pom_ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
                              SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p) != SYSFUN_OK)
    {
        MLDSNP_POM_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    *interval_p = msg_data_p->data.ui32_v;

    MLDSNP_POM_DEBUG_LINE();
    return msg_data_p->type.result_bool;
}/*End of MLDSNP_POM_GetOtherQueryPresentInterval*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_POM_GetQuerierRunningStatus
*------------------------------------------------------------------------------
* Purpose: This function get the querier running status of the input vlan
* INPUT  : vid  - the vlan id
* OUTPUT : *status_p  - the querier running status
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_POM_GetQuerierRunningStatus(
    UI16_T vid,
    MLDSNP_TYPE_QuerierStatus_T *status_p)
{
    const UI32_T msg_buf_size = (sizeof(((MLDSNP_OM_IPCMsg_T *)NULL)->data.running_queier)
                                 + MLDSNP_OM_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    MLDSNP_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size, resp_size;
    MLDSNP_POM_DEBUG_LINE();

    /*assign size*/
    req_size = sizeof(msg_data_p->data.running_queier)
               + MLDSNP_OM_MSGBUF_TYPE_SIZE;
    resp_size = msg_buf_size;

    msg_p = (SYSFUN_Msg_T *) & space_msg;
    msg_p->cmd = SYS_MODULE_MLDSNP;
    msg_p->msg_size = req_size;

    msg_data_p = (MLDSNP_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = MLDSNP_OM_IPCCMD_GETQUERIERRUNNINGSTATUS;

    /*assign input*/
    msg_data_p->data.running_queier.vid = vid;

    /*send ipc*/
    if (SYSFUN_SendRequestMsg(mldsnp_pom_ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
                              SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p) != SYSFUN_OK)
    {
        MLDSNP_POM_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    *status_p = msg_data_p->data.running_queier.status;

    MLDSNP_POM_DEBUG_LINE();
    return msg_data_p->type.result_ui32;
}/*End of MLDSNP_POM_GetQuerierRunningStatus*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_POM_GetQuerierStatus
*------------------------------------------------------------------------------
* Purpose: This function set the querier status of input vlan
* INPUT  : none
* OUTPUT : *status  - the querier status
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_POM_GetQuerierStatus(
    MLDSNP_TYPE_QuerierStatus_T *status_p)
{
    const UI32_T msg_buf_size = (sizeof(((MLDSNP_OM_IPCMsg_T *)NULL)->data.querier_status)
                                 + MLDSNP_OM_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    MLDSNP_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size, resp_size;
    MLDSNP_POM_DEBUG_LINE();

    /*assign size*/
    req_size = MLDSNP_OM_MSGBUF_TYPE_SIZE;
    resp_size = msg_buf_size;

    msg_p = (SYSFUN_Msg_T *) & space_msg;
    msg_p->cmd = SYS_MODULE_MLDSNP;
    msg_p->msg_size = req_size;

    msg_data_p = (MLDSNP_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = MLDSNP_OM_IPCCMD_GETQUERIERSTATUS;

    /*assign input*/

    /*send ipc*/
    if (SYSFUN_SendRequestMsg(mldsnp_pom_ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
                              SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p) != SYSFUN_OK)
    {
        MLDSNP_POM_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    *status_p = msg_data_p->data.querier_status;

    MLDSNP_POM_DEBUG_LINE();
    return msg_data_p->type.result_bool;
}/*End of MLDSNP_POM_GetQuerierStatus*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_POM_GetQueryInterval
*------------------------------------------------------------------------------
* Purpose: This funcgtion get the query interval
* INPUT  : None
* OUTPUT : *interval_p - the interval in seconds
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :   7.2, 9.2
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_POM_GetQueryInterval(
    UI16_T *interval_p)
{
    const UI32_T msg_buf_size = (sizeof(((MLDSNP_OM_IPCMsg_T *)NULL)->data.ui16_v)
                                 + MLDSNP_OM_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    MLDSNP_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size, resp_size;
    MLDSNP_POM_DEBUG_LINE();

    /*assign size*/
    req_size = MLDSNP_OM_MSGBUF_TYPE_SIZE;
    resp_size = msg_buf_size;

    msg_p = (SYSFUN_Msg_T *) & space_msg;
    msg_p->cmd = SYS_MODULE_MLDSNP;
    msg_p->msg_size = req_size;

    msg_data_p = (MLDSNP_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = MLDSNP_OM_IPCCMD_GETQUERYINTERVAL;

    /*assign input*/

    /*send ipc*/
    if (SYSFUN_SendRequestMsg(mldsnp_pom_ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
                              SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p) != SYSFUN_OK)
    {
        MLDSNP_POM_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    *interval_p = msg_data_p->data.ui16_v;

    MLDSNP_POM_DEBUG_LINE();
    return msg_data_p->type.result_bool;
}/*End of MLDSNP_POM_GetQueryInterval*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_POM_GetQueryResponseInterval
*------------------------------------------------------------------------------
* Purpose: This function get the query response interval
* INPUT  : None
* OUTPUT : *interval_p  - the query response interval
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :   7.3, 9.3
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_POM_GetQueryResponseInterval(
    UI16_T *interval_p)
{
    const UI32_T msg_buf_size = (sizeof(((MLDSNP_OM_IPCMsg_T *)NULL)->data.ui16_v)
                                 + MLDSNP_OM_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    MLDSNP_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size, resp_size;
    MLDSNP_POM_DEBUG_LINE();

    /*assign size*/
    req_size = MLDSNP_OM_MSGBUF_TYPE_SIZE;
    resp_size = msg_buf_size;

    msg_p = (SYSFUN_Msg_T *) & space_msg;
    msg_p->cmd = SYS_MODULE_MLDSNP;
    msg_p->msg_size = req_size;

    msg_data_p = (MLDSNP_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = MLDSNP_OM_IPCCMD_GETQUERYRESPONSEINTERVAL;

    /*assign input*/

    /*send ipc*/
    if (SYSFUN_SendRequestMsg(mldsnp_pom_ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
                              SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p) != SYSFUN_OK)
    {
        MLDSNP_POM_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    *interval_p = msg_data_p->data.ui16_v;

    MLDSNP_POM_DEBUG_LINE();
    return msg_data_p->type.result_bool;
}/*End of MLDSNP_POM_GetQueryResponseInterval*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_POM_GetRobustnessValue
*------------------------------------------------------------------------------
* Purpose: This function get the robust ess value
* INPUT  : None
* OUTPUT : *value_p - the robustness value
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :  7.1, 7.9, 9.1, 9.9
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_POM_GetRobustnessValue(
    UI16_T *value_p)
{
    const UI32_T msg_buf_size = (sizeof(((MLDSNP_OM_IPCMsg_T *)NULL)->data.ui16_v)
                                 + MLDSNP_OM_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    MLDSNP_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size, resp_size;
    MLDSNP_POM_DEBUG_LINE();

    /*assign size*/
    req_size = MLDSNP_OM_MSGBUF_TYPE_SIZE;
    resp_size = msg_buf_size;

    msg_p = (SYSFUN_Msg_T *) & space_msg;
    msg_p->cmd = SYS_MODULE_MLDSNP;
    msg_p->msg_size = req_size;

    msg_data_p = (MLDSNP_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = MLDSNP_OM_IPCCMD_GETROBUSTNESSVALUE;

    /*assign input*/

    /*send ipc*/
    if (SYSFUN_SendRequestMsg(mldsnp_pom_ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
                              SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p) != SYSFUN_OK)
    {
        MLDSNP_POM_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    *value_p = msg_data_p->data.ui16_v;

    MLDSNP_POM_DEBUG_LINE();
    return msg_data_p->type.result_bool;
}/*End of MLDSNP_POM_GetRobustnessValue*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_POM_GetRouterExpireTime
*------------------------------------------------------------------------------
* Purpose: This function get the router expire time
* INPUT  :
* OUTPUT : exp_time_p  - the expire time
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_POM_GetRouterExpireTime(
    UI16_T *exp_time_p)
{
    const UI32_T msg_buf_size = (sizeof(((MLDSNP_OM_IPCMsg_T *)NULL)->data.ui16_v)
                                 + MLDSNP_OM_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    MLDSNP_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size, resp_size;
    MLDSNP_POM_DEBUG_LINE();

    /*assign size*/
    req_size = MLDSNP_OM_MSGBUF_TYPE_SIZE;
    resp_size = msg_buf_size;

    msg_p = (SYSFUN_Msg_T *) & space_msg;
    msg_p->cmd = SYS_MODULE_MLDSNP;
    msg_p->msg_size = req_size;

    msg_data_p = (MLDSNP_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = MLDSNP_OM_IPCCMD_GETROUTEREXPIRETIME;

    /*assign input*/

    /*send ipc*/
    if (SYSFUN_SendRequestMsg(mldsnp_pom_ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
                              SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p) != SYSFUN_OK)
    {
        MLDSNP_POM_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    *exp_time_p = msg_data_p->data.ui16_v;

    MLDSNP_POM_DEBUG_LINE();
    return msg_data_p->type.result_bool;
}/*End of MLDSNP_POM_GetRouterExpireTime*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_POM_GetRunningGlobalConf
*------------------------------------------------------------------------------
* Purpose: This function get the running configuration
* INPUT  : None
*
* OUTPUT : *mldsnp_global_conf_p - the global configuration
* RETUEN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   - not same as default
*           SYS_TYPE_GET_RUNNING_CFG_FAIL    - get failure
*           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE - same as default
* NOTES  :
*------------------------------------------------------------------------------*/
UI32_T MLDSNP_POM_GetRunningGlobalConf(
    MLDSNP_OM_RunningCfg_T *mldsnp_global_conf_p)
{
    const UI32_T msg_buf_size = (sizeof(((MLDSNP_OM_IPCMsg_T *)NULL)->data.mldsnp_running_global_conf)
                                 + MLDSNP_OM_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    MLDSNP_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size, resp_size;
    MLDSNP_POM_DEBUG_LINE();

    /*assign size*/
    req_size = MLDSNP_OM_MSGBUF_TYPE_SIZE;
    resp_size = msg_buf_size;

    msg_p = (SYSFUN_Msg_T *) & space_msg;
    msg_p->cmd = SYS_MODULE_MLDSNP;
    msg_p->msg_size = req_size;

    msg_data_p = (MLDSNP_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = MLDSNP_OM_IPCCMD_GETRUNNINGGLOBALCONF;

    /*assign input*/

    /*send ipc*/
    if (SYSFUN_SendRequestMsg(mldsnp_pom_ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
                              SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p) != SYSFUN_OK)
    {
        MLDSNP_POM_DEBUG_LINE();
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    /*assign output*/
    memcpy(mldsnp_global_conf_p, &msg_data_p->data.mldsnp_running_global_conf, sizeof(MLDSNP_OM_RunningCfg_T));

    MLDSNP_POM_DEBUG_LINE();
    return msg_data_p->type.result_ui32;
}/*End of MLDSNP_POM_GetRunningGlobalConf*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_POM_GetRunningImmediateLeaveStatus
*------------------------------------------------------------------------------
* Purpose: This function get the immediate running status
* INPUT  : vid  - the vlan id
*
* OUTPUT : *status_p   - the immediate status
* RETUEN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   - not same as default
*           SYS_TYPE_GET_RUNNING_CFG_FAIL    - get failure
*           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE - same as default
* NOTES  :
*------------------------------------------------------------------------------*/
UI32_T MLDSNP_POM_GetRunningImmediateLeaveStatus(
    UI32_T vid,
    MLDSNP_TYPE_ImmediateStatus_T *imme_status_p,
    MLDSNP_TYPE_ImmediateByHostStatus_T *imme_byhost_status_p)
{
    const UI32_T msg_buf_size = (sizeof(((MLDSNP_OM_IPCMsg_T *)NULL)->data.immediate_leave)
                                 + MLDSNP_OM_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    MLDSNP_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size, resp_size;
    MLDSNP_POM_DEBUG_LINE();
    /*assign size*/
    req_size = sizeof(msg_data_p->data.immediate_leave.vid)
               + MLDSNP_OM_MSGBUF_TYPE_SIZE;
    resp_size = msg_buf_size;

    msg_p = (SYSFUN_Msg_T *) & space_msg;
    msg_p->cmd = SYS_MODULE_MLDSNP;
    msg_p->msg_size = req_size;

    msg_data_p = (MLDSNP_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = MLDSNP_OM_IPCCMD_GETRUNNINGIMMEDIATELEAVESTATUS;

    /*assign input*/
    msg_data_p->data.immediate_leave.vid = vid;

    /*send ipc*/
    if (SYSFUN_SendRequestMsg(mldsnp_pom_ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
                              SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p) != SYSFUN_OK)
    {
        MLDSNP_POM_DEBUG_LINE();
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    /*assign output*/
    *imme_status_p = msg_data_p->data.immediate_leave.status;
    *imme_byhost_status_p = msg_data_p->data.immediate_leave.byhost_status;

    MLDSNP_POM_DEBUG_LINE();
    return msg_data_p->type.result_ui32;
}/*End of MLDSNP_POM_GetRunningImmediateLeaveStatus*/


/*------------------------------------------------------------------------------
* Function : MLDSNP_POM_GetRunningRouterPortList
*------------------------------------------------------------------------------
* Purpose: This function get next the run static router port bit list
* INPUT  : *vid - the vlan id to get next vlan id
*
* OUTPUT :  *router_port_bitmap_p  - the router port bit map pointer
*
* RETUEN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   - not same as default
*           SYS_TYPE_GET_RUNNING_CFG_FAIL    - get failure
*           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE - same as default
* NOTES  :
*------------------------------------------------------------------------------*/
UI32_T MLDSNP_POM_GetRunningRouterPortList(
    UI16_T vid,
    UI8_T  *router_port_bitmap_p)
{
    const UI32_T msg_buf_size = (sizeof(((MLDSNP_OM_IPCMsg_T *)NULL)->data.running_router_portlist)
                                 + MLDSNP_OM_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    MLDSNP_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size, resp_size;
    MLDSNP_POM_DEBUG_LINE();

    /*assign size*/
    req_size = sizeof(msg_data_p->data.running_router_portlist.vid)
               + MLDSNP_OM_MSGBUF_TYPE_SIZE;
    resp_size = msg_buf_size;

    msg_p = (SYSFUN_Msg_T *) & space_msg;
    msg_p->cmd = SYS_MODULE_MLDSNP;
    msg_p->msg_size = req_size;

    msg_data_p = (MLDSNP_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = MLDSNP_OM_IPCCMD_GETRUNNINGROUTERPORTLIST;

    /*assign input*/
    msg_data_p->data.running_router_portlist.vid = vid;

    /*send ipc*/
    if (SYSFUN_SendRequestMsg(mldsnp_pom_ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
                              SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p) != SYSFUN_OK)
    {
        MLDSNP_POM_DEBUG_LINE();
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    /*assign output*/
    memcpy(router_port_bitmap_p,
           msg_data_p->data.running_router_portlist.router_port_bitmap,
           SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);

    MLDSNP_POM_DEBUG_LINE();
    return msg_data_p->type.result_ui32;
}/*End of MLDSNP_POM_GetRunningRouterPortList*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_POM_GetUnknownFloodBehavior
*------------------------------------------------------------------------------
* Purpose: This function get the unknown multicast data flood behavior
* INPUT  :  flood_behavior_p - the returned router port info
*          vlan_id          - which vlan to get
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_POM_GetUnknownFloodBehavior(
    UI32_T vlan_id,
    MLDSNP_TYPE_UnknownBehavior_T *flood_behavior_p)
{
    const UI32_T msg_buf_size = (sizeof(((MLDSNP_OM_IPCMsg_T *)NULL)->data.flood)
                                 + MLDSNP_OM_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    MLDSNP_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size, resp_size;
    MLDSNP_POM_DEBUG_LINE();

    /*assign size*/
    req_size = msg_buf_size;
    resp_size = msg_buf_size;

    msg_p = (SYSFUN_Msg_T *) & space_msg;
    msg_p->cmd = SYS_MODULE_MLDSNP;
    msg_p->msg_size = req_size;

    msg_data_p = (MLDSNP_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = MLDSNP_OM_IPCCMD_GETUNKNOWNFLOODBEHAVIOR;

    /*assign input*/
    msg_data_p->data.flood.vlan_id = vlan_id;

    /*send ipc*/
    if (SYSFUN_SendRequestMsg(mldsnp_pom_ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
                              SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p) != SYSFUN_OK)
    {
        MLDSNP_POM_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    *flood_behavior_p = msg_data_p->data.flood.flood_behavior;

    MLDSNP_POM_DEBUG_LINE();
    return msg_data_p->type.result_bool;
}/*End of MLDSNP_POM_GetUnknownFloodBehavior*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_POM_GetRunningUnknownFloodBehavior
*------------------------------------------------------------------------------
* Purpose: This function get the unknown multicast data flood behavior
* INPUT  : flood_behavior_p - the returned router port info
*          vlan_id          - which vlan to get
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
UI32_T MLDSNP_POM_GetRunningUnknownFloodBehavior(
    UI32_T vlan_id,
    MLDSNP_TYPE_UnknownBehavior_T *flood_behavior_p)
{
    const UI32_T msg_buf_size = (sizeof(((MLDSNP_OM_IPCMsg_T *)NULL)->data.flood)
                                 + MLDSNP_OM_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    MLDSNP_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size, resp_size;
    MLDSNP_POM_DEBUG_LINE();

    /*assign size*/
    req_size = msg_buf_size;
    resp_size = msg_buf_size;

    msg_p = (SYSFUN_Msg_T *) & space_msg;
    msg_p->cmd = SYS_MODULE_MLDSNP;
    msg_p->msg_size = req_size;

    msg_data_p = (MLDSNP_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = MLDSNP_OM_IPCCMD_GETRUNNINGUNKNOWNFLOODBEHAVIOR;

    /*assign input*/
    msg_data_p->data.flood.vlan_id = vlan_id;
    /*send ipc*/
    if (SYSFUN_SendRequestMsg(mldsnp_pom_ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
                              SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p) != SYSFUN_OK)
    {
        MLDSNP_POM_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    *flood_behavior_p = msg_data_p->data.flood.flood_behavior;

    MLDSNP_POM_DEBUG_LINE();
    return msg_data_p->type.result_ui32;
}/*End of MLDSNP_POM_GetUnknownFloodBehavior*/
/*------------------------------------------------------------------------------
* Function : MLDSNP_POM_GetNextRunningVlanConfig
*------------------------------------------------------------------------------
* Purpose: This function get the vla configuration
* INPUT  : flood_behavior_p - the returned router port info
*          vlan_id          - which vlan to get
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_POM_GetNextRunningVlanConfig(
    UI32_T *next_vlan_p,
    MLDSNP_OM_VlanRunningCfg_T *vlan_cfg_p)
{
    const UI32_T msg_buf_size = (sizeof(((MLDSNP_OM_IPCMsg_T *)NULL)->data.vlan_cfg)
                                 + MLDSNP_OM_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    MLDSNP_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size, resp_size;
    MLDSNP_POM_DEBUG_LINE();

    /*assign size*/
    req_size = msg_buf_size;
    resp_size = msg_buf_size;

    msg_p = (SYSFUN_Msg_T *) & space_msg;
    msg_p->cmd = SYS_MODULE_MLDSNP;
    msg_p->msg_size = req_size;

    msg_data_p = (MLDSNP_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = MLDSNP_OM_IPCCMD_GETRUNNINGVLANCFG;

    /*assign input*/
    msg_data_p->data.vlan_cfg.vlan_id = *next_vlan_p;
    /*send ipc*/
    if (SYSFUN_SendRequestMsg(mldsnp_pom_ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
                              SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p) != SYSFUN_OK)
    {
        MLDSNP_POM_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    *vlan_cfg_p = msg_data_p->data.vlan_cfg.vlan_cfg;
    *next_vlan_p =msg_data_p->data.vlan_cfg.vlan_id;

    MLDSNP_POM_DEBUG_LINE();
    return msg_data_p->type.result_bool;
}/*End of MLDSNP_POM_GetUnknownFloodBehavior*/
#if (SYS_CPNT_MLDSNP_PROXY == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_POM_GetProxyReporting
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to get MLDSNP status.
 * INPUT   : *mldsnp_status - MLDSNP status output buffer
 * OUTPUT  : *mldsnp_status - MLDSNP status
 * RETURN  : TRUE  - success
 *           FALSE - failure
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_POM_GetProxyReporting(MLDSNP_TYPE_ProxyReporting_T *status)
{
    const UI32_T msg_buf_size = (sizeof(((MLDSNP_OM_IPCMsg_T *)NULL)->data.proxy_reporting)
                                 + MLDSNP_OM_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    MLDSNP_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size, resp_size;

    MLDSNP_POM_DEBUG_LINE();

    /*assign size*/
    req_size = MLDSNP_OM_MSGBUF_TYPE_SIZE;
    resp_size = msg_buf_size;

    msg_p = (SYSFUN_Msg_T *) & space_msg;
    msg_p->cmd = SYS_MODULE_MLDSNP;
    msg_p->msg_size = req_size;

    msg_data_p = (MLDSNP_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = MLDSNP_OM_IPCCMD_GET_PROXY_REPORTING;

    /*assign input*/

    /*send ipc*/
    if (SYSFUN_SendRequestMsg(mldsnp_pom_ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
                              SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p) != SYSFUN_OK)
    {
        MLDSNP_POM_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    *status = msg_data_p->data.proxy_reporting;

    MLDSNP_POM_DEBUG_LINE();
    return msg_data_p->type.result_bool;
}
/*------------------------------------------------------------------------------
* Function : MLDSNP_POM_GetUnsolicitedReportInterval
*------------------------------------------------------------------------------
* Purpose: This function Get the unsolicitedReportInterval
* INPUT  : None
* OUTPUT : *interval_p  - the inteval in seconds
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :  7.9, 9.11
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_POM_GetUnsolicitedReportInterval(
    UI32_T *interval_p)
{
    const UI32_T msg_buf_size = (sizeof(((MLDSNP_OM_IPCMsg_T *)NULL)->data.ui32_v)
                                 + MLDSNP_OM_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    MLDSNP_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size, resp_size;

    MLDSNP_POM_DEBUG_LINE();

    /*assign size*/
    req_size = MLDSNP_OM_MSGBUF_TYPE_SIZE;
    resp_size = msg_buf_size;

    msg_p = (SYSFUN_Msg_T *) & space_msg;
    msg_p->cmd = SYS_MODULE_MLDSNP;
    msg_p->msg_size = req_size;

    msg_data_p = (MLDSNP_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = MLDSNP_OM_IPCCMD_GETUNSOLICITEDREPORTINTERVAL;

    /*assign input*/

    /*send ipc*/
    if (SYSFUN_SendRequestMsg(mldsnp_pom_ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
                              SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p) != SYSFUN_OK)
    {
        MLDSNP_POM_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    *interval_p = msg_data_p->data.ui32_v;

    MLDSNP_POM_DEBUG_LINE();
    return msg_data_p->type.result_bool;
}/*End of MLDSNP_POM_GetUnsolicitedReportInterval*/
#endif

/*------------------------------------------------------------------------------
* Function : MLDSNP_POM_GetVlanRouterPortlist
*------------------------------------------------------------------------------
* Purpose: This function get the vlan's router list
* INPUT  : vid                       - the vlan id
*
* OUTPUT : router_port_list  - the router port info
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  : This function is only provide to UI to access
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_POM_GetVlanRouterPortlist(
    UI16_T                     vid,
    MLDSNP_OM_RouterPortList_T *router_port_list)
{
    const UI32_T msg_buf_size = (sizeof(((MLDSNP_OM_IPCMsg_T *)NULL)->data.vlan_router_portlist)
                                 + MLDSNP_OM_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    MLDSNP_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size, resp_size;

    MLDSNP_POM_DEBUG_LINE();

    /*assign size*/
    req_size = sizeof(msg_data_p->data.vlan_router_portlist.vid)
               + MLDSNP_OM_MSGBUF_TYPE_SIZE;
    resp_size = msg_buf_size;

    msg_p = (SYSFUN_Msg_T *) & space_msg;
    msg_p->cmd = SYS_MODULE_MLDSNP;
    msg_p->msg_size = req_size;

    msg_data_p = (MLDSNP_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = MLDSNP_OM_IPCCMD_GETVLANROUTERPORTLIST;

    /*assign input*/
    msg_data_p->data.vlan_router_portlist.vid = vid;

    /*send ipc*/
    if (SYSFUN_SendRequestMsg(mldsnp_pom_ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
                              SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p) != SYSFUN_OK)
    {
        MLDSNP_POM_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    memcpy(router_port_list, &msg_data_p->data.vlan_router_portlist.router_port_list, sizeof(MLDSNP_OM_RouterPortList_T));

    MLDSNP_POM_DEBUG_LINE();
    return msg_data_p->type.result_bool;
}/*ENd of MLDSNP_POM_GetVlanRouterPortlist*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_POM_GetNextVlanRouterPortlist
*------------------------------------------------------------------------------
* Purpose: This function get the next vlan's router list
* INPUT  : *vid_p                       - the vlan id
*
* OUTPUT : router_port_list  - the router port info
*         *vid_p        - the next vlan id
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  : This function is only provide to UI to access
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_POM_GetNextVlanRouterPortlist(
    UI16_T                     *vid_p,
    MLDSNP_OM_RouterPortList_T *router_port_list)
{
    const UI32_T msg_buf_size = (sizeof(((MLDSNP_OM_IPCMsg_T *)NULL)->data.vlan_router_portlist)
                                 + MLDSNP_OM_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    MLDSNP_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size, resp_size;

    MLDSNP_POM_DEBUG_LINE();

    /*assign size*/
    req_size = sizeof(msg_data_p->data.vlan_router_portlist.vid)
               + MLDSNP_OM_MSGBUF_TYPE_SIZE;
    resp_size = msg_buf_size;

    msg_p = (SYSFUN_Msg_T *) & space_msg;
    msg_p->cmd = SYS_MODULE_MLDSNP;
    msg_p->msg_size = req_size;

    msg_data_p = (MLDSNP_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = MLDSNP_OM_IPCCMD_GETNEXTVLANROUTERPORTLIST;

    /*assign input*/
    msg_data_p->data.vlan_router_portlist.vid = *vid_p;

    /*send ipc*/
    if (SYSFUN_SendRequestMsg(mldsnp_pom_ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
                              SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p) != SYSFUN_OK)
    {
        MLDSNP_POM_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    *vid_p = msg_data_p->data.vlan_router_portlist.vid;
    memcpy(router_port_list, &msg_data_p->data.vlan_router_portlist.router_port_list, sizeof(MLDSNP_OM_RouterPortList_T));

    MLDSNP_POM_DEBUG_LINE();
    return msg_data_p->type.result_bool;
}/*ENd of MLDSNP_POM_GetNextVlanRouterPortlist*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_POM_GetRouterPortExpireInterval
*------------------------------------------------------------------------------
* Purpose: This function get the next vlan's router list
* INPUT  : vid_p  - the vlan id
*          lport  - which port
* OUTPUT : *expire_p - expire time
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  : This function is only provide to UI to access
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_POM_GetRouterPortExpireInterval(
    UI16_T vid,
    UI32_T lport,
    UI32_T *expire_p)
{
    const UI32_T msg_buf_size = (sizeof(((MLDSNP_OM_IPCMsg_T *)NULL)->data.u32a1_u32a2_u32_a3)
                                 + MLDSNP_OM_MSGBUF_TYPE_SIZE);
    SYSFUN_Msg_T *msg_p;
    MLDSNP_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];
    UI32_T req_size, resp_size;

    MLDSNP_POM_DEBUG_LINE();

    /*assign size*/
    req_size = sizeof(msg_data_p->data.u32a1_u32a2_u32_a3)
               + MLDSNP_OM_MSGBUF_TYPE_SIZE;
    resp_size = msg_buf_size;

    msg_p = (SYSFUN_Msg_T *) & space_msg;
    msg_p->cmd = SYS_MODULE_MLDSNP;
    msg_p->msg_size = req_size;

    msg_data_p = (MLDSNP_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = MLDSNP_OM_IPCCMD_GETVLANROUTERPORTEXPIRE;

    /*assign input*/
    msg_data_p->data.u32a1_u32a2_u32_a3.u32_a1 = vid;
    msg_data_p->data.u32a1_u32a2_u32_a3.u32_a2 = lport;

    /*send ipc*/
    if (SYSFUN_SendRequestMsg(mldsnp_pom_ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
                              SYSFUN_SYSTEM_EVENT_IPCMSG, resp_size, msg_p) != SYSFUN_OK)
    {
        MLDSNP_POM_DEBUG_LINE();
        return FALSE;
    }

    /*assign output*/
    *expire_p = msg_data_p->data.u32a1_u32a2_u32_a3.u32_a3;

    MLDSNP_POM_DEBUG_LINE();
    return msg_data_p->type.result_bool;
}/*ENd of MLDSNP_POM_GetNextVlanRouterPortlist*/

#if (SYS_CPNT_FILTER_THROOTTLE_MLDSNP == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_POM_GetMLDFilterStatus
 *-------------------------------------------------------------------------
 * PURPOSE : Get MLD filter status
 * INPUT   : None
 * OUTPUT  : *status - filter status
 * RETURN  : TRUE  - success
 *           FALSE - fail
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_POM_GetMLDFilterStatus(UI32_T *status)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(MLDSNP_OM_GET_MSGBUFSIZE(MLDSNP_OM_IPCMsg_MldSnoopFilter_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    MLDSNP_OM_IPCMsg_MldSnoopFilter_T *data_p;

    if (status == NULL)
        return FALSE;

    data_p = MLDSNP_OM_MSG_DATA(msg_p);

    MLDSNP_POM_SendMsg(MLDSNP_OM_IPCCMD_GET_MLD_FILTER_STATUS,
                       msg_p,
                       MLDSNP_OM_GET_MSGBUFSIZE(MLDSNP_OM_IPCMsg_MldSnoopFilter_T),
                       MLDSNP_OM_GET_MSGBUFSIZE(MLDSNP_OM_IPCMsg_MldSnoopFilter_T),
                       (UI32_T)FALSE);

    *status = data_p->mldsnp_FilterStatus;

    return (BOOL_T)MLDSNP_OM_MSG_RETVAL(msg_p);
}
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_POM_GetNextMLDProfileID
 *-------------------------------------------------------------------------
 * PURPOSE : Get next profile id
 * INPUT   : *pid - current profile id
 * OUTPUT  : *pid - next profile id
 * RETURN  : TRUE  - success
 *           FALSE - fail
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_POM_GetNextMLDProfileID(UI32_T *pid)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(MLDSNP_OM_GET_MSGBUFSIZE(MLDSNP_OM_IPCMsg_MldSnoopProfile_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    MLDSNP_OM_IPCMsg_MldSnoopProfile_T *data_p;

    data_p = MLDSNP_OM_MSG_DATA(msg_p);
    data_p->mldsnp_Profile_id = *pid;

    MLDSNP_POM_SendMsg(MLDSNP_OM_IPCCMD_GETNEXT_MLD_PRIFILE_ID,
                       msg_p,
                       MLDSNP_OM_GET_MSGBUFSIZE(MLDSNP_OM_IPCMsg_MldSnoopProfile_T),
                       MLDSNP_OM_GET_MSGBUFSIZE(MLDSNP_OM_IPCMsg_MldSnoopProfile_T),
                       (UI32_T)FALSE);
    *pid = data_p->mldsnp_Profile_id;

    return (BOOL_T)MLDSNP_OM_MSG_RETVAL(msg_p);
}
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_POM_GetMLDProfileAccessMode
 *-------------------------------------------------------------------------
 * PURPOSE : Get profile access mode
 * INPUT   : pid - which profile di to get
 * OUTPUT  : *mode  - profile access mode
 * RETURN  : TRUE  - success
 *           FALSE - fail
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_POM_GetMLDProfileAccessMode(UI32_T pid, UI32_T *mode)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(MLDSNP_OM_GET_MSGBUFSIZE(MLDSNP_OM_IPCMsg_MldSnoopProfileMode_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    MLDSNP_OM_IPCMsg_MldSnoopProfileMode_T *data_p;

    data_p = MLDSNP_OM_MSG_DATA(msg_p);

    data_p->mldsnp_Profile_id = pid;

    MLDSNP_POM_SendMsg(MLDSNP_OM_IPCCMD_GET_MLD_PRIFILE_MODE,
                       msg_p,
                       MLDSNP_OM_GET_MSGBUFSIZE(MLDSNP_OM_IPCMsg_MldSnoopProfileMode_T),
                       MLDSNP_OM_GET_MSGBUFSIZE(MLDSNP_OM_IPCMsg_MldSnoopProfileMode_T),
                       (UI32_T)FALSE);
    *mode = data_p->profile_mode;

    return (BOOL_T)MLDSNP_OM_MSG_RETVAL(msg_p);
}
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_POM_GetNextMLDProfileGroupbyPid
 *-------------------------------------------------------------------------
 * PURPOSE : Get profile next group
 * INPUT   : pid  - profile id
 * OUTPUT  : *start_ip  - group start ip
 *           *end_ip    - group end ip
 * RETURN  : TRUE  - success
 *           FALSE - fail
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_POM_GetNextMLDProfileGroupbyPid(UI32_T pid, UI8_T *start_ip, UI8_T *end_ip)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(MLDSNP_OM_GET_MSGBUFSIZE(MLDSNP_OM_IPCMsg_MldSnoopProfileRange_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    MLDSNP_OM_IPCMsg_MldSnoopProfileRange_T *data_p;

    data_p = MLDSNP_OM_MSG_DATA(msg_p);
    data_p->mldsnp_Profile_id = pid;
    memcpy(data_p->ip_begin, start_ip, SYS_ADPT_IPV6_ADDR_LEN);
    memcpy(data_p->ip_end, end_ip, SYS_ADPT_IPV6_ADDR_LEN);

    MLDSNP_POM_SendMsg(MLDSNP_OM_IPCCMD_GETNEXT_MLD_PRIFILE_GROUP,
                       msg_p,
                       MLDSNP_OM_GET_MSGBUFSIZE(MLDSNP_OM_IPCMsg_MldSnoopProfileRange_T),
                       MLDSNP_OM_GET_MSGBUFSIZE(MLDSNP_OM_IPCMsg_MldSnoopProfileRange_T),
                       (UI32_T)FALSE);

    memcpy(start_ip, data_p->ip_begin, SYS_ADPT_IPV6_ADDR_LEN);
    memcpy(end_ip, data_p->ip_end, SYS_ADPT_IPV6_ADDR_LEN);

    return (BOOL_T)MLDSNP_OM_MSG_RETVAL(msg_p);
}
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_POM_GetPortMLDProfileID
 *-------------------------------------------------------------------------
 * PURPOSE : Get port bind profile id
 * INPUT   : port - which port to get
 * OUTPUT  : *pid - which profile id bind to this port
 * RETURN  : TRUE  - success
 *           FALSE - fail
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_POM_GetPortMLDProfileID(UI32_T port, UI32_T *pid)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(MLDSNP_OM_GET_MSGBUFSIZE(MLDSNP_OM_IPCMsg_MldSnoopProfilePort_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    MLDSNP_OM_IPCMsg_MldSnoopProfilePort_T *data_p;

    data_p = MLDSNP_OM_MSG_DATA(msg_p);

    data_p->port = port;

    MLDSNP_POM_SendMsg(MLDSNP_OM_IPCCMD_GET_MLD_PRIFILE_PORT,
                       msg_p,
                       MLDSNP_OM_GET_MSGBUFSIZE(MLDSNP_OM_IPCMsg_MldSnoopProfilePort_T),
                       MLDSNP_OM_GET_MSGBUFSIZE(MLDSNP_OM_IPCMsg_MldSnoopProfilePort_T),
                       (UI32_T)FALSE);

    *pid = data_p->mldsnp_Profile_id;

    return (BOOL_T)MLDSNP_OM_MSG_RETVAL(msg_p);
}
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_POM_IsMLDProfileExist
 *-------------------------------------------------------------------------
 * PURPOSE : Check this profile id exist
 * INPUT   : profile_id  - profile id to check
 * OUTPUT  : None
 * RETURN  : TRUE - exist
 *           FALSE- not exist
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_POM_IsMLDProfileExist(UI32_T profile_id)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(MLDSNP_OM_GET_MSGBUFSIZE(MLDSNP_OM_IPCMsg_MldSnoopProfile_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    MLDSNP_OM_IPCMsg_MldSnoopProfile_T *data_p;

    data_p = MLDSNP_OM_MSG_DATA(msg_p);
    data_p->mldsnp_Profile_id = profile_id;

    MLDSNP_POM_SendMsg(MLDSNP_OM_IPCCMD_EXIST_MLD_CREATE_PRIFILE,
                       msg_p,
                       MLDSNP_OM_GET_MSGBUFSIZE(MLDSNP_OM_IPCMsg_MldSnoopProfile_T),
                       MLDSNP_OM_GET_MSGBUFSIZE(MLDSNP_OM_IPCMsg_MldSnoopProfile_T),
                       (UI32_T)FALSE);

    return (BOOL_T)MLDSNP_OM_MSG_RETVAL(msg_p);
}
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_POM_GetMLDThrottlingInfo
 *-------------------------------------------------------------------------
 * PURPOSE : Get throttle information
 * INPUT   : port - which port to get
 * OUTPUT  : *info- throttle information
 * RETURN  : TRUE  - success
 *           FALSE - fail
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_POM_GetMLDThrottlingInfo(UI32_T port, MLDSNP_OM_Throttle_T *info)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(MLDSNP_OM_GET_MSGBUFSIZE(MLDSNP_OM_IPCMsg_MldSnoopThrottleInfo_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    MLDSNP_OM_IPCMsg_MldSnoopThrottleInfo_T *data_p;

    data_p = MLDSNP_OM_MSG_DATA(msg_p);

    data_p->port = port;

    MLDSNP_POM_SendMsg(MLDSNP_OM_IPCCMD_GET_MLD_THROTTLE_INFO,
                       msg_p,
                       MLDSNP_OM_GET_MSGBUFSIZE(MLDSNP_OM_IPCMsg_MldSnoopThrottleInfo_T),
                       MLDSNP_OM_GET_MSGBUFSIZE(MLDSNP_OM_IPCMsg_MldSnoopThrottleInfo_T),
                       (UI32_T)FALSE);

    memcpy(info, &data_p->throttling_info, sizeof(data_p->throttling_info));

    return (BOOL_T)MLDSNP_OM_MSG_RETVAL(msg_p);
}
#endif

#if (SYS_CPNT_MLDSNP_MLD_REPORT_LIMIT_PER_SECOND_PER_PORT== TRUE || SYS_CPNT_MLDSNP_MLD_REPORT_LIMIT_PER_SECOND_PER_VLAN == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_POM_GetMldReportLimitPerSec
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to set mld report limit value per second
 * INPUT   : ifindex       - which port or vlan to get
 * OUTPUT  : limit_per_sec - per second limit value
 * RETURN  : TRUE  - success
 *           FALSE - fail
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_POM_GetMldReportLimitPerSec(UI32_T ifindex, UI16_T  *limit_per_sec)
{
    UI8_T msgbuf[SYSFUN_SIZE_OF_MSG(MLDSNP_OM_GET_MSGBUFSIZE(MLDSNP_OM_IPCMsg_GS2_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    MLDSNP_OM_IPCMsg_GS2_T *data_p;

    data_p = MLDSNP_OM_MSG_DATA(msg_p);
    data_p->value1 = ifindex;

    MLDSNP_POM_SendMsg(MLDSNP_OM_IPCCMD_GET_MLD_REPORT_LIMIT_PER_SECOND,
                       msg_p,
                       MLDSNP_OM_GET_MSGBUFSIZE(MLDSNP_OM_IPCMsg_GS2_T),
                       MLDSNP_OM_GET_MSGBUFSIZE(MLDSNP_OM_IPCMsg_GS2_T),
                       (UI32_T)FALSE);

    *limit_per_sec = data_p->value2;

    return (BOOL_T)MLDSNP_OM_MSG_RETVAL(msg_p);
}/*End of MLDSNP_POM_GetMldReportLimitPerSec*/


/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_POM_GetRunnningMldReportLimitPerSec
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to get mld report limit value per second configuration
 * INPUT   : ifindex       - which port or vlan to get
 * OUTPUT  : limit_per_sec - per second limit value
 * RETURN  : TRUE  - success
 *           FALSE - fail
 * NOTE    :
 *-------------------------------------------------------------------------
 */
UI32_T MLDSNP_POM_GetRunnningMldReportLimitPerSec(UI32_T ifindex, UI16_T *limit_per_sec)
{
    if (FALSE == MLDSNP_POM_GetMldReportLimitPerSec(ifindex, limit_per_sec))
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

#if (SYS_CPNT_MLDSNP_MLD_REPORT_LIMIT_PER_SECOND_PER_VLAN == TRUE)
    if (ifindex >= SYS_ADPT_VLAN_1_IF_INDEX_NUMBER)
    {
        if (*limit_per_sec != SYS_DFLT_MLDSNP_MLD_REPORT_LIMIT_PER_SECOND_PER_VLAN)
            return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }
#endif
#if (SYS_CPNT_MLDSNP_MLD_REPORT_LIMIT_PER_SECOND_PER_PORT == TRUE)
    if (ifindex < SYS_ADPT_VLAN_1_IF_INDEX_NUMBER)
    {
        if (*limit_per_sec != SYS_DFLT_MLDSNP_MLD_REPORT_LIMIT_PER_SECOND_PER_PORT)
        {
            return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
        }
    }
#endif
    return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
}/*End of MLDSNP_POM_GetRunnningMldReportLimitPerSec*/
#endif /*#if (SYS_CPNT_MLDSNP_MLD_REPORT_LIMIT_PER_SECOND_PER_PORT== TRUE || SYS_CPNT_MLDSNP_MLD_REPORT_LIMIT_PER_SECOND_PER_VLAN == TRUE)*/

#if (SYS_CPNT_MLDSNP_QUERY_DROP == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_GetQueryDropStatus
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to get port query guard status
 * INPUT   : lport  - which port to get information
 *           status - the enable or diable status
 * OUTPUT  : status  - the enabled or disable status
 * RETURN  : TRUE  - success
 *           FALSE - failure
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_POM_GetQueryDropStatus(UI32_T lport, UI32_T  *status)
{
    UI8_T msgbuf[SYSFUN_SIZE_OF_MSG(MLDSNP_OM_GET_MSGBUFSIZE(MLDSNP_OM_IPCMsg_GS2_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    MLDSNP_OM_IPCMsg_GS2_T *data_p;

    data_p = MLDSNP_OM_MSG_DATA(msg_p);

    data_p->value1 = lport;

    MLDSNP_POM_SendMsg(MLDSNP_OM_IPCCMD_GET_QUERY_GUARD_STATUS,
                       msg_p,
                       MLDSNP_OM_GET_MSGBUFSIZE(MLDSNP_OM_IPCMsg_GS2_T),
                       MLDSNP_OM_GET_MSGBUFSIZE(MLDSNP_OM_IPCMsg_GS2_T),
                       (UI32_T)FALSE);

    *status = data_p->value2;
    return (BOOL_T)MLDSNP_OM_MSG_RETVAL(msg_p);
}/*End of MLDSNP_POM_GetQueryDropStatus*/


/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_POM_GetRunningQuaryGuardStatus
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to get query guard status
 * INPUT   : lport  - which port to get information
 *           status - the enable or diable status
 * OUTPUT  : status  - the enabled or disable status
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE  - no change
 *           SYS_TYPE_GET_RUNNING_CFG_SUCCESS - not default value
 * NOTE    :
 *-------------------------------------------------------------------------
 */
UI32_T MLDSNP_POM_GetRunningQuaryGuardStatus(UI32_T lport, UI32_T *status)
{
    UI8_T msgbuf[SYSFUN_SIZE_OF_MSG(MLDSNP_OM_GET_MSGBUFSIZE(MLDSNP_OM_IPCMsg_GS2_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    MLDSNP_OM_IPCMsg_GS2_T *data_p;

    data_p = MLDSNP_OM_MSG_DATA(msg_p);

    data_p->value1 = lport;

    MLDSNP_POM_SendMsg(MLDSNP_OM_IPCCMD_GET_RUNNING_QUERY_GUARD_STATUS,
                       msg_p,
                       MLDSNP_OM_GET_MSGBUFSIZE(MLDSNP_OM_IPCMsg_GS2_T),
                       MLDSNP_OM_GET_MSGBUFSIZE(MLDSNP_OM_IPCMsg_GS2_T),
                       (UI32_T)FALSE);

    *status = data_p->value2;

    return MLDSNP_OM_MSG_RETVAL(msg_p);
}/*End of MLDSNP_POM_GetRunningQuaryGuardStatus*/
#endif /*#if (SYS_CPNT_MLDSNP_QUERY_DROP == TRUE)*/

#if (SYS_CPNT_IPV6_MULTICAST_DATA_DROP== TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_POM_GetMulticastDataDropStatus
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to get multicast data guard
 * INPUT   : lport  - which port to get information
 *           status - the enable or diable status
 * OUTPUT  : status  - the enabled or disable status
 * RETURN  : TRUE  - success
 *           FALSE - failure
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_POM_GetMulticastDataDropStatus(UI32_T lport, UI32_T  *status)
{
    UI8_T msgbuf[SYSFUN_SIZE_OF_MSG(MLDSNP_OM_GET_MSGBUFSIZE(MLDSNP_OM_IPCMsg_GS2_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    MLDSNP_OM_IPCMsg_GS2_T *data_p;

    data_p = MLDSNP_OM_MSG_DATA(msg_p);

    data_p->value1 = lport;

    MLDSNP_POM_SendMsg(MLDSNP_OM_IPCCMD_GET_MULTICAST_DATA_DROP_STATUS,
                       msg_p,
                       MLDSNP_OM_GET_MSGBUFSIZE(MLDSNP_OM_IPCMsg_GS2_T),
                       MLDSNP_OM_GET_MSGBUFSIZE(MLDSNP_OM_IPCMsg_GS2_T),
                       (UI32_T)FALSE);

    *status = data_p->value2;
    return (BOOL_T)MLDSNP_OM_MSG_RETVAL(msg_p);
}/*End of MLDSNP_POM_GetMulticastDataDropStatus*/


/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_POM_GetRunningMulticastDataDropStatus
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to get multicast data guard status
 * INPUT   : lport  - which port to get information
 *           status - the enable or diable status
 * OUTPUT  : status  - the enabled or disable status
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE  - no change
 *           SYS_TYPE_GET_RUNNING_CFG_SUCCESS - not default value
 * NOTE    :
 *-------------------------------------------------------------------------
 */
UI32_T MLDSNP_POM_GetRunningMulticastDataDropStatus(UI32_T lport, UI32_T *status)
{
    UI8_T msgbuf[SYSFUN_SIZE_OF_MSG(MLDSNP_OM_GET_MSGBUFSIZE(MLDSNP_OM_IPCMsg_GS2_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    MLDSNP_OM_IPCMsg_GS2_T *data_p;

    data_p = MLDSNP_OM_MSG_DATA(msg_p);

    data_p->value1 = lport;

    MLDSNP_POM_SendMsg(MLDSNP_OM_IPCCMD_GET_RUNNING_MULTICAST_DATA_DROP_STATUS,
                       msg_p,
                       MLDSNP_OM_GET_MSGBUFSIZE(MLDSNP_OM_IPCMsg_GS2_T),
                       MLDSNP_OM_GET_MSGBUFSIZE(MLDSNP_OM_IPCMsg_GS2_T),
                       (UI32_T)FALSE);

    *status = data_p->value2;

    return MLDSNP_OM_MSG_RETVAL(msg_p);
}/*End of MLDSNP_POM_GetRunningMulticastDataDropStatus*/
#endif

/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_POM_GetTotalEntry
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to get total created entries in om
 * INPUT   : None
 * OUTPUT  : total_p  - total entries
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE  - no change
 *           SYS_TYPE_GET_RUNNING_CFG_SUCCESS - not default value
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_POM_GetTotalEntry(UI32_T *total_p)
{
    UI8_T msgbuf[SYSFUN_SIZE_OF_MSG(MLDSNP_OM_GET_MSGBUFSIZE(MLDSNP_OM_IPCMsg_GS1_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    MLDSNP_OM_IPCMsg_GS1_T *data_p;

    data_p = MLDSNP_OM_MSG_DATA(msg_p);

    MLDSNP_POM_SendMsg(MLDSNP_OM_IPCCMD_GET_TOTAL_ENTRY,
                       msg_p,
                       MLDSNP_OM_GET_MSGBUFSIZE(MLDSNP_OM_IPCMsg_GS1_T),
                       MLDSNP_OM_GET_MSGBUFSIZE(MLDSNP_OM_IPCMsg_GS1_T),
                       (UI32_T)FALSE);

    *total_p = data_p->value1;
    return (BOOL_T)MLDSNP_OM_MSG_RETVAL(msg_p);
}
#endif /*#ifndef UNIT_TEST*/

