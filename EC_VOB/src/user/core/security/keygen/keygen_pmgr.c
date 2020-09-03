/*-----------------------------------------------------------------------------
 * Module   : keygen_pmgr.c
 *-----------------------------------------------------------------------------
 * PURPOSE  : Provide APIs to access KEYGEN control functions.
 *-----------------------------------------------------------------------------
 * NOTES    :
 *
 *-----------------------------------------------------------------------------
 * HISTORY  : 09/27/2007 - Squid Ro, Create
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
#include "keygen_mgr.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void KEYGEN_PMGR_SendMsg(UI32_T cmd, SYSFUN_Msg_T *msg_p, UI32_T req_size, UI32_T res_size, UI32_T ret_val);

/* STATIC VARIABLE DECLARATIONS
 */
static SYSFUN_MsgQ_T ipcmsgq_handle;

/* MACRO FUNCTIONS DECLARACTION
 */

/* EXPORTED SUBPROGRAM BODIES
 */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - KEYGEN_PMGR_InitiateProcessResources
 *-------------------------------------------------------------------------
 * PURPOSE : Initiate resource used in the calling process.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE  - success
 *           FALSE - failure
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
BOOL_T KEYGEN_PMGR_InitiateProcessResources(void)
{
    if (SYSFUN_GetMsgQ(KEYGEN_MGR_IPCMSG_KEY, SYSFUN_MSGQ_BIDIRECTIONAL, &ipcmsgq_handle) != SYSFUN_OK)
    {
        printf("\r\n%s(): SYSFUN_GetMsgQ fail.", __FUNCTION__);
        return FALSE;
    }

    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - KEYGEN_PMGR_DeleteHostKeyPair
 *-------------------------------------------------------------------------
 * PURPOSE : Delete RSA/DSA public and private key pair of host key.
 * INPUT   : key_type - Type of host key.
 *              (KEY_TYPE_RSA, KEY_TYPE_DSA, KEY_TYPE_BOTH_RSA_AND_DSA)
 * OUTPUT  : None
 * RETURN  : TRUE  - success
 *           FALSE - failure
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
BOOL_T KEYGEN_PMGR_DeleteHostKeyPair(UI32_T key_type)
{
    UI8_T                               msgbuf[SYSFUN_SIZE_OF_MSG(KEYGEN_MGR_GET_MSGBUFSIZE(KEYGEN_MGR_IPCMsg_HostKeyType_T))];
    SYSFUN_Msg_T                        *msg_p = (SYSFUN_Msg_T *)msgbuf;
    KEYGEN_MGR_IPCMsg_HostKeyType_T     *data_p;

    data_p = KEYGEN_MGR_MSG_DATA(msg_p);
    data_p->key_type = key_type;

    KEYGEN_PMGR_SendMsg(KEYEGN_MGR_IPC_CMD_DELETE_HOST_KEY_PAIR,
                      msg_p,
                      KEYGEN_MGR_GET_MSGBUFSIZE(KEYGEN_MGR_IPCMsg_HostKeyType_T),
                      KEYGEN_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      (UI32_T)FALSE);

    return (BOOL_T)KEYGEN_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - KEYGEN_PMGR_GenerateHostKeyPair
 * ------------------------------------------------------------------------|
 * FUNCTION : Generate RSA/DSA public and private key pair of host key.
 * INPUT    : key_type - Type of host key.
 *              (KEY_TYPE_RSA, KEY_TYPE_DSA, KEY_TYPE_BOTH_RSA_AND_DSA)
 * OUTPUT	: None
 * RETURN   : TRUE/FALSE
 * NOTE		: This API is invoked in CLI command "ip ssh crypto host-key grnerate".
 * ------------------------------------------------------------------------*/
BOOL_T KEYGEN_PMGR_GenerateHostKeyPair(UI32_T key_type)
{
    UI8_T                               msgbuf[SYSFUN_SIZE_OF_MSG(KEYGEN_MGR_GET_MSGBUFSIZE(KEYGEN_MGR_IPCMsg_HostKeyType_T))];
    SYSFUN_Msg_T                        *msg_p = (SYSFUN_Msg_T *)msgbuf;
    KEYGEN_MGR_IPCMsg_HostKeyType_T     *data_p;

    data_p = KEYGEN_MGR_MSG_DATA(msg_p);
    data_p->key_type = key_type;

    KEYGEN_PMGR_SendMsg(KEYEGN_MGR_IPC_CMD_GENERATE_HOST_KEY_PAIR,
                      msg_p,
                      KEYGEN_MGR_GET_MSGBUFSIZE(KEYGEN_MGR_IPCMsg_HostKeyType_T),
                      KEYGEN_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      (UI32_T)FALSE);

    return (BOOL_T)KEYGEN_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - KEYGEN_PMGR_WriteHostKey2Flash
 * ------------------------------------------------------------------------|
 * FUNCTION : This function write host key from memory to flash.
 * INPUT    : None
 * OUTPUT	: None
 * RETURN   : TRUE/FALSE
 * NOTE		: This API is invoked in CLI command "ip ssh save host-key"
 * ------------------------------------------------------------------------*/
BOOL_T KEYGEN_PMGR_WriteHostKey2Flash(void)
{
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG(KEYGEN_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA())];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;

    KEYGEN_PMGR_SendMsg(KEYEGN_MGR_IPC_CMD_WRITE_HOST_KEY_2_FLASH,
                      msg_p,
                      KEYGEN_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      KEYGEN_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      (UI32_T)FALSE);

    return (BOOL_T)KEYGEN_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - KEYGEN_PMGR_GetServerCertificate
 * ------------------------------------------------------------------------|
 * FUNCTION : This function get server certificate.
 * INPUT    : None
 * OUTPUT	: cert
 * RETURN   : TRUE/FALSE
 * NOTE		: This API is invoked in HTTP module
 * ------------------------------------------------------------------------*/
BOOL_T KEYGEN_PMGR_GetServerCertificate(UI8_T *cert)
{
    UI8_T                                       msgbuf[SYSFUN_SIZE_OF_MSG(KEYGEN_MGR_GET_MSGBUFSIZE(KEYGEN_MGR_IPCMsg_GetServerCertificate_T))];
    SYSFUN_Msg_T                                *msg_p = (SYSFUN_Msg_T *)msgbuf;
    KEYGEN_MGR_IPCMsg_GetServerCertificate_T    *data_p;

    data_p = KEYGEN_MGR_MSG_DATA(msg_p);

    KEYGEN_PMGR_SendMsg(KEYEGN_MGR_IPC_CMD_GET_SERVER_CERTIFICATE,
                      msg_p,
                      KEYGEN_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      KEYGEN_MGR_GET_MSGBUFSIZE(KEYGEN_MGR_IPCMsg_GetServerCertificate_T),
                      (UI32_T)FALSE);

    if ( TRUE == (BOOL_T) KEYGEN_MGR_MSG_RETVAL(msg_p))
    {
        memcpy (cert, data_p->cert, SERVER_CERTIFICATE_FILE_LENGTH);
    }
    return (BOOL_T) KEYGEN_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - KEYGEN_PMGR_GetServerPrivateKey
 * ------------------------------------------------------------------------|
 * FUNCTION : This function get server private key which in certificate.
 * INPUT    : None
 * OUTPUT	: key
 * RETURN   : TRUE/FALSE
 * NOTE		: This API is invoked in HTTP module
 * ------------------------------------------------------------------------*/
BOOL_T KEYGEN_PMGR_GetServerPrivateKey(UI8_T *key)
{
    UI8_T                                   msgbuf[SYSFUN_SIZE_OF_MSG(KEYGEN_MGR_GET_MSGBUFSIZE(KEYGEN_MGR_IPCMsg_GetServerPrivateKey_T))];
    SYSFUN_Msg_T                            *msg_p = (SYSFUN_Msg_T *)msgbuf;
    KEYGEN_MGR_IPCMsg_GetServerPrivateKey_T *data_p;

    data_p = KEYGEN_MGR_MSG_DATA(msg_p);
    KEYGEN_PMGR_SendMsg(KEYEGN_MGR_IPC_CMD_GET_SERVER_PRIVATE_KEY,
                      msg_p,
                      KEYGEN_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      KEYGEN_MGR_GET_MSGBUFSIZE(KEYGEN_MGR_IPCMsg_GetServerPrivateKey_T),
                      (UI32_T)FALSE);

    if ( TRUE == (BOOL_T) KEYGEN_MGR_MSG_RETVAL(msg_p))
    {
        memcpy (key, data_p->server_private, sizeof (data_p->server_private));
    }

    return (BOOL_T) KEYGEN_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - KEYGEN_PMGR_SetNewCertificate
 * ------------------------------------------------------------------------|
 * FUNCTION : This function set server data include(certificate, private_key, pass_phase
 * INPUT    : server_certificate, server_private_key, server_pass_phase
 * OUTPUT	: None
 * RETURN   : TRUE/FALSE
 * NOTE		: This API is invoked in HTTP module
 * ------------------------------------------------------------------------*/
BOOL_T KEYGEN_PMGR_SetNewCertificate(UI8_T *server_certificate, UI8_T *server_private_key, UI8_T *server_pass_phrase)
{
    UI8_T               msgbuf[SYSFUN_SIZE_OF_MSG(KEYGEN_MGR_GET_MSGBUFSIZE(KEYGEN_MGR_IPCMsg_ServerData_T))];
    SYSFUN_Msg_T        *msg_p = (SYSFUN_Msg_T *)msgbuf;
    KEYGEN_MGR_IPCMsg_ServerData_T    *data_p;

    data_p = KEYGEN_MGR_MSG_DATA(msg_p);

    /* server certificate */
    memcpy(data_p->cert, server_certificate, SERVER_CERTIFICATE_FILE_LENGTH);

    /* server private key */
    memcpy(data_p->server_private, server_private_key, SERVER_PRIVATE_KEY_FILE_LENGTH);

    /* pass phrase */
    memcpy(data_p->pass_phrase, server_pass_phrase, SERVER_PASS_PHRASE_FILE_LENGTH);

    KEYGEN_PMGR_SendMsg(KEYEGN_MGR_IPC_CMD_WRITE_SERTIFICATE,
                      msg_p,
                      KEYGEN_MGR_GET_MSGBUFSIZE(KEYGEN_MGR_IPCMsg_ServerData_T),
                      KEYGEN_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      (UI32_T)FALSE);

    return (BOOL_T)KEYGEN_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - KEYGEN_PMGR_GetServerPassPhrase
 * ------------------------------------------------------------------------|
 * FUNCTION : This function get server pass phrase
 * INPUT    : None
 * OUTPUT	: passwd
 * RETURN   : TRUE/FALSE
 * NOTE		: This API is invoked in HTTP module
 * ------------------------------------------------------------------------*/
BOOL_T KEYGEN_PMGR_GetServerPassPhrase(UI8_T *passwd)
{
    UI8_T                                   msgbuf[SYSFUN_SIZE_OF_MSG(KEYGEN_MGR_GET_MSGBUFSIZE(KEYGEN_MGR_IPCMsg_GetServerPassPhrase_T))];
    SYSFUN_Msg_T                            *msg_p = (SYSFUN_Msg_T *)msgbuf;
    KEYGEN_MGR_IPCMsg_GetServerPassPhrase_T    *data_p;

    data_p = KEYGEN_MGR_MSG_DATA(msg_p);
    KEYGEN_PMGR_SendMsg(KEYEGN_MGR_IPC_CMD_GET_SERVER_PASS_PHRASE,
                      msg_p,
                      KEYGEN_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      KEYGEN_MGR_GET_MSGBUFSIZE(KEYGEN_MGR_IPCMsg_GetServerPassPhrase_T),
                      (UI32_T)FALSE);

    if ( TRUE == (BOOL_T) KEYGEN_MGR_MSG_RETVAL(msg_p))
    {
        memcpy (passwd, data_p->pass_phrase, sizeof (data_p->pass_phrase));
    }

    return (BOOL_T) KEYGEN_MGR_MSG_RETVAL(msg_p);
}

void *KEYGEN_PMGR_GetHttpsTempPublicKeyPair(int nExport, int nKeyLen)
{
    return NULL;
}

#if(SYS_CPNT_SSH2 == TRUE)
/*-------------------------------------------------------------------------|
 * ROUTINE NAME - KEYGEN_PMGR_GetHostPublicKey
 * ------------------------------------------------------------------------|
 * FUNCTION : Get host public key type.
 * INPUT    : None
 * OUTPUT	: rsa_key - pointer of buffer to storage rsa public key.
 *            dsa_key - pointer of buffer to storage dsa public key.
 * RETURN   : TRUE/FALSE
 * NOTE		: This API is invoked in CLI command "show public-key".
 * ------------------------------------------------------------------------*/
BOOL_T KEYGEN_PMGR_GetHostPublicKey(UI8_T *rsa_key, UI8_T *dsa_key)
{
    UI8_T                                   msgbuf[SYSFUN_SIZE_OF_MSG(KEYGEN_MGR_GET_MSGBUFSIZE(KEYGEN_MGR_IPCMsg_GetHostPublicKey_T))];
    SYSFUN_Msg_T                            *msg_p = (SYSFUN_Msg_T *)msgbuf;
    KEYGEN_MGR_IPCMsg_GetHostPublicKey_T    *data_p;

    data_p = KEYGEN_MGR_MSG_DATA(msg_p);

    KEYGEN_PMGR_SendMsg(KEYEGN_MGR_IPC_CMD_GET_HOST_PUBLIC_KEY,
                      msg_p,
                      KEYGEN_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      KEYGEN_MGR_GET_MSGBUFSIZE(KEYGEN_MGR_IPCMsg_GetHostPublicKey_T),
                      (UI32_T)FALSE);

    if ( TRUE == (BOOL_T) KEYGEN_MGR_MSG_RETVAL(msg_p))
    {
        memcpy (rsa_key, data_p->rsa_key, sizeof (data_p->rsa_key));
        memcpy (dsa_key, data_p->dsa_key, sizeof (data_p->dsa_key));
    }

    return (BOOL_T) KEYGEN_MGR_MSG_RETVAL(msg_p);
}

/* for SNMP
 */
/*-------------------------------------------------------------------------|
 * ROUTINE NAME - KEYGEN_PMGR_GetHostPublicKeyFingerPrint
 * ------------------------------------------------------------------------|
 * FUNCTION : Get host public key fingerprint.
 * INPUT    : None
 * OUTPUT	: rsa_sha1_fingerprint --  pointer of buffer to storage rsa public key sha1 fingerprint.
 *            rsa_md5_fingerprint  --  pointer of buffer to storage rsa public key md5 fingerprint.
 *            dsa_sha1_fingerprint --  pointer of buffer to storage dsa public key sha1 fingerprint.
 *            dsa_md5_fingerprint  --  pointer of buffer to storage dsa public key md5 fingerprint.
 * RETURN   : TRUE/FALSE
 * NOTE		: This API is invoked in UI.
 * ------------------------------------------------------------------------*/
BOOL_T KEYGEN_PMGR_GetHostPublicKeyFingerPrint(
    UI8_T   *rsa_sha1_fingerprint,  UI8_T   *rsa_md5_fingerprint,
    UI8_T   *dsa_sha1_fingerprint,  UI8_T   *dsa_md5_fingerprint)
{
    UI8_T                                   msgbuf[SYSFUN_SIZE_OF_MSG(KEYGEN_MGR_GET_MSGBUFSIZE(KEYGEN_MGR_IPCMsg_HostPbkeyFngrPrnt_T))];
    SYSFUN_Msg_T                            *msg_p = (SYSFUN_Msg_T *)msgbuf;
    KEYGEN_MGR_IPCMsg_HostPbkeyFngrPrnt_T   *data_p;

    data_p = KEYGEN_MGR_MSG_DATA(msg_p);
    KEYGEN_PMGR_SendMsg(KEYGEN_MGR_IPC_CMD_GET_HOST_PBKEY_FNGR_PRNT,
                      msg_p,
                      KEYGEN_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
                      KEYGEN_MGR_GET_MSGBUFSIZE(KEYGEN_MGR_IPCMsg_HostPbkeyFngrPrnt_T),
                      (UI32_T)FALSE);

    if ( TRUE == (BOOL_T) KEYGEN_MGR_MSG_RETVAL(msg_p))
    {
        memcpy (rsa_sha1_fingerprint, data_p->rsa_sha1_fp, sizeof (data_p->rsa_sha1_fp));
        memcpy (rsa_md5_fingerprint,  data_p->rsa_md5_fp,  sizeof (data_p->rsa_md5_fp));
        memcpy (dsa_sha1_fingerprint, data_p->dsa_sha1_fp, sizeof (data_p->dsa_sha1_fp));
        memcpy (dsa_md5_fingerprint,  data_p->dsa_md5_fp,  sizeof (data_p->dsa_md5_fp));
    }

    return (BOOL_T) KEYGEN_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - KEYGEN_PMGR_GetNextUserPublicKey
 * ------------------------------------------------------------------------|
 * FUNCTION : Get user's public key.
 * INPUT    : username   --  username.
 * OUTPUT	: username   --  Next username.
 *            rsa_key    --  pointer of buffer to storage rsa public key.
 *            dsa_key    --  pointer of buffer to storage dsa public key.key sha1 fingerprint.
 * RETURN   : TRUE/FALSE
 * NOTES    : This API is invoked in SSHD_MGR_GetNextUserPublicKey()
 *            Initiation username is empty string.
 * ------------------------------------------------------------------------*/
BOOL_T KEYGEN_PMGR_GetNextUserPublicKey(
    UI8_T   *username,  UI8_T   *rsa_key,   UI8_T   *dsa_key)
{
    UI8_T                           msgbuf[SYSFUN_SIZE_OF_MSG(KEYGEN_MGR_GET_MSGBUFSIZE(KEYGEN_MGR_IPCMsg_UserPbkey_T))];
    SYSFUN_Msg_T                    *msg_p = (SYSFUN_Msg_T *)msgbuf;
    KEYGEN_MGR_IPCMsg_UserPbkey_T   *data_p;

    data_p = KEYGEN_MGR_MSG_DATA(msg_p);

    memcpy (data_p->username, username, sizeof (data_p->username));

    KEYGEN_PMGR_SendMsg(KEYGEN_MGR_IPC_CMD_GET_NXT_USER_PBKEY,
                      msg_p,
                      KEYGEN_MGR_GET_MSGBUFSIZE(SYS_ADPT_MAX_USER_NAME_LEN+1),
                      KEYGEN_MGR_GET_MSGBUFSIZE(KEYGEN_MGR_IPCMsg_UserPbkey_T),
                      (UI32_T)FALSE);

    if ( TRUE == (BOOL_T) KEYGEN_MGR_MSG_RETVAL(msg_p))
    {
        memcpy (username, data_p->username, sizeof (data_p->username));
        memcpy (rsa_key,  data_p->rsa_key,  sizeof (data_p->rsa_key));
        memcpy (dsa_key,  data_p->dsa_key,  sizeof (data_p->dsa_key));
    }

    return (BOOL_T) KEYGEN_MGR_MSG_RETVAL(msg_p);
}
#endif /* #if (SYS_CPNT_SSH2 == TRUE) */


/* LOCAL SUBPROGRAM BODIES
 */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - KEYGEN_PMGR_SendMsg
 *-------------------------------------------------------------------------
 * PURPOSE : To send an IPC message.
 * INPUT   : cmd       - the KEYGEN message command.
 *           msg_p     - the buffer of the IPC message.
 *           req_size  - the size of KEYGEN request message.
 *           res_size  - the size of KEYGEN response message.
 *           ret_val   - the return value to set when IPC is failed.
 * OUTPUT  : msg_p     - the response message.
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
static void KEYGEN_PMGR_SendMsg(UI32_T cmd, SYSFUN_Msg_T *msg_p, UI32_T req_size, UI32_T res_size, UI32_T ret_val)
{
    UI32_T ret;

    msg_p->cmd = SYS_MODULE_KEYGEN;
    msg_p->msg_size = req_size;

    KEYGEN_MGR_MSG_CMD(msg_p) = cmd;

    ret = SYSFUN_SendRequestMsg(ipcmsgq_handle,
                                msg_p,
                                SYSFUN_TIMEOUT_WAIT_FOREVER,
                                SYSFUN_SYSTEM_EVENT_IPCMSG,
                                res_size,
                                msg_p);

    /* If IPC is failed, set return value as ret_val.
     */
    if ((ret != SYSFUN_OK) || (KEYGEN_MGR_MSG_RETVAL(msg_p) == FALSE))
        KEYGEN_MGR_MSG_RETVAL(msg_p) = ret_val;
}

/* FUNCTION NAME : KEYGEN_PMGR_CheckSshdHostkey
 * PURPOSE:
 *      This function to check host key is exist or not.
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN:
 *      TRUE  - Host key is exist.
 *      FALSE - Host key not exist.
 *
 * NOTES:
 *      This API is invoked in SSHD_MGR_SetSshdStatus.
 */
BOOL_T KEYGEN_PMGR_CheckSshdHostkey()
{
    /* not ready...
     */
    return FALSE;
}
/* End of this file */

