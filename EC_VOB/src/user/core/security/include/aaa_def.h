/* MODULE NAME: aaa_def.h
 * PURPOSE:
 *  Implement AAA
 *
 * NOTES:
 *
 * History:
 *    2004/03/24 : mfhorng      Create this file
 *
 * Copyright(C)      Accton Corporation, 2004
 */
#ifndef AAA_DEF_H
#define AAA_DEF_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_cpnt.h"

#if (SYS_CPNT_AAA == TRUE)

#include "sys_type.h"
#include "sys_adpt.h"
#include "sys_dflt.h"
#include "l_inet.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define AAA_SUPPORT_ACCTON_AAA_MIB      (TRUE && SYS_CPNT_AAA) /* support accton aaa mib or not */


/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

typedef enum AAA_ServerGroupType_E
{
    GROUP_RADIUS,
    GROUP_TACACS_PLUS,
    GROUP_UNKNOWN,          /* a method may map to a non-existent group */
} AAA_ServerGroupType_T;

typedef enum AAA_ConfigureMode_E
{
    AAA_AUTO_CONFIGURE, /* configured by aaa accounting default */
    AAA_MANUAL_CONFIGURE, /* configured by accounting dot1x */
} AAA_ConfigureMode_T;

typedef enum AAA_WarningType_E
{
    AAA_NO_WARNING,

    AAA_LIST_REF2_BAD_GROUP,            /* method list reference a non-existent group */
    AAA_GROUP_REF2_BAD_SERVER,          /* server group reference a non-existent server host */
    AAA_GROUP_HAS_NO_ENTRY,             /* server group doesn't have any entry */
    AAA_ACC_DOT1X_REF2_BAD_LIST,        /* accounting dot1x reference a non-existent method-list */
    AAA_ACC_EXEC_REF2_BAD_LIST,         /* accounting exec reference a non-existent method-list */

#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)
    AAA_ACC_COMMAND_REF2_BAD_LIST,      /* accounting command reference a non-existent method-list */
#endif

    AAA_AUTHOR_EXEC_REF2_BAD_LIST,      /* authorization exec reference a non-existent method-list */
} AAA_WarningType_T;

typedef struct AAA_WarningInfo_S
{
    AAA_WarningType_T   warning_type;
} AAA_WarningInfo_T;

typedef struct AAA_RadiusGroupEntryInterface_S
{
    UI16_T  group_index; /* array index + 1 */
    char    group_name[SYS_ADPT_MAX_LEN_OF_AAA_SERVER_GROUP_NAME + 1];
} AAA_RadiusGroupEntryInterface_T;

typedef struct AAA_TacacsPlusGroupEntryInterface_S
{
    UI16_T  group_index; /* array index + 1 */
    char   group_name[SYS_ADPT_MAX_LEN_OF_AAA_SERVER_GROUP_NAME + 1];
} AAA_TacacsPlusGroupEntryInterface_T;

typedef struct AAA_RadiusEntryInterface_S
{
    UI16_T  radius_index; /* array index + 1 */
    UI32_T  radius_server_index; /* mapping to radius_om */
} AAA_RadiusEntryInterface_T;

typedef struct AAA_TacacsPlusEntryInterface_S
{
    UI16_T  tacacs_index; /* array index + 1 */
    UI32_T  tacacs_server_index; /* mapping to tacacs_om */
} AAA_TacacsPlusEntryInterface_T;

typedef enum AAA_Authentic_E
{
    AAA_AUTHEN_BY_UNKNOWN,
    AAA_AUTHEN_BY_RADIUS,
    AAA_AUTHEN_BY_TACACS_PLUS,
    AAA_AUTHEN_BY_LOCAL,
} AAA_Authentic_T;

typedef enum AAA_ClientType_E
{
    AAA_CLIENT_TYPE_MIN         = VAL_aaaMethodClientType_dot1x,
    AAA_CLIENT_TYPE_DOT1X  = VAL_aaaMethodClientType_dot1x,
    AAA_CLIENT_TYPE_EXEC   = VAL_aaaMethodClientType_exec,

#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE || SYS_CPNT_AUTHORIZATION_COMMAND == TRUE)
    AAA_CLIENT_TYPE_COMMANDS    = VAL_aaaMethodClientType_commands,
    AAA_CLIENT_TYPE_NUMBER      = 3,
#else
    AAA_CLIENT_TYPE_NUMBER = 2,
#endif

} AAA_ClientType_T;

typedef enum AAA_ExecType_E
{
    AAA_EXEC_TYPE_NONE = 0,  /* for get next */

    AAA_EXEC_TYPE_CONSOLE,   /* CLI: line console */
    AAA_EXEC_TYPE_VTY,       /* CLI: line vty */
    AAA_EXEC_TYPE_HTTP,      /* HTTP */

    AAA_EXEC_TYPE_SUPPORT_NUMBER = AAA_EXEC_TYPE_HTTP,
} AAA_ExecType_T;

typedef enum AAA_EntryStatus_E
{
    AAA_ENTRY_DESTROYED,
    AAA_ENTRY_READY,
} AAA_EntryStatus_T;

typedef struct AAA_QueryGroupIndexResult_S
{
    UI16_T  group_index; /* array index + 1 */
    AAA_ServerGroupType_T   group_type;
} AAA_QueryGroupIndexResult_T;

typedef struct AAA_AccInterface_S
{
    AAA_ClientType_T    client_type;
    UI32_T              ifindex;        /* if client_type == AAA_CLIENT_TYPE_DOT1X,
                                         *     ifindex == l_port
                                         *
                                         * if client_type == AAA_CLIENT_TYPE_EXEC,
                                         *     ifindex implies console or telnet's session id
                                         *
                                         * if client_type == AAA_CLIENT_TYPE_COMMANDS,
                                         *    ifindex implies console or telnet's session id
                                         */

#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE || SYS_CPNT_AUTHORIZATION_COMMAND == TRUE)
    UI32_T              priv_lvl;
#endif /*#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)*/
} AAA_AccInterface_T, AAA_ListType_T;


#if (SYS_CPNT_ACCOUNTING == TRUE)

typedef enum AAA_AccRequestType_E
{
    AAA_ACC_START,
    AAA_ACC_STOP,

#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)
    AAA_ACC_CMD_START,
    AAA_ACC_CMD_STOP,
#endif
} AAA_AccRequestType_T;

typedef enum AAA_AccAuthentic_E
{
    AAA_ACC_AUTH_BY_UNKNOWN,
    AAA_ACC_AUTH_BY_RADIUS,
    AAA_ACC_AUTH_BY_TACACS_PLUS,
    AAA_ACC_AUTH_BY_LOCAL,
} AAA_AccAuthentic_T;

typedef enum AAA_AccTerminateCause_E
{
    AAA_ACC_TERM_BY_UNKNOWN,

    /* following constants are compliant with RFC2866 */
    AAA_ACC_TERM_BY_USER_REQUEST            = 1,
    AAA_ACC_TERM_BY_LOST_CARRIER,
    AAA_ACC_TERM_BY_LOST_SERVICE,
    AAA_ACC_TERM_BY_ACCT_IDLE_TIMEOUT,
    AAA_ACC_TERM_BY_ACCT_SESSION_TIMEOUT,
    AAA_ACC_TERM_BY_ADMIN_RESET,
    AAA_ACC_TERM_BY_ADMIN_REBOOT,
    AAA_ACC_TERM_BY_PORT_ERROR,
    AAA_ACC_TERM_BY_NAS_ERROR,
    AAA_ACC_TERM_BY_NAS_REQUEST,
    AAA_ACC_TERM_BY_NAS_REBOOT,
    AAA_ACC_TERM_BY_PORT_UNNEEDED,
    AAA_ACC_TERM_BY_PORT_PREEMPTED,
    AAA_ACC_TERM_BY_PORT_SUSPENDED,
    AAA_ACC_TERM_BY_SERVICE_UNAVAILABLE,
    AAA_ACC_TERM_BY_CALLBACK,
    AAA_ACC_TERM_BY_USER_ERROR,
    AAA_ACC_TERM_BY_HOST_REQUEST,
} AAA_AccTerminateCause_T;

typedef enum AAA_AccStatus_E
{
    ACCOUNTING_ENABLED,
    ACCOUNTING_DISABLED,
} AAA_AccStatus_T;

typedef enum AAA_AccWorkingMode_E
{
/*    ACCOUNTING_NONE,*/
    ACCOUNTING_START_STOP   = VAL_aaaAccountMethodMode_start_stop,
/*    ACCOUNTING_STOP,*/
} AAA_AccWorkingMode_T;

typedef enum AAA_AccCallback_ResultType_E
{
    AAA_ACC_CALLBACK_SUCCEEDED,
    AAA_ACC_CALLBACK_FAILED,
    AAA_ACC_CALLBACK_TIMEOUT,
    AAA_ACC_CALLBACK_POSTPONED,  /* setting doesn't complete */
} AAA_AccCallback_ResultType_T;

typedef enum AAA_AccExecType_E
{
    AAA_ACC_EXEC_NONE = 0,  /* for get next */

    AAA_ACC_EXEC_CONSOLE,   /* CLI: line console */
    AAA_ACC_EXEC_VTY,       /* CLI: line vty */

    AAA_ACC_EXEC_SUPPORT_NUMBER = AAA_ACC_EXEC_VTY, /* to calculate the number of accounting exec type automatically */
} AAA_AccExecType_T;

typedef enum AAA_AccConnectStatus_E
{
    AAA_ACC_CNET_DORMANT = 0,   /* imcomplete accounting configuration */
    AAA_ACC_CNET_IDLE,          /* doesn't send request yet */
    AAA_ACC_CNET_CONNECTING,    /* waiting start request's response */
    AAA_ACC_CNET_CONNECTED,     /* recevied response */
    AAA_ACC_CNET_TIMEOUT,       /* waiting response but timeout */
    AAA_ACC_CNET_FAILED,        /* radius/tacacs+ can't execute accounting */
} AAA_AccConnectStatus_T;

typedef struct AAA_AccDot1xEntry_S
{
    UI32_T  ifindex; /* l_port (array index + 1) */
    char    list_name[SYS_ADPT_MAX_LEN_OF_ACCOUNTING_LIST_NAME + 1];

    AAA_ConfigureMode_T configure_mode;
} AAA_AccDot1xEntry_T;

typedef struct AAA_AccExecEntry_S
{
    AAA_ExecType_T      exec_type;

    char    list_name[SYS_ADPT_MAX_LEN_OF_ACCOUNTING_LIST_NAME + 1];

    AAA_ConfigureMode_T configure_mode;
} AAA_AccExecEntry_T;

#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)
typedef struct AAA_AccCommandEntry_S
{
    UI32_T              priv_lvl;   /*Key*/
    AAA_ExecType_T      exec_type;  /*Key*/

    AAA_ConfigureMode_T configure_mode;
    char                list_name[SYS_ADPT_MAX_LEN_OF_ACCOUNTING_LIST_NAME + 1];
} AAA_AccCommandEntry_T;
#endif /* #if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE) */

#if (SYS_CPNT_AUTHORIZATION_COMMAND == TRUE)
typedef struct AAA_AuthorCommandEntry_S
{
    UI32_T              priv_lvl;   /*Key*/
    AAA_ExecType_T      exec_type;  /*Key*/

    AAA_ConfigureMode_T configure_mode;
    char                list_name[SYS_ADPT_MAX_LEN_OF_AUTHORIZATION_LIST_NAME + 1];

} AAA_AuthorCommandEntry_T;
#endif /* #if (SYS_CPNT_AUTHORIZATION_COMMAND == TRUE) */

typedef struct AAA_AccCallback_Result_S
{
    UI32_T  identifier; /* callback function's parameter
                           so caller can know which request's result back */

    AAA_AccCallback_ResultType_T    result_type;
} AAA_AccCallback_Result_T;

typedef void (*AAA_AccRequest_Callback_T)(AAA_AccCallback_Result_T *result); /* callback to the CSC which placed this request */

typedef struct AAA_AccRequest_S
{
    UI32_T                      ifindex;        /* if client_type == AAA_CLIENT_TYPE_DOT1X, ifindex == l_port
                                                 * if client_type == AAA_CLIENT_TYPE_EXEC, ifindex == AAA_ExecType_T
                                                 */

    char                        user_name[SYS_ADPT_MAX_USER_NAME_LEN + 1];

    AAA_ClientType_T            client_type;    /* caller's type */
    AAA_AccRequestType_T        request_type;

    UI32_T                      auth_privilege;     /* meaningfully only while request type is START */
    AAA_Authentic_T             auth_by_whom;       /* meaningfully only while request type is START */
    AAA_AccTerminateCause_T     terminate_cause;    /* meaningfully only while request type is STOP */

    AAA_AccWorkingMode_T        current_working_mode;

#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)
    UI32_T                      command_privilege;  /* command privilege level */
    char                        command[SYS_ADPT_CLI_MAX_BUFSIZE + 1];  /* include no-printable char e.g., <cr> */
#endif

    UI32_T                      identifier;     /* callback function's parameter
                                                 * so caller can know which request's result back */

    UI32_T                      serial_number;  /* this value will be fill by aaa */

    L_INET_AddrIp_T             rem_ip_addr;                     /* the caller's IP address for telnet/SSH request */
    UI8_T                       auth_mac[SYS_ADPT_MAC_ADDR_LEN]; /* the caller's MAC address for dot1x request */

    AAA_AccRequest_Callback_T   call_back_func; /* callback to the CSC which placed this request */
} AAA_AccRequest_T;

typedef struct AAA_AccListEntryInterface_S
{
    UI16_T                  list_index; /* array index + 1 */

    char                    list_name[SYS_ADPT_MAX_LEN_OF_ACCOUNTING_LIST_NAME + 1];
    char                    group_name[SYS_ADPT_MAX_LEN_OF_AAA_SERVER_GROUP_NAME + 1];

    AAA_ClientType_T        client_type;    /* distinguish between DOT1X and EXEC method-list */

#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)
    UI32_T                  priv_lvl;
#endif /*#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)*/

    AAA_ServerGroupType_T   group_type;
    AAA_AccWorkingMode_T    working_mode;
} AAA_AccListEntryInterface_T;

#if 0
typedef struct AAA_QueryGroupIndexResult_S
{
    UI16_T  group_index; /* array index + 1 */
    AAA_ServerGroupType_T   group_type;
} AAA_QueryGroupIndexResult_T;
#endif
typedef struct AAA_QueryAccDot1xPortListResult_S
{
    UI16_T  list_index;
    UI8_T   port_list[SYS_ADPT_TOTAL_NBR_OF_LPORT];
} AAA_QueryAccDot1xPortListResult_T;

typedef struct AAA_AccUserRunningInfo_S
{
    UI32_T  session_start_time;     /* in seconds*/
    UI32_T  in_service_ip;          /* the server ip in service */

    AAA_AccConnectStatus_T  connect_status;
} AAA_AccUserRunningInfo_T;

typedef struct AAA_AccUserInfoInterface_S
{
    UI32_T  ifindex;        /* if client_type == AAA_CLIENT_TYPE_DOT1X, ifindex == l_port
                               if client_type == AAA_CLIENT_TYPE_EXEC, ifindex implies console or telnet's session id
                             */

    UI32_T  accounting_start_time;  /* unit is seconds,
                                       the time that components demand to start accounting
                                     */

    UI32_T                      auth_privilege;
    AAA_Authentic_T             auth_by_whom;

    AAA_ClientType_T            client_type;
    AAA_ServerGroupType_T       group_type;     /* keep accounting via tacacs+ or radius */
    AAA_AccUserRunningInfo_T    running_info;

    UI16_T  user_index;     /* array index + 1 */
    char    user_name[SYS_ADPT_MAX_USER_NAME_LEN + 1];
} AAA_AccUserInfoInterface_T;


#endif /* SYS_CPNT_ACCOUNTING == TRUE */



#if (AAA_SUPPORT_ACCTON_AAA_MIB == TRUE)

typedef struct AAA_RadiusGroupEntryMIBInterface_S
{
    UI16_T  group_index; /* array index + 1 */
    char    group_name[SYS_ADPT_MAX_LEN_OF_AAA_SERVER_GROUP_NAME + 1];
    UI8_T   radius_server_list; /* server bitmap, 1010 0000 implies server 1 & 3 */
} AAA_RadiusGroupEntryMIBInterface_T;

typedef struct AAA_TacacsPlusGroupEntryMIBInterface_S
{
    UI16_T  group_index; /* array index + 1 */
    UI8_T   group_name[SYS_ADPT_MAX_LEN_OF_AAA_SERVER_GROUP_NAME + 1];
    UI8_T   tacacsplus_server_list; /* server bitmap, 1010 0000 implies server 1 & 3 */
} AAA_TacacsPlusGroupEntryMIBInterface_T;

#endif /* AAA_SUPPORT_ACCTON_AAA_MIB == TRUE */

#if (SYS_CPNT_AUTHORIZATION == TRUE)
typedef struct AAA_AuthorRequest_S
{
    UI32_T              ifindex;        /* if client_type == AAA_AUTHOR_DOT1X, ifindex == l_port
                                         * if client_type == AAA_AUTHOR_EXEC, ifindex implies console or telnet's session id
                                         */
    L_INET_AddrIp_T     rem_ip_addr;    /* remote IP address */
    char                user_name[SYS_ADPT_MAX_USER_NAME_LEN + 1];
    char                password[SYS_ADPT_MAX_PASSWORD_LEN + 1];

    AAA_ClientType_T    client_type;        /* caller's type */

    UI32_T              current_privilege;
    AAA_Authentic_T     authen_by_whom;     /* meaningfully only while request type is START */

    UI32_T              command_privilege;  /* command privilege level */

#if (SYS_CPNT_AUTHORIZATION_COMMAND == TRUE)
    char                command[SYS_ADPT_CLI_MAX_BUFSIZE + 1];  /* include no-printable char e.g., <cr> */
#endif

} AAA_AuthorRequest_T;

typedef enum AAA_AuthorRequestReturnType_E
{
    AAA_AuthorRequest_START, /* initial value when request */
    AAA_AuthorRequest_SUCCEEDED,
    AAA_AuthorRequest_FAILED,
    AAA_AuthorRequest_TIMEOUT,
    AAA_AuthorRequest_CONFIG_IMCOMPLETE,
    AAA_AuthorRequest_SUCCEEDED_WITH_NO_PRIV,
} AAA_AuthorRequestReturnType_T;

typedef struct AAA_AuthorReply_S
{
    UI32_T                         new_privilege;
    AAA_AuthorRequestReturnType_T  return_type;
} AAA_AuthorReply_T;

typedef struct AAA_AuthorListEntryInterface_S
{
    UI16_T  list_index; /* array index + 1 */

    char    list_name[SYS_ADPT_MAX_LEN_OF_AUTHORIZATION_LIST_NAME + 1]; /* key */
    char    group_name[SYS_ADPT_MAX_LEN_OF_AAA_SERVER_GROUP_NAME + 1];

    AAA_ListType_T          list_type; /* key */

    AAA_ServerGroupType_T   group_type;
} AAA_AuthorListEntryInterface_T;

typedef struct AAA_AuthorListEntry_S
{
    UI16_T  list_index; /* array index + 1 */

    char    list_name[SYS_ADPT_MAX_LEN_OF_AUTHORIZATION_LIST_NAME + 1]; /* key */
    char    group_name[SYS_ADPT_MAX_LEN_OF_AAA_SERVER_GROUP_NAME + 1];

    AAA_ListType_T          list_type; /* key */

    AAA_ServerGroupType_T   group_type;
    AAA_EntryStatus_T       entry_status;
} AAA_AuthorListEntry_T;

typedef struct AAA_AuthorExecEntry_S
{
    AAA_ExecType_T   exec_type;

    char    list_name[SYS_ADPT_MAX_LEN_OF_ACCOUNTING_LIST_NAME + 1];

    AAA_ConfigureMode_T configure_mode; /* Auto or Manual */
} AAA_AuthorExecEntry_T;
#endif /* SYS_CPNT_AUTHORIZATION == TRUE */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

#endif /* SYS_CPNT_AAA == TRUE */

#endif /* End of AAA_DEF_H */

