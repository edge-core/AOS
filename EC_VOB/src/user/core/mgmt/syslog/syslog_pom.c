/*-----------------------------------------------------------------------------
 * FILE NAME: SYSLOG_POM.C
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *    This file implements the APIs for SYSLOG OM IPC.
 *
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    2007/07/30     --- Rich, Create
 *
 * Copyright(C)      Accton Corporation, 2007
 *-----------------------------------------------------------------------------
 */

/* INCLUDE FILE DECLARATIONS
 */

#include <stdio.h>
#include <string.h>
#include "sys_bld.h"
#include "sys_module.h"
#include "sys_type.h"
#include "l_mm.h"
#include "sysfun.h"
#include "syslog_om.h"
#include "syslog_pom.h"
#include "syslog_type.h"


/* NAMING CONSTANT DECLARATIONS
 */

/* trace id definition when using L_MM
 */
/*
enum
{
    SYSLOG_POM_TRACEID_GET_FACILITY_TYPE,
    SYSLOG_POM_TRACEID_GET_REMOTE_LOG_LEVEL,
    SYSLOG_POM_TRACEID_GET_REMOTE_LOG_STATUS,
    SYSLOG_POM_TRACEID_GET_SERVER_IPADDR,
    SYSLOG_POM_TRACEID_GET_SYSLOG_STATUS,
    SYSLOG_POM_TRACEID_GET_UC_LOG_LEVEL,
};
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
 * ROUTINE NAME : SYSLOG_POM_InitiateProcessResource
 *-----------------------------------------------------------------------------
 * PURPOSE : Initiate resource for SYSLOG_POM in the calling process.
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
BOOL_T SYSLOG_POM_InitiateProcessResource(void)
{
    /* Given that CSCA is run in L2_L4_PROC
     */
    if (SYSFUN_GetMsgQ(SYS_BLD_CORE_UTIL_PROC_OM_IPCMSGQ_KEY,
            SYSFUN_MSGQ_BIDIRECTIONAL, &ipcmsgq_handle) != SYSFUN_OK)
    {
        printf("%s(): SYSFUN_GetMsgQ fail.\n", __FUNCTION__);
        return FALSE;
    }

    return TRUE;
} /* End of VLAN_POM_InitiateProcessResource */

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYSLOG_POM_GetRemoteLogStatus
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call SYSLOG_OM_GetRemoteLogStatus through the IPC msgq.
 * INPUT:
 *    None
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    port
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
UI32_T SYSLOG_POM_GetRemoteLogStatus(UI32_T *status)
{
    const UI32_T msg_size = SYSLOG_OM_GET_MSGBUFSIZE(syslog_remote_config);
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    SYSLOG_OM_IPCMsg_T *msg_p;
    UI32_T result;

    msgbuf_p->cmd = SYS_MODULE_SYSLOG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (SYSLOG_OM_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = SYSLOG_OM_IPC_GET_REMOTE_LOG_STATUS;


    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return SYSLOG_REMOTE_INVALID_BUFFER;
    }

    *status = msg_p->data.syslog_remote_config.status;
    result = msg_p->type.ret_ui32;


    return result;
} /* End of SYSLOG_POM_GetRemoteLogStatus */

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYSLOG_POM_GetRemoteLogServerIPAddr
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call SYSLOG_OM_GetRemoteLogServerIPAddr through the IPC msgq.
 * INPUT:
 *    None
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    port
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
UI32_T SYSLOG_POM_GetRemoteLogServerIPAddr(UI8_T index, L_INET_AddrIp_T *ip_address)
{
    const UI32_T msg_size = SYSLOG_OM_GET_MSGBUFSIZE(syslog_remote_config);
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    SYSLOG_OM_IPCMsg_T *msg_p;
    UI32_T result;


    msgbuf_p->cmd = SYS_MODULE_SYSLOG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (SYSLOG_OM_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = SYSLOG_OM_IPC_GET_SERVER_IPADDR;
    msg_p->data.syslog_remote_config.server_index = (UI32_T)index;


    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {

        return SYSLOG_REMOTE_INVALID_BUFFER;
    }

    memcpy(ip_address, &msg_p->data.syslog_remote_config.server_config.ipaddr, sizeof(L_INET_AddrIp_T));
    result = msg_p->type.ret_ui32;


    return result;
} /* End of SYSLOG_POM_GetRemoteLogServerIPAddr */

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYSLOG_POM_GetSyslogStatus
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call SYSLOG_OM_GetSyslogStatus through the IPC msgq.
 * INPUT:
 *    None
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    port
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T SYSLOG_POM_GetSyslogStatus(UI32_T *syslog_status)
{
    const UI32_T msg_size = SYSLOG_OM_GET_MSGBUFSIZE(syslog_om_data);
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    SYSLOG_OM_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_SYSLOG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (SYSLOG_OM_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = SYSLOG_OM_IPC_GET_SYSLOG_STATUS;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }
    *syslog_status = msg_p->data.syslog_om_data.syslog_status;

    return TRUE;
} /* End of SYSLOG_POM_GetSyslogStatus */

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYSLOG_POM_GetUcLogLevel
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This api will call SYSLOG_OM_GetUcLogLevel through the IPC msgq.
 * INPUT:
 *    None
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    port
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T SYSLOG_POM_GetUcLogLevel(UI32_T *uc_log_level)
{
    const UI32_T msg_size = SYSLOG_OM_GET_MSGBUFSIZE(syslog_om_data);
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    SYSLOG_OM_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_SYSLOG;
    msgbuf_p->msg_size = msg_size;
    msg_p = (SYSLOG_OM_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = SYSLOG_OM_IPC_GET_UC_LOG_LEVEL;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0/*need not to send event*/,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {

        return FALSE;
    }
    *uc_log_level = msg_p->data.syslog_om_data.uc_log_level;

    return TRUE;
} /* End of SYSLOG_POM_GetSyslogStatus */
