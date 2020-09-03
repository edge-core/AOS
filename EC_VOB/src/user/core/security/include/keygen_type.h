/* MODULE NAME: keygen_type.h
* PURPOSE:
*
* NOTES:
*
* History:
*       Date          -- Modifier,  Reason
*     2002-10-16      -- Isiah , created.
*
* Copyright(C)      Accton Corporation, 2002
*/



#ifndef KEYGEN_TYPE_H

#define KEYGEN_TYPE_H




/* INCLUDE FILE DECLARATIONS
 */
#include "sys_dflt.h"
#include "sys_bld.h"
#include "sys_adpt.h"
#include "sys_cpnt.h"


/* NAMING CONSTANT DECLARATIONS
 */
#define	SERVER_CERTIFICATE_FILE_LENGTH	5120
#define	SERVER_PRIVATE_KEY_FILE_LENGTH	2048
#define TEMP_RSA_KEY_512B_FILE_LENGTH	1024
#define TEMP_RSA_KEY_768B_FILE_LENGTH	1024
#define TEMP_RSA_KEY_1024B_FILE_LENGTH	1024
#define	SERVER_PASS_PHRASE_FILE_LENGTH	512
#define	CERTIFICATE_FILENAME	        "certificate"
#define NUMBER_OF_TEMP_KEY              4
#define HOST_RSA_KEY_1024B_FILE_LENGTH  1024
#define HOST_DSA_KEY_1024B_FILE_LENGTH  1024
#define USER_RSA_PUBLIC_KEY_FILE_LENGTH 512
#define USER_DSA_PUBLIC_KEY_FILE_LENGTH 768
#define HOST_RSA_PUBLIC_KEY_SHA1_FINGERPRINT_LENGTH 128
#define HOST_DSA_PUBLIC_KEY_SHA1_FINGERPRINT_LENGTH 128
#define HOST_RSA_PUBLIC_KEY_MD5_FINGERPRINT_LENGTH  128
#define HOST_DSA_PUBLIC_KEY_MD5_FINGERPRINT_LENGTH  128
#define USER_NAME_FILE_LENGTH           (((SYS_ADPT_MAX_USER_NAME_LEN/8)+1)*8)
#define USER_PUBLIC_KEY_FILE_LENGTH     (USER_NAME_FILE_LENGTH + \
                                        USER_RSA_PUBLIC_KEY_FILE_LENGTH + \
                                        USER_DSA_PUBLIC_KEY_FILE_LENGTH)

#if( SYS_CPNT_SSH2 == TRUE )
#define WRITE_TO_FLASH_FILES_LENGTH     (SERVER_CERTIFICATE_FILE_LENGTH + \
                                        SERVER_PRIVATE_KEY_FILE_LENGTH + \
                                        SERVER_PASS_PHRASE_FILE_LENGTH + \
                                        HOST_RSA_KEY_1024B_FILE_LENGTH + \
                                        HOST_DSA_KEY_1024B_FILE_LENGTH + \
                                        USER_RSA_PUBLIC_KEY_FILE_LENGTH + \
                                        USER_DSA_PUBLIC_KEY_FILE_LENGTH + \
                                        HOST_RSA_PUBLIC_KEY_SHA1_FINGERPRINT_LENGTH + \
                                        HOST_RSA_PUBLIC_KEY_MD5_FINGERPRINT_LENGTH + \
                                        HOST_DSA_PUBLIC_KEY_SHA1_FINGERPRINT_LENGTH + \
                                        HOST_DSA_PUBLIC_KEY_MD5_FINGERPRINT_LENGTH + \
                                        (USER_PUBLIC_KEY_FILE_LENGTH * SYS_ADPT_MAX_NBR_OF_LOGIN_USER))
#define	SSHD_OM_FILES_LENGTH			(WRITE_TO_FLASH_FILES_LENGTH + \
                                        ((TEMP_RSA_KEY_512B_FILE_LENGTH + \
                                        TEMP_RSA_KEY_768B_FILE_LENGTH + \
                                        TEMP_RSA_KEY_1024B_FILE_LENGTH) * 4))
#else
#define WRITE_TO_FLASH_FILES_LENGTH     8192
#define	SSHD_OM_FILES_LENGTH			20480
#endif


/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
/* for trace_id of user_id when allocate buffer with l_mm
 */
enum
{
    KEYGEN_TYPE_TRACE_ID_KEYGEN_MGR_INITCERTIFICATEFILES=0,
    KEYGEN_TYPE_TRACE_ID_KEYGEN_LOADCERTIFICATEFILES,
    KEYGEN_TYPE_TRACE_ID_KEYGEN_MGR_GENERATERSAHOSTKEYPAIR,
    KEYGEN_TYPE_TRACE_ID_KEYGEN_MGR_GENERATEDSAHOSTKEYPAIR
};

/* the MSGQ of KEYGEN TASK is UNIDIRECTIONAL,
 *   so the msg_type can not be 0.
 */
typedef enum KEYGEN_TYPE_EVENT_MASK_E
{
    KEYGEN_TYPE_EVENT_GEN_KEY             =   0x0001L,
    KEYGEN_TYPE_EVENT_DEL_KEY             =   0x0002L,
    KEYGEN_TYPE_EVENT_WRITE_KEY           =   0x0003L,
}KEYGEN_MsgTYPE_T;

/* Consistent with SSHD_SetUserPublicKeyResult_T.
 */
typedef enum
{
    KEYGEN_SET_USER_PUBLIC_KEY_FAIL = 0L,
    KEYGEN_SET_USER_PUBLIC_KEY_SUCC,
    KEYGEN_SET_USER_PUBLIC_KEY_INVALID_KEY,
    KEYGEN_SET_USER_PUBLIC_KEY_INVALID_KEY_TYPE,
    KEYGEN_SET_USER_PUBLIC_KEY_STORAGE_FULL,
    KEYGEN_SET_USER_PUBLIC_KEY_OUT_OF_MEMORY,
    KEYGEN_SET_USER_PUBLIC_KEY_WRITE_ERROR,
} KEYGEN_SetUserPublicKeyResult_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

#define	KEYGEN_EVENT_MSG                 BIT_1

typedef struct
{
    KEYGEN_MsgTYPE_T    msgtype;
    //richKEYGEN_MSG_KEYTYPE_T             key_type;
    void            (*status_callback) (UI32_T status);
    void            (*action_callback) (UI32_T action);
    UI8_T          *username;
    UI32_T              reserved[2];
} Msg_T;

typedef enum /* host key type */
{
    KEY_TYPE_NONE = 1L,
    KEY_TYPE_RSA,
    KEY_TYPE_DSA,
    KEY_TYPE_BOTH_RSA_AND_DSA,
}KEYGEN_MSG_KEYTYPE_T;


#endif /* ifndef KEYGEN_TYPE_H */



