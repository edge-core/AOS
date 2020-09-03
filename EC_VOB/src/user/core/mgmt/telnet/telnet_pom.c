/* MODULE NAME:  TELNET_pom.c
 * PURPOSE:
 * POM for telnet.
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
#include "sys_module.h"
#include "l_mm.h"
#include "sys_bld.h"

#include "telnet_pom.h"
#include "telnet_om.h"
#include "telnetd.h"

/* NAMING CONSTANT DECLARATIONS
 */
/* trace id definition when using L_MM
 */
enum
{
    TELNET_POM_TRACEID_IPC_GET_TNPD_PORT,
    TELNET_POM_TRACEID_IPC_GET_TNPD_STATUS,
};

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
 * ROUTINE NAME : TELNET_POM_InitiateProcessResource
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Initiate resource for TELNET_POM in the calling process.
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
BOOL_T TELNET_POM_InitiateProcessResource(void)
{
    /* Given that TELNET is run in CLI_PROC
     */
    if(SYSFUN_GetMsgQ(SYS_BLD_CLI_PROC_OM_IPCMSGQ_KEY,SYSFUN_MSGQ_BIDIRECTIONAL, &ipcmsgq_handle)!=SYSFUN_OK)
    {
        TELNETD_ErrorPrintf("%s(): SYSFUN_GetMsgQ fail.\n", __FUNCTION__);
        return FALSE;
    }

    return TRUE;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : TELNET_POM_GetTnpdPort
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will get telnet port from OM.
 * INPUT: 
 *    None.
 *
 * OUTPUT:
 *    *port_p --   port 
 *
 * RETURN:
 *    TRUE   -- Success
 *    FALSE -- Fail
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T TELNET_POM_GetTnpdPort(UI32_T *port_p)
{
    const UI32_T msg_size = TELNET_OM_GET_MSGBUFSIZE(port_data);    
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;    
    TELNET_OM_IPCMsg_T   *data_p;

    if(port_p==NULL)
        return FALSE;

    /* msgbuf_p will be used on both request and response
     * msg_size shall be max(req_buf_size, resp_buf_size)
     */
    

    msgbuf_p->cmd = SYS_MODULE_TELNET;
    msgbuf_p->msg_size = msg_size;

    data_p = (TELNET_OM_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = TELNET_OM_IPC_GET_TNPD_PORT;


    if(SYSFUN_SendRequestMsg(ipcmsgq_handle,
                             msgbuf_p,
                             SYSFUN_TIMEOUT_WAIT_FOREVER,
                             0/*need not to send event*/,
                             TELNET_OM_GET_MSGBUFSIZE(port_data),
                             msgbuf_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    *port_p = data_p->data.port_data.port;
    return data_p->type.result;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : TELNET_POM_GetTnpdStatus
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will get telnet status from OM.
 * INPUT: 
 *    None.
 *
 * OUTPUT:
 *    *status_p -- status
 *
 * RETURN:
 *    TRUE   -- Success
 *    FALSE  -- Fail
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T TELNET_POM_GetTnpdStatus(TELNET_State_T *status_p)
{
    const UI32_T msg_size = TELNET_OM_GET_MSGBUFSIZE(status_data);    
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    TELNET_OM_IPCMsg_T   *data_p;
    BOOL_T              result;

    /* msgbuf_p will be used on both request and response
     * msg_size shall be max(req_buf_size, resp_buf_size)
     */
   
    msgbuf_p->cmd = SYS_MODULE_TELNET;

    data_p = (TELNET_OM_IPCMsg_T*)msgbuf_p->msg_buf;
    data_p->type.cmd = TELNET_OM_IPC_GET_TNPD_STATUS;    

    msgbuf_p->msg_size = msg_size;

    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        0/*need not to send event*/, TELNET_OM_GET_MSGBUFSIZE(status_data),
        msgbuf_p)!=SYSFUN_OK)
    {
        L_MM_Free(msgbuf_p);
        return FALSE;
    }

    *status_p = data_p->data.status_data.state;
    result = data_p->type.result;
    L_MM_Free(msgbuf_p);
    return result;

}
/* LOCAL SUBPROGRAM BODIES
 */

