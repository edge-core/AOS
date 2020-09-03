/* Project Name: New Feature
 * File_Name : tacacs_mgr.h
 * Purpose     : TACACS initiation and TACACS task creation
 *
 * 2002/05/07    : Kevin Cheng  Create this file
 *
 * Copyright(C)      Accton Corporation, 2001, 2002
 *
 * Note    : Designed for new platform (Mercury)
 */
#ifndef TACACS_MGR_H
#define TACACS_MGR_H

//#include "tacacs_om.h"
#include "sys_dflt.h"
#include "tacacs_type.h"
#include "security_backdoor.h"
#include "sysfun.h"

/* NAMING CONSTANT DECLARATIONS
 */
/* support backdoor functions or not
 */
#define TACACS_PLUS_SUPPORT_ACCTON_BACKDOOR     (TRUE && SECURITY_SUPPORT_ACCTON_BACKDOOR)

enum
{
    TACACS_MGR,
    TACACS_AUTHEN_C,
    TACACS_AUTHOR_C,
    TACACS_ACCT_C,
    TACACS_ACCT,
    TACACS_PACKET,
};

#ifndef TACACS_NEWLINE
#define TACACS_NEWLINE                                      "\r\n"
#endif

#define TACACS_IS_DEBUG_ERROR_ON(flag)                      (flag)

#define TACACS_PRINT_HEADER()                               \
    {                                                       \
        BACKDOOR_MGR_Printf("[%s:%d]" TACACS_NEWLINE,       \
            __FUNCTION__, __LINE__);                        \
    }

#define TACACS_PRINT(fmt, ...)                              \
    {                                                       \
        BACKDOOR_MGR_Printf(fmt, ##__VA_ARGS__);            \
    }

#define TACACS_LOG(flag, fmt,...)                           \
    {                                                       \
        if (TACACS_IS_DEBUG_ERROR_ON(flag))                 \
        {                                                   \
            TACACS_PRINT_HEADER();                          \
            TACACS_PRINT(fmt TACACS_NEWLINE, ##__VA_ARGS__);\
            fflush(stdout);                                  \
        }                                                   \
    }

#define TACACS_AUTHENTICATION_CONNECT_FAIL	-1
#define TACACS_AUTHENTICATION_SUCCESS		1
#define TACACS_AUTHENTICATION_FAIL              2

typedef struct
{
    UI16_T  user_index;
    UI8_T   *msg_data_p;
    UI32_T  reserve1[2];
} TACACS_Acc_Queue_T;

enum TACACS_TASK_EVENT_MASK_E
{
    TACACS_TASK_EVENT_NONE              = 0x0000L,
    TACACS_TASK_EVENT_TIMER             = 0x0001L,
    TACACS_TASK_EVENT_ENTER_TRANSITION  = 0x0002L,
    TACACS_TASK_EVENT_ACC_REQ           = 0x0004L,
    TACACS_TASK_EVENT_ACC_START         = 0x0008L,
    TACACS_TASK_EVENT_ACC_STOP          = 0x0010L,
    TACACS_TASK_EVENT_ALL               = 0xFFFFL
};

typedef enum TACACS_Authentic_E
{
    TACACS_AUTHEN_BY_UNKNOWN,
    TACACS_AUTHEN_BY_RADIUS,
    TACACS_AUTHEN_BY_TACACS_PLUS,
    TACACS_AUTHEN_BY_LOCAL,
} TACACS_Authentic_T;

typedef enum TACACS_AuthorRequestReturnType_E
{
    TACACS_AuthorRequest_START, /* initial value when request */
    TACACS_AuthorRequest_SUCCEEDED,
    TACACS_AuthorRequest_FAILED,
    TACACS_AuthorRequest_TIMEOUT,
    TACACS_AuthorRequest_CONFIG_IMCOMPLETE,
    TACACS_AuthorRequest_SUCCEEDED_WITH_NO_PRIV,
} TACACS_AuthorRequestReturnType_T;

typedef struct TACACS_AuthorRequest_S
{
    UI32_T                  ifindex;        /* if client_type == AAA_AUTHOR_EXEC, ifindex implies console or telnet's session id */
    TACACS_SessType_T       sess_type;
    L_INET_AddrIp_T         rem_ip_addr;
    char                    user_name[SYS_ADPT_MAX_USER_NAME_LEN + 1];
    char                    password[SYS_ADPT_MAX_PASSWORD_LEN + 1];
    UI32_T                  server_ip;
    UI32_T                  server_port;
    UI32_T                  retransmit;
    UI32_T                  timeout;
    UI8_T                   secret[TACACS_AUTH_KEY_MAX_LEN + 1];
    UI32_T                  current_privilege;
    TACACS_Authentic_T      authen_by_whom;

#if (SYS_CPNT_TACACS_PLUS_AUTHORIZATION_COMMAND == TRUE)
    char                    command[SYS_ADPT_CLI_MAX_BUFSIZE + 1];
#endif

} TACACS_AuthorRequest_T;


#define TACACS_MGR_ASYNC_MAX_COOKIE_SIZE 256

typedef struct
{
    TACACS_SessType_T   sess_type;
    union
    {
        UI32_T          ifindex;        /* if client_type == AAA_AUTHOR_EXEC, ifindex implies console or telnet's session id */
        UI32_T          sess_id;
    };
    L_INET_AddrIp_T     rem_ip_addr;
    char                user_name[SYS_ADPT_MAX_USER_NAME_LEN + 1];
    char                password[SYS_ADPT_MAX_PASSWORD_LEN + 1];
    UI32_T              current_privilege;
    TACACS_Authentic_T  authen_by_whom;
    UI32_T              cookie_size;
    UI8_T               cookie[TACACS_MGR_ASYNC_MAX_COOKIE_SIZE];
} TACACS_AsyncAuthorRequest_T;

typedef struct TACACS_AuthorReply_S
{
    UI32_T                            new_privilege;
    TACACS_AuthorRequestReturnType_T  return_type;
} TACACS_AuthorReply_T;

/* definitions of command which will be used in ipc message
 */
enum
{
    TACACS_MGR_IPCCMD_SET_SERVER_PORT,
    TACACS_MGR_IPCCMD_SET_SERVER_RETRANSMIT,
    TACACS_MGR_IPCCMD_SET_SERVER_TIMEOUT,
    TACACS_MGR_IPCCMD_SET_SERVER_SECRET,
    TACACS_MGR_IPCCMD_SET_SERVER_IP,
    TACACS_MGR_IPCCMD_SET_SERVER_HOST,
    TACACS_MGR_IPCCMD_SETSERVERHOSTBYIPADDRESS,
    TACACS_MGR_IPCCMD_DESTROY_SERVER_HOST_BY_INDEX,
    TACACS_MGR_IPCCMD_DESTROY_SERVER_HOST_BY_IP_ADDRESS,
    TACACS_MGR_IPCCMD_AUTHOR_CHECK,

    TACACS_MGR_IPCCMD_FOLLOWISASYNCHRONISMIPC,

    TACACS_MGR_IPCCMD_ASYNC_LOGIN_AUTH,
    TACACS_MGR_IPCCMD_ASYNC_AUTHEN_ENABLE,
};

/*use to the definition of IPC message buffer*/
typedef struct
{
    union
    {
        UI32_T cmd;          /*cmd fnction id*/
        BOOL_T result_bool;  /*respond bool return*/
        UI32_T result_ui32;  /*respond ui32 return*/
        UI32_T result_i32;  /*respond i32 return*/
    }type;

    union
    {

        BOOL_T bool_v;
        UI8_T  ui8_v;
        I8_T   i8_v;
        UI32_T ui32_v;
        UI16_T ui16_v;
        I32_T i32_v;
        I16_T i16_v;
        UI8_T ip4_v[4];
        UI8_T serversecret[TACACS_AUTH_KEY_MAX_LEN+1];

        struct
        {
            UI32_T index;
            TACACS_Server_Host_T server_host;
        }index_serverhost;

        TACACS_Server_Host_T server_host;

        TACACS_AuthorRequest_T author_request;

        struct
        {
            TACACS_AuthorRequest_T request;
            TACACS_AuthorReply_T reply;
        }request_reply;

        TACACS_AsyncAuthorRequest_T async_author_request;

    } data; /* contains the supplemntal data for the corresponding cmd */
}TACACS_MGR_IPCMsg_T;

#define TACACS_MGR_MSGBUF_TYPE_SIZE     sizeof(((TACACS_MGR_IPCMsg_T *)0)->type)

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - TACACS_MGR_SetConfigSettingToDefault
 * ---------------------------------------------------------------------
 * PURPOSE: This function will set all the config values which are belongs
 *          to TACACS_MGR to default value.
 * INPUT : None
 * OUTPUT: None
 * RETURN: TRUE/FALSE
 * NOTES : This is local function and only will be implemented by
 *        TACACS_MGR_EnterMasterMode()
 * ---------------------------------------------------------------------
 */
BOOL_T TACACS_MGR_SetConfigSettingToDefault();

/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_MGR_CurrentOperationMode
 *---------------------------------------------------------------------------
 * PURPOSE:  This function return the current opearion mode of TACACS 's task
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   TACACS_operation_mode
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
 SYS_TYPE_Stacking_Mode_T TACACS_MGR_CurrentOperationMode();

/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_MGR_EnterMasterMode
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will make the TACACS client enter the master mode.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
 void TACACS_MGR_EnterMasterMode();

/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_MGR_EnterSlaveMode
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will make the TACACS client enter the Slave mode.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
void TACACS_MGR_EnterSlaveMode();

/* FUNCTION	NAME : TACACS_MGR_SetTransitionMode
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
void TACACS_MGR_SetTransitionMode(void);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_MGR_EnterTransition Mode
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will make the TACACS client enter the transition mode.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
void TACACS_MGR_EnterTransitionMode();

BOOL_T TACACS_MGR_Initiate_System_Resources(void);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_MGR_Create_InterCSC_Relation
 *---------------------------------------------------------------------------
 * PURPOSE:  This function initializes all function pointer registration operations.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
void TACACS_MGR_Create_InterCSC_Relation(void);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_MGR_Set_Server_Port
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set the UDP port number of the remote TACACS server
 * INPUT:    Prot number
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
 BOOL_T TACACS_MGR_Set_Server_Port(UI32_T serverport);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_MGR_SetServerRetransmit
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set the global retransmit of the remote TACACS server
 * INPUT  :  retransmit
 * OUTPUT :  None.
 * RETURN :  TRUE/FALSE
 * NOTE   :  None.
 *---------------------------------------------------------------------------
 */
BOOL_T TACACS_MGR_SetServerRetransmit(UI32_T retransmit);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_MGR_SetServerTimeout
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set the global timeout of the remote TACACS server
 * INPUT  :  Prot number
 * OUTPUT :  None.
 * RETURN :  TRUE/FALSE
 * NOTE   :  None.
 *---------------------------------------------------------------------------
 */
BOOL_T TACACS_MGR_SetServerTimeout(UI32_T timeout);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_MGR_Set_Server_Secret
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set the shared secret text string used between
 *           the TACACS client and server.
 * INPUT:    secret text string
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
 BOOL_T TACACS_MGR_Set_Server_Secret(UI8_T *serversecret);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_MGR_Set_Server_IP
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set the IP address of the remote TACACS server
 * INPUT:    TACACS server IP address
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
 BOOL_T TACACS_MGR_Set_Server_IP(UI32_T serverip);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_Auth_Check
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will call TACACS client to do authentication
 * INPUT:    TACACS Authentication username  and password
 * RETURN:   TACACS Authentication result
* 	     TACACS_AUTHENTICATION_CONNECT_FAIL		-1 = connect error
 *           TACACS_AUTHENTICATION_SUCCESS		 1 = Authentication PASS
 *           TACACS_AUTHENTICATION_FAIL		 	 2 = Authentication FAIL
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
 I32_T TACACS_MGR_Auth_Check(
    const char *username,
    const char *password,
    TACACS_SessType_T sess_type,
    UI32_T sess_id,
    const L_INET_AddrIp_T *rem_addr,
    UI32_T *privilege
);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_Auth_Enable_Requests
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will call TACACS client to do enable request that
 *           change privilege level to 15
 * INPUT:    password for TACACS Authentication enable reauest
 * RETURN:   TACACS Authentication result
* 	     TACACS_AUTHENTICATION_CONNECT_FAIL		-1 = connect error
 *           TACACS_AUTHENTICATION_SUCCESS		 1 = Authentication PASS
 *           TACACS_AUTHENTICATION_FAIL		 	 2 = Authentication FAIL
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
 I32_T TACACS_MGR_Auth_Enable_Requests(
    const char *username,
    const char *password,
    TACACS_SessType_T sess_type,
    UI32_T sess_id,
    const L_INET_AddrIp_T *rem_addr,
    UI32_T *privilege
);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_MGR_Set_Server_Host
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set the TACACS server host
 * INPUT:    server_index (1-based), server_host
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE
 * NOTE:    server_ip;
 *		    server_port (1-65535)  - set 0 will use the global TACACS configuration
 *       	timeout     (1-65535)  - set 0 will use the global TACACS configuration
 *       	retransmit  (1-65535)  - set 0 will use the global TACACS configuration
 *        	secret      (length < TACACS_MAX_SECRET_LENGTH)  - set NULL will use the global TACACS configuration
 *---------------------------------------------------------------------------
 */
BOOL_T TACACS_MGR_Set_Server_Host(UI32_T server_index,TACACS_Server_Host_T *server_host);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_MGR_SetServerHostByIpAddress
 *-------------------------------------------------------------------------
 * PURPOSE  : setup server host by server_ip
 * INPUT    : server_ip, server_port, timeout, retransmit, secret, acct_port
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : if server_port, timeout, retransmit, acct_port == 0 then use global value
 *            if strlen(secret) == 0 then use global value
 *            if specified ip doesn't exist, then create it. or modify it
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_MGR_SetServerHostByIpAddress(TACACS_Server_Host_T *server_host);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_MGR_Destroy_Server_Host_By_Index
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will destroy the TACACS server host
 * INPUT:    server_index (1-based)
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
BOOL_T TACACS_MGR_Destroy_Server_Host_By_Index(UI32_T server_index);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_MGR_Destroy_Server_Host_By_Ip_Address
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will destroy the TACACS server host
 * INPUT:    server_ip
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
BOOL_T TACACS_MGR_Destroy_Server_Host_By_Ip_Address(UI32_T server_ip);
#if (SYS_CPNT_TACACS_PLUS_ACCOUNTING == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_MGR_GetRunningServerAcctPort
 *-------------------------------------------------------------------------
 * PURPOSE  : This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *            the non-default TACACS accounting port is successfully retrieved.
 *            Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT    : none
 * OUTPUT   : acct_port
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    : 1. This function shall only be invoked by CLI to save the
 *               "running configuration" to local or remote files.
 *            2. Since only non-default configuration will be saved, this
 *               function shall return non-default value.
 *-------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T TACACS_MGR_GetRunningServerAcctPort(UI32_T *acct_port);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_MGR_GetServerAcctPort
 *-------------------------------------------------------------------------
 * PURPOSE  : get global server accounting port
 * INPUT    : none
 * OUTPUT   : acct_port
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_MGR_GetServerAcctPort(UI32_T *acct_port);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_MGR_SetServerAcctPort
 *-------------------------------------------------------------------------
 * PURPOSE  : set global server accounting port
 * INPUT    : acct_port
 * OUTPUT   : none
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_MGR_SetServerAcctPort(UI32_T acct_port);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_MGR_GetAccUserEntryQty
 *-------------------------------------------------------------------------
 * PURPOSE  : get the number of user entries
 * INPUT    : none.
 * OUTPUT   : qty.
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : exclude "dead" user which connect_status ==
 *            AAA_ACC_CNET_DORMANT, AAA_ACC_CNET_IDLE, AAA_ACC_CNET_FAILED
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_MGR_GetAccUserEntryQty(UI32_T *qty);
/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_MGR_GetNextAccUserEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : get the next user entry by index.
 * INPUT    : entry->user_index
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : exclude "dead" user which connect_status ==
 *            AAA_ACC_CNET_DORMANT, AAA_ACC_CNET_IDLE, AAA_ACC_CNET_FAILED
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_MGR_GetNextAccUserEntry(TPACC_UserInfoInterface_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_MGR_GetNextAccUserEntryFilterByName
 *-------------------------------------------------------------------------
 * PURPOSE  : get the next user with specified name by index.
 * INPUT    : entry->user_index, entry->user_name
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : exclude "dead" user which connect_status ==
 *            AAA_ACC_CNET_DORMANT, AAA_ACC_CNET_IDLE, AAA_ACC_CNET_FAILED
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_MGR_GetNextAccUserEntryFilterByName(TPACC_UserInfoInterface_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_MGR_ChangeAccUserServer
 *-------------------------------------------------------------------------
 * PURPOSE  : assign another server to user
 * INPUT    : ip_address
 * OUTPUT   : none
 * RETURN   : TACACS_AccObjectProcessResult_T
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
TACACS_AccObjectProcessResult_T TACACS_MGR_ChangeAccUserServer(TACACS_AccUserInfo_T *user_info);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_MGR_GetMainTaskId
 *-------------------------------------------------------------------------
 * PURPOSE  : get tacacs main task id
 * INPUT    : task_id     --  buffer of task id
 * OUTPUT   : task_id     --  task id
 * RETURN   : TRUE - success, FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_MGR_GetMainTaskId(UI32_T *task_id);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_MGR_SetMainTaskId
 *-------------------------------------------------------------------------
 * PURPOSE  : set tacacs main task id
 * INPUT    : task_id     --  task id
 * OUTPUT   : none
 * RETURN   : TRUE - success, FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_MGR_SetMainTaskId(UI32_T task_id);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_MGR_GetAccMsgQId
 *-------------------------------------------------------------------------
 * PURPOSE  : get tacacs acc message queue id
 * INPUT    : queue_id     --  buffer of queue id
 * OUTPUT   : queue_id     --  queue id
 * RETURN   : TRUE - success, FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_MGR_GetAccMsgQId(UI32_T *queue_id);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_MGR_SetAccUserEntryTaskId
 *-------------------------------------------------------------------------
 * PURPOSE  : set task id of acc user
 * INPUT    : user_index  --  user index
 *            task_id     --  task id
 * OUTPUT   : none
 * RETURN   : TRUE - success, FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_MGR_SetAccUserEntryTaskId(UI16_T user_index, UI32_T task_id);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_MGR_SetAccUserEntryLastUpdateTime
 *-------------------------------------------------------------------------
 * PURPOSE  : set the last update time by specific user index
 * INPUT    : user_index, update_time
 * OUTPUT   : none
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_MGR_SetAccUserEntryLastUpdateTime(UI16_T user_index, UI32_T update_time);

#endif /* SYS_CPNT_TACACS_PLUS_ACCOUNTING == TRUE */


/* FUNCTION NAME - TACACS_MGR_HandleHotInsertion
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
void TACACS_MGR_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default);



/* FUNCTION NAME - TACACS_MGR_HandleHotRemoval
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
void TACACS_MGR_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port);

 /*------------------------------------------------------------------------------
  * ROUTINE NAME : TACACS_MGR_HandleIPCReqMsg
  *------------------------------------------------------------------------------
  * PURPOSE:
  *    Handle the ipc request message for cluster mgr.
  * INPUT:
  *    msg_p         --  the request ipc message buffer
  *    ipcmsgq_p     --  The handle of ipc message queue. The response message
  *                      will be sent through this handle.
  *
  * OUTPUT:
  *    None.
  *
  * RETURN:
  *    TRUE  --  There is a response need to send.
  *    FALSE --  No response need to send.
  * NOTES:
  *    None.
  *------------------------------------------------------------------------------
  */
BOOL_T TACACS_MGR_HandleIPCReqMsg(SYSFUN_Msg_T* msg_p);

 /*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_MGR_CreateTaskMsgQ
 *---------------------------------------------------------------------------
 * PURPOSE:  This function create msgQ for task
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE.
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
BOOL_T TACACS_MGR_CreateTaskMsgQ(void);

#if (SYS_CPNT_AUTHORIZATION == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_MGR_Author_Check
 *-------------------------------------------------------------------------
 * PURPOSE  : do authorization function
 * INPUT    : *request -- input of authorization request
 * OUTPUT   : *reply -- output of authorization request
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_MGR_Author_Check(TACACS_AuthorRequest_T *request,TACACS_AuthorReply_T *reply);
#endif /* SYS_CPNT_AUTHORIZATION == TRUE */

/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_MGR_GetDebugFlag
 *---------------------------------------------------------------------------
 * PURPOSE:  Get debug flag
 * INPUT:    file_type
 * OUTPUT:   None.
 * RETURN:   If succeeded, debug flag is returned. Otherwise, 0 is returned.
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
BOOL_T TACACS_MGR_GetDebugFlag(UI32_T file_type);

#endif /* End of TACACS_MGR_H */

