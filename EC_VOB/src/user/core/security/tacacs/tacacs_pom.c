/* MODULE NAME:  tacacs_pom.c
 * PURPOSE:
 * This is a sample code for implementation of POM for OM in kernel space.
 *
 * NOTES:
 *
 * REASON:
 * Description:
 * HISTORY
 *    5/3/2007 - Eli Lin, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_module.h"
#include "sysfun.h"
#include "tacacs_om_private.h"
#include "tacacs_om.h"
#include "l_mm.h"
#include <string.h>
#include "sys_bld.h"

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
 * ROUTINE NAME : CLUSTER_POM_Init
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
void TACACS_POM_Init(void)
{
    if(SYSFUN_GetMsgQ(SYS_BLD_AUTH_PROTOCOL_PROC_OM_IPCMSGQ_KEY,
        SYSFUN_MSGQ_BIDIRECTIONAL, &ipcmsgq_handle)!=SYSFUN_OK)
    {
        printf("%s(): SYSFUN_GetMsgQ fail.\n", __FUNCTION__);
    }
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - TACACS_POM_GetRunningServerPort
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default TACACS server port is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: serverport
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T TACACS_POM_GetRunningServerPort(UI32_T *serverport)
{
    const UI32_T msg_buf_size=(sizeof(((TACACS_OM_IPCMsg_T *)0)->data.ui32_v)
        +sizeof(((TACACS_OM_IPCMsg_T *)0)->type));
    SYSFUN_Msg_T *msg_p;
    TACACS_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_TACACS;
    msg_p->msg_size = msg_buf_size;

    msg_data_p=(TACACS_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = TACACS_OM_IPCCMD_GET_RUNNING_SERVER_PORT;


    /*assign input
     */

    /*send ipc
     */
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        0,msg_buf_size,msg_p)!=SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    /*assign output
     */
    *serverport=msg_data_p->data.ui32_v;

    /*return result
     */
    return msg_data_p->type.result_ui32;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_POM_Get_Server_Port
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the UDP port number of the remote TACACS server
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   Prot number
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
UI32_T TACACS_POM_Get_Server_Port(void)
{
    const UI32_T msg_buf_size=sizeof(((TACACS_OM_IPCMsg_T *)0)->type);
    SYSFUN_Msg_T *msg_p;
    TACACS_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_TACACS;
    msg_p->msg_size = msg_buf_size;

    msg_data_p=(TACACS_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = TACACS_OM_IPCCMD_GET_SERVER_PORT;


    /*assign input
     */

    /*send ipc
     */
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        0,msg_buf_size,msg_p)!=SYSFUN_OK)
    {
        return 0;
    }

    /*assign output
     */

    /*return result
     */
    return msg_data_p->type.result_ui32;
}
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - TACACS_POM_GetRunningServerSecret
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default TACACS server secret is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: serversecret[MAX_SECRET_LENGTH + 1]
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T  TACACS_POM_GetRunningServerSecret(UI8_T serversecret[])
{
    const UI32_T msg_buf_size=(sizeof(((TACACS_OM_IPCMsg_T *)0)->data.serversecret)
        +sizeof(((TACACS_OM_IPCMsg_T *)0)->type));
    SYSFUN_Msg_T *msg_p;
    TACACS_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_TACACS;
    msg_p->msg_size = msg_buf_size;

    msg_data_p=(TACACS_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = TACACS_OM_IPCCMD_GET_RUNNING_SERVER_SECRET;

    /*assign input
     */

    /*send ipc
     */
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        0,msg_buf_size,msg_p)!=SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    /*assign output
     */
     memcpy(serversecret,msg_data_p->data.serversecret,sizeof(msg_data_p->data.serversecret));

    /*return result
     */
    return msg_data_p->type.result_ui32;
}
/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_POM_Get_Server_Secret
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the shared secret text string used between
 *           the TACACS client and server.
 * INPUT:    secret.
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
BOOL_T TACACS_POM_Get_Server_Secret(UI8_T* secret)
{
    const UI32_T msg_buf_size=(sizeof(((TACACS_OM_IPCMsg_T *)0)->data.serversecret)
        +sizeof(((TACACS_OM_IPCMsg_T *)0)->type));
    SYSFUN_Msg_T *msg_p;
    TACACS_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_TACACS;
    msg_p->msg_size = msg_buf_size;

    msg_data_p=(TACACS_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = TACACS_OM_IPCCMD_GET_SERVER_SECRET;


    /*assign input
     */

    /*send ipc
     */
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        0,msg_buf_size,msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    /*assign output
     */
    memcpy(secret,msg_data_p->data.serversecret,sizeof(msg_data_p->data.serversecret));

    /*return result
     */
    return msg_data_p->type.result_bool;
}
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - TACACS_POM_GetRunningServerIP
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default TACACS server IP is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: serverip
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T TACACS_POM_GetRunningServerIP(UI32_T *serverip)
{
    const UI32_T msg_buf_size=(sizeof(((TACACS_OM_IPCMsg_T *)0)->data.ip4_v)
        +sizeof(((TACACS_OM_IPCMsg_T *)0)->type));
    SYSFUN_Msg_T *msg_p;
    TACACS_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_TACACS;
    msg_p->msg_size = msg_buf_size;

    msg_data_p=(TACACS_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = TACACS_OM_IPCCMD_GET_RUNNING_SERVER_IP;

    /*assign input
     */

    /*send ipc
     */
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        0,msg_buf_size,msg_p)!=SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    /*assign output
     */
    memcpy(serverip,msg_data_p->data.ip4_v,sizeof(msg_data_p->data.ip4_v));

    /*return result
     */
    return msg_data_p->type.result_ui32;
}
/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_POM_Get_Server_IP
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the IP address of the remote TACACS server
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   TACACS server IP address
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
UI32_T  TACACS_POM_Get_Server_IP(void)
{
    const UI32_T msg_buf_size=(sizeof(((TACACS_OM_IPCMsg_T *)0)->data.ip4_v)
        +sizeof(((TACACS_OM_IPCMsg_T *)0)->type));
    SYSFUN_Msg_T *msg_p;
    TACACS_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_TACACS;
    msg_p->msg_size = msg_buf_size;

    msg_data_p=(TACACS_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = TACACS_OM_IPCCMD_GET_SERVER_IP;


    /*assign input
     */

    /*send ipc
     */
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        0,msg_buf_size,msg_p)!=SYSFUN_OK)
    {
        return 0;
    }

    /*assign output
     */

    /*return result
     */
    return msg_data_p->type.result_ui32;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_POM_GetRunningServerRetransmit
 *---------------------------------------------------------------------------
 * PURPOSE:  This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *           the non-default TACACS retransmit is successfully retrieved.
 *           Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT:    None.
 * OUTPUT:   retransmit.
 * RETURN:   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
             SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
             SYS_TYPE_GET_RUNNING_CFG_FAIL.
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */

UI32_T TACACS_POM_GetRunningServerRetransmit(UI32_T *retransmit)
{
    const UI32_T msg_buf_size=(sizeof(((TACACS_OM_IPCMsg_T *)0)->data.ui32_v)
                              +sizeof(((TACACS_OM_IPCMsg_T *)0)->type));
    SYSFUN_Msg_T *msg_p;
    TACACS_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_TACACS;
    msg_p->msg_size = msg_buf_size;

    msg_data_p=(TACACS_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = TACACS_OM_IPCCMD_GET_RUNNING_SERVER_RETRANSMIT;


    /*assign input
     */

    /*send ipc
     */
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        0,msg_buf_size,msg_p)!=SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    /*assign output
     */
    *retransmit = msg_data_p->data.ui32_v;

    /*return result
     */
    return msg_data_p->type.result_ui32;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_POM_GetServerRetransmit
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the retransmit times of the remote TACACS server
 * INPUT:    None.
 * OUTPUT:   retransmit.
 * RETURN:   TRUE/FALSE
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
UI32_T TACACS_POM_GetServerRetransmit(void)
{
    const UI32_T msg_buf_size=(sizeof(((TACACS_OM_IPCMsg_T *)0)->data.ui32_v)
                              +sizeof(((TACACS_OM_IPCMsg_T *)0)->type));
    SYSFUN_Msg_T *msg_p;
    TACACS_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_TACACS;
    msg_p->msg_size = msg_buf_size;

    msg_data_p=(TACACS_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = TACACS_OM_IPCCMD_GET_SERVER_RETRANSMIT;


    /*assign input
     */

    /*send ipc
     */
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        0,msg_buf_size,msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    /*assign output
     */

    /*return result
     */
    return msg_data_p->type.result_ui32;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_POM_GetRunningServerTimeout
 *---------------------------------------------------------------------------
 * PURPOSE:  This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *           the non-default TACACS timeout is successfully retrieved.
 *           Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT:    None.
 * OUTPUT:   timeout.
 * RETURN:   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
             SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
             SYS_TYPE_GET_RUNNING_CFG_FAIL.
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */

UI32_T TACACS_POM_GetRunningServerTimeout(UI32_T *timeout)
{
    const UI32_T msg_buf_size=(sizeof(((TACACS_OM_IPCMsg_T *)0)->data.ui32_v)
                              +sizeof(((TACACS_OM_IPCMsg_T *)0)->type));
    SYSFUN_Msg_T *msg_p;
    TACACS_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_TACACS;
    msg_p->msg_size = msg_buf_size;

    msg_data_p=(TACACS_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = TACACS_OM_IPCCMD_GET_RUNNING_SERVER_TIMEOUT;


    /*assign input
     */

    /*send ipc
     */
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        0,msg_buf_size,msg_p)!=SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    /*assign output
     */
    *timeout = msg_data_p->data.ui32_v;

    /*return result
     */
    return msg_data_p->type.result_ui32;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_POM_GetServerTimeout
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the IP address of the remote TACACS server
 * INPUT:    None.
 * OUTPUT:   timeout.
 * RETURN:   TRUE/FALSE
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
UI32_T  TACACS_POM_GetServerTimeout(void)
{
    const UI32_T msg_buf_size=(sizeof(((TACACS_OM_IPCMsg_T *)0)->data.ui32_v)
                              +sizeof(((TACACS_OM_IPCMsg_T *)0)->type));
    SYSFUN_Msg_T *msg_p;
    TACACS_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_TACACS;
    msg_p->msg_size = msg_buf_size;

    msg_data_p=(TACACS_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = TACACS_OM_IPCCMD_GET_SERVER_TIMEOUT;


    /*assign input
     */

    /*send ipc
     */
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        0,msg_buf_size,msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    /*assign output
     */

    /*return result
     */
    return msg_data_p->type.result_ui32;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_POM_GetNext_Server_Host
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will copy server_host setting from server entry
 * INPUT:    server_index
 * OUTPUT:   next server_index (current index + 1), server_host
 * RETURN:   TRUE -- succeeded, FALSE -- Failed
 * NOTE:     index RANGE (0..SYS_ADPT_MAX_NBR_OF_TACACS_SERVERS - 1)
 *---------------------------------------------------------------------------
 */
BOOL_T TACACS_POM_GetNext_Server_Host(UI32_T *index,TACACS_Server_Host_T *server_host)
{
    const UI32_T msg_buf_size=(sizeof(((TACACS_OM_IPCMsg_T *)0)->data.index_serverhost)
        +sizeof(((TACACS_OM_IPCMsg_T *)0)->type));
    SYSFUN_Msg_T *msg_p;
    TACACS_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_TACACS;
    msg_p->msg_size = msg_buf_size;

    msg_data_p=(TACACS_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = TACACS_OM_IPCCMD_GET_NEXT_SERVER_HOST;


    /*assign input
     */
    msg_data_p->data.index_serverhost.index=*index;

    /*send ipc
     */
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        0,msg_buf_size,msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    /*assign output
     */
    *index=msg_data_p->data.index_serverhost.index;
    *server_host=msg_data_p->data.index_serverhost.server_host;

    /*return result
     */
    return msg_data_p->type.result_bool;
}
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - TACACS_POM_GetNextRunning_Server_Host
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default TACACS server IP is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: serverip
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T TACACS_POM_GetNextRunning_Server_Host(UI32_T *index,TACACS_Server_Host_T *server_host)
{
    const UI32_T msg_buf_size=(sizeof(((TACACS_OM_IPCMsg_T *)0)->data.index_serverhost)
        +sizeof(((TACACS_OM_IPCMsg_T *)0)->type));
    SYSFUN_Msg_T *msg_p;
    TACACS_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_TACACS;
    msg_p->msg_size = msg_buf_size;

    msg_data_p=(TACACS_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = TACACS_OM_IPCCMD_GET_NEXT_RUNNING_SERVER_HOST;


    /*assign input
     */
    msg_data_p->data.index_serverhost.index=*index;

    /*send ipc
     */
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        0,msg_buf_size,msg_p)!=SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    /*assign output
     */
    *index=msg_data_p->data.index_serverhost.index;
    *server_host=msg_data_p->data.index_serverhost.server_host;

    /*return result
     */
    return msg_data_p->type.result_ui32;
}
/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_POM_Get_Server_Host
 *---------------------------------------------------------------------------
 * PURPOSE  : This function will copy server_host setting from server entry
 * INPUT    : server_index (1-based)
 * OUTPUT   : server_host
 * RETURN   : TRUE -- succeeded, FALSE -- Failed
 * NOTE     : index RANGE (1..SYS_ADPT_MAX_NBR_OF_TACACS_SERVERS)
 *            fail (1). if out of range (2). used_flag == false
 *---------------------------------------------------------------------------*/
BOOL_T TACACS_POM_Get_Server_Host(UI32_T server_index, TACACS_Server_Host_T *server_host)
{
    const UI32_T msg_buf_size=(sizeof(((TACACS_OM_IPCMsg_T *)0)->data.index_serverhost)
        +sizeof(((TACACS_OM_IPCMsg_T *)0)->type));
    SYSFUN_Msg_T *msg_p;
    TACACS_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_TACACS;
    msg_p->msg_size = msg_buf_size;

    msg_data_p=(TACACS_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = TACACS_OM_IPCCMD_GET_SERVER_HOST;


    /*assign input
     */
    msg_data_p->data.index_serverhost.index=server_index;

    /*send ipc
     */
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        0,msg_buf_size,msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    /*assign output
     */
    *server_host=msg_data_p->data.index_serverhost.server_host;

    /*return result
     */
    return msg_data_p->type.result_bool;
}
/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_POM_GetServerHostMaxRetransmissionTimeout
 *-------------------------------------------------------------------------
 * PURPOSE  : query the max of retransmission timeout of server hosts
 * INPUT    : none
 * OUTPUT   : max_retransmission_timeout
 * RETURN   : TRUE - succeeded; FALSE - failed
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_POM_GetServerHostMaxRetransmissionTimeout(UI32_T *max_retransmission_timeout)
{
    const UI32_T msg_buf_size=(sizeof(((TACACS_OM_IPCMsg_T *)0)->data.ui32_v)
        +sizeof(((TACACS_OM_IPCMsg_T *)0)->type));
    SYSFUN_Msg_T *msg_p;
    TACACS_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_TACACS;
    msg_p->msg_size = msg_buf_size;

    msg_data_p=(TACACS_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = TACACS_OM_IPCCMD_GETSERVERHOSTMAXRETRANSMISSIONTIMEOUT;


    /*assign input
     */

    /*send ipc
     */
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        0,msg_buf_size,msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    /*assign output
     */
    *max_retransmission_timeout=msg_data_p->data.ui32_v;

    /*return result
     */
    return msg_data_p->type.result_bool;
}
/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_POM_IsServerHostValid
 *-------------------------------------------------------------------------
 * PURPOSE  : query whether this server host is valid or not
 * INPUT    : server_index (1-based)
 * OUTPUT   : none
 * RETURN   : TRUE - valid; FALSE - invalid
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_POM_IsServerHostValid(UI32_T server_index)
{
    const UI32_T msg_buf_size=(sizeof(((TACACS_OM_IPCMsg_T *)0)->data.ui32_v)
        +sizeof(((TACACS_OM_IPCMsg_T *)0)->type));
    SYSFUN_Msg_T *msg_p;
    TACACS_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_TACACS;
    msg_p->msg_size = msg_buf_size;

    msg_data_p=(TACACS_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = TACACS_OM_IPCCMD_ISSERVERHOSTVALID;


    /*assign input
     */
    msg_data_p->data.ui32_v=server_index;

    /*send ipc
     */
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        0,msg_buf_size,msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    /*assign output
     */

    /*return result
     */
    return msg_data_p->type.result_bool;
}
/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_POM_LookupServerIndexByIpAddress
 *-------------------------------------------------------------------------
 * PURPOSE  : lookup server host by ip_address
 * INPUT    : ip_address
 * OUTPUT   : server_index (1-based)
 * RETURN   : TRUE - found; FALSE - not found
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_POM_LookupServerIndexByIpAddress(UI32_T ip_address, UI32_T *server_index)
{
    const UI32_T msg_buf_size=(sizeof(((TACACS_OM_IPCMsg_T *)0)->data.ip_serverindex)
        +sizeof(((TACACS_OM_IPCMsg_T *)0)->type));
    SYSFUN_Msg_T *msg_p;
    TACACS_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_TACACS;
    msg_p->msg_size = msg_buf_size;

    msg_data_p=(TACACS_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = TACACS_OM_IPCCMD_LOOKUPSERVERINDEXBYIPADDRESS;


    /*assign input
     */
    msg_data_p->data.ip_serverindex.ip_address=ip_address;

    /*send ipc
     */
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        0,msg_buf_size,msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    /*assign output
     */
    *server_index=msg_data_p->data.ip_serverindex.server_index;

    /*return result
     */
    return msg_data_p->type.result_bool;
}
/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_POM_GetRunningServerAcctPort
 *-------------------------------------------------------------------------
 * PURPOSE  : This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *            the non-default TACACS accounting port is successfully retrieved.
 *            Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT    : none
 * OUTPUT   : acct_port
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    : 1. This function shall only be invoked by CLI to save the
 *               "running configuration" to local or remote files.
 *            2. Since only non-default configuration will be saved, this
 *               function shall return non-default value.
 *-------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T TACACS_POM_GetRunningServerAcctPort(UI32_T *acct_port)
{
    const UI32_T msg_buf_size=(sizeof(((TACACS_OM_IPCMsg_T *)0)->data.ui32_v)
        +sizeof(((TACACS_OM_IPCMsg_T *)0)->type));
    SYSFUN_Msg_T *msg_p;
    TACACS_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_TACACS;
    msg_p->msg_size = msg_buf_size;

    msg_data_p=(TACACS_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = TACACS_OM_IPCCMD_GETRUNNINGSERVERACCTPORT;


    /*assign input
     */


    /*send ipc
     */
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        0,msg_buf_size,msg_p)!=SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    /*assign output
     */
    *acct_port=msg_data_p->data.ui32_v;

    /*return result
     */
    return msg_data_p->type.result_ui32;
}
#if (SYS_CPNT_TACACS_PLUS_ACCOUNTING == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_POM_GetAccUserEntryQtyFilterByName
 *-------------------------------------------------------------------------
 * PURPOSE  : get the number of user with specified name
 * INPUT    : name
 * OUTPUT   : qty.
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : exclude "dead" user which connect_status ==
 *            AAA_ACC_CNET_DORMANT, AAA_ACC_CNET_IDLE, AAA_ACC_CNET_FAILED
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_POM_GetAccUserEntryQtyFilterByName(UI8_T *name, UI32_T *qty)
{
    const UI32_T msg_buf_size=(sizeof(((TACACS_OM_IPCMsg_T *)0)->data.name_qty)
        +sizeof(((TACACS_OM_IPCMsg_T *)0)->type));
    SYSFUN_Msg_T *msg_p;
    TACACS_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_TACACS;
    msg_p->msg_size = msg_buf_size;

    msg_data_p=(TACACS_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = TACACS_OM_IPCCMD_GETACCUSERENTRYQTYFILTERBYNAME;


    /*assign input
     */
    memcpy(msg_data_p->data.name_qty.name,name,sizeof(msg_data_p->data.name_qty.name));

    /*send ipc
     */
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        0,msg_buf_size,msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    /*assign output
     */
    *qty=msg_data_p->data.name_qty.qty;

    /*return result
     */
    return msg_data_p->type.result_bool;
}
/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_POM_GetAccUserEntryQtyFilterByType
 *-------------------------------------------------------------------------
 * PURPOSE  : get the number of user with specified client_type
 * INPUT    : client_type
 * OUTPUT   : qty.
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : exclude "dead" user which connect_status ==
 *            AAA_ACC_CNET_DORMANT, AAA_ACC_CNET_IDLE, AAA_ACC_CNET_FAILED
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_POM_GetAccUserEntryQtyFilterByType(AAA_ClientType_T client_type, UI32_T *qty)
{
    const UI32_T msg_buf_size=(sizeof(((TACACS_OM_IPCMsg_T *)0)->data.type_qty)
        +sizeof(((TACACS_OM_IPCMsg_T *)0)->type));
    SYSFUN_Msg_T *msg_p;
    TACACS_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_TACACS;
    msg_p->msg_size = msg_buf_size;

    msg_data_p=(TACACS_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = TACACS_OM_IPCCMD_GETACCUSERENTRYQTYFILTERBYTYPE;


    /*assign input
     */
    msg_data_p->data.type_qty.client_type=client_type;

    /*send ipc
     */
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        0,msg_buf_size,msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    /*assign output
     */
    *qty=msg_data_p->data.type_qty.qty;

    /*return result
     */
    return msg_data_p->type.result_bool;
}
/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_POM_GetAccUserEntryQtyFilterByNameAndType
 *-------------------------------------------------------------------------
 * PURPOSE  : get the number of user with specified name and client_type
 * INPUT    : name, client_type
 * OUTPUT   : qty.
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : exclude "dead" user which connect_status ==
 *            AAA_ACC_CNET_DORMANT, AAA_ACC_CNET_IDLE, AAA_ACC_CNET_FAILED
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_POM_GetAccUserEntryQtyFilterByNameAndType(UI8_T *name, AAA_ClientType_T client_type, UI32_T *qty)
{
    const UI32_T msg_buf_size=(sizeof(((TACACS_OM_IPCMsg_T *)0)->data.name_type_qty)
        +sizeof(((TACACS_OM_IPCMsg_T *)0)->type));
    SYSFUN_Msg_T *msg_p;
    TACACS_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_TACACS;
    msg_p->msg_size = msg_buf_size;

    msg_data_p=(TACACS_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = TACACS_OM_IPCCMD_GETACCUSERENTRYQTYFILTERBYNAMEANDTYPE;


    /*assign input
     */
    memcpy(msg_data_p->data.name_type_qty.name,name,sizeof(msg_data_p->data.name_type_qty.name));
    msg_data_p->data.name_type_qty.client_type=client_type;

    /*send ipc
     */
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        0,msg_buf_size,msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    /*assign output
     */
    *qty=msg_data_p->data.name_type_qty.qty;
    /*return result
     */
    return msg_data_p->type.result_bool;
}
/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_POM_GetNextAccUserEntryFilterByType
 *-------------------------------------------------------------------------
 * PURPOSE  : get the next user with specified client_type by index.
 * INPUT    : entry->user_index, entry->client_type
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : exclude "dead" user which connect_status ==
 *            AAA_ACC_CNET_DORMANT, AAA_ACC_CNET_IDLE, AAA_ACC_CNET_FAILED
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_POM_GetNextAccUserEntryFilterByType(TPACC_UserInfoInterface_T *entry)
{
    const UI32_T msg_buf_size=(sizeof(((TACACS_OM_IPCMsg_T *)0)->data.userinfointerface)
        +sizeof(((TACACS_OM_IPCMsg_T *)0)->type));
    SYSFUN_Msg_T *msg_p;
    TACACS_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_TACACS;
    msg_p->msg_size = msg_buf_size;

    msg_data_p=(TACACS_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = TACACS_OM_IPCCMD_GETNEXTACCUSERENTRYFILTERBYTYPE;


    /*assign input
     */
    msg_data_p->data.userinfointerface=*entry;

    /*send ipc
     */
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        0,msg_buf_size,msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    /*assign output
     */
    *entry=msg_data_p->data.userinfointerface;

    /*return result
     */
    return msg_data_p->type.result_bool;
}
/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_POM_GetAccUserRunningInfo
 *-------------------------------------------------------------------------
 * PURPOSE  : get running_info by ifindex, user name, client type
 * INPUT    : ifindex, name, client_type
 * OUTPUT   : running_info
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : suppose (ifindex + name + client_type) is unique
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_POM_GetAccUserRunningInfo(UI32_T ifindex, const UI8_T *name, AAA_ClientType_T client_type, AAA_AccUserRunningInfo_T *running_info)
{
    const UI32_T msg_buf_size=(sizeof(((TACACS_OM_IPCMsg_T *)0)->data.accuserrunninginfo)
        +sizeof(((TACACS_OM_IPCMsg_T *)0)->type));
    SYSFUN_Msg_T *msg_p;
    TACACS_OM_IPCMsg_T *msg_data_p;
    UI8_T space_msg[SYSFUN_SIZE_OF_MSG(msg_buf_size)];

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_TACACS;
    msg_p->msg_size = msg_buf_size;

    msg_data_p=(TACACS_OM_IPCMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = TACACS_OM_IPCCMD_GETACCUSERRUNNINGINFO;


    /*assign input
     */
    msg_data_p->data.accuserrunninginfo.ifindex=ifindex;
    memcpy(msg_data_p->data.accuserrunninginfo.name,
        name,sizeof(msg_data_p->data.accuserrunninginfo.name));
    msg_data_p->data.accuserrunninginfo.client_type=client_type;

    /*send ipc
     */
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        0,msg_buf_size,msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    /*assign output
     */
    *running_info=msg_data_p->data.accuserrunninginfo.running_info;

    /*return result
     */
    return msg_data_p->type.result_bool;
}
#endif /*#if (SYS_CPNT_TACACS_PLUS_ACCOUNTING == TRUE)*/

