/*---------------------------------------------------------------------------
 * Module   : radius_pmgr.c
 *---------------------------------------------------------------------------
 * PURPOSE  : Provide APIs to access RADIUS.
 *---------------------------------------------------------------------------
 * NOTES    :
 *
 *---------------------------------------------------------------------------
 * HISTORY  : 08/15/2007 - Wakka Tu, Create
 *
 *---------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2007
 *---------------------------------------------------------------------------
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <string.h>
#include "sys_type.h"
#include "sys_module.h"
#include "sysfun.h"
#include "radius_mgr.h"
#include "radius_pmgr.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void RADIUS_PMGR_SendMsg(UI32_T cmd, SYSFUN_Msg_T *msg_p, UI32_T req_size, UI32_T res_size, UI32_T ret_val);

/* STATIC VARIABLE DECLARATIONS
 */
static SYSFUN_MsgQ_T ipcmsgq_handle;

/* MACRO FUNCTIONS DECLARACTION
 */

/* EXPORTED SUBPROGRAM BODIES
 */
/*---------------------------------------------------------------------------
 * FUNCTION NAME - RADIUS_PMGR_InitiateProcessResources
 *---------------------------------------------------------------------------
 * PURPOSE : Initiate resource used in the calling process.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE  - success
 *           FALSE - failure
 * NOTE    : None
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_PMGR_InitiateProcessResources(void)
{
    if (SYSFUN_GetMsgQ(RADIUS_MGR_IPCMSG_KEY, SYSFUN_MSGQ_BIDIRECTIONAL, &ipcmsgq_handle) != SYSFUN_OK)
    {
        printf("\r\n%s(): SYSFUN_GetMsgQ fail.", __FUNCTION__);
        return FALSE;
    }

    return TRUE;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_PMGR_Set_Request_Timeout
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set the number of times the RADIUS client
 *           waits for a reply from RADIUS server
 * INPUT:    timeout value.
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_PMGR_Set_Request_Timeout( UI32_T timeval )
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(RADIUS_MGR_GET_MSGBUFSIZE(RADIUS_MGR_IPCMsg_RequestTimeout_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    RADIUS_MGR_IPCMsg_RequestTimeout_T *data_p;

    data_p = RADIUS_MGR_MSG_DATA(msg_p);
    data_p->timeval = timeval;

    RADIUS_PMGR_SendMsg(RADIUS_MGR_IPC_CMD_SET_REQUEST_TIMEOUT,
                        msg_p,
                        RADIUS_MGR_GET_MSGBUFSIZE(RADIUS_MGR_IPCMsg_RequestTimeout_T),
                        RADIUS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                        (UI32_T)FALSE);

    return (BOOL_T)RADIUS_MGR_MSG_RETVAL(msg_p);
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_PMGR_Set_Server_Port
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set the UDP port number of the remote RADIUS server
 * INPUT:    Prot number
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_PMGR_Set_Server_Port(UI32_T serverport)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(RADIUS_MGR_GET_MSGBUFSIZE(RADIUS_MGR_IPCMsg_ServerPort_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    RADIUS_MGR_IPCMsg_ServerPort_T *data_p;

    data_p = RADIUS_MGR_MSG_DATA(msg_p);
    data_p->serverport = serverport;

    RADIUS_PMGR_SendMsg(RADIUS_MGR_IPC_CMD_SET_SERVER_PORT,
                        msg_p,
                        RADIUS_MGR_GET_MSGBUFSIZE(RADIUS_MGR_IPCMsg_ServerPort_T),
                        RADIUS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                        (UI32_T)FALSE);

    return (BOOL_T)RADIUS_MGR_MSG_RETVAL(msg_p);
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_PMGR_Set_Server_Secret
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set the shared secret text string used between
 *           the RADIUS client and server.
 * INPUT:    secret text string
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_PMGR_Set_Server_Secret( UI8_T * serversecret )
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(RADIUS_MGR_GET_MSGBUFSIZE(RADIUS_MGR_IPCMsg_ServerSecret_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    RADIUS_MGR_IPCMsg_ServerSecret_T *data_p;

    data_p = RADIUS_MGR_MSG_DATA(msg_p);
    strncpy((char *)data_p->serversecret, (char *)serversecret, sizeof(data_p->serversecret));
    data_p->serversecret[sizeof(data_p->serversecret)-1] = 0;

    RADIUS_PMGR_SendMsg(RADIUS_MGR_IPC_CMD_SET_SERVER_SECRET,
                        msg_p,
                        RADIUS_MGR_GET_MSGBUFSIZE(RADIUS_MGR_IPCMsg_ServerSecret_T),
                        RADIUS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                        (UI32_T)FALSE);

    return (BOOL_T)RADIUS_MGR_MSG_RETVAL(msg_p);
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_PMGR_Set_Retransmit_Times
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set the number of times the RADIUS client
 *           transmits each RADIUS request to the server before giving up
 * INPUT:    retransmit times
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_PMGR_Set_Retransmit_Times(UI32_T retryval)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(RADIUS_MGR_GET_MSGBUFSIZE(RADIUS_MGR_IPCMsg_RetransmitTimes_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    RADIUS_MGR_IPCMsg_RetransmitTimes_T *data_p;

    data_p = RADIUS_MGR_MSG_DATA(msg_p);
    data_p->retryval = retryval;

    RADIUS_PMGR_SendMsg(RADIUS_MGR_IPC_CMD_SET_RETRANSMIT_TIMES,
                        msg_p,
                        RADIUS_MGR_GET_MSGBUFSIZE(RADIUS_MGR_IPCMsg_RetransmitTimes_T),
                        RADIUS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                        (UI32_T)FALSE);

    return (BOOL_T)RADIUS_MGR_MSG_RETVAL(msg_p);
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_PMGR_Set_Server_IP
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set the IP address of the remote RADIUS server
 * INPUT:    RADIUS server IP address
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_PMGR_Set_Server_IP(UI32_T serverip)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(RADIUS_MGR_GET_MSGBUFSIZE(RADIUS_MGR_IPCMsg_ServerIP_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    RADIUS_MGR_IPCMsg_ServerIP_T *data_p;

    data_p = RADIUS_MGR_MSG_DATA(msg_p);
    data_p->serverip = serverip;

    RADIUS_PMGR_SendMsg(RADIUS_MGR_IPC_CMD_SET_SERVER_IP,
                        msg_p,
                        RADIUS_MGR_GET_MSGBUFSIZE(RADIUS_MGR_IPCMsg_ServerIP_T),
                        RADIUS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                        (UI32_T)FALSE);

    return (BOOL_T)RADIUS_MGR_MSG_RETVAL(msg_p);
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_PMGR_Auth_Check
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will call RADIUS client to do authentication
 * INPUT:    RADIUS Authentication username  and password
 * OUTPUT:   RADIUS Authentication privilege
 *           privilege: RADIUS client service type
 *            15 = AUTH_ADMINISTRATIVE
 *             0 = AUTH_LOGIN
 * RETURN:   RADIUS Authentication result
 *        -3 = not in master mode
 *            -2 = receive illegal packet
 *            -1 = Authentication failure
 *             0 = Authentication OK
 *             1 = Authentication timeout
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
I32_T RADIUS_PMGR_Auth_Check(UI8_T *username,UI8_T *password,I32_T *privilege, UI32_T cookie) /*maggie liu for RADIUS authentication ansync*/
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(RADIUS_MGR_GET_MSGBUFSIZE(RADIUS_MGR_IPCMsg_AuthCheck_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    RADIUS_MGR_IPCMsg_AuthCheck_T *data_p;
    UI32_T ret;

    if (username == NULL || password == NULL)
        return -2;

    data_p = RADIUS_MGR_MSG_DATA(msg_p);

    strncpy((char *)data_p->username, (char *)username, sizeof(data_p->username));
    data_p->username[sizeof(data_p->username)-1] = 0;

    strncpy((char *)data_p->password, (char *)password, sizeof(data_p->password));
    data_p->password[sizeof(data_p->password)-1] = 0;

    msg_p->cmd = SYS_MODULE_RADIUS;
    msg_p->msg_size = RADIUS_MGR_GET_MSGBUFSIZE(RADIUS_MGR_IPCMsg_AuthCheck_T);
    RADIUS_MGR_MSG_CMD(msg_p) = RADIUS_MGR_IPC_CMD_AUTH_CHECK;

    ret = SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG, RADIUS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
        msg_p);

    if (   (SYSFUN_OK != ret)
        || (RADIUS_MGR_IPC_RESULT_FAIL == RADIUS_MGR_MSG_RETVAL(msg_p))
        )
    {
        return -1;
    }

    return (I32_T)RADIUS_MGR_MSG_RETVAL(msg_p);
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_PMGR_AsyncLoginAuth
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will call RADIUS client to do authentication
 * INPUT:    username       - user name
 *           password       - password
 *           cookie         - caller cookie
 *           cookie_size    - size of cookie
 * OUTPUT:   None
 * RETURN:   TRUE - request is send successfully; FALSE - IPC error
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_PMGR_AsyncLoginAuth(
    const char *username,
    const char *password,
    void *cookie,
    UI32_T cookie_size)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(RADIUS_MGR_GET_MSGBUFSIZE(RADIUS_MGR_IPCMsg_AsyncLoginAuth_2_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    RADIUS_MGR_IPCMsg_AsyncLoginAuth_2_T *data_p;

    if (username == NULL || password == NULL)
    {
        return FALSE;
    }

    data_p = RADIUS_MGR_MSG_DATA(msg_p);

    strncpy((char *)data_p->username, (char *)username, sizeof(data_p->username));
    data_p->username[sizeof(data_p->username)-1] = 0;

    strncpy((char *)data_p->password, (char *)password, sizeof(data_p->password));
    data_p->password[sizeof(data_p->password)-1] = 0;

    if (sizeof(data_p->cookie) < cookie_size)
    {
        printf("%s: RADIUS_MGR_IPCMsg_AsyncLoginAuth_2_T.cookie too small\r\n",
            __FUNCTION__);
        return FALSE;
    }

    memcpy(data_p->cookie, cookie, cookie_size);
    data_p->cookie_size = cookie_size;

    RADIUS_MGR_MSG_RETVAL(msg_p) = TRUE;
    RADIUS_PMGR_SendMsg(RADIUS_MGR_IPC_CMD_ASYNC_LOGIN_AUTH,
                        msg_p,
                        RADIUS_MGR_GET_MSGBUFSIZE(RADIUS_MGR_IPCMsg_AsyncLoginAuth_2_T),
                        0,
                        FALSE);

    return RADIUS_MGR_MSG_RETVAL(msg_p);
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_PMGR_AsyncEnablePasswordAuth
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will call RADIUS client to do enable authentication
 * INPUT:    username       - user name
 *           password       - password
 *           cookie         - caller cookie
 *           cookie_size    - size of cookie
 * OUTPUT:   None
 * RETURN:   TRUE - request is send successfully; FALSE - IPC error
 * NOTE:     None
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_PMGR_AsyncEnablePasswordAuth(
    const char *username,
    const char *password,
    void *cookie,
    UI32_T cookie_size)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(RADIUS_MGR_GET_MSGBUFSIZE(RADIUS_MGR_IPCMsg_AsyncEnablePasswordAuth_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    RADIUS_MGR_IPCMsg_AsyncEnablePasswordAuth_T *data_p;

    if (username == NULL || password == NULL)
    {
        return FALSE;
    }

    data_p = RADIUS_MGR_MSG_DATA(msg_p);

    strncpy((char *)data_p->username, (char *)username, sizeof(data_p->username));
    data_p->username[sizeof(data_p->username)-1] = 0;

    strncpy((char *)data_p->password, (char *)password, sizeof(data_p->password));
    data_p->password[sizeof(data_p->password)-1] = 0;

    if (sizeof(data_p->cookie) < cookie_size)
    {
        printf("%s: RADIUS_MGR_IPCMsg_AsyncLoginAuth_2_T.cookie too small\r\n",
            __FUNCTION__);
        return FALSE;
    }

    memcpy(data_p->cookie, cookie, cookie_size);
    data_p->cookie_size = cookie_size;

    RADIUS_MGR_MSG_RETVAL(msg_p) = TRUE;
    RADIUS_PMGR_SendMsg(RADIUS_MGR_IPC_CMD_ASYNC_ENABLE_PASSWORD_AUTH,
                        msg_p,
                        RADIUS_MGR_GET_MSGBUFSIZE(RADIUS_MGR_IPCMsg_AsyncEnablePasswordAuth_T),
                        0,
                        FALSE);

    return RADIUS_MGR_MSG_RETVAL(msg_p);
}


/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_PMGR_AsyncEapAuthCheck
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will call RADIUS client to do EAP authentication
 * INPUT:    eap_data     --- EAP packet data
 *           eap_datalen  --- EAP packet data length
 *           radius_id    --- RADIUS sequent ID
 *           state_data   --- RADIUS STATE type packet data
 *           state_datale --- RADIUS STATE type packet data length
 *           src_port     --- source port
 *           src_mac      --- source mac address
 *           src_vid      --- source vid
 *           cookie       --- MSGQ_ID for return result
 *           service_type --- which component need to be service
 *           server_ip    --- Use this server IP address first. 0 means not
 *                            specified
 *           username_p   --- User name
 *           flag         --- Control flag
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_PMGR_AsyncEapAuthCheck(
    UI8_T   *eap_data,
    UI32_T  eap_datalen,
    UI32_T  radius_id,
    UI8_T   *state_data,
    UI32_T  state_datalen,
    UI32_T  src_port,
    UI8_T   *src_mac,
    UI32_T  src_vid,
    UI32_T  cookie,
    UI32_T  service_type,
    UI32_T  server_ip,
    char    *username_p,
    RADIUS_AsyncRequestControlFlag_T flag)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(RADIUS_MGR_GET_MSGBUFSIZE(RADIUS_MGR_IPCMsg_AsyncAuthCheck_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    RADIUS_MGR_IPCMsg_AsyncAuthCheck_T *data_p;

    data_p = RADIUS_MGR_MSG_DATA(msg_p);

    if (eap_datalen > sizeof(data_p->eap_data) ||
        state_datalen > sizeof(data_p->state_data))
        return FALSE;

    memcpy(data_p->eap_data,   eap_data,   eap_datalen);
    memcpy(data_p->state_data, state_data, state_datalen);
    memcpy(data_p->src_mac,    src_mac,    SYS_ADPT_MAC_ADDR_LEN);
    data_p->eap_datalen     = eap_datalen;
    data_p->state_datalen   = state_datalen;
    data_p->radius_id       = radius_id;
    data_p->src_port        = src_port;
    data_p->src_vid         = src_vid;
    data_p->cookie          = cookie;
    data_p->service_type    = service_type;
    data_p->server_ip       = server_ip;
    data_p->flag            = flag;

    strncpy(data_p->username, username_p, sizeof(data_p->username) - 1);
    data_p->username[sizeof(data_p->username) - 1] = '\0';

    RADIUS_PMGR_SendMsg(RADIUS_MGR_IPC_CMD_ASYNC_EAP_AUTH_CHECK,
                        msg_p,
                        RADIUS_MGR_GET_MSGBUFSIZE(RADIUS_MGR_IPCMsg_AsyncAuthCheck_T),
                        RADIUS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                        (UI32_T)FALSE);

    return (BOOL_T)RADIUS_MGR_MSG_RETVAL(msg_p);
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_PMGR_Set_Server_Host
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set the RADIUS server host
 * INPUT:    server_index,server_host
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE
 * NOTE:        server_ip
{
}
 *      server_port (1-65535)  - set 0 will use the global radius configuration
 *          timeout     (1-65535)  - set 0 will use the global radius configuration
 *          retransmit  (1-65535)  - set 0 will use the global radius configuration
 *          secret      (length < RADIUS_MAX_SECRET_LENGTH)  - set NULL will use the global radius configuration
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_PMGR_Set_Server_Host(UI32_T server_index,RADIUS_Server_Host_T *server_host)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(RADIUS_MGR_GET_MSGBUFSIZE(RADIUS_MGR_IPCMsg_SetServerHost_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    RADIUS_MGR_IPCMsg_SetServerHost_T *data_p;

    data_p = RADIUS_MGR_MSG_DATA(msg_p);
    data_p->server_index = server_index;
    memcpy(&data_p->server_host, server_host, sizeof(data_p->server_host));

    RADIUS_PMGR_SendMsg(RADIUS_MGR_IPC_CMD_SET_SERVER_HOST,
                        msg_p,
                        RADIUS_MGR_GET_MSGBUFSIZE(RADIUS_MGR_IPCMsg_SetServerHost_T),
                        RADIUS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                        (UI32_T)FALSE);

    return (BOOL_T)RADIUS_MGR_MSG_RETVAL(msg_p);
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_PMGR_Destroy_Server_Host
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will destroy the RADIUS server host
 * INPUT:    server_index
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_PMGR_Destroy_Server_Host(UI32_T server_index)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(RADIUS_MGR_GET_MSGBUFSIZE(RADIUS_MGR_IPCMsg_DestroyServerHost_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    RADIUS_MGR_IPCMsg_DestroyServerHost_T *data_p;

    data_p = RADIUS_MGR_MSG_DATA(msg_p);
    data_p->server_index = server_index;

    RADIUS_PMGR_SendMsg(RADIUS_MGR_IPC_CMD_DESTROY_SERVER_HOST,
                        msg_p,
                        RADIUS_MGR_GET_MSGBUFSIZE(RADIUS_MGR_IPCMsg_DestroyServerHost_T),
                        RADIUS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                        (UI32_T)FALSE);

    return (BOOL_T)RADIUS_MGR_MSG_RETVAL(msg_p);
}

#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE) /*maggie liu, 2009-03-09*/
/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_PMGR_SetServerAcctPort
 *---------------------------------------------------------------------------
 * PURPOSE  : set global server accounting port
 * INPUT    : acct_port
 * OUTPUT   : none
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_PMGR_SetServerAcctPort(UI32_T acct_port)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(RADIUS_MGR_GET_MSGBUFSIZE(RADIUS_MGR_IPCMsg_ServerAcctPort_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    RADIUS_MGR_IPCMsg_ServerAcctPort_T *data_p;

    data_p = RADIUS_MGR_MSG_DATA(msg_p);
    data_p->acct_port = acct_port;

    RADIUS_PMGR_SendMsg(RADIUS_MGR_IPC_CMD_GET_SERVER_ACCT_PORT,
                        msg_p,
                        RADIUS_MGR_GET_MSGBUFSIZE(RADIUS_MGR_IPCMsg_ServerAcctPort_T),
                        RADIUS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                        (UI32_T)FALSE);

    return (BOOL_T)RADIUS_MGR_MSG_RETVAL(msg_p);
}
#endif
/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_PMGR_RadaAuthCheck
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will call RADIUS client to do RADA authentication
 * INPUT:    src_port, src_mac, rada_username, rada_passwd,
 *           cookie (MSGQ_ID to return the result)
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_PMGR_RadaAuthCheck(
    UI32_T  src_port,       UI8_T   *src_mac,
    char    *rada_username, char    *rada_passwd,   UI32_T cookie)
{
    UI8_T                               msgbuf[SYSFUN_SIZE_OF_MSG(RADIUS_MGR_GET_MSGBUFSIZE(RADIUS_MGR_IPCMsg_RadaAuthCheck_T))];
    SYSFUN_Msg_T                        *msg_p = (SYSFUN_Msg_T *)msgbuf;
    RADIUS_MGR_IPCMsg_RadaAuthCheck_T   *data_p;

    data_p = RADIUS_MGR_MSG_DATA(msg_p);

    data_p->src_lport   = src_port;
    data_p->cookie      = cookie;
    memcpy (data_p->src_mac,       src_mac,       sizeof(data_p->src_mac));
    memcpy (data_p->rada_username, rada_username, sizeof(data_p->rada_username));
    memcpy (data_p->rada_password, rada_passwd,   sizeof(data_p->rada_password));

    RADIUS_PMGR_SendMsg(RADIUS_MGR_IPC_CMD_RADA_AUTH_CHECK,
                        msg_p,
                        RADIUS_MGR_GET_MSGBUFSIZE(RADIUS_MGR_IPCMsg_RadaAuthCheck_T),
                        RADIUS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                        (UI32_T)FALSE);

    return (BOOL_T)RADIUS_MGR_MSG_RETVAL(msg_p);
}

/* LOCAL SUBPROGRAM BODIES
 */
/*---------------------------------------------------------------------------
 * FUNCTION NAME - RADIUS_PMGR_SendMsg
 *---------------------------------------------------------------------------
 * PURPOSE : To send an IPC message.
 * INPUT   : cmd       - the RADIUS message command.
 *           msg_p     - the buffer of the IPC message.
 *           req_size  - the size of RADIUS request message.
 *           res_size  - the size of RADIUS response message.
 *           ret_val   - the return value to set when IPC is failed.
 * OUTPUT  : msg_p     - the response message.
 * RETURN  : None
 * NOTE    : None
 *---------------------------------------------------------------------------
 */
static void RADIUS_PMGR_SendMsg(UI32_T cmd, SYSFUN_Msg_T *msg_p, UI32_T req_size, UI32_T res_size, UI32_T ret_val)
{
    UI32_T ret;

    msg_p->cmd = SYS_MODULE_RADIUS;
    msg_p->msg_size = req_size;

    RADIUS_MGR_MSG_CMD(msg_p) = cmd;

    if(cmd<RADIUS_MGR_IPC_CMD_FOLLOWISASYNCHRONISMIPC)
    {
        ret = SYSFUN_SendRequestMsg(ipcmsgq_handle,
                                    msg_p,
                                    SYSFUN_TIMEOUT_WAIT_FOREVER,
                                    SYSFUN_SYSTEM_EVENT_IPCMSG,
                                    res_size,
                                    msg_p);
    }
    else
    {
        ret = SYSFUN_SendRequestMsg(ipcmsgq_handle,
                                    msg_p,
                                    SYSFUN_TIMEOUT_NOWAIT,
                                    SYSFUN_SYSTEM_EVENT_IPCMSG,
                                    0,
                                    NULL);
    }
    /* If IPC is failed, set return value as ret_val.
     */
    if (ret != SYSFUN_OK ||
        RADIUS_MGR_MSG_RETVAL(msg_p) == RADIUS_MGR_IPC_RESULT_FAIL)
        RADIUS_MGR_MSG_RETVAL(msg_p) = ret_val;
}

#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)
/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_PMGR_GetAccClientIdentifier
 *---------------------------------------------------------------------------
 * PURPOSE  : This function to get the NAS-Identifier of the RADIUS accounting client.
 * INPUT    : none.
 * OUTPUT   : client_identifier  --  the NAS-Identifier of the RADIUS accounting client.
 * RETURN   : TRUE to indicate successful and FALSE to indicate failure.
 * NOTES    : none.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_PMGR_GetAccClientIdentifier(RADACC_AccClientIdentifier_T *client_identifier)
{

    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(RADIUS_MGR_GET_MSGBUFSIZE(RADACC_AccClientIdentifier_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    RADACC_AccClientIdentifier_T *data_p;

    data_p = RADIUS_MGR_MSG_DATA(msg_p);

    RADIUS_PMGR_SendMsg(RADIUS_MGR_IPC_CMD_GET_ACC_CLIENT_IDENTIFIER,
                        msg_p,
                        RADIUS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                        RADIUS_MGR_GET_MSGBUFSIZE(RADACC_AccClientIdentifier_T),
                        (UI32_T)FALSE);

    *client_identifier=*data_p;

    return (BOOL_T)RADIUS_MGR_MSG_RETVAL(msg_p);
}
#endif /* #if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE) */

/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_PMGR_SubmitRequest
 *---------------------------------------------------------------------------
 * PURPOSE  : Submit a request, call RADIUS_MGR_SubmitRequest to create a
 *            request in request queue.
 * INPUT    : request_p
 * OUTPUT   : None
 * RETURN   : RADIUS_RETURN_FAIL
 *            RADIUS_RETURN_SUCCESS
 * NOTES    :
 *---------------------------------------------------------------------------
 */
BOOL_T
RADIUS_PMGR_SubmitRequest(
    RADIUS_MGR_RequestContext_T request_p)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(RADIUS_MGR_GET_MSGBUFSIZE(RADIUS_MGR_IPCMsg_SubmitRequest_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    RADIUS_MGR_IPCMsg_SubmitRequest_T *data_p;

    data_p = RADIUS_MGR_MSG_DATA(msg_p);
    memcpy(&data_p->request_p, &request_p, sizeof(data_p->request_p));

    RADIUS_PMGR_SendMsg(RADIUS_MGR_IPC_CMD_SUBMIT_REQUEST,
                        msg_p,
                        RADIUS_MGR_GET_MSGBUFSIZE(RADIUS_MGR_IPCMsg_SubmitRequest_T),
                        RADIUS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                        (UI32_T)FALSE);

    return (BOOL_T)RADIUS_MGR_MSG_RETVAL(msg_p);
}
