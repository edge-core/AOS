/* MODULE NAME:  tacacs_pmgr.c
 * PURPOSE:
 *    This is a sample code for implementation of MGR.
 *
 * NOTES:
 *
 * REASON:
 * Description:
 * HISTORY
 *    5/2/2007 - Eli Lin, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include <string.h>
#include "sysfun.h"
#include "sys_module.h"
#include "l_mm.h"
#include "sys_bld.h"
#include "tacacs_type.h"
#include "tacacs_om.h"
#include "tacacs_mgr.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */
static SYSFUN_MsgQ_T ipcmsgq_handle;

/* EXPORTED SUBPROGRAM BODIES
 */
/*------------------------------------------------------------------------------
 * ROUTINE NAME : TACACS_PMGR_Init
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Do initialization procedures for TACACS_PMGR.
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
void TACACS_PMGR_Init(void)
{
    /* Given that TACACS PMGR requests are handled in TACACSGROUP of L2L4_PROC
     */
    if(SYSFUN_GetMsgQ(SYS_BLD_AUTH_PROTOCOL_GROUP_IPCMSGQ_KEY,
        SYSFUN_MSGQ_BIDIRECTIONAL, &ipcmsgq_handle)!=SYSFUN_OK)
    {
        printf("%s(): SYSFUN_GetMsgQ fail.\n", __FUNCTION__);
    }
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_PMGR_Set_Server_Port
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set the TCP port number of the remote TACACS server
 * INPUT:    Prot number
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
BOOL_T TACACS_PMGR_Set_Server_Port(UI32_T serverport)
{
    const UI32_T msg_buf_size=(sizeof(((TACACS_MGR_IPCMsg_T *)0)->data.ui32_v)
        +sizeof(((TACACS_MGR_IPCMsg_T *)0)->type));
    SYSFUN_Msg_T *msg_p;
    TACACS_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_TACACS;
    msg_p->msg_size = msg_buf_size;

    msg_data_p=(TACACS_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = TACACS_MGR_IPCCMD_SET_SERVER_PORT;

    msg_data_p->data.ui32_v=serverport;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, TACACS_MGR_MSGBUF_TYPE_SIZE, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_data_p->type.result_bool;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_PMGR_SetServerRetransmit
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set the global retransmit of the remote TACACS server
 * INPUT:    retransmit
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
BOOL_T TACACS_PMGR_SetServerRetransmit(UI32_T retransmit)
{
    const UI32_T msg_buf_size=(sizeof(((TACACS_MGR_IPCMsg_T *)0)->data.ui32_v)
        +sizeof(((TACACS_MGR_IPCMsg_T *)0)->type));
    SYSFUN_Msg_T *msg_p;
    TACACS_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_TACACS;
    msg_p->msg_size = msg_buf_size;

    msg_data_p=(TACACS_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = TACACS_MGR_IPCCMD_SET_SERVER_RETRANSMIT;

    msg_data_p->data.ui32_v = retransmit;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, TACACS_MGR_MSGBUF_TYPE_SIZE, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_data_p->type.result_bool;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_PMGR_SetServerTimeout
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set the global timeout of the remote TACACS server
 * INPUT:    timeout
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
BOOL_T TACACS_PMGR_SetServerTimeout(UI32_T timeout)
{
    const UI32_T msg_buf_size=(sizeof(((TACACS_MGR_IPCMsg_T *)0)->data.ui32_v)
        +sizeof(((TACACS_MGR_IPCMsg_T *)0)->type));
    SYSFUN_Msg_T *msg_p;
    TACACS_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_TACACS;
    msg_p->msg_size = msg_buf_size;

    msg_data_p=(TACACS_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = TACACS_MGR_IPCCMD_SET_SERVER_TIMEOUT;

    msg_data_p->data.ui32_v = timeout;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, TACACS_MGR_MSGBUF_TYPE_SIZE, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_data_p->type.result_bool;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_PMGR_Set_Server_Secret
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set the shared secret text string used between
 *           the TACACS client and server.
 * INPUT:    secret text string
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
BOOL_T TACACS_PMGR_Set_Server_Secret(UI8_T *serversecret)
{
    const UI32_T msg_buf_size=(sizeof(((TACACS_MGR_IPCMsg_T *)0)->data.serversecret)
        +sizeof(((TACACS_MGR_IPCMsg_T *)0)->type));
    SYSFUN_Msg_T *msg_p;
    TACACS_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_TACACS;
    msg_p->msg_size = msg_buf_size;

    msg_data_p=(TACACS_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = TACACS_MGR_IPCCMD_SET_SERVER_SECRET;

    memcpy(msg_data_p->data.serversecret,serversecret,sizeof(msg_data_p->data.serversecret));

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, TACACS_MGR_MSGBUF_TYPE_SIZE, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_data_p->type.result_bool;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_PMGR_Set_Server_IP
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set the IP address of the remote TACACS server
 * INPUT:    TACACS server IP address
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
BOOL_T TACACS_PMGR_Set_Server_IP(UI32_T serverip)
{
    const UI32_T msg_buf_size=(sizeof(((TACACS_MGR_IPCMsg_T *)0)->data.ui32_v)
        +sizeof(((TACACS_MGR_IPCMsg_T *)0)->type));
    SYSFUN_Msg_T *msg_p;
    TACACS_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_TACACS;
    msg_p->msg_size = msg_buf_size;

    msg_data_p=(TACACS_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = TACACS_MGR_IPCCMD_SET_SERVER_IP;

    msg_data_p->data.ui32_v=serverip;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, TACACS_MGR_MSGBUF_TYPE_SIZE, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_data_p->type.result_bool;
}

#if (SYS_CPNT_TACACS_PLUS_AUTHENTICATION == TRUE)
/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_PMGR_AnsyncLoginAuth
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will call TACACS client to do authentication
 * INPUT:    username       - user name
 *           password       - password
 *           sess_type      - session type
 *           sess_id        - session ID
 *           rem_ip_addr    - remote IP address
 *           cookie         - caller cookie
 *           cookie_size    - size of cookie
 * OUTPUT:   None
 * RETURN:   TRUE - request is send successfully; FALSE - IPC error
 * NOTE:     None
 *---------------------------------------------------------------------------
 */
 BOOL_T TACACS_PMGR_AnsyncLoginAuth(
    const char *username,
    const char *password,
    TACACS_SessType_T sess_type,
    UI32_T sess_id,
    L_INET_AddrIp_T *rem_ip_addr,
    void *cookie,
    UI32_T cookie_size)
{
    const UI32_T msg_buf_size=(sizeof(((TACACS_MGR_IPCMsg_T *)0)->data.async_author_request)
        +sizeof(((TACACS_MGR_IPCMsg_T *)0)->type));
    SYSFUN_Msg_T *msg_p;
    TACACS_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_TACACS;
    msg_p->msg_size = msg_buf_size;

    msg_data_p=(TACACS_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = TACACS_MGR_IPCCMD_ASYNC_LOGIN_AUTH;

    memcpy(msg_data_p->data.async_author_request.user_name,
           username,
           sizeof(msg_data_p->data.async_author_request.user_name));
    memcpy(msg_data_p->data.async_author_request.password,
        password,
        sizeof(msg_data_p->data.async_author_request.password));
    msg_data_p->data.async_author_request.sess_type = sess_type;
    msg_data_p->data.async_author_request.sess_id = sess_id;

    if (rem_ip_addr)
    {
        msg_data_p->data.async_author_request.rem_ip_addr = *rem_ip_addr;
    }

    if (sizeof(msg_data_p->data.async_author_request.cookie) < cookie_size)
    {
        printf("%s: TACACS_AsyncAuthorRequest_T.cookie too small\r\n",
            __FUNCTION__);
        return FALSE;
    }

    memcpy(msg_data_p->data.async_author_request.cookie, cookie, cookie_size);
    msg_data_p->data.async_author_request.cookie_size = cookie_size;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, 0, NULL) != SYSFUN_OK)
    {
        return FALSE;
    }

    return TRUE;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_PMGR_AnsyncEnablePasswordAuth
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will call TACACS client to do enable authentication
 * INPUT:    username       - user name
 *           password       - password
 *           sess_type      - session type
 *           sess_id        - session ID
 *           rem_ip_addr    - remote IP address
 *           cookie         - caller cookie
 *           cookie_size    - size of cookie
 * OUTPUT:   None
 * RETURN:   TRUE - request is send successfully; FALSE - IPC error
 * NOTE:     None
 *---------------------------------------------------------------------------
 */
BOOL_T TACACS_PMGR_AnsyncEnablePasswordAuth(
    const char *username,
    const char *password,
    TACACS_SessType_T sess_type,
    UI32_T sess_id,
    L_INET_AddrIp_T *rem_ip_addr,
    void *cookie,
    UI32_T cookie_size)
{
    const UI32_T msg_buf_size=(sizeof(((TACACS_MGR_IPCMsg_T *)0)->data.async_author_request)
        +sizeof(((TACACS_MGR_IPCMsg_T *)0)->type));
    SYSFUN_Msg_T *msg_p;
    TACACS_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_TACACS;
    msg_p->msg_size = msg_buf_size;

    msg_data_p=(TACACS_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = TACACS_MGR_IPCCMD_ASYNC_AUTHEN_ENABLE;

    memcpy(msg_data_p->data.async_author_request.user_name,
           username,
           sizeof(msg_data_p->data.async_author_request.user_name));
    memcpy(msg_data_p->data.async_author_request.password,
        password,
        sizeof(msg_data_p->data.async_author_request.password));
    msg_data_p->data.async_author_request.sess_type = sess_type;
    msg_data_p->data.async_author_request.sess_id = sess_id;

    if (rem_ip_addr)
    {
        msg_data_p->data.async_author_request.rem_ip_addr = *rem_ip_addr;
    }

    if (sizeof(msg_data_p->data.async_author_request.cookie) < cookie_size)
    {
        printf("%s: TACACS_AsyncAuthorRequest_T.cookie too small\r\n",
            __FUNCTION__);
        return FALSE;
    }

    memcpy(msg_data_p->data.async_author_request.cookie, cookie, cookie_size);
    msg_data_p->data.async_author_request.cookie_size = cookie_size;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, 0, NULL) != SYSFUN_OK)
    {
        return FALSE;
    }

    return TRUE;
}

#endif /* #if (SYS_CPNT_TACACS_PLUS_AUTHENTICATION == TRUE) */

/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_PMGR_Set_Server_Host
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set the TACACS server host
 * INPUT:    server_index (1-based), server_host
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE
 * NOTE:    server_ip;
 *		    server_port (1-65535)  - set 0 will use the global TACACS configuration
 *       	timeout     (1-65535)  - set 0 will use the global TACACS configuration
 *       	retransmit  (1-65535)  - set 0 will use the global TACACS configuration
 *        	secret      (length < TACACS_MAX_SECRET_LENGTH)  - set NULL will use the global TACACS configuration
 *---------------------------------------------------------------------------
 */
BOOL_T TACACS_PMGR_Set_Server_Host(UI32_T server_index,TACACS_Server_Host_T *server_host)
{
    const UI32_T msg_buf_size=(sizeof(((TACACS_MGR_IPCMsg_T *)0)->data.index_serverhost)
        +sizeof(((TACACS_MGR_IPCMsg_T *)0)->type));
    SYSFUN_Msg_T *msg_p;
    TACACS_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_TACACS;
    msg_p->msg_size = msg_buf_size;

    msg_data_p=(TACACS_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = TACACS_MGR_IPCCMD_SET_SERVER_HOST;

    msg_data_p->data.index_serverhost.index=server_index;
    msg_data_p->data.index_serverhost.server_host=*server_host;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, TACACS_MGR_MSGBUF_TYPE_SIZE, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_data_p->type.result_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_PMGR_SetServerHostByIpAddress
 *-------------------------------------------------------------------------
 * PURPOSE  : setup server host by server_ip
 * INPUT    : server_ip, server_port, timeout, retransmit, secret, acct_port
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : if server_port, timeout, retransmit, acct_port == 0 then use global value
 *            if strlen(secret) == 0 then use global value
 *            if specified ip doesn't exist, then create it. or modify it
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_PMGR_SetServerHostByIpAddress(TACACS_Server_Host_T *server_host)
{
    const UI32_T msg_buf_size=(sizeof(((TACACS_MGR_IPCMsg_T *)0)->data.server_host)
        +sizeof(((TACACS_MGR_IPCMsg_T *)0)->type));
    SYSFUN_Msg_T *msg_p;
    TACACS_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_TACACS;
    msg_p->msg_size = msg_buf_size;

    msg_data_p=(TACACS_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = TACACS_MGR_IPCCMD_SETSERVERHOSTBYIPADDRESS;

    msg_data_p->data.server_host=*server_host;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, TACACS_MGR_MSGBUF_TYPE_SIZE, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_data_p->type.result_bool;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_PMGR_Destroy_Server_Host_By_Index
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will destroy the TACACS server host
 * INPUT:    server_index (1-based)
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
BOOL_T TACACS_PMGR_Destroy_Server_Host_By_Index(UI32_T server_index)
{
    const UI32_T msg_buf_size=(sizeof(((TACACS_MGR_IPCMsg_T *)0)->data.ui32_v)
        +sizeof(((TACACS_MGR_IPCMsg_T *)0)->type));
    SYSFUN_Msg_T *msg_p;
    TACACS_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_TACACS;
    msg_p->msg_size = msg_buf_size;

    msg_data_p=(TACACS_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = TACACS_MGR_IPCCMD_DESTROY_SERVER_HOST_BY_INDEX;

    msg_data_p->data.ui32_v=server_index;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, TACACS_MGR_MSGBUF_TYPE_SIZE, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_data_p->type.result_bool;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_PMGR_Destroy_Server_Host_By_Ip_Address
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will destroy the TACACS server host
 * INPUT:    server_ip
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
BOOL_T TACACS_PMGR_Destroy_Server_Host_By_Ip_Address(UI32_T server_ip)
{
    const UI32_T msg_buf_size=(sizeof(((TACACS_MGR_IPCMsg_T *)0)->data.ui32_v)
        +sizeof(((TACACS_MGR_IPCMsg_T *)0)->type));
    SYSFUN_Msg_T *msg_p;
    TACACS_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_TACACS;
    msg_p->msg_size = msg_buf_size;

    msg_data_p=(TACACS_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = TACACS_MGR_IPCCMD_DESTROY_SERVER_HOST_BY_IP_ADDRESS;

    msg_data_p->data.ui32_v=server_ip;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, TACACS_MGR_MSGBUF_TYPE_SIZE, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_data_p->type.result_bool;
}

#if (SYS_CPNT_AUTHORIZATION == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_PMGR_Author_Check
 *-------------------------------------------------------------------------
 * PURPOSE  : do authorization function
 * INPUT    : *request -- input of authorization request
 * OUTPUT   : *reply -- output of authorization request
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------
 */
BOOL_T TACACS_PMGR_Author_Check(TACACS_AuthorRequest_T *request,TACACS_AuthorReply_T *reply)
{
    const UI32_T msg_buf_size=(sizeof(((TACACS_MGR_IPCMsg_T *)0)->data.request_reply)
        +sizeof(((TACACS_MGR_IPCMsg_T *)0)->type));
    SYSFUN_Msg_T *msg_p;
    TACACS_MGR_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_TACACS;
    msg_p->msg_size = msg_buf_size;

    msg_data_p=(TACACS_MGR_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = TACACS_MGR_IPCCMD_AUTHOR_CHECK;

    msg_data_p->data.request_reply.request=*request;
    msg_data_p->data.request_reply.reply=*reply;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_buf_size, msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_data_p->type.result_bool;
}
#endif /*#if (SYS_CPNT_AUTHORIZATION == TRUE)*/

