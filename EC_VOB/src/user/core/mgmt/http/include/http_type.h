/* MODULE NAME: http_type.h
 * PURPOSE:
 *   {1. What is covered in this file - function and scope.}
 *   {2. Related documents or hardware information}
 * NOTES:
 *     {Something must be known or noticed}
 *   {1. How to use these functions - Give an example.}
 *   {2. Sequence of messages if applicable.}
 *   {3. Any design limitation}
 *   {4. Any performance limitation}
 *   {5. Is it a reusable component}
 *
 * CREATOR:  Isiah           Date 2002-04
 *
 * Copyright(C)      Accton Corporation, 2002
 */



#ifndef HTTP_TYPE_H

#define HTTP_TYPE_H




/* INCLUDE FILE DECLARATIONS
 */
#include <stddef.h>
#include <time.h>

#include "sys_type.h"
#include "sys_module.h"

#include "sys_cpnt.h"
#include "sys_adpt.h"
#include "sys_dflt.h"
#include "l_inet.h"







/* NAMING CONSTANT DECLARATIONS
 */

#define HTTP_DEFAULT_PORT_NUMBER    SYS_DFLT_IP_HTTP_PORT
#define HTTP_DEFAULT_STATE          SYS_DFLT_IP_HTTP_STATE

#if (SYS_CPNT_HTTPS == TRUE)
#define HTTP_DEFAULT_SESSION_CACHE_TIMEOUT      SYS_DFLT_SSL_SESSION_CACHE_TIMEOUT
#define HTTP_DEFAULT_SECURE_PORT_NUMBER			SYS_DFLT_IP_HTTP_SECURE_PORT
#define HTTP_DEFAULT_SECURE_HTTP_STATE			SYS_DFLT_IP_SECURE_HTTP_STATE
#define HTTP_DEFAULT_REDIRECT_HTTP_STATE        SYS_DFLT_IP_REDIRECT_HTTP_STATE

#define NUMBER_OF_TEMP_KEY              4

#define HTTP_DEFAULT_SUBJECT_NAME_LEN   64
#define HTTP_DEFAULT_ISSUER_NAME_LEN    64
#define HTTP_DEFAULT_CERTIFICATE_VALID_TIME_STRING_LEN  16
#define HTTP_DEFAULT_SHA1_FINGERPRINT   64
#define HTTP_DEFAULT_MD5_FINGERPRINT    64
#define HTTP_TYPE_KEY_PASSWD_MAX_LEN    21

#endif /* if (SYS_CPNT_HTTPS == TRUE) */







/* MACRO FUNCTION DECLARATIONS
 */








/* DATA TYPE DECLARATIONS
 */
/* for trace_id of user_id when allocate buffer with l_mm
 */
enum
{
    HTTP_TYPE_TRACE_ID_SET_ENV = 0,
    HTTP_TYPE_TRACE_ID_HTTP_MGR_GET_CERTIFICATE,
    HTTP_TYPE_TRACE_ID_HTTP_PROCESS_REQUEST,
    HTTP_TYPE_TRACE_ID_CLEANUP_URI,
    HTTP_TYPE_TRACE_ID_SEND_ERROR_REPLY,
    HTTP_TYPE_TRACE_ID_HTTP_MGR_GET_CERTIFICATE_INFO,
    HTTP_TYPE_TRACE_ID_HTTP_MGR_ASYNC_GET_CERTIFICATE,
    HTTP_TYPE_TRACE_ID_HTTP_FINGERPRINT_HEX,
    HTTP_TYPE_TRACE_ID_PARSE_REQUEST,
    HTTP_TYPE_TRACE_ID_READ_HEADER,
    HTTP_TYPE_TRACE_ID_HTTP_GET_HOST_IP_FROM_REQUEST,
    HTTP_TYPE_TRACE_ID_SET_ENV_TOKEN,
    HTTP_TYPE_TRACE_ID_SET_ENV_PTR
};

typedef enum HTTP_STATE_E
{
    HTTP_STATE_ENABLED = VAL_ipHttpState_enabled,
    HTTP_STATE_DISABLED = VAL_ipHttpState_disabled
} HTTP_STATE_T;




#if (SYS_CPNT_HTTPS == TRUE)
typedef enum SECURE_HTTP_STATE_E
{
    SECURE_HTTP_STATE_ENABLED = 1L,//VAL_ipHttpsState_enabled,
    SECURE_HTTP_STATE_DISABLED = 2L//VAL_ipHttpsState_disabled
} SECURE_HTTP_STATE_T;

typedef enum HTTPS_DEBUG_SESSION_STATE_E
{
    HTTPS_DEBUG_SESSION_STATE_ENABLED = 1L,//VAL_ipHttpsDebugSession_enabled,
    HTTPS_DEBUG_SESSION_STATE_DISABLED = 2L//VAL_ipHttpsDebugSession_disabled
} HTTPS_DEBUG_SESSION_STATE_T;

typedef enum HTTPS_DEBUG_STATE_STATE_E
{
    HTTPS_DEBUG_STATE_STATE_ENABLED = 1L,//VAL_ipHttpsDebugState_enabled,
    HTTPS_DEBUG_STATE_STATE_DISABLED = 2L//VAL_ipHttpsDebugState_disabled
} HTTPS_DEBUG_STATE_STATE_T;
#endif


enum /* function number */
{
    HTTP_TASK_Enter_Main_Routine_FUNC_NO = 0,
    HTTP_Init_Socket_FUNC_NO,
    HTTP_process_request_FUNC_NO,
    HTTP_Handshake_FUNC_NO,
#if (SYS_CPNT_HTTPS == TRUE)
    HTTP_MGR_Get_Server_Certificate_to_Buffer_FUNC_NO,
    HTTP_MGR_Get_Server_Private_Key_to_Buffer_FUNC_NO,
    HTTP_MGR_Set_Server_Private_Key_to_Flash_FUNC_NO,
    HTTP_MGR_Get_Certificate_FUNC_NO,
    HTTP_MGR_Set_Secure_Port_FUNC_NO,
#endif
    HTTP_MGR_Set_Http_Port_FUNC_NO,
    HTTP_TASK_Create_Task_FUNC_NO,
};

#if (SYS_CPNT_HTTPS == TRUE)
enum /* error code */
{
    EVP_PKEY_new_ErrNo          = 0,
    EVP_PKEY_assign_RSA_ErrNo,
    BIO_write_filename_ErrNo,
};

typedef enum HTTP_HandshakeStatus_E
{
    HTTP_HANDSKAHE_SUCCESS      = 1L,
    HTTP_HANDSKAHE_FAILURE      = 2L,
    HTTP_HANDSKAHE_NO_PACKET    = 3L,
    HTTP_HANDSKAHE_PORT_FAILURE = 4L,
    HTTP_HANDSKAHE_NO_SECOND_HELLO      = 5L,
    HTTP_HANDSKAHE_BROKEN       = 6L
} HTTP_HandshakeStatus_T;

typedef struct HTTP_CertificateInfo_S
{
	UI8_T                       subject[HTTP_DEFAULT_SUBJECT_NAME_LEN+1];
	UI8_T                       issuer[HTTP_DEFAULT_ISSUER_NAME_LEN+1];
	UI8_T                       valid_begin[HTTP_DEFAULT_CERTIFICATE_VALID_TIME_STRING_LEN];
	UI8_T                       valid_end[HTTP_DEFAULT_CERTIFICATE_VALID_TIME_STRING_LEN];
	UI8_T                       sha1_fingerprint[HTTP_DEFAULT_SHA1_FINGERPRINT];
	UI8_T                       md5_fingerprint[HTTP_DEFAULT_MD5_FINGERPRINT];
}HTTP_CertificateInfo_T;

typedef enum HTTP_Redirect_Status_E
{
    HTTP_REDIRECT_STATE_ENABLED = 1L,
    HTTP_REDIRECT_STATE_DISABLED= 2L
} HTTP_Redirect_Status_T;

typedef enum HTTP_GetCertificateStatus_E
{
    HTTP_GET_CERT_NOTHING       = 0L,
    HTTP_GET_CERT_DOING         = 1L,
    HTTP_GET_CERT_ERROR         = 2L,
    HTTP_GET_CERT_BUSY          = 3L,
    HTTP_GET_CERT_DONE          = 4L,
    HTTP_GET_CERT_NOMEMORY      = 5L,
    HTTP_GET_CERT_FILE_ERROR    = 6L,
    HTTP_GET_CERT_FILE_PHRAS_ERROR      = 7L,
    HTTP_GET_CERT_CERT_PRIVATE_NOMATCH  = 8L
} HTTP_GetCertificateStatus_T;

typedef struct HTTP_DownloadCertificateEntry_S
{
    L_INET_AddrIp_T             tftp_server;
    char                        tftp_cert_file[MAXSIZE_tftpSrcFile+1];
    char                        tftp_private_file[MAXSIZE_tftpSrcFile+1];
    char                        tftp_private_password[HTTP_TYPE_KEY_PASSWD_MAX_LEN];
    void                        *cookie;
    BOOL_T                      setting;
    HTTP_GetCertificateStatus_T status;
}HTTP_DownloadCertificateEntry_T;

#endif /* if (SYS_CPNT_HTTPS == TRUE) */


/*isiah.2003-06-25. move out from #if(SYS_CPNT_HTTPS == TRUE) for add show web connection.*/
typedef enum CONNECTION_STATE_E
{
    HTTP_CONNECTION             = 1L,
    HTTPS_CONNECTION            = 2L,
    UNKNOW_CONNECTION           = 3L
} CONNECTION_STATE_T;

#define HTTP_MAX_SESSION_ID_STR_LEN                 (16*2)
/*isiah.2003-06-12. add for show web connection.*/
typedef struct HTTP_Session_S
{
	L_INET_AddrIp_T             remote_ip;
	L_INET_AddrIp_T             local_ip;
	char                        username[SYS_ADPT_MAX_USER_NAME_LEN + 1];
	char                        session_id[HTTP_MAX_SESSION_ID_STR_LEN + 1];
	int                         auth_level;
	UI32_T                      last_access_time;
	UI32_T                      protocol;
	BOOL_T                      is_send_log;
} HTTP_Session_T;

// TODO: Remove this enum
typedef enum
{
    HTTP_TYPE_RESP_FAIL         = -1,
    HTTP_TYPE_RESP_NOT_FOUND    = 0,
    HTTP_TYPE_RESP_DONE         = 1,
    HTTP_TYPE_RESP_PENDING      = 2,
    HTTP_TYPE_RESP_BAD_REQ      = 3,
    HTTP_TYPE_RESP_NO_RESOURCE  = 4,
    HTTP_TYPE_RESP_DISCONNECT   = 99,
    HTTP_TYPE_RESP_X            = 200
} HTTP_TYPE_RESP_STATUS_T;

#ifndef HTTP_LOC

#ifndef _TYPEDEF_HTTP_REQUEST_T
typedef struct HTTP_Request_ST  HTTP_Request_T;
# endif

#ifndef _TYPEDEF_HTTP_RESPONSE_T
typedef struct HTTP_Response_ST HTTP_Response_T;
#endif

#ifndef _TYPEDEF_HTTP_EVENT_T
typedef struct HTTP_Event_ST    HTTP_Event_T;
#endif

#endif /* HTTP_LOC */

typedef enum
{
    HTTP_NIL                    = 0, /* NULL */
    HTTP_INST_TYPE_FIRST        = 1,
    HTTP_INST_CONNECTION        = HTTP_INST_TYPE_FIRST,
    HTTP_INST_TYPE_LAST         = HTTP_INST_CONNECTION,
} HTTP_INSTANCE_TYPE_T;

/*
 * Typedef: struct RULE_TYPE_SHM_POINTER_T
 *
 * Description: pointer of shared memory.
 *
 * Fields:
 *  type                        - Type.
 *  offset                      - Offset from based address.
 *
 */

typedef struct
{
    HTTP_INSTANCE_TYPE_T        type;
    UI32_T                      offset;
} HTTP_SHM_POINTER_T;

struct HTTP_INSTANCE_ST;

typedef union
{
    HTTP_SHM_POINTER_T          ptr;
    struct HTTP_INSTANCE_ST    *in;
} HTTP_POINTER_T;

/*
 * Typedef: struct RULE_TYPE_INSTANCE_LINKS_T
 *
 * Description: Link conllection of double linked list.
 *
 * Fields:
 *  prev                        - Points to the previous sibling node.
 *  next                        - Points to the next sibling node.
 *  first_node                  - Points to first child node.
 *  last_node                   - Points to last child node.
 *
 */

typedef struct
{
    HTTP_POINTER_T              prev;
    HTTP_POINTER_T              next;
    HTTP_POINTER_T              first_node;
    HTTP_POINTER_T              last_node;
    HTTP_POINTER_T              parent;
} HTTP_INSTANCE_LINKS_T;

/*
 * Typedef: struct RULE_TYPE_INSTANCE_T
 *
 * Description: Class / rule instance's header (base class). Each instance was
 *              a node of double linked list. It also contain a double linked
 *              list.
 *
 * Fields:
 *  links                       - Points to the previous, next node
 *                                and new list.
 *  type                        - Type of the instance.
 *  data                        - DON'T access this value. Cases
 *                                the instance to derived data
 *                                structure based on type.
 *
 */

typedef struct HTTP_INSTANCE_ST
{
    HTTP_INSTANCE_LINKS_T       links;
    HTTP_INSTANCE_TYPE_T        type;
} HTTP_INSTANCE_T, *HTTP_INSTANCE_PTR_T;

typedef struct HTTP_CONNECTION_STAT_ST
{
    size_t                      total_request;  /* total = new connecton +
                                                 * reused connection
                                                 */
    size_t                      total_reused;

    struct timeval              start_time;
    struct timeval              total_process;
} HTTP_CONNECTION_STAT_T;

enum
{
    HTTP_CONN_FD_NET            = 0,
    HTTP_CONN_FD_SYSCB_RCV      = 1,
    HTTP_CONN_FD_SYSCB_SND      = 2,
    HTTP_CONN_TOTAL_FDS         = 3
};

typedef struct
{
    HTTP_INSTANCE_T             super;

    struct HTTP_Worker_ST      *worker;
    UI32_T                      tid;
    int                         fds[HTTP_CONN_TOTAL_FDS];

    struct HTTP_Request_ST     *req;
    struct HTTP_Response_ST    *res;

    CONNECTION_STATE_T          conn_state;

    unsigned                    done:1;         /* 1 means finished the request */
    unsigned                    keepalive:1;    /* 1 means can reuse the connection */
    unsigned                    wait_syscb:1;   /* 1 means wait for system-callback */

    // FIXME: remove this flag
    unsigned                    special_workaround:1;

#if (SYS_CPNT_CLUSTER == TRUE)
    /* http cluster relay socket id
     */
    int                         socket_relay_id;

    /* http cluster:keep the relaying status for each http connection session
     */
    BOOL_T                      is_relaying;
#endif

    HTTP_CONNECTION_STAT_T      stat;
} HTTP_Connection_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */










#endif /* ifndef HTTP_TYPE_H */



