/* MODULE NAME: sshd_type.h
* PURPOSE: 
*
*
* NOTES:
*   
* History:                                                               
*       Date          -- Modifier,  Reason
*     2002-05-24      -- Isiah , created.
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
#define SSHD_DEFAULT_MAX_SESSION_NUMBER			SYS_ADPT_MAX_SSH_NUMBER

#define TNSHD_SERVICE_SOCKET_PORT_NUMBER        SYS_BLD_TELNET_SERVICE_SOCKET_PORT

#define SSHD_MAX_SSH_VERSION_LEN	5
#define SSHD_MAX_SESSION_STATUS_LEN	31

#ifndef SSH_CIPHER_NONE
#define SSH_CIPHER_NONE		0
#endif
#ifndef SSH_CIPHER_DES
#define SSH_CIPHER_DES		2
#endif
#ifndef SSH_CIPHER_3DES
#define SSH_CIPHER_3DES		3
#endif


#define SSHD_STATE_T                    SSHD_State_T
#define SSHD_CONNECTION_STATE_T         SSHD_ConnectionState_T
#define SSHD_CONNECTION_ENCRYPTION_T    SSHD_ConnectionEncryption_T
#define SSHD_CONNECTION_INFO_T          SSHD_ConnectionInfo_T


#if 0
#ifdef HTTPS

#define	FILES_LENGTH					10240
#define RESERVED_FILES_LENGTH			16384
#define	SERVER_CERTIFICATE_FILE_LENGTH	5120
#define	SERVER_PRIVATE_KEY_FILE_LENGTH	2048
#define TEMP_RSA_KEY_512B_FILE_LENGTH	1024
#define TEMP_RSA_KEY_1024B_FILE_LENGTH	1024
#define	SERVER_PASS_PHRASE_FILE_LENGTH	512

#define	CERTIFICATE_FILENAME	"$certificate"
#endif /* end of #if 0 */

#endif /* ifdef HTTPS */


enum /* function number */
{
    SSHD_TASK_EnterMainRoutine_FUNC_NO = 0,
    SSHD_TASK_CreateTask_FUNC_NO,
    dolisten_FUNC_NO,
    SSHD_MGR_GetSessionPair_FUNC_NO,
    SSHD_MGR_GetNextSshConnectionEntry_FUNC_NO,
    SSHD_MGR_GetSshConnectionEntry_FUNC_NO,
    SSHD_MGR_CheckSshConnection_FUNC_NO,
};





/* MACRO FUNCTION DECLARATIONS
 */








/* DATA TYPE DECLARATIONS
 */
typedef enum SSHD_State_E
{
    SSHD_STATE_ENABLED = 1L, /* VAL_ipSshdState_enabled  */
    SSHD_STATE_DISABLED = 2L /* VAL_ipSshdState_disabled */
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
    CIPHER_3DES = 3L
} SSHD_ConnectionEncryption_T;



typedef struct SSHD_ConnectionInfo_S
{
	UI32_T  connection_id;
	UI32_T	major_version;
	UI32_T  minor_version;
	SSHD_ConnectionState_T	        status;
	SSHD_ConnectionEncryption_T    cipher; 
	UI8_T   username[SYS_ADPT_MAX_USER_NAME_LEN+1];
}SSHD_ConnectionInfo_T;







/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */










#endif /* ifndef SSHD_TYPE_H */



