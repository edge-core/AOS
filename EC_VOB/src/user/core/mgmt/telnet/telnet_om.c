/* MODULE NAME:  telnet_om.c
 * PURPOSE:
 * OM for telnet.
 *
 * NOTES:
 *
 * REASON:
 * Description:
 * HISTORY
 *    06/03/2007 - Rich Lee, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */
 
/* INCLUDE FILE DECLARATIONS
 */
#include "sys_bld.h"
#include "sys_module.h"
#include "telnet_om.h"
#include "telnetd.h"


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
static TELNET_State_T telnet_status = SYS_DFLT_TELNET_DEFAULT_STATE;
static UI32_T telnet_port = SYS_DFLT_TELNET_SOCKET_PORT;
static UI32_T telnet_max_session = SYS_DFLT_TELNET_DEFAULT_MAX_SESSION;

/* sema for om protect
 */
static UI32_T    telnet_om_semid;

/* EXPORTED SUBPROGRAM BODIES
 */

/*------------------------------------------------------------------------------
 * ROUTINE NAME : TELNET_OM_InitateProcessResource
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Initiate resource used in this process.
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE   -- Success
 *    FALSE  -- Fail
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T TELNET_OM_InitateProcessResource(void)
{
    if(SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_TELNET_OM, &telnet_om_semid)!=SYSFUN_OK)
    {
        TELNETD_ErrorPrintf("%s:get om sem id fail.\n", __FUNCTION__);
        return FALSE;
    }
    return TRUE;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : TELNET_OM_GetTnpdPort
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will get telnet port from OM.
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    *port_p -- tcp port for telnet.
 *
 * RETURN:
 *    TRUE/FALSE
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T TELNET_OM_GetTnpdPort(UI32_T *port_p)
{
    UI32_T    orig_priority;

    if(port_p == NULL)
        return FALSE;
             
    orig_priority=TELNET_OM_EnterCriticalSection(telnet_om_semid);
    *port_p = telnet_port;
    TELNET_OM_LeaveCriticalSection(telnet_om_semid,orig_priority);
    return TRUE;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : TELNET_OM_SetTnpdPort
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will set telnet port to OM.
 * INPUT: 
 *    port -- telnet port
 *
 * OUTPUT:
 *    None
 *
 * RETURN:
 *    TRUE
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T TELNET_OM_SetTnpdPort(UI32_T port)
{
    UI32_T    orig_priority;

    /* BODY */
    orig_priority=TELNET_OM_EnterCriticalSection(telnet_om_semid);
    telnet_port = port;
    TELNET_OM_LeaveCriticalSection(telnet_om_semid,orig_priority);
    
    return TRUE;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : TELNET_OM_GetTnpdStatus
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will get telnet status from OM.
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    *state_p -- TELNET_STATE_ENABLED / TELNET_STATE_DISABLED
 *
 * RETURN:
 *    TRUE/FALSE
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T TELNET_OM_GetTnpdStatus(TELNET_State_T *state_p)
{
    UI32_T    orig_priority;

    if(state_p == NULL)
        return FALSE;

    orig_priority=TELNET_OM_EnterCriticalSection(telnet_om_semid);
    *state_p = telnet_status;
    TELNET_OM_LeaveCriticalSection(telnet_om_semid,orig_priority);
    return TRUE;
}   

/*------------------------------------------------------------------------------
 * ROUTINE NAME : TELNET_OM_SetTnpdMaxSession
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will set telnet max session to OM.
 *
 * INPUT: 
 *    maxSession -- max session number
 *
 * OUTPUT:
 *    None
 *
 * RETURN:
 *    TRUE
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T TELNET_OM_SetTnpdMaxSession(UI32_T maxSession)
{
    UI32_T    orig_priority;

    /* BODY */
    if (maxSession > SYS_DFLT_TELNET_DEFAULT_MAX_SESSION)
    {
        return FALSE;
    }

    orig_priority=TELNET_OM_EnterCriticalSection(telnet_om_semid);
    telnet_max_session = maxSession;
    TELNET_OM_LeaveCriticalSection(telnet_om_semid,orig_priority);
    
    return TRUE;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : TELNET_OM_GetTnpdMaxSession
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will get telnet max session from OM.
 *
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    max session number
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
UI32_T TELNET_OM_GetTnpdMaxSession()
{
    UI32_T    orig_priority;
    UI32_T    maxSession;

    orig_priority = TELNET_OM_EnterCriticalSection(telnet_om_semid);
    maxSession = telnet_max_session;
    TELNET_OM_LeaveCriticalSection(telnet_om_semid,orig_priority);

    return maxSession;
}   

/*------------------------------------------------------------------------------
 * ROUTINE NAME : TELNET_OM_SetTnpdStatus
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will set telnet status to OM.
 * INPUT:
 *    state.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T TELNET_OM_SetTnpdStatus(TELNET_State_T state)
{
    UI32_T    orig_priority;

    orig_priority=TELNET_OM_EnterCriticalSection(telnet_om_semid);
    telnet_status = state;
    TELNET_OM_LeaveCriticalSection(telnet_om_semid,orig_priority);    

    return TRUE;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : TELNET_OM_HandleIPCReqMsg
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Handle the ipc request message for TELNET om.
 * INPUT:
 *    ipcmsg_p  --  input request ipc message buffer
 *
 * OUTPUT:
 *    ipcmsg_p  --  output response ipc message buffer
 *
 * RETURN:
 *    TRUE  --  There is a response need to send.
 *    FALSE --  No response need to send.
 *
 * NOTES:
 *    1.The size of ipcmsg_p.msg_buf must be large enough to carry any response
 *      messages.
 *------------------------------------------------------------------------------
 */
BOOL_T TELNET_OM_HandleIPCReqMsg(SYSFUN_Msg_T* ipcmsg_p)
{
    TELNET_OM_IPCMsg_T *om_msg_p;
    BOOL_T           need_resp;

    if(ipcmsg_p==NULL)
        return FALSE;

    om_msg_p= (TELNET_OM_IPCMsg_T*)ipcmsg_p->msg_buf;

    switch(om_msg_p->type.cmd)
    {
        case TELNET_OM_IPC_GET_TNPD_PORT:
            om_msg_p->type.result = TELNET_OM_GetTnpdPort(&(om_msg_p->data.port_data.port));  
            ipcmsg_p->msg_size = TELNET_OM_GET_MSGBUFSIZE(port_data);
            need_resp=TRUE;
            break;
            
        case TELNET_OM_IPC_GET_TNPD_STATUS:
            om_msg_p->type.result = TELNET_OM_GetTnpdStatus(&(om_msg_p->data.status_data.state));
            ipcmsg_p->msg_size = TELNET_OM_GET_MSGBUFSIZE(status_data);
            need_resp=TRUE;
            break;
            
        default:
            TELNETD_ErrorPrintf("%s(): Invalid cmd.\n", __FUNCTION__);
            /* Unknow command. There is no way to idntify whether this
             * ipc message need or not need a response. If we response to
             * a asynchronous msg, then all following synchronous msg will
             * get wrong responses and that might not easy to debug.
             * If we do not response to a synchronous msg, the requester
             * will be blocked forever. It should be easy to debug that
             * error.
             */
            need_resp=FALSE;
    }
    return need_resp;
}

/* LOCAL SUBPROGRAM BODIES
 */

