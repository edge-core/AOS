#ifndef TACACS_TYPE_H
#define TACACS_TYPE_H

#include "sys_type.h"
#include "sys_adpt.h"
#include "sys_dflt.h"
#include "leaf_es3626a.h"
#include "l_inet.h"     /* L_INET_MAX_IPADDR_STR_LEN */

#if (SYS_CPNT_TACACS_PLUS_ACCOUNTING == TRUE)
#include "aaa_def.h"
#endif

#define TACACS_DEFAULT_TIMEOUT          SYS_DFLT_TACACS_AUTH_CLIENT_TIMEOUTS
#define TACACS_DEFAULT_SERVER_PORT      SYS_DFLT_TACACS_AUTH_CLIENT_SERVER_PORT_NUMBER
#define TACACS_DEFAULT_SERVER_IP        SYS_DFLT_TACACS_AUTH_SERVER_ADDRESS
#define TACACS_DEFAULT_SERVER_KEY       ""
#define TACACS_NOT_IN_MASTER_MODE       -3
#define TACACS_MAX_RETRANSMIT_TIMES     SYS_ADPT_TACACS_MAX_RETRANSMIT
#define TACACS_MIN_RETRANSMIT_TIMES     SYS_ADPT_TACACS_MIN_RETRANSMIT
#define TACACS_MAX_SERVER_PORT          MAX_tacacsPlusServerPortNumber
#define TACACS_MIN_SERVER_PORT          MIN_tacacsPlusServerPortNumber
#define TACACS_MAX_REQUEST_TIMEOUT      SYS_ADPT_TACACS_MAX_TIMEOUT
#define TACACS_MIN_REQUEST_TIMEOUT      SYS_ADPT_TACACS_MIN_TIMEOUT
#define TACSCA_MAX_SECRET_LENGTH        MAXSIZE_tacacsServerKey
#define TACACS_MAX_NBR_OF_SERVERS       1

/* NAMING CONSTANT DECLARATIONS
 */
#define TACACS_TASK_TIMER_PERIOD        200 /* ticks */
#define TACACS_AUTH_KEY_MAX_LEN         MAXSIZE_tacacsPlusServerKey/*32*/

#define TACACS_NOT_IN_MASTER_MODE_RC    (-3)
#define TACACS_BADRESP_RC               (-2)
#define TACACS_ERROR_RC                 (-1)
#define TACACS_OK_RC                    0
#define TACACS_TIMEOUT_RC               1
#define TACACS_CHALLENGE_RC             2
/* a TACACS+ packet (request & reply) is composed of
 * packet head and packet body
 */
#define TACACS_LIB_MAX_LEN_OF_PACKET_BODY   256

#define TACACS_PRIVILEGE_OF_ADMIN           15
#define TACACS_PRIVILEGE_OF_GUEST           0

/* server_msg is defined in TACACS+ Authentication REPLY packet body */
#define TACACS_LIB_MAX_LEN_OF_SERVER_MSG                    64

/* data_msg is defined in TACACS+ Authentication
 * START & REPLY & CONTINUE packet body
 */
#define TACACS_LIB_MAX_LEN_OF_DATA                          64

/* port field is defined in TACACS+ Authentication START request
 * and Authorization request
 */
#define TACACS_LIB_MAX_LEN_OF_PORT_STRING                   10

/* Attribute-Value Pairs
 * used in TACACS+ Authorization & Accounting
 */
#define TACACS_LIB_MAX_NBR_OF_AV_PAIRS                      10

/* The attribute and the value are separated by either a '=' or a '*'
 * and used in TACACS+ Authorization & Accounting
 */
#define TACACS_LIB_AV_PAIR_OPTIONAL_CHAR                    '*'
#define TACACS_LIB_AV_PAIR_MANDATORY_CHAR                   '='

#define TACACS_TYPE_MAX_LEN_OF_NAS_PORT                      14 /* "Vty-4294967295" */
#define TACACS_TYPE_MAX_LEN_OF_REM_ADDR                      (L_INET_MAX_IPADDR_STR_LEN)


/* DATA TYPE DECLARATIONS
 */

typedef enum
{
    TACACS_SESS_TYPE_UNKNOWN,
    TACACS_SESS_TYPE_CONSOLE,
    TACACS_SESS_TYPE_TELNET,
    TACACS_SESS_TYPE_SSH,
    TACACS_SESS_TYPE_HTTP,
    TACACS_SESS_TYPE_HTTPS,
} TACACS_SessType_T;

#if (SYS_CPNT_TACACS_PLUS_ACCOUNTING == TRUE)
typedef enum TACACS_AccEntryStatus_E
{
    TPACC_ENTRY_DESTROYED,
    TPACC_ENTRY_READY,
} TACACS_AccEntryStatus_T;

typedef enum TACACS_AccObjectProcessResult_E
{
    TPACC_OPROCESS_SUCCESS,
    TPACC_OPROCESS_FAILURE,
    TPACC_OPROCESS_DELETED, /* object had been deleted, should not use thereafter */
} TACACS_AccObjectProcessResult_T;

typedef struct TACACS_AccUserCtrlBitmap_S
{
    UI8_T   start_packet_sent   :1; /* start-packet has been sent or not */
    UI8_T   stop_packet_sent    :1; /* stop-packet has been sent or not */
    UI8_T   start_packet_wait   :1; /* start-packet can't be sent because no available server */
    UI8_T   reserved            :5;
} TACACS_AccUserCtrlBitmap_T;

typedef struct TACACS_AAATacacsEntryInfo_S
{
    UI16_T  aaa_group_index;        /* index of aaa tacacs group */
    UI16_T  aaa_tacacs_index;       /* index of aaa tacacs entry in tacacs group */
    UI32_T  active_server_index;    /* index of server host */
    UI16_T  aaa_tacacs_order;       /* keep the tacacs order in the tacacs group */
} TACACS_AAATacacsEntryInfo_T;

typedef struct TACACS_AccUserInfo_S
{
    UI16_T                          user_index;     /* array index + 1 */

    UI32_T                          ifindex;        /* if client_type == AAA_CLIENT_TYPE_DOT1X, ifindex == l_port
                                                       if client_type == AAA_CLIENT_TYPE_EXEC, ifindex implies console or telnet's session id
                                                     */
    L_INET_AddrIp_T                 rem_ip_addr;
    char                            user_name[SYS_ADPT_MAX_USER_NAME_LEN + 1];

    UI32_T                          accounting_start_time;  /* unit is seconds */
    UI32_T                          session_start_time;     /* use for caculating session-time (in seconds) */
    UI32_T                          last_update_time;       /* unit is seconds */
    TACACS_AAATacacsEntryInfo_T     tacacs_entry_info;
    TACACS_AccUserCtrlBitmap_T      ctrl_bitmap;
    AAA_AccConnectStatus_T          connect_status;

    /* reserved identifier and call_back_func if can't hook a request currently
       and them will be transfer to request if successfully to hook a request
     */
    UI32_T  identifier; /* callback function's parameter
                           so caller can know which request's result back */
    AAA_AccRequest_Callback_T       call_back_func; /* callback to the CSC which placed this request */

    AAA_ClientType_T                client_type;
    TACACS_AccEntryStatus_T         entry_status;

    UI32_T                          serial_number;      /* a serial number, that be take as task_id in TACACS account packet */
    UI32_T task_id;
    UI32_T                          auth_privilege;     /* meaningfully only while request type is START */
    AAA_Authentic_T                 auth_by_whom;       /* meaningfully only while request type is START */

    struct TACACS_AccUserInfo_S    *prev_user;
    struct TACACS_AccUserInfo_S    *next_user;
} TACACS_AccUserInfo_T;

typedef enum TPACC_ClientEnable_E
{
    TPACC_DISABLE,
    TPACC_ENABLE,
} TPACC_ClientEnable_T;

typedef enum TPACC_AcctStatusType_E
{
    TPACC_START            = 1,
    TPACC_STOP             = 2,
    TPACC_InterimUpdate    = 3,
} TPACC_AcctStatusType_T;

#if (SYS_CPNT_TACACS_PLUS_ACCOUNTING_COMMAND == TRUE)
typedef struct TACACS_AccCmdUserInfo_S
{
    UI32_T  serial_number;  /* a serial number, that be take as task_id in TACACS account packet */
    UI32_T  cmd_pri;        /* command privilege level */
    char    cmd[SYS_ADPT_CLI_MAX_BUFSIZE + 1]; /* include no-printable char e.g., <cr> */
    char    user_name[SYS_ADPT_MAX_USER_NAME_LEN + 1];
} TACACS_AccCmdUserInfo_T;

typedef struct TPACC_AccCommandmdMessage_S
{
    UI32_T                      ifindex;
    L_INET_AddrIp_T             rem_ip_addr;            /* caller's IP address */
    UI32_T                      user_auth_privilege;    /* meaningfully only while request type is START */
    UI32_T                      accounting_start_time;  /* unit is seconds */
    UI32_T                      serial_number;          /* a serial number, that be take as task_id in
                                                         * TACACS account packet
                                                         */
    UI32_T                      acct_flag;              /* accounting flag: start, stop */
    UI32_T                      command_privilege;
    AAA_Authentic_T             user_auth_by_whom;      /* meaningfully only while request type is START */
    TACACS_AAATacacsEntryInfo_T tacacs_entry_info;

    char                        user_name[SYS_ADPT_MAX_USER_NAME_LEN + 1];
    char                        command[SYS_ADPT_CLI_MAX_BUFSIZE + 1];
} TPACC_AccCommandmdMessage_T;
#endif

typedef struct TPACC_UserInfoInterface_S
{
    UI16_T  user_index;     /* array index + 1 */
    UI32_T  ifindex;        /* if client_type == AAA_CLIENT_TYPE_DOT1X, ifindex == l_port
                               if client_type == AAA_CLIENT_TYPE_EXEC, ifindex implies console or telnet's session id
                             */
    UI8_T   user_name[SYS_ADPT_MAX_USER_NAME_LEN + 1];
    UI32_T  accounting_start_time;  /* unit is seconds */

    AAA_ClientType_T         client_type;
} TPACC_UserInfoInterface_T;
#endif /* SYS_CPNT_TACACS_PLUS_ACCOUNTING == TRUE */

typedef struct TACACS_Server_Host_S {
    UI32_T   server_ip;
    UI32_T   server_port;
    UI32_T   retransmit;
    UI32_T   timeout;
    UI32_T   server_index; /* array index + 1 */
#if (SYS_CPNT_TACACS_PLUS_ACCOUNTING == TRUE)
    UI32_T   acct_port;
#endif /* SYS_CPNT_TACACS_PLUS_ACCOUNTING == TRUE */
    UI8_T    secret[TACACS_AUTH_KEY_MAX_LEN + 1];
    BOOL_T   used_flag;

} TACACS_Server_Host_T;

/* for trace_id of user_id when allocate buffer with l_mm
 */
enum
{
    TACACS_TYPE_TRACE_ID_TACACS_TASK_MAINROUTINE = 0,
    TACACS_TYPE_TRACE_ID_TACACS_ACCOUNT_SEND_REQUEST,
    TACACS_TYPE_TRACE_ID_TACACS_ACCOUNT_GET_REPLY,
    TACACS_TYPE_TRACE_ID_TACACS_ACCOUNT_C_MAIN,
    TACACS_TYPE_TRACE_ID_TACACS_AUTHOR_SEND_REQUEST,
    TACACS_TYPE_TRACE_ID_TACACS_AUTHOR_GET_RESPONSE,
    TACACS_TYPE_TRACE_ID_TACACS_ACCOUNT_C_ACCTCMDMAIN_D,
    TACACS_TYPE_TRACE_ID_TACACS_ACCOUNT_C_ACCTCMDMAIN_M,
    TACACS_TYPE_TRACE_ID_TACACS_ASYNCACCOUNTINGREQUEST_CB,
};

#endif

