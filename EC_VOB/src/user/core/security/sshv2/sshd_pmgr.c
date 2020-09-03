/*-----------------------------------------------------------------------------
 * Module   : sshd_pmgr.c
 *-----------------------------------------------------------------------------
 * PURPOSE  : Provide APIs to access SSHD control functions.
 *-----------------------------------------------------------------------------
 * NOTES    :
 *
 *-----------------------------------------------------------------------------
 * HISTORY  : 09/28/2007 - Squid Ro, Create
 *
 *-----------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2007
 *-----------------------------------------------------------------------------
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <string.h>
#include "sys_type.h"
#include "sys_module.h"
#include "sysfun.h"
#include "sshd_mgr.h"
#include "buffer_mgr.h"


/* NAMING CONSTANT DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void SSHD_PMGR_SendMsg(UI32_T cmd, SYSFUN_Msg_T *msg_p, UI32_T req_size, UI32_T res_size, UI32_T ret_val);

/* STATIC VARIABLE DECLARATIONS
 */
static SYSFUN_MsgQ_T ipcmsgq_handle;

/* MACRO FUNCTIONS DECLARACTION
 */

/* EXPORTED SUBPROGRAM BODIES
 */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - SSHD_PMGR_InitiateProcessResources
 *-------------------------------------------------------------------------
 * PURPOSE : Initiate resource used in the calling process.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE  - success
 *           FALSE - failure
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
BOOL_T SSHD_PMGR_InitiateProcessResources(void)
{
    if (SYSFUN_GetMsgQ(SSHD_MGR_IPCMSG_KEY, SYSFUN_MSGQ_BIDIRECTIONAL, &ipcmsgq_handle) != SYSFUN_OK)
    {
        printf("\r\n%s(): SYSFUN_GetMsgQ fail.", __FUNCTION__);
        return FALSE;
    }

    return TRUE;
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SSHD_PMGR_DeleteUserPublicKey
 * ------------------------------------------------------------------------|
 * FUNCTION : delete user's public key.
 * INPUT    : username - name of user of public key will be delete.
 *            key_type - type of host key.
 *              (KEY_TYPE_RSA, KEY_TYPE_DSA and KEY_TYPE_BOTH_RSA_AND_DSA)
 * OUTPUT	: None
 * RETURN   : TRUE/FALSE
 * NOTE		: invoked in CLI command "delete public-key".
 * ------------------------------------------------------------------------*/
BOOL_T SSHD_PMGR_DeleteUserPublicKey(UI8_T *username, UI32_T key_type)
{
    UI8_T                           msgbuf[SYSFUN_SIZE_OF_MSG(SSHD_MGR_GET_MSGBUFSIZE(SSHD_MGR_IPCMsg_DelUserPbKey_T))];
    SYSFUN_Msg_T                    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SSHD_MGR_IPCMsg_DelUserPbKey_T  *data_p;

    memset (msgbuf, 0, sizeof (msgbuf));

    data_p = SSHD_MGR_MSG_DATA(msg_p);

    strncpy ((char *)data_p->username, (char *)username, SYS_ADPT_MAX_USER_NAME_LEN);
    data_p->key_type = key_type;

    SSHD_PMGR_SendMsg(SSHD_MGR_IPC_CMD_DEL_USER_PBKEY,
                      msg_p,
                      SSHD_MGR_GET_MSGBUFSIZE(SSHD_MGR_IPCMsg_DelUserPbKey_T),
                      SSHD_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      (UI32_T)FALSE);

    return (BOOL_T)SSHD_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SSHD_PMGR_GetAuthenticationRetries
 * ------------------------------------------------------------------------|
 * FUNCTION : get number of retries for authentication user.
 * INPUT    : None
 * OUTPUT	: None
 * RETURN   : number of retries for authentication user.
 * NOTE		: maybe invoked in CLI command 'show ip ssh'.
 * ------------------------------------------------------------------------*/
UI32_T SSHD_PMGR_GetAuthenticationRetries(void)
{
    UI8_T                           msgbuf[SYSFUN_SIZE_OF_MSG(SSHD_MGR_GET_MSGBUFSIZE(SSHD_MGR_IPCMsg_AuthenRetries_T))];
    SYSFUN_Msg_T                    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SSHD_MGR_IPCMsg_AuthenRetries_T *data_p;
    UI32_T                          res = 0;

    data_p = SSHD_MGR_MSG_DATA(msg_p);

    SSHD_PMGR_SendMsg(SSHD_MGR_IPC_CMD_GET_AUTHEN_RETRIES,
                      msg_p,
                      SSHD_MGR_GET_MSGBUFSIZE(SSHD_MGR_IPCMsg_AuthenRetries_T),
                      SSHD_MGR_GET_MSGBUFSIZE(SSHD_MGR_IPCMsg_AuthenRetries_T),
                      (UI32_T)FALSE);

    if (TRUE == (BOOL_T)SSHD_MGR_MSG_RETVAL(msg_p))
    {
        res = data_p->retries;
    }

    return res;
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SSHD_PMGR_GetNegotiationTimeout
 * ------------------------------------------------------------------------|
 * FUNCTION : get number of negotiation timeout.
 * INPUT    : None
 * OUTPUT	: None
 * RETURN   : number of negotiation timeout.
 * NOTE		: maybe invoked in CLI command 'show ip ssh'.
 * ------------------------------------------------------------------------*/
UI32_T SSHD_PMGR_GetNegotiationTimeout(void)
{
    UI8_T                           msgbuf[SYSFUN_SIZE_OF_MSG(SSHD_MGR_GET_MSGBUFSIZE(SSHD_MGR_IPCMsg_NegoTimeout_T))];
    SYSFUN_Msg_T                    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SSHD_MGR_IPCMsg_NegoTimeout_T   *data_p;
    UI32_T                          res = 0;

    data_p = SSHD_MGR_MSG_DATA(msg_p);

    SSHD_PMGR_SendMsg(SSHD_MGR_IPC_CMD_GET_NEGO_TIMEOUT,
                      msg_p,
                      SSHD_MGR_GET_MSGBUFSIZE(SSHD_MGR_IPCMsg_NegoTimeout_T),
                      SSHD_MGR_GET_MSGBUFSIZE(SSHD_MGR_IPCMsg_NegoTimeout_T),
                      (UI32_T)FALSE);

    if (TRUE == (BOOL_T)SSHD_MGR_MSG_RETVAL(msg_p))
    {
        res = data_p->timeout;
    }

    return res;
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SSHD_PMGR_GetNextSshConnectionEntry
 * ------------------------------------------------------------------------|
 * FUNCTION : get next active connection entry.
 * INPUT    : cid  - previous active connection id.
 * OUTPUT	: cid  - current active connection id.
 *            info - current active connection information.
 * RETURN   : TRUE/FALSE
 * NOTE		: invoked in CLI command "show ssh".
 *            initial input value is -1.
 * ------------------------------------------------------------------------*/
BOOL_T SSHD_PMGR_GetNextSshConnectionEntry(I32_T *cid, SSHD_ConnectionInfo_T *info)
{
    UI8_T                           msgbuf[SYSFUN_SIZE_OF_MSG(SSHD_MGR_GET_MSGBUFSIZE(SSHD_MGR_IPCMsg_ConnEntry_T))];
    SYSFUN_Msg_T                    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SSHD_MGR_IPCMsg_ConnEntry_T     *data_p;

    data_p = SSHD_MGR_MSG_DATA(msg_p);
    data_p->cid = *cid;

    SSHD_PMGR_SendMsg(SSHD_MGR_IPC_CMD_GET_NEXT_CONN_ENTRY,
                      msg_p,
                      SSHD_MGR_GET_MSGBUFSIZE(SSHD_MGR_IPCMsg_ConnEntry_T),
                      SSHD_MGR_GET_MSGBUFSIZE(SSHD_MGR_IPCMsg_ConnEntry_T),
                      (UI32_T)FALSE);

    if (TRUE == (BOOL_T) SSHD_MGR_MSG_RETVAL(msg_p))
    {
        *cid = data_p->cid;
        memcpy (info, &data_p->info, sizeof (data_p->info));

        return TRUE;
    }

    return FALSE;
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SSHD_PMGR_GetNextUserPublicKey
 * ------------------------------------------------------------------------|
 * FUNCTION : get user's public key.
 * INPUT    : username - username
 * OUTPUT	: username - next user name
 *            rsa_key  - pointer of buffer to storage rsa public key.
 *            dsa_key  - pointer of buffer to storage dsa public key.
 * RETURN   : TRUE/FALSE
 * NOTE		: invoked in CLI command "show public-key".
 *            initiation username is empty string.
 * ------------------------------------------------------------------------*/
BOOL_T SSHD_PMGR_GetNextUserPublicKey(UI8_T *username, UI8_T *rsa_key, UI8_T *dsa_key)
{
    UI8_T                           msgbuf[SYSFUN_SIZE_OF_MSG(SSHD_MGR_GET_MSGBUFSIZE(SSHD_MGR_IPCMsg_UserPbKey_T))];
    SYSFUN_Msg_T                    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SSHD_MGR_IPCMsg_UserPbKey_T     *data_p;

    memset (msgbuf, 0, sizeof (msgbuf));

    data_p = SSHD_MGR_MSG_DATA(msg_p);

    strncpy ((char *)data_p->username, (char *)username, SYS_ADPT_MAX_USER_NAME_LEN);

    SSHD_PMGR_SendMsg(SSHD_MGR_IPC_CMD_GET_NEXT_USER_PBKEY,
                      msg_p,
                      SSHD_MGR_GET_MSGBUFSIZE(SSHD_MGR_IPCMsg_UserPbKey_T),
                      SSHD_MGR_GET_MSGBUFSIZE(SSHD_MGR_IPCMsg_UserPbKey_T),
                      (UI32_T)FALSE);

    if (TRUE == (BOOL_T) SSHD_MGR_MSG_RETVAL(msg_p))
    {
        strncpy ((char *)username,(char *)data_p->username, SYS_ADPT_MAX_USER_NAME_LEN);
        memcpy  (rsa_key, data_p->rsa_key, sizeof (data_p ->rsa_key));
        memcpy  (dsa_key, data_p->dsa_key, sizeof (data_p ->dsa_key));
        return TRUE;
    }

    return FALSE;
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SSHD_PMGR_GetServerKeySize
 * ------------------------------------------------------------------------|
 * FUNCTION : get version of ssh server.
 * INPUT    : key_size - number of bits for server key.
 * OUTPUT	: None
 * RETURN   : TRUE/FALSE
 * NOTE		: maybe invoked in CLI command 'show ip ssh'.
 * ------------------------------------------------------------------------*/
BOOL_T SSHD_PMGR_GetServerKeySize(UI32_T *key_size)
{
    UI8_T                       msgbuf[SYSFUN_SIZE_OF_MSG(SSHD_MGR_GET_MSGBUFSIZE(SSHD_MGR_IPCMsg_SvrKSize_T))];
    SYSFUN_Msg_T                *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SSHD_MGR_IPCMsg_SvrKSize_T  *data_p;

    data_p = SSHD_MGR_MSG_DATA(msg_p);

    SSHD_PMGR_SendMsg(SSHD_MGR_IPC_CMD_GET_SERVER_KEYSIZE,
                      msg_p,
                      SSHD_MGR_GET_MSGBUFSIZE(SSHD_MGR_IPCMsg_SvrKSize_T),
                      SSHD_MGR_GET_MSGBUFSIZE(SSHD_MGR_IPCMsg_SvrKSize_T),
                      (UI32_T)FALSE);

    if (TRUE == (BOOL_T)SSHD_MGR_MSG_RETVAL(msg_p))
    {
        *key_size = data_p->key_size;
    }

    return (BOOL_T)SSHD_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SSHD_PMGR_GetSshConnectionPrivilege
 * ------------------------------------------------------------------------|
 * FUNCTION : get user privilege of ssh connection, index is connection id.
 * INPUT    : cid       - connection_id.
 * OUTPUT	: privilege - privilege.
 * RETURN   : TRUE/FALSE
 * NOTE		: invoked in CLI.
 * ------------------------------------------------------------------------*/
BOOL_T SSHD_PMGR_GetSshConnectionPrivilege(UI32_T cid, UI32_T *privilege)
{
    UI8_T                           msgbuf[SYSFUN_SIZE_OF_MSG(SSHD_MGR_GET_MSGBUFSIZE(SSHD_MGR_IPCMsg_SshdConnPriv_T))];
    SYSFUN_Msg_T                    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SSHD_MGR_IPCMsg_SshdConnPriv_T  *data_p;

    data_p = SSHD_MGR_MSG_DATA(msg_p);

    data_p->cid = cid;

    SSHD_PMGR_SendMsg(SSHD_MGR_IPC_CMD_GET_SSH_CONN_PRIV,
                      msg_p,
                      SSHD_MGR_GET_MSGBUFSIZE(SSHD_MGR_IPCMsg_SshdConnPriv_T),
                      SSHD_MGR_GET_MSGBUFSIZE(SSHD_MGR_IPCMsg_SshdConnPriv_T),
                      (UI32_T)FALSE);

    if (TRUE == (BOOL_T)SSHD_MGR_MSG_RETVAL(msg_p))
    {
        *privilege = data_p->privilege;
    }

    return (BOOL_T)SSHD_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SSHD_PMGR_GetSshConnectionUsername
 * ------------------------------------------------------------------------|
 * FUNCTION : get username of ssh connection, index is connection id.
 * INPUT    : cid      - connection_id.
 * OUTPUT	: username - username.
 * RETURN   : TRUE/FALSE
 * NOTE		: invoked in CLI.
 * ------------------------------------------------------------------------*/
BOOL_T SSHD_PMGR_GetSshConnectionUsername(UI32_T cid, UI8_T *username)
{
    UI8_T                           msgbuf[SYSFUN_SIZE_OF_MSG(SSHD_MGR_GET_MSGBUFSIZE(SSHD_MGR_IPCMsg_SshdConnName_T))];
    SYSFUN_Msg_T                    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SSHD_MGR_IPCMsg_SshdConnName_T  *data_p;

    data_p = SSHD_MGR_MSG_DATA(msg_p);

    SSHD_PMGR_SendMsg(SSHD_MGR_IPC_CMD_GET_SSH_CONN_NAME,
                      msg_p,
                      SSHD_MGR_GET_MSGBUFSIZE(SSHD_MGR_IPCMsg_SshdConnName_T),
                      SSHD_MGR_GET_MSGBUFSIZE(SSHD_MGR_IPCMsg_SshdConnName_T),
                      (UI32_T)FALSE);

    if (TRUE == (BOOL_T)SSHD_MGR_MSG_RETVAL(msg_p))
    {
        memcpy (username, data_p->username, sizeof (data_p->username));
    }

    return (BOOL_T)SSHD_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SSHD_PMGR_GetSshdStatus
 * ------------------------------------------------------------------------|
 * FUNCTION : get sshd state.
 * INPUT    : None
 * OUTPUT	: sshd state.
 * RETURN   : TRUE/FALSE
 * NOTE		: maybe invoked in CLI command 'show ip ssh'.
 * ------------------------------------------------------------------------*/
UI32_T SSHD_PMGR_GetSshdStatus(void)
{
    UI8_T                           msgbuf[SYSFUN_SIZE_OF_MSG(SSHD_MGR_GET_MSGBUFSIZE(SSHD_MGR_IPCMsg_SshdState_T))];
    SYSFUN_Msg_T                    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SSHD_MGR_IPCMsg_SshdState_T     *data_p;
    UI32_T                          res = 0;

    data_p = SSHD_MGR_MSG_DATA(msg_p);

    SSHD_PMGR_SendMsg(SSHD_MGR_IPC_CMD_GET_SSHD_STATUS,
                      msg_p,
                      SSHD_MGR_GET_MSGBUFSIZE(SSHD_MGR_IPCMsg_SshdState_T),
                      SSHD_MGR_GET_MSGBUFSIZE(SSHD_MGR_IPCMsg_SshdState_T),
                      (UI32_T)FALSE);

    if (TRUE == (BOOL_T)SSHD_MGR_MSG_RETVAL(msg_p))
    {
        res = data_p->state;
    }

    return res;
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SSHD_PMGR_GetSshServerVersion
 * ------------------------------------------------------------------------|
 * FUNCTION : get version of ssh server.
 * INPUT    : major - number of major version.
 *            minor - number of minor version.
 * OUTPUT	: None
 * RETURN   : TRUE/FALSE
 * NOTE		: maybe invoked in CLI command 'show ip ssh'.
 * ------------------------------------------------------------------------*/
BOOL_T SSHD_PMGR_GetSshServerVersion(UI32_T *major, UI32_T *minor)
{
    UI8_T                           msgbuf[SYSFUN_SIZE_OF_MSG(SSHD_MGR_GET_MSGBUFSIZE(SSHD_MGR_IPCMsg_SvrVersion_T))];
    SYSFUN_Msg_T                    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SSHD_MGR_IPCMsg_SvrVersion_T    *data_p;

    data_p = SSHD_MGR_MSG_DATA(msg_p);

    SSHD_PMGR_SendMsg(SSHD_MGR_IPC_CMD_GET_SERVER_VERSION,
                      msg_p,
                      SSHD_MGR_GET_MSGBUFSIZE(SSHD_MGR_IPCMsg_SvrVersion_T),
                      SSHD_MGR_GET_MSGBUFSIZE(SSHD_MGR_IPCMsg_SvrVersion_T),
                      (UI32_T)FALSE);

    if (TRUE == (BOOL_T)SSHD_MGR_MSG_RETVAL(msg_p))
    {
        *major = data_p->major;
        *minor = data_p->minor;
    }

    return (BOOL_T)SSHD_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SSHD_PMGR_GetUserPublicKey
 * ------------------------------------------------------------------------|
 * FUNCTION : get user's public key.
 * INPUT    : username - username
 * OUTPUT	: rsa_key  - pointer of buffer to storage rsa public key.
 *            dsa_key  - pointer of buffer to storage dsa public key.
 * RETURN   : TRUE/FALSE
 * NOTE		: invoked in CLI command "show public-key".
 * ------------------------------------------------------------------------*/
BOOL_T SSHD_PMGR_GetUserPublicKey(UI8_T *username, UI8_T *rsa_key, UI8_T *dsa_key)
{
    UI8_T                           msgbuf[SYSFUN_SIZE_OF_MSG(SSHD_MGR_GET_MSGBUFSIZE(SSHD_MGR_IPCMsg_UserPbKey_T))];
    SYSFUN_Msg_T                    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SSHD_MGR_IPCMsg_UserPbKey_T     *data_p;

    memset (msgbuf, 0, sizeof (msgbuf));

    data_p = SSHD_MGR_MSG_DATA(msg_p);

    strncpy ((char *)data_p->username, (char *)username, SYS_ADPT_MAX_USER_NAME_LEN);

    SSHD_PMGR_SendMsg(SSHD_MGR_IPC_CMD_GET_USER_PBKEY,
                      msg_p,
                      SSHD_MGR_GET_MSGBUFSIZE(SSHD_MGR_IPCMsg_UserPbKey_T),
                      SSHD_MGR_GET_MSGBUFSIZE(SSHD_MGR_IPCMsg_UserPbKey_T),
                      (UI32_T)FALSE);

    if (TRUE == (BOOL_T) SSHD_MGR_MSG_RETVAL(msg_p))
    {
        memcpy  (rsa_key, data_p->rsa_key, sizeof (data_p ->rsa_key));
        memcpy  (dsa_key, data_p->dsa_key, sizeof (data_p ->dsa_key));
        return TRUE;
    }

    return FALSE;
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SSHD_PMGR_SetAuthenticationRetries
 * ------------------------------------------------------------------------|
 * FUNCTION : set number of retries for authentication user.
 * INPUT    : retries - number of retries for authentication user.
 * OUTPUT	: None
 * RETURN   : TRUE/FALSE
 * NOTE		: maybe invoked in CLI command 'ip ssh authentication-retries'.
 * ------------------------------------------------------------------------*/
BOOL_T SSHD_PMGR_SetAuthenticationRetries(UI32_T retries)
{
    UI8_T                           msgbuf[SYSFUN_SIZE_OF_MSG(SSHD_MGR_GET_MSGBUFSIZE(SSHD_MGR_IPCMsg_AuthenRetries_T))];
    SYSFUN_Msg_T                    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SSHD_MGR_IPCMsg_AuthenRetries_T *data_p;

    data_p = SSHD_MGR_MSG_DATA(msg_p);

    data_p->retries = retries;

    SSHD_PMGR_SendMsg(SSHD_MGR_IPC_CMD_SET_AUTHEN_RETRIES,
                      msg_p,
                      SSHD_MGR_GET_MSGBUFSIZE(SSHD_MGR_IPCMsg_AuthenRetries_T),
                      SSHD_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      (UI32_T)FALSE);

    return (BOOL_T)SSHD_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SSHD_PMGR_SetConnectionIDAndTnshID
 * ------------------------------------------------------------------------|
 * FUNCTION : set tnsh id and ssh connection id to session record.
 * INPUT    : tnsh_port - the port connect to TNSHD.
 *            tid       - TNSH id.
 *            cid       - ssh connection id.
 * OUTPUT	: None
 * RETURN   : TRUE      - tnsh_port found and ssh server enabled,
 *                          or don't found tnsh_port.
 *            FALSE     - tnsh_port found and ssh server disabled.
 * NOTE		: nvoked in TNSHD_ChildTask().
 * ------------------------------------------------------------------------*/
BOOL_T SSHD_PMGR_SetConnectionIDAndTnshID(UI32_T tnsh_port, UI32_T tid, UI32_T cid)
{
    UI8_T                           msgbuf[SYSFUN_SIZE_OF_MSG(SSHD_MGR_GET_MSGBUFSIZE(SSHD_MGR_IPCMsg_CidAndTnshid_T))];
    SYSFUN_Msg_T                    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SSHD_MGR_IPCMsg_CidAndTnshid_T  *data_p;

    data_p = SSHD_MGR_MSG_DATA(msg_p);

    data_p->tnsh_port = tnsh_port;
    data_p->tid       = tid;
    data_p->cid       = cid;

    SSHD_PMGR_SendMsg(SSHD_MGR_IPC_CMD_SET_CID_AND_TNSHID,
                      msg_p,
                      SSHD_MGR_GET_MSGBUFSIZE(SSHD_MGR_IPCMsg_CidAndTnshid_T),
                      SSHD_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      (UI32_T)FALSE);

    return (BOOL_T)SSHD_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SSHD_PMGR_SetNegotiationTimeout
 * ------------------------------------------------------------------------|
 * FUNCTION : set number of negotiation timeout.
 * INPUT    : timeout - negotiation timeout
 * OUTPUT	: None
 * RETURN   : TRUE/FALSE
 * NOTE		: maybe invoked in CLI command 'ip ssh timeout'.
 * ------------------------------------------------------------------------*/
BOOL_T SSHD_PMGR_SetNegotiationTimeout(UI32_T timeout)
{
    UI8_T                           msgbuf[SYSFUN_SIZE_OF_MSG(SSHD_MGR_GET_MSGBUFSIZE(SSHD_MGR_IPCMsg_NegoTimeout_T))];
    SYSFUN_Msg_T                    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SSHD_MGR_IPCMsg_NegoTimeout_T   *data_p;

    data_p = SSHD_MGR_MSG_DATA(msg_p);

    data_p->timeout = timeout;

    SSHD_PMGR_SendMsg(SSHD_MGR_IPC_CMD_SET_NEGO_TIMEOUT,
                      msg_p,
                      SSHD_MGR_GET_MSGBUFSIZE(SSHD_MGR_IPCMsg_NegoTimeout_T),
                      SSHD_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      (UI32_T)FALSE);

    return (BOOL_T)SSHD_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SSHD_PMGR_SetServerKeySize
 * ------------------------------------------------------------------------|
 * FUNCTION : set number of bits for server key.
 * INPUT    : key_size - number of bits for server key, range is
 *                       from 512 to 896 bits.
 * OUTPUT	: None
 * RETURN   : TRUE/FALSE
 * NOTE		: maybe invoked in CLI command 'ip ssh server-key size'.
 * ------------------------------------------------------------------------*/
BOOL_T SSHD_PMGR_SetServerKeySize(UI32_T key_size)
{
    UI8_T                           msgbuf[SYSFUN_SIZE_OF_MSG(SSHD_MGR_GET_MSGBUFSIZE(SSHD_MGR_IPCMsg_SvrKSize_T))];
    SYSFUN_Msg_T                    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SSHD_MGR_IPCMsg_SvrKSize_T      *data_p;

    data_p = SSHD_MGR_MSG_DATA(msg_p);

    data_p->key_size = key_size;

    SSHD_PMGR_SendMsg(SSHD_MGR_IPC_CMD_SET_SERVER_KEYSIZE,
                      msg_p,
                      SSHD_MGR_GET_MSGBUFSIZE(SSHD_MGR_IPCMsg_SvrKSize_T),
                      SSHD_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      (UI32_T)FALSE);

    return (BOOL_T)SSHD_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SSHD_PMGR_SetSshdStatus
 * ------------------------------------------------------------------------|
 * FUNCTION : set sshd state.
 * INPUT    : state - SSHD status.
 * OUTPUT	: None
 * RETURN   : TRUE/FALSE
 * NOTE		: maybe invoked in CLI command 'ip ssh server'.
 * ------------------------------------------------------------------------*/
BOOL_T SSHD_PMGR_SetSshdStatus (UI32_T state)
{
    UI8_T                           msgbuf[SYSFUN_SIZE_OF_MSG(SSHD_MGR_GET_MSGBUFSIZE(SSHD_MGR_IPCMsg_SshdState_T))];
    SYSFUN_Msg_T                    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SSHD_MGR_IPCMsg_SshdState_T     *data_p;

    data_p = SSHD_MGR_MSG_DATA(msg_p);

    data_p->state = state;

    SSHD_PMGR_SendMsg(SSHD_MGR_IPC_CMD_SET_SSHD_STATUS,
                      msg_p,
                      SSHD_MGR_GET_MSGBUFSIZE(SSHD_MGR_IPCMsg_SshdState_T),
                      SSHD_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      (UI32_T)FALSE);

    return (BOOL_T)SSHD_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SSHD_PMGR_GetGenerateHostKeyStatus
 * ------------------------------------------------------------------------|
 * FUNCTION : get generate host key status of ssh server.
 * INPUT    : none
 * OUTPUT	: action_result - result of host key generation.
 * RETURN   : TRUE/FALSE
 * NOTE		: maybe invoked in CLI command 'show ip ssh'.
 * ------------------------------------------------------------------------*/
BOOL_T SSHD_PMGR_GetGenerateHostKeyStatus(UI32_T *action_result)
{
    UI8_T                       msgbuf[SYSFUN_SIZE_OF_MSG(SSHD_MGR_GET_MSGBUFSIZE(SSHD_MGR_IPCMsg_SshdGenHostKeyState_T))];
    SYSFUN_Msg_T                *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SSHD_MGR_IPCMsg_SshdGenHostKeyState_T  *data_p;

    data_p = SSHD_MGR_MSG_DATA(msg_p);

    SSHD_PMGR_SendMsg(SSHD_MGR_IPC_CMD_GET_GENERATE_HOST_KEY_STATUS,
                      msg_p,
                      SSHD_MGR_GET_MSGBUFSIZE(SSHD_MGR_IPCMsg_SshdGenHostKeyState_T),
                      SSHD_MGR_GET_MSGBUFSIZE(SSHD_MGR_IPCMsg_SshdGenHostKeyState_T),
                      (UI32_T)FALSE);

    if (TRUE == (BOOL_T)SSHD_MGR_MSG_RETVAL(msg_p))
    {
        *action_result = data_p->state;
    }

    return (BOOL_T)SSHD_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SSHD_PMGR_GetGenerateHostKeyAction
 * ------------------------------------------------------------------------|
 * FUNCTION : Get tpye of host key generation.
 * INPUT    : none
 * OUTPUT	: UI32_T  *action_type    --  tpye of host key generation.
 * RETURN   : TRUE/FALSE
 * NOTE		: maybe invoked in CLI command 'show ip ssh'.
 * ------------------------------------------------------------------------*/
BOOL_T SSHD_PMGR_GetGenerateHostKeyAction(UI32_T *action_type)
{
    UI8_T                       msgbuf[SYSFUN_SIZE_OF_MSG(SSHD_MGR_GET_MSGBUFSIZE(SSHD_MGR_IPCMsg_SshdGenHostKeyAction_T))];
    SYSFUN_Msg_T                *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SSHD_MGR_IPCMsg_SshdGenHostKeyAction_T  *data_p;

    data_p = SSHD_MGR_MSG_DATA(msg_p);

    SSHD_PMGR_SendMsg(SSHD_MGR_IPC_CMD_GET_GENERATE_HOST_KEY_ACTION,
                      msg_p,
                      SSHD_MGR_GET_MSGBUFSIZE(SSHD_MGR_IPCMsg_SshdGenHostKeyAction_T),
                      SSHD_MGR_GET_MSGBUFSIZE(SSHD_MGR_IPCMsg_SshdGenHostKeyAction_T),
                      (UI32_T)FALSE);

    if (TRUE == (BOOL_T)SSHD_MGR_MSG_RETVAL(msg_p))
    {
        *action_type = data_p->action;
    }

    return (BOOL_T)SSHD_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SSHD_PMGR_GetWriteHostKey2FlashAction
 * ------------------------------------------------------------------------|
 * FUNCTION : Get tpye of host key writing.
 * INPUT    : none
 * OUTPUT	: action_type - tpye of host key writing
 * RETURN   : TRUE/FALSE
 * NOTE		: maybe invoked in CLI command 'show ip ssh'.
 * ------------------------------------------------------------------------*/
BOOL_T SSHD_PMGR_GetWriteHostKey2FlashAction(UI32_T *action_type)
{
    UI8_T                       msgbuf[SYSFUN_SIZE_OF_MSG(SSHD_MGR_GET_MSGBUFSIZE(SSHD_MGR_IPCMsg_SshdWriteHK2FAction_T))];
    SYSFUN_Msg_T                *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SSHD_MGR_IPCMsg_SshdWriteHK2FAction_T  *data_p;

    data_p = SSHD_MGR_MSG_DATA(msg_p);

    SSHD_PMGR_SendMsg(SSHD_MGR_IPC_CMD_GET_WRITE_HOST_KEY_2_FLASH_ACTION,
                      msg_p,
                      SSHD_MGR_GET_MSGBUFSIZE(SSHD_MGR_IPCMsg_SshdWriteHK2FAction_T),
                      SSHD_MGR_GET_MSGBUFSIZE(SSHD_MGR_IPCMsg_SshdWriteHK2FAction_T),
                      (UI32_T)FALSE);

    if (TRUE == (BOOL_T)SSHD_MGR_MSG_RETVAL(msg_p))
    {
        *action_type = data_p->action;
    }

    return (BOOL_T)SSHD_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SSHD_PMGR_AsyncWriteHostKey2Flash
 * ------------------------------------------------------------------------|
 * FUNCTION : This function write host key from memory to flash.
 * INPUT    : none
 * OUTPUT	: None
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
BOOL_T SSHD_PMGR_AsyncWriteHostKey2Flash ()
{
    UI8_T                           msgbuf[SYSFUN_SIZE_OF_MSG(SSHD_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA())];
    SYSFUN_Msg_T                    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    /*SSHD_MGR_IPCMsg_SshdState_T     *data_p;*/

    /*data_p = SSHD_MGR_MSG_DATA(msg_p);*/

    SSHD_PMGR_SendMsg(SSHD_MGR_IPC_CMD_ASYNC_WRITE_HOST_KEY_2_FLASH,
                      msg_p,
                      SSHD_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      SSHD_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      (UI32_T)FALSE);

    return (BOOL_T)SSHD_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SSHD_PMGR_GetWriteHostKey2FlashStatus
 * ------------------------------------------------------------------------|
 * FUNCTION : Get result of host key writing.
 * INPUT    : none
 * OUTPUT	: *action_result  --  result of host key writing.
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
BOOL_T SSHD_PMGR_GetWriteHostKey2FlashStatus(UI32_T *action_result)
{
    UI8_T                       msgbuf[SYSFUN_SIZE_OF_MSG(SSHD_MGR_GET_MSGBUFSIZE(SSHD_MGR_IPCMsg_SshdWriteHK2FState_T))];
    SYSFUN_Msg_T                *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SSHD_MGR_IPCMsg_SshdWriteHK2FState_T  *data_p;

    data_p = SSHD_MGR_MSG_DATA(msg_p);

    SSHD_PMGR_SendMsg(SSHD_MGR_IPC_CMD_GET_WRITE_HOST_KEY_2_FLASH_STATUS,
                      msg_p,
                      SSHD_MGR_GET_MSGBUFSIZE(SSHD_MGR_IPCMsg_SshdWriteHK2FState_T),
                      SSHD_MGR_GET_MSGBUFSIZE(SSHD_MGR_IPCMsg_SshdWriteHK2FState_T),
                      (UI32_T)FALSE);

    if (TRUE == (BOOL_T)SSHD_MGR_MSG_RETVAL(msg_p))
    {
        *action_result = data_p->state;
    }

    return (BOOL_T)SSHD_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SSHD_PMGR_AsyncGenerateHostKeyPair
 * ------------------------------------------------------------------------|
 * FUNCTION : Generate RSA/DSA public and private key pair of host key.
 * INPUT    : UI32_T  key_type    --  Type of host key.(KEY_TYPE_RSA, KEY_TYPE_DSA, KEY_TYPE_BOTH_RSA_AND_DSA)
 * OUTPUT	: None
 * RETURN   : TRUE/FALSE
 * NOTE		: maybe invoked in CLI command 'ip ssh server-key size'.
 * ------------------------------------------------------------------------*/
BOOL_T SSHD_PMGR_AsyncGenerateHostKeyPair(UI32_T key_type)
{
    UI8_T                           msgbuf[SYSFUN_SIZE_OF_MSG(SSHD_MGR_GET_MSGBUFSIZE(SSHD_MGR_IPCMsg_SshdAsyncGenHKPair_T))];
    SYSFUN_Msg_T                    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SSHD_MGR_IPCMsg_SshdAsyncGenHKPair_T      *data_p;

    data_p = SSHD_MGR_MSG_DATA(msg_p);

    data_p->type = key_type;

    SSHD_PMGR_SendMsg(SSHD_MGR_IPC_CMD_ASYNC_GENERATE_HOST_KEY_PAIR,
                      msg_p,
                      SSHD_MGR_GET_MSGBUFSIZE(SSHD_MGR_IPCMsg_SshdAsyncGenHKPair_T),
                      SSHD_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      (UI32_T)FALSE);

    return (BOOL_T)SSHD_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SSHD_PMGR_GetSshConnectionEntry
 * ------------------------------------------------------------------------|
 * FUNCTION : get active connection entry.
 * INPUT    : cid  - previous active connection id.
 * OUTPUT	: info - current active connection information.
 * RETURN   : TRUE/FALSE
 * NOTE		: invoked in CLI command "show ssh".
 *            initial input value is -1.
 * ------------------------------------------------------------------------*/
BOOL_T SSHD_PMGR_GetSshConnectionEntry(I32_T cid, SSHD_ConnectionInfo_T *info)
{
    UI8_T                           msgbuf[SYSFUN_SIZE_OF_MSG(SSHD_MGR_GET_MSGBUFSIZE(SSHD_MGR_IPCMsg_ConnEntry_T))];
    SYSFUN_Msg_T                    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SSHD_MGR_IPCMsg_ConnEntry_T     *data_p;

    data_p = SSHD_MGR_MSG_DATA(msg_p);
    data_p->cid = cid;

    SSHD_PMGR_SendMsg(SSHD_MGR_IPC_CMD_GET_CONN_ENTRY,
                      msg_p,
                      SSHD_MGR_GET_MSGBUFSIZE(SSHD_MGR_IPCMsg_ConnEntry_T),
                      SSHD_MGR_GET_MSGBUFSIZE(SSHD_MGR_IPCMsg_ConnEntry_T),
                      (UI32_T)FALSE);

    if (TRUE == (BOOL_T) SSHD_MGR_MSG_RETVAL(msg_p))
    {
        memcpy (info, &data_p->info, sizeof (data_p->info));

        return TRUE;
    }

    return FALSE;
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SSHD_PMGR_AsyncDeleteUserPublicKey
 * ------------------------------------------------------------------------|
 * FUNCTION : Delete RSA/DSA public key of user key.
 * INPUT    : username - name of user of public key will be delete.
 *            key_type - Type of host key.(KEY_TYPE_RSA, KEY_TYPE_DSA, KEY_TYPE_BOTH_RSA_AND_DSA)
 * OUTPUT	: None
 * RETURN   : TRUE/FALSE
 * NOTE		: This API is invoked in SNMP sshUserKeyDelAction().
 * ------------------------------------------------------------------------*/
BOOL_T SSHD_PMGR_AsyncDeleteUserPublicKey(UI8_T *username, UI32_T key_type)
{
    UI8_T                           msgbuf[SYSFUN_SIZE_OF_MSG(SSHD_MGR_GET_MSGBUFSIZE(SSHD_MGR_IPCMsg_DelUserPbKey_T))];
    SYSFUN_Msg_T                    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SSHD_MGR_IPCMsg_DelUserPbKey_T  *data_p;

    memset (msgbuf, 0, sizeof (msgbuf));

    data_p = SSHD_MGR_MSG_DATA(msg_p);

    strncpy ((char *)data_p->username, (char *)username, SYS_ADPT_MAX_USER_NAME_LEN);
    data_p->key_type = key_type;

    SSHD_PMGR_SendMsg(SSHD_MGR_IPC_CMD_ASYNC_DELETE_USER_PUBLIC_KEY,
                      msg_p,
                      SSHD_MGR_GET_MSGBUFSIZE(SSHD_MGR_IPCMsg_DelUserPbKey_T),
                      SSHD_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      (UI32_T)FALSE);

    return (BOOL_T)SSHD_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SSHD_PMGR_GetRunningServerKeySize
 * ------------------------------------------------------------------------|
 * FUNCTION : This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
 *            specific server key size non-default values
 *            can be retrieved successfully. Otherwise, SYS_TYPE_GET_RUNNING_CFG_FAIL
 *            or SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT    : None
 * OUTPUT	: key_size - number of bits for server key.
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE		: 1. This function shall only be invoked by CLI to save the
 *               "running configuration" to local or remote files.
 *            2. Since only non-default configuration will be saved, this
 *               function shall return non-default structure for each field for the device.
 * ------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T  SSHD_PMGR_GetRunningServerKeySize(UI32_T *key_size)
{
    UI8_T                       msgbuf[SYSFUN_SIZE_OF_MSG(SSHD_MGR_GET_MSGBUFSIZE(SSHD_MGR_IPCMsg_SvrKSize_T))];
    SYSFUN_Msg_T                *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SSHD_MGR_IPCMsg_SvrKSize_T  *data_p;

    data_p = SSHD_MGR_MSG_DATA(msg_p);

    SSHD_PMGR_SendMsg(SSHD_MGR_IPC_CMD_GET_RUNN_SERVER_KEYSIZE,
                      msg_p,
                      SSHD_MGR_GET_MSGBUFSIZE(SSHD_MGR_IPCMsg_SvrKSize_T),
                      SSHD_MGR_GET_MSGBUFSIZE(SSHD_MGR_IPCMsg_SvrKSize_T),
                      (UI32_T)FALSE);

    if (SYS_TYPE_GET_RUNNING_CFG_SUCCESS == SSHD_MGR_MSG_RETVAL(msg_p))
    {
        *key_size = data_p->key_size;
    }

    return (SYS_TYPE_Get_Running_Cfg_T)SSHD_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SSHD_PMGR_GetDeleteUserPublicKeyAction
 * ------------------------------------------------------------------------|
 * FUNCTION : Get tpye of user key delete.
 * INPUT    :
 * OUTPUT	: action_type --  tpye of user key delete.
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
BOOL_T SSHD_PMGR_GetDeleteUserPublicKeyAction(UI32_T *action_type)
{
    UI8_T                       msgbuf[SYSFUN_SIZE_OF_MSG(SSHD_MGR_GET_MSGBUFSIZE(SSHD_MGR_IPCMsg_SshdDeleteUPKeyAction_T))];
    SYSFUN_Msg_T                *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SSHD_MGR_IPCMsg_SshdDeleteUPKeyAction_T  *data_p;

    data_p = SSHD_MGR_MSG_DATA(msg_p);

    SSHD_PMGR_SendMsg(SSHD_MGR_IPC_CMD_GET_DELETE_USER_PUBLIC_KEY_ACTION,
                      msg_p,
                      SSHD_MGR_GET_MSGBUFSIZE(SSHD_MGR_IPCMsg_SshdDeleteUPKeyAction_T),
                      SSHD_MGR_GET_MSGBUFSIZE(SSHD_MGR_IPCMsg_SshdDeleteUPKeyAction_T),
                      (UI32_T)FALSE);

    if (SYS_TYPE_GET_RUNNING_CFG_SUCCESS == SSHD_MGR_MSG_RETVAL(msg_p))
    {
        *action_type = data_p->action;
    }

    return (SYS_TYPE_Get_Running_Cfg_T)SSHD_MGR_MSG_RETVAL(msg_p);
}

/* FUNCTION NAME:  SSHD_PMGR_SetUserPublicKey
 * PURPOSE:
 *          This function to save user's public key to flash.
 *
 * INPUT:
 *          UI8_T   *username   --  Username of public key.
 *          UI32_T  key_type    --  Type of host key.(KEY_TYPE_RSA, KEY_TYPE_DSA)
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          One of SSHD_SetUserPublicKeyResult_T to indicate that the result.
 * NOTES:
 *          This function is invoked in CLI command "copy tftp public-key".
 *          This function have be called after SSHD_MGR_GetUserPublicKeyFromXfer().
 */
SSHD_SetUserPublicKeyResult_T SSHD_PMGR_SetUserPublicKey(UI8_T *username, UI32_T key_type)
{
    UI8_T                       msgbuf[SYSFUN_SIZE_OF_MSG(SSHD_MGR_GET_MSGBUFSIZE(SSHD_MGR_IPCMsg_SshdSetUserPublicKey_T))];
    SYSFUN_Msg_T                *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SSHD_MGR_IPCMsg_SshdSetUserPublicKey_T  *data_p;

    if(username == NULL)
        return SSHD_SET_USER_PUBLIC_KEY_FAIL;

    data_p = SSHD_MGR_MSG_DATA(msg_p);

    data_p->type = key_type;
    strcpy((char *)data_p->username, (char *)username);

    SSHD_PMGR_SendMsg(SSHD_MGR_IPC_CMD_SET_USER_PUBLIC_KEY,
                      msg_p,
                      SSHD_MGR_GET_MSGBUFSIZE(SSHD_MGR_IPCMsg_SshdSetUserPublicKey_T),
                      SSHD_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      (UI32_T)SSHD_SET_USER_PUBLIC_KEY_FAIL);

    return SSHD_MGR_MSG_RETVAL(msg_p);
}

/* FUNCTION NAME:  SSHD_PMGR_CopyUserPublicKey
 * PURPOSE:
 *          This function to copy public key from xfer tftp buffer to sshd buffer
 *
 * INPUT:
 *          UI8_T   *key_buf   --  xfer tftp public key buffer address
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          This function is invoked in CLI command "copy tftp public-key".
 *          This function have be called before SSHD_PMGR_SetUserPublicKey().
 */
BOOL_T SSHD_PMGR_CopyUserPublicKey(UI8_T *key_buf)
{
    UI8_T                       msgbuf[SYSFUN_SIZE_OF_MSG(SSHD_MGR_GET_MSGBUFSIZE(SSHD_MGR_IPCMsg_SshdSetUserPublicKey_T))];
    SYSFUN_Msg_T                *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SSHD_MGR_IPCMsg_SshdSetUserPublicKey_T  *data_p;

    data_p = SSHD_MGR_MSG_DATA(msg_p);

	data_p->key_buf_offset = BUFFER_MGR_GetOffset(key_buf);

    SSHD_PMGR_SendMsg(SSHD_MGR_IPC_CMD_COPY_USER_PUBLIC_KEY,
                      msg_p,
                      SSHD_MGR_GET_MSGBUFSIZE(SSHD_MGR_IPCMsg_SshdSetUserPublicKey_T),
                      SSHD_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      (UI32_T)FALSE);

    return SSHD_MGR_MSG_RETVAL(msg_p);
}

/* FUNCTION NAME : SSHD_MGR_GetUserPublicKeyType
 * PURPOSE:
 *      Get user's public key type.
 *
 * INPUT:
 *      UI8_T   *username   -- username.
 *
 * OUTPUT:
 *      UI32_T  *key_type   --  Type of host key.(KEY_TYPE_RSA, KEY_TYPE_DSA, KEY_TYPE_BOTH_RSA_AND_DSA)
 *
 * RETURN:
 *      TRUE  - User has public key.
 *      FALSE - User has not public key.
 *
 * NOTES:
 *      This API is invoked in CLI command "show users".
 */
BOOL_T SSHD_PMGR_GetUserPublicKeyType(UI8_T *username, UI32_T *key_type)
{
    UI8_T                       msgbuf[SYSFUN_SIZE_OF_MSG(SSHD_MGR_GET_MSGBUFSIZE(SSHD_MGR_IPCMsg_SshdGetUserPublicKeyType_T))];
    SYSFUN_Msg_T                *msg_p = (SYSFUN_Msg_T *)msgbuf;
    SSHD_MGR_IPCMsg_SshdGetUserPublicKeyType_T  *data_p;

    if(username == NULL)
        return FALSE;

    data_p = SSHD_MGR_MSG_DATA(msg_p);

    strcpy((char *)data_p->username, (char *)username);

    SSHD_PMGR_SendMsg(SSHD_MGR_IPC_CMD_GET_USER_PUBLIC_KEY_TYPE,
                      msg_p,
                      SSHD_MGR_GET_MSGBUFSIZE(SSHD_MGR_IPCMsg_SshdGetUserPublicKeyType_T),
                      SSHD_MGR_GET_MSGBUFSIZE(SSHD_MGR_IPCMsg_SshdGetUserPublicKeyType_T),
                      (UI32_T)FALSE);

    if (TRUE == (BOOL_T)SSHD_MGR_MSG_RETVAL(msg_p))
    {
        *key_type = data_p->type;
    }

    return SSHD_MGR_MSG_RETVAL(msg_p);
}

/* LOCAL SUBPROGRAM BODIES
 */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - SSHD_PMGR_SendMsg
 *-------------------------------------------------------------------------
 * PURPOSE : To send an IPC message.
 * INPUT   : cmd       - the PSEC message command.
 *           msg_p     - the buffer of the IPC message.
 *           req_size  - the size of PSEC request message.
 *           res_size  - the size of PSEC response message.
 *           ret_val   - the return value to set when IPC is failed.
 * OUTPUT  : msg_p     - the response message.
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
static void SSHD_PMGR_SendMsg(UI32_T cmd, SYSFUN_Msg_T *msg_p, UI32_T req_size, UI32_T res_size, UI32_T ret_val)
{
    UI32_T ret;

    msg_p->cmd = SYS_MODULE_SSH;
    msg_p->msg_size = req_size;

    SSHD_MGR_MSG_CMD(msg_p) = cmd;

    ret = SYSFUN_SendRequestMsg(ipcmsgq_handle,
                                msg_p,
                                SYSFUN_TIMEOUT_WAIT_FOREVER,
                                SYSFUN_SYSTEM_EVENT_IPCMSG,
                                res_size,
                                msg_p);

    /* If IPC is failed, set return value as ret_val.
     */
    if ((ret != SYSFUN_OK) || (SSHD_MGR_MSG_RETVAL(msg_p) == FALSE))
        SSHD_MGR_MSG_RETVAL(msg_p) = ret_val;
}

/* End of this file */

