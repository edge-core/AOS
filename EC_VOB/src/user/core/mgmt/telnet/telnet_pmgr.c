/* MODULE NAME:  telnet_pmgr.c
 * PURPOSE:
 *    PMGR implement for telnet.
 *
 * NOTES:
 *
 * REASON:
 * Description:
 * HISTORY
 *    06/02/2007 - Rich Lee, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */
 
/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>

#include "sys_bld.h"
#include "sys_module.h"
#include "l_mm.h"
#include "sysfun.h"

#include "telnet_pmgr.h"
#include "telnet_mgr.h"
#include "telnetd.h"

/* NAMING CONSTANT DECLARATIONS
 */
/* trace id definition when using L_MM
 */
enum
{
    TELNET_PMGR_TRACEID_IPC_SET_TNPD_PORT,
    TELNET_PMGR_TRACEID_IPC_SET_TNPD_STATUS,
};

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */
static SYSFUN_MsgQ_T telnet_group_ipcmsgq_handle;

/* EXPORTED SUBPROGRAM BODIES
 */
/*------------------------------------------------------------------------------
 * ROUTINE NAME : TELNET_PMGR_InitiateProcessResource
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Initiate resource used in the calling process, means the process that use
 *    this pmgr functions should call this init.
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE  --  Sucess
 *    FALSE --  Error
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T TELNET_PMGR_InitiateProcessResource(void)
{
    if(SYSFUN_GetMsgQ(SYS_BLD_TELNET_GROUP_IPCMSGQ_KEY, SYSFUN_MSGQ_BIDIRECTIONAL, &telnet_group_ipcmsgq_handle)!=SYSFUN_OK)
    {
        TELNETD_ErrorPrintf("%s(): SYSFUN_GetMsgQ fail.\n", __FUNCTION__);
        return FALSE;
    }

    return TRUE;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : TELNET_PMGR_SetTnpdPort
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call TELNET_MGR_SetTnpdPort through the IPC msgq.
 *
 * INPUT:
 *    port  -- tcp port for telnet
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE  -- Success
 *    FALSE -- Fail
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T TELNET_PMGR_SetTnpdPort(UI32_T port)
{
    const UI32_T          msg_size = TELNET_MGR_GET_MSGBUFSIZE(port_data);    
    UI8_T                 ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T         *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;    
    TELNET_MGR_IPCMsg_T  *data_p;

    /* msgbuf_p will be used on both request and response
     * msg_size shall be max(req_buf_size, resp_buf_size)
     */
    msgbuf_p->cmd = SYS_MODULE_TELNET;

    data_p = (TELNET_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = TELNET_MGR_IPC_SET_TNPD_PORT;
    data_p->data.port_data.port = port;    

    msgbuf_p->msg_size = msg_size;

    if(SYSFUN_SendRequestMsg(telnet_group_ipcmsgq_handle,
                             msgbuf_p,
                             SYSFUN_TIMEOUT_WAIT_FOREVER,
                             SYSFUN_SYSTEM_EVENT_IPCMSG,
                             TELNET_MGR_MSGBUF_TYPE_SIZE,
                             msgbuf_p)!=SYSFUN_OK)
    {
        TELNETD_ErrorPrintf("%s(): SYSFUN_SendRequestMsg() return fail\r\n", __FUNCTION__);
        return FALSE;
    }

    return data_p->type.result;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : TELNET_PMGR_GetTnpdPort
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call TELNET_MGR_GetTnpdPort through the IPC msgq.
 *
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    pPort -- tcp port for telnet
 *
 * RETURN:
 *    TRUE  -- Success
 *    FALSE -- Fail
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T TELNET_PMGR_GetTnpdPort(UI32_T *pPort)
{
    const UI32_T          msg_size = TELNET_MGR_GET_MSGBUFSIZE(port_data);    
    UI8_T                 ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T         *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;    
    TELNET_MGR_IPCMsg_T  *data_p;

    /* msgbuf_p will be used on both request and response
     * msg_size shall be max(req_buf_size, resp_buf_size)
     */
    msgbuf_p->cmd = SYS_MODULE_TELNET;
    msgbuf_p->msg_size = msg_size;

    data_p = (TELNET_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = TELNET_MGR_IPC_GET_TNPD_PORT;

    if(SYSFUN_SendRequestMsg(telnet_group_ipcmsgq_handle,
                             msgbuf_p,
                             SYSFUN_TIMEOUT_WAIT_FOREVER,
                             SYSFUN_SYSTEM_EVENT_IPCMSG,
                             msg_size,
                             msgbuf_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    *pPort = data_p->data.port_data.port;

    return data_p->type.result;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : TELNET_PMGR_SetTnpdStatus
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call TELNET_MGR_SetTnpdStatus through the IPC msgq.
 *
 * INPUT:
 *    state
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE   -- Success
 *    FALSE -- Fail
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T TELNET_PMGR_SetTnpdStatus(TELNET_State_T state)
{
    const UI32_T msg_size = TELNET_MGR_GET_MSGBUFSIZE(status_data);    
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;    
    TELNET_MGR_IPCMsg_T  *data_p;

    /* msgbuf_p will be used on both request and response
     * msg_size shall be max(req_buf_size, resp_buf_size)
     */
    msgbuf_p->cmd = SYS_MODULE_TELNET;
    msgbuf_p->msg_size = msg_size;

    data_p = (TELNET_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = TELNET_MGR_IPC_SET_TNPD_STATUS;
    data_p->data.status_data.state = state;

    if(SYSFUN_SendRequestMsg(telnet_group_ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, TELNET_MGR_MSGBUF_TYPE_SIZE, msgbuf_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    return data_p->type.result;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : TELNET_PMGR_GetTnpdStatus
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call TELNET_MGR_GetTnpdStatus through the IPC msgq.
 *
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    pState -- telnet server status
 *
 * RETURN:
 *    TRUE  -- Success
 *    FALSE -- Fail
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T TELNET_PMGR_GetTnpdStatus(TELNET_State_T *pState)
{
    const UI32_T          msg_size = TELNET_MGR_GET_MSGBUFSIZE(status_data);    
    UI8_T                 ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T         *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;    
    TELNET_MGR_IPCMsg_T  *data_p;

    /* msgbuf_p will be used on both request and response
     * msg_size shall be max(req_buf_size, resp_buf_size)
     */
    msgbuf_p->cmd = SYS_MODULE_TELNET;
    msgbuf_p->msg_size = msg_size;

    data_p = (TELNET_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = TELNET_MGR_IPC_GET_TNPD_STATUS;

    if(SYSFUN_SendRequestMsg(telnet_group_ipcmsgq_handle,
                             msgbuf_p,
                             SYSFUN_TIMEOUT_WAIT_FOREVER,
                             SYSFUN_SYSTEM_EVENT_IPCMSG,
                             msg_size,
                             msgbuf_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    *pState = data_p->data.status_data.state;

    return data_p->type.result;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : TELNET_PMGR_SetTnpdMaxSession
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call TELNET_MGR_SetTnpdMaxSessions through the IPC msgq.
 *
 * INPUT:
 *    maxSessions -- max number of telnet sessions
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE  -- Success
 *    FALSE -- Fail
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T TELNET_PMGR_SetTnpdMaxSession(UI32_T maxSession)
{
    const UI32_T          msg_size = TELNET_MGR_GET_MSGBUFSIZE(max_session_data);    
    UI8_T                 ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T         *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;    
    TELNET_MGR_IPCMsg_T  *data_p;

    /* msgbuf_p will be used on both request and response
     * msg_size shall be max(req_buf_size, resp_buf_size)
     */
    msgbuf_p->cmd = SYS_MODULE_TELNET;
    msgbuf_p->msg_size = msg_size;

    data_p = (TELNET_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = TELNET_MGR_IPC_SET_TNPD_MAX_SESSION;
    data_p->data.max_session_data.maxSession = maxSession;


    if(SYSFUN_SendRequestMsg(telnet_group_ipcmsgq_handle,
                             msgbuf_p,
                             SYSFUN_TIMEOUT_WAIT_FOREVER,
                             SYSFUN_SYSTEM_EVENT_IPCMSG,
                             msg_size,
                             msgbuf_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    return data_p->type.result;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : TELNET_PMGR_GetTnpdMaxSession
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call TELNET_MGR_GetTnpdStatus through the IPC msgq.
 *
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    pState -- telnet server status
 *
 * RETURN:
 *    TRUE  -- Success
 *    FALSE -- Fail
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T TELNET_PMGR_GetTnpdMaxSession(UI32_T *pMaxSession)
{
    const UI32_T          msg_size = TELNET_MGR_GET_MSGBUFSIZE(max_session_data);    
    UI8_T                 ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T         *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;    
    TELNET_MGR_IPCMsg_T  *data_p;

    /* msgbuf_p will be used on both request and response
     * msg_size shall be max(req_buf_size, resp_buf_size)
     */
    msgbuf_p->cmd = SYS_MODULE_TELNET;
    msgbuf_p->msg_size = msg_size;

    data_p = (TELNET_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = TELNET_MGR_IPC_GET_TNPD_MAX_SESSION;

    if(SYSFUN_SendRequestMsg(telnet_group_ipcmsgq_handle,
                             msgbuf_p,
                             SYSFUN_TIMEOUT_WAIT_FOREVER,
                             SYSFUN_SYSTEM_EVENT_IPCMSG,
                             msg_size,
                             msgbuf_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    *pMaxSession = data_p->data.max_session_data.maxSession;

    return data_p->type.result;
}

/* Add telnet client, Jinnee.Jin, 2008, 3, 5 */
/* ------------------------------------------------------------------------
 * FUNCTION NAME - CLI_PMGR_CurrentTelnetSession
 * ------------------------------------------------------------------------
 * PURPOSE  : This function will get current telnet seesion
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : BOOL_T : TRUE  -- success.
 *                     FLASE -- fail.
 * NOTE     : 
 * ------------------------------------------------------------------------
 */
BOOL_T TELNET_PMGR_CurrentTelnetSession(UI16_T remote_port, char *myopts, char *hisopts)
{
    const UI32_T msg_size = TELNET_MGR_GET_MSGBUFSIZE(options_data);
    UI8_T               ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T        *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    TELNET_MGR_IPCMsg_T *data_p;
    BOOL_T              ret;


    msgbuf_p->cmd = SYS_MODULE_TELNET;

    data_p = (TELNET_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = TELNET_MGR_IPC_GET_TNPD_OPTIONS;
    data_p->data.options_data.remote_port = remote_port;
    msgbuf_p->msg_size = msg_size;

    ret =SYSFUN_SendRequestMsg(telnet_group_ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, TELNET_MGR_MSGBUF_TYPE_SIZE, msgbuf_p);
    if(ret !=SYSFUN_OK)
    {
        return FALSE;
    }

    return (BOOL_T) data_p->type.result;
}

#if (SYS_CPNT_CLUSTER == TRUE)

/*------------------------------------------------------------------------------
 * ROUTINE NAME : TELNET_PMGR_SetTelnetRelaying
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call TELNET_MGR_SetTelnetRelaying through the IPC msgq.
 *
 * INPUT:
 *    UI32_T task_id, BOOL_T bRelaying, UI32_T memberId
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE   -- Success
 *    FALSE -- Fail
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T TELNET_PMGR_SetTelnetRelaying(UI32_T task_id, BOOL_T bRelaying, UI8_T memberId)
{
    const UI32_T msg_size = TELNET_MGR_GET_MSGBUFSIZE(cluster_relay_data);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    TELNET_MGR_IPCMsg_T  *data_p;

    msgbuf_p->cmd = SYS_MODULE_TELNET;
    msgbuf_p->msg_size = msg_size;

    data_p = (TELNET_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = TELNET_MGR_IPC_SET_TELNET_RELAYING;
    data_p->data.cluster_relay_data.task_id = task_id;
    data_p->data.cluster_relay_data.bRelaying = bRelaying;
    data_p->data.cluster_relay_data.memberId = memberId;

    if(SYSFUN_SendRequestMsg(telnet_group_ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, TELNET_MGR_MSGBUF_TYPE_SIZE, msgbuf_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    return data_p->type.result;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : TELNET_PMGR_ClusterToMemberFromUART
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call TELNET_MGR_ClusterToMemberFromUART through the IPC msgq.
 *
 * INPUT:
 *    UI8_T member_id --member id.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE   -- Success
 *    FALSE -- Fail
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T TELNET_PMGR_ClusterToMemberFromUART(UI8_T member_id)
{
    const UI32_T msg_size = TELNET_MGR_GET_MSGBUFSIZE(cluster_member_data);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    TELNET_MGR_IPCMsg_T  *data_p;

    /* msgbuf_p will be used on both request and response
     * msg_size shall be max(req_buf_size, resp_buf_size)
     */
    msgbuf_p->cmd = SYS_MODULE_TELNET;
    msgbuf_p->msg_size = msg_size;

    data_p = (TELNET_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = TELNET_MGR_IPC_CLUSTER_TO_MEMBER_FROM_UART;
    data_p->data.cluster_member_data.memberId = member_id;

    if(SYSFUN_SendRequestMsg(telnet_group_ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, TELNET_MGR_MSGBUF_TYPE_SIZE, msgbuf_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    return data_p->type.result;
}

#endif  /* #if (SYS_CPNT_CLUSTER == TRUE) */

/* LOCAL SUBPROGRAM BODIES
 */

