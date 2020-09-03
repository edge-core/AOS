/* MODULE NAME:  sshd_mgr.c
* PURPOSE:
*   Initialize the resource and provide some functions for the sshd module.
*
* NOTES:
*
* History:
*       Date          -- Modifier,  Reason
*     2003-04-02      -- Isiah , created.
*
* Copyright(C)      Accton Corporation, 2003
*/



/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sshd_task.h"
#include "sshd_mgr.h"
#include "sshd_om.h"
#include "sysfun.h"
#include "keygen_type.h"
#include "keygen_mgr.h"
#include "fs_type.h"
#include "userauth.h"
#include "backdoor_mgr.h"
#include "l_threadgrp.h"
#include "userauth_pmgr.h"

#include "bufaux.h"
#include "uuencode.h"
#include "key.h"
#include "xmalloc.h"

#if(SYS_CPNT_SSH2_TFTP_INFO_IN_CFGDB == TRUE)
    #include "leaf_es3626a.h"
    #include "cfgdb_mgr.h"
#endif /* #if(SYS_CPNT_SSH2_TFTP_INFO_IN_CFGDB == TRUE) */

#include "xfer_pmgr.h"
#include "buffer_mgr.h"
#include "cli_proc_comm.h"

#include "openssl/bio.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
#if(SYS_CPNT_SSH2_TFTP_INFO_IN_CFGDB == TRUE)
typedef struct
{
    UI32_T  tftp_server_ip;
    UI8_T   file_name[MAXSIZE_tftpSrcFile + 1];
    UI8_T   Reserved[28];
}SSHD_MGR_LastSshdInfo_T;
#endif /* #if(SYS_CPNT_SSH2_TFTP_INFO_IN_CFGDB == TRUE) */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static BOOL_T SSHD_MGR_do_convert_from_ssh2();

/* STATIC VARIABLE DECLARATIONS
 */
SYSFUN_DECLARE_CSC

static  UI8_T   public_key[USER_DSA_PUBLIC_KEY_FILE_LENGTH + 1];
#if(SYS_CPNT_SSH2_TFTP_INFO_IN_CFGDB == TRUE)
#define SSHD_MGR_NUM_OF_LAST_SSHD_INFO    3
static SSHD_MGR_LastSshdInfo_T  last_sshd_info[SSHD_MGR_NUM_OF_LAST_SSHD_INFO];//0:publicKey,1,2:Reserved
static UI32_T                   last_sshd_info_session_handler;
#endif /* #if(SYS_CPNT_SSH2_TFTP_INFO_IN_CFGDB == TRUE) */

static BOOL_T SSHD_MGR_CopyUserPublicKey(UI8_T *key_buffer);

#ifndef UNIT_TEST
/* EXPORTED SUBPROGRAM BODIES
 */
/*------------------------------------------------------------------------------
 * ROUTINE NAME : SSH_MGR_HandleIPCReqMsg
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Handle the ipc request message for ssh mgr.
 * INPUT:
 *    ipcmsg_p  --  input request ipc message buffer
 *
 * OUTPUT:
 *    ipcmsg_p  --  output response ipc message buffer
 *
 * RETURN:
 *    TRUE  - There is a response need to send.
 *    FALSE - There is no response to send.
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T SSHD_MGR_HandleIPCReqMsg(SYSFUN_Msg_T *ipcmsg_p)
{
    UI32_T  cmd;

    if(ipcmsg_p == NULL)
        return FALSE;

    /* Every ipc request will fail when operating mode is transition mode
     */
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_TRANSITION_MODE)
    {
        SSHD_MGR_MSG_RETVAL(ipcmsg_p) = FALSE;
        ipcmsg_p->msg_size = SSHD_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
        return TRUE;
    }

    switch((cmd = SSHD_MGR_MSG_CMD(ipcmsg_p)))
    {
        case SSHD_MGR_IPC_CMD_DEL_USER_PBKEY:
        {
            SSHD_MGR_IPCMsg_DelUserPbKey_T  *data_p = SSHD_MGR_MSG_DATA(ipcmsg_p);
            SSHD_MGR_MSG_RETVAL(ipcmsg_p) = SSHD_MGR_DeleteUserPublicKey(data_p->username, data_p->key_type);
            ipcmsg_p->msg_size = SSHD_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case SSHD_MGR_IPC_CMD_GET_AUTHEN_RETRIES:
        {
            SSHD_MGR_IPCMsg_AuthenRetries_T *data_p = SSHD_MGR_MSG_DATA(ipcmsg_p);
            data_p->retries = SSHD_MGR_GetAuthenticationRetries();
            SSHD_MGR_MSG_RETVAL(ipcmsg_p) = TRUE;
            ipcmsg_p->msg_size = SSHD_MGR_GET_MSGBUFSIZE(SSHD_MGR_IPCMsg_AuthenRetries_T);
            break;
        }

        case SSHD_MGR_IPC_CMD_GET_NEGO_TIMEOUT:
        {
            SSHD_MGR_IPCMsg_NegoTimeout_T   *data_p = SSHD_MGR_MSG_DATA(ipcmsg_p);
            data_p->timeout = SSHD_MGR_GetNegotiationTimeout();
            SSHD_MGR_MSG_RETVAL(ipcmsg_p) = TRUE;
            ipcmsg_p->msg_size = SSHD_MGR_GET_MSGBUFSIZE(SSHD_MGR_IPCMsg_NegoTimeout_T);
            break;
        }

        case SSHD_MGR_IPC_CMD_GET_NEXT_CONN_ENTRY:
        {
            SSHD_MGR_IPCMsg_ConnEntry_T *data_p = SSHD_MGR_MSG_DATA(ipcmsg_p);
            SSHD_MGR_MSG_RETVAL(ipcmsg_p) = SSHD_MGR_GetNextSshConnectionEntry(&data_p->cid, &data_p->info);
            ipcmsg_p->msg_size = SSHD_MGR_GET_MSGBUFSIZE(SSHD_MGR_IPCMsg_ConnEntry_T);
            break;
        }

        case SSHD_MGR_IPC_CMD_GET_NEXT_USER_PBKEY:
        {
            SSHD_MGR_IPCMsg_UserPbKey_T *data_p = SSHD_MGR_MSG_DATA(ipcmsg_p);
            SSHD_MGR_MSG_RETVAL(ipcmsg_p) = SSHD_MGR_GetNextUserPublicKey(data_p->username, data_p->rsa_key, data_p->dsa_key);
            ipcmsg_p->msg_size = SSHD_MGR_GET_MSGBUFSIZE(SSHD_MGR_IPCMsg_UserPbKey_T);
            break;
        }

        case SSHD_MGR_IPC_CMD_GET_SERVER_KEYSIZE:
        {
            SSHD_MGR_IPCMsg_SvrKSize_T  *data_p = SSHD_MGR_MSG_DATA(ipcmsg_p);
            SSHD_MGR_MSG_RETVAL(ipcmsg_p) = SSHD_MGR_GetServerKeySize(&data_p->key_size);
            ipcmsg_p->msg_size = SSHD_MGR_GET_MSGBUFSIZE(SSHD_MGR_IPCMsg_SvrKSize_T);
            break;
        }

        case SSHD_MGR_IPC_CMD_GET_SSH_CONN_NAME:
        {
            SSHD_MGR_IPCMsg_SshdConnName_T  *data_p = SSHD_MGR_MSG_DATA(ipcmsg_p);
            SSHD_MGR_MSG_RETVAL(ipcmsg_p) = SSHD_MGR_GetSshConnectionUsername(data_p->cid, data_p->username);
            ipcmsg_p->msg_size = SSHD_MGR_GET_MSGBUFSIZE(SSHD_MGR_IPCMsg_SshdConnName_T);
            break;
        }

        case SSHD_MGR_IPC_CMD_GET_SSH_CONN_PRIV:
        {
            SSHD_MGR_IPCMsg_SshdConnPriv_T  *data_p = SSHD_MGR_MSG_DATA(ipcmsg_p);
            SSHD_MGR_MSG_RETVAL(ipcmsg_p) = SSHD_MGR_GetSshConnectionPrivilege(data_p->cid, &data_p->privilege);
            ipcmsg_p->msg_size = SSHD_MGR_GET_MSGBUFSIZE(SSHD_MGR_IPCMsg_SshdConnPriv_T);
            break;
        }

        case SSHD_MGR_IPC_CMD_GET_SSHD_STATUS:
        {
            SSHD_MGR_IPCMsg_SshdState_T *data_p = SSHD_MGR_MSG_DATA(ipcmsg_p);
            data_p->state= SSHD_MGR_GetSshdStatus();
            SSHD_MGR_MSG_RETVAL(ipcmsg_p) = TRUE;
            ipcmsg_p->msg_size = SSHD_MGR_GET_MSGBUFSIZE(SSHD_MGR_IPCMsg_SshdState_T);
            break;
        }

        case SSHD_MGR_IPC_CMD_GET_SERVER_VERSION:
        {
            SSHD_MGR_IPCMsg_SvrVersion_T    *data_p = SSHD_MGR_MSG_DATA(ipcmsg_p);
            SSHD_MGR_MSG_RETVAL(ipcmsg_p) = SSHD_MGR_GetSshServerVersion(&data_p->major, &data_p->minor);
            ipcmsg_p->msg_size = SSHD_MGR_GET_MSGBUFSIZE(SSHD_MGR_IPCMsg_SvrVersion_T);
            break;
        }

        case SSHD_MGR_IPC_CMD_GET_USER_PBKEY:
        {
            SSHD_MGR_IPCMsg_UserPbKey_T *data_p = SSHD_MGR_MSG_DATA(ipcmsg_p);
            SSHD_MGR_MSG_RETVAL(ipcmsg_p) = SSHD_MGR_GetUserPublicKey(data_p->username, data_p->rsa_key, data_p->dsa_key);
            ipcmsg_p->msg_size = SSHD_MGR_GET_MSGBUFSIZE(SSHD_MGR_IPCMsg_UserPbKey_T);
            break;
        }

        case SSHD_MGR_IPC_CMD_SET_AUTHEN_RETRIES:
        {
            SSHD_MGR_IPCMsg_AuthenRetries_T *data_p = SSHD_MGR_MSG_DATA(ipcmsg_p);
            SSHD_MGR_MSG_RETVAL(ipcmsg_p) = SSHD_MGR_SetAuthenticationRetries(data_p->retries);
            ipcmsg_p->msg_size = SSHD_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case SSHD_MGR_IPC_CMD_SET_CID_AND_TNSHID:
        {
            SSHD_MGR_IPCMsg_CidAndTnshid_T *data_p = SSHD_MGR_MSG_DATA(ipcmsg_p);
            SSHD_MGR_MSG_RETVAL(ipcmsg_p) = SSHD_MGR_SetConnectionIDAndTnshID(data_p->tnsh_port, data_p->tid, data_p->cid);
            ipcmsg_p->msg_size = SSHD_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case SSHD_MGR_IPC_CMD_SET_NEGO_TIMEOUT:
        {
            SSHD_MGR_IPCMsg_NegoTimeout_T   *data_p = SSHD_MGR_MSG_DATA(ipcmsg_p);
            SSHD_MGR_MSG_RETVAL(ipcmsg_p) = SSHD_MGR_SetNegotiationTimeout(data_p->timeout);
            ipcmsg_p->msg_size = SSHD_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case SSHD_MGR_IPC_CMD_SET_SERVER_KEYSIZE:
        {
            SSHD_MGR_IPCMsg_SvrKSize_T  *data_p = SSHD_MGR_MSG_DATA(ipcmsg_p);
            SSHD_MGR_MSG_RETVAL(ipcmsg_p) = SSHD_MGR_SetServerKeySize(data_p->key_size);
            ipcmsg_p->msg_size = SSHD_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case SSHD_MGR_IPC_CMD_SET_SSHD_STATUS:
        {
            SSHD_MGR_IPCMsg_SshdState_T *data_p = SSHD_MGR_MSG_DATA(ipcmsg_p);
            SSHD_MGR_MSG_RETVAL(ipcmsg_p) = SSHD_MGR_SetSshdStatus(data_p->state);
            ipcmsg_p->msg_size = SSHD_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case SSHD_MGR_IPC_CMD_GET_GENERATE_HOST_KEY_STATUS:
        {
            SSHD_MGR_IPCMsg_SshdGenHostKeyState_T *data_p = SSHD_MGR_MSG_DATA(ipcmsg_p);
            SSHD_MGR_MSG_RETVAL(ipcmsg_p) = SSHD_MGR_GetGenerateHostKeyStatus(&data_p->state);
            ipcmsg_p->msg_size = SSHD_MGR_GET_MSGBUFSIZE(SSHD_MGR_IPCMsg_SshdGenHostKeyState_T);
            break;
        }

        case SSHD_MGR_IPC_CMD_GET_GENERATE_HOST_KEY_ACTION:
        {
            SSHD_MGR_IPCMsg_SshdGenHostKeyAction_T *data_p = SSHD_MGR_MSG_DATA(ipcmsg_p);
            SSHD_MGR_MSG_RETVAL(ipcmsg_p) = SSHD_MGR_GetGenerateHostKeyAction(&data_p->action);
            ipcmsg_p->msg_size = SSHD_MGR_GET_MSGBUFSIZE(SSHD_MGR_IPCMsg_SshdGenHostKeyAction_T);
            break;
        }

        case SSHD_MGR_IPC_CMD_GET_WRITE_HOST_KEY_2_FLASH_STATUS:
        {
            SSHD_MGR_IPCMsg_SshdWriteHK2FState_T *data_p = SSHD_MGR_MSG_DATA(ipcmsg_p);
            SSHD_MGR_MSG_RETVAL(ipcmsg_p) = SSHD_MGR_GetWriteHostKey2FlashStatus(&data_p->state);
            ipcmsg_p->msg_size = SSHD_MGR_GET_MSGBUFSIZE(SSHD_MGR_IPCMsg_SshdWriteHK2FState_T);
            break;
        }

        case SSHD_MGR_IPC_CMD_GET_WRITE_HOST_KEY_2_FLASH_ACTION:
        {
            SSHD_MGR_IPCMsg_SshdWriteHK2FAction_T *data_p = SSHD_MGR_MSG_DATA(ipcmsg_p);
            SSHD_MGR_MSG_RETVAL(ipcmsg_p) = SSHD_MGR_GetWriteHostKey2FlashAction(&data_p->action);
            ipcmsg_p->msg_size = SSHD_MGR_GET_MSGBUFSIZE(SSHD_MGR_IPCMsg_SshdWriteHK2FAction_T);
            break;
        }

        case SSHD_MGR_IPC_CMD_ASYNC_WRITE_HOST_KEY_2_FLASH:
        {
            SSHD_MGR_MSG_RETVAL(ipcmsg_p) = SSHD_MGR_AsyncWriteHostKey2Flash();
            ipcmsg_p->msg_size = SSHD_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case SSHD_MGR_IPC_CMD_ASYNC_GENERATE_HOST_KEY_PAIR:
        {
            SSHD_MGR_IPCMsg_SshdAsyncGenHKPair_T  *data_p = SSHD_MGR_MSG_DATA(ipcmsg_p);
            SSHD_MGR_MSG_RETVAL(ipcmsg_p) = SSHD_MGR_AsyncGenerateHostKeyPair(data_p->type);
            ipcmsg_p->msg_size = SSHD_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case SSHD_MGR_IPC_CMD_GET_CONN_ENTRY:
        {
            SSHD_MGR_IPCMsg_ConnEntry_T *data_p = SSHD_MGR_MSG_DATA(ipcmsg_p);
            SSHD_MGR_MSG_RETVAL(ipcmsg_p) =
                SSHD_MGR_GetSshConnectionEntry(data_p->cid, &data_p->info);
            ipcmsg_p->msg_size = SSHD_MGR_GET_MSGBUFSIZE(SSHD_MGR_IPCMsg_ConnEntry_T);
            break;
        }

        case SSHD_MGR_IPC_CMD_ASYNC_DELETE_USER_PUBLIC_KEY:
        {
            SSHD_MGR_IPCMsg_DelUserPbKey_T  *data_p = SSHD_MGR_MSG_DATA(ipcmsg_p);
            SSHD_MGR_MSG_RETVAL(ipcmsg_p) = SSHD_MGR_AsyncDeleteUserPublicKey(data_p->username, data_p->key_type);
            ipcmsg_p->msg_size = SSHD_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case SSHD_MGR_IPC_CMD_GET_DELETE_USER_PUBLIC_KEY_ACTION:
        {
            SSHD_MGR_IPCMsg_SshdDeleteUPKeyAction_T  *data_p = SSHD_MGR_MSG_DATA(ipcmsg_p);
            SSHD_MGR_MSG_RETVAL(ipcmsg_p) = SSHD_MGR_GetDeleteUserPublicKeyAction(&data_p->action);
            ipcmsg_p->msg_size = SSHD_MGR_GET_MSGBUFSIZE(SSHD_MGR_IPCMsg_SshdDeleteUPKeyAction_T);
            break;
        }

        case SSHD_MGR_IPC_CMD_SET_USER_PUBLIC_KEY:
        {
            SSHD_MGR_IPCMsg_SshdSetUserPublicKey_T  *data_p = SSHD_MGR_MSG_DATA(ipcmsg_p);
            SSHD_MGR_MSG_RETVAL(ipcmsg_p) = SSHD_MGR_SetUserPublicKey(data_p->username, data_p->type);
            ipcmsg_p->msg_size = SSHD_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }
        case SSHD_MGR_IPC_CMD_COPY_USER_PUBLIC_KEY:
        {
            SSHD_MGR_IPCMsg_SshdSetUserPublicKey_T  *data_p = SSHD_MGR_MSG_DATA(ipcmsg_p);
            SSHD_MGR_MSG_RETVAL(ipcmsg_p) = SSHD_MGR_CopyUserPublicKey(BUFFER_MGR_GetPtr(data_p->key_buf_offset));
            ipcmsg_p->msg_size = SSHD_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }
        case SSHD_MGR_IPC_CMD_GET_USER_PUBLIC_KEY_TYPE:
        {
            SSHD_MGR_IPCMsg_SshdGetUserPublicKeyType_T *data_p = SSHD_MGR_MSG_DATA(ipcmsg_p);
            SSHD_MGR_MSG_RETVAL(ipcmsg_p) = SSHD_MGR_GetUserPublicKeyType(data_p->username, &data_p->type);
            ipcmsg_p->msg_size = SSHD_MGR_GET_MSGBUFSIZE(SSHD_MGR_IPCMsg_SshdGetUserPublicKeyType_T);
            break;
        }

        default:
        {
            SSHD_MGR_MSG_RETVAL(ipcmsg_p) = FALSE;
            SYSFUN_Debug_Printf("*** %s(): Invalid cmd.\n", __FUNCTION__);
            return FALSE;
        }
    } /* switch ipcmsg_p->cmd */

    if (SSHD_MGR_MSG_RETVAL(ipcmsg_p) == FALSE)
    {
        SYSFUN_Debug_Printf("*** %s(): [cmd: %ld] failed\n", __FUNCTION__, cmd);
    }

    return TRUE;
}

/* FUNCTION NAME:  SSHD_MGR_Init
 * PURPOSE:
 *          Initiate the semaphore for SSHD objects
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          This function is invoked in SSHD_INIT_Initiate_System_Resources.
 */
BOOL_T SSHD_MGR_Init(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */

    /* BODY */
#if(SYS_CPNT_SSH2_TFTP_INFO_IN_CFGDB == TRUE)
    memset(&last_sshd_info, 0, SSHD_MGR_NUM_OF_LAST_SSHD_INFO * sizeof(SSHD_MGR_LastSshdInfo_T));
    last_sshd_info_session_handler = 0;
#endif /* #if(SYS_CPNT_SSH2_TFTP_INFO_IN_CFGDB == TRUE) */
    if ( SSHD_OM_Init() == FALSE )
    {
        return FALSE;
    }

    return TRUE;
}



/* FUNCTION NAME:  SSHD_MGR_EnterMasterMode
 * PURPOSE:
 *          This function initiates all the system database, and also configures
 *          the switch to the initiation state based on the specified "System Boot
 *          Configruation File". After that, the SSHD subsystem will enter the
 *          Master Operation mode.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          1. If "System Boot Configruation File" does not exist, the system database and
 *              switch will be initiated to the factory default value.
 *          2. SSHD will handle network requests only when this subsystem
 *              is in the Master Operation mode
 *          3. This function is invoked in SSHD_INIT_EnterMasterMode.
 */
BOOL_T SSHD_MGR_EnterMasterMode(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
#if 0
    SSHD_Session_Record_T   *session_record;
/*  SSHD_Session_Connection_T   *connection_record;*/
    SSHD_Context_T      *ssh_context;
    UI32_T  i;
#endif

    /* BODY */

#if(SYS_CPNT_SSH2_TFTP_INFO_IN_CFGDB == TRUE)
    BOOL_T  need_to_sync;

    CFGDB_PMGR_Open(CFGDB_MGR_SECTION_ID_SSHD_1,
                   sizeof(SSHD_MGR_LastSshdInfo_T),
                   SSHD_MGR_NUM_OF_LAST_SSHD_INFO,
                   &last_sshd_info_session_handler,
                   CFGDB_MGR_SECTION_TYPE_EXTERNAL_AND_GLOBAL,
                   &need_to_sync);
    if (need_to_sync)
    {
        CFGDB_PMGR_SyncSection(last_sshd_info_session_handler, last_sshd_info );
    }
    else
    {
        CFGDB_PMGR_ReadSection(last_sshd_info_session_handler, last_sshd_info);
    }
#endif /* #if(SYS_CPNT_SSH2_TFTP_INFO_IN_CFGDB == TRUE) */
    SSHD_OM_SetSshdStatus(SSHD_DEFAULT_STATE);
    SSHD_OM_SetSshdPort(SSHD_DEFAULT_PORT_NUMBER);
    SSHD_OM_SetAuthenticationRetries(SSHD_DEFAULT_AUTHENTICATION_RETRIES);
    SSHD_OM_SetNegotiationTimeout(SSHD_DEFAULT_NEGOTIATION_TIMEOUT);
    SSHD_OM_SetSshServerVersion(2,0);
    SSHD_OM_SetSshRsaAuthenticationStatus(SYS_DFLT_PASSWORD_AUTHENTICATION_STATE);
    SSHD_OM_SetSshPubkeyAuthenticationStatus(SYS_DFLT_PUBKEY_AUTHENTICATION_STATE);
    SSHD_OM_SetSshPasswordAuthenticationStatus(SYS_DFLT_PASSWORD_AUTHENTICATION_STATE);

/*isiah.2003-08-11*/
    SSLeay_add_all_algorithms();

#if 0
    /*
     */
    SSHD_OM_ResetSshdSessionRecord();
    SSHD_OM_SetAuthenticationRetries(SSHD_DEFAULT_AUTHENTICATION_RETRIES);
    SSHD_OM_SetNegotiationTimeout(SSHD_DEFAULT_NEGOTIATION_TIMEOUT);
    SSHD_OM_SetSshServerVersion(1,5);
    session_record = SSHD_OM_GetSshdSessionRecord();
    for ( i=1 ; i<=SSHD_DEFAULT_MAX_SESSION_NUMBER ; i++ )
    {
        ssh_context = &(session_record->connection[i].ssh_context);
        ssh_context->cipher = malloc(sizeof(SSHD_Cipher_T));
        if ( !ssh_context->cipher )
        {

            return FALSE;
        }
        memset(ssh_context->cipher,0,sizeof(SSHD_Cipher_T));
    }
#endif

//
/*  SSHD_OM_SetOpMode(SYS_TYPE_STACKING_MASTER_MODE);*/

/* set mgr in master mode */
    SYSFUN_ENTER_MASTER_MODE();

    return TRUE;

}



/* FUNCTION NAME:  SSHD_MGR_EnterTransitionMode
 * PURPOSE:
 *          This function forces this subsystem enter the Transition Operation mode.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_MGR_EnterTransitionMode(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
/*    SYS_TYPE_Stacking_Mode_T opmode;*/
#if 0
    UI32_T  i;
    SSHD_Session_Record_T   *session_record;
    SSHD_Context_T      *ssh_context;
#endif

    /* BODY */
    SYSFUN_ENTER_TRANSITION_MODE();

#if 0
        session_record = SSHD_OM_GetSshdSessionRecord();


        for ( i=1 ; i<=SSHD_DEFAULT_MAX_SESSION_NUMBER ; i++ )
        {
            while(1)
            {
                if ( (session_record->connection[i].keepalive == 0) && (session_record->connection[i].tid == 0) )
                {
                    ssh_context = &(session_record->connection[i].ssh_context);
                if ( ssh_context->cipher != NULL )
                {
                    free(ssh_context->cipher);
                    ssh_context->cipher = NULL;
                }
                    break;
                }
                else
                {
                SYSFUN_Sleep(10);
                }
            }
        }
#endif

    return TRUE;
}



/* FUNCTION NAME:  SSHD_MGR_EnterSlaveMode
 * PURPOSE:
 *          This function forces this subsystem enter the Slave Operation mode.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          none.
 * NOTES:
 *          In Slave Operation mode, any network requests
 *          will be ignored.
 */
void SSHD_MGR_EnterSlaveMode(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */

    /* BODY */
    SYSFUN_ENTER_SLAVE_MODE();

    return;

}



/* FUNCTION NAME : SSHD_MGR_SetTransitionMode
 * PURPOSE:
 *      Set transition mode.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      None.
 */
void SSHD_MGR_SetTransitionMode(void)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLES DEFINITION
     */

    /* BODY */

    /* set transition flag to prevent calling request */
    SYSFUN_SET_TRANSITION_MODE();
}



/* FUNCTION NAME : SSHD_MGR_GetOperationMode
 * PURPOSE:
 *      Get current sshd operation mode (master / slave / transition).
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *          SYS_TYPE_Stacking_Mode_T - opmode.
 *
 * NOTES:
 *      None.
 */
SYS_TYPE_Stacking_Mode_T SSHD_MGR_GetOperationMode(void)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLES DEFINITION
     */

    /* BODY */
    return ( SYSFUN_GET_CSC_OPERATING_MODE() );
}

#if(SYS_CPNT_SSH2_TFTP_INFO_IN_CFGDB == TRUE)
/*------------------------------------------------------------------------
 * ROUTINE NAME - SSHD_MGR_SetLastPublicKeyIp
 *------------------------------------------------------------------------
 * FUNCTION: Set the last ip for publickey
 * INPUT   : UI32_T  tftp_server_ip
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    :
 *------------------------------------------------------------------------*/
BOOL_T SSHD_MGR_SetLastPublicKeyIp(UI32_T tftp_server_ip)
{
    if (SYS_TYPE_STACKING_SLAVE_MODE == SYSFUN_GET_CSC_OPERATING_MODE())
    {
       return FALSE;
    }

    last_sshd_info[0].tftp_server_ip = tftp_server_ip;
    CFGDB_PMGR_WriteSection(last_sshd_info_session_handler, last_sshd_info);

    return TRUE;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - SSHD_MGR_GetLastPublicKeyIp
 *------------------------------------------------------------------------
 * FUNCTION: Get the last ip for publickey
 * INPUT   : None
 * OUTPUT  : UI32_T  *tftp_server_ip_p
 * RETURN  : TRUE/FALSE
 * NOTE    :
 *------------------------------------------------------------------------*/
BOOL_T SSHD_MGR_GetLastPublicKeyIp(UI32_T *tftp_server_ip_p)
{
    if (SYS_TYPE_STACKING_SLAVE_MODE == SYSFUN_GET_CSC_OPERATING_MODE())
    {
       return FALSE;
    }

    *tftp_server_ip_p = last_sshd_info[0].tftp_server_ip;
    return TRUE;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - SSHD_MGR_SetLastPublicKeyFileName
 *------------------------------------------------------------------------
 * FUNCTION: Set the last filename for publickey
 * INPUT   : UI8_T *file_name_p
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : file name length is restricted between MINSIZE_tftpSrcFile and
 *           MAXSIZE_tftpSrcFile
 *------------------------------------------------------------------------*/
BOOL_T SSHD_MGR_SetLastPublicKeyFileName(UI8_T *file_name_p)
{
    if (SYS_TYPE_STACKING_SLAVE_MODE == SYSFUN_GET_CSC_OPERATING_MODE())
    {
       return FALSE;
    }

    if(NULL == *file_name_p)
    {
        memset(last_sshd_info[0].file_name, 0, sizeof(last_sshd_info[0].file_name));
    }
    else if (strlen(file_name_p) > MAXSIZE_tftpSrcFile)
    {
        strncpy(last_sshd_info[0].file_name, file_name_p, strlen(file_name_p));
    }
    else
    {
        strcpy (last_sshd_info[0].file_name, file_name_p);
    }
    CFGDB_PMGR_WriteSection(last_sshd_info_session_handler, last_sshd_info);

    return TRUE;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - SSHD_MGR_GetLastPublicKeyFileName
 *------------------------------------------------------------------------
 * FUNCTION: Get the last filename for publickey
 * INPUT   : None
 * OUTPUT  : UI8_T *file_name_p
 * RETURN  : TRUE/FALSE
 * NOTE    : file name length is restricted between MINSIZE_tftpSrcFile and
 *           MAXSIZE_tftpSrcFile
 *------------------------------------------------------------------------*/
BOOL_T SSHD_MGR_GetLastPublicKeyFileName(UI8_T *file_name_p)
{
    if (SYS_TYPE_STACKING_SLAVE_MODE == SYSFUN_GET_CSC_OPERATING_MODE())
    {
       return FALSE;
    }

    strcpy(file_name_p, last_sshd_info[0].file_name);

    return TRUE;
}
#endif /* #if(SYS_CPNT_SSH2_TFTP_INFO_IN_CFGDB == TRUE) */

/* FUNCTION NAME:  SSHD_MGR_SetSshdStatus
 * PURPOSE:
 *          This function set sshd state.
 *
 * INPUT:
 *          SSHD_State_T - SSHD status.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          none.
 * NOTES:
 *          This function maybe invoked in CLI command 'ip ssh server'.
 */
BOOL_T SSHD_MGR_SetSshdStatus (SSHD_State_T state)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T ret = FALSE;
    SSHD_State_T    sshd_state = SSHD_STATE_DISABLED;

    /* BODY */
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }
    else
    {
        if( state != SSHD_STATE_DISABLED )
        {
            if( KEYGEN_MGR_CheckSshdHostkey() == TRUE )
            {
                sshd_state = SSHD_OM_GetSshdStatus();
                if( sshd_state == SSHD_STATE_DISABLED )
                {
                    SSHD_OM_SetSshdStatus(state);
                    ret = TRUE;
                }
                else
                {
                    ret = TRUE;
                }
            }
            else
            {
                ret = FALSE;
            }
            return ret;
        }
        else
        {

            SSHD_OM_SetSshdStatus(state);

            ret = TRUE;
            return ret;
        }
    }
}



/* FUNCTION NAME:  SSHD_MGR_GetSshdStatus
 * PURPOSE:
 *          This function get sshd state.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          SSHD_State_T - SSHD status.
 * NOTES:
 *          This function maybe invoked in CLI command 'show ip ssh'.
 */
SSHD_State_T SSHD_MGR_GetSshdStatus(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    SSHD_State_T    state = SSHD_STATE_DISABLED;



    /* BODY */
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {

    }
    else
    {
        state = SSHD_OM_GetSshdStatus();
    }

    return ( state );
}



/* FUNCTION NAME:  SSHD_MGR_SetSshdPort
 * PURPOSE:
 *          This function set sshd port number.
 *
 * INPUT:
 *          UI32_T - SSHD port number.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          none.
 * NOTES:
 *          This function maybe invoked in CLI command 'ip ssh port'.
 */
BOOL_T SSHD_MGR_SetSshdPort (UI32_T port)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  is_changed;

    /* BODY */
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }
    else
    {
        is_changed = SSHD_OM_SetSshdPort(port);


        if ( is_changed == TRUE )
        {
            /* close socket */
//            SSHD_TASK_PortChanged();
        }
    }

    return TRUE;
}



/* FUNCTION NAME:  SSHD_MGR_GetSshdPort
 * PURPOSE:
 *          This function get sshd port number.
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          UI32_T - SSHD port value.
 * NOTES:
 *          default is tcp/22.
 */
UI32_T SSHD_MGR_GetSshdPort(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    UI32_T  port = 0;

    /* BODY */
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {

    }
    else
    {
        port = SSHD_OM_GetSshdPort();
    }

    return port;
}



/* FUNCTION NAME:  SSHD_MGR_SetAuthenticationRetries
 * PURPOSE:
 *          This function set number of retries for authentication user.
 *
 * INPUT:
 *          UI32_T -- number of retries for authentication user.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE.
 * NOTES:
 *          This function maybe invoked in CLI command 'ip ssh authentication-retries'.
 */
BOOL_T SSHD_MGR_SetAuthenticationRetries(UI32_T retries)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */

    /* BODY */
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }
    else
    {
        SSHD_OM_SetAuthenticationRetries(retries);

    }

    return TRUE;
}



/* FUNCTION NAME:  SSHD_MGR_GetAuthenticationRetries
 * PURPOSE:
 *          This function get number of retries for authentication user.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          UI32_T --  number of retries for authentication user.
 * NOTES:
 *          This function maybe invoked in CLI command 'show ip ssh'.
 */
UI32_T SSHD_MGR_GetAuthenticationRetries(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    UI32_T  retries = 0;

    /* BODY */

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
    }
    else
    {
        retries = SSHD_OM_GetAuthenticationRetries();
    }

    return retries;
}



/* FUNCTION NAME:  SSHD_MGR_SetNegotiationTimeout
 * PURPOSE:
 *          This function set number of negotiation timeout .
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE.
 * NOTES:
 *          This function maybe invoked in CLI command 'ip ssh timeout'.
 */
BOOL_T SSHD_MGR_SetNegotiationTimeout(UI32_T timeout)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */

    /* BODY */
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }
    else
    {
        SSHD_OM_SetNegotiationTimeout(timeout);
    }

    return TRUE;
}



/* FUNCTION NAME:  SSHD_MGR_GetNegotiationTimeout
 * PURPOSE:
 *          This function get number of negotiation timeout .
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          UI32_T --  number of negotiation timeout .
 * NOTES:
 *          This function maybe invoked in CLI command 'show ip ssh'.
 */
UI32_T SSHD_MGR_GetNegotiationTimeout(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    UI32_T  timeout = 0;

    /* BODY */
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
    }
    else
    {
        timeout = SSHD_OM_GetNegotiationTimeout();
    }

    return timeout;
}



/* FUNCTION NAME:  SSHD_MGR_GetSshServerVersion()
 * PURPOSE:
 *          This function get version of ssh server.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          UI32_T * - number of major version.
 *          UI32_T * - number of minor version.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          This function maybe invoked in CLI command 'show ip ssh'.
 */
BOOL_T SSHD_MGR_GetSshServerVersion(UI32_T *major, UI32_T *minor)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */

    /* BODY */
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }
    else
    {
        SSHD_OM_GetSshServerVersion(major,minor);
    }

    return TRUE;
}



/* FUNCTION NAME:  SSHD_MGR_GetRunningNegotiationTimeout
 * PURPOSE:
 *          This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
 *          specific SSH negotiation timeout with non-default values
 *          can be retrieved successfully. Otherwise, SYS_TYPE_GET_RUNNING_CFG_FAIL
 *          or SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          UI32_T * - Negotiation Timeout.
 *
 * RETURN:
 *          SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES:
 *          1. This function shall only be invoked by CLI to save the
 *             "running configuration" to local or remote files.
 *          2. Since only non-default configuration will be saved, this
 *             function shall return non-default structure for each field for the device.
 */
SYS_TYPE_Get_Running_Cfg_T  SSHD_MGR_GetRunningNegotiationTimeout(UI32_T *timeout)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */

    /* BODY */

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }
    else
    {
        *timeout = SSHD_MGR_GetNegotiationTimeout();
        if ( *timeout != SSHD_DEFAULT_NEGOTIATION_TIMEOUT )
        {
            return SYS_TYPE_GET_RUNNING_CFG_SUCCESS ;
        }
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
}



/* FUNCTION NAME:  SSHD_MGR_GetRunningAuthenticationRetries
 * PURPOSE:
 *          This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
 *          specific SSH authentication retries times with non-default values
 *          can be retrieved successfully. Otherwise, SYS_TYPE_GET_RUNNING_CFG_FAIL
 *          or SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          UI32_T * - Authentication Retries.
 *
 * RETURN:
 *          SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES:
 *          1. This function shall only be invoked by CLI to save the
 *             "running configuration" to local or remote files.
 *          2. Since only non-default configuration will be saved, this
 *             function shall return non-default structure for each field for the device.
 */
SYS_TYPE_Get_Running_Cfg_T  SSHD_MGR_GetRunningAuthenticationRetries(UI32_T *retries)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */

    /* BODY */

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }
    else
    {
        *retries = SSHD_MGR_GetAuthenticationRetries();
        if ( *retries != SSHD_DEFAULT_AUTHENTICATION_RETRIES )
        {
            return SYS_TYPE_GET_RUNNING_CFG_SUCCESS ;
        }
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
}



/* FUNCTION NAME:  SSHD_MGR_GetRunningSshdStatus
 * PURPOSE:
 *          This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
 *          specific Sshd Status with non-default values
 *          can be retrieved successfully. Otherwise, SYS_TYPE_GET_RUNNING_CFG_FAIL
 *          or SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          UI32_T * - Sshd Status.
 *
 * RETURN:
 *          SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES:
 *          1. This function shall only be invoked by CLI to save the
 *             "running configuration" to local or remote files.
 *          2. Since only non-default configuration will be saved, this
 *             function shall return non-default structure for each field for the device.
 */
SYS_TYPE_Get_Running_Cfg_T  SSHD_MGR_GetRunningSshdStatus(UI32_T *state)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */

    /* BODY */

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }
    else
    {
        *state = SSHD_MGR_GetSshdStatus();
        if ( *state != SSHD_DEFAULT_STATE )
        {
            return SYS_TYPE_GET_RUNNING_CFG_SUCCESS ;
        }
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
}



/* FUNCTION NAME:  SSHD_MGR_SetServerKeySize
 * PURPOSE:
 *          This function set number of bits for server key.
 *
 * INPUT:
 *          UI32_T key_size --  number of bits for server key. The number of key size range is
 *                              512 to 896 bits.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE  - Success.
 *          FALSE - Fault.
 * NOTES:
 *          This function maybe invoked in CLI command 'ip ssh server-key size'.
 */
BOOL_T SSHD_MGR_SetServerKeySize(UI32_T key_size)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret = FALSE;

    /* BODY */
    if( SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE )
    {
        return FALSE;
    }
    else
    {
        ret = SSHD_OM_SetServerKeySize(key_size);
        return ret;
    }
}



/* FUNCTION NAME:  SSHD_MGR_GetServerKeySize
 * PURPOSE:
 *          This function get number of bits for server key.
 *
 * INPUT:
 *          UI32_T *key_size    --  number of bits for server key.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE  - Success.
 *          FALSE - Fault.
 * NOTES:
 *
 */
BOOL_T SSHD_MGR_GetServerKeySize(UI32_T *key_size)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret = FALSE;

    /* BODY */
    if( SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE )
    {
        return FALSE;
    }
    else
    {
        ret = SSHD_OM_GetServerKeySize(key_size);
        return ret;
    }
}



/* FUNCTION NAME:  SSHD_MGR_GetRunningServerKeySize
 * PURPOSE:
 *          This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
 *          specific server key size non-default values
 *          can be retrieved successfully. Otherwise, SYS_TYPE_GET_RUNNING_CFG_FAIL
 *          or SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          UI32_T *key_size    --  number of bits for server key.
 *
 * RETURN:
 *          SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES:
 *          1. This function shall only be invoked by CLI to save the
 *             "running configuration" to local or remote files.
 *          2. Since only non-default configuration will be saved, this
 *             function shall return non-default structure for each field for the device.
 */
SYS_TYPE_Get_Running_Cfg_T  SSHD_MGR_GetRunningServerKeySize(UI32_T *key_size)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */

    /* BODY */

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }
    else
    {
        SSHD_MGR_GetServerKeySize(key_size);
        if ( *key_size != SSHD_DEFAULT_SERVER_KEY_SIZE )
        {
            return SYS_TYPE_GET_RUNNING_CFG_SUCCESS ;
        }
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
}
#endif /* #ifndef UNIT_TEST */




/* FUNCTION NAME:  SSHD_MGR_SetUserPublicKey
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
SSHD_SetUserPublicKeyResult_T SSHD_MGR_SetUserPublicKey(UI8_T *username, UI32_T key_type)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    char    *line;
    int     allowed = 0;
    u_int   bits;
    u_long  linenum = 0;
    Key     *key;
    int     found_key = 0;
    Key     *found;
    USERAUTH_LoginLocal_T login_user;
    I32_T   key_bits;
    BIO     *bio_p = NULL;
    BOOL_T  ret_value = FALSE;

    /* BODY */
    if(SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return SSHD_SET_USER_PUBLIC_KEY_FAIL;
    }
    else
    {
        if( key_type == KEY_TYPE_RSA )
        {
            if( strlen((char *)public_key) > USER_RSA_PUBLIC_KEY_FILE_LENGTH )
            {
                return SSHD_SET_USER_PUBLIC_KEY_INVALID_KEY;
            }

            if (NULL == strstr((char *)public_key, SSH_RSA))
            {
                return SSHD_SET_USER_PUBLIC_KEY_INVALID_KEY;
            }

            /* Flag indicating whether the key is allowed. */
            allowed = 0;

            key = key_new(KEY_RSA1);

            /*
             * Go though the accepted keys, looking for the current key.  If
             * found, perform a challenge-response dialog to verify that the
             * user really has the corresponding private key.
             */
            line = (char *)malloc(USER_RSA_PUBLIC_KEY_FILE_LENGTH + 1);
            if( line == NULL )
            {
                return SSHD_SET_USER_PUBLIC_KEY_OUT_OF_MEMORY;
            }

            bio_p = BIO_new_mem_buf(public_key, -1);

            while(BIO_gets(bio_p, line, USER_RSA_PUBLIC_KEY_FILE_LENGTH + 1))
            {
                char *cp;
                /*char *options;*/

                linenum++;

                /* Skip leading whitespace, empty and comment lines. */
                for (cp = line; *cp == ' ' || *cp == '\t'; cp++)
                    ;
                if (!*cp || *cp == '\n' || *cp == '#')
                    continue;

                /*
                 * Check if there are options for this key, and if so,
                 * save their starting address and skip the option part
                 * for now.  If there are no options, set the starting
                 * address to NULL.
                 */
                if (*cp < '0' || *cp > '9') {
                    int quoted = 0;
                    /*options = cp;*/
                    for (; *cp && (quoted || (*cp != ' ' && *cp != '\t')); cp++) {
                        if (*cp == '\\' && cp[1] == '"')
                            cp++;   /* Skip both */
                        else if (*cp == '"')
                            quoted = !quoted;
                    }
                }
                /*
                else
                    options = NULL;
                */

                /* Parse the key from the line. */
                if (hostfile_read_key(&cp, &bits, key) == 0) {
/*                  debug("%.100s, line %lu: non ssh1 key syntax",
                        public_key, linenum);*/
                    continue;
                }
                /* cp now points to the comment part. */

                /* Check if the we have found the desired key (identified by its modulus). */
/*              if (BN_cmp(key->rsa->n, client_n) != 0)
                    continue;*/

                /* check the real bits  */
/*              if (bits != BN_num_bits(key->rsa->n))
                    log("Warning: %s, line %lu: keysize mismatch: "
                        "actual %d vs. announced %d.",
                        file, linenum, BN_num_bits(key->rsa->n), bits);*/

                /* We have found the desired key. */
                /* break out, this key is allowed */

                /* EPR3526VA-PoE-00077
                   don't allow invalid modulus size key file
                */
                key_bits = key_size(key);
                if ((SSH_RSA_MINIMUM_MODULUS_SIZE > key_bits) ||
                    (SSH_RSA_MAXIMUM_MODULUS_SIZE < key_bits))
                {
                    break;
                }
                allowed = 1;
                break;
            }

            free(line);
            BIO_free(bio_p);

            /* return key if allowed */
            key_free(key);

            if( allowed == 0 )
            {
                if ( SSHD_MGR_do_convert_from_ssh2( ) == FALSE )
                {
                    return SSHD_SET_USER_PUBLIC_KEY_INVALID_KEY;
                }
            }
        }
        else if( key_type == KEY_TYPE_DSA )
        {
            if( strlen((char *)public_key) > USER_DSA_PUBLIC_KEY_FILE_LENGTH )
            {
                return SSHD_SET_USER_PUBLIC_KEY_INVALID_KEY;
            }

            if (NULL == strstr((char *)public_key, SSH_DSA))
            {
                return SSHD_SET_USER_PUBLIC_KEY_INVALID_KEY;
            }

            found_key = 0;
            found = key_new(KEY_DSA);

            line = (char *)malloc(USER_DSA_PUBLIC_KEY_FILE_LENGTH + 1);
            if( line == NULL )
            {
                return SSHD_SET_USER_PUBLIC_KEY_OUT_OF_MEMORY;
            }

            bio_p = BIO_new_mem_buf(public_key, -1);

            while(BIO_gets(bio_p, line, USER_DSA_PUBLIC_KEY_FILE_LENGTH + 1))
            {
                char *cp; /*, *options = NULL;*/
                linenum++;
                /* Skip leading whitespace, empty and comment lines. */
                for (cp = line; *cp == ' ' || *cp == '\t'; cp++)
                    ;
                if (!*cp || *cp == '\n' || *cp == '#')
                    continue;

                if (key_read(found, &cp) != 1) {
                    /* no key?  check if there are options for this key */
                    int quoted = 0;
//                  debug2("user_key_allowed: check options: '%s'", cp);
                    /*options = cp;*/
                    for (; *cp && (quoted || (*cp != ' ' && *cp != '\t')); cp++) {
                        if (*cp == '\\' && cp[1] == '"')
                            cp++;   /* Skip both */
                        else if (*cp == '"')
                            quoted = !quoted;
                    }
                    /* Skip remaining whitespace. */
                    for (; *cp == ' ' || *cp == '\t'; cp++)
                        ;
                    if (key_read(found, &cp) != 1) {
//                      debug2("user_key_allowed: advance: '%s'", cp);
                        /* still no key?  advance to next line*/
                        continue;
                    }
                }
                if( found->type == KEY_DSA )
                {
                    /* EPR3526VA-PoE-00077
                       don't allow invalid modulus size key file
                    */
                    key_bits = key_size(found);
                    if ((SSH_DSA_MINIMUM_MODULUS_SIZE > key_bits) ||
                        (SSH_DSA_MAXIMUM_MODULUS_SIZE < key_bits))
                    {
                        break;
                    }

                    found_key = 1;
                    break;
                }
            }
            free(line);
            BIO_free(bio_p);

            key_free(found);
            if( found_key == 0 )
            {
                if( SSHD_MGR_do_convert_from_ssh2() == FALSE )
                {
                    return SSHD_SET_USER_PUBLIC_KEY_INVALID_KEY;
                }
            }
        }
        else
        {
            return SSHD_SET_USER_PUBLIC_KEY_INVALID_KEY_TYPE;
        }

        ret_value = KEYGEN_MGR_SetUserPublicKey(public_key, username, key_type);

        return ret_value;
    }
}

static BOOL_T SSHD_MGR_CopyUserPublicKey(UI8_T *key_buffer)
{
    if (NULL == key_buffer)
    {
        return FALSE;
    }

    memset((char *)public_key, 0, sizeof(public_key));
    strncpy((char *)public_key, (char *)key_buffer, sizeof(public_key));

    if ('\0' != public_key[USER_DSA_PUBLIC_KEY_FILE_LENGTH])
    {
        return FALSE;
    }

    return TRUE;
}

#ifndef UNIT_TEST
/* FUNCTION NAME:  SSHD_MGR_DeleteUserPublicKey
 * PURPOSE:
 *          This function to delete user's public key.
 *
 * INPUT:
 *          UI8_T   *username   --  name of user of public key will be delete.
 *          UI32_T  key_type    --  Type of host key.(KEY_TYPE_RSA, KEY_TYPE_DSA and KEY_TYPE_BOTH_RSA_AND_DSA)
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          This function is invoked in CLI command "delete public-key".
 */
BOOL_T SSHD_MGR_DeleteUserPublicKey(UI8_T *username, UI32_T key_type)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;

    /* BODY */
    if(SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }
    else
    {
        ret = KEYGEN_MGR_DeleteUserPublicKey(username, key_type);
        return ret;
    }
}



/* FUNCTION NAME : SSHD_MGR_GetUserPublicKeyType
 * PURPOSE:
 *      Get user's public key type.
 *
 * INPUT:
 *      UI8_T   *username   -- username.
 *
 * OUTPUT:
 *      UI32_T  *key_type   --  Type of host key.(KEY_TYPE_RSA, KEY_TYPE_DSA, KEY_TYPE_BOTH_RSA_AND_DSA, KEY_TYPE_NONE)
 *
 * RETURN:
 *      TRUE  - User has public key.
 *      FALSE - User has not public key.
 *
 * NOTES:
 *      This API is invoked in CLI command "show users".
 */
BOOL_T SSHD_MGR_GetUserPublicKeyType(UI8_T *username, UI32_T *key_type)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;

    /* BODY */
    if(SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }
    else
    {
    ret = KEYGEN_MGR_GetUserPublicKeyType(username, key_type);
    return ret;
}
}



/* FUNCTION NAME : SSHD_MGR_GetUserPublicKey
 * PURPOSE:
 *      Get user's public key.
 *
 * INPUT:
 *      UI_8_T  *username   --  username.
 *
 * OUTPUT:
 *      UI8_T   *rsa_key    --  pointer of buffer to storage rsa public key.
 *      UI8_T   *dsa_key    --  pointer of buffer to storage dsa public key.
 *
 * RETURN:
 *      TRUE  - Success.
 *      FALSE - Fault.
 *
 * NOTES:
 *      This API is invoked in CLI command "show public-key".
 */
BOOL_T SSHD_MGR_GetUserPublicKey(UI8_T *username, UI8_T *rsa_key, UI8_T *dsa_key)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;

    /* BODY */
    if(SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }
    else
    {
    ret = KEYGEN_MGR_GetUserPublicKey(username, rsa_key, dsa_key);
    return ret;
}
}



/* FUNCTION NAME : SSHD_MGR_GetNextUserPublicKey
 * PURPOSE:
 *      Get user's public key.
 *
 * INPUT:
 *      UI_8_T  *username   --  username.
 *
 * OUTPUT:
 *      UI_8_T  *username   --  Next username.
 *      UI8_T   *rsa_key    --  pointer of buffer to storage rsa public key.
 *      UI8_T   *dsa_key    --  pointer of buffer to storage dsa public key.
 *
 * RETURN:
 *      TRUE  - The output value is current active connection info.
 *      FALSE - The output value is invalid.
 *
 * NOTES:
 *      This API is invoked in CLI command "show public-key".
 *      Initiation username is empty string.
 */
BOOL_T SSHD_MGR_GetNextUserPublicKey(UI8_T *username, UI8_T *rsa_key, UI8_T *dsa_key)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;

    /* BODY */
    if(SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }
    else
    {
    ret = KEYGEN_MGR_GetNextUserPublicKey(username, rsa_key, dsa_key);
    return ret;
}
}



/* FUNCTION NAME : SSHD_MGR_Dsa2SshDss
 * PURPOSE:
 *      Convert dsa key to ssh-dss format.
 *
 * INPUT:
 *      void    *dsa        --  pointer of dsa key.
 *
 * OUTPUT:
 *      UI8_T   *dsa_key    --  pointer of buffer to storage dsa public key.
 *
 * RETURN:
 *      TRUE  - Success.
 *      FALSE - Faulier.
 *
 * NOTES:
 *      This API is invoked in CLI command KEYGEN_MGR_GetHostPublicKey.
 */
BOOL_T SSHD_MGR_Dsa2SshDss(void *dsa, UI8_T *dsa_key)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;
    DSA *tmp_dsa;
    Buffer b;
    int len, n;
    char *uu;

    /* BODY */
    if(SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }
    else
    {
        tmp_dsa = (DSA *) dsa;
        buffer_init(&b);
        buffer_put_cstring(&b, "ssh-dss");
        buffer_put_bignum2(&b, tmp_dsa->p);
        buffer_put_bignum2(&b, tmp_dsa->q);
        buffer_put_bignum2(&b, tmp_dsa->g);
        buffer_put_bignum2(&b, tmp_dsa->pub_key);
        len = buffer_len(&b);
        uu = (char *)malloc(2*len);
        n = uuencode(buffer_ptr(&b), len, uu, 2*len);
        if (n > 0)
        {
            strcpy((char *)dsa_key, "ssh-dss ");
            strcat((char *)dsa_key, uu);
            ret = TRUE;
        }
        else
        {
            ret = FALSE;
        }
        free(uu);
        memset(buffer_ptr(&b), 0, len);
        buffer_free(&b);
        return ret;
    }
}



/* FUNCTION NAME : SSHD_MGR_SetConnectionIDAndTnshID
 * PURPOSE:
 *      Set tnsh id and ssh connection id to session record.
 *
 * INPUT:
 *      UI32_T  tnsh_port   --  the port connect to TNSHD.
 *      UI32_T  tid         --  TNSH id.
 *      UI32_T  cid         --  ssh connection id.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      TRUE  - tnsh_port found and ssh server enabled, or don't found tnsh_port.
 *      FALSE - tnsh_port found and ssh server disabled.
 *
 * NOTES:
 *      This function invoked in TNSHD_ChildTask().
 */
BOOL_T SSHD_MGR_SetConnectionIDAndTnshID(UI32_T tnsh_port, UI32_T tid, UI32_T cid)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret = FALSE;

    /* BODY */
    if( SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE )
    {
        return FALSE;
    }
    else
    {
        ret = SSHD_OM_SetConnectionIDAndTnshID(tnsh_port, tid, cid);
        return ret;
    }
}



/* FUNCTION NAME : SSHD_MGR_GetSshConnectionUsername
 * PURPOSE:
 *      Get username of ssh connection. Index  is connection id.
 *
 * INPUT:
 *      UI32_T  cid         --  connection_id.
 *
 * OUTPUT:
 *      UI8_T   *username   --  username.
 *
 * RETURN:
 *      TRUE  - Success.
 *      FALSE - Fault.
 *
 * NOTES:
 *      This API is invoked in CLI
 */
BOOL_T SSHD_MGR_GetSshConnectionUsername(UI32_T cid, UI8_T *username)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret = FALSE;

    /* BODY */
    if( SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE )
    {
        return FALSE;
    }
    else
    {
        ret = SSHD_OM_GetSshConnectionUsername(cid, username);
        return ret;
    }
}



/* FUNCTION NAME : SSHD_MGR_GetSshConnectionPassword
 * PURPOSE:
 *      Get password of ssh connection. Index  is connection id.
 *
 * INPUT:
 *      UI32_T  cid             --  connection_id.
 *
 * OUTPUT:
 *      UI8_T   *password       --  password.
 *      UI32_T  password_size   --  buffer size of password
 *
 * RETURN:
 *      TRUE  - Success.
 *      FALSE - Fault.
 *
 * NOTES:
 *      This API is invoked in CLI
 */
BOOL_T SSHD_MGR_GetSshConnectionPassword(UI32_T cid, char *password, UI32_T password_size)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret = FALSE;

    /* BODY */
    if( SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE )
    {
        return FALSE;
    }
    else
    {
        ret = SSHD_OM_GetSshConnectionPassword(cid, password, password_size);
        return ret;
    }
}



/* FUNCTION NAME : SSHD_MGR_GetSshConnectionPrivilege
 * PURPOSE:
 *      Get username privilege of ssh connection. Index  is connection id.
 *
 * INPUT:
 *      UI32_T  cid         --  connection_id.
 *
 * OUTPUT:
 *      UI32_T  *privilege  --  privilege.
 *
 * RETURN:
 *      TRUE  - Success.
 *      FALSE - Fault.
 *
 * NOTES:
 *      This API is invoked in CLI
 */
BOOL_T SSHD_MGR_GetSshConnectionPrivilege(UI32_T cid, UI32_T *privilege)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret = FALSE;

    /* BODY */
    if( SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE )
    {
        return FALSE;
    }
    else
    {
        ret = SSHD_OM_GetSshConnectionPrivilege(cid, privilege);
        return ret;
    }
}



/* FUNCTION NAME : SSHD_MGR_GetSshConnectionAuthResult
 * PURPOSE:
 *      Get authentication result of the specified connection ID.
 *
 * INPUT:
 *      cid         --  connection ID.
 *
 * OUTPUT:
 *      auth_result --  authentication result.
 *
 * RETURN:
 *      TRUE  - Success.
 *      FALSE - Fault.
 *
 * NOTES:
 *      None.
 */
BOOL_T SSHD_MGR_GetSshConnectionAuthResult(
    UI32_T cid,
    USERAUTH_AuthResult_T *auth_result_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret = FALSE;

    /* BODY */
    if( SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE )
    {
        return FALSE;
    }
    else
    {
        ret = SSHD_OM_GetSshConnectionAuthResult(cid, auth_result_p);
        return ret;
    }
}




/* FUNCTION NAME : SSHD_MGR_GetNextSshConnectionEntry
 * PURPOSE:
 *      Get next active connection entry.
 *
 * INPUT:
 *      UI32_T * -- previous active connection id.
 *
 * OUTPUT:
 *      UI32_T * -- current active connection id.
 *      SSHD_ConnectionInfo_T * -- current active connection information.
 *
 * RETURN:
 *      TRUE  - The output value is current active connection info.
 *      FALSE - The output value is invalid.
 *
 * NOTES:
 *      This function invoked in CLI command "show ssh".
 *      Initial input value is -1.
 */
BOOL_T SSHD_MGR_GetNextSshConnectionEntry(I32_T *cid, SSHD_ConnectionInfo_T *info)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T rc;

    /* BODY */
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }
    else
    {
        if ( (cid == NULL) || (info == NULL) )
        {
            return FALSE;
        }

        rc = SSHD_OM_GetNextSshConnectionEntry(cid,info);
    }

    return rc;
}



/* FUNCTION NAME : SSHD_MGR_GetSshConnectionEntry
 * PURPOSE:
 *      Get Specify active connection entry.
 *
 * INPUT:
 *      UI32_T   -- Specify active connection id.
 *
 * OUTPUT:
 *      SSHD_ConnectionInfo_T * -- Specify active connection information.
 *
 * RETURN:
 *      TRUE  - The output value is Specify active connection info.
 *      FALSE - The output value is invalid.
 *
 * NOTES:
 *      This function invoked in SNMP .
 */
BOOL_T SSHD_MGR_GetSshConnectionEntry(UI32_T cid, SSHD_ConnectionInfo_T *info)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T rc;

    /* BODY */
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }
    else
    {
        if ( (cid < 0) || (info == NULL) )
        {
            return FALSE;
        }

        rc = SSHD_OM_GetSshConnectionEntry(cid,info);
    }

    return rc;
}



/* FUNCTION NAME : SSHD_MGR_GetSessionPair
 * PURPOSE:
 *      Retrieve a session pair from session record.
 *
 * INPUT:
 *      UI32_T  -- the port connect to TNSHD.
 *
 * OUTPUT:
 *      UI32_T * -- the ip of remote site in socket.
 *      UI32_T * -- the port of remote site in socket.
 *
 * RETURN:
 *      TRUE to indicate successful and FALSE to indicate failure.
 *
 * NOTES:
 *      This function invoked in CLI_TASK_SetSessionContext().
 */
BOOL_T SSHD_MGR_GetSessionPair(UI32_T tnsh_port, L_INET_AddrIp_T *user_addr, UI32_T *user_port)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T rc;

    /* BODY */
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }
    else
    {
        if ((user_addr == NULL) || (user_port == NULL))
        {
            return FALSE;
        }

        *user_port = 0;

        rc = SSHD_OM_GetSessionPair(tnsh_port, user_addr, user_port);
    }

    return rc;
}



/* FUNCTION NAME : SSHD_MGR_AsyncGenerateHostKeyPair
 * PURPOSE:
 *      Generate RSA/DSA public and private key pair of host key.
 *
 * INPUT:
 *      UI32_T  key_type    --  Type of host key.(KEY_TYPE_RSA, KEY_TYPE_DSA, KEY_TYPE_BOTH_RSA_AND_DSA)
 *
 * OUTPUT:
 *
 * RETURN:
 *      TRUE  - Success.
 *      FALSE - Fault.
 *
 * NOTES:
 *      This API is invoked in SNMP sshHostKeyGenAction().
 */
BOOL_T SSHD_MGR_AsyncGenerateHostKeyPair(UI32_T key_type)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    UI32_T  gen_host_key_status, tid;

    /* BODY */
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }
    else
    {
        if( (key_type != KEY_TYPE_RSA) && (key_type != KEY_TYPE_DSA) && (key_type != KEY_TYPE_BOTH_RSA_AND_DSA) )
        {
            return FALSE;
        }

        SSHD_OM_GetGenerateHostKeyAction(&gen_host_key_status);
        if( gen_host_key_status != NON_GENERATE_KEY )
        {
            return FALSE;
        }

        switch( key_type )
        {
            case KEY_TYPE_RSA:
                gen_host_key_status = GENERATE_RSA_KEY;
                break;
            case KEY_TYPE_DSA:
                gen_host_key_status = GENERATE_DSA_KEY;
                break;
            case KEY_TYPE_BOTH_RSA_AND_DSA:
                gen_host_key_status = GENERATE_RSA_AND_DSA_KEY;
                break;
        }

        SSHD_OM_SetGenerateHostKeyAction(gen_host_key_status);

        if( SYSFUN_SpawnThread(SYS_BLD_SSHD_CSC_ASYNC_GEN_HOST_KEY_THREAD_PRIORITY,
                               SYS_BLD_SSHD_CSC_ASYNC_GEN_HOST_KEY_SCHED_POLICY,
                               SYS_BLD_SSHD_CSC_ASYNC_GEN_HOST_KEY_THREAD_NAME,
                               SYS_BLD_SSHD_ASYNC_GEN_HOST_TASK_STACK_SIZE,
                               SYSFUN_TASK_FP,
                               KEYGEN_MGR_AsyncGenerateHostKeyPair,
                               NULL,
                               &tid) != SYSFUN_OK )
        {
            return FALSE;
        } /* End of if */

        return TRUE;
    }
}



/* FUNCTION NAME : SSHD_MGR_GetGenerateHostKeyAction
 * PURPOSE:
 *      Get tpye of host key generation.
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      UI32_T  *action_type    --  tpye of host key generation.
 *
 * RETURN:
 *      TRUE  - Success.
 *      FALSE - Fault.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_MGR_GetGenerateHostKeyAction(UI32_T *action_type)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;

    /* BODY */
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }
    else
    {
        ret = SSHD_OM_GetGenerateHostKeyAction(action_type);
        return ret;
    }
}

int SSHD_MGR_GenerateKey_CallBack(int a, int b, BN_GENCB *cb)
{
    UI32_T action_type;
    BOOL_T ret;

    ret = SSHD_OM_GetGenerateHostKeyAction(&action_type);

    if (ret == FALSE || action_type == CANCEL_GENERATING_KEY)
        return 0;

    return 1;
}



/* FUNCTION NAME : SSHD_MGR_GetGenerateHostKeyStatus
 * PURPOSE:
 *      Get result of host key generation.
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      UI32_T  *action_result  --  result of host key generation.
 *
 * RETURN:
 *      TRUE  - Success.
 *      FALSE - Fault.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_MGR_GetGenerateHostKeyStatus(UI32_T *action_result)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;

    /* BODY */
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }
    else
    {
        ret = SSHD_OM_GetGenerateHostKeyStatus(action_result);
        return ret;
    }
}

/* FUNCTION NAME : SSHD_MGR_GetDeleteUserPublicKeyAction
 * PURPOSE:
 *      Get tpye of user key delete.
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      UI32_T  *action_type    --  tpye of user key delete.
 *
 * RETURN:
 *      TRUE  - Success.
 *      FALSE - Fault.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_MGR_GetDeleteUserPublicKeyAction(UI32_T *action_type)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;

    /* BODY */
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }
    else
    {
        ret = SSHD_OM_GetDeleteUserPublicKeyAction(action_type);
        return ret;
    }
}

/* FUNCTION NAME : SSHD_MGR_AsyncDeleteUserPublicKey
 * PURPOSE:
 *      Delete RSA/DSA public key of user key.
 *
 * INPUT:
 *      UI8_T   *username   --  username.
 *      UI32_T  key_type    --  Type of host key.(KEY_TYPE_RSA, KEY_TYPE_DSA, KEY_TYPE_BOTH_RSA_AND_DSA)
 *
 * OUTPUT:
 *
 * RETURN:
 *      TRUE  - Success.
 *      FALSE - Fault.
 *
 * NOTES:
 *      This API is invoked in SNMP sshUserKeyDelAction().
 */
BOOL_T SSHD_MGR_AsyncDeleteUserPublicKey(UI8_T *username, UI32_T key_type)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    UI32_T  del_user_key_status;
    UI32_T  tid;
    UI8_T   *name;

    /* BODY */
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }
    else
    {
        if( (key_type != KEY_TYPE_RSA) && (key_type != KEY_TYPE_DSA) && (key_type != KEY_TYPE_BOTH_RSA_AND_DSA) )
        {
            return FALSE;
        }

        SSHD_OM_GetDeleteUserPublicKeyAction(&del_user_key_status);
        if( del_user_key_status != NON_DELETE_KEY )
        {
            return FALSE;
        }

        switch( key_type )
        {
            case KEY_TYPE_RSA:
                del_user_key_status = DELETE_RSA_KEY;
                break;
            case KEY_TYPE_DSA:
                del_user_key_status = DELETE_DSA_KEY;
                break;
            case KEY_TYPE_BOTH_RSA_AND_DSA:
                del_user_key_status = DELETE_RSA_AND_DSA_KEY;
                break;
        }

        SSHD_OM_SetDeleteUserPublicKeyAction(del_user_key_status);

        name = (UI8_T *)malloc(SYS_ADPT_MAX_USER_NAME_LEN+1);
        strcpy((char *)name, (char *)username);

        if( SYSFUN_SpawnThread(SYS_BLD_SSHD_CSC_ASYNC_DEL_USER_KEY_THREAD_PRIORITY,
                               SYS_BLD_SSHD_CSC_ASYNC_DEL_USER_KEY_SCHED_POLICY,
                               SYS_BLD_SSHD_CSC_ASYNC_DEL_USER_KEY_THREAD_NAME,
                               SYS_BLD_TASK_LARGE_STACK_SIZE,
                               SYSFUN_TASK_FP,
                               KEYGEN_MGR_AsyncDeleteUserPublicKey,
                               name,
                               &tid) != SYSFUN_OK )
        {
            free(name);
            return FALSE;
        } /* End of if */

        return TRUE;
    }
}

/* FUNCTION NAME : SSHD_MGR_GetDeleteUserPublicKeyStatus
 * PURPOSE:
 *      Get result of user key delete.
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      UI32_T  *action_result  --  result of user key delete.
 *
 * RETURN:
 *      TRUE  - Success.
 *      FALSE - Fault.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_MGR_GetDeleteUserPublicKeyStatus(UI32_T *action_result)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;

    /* BODY */
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }
    else
    {
        ret = SSHD_OM_GetDeleteUserPublicKeyStatus(action_result);
        return ret;
    }
}



/* FUNCTION NAME : SSHD_MGR_AsyncWriteHostKey2Flash
 * PURPOSE:
 *      This function write host key from memory to flash.
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *
 * RETURN:
 *      TRUE  - Success.
 *      FALSE - Fault.
 *
 * NOTES:
 *      This API is invoked in SNMP sshHostKeySaveAction().
 */
BOOL_T SSHD_MGR_AsyncWriteHostKey2Flash()
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    UI32_T  write_host_key_status;
    UI32_T  tid;

    /* BODY */
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }
    else
    {
        SSHD_OM_GetWriteHostKey2FlashAction(&write_host_key_status);
        if( write_host_key_status != NON_SAVE )
        {
            return FALSE;
        }
        SSHD_OM_SetWriteHostKey2FlashAction(SAVING);

        if( SYSFUN_SpawnThread(SYS_BLD_SSHD_CSC_ASYNC_WRITE_HOST_KEY_THREAD_PRIORITY,
                               SYS_BLD_SSHD_CSC_ASYNC_WRITE_HOST_KEY_SCHED_POLICY,
                               SYS_BLD_SSHD_CSC_ASYNC_WRITE_HOST_KEY_THREAD_NAME,
                               SYS_BLD_TASK_LARGE_STACK_SIZE,
                               SYSFUN_TASK_FP,
                               KEYGEN_MGR_AsyncWriteHostKey2Flash,
                               NULL,
                               &tid) != SYSFUN_OK )
        {
            return FALSE;
        } /* End of if */

        return TRUE;
    }
}

/* FUNCTION NAME : SSHD_MGR_SetWriteHostKey2FlashAction
 * PURPOSE:
 *      Set tpye of host key writing.
 *
 * INPUT:
 *      UI32_T  action_type --  tpye of host key writing.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      TRUE  - Success.
 *      FALSE - Fault.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_MGR_SetWriteHostKey2FlashAction(UI32_T action)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;

    /* BODY */
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }
    else
    {
        ret = SSHD_OM_SetWriteHostKey2FlashAction(action);
        return ret;
    }
}

/* FUNCTION NAME : SSHD_MGR_SetWriteHostKey2FlashStatus
 * PURPOSE:
 *      Set result of host key writing.
 *
 * INPUT:
 *      UI32_T  action_result   --  result of host key writing.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      TRUE  - Success.
 *      FALSE - Fault.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_MGR_SetWriteHostKey2FlashStatus(UI32_T status)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;

    /* BODY */
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }
    else
    {
        ret = SSHD_OM_SetWriteHostKey2FlashStatus(status);
        return ret;
    }
}

/* FUNCTION NAME : SSHD_MGR_SetGenerateHostKeyAction
 * PURPOSE:
 *      Set tpye of host key generation.
 *
 * INPUT:
 *      UI32_T  action_type --  tpye of host key generation.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      TRUE  - Success.
 *      FALSE - Fault.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_MGR_SetGenerateHostKeyAction(UI32_T action_type)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;

    /* BODY */
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }
    else
    {
        ret = SSHD_OM_SetGenerateHostKeyAction(action_type);
        return ret;
    }
}

/* FUNCTION NAME : SSHD_MGR_SetGenerateHostKeyStatus
 * PURPOSE:
 *      Set result of host key generation.
 *
 * INPUT:
 *      UI32_T  action_result   --  result of host key generation.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      TRUE  - Success.
 *      FALSE - Fault.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_MGR_SetGenerateHostKeyStatus(UI32_T action_result)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;

    /* BODY */
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }
    else
    {
        ret = SSHD_OM_SetGenerateHostKeyStatus(action_result);
        return ret;
    }
}

/* FUNCTION NAME : SSHD_MGR_SetDeleteUserPublicKeyAction
 * PURPOSE:
 *      Set tpye of user key delete.
 *
 * INPUT:
 *      UI32_T  action_type --  tpye of user key delete.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      TRUE  - Success.
 *      FALSE - Fault.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_MGR_SetDeleteUserPublicKeyAction(UI32_T action_type)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;

    /* BODY */
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }
    else
    {
        ret = SSHD_OM_SetDeleteUserPublicKeyAction(action_type);
        return ret;
    }
}

/* FUNCTION NAME : SSHD_MGR_SetDeleteUserPublicKeyStatus
 * PURPOSE:
 *      Set result of user key delete.
 *
 * INPUT:
 *      UI32_T  action_result   --  result of user key delete.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      TRUE  - Success.
 *      FALSE - Fault.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_MGR_SetDeleteUserPublicKeyStatus(UI32_T action_result)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;

    /* BODY */
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }
    else
    {
        ret = SSHD_OM_SetDeleteUserPublicKeyStatus(action_result);
        return ret;
    }
}

/* FUNCTION NAME : SSHD_MGR_GetWriteHostKey2FlashAction
 * PURPOSE:
 *      Get tpye of host key writing.
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      UI32_T  *action_type    --  tpye of host key writing.
 *
 * RETURN:
 *      TRUE  - Success.
 *      FALSE - Fault.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_MGR_GetWriteHostKey2FlashAction(UI32_T *action_type)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;

    /* BODY */
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }
    else
    {
        ret = SSHD_OM_GetWriteHostKey2FlashAction(action_type);
        return ret;
    }
}

/* FUNCTION NAME : SSHD_MGR_GetWriteHostKey2FlashStatus
 * PURPOSE:
 *      Get result of host key writing.
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      UI32_T  *action_result  --  result of host key writing.
 *
 * RETURN:
 *      TRUE  - Success.
 *      FALSE - Fault.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_MGR_GetWriteHostKey2FlashStatus(UI32_T *action_result)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;

    /* BODY */
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }
    else
    {
        ret = SSHD_OM_GetWriteHostKey2FlashStatus(action_result);
        return ret;
    }
}



/* FUNCTION NAME:  SSHD_MGR_SetSshPasswordAuthenticationStatus
 * PURPOSE:
 *          This function set password authentication state.
 *
 * INPUT:
 *          SSHD_PasswordAuthenticationStatus_T state   --  Password Authentication Status.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          none.
 * NOTES:
 *          This function maybe invoked in CLI command.
 */
BOOL_T SSHD_MGR_SetSshPasswordAuthenticationStatus(SSHD_PasswordAuthenticationStatus_T state)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T ret = FALSE;
    SSHD_State_T    sshd_state = SSHD_STATE_DISABLED;

    /* BODY */
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }
    else
    {
        sshd_state = SSHD_OM_GetSshdStatus();
        if( sshd_state == SSHD_STATE_DISABLED )
        {
            SSHD_OM_SetSshPasswordAuthenticationStatus(state);

            ret = TRUE;
        }
        else
        {

            ret = FALSE;
        }
        return ret;
    }
}



/* FUNCTION NAME:  SSHD_MGR_GetSshPasswordAuthenticationStatus
 * PURPOSE:
 *          This function get password authentication state.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          SSHD_PasswordAuthenticationStatus_T *state  --  Password Authentication Status.
 *
 * RETURN:
 *          none.
 * NOTES:
 *          This function maybe invoked in CLI command.
 */
BOOL_T SSHD_MGR_GetSshPasswordAuthenticationStatus(SSHD_PasswordAuthenticationStatus_T *state)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T ret = FALSE;

    /* BODY */
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }
    else
    {
        ret = SSHD_OM_GetSshPasswordAuthenticationStatus(state);
        return ret;
    }
}



/* FUNCTION NAME:  SSHD_MGR_SetSshPubkeyAuthenticationStatus
 * PURPOSE:
 *          This function set password authentication state.
 *
 * INPUT:
 *          SSHD_PubkeyAuthenticationStatus_T state   --  Pubkey Authentication Status.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          none.
 * NOTES:
 *          This function maybe invoked in CLI command.
 */
BOOL_T SSHD_MGR_SetSshPubkeyAuthenticationStatus(SSHD_PubkeyAuthenticationStatus_T state)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T ret = FALSE;
    SSHD_State_T    sshd_state = SSHD_STATE_DISABLED;

    /* BODY */
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }
    else
    {
        sshd_state = SSHD_OM_GetSshdStatus();
        if( sshd_state == SSHD_STATE_DISABLED )
        {
            SSHD_OM_SetSshPubkeyAuthenticationStatus(state);

            ret = TRUE;
        }
        else
        {

            ret = FALSE;
        }
        return ret;
    }
}



/* FUNCTION NAME:  SSHD_MGR_GetSshPubkeyAuthenticationStatus
 * PURPOSE:
 *          This function get pubkey authentication state.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          SSHD_PubkeyAuthenticationStatus_T *state  --  Pubkey Authentication Status.
 *
 * RETURN:
 *          none.
 * NOTES:
 *          This function maybe invoked in CLI command.
 */
BOOL_T SSHD_MGR_GetSshPubkeyAuthenticationStatus(SSHD_PubkeyAuthenticationStatus_T *state)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T ret = FALSE;

    /* BODY */
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }
    else
    {
        ret = SSHD_OM_GetSshPubkeyAuthenticationStatus(state);
        return ret;
    }
}



/* FUNCTION NAME:  SSHD_MGR_SetSshRsaAuthenticationStatus
 * PURPOSE:
 *          This function set rsa authentication state.
 *
 * INPUT:
 *          SSHD_RsaAuthenticationStatus_T state   --  Rsa Authentication Status.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          none.
 * NOTES:
 *          This function maybe invoked in CLI command.
 */
BOOL_T SSHD_MGR_SetSshRsaAuthenticationStatus(SSHD_RsaAuthenticationStatus_T state)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T ret = FALSE;
    SSHD_State_T    sshd_state = SSHD_STATE_DISABLED;

    /* BODY */
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }
    else
    {
        sshd_state = SSHD_OM_GetSshdStatus();
        if( sshd_state == SSHD_STATE_DISABLED )
        {
            SSHD_OM_SetSshRsaAuthenticationStatus(state);

            ret = TRUE;
        }
        else
        {

            ret = FALSE;
        }
        return ret;
    }
}



/* FUNCTION NAME:  SSHD_MGR_GetSshRsaAuthenticationStatus
 * PURPOSE:
 *          This function get rsa authentication state.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          SSHD_RsaAuthenticationStatus_T *state  --  Rsa Authentication Status.
 *
 * RETURN:
 *          none.
 * NOTES:
 *          This function maybe invoked in CLI command.
 */
BOOL_T SSHD_MGR_GetSshRsaAuthenticationStatus(SSHD_RsaAuthenticationStatus_T *state)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T ret = FALSE;

    /* BODY */
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }
    else
    {
        ret = SSHD_OM_GetSshRsaAuthenticationStatus(state);
        return ret;
    }
}



/* FUNCTION NAME : SSHD_MGR_Dsa2FingerPrint
 * PURPOSE:
 *      Convert dsa key to sha1 and md5 fingerprint.
 *
 * INPUT:
 *      void    *dsa        --  pointer of dsa key.
 *
 * OUTPUT:
 *      UI8_T   *dsa_sha1_fingerprint   --  pointer of buffer to storage dsa sha1 fingerprint.
 *      UI8_T   *dsa_md5_fingerprint    --  pointer of buffer to storage dsa md5 fingerprint.
 *
 * RETURN:
 *      TRUE  - Success.
 *      FALSE - Faulier.
 *
 * NOTES:
 *      This API is invoked in CLI command KEYGEN_MGR_GetHostPublicKey.
 */
BOOL_T SSHD_MGR_Dsa2FingerPrint(void *dsa, UI8_T *dsa_sha1_fingerprint, UI8_T *dsa_md5_fingerprint)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    DSA *tmp_dsa;
    Buffer b;
    int len;
    const EVP_MD *md = NULL;
    EVP_MD_CTX ctx;
    UI8_T *blob = NULL;
    UI8_T *dgst_raw = NULL;
    unsigned int dgst_raw_length = 0;
    UI8_T *retval;

    /* BODY */
    if(SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }
    else
    {
        tmp_dsa = (DSA *) dsa;
        buffer_init(&b);
        buffer_put_cstring(&b, "ssh-dss");
        buffer_put_bignum2(&b, tmp_dsa->p);
        buffer_put_bignum2(&b, tmp_dsa->q);
        buffer_put_bignum2(&b, tmp_dsa->g);
        buffer_put_bignum2(&b, tmp_dsa->pub_key);
        len = buffer_len(&b);
        blob = malloc(len);
        if( blob == NULL )
        {
            buffer_free(&b);
            return FALSE;
        }
        memcpy(blob, buffer_ptr(&b), len);
        memset(buffer_ptr(&b), 0, len);
        buffer_free(&b);

        dgst_raw = malloc(EVP_MAX_MD_SIZE);
        if( dgst_raw == NULL )
        {
            free(blob);
            return FALSE;
        }

        md = EVP_sha1();
        EVP_DigestInit(&ctx, md);
        EVP_DigestUpdate(&ctx, blob, len);
        EVP_DigestFinal(&ctx, dgst_raw, &dgst_raw_length);
        retval = (UI8_T *)key_fingerprint_bubblebabble(dgst_raw, (UI32_T)dgst_raw_length);
        strcpy((char *)dsa_sha1_fingerprint, (char *)retval);
        xfree(retval);

        memset(dgst_raw, 0, EVP_MAX_MD_SIZE);
        md = EVP_md5();
        EVP_DigestInit(&ctx, md);
        EVP_DigestUpdate(&ctx, blob, len);
        EVP_DigestFinal(&ctx, dgst_raw, &dgst_raw_length);
        retval = (UI8_T *)key_fingerprint_hex(dgst_raw, (UI32_T)dgst_raw_length);
        strcpy((char *)dsa_md5_fingerprint, (char *)retval);
        xfree(retval);

        memset(blob, 0, len);
        free(blob);
        free(dgst_raw);
        return TRUE;
    }
}



/* FUNCTION NAME : SSHD_MGR_Rsa2FingerPrint
 * PURPOSE:
 *      Convert rsa key to sha1 and md5 fingerprint.
 *
 * INPUT:
 *      void    *rsa        --  pointer of rsa key.
 *
 * OUTPUT:
 *      UI8_T   *rsa_sha1_fingerprint   --  pointer of buffer to storage rsa sha1 fingerprint.
 *      UI8_T   *rsa_md5_fingerprint    --  pointer of buffer to storage rsa md5 fingerprint.
 *
 * RETURN:
 *      TRUE  - Success.
 *      FALSE - Faulier.
 *
 * NOTES:
 *      This API is invoked in CLI command KEYGEN_MGR_GetHostPublicKey.
 */
BOOL_T SSHD_MGR_Rsa2FingerPrint(void *rsa, UI8_T *rsa_sha1_fingerprint, UI8_T *rsa_md5_fingerprint)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    RSA *tmp_rsa;
    int len;
    const EVP_MD *md = NULL;
    EVP_MD_CTX ctx;
    UI8_T *blob = NULL;
    UI8_T *dgst_raw = NULL;
    unsigned int dgst_raw_length = 0;
    int nlen, elen;
    UI8_T *retval;

    /* BODY */
    if(SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }
    else
    {
        tmp_rsa = (RSA *) rsa;
        nlen = BN_num_bytes(tmp_rsa->n);
        elen = BN_num_bytes(tmp_rsa->e);
        len = nlen + elen;
        blob = malloc(len);
        if( blob == NULL )
        {
            return FALSE;
        }
        BN_bn2bin(tmp_rsa->n, blob);
        BN_bn2bin(tmp_rsa->e, blob + nlen);

        dgst_raw = malloc(EVP_MAX_MD_SIZE);
        if( dgst_raw == NULL )
        {
            free(blob);
            return FALSE;
        }

        md = EVP_sha1();
        EVP_DigestInit(&ctx, md);
        EVP_DigestUpdate(&ctx, blob, len);
        EVP_DigestFinal(&ctx, dgst_raw, &dgst_raw_length);
        retval = (UI8_T *)key_fingerprint_bubblebabble(dgst_raw, (UI32_T)dgst_raw_length);
        strcpy((char *)rsa_sha1_fingerprint, (char *)retval);
        xfree(retval);

        memset(dgst_raw, 0, EVP_MAX_MD_SIZE);
        md = EVP_md5();
        EVP_DigestInit(&ctx, md);
        EVP_DigestUpdate(&ctx, blob, len);
        EVP_DigestFinal(&ctx, dgst_raw, &dgst_raw_length);
        retval = (UI8_T *)key_fingerprint_hex(dgst_raw, (UI32_T)dgst_raw_length);
        strcpy((char *)rsa_md5_fingerprint, (char *)retval);
        xfree(retval);

        memset(blob, 0, len);
        free(blob);
        free(dgst_raw);
        return TRUE;
    }
}



/* FUNCTION NAME : SSHD_MGR_CheckSshConnection
 * PURPOSE:
 *      Check connection is ssh or not.
 *
 * INPUT:
 *      UI32_T  cid --  connection id.
 *
 * OUTPUT:
 *
 * RETURN:
 *      TRUE  - This connection is ssh connection.
 *      FALSE - This connection is not ssh connection.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_MGR_CheckSshConnection(UI32_T cid)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;

    /* BODY */
    if(SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return FALSE;
    }
    else
    {
        ret = SSHD_OM_CheckSshConnection(cid);
        return (ret);
    }
}



/* FUNCTION NAME - SSHD_MGR_HandleHotInsertion
 *
 * PURPOSE  : This function will initialize the port OM of the module ports
 *            when the option module is inserted.
 *
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port
 *                                     inserted
 *            number_of_port        -- the number of ports on the inserted
 *                                     module
 *            use_default           -- the flag indicating the default
 *                                     configuration is used without further
 *                                     provision applied; TRUE if a new module
 *                                     different from the original one is
 *                                     inserted
 *
 * OUTPUT   : None
 *
 * RETURN   : None
 *
 * NOTE     : Only one module is inserted at a time.
 */
void SSHD_MGR_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */

    /* BODY */
    return;
}



/* FUNCTION NAME - SSHD_MGR_HandleHotRemoval
 *
 * PURPOSE  : This function will clear the port OM of the module ports when
 *            the option module is removed.
 *
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port
 *                                     removed
 *            number_of_port        -- the number of ports on the removed
 *                                     module
 *
 * OUTPUT   : None
 *
 * RETURN   : None
 *
 * NOTE     : Only one module is removed at a time.
 */
void SSHD_MGR_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */

    /* BODY */
    return;
}



/* FUNCTION NAME:  SSHD_MGR_BackdoorFunction
 * PURPOSE:
 *          Display back door available function and accept user seletion.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          none.
 * NOTES:
 *          .
 */
void SSHD_MGR_BackdoorFunction()
{
    I32_T   rc;
    UI32_T                  backdoor_member_id;
    L_THREADGRP_Handle_T    tg_handle;
    UI8_T                   keyin[256];
    BOOL_T                  qflag = FALSE;

    tg_handle =(L_THREADGRP_Handle_T) CLI_PROC_COMM_GetSSH_GROUPTGHandle();

    /* Join thread group
     */
    if(L_THREADGRP_Join(tg_handle, SYS_BLD_BACKDOOR_THREAD_PRIORITY, &backdoor_member_id)==FALSE)
    {
        printf("%s: L_THREADGRP_Join fail.\n", __FUNCTION__);
        return;
    }

    while(1)
    {
        BACKDOOR_MGR_Printf("\r\n1. ip ssh crypto host-key generate rsa.");
        BACKDOOR_MGR_Printf("\r\n2. ip ssh save host-key.");
        BACKDOOR_MGR_Printf("\r\n3. ip ssh server-key size.");
        BACKDOOR_MGR_Printf("\r\n4. ip ssh server.");
        BACKDOOR_MGR_Printf("\r\n5. ip ssh password authentication.");
        BACKDOOR_MGR_Printf("\r\n6. ip ssh pubkey authentication.");
        BACKDOOR_MGR_Printf("\r\n7. ip ssh rsa authentication.");
        BACKDOOR_MGR_Printf("\r\n0. Exit.\r\n");
        BACKDOOR_MGR_Printf("\r\nEnter your choice: ");

        *keyin = 0;
        BACKDOOR_MGR_RequestKeyIn((char *)keyin, 1);
        BACKDOOR_MGR_Printf("\r\n");

        /* Get execution permission from the thread group handler if necessary
        */
        L_THREADGRP_Execution_Request(tg_handle, backdoor_member_id);

        switch(*keyin)
        {
            case '0':
                qflag = TRUE;
                break;

            case '1':
                KEYGEN_MGR_GenerateHostKeyPair(3);
                break;

            case '2':
                KEYGEN_MGR_WriteHostKey2Flash();
                break;

            case '3':
                BACKDOOR_MGR_Printf("\r\n1. key_size: ");
                *keyin = 0;
                BACKDOOR_MGR_RequestKeyIn((char *)keyin, 5);
                BACKDOOR_MGR_Printf("\r\n");
                rc = atoi((char *)keyin);
                SSHD_MGR_SetServerKeySize(rc);
                break;

            case '4':
                BACKDOOR_MGR_Printf("\r\n1. Protocol(3:enabled P1, 4:enabled P2, 5:enabled all): ");
                *keyin = 0;
                BACKDOOR_MGR_RequestKeyIn((char *)keyin, 5);
                BACKDOOR_MGR_Printf("\r\n");
                rc = atoi((char *)keyin);
                SSHD_MGR_SetSshdStatus(rc);
                break;

            case '5':
                BACKDOOR_MGR_Printf("\r\n1. choose:(1:enabled, 2:enabled): ");
                *keyin = 0;
                BACKDOOR_MGR_RequestKeyIn((char *)keyin, 5);
                BACKDOOR_MGR_Printf("\r\n");
                rc = atoi((char *)keyin);
                SSHD_MGR_SetSshPasswordAuthenticationStatus(rc);
                break;

            case '6':
                BACKDOOR_MGR_Printf("\r\n1. choose:(1:enabled, 2:enabled): ");
                *keyin = 0;
                BACKDOOR_MGR_RequestKeyIn((char *)keyin, 5);
                BACKDOOR_MGR_Printf("\r\n");
                rc = atoi((char *)keyin);
                SSHD_MGR_SetSshPubkeyAuthenticationStatus(rc);
                break;

            case '7':
                BACKDOOR_MGR_Printf("\r\n1. choose:(1:enabled, 2:enabled): ");
                *keyin = 0;
                BACKDOOR_MGR_RequestKeyIn((char *)keyin, 5);
                BACKDOOR_MGR_Printf("\r\n");
                rc = atoi((char *)keyin);
                SSHD_MGR_SetSshRsaAuthenticationStatus(rc);
                break;

            default:
                break;
        }
        /* Release execution permission from the thread group handler if necessary
         */
        L_THREADGRP_Execution_Release(tg_handle, backdoor_member_id);

        if (TRUE == qflag)
        {
            L_THREADGRP_Leave(tg_handle, backdoor_member_id);
            return;
      }

      BACKDOOR_MGR_Printf("\r\n\r\n\r\n\r\n");
      BACKDOOR_MGR_Printf("---------------------------\r\n");
   }
}
#endif /* #ifndef UNIT_TEST */



/* LOCAL SUBPROGRAM BODIES
 */
static BOOL_T SSHD_MGR_do_convert_from_ssh2()
{
    Key *k;
    int blen;
    u_int len;
    char line[USER_DSA_PUBLIC_KEY_FILE_LENGTH + 1], *p;
/*  u_char blob[8096];
    char encoded[8096];*/
    u_char *blob;
    char *encoded;
//  struct stat st;
    int escaped = 0; /*, private = 0;*//*, ok;*/
//  FILE *fp;
    BIO *bio_p = NULL;

    blob = (u_char *)malloc(8096);
    encoded = (char *)malloc(8096);
    encoded[0] = '\0';

    bio_p = BIO_new_mem_buf(public_key, -1);

    while(BIO_gets(bio_p, line, sizeof(line)))
    {
        if (!(p = (char *)strchr(line, '\n')))
        {
//          fprintf(stderr, "input line too long.\n");
            free(blob);
            free(encoded);
            BIO_free(bio_p);
            return FALSE;
        }
        if (p > line && p[-2] == '\\')
            escaped++;
        if (strncmp(line, "----", 4) == 0 ||
            strstr(line, ": ") != NULL)
        {
            /*
            if (strstr(line, SSH_COM_PRIVATE_BEGIN) != NULL)
                private = 1;
            */
            if (strstr(line, " END ") != NULL)
            {
                break;
            }
            /* fprintf(stderr, "ignore: %s", line); */
            continue;
        }
        if (escaped)
        {
            escaped--;
            /* fprintf(stderr, "escaped: %s", line); */
            continue;
        }
        *p = '\0';
//      strlcat(encoded, line, sizeof(encoded));
        strncat(encoded, line, 8096);
    }
    len = strlen(encoded);
    if (((len % 4) == 3) &&
        (encoded[len-1] == '=') &&
        (encoded[len-2] == '=') &&
        (encoded[len-3] == '='))
        encoded[len-3] = '\0';
//  blen = uudecode(encoded, blob, sizeof(blob));
    blen = uudecode(encoded, blob, 8096);
    if (blen < 4)
    {
//      fprintf(stderr, "uudecode failed.\n");
        free(blob);
        free(encoded);
        BIO_free(bio_p);
        return FALSE;
    }
/*  k = private ?
        do_convert_private_ssh2_from_blob(blob, blen) :
        key_from_blob(blob, blen);*/
    k = key_from_blob(blob, blen);

    if (k == NULL)
    {
//      fprintf(stderr, "decode blob failed.\n");
        free(blob);
        free(encoded);
        BIO_free(bio_p);
        return FALSE;
    }

    /* Now not support rsa public key on sshv2
     */
    if (k->type != KEY_DSA)
    {
        key_free(k);
        BIO_free(bio_p);
        free(blob);
        free(encoded);
        return FALSE;
    }

/*  ok = private ?
        (k->type == KEY_DSA ?
         PEM_write_DSAPrivateKey(stdout, k->dsa, NULL, NULL, 0, NULL, NULL) :
         PEM_write_RSAPrivateKey(stdout, k->rsa, NULL, NULL, 0, NULL, NULL)) :
        key_write(k, stdout);
    if (!ok) {
        fprintf(stderr, "key write failed");
        exit(1);
    }*/
    key_free(k);
/*  if (!private)
        fprintf(stdout, "\n");*/
    BIO_free(bio_p);
    free(blob);
    free(encoded);
    return TRUE;
//  exit(0);
}
