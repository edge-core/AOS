#ifndef  RADIUS_OM_H
#define  RADIUS_OM_H

/* Project Name: New Feature
 * File_Name : Radius_om.h
 * Purpose     : Radius initiation and Radius task creation
 *
 * 2001/11/22    : Kevin Cheng  Create this file
 *
 * Copyright(C)      Accton Corporation, 2001, 2002
 *
 * Note    : Designed for new platform (Mercury)
 */
#include <stddef.h>
#include "sys_dflt.h"
#include "sys_adpt.h"
#include "sys_bld.h"
#include "sysfun.h"
//#include "radiusclient.h"
#include <string.h>
#include "radius_type.h"
#include "leaf_2618.h"
#include "security_backdoor.h"
#include "1x_types.h"

#define RADIUS_OM_DEBUG_PRINT0(op, msg)                               SECURITY_DEBUG_PRINT0(RADIUS_OM_GetDebugFlag(), op, RADIUS_OM_GetDebugPrompt(op), msg)
#define RADIUS_OM_DEBUG_PRINT1(op, msg, arg)                          SECURITY_DEBUG_PRINT1(RADIUS_OM_GetDebugFlag(), op, RADIUS_OM_GetDebugPrompt(op), msg, arg)
#define RADIUS_OM_DEBUG_PRINT2(op, msg, arg1, arg2)                   SECURITY_DEBUG_PRINT2(RADIUS_OM_GetDebugFlag(), op, RADIUS_OM_GetDebugPrompt(op), msg, arg1, arg2)
#define RADIUS_OM_DEBUG_PRINT3(op, msg, arg1, arg2, arg3)             SECURITY_DEBUG_PRINT3(RADIUS_OM_GetDebugFlag(), op, RADIUS_OM_GetDebugPrompt(op), msg, arg1, arg2, arg3)
#define RADIUS_OM_DEBUG_PRINT4(op, msg, arg1, arg2, arg3, arg4)       SECURITY_DEBUG_PRINT4(RADIUS_OM_GetDebugFlag(), op, RADIUS_OM_GetDebugPrompt(op), msg, arg1, arg2, arg3, arg4)
#define RADIUS_OM_DEBUG_PRINT5(op, msg, arg1, arg2, arg3, arg4, arg5) SECURITY_DEBUG_PRINT5(RADIUS_OM_GetDebugFlag(), op, RADIUS_OM_GetDebugPrompt(op), msg, arg1, arg2, arg3, arg4, arg5)

#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)
#define RADIUS_MAX_LEN_OF_ACC_ATTRIBUTES    256 /* should be large than secret */
#endif /* SYS_CPNT_RADIUS_ACCOUNTING == TRUE */

/*------------------------------------------------------------------------
 * LOCAL VARIABLES
 *-----------------------------------------------------------------------*/
/*------------------------------------------------------------------------
 * DEFAULT VARIABLES DECLARTIONS
 *-----------------------------------------------------------------------*/
#if 0
#define RADIUS_Default_Server_Port          SYS_DFLT_RADIUS_AUTH_CLIENT_SERVER_PORT_NUMBER
#define RADIUS_Default_Retransmit_Times     SYS_DFLT_RADIUS_AUTH_CLIENT_ACCESS_RETRANSMISSIONS
#define RADIUS_Default_Timeout              SYS_DFLT_RADIUS_AUTH_CLIENT_TIMEOUTS
#define RADIUS_Default_Server_IP            SYS_DFLT_RADIUS_AUTH_SERVER_ADDRESS
#define RADIUS_Default_Server_Secret        ""
#define RADIUS_NOT_IN_MASTER_MODE          -3

#define AUTH_ADMINISTRATIVE                15
#define AUTH_LOGIN	                    0
#define AUTH_LOGIN_ERROR		   -1

//#define RADIUS_MGR_MAX_SERVER               1
#define RADIUS_MAX_RETRANSMIT_TIMES        30
#define RADIUS_MIN_RETRANSMIT_TIMES         1
#define RADIUS_MAX_SERVER_PORT          65535
#define RADIUS_MIN_SERVER_PORT              1
#define RADIUS_MAX_REQUEST_TIMEOUT      65535
#define RADIUS_MIN_REQUEST_TIMEOUT          1

#define RADIUS_MAX_SECRET_LENGTH           SYS_ADPT_RADIUS_SECRET_KEY_MAX_LENGTH
#endif

#define RADIUS_OM_MAX_NBR_OF_AUTH_REQUEST                   256
#define RADIUS_OM_MAX_NBR_OF_ACCT_REQUEST                   SYS_ADPT_MAX_NBR_OF_RADIUS_ACC_USERS
#define RADIUS_OM_MAX_NBR_OF_WAIT_AUTH_REQUEST              256
#define RADIUS_OM_MAX_NBR_OF_WAIT_ACCT_REQUEST              SYS_ADPT_MAX_NBR_OF_RADIUS_ACC_USERS
#define RADIUS_OM_MAX_NBR_OF_REQUEST                        (RADIUS_OM_MAX_NBR_OF_AUTH_REQUEST + RADIUS_OM_MAX_NBR_OF_ACCT_REQUEST)
#define RADIUS_OM_MAX_NBR_OF_WAIT_REQUEST                   (RADIUS_OM_MAX_NBR_OF_WAIT_AUTH_REQUEST + RADIUS_OM_MAX_NBR_OF_WAIT_ACCT_REQUEST)
#define RADIUS_OM_MAX_NBR_OF_IDENTIFIER                     256 /* Value range of identifier field in RADIUS packet */

#define RADIUS_OM_IS_REQUEST_ID_VALID(id)                   ((id >= 1) && (id <= RADIUS_OM_MAX_NBR_OF_REQUEST))
#define RADIUS_OM_IS_REQUEST_STATE_VALID(state) ((state > RADIUS_REQUEST_STATE_INVALID) && (state < RADIUS_REQUEST_STATE_MAX))
#define RADIUS_OM_IS_REQUEST_IDENTIFIER_VALID(identifier)   (identifier < RADIUS_OM_MAX_NBR_OF_IDENTIFIER)

#define RADIUS_OM_INVALID_IDENTIFIER                        (RADIUS_OM_MAX_NBR_OF_IDENTIFIER + 1)
#define RADIUS_OM_INVALID_TIMER_LINKED_LIST_TIME            (0xFFFFFFFF)

#define RADIUS_OM_IS_ACC_USER_INDEX_VALID(id)               ((0 < id) && (id <= SYS_ADPT_MAX_NBR_OF_RADIUS_ACC_USERS))

#define RADIUS_OM_MAX_USER_AUTH_COOKIE_LEN  256

/* The key to get igmpsnp mgr msgq.
 */
#define RADIUS_OM_IPCMSG_KEY    SYS_BLD_AUTH_PROTOCOL_PROC_OM_IPCMSGQ_KEY

/* The commands for IPC message.
 */
enum {
    RADIUS_OM_IPC_CMD_GET_RUNNING_REQUEST_TIMEOUT,
    RADIUS_OM_IPC_CMD_GET_REQUEST_TIMEOUT,
    RADIUS_OM_IPC_CMD_GET_RUNNING_SERVER_PORT,
    RADIUS_OM_IPC_CMD_GET_SERVER_PORT,
    RADIUS_OM_IPC_CMD_GET_RUNNING_SERVER_SECRET,
    RADIUS_OM_IPC_CMD_GET_SERVER_SECRET,
    RADIUS_OM_IPC_CMD_GET_RUNNING_RETRANSMIT_TIMES,
    RADIUS_OM_IPC_CMD_GET_RETRANSMIT_TIMES,
    RADIUS_OM_IPC_CMD_GET_RUNNING_SERVER_IP,
    RADIUS_OM_IPC_CMD_GET_SERVER_IP,
    RADIUS_OM_IPC_CMD_GET_UNKNOW_ADDRESS_PACKETS,
    RADIUS_OM_IPC_CMD_GET_NAS_ID,
    RADIUS_OM_IPC_CMD_GET_AUTH_SERVER_TABLE,
    RADIUS_OM_IPC_CMD_GET_NEXT_AUTH_SERVER_TABLE,
    RADIUS_OM_IPC_CMD_GET_NEXT_SERVER_HOST,
    RADIUS_OM_IPC_CMD_GET_NEXT_RUNNING_SERVER_HOST,
    RADIUS_OM_IPC_CMD_GET_SERVER_HOST,
    RADIUS_OM_IPC_CMD_GET_RUNNING_SERVER_ACCT_PORT,
    RADIUS_OM_IPC_CMD_GET_SERVER_ACCT_PORT,
    RADIUS_POM_GET_ACC_CLIENT_INVALID_SERVER_ADDRESSES
};

/* MACRO FUNCTION DECLARATIONS
 */
/*---------------------------------------------------------------------------
 * MACRO NAME - RADIUS_OM_GET_MSGBUFSIZE
 *---------------------------------------------------------------------------
 * PURPOSE : Get the size of RADIUS message that contains the specified
 *           type of data.
 * INPUT   : type_name - the type name of data for the message.
 * OUTPUT  : None
 * RETURN  : The size of RADIUS message.
 * NOTE    : None
 *---------------------------------------------------------------------------
 */
#define RADIUS_OM_GET_MSGBUFSIZE(type_name) \
    (offsetof(RADIUS_OM_IPCMsg_T, data) + sizeof(type_name))

/*---------------------------------------------------------------------------
 * MACRO NAME - RADIUS_OM_GET_MSGBUFSIZE_FOR_EMPTY_DATA
 *---------------------------------------------------------------------------
 * PURPOSE : Get the size of RADIUS message that has no data block.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : The size of RADIUS message.
 * NOTE    : None
 *---------------------------------------------------------------------------
 */
#define RADIUS_OM_GET_MSGBUFSIZE_FOR_EMPTY_DATA() \
    sizeof(RADIUS_OM_IPCMsg_Type_T)

/*---------------------------------------------------------------------------
 * MACRO NAME - RADIUS_OM_MSG_CMD
 *              RADIUS_OM_MSG_RETVAL
 *---------------------------------------------------------------------------
 * PURPOSE : Get the RADIUS command/return value of an IPC message.
 * INPUT   : msg_p - the IPC message.
 * OUTPUT  : None
 * RETURN  : The RADIUS command/return value; it's allowed to be used as lvalue.
 * NOTE    : None
 *---------------------------------------------------------------------------
 */
#define RADIUS_OM_MSG_CMD(msg_p)    (((RADIUS_OM_IPCMsg_T *)(msg_p)->msg_buf)->type.cmd)
#define RADIUS_OM_MSG_RETVAL(msg_p) (((RADIUS_OM_IPCMsg_T *)(msg_p)->msg_buf)->type.result)

/*---------------------------------------------------------------------------
 * MACRO NAME - RADIUS_OM_MSG_DATA
 *---------------------------------------------------------------------------
 * PURPOSE : Get the data block of an IPC message.
 * INPUT   : msg_p - the IPC message.
 * OUTPUT  : None
 * RETURN  : The pointer of the data block.
 * NOTE    : None
 *---------------------------------------------------------------------------
 */
#define RADIUS_OM_MSG_DATA(msg_p)   ((void *)&((RADIUS_OM_IPCMsg_T *)(msg_p)->msg_buf)->data)

#define RADIUS_OM_ASYNC_MAX_COOKIE_SIZE 256

/* DATA TYPE DECLARATIONS
 */
typedef struct RADIUS_LightServerHost_S {
    UI32_T   server_index; /* array index + 1 */

    UI32_T   server_ip;
    UI32_T   auth_port;

#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)
    UI32_T  acct_port;
#endif /* SYS_CPNT_RADIUS_ACCOUNTING == TRUE */

    UI32_T   timeout;
    UI32_T   retransmit;
    UI8_T    secret[MAXSIZE_radiusServerGlobalKey + 1];

/*    RADIUS_ServerStatus_T   server_status;*/ /* add radius server status since radius CLI command version 0.31 */
} RADIUS_LightServerHost_T;

#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)

typedef enum RADACC_EntryStatus_E
{
    RADACC_ENTRY_DESTROYED,
    RADACC_ENTRY_READY,
} RADACC_EntryStatus_T;

typedef enum RADACC_RequestState_E
{
    RADACC_REQUEST_SENDING,     /* RADIUS_MGR_SendNextAccRequest tackles the requests under this state */
    RADACC_ACK_RECEIVING,       /* task tackles those sockets of requests under this state */
} RADACC_RequestState_T;


typedef enum RADACC_ObjectProcessResult_E
{
    RADACC_OPROCESS_SUCCESS,
    RADACC_OPROCESS_FAILURE,
    RADACC_OPROCESS_DELETED, /* object had been deleted, should not use thereafter */
} RADACC_ObjectProcessResult_T;

/* consider alignment issue, using memcpy() is safest in assignment statement
   so only UI8_T array here
 */
typedef struct RADACC_AccRequestHeader_S
{
    UI8_T   code;
    UI8_T   identifier;
    UI8_T   length[2];
    UI8_T   authenticator[16];
} RADACC_AccRequestHeader_T;

typedef struct RADACC_AccRequestPacket_S
{
    RADACC_AccRequestHeader_T   header;

    UI8_T   attributes[RADIUS_MAX_LEN_OF_ACC_ATTRIBUTES];
} RADACC_AccRequestPacket_T;

typedef struct RADACC_AccRequestReserveredData_S
{
    UI8_T   identifier;
    UI8_T   request_authenticator[AUTH_VECTOR_LEN];
} RADACC_AccRequestReserveredData_T;

typedef struct RADACC_AccReservedSendInfo_S
{
    UI32_T  server_index;           /* index of server host */

    UI32_T  request_sent_time;      /* time of request sent */
    int     sock_id;                /* socket id used for this requequest */
    UI32_T  retry_times;            /* how many times to retry*/
} RADACC_AccReservedSendInfo_T;

typedef struct RADACC_RequestInfo_S
{
    UI16_T  request_index;          /* array index + 1 */
    UI16_T  user_index;             /* index of user list */
    UI32_T  create_time;

    RADACC_AccRequestReserveredData_T   request_data;
    RADACC_AccReservedSendInfo_T        send_data;

    RADACC_AcctStatusType_T     request_type;
    RADACC_RequestState_T       request_state;

    UI32_T  identifier; /* callback function's parameter
                           so caller can know which request's result back */
    AAA_AccRequest_Callback_T   call_back_func; /* callback to the CSC which placed this request */

    RADACC_EntryStatus_T        entry_status;

    struct RADACC_RequestInfo_S *prev_request;
    struct RADACC_RequestInfo_S *next_request;
} RADACC_RequestInfo_T;

typedef struct RADACC_UserCtrlBitmap_S
{
    UI8_T   start_packet_response   :1; /* server response (ACC-Start) had been received */
    UI8_T   reserved                :3;
} RADACC_UserCtrlBitmap_T;

typedef struct RADACC_AAARadiusEntryInfo_S
{
    UI16_T  aaa_group_index;        /* index of aaa radius group */
    UI16_T  aaa_radius_index;       /* index of aaa radius entry in radius group */
    UI32_T  active_server_index;    /* index of server host */
    UI16_T  aaa_radius_order;       /* keep the radius order in the radius group */
} RADACC_AAARadiusEntryInfo_T;

typedef struct RADACC_UserStatisticInfo_S
{
    UI32_T  ifInOctets;
    UI32_T  ifOutOctets;
    UI32_T  ifInUcastPkts;
    UI32_T  ifOutUcastPkts;
} RADACC_UserStatisticInfo_T;

typedef struct RADACC_UserInfo_S
{
    UI16_T  user_index;     /* array index + 1 */

    UI32_T  ifindex;        /* if client_type == AAA_CLIENT_TYPE_DOT1X, ifindex == l_port
                               if client_type == AAA_CLIENT_TYPE_EXEC, ifindex implies console or telnet's session id
                             */
    UI8_T   user_name[SYS_ADPT_MAX_USER_NAME_LEN + 1];

    UI32_T  session_id;

    UI32_T  accounting_start_time;  /* unit is seconds */
    UI32_T  session_start_time;     /* use for caculating session-time (in seconds) */
    UI32_T  last_update_time;       /* unit is seconds */

    UI32_T  accounting_interim_interval;    /* unit is seconds */

    UI32_T  stop_retry_time;        /* when the radius stop accounting */
    BOOL_T  stop_retry_flag;        /* TRUE if there is no next server to retry */
    UI8_T   request_counter;        /* avoid update interval is smaller than time-out */

    UI32_T  request_index;          /* Ongoing request (start, update or stop) index */

    RADACC_AAARadiusEntryInfo_T radius_entry_info;
    RADACC_UserCtrlBitmap_T     ctrl_bitmap;
    AAA_AccConnectStatus_T      connect_status;

    /* reserved identifier and call_back_func if can't hook a request currently
       and them will be transfer to request if successfully to hook a request
     */
    UI32_T  identifier; /* callback function's parameter
                           so caller can know which request's result back */
    AAA_AccRequest_Callback_T   call_back_func; /* callback to the CSC which placed this request */

    AAA_ClientType_T            client_type;

    AAA_AccAuthentic_T          auth_by_whom;       /* meaningfully only while START */
    AAA_AccTerminateCause_T     terminate_cause;    /* meaningfully only while STOP */

    RADACC_UserStatisticInfo_T  statistic_info;

    RADACC_EntryStatus_T        entry_status;

    UI8_T                       auth_mac[SYS_ADPT_MAC_ADDR_LEN];

    BOOL_T                      destroy_flag; /* denote that the user is marked as required to cancel */
} RADACC_UserInfo_T;

#endif /* SYS_CPNT_RADIUS_ACCOUNTING == TRUE */

typedef enum RADIUS_OM_RequestState_E 
{
    RADIUS_REQUEST_STATE_INVALID,
    RADIUS_REQUEST_STATE_INIT,
    RADIUS_REQUEST_STATE_SEND,
    RADIUS_REQUEST_STATE_WAIT_RESPONSE,
    RADIUS_REQUEST_STATE_RECEIVE,
    RADIUS_REQUEST_STATE_TIMEOUT,
    RADIUS_REQUEST_STATE_SUCCESS,
    RADIUS_REQUEST_STATE_FAIL,
    RADIUS_REQUEST_STATE_CANCELLED,
    RADIUS_REQUEST_STATE_DESTROY,
    RADIUS_REQUEST_STATE_MAX
} RADIUS_OM_RequestState_T;

typedef enum RADIUS_OM_RequestType_E 
{
    RADIUS_REQUEST_TYPE_UNSPECIFIED,
    RADIUS_REQUEST_TYPE_IGMPAUTH,
    RADIUS_REQUEST_TYPE_DOT1X_AUTH,
    RADIUS_REQUEST_TYPE_RADA_AUTH,
    RADIUS_REQUEST_TYPE_WEB_AUTH,
    RADIUS_REQUEST_TYPE_USER_AUTH,
    RADIUS_REQUEST_TYPE_ACCOUNTING_CREATE, /* only used for create accounting */
    RADIUS_REQUEST_TYPE_ACCOUNTING,
    RADIUS_REQUEST_TYPE_MAX
} RADIUS_OM_RadiusRequestType_T;

typedef struct RADIUS_OM_IGMPAuthRequestData_S
{
    UI8_T    auth_mac[SYS_ADPT_MAC_ADDR_LEN];
    UI32_T   auth_port;
    UI32_T   ip_address;
    UI32_T   vlan_id;
    UI32_T   src_ip;
    UI8_T    msg_type;
} RADIUS_OM_IGMPAuthRequestData_T;

typedef struct RADIUS_OM_Dot1xAuthRequestData_S
{
    UI32_T  server_ip;

    char    username[DOT1X_USERNAME_LENGTH + 1];
    UI32_T  src_port;
    UI8_T   src_mac[SYS_ADPT_MAC_ADDR_LEN];
    UI32_T  src_vid;

    UI32_T  cookie;

    UI8_T   eap_data[EAP_MESSAGE_SIZE];
    UI32_T  eap_data_len;

    UI8_T   state_data[STATE_MESSAGE_SIZE];
    UI32_T  state_data_len;
} RADIUS_OM_Dot1xAuthRequestData_T;

typedef struct RADIUS_OM_RadaAuthRequestData_S
{
    UI32_T  src_port;
    UI8_T   src_mac[SYS_ADPT_MAC_ADDR_LEN];

    UI32_T  cookie;

    char    username[RADIUS_MAX_MAC_STRING_LENGTH + 1];
    char    password[RADIUS_MAX_MAC_STRING_LENGTH + 1];
} RADIUS_OM_RadaAuthRequestData_T;

typedef struct RADIUS_OM_UserAuthRequestData_S
{
    char    username[SYS_ADPT_MAX_USER_NAME_LEN + 1];
    char    password[SYS_ADPT_MAX_PASSWORD_LEN + 1];

    union
    {
        UI32_T  web;

        struct
        {
            UI8_T   value[RADIUS_OM_ASYNC_MAX_COOKIE_SIZE];
            UI32_T  len;
        } cli;
    } cookie;
} RADIUS_OM_UserAuthRequestData_T;

#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)
typedef struct RADIUS_OM_AccountingCreateRequestData_S
{
    AAA_AccRequest_T    request;
} RADIUS_OM_AccountingCreateRequestData_T;

typedef struct RADIUS_OM_AccountingRequestData_S
{
    RADACC_AcctStatusType_T request_type;
    UI16_T                  user_index;
    UI32_T                  created_time;
} RADIUS_OM_AccountingRequestData_T;
#endif /* #if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE) */

typedef struct RADIUS_OM_RequestEntry_S
{
    RADIUS_OM_RadiusRequestType_T               type;
    BOOL_T                                      is_wait_for_response;

    union
    {
        RADIUS_OM_IGMPAuthRequestData_T         igmp_auth_data;
        RADIUS_OM_Dot1xAuthRequestData_T        dot1x_auth_data;
        RADIUS_OM_RadaAuthRequestData_T         rada_auth_data;
        RADIUS_OM_UserAuthRequestData_T         user_auth_data;
#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)
        RADIUS_OM_AccountingCreateRequestData_T accounting_create_data;
        RADIUS_OM_AccountingRequestData_T       accounting_data;
#endif /* #if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE) */
    };
} RADIUS_OM_RequestEntry_T;

typedef struct RADIUS_OM_ServerArray_S
{
    UI32_T servers[SYS_ADPT_MAX_NBR_OF_RADIUS_SERVERS];
    UI32_T count;
    UI32_T current_index;
} RADIUS_OM_ServerArray_T;

typedef struct RADIUS_OM_RequestResult_S
{
    UI32_T  server_ip;

    union
    {
        struct RADIUS_OM_Dot1xAuthResult_T
        {
            UI32_T  tunnel_type;
            UI32_T  tunnel_medium_type;
            char    tunnel_private_group_id[SYS_ADPT_NETACCESS_MAX_LEN_OF_VLAN_LIST + 1];
            char    filter_id[SYS_ADPT_NETACCESS_MAX_LEN_OF_QOS_PROFILE + 1];
            UI32_T  session_timeout;
        } dot1x_auth;
        struct RADIUS_OM_RadaAuthResult_T
        {
            UI32_T  tunnel_type;
            UI32_T  tunnel_medium_type;
            char    tunnel_private_group_id[SYS_ADPT_NETACCESS_MAX_LEN_OF_VLAN_LIST + 1];
            char    filter_id[SYS_ADPT_NETACCESS_MAX_LEN_OF_QOS_PROFILE + 1];
            UI32_T  session_timeout;
        } rada_auth;
        struct RADIUS_OM_UserAuthResult_T
        {
            I32_T   privilege;
        } user_auth;
    };
} RADIUS_OM_RequestResult_T;

typedef struct RADIUS_OM_Request_S
{
    /* State in state machine
     */
    RADIUS_OM_RequestState_T    state;

    /* RADIUS server
     */
    UI32_T                      retry_times; /* current retry times */
    char                        secret_key[MAXSIZE_radiusServerGlobalKey + 1]; /* secret key of current RADIUS server */
    UI32_T                      server_index; /* current index of RADIUS server */
    RADIUS_OM_ServerArray_T     server_array;

    /* Metadata of RADIUS packet
     */
    UI32_T                      identifier;
    UI8_T                       vector[AUTH_VECTOR_LEN];
    UI32_T                      last_sent_time; /* in time tick */

    /* Request data (ex: username, password, etc.)
     */
    RADIUS_OM_RequestEntry_T    request_data;

    /* Response packet
     */
    UI8_T                       *response_packet_p;
    UI32_T                      response_packet_len;

    RADIUS_OM_RequestResult_T   result_data;

    BOOL_T                      is_pending_counter_increased;
    BOOL_T                      destroy_flag; /* denote that the request is marked as required to cancel */

    BOOL_T                      is_used;
} RADIUS_OM_RadiusRequest_T;

/* Message declarations for IPC.
 */
typedef struct
{
    UI32_T timeout;
} RADIUS_OM_IPCMsg_RequestTimeout_T;

typedef struct
{
    UI32_T serverport;
} RADIUS_OM_IPCMsg_ServerPort_T;

typedef struct
{
    UI8_T  serversecret[MAXSIZE_radiusServerGlobalKey + 1];
} RADIUS_OM_IPCMsg_ServerSecret_T;

typedef struct
{
    UI32_T retimes;
} RADIUS_OM_IPCMsg_RetransmitTimes_T;

typedef struct
{
    UI32_T serverip;
} RADIUS_OM_IPCMsg_ServerIP_T;

typedef struct
{
    UI8_T  nas_id[MAXSIZE_radiusAuthClientIdentifier + 1];
} RADIUS_OM_IPCMsg_NAS_ID_T;

typedef struct
{
    UI32_T index;
    AuthServerEntry ServerEntry;
} RADIUS_OM_IPCMsg_AuthServerTable_T;

typedef struct
{
    UI32_T index;
    RADIUS_Server_Host_T server_host;
} RADIUS_OM_IPCMsg_ServerHost_T;

typedef struct
{
    UI32_T acct_port;
} RADIUS_OM_IPCMsg_ServerAcctPort_T;

typedef struct
{
    UI32_T counter;
} RADIUS_OM_IPCMsg_Counter_T;

typedef union
{
    UI32_T cmd;    /* for sending IPC request. CSCA_MGR_IPC_CMD1 ... */
    UI32_T result; /* for response */
} RADIUS_OM_IPCMsg_Type_T;

typedef union
{
    RADIUS_OM_IPCMsg_RequestTimeout_T   request_timeout_data;
    RADIUS_OM_IPCMsg_ServerPort_T       server_port_data;
    RADIUS_OM_IPCMsg_ServerSecret_T     server_secret_data;
    RADIUS_OM_IPCMsg_RetransmitTimes_T  retransmit_times_data;
    RADIUS_OM_IPCMsg_ServerIP_T         server_ip_data;
    RADIUS_OM_IPCMsg_NAS_ID_T           nas_id_data;
    RADIUS_OM_IPCMsg_AuthServerTable_T  auth_server_table_data;
    RADIUS_OM_IPCMsg_ServerHost_T       server_host_data;
    RADIUS_OM_IPCMsg_ServerAcctPort_T   server_acct_port;
    RADIUS_OM_IPCMsg_Counter_T          counter_data;
} RADIUS_OM_IPCMsg_Data_T;

typedef struct
{
    RADIUS_OM_IPCMsg_Type_T type;
    RADIUS_OM_IPCMsg_Data_T data;
} RADIUS_OM_IPCMsg_T;



/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_GetDebugFlag
 *---------------------------------------------------------------------------
 * PURPOSE  : get the debug flag
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *---------------------------------------------------------------------------
 */
UI32_T RADIUS_OM_GetDebugFlag();

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_SetDebugFlag
 *---------------------------------------------------------------------------
 * PURPOSE  : set the debug flag
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *---------------------------------------------------------------------------
 */
void RADIUS_OM_SetDebugFlag(UI32_T flag);

/* FUNCTION NAME:  RADIUS_OM_GetDebugPrompt
 *---------------------------------------------------------------------------
 * PURPOSE  : get the debug prompt
 * INPUT    : flag -- debug flag
 * OUTPUT   : none
 * RETURN   : string form of debug flag
 * NOTES    : none
 *---------------------------------------------------------------------------
 */
const char* RADIUS_OM_GetDebugPrompt(UI32_T flag);

/*---------------------------------------------------------------------------
 * FUNCTION NAME - RADIUS_OM_CreatSem
 *---------------------------------------------------------------------------
 * PURPOSE  :   Initiate the semaphore for RADIUS objects
 * INPUT    :   None
 * OUTPUT   :   None
 * RETURN   :   None
 * NOTE     :   None
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_CreatSem(void);

void RADIUS_OM_EnterSem(void);

void RADIUS_OM_LeaveSem(void);

UI32_T RADIUS_OM_Get_TaskId();

void RADIUS_OM_Set_TaskId(UI32_T task_id);

/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_OM_SetConfigSettingToDefault
 *---------------------------------------------------------------------------
 * PURPOSE: This function will set default value of RADIUS configuration
 * INPUT:  None.
 * OUTPUT: None.
 * RETURN: None.
 * NOTES:  None.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_SetConfigSettingToDefault();

#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_AccInitialize
 *---------------------------------------------------------------------------
 * PURPOSE  : (re-)initialize accounting om
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : FALSE - failed
 * NOTES    : none.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_AccInitialize();

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_AccFinalize
 *---------------------------------------------------------------------------
 * PURPOSE  : clean accounting om resource
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none.
 *---------------------------------------------------------------------------
 */
void RADIUS_OM_AccFinalize();

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_AccIncSessionIdBootPart
 *---------------------------------------------------------------------------
 * PURPOSE  : increase boot part of session id
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none.
 *---------------------------------------------------------------------------
 */
void RADIUS_OM_AccIncSessionIdBootPart();

#endif /* SYS_CPNT_RADIUS_ACCOUNTING == TRUE */



/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_OM_GetRunningRequestTimeout
 *---------------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default RADIUS request timeout is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: timeout
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 *---------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T RADIUS_OM_GetRunningRequestTimeout(UI32_T *timeout);
/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_Get_Request_Timeout
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the number of times the RADIUS client
 *           waits for a reply from RADIUS server
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   timeout value.
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
UI32_T RADIUS_OM_Get_Request_Timeout(void);
/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_Set_Request_Timeout
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set the number of times the RADIUS client
 *           waits for a reply from RADIUS server
 * INPUT:    timeout value.
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_Set_Request_Timeout(UI32_T timeval);
/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_OM_GetRunningServerPort
 *---------------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default RADIUS server port is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: serverport
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 *---------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T RADIUS_OM_GetRunningServerPort(UI32_T *serverport);
/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_Get_Server_Port
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the UDP port number of the remote RADIUS server
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   Prot number
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
UI32_T RADIUS_OM_Get_Server_Port(void);
/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_Set_Server_Port
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set the UDP port number of the remote RADIUS server
 * INPUT:    Prot number
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_Set_Server_Port(UI32_T serverport);



#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)

 /*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_OM_GetRunningServerAcctPort
 *---------------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default RADIUS server port is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: serverport
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 *---------------------------------------------------------------------------
 */
UI32_T RADIUS_OM_GetRunningServerAcctPort(UI32_T *acct_port);

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_GetServerAcctPort
 *---------------------------------------------------------------------------
 * PURPOSE  : get global server accounting port
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : port number
 * NOTES    : none
 *---------------------------------------------------------------------------
 */
UI32_T RADIUS_OM_GetServerAcctPort();

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_SetServerAcctPort
 *---------------------------------------------------------------------------
 * PURPOSE  : set global server accounting port
 * INPUT    : acct_port
 * OUTPUT   : none
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_SetServerAcctPort(UI32_T acct_port);

#endif /* SYS_CPNT_RADIUS_ACCOUNTING == TRUE */



/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_OM_GetRunningServerSecret
 *---------------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default RADIUS server secret is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: serversecret[MAX_SECRET_LENGTH + 1]
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 *---------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T  RADIUS_OM_GetRunningServerSecret(UI8_T serversecret[]);
/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_Get_Server_Secret
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the shared secret text string used between
 *           the RADIUS client and server.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   secret text string pointer
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
UI8_T * RADIUS_OM_Get_Server_Secret(void);
/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_Set_Server_Secret
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set the shared secret text string used between
 *           the RADIUS client and server.
 * INPUT:    secret text string
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_Set_Server_Secret(UI8_T *serversecret);
/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_OM_GetRunningRetransmitTimes
 *---------------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default RADIUS retransmit times is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: retimes
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 *---------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T RADIUS_OM_GetRunningRetransmitTimes(UI32_T *retimes);
/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_Get_Retransmit_Times
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the number of times the RADIUS client
 *           transmits each RADIUS request to the server before giving up
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   retransmit times
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
UI32_T RADIUS_OM_Get_Retransmit_Times(void);
/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_Set_Retransmit_Times
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set the number of times the RADIUS client
 *           transmits each RADIUS request to the server before giving up
 * INPUT:    retransmit times
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_Set_Retransmit_Times(UI32_T retryval);
/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_OM_GetRunningServerIP
 *---------------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default RADIUS server IP is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: serverip
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 *---------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T RADIUS_OM_GetRunningServerIP(UI32_T *serverip);
/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_Get_Server_IP
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the IP address of the remote RADIUS server
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   RADIUS server IP address
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
UI32_T  RADIUS_OM_Get_Server_IP(void);
/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_Set_Server_IP
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set the IP address of the remote RADIUS server
 * INPUT:    RADIUS server IP address
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_Set_Server_IP(UI32_T serverip);
/*  For MIB */
/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_SNMP_Get_UnknowAddress_Packets
 *---------------------------------------------------------------------------
 * PURPOSE:  Get the number of RADIUS Access-Response packets
 *           received from unknown addresses.
 * INPUT:    None
 * OUTPUT:   None.
 * RETURN:   Number of RADIUS Access-Response packets received from unknown addresses
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
UI32_T  RADIUS_OM_SNMP_Get_UnknowAddress_Packets(void);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_SNMP_Set_UnknowAddress_Packets
 *---------------------------------------------------------------------------
 * PURPOSE:  Set the number of RADIUS Access-Response packets
 *           received from unknown addresses.
 * INPUT:    counts
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
void RADIUS_OM_SNMP_Set_UnknowAddress_Packets(UI32_T counts);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_SNMP_Get_NAS_ID
 *---------------------------------------------------------------------------
 * PURPOSE:  Get he NAS-Identifier of the RADIUS authentication client.
 *           This is not necessarily the same as sysName in MIB II.
 * INPUT:    None
 * OUTPUT:   None.
 * RETURN:   NASID
 *           NASID = NULL  ---No NAS ID
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_SNMP_Get_NAS_ID(UI8_T *nas_id);
/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_SNMP_GetAuthServerTable
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will call RADIUS client MIB to get radiusAuthServerTable
 * INPUT:    adiusAuthServerIndex
 * OUTPUT:   AuthServerEntry pointer
 * RETURN:   Get table TRUE/FALSE
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_SNMP_GetAuthServerTable(UI32_T Index,AuthServerEntry *ServerEntry);
/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_SNMP_GetNextAuthServerTable
 *---------------------------------------------------------------------------
 * PURPOSE:  This funtion returns true if the next available table entry info
 *           can be successfully retrieved. Otherwise, false is returned.
 * INPUT:    RadiusAuthServerIndex ,
 * OUTPUT:   NextAuthServerEntry pointer
 * RETURN:   Get next table TRUE/FALSE
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_SNMP_GetNextAuthServerTable(UI32_T *Index,AuthServerEntry *ServerEntry);

/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_OM_IncreaseServerPendingRequestCounter
 *---------------------------------------------------------------------------
 * PURPOSE  : Increase the pending request counter of specified RADIUS server
 *            in MIB.
 * INPUT    : server_index  - RADIUS server index
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    : None
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_IncreaseServerPendingRequestCounter(UI32_T server_index);

/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_OM_DecreaseServerPendingRequestCounter
 *---------------------------------------------------------------------------
 * PURPOSE  : Decrease the pending request counter of specified RADIUS server
 *            in MIB.
 * INPUT    : server_index  - RADIUS server index
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    : None
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_DecreaseServerPendingRequestCounter(UI32_T server_index);

/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_OM_IncreaseServerTimeoutRequestCounter
 *---------------------------------------------------------------------------
 * PURPOSE  : Increase the timeout request counter of specified RADIUS server
 *            in MIB.
 * INPUT    : server_index  - RADIUS server index
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    : None
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_IncreaseServerTimeoutCounter(UI32_T server_index);

/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_OM_IncreaseServerAccessRetransmissionsCounter
 *---------------------------------------------------------------------------
 * PURPOSE  : Increase the access retransmission counter of specified RADIUS
 *            server in MIB.
 * INPUT    : server_index  - RADIUS server index
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    : None
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_IncreaseServerAccessRetransmissionsCounter(
    UI32_T server_index);

/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_OM_IncreaseServerAccessRequestCounter
 *---------------------------------------------------------------------------
 * PURPOSE  : Increase the access request counter of specified RADIUS server
 *            in MIB.
 * INPUT    : server_index  - RADIUS server index
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    : None
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_IncreaseServerAccessRequestCounter(UI32_T server_index);

/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_OM_SetServerRoundTripTime
 *---------------------------------------------------------------------------
 * PURPOSE  : Increase the round trip time of specified RADIUS server in MIB.
 * INPUT    : server_index  - RADIUS server index
 *            time          - Round trip time
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    : None
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_SetServerRoundTripTime(UI32_T server_index, UI32_T time);

/****************************************************************************/
/*UI32_T RADIUS_OM_Get_Global_Identity ();*/
BOOL_T RADIUS_OM_Set_Global_Identity (UI32_T identity);
UI8_T * RADIUS_OM_Get_State_Attr();
BOOL_T RADIUS_OM_Set_State_Attr(UI8_T * attribute,UI32_T attribute_len);
UI32_T RADIUS_OM_Get_State_Attr_Len();
BOOL_T RADIUS_OM_Set_State_Attr_Len(UI32_T attribute_len);
UI32_T RADIUS_OM_MIB_Get_RoundTripTimeCounter();
BOOL_T RADIUS_OM_MIB_Set_RoundTripTimeCounter(UI32_T mib_counter);
UI32_T RADIUS_OM_MIB_Get_AccessRequestsCounter();
BOOL_T RADIUS_OM_MIB_Set_AccessRequestsCounter(UI32_T mib_counter);
UI32_T RADIUS_OM_MIB_Get_AccessAcceptsCounter();
BOOL_T RADIUS_OM_MIB_Set_AccessAcceptsCounter(UI32_T mib_counter);
UI32_T RADIUS_OM_MIB_Get_AccessRejectsCounter();
BOOL_T RADIUS_OM_MIB_Set_AccessRejectsCounter(UI32_T mib_counter);
UI32_T RADIUS_OM_MIB_Get_AccessChallengesCounter();
BOOL_T RADIUS_OM_MIB_Set_AccessChallengesCounter(UI32_T mib_counter);
UI32_T RADIUS_OM_MIB_Get_MalformedAccessResponsesCounter();
BOOL_T RADIUS_OM_MIB_Set_MalformedAccessResponsesCounter(UI32_T mib_counter);
UI32_T RADIUS_OM_MIB_Get_BadAuthenticatorsCounter();
BOOL_T RADIUS_OM_MIB_Set_BadAuthenticatorsCounter(UI32_T mib_counter);
UI32_T RADIUS_OM_MIB_Get_PendingRequestsCounter();
BOOL_T RADIUS_OM_MIB_Set_PendingRequestsCounter(UI32_T mib_counter);
UI32_T RADIUS_OM_MIB_Get_UnknownTypesCounter();
BOOL_T RADIUS_OM_MIB_Set_UnknownTypesCounter(UI32_T mib_counter);
UI32_T RADIUS_OM_MIB_Get_PacketsDroppedCounter();
BOOL_T RADIUS_OM_MIB_Set_PacketsDroppedCounter(UI32_T mib_counter);
R_EAP_DATA* RADIUS_OM_Get_Eap_Message_Ptr();
R_STATE_DATA* RADIUS_OM_Get_State_Message_Ptr();

BOOL_T RADIUS_OM_Set_Server_Host(UI32_T server_index,RADIUS_Server_Host_T *server_host);
BOOL_T RADIUS_OM_Destroy_Server_Host(UI32_T server_index);
BOOL_T RADIUS_OM_GetNext_Server_Host(UI32_T *index,RADIUS_Server_Host_T *server_host);
SYS_TYPE_Get_Running_Cfg_T RADIUS_OM_GetNextRunning_Server_Host(UI32_T *index,RADIUS_Server_Host_T *server_host);
BOOL_T RADIUS_OM_Get_Server_Host(UI32_T index,RADIUS_Server_Host_T *server_host);

/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_OM_Reset_All_Server_Host_Status
 *---------------------------------------------------------------------------
 * PURPOSE: Reset all server status to unknown if all host status are
 *          unreachable
 * INPUT:   None.
 * OUTPUT:  None.
 * RETURN:  None.
 * NOTES:   None.
 *---------------------------------------------------------------------------
 */
void RADIUS_OM_Reset_All_Server_Host_Status();

/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_OM_Set_Server_Host_Status
 *---------------------------------------------------------------------------
 * PURPOSE: Set host status
 * INPUT:   index   - Server host index. 1-based.
 *          status  - Server status
 * OUTPUT:  None.
 * RETURN:  TRUE/FALSE
 * NOTES:   None.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_Set_Server_Host_Status(UI32_T index, RADIUS_ServerStatus_T status);


/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_GetLightServerHost
 *---------------------------------------------------------------------------
 * PURPOSE  : get specified server entry by index
 * INPUT    : server_index (1-based)
 * OUTPUT   : server_host
 * RETURN   : TRUE -- succeeded, FALSE -- Failed
 * NOTE     : index RANGE (1..SYS_ADPT_MAX_NBR_OF_RADIUS_SERVERS)
 *            fail (1). if out of range (2). used_flag == false
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_GetLightServerHost(UI32_T server_index, RADIUS_LightServerHost_T *server_host);

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_Get_Server_Host_Entry
 *---------------------------------------------------------------------------
 * PURPOSE  : get server host entry by server_index
 * INPUT    : server_index (1-based)
 * OUTPUT   : none
 * RETURN   : NULL - not found
 * NOTES    : call by rc_send_server
 *---------------------------------------------------------------------------
 */
RADIUS_Server_Host_T *RADIUS_OM_Get_Server_Host_Entry(UI32_T server_index);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_GetServerHostIpAddress
 *---------------------------------------------------------------------------
 * PURPOSE  : get the ip address by server_index
 * INPUT    : server_index (1-based)
 * OUTPUT   : ip_address
 * RETURN   : TRUE -- succeeded, FALSE -- Failed
 * NOTE     : none
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_GetServerHostIpAddress(UI32_T server_index, UI32_T *ip_address);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_GetServerHostTimeout
 *---------------------------------------------------------------------------
 * PURPOSE  : get the retransmission_timeout by server_index
 * INPUT    : server_index (1-based)
 * OUTPUT   : retransmission_timeout
 * RETURN   : TRUE -- succeeded, FALSE -- Failed
 * NOTE     : none
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_GetServerHostTimeout(UI32_T server_index, UI32_T *retransmission_timeout);

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_GetServerHostMaxRetransmissionTimeout
 *---------------------------------------------------------------------------
 * PURPOSE  : query the max of retransmission timeout of server hosts
 * INPUT    : none
 * OUTPUT   : max_retransmission_timeout
 * RETURN   : TRUE - succeeded; FALSE - failed
 * NOTES    : none
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_GetServerHostMaxRetransmissionTimeout(UI32_T *max_retransmission_timeout);

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_IsServerHostActive
 *---------------------------------------------------------------------------
 * PURPOSE  : query whether this server host is active or not
 * INPUT    : server_index (1-based)
 * OUTPUT   : none
 * RETURN   : TRUE - valid; FALSE - invalid
 * NOTES    : an entry is invalid entry if server_status != RADIUS_SERVER_STATUS_ACTIVE
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_IsServerHostActive(UI32_T server_index);

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_IsServerHostValid
 *---------------------------------------------------------------------------
 * PURPOSE  : query whether this server host is valid or not
 * INPUT    : server_index (1-based)
 * OUTPUT   : none
 * RETURN   : TRUE - valid; FALSE - invalid
 * NOTES    : none
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_IsServerHostValid(UI32_T server_index);

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_LookupServerIndexByIpAddress
 *---------------------------------------------------------------------------
 * PURPOSE  : lookup server host by ip_address
 * INPUT    : ip_address
 * OUTPUT   : server_index (1-based)
 * RETURN   : TRUE - found; FALSE - not found
 * NOTES    : none
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_LookupServerIndexByIpAddress(UI32_T ip_address, UI32_T *server_index);



I32_T RADIUS_OM_Get_Filter_Id_Privilege();
BOOL_T RADIUS_OM_Set_Filter_Id_Privilege( I32_T privilege);

#if(SYS_ADPT_RADIUS_USE_FIELD_ID_AS_PRIVILEGE == TRUE)
    BOOL_T RADIUS_OM_SetAuthCheckingServiceTypeEnabled(BOOL_T mode);
    BOOL_T RADIUS_OM_GetAuthCheckingServiceTypeEnabled(BOOL_T *mode);
    SYS_TYPE_Get_Running_Cfg_T RADIUS_OM_GetRunningAuthCheckingServiceTypeEnabled(BOOL_T *mode);
#endif
#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_GetAccPacketNeedToHook
 *---------------------------------------------------------------------------
 * PURPOSE  : none
 * INPUT    : none
 * OUTPUT   : none.
 * RETURN   : TRUE - need to hook
 * NOTES    : none.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_GetAccPacketNeedToHook();

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_SetAccPacketNeedToHook
 *---------------------------------------------------------------------------
 * PURPOSE  : set need_to_hook flag
 * INPUT    : need_flag
 * OUTPUT   : none.
 * RETURN   : none
 * NOTES    : none.
 *---------------------------------------------------------------------------
 */
void RADIUS_OM_SetAccPacketNeedToHook(BOOL_T need_flag);


/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_GetAccClientInvalidServerAddresses
 *---------------------------------------------------------------------------
 * PURPOSE  : get the number of RADIUS Accounting-Response packets received from unknown addresses.
 * INPUT    : none.
 * OUTPUT   : invalid_server_address_counter
 * RETURN   : TRUE - successful, FALSE - failure
 * NOTES    : none.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_GetAccClientInvalidServerAddresses(UI32_T *invalid_server_address_counter);

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_SetAccClientInvalidServerAddresses
 *---------------------------------------------------------------------------
 * PURPOSE  : set the number of RADIUS Accounting-Response packets received from unknown addresses.
 * INPUT    : invalid_server_address_counter
 * OUTPUT   : none.
 * RETURN   : TRUE - successful, FALSE - failure
 * NOTES    : none.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_SetAccClientInvalidServerAddresses(UI32_T invalid_server_address_counter);

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_IncAccClientInvalidServerAddresses
 *---------------------------------------------------------------------------
 * PURPOSE  : increase the number of RADIUS Accounting-Response packets received from unknown addresses.
 * INPUT    : inc_qty
 * OUTPUT   : none.
 * RETURN   : TRUE - successful, FALSE - failure
 * NOTES    : none.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_IncAccClientInvalidServerAddresses(UI32_T inc_qty);

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_SetAccClientRoundTripTime
 *---------------------------------------------------------------------------
 * PURPOSE  : set the time interval between the most recent Accounting-Response
 *            and the Accounting-Request that matched it from this RADIUS accounting server.
 * INPUT    : server_index (1-based), time -- the time interval.
 * OUTPUT   : none.
 * RETURN   : TRUE - successful, FALSE - failure
 * NOTES    : none.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_SetAccClientRoundTripTime(UI32_T server_index, UI32_T time);

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_IncAccClientRetransmissions
 *---------------------------------------------------------------------------
 * PURPOSE  : increase the number of RADIUS Accounting-Request packets retransmitted
 *            to this RADIUS accounting server.
 * INPUT    : server_index (1-based). inc_qty
 * OUTPUT   :  none.
 * RETURN   : TRUE - successful, FALSE - failure
 * NOTES    : none.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_IncAccClientRetransmissions(UI32_T server_index, UI32_T inc_qty);

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_IncAccClientResponses
 *---------------------------------------------------------------------------
 * PURPOSE  : increase the number of RADIUS packets received on the
 *            accounting port from this server.
 * INPUT    : server_index (1-based). inc_qty
 * OUTPUT   : none.
 * RETURN   : TRUE - successful, FALSE - failure
 * NOTES    : none.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_IncAccClientResponses(UI32_T server_index, UI32_T inc_qty);

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_IncAccClientMalformedResponses
 *---------------------------------------------------------------------------
 * PURPOSE  : increase the number of malformed RADIUS Accounting-Response
 *            packets received from this server. Malformed packets
 *            include packets with an invalid length. Bad
 *            authenticators and unknown types are not included as
 *            malformed accounting responses.
 * INPUT    : server_index (1-based). inc_qty
 * OUTPUT   : none.
 * RETURN   : TRUE - successful, FALSE - failure
 * NOTES    : none.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_IncAccClientMalformedResponses(UI32_T server_index, UI32_T inc_qty);

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_IncAccClientBadAuthenticators
 *---------------------------------------------------------------------------
 * PURPOSE  : increase the number of RADIUS Accounting-Response
 *            packets which contained invalid authenticators
 *            received from this server.
 * INPUT    : server_index (1-based). inc_qty
 * OUTPUT   : none.
 * RETURN   : TRUE - successful, FALSE - failure
 * NOTES    : none.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_IncAccClientBadAuthenticators(UI32_T server_index, UI32_T inc_qty);

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_IncAccClientPendingRequests
 *---------------------------------------------------------------------------
 * PURPOSE  : increase the number of RADIUS Accounting-Request packets
 *            sent to this server that have not yet timed out or
 *            received a response.
 * INPUT    : server_index (1-based). inc_qty -- negative qty implies decrease
 * OUTPUT   : none.
 * RETURN   : TRUE - successful, FALSE - failure
 * NOTES    : none.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_IncAccClientPendingRequests(UI32_T server_index, I32_T inc_qty);

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_IncAccClientTimeouts
 *---------------------------------------------------------------------------
 * PURPOSE  : increase the number of accounting timeouts to this server.
 * INPUT    : server_index (1-based). inc_qty
 * OUTPUT   : none.
 * RETURN   : TRUE - successful, FALSE - failure
 * NOTES    : none.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_IncAccClientTimeouts(UI32_T server_index, UI32_T inc_qty);

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_IncAccClientUnknownTypes
 *---------------------------------------------------------------------------
 * PURPOSE  : increase the number of RADIUS packets of unknown type which
 *            were received from this server on the accounting port.
 * INPUT    : server_index (1-based). inc_qty
 * OUTPUT   : none.
 * RETURN   : TRUE - successful, FALSE - failure
 * NOTES    : none.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_IncAccClientUnknownTypes(UI32_T server_index, UI32_T inc_qty);

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_IncAccClientPacketsDropped
 *---------------------------------------------------------------------------
 * PURPOSE  : increase the number of RADIUS packets which were received from
 *            this server on the accounting port and dropped for some
 *            other reason.
 * INPUT    : server_index (1-based). inc_qty
 * OUTPUT   : none.
 * RETURN   : TRUE - successful, FALSE - failure
 * NOTES    : none.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_IncAccClientPacketsDropped(UI32_T server_index, UI32_T inc_qty);


/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_GetAccUserEntryQty
 *---------------------------------------------------------------------------
 * PURPOSE  : get the number of user entries
 * INPUT    : none.
 * OUTPUT   : qty.
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : exclude "dead" user which connect_status ==
 *            AAA_ACC_CNET_DORMANT, AAA_ACC_CNET_IDLE, AAA_ACC_CNET_FAILED
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_GetAccUserEntryQty(UI32_T *qty);

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_GetAccUserEntryQtyFilterByNameAndType
 *---------------------------------------------------------------------------
 * PURPOSE  : get the number of user with specified name and client_type
 * INPUT    : name, client_type
 * OUTPUT   : qty.
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : exclude "dead" user which connect_status ==
 *            AAA_ACC_CNET_DORMANT, AAA_ACC_CNET_IDLE, AAA_ACC_CNET_FAILED
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_GetAccUserEntryQtyFilterByNameAndType(const char *name, AAA_ClientType_T client_type, UI32_T *qty);

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_GetAccUserEntryQtyFilterByType
 *---------------------------------------------------------------------------
 * PURPOSE  : get the number of user with specified client_type
 * INPUT    : client_type
 * OUTPUT   : qty.
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : exclude "dead" user which connect_status ==
 *            AAA_ACC_CNET_DORMANT, AAA_ACC_CNET_IDLE, AAA_ACC_CNET_FAILED
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_GetAccUserEntryQtyFilterByType(AAA_ClientType_T client_type, UI32_T *qty);

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_GetAccUserEntryQtyFilterByPort
 *---------------------------------------------------------------------------
 * PURPOSE  : get the number of user from specified port
 * INPUT    : ifindex
 * OUTPUT   : qty.
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : exclude "dead" user which connect_status ==
 *            AAA_ACC_CNET_DORMANT, AAA_ACC_CNET_IDLE, AAA_ACC_CNET_FAILED
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_GetAccUserEntryQtyFilterByPort(UI32_T ifindex, UI32_T *qty);

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_GetNextAccUserEntry
 *---------------------------------------------------------------------------
 * PURPOSE  : copy next accounting user to user_entry
 * INPUT    : user_index
 * OUTPUT   : entry
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : prev_user, next_user are unavailable
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_GetNextAccUserEntry(RADACC_UserInfo_T *entry);

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_GetAccUserEntry
 *---------------------------------------------------------------------------
 * PURPOSE  : copy accounting user to user_entry
 * INPUT    : user_index
 * OUTPUT   : entry
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : prev_user, next_user are unavailable
 *            fail (1). if out of range (2). entry_tatus == DESTROYED
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_GetAccUserEntry(RADACC_UserInfo_T *entry);

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_GetAccUserEntryByKey
 *---------------------------------------------------------------------------
 * PURPOSE  : query specific user entry from user list
 * INPUT    : ifindex, user_name, client_type
 * OUTPUT   : entry
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : prev_user, next_user are unavailable
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_GetAccUserEntryByKey(UI32_T ifindex, const char *user_name, AAA_ClientType_T client_type, RADACC_UserInfo_T *entry);

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_GetAccUserEntryActiveServerIdx
 *---------------------------------------------------------------------------
 * PURPOSE  : get active server index by user index
 * INPUT    : user_index
 * OUTPUT   : active_server_index
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_GetAccUserEntryActiveServerIdx(UI16_T user_index, UI32_T *active_server_index);

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_DoesAccUserExist
 *---------------------------------------------------------------------------
 * PURPOSE  : check whether the specified accounting user exist or not
 * INPUT    : user_index
 * OUTPUT   : none
 * RETURN   : TRUE - exist, FALSE - not exist
 * NOTES    : none
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_DoesAccUserExist(UI16_T user_index);

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_CreateAccUserEntry
 *---------------------------------------------------------------------------
 * PURPOSE  : create an accounting user for request
 * INPUT    : request, sys_time
 * OUTPUT   : none
 * RETURN   : user_index - succeeded, 0 - failed
 * NOTES    : none
 *---------------------------------------------------------------------------
 */
UI16_T RADIUS_OM_CreateAccUserEntry(const AAA_AccRequest_T *request, UI32_T sys_time);

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_FreeAccUser
 *---------------------------------------------------------------------------
 * PURPOSE: recycle specific user entry from user list
 * INPUT:   user_index (1-based)
 * OUTPUT:  none.
 * RETURN:  TRUE/FALSE
 * NOTES:   none.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_FreeAccUser(UI16_T user_index);

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_SetAccUserEntrySessionStartTime
 *---------------------------------------------------------------------------
 * PURPOSE  : set the session start time by specific user index
 * INPUT    : user_index, start_time
 * OUTPUT   : none
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_SetAccUserEntrySessionStartTime(UI16_T user_index, UI32_T start_time);

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_SetAccUserEntryLastUpdateTime
 *---------------------------------------------------------------------------
 * PURPOSE  : set the last update time by specific user index
 * INPUT    : user_index, update_time
 * OUTPUT   : none
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_SetAccUserEntryLastUpdateTime(UI16_T user_index, UI32_T update_time);

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_SetAccUserEntryAAARadiusInfo
 *---------------------------------------------------------------------------
 * PURPOSE  : set aaa radius info by user index
 * INPUT    : user_index, entry
 * OUTPUT   : none
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_SetAccUserEntryAAARadiusInfo(UI16_T user_index, RADACC_AAARadiusEntryInfo_T *entry);

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_SetAccUserEntryStartResponseSent
 *---------------------------------------------------------------------------
 * PURPOSE  : set the start package response flag by specific user index
 * INPUT    : user_index, start_response_flag
 * OUTPUT   : none
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_SetAccUserEntryStartResponseSent(UI16_T user_index, BOOL_T start_response_flag);

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_SetAccUserEntryStopRetry
 *---------------------------------------------------------------------------
 * PURPOSE  : set the stop retry flag and stop time by specific user index
 * INPUT    : user_index, stop_retry, sys_time
 * OUTPUT   : none
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_SetAccUserEntryStopRetry(UI16_T user_index, BOOL_T stop_retry, UI32_T sys_time);

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_SetAccUserEntryConnectStatus
 *---------------------------------------------------------------------------
 * PURPOSE  : setup connection status by specific user index
 * INPUT    : user_index, connect_status
 * OUTPUT   : none
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_SetAccUserEntryConnectStatus(UI16_T user_index, AAA_AccConnectStatus_T connect_status);

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_IncAccUserEntryRequestCounter
 *---------------------------------------------------------------------------
 * PURPOSE  : increase/decrease request_counter by user_index
 * INPUT    : user_index, qty - negative implies decrease
 * OUTPUT   : none
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_IncAccUserEntryRequestCounter(UI16_T user_index, I16_T qty);

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_ResetAccUserEntryCallbackInfo
 *---------------------------------------------------------------------------
 * PURPOSE  : reset the call_back_func, identifier by specific user index
 * INPUT    : user_index
 * OUTPUT   : none
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_ResetAccUserEntryCallbackInfo(UI16_T user_index);

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_SetAccUserEntryTerminateCause
 *---------------------------------------------------------------------------
 * PURPOSE  : set the terminate_cause by specific user index
 * INPUT    : user_index, terminate_cause
 * OUTPUT   : none
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_SetAccUserEntryTerminateCause(UI16_T user_index, AAA_AccTerminateCause_T terminate_cause);

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_SetAccUserEntryStatisticInfo
 *---------------------------------------------------------------------------
 * PURPOSE  : set the statistic_info by specific user index
 * INPUT    : user_index, statistic_info
 * OUTPUT   : none
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_SetAccUserEntryStatisticInfo(UI16_T user_index, RADACC_UserStatisticInfo_T *statistic_info);

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_GetAccUserEntrySessionId
 *---------------------------------------------------------------------------
 * PURPOSE  : get specified user's session id
 * INPUT    : user_index, buffer_size
 * OUTPUT   : id_buffer
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_GetAccUserEntrySessionId(UI16_T user_index, UI8_T *id_buffer, UI16_T buffer_size);

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_GetAccUserEntryTimeout
 *---------------------------------------------------------------------------
 * PURPOSE  : Get timeout of the specified accounting user.
 * INPUT    : user_index    - User index
 * OUTPUT   : timeout_p     - Timeout
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : None
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_GetAccUserEntryTimeout(UI16_T user_index, UI32_T *timeout_p);

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_SetAccUserEntryTimeout
 *---------------------------------------------------------------------------
 * PURPOSE  : Set timeout of the specified accounting user.
 * INPUT    : user_index    - User index
 *            timeout       - Timeout
 * OUTPUT   : None
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : None
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_SetAccUserEntryTimeout(UI16_T user_index, UI32_T timeout);

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_GetAccUserEntryRequestIndex
 *---------------------------------------------------------------------------
 * PURPOSE  : Get request index of the specified accounting user.
 * INPUT    : user_index        - User index
 * OUTPUT   : request_index_p   - Request index
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : None
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_GetAccUserEntryRequestIndex(UI16_T user_index,
    UI32_T *request_index_p);

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_SetAccUserEntryRequestIndex
 *---------------------------------------------------------------------------
 * PURPOSE  : Set request index of the specified accounting user.
 * INPUT    : user_index    - User index
 *            request_index - Request index
 * OUTPUT   : None
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : None
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_SetAccUserEntryRequestIndex(UI16_T user_index,
    UI32_T request_index);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_SetAccUserEntryDestroyFlag
 *---------------------------------------------------------------------------
 * PURPOSE: Set the destroy flag of the specified accounting user.
 * INPUT:   user_index - User index
 *          flag            - Destroy flag
 * OUTPUT:  None
 * RETURN:  RADIUS_RETURN_SUCCESS - succeeded,
 *          RADIUS_RETURN_FAIL - failed
 * NOTE:    None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_SetAccUserEntryDestroyFlag(UI32_T user_index,
    BOOL_T flag);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_GetAccUserEntryDestroyFlag
 *---------------------------------------------------------------------------
 * PURPOSE:  Get the destroy flag of the specified accounting user.
 * INPUT:    user_index - User index
 * OUTPUT:   flag_p     - Destroy flag
 * RETURN:   RADIUS_RETURN_SUCCESS - succeeded,
 *           RADIUS_RETURN_FAIL - failed
 * NOTE:     None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_GetAccUserEntryDestroyFlag(
    UI32_T user_index, BOOL_T *flag_p);

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_ResortAccUserEntryByTimeoutOrder
 *---------------------------------------------------------------------------
 * PURPOSE  : Resort the accounting users list by ascending timeout order.
 * INPUT    : user_index    - User index
 * OUTPUT   : None
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : None
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_ResortAccUserEntryByTimeoutOrder(UI16_T user_index);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_GetRecentAccUserEntryTimeout
 *---------------------------------------------------------------------------
 * PURPOSE:  Get the smallest timeout between all accounting users.
 * INPUT:    None
 * OUTPUT    timeout_p  - Recent timeout
 * RETURN:   RADIUS_RETURN_SUCCESS - succeeded,
 *           RADIUS_RETURN_FAIL - failed
 * NOTE:     None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_GetRecentAccUserEntryTimeout(UI32_T *timeout_p);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_GetNextAccUserByTimeoutOrder
 *---------------------------------------------------------------------------
 * PURPOSE: Get the next accounting user by ascending timeout order.
 * INPUT:   user_index_p    - User index (0 for get first user)
 * OUTPUT:  user_index_p    - Next user index by ascending timeout order
 * RETURN:  RADIUS_RETURN_SUCCESS - succeeded,
 *          RADIUS_RETURN_FAIL - failed
 * NOTE:    None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_GetNextAccUserByTimeoutOrder(
    UI32_T *user_index_p);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_HasAnyAccUserEntry
 *---------------------------------------------------------------------------
 * PURPOSE: Check whether the accounting user entry.
 * INPUT:   None
 * OUTPUT:  None
 * RETURN:  TRUE/FALSE
 * NOTE:    None
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_HasAnyAccUserEntry();

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_GetNextAccRequestEntry
 *---------------------------------------------------------------------------
 * PURPOSE  : copy next accounting request to entry
 * INPUT    : request_index
 * OUTPUT   : request
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : prev_request, next_request are unavailable
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_GetNextAccRequestEntry(RADACC_RequestInfo_T *entry);

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_GetAccRequestEntry
 *---------------------------------------------------------------------------
 * PURPOSE  : copy accounting request to entry
 * INPUT    : request_index
 * OUTPUT   : request
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : prev_request, next_request are unavailable
 *            fail (1). if out of range (2). entry_tatus == DESTROYED
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_GetAccRequestEntry(RADACC_RequestInfo_T *entry);

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_CreateAccRequestEntry
 *---------------------------------------------------------------------------
 * PURPOSE  : create an accounting request for user_entry
 * INPUT    : user_entry, request_type, sys_time
 * OUTPUT   : none
 * RETURN   : request_index - succeeded, 0 - failed
 * NOTES    : none
 *---------------------------------------------------------------------------
 */
UI16_T RADIUS_OM_CreateAccRequestEntry(const RADACC_UserInfo_T *user_info, RADACC_AcctStatusType_T request_type, UI32_T sys_time);

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_FreeAccRequest
 *---------------------------------------------------------------------------
 * PURPOSE  : recycle accounting request
 * INPUT    : request_index (1-based)
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *---------------------------------------------------------------------------
 */
void RADIUS_OM_FreeAccRequest(UI16_T request_index);

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_FreeAccRequestByUserIdx
 *---------------------------------------------------------------------------
 * PURPOSE  : recycle the requests according to their user_index
 * INPUT    : user_index
 * OUTPUT   : none.
 * RETURN   : none
 * NOTES    : none.
 *---------------------------------------------------------------------------
 */
//void RADIUS_OM_FreeAccRequestByUserIdx(UI16_T user_index);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_SetAccRequestRQData
 *---------------------------------------------------------------------------
 * PURPOSE  : set the request reserved data by request index
 * INPUT    : request_index (1-based), request_data
 * OUTPUT   : none
 * RETURN   : TRUE -- succeeded, FALSE -- Failed
 * NOTE     : none
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_SetAccRequestRQData(UI16_T request_index, RADACC_AccRequestReserveredData_T *request_data);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_SetAccRequestType
 *---------------------------------------------------------------------------
 * PURPOSE  : change the request_type by request index
 * INPUT    : request_index (1-based), request_type
 * OUTPUT   : none
 * RETURN   : TRUE -- succeeded, FALSE -- Failed
 * NOTE     : none
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_SetAccRequestType(UI16_T request_index, RADACC_AcctStatusType_T request_type);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_SetAccRequestState
 *---------------------------------------------------------------------------
 * PURPOSE  : set the state by request index
 * INPUT    : request_index (1-based), state
 * OUTPUT   : none
 * RETURN   : TRUE -- succeeded, FALSE -- Failed
 * NOTE     : none
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_SetAccRequestState(UI16_T request_index, RADACC_RequestState_T state);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_SetAccRequestSockId
 *---------------------------------------------------------------------------
 * PURPOSE  : set the socket id by request index
 * INPUT    : request_index (1-based), sockid
 * OUTPUT   : none
 * RETURN   : TRUE -- succeeded, FALSE -- Failed
 * NOTE     : none
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_SetAccRequestSockId(UI16_T request_index, int sockid);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_SetAccRequestSentTime
 *---------------------------------------------------------------------------
 * PURPOSE  : set the request sent time by request index
 * INPUT    : request_index (1-based), sent_time
 * OUTPUT   : none
 * RETURN   : TRUE -- succeeded, FALSE -- Failed
 * NOTE     : none
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_SetAccRequestSentTime(UI16_T request_index, UI32_T sent_time);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_SetAccRequestServerIdx
 *---------------------------------------------------------------------------
 * PURPOSE  : set the server index by request index
 * INPUT    : request_index (1-based), server_index
 * OUTPUT   : none
 * RETURN   : TRUE -- succeeded, FALSE -- Failed
 * NOTE     : none
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_SetAccRequestServerIdx(UI16_T request_index, UI32_T server_index);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_ResetAccRequestRetryTimes
 *---------------------------------------------------------------------------
 * PURPOSE  : reset retry times = 0 by request index
 * INPUT    : request_index (1-based)
 * OUTPUT   : none
 * RETURN   : TRUE -- succeeded, FALSE -- Failed
 * NOTE     : none
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_ResetAccRequestRetryTimes(UI16_T request_index);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_IncAccRequestRetryTimes
 *---------------------------------------------------------------------------
 * PURPOSE  : increase/decrease retry times by request index
 * INPUT    : request_index (1-based), qty - negative qty implies decrease
 * OUTPUT   : none
 * RETURN   : TRUE -- succeeded, FALSE -- Failed
 * NOTE     : none
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_IncAccRequestRetryTimes(UI16_T request_index, I16_T qty);


#endif /* SYS_CPNT_RADIUS_ACCOUNTING == TRUE */

/*---------------------------------------------------------------------------
 * FUNCTION NAME - RADIUS_OM_HandleIPCReqMsg
 *---------------------------------------------------------------------------
 * PURPOSE : Handle the ipc request message for RADIUS om.
 * INPUT   : ipcmsg_p  --  input request ipc message buffer
 * OUTPUT  : ipcmsg_p  --  output response ipc message buffer
 * RETUEN  : TRUE  - need to send response.
 *           FALSE - not need to send response.
 * NOTE    : None
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_HandleIPCReqMsg(SYSFUN_Msg_T *ipcmsg_p);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_HasAnyFreeRequestEntry
 *---------------------------------------------------------------------------
 * PURPOSE: Check whether the free request entry in process queue.
 * INPUT:   None
 * OUTPUT:  None
 * RETURN:  TRUE/FALSE
 * NOTE:    None
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_HasAnyFreeRequestEntry();

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_HasAnyRequestEntry
 *---------------------------------------------------------------------------
 * PURPOSE: Check whether the request entry in process queue.
 * INPUT:   None
 * OUTPUT:  None
 * RETURN:  TRUE/FALSE
 * NOTE:    None
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_HasAnyRequestEntry();

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_AddRequest
 *---------------------------------------------------------------------------
 * PURPOSE: Add the request into process queue.
 * INPUT:   request_data_p  - Request data
 * OUTPUT:  request_index_p - Request index
 * RETURN:  RADIUS_RETURN_SUCCESS - succeeded,
 *          RADIUS_RETURN_FAIL - failed
 * NOTE:    None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_AddRequest(
    RADIUS_OM_RequestEntry_T *request_data_p, UI32_T *request_index_p);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_ResortRequestByTimeoutOrder
 *---------------------------------------------------------------------------
 * PURPOSE: Resort the request in process queue by ascending timeout order.
 * INPUT:   request_index   - Request index
 * OUTPUT:  None
 * RETURN:  RADIUS_RETURN_SUCCESS - succeeded,
 *          RADIUS_RETURN_FAIL - failed
 * NOTE:    None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_ResortRequestByTimeoutOrder(
    UI32_T request_index);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_DeleteRequest
 *---------------------------------------------------------------------------
 * PURPOSE: Ddd the request from process queue.
 * INPUT:   request_index   - Request index
 * OUTPUT:  None
 * RETURN:  RADIUS_RETURN_SUCCESS - succeeded,
 *          RADIUS_RETURN_FAIL - failed
 * NOTE:    None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_DeleteRequest(UI32_T request_index);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_GetNextCancelRequest
 *---------------------------------------------------------------------------
 * PURPOSE: Get the next request marked to be canceled.
 * INPUT:   request_index_p - Request index (0 for get first request)
 * OUTPUT:  request_index_p - Next request index by ascending timeout order
 * RETURN:  RADIUS_RETURN_SUCCESS - succeeded,
 *          RADIUS_RETURN_FAIL - failed
 * NOTE:    None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_GetNextCancelRequest(
    UI32_T *request_index_p);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_GetNextRequestByTimeoutOrder
 *---------------------------------------------------------------------------
 * PURPOSE: Get the next request by ascending timeout order.
 * INPUT:   request_index_p - Request index (0 for get first request)
 * OUTPUT:  request_index_p - Next request index by ascending timeout order
 * RETURN:  RADIUS_RETURN_SUCCESS - succeeded,
 *          RADIUS_RETURN_FAIL - failed
 * NOTE:    None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_GetNextRequestByTimeoutOrder(UI32_T
    *request_index_p);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_SetRequestDestroyFlag
 *---------------------------------------------------------------------------
 * PURPOSE: Set the destroy flag in specified requests.
 * INPUT:   request_index   - Request index
 *          flag            - Destroy flag
 * OUTPUT:  None
 * RETURN:  RADIUS_RETURN_SUCCESS - succeeded,
 *          RADIUS_RETURN_FAIL - failed
 * NOTE:    None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_SetRequestDestroyFlag(UI32_T request_index,
    BOOL_T flag);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_GetRequestDestroyFlag
 *---------------------------------------------------------------------------
 * PURPOSE:  Get the destroy flag of the specified request.
 * INPUT:    request_index  - Request index
 * OUTPUT:   flag_p         - Destroy flag
 * RETURN:   RADIUS_RETURN_SUCCESS - succeeded,
 *           RADIUS_RETURN_FAIL - failed
 * NOTE:     None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_GetRequestDestroyFlag(UI32_T request_index,
    BOOL_T *flag_p);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_GetRequestIndex
 *---------------------------------------------------------------------------
 * PURPOSE:  Get the request index of the specified request.
 * INPUT:    request_p          - The specified request
 * OUTPUT:   request_index_p    - Found request index
 * RETURN:   RADIUS_RETURN_SUCCESS - succeeded,
 *           RADIUS_RETURN_FAIL - failed
 * NOTE:     None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_GetRequestIndex(
    RADIUS_OM_RequestEntry_T *request_p, UI32_T *request_index_p);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_AddAuthReqIntoWaitingQueue
 *---------------------------------------------------------------------------
 * PURPOSE:  Add the authentication request into the waiting queue.
 * INPUT:    request_entry_p    - Request
 * OUTPUT:   None
 * RETURN:   RADIUS_RETURN_SUCCESS - succeeded,
 *           RADIUS_RETURN_FAIL - failed
 * NOTE:     None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_AddAuthReqIntoWaitingQueue(
    RADIUS_OM_RequestEntry_T *request_entry_p);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_DeleteFirstAuthReqFromWaitingQueue
 *---------------------------------------------------------------------------
 * PURPOSE:  Delete the first authentication request from the waiting queue.
 * INPUT:    None
 * OUTPUT:   request_entry_p    - First request
 * RETURN:   RADIUS_RETURN_SUCCESS - succeeded,
 *           RADIUS_RETURN_FAIL - failed
 * NOTE:     None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_DeleteFirstAuthReqFromWaitingQueue(
    RADIUS_OM_RequestEntry_T *request_entry_p);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_DeleteAuthReqFromWaitingQueue
 *---------------------------------------------------------------------------
 * PURPOSE:  Delete the specified authentication request from the waiting
 *           queue.
 * INPUT:    request_entry_p    - Specified request
 * OUTPUT:   None
 * RETURN:   RADIUS_RETURN_SUCCESS - succeeded,
 *           RADIUS_RETURN_FAIL - failed
 * NOTE:     None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_DeleteAuthReqFromWaitingQueue(
    RADIUS_OM_RequestEntry_T *request_entry_p);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_GetNextAuthReqFromWaitingQueue
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the next authentication request from the
 *           waiting queue.
 * INPUT:    request_entry_p - entry of the request
 * OUTPUT:   request_entry_p - entry of the request to the inputed request
 * RETURN:   RADIUS_RETURN_SUCCESS - succeeded,
 *           RADIUS_RETURN_FAIL - failed
 * NOTE:     None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_GetNextAuthReqFromWaitingQueue(
    RADIUS_OM_RequestEntry_T *request_entry_p);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_GetRequestState
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the state of the specific request
 * INPUT:    request_id - request id
 * OUTPUT:   state_p    - state of the request
 * RETURN:   RADIUS_RETURN_SUCCESS - succeeded,
 *           RADIUS_RETURN_FAIL - failed
 * NOTE:
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_GetRequestState(UI32_T request_id,
    RADIUS_OM_RequestState_T *state_p);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_SetRequestState
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set the state of the specific request
 * INPUT:    request_id - request id
 *           state      - state of the request
 * OUTPUT:   None
 * RETURN:   RADIUS_RETURN_SUCCESS - succeeded,
 *           RADIUS_RETURN_FAIL - failed
 * NOTE:
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_SetRequestState(UI32_T request_id,
    RADIUS_OM_RequestState_T state);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_GetRequestIdentifier
 *---------------------------------------------------------------------------
 * PURPOSE:  Get the identifier of the specified request.
 * INPUT:    request_id     - Request id
 * OUTPUT:   identifier_p   - Identifier
 * RETURN:   RADIUS_RETURN_SUCCESS - succeeded,
 *           RADIUS_RETURN_FAIL - failed
 * NOTE:
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_GetRequestIdentifier(UI32_T request_id,
    UI32_T *identifier_p);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_SetRequestIdentifier
 *---------------------------------------------------------------------------
 * PURPOSE:  Set the identifier of the specified request.
 * INPUT:    request_id - Request id
 *           identifier - Identifier
 * OUTPUT:   None
 * RETURN:   RADIUS_RETURN_SUCCESS - succeeded,
 *           RADIUS_RETURN_FAIL - failed
 * NOTE:
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_SetRequestIdentifier(UI32_T request_id,
    UI32_T identifier);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_GetRequestTimeout
 *---------------------------------------------------------------------------
 * PURPOSE:  Get the timeout of the specified request.
 * INPUT:    request_id - Request id
 * OUTPUT:   timeout_p  - Timeout
 * RETURN:   RADIUS_RETURN_SUCCESS - succeeded,
 *           RADIUS_RETURN_FAIL - failed
 * NOTE:
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_GetRequestTimeout(UI32_T request_id,
    UI32_T *timeout_p);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_SetRequestTimeout
 *---------------------------------------------------------------------------
 * PURPOSE:  Set the timeout of the specified request.
 * INPUT:    request_id - Request id
 *           timeout    - Timeout
 * OUTPUT:   None
 * RETURN:   RADIUS_RETURN_SUCCESS - succeeded,
 *           RADIUS_RETURN_FAIL - failed
 * NOTE:
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_SetRequestTimeout(UI32_T request_id,
    UI32_T timeout);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_GetRequestServerIndex
 *---------------------------------------------------------------------------
 * PURPOSE:  Get the server index of the specified request.
 * INPUT:    request_id     - Request id
 * OUTPUT:   server_index_p - Server index of the request
 * RETURN:   RADIUS_RETURN_SUCCESS - succeeded,
 *           RADIUS_RETURN_FAIL - failed
 * NOTE:     None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_GetRequestServerIndex(UI32_T request_id,
    UI32_T *server_index_p);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_SetRequestServerIndex
 *---------------------------------------------------------------------------
 * PURPOSE:  Set the server index of the specified request.
 * INPUT:    request_id     - Request id
 *           server_index   - Server index of the request
 * OUTPUT:   None
 * RETURN:   RADIUS_RETURN_SUCCESS - succeeded,
 *           RADIUS_RETURN_FAIL - failed
 * NOTE:     None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_SetRequestServerIndex(UI32_T request_id,
    UI32_T server_index);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_GetRequestVector
 *---------------------------------------------------------------------------
 * PURPOSE:  Get the vector of the specified request.
 * INPUT:    request_id - Request id
 * OUTPUT:   vector_p   - Vector
 * RETURN:   RADIUS_RETURN_SUCCESS - succeeded,
 *           RADIUS_RETURN_FAIL - failed
 * NOTE:     None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_GetRequestVector(UI32_T request_id,
    UI8_T *vector_p);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_SetRequestVector
 *---------------------------------------------------------------------------
 * PURPOSE:  Set the vector of the specified request.
 * INPUT:    request_id - Request id
 *           vector_p   - Vector
 * OUTPUT:   None
 * RETURN:   RADIUS_RETURN_SUCCESS - succeeded,
 *           RADIUS_RETURN_FAIL - failed
 * NOTE:     None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_SetRequestVector(UI32_T request_id,
    UI8_T *vector_p);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_GetRequestSecretKey
 *---------------------------------------------------------------------------
 * PURPOSE:  Set the secret key of the specified request
 * INPUT:    request_id     - Request id
 * OUTPUT    secret_key_p   - Secret key
 * RETURN:   RADIUS_RETURN_SUCCESS - succeeded,
 *           RADIUS_RETURN_FAIL - failed
 * NOTE:     None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_GetRequestSecretKey(UI32_T request_id,
    char *secret_key_p);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_SetRequestSecretKey
 *---------------------------------------------------------------------------
 * PURPOSE:  Set the secret key of the specified request
 * INPUT:    request_id     - Request id
 *           secret_key_p   - Secret key
 * OUTPUT:   None
 * RETURN:   RADIUS_RETURN_SUCCESS - succeeded,
 *           RADIUS_RETURN_FAIL - failed
 * NOTE:     None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_SetRequestSecretKey(UI32_T request_id,
    char *secret_key_p);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_GetRequestLastSentTime
 *---------------------------------------------------------------------------
 * PURPOSE:  Get the last sent time of the specified request.
 * INPUT:    request_id         - Request id
 * OUTPUT:   last_sent_time_p   - Last sent time of the request
 * RETURN:   RADIUS_RETURN_SUCCESS - succeeded,
 *           RADIUS_RETURN_FAIL - failed
 * NOTE:     None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_GetRequestLastSentTime(UI32_T request_id,
    UI32_T *last_sent_time_p);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_SetRequestLastSentTime
 *---------------------------------------------------------------------------
 * PURPOSE:  Set the last sent time of the specified request.
 * INPUT:    request_id     - Request id
 *           last_sent_time - Last sent time of the request
 * OUTPUT:   None
 * RETURN:   RADIUS_RETURN_SUCCESS - succeeded,
 *           RADIUS_RETURN_FAIL - failed
 * NOTE:     None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_SetRequestLastSentTime(UI32_T request_id,
    UI32_T last_sent_time);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_GetRequestServerArray
 *---------------------------------------------------------------------------
 * PURPOSE:  Get the servers of the specified request.
 * INPUT:    request_i]ndex - Request index
 * OUTPUT:   server_array_p - Server array
 * RETURN:   RADIUS_RETURN_SUCCESS - succeeded,
 *           RADIUS_RETURN_FAIL - failed
 * NOTE:     None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_GetRequestServerArray(UI32_T request_index,
    RADIUS_OM_ServerArray_T *server_array_p);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_SetRequestServerArray
 *---------------------------------------------------------------------------
 * PURPOSE:  Set the servers of the specified request.
 * INPUT:    request_i]ndex - Request index
 *           server_array_p - Server array
 * OUTPUT:   None
 * RETURN:   RADIUS_RETURN_SUCCESS - succeeded,
 *           RADIUS_RETURN_FAIL - failed
 * NOTE:     None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_SetRequestServerArray(UI32_T request_index,
    RADIUS_OM_ServerArray_T *server_array_p);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_GetRequestPendingRequestCounterFlag
 *---------------------------------------------------------------------------
 * PURPOSE:  Get the flag of pending request counter of the specified
 *           request.
 * INPUT:    request_id - Request id
 * OUTPUT:   flag_p     - Flag
 * RETURN:   RADIUS_RETURN_SUCCESS - succeeded,
 *           RADIUS_RETURN_FAIL - failed
 * NOTE:     None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_GetRequestPendingRequestCounterFlag(
    UI32_T request_id, BOOL_T *flag_p);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_SetRequestPendingRequestCounterFlag
 *---------------------------------------------------------------------------
 * PURPOSE:  Set the flag of pending request counter of the specified
 *           request.
 * INPUT:    request_id - Request id
 *           flag       - Flag
 * OUTPUT:   None
 * RETURN:   RADIUS_RETURN_SUCCESS - succeeded,
 *           RADIUS_RETURN_FAIL - failed
 * NOTE:     None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_SetRequestPendingRequestCounterFlag(
    UI32_T request_id, BOOL_T flag);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_GetRequestResultData
 *---------------------------------------------------------------------------
 * PURPOSE: Get the result data of the specified request.
 * INPUT:   request_id      - Request id
 * OUTPUT:  result_data_p   - Result data
 * RETURN:  RADIUS_RETURN_SUCCESS - succeeded,
 *          RADIUS_RETURN_FAIL - failed
 * NOTE:    None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_GetRequestResultData(UI32_T request_id,
    RADIUS_OM_RequestResult_T *result_data_p);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_GetRequestResponsePacket
 *---------------------------------------------------------------------------
 * PURPOSE: Get the response packet of the specified request.
 * INPUT:   request_id              - Request id
 * OUTPUT:  response_packet_pp      - Pointer of response packet
 *          response_packet_len_p   - Length of response packet
 * RETURN:  RADIUS_RETURN_SUCCESS - succeeded,
 *          RADIUS_RETURN_FAIL - failed
 * NOTE:    None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_GetRequestResponsePacket(UI32_T request_id,
    UI8_T **response_packet_pp, UI32_T *response_packet_len_p);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_SetRequestResponsePacket
 *---------------------------------------------------------------------------
 * PURPOSE: Set the response packet of the specified request.
 * INPUT:   request_id          - Request id
 *          response_packet_p   - Pointer of response packet
 *          response_packet_len - Length of response packet
 * OUTPUT:  None
 * RETURN:  RADIUS_RETURN_SUCCESS - succeeded,
 *          RADIUS_RETURN_FAIL - failed
 * NOTE:    None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_SetRequestResponsePacket(UI32_T request_id,
    UI8_T *response_packet_p, UI32_T response_packet_len);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_SetRequestResultData
 *---------------------------------------------------------------------------
 * PURPOSE: Set the result data of the specified request.
 * INPUT:   request_id      - Request id
 *          result_data_p   - Result data
 * OUTPUT:  None
 * RETURN:  RADIUS_RETURN_SUCCESS - succeeded,
 *          RADIUS_RETURN_FAIL - failed
 * NOTE:    None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_SetRequestResultData(UI32_T request_id,
    RADIUS_OM_RequestResult_T *result_data_p);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_SetRequestResultServerIp
 *---------------------------------------------------------------------------
 * PURPOSE: Set the server IP address of the last request end of the process.
 * INPUT:   request_id  - Request id
 *          server_ip   - Server IP address
 * OUTPUT:  None
 * RETURN:  RADIUS_RETURN_SUCCESS - succeeded,
 *          RADIUS_RETURN_FAIL - failed
 * NOTE:    None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_SetRequestResultServerIp(UI32_T request_id,
    UI32_T server_ip);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_GetRecentRequestTimeout
 *---------------------------------------------------------------------------
 * PURPOSE:  Get the recent request timeout (closest to current time)
 *           between all processing requests.
 * INPUT:    None
 * OUTPUT    timeout_p  - Recent request timeout
 * RETURN:   RADIUS_RETURN_SUCCESS - succeeded,
 *           RADIUS_RETURN_FAIL - failed
 * NOTE:     None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_GetRecentRequestTimeout(UI32_T *timeout_p);

/*---------------------------------------------------------------------------
* ROUTINE NAME - RADIUS_OM_GetRequestType
*---------------------------------------------------------------------------
* PURPOSE:  Get the request type of the specified request.
* INPUT:    request_id - Request id
* OUTPUT:   type_p     - Request type
* RETURN:   RADIUS_RETURN_SUCCESS - succeeded,
*           RADIUS_RETURN_FAIL - failed
* NOTE:     None
*---------------------------------------------------------------------------
*/
RADIUS_ReturnValue_T RADIUS_OM_GetRequestType(UI32_T request_id,
    RADIUS_OM_RadiusRequestType_T *type_p);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_GetRequestData
 *---------------------------------------------------------------------------
 * PURPOSE:  Get the data of the specified request.
 * INPUT:    request_id - Request id
 * OUTPUT:   data_p     - Request data
 * RETURN:   RADIUS_RETURN_SUCCESS - succeeded,
 *           RADIUS_RETURN_FAIL - failed
 * NOTE:     None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_GetRequestData(UI32_T request_id,
    RADIUS_OM_RequestEntry_T *data_p);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_GetRequestRetryTimes
 *---------------------------------------------------------------------------
 * PURPOSE:  Get the retry times of the specified request.
 * INPUT:    request_id     - Request id
 * OUTPUT:   retry_times_p  - Retry times of the request
 * RETURN:   RADIUS_RETURN_SUCCESS - succeeded,
 *           RADIUS_RETURN_FAIL - failed
 * NOTE:     None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_GetRequestRetryTimes(UI32_T request_id,
    UI32_T *retry_times_p);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_SetRequestRetryTimes
 *---------------------------------------------------------------------------
 * PURPOSE:  Set the retry times of the specified request.
 * INPUT:    request_id     - Request id
 *           retry_times    - Retry times of the request
 * OUTPUT:   None
 * RETURN:   RADIUS_RETURN_SUCCESS - succeeded,
 *           RADIUS_RETURN_FAIL - failed
 * NOTE:     None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_SetRequestRetryTimes(UI32_T request_id,
    UI32_T retry_times);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_SetAuthReqIdQueueSocketId
 *---------------------------------------------------------------------------
 * PURPOSE: Set the socket ID in the authentication request identifier queue.
 * INPUT:   socket_id   - Socket ID.
 * OUTPUT:  None
 * RETURN:  RADIUS_RETURN_SUCCESS - succeeded,
 *          RADIUS_RETURN_FAIL - failed
 * NOTE:    None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_SetAuthReqIdQueueSocketId(int socket_id);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_GetAuthReqIdQueueSocketId
 *---------------------------------------------------------------------------
 * PURPOSE: Get the socket ID in the authentication request identifier queue.
 * INPUT:   None
 * OUTPUT:  socket_id_p - Socket ID.
 * RETURN:  RADIUS_RETURN_SUCCESS - succeeded,
 *          RADIUS_RETURN_FAIL - failed
 * NOTE:    None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_GetAuthReqIdQueueSocketId(int *socket_id_p);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_AddRequestIntoAuthReqIdQueue
 *---------------------------------------------------------------------------
 * PURPOSE: Add a new request into the authentication request identifier
 *          queue.
 * INPUT:   request_index   - Request index
 * OUTPUT:  id_p            - Identifier
 * RETURN:  RADIUS_RETURN_SUCCESS - succeeded,
 *          RADIUS_RETURN_FAIL - failed
 * NOTE:    None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_AddRequestIntoAuthReqIdQueue(
    UI32_T request_index, UI32_T *id_p);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_DeleteRequestFromAuthReqIdQueue
 *---------------------------------------------------------------------------
 * PURPOSE: Delete the request from the authentication request identifier
 *          queue.
 * INPUT:   id  - Identifier
 * OUTPUT:  None
 * RETURN:  RADIUS_RETURN_SUCCESS - succeeded,
 *          RADIUS_RETURN_FAIL - failed
 * NOTE:    None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_DeleteRequestFromAuthReqIdQueue(UI32_T id);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_HasAnyRequestInAuthReqIdQueue
 *---------------------------------------------------------------------------
 * PURPOSE: Check whether there is any request in authentication request
 *          identifier queue.
 * INPUT:   None
 * OUTPUT:  None
 * RETURN:  TRUE/FALSE
 * NOTE:    None
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_HasAnyRequestInAuthReqIdQueue();

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_GetRequestByIdFromAuthReqIdQueue
 *---------------------------------------------------------------------------
 * PURPOSE: Get the belonging request index by identifier from
 *          authentication request ID queue.
 * INPUT:   id              - Identifier
 * OUTPUT:  request_index_p - Request index
 * RETURN:  RADIUS_RETURN_SUCCESS - succeeded,
 *          RADIUS_RETURN_FAIL - failed
 * NOTE:    None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_GetRequestByIdFromAuthReqIdQueue(UI32_T id,
    UI32_T *request_index_p);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_SetAcctReqIdQueueSocketId
 *---------------------------------------------------------------------------
 * PURPOSE: Set the socket ID in the accounting request identifier queue.
 * INPUT:   socket_id   - Socket ID.
 * OUTPUT:  None
 * RETURN:  RADIUS_RETURN_SUCCESS - succeeded,
 *          RADIUS_RETURN_FAIL - failed
 * NOTE:    None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_SetAcctReqIdQueueSocketId(int socket_id);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_GetAcctReqIdQueueSocketId
 *---------------------------------------------------------------------------
 * PURPOSE: Get the socket ID in the accounting request identifier queue.
 * INPUT:   None
 * OUTPUT:  socket_id_p - Socket ID.
 * RETURN:  RADIUS_RETURN_SUCCESS - succeeded,
 *          RADIUS_RETURN_FAIL - failed
 * NOTE:    None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_GetAcctReqIdQueueSocketId(int *socket_id_p);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_AddRequestIntoAcctReqIdQueue
 *---------------------------------------------------------------------------
 * PURPOSE: Add a new request into the accounting request identifier
 *          queue.
 * INPUT:   request_index   - Request index
 * OUTPUT:  id_p            - Identifier
 * RETURN:  RADIUS_RETURN_SUCCESS - succeeded,
 *          RADIUS_RETURN_FAIL - failed
 * NOTE:    None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_AddRequestIntoAcctReqIdQueue(
    UI32_T request_index, UI32_T *id_p);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_DeleteRequestFromAcctReqIdQueue
 *---------------------------------------------------------------------------
 * PURPOSE: Delete the request from the accounting request identifier
 *          queue.
 * INPUT:   id  - Identifier
 * OUTPUT:  None
 * RETURN:  RADIUS_RETURN_SUCCESS - succeeded,
 *          RADIUS_RETURN_FAIL - failed
 * NOTE:    None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_DeleteRequestFromAcctReqIdQueue(UI32_T id);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_HasAnyRequestInAcctReqIdQueue
 *---------------------------------------------------------------------------
 * PURPOSE: Check whether there is any request in accounting request
 *          identifier queue.
 * INPUT:   None
 * OUTPUT:  None
 * RETURN:  TRUE/FALSE
 * NOTE:    None
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_HasAnyRequestInAcctReqIdQueue();

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_GetRequestByIdFromAcctReqIdQueue
 *---------------------------------------------------------------------------
 * PURPOSE: Get the belonging request index by identifier from
 *          accounting request ID queue.
 * INPUT:   id              - Identifier
 * OUTPUT:  request_index_p - Request index
 * RETURN:  RADIUS_RETURN_SUCCESS - succeeded,
 *          RADIUS_RETURN_FAIL - failed
 * NOTE:    None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_GetRequestByIdFromAcctReqIdQueue(UI32_T id,
    UI32_T *request_index_p);

#endif /* RADIUS_OM_H */

