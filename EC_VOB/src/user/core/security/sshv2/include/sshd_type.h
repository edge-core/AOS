/* MODULE NAME: sshd_type.h
* PURPOSE:
*
*
* NOTES:
*
* History:
*       Date          -- Modifier,  Reason
*     2003-03-27      -- Isiah , created.
*
* Copyright(C)      Accton Corporation, 2002
*/



#ifndef SSHD_TYPE_H

#define SSHD_TYPE_H




/* INCLUDE FILE DECLARATIONS
 */

#include "sys_dflt.h"
#include "sys_bld.h"
#include "sys_adpt.h"







/* NAMING CONSTANT DECLARATIONS
 */

#define SSHD_DEFAULT_PORT_NUMBER    SYS_DFLT_IP_SSH_PORT
#define SSHD_DEFAULT_STATE          SYS_DFLT_IP_SSH_STATE
#define SSHD_DEFAULT_AUTHENTICATION_RETRIES		SYS_DFLT_IP_SSH_AUTHENTICATION_RETRIES
#define SSHD_DEFAULT_NEGOTIATION_TIMEOUT		SYS_DFLT_IP_SSH_TIMEOUT
#define SSHD_DEFAULT_SERVER_KEY_SIZE            SYS_DFLT_SSH_SERVER_KEY_SIZE
#define SSHD_MAX_SERVER_KEY_SIZE                SYS_ADPT_MAX_SSH_SERVER_KEY_SIZE
#define SSHD_MIN_SERVER_KEY_SIZE                SYS_ADPT_MIN_SSH_SERVER_KEY_SIZE

#define TNSHD_SERVICE_SOCKET_PORT_NUMBER        SYS_BLD_TELNET_SERVICE_SOCKET_PORT

#define SSHD_MAX_SSH_CIPHER_STRING_LEN          SYS_ADPT_MAX_SSH_CIPHER_STRING_LEN

#define SSHD_DEFAULT_PASSWORD_AUTHENTICATION_STATE  SYS_DFLT_PASSWORD_AUTHENTICATION_STATE
#define SSHD_DEFAULT_PUBKEY_AUTHENTICATION_STATE    SYS_DFLT_PUBKEY_AUTHENTICATION_STATE
#define SSHD_DEFAULT_RSA_AUTHENTICATION_STATE       SYS_DFLT_RSA_AUTHENTICATION_STATE

#define SSH_COM_PUBLIC_BEGIN		"---- BEGIN SSH2 PUBLIC KEY ----"
#define SSH_COM_PUBLIC_END		"---- END SSH2 PUBLIC KEY ----"
#define SSH_COM_PRIVATE_BEGIN		"---- BEGIN SSH2 ENCRYPTED PRIVATE KEY ----"

/* use in authentication result return & check */
#define SSH_USER_AUTH_RESULT_OK                 1
#define SSH_USER_AUTH_RESULT_FAIL               0

/* MACRO FUNCTION DECLARATIONS
 */








/* DATA TYPE DECLARATIONS
 */
typedef enum SSHD_State_E
{
    SSHD_STATE_ENABLED = 1L, /* VAL_ipSshdState_enabled  */
    SSHD_STATE_DISABLED = 2L, /* VAL_ipSshdState_disabled */
    SSHD_STATE_PROTOCOL1 = 3L,
    SSHD_STATE_PROTOCOL2 = 4L,
    SSHD_STATE_PROTOCOL_ALL = 5L
} SSHD_State_T;

typedef enum SSHD_ConnectionState_E
{
    NEGOTIATION_STARTED = 1L,
    AUTHENTICATION_STARTED = 2L,
    SESSION_STARTED = 3L
} SSHD_ConnectionState_T;



typedef enum SSHD_ConnectionEncryption_E
{
    CIPHER_NONE = 0L,
    CIPHER_DES  = 2L,
    CIPHER_3DES = 3L,
    CIPHER_BLOWFISH = 6L,
    CIPHER_AES128 = 9L,
    CIPHER_AES192 = 10L,
    CIPHER_AES256 = 11L
} SSHD_ConnectionEncryption_T;



#define SSHD_STATE_T                    SSHD_State_T
#define SSHD_CONNECTION_STATE_T         SSHD_ConnectionState_T
#define SSHD_CONNECTION_ENCRYPTION_T    SSHD_ConnectionEncryption_T
#define SSHD_CONNECTION_INFO_T          SSHD_ConnectionInfo_T

typedef struct SSHD_ConnectionInfo_S
{
	UI32_T  connection_id;
	UI32_T	major_version;
	UI32_T  minor_version;
	SSHD_ConnectionState_T	        status;
	UI8_T   cipher[SYS_ADPT_MAX_SSH_CIPHER_STRING_LEN+1];
	UI8_T   username[SYS_ADPT_MAX_USER_NAME_LEN+1];
}SSHD_ConnectionInfo_T;



typedef enum SSHD_GenerateHostKeyActionType_E
{
    NON_GENERATE_KEY = 1L,
    GENERATE_RSA_KEY,
    GENERATE_DSA_KEY,
    GENERATE_RSA_AND_DSA_KEY,
    CANCEL_GENERATING_KEY
} SSHD_GenerateHostKeyActionType_T;



typedef enum SSHD_SaveHostKeyActionType_E
{
    NON_SAVE = 1L,
    SAVING
} SSHD_SaveHostKeyActionType_T;



typedef enum SSHD_AsyncActionResult_E
{
    UNKNOW_STATE = 1L,
    SUCCESS_STATE,
    FAILURE_STATE
} SSHD_AsyncActionResult_T;



typedef enum SSHD_DeleteUserKeyActionType_E
{
    NON_DELETE_KEY = 1L,
    DELETE_RSA_KEY,
    DELETE_DSA_KEY,
    DELETE_RSA_AND_DSA_KEY
} SSHD_DeleteUserKeyActionType_T;



typedef enum SSHD_PasswordAuthenticationStatus_E
{
    ENABLED_PWD_AUTHENTICATION = 1L,
    DISABLED_PWD_AUTHENTICATION
} SSHD_PasswordAuthenticationStatus_T;



typedef enum SSHD_PubkeyAuthenticationStatus_E
{
    ENABLED_PUBKEY_AUTHENTICATION = 1L,
    DISABLED_PUBKEY_AUTHENTICATION
} SSHD_PubkeyAuthenticationStatus_T;



typedef enum SSHD_RsaAuthenticationStatus_E
{
    ENABLED_RSA_AUTHENTICATION = 1L,
    DISABLED_RSA_AUTHENTICATION
} SSHD_RsaAuthenticationStatus_T;

typedef enum
{
    SSHD_SET_USER_PUBLIC_KEY_FAIL = 0L,
    SSHD_SET_USER_PUBLIC_KEY_SUCC,
    SSHD_SET_USER_PUBLIC_KEY_INVALID_KEY,
    SSHD_SET_USER_PUBLIC_KEY_INVALID_KEY_TYPE,
    SSHD_SET_USER_PUBLIC_KEY_STORAGE_FULL,
    SSHD_SET_USER_PUBLIC_KEY_OUT_OF_MEMORY,
    SSHD_SET_USER_PUBLIC_KEY_WRITE_ERROR,
} SSHD_SetUserPublicKeyResult_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */










#endif /* ifndef SSHD_TYPE_H */



