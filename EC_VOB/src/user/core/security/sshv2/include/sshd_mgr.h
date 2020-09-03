/* MODULE NAME:  sshd_mgr.h
* PURPOSE:
*   Initialize the resource and provide some functions for the sshd module.
*
* NOTES:
*
* History:
*       Date          -- Modifier,  Reason
*     2003-03-27      -- Isiah , created.
*
* Copyright(C)      Accton Corporation, 2003
*/

#ifndef SSHD_MGR_H

#define SSHD_MGR_H

#include "openssl/bn.h"

/* INCLUDE FILE DECLARATIONS
 */
#include <stddef.h>
#include "sys_type.h"
#include "sshd_type.h"
#include "sysfun.h"
#include "keygen_type.h"
#include "l_inet.h"
#include "userauth.h"

/* NAMING CONSTANT DECLARATIONS
 */
/* The key to get SSHD mgr msgq.
 */
#define SSHD_MGR_IPCMSG_KEY    SYS_BLD_SSH_GROUP_IPCMSGQ_KEY

/* The commands for IPC message.
 */
enum {
    SSHD_MGR_IPC_CMD_DEL_USER_PBKEY,
    SSHD_MGR_IPC_CMD_GET_AUTHEN_RETRIES,
    SSHD_MGR_IPC_CMD_GET_NEGO_TIMEOUT,
    SSHD_MGR_IPC_CMD_GET_NEXT_CONN_ENTRY,
    SSHD_MGR_IPC_CMD_GET_NEXT_USER_PBKEY,
    SSHD_MGR_IPC_CMD_GET_SERVER_KEYSIZE,
    SSHD_MGR_IPC_CMD_GET_SSH_CONN_NAME,
    SSHD_MGR_IPC_CMD_GET_SSH_CONN_PRIV,
    SSHD_MGR_IPC_CMD_GET_SSHD_STATUS,
    SSHD_MGR_IPC_CMD_GET_SERVER_VERSION,
    SSHD_MGR_IPC_CMD_GET_USER_PBKEY,
    SSHD_MGR_IPC_CMD_SET_AUTHEN_RETRIES,
    SSHD_MGR_IPC_CMD_SET_CID_AND_TNSHID,
    SSHD_MGR_IPC_CMD_SET_NEGO_TIMEOUT,
    SSHD_MGR_IPC_CMD_SET_SERVER_KEYSIZE,
    SSHD_MGR_IPC_CMD_SET_SSHD_STATUS,
    SSHD_MGR_IPC_CMD_GET_RUNN_SERVER_KEYSIZE,
    SSHD_MGR_IPC_CMD_GET_GENERATE_HOST_KEY_STATUS,
    SSHD_MGR_IPC_CMD_GET_GENERATE_HOST_KEY_ACTION,
    SSHD_MGR_IPC_CMD_GET_WRITE_HOST_KEY_2_FLASH_STATUS,
    SSHD_MGR_IPC_CMD_GET_WRITE_HOST_KEY_2_FLASH_ACTION,
    SSHD_MGR_IPC_CMD_ASYNC_WRITE_HOST_KEY_2_FLASH,
    SSHD_MGR_IPC_CMD_ASYNC_GENERATE_HOST_KEY_PAIR,
    SSHD_MGR_IPC_CMD_GET_CONN_ENTRY,
    SSHD_MGR_IPC_CMD_ASYNC_DELETE_USER_PUBLIC_KEY,
    SSHD_MGR_IPC_CMD_GET_DELETE_USER_PUBLIC_KEY_ACTION,
    /*SSHD_MGR_IPC_CMD_GET_USER_PUBLIC_KEY_FROM_XFER,*/
    SSHD_MGR_IPC_CMD_SET_USER_PUBLIC_KEY,
    SSHD_MGR_IPC_CMD_COPY_USER_PUBLIC_KEY,
    SSHD_MGR_IPC_CMD_GET_USER_PUBLIC_KEY_TYPE,
};

/* MACRO FUNCTION DECLARATIONS
 */
/*-------------------------------------------------------------------------
 * MACRO NAME - SSHD_MGR_GET_MSGBUFSIZE
 *-------------------------------------------------------------------------
 * PURPOSE : Get the size of SSHD message that contains the specified
 *           type of data.
 * INPUT   : type_name - the type name of data for the message.
 * OUTPUT  : None
 * RETURN  : The size of SSHD message.
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
#define SSHD_MGR_GET_MSGBUFSIZE(type_name) \
    (offsetof(SSHD_MGR_IPCMsg_T, data) + sizeof(type_name))

/*-------------------------------------------------------------------------
 * MACRO NAME - SSHD_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA
 *-------------------------------------------------------------------------
 * PURPOSE : Get the size of SSHD message that has no data block.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : The size of SSHD message.
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
#define SSHD_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA() \
    sizeof(SSHD_MGR_IPCMsg_Type_T)

/*-------------------------------------------------------------------------
 * MACRO NAME - SSHD_MGR_MSG_CMD
 *              SSHD_MGR_MSG_RETVAL
 *-------------------------------------------------------------------------
 * PURPOSE : Get the SSHD command/return value of an IPC message.
 * INPUT   : msg_p - the IPC message.
 * OUTPUT  : None
 * RETURN  : The SSHD command/return value; it's allowed to be used as lvalue.
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
#define SSHD_MGR_MSG_CMD(msg_p)    (((SSHD_MGR_IPCMsg_T *)(msg_p)->msg_buf)->type.cmd)
#define SSHD_MGR_MSG_RETVAL(msg_p) (((SSHD_MGR_IPCMsg_T *)(msg_p)->msg_buf)->type.result)

/*-------------------------------------------------------------------------
 * MACRO NAME - SSHD_MGR_MSG_DATA
 *-------------------------------------------------------------------------
 * PURPOSE : Get the data block of an IPC message.
 * INPUT   : msg_p - the IPC message.
 * OUTPUT  : None
 * RETURN  : The pointer of the data block.
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
#define SSHD_MGR_MSG_DATA(msg_p)   ((void *)&((SSHD_MGR_IPCMsg_T *)(msg_p)->msg_buf)->data)

/* DATA TYPE DECLARATIONS
 */
/* Message declarations for IPC.
 */
typedef union
{
    UI32_T  cmd;        /* for sending IPC request. CSCA_MGR_IPC_CMD1 ... */
    UI32_T  result;     /* for response */
} SSHD_MGR_IPCMsg_Type_T;

typedef struct
{
    UI32_T  key_type;
    UI8_T   username [SYS_ADPT_MAX_USER_NAME_LEN+1];
} SSHD_MGR_IPCMsg_DelUserPbKey_T;

typedef struct
{
    I32_T                   cid;
    SSHD_ConnectionInfo_T   info;
} SSHD_MGR_IPCMsg_ConnEntry_T;

typedef struct
{
    UI8_T   username [SYS_ADPT_MAX_USER_NAME_LEN+1];
    UI8_T   rsa_key[USER_RSA_PUBLIC_KEY_FILE_LENGTH + 1];
    UI8_T   dsa_key[USER_DSA_PUBLIC_KEY_FILE_LENGTH + 1];
} SSHD_MGR_IPCMsg_UserPbKey_T;

typedef struct
{
    UI32_T  key_size;
} SSHD_MGR_IPCMsg_SvrKSize_T;

typedef struct
{
    UI32_T  major;
    UI32_T  minor;
} SSHD_MGR_IPCMsg_SvrVersion_T;

typedef struct
{
    UI32_T  retries;
} SSHD_MGR_IPCMsg_AuthenRetries_T;

typedef struct
{
    UI32_T  tnsh_port;
    UI32_T  tid;
    UI32_T  cid;
} SSHD_MGR_IPCMsg_CidAndTnshid_T;

typedef struct
{
    UI32_T  timeout;
} SSHD_MGR_IPCMsg_NegoTimeout_T;

typedef struct
{
    UI32_T  state;
} SSHD_MGR_IPCMsg_SshdState_T;

typedef struct
{
    UI32_T  cid;
    UI8_T   username [SYS_ADPT_MAX_USER_NAME_LEN+1];
} SSHD_MGR_IPCMsg_SshdConnName_T;

typedef struct
{
    UI32_T  cid;
    UI32_T  privilege;
} SSHD_MGR_IPCMsg_SshdConnPriv_T;

typedef struct
{
    UI32_T  state;
} SSHD_MGR_IPCMsg_SshdGenHostKeyState_T;

typedef struct
{
    UI32_T  action;
} SSHD_MGR_IPCMsg_SshdGenHostKeyAction_T;

typedef struct
{
    UI32_T  state;
} SSHD_MGR_IPCMsg_SshdWriteHK2FState_T;

typedef struct
{
    UI32_T  action;
} SSHD_MGR_IPCMsg_SshdWriteHK2FAction_T;

typedef struct
{
    UI32_T  type;
} SSHD_MGR_IPCMsg_SshdAsyncGenHKPair_T;

typedef struct
{
    UI32_T  action;
} SSHD_MGR_IPCMsg_SshdDeleteUPKeyAction_T;

typedef struct
{
    UI8_T   username [SYS_ADPT_MAX_USER_NAME_LEN+1];
    UI32_T  type;
	I32_T   key_buf_offset;
} SSHD_MGR_IPCMsg_SshdSetUserPublicKey_T;

typedef struct
{
    UI8_T   username [SYS_ADPT_MAX_USER_NAME_LEN+1];
    UI32_T  type;
} SSHD_MGR_IPCMsg_SshdGetUserPublicKeyType_T;

typedef union
{
    SSHD_MGR_IPCMsg_DelUserPbKey_T      dupbk; /* for del user public key           */
    SSHD_MGR_IPCMsg_UserPbKey_T         gupbk; /* for get/ getnext user public key  */
    SSHD_MGR_IPCMsg_ConnEntry_T         conne; /* for get next connection entry     */
    SSHD_MGR_IPCMsg_SvrKSize_T          svrkz; /* for get/set server key size       */
    SSHD_MGR_IPCMsg_SvrVersion_T        svrsn; /* for get/set server version        */
    SSHD_MGR_IPCMsg_AuthenRetries_T     aretr; /* for get/set authen retries        */
    SSHD_MGR_IPCMsg_CidAndTnshid_T      catns; /* for set cid and tnshid            */
    SSHD_MGR_IPCMsg_NegoTimeout_T       ntout; /* for get/set nego timeout          */
    SSHD_MGR_IPCMsg_SshdState_T         state; /* for get/set sshd state            */
    SSHD_MGR_IPCMsg_SshdConnName_T      connu; /* for get sshd connection username  */
    SSHD_MGR_IPCMsg_SshdConnPriv_T      connp; /* for get sshd connection privilege */
    SSHD_MGR_IPCMsg_SshdGenHostKeyState_T  ghkst; /* for get/set sshd generate host key state */
    SSHD_MGR_IPCMsg_SshdGenHostKeyAction_T ghkac; /* for get/set sshd generate host key action */
    SSHD_MGR_IPCMsg_SshdWriteHK2FState_T   gwhfs; /* for get/set sshd write host key 2 flash status */
    SSHD_MGR_IPCMsg_SshdWriteHK2FAction_T  gwhfa; /* for get/set sshd write host key 2 flash action */
    SSHD_MGR_IPCMsg_SshdAsyncGenHKPair_T   aghkp;  /* for get/set sshd type of generate host key pair */
    SSHD_MGR_IPCMsg_SshdDeleteUPKeyAction_T  dupka;  /* for get delete user public key action */
    SSHD_MGR_IPCMsg_SshdSetUserPublicKey_T setpublickey; /* set user public key */
    SSHD_MGR_IPCMsg_SshdGetUserPublicKeyType_T getpublickeytype;

} SSHD_MGR_IPCMsg_Data_T;

typedef struct
{
    SSHD_MGR_IPCMsg_Type_T    type;
    SSHD_MGR_IPCMsg_Data_T    data;
} SSHD_MGR_IPCMsg_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
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
BOOL_T SSH_MGR_HandleIPCReqMsg(SYSFUN_Msg_T* ipcmsg_p);

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
BOOL_T SSHD_MGR_Init(void);



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
BOOL_T SSHD_MGR_EnterMasterMode(void);



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
BOOL_T SSHD_MGR_EnterTransitionMode(void);



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
void SSHD_MGR_EnterSlaveMode(void);



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
void SSHD_MGR_SetTransitionMode(void);



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
SYS_TYPE_Stacking_Mode_T SSHD_MGR_GetOperationMode(void);

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
BOOL_T SSHD_MGR_SetLastPublicKeyIp(UI32_T tftp_server_ip);

/*------------------------------------------------------------------------
 * ROUTINE NAME - SSHD_MGR_GetLastPublicKeyIp
 *------------------------------------------------------------------------
 * FUNCTION: Get the last ip for publickey
 * INPUT   : None
 * OUTPUT  : UI32_T  *tftp_server_ip_p
 * RETURN  : TRUE/FALSE
 * NOTE    :
 *------------------------------------------------------------------------*/
BOOL_T SSHD_MGR_GetLastPublicKeyIp(UI32_T *tftp_server_ip_p) ;

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
BOOL_T SSHD_MGR_SetLastPublicKeyFileName(UI8_T *file_name_p);

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
BOOL_T SSHD_MGR_GetLastPublicKeyFileName(UI8_T *file_name_p);
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
BOOL_T SSHD_MGR_SetSshdStatus(SSHD_State_T state);



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
SSHD_State_T SSHD_MGR_GetSshdStatus(void);



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
BOOL_T SSHD_MGR_SetSshdPort (UI32_T port);



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
UI32_T SSHD_MGR_GetSshdPort(void);



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
BOOL_T SSHD_MGR_SetAuthenticationRetries(UI32_T retries);



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
UI32_T SSHD_MGR_GetAuthenticationRetries(void);



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
BOOL_T SSHD_MGR_SetNegotiationTimeout(UI32_T timeout);



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
UI32_T SSHD_MGR_GetNegotiationTimeout(void);



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
BOOL_T SSHD_MGR_GetSshServerVersion(UI32_T *major, UI32_T *minor);



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
SYS_TYPE_Get_Running_Cfg_T  SSHD_MGR_GetRunningNegotiationTimeout(UI32_T *timeout);



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
SYS_TYPE_Get_Running_Cfg_T  SSHD_MGR_GetRunningAuthenticationRetries(UI32_T *retries);



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
SYS_TYPE_Get_Running_Cfg_T  SSHD_MGR_GetRunningSshdStatus(UI32_T *state);



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
BOOL_T SSHD_MGR_SetServerKeySize(UI32_T key_size);



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
BOOL_T SSHD_MGR_GetServerKeySize(UI32_T *key_size);



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
SYS_TYPE_Get_Running_Cfg_T  SSHD_MGR_GetRunningServerKeySize(UI32_T *key_size);





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
SSHD_SetUserPublicKeyResult_T SSHD_MGR_SetUserPublicKey(UI8_T *username, UI32_T key_type);



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
BOOL_T SSHD_MGR_DeleteUserPublicKey(UI8_T *username, UI32_T key_type);



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
BOOL_T SSHD_MGR_GetUserPublicKeyType(UI8_T *username, UI32_T *key_type);



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
BOOL_T SSHD_MGR_GetUserPublicKey(UI8_T *username, UI8_T *rsa_key, UI8_T *dsa_key);



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
BOOL_T SSHD_MGR_Dsa2SshDss(void *dsa, UI8_T *dsa_key);



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
BOOL_T SSHD_MGR_GetNextUserPublicKey(UI8_T *username, UI8_T *rsa_key, UI8_T *dsa_key);



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
BOOL_T SSHD_MGR_SetConnectionIDAndTnshID(UI32_T tnsh_port, UI32_T tid, UI32_T cid);



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
BOOL_T SSHD_MGR_GetSshConnectionUsername(UI32_T cid, UI8_T *username);



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
BOOL_T SSHD_MGR_GetSshConnectionPassword(UI32_T cid, char *password, UI32_T password_size);



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
BOOL_T SSHD_MGR_GetSshConnectionPrivilege(UI32_T cid, UI32_T *privilege);



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
    USERAUTH_AuthResult_T *auth_result_p);



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
BOOL_T SSHD_MGR_GetNextSshConnectionEntry(I32_T *cid, SSHD_ConnectionInfo_T *info);



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
BOOL_T SSHD_MGR_GetSshConnectionEntry(UI32_T cid, SSHD_ConnectionInfo_T *info);



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
BOOL_T SSHD_MGR_GetSessionPair(UI32_T tnsh_port, L_INET_AddrIp_T *user_addr, UI32_T *user_port);



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
BOOL_T SSHD_MGR_AsyncGenerateHostKeyPair(UI32_T key_type);



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
BOOL_T SSHD_MGR_GetGenerateHostKeyAction(UI32_T *action_type);



int SSHD_MGR_GenerateKey_CallBack(int a, int b, BN_GENCB *cb);



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
BOOL_T SSHD_MGR_SetGenerateHostKeyAction(UI32_T action_type);



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
BOOL_T SSHD_MGR_GetGenerateHostKeyStatus(UI32_T *action_result);



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
BOOL_T SSHD_MGR_SetGenerateHostKeyStatus(UI32_T action_result);



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
BOOL_T SSHD_MGR_AsyncDeleteUserPublicKey(UI8_T *username, UI32_T key_type);



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
BOOL_T SSHD_MGR_GetDeleteUserPublicKeyAction(UI32_T *action_type);



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
BOOL_T SSHD_MGR_SetDeleteUserPublicKeyAction(UI32_T action_type);



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
BOOL_T SSHD_MGR_GetDeleteUserPublicKeyStatus(UI32_T *action_result);



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
BOOL_T SSHD_MGR_SetDeleteUserPublicKeyStatus(UI32_T action_result);



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
BOOL_T SSHD_MGR_AsyncWriteHostKey2Flash();



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
BOOL_T SSHD_MGR_GetWriteHostKey2FlashAction(UI32_T *action_type);



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
BOOL_T SSHD_MGR_SetWriteHostKey2FlashAction(UI32_T action_type);



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
BOOL_T SSHD_MGR_GetWriteHostKey2FlashStatus(UI32_T *action_result);



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
BOOL_T SSHD_MGR_SetWriteHostKey2FlashStatus(UI32_T action_result);



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
BOOL_T SSHD_MGR_SetSshPasswordAuthenticationStatus(SSHD_PasswordAuthenticationStatus_T state);



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
BOOL_T SSHD_MGR_GetSshPasswordAuthenticationStatus(SSHD_PasswordAuthenticationStatus_T *state);



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
BOOL_T SSHD_MGR_SetSshPubkeyAuthenticationStatus(SSHD_PubkeyAuthenticationStatus_T state);



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
BOOL_T SSHD_MGR_GetSshPubkeyAuthenticationStatus(SSHD_PubkeyAuthenticationStatus_T *state);



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
BOOL_T SSHD_MGR_SetSshRsaAuthenticationStatus(SSHD_RsaAuthenticationStatus_T state);



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
BOOL_T SSHD_MGR_GetSshRsaAuthenticationStatus(SSHD_RsaAuthenticationStatus_T *state);



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
BOOL_T SSHD_MGR_Dsa2FingerPrint(void *dsa, UI8_T *dsa_sha1_fingerprint, UI8_T *dsa_md5_fingerprint);



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
BOOL_T SSHD_MGR_Rsa2FingerPrint(void *rsa, UI8_T *rsa_sha1_fingerprint, UI8_T *rsa_md5_fingerprint);



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
BOOL_T SSHD_MGR_CheckSshConnection(UI32_T cid);



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
void SSHD_MGR_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default);



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
void SSHD_MGR_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port);



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
void SSHD_MGR_BackdoorFunction();



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
BOOL_T SSHD_MGR_HandleIPCReqMsg(SYSFUN_Msg_T *ipcmsg_p);

#endif /* #ifndef SSHD_MGR_H */
