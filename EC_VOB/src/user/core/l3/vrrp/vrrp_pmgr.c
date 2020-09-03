/* MODULE NAME:  vrrp_pmgr.c
 * PURPOSE:
 *     VRRP PMGR APIs.
 *
 *
 * NOTES:
 *
 * HISTORY
 *    3/28/2009 - Donny.Li     , Created
 *
 * Copyright(C)      Accton Corporation, 2009
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include <string.h>
#include "sys_module.h"
#include "sysfun.h"
#include "sys_bld.h"
#include "sys_dflt.h"
#include "sys_type.h"
#include "vrrp_mgr.h"
#include "vrrp_pmgr.h"

static SYSFUN_MsgQ_T ipcmsgq_handle = 0;


BOOL_T VRRP_PMGR_InitiateProcessResource(void)
{
    if (SYSFUN_OK != SYSFUN_GetMsgQ(SYS_BLD_VRRP_GROUP_IPCMSGQ_KEY, SYSFUN_MSGQ_BIDIRECTIONAL,
                                    &ipcmsgq_handle))
    {
        printf("%s(): SYSFUN_GetMsgQ fail.\n", __FUNCTION__);
        return FALSE;
    }
    return TRUE;
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_SetIpAddress
 *--------------------------------------------------------------------------
 * PURPOSE  : Set the ip address for the specific vrrp on the interface
 * INPUT    : if_index:   ifindex
 *            vrid:       id of vrrp
 *            ip_addr:    ip address to be set or delete
 *            row_status: VAL_vrrpAssoIpAddrRowStatus_notInService or
 *                        VAL_vrrpAssoIpAddrRowStatus_destroy to delete this ip address
 *                        VAL_vrrpAssoIpAddrRowStatus_createAndGo or
 *                        VAL_vrrpAssoIpAddrRowStatus_active to add this ip address
 * OUTPUT   : none
 * RETURN   : VRRP_TYPE_PARAMETER_ERROR/VRRP_TYPE_OPER_ENTRY_NOT_EXIST/
 *            VRRP_TYPE_EXCESS_VRRP_NUMBER_ON_CHIP/TRUE/FALSE/
 * NOTES    :
 *-------------------------------------------------------------------------- */

UI32_T VRRP_PMGR_SetIpAddress(UI32_T if_index, UI8_T vrid, UI8_T *ip_addr, UI32_T row_status)
{
    VRRP_MGR_IPCMsg_T *msg_p = NULL;
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp1))];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    
    msgbuf_p->cmd = SYS_MODULE_VRRP;
    msgbuf_p->msg_size = VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp1);
    msg_p = (VRRP_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VRRP_MGR_IPC_IPADDRESSSET;
    
    memset(&(msg_p->data.arg_grp1), 0, sizeof(msg_p->data.arg_grp1));
    msg_p->data.arg_grp1.arg1 = if_index;
    msg_p->data.arg_grp1.arg2 = vrid;
    memcpy(msg_p->data.arg_grp1.arg3, ip_addr, INET_ADDRESS_IPV4_SIZE);
    msg_p->data.arg_grp1.arg4 = row_status;
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp1), msgbuf_p) != SYSFUN_OK)
    {
        return VRRP_TYPE_RESULT_FAIL;
    }
    return msg_p->type.result_ui32;
    
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_SetPrimaryIp
 *--------------------------------------------------------------------------
 * PURPOSE  : Set the primary ip address for the spicific vrrp on the interface
 *            for MIB
 * INPUT    : if_index:   ifindex
 *            vrid:       id of vrrp
 *            ip_addr:    ip address to be set or delete
 * OUTPUT   : none
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_INTERNAL_ERROR
 * NOTES    : none
 *-------------------------------------------------------------------------- */
UI32_T VRRP_PMGR_SetPrimaryIp(UI32_T if_index, UI8_T vrid, UI8_T *ip_addr)
{
    VRRP_MGR_IPCMsg_T *msg_p = NULL;
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp2))];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    
    msgbuf_p->cmd = SYS_MODULE_VRRP;
    msgbuf_p->msg_size = VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp2);
    msg_p = (VRRP_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VRRP_MGR_IPC_PRIMARYIPSET;
    
    memset(&(msg_p->data.arg_grp2), 0, sizeof(msg_p->data.arg_grp2));
    msg_p->data.arg_grp2.arg1 = if_index;
    msg_p->data.arg_grp2.arg2 = vrid;
    memcpy(msg_p->data.arg_grp2.arg3, ip_addr, INET_ADDRESS_IPV4_SIZE);
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp2), msgbuf_p) != SYSFUN_OK)
    {
        return VRRP_TYPE_RESULT_FAIL;
    }
    return msg_p->type.result_ui32;
    
}


/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_GetNextIpAddress
 *--------------------------------------------------------------------------
 * PURPOSE  : Get the next ip address for vrrp
 * INPUT    : if_index: the specific interface
 *            vrid:     the specific vrrp group id
 *            ip_addr:  the buffer to get the ip address
 * OUTPUT   : next associated ip address of the specific ifindex and vrid
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR
 * NOTES    : none
 *-------------------------------------------------------------------------- */
UI32_T VRRP_PMGR_GetNextIpAddress(UI32_T if_index, UI8_T vrid, UI8_T *ip_addr)
{
    VRRP_MGR_IPCMsg_T *msg_p = NULL;
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp2))];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    
    msgbuf_p->cmd = SYS_MODULE_VRRP;
    msgbuf_p->msg_size = VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp2);
    msg_p = (VRRP_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VRRP_MGR_IPC_IPADDRESSGETNEXT;
    
    memset(&(msg_p->data.arg_grp2), 0, sizeof(msg_p->data.arg_grp2));
    msg_p->data.arg_grp2.arg1 = if_index;
    msg_p->data.arg_grp2.arg2 = vrid;
    memcpy(msg_p->data.arg_grp2.arg3, ip_addr, INET_ADDRESS_IPV4_SIZE);
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp2), msgbuf_p) != SYSFUN_OK)
    {
        return VRRP_TYPE_RESULT_FAIL;
    }
    memcpy(ip_addr, msg_p->data.arg_grp2.arg3, INET_ADDRESS_IPV4_SIZE);
    
    return msg_p->type.result_ui32;
    
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_SetOperAdminStatus
 *------------------------------------------------------------------------------
 * PURPOSE  : This funciton returns true if the VRRP status is successfully Set.
 *            Otherwise, return false.
 * INPUT    : vrrp_admin_status -
              VAL_vrrpOperAdminState_up 1L \
 *            VAL_vrrpOperAdminState_down 2L
              ifIndex  ,  vrid
 * OUTPUT   : none
 * RETURN   : TRUE/FALSE/VRRP_TYPE_EXCESS_VRRP_NUMBER_ON_CHIP
 * NOTES    : The administrative status requested by management for VRRP.  The
 *            value enabled(1) indicates that specify VRRP virtual router should
 *            be enabled on this interface.  When disabled(2), VRRP virtual Router
 *            is not operated. Row status must be up before set enabled(1).
 *------------------------------------------------------------------------------*/
UI32_T VRRP_PMGR_SetOperAdminStatus(UI32_T if_index, UI8_T vrid, UI32_T vrrp_admin_status)
{
    VRRP_MGR_IPCMsg_T *msg_p = NULL;
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp3))];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    
    msgbuf_p->cmd = SYS_MODULE_VRRP;
    msgbuf_p->msg_size = VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp3);
    msg_p = (VRRP_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VRRP_MGR_IPC_OPERADMINSTATUSSET;
    
    memset(&(msg_p->data.arg_grp3), 0, sizeof(msg_p->data.arg_grp3));
    msg_p->data.arg_grp3.arg1 = if_index;
    msg_p->data.arg_grp3.arg2 = vrid;
    msg_p->data.arg_grp3.arg3 = vrrp_admin_status;
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp3), msgbuf_p) != SYSFUN_OK)
    {
        return VRRP_TYPE_RESULT_FAIL;
    }
    return msg_p->type.result_ui32;
    
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_GetOperAdminStatus
 *------------------------------------------------------------------------------
 * PURPOSE  : This funciton returns true if the VRRP status is successfully Get.
 *            Otherwise, return false.
 * INPUT    : vrrp_admin_status -
              VAL_vrrpOperAdminState_up 1L \
 *            VAL_vrrpOperAdminState_down 2L
              ifIndex  ,  vrid
 * OUTPUT   : vrrp admin status for the specific ifindex and vrid
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T VRRP_PMGR_GetOperAdminStatus(UI32_T if_index, UI8_T vrid, UI32_T *vrrp_admin_status)
{
    VRRP_MGR_IPCMsg_T *msg_p = NULL;
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp3))];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    
    msgbuf_p->cmd = SYS_MODULE_VRRP;
    msgbuf_p->msg_size = VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp3);
    msg_p = (VRRP_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VRRP_MGR_IPC_OPERADMINSTATUSGET;
    
    memset(&(msg_p->data.arg_grp3), 0, sizeof(msg_p->data.arg_grp3));
    msg_p->data.arg_grp3.arg1 = if_index;
    msg_p->data.arg_grp3.arg2 = vrid;
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp3), msgbuf_p) != SYSFUN_OK)
    {
        return VRRP_TYPE_RESULT_FAIL;
    }
    *vrrp_admin_status = msg_p->data.arg_grp3.arg3;
    
    return msg_p->type.result_ui32;
    
}
/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_SetAuthenticationType
 *--------------------------------------------------------------------------
 * PURPOSE  : Set the authentication type for each interface
 * INPUT    : if_Index, vrid,
 *            auth_type -
 *               VAL_vrrpOperAuthType_noAuthentication	     1L
 *               VAL_vrrpOperAuthType_simpleTextPassword	 2L
 *               VAL_vrrpOperAuthType_ipAuthenticationHeader 3L
 * OUTPUT   : none
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_INTERNAL_ERROR
 * NOTES    : none
 *-------------------------------------------------------------------------- */
UI32_T VRRP_PMGR_SetAuthenticationType(UI32_T if_index, UI8_T vrid, UI32_T vrrp_oper_auth_type)
{
    VRRP_MGR_IPCMsg_T *msg_p = NULL;
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp3))];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    
    msgbuf_p->cmd = SYS_MODULE_VRRP;
    msgbuf_p->msg_size = VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp3);
    msg_p = (VRRP_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VRRP_MGR_IPC_OPERAUTHTYPESET;
    
    memset(&(msg_p->data.arg_grp3), 0, sizeof(msg_p->data.arg_grp3));
    msg_p->data.arg_grp3.arg1 = if_index;
    msg_p->data.arg_grp3.arg2 = vrid;
    msg_p->data.arg_grp3.arg3 = vrrp_oper_auth_type;
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp3), msgbuf_p) != SYSFUN_OK)
    {
        return VRRP_TYPE_RESULT_FAIL;
    }
    return msg_p->type.result_ui32;
    
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_GetAuthenticationType
 *--------------------------------------------------------------------------
 * PURPOSE  : Get the authentication type for each interface
 * INPUT    : if_Index, vrid
 * OUTPUT   : authentication type of the specific ifindex and vrid
 *               VAL_vrrpOperAuthType_noAuthentication	     1L
 *               VAL_vrrpOperAuthType_simpleTextPassword	 2L
 *               VAL_vrrpOperAuthType_ipAuthenticationHeader 3L
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR
 * NOTES    : none
 *-------------------------------------------------------------------------- */
UI32_T VRRP_PMGR_GetAuthenticationType(UI32_T if_index, UI8_T vrid, UI32_T *vrrp_oper_auth_type)
{
    VRRP_MGR_IPCMsg_T *msg_p = NULL;
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp3))];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    
    msgbuf_p->cmd = SYS_MODULE_VRRP;
    msgbuf_p->msg_size = VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp3);
    msg_p = (VRRP_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VRRP_MGR_IPC_OPERAUTHTYPEGET;
    
    memset(&(msg_p->data.arg_grp3), 0, sizeof(msg_p->data.arg_grp3));
    msg_p->data.arg_grp3.arg1 = if_index;
    msg_p->data.arg_grp3.arg2 = vrid;
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp3), msgbuf_p) != SYSFUN_OK)
    {
        return VRRP_TYPE_RESULT_FAIL;
    }
    *vrrp_oper_auth_type = msg_p->data.arg_grp3.arg3;
    
    return msg_p->type.result_ui32;
    
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_SetAuthenticationKey
 *--------------------------------------------------------------------------
 * PURPOSE  : Set the authentication key for each interface
 * INPUT    : ifindex, vrid
 * OUTPUT   : none
 * RETURN   : VRRP_TYPE_OK/
 *		      VRRP_TYPE_PARAMETER_ERROR/
 *			  VRRP_TYPE_INTERNAL_ERROR
 * NOTES    : none
 *-------------------------------------------------------------------------- */
UI32_T VRRP_PMGR_SetAuthenticationKey(UI32_T if_index, UI8_T vrid, UI8_T *vrrp_oper_auth_key)
{
    VRRP_MGR_IPCMsg_T *msg_p = NULL;
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp4))];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    
    msgbuf_p->cmd = SYS_MODULE_VRRP;
    msgbuf_p->msg_size = VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp4);
    msg_p = (VRRP_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VRRP_MGR_IPC_OPERAUTHKEYSET;
    
    memset(&(msg_p->data.arg_grp4), 0, sizeof(msg_p->data.arg_grp4));
    msg_p->data.arg_grp4.arg1 = if_index;
    msg_p->data.arg_grp4.arg2 = vrid;
    strncpy((char *)msg_p->data.arg_grp4.arg3, (char *)vrrp_oper_auth_key, strlen((char *)vrrp_oper_auth_key));
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp4), msgbuf_p) != SYSFUN_OK)
    {
        return VRRP_TYPE_RESULT_FAIL;
    }
    return msg_p->type.result_ui32;
    
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_GetAuthenticationKey
 *--------------------------------------------------------------------------
 * PURPOSE  : Get the authentication key for each interface
 * INPUT    : ifindex, vrid
 * OUTPUT   : authentication key of the specific ifindex and vrid
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR
 * NOTES    : none
 *-------------------------------------------------------------------------- */
UI32_T VRRP_PMGR_GetAuthenticationKey(UI32_T if_index, UI8_T vrid, UI8_T *vrrp_oper_auth_key)
{
    VRRP_MGR_IPCMsg_T *msg_p = NULL;
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp4))];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    
    msgbuf_p->cmd = SYS_MODULE_VRRP;
    msgbuf_p->msg_size = VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp4);
    msg_p = (VRRP_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VRRP_MGR_IPC_OPERAUTHKEYGET;
    
    memset(&(msg_p->data.arg_grp4), 0, sizeof(msg_p->data.arg_grp4));
    msg_p->data.arg_grp4.arg1 = if_index;
    msg_p->data.arg_grp4.arg2 = vrid;
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp4), msgbuf_p) != SYSFUN_OK)
    {
        return VRRP_TYPE_RESULT_FAIL;
    }
    
    strncpy((char *)vrrp_oper_auth_key, (char *)msg_p->data.arg_grp4.arg3, strlen((char *)msg_p->data.arg_grp4.arg3));
    return msg_p->type.result_ui32;
    
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_SetAuthentication
 *------------------------------------------------------------------------------
 * PURPOSE  : This funciton returns true if the authentication data is set successfully.
              Otherwise, return false.
 * INPUT    : ifIndex, vrid,
 *            auth_type -
              	VAL_vrrpOperAuthType_noAuthentication	1L \
              	VAL_vrrpOperAuthType_simpleTextPassword	2L \
              	VAL_vrrpOperAuthType_ipAuthenticationHeader 3L
              auth_key
 * OUTPUT   : none
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR/
 *			  VRRP_TYPE_INTERNAL_ERROR
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T VRRP_PMGR_SetAuthentication(UI32_T if_index, UI8_T vrid, UI32_T auth_type, UI8_T *auth_key)
{
    VRRP_MGR_IPCMsg_T *msg_p = NULL;
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp5))];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    
    msgbuf_p->cmd = SYS_MODULE_VRRP;
    msgbuf_p->msg_size = VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp5);
    msg_p = (VRRP_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VRRP_MGR_IPC_OPERAUTHSET;
    
    memset(&(msg_p->data.arg_grp5), 0, sizeof(msg_p->data.arg_grp5));
    msg_p->data.arg_grp5.arg1 = if_index;
    msg_p->data.arg_grp5.arg2 = vrid;
    msg_p->data.arg_grp5.arg3 = auth_type;
    strncpy((char *)msg_p->data.arg_grp5.arg4, (char *)auth_key, strlen((char *)auth_key));
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp5), msgbuf_p) != SYSFUN_OK)
    {
        return VRRP_TYPE_RESULT_FAIL;
    }
    return msg_p->type.result_ui32;
    
}
/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_SetPriority
 *--------------------------------------------------------------------------
 * PURPOSE  : Set the priority for each vrrp
 * INPUT    : ifindex, vrid
 * OUTPUT   : none
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_INTERNAL_ERROR
 * NOTES    : MIN_vrrpOperPriority	0L
 *            MAX_vrrpOperPriority	255L
 *-------------------------------------------------------------------------- */
UI32_T VRRP_PMGR_SetPriority(UI32_T if_index, UI8_T vrid, UI32_T vrrp_oper_priority)
{
    VRRP_MGR_IPCMsg_T *msg_p = NULL;
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp3))];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    
    msgbuf_p->cmd = SYS_MODULE_VRRP;
    msgbuf_p->msg_size = VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp3);
    msg_p = (VRRP_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VRRP_MGR_IPC_PRIORITYSET;
    
    memset(&(msg_p->data.arg_grp3), 0, sizeof(msg_p->data.arg_grp3));
    msg_p->data.arg_grp3.arg1 = if_index;
    msg_p->data.arg_grp3.arg2 = vrid;
    msg_p->data.arg_grp3.arg3 = vrrp_oper_priority;
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp3), msgbuf_p) != SYSFUN_OK)
    {
        return VRRP_TYPE_RESULT_FAIL;
    }
    return msg_p->type.result_ui32;
    
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_GetPriority
 *--------------------------------------------------------------------------
 * PURPOSE  : Get the priority for each vrrp (RFC2338, p.14)
 * INPUT    : ifindex, vrid
 * OUTPUT   : vrrp priority of the specific ifindex oand vrid
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR
 * NOTES    : none
 *-------------------------------------------------------------------------- */
UI32_T VRRP_PMGR_GetPriority(UI32_T if_index, UI8_T vrid, UI32_T *vrrp_oper_priority)
{
    VRRP_MGR_IPCMsg_T *msg_p = NULL;
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp3))];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    
    msgbuf_p->cmd = SYS_MODULE_VRRP;
    msgbuf_p->msg_size = VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp3);
    msg_p = (VRRP_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VRRP_MGR_IPC_PRIORITYGET;
    
    memset(&(msg_p->data.arg_grp3), 0, sizeof(msg_p->data.arg_grp3));
    msg_p->data.arg_grp3.arg1 = if_index;
    msg_p->data.arg_grp3.arg2 = vrid;
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp3), msgbuf_p) != SYSFUN_OK)
    {
        return VRRP_TYPE_RESULT_FAIL;
    }
    *vrrp_oper_priority = msg_p->data.arg_grp3.arg3;
    
    return msg_p->type.result_ui32;
    
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_SetPreemptionMode
 *--------------------------------------------------------------------------
 * PURPOSE  : Set the preemption mode for each vrrp
 * INPUT    : ifindex, vrid, preempt mode and delay time for this vrrp group
 * OUTPUT   : none
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR
 * NOTES    : the preempt_delay_time is private for our implementation.
 *            SYS_ADPT_MIN_VIRTUAL_ROUTER_PRE_DELAY   0
 *            SYS_ADPT_MAX_VIRTUAL_ROUTER_PRE_DELAY   120
 *-------------------------------------------------------------------------- */
UI32_T VRRP_PMGR_SetPreemptionMode(UI32_T if_index, UI8_T vrid, UI32_T vrrp_oper_preempt_mode, UI32_T delay)
{
    VRRP_MGR_IPCMsg_T *msg_p = NULL;
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp6))];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    
    msgbuf_p->cmd = SYS_MODULE_VRRP;
    msgbuf_p->msg_size = VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp6);
    msg_p = (VRRP_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VRRP_MGR_IPC_PREEMPTIONMODESET;
    
    memset(&(msg_p->data.arg_grp6), 0, sizeof(msg_p->data.arg_grp6));
    msg_p->data.arg_grp6.arg1 = if_index;
    msg_p->data.arg_grp6.arg2 = vrid;
    msg_p->data.arg_grp6.arg3 = vrrp_oper_preempt_mode;
    msg_p->data.arg_grp6.arg4 = delay;
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp6), msgbuf_p) != SYSFUN_OK)
    {
        return VRRP_TYPE_RESULT_FAIL;
    }
    return msg_p->type.result_ui32;
    
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_SetPreemptMode
 *--------------------------------------------------------------------------
 * PURPOSE  : Set the preemption mode for each vrrp  for MIB
 * INPUT    : ifindex, vrid, preempt mode for this vrrp group
 * OUTPUT   : none
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_INTERNAL_ERROR
 * NOTES    : the preempt_delay_time is private for our implementation.
 *            SYS_ADPT_MIN_VIRTUAL_ROUTER_PRE_DELAY   0
 *            SYS_ADPT_MAX_VIRTUAL_ROUTER_PRE_DELAY   120
 *-------------------------------------------------------------------------- */
UI32_T VRRP_PMGR_SetPreemptMode(UI32_T if_index, UI8_T vrid, UI32_T vrrp_oper_preempt_mode)
{
    VRRP_MGR_IPCMsg_T *msg_p = NULL;
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp3))];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    
    msgbuf_p->cmd = SYS_MODULE_VRRP;
    msgbuf_p->msg_size = VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp3);
    msg_p = (VRRP_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VRRP_MGR_IPC_PREEMPTMODESET;
    
    memset(&(msg_p->data.arg_grp3), 0, sizeof(msg_p->data.arg_grp3));
    msg_p->data.arg_grp3.arg1 = if_index;
    msg_p->data.arg_grp3.arg2 = vrid;
    msg_p->data.arg_grp3.arg3 = vrrp_oper_preempt_mode;
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp3), msgbuf_p) != SYSFUN_OK)
    {
        return VRRP_TYPE_RESULT_FAIL;
    }
    return msg_p->type.result_ui32;
    
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_GetPreemptionMode
 *--------------------------------------------------------------------------
 * PURPOSE  : Get the preemption mode for each vrrp
 * INPUT    : ifindex, vrid
 * OUTPUT   : preempt mode and preempt delay time for this group
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR
 * NOTES    : none
 *-------------------------------------------------------------------------- */
UI32_T VRRP_PMGR_GetPreemptionMode(UI32_T if_index, UI8_T vrid, UI32_T *vrrp_oper_preempt_mode, UI32_T *delay)
{
    VRRP_MGR_IPCMsg_T *msg_p = NULL;
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp6))];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    
    msgbuf_p->cmd = SYS_MODULE_VRRP;
    msgbuf_p->msg_size = VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp6);
    msg_p = (VRRP_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VRRP_MGR_IPC_PREEMPTMODEGET;
    
    memset(&(msg_p->data.arg_grp6), 0, sizeof(msg_p->data.arg_grp6));
    msg_p->data.arg_grp6.arg1 = if_index;
    msg_p->data.arg_grp6.arg2 = vrid;
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp6), msgbuf_p) != SYSFUN_OK)
    {
        return VRRP_TYPE_RESULT_FAIL;
    }
    *vrrp_oper_preempt_mode = msg_p->data.arg_grp6.arg3;
    *delay = msg_p->data.arg_grp6.arg4;
    return msg_p->type.result_ui32;
    
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_SetAdvertisementInterval
 *--------------------------------------------------------------------------
 * PURPOSE  : Set the advertisement interval for each vrrp
 * INPUT    : ifindex, vrid, advertisement interval for the group
 * OUTPUT   : none
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_INTERNAL_ERROR
 * NOTES    : SYS_ADPT_MIN_VIRTUAL_ROUTER_ADVER_INT   1
 *            SYS_ADPT_MAX_VIRTUAL_ROUTER_ADVER_INT 255
 *-------------------------------------------------------------------------- */
UI32_T VRRP_PMGR_SetAdvertisementInterval(UI32_T if_index, UI8_T vrid, UI32_T vrrp_oper_advertise_interval)
{
    VRRP_MGR_IPCMsg_T *msg_p = NULL;
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp3))];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    
    msgbuf_p->cmd = SYS_MODULE_VRRP;
    msgbuf_p->msg_size = VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp3);
    msg_p = (VRRP_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VRRP_MGR_IPC_ADVERINTERVALSET;
    
    memset(&(msg_p->data.arg_grp3), 0, sizeof(msg_p->data.arg_grp3));
    msg_p->data.arg_grp3.arg1 = if_index;
    msg_p->data.arg_grp3.arg2 = vrid;
    msg_p->data.arg_grp3.arg3 = vrrp_oper_advertise_interval;
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp3), msgbuf_p) != SYSFUN_OK)
    {
        return VRRP_TYPE_RESULT_FAIL;
    }
    return msg_p->type.result_ui32;
    
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_GetAdvertisementInterval
 *--------------------------------------------------------------------------
 * PURPOSE  : Get the advertisement interval for each vrrp
 * INPUT    : ifindex, vrid
 * OUTPUT   : advertisement interval of the specific vrrp group
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR
 * NOTES    : none
 *-------------------------------------------------------------------------- */
UI32_T VRRP_PMGR_GetAdvertisementInterval(UI32_T if_index, UI8_T vrid, UI32_T *vrrp_oper_advertise_interval)
{
    VRRP_MGR_IPCMsg_T *msg_p = NULL;
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp3))];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    
    msgbuf_p->cmd = SYS_MODULE_VRRP;
    msgbuf_p->msg_size = VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp3);
    msg_p = (VRRP_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VRRP_MGR_IPC_ADVERINTERVALGET;
    
    memset(&(msg_p->data.arg_grp3), 0, sizeof(msg_p->data.arg_grp3));
    msg_p->data.arg_grp3.arg1 = if_index;
    msg_p->data.arg_grp3.arg2 = vrid;
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp3), msgbuf_p) != SYSFUN_OK)
    {
        return VRRP_TYPE_RESULT_FAIL;
    }
    *vrrp_oper_advertise_interval = msg_p->data.arg_grp3.arg3;
    
    return msg_p->type.result_ui32;
    
}

#if (SYS_CPNT_VRRP_PING == TRUE)
/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_SetPingStatus
 *--------------------------------------------------------------------------
 * PURPOSE  : Set the ping enable status of VRRP
 * INPUT    : ping_status	--	ping enable status
 * OUTPUT   : None
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR
 * NOTES    : ping_status:
 * 			  VRRP_TYPE_PING_STATUS_ENABLE/
 *			  VRRP_TYPE_PING_STATUS_DISABLE
 *
 *--------------------------------------------------------------------------
 */
UI32_T VRRP_PMGR_SetPingStatus(UI32_T ping_status)
{
    VRRP_MGR_IPCMsg_T *msg_p = NULL;
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp8))];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    
    msgbuf_p->cmd = SYS_MODULE_VRRP;
    msgbuf_p->msg_size = VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp8);
    msg_p = (VRRP_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VRRP_MGR_IPC_PINGSET;
    
    memset(&(msg_p->data.arg_grp8), 0, sizeof(msg_p->data.arg_grp8));
    msg_p->data.arg_grp8.arg1 = ping_status;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp8), msgbuf_p) != SYSFUN_OK)
    {
        return VRRP_TYPE_RESULT_FAIL;
    }
    return msg_p->type.result_ui32;
}

#endif

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_SetVrrpOperRowStatus
 *--------------------------------------------------------------------------
 * PURPOSE  : Set the VRRP operation entry of the specific vrrp
 * INPUT    : vrrp_oper_entry->if_index, vrrp_oper_entry->vrid,
 *            action    row_status to be set to the specific vrrp_oper_entry
 * OUTPUT   : none
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR/
 *			  VRRP_TYPE_INTERNAL_ERROR
 * NOTES    : action --
 *            VAL_vrrpOperRowStatus_active	      1L
 *            VAL_vrrpOperRowStatus_notInService    2L
 *            VAL_vrrpOperRowStatus_notReady        3L
 *            VAL_vrrpOperRowStatus_createAndGo	  4L
 *            VAL_vrrpOperRowStatus_createAndWait   5L
 *            VAL_vrrpOperRowStatus_destroy	      6L
 *-------------------------------------------------------------------------- */
UI32_T VRRP_PMGR_SetVrrpOperRowStatus(VRRP_OPER_ENTRY_T *vrrp_oper_entry, UI32_T action)
{
    VRRP_MGR_IPCMsg_T *msg_p = NULL;
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp7))];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    
    msgbuf_p->cmd = SYS_MODULE_VRRP;
    msgbuf_p->msg_size = VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp7);
    msg_p = (VRRP_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VRRP_MGR_IPC_OPERROWSTATUSSET;
    
    memset(&(msg_p->data.arg_grp7), 0, sizeof(msg_p->data.arg_grp7));
    memcpy(&(msg_p->data.arg_grp7.arg1), vrrp_oper_entry, sizeof(msg_p->data.arg_grp7.arg1));
    msg_p->data.arg_grp7.arg2 = action;
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp7), msgbuf_p) != SYSFUN_OK)
    {
        return VRRP_TYPE_RESULT_FAIL;
    }
    
    return msg_p->type.result_ui32;
    
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_SetVrrpOperEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : Set the VRRP operation entry of the specific vrrp
 * INPUT    : vrrp_oper_entry
 * OUTPUT   : none
 * RETURN   : VRRP_TYPE_OK/
 *		      VRRP_TYPE_PARAMETER_ERROR/
 *			  VRRP_TYPE_INTERNAL_ERROR
 * NOTES    : use (vrrp_oper_entry->if_index, vrrp_oper_entry->vrid) as index
 *-------------------------------------------------------------------------- */
UI32_T VRRP_PMGR_SetVrrpOperEntry(VRRP_OPER_ENTRY_T *vrrp_oper_entry)
{
    VRRP_MGR_IPCMsg_T *msg_p = NULL;
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.oper_entry))];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    
    
    msgbuf_p->cmd = SYS_MODULE_VRRP;
    msgbuf_p->msg_size = VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.oper_entry);
    msg_p = (VRRP_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VRRP_MGR_IPC_OPERENTRYSET;
    
    memset(&(msg_p->data.oper_entry), 0, sizeof(VRRP_OPER_ENTRY_T));
    memcpy(&(msg_p->data.oper_entry), vrrp_oper_entry, sizeof(VRRP_OPER_ENTRY_T));
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.oper_entry), msgbuf_p) != SYSFUN_OK)
    {
        return VRRP_TYPE_RESULT_FAIL;
    }
    
    return msg_p->type.result_ui32;
    
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_DeleteVrrpOperEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : Delete the VRRP operation entry of the specific vrrp
 * INPUT    : vrrp_oper_entry
 * OUTPUT   : none
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR/
 *			  VRRP_TYPE_INTERNAL_ERROR
 * NOTES    : use (vrrp_oper_entry->ifindex, vrrp_oper_entry->vrid) as index
 *-------------------------------------------------------------------------- */
UI32_T VRRP_PMGR_DeleteVrrpOperEntry(VRRP_OPER_ENTRY_T *vrrp_oper_entry)
{
    VRRP_MGR_IPCMsg_T *msg_p = NULL;
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.oper_entry))];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    
    msgbuf_p->cmd = SYS_MODULE_VRRP;
    msgbuf_p->msg_size = VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.oper_entry);
    msg_p = (VRRP_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VRRP_MGR_IPC_OPERENTRYDEL;
    
    memset(&(msg_p->data.oper_entry), 0, sizeof(VRRP_OPER_ENTRY_T));
    memcpy(&(msg_p->data.oper_entry), vrrp_oper_entry, sizeof(VRRP_OPER_ENTRY_T));
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.oper_entry), msgbuf_p) != SYSFUN_OK)
    {
        return VRRP_TYPE_RESULT_FAIL;
    }
    
    return msg_p->type.result_ui32;
    
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_GetNextVrrpOperEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : Get the next VRRP operation entry
 * INPUT    : buffer which pointers to vrrp_oper_entry
 * OUTPUT   : next ifindex, next vrid, next vrrp oper entry
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR/
 *			  VRRP_TYPE_INTERNAL_ERROR
 * NOTES    : use (vrrp_oper_entry->if_index, vrrp_oper_entry->vrid) as index
 *-------------------------------------------------------------------------- */
UI32_T VRRP_PMGR_GetNextVrrpOperEntry(VRRP_OPER_ENTRY_T *vrrp_oper_entry)
{
    VRRP_MGR_IPCMsg_T *msg_p = NULL;
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.oper_entry))];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    
    msgbuf_p->cmd = SYS_MODULE_VRRP;
    msgbuf_p->msg_size = VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.oper_entry);
    msg_p = (VRRP_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VRRP_MGR_IPC_OPERENTRYGETNEXT;
    
    memset(&(msg_p->data.oper_entry), 0, sizeof(VRRP_OPER_ENTRY_T));
    memcpy(&(msg_p->data.oper_entry), vrrp_oper_entry, sizeof(VRRP_OPER_ENTRY_T));
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.oper_entry), msgbuf_p) != SYSFUN_OK)
    {
        return VRRP_TYPE_RESULT_FAIL;
    }
    memcpy(vrrp_oper_entry, &(msg_p->data.oper_entry), sizeof(VRRP_OPER_ENTRY_T));
    return msg_p->type.result_ui32;
    
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_GetDefaultVrrpOperEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : Get the default VRRP operation entry of the specific vrrp
 * INPUT    : buffer which pointers to vrrp_oper_entry
 * OUTPUT   : vrrp oper entry
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR/
 *		      VRRP_TYPE_INTERNAL_ERROR
 * NOTES    : use (vrrp_oper_entry->if_index, vrrp_oper_entry->vrid) as index
 *-------------------------------------------------------------------------- */
UI32_T VRRP_PMGR_GetDefaultVrrpOperEntry(VRRP_OPER_ENTRY_T *vrrp_oper_entry)
{
    VRRP_MGR_IPCMsg_T *msg_p = NULL;
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.oper_entry))];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    
    msgbuf_p->cmd = SYS_MODULE_VRRP;
    msgbuf_p->msg_size = VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.oper_entry);
    msg_p = (VRRP_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VRRP_MGR_IPC_DEFAULTOPERENTRYGET;
    
    memset(&(msg_p->data.oper_entry), 0, sizeof(VRRP_OPER_ENTRY_T));
    memcpy(&(msg_p->data.oper_entry), vrrp_oper_entry, sizeof(VRRP_OPER_ENTRY_T));
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.oper_entry), msgbuf_p) != SYSFUN_OK)
    {
        return VRRP_TYPE_RESULT_FAIL;
    }
    
    memcpy(vrrp_oper_entry, &(msg_p->data.oper_entry), sizeof(VRRP_OPER_ENTRY_T));
    
    return msg_p->type.result_ui32;
    
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_SetVrrpAssocIpAddrEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : Set the VRRP associated IP addresses entry of the specific vrrp
 * INPUT    : vrrp_assoc_ip_addr
 * OUTPUT   : none
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR/
 *			  VRRP_TYPE_INTERNAL_ERROR
 * NOTES    : use (vrrp_assoc_ip_addr->ifindex, vrrp_assoc_ip_addr->vrid,
 *            vrrp_assoc_ip_addr->assoc_ip_addr) as index
 *-------------------------------------------------------------------------- */
UI32_T VRRP_PMGR_SetVrrpAssocIpAddrEntry(VRRP_ASSOC_IP_ENTRY_T *vrrp_assoc_ip_addr)
{
    VRRP_MGR_IPCMsg_T *msg_p = NULL;
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.assoc_ip_addr))];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    
    msgbuf_p->cmd = SYS_MODULE_VRRP;
    msgbuf_p->msg_size = VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.assoc_ip_addr);
    msg_p = (VRRP_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VRRP_MGR_IPC_ASSOCIPADDRENTRYSET;
    
    memset(&(msg_p->data.assoc_ip_addr), 0, sizeof(VRRP_ASSOC_IP_ENTRY_T));
    memcpy(&(msg_p->data.assoc_ip_addr), vrrp_assoc_ip_addr, sizeof(VRRP_ASSOC_IP_ENTRY_T));
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.assoc_ip_addr), msgbuf_p) != SYSFUN_OK)
    {
        return VRRP_TYPE_RESULT_FAIL;
    }
    
    return msg_p->type.result_ui32;
    
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_DeleteAssoIpEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : Delete the VRRP associated IP addresses entry of the specific vrrp
 * INPUT    : vrrp_assoc_ip_addr
 * OUTPUT   : none
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR/
 *			  VRRP_TYPE_INTERNAL_ERROR
 * NOTES    : use (vrrp_assoc_ip_addr->ifindex, vrrp_assoc_ip_addr->vrid,
 *            vrrp_assoc_ip_addr->assoc_ip_addr) as index
 *-------------------------------------------------------------------------- */
UI32_T VRRP_PMGR_DeleteAssoIpEntry(VRRP_ASSOC_IP_ENTRY_T *vrrp_assoc_ip_addr)
{
    VRRP_MGR_IPCMsg_T *msg_p = NULL;
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.assoc_ip_addr))];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    
    msgbuf_p->cmd = SYS_MODULE_VRRP;
    msgbuf_p->msg_size = VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.assoc_ip_addr);
    msg_p = (VRRP_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VRRP_MGR_IPC_ASSOCIPADDRENTRYDEL;
    
    memset(&(msg_p->data.assoc_ip_addr), 0, sizeof(VRRP_ASSOC_IP_ENTRY_T));
    memcpy(&(msg_p->data.assoc_ip_addr), vrrp_assoc_ip_addr, sizeof(VRRP_ASSOC_IP_ENTRY_T));
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.assoc_ip_addr), msgbuf_p) != SYSFUN_OK)
    {
        return VRRP_TYPE_RESULT_FAIL;
    }
    
    return msg_p->type.result_ui32;
    
}


/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_GetVrrpAssocIpAddrEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : Get the VRRP associated IP addresses entry of the specific vrrp
 * INPUT    : buffer which pointers to vrrp_assoc_ip_addr
 * OUTPUT   : vrrp_assoc_ip_addr
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_INTERNAL_ERROR
 * NOTES    : use (vrrp_assoc_ip_addr->ifindex, vrrp_assoc_ip_addr->vrid,
 *            vrrp_assoc_ip_addr->assoc_ip_addr) as index
 *-------------------------------------------------------------------------- */
UI32_T VRRP_PMGR_GetVrrpAssocIpAddrEntry(VRRP_ASSOC_IP_ENTRY_T *vrrp_assoc_ip_addr)
{
    VRRP_MGR_IPCMsg_T *msg_p = NULL;
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.assoc_ip_addr))];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    
    msgbuf_p->cmd = SYS_MODULE_VRRP;
    msgbuf_p->msg_size = VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.assoc_ip_addr);
    msg_p = (VRRP_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VRRP_MGR_IPC_ASSOCIPADDRENTRYGET;
    
    memset(&(msg_p->data.assoc_ip_addr), 0, sizeof(VRRP_ASSOC_IP_ENTRY_T));
    memcpy(&(msg_p->data.assoc_ip_addr), vrrp_assoc_ip_addr, sizeof(VRRP_ASSOC_IP_ENTRY_T));
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.assoc_ip_addr), msgbuf_p) != SYSFUN_OK)
    {
        return VRRP_TYPE_RESULT_FAIL;
    }
    
    memcpy(vrrp_assoc_ip_addr, &(msg_p->data.assoc_ip_addr), sizeof(VRRP_ASSOC_IP_ENTRY_T));
    
    return msg_p->type.result_ui32;
    
}


/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_GetNextVrrpAssocIpAddrEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : Get the next VRRP associated IP addresses entry of the specific vrrp
 * INPUT    : buffer which pointers to vrrp_assoc_ip_addr
 * OUTPUT   : next ifindex, next vrid, next vrrp_assoc_ip_addr
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_INTERNAL_ERROR
 * NOTES    : use (vrrp_assoc_ip_addr->ifindex, vrrp_assoc_ip_addr->vrid,
 *            vrrp_assoc_ip_addr->assoc_ip_addr) as index
 *-------------------------------------------------------------------------- */
UI32_T VRRP_PMGR_GetNextVrrpAssocIpAddrEntry(VRRP_ASSOC_IP_ENTRY_T *vrrp_assoc_ip_addr)
{
    VRRP_MGR_IPCMsg_T *msg_p = NULL;
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.assoc_ip_addr))];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    
    msgbuf_p->cmd = SYS_MODULE_VRRP;
    msgbuf_p->msg_size = VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.assoc_ip_addr);
    msg_p = (VRRP_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VRRP_MGR_IPC_ASSOCIPADDRENTRYGETNEXT;
    
    memset(&(msg_p->data.assoc_ip_addr), 0, sizeof(VRRP_ASSOC_IP_ENTRY_T));
    memcpy(&(msg_p->data.assoc_ip_addr), vrrp_assoc_ip_addr, sizeof(VRRP_ASSOC_IP_ENTRY_T));
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.assoc_ip_addr), msgbuf_p) != SYSFUN_OK)
    {
        return VRRP_TYPE_RESULT_FAIL;
    }
    
    memcpy(vrrp_assoc_ip_addr, &(msg_p->data.assoc_ip_addr), sizeof(VRRP_ASSOC_IP_ENTRY_T));
    
    return msg_p->type.result_ui32;
    
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_GetNextVrrpAssocIpAddrEntryBySnmp
 *--------------------------------------------------------------------------
 * PURPOSE  : Get the next VRRP associated IP addresses entry
 * INPUT    : buffer which pointers to vrrp_assoc_ip_addr
 * OUTPUT   : next ifindex, next vrid, next vrrp_assoc_ip_addr
 * RETURN   : TRUE/FALSE
 * NOTES    :(1)use (vrrp_assoc_ip_addr->ifindex, vrrp_assoc_ip_addr->vrid,
 *              vrrp_assoc_ip_addr->assoc_ip_addr) as index
 *           (2)This API is used by snmp to getnext AssocIpAddrEntry through
 *              whole system
 *-------------------------------------------------------------------------- */
BOOL_T VRRP_PMGR_GetNextVrrpAssocIpAddrEntryBySnmp(VRRP_ASSOC_IP_ENTRY_T *vrrp_assoc_ip_addr)
{
    VRRP_MGR_IPCMsg_T *msg_p = NULL;
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.assoc_ip_addr))];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    
    msgbuf_p->cmd = SYS_MODULE_VRRP;
    msgbuf_p->msg_size = VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.assoc_ip_addr);
    msg_p = (VRRP_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VRRP_MGR_IPC_ASSOCIPADDRENTRYGETNEXT_SNMP;
    
    memset(&(msg_p->data.assoc_ip_addr), 0, sizeof(VRRP_ASSOC_IP_ENTRY_T));
    memcpy(&(msg_p->data.assoc_ip_addr), vrrp_assoc_ip_addr, sizeof(VRRP_ASSOC_IP_ENTRY_T));
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.assoc_ip_addr), msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }
    
    memcpy(vrrp_assoc_ip_addr, &(msg_p->data.assoc_ip_addr), sizeof(VRRP_ASSOC_IP_ENTRY_T));
    
    return msg_p->type.result_bool;
}


/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_GetIpAddrCount
 *--------------------------------------------------------------------------
 * PURPOSE  : Get the number of IP addresses for each vrrp (RFC2338, p.14)
 * INPUT    : ifindex, vrid
 * OUTPUT   : ip address count of the specific vrrp group
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR/
 * NOTES    : none
 *-------------------------------------------------------------------------- */
UI32_T VRRP_PMGR_GetIpAddrCount(UI32_T if_index, UI8_T vrid, UI32_T *vrrp_oper_ip_addr_count)
{
    VRRP_MGR_IPCMsg_T *msg_p = NULL;
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp3))];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    
    msgbuf_p->cmd = SYS_MODULE_VRRP;
    msgbuf_p->msg_size = VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp3);
    msg_p = (VRRP_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VRRP_MGR_IPC_OPERIPADDRCOUNTGET;
    
    memset(&(msg_p->data.arg_grp3), 0, sizeof(msg_p->data.arg_grp3));
    msg_p->data.arg_grp3.arg1 = if_index;
    msg_p->data.arg_grp3.arg2 = vrid;
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp3), msgbuf_p) != SYSFUN_OK)
    {
        return VRRP_TYPE_RESULT_FAIL;
    }
    *vrrp_oper_ip_addr_count = msg_p->data.arg_grp3.arg3;
    
    return msg_p->type.result_ui32;
    
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_GetOperState
 *------------------------------------------------------------------------------
 * PURPOSE  : This funciton gets the operation state of the specific vrid and ifindex
 * INPUT    : ifIndex  ,  vrid
 * OUTPUT   : vrrp oper state of the specific vrrp group -
 *            VAL_vrrpOperState_initialize       1L
 *            VAL_vrrpOperState_backup           2L
 *            VAL_vrrpOperState_master           3L
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR/
 * NOTES    : The administrative status requested by management for VRRP.  The
 *            value enabled(1) indicates that specify VRRP virtual router should
 *            be enabled on this interface.  When disabled(2), VRRP virtual Router
 *            is not operated. This object affects all VRRP Applicant and Registrar
 *            state machines.
 *------------------------------------------------------------------------------*/
UI32_T VRRP_PMGR_GetOperState(UI32_T if_index, UI8_T vrid, UI32_T *vrrp_oper_state)
{
    VRRP_MGR_IPCMsg_T *msg_p = NULL;
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp3))];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    
    msgbuf_p->cmd = SYS_MODULE_VRRP;
    msgbuf_p->msg_size = VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp3);
    msg_p = (VRRP_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VRRP_MGR_IPC_OPERSTATEGET;
    
    memset(&(msg_p->data.arg_grp3), 0, sizeof(msg_p->data.arg_grp3));
    msg_p->data.arg_grp3.arg1 = if_index;
    msg_p->data.arg_grp3.arg2 = vrid;
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp3), msgbuf_p) != SYSFUN_OK)
    {
        return VRRP_TYPE_RESULT_FAIL;
    }
    *vrrp_oper_state = msg_p->data.arg_grp3.arg3;
    
    return msg_p->type.result_ui32;
    
}


/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_SetOperProtocol
 *------------------------------------------------------------------------------
 * PURPOSE  : Set operation protocol for the specific ifindex and vrid
 * INPUT    : ifIndex  ,  vrid, operation protocol
 * OUTPUT   : vrrp oper protocol of the specific vrrp group -
 *            VAL_vrrpOperProtocol_ip                 1L
 *            VAL_vrrpOperProtocol_bridge             2L
 *            VAL_vrrpOperProtocol_decnet             3L
 *            VAL_vrrpOperProtocol_other              4L
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_INTERNAL_ERROR
 * NOTES    : This funciton returns true if the VRRP status is successfully Get.
 *            Otherwise, return false.
 *------------------------------------------------------------------------------*/
UI32_T VRRP_PMGR_SetOperProtocol(UI32_T if_index, UI8_T vrid, UI32_T vrrp_oper_protocol)
{
    VRRP_MGR_IPCMsg_T *msg_p = NULL;
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp3))];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    
    msgbuf_p->cmd = SYS_MODULE_VRRP;
    msgbuf_p->msg_size = VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp3);
    msg_p = (VRRP_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VRRP_MGR_IPC_OPERPROTOCOLSET;
    
    memset(&(msg_p->data.arg_grp3), 0, sizeof(msg_p->data.arg_grp3));
    msg_p->data.arg_grp3.arg1 = if_index;
    msg_p->data.arg_grp3.arg2 = vrid;
    msg_p->data.arg_grp3.arg3 = vrrp_oper_protocol;
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp3), msgbuf_p) != SYSFUN_OK)
    {
        return VRRP_TYPE_RESULT_FAIL;
    }
    return msg_p->type.result_ui32;
    
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_GetOperProtocol
 *------------------------------------------------------------------------------
 * PURPOSE  : This funciton returns true if the VRRP status is successfully Get.
 *            Otherwise, return false.
 * INPUT    : ifIndex  ,  vrid
 * OUTPUT   : vrrp oper protocol of the specific vrrp group -
 *            VAL_vrrpOperProtocol_ip                 1L
 *            VAL_vrrpOperProtocol_bridge             2L
 *            VAL_vrrpOperProtocol_decnet             3L
 *            VAL_vrrpOperProtocol_other              4L
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR
 * NOTES    : None
 *------------------------------------------------------------------------------*/
UI32_T VRRP_PMGR_GetOperProtocol(UI32_T if_index, UI8_T vrid, UI32_T *vrrp_oper_protocol)
{
    VRRP_MGR_IPCMsg_T *msg_p = NULL;
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp3))];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    
    msgbuf_p->cmd = SYS_MODULE_VRRP;
    msgbuf_p->msg_size = VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp3);
    msg_p = (VRRP_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VRRP_MGR_IPC_OPERPROTOCOLGET;
    
    memset(&(msg_p->data.arg_grp3), 0, sizeof(msg_p->data.arg_grp3));
    msg_p->data.arg_grp3.arg1 = if_index;
    msg_p->data.arg_grp3.arg2 = vrid;
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp3), msgbuf_p) != SYSFUN_OK)
    {
        return VRRP_TYPE_RESULT_FAIL;
    }
    *vrrp_oper_protocol = msg_p->data.arg_grp3.arg3;
    
    return msg_p->type.result_ui32;
    
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_GetVersionNumber
 *------------------------------------------------------------------------------
 * PURPOSE  : It is used to identify the vrrp version of the system.
 * INPUT    : buffer to be put in the version number
 * OUTPUT   : The vrrp version number
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR
 * NOTES    : 1. Please refer to RFC 2338 , section 5.3.1 for detailed information.
 *            2. It's always return "2" right now.
 *------------------------------------------------------------------------------*/
UI32_T VRRP_PMGR_GetVersionNumber(UI32_T *version)
{
    VRRP_MGR_IPCMsg_T *msg_p = NULL;
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.version))];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    
    msgbuf_p->cmd = SYS_MODULE_VRRP;
    msgbuf_p->msg_size = VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.version);
    msg_p = (VRRP_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VRRP_MGR_IPC_VERSIONNUMBERGET;
    
    memset(&(msg_p->data.version), 0, sizeof(msg_p->data.version));
    msg_p->data.version = *version;
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.version), msgbuf_p) != SYSFUN_OK)
    {
        return VRRP_TYPE_RESULT_FAIL;
    }
    *version = msg_p->data.version;
    
    return msg_p->type.result_ui32;
    
}


/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_GetVrrpSysStatistics
 *------------------------------------------------------------------------------
 * PURPOSE  : To retrive the system's vrrp statistics.
 * INPUT    : none
 * OUTPUT   : *vrrp_sys_statistics -- the statistics of the vrrp sytem,
 *            ChkSumErrors \ versionErrors \ vridErrors
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR/
 *			  VRRP_TYPE_INTERNAL_ERROR
 * NOTES    : none
 *------------------------------------------------------------------------------*/
UI32_T VRRP_PMGR_GetVrrpSysStatistics(VRRP_OM_Router_Statistics_Info_T *vrrp_router_statistics)
{
    VRRP_MGR_IPCMsg_T *msg_p = NULL;
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.vrrp_router_statistics))];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    
    msgbuf_p->cmd = SYS_MODULE_VRRP;
    msgbuf_p->msg_size = VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.vrrp_router_statistics);
    msg_p = (VRRP_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VRRP_MGR_IPC_VRRPSYSSTATISTICSGET;
    
    memset(&(msg_p->data.vrrp_router_statistics), 0, sizeof(VRRP_OM_Router_Statistics_Info_T));
    memcpy(&(msg_p->data.vrrp_router_statistics), vrrp_router_statistics, sizeof(VRRP_OM_Router_Statistics_Info_T));
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.vrrp_router_statistics), msgbuf_p) != SYSFUN_OK)
    {
        return VRRP_TYPE_RESULT_FAIL;
    }
    
    memcpy(vrrp_router_statistics, &(msg_p->data.vrrp_router_statistics), sizeof(VRRP_OM_Router_Statistics_Info_T));
    
    return msg_p->type.result_ui32;
    
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_ClearVrrpSysStatistics
 *------------------------------------------------------------------------------
 * PURPOSE  : To clear the system's vrrp statistics.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_INTERNAL_ERROR
 * NOTES    : none
 *------------------------------------------------------------------------------*/
UI32_T VRRP_PMGR_ClearVrrpSysStatistics(void)
{
    VRRP_MGR_IPCMsg_T *msg_p = NULL;
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(VRRP_MGR_IPCMSG_TYPE_SIZE)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    
    msgbuf_p->cmd = SYS_MODULE_VRRP;
    msgbuf_p->msg_size = VRRP_MGR_IPCMSG_TYPE_SIZE;
    msg_p = (VRRP_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VRRP_MGR_IPC_VRRPSYSSTATISTICSCLEAR;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              VRRP_MGR_IPCMSG_TYPE_SIZE, msgbuf_p) != SYSFUN_OK)
    {
        return VRRP_TYPE_RESULT_FAIL;
    }
    
    return msg_p->type.result_ui32;
    
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_ClearVrrpGroupStatistics
 *------------------------------------------------------------------------------
 * PURPOSE  : To clear the specific vrrp group statistics.
 * INPUT    : ifindex, vrid
 * OUTPUT   : none
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_INTERNAL_ERROR
 * NOTES    : none
 *------------------------------------------------------------------------------*/
UI32_T VRRP_PMGR_ClearVrrpGroupStatistics(UI32_T if_index, UI8_T vrid)
{
    VRRP_MGR_IPCMsg_T *msg_p = NULL;
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp8))];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    
    msgbuf_p->cmd = SYS_MODULE_VRRP;
    msgbuf_p->msg_size = VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp8);
    msg_p = (VRRP_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VRRP_MGR_IPC_VRRPGROUPSTATISTICSCLEAR;
    
    memset(&(msg_p->data.arg_grp8), 0, sizeof(msg_p->data.arg_grp8));
    msg_p->data.arg_grp8.arg1 = if_index;
    msg_p->data.arg_grp8.arg2 = vrid;
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp8), msgbuf_p) != SYSFUN_OK)
    {
        return VRRP_TYPE_RESULT_FAIL;
    }
    return msg_p->type.result_ui32;
    
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_GetVrrpGroupStatistics
 *------------------------------------------------------------------------------
 * PURPOSE  : To retrive the specific vrrp group statistics.
 * INPUT    : ifindex, vrid, buffer to be put in statistics info
 * OUTPUT   : *vrrp_sys_statistics -- the statistics of the vrrp sytem,
 *            ChkSumErrors \ versionErrors \ vridErrors
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR
 * NOTES    : none
 *------------------------------------------------------------------------------*/
UI32_T VRRP_PMGR_GetVrrpGroupStatistics(UI32_T if_index, UI8_T vrid, VRRP_OM_Vrrp_Statistics_Info_T *vrrp_group_statistics)
{
    VRRP_MGR_IPCMsg_T *msg_p = NULL;
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp9))];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    
    msgbuf_p->cmd = SYS_MODULE_VRRP;
    msgbuf_p->msg_size = VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp9);
    msg_p = (VRRP_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VRRP_MGR_IPC_VRRPGROUPSTATISTICSGET;
    
    memset(&(msg_p->data.arg_grp9), 0, sizeof(VRRP_OM_Vrrp_Statistics_Info_T));
    msg_p->data.arg_grp9.arg1 = if_index;
    msg_p->data.arg_grp9.arg2 = vrid;
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp9), msgbuf_p) != SYSFUN_OK)
    {
        return VRRP_TYPE_RESULT_FAIL;
    }
    
    memcpy(vrrp_group_statistics, &(msg_p->data.arg_grp9.arg3), sizeof(VRRP_OM_Vrrp_Statistics_Info_T));
    
    return msg_p->type.result_ui32;
    
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_GetNextVrrpGroupStatistics
 *------------------------------------------------------------------------------
 * PURPOSE  : To retrive the next specific vrrp group statistics.
 * INPUT    : ifindex, vrid, buffer to be put in statistics info
 * OUTPUT   : *vrrp_sys_statistics -- the statistics of the vrrp sytem,
 *            ChkSumErrors \ versionErrors \ vridErrors
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR
 * NOTES    : none
 *------------------------------------------------------------------------------*/
UI32_T VRRP_PMGR_GetNextVrrpGroupStatistics(UI32_T *if_index, UI8_T *vrid, VRRP_OM_Vrrp_Statistics_Info_T *vrrp_group_statistics)
{
    VRRP_MGR_IPCMsg_T *msg_p = NULL;
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp9))];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    
    msgbuf_p->cmd = SYS_MODULE_VRRP;
    msgbuf_p->msg_size = VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp9);
    msg_p = (VRRP_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VRRP_MGR_IPC_VRRPGROUPSTATISTICSGETNEXT;
    
    memset(&(msg_p->data.arg_grp9), 0, sizeof(VRRP_OM_Vrrp_Statistics_Info_T));
    msg_p->data.arg_grp9.arg1 = *if_index;
    msg_p->data.arg_grp9.arg2 = *vrid;
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp9), msgbuf_p) != SYSFUN_OK)
    {
        return VRRP_TYPE_RESULT_FAIL;
    }
    
    *if_index = msg_p->data.arg_grp9.arg1;
    *vrid = msg_p->data.arg_grp9.arg2;
    memcpy(vrrp_group_statistics, &(msg_p->data.arg_grp9.arg3), sizeof(VRRP_OM_Vrrp_Statistics_Info_T));
    
    return msg_p->type.result_ui32;
    
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_GetRunningVrrpPriority
 *------------------------------------------------------------------------------
 * PURPOSE  : Get VRRP status and check whether it is default value.
 * INPUT    : ifindex,
 *            vrid,
 *            priority -- output buffer
 * OUTPUT   : priority -- output vrrp priority
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- not same as default
 *          : SYS_TYPE_GET_RUNNING_CFG_FAIL      -- get failure
 *          : SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 * NOTES    : none
 *------------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T VRRP_PMGR_GetRunningVrrpPriority(UI32_T ifindex, UI8_T vrid, UI32_T *priority)
{
    VRRP_MGR_IPCMsg_T *msg_p = NULL;
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp3))];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    
    msgbuf_p->cmd = SYS_MODULE_VRRP;
    msgbuf_p->msg_size = VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp3);
    msg_p = (VRRP_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VRRP_MGR_IPC_RUNNINGPRIORITYGET;
    
    memset(&(msg_p->data.arg_grp3), 0, sizeof(msg_p->data.arg_grp3));
    msg_p->data.arg_grp3.arg1 = ifindex;
    msg_p->data.arg_grp3.arg2 = vrid;
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp3), msgbuf_p) != SYSFUN_OK)
    {
        return VRRP_TYPE_RESULT_FAIL;
    }
    *priority = msg_p->data.arg_grp3.arg3;
    
    return msg_p->type.result_ui32;
    
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_GetNextRunningVrrpPriority
 *------------------------------------------------------------------------------
 * PURPOSE  : Get VRRP status and check whether it is default value.
 * INPUT    : ifindex,
 *            vrid,
 *            priority -- output buffer
 * OUTPUT   : ifindex  -- next ifindex
 *            vrid     -- next vrid
 *            priority -- output vrrp priority
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- not same as default
 *          : SYS_TYPE_GET_RUNNING_CFG_FAIL      -- get failure
 *          : SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 * NOTES    : none
 *------------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T VRRP_PMGR_GetNextRunningVrrpPriority(UI32_T *ifindex, UI8_T *vrid, UI32_T *priority)
{
    VRRP_MGR_IPCMsg_T *msg_p = NULL;
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp3))];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    
    msgbuf_p->cmd = SYS_MODULE_VRRP;
    msgbuf_p->msg_size = VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp3);
    msg_p = (VRRP_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VRRP_MGR_IPC_RUNNINGPRIORITYGETNEXT;
    
    memset(&(msg_p->data.arg_grp3), 0, sizeof(msg_p->data.arg_grp3));
    msg_p->data.arg_grp3.arg1 = *ifindex;
    msg_p->data.arg_grp3.arg2 = *vrid;
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp3), msgbuf_p) != SYSFUN_OK)
    {
        return VRRP_TYPE_RESULT_FAIL;
    }
    *ifindex = msg_p->data.arg_grp3.arg1;
    *vrid = msg_p->data.arg_grp3.arg2;
    *priority = msg_p->data.arg_grp3.arg3;
    
    return msg_p->type.result_ui32;
    
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_GetRunningVrrpAuthType
 *------------------------------------------------------------------------------
 * PURPOSE  : Get VRRP status and check whether it is default value.
 * INPUT    : ifindex,
 *            vrid,
 *            auth_type -- output buffer
 * OUTPUT   : auth_type -- output vrrp authentication type
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- not same as default
 *          : SYS_TYPE_GET_RUNNING_CFG_FAIL      -- get failure
 *          : SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 * NOTES    : none
 *------------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T VRRP_PMGR_GetRunningVrrpAuthType(UI32_T ifindex, UI8_T vrid, UI32_T *auth_type, UI8_T *vrrp_oper_auth_key)
{
    VRRP_MGR_IPCMsg_T *msg_p = NULL;
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp5))];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    
    msgbuf_p->cmd = SYS_MODULE_VRRP;
    msgbuf_p->msg_size = VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp5);
    msg_p = (VRRP_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VRRP_MGR_IPC_RUNNINGAUTHTYPEGET;
    
    memset(&(msg_p->data.arg_grp5), 0, sizeof(msg_p->data.arg_grp5));
    msg_p->data.arg_grp5.arg1 = ifindex;
    msg_p->data.arg_grp5.arg2 = vrid;
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp5), msgbuf_p) != SYSFUN_OK)
    {
        return VRRP_TYPE_RESULT_FAIL;
    }
    
    *auth_type = msg_p->data.arg_grp5.arg3;
    strncpy((char *)vrrp_oper_auth_key, (char *)msg_p->data.arg_grp5.arg4, strlen((char *)msg_p->data.arg_grp5.arg4));
    return msg_p->type.result_ui32;
    
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_GetNextRunningVrrpAuthType
 *------------------------------------------------------------------------------
 * PURPOSE  : Get VRRP status and check whether it is default value.
 * INPUT    : ifindex,
 *            vrid,
 *            auth_type -- output buffer
 * OUTPUT   : ifindex   -- next ifindex
 *            vrid      -- next vrid
 *            auth_type -- output vrrp authentication type
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- not same as default
 *          : SYS_TYPE_GET_RUNNING_CFG_FAIL      -- get failure
 *          : SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 * NOTES    : none
 *------------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T VRRP_PMGR_GetNextRunningVrrpAuthType(UI32_T *ifindex, UI8_T *vrid, UI32_T *auth_type, UI8_T *vrrp_oper_auth_key)
{
    VRRP_MGR_IPCMsg_T *msg_p = NULL;
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp5))];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    
    msgbuf_p->cmd = SYS_MODULE_VRRP;
    msgbuf_p->msg_size = VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp5);
    msg_p = (VRRP_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VRRP_MGR_IPC_RUNNINGAUTHTYPEGETNEXT;
    
    memset(&(msg_p->data.arg_grp5), 0, sizeof(msg_p->data.arg_grp5));
    msg_p->data.arg_grp5.arg1 = *ifindex;
    msg_p->data.arg_grp5.arg2 = *vrid;
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp5), msgbuf_p) != SYSFUN_OK)
    {
        return VRRP_TYPE_RESULT_FAIL;
    }
    
    *ifindex = msg_p->data.arg_grp5.arg1;
    *vrid = msg_p->data.arg_grp5.arg2;
    *auth_type = msg_p->data.arg_grp5.arg3;
    strncpy((char *)vrrp_oper_auth_key, (char *)msg_p->data.arg_grp5.arg4, strlen((char *)msg_p->data.arg_grp5.arg4));
    return msg_p->type.result_ui32;
    
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_GetRunningVrrpAdverInt
 *------------------------------------------------------------------------------
 * PURPOSE  : Get VRRP status and check whether it is default value.
 * INPUT    : ifindex,
 *            vrid,
 *            interval -- output buffer
 * OUTPUT   : interval -- output vrrp advertisement interval
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- not same as default
 *          : SYS_TYPE_GET_RUNNING_CFG_FAIL      -- get failure
 *          : SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 * NOTES    : none
 *------------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T VRRP_PMGR_GetRunningVrrpAdverInt(UI32_T ifindex, UI8_T vrid, UI32_T *interval)
{
    VRRP_MGR_IPCMsg_T *msg_p = NULL;
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp3))];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    
    msgbuf_p->cmd = SYS_MODULE_VRRP;
    msgbuf_p->msg_size = VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp3);
    msg_p = (VRRP_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VRRP_MGR_IPC_RUNNINGADVERINTGET;
    
    memset(&(msg_p->data.arg_grp3), 0, sizeof(msg_p->data.arg_grp3));
    msg_p->data.arg_grp3.arg1 = ifindex;
    msg_p->data.arg_grp3.arg2 = vrid;
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp3), msgbuf_p) != SYSFUN_OK)
    {
        return VRRP_TYPE_RESULT_FAIL;
    }
    *interval = msg_p->data.arg_grp3.arg3;
    
    return msg_p->type.result_ui32;
    
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_GetNextRunningVrrpAdverInt
 *------------------------------------------------------------------------------
 * PURPOSE  : Get VRRP status and check whether it is default value.
 * INPUT    : ifindex,
 *            vrid,
 *            interval -- output buffer
 * OUTPUT   : ifindex  -- next ifindex
 *            vrid     -- next vrid
 *            interval -- output vrrp advertisement interval
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- not same as default
 *          : SYS_TYPE_GET_RUNNING_CFG_FAIL      -- get failure
 *          : SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 * NOTES    : none
 *------------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T VRRP_PMGR_GetNextRunningVrrpAdverInt(UI32_T *ifindex, UI8_T *vrid, UI32_T *interval)
{
    VRRP_MGR_IPCMsg_T *msg_p = NULL;
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp3))];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    
    msgbuf_p->cmd = SYS_MODULE_VRRP;
    msgbuf_p->msg_size = VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp3);
    msg_p = (VRRP_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VRRP_MGR_IPC_RUNNINGADVERINTGETNEXT;
    
    memset(&(msg_p->data.arg_grp3), 0, sizeof(msg_p->data.arg_grp3));
    msg_p->data.arg_grp3.arg1 = *ifindex;
    msg_p->data.arg_grp3.arg2 = *vrid;
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp3), msgbuf_p) != SYSFUN_OK)
    {
        return VRRP_TYPE_RESULT_FAIL;
    }
    *ifindex = msg_p->data.arg_grp3.arg1;
    *vrid = msg_p->data.arg_grp3.arg2;
    *interval = msg_p->data.arg_grp3.arg3;
    
    return msg_p->type.result_ui32;
    
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_GetRunningVrrpPreemptMode
 *------------------------------------------------------------------------------
 * PURPOSE  : Get VRRP status and check whether it is default value.
 * INPUT    : ifindex,
 *            vrid,
 *            preempt_mode -- output buffer
 * OUTPUT   : preempt_mode -- output vrrp preempt mode
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- not same as default
 *          : SYS_TYPE_GET_RUNNING_CFG_FAIL      -- get failure
 *          : SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 * NOTES    : none
 *------------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T VRRP_PMGR_GetRunningVrrpPreemptMode(UI32_T ifindex, UI8_T vrid, UI32_T *preempt_mode)
{
    VRRP_MGR_IPCMsg_T *msg_p = NULL;
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp3))];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    
    msgbuf_p->cmd = SYS_MODULE_VRRP;
    msgbuf_p->msg_size = VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp3);
    msg_p = (VRRP_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VRRP_MGR_IPC_RUNNINGPREEMPTGET;
    
    memset(&(msg_p->data.arg_grp3), 0, sizeof(msg_p->data.arg_grp3));
    msg_p->data.arg_grp3.arg1 = ifindex;
    msg_p->data.arg_grp3.arg2 = vrid;
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp3), msgbuf_p) != SYSFUN_OK)
    {
        return VRRP_TYPE_RESULT_FAIL;
    }
    *preempt_mode = msg_p->data.arg_grp3.arg3;
    
    return msg_p->type.result_ui32;
    
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_GetNextRunningVrrpPreemptMode
 *------------------------------------------------------------------------------
 * PURPOSE  : Get VRRP status and check whether it is default value.
 * INPUT    : ifindex,
 *            vrid,
 *            preempt_mode -- output buffer
 * OUTPUT   : ifindex      -- next ifindex
 *            vrid         -- next vrid
 *            preempt_mode -- output vrrp preempt mode
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- not same as default
 *          : SYS_TYPE_GET_RUNNING_CFG_FAIL      -- get failure
 *          : SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 * NOTES    : none
 *------------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T VRRP_PMGR_GetNextRunningVrrpPreemptMode(UI32_T *ifindex, UI8_T *vrid, UI32_T *preempt_mode)
{
    VRRP_MGR_IPCMsg_T *msg_p = NULL;
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp3))];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    
    msgbuf_p->cmd = SYS_MODULE_VRRP;
    msgbuf_p->msg_size = VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp3);
    msg_p = (VRRP_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VRRP_MGR_IPC_RUNNINGPREEMPTGETNEXT;
    
    memset(&(msg_p->data.arg_grp3), 0, sizeof(msg_p->data.arg_grp3));
    msg_p->data.arg_grp3.arg1 = *ifindex;
    msg_p->data.arg_grp3.arg2 = *vrid;
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp3), msgbuf_p) != SYSFUN_OK)
    {
        return VRRP_TYPE_RESULT_FAIL;
    }
    *ifindex = msg_p->data.arg_grp3.arg1;
    *vrid = msg_p->data.arg_grp3.arg2;
    *preempt_mode = msg_p->data.arg_grp3.arg3;
    
    return msg_p->type.result_ui32;
    
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_GetRunningVrrpPreemptDelay
 *------------------------------------------------------------------------------
 * PURPOSE  : Get VRRP status and check whether it is default value.
 * INPUT    : ifindex,
 *            vrid,
 *            preempt_mode -- output buffer
 * OUTPUT   : preempt_mode -- output vrrp preempt mode
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- not same as default
 *          : SYS_TYPE_GET_RUNNING_CFG_FAIL      -- get failure
 *          : SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 * NOTES    : none
 *------------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T VRRP_PMGR_GetRunningVrrpPreemptDelay(UI32_T ifindex, UI8_T vrid, UI32_T *preempt_delay)
{
    VRRP_MGR_IPCMsg_T *msg_p = NULL;
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp3))];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    
    msgbuf_p->cmd = SYS_MODULE_VRRP;
    msgbuf_p->msg_size = VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp3);
    msg_p = (VRRP_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VRRP_MGR_IPC_RUNNINGPREEMPTDELAYGET;
    
    memset(&(msg_p->data.arg_grp3), 0, sizeof(msg_p->data.arg_grp3));
    msg_p->data.arg_grp3.arg1 = ifindex;
    msg_p->data.arg_grp3.arg2 = vrid;
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp3), msgbuf_p) != SYSFUN_OK)
    {
        return VRRP_TYPE_RESULT_FAIL;
    }
    *preempt_delay = msg_p->data.arg_grp3.arg3;
    
    return msg_p->type.result_ui32;
    
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_GetNextRunningVrrpPreemptDelay
 *------------------------------------------------------------------------------
 * PURPOSE  : Get VRRP status and check whether it is default value.
 * INPUT    : ifindex,
 *            vrid,
 *            preempt_mode -- output buffer
 * OUTPUT   : ifindex      -- next ifindex
 *            vrid         -- next vrid
 *            preempt_mode -- output vrrp preempt mode
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- not same as default
 *          : SYS_TYPE_GET_RUNNING_CFG_FAIL      -- get failure
 *          : SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 * NOTES    : none
 *------------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T VRRP_PMGR_GetNextRunningVrrpPreemptDelay(UI32_T *ifindex, UI8_T *vrid, UI32_T *preempt_delay)
{
    VRRP_MGR_IPCMsg_T *msg_p = NULL;
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp3))];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    
    msgbuf_p->cmd = SYS_MODULE_VRRP;
    msgbuf_p->msg_size = VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp3);
    msg_p = (VRRP_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VRRP_MGR_IPC_RUNNINGPREEMPTDELAYGETNEXT;
    
    memset(&(msg_p->data.arg_grp3), 0, sizeof(msg_p->data.arg_grp3));
    msg_p->data.arg_grp3.arg1 = *ifindex;
    msg_p->data.arg_grp3.arg2 = *vrid;
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp3), msgbuf_p) != SYSFUN_OK)
    {
        return VRRP_TYPE_RESULT_FAIL;
    }
    *ifindex = msg_p->data.arg_grp3.arg1;
    *vrid = msg_p->data.arg_grp3.arg2;
    *preempt_delay = msg_p->data.arg_grp3.arg3;
    
    return msg_p->type.result_ui32;
    
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_GetRunningVrrpProtocol
 *------------------------------------------------------------------------------
 * PURPOSE  : Get VRRP status and check whether it is default value.
 * INPUT    : ifindex,
 *            vrid,
 *            protocol -- output buffer
 * OUTPUT   : protocol -- output vrrp protocol
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- not same as default
 *          : SYS_TYPE_GET_RUNNING_CFG_FAIL      -- get failure
 *          : SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 * NOTES    : none
 *------------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T VRRP_PMGR_GetRunningVrrpProtocol(UI32_T ifindex, UI8_T vrid, UI32_T *protocol)
{
    VRRP_MGR_IPCMsg_T *msg_p = NULL;
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp3))];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    
    msgbuf_p->cmd = SYS_MODULE_VRRP;
    msgbuf_p->msg_size = VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp3);
    msg_p = (VRRP_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VRRP_MGR_IPC_RUNNINGPROTOCOLGET;
    
    memset(&(msg_p->data.arg_grp3), 0, sizeof(msg_p->data.arg_grp3));
    msg_p->data.arg_grp3.arg1 = ifindex;
    msg_p->data.arg_grp3.arg2 = vrid;
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp3), msgbuf_p) != SYSFUN_OK)
    {
        return VRRP_TYPE_RESULT_FAIL;
    }
    *protocol = msg_p->data.arg_grp3.arg3;
    
    return msg_p->type.result_ui32;
    
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_GetNextRunningVrrpProtocol
 *------------------------------------------------------------------------------
 * PURPOSE  : Get VRRP status and check whether it is default value.
 * INPUT    : ifindex,
 *            vrid,
 *            protocol -- output buffer
 * OUTPUT   : ifindex  -- next ifindex
 *            vrid     -- next vrid
 *            protocol -- output vrrp protocol
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- not same as default
 *          : SYS_TYPE_GET_RUNNING_CFG_FAIL      -- get failure
 *          : SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 * NOTES    : none
 *------------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T VRRP_PMGR_GetNextRunningVrrpProtocol(UI32_T *ifindex, UI8_T *vrid, UI32_T *protocol)
{
    VRRP_MGR_IPCMsg_T *msg_p = NULL;
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp3))];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    
    msgbuf_p->cmd = SYS_MODULE_VRRP;
    msgbuf_p->msg_size = VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp3);
    msg_p = (VRRP_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VRRP_MGR_IPC_RUNNINGPROTOCOLGETNEXT;
    
    memset(&(msg_p->data.arg_grp3), 0, sizeof(msg_p->data.arg_grp3));
    msg_p->data.arg_grp3.arg1 = *ifindex;
    msg_p->data.arg_grp3.arg2 = *vrid;
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp3), msgbuf_p) != SYSFUN_OK)
    {
        return VRRP_TYPE_RESULT_FAIL;
    }
    *ifindex = msg_p->data.arg_grp3.arg1;
    *vrid = msg_p->data.arg_grp3.arg2;
    *protocol = msg_p->data.arg_grp3.arg3;
    
    return msg_p->type.result_ui32;
    
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_GetRunningVrrpAssocIp
 *------------------------------------------------------------------------------
 * PURPOSE  : Get VRRP status and check whether it is default value.
 * INPUT    : ifindex,
 *            vrid,
 *            ip_addr  -- output buffer
 * OUTPUT   : ip_addr_count  -- output vrrp associated ip address
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- not same as default
 *          : SYS_TYPE_GET_RUNNING_CFG_FAIL      -- get failure
 *          : SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 * NOTES    : none
 *------------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T VRRP_PMGR_GetRunningVrrpAssocIp(UI32_T ifindex, UI8_T vrid, UI8_T *ip_addr_count)
{
    VRRP_MGR_IPCMsg_T *msg_p = NULL;
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp3))];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    
    msgbuf_p->cmd = SYS_MODULE_VRRP;
    msgbuf_p->msg_size = VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp3);
    msg_p = (VRRP_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VRRP_MGR_IPC_RUNNINGASSOCIPGET;
    
    memset(&(msg_p->data.arg_grp3), 0, sizeof(msg_p->data.arg_grp3));
    msg_p->data.arg_grp3.arg1 = ifindex;
    msg_p->data.arg_grp3.arg2 = vrid;
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp3), msgbuf_p) != SYSFUN_OK)
    {
        return VRRP_TYPE_RESULT_FAIL;
    }
    *ip_addr_count = msg_p->data.arg_grp3.arg3;
    
    return msg_p->type.result_ui32;
    
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_GetNextRunningVrrpAssocIp
 *------------------------------------------------------------------------------
 * PURPOSE  : Get VRRP status and check whether it is default value.
 * INPUT    : ifindex,
 *            vrid,
 *            ip_addr  -- output buffer
 * OUTPUT   : ifindex  -- next ifindex
 *            vrid     -- next vrid
 *            ip_addr  -- output vrrp associated ip address
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- not same as default
 *          : SYS_TYPE_GET_RUNNING_CFG_FAIL      -- get failure
 *          : SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 * NOTES    : none
 *------------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T VRRP_PMGR_GetNextRunningVrrpAssocIp(UI32_T *ifindex, UI8_T *vrid, UI8_T *ip_addr)
{
    VRRP_MGR_IPCMsg_T *msg_p = NULL;
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp2))];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    
    msgbuf_p->cmd = SYS_MODULE_VRRP;
    msgbuf_p->msg_size = VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp2);
    msg_p = (VRRP_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VRRP_MGR_IPC_RUNNINGASSOCIPGETNEXT;
    
    memset(&(msg_p->data.arg_grp2), 0, sizeof(msg_p->data.arg_grp2));
    msg_p->data.arg_grp2.arg1 = *ifindex;
    msg_p->data.arg_grp2.arg2 = *vrid;
    memcpy(msg_p->data.arg_grp2.arg3, ip_addr, sizeof(msg_p->data.arg_grp2.arg3));
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp2), msgbuf_p) != SYSFUN_OK)
    {
        return VRRP_TYPE_RESULT_FAIL;
    }
    
    *ifindex = msg_p->data.arg_grp2.arg1;
    *vrid = msg_p->data.arg_grp2.arg2;
    memcpy(ip_addr, msg_p->data.arg_grp2.arg3, sizeof(msg_p->data.arg_grp2.arg3));
    
    return msg_p->type.result_ui32;
    
}

#if (SYS_CPNT_VRRP_PING == TRUE)
/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_GetRunningPingStatus
 *------------------------------------------------------------------------------
 * PURPOSE  : Get VRRP ping status and check whether it is default value.
 * INPUT    : ping_status	--	ping enable status
 * OUTPUT   : ping_status	--	ping enable status
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- not same as default
 *          : SYS_TYPE_GET_RUNNING_CFG_FAIL      -- get failure
 *          : SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 * NOTES    : none
 *------------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T VRRP_PMGR_GetRunningPingStatus(UI32_T *ping_status)
{
    VRRP_MGR_IPCMsg_T *msg_p = NULL;
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp8))];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    
    if (NULL == ping_status)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
        
    msgbuf_p->cmd = SYS_MODULE_VRRP;
    msgbuf_p->msg_size = VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp8);
    msg_p = (VRRP_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VRRP_MGR_IPC_RUNNINGPINGSTATUSGET;
    
    memset(&(msg_p->data.arg_grp8), 0, sizeof(msg_p->data.arg_grp8));
    msg_p->data.arg_grp8.arg1 = *ping_status;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp8), msgbuf_p) != SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }
    
    *ping_status = msg_p->data.arg_grp8.arg1;
    
    return msg_p->type.result_ui32;
}
#endif


