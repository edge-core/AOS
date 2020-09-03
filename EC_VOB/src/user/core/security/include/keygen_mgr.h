/* MODULE NAME:  keygen_mgr.h
* PURPOSE:
*   Initialize the resource and provide some functions for the keygen module.
*
* NOTES:
*
* History:
*       Date          -- Modifier,  Reason
*     2002-10-16      -- Isiah , created.
*
* Copyright(C)      Accton Corporation, 2002
*/

#ifndef KEYGEN_MGR_H

#define KEYGEN_MGR_H



/* INCLUDE FILE DECLARATIONS
 */
#include <stddef.h>
#include "sys_cpnt.h"
#include "sys_type.h"
#include "keygen_type.h"
#include "sysfun.h"

/* NAMING CONSTANT DECLARATIONS
 */
/* The key to get keygen mgr msgq.
 */
#define KEYGEN_MGR_IPCMSG_KEY    SYS_BLD_SSH_GROUP_IPCMSGQ_KEY

/* The commands for IPC message.
 */
enum {
    KEYEGN_MGR_IPC_CMD_DELETE_HOST_KEY_PAIR,        /*  0 */
    KEYEGN_MGR_IPC_CMD_GENERATE_HOST_KEY_PAIR,      /*  1 */
    KEYEGN_MGR_IPC_CMD_GET_HOST_PUBLIC_KEY,         /*  2 */
    KEYEGN_MGR_IPC_CMD_WRITE_HOST_KEY_2_FLASH,      /*  3 */
    KEYEGN_MGR_IPC_CMD_GET_SERVER_CERTIFICATE,      /*  4 */
    KEYEGN_MGR_IPC_CMD_GET_SERVER_PRIVATE_KEY,      /*  5 */
    KEYEGN_MGR_IPC_CMD_GET_SERVER_PASS_PHRASE,      /*  6 */
    KEYEGN_MGR_IPC_CMD_WRITE_SERTIFICATE,           /*  7 */
    /* for SNMP
     */
    KEYGEN_MGR_IPC_CMD_GET_HOST_PBKEY_FNGR_PRNT,    /*  8 */
    KEYGEN_MGR_IPC_CMD_GET_NXT_USER_PBKEY,          /*  9 */
};


/* MACRO DEFINITIONS
 */
/*-------------------------------------------------------------------------
 * MACRO NAME - KEYGEN_MGR_GET_MSGBUFSIZE
 *-------------------------------------------------------------------------
 * PURPOSE : Get the size of KEYGEN message that contains the specified
 *           type of data.
 * INPUT   : type_name - the type name of data for the message.
 * OUTPUT  : None
 * RETURN  : The size of KEYGEN message.
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
#define KEYGEN_MGR_GET_MSGBUFSIZE(type_name) \
    (offsetof(KEYGEN_MGR_IPCMsg_T, data) + sizeof(type_name))

/*-------------------------------------------------------------------------
 * MACRO NAME - KEYGEN_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA
 *-------------------------------------------------------------------------
 * PURPOSE : Get the size of KEYGEN message that has no data block.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : The size of KEYGEN message.
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
#define KEYGEN_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA() \
    sizeof(KEYGEN_MGR_IPCMsg_Type_T)

/*-------------------------------------------------------------------------
 * MACRO NAME - KEYGEN_MGR_MSG_CMD
 *              KEYGEN_MGR_MSG_RETVAL
 *-------------------------------------------------------------------------
 * PURPOSE : Get the KEYGEN command/return value of an IPC message.
 * INPUT   : msg_p - the IPC message.
 * OUTPUT  : None
 * RETURN  : The KEYGEN command/return value; it's allowed to be used as lvalue.
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
#define KEYGEN_MGR_MSG_CMD(msg_p)    (((KEYGEN_MGR_IPCMsg_T *)(msg_p)->msg_buf)->type.cmd)
#define KEYGEN_MGR_MSG_RETVAL(msg_p) (((KEYGEN_MGR_IPCMsg_T *)(msg_p)->msg_buf)->type.result)

/*-------------------------------------------------------------------------
 * MACRO NAME - KEYGEN_MGR_MSG_DATA
 *-------------------------------------------------------------------------
 * PURPOSE : Get the data block of an IPC message.
 * INPUT   : msg_p - the IPC message.
 * OUTPUT  : None
 * RETURN  : The pointer of the data block.
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
#define KEYGEN_MGR_MSG_DATA(msg_p)   ((void *)&((KEYGEN_MGR_IPCMsg_T *)(msg_p)->msg_buf)->data)


/* DATA TYPE DECLARATIONS
 */
/* Message declarations for IPC.
 */
typedef union
{
    UI32_T  cmd;    /* for sending IPC request. CSCA_MGR_IPC_CMD1 ... */
    UI32_T  result; /* for response */
} KEYGEN_MGR_IPCMsg_Type_T;

typedef struct
{
    UI32_T  key_type;
} KEYGEN_MGR_IPCMsg_HostKeyType_T; /* for gen/delete host key pair */

    typedef struct
    {
        UI8_T   rsa_key[HOST_RSA_KEY_1024B_FILE_LENGTH];
        UI8_T   dsa_key[HOST_DSA_KEY_1024B_FILE_LENGTH];
    } KEYGEN_MGR_IPCMsg_GetHostPublicKey_T;

    typedef struct
    {
        UI8_T   username[SYS_ADPT_MAX_USER_NAME_LEN+1];
        UI8_T   rsa_key[USER_RSA_PUBLIC_KEY_FILE_LENGTH + 1];
        UI8_T   dsa_key[USER_DSA_PUBLIC_KEY_FILE_LENGTH + 1];
    }KEYGEN_MGR_IPCMsg_UserPbkey_T;

    typedef struct
    {
        UI8_T rsa_sha1_fp[SIZE_sshRsaHostKeySHA1FingerPrint+1];
        UI8_T rsa_md5_fp [SIZE_sshRsaHostKeyMD5FingerPrint+1];
        UI8_T dsa_sha1_fp[SIZE_sshDsaHostKeySHA1FingerPrint+1];
        UI8_T dsa_md5_fp [SIZE_sshDsaHostKeyMD5FingerPrint+1];
    }KEYGEN_MGR_IPCMsg_HostPbkeyFngrPrnt_T;

typedef struct
{
    UI8_T cert[SERVER_CERTIFICATE_FILE_LENGTH];
}KEYGEN_MGR_IPCMsg_GetServerCertificate_T;

typedef struct
{
    UI8_T server_private[SERVER_PRIVATE_KEY_FILE_LENGTH];
}KEYGEN_MGR_IPCMsg_GetServerPrivateKey_T;

typedef struct
{
    UI8_T pass_phrase[SERVER_PASS_PHRASE_FILE_LENGTH];
}KEYGEN_MGR_IPCMsg_GetServerPassPhrase_T;

typedef struct
{
    UI8_T cert[SERVER_CERTIFICATE_FILE_LENGTH];
    UI8_T server_private[SERVER_PRIVATE_KEY_FILE_LENGTH];
    UI8_T pass_phrase[SERVER_PASS_PHRASE_FILE_LENGTH];
}KEYGEN_MGR_IPCMsg_ServerData_T;

typedef union
{
    KEYGEN_MGR_IPCMsg_HostKeyType_T             hktype;
    KEYGEN_MGR_IPCMsg_GetHostPublicKey_T        get_hpb;
    KEYGEN_MGR_IPCMsg_UserPbkey_T               user_pbkey;
    KEYGEN_MGR_IPCMsg_HostPbkeyFngrPrnt_T       host_pbkey_fp;
    KEYGEN_MGR_IPCMsg_GetServerCertificate_T    cert;
    KEYGEN_MGR_IPCMsg_GetServerPrivateKey_T     server_private;
    KEYGEN_MGR_IPCMsg_GetServerPassPhrase_T     server_pass;
    KEYGEN_MGR_IPCMsg_ServerData_T              server_data;
} KEYGEN_MGR_IPCMsg_Data_T;

typedef struct
{
    KEYGEN_MGR_IPCMsg_Type_T    type;
    KEYGEN_MGR_IPCMsg_Data_T    data;
} KEYGEN_MGR_IPCMsg_T;


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/* FUNCTION NAME:  KEYGEN_MGR_Init
 * PURPOSE:
 *          Initiate the semaphore for KEYGEN objects
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
 *          This function is invoked in KEYGEN_INIT_Initiate_System_Resources.
 */
BOOL_T KEYGEN_MGR_Init(void);



/* FUNCTION NAME:  KEYGEN_MGR_EnterMasterMode
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
 *				switch will be initiated to the factory default value.
 *			2. KEYGEN will handle network requests only when this subsystem
 *				is in the Master Operation mode
 *			3. This function is invoked in KEYGEN_INIT_EnterMasterMode.
 */
BOOL_T KEYGEN_MGR_EnterMasterMode(void);



/* FUNCTION NAME:  KEYGEN_MGR_EnterTransitionMode
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
 *			.
 */
BOOL_T KEYGEN_MGR_EnterTransitionMode(void);



/* FUNCTION NAME:  KEYGEN_MGR_EnterSlaveMode
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
void KEYGEN_MGR_EnterSlaveMode(void);



/* FUNCTION	NAME : KEYGEN_MGR_SetTransitionMode
 * PURPOSE:
 *		Set transition mode.
 *
 * INPUT:
 *		None.
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		None.
 *
 * NOTES:
 *		None.
 */
void KEYGEN_MGR_SetTransitionMode(void);



/* FUNCTION	NAME : KEYGEN_MGR_GetOperationMode
 * PURPOSE:
 *		Get current sshd operation mode (master / slave / transition).
 *
 * INPUT:
 *		None.
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *          SYS_TYPE_Stacking_Mode_T - opmode.
 *
 * NOTES:
 *		None.
 */
SYS_TYPE_Stacking_Mode_T KEYGEN_MGR_GetOperationMode(void);



/* FUNCTION NAME:  KEYGEN_MGR_GenTempPublicKeyPair
 * PURPOSE:
 *			KEYGEN private key generate main routine.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          return TRUE to indicate success and FASLE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T KEYGEN_MGR_GenTempPublicKeyPair();



/* FUNCTION NAME:  KEYGEN_MGR_GetServerCertificate
 * PURPOSE:
 *          Get server certificate.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          UI8_T   *cert   -- server certificate.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T KEYGEN_MGR_GetServerCertificate(UI8_T *cert);



/* FUNCTION NAME:  KEYGEN_MGR_GetServerPrivateKey
 * PURPOSE:
 *          Get server private key.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          UI8_T   *key    -- server private key.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T KEYGEN_MGR_GetServerPrivateKey(UI8_T *key);



/* FUNCTION NAME:  KEYGEN_MGR_GetServerPassPhrase
 * PURPOSE:
 *          Get password for read server private key.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          UI8_T   *passwd -- password for read server private key.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T KEYGEN_MGR_GetServerPassPhrase(UI8_T *passwd);



/* FUNCTION NAME:  KEYGEN_MGR_GetHttpsTempPublicKeyPair
 * PURPOSE:
 *          Get 512 and 1024 bits temporary keys
 *
 * INPUT:
 *          int nExport -- 1 to indicate an export cipher is used and 0 to indicate not.
 *
 *          int nKeyLen -- temporary key length,valid value are 512 and 1024 .
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          RSA *rsa    -- RSA structure.
 * NOTES:
 *          This API is invoked in HTTP_Get_Temp_PublicKey_Pair
 */
void *KEYGEN_MGR_GetHttpsTempPublicKeyPair(int nExport, int nKeyLen);



/* FUNCTION NAME:  KEYGEN_MGR_SetNewCertificate
 * PURPOSE:
 *          Set new certificate, private key, and pass phrase to Files_Buffer and file system.
 *
 * INPUT:
 *          UI8_T   *server_certificate --  pointer of server certificate.
 *          UI8_T   *server_private_key --  pointer of server private key.
 *          UI8_T   *server_pass_phrase --  pointer of key pass phrase.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T KEYGEN_MGR_SetNewCertificate(UI8_T *server_certificate, UI8_T *server_private_key, UI8_T *server_pass_phrase);



/* FUNCTION NAME:  KEYGEN_MGR_GetSshdHostkey
 * PURPOSE:
 *          This function get hostkey of sshd.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          void * -pointer of rsa key .
 * NOTES:
 *          .
 */
void *KEYGEN_MGR_GetSshdHostkey();



/* FUNCTION NAME:  KEYGEN_MGR_GetSshRsadHostkey
 * PURPOSE:
 *          This function get RSA hostkey of sshd.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          void * -pointer of rsa key .
 * NOTES:
 *          .
 */
void *KEYGEN_MGR_GetSshdRsaHostkey();



/* FUNCTION NAME:  KEYGEN_MGR_GetSshDsadHostkey
 * PURPOSE:
 *          This function get DSA hostkey of sshd.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          void * -pointer of rsa key .
 * NOTES:
 *          .
 */
void *KEYGEN_MGR_GetSshdDsaHostkey();



/* FUNCTION NAME:  KEYGEN_MGR_GetSshdServerkey
 * PURPOSE:
 *          This function get serverkey of sshd.
 *
 * INPUT:
 *          UI32_T - index of RSA key.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          void * -pointer of rsa key .
 * NOTES:
 *          .
 */
void *KEYGEN_MGR_GetSshdServerkey(UI32_T index);



/* FUNCTION NAME : KEYGEN_MGR_GenerateHostKeyPair
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
 *      This API is invoked in CLI command "ip ssh crypto host-key grnerate".
 *      This function generate a host key only storage in memory.
 */
BOOL_T KEYGEN_MGR_GenerateHostKeyPair(UI32_T key_type);



/* FUNCTION NAME : KEYGEN_MGR_WriteHostKey2Flash
 * PURPOSE:
 *      This function write host key from memory to flash.
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN:
 *      TRUE  - Success.
 *      FALSE - Fault.
 *
 * NOTES:
 *      This API is invoked in CLI command "ip ssh save host-key".
 */
BOOL_T KEYGEN_MGR_WriteHostKey2Flash();



/* FUNCTION NAME : KEYGEN_MGR_DeleteHostKeyPair
 * PURPOSE:
 *      Delete RSA/DSA public and private key pair of host key.
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
 *      This API is invoked in CLI command "ip ssh crypto zeroize".
 *      This function only clear a host key from memory.
 */
BOOL_T KEYGEN_MGR_DeleteHostKeyPair(UI32_T key_type);



/* FUNCTION NAME : KEYGEN_MGR_CheckSshdHostkey
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
BOOL_T KEYGEN_MGR_CheckSshdHostkey();



/* FUNCTION NAME : KEYGEN_MGR_GetHostPublicKey
 * PURPOSE:
 *      Get host public key type.
 *
 * INPUT:
 *      none.
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
BOOL_T KEYGEN_MGR_GetHostPublicKey(UI8_T *rsa_key, UI8_T *dsa_key);



/* FUNCTION NAME:  KEYGEN_MGR_SetUserPublicKey
 * PURPOSE:
 *          This function set user public key.
 *
 * INPUT:
 *          UI8_T   *public_key --  user public key buffer.
 *          UI8_T   *username   --  username.
 *          UI32_T  key_type    --  Type of user public key.(KEY_TYPE_RSA, KEY_TYPE_DSA)
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *      One of KEYGEN_SetUserPublicKeyResult_T to indicate that the result.
 * NOTES:
 *          .
 */
KEYGEN_SetUserPublicKeyResult_T KEYGEN_MGR_SetUserPublicKey(UI8_T *public_key, UI8_T *username, UI32_T key_type);



/* FUNCTION NAME:  KEYGEN_MGR_DeleteUserPublicKey
 * PURPOSE:
 *          This function delete user public key.
 *
 * INPUT:
 *			UI8_T   *username   --  name of user of public key will be delete.
 *			UI32_T  key_type    --  Type of host key.(KEY_TYPE_RSA, KEY_TYPE_DSA and KEY_TYPE_BOTH_RSA_AND_DSA)
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T KEYGEN_MGR_DeleteUserPublicKey(UI8_T *username, UI32_T key_type);



/* FUNCTION NAME : KEYGEN_MGR_GetUserPublicKeyType
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
 *      This API is invoked in SSHD_MGR_GetUserPublicKeyType()
 */
BOOL_T KEYGEN_MGR_GetUserPublicKeyType(UI8_T *username, UI32_T *key_type);



/* FUNCTION NAME : KEYGEN_MGR_GetUserPublicKey
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
 *      This API is invoked in SSHD_MGR_GetUserPublicKey()
 */
BOOL_T KEYGEN_MGR_GetUserPublicKey(UI8_T *username, UI8_T *rsa_key, UI8_T *dsa_key);



/* FUNCTION NAME : KEYGEN_MGR_GetNextUserPublicKey
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
 *      This API is invoked in SSHD_MGR_GetNextUserPublicKey()
 *      Initiation username is empty string.
 */
BOOL_T KEYGEN_MGR_GetNextUserPublicKey(UI8_T *username, UI8_T *rsa_key, UI8_T *dsa_key);

/* FUNCTION NAME : KEYGEN_MGR_HandleGenerateHostKeyPairEvent
 * PURPOSE:
 *      Generate RSA/DSA public and private key pair of host key.
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      TRUE  - Success.
 *      FALSE - Fault.
 *
 * NOTES:
 *
 */
BOOL_T KEYGEN_MGR_HandleAsyncWriteHostKey2FlashEvent(SYSFUN_Msg_T *msg_p);


/* FUNCTION NAME : KEYGEN_MGR_AsyncGenerateHostKeyPair
 * PURPOSE:
 *      Generate RSA/DSA public and private key pair of host key.
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      TRUE  - Success.
 *      FALSE - Fault.
 *
 * NOTES:
 *      This API is invoked in SSHD_MGR_AsyncGenerateHostKeyPair().
 */
BOOL_T KEYGEN_MGR_AsyncGenerateHostKeyPair();

/* FUNCTION NAME : KEYGEN_MGR_AsyncDeleteUserPublicKey
 * PURPOSE:
 *      Delete; RSA/DSA public key of user key.
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      TRUE  - Success.
 *      FALSE - Fault.
 *
 * NOTES:
 *      This API is invoked in SSHD_MGR_AsyncDeleteUserPublicKey().
 */
BOOL_T KEYGEN_MGR_HandleDeleteUserPublicKeyEvent(SYSFUN_Msg_T *msg_p);
//richBOOL_T KEYGEN_MGR_HandleDeleteUserPublicKeyEvent(Msg_T *msg);

/* FUNCTION NAME : KEYGEN_MGR_AsyncDeleteUserPublicKey
 * PURPOSE:
 *      Delete; RSA/DSA public key of user key.
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      TRUE  - Success.
 *      FALSE - Fault.
 *
 * NOTES:
 *      This API is invoked in SSHD_MGR_AsyncDeleteUserPublicKey().
 */
BOOL_T KEYGEN_MGR_AsyncDeleteUserPublicKey(UI8_T *username);

/* FUNCTION NAME : KEYGEN_MGR_AsyncWriteHostKey2Flash
 * PURPOSE:
 *      This function write host key from memory to flash.
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN:
 *      TRUE  - Success.
 *      FALSE - Fault.
 *
 * NOTES:
 *      This API is invoked in SSHD_MGR_AsyncWriteHostKey2Flash.
 */
//richBOOL_T KEYGEN_MGR_HandleAsyncWriteHostKey2FlashEvent(Msg_T *msg);

/* FUNCTION NAME : KEYGEN_MGR_AsyncWriteHostKey2Flash
 * PURPOSE:
 *      This function write host key from memory to flash.
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN:
 *      TRUE  - Success.
 *      FALSE - Fault.
 *
 * NOTES:
 *      This API is invoked in SSHD_MGR_AsyncWriteHostKey2Flash.
 */
BOOL_T KEYGEN_MGR_AsyncWriteHostKey2Flash();



/* FUNCTION NAME : KEYGEN_MGR_GetHostPublicKeyFingerPrint
 * PURPOSE:
 *      Get host public key fingerprint.
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      UI8_T   *rsa_sha1_fingerprint   --  pointer of buffer to storage rsa public key sha1 fingerprint.
 *      UI8_T   *rsa_md5_fingerprint    --  pointer of buffer to storage rsa public key md5 fingerprint.
 *      UI8_T   *dsa_sha1_fingerprint   --  pointer of buffer to storage dsa public key sha1 fingerprint.
 *      UI8_T   *dsa_md5_fingerprint    --  pointer of buffer to storage dsa public key md5 fingerprint.
 *
 * RETURN:
 *      TRUE  - Success.
 *      FALSE - Fault.
 *
 * NOTES:
 *      This API is invoked in UI.
 */
BOOL_T KEYGEN_MGR_GetHostPublicKeyFingerPrint(UI8_T *rsa_sha1_fingerprint, UI8_T *rsa_md5_fingerprint, UI8_T *dsa_sha1_fingerprint, UI8_T *dsa_md5_fingerprint);



/* FUNCTION NAME : KEYGEN_MGR_SetRandomString
 * PURPOSE:
 *      This function provide user input any random string to improve random seed for
 *      generate host key. This function can accept any length srting.
 *
 * INPUT:
 *      UI8_T   *random_string  --  random string.
 *      UI32_T  string_length   --  length of random string.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      TRUE  - Success.
 *      FALSE - Fault.
 *
 * NOTES:
 *      This API is invoked in UI, if you want to improve random seeed for generate host key.
 */
BOOL_T KEYGEN_MGR_SetRandomString(UI8_T *random_string, UI32_T string_length);




/* FUNCTION NAME - KEYGEN_MGR_HandleHotInsertion
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
void KEYGEN_MGR_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default);



/* FUNCTION NAME - KEYGEN_MGR_HandleHotRemoval
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
void KEYGEN_MGR_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port);



/* FUNCTION NAME:  KEYGEN_MGR_InitCertificateFiles
 * PURPOSE:
 *          Set default files to Files_Buffer and file system.
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
 *          none.
 */
BOOL_T KEYGEN_MGR_InitCertificateFiles();

/* FUNCTION NAME : KEYGEN_MGR_Dsa2SshDss
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
 *      This API is copy from SSHD_MGR_Dsa2SshDss() for code layer issue,
 *      and let buffer.c(.h), Bufaux.c, Uuencode.c, Base64.c be library
 */
BOOL_T KEYGEN_MGR_Dsa2SshDss(void *dsa, UI8_T *dsa_key);

/* FUNCTION NAME : KEYGEN_MGR_Dsa2FingerPrint
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
 *      This API is copy from SSHD_MGR_Dsa2SshDss() for code layer issue,
 *      and let buffer.c(.h), Bufaux.c, Key.c
 */
BOOL_T KEYGEN_MGR_Dsa2FingerPrint(void *dsa, UI8_T *dsa_sha1_fingerprint, UI8_T *dsa_md5_fingerprint);

/* FUNCTION NAME : KEYGEN_MGR_Rsa2FingerPrint
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
 *      This API is copy from SSHD_MGR_Dsa2SshDss() for code layer issue,
 *      and let buffer.c(.h), Bufaux.c, Key.c
 */
BOOL_T KEYGEN_MGR_Rsa2FingerPrint(void *rsa, UI8_T *rsa_sha1_fingerprint, UI8_T *rsa_md5_fingerprint);


#endif /* #ifndef KEYGEN_MGR_H */



