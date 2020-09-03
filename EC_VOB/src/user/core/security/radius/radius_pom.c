/*-----------------------------------------------------------------------------
 * Module   : radius_pom.c
 *-----------------------------------------------------------------------------
 * PURPOSE  : Provide APIs to access RADIUS.
 *-----------------------------------------------------------------------------
 * NOTES    :
 *
 *-----------------------------------------------------------------------------
 * HISTORY  : 08/15/2007 - Wakka Tu, Create
 *
 *-----------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2007
 *-----------------------------------------------------------------------------
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <stdint.h>
#include <string.h>
#include "sys_type.h"
#include "sys_module.h"
#include "sysfun.h"
#include "radius_om.h"
#include "radius_pom.h"
#include "leaf_es3626a.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void RADIUS_POM_SendMsg(UI32_T cmd, SYSFUN_Msg_T *msg_p, UI32_T req_size, UI32_T res_size, UI32_T ret_val);

/* STATIC VARIABLE DECLARATIONS
 */
static SYSFUN_MsgQ_T ipcmsgq_handle;

static UI8_T serversecret[MAXSIZE_radiusServerGlobalKey + 1] = { 0 };

/* MACRO FUNCTIONS DECLARACTION
 */

/* EXPORTED SUBPROGRAM BODIES
 */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - RADIUS_POM_InitiateProcessResources
 *-------------------------------------------------------------------------
 * PURPOSE : Initiate resource used in the calling process.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE  - success
 *           FALSE - failure
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
BOOL_T RADIUS_POM_InitiateProcessResources(void)
{
    if (SYSFUN_GetMsgQ(RADIUS_OM_IPCMSG_KEY, SYSFUN_MSGQ_BIDIRECTIONAL, &ipcmsgq_handle) != SYSFUN_OK)
    {
        printf("\r\n%s(): SYSFUN_GetMsgQ fail.", __FUNCTION__);
        return FALSE;
    }

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_POM_GetRunningRequestTimeout
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default RADIUS request timeout is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: timeout
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T RADIUS_POM_GetRunningRequestTimeout(UI32_T *timeout)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(RADIUS_OM_GET_MSGBUFSIZE(RADIUS_OM_IPCMsg_RequestTimeout_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    RADIUS_OM_IPCMsg_RequestTimeout_T *data_p;

    if (timeout == NULL)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    data_p = RADIUS_OM_MSG_DATA(msg_p);

    RADIUS_POM_SendMsg(RADIUS_OM_IPC_CMD_GET_RUNNING_REQUEST_TIMEOUT,
                       msg_p,
                       RADIUS_OM_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                       RADIUS_OM_GET_MSGBUFSIZE(RADIUS_OM_IPCMsg_RequestTimeout_T),
                       SYS_TYPE_GET_RUNNING_CFG_FAIL);

    *timeout = data_p->timeout;

    return RADIUS_OM_MSG_RETVAL(msg_p);
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_POM_Get_Request_Timeout
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the number of times the RADIUS client
 *           waits for a reply from RADIUS server
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   timeout value.
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
UI32_T RADIUS_POM_Get_Request_Timeout(void)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(RADIUS_OM_GET_MSGBUFSIZE_FOR_EMPTY_DATA())];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;

    RADIUS_POM_SendMsg(RADIUS_OM_IPC_CMD_GET_REQUEST_TIMEOUT,
                       msg_p,
                       RADIUS_OM_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                       RADIUS_OM_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                       0);

    return RADIUS_OM_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_POM_GetRunningServerPort
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default RADIUS server port is successfully retrieved.
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
UI32_T RADIUS_POM_GetRunningServerPort(UI32_T *serverport)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(RADIUS_OM_GET_MSGBUFSIZE(RADIUS_OM_IPCMsg_ServerPort_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    RADIUS_OM_IPCMsg_ServerPort_T *data_p;

    if (serverport == NULL)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    data_p = RADIUS_OM_MSG_DATA(msg_p);

    RADIUS_POM_SendMsg(RADIUS_OM_IPC_CMD_GET_RUNNING_SERVER_PORT,
                       msg_p,
                       RADIUS_OM_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                       RADIUS_OM_GET_MSGBUFSIZE(RADIUS_OM_IPCMsg_ServerPort_T),
                       SYS_TYPE_GET_RUNNING_CFG_FAIL);

    *serverport = data_p->serverport;

    return RADIUS_OM_MSG_RETVAL(msg_p);
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_POM_Get_Server_Port
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the UDP port number of the remote RADIUS server
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   Prot number
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
UI32_T RADIUS_POM_Get_Server_Port(void)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(RADIUS_OM_GET_MSGBUFSIZE_FOR_EMPTY_DATA())];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;

    RADIUS_POM_SendMsg(RADIUS_OM_IPC_CMD_GET_SERVER_PORT,
                       msg_p,
                       RADIUS_OM_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                       RADIUS_OM_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                       0);

    return RADIUS_OM_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_POM_GetRunningServerSecret
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default RADIUS server secret is successfully retrieved.
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
UI32_T  RADIUS_POM_GetRunningServerSecret(UI8_T *serversecret)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(RADIUS_OM_GET_MSGBUFSIZE(RADIUS_OM_IPCMsg_ServerSecret_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    RADIUS_OM_IPCMsg_ServerSecret_T *data_p;

    if (serversecret == NULL)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    data_p = RADIUS_OM_MSG_DATA(msg_p);

    RADIUS_POM_SendMsg(RADIUS_OM_IPC_CMD_GET_RUNNING_SERVER_SECRET,
                       msg_p,
                       RADIUS_OM_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                       RADIUS_OM_GET_MSGBUFSIZE(RADIUS_OM_IPCMsg_ServerSecret_T),
                       SYS_TYPE_GET_RUNNING_CFG_FAIL);

    if (RADIUS_OM_MSG_RETVAL(msg_p) != SYS_TYPE_GET_RUNNING_CFG_FAIL)
        strncpy((char *)serversecret, (char *)data_p->serversecret, sizeof(data_p->serversecret));

    return RADIUS_OM_MSG_RETVAL(msg_p);
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_POM_Get_Server_Secret
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the shared secret text string used between
 *           the RADIUS client and server.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   secret text string pointer
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
UI8_T * RADIUS_POM_Get_Server_Secret(void)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(RADIUS_OM_GET_MSGBUFSIZE(RADIUS_OM_IPCMsg_ServerSecret_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    RADIUS_OM_IPCMsg_ServerSecret_T *data_p;

    data_p = RADIUS_OM_MSG_DATA(msg_p);

    RADIUS_POM_SendMsg(RADIUS_OM_IPC_CMD_GET_SERVER_SECRET,
                       msg_p,
                       RADIUS_OM_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                       RADIUS_OM_GET_MSGBUFSIZE(RADIUS_OM_IPCMsg_ServerSecret_T),
                       (uintptr_t)NULL);

    if (RADIUS_OM_MSG_RETVAL(msg_p))
        strncpy((char *)serversecret, (char *)data_p->serversecret, sizeof(data_p->serversecret));

    return serversecret;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_POM_GetRunningRetransmitTimes
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default RADIUS retransmit times is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: retimes
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T RADIUS_POM_GetRunningRetransmitTimes(UI32_T *retimes)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(RADIUS_OM_GET_MSGBUFSIZE(RADIUS_OM_IPCMsg_RetransmitTimes_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    RADIUS_OM_IPCMsg_RetransmitTimes_T *data_p;

    if (retimes == NULL)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    data_p = RADIUS_OM_MSG_DATA(msg_p);

    RADIUS_POM_SendMsg(RADIUS_OM_IPC_CMD_GET_RUNNING_RETRANSMIT_TIMES,
                       msg_p,
                       RADIUS_OM_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                       RADIUS_OM_GET_MSGBUFSIZE(RADIUS_OM_IPCMsg_RetransmitTimes_T),
                       SYS_TYPE_GET_RUNNING_CFG_FAIL);

    *retimes = data_p->retimes;

    return RADIUS_OM_MSG_RETVAL(msg_p);
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_POM_Get_Retransmit_Times
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the number of times the RADIUS client
 *           transmits each RADIUS request to the server before giving up
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   retransmit times
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
UI32_T RADIUS_POM_Get_Retransmit_Times(void)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(RADIUS_OM_GET_MSGBUFSIZE_FOR_EMPTY_DATA())];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;

    RADIUS_POM_SendMsg(RADIUS_OM_IPC_CMD_GET_RETRANSMIT_TIMES,
                       msg_p,
                       RADIUS_OM_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                       RADIUS_OM_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                       0);

    return RADIUS_OM_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_POM_GetRunningServerIP
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default RADIUS server IP is successfully retrieved.
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
UI32_T RADIUS_POM_GetRunningServerIP(UI32_T *serverip)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(RADIUS_OM_GET_MSGBUFSIZE(RADIUS_OM_IPCMsg_ServerIP_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    RADIUS_OM_IPCMsg_ServerIP_T *data_p;

    if (serverip == NULL)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    data_p = RADIUS_OM_MSG_DATA(msg_p);

    RADIUS_POM_SendMsg(RADIUS_OM_IPC_CMD_GET_RUNNING_SERVER_IP,
                       msg_p,
                       RADIUS_OM_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                       RADIUS_OM_GET_MSGBUFSIZE(RADIUS_OM_IPCMsg_ServerIP_T),
                       SYS_TYPE_GET_RUNNING_CFG_FAIL);

    *serverip = data_p->serverip;

    return RADIUS_OM_MSG_RETVAL(msg_p);
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_POM_Get_Server_IP
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the IP address of the remote RADIUS server
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   RADIUS server IP address
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
UI32_T  RADIUS_POM_Get_Server_IP(void)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(RADIUS_OM_GET_MSGBUFSIZE_FOR_EMPTY_DATA())];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;

    RADIUS_POM_SendMsg(RADIUS_OM_IPC_CMD_GET_SERVER_IP,
                       msg_p,
                       RADIUS_OM_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                       RADIUS_OM_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                       0);

    return RADIUS_OM_MSG_RETVAL(msg_p);
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_POM_Get_UnknowAddress_Packets
 *---------------------------------------------------------------------------
 * PURPOSE:  Get the number of RADIUS Access-Response packets
 *           received from unknown addresses.
 * INPUT:    None
 * OUTPUT:   None.
 * RETURN:   Number of RADIUS Access-Response packets received from unknown addresses
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
UI32_T  RADIUS_POM_Get_UnknowAddress_Packets(void)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(RADIUS_OM_GET_MSGBUFSIZE_FOR_EMPTY_DATA())];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;

    RADIUS_POM_SendMsg(RADIUS_OM_IPC_CMD_GET_UNKNOW_ADDRESS_PACKETS,
                       msg_p,
                       RADIUS_OM_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                       RADIUS_OM_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                       0);

    return RADIUS_OM_MSG_RETVAL(msg_p);
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_POM_Get_NAS_ID
 *---------------------------------------------------------------------------
 * PURPOSE:  Get he NAS-Identifier of the RADIUS authentication client.
 *           This is not necessarily the same as sysName in MIB II.
 * INPUT:    None
 * OUTPUT:   None.
 * RETURN:   NASID
 *           NASID = NULL  ---No NAS ID
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_POM_Get_NAS_ID(UI8_T *nas_id)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(RADIUS_OM_GET_MSGBUFSIZE(RADIUS_OM_IPCMsg_NAS_ID_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    RADIUS_OM_IPCMsg_NAS_ID_T *data_p;

    if (nas_id == NULL)
        return FALSE;

    data_p = RADIUS_OM_MSG_DATA(msg_p);

    RADIUS_POM_SendMsg(RADIUS_OM_IPC_CMD_GET_NAS_ID,
                       msg_p,
                       RADIUS_OM_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                       RADIUS_OM_GET_MSGBUFSIZE(RADIUS_OM_IPCMsg_NAS_ID_T),
                       (UI32_T)FALSE);

    if (RADIUS_OM_MSG_RETVAL(msg_p) == (UI32_T)TRUE)
        memcpy(nas_id, data_p->nas_id, sizeof(data_p->nas_id));

    return (BOOL_T)RADIUS_OM_MSG_RETVAL(msg_p);
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_POM_GetAuthServerTable
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will call RADIUS client MIB to get radiusAuthServerTable
 * INPUT:    adiusAuthServerIndex
 * OUTPUT:   AuthServerEntry pointer
 * RETURN:   Get table TRUE/FALSE
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_POM_GetAuthServerTable(UI32_T index,AuthServerEntry *ServerEntry)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(RADIUS_OM_GET_MSGBUFSIZE(RADIUS_OM_IPCMsg_AuthServerTable_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    RADIUS_OM_IPCMsg_AuthServerTable_T *data_p;

    if (ServerEntry == NULL)
        return FALSE;

    data_p = RADIUS_OM_MSG_DATA(msg_p);
    data_p->index = index;

    RADIUS_POM_SendMsg(RADIUS_OM_IPC_CMD_GET_AUTH_SERVER_TABLE,
                       msg_p,
                       RADIUS_OM_GET_MSGBUFSIZE(RADIUS_OM_IPCMsg_AuthServerTable_T),
                       RADIUS_OM_GET_MSGBUFSIZE(RADIUS_OM_IPCMsg_AuthServerTable_T),
                       (UI32_T)FALSE);

    memcpy(ServerEntry, &data_p->ServerEntry, sizeof(*ServerEntry));

    return (BOOL_T)RADIUS_OM_MSG_RETVAL(msg_p);
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_POM_GetNextAuthServerTable
 *---------------------------------------------------------------------------
 * PURPOSE:  This funtion returns true if the next available table entry info
 *           can be successfully retrieved. Otherwise, false is returned.
 * INPUT:    RadiusAuthServerIndex ,
 * OUTPUT:   NextAuthServerEntry pointer
 * RETURN:   Get next table TRUE/FALSE
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_POM_GetNextAuthServerTable(UI32_T *index,AuthServerEntry *ServerEntry)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(RADIUS_OM_GET_MSGBUFSIZE(RADIUS_OM_IPCMsg_AuthServerTable_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    RADIUS_OM_IPCMsg_AuthServerTable_T *data_p;

    if (ServerEntry == NULL)
        return FALSE;

    data_p = RADIUS_OM_MSG_DATA(msg_p);
    data_p->index = *index;

    RADIUS_POM_SendMsg(RADIUS_OM_IPC_CMD_GET_NEXT_AUTH_SERVER_TABLE,
                       msg_p,
                       RADIUS_OM_GET_MSGBUFSIZE(RADIUS_OM_IPCMsg_AuthServerTable_T),
                       RADIUS_OM_GET_MSGBUFSIZE(RADIUS_OM_IPCMsg_AuthServerTable_T),
                       (UI32_T)FALSE);

    *index = data_p->index;
    memcpy(ServerEntry, &data_p->ServerEntry, sizeof(*ServerEntry));

    return (BOOL_T)RADIUS_OM_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_POM_GetNext_Server_Host
 * ---------------------------------------------------------------------
 * PURPOSE: Get next RADIUS server ip.
 * INPUT:  index
 * OUTPUT: index
 *         server_host
 * RETURN: TRUE/FALSE
 * NOTES:
 * ---------------------------------------------------------------------
 */
BOOL_T RADIUS_POM_GetNext_Server_Host(UI32_T *index,RADIUS_Server_Host_T *server_host)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(RADIUS_OM_GET_MSGBUFSIZE(RADIUS_OM_IPCMsg_ServerHost_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    RADIUS_OM_IPCMsg_ServerHost_T *data_p;

    if (server_host == NULL)
        return FALSE;

    data_p = RADIUS_OM_MSG_DATA(msg_p);
    data_p->index = *index;

    RADIUS_POM_SendMsg(RADIUS_OM_IPC_CMD_GET_NEXT_SERVER_HOST,
                       msg_p,
                       RADIUS_OM_GET_MSGBUFSIZE(RADIUS_OM_IPCMsg_ServerHost_T),
                       RADIUS_OM_GET_MSGBUFSIZE(RADIUS_OM_IPCMsg_ServerHost_T),
                       (UI32_T)FALSE);

    *index = data_p->index;
    memcpy(server_host, &data_p->server_host, sizeof(*server_host));

    return (BOOL_T)RADIUS_OM_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_POM_GetNextRunning_Server_Host
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default RADIUS server IP is successfully retrieved.
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
UI32_T RADIUS_POM_GetNextRunning_Server_Host(UI32_T *index,RADIUS_Server_Host_T *server_host)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(RADIUS_OM_GET_MSGBUFSIZE(RADIUS_OM_IPCMsg_ServerHost_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    RADIUS_OM_IPCMsg_ServerHost_T *data_p;

    if (server_host == NULL)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    data_p = RADIUS_OM_MSG_DATA(msg_p);
    data_p->index = *index;

    RADIUS_POM_SendMsg(RADIUS_OM_IPC_CMD_GET_NEXT_RUNNING_SERVER_HOST,
                       msg_p,
                       RADIUS_OM_GET_MSGBUFSIZE(RADIUS_OM_IPCMsg_ServerHost_T),
                       RADIUS_OM_GET_MSGBUFSIZE(RADIUS_OM_IPCMsg_ServerHost_T),
                       SYS_TYPE_GET_RUNNING_CFG_FAIL);

    *index = data_p->index;
    memcpy(server_host, &data_p->server_host, sizeof(*server_host));

    return RADIUS_OM_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_POM_Get_Server_Host
 * ---------------------------------------------------------------------
 * PURPOSE: Get RADIUS server ip.
 * INPUT:  index
 * OUTPUT: server_host
 * RETURN: TRUE/FALSE
 * NOTES:
 * ---------------------------------------------------------------------
 */
BOOL_T RADIUS_POM_Get_Server_Host(UI32_T index,RADIUS_Server_Host_T *server_host)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(RADIUS_OM_GET_MSGBUFSIZE(RADIUS_OM_IPCMsg_ServerHost_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    RADIUS_OM_IPCMsg_ServerHost_T *data_p;

    if (server_host == NULL)
        return FALSE;

    data_p = RADIUS_OM_MSG_DATA(msg_p);
    data_p->index = index;

    RADIUS_POM_SendMsg(RADIUS_OM_IPC_CMD_GET_SERVER_HOST,
                       msg_p,
                       RADIUS_OM_GET_MSGBUFSIZE(RADIUS_OM_IPCMsg_ServerHost_T),
                       RADIUS_OM_GET_MSGBUFSIZE(RADIUS_OM_IPCMsg_ServerHost_T),
                       (UI32_T)FALSE);

    memcpy(server_host, &data_p->server_host, sizeof(*server_host));

    return (BOOL_T)RADIUS_OM_MSG_RETVAL(msg_p);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_POM_GetRunningServerAcctPort
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default RADIUS server port is successfully retrieved.
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
UI32_T RADIUS_POM_GetRunningServerAcctPort(UI32_T *acct_port)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(RADIUS_OM_GET_MSGBUFSIZE(RADIUS_OM_IPCMsg_ServerAcctPort_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    RADIUS_OM_IPCMsg_ServerAcctPort_T *data_p;

    if (acct_port == NULL)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    data_p = RADIUS_OM_MSG_DATA(msg_p);

    RADIUS_POM_SendMsg(RADIUS_OM_IPC_CMD_GET_RUNNING_SERVER_ACCT_PORT,
                       msg_p,
                       RADIUS_OM_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                       RADIUS_OM_GET_MSGBUFSIZE(RADIUS_OM_IPCMsg_ServerAcctPort_T),
                       SYS_TYPE_GET_RUNNING_CFG_FAIL);

    *acct_port = data_p->acct_port;

    return RADIUS_OM_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_POM_GetServerAcctPort
 *-------------------------------------------------------------------------
 * PURPOSE  : get global server accounting port
 * INPUT    : none
 * OUTPUT   : acct_port
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T RADIUS_POM_GetServerAcctPort(UI32_T *acct_port)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(RADIUS_OM_GET_MSGBUFSIZE(RADIUS_OM_IPCMsg_ServerAcctPort_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    RADIUS_OM_IPCMsg_ServerAcctPort_T *data_p;

    if (acct_port == NULL)
        return FALSE;

    data_p = RADIUS_OM_MSG_DATA(msg_p);

    RADIUS_POM_SendMsg(RADIUS_OM_IPC_CMD_GET_SERVER_ACCT_PORT,
                       msg_p,
                       RADIUS_OM_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                       RADIUS_OM_GET_MSGBUFSIZE(RADIUS_OM_IPCMsg_ServerAcctPort_T),
                       (UI32_T)FALSE);

    *acct_port = data_p->acct_port;

    return (BOOL_T)RADIUS_OM_MSG_RETVAL(msg_p);
}

/* LOCAL SUBPROGRAM BODIES
 */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - RADIUS_POM_SendMsg
 *-------------------------------------------------------------------------
 * PURPOSE : To send an IPC message.
 * INPUT   : cmd       - the RADIUS message command.
 *           msg_p     - the buffer of the IPC message.
 *           req_size  - the size of RADIUS request message.
 *           res_size  - the size of RADIUS response message.
 *           ret_val   - the return value to set when IPC is failed.
 * OUTPUT  : msg_p     - the response message.
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
static void RADIUS_POM_SendMsg(UI32_T cmd, SYSFUN_Msg_T *msg_p, UI32_T req_size, UI32_T res_size, UI32_T ret_val)
{
    UI32_T ret;

    msg_p->cmd = SYS_MODULE_RADIUS;
    msg_p->msg_size = req_size;

    RADIUS_OM_MSG_CMD(msg_p) = cmd;

    ret = SYSFUN_SendRequestMsg(ipcmsgq_handle,
                                msg_p,
                                SYSFUN_TIMEOUT_WAIT_FOREVER,
                                0,
                                res_size,
                                msg_p);

    /* If IPC is failed, set return value as ret_val.
     */
    if (ret != SYSFUN_OK)
        RADIUS_OM_MSG_RETVAL(msg_p) = ret_val;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_POM_GetAccClientInvalidServerAddresses
 *-------------------------------------------------------------------------
 * PURPOSE  : get the number of RADIUS Accounting-Response packets received from unknown addresses.
 * INPUT    : none.
 * OUTPUT   : invalid_server_address_counter
 * RETURN   : TRUE - successful, FALSE - failure
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T RADIUS_POM_GetAccClientInvalidServerAddresses(UI32_T *invalid_server_address_counter)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(RADIUS_OM_GET_MSGBUFSIZE(RADIUS_OM_IPCMsg_Counter_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    RADIUS_OM_IPCMsg_Counter_T *data_p;

    if (invalid_server_address_counter == NULL)
        return FALSE;

    data_p = RADIUS_OM_MSG_DATA(msg_p);

    RADIUS_POM_SendMsg(RADIUS_POM_GET_ACC_CLIENT_INVALID_SERVER_ADDRESSES,
                       msg_p,
                       RADIUS_OM_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                       RADIUS_OM_GET_MSGBUFSIZE(RADIUS_OM_IPCMsg_Counter_T),
                       (UI32_T)FALSE);

    *invalid_server_address_counter = data_p->counter;

    return (BOOL_T)RADIUS_OM_MSG_RETVAL(msg_p);
}


