/* Project Name: New Feature
 * File_Name : Radius_mgr.h
 * Purpose     : Radius initiation and Radius task creation
 *
 * 2001/11/22    : Kevin Cheng  Create this file
 *
 * Copyright(C)      Accton Corporation, 2001, 2002
 *
 * Note    : Designed for new platform (Mercury)
 */
#ifndef RADIUS_MGR_H
#define RADIUS_MGR_H

/* INCLUDE FILE DECLARATIONS
 */
#include <stddef.h>
#include "radius_om.h"
#include "sys_type.h"
#include "sys_bld.h"
#include "sysfun.h"
#include "security_backdoor.h"
#include "radius_type.h"
#include "leaf_es3626a.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define RADIUS_SUPPORT_ACCTON_BACKDOOR      (SYS_CPNT_RADIUS && SECURITY_SUPPORT_ACCTON_BACKDOOR) /* support backdoor functions */

#define RADIUS_AUTHENTICATION_BADRESP   BADRESP_RC
#define RADIUS_AUTHENTICATION_SUCCESS   OK_RC
#define RADIUS_AUTHENTICATION_TIMEOUT   TIMEOUT_RC


/* The key to get igmpsnp mgr msgq.
 */
#define RADIUS_MGR_IPCMSG_KEY    SYS_BLD_AUTH_PROTOCOL_GROUP_IPCMSGQ_KEY

/* These value will be use by mgr handler to set msg.type.result
 *   RADIUS_MGR_IPC_RESULT_OK   - only use when API has no return value
 *                                and mgr deal this request.
 *   RADIUS_MGR_IPC_RESULT_FAIL - it denote that mgr handler can't deal
 *                                the request. (ex. in transition mode)
 */
#define RADIUS_MGR_IPC_RESULT_OK    (0)
#define RADIUS_MGR_IPC_RESULT_FAIL  (-1)

#define RADIUS_MGR_MAX_SELECT_CHECK_INTERVAL    10 /* in seconds */

/* The commands for IPC message.
 */
enum {
    RADIUS_MGR_IPC_CMD_SET_REQUEST_TIMEOUT,
    RADIUS_MGR_IPC_CMD_SET_SERVER_PORT,
    RADIUS_MGR_IPC_CMD_SET_SERVER_SECRET,
    RADIUS_MGR_IPC_CMD_SET_RETRANSMIT_TIMES,
    RADIUS_MGR_IPC_CMD_SET_SERVER_IP,
    RADIUS_MGR_IPC_CMD_SET_SERVER_HOST,
    RADIUS_MGR_IPC_CMD_DESTROY_SERVER_HOST,
    RADIUS_MGR_IPC_CMD_GET_SERVER_ACCT_PORT,
    RADIUS_MGR_IPC_CMD_RADA_AUTH_CHECK,
    RADIUS_MGR_IPC_CMD_GET_ACC_CLIENT_IDENTIFIER,

    RADIUS_MGR_IPC_CMD_FOLLOWISASYNCHRONISMIPC,

    RADIUS_MGR_IPC_CMD_ASYNC_EAP_AUTH_CHECK,
    RADIUS_MGR_IPC_CMD_ASYNC_LOGIN_AUTH,
    RADIUS_MGR_IPC_CMD_ASYNC_ENABLE_PASSWORD_AUTH,
    RADIUS_MGR_IPC_CMD_SUBMIT_REQUEST,
    RADIUS_MGR_IPC_CMD_AUTH_CHECK,
};

typedef struct RADIUS_MGR_RequestContext_S
{
    RADIUS_OM_RadiusRequestType_T               type;

    BOOL_T                                      blocking;

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
}RADIUS_MGR_RequestContext_T;

/* MACRO FUNCTION DECLARATIONS
 */
/*---------------------------------------------------------------------------
 * MACRO NAME - RADIUS_MGR_GET_MSGBUFSIZE
 *---------------------------------------------------------------------------
 * PURPOSE : Get the size of RADIUS message that contains the specified
 *           type of data.
 * INPUT   : type_name - the type name of data for the message.
 * OUTPUT  : None
 * RETURN  : The size of RADIUS message.
 * NOTE    : None
 *---------------------------------------------------------------------------
 */
#define RADIUS_MGR_GET_MSGBUFSIZE(type_name) \
    (offsetof(RADIUS_MGR_IPCMsg_T, data) + sizeof(type_name))

/*---------------------------------------------------------------------------
 * MACRO NAME - RADIUS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA
 *---------------------------------------------------------------------------
 * PURPOSE : Get the size of RADIUS message that has no data block.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : The size of RADIUS message.
 * NOTE    : None
 *---------------------------------------------------------------------------
 */
#define RADIUS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA() \
    sizeof(RADIUS_MGR_IPCMsg_Type_T)

/*---------------------------------------------------------------------------
 * MACRO NAME - RADIUS_MGR_MSG_CMD
 *              RADIUS_MGR_MSG_RETVAL
 *---------------------------------------------------------------------------
 * PURPOSE : Get the RADIUS command/return value of an IPC message.
 * INPUT   : msg_p - the IPC message.
 * OUTPUT  : None
 * RETURN  : The RADIUS command/return value; it's allowed to be used as lvalue.
 * NOTE    : None
 *---------------------------------------------------------------------------
 */
#define RADIUS_MGR_MSG_CMD(msg_p)    (((RADIUS_MGR_IPCMsg_T *)(msg_p)->msg_buf)->type.cmd)
#define RADIUS_MGR_MSG_RETVAL(msg_p) (((RADIUS_MGR_IPCMsg_T *)(msg_p)->msg_buf)->type.result)

/*---------------------------------------------------------------------------
 * MACRO NAME - RADIUS_MGR_MSG_DATA
 *---------------------------------------------------------------------------
 * PURPOSE : Get the data block of an IPC message.
 * INPUT   : msg_p - the IPC message.
 * OUTPUT  : None
 * RETURN  : The pointer of the data block.
 * NOTE    : None
 *---------------------------------------------------------------------------
 */
#define RADIUS_MGR_MSG_DATA(msg_p)   ((void *)&((RADIUS_MGR_IPCMsg_T *)(msg_p)->msg_buf)->data)

/* DATA TYPE DECLARATIONS
 */

enum RADIUS_SERVICE_TYPE_E
{
    RADIUS_DOT1X_SERVICE = 0,
    RADIUS_MAX_SERVICE_NO
};

enum RADIUS_EVENT_MASK_E
{
    RADIUS_EVENT_NONE                   = 0x0000L,
    RADIUS_TASK_EVENT_ENTER_TRANSITION  = 0x0002L,
    RADIUS_EVENT_NEW_REQ                = 0x0004L,
    RADIUS_EVENT_TIMEOUT                = 0x0008L,
    RADIUS_EVENT_AUTH_SOCKET_AVAILABLE  = 0x0010L,
    RADIUS_EVENT_ACCT_SOCKET_AVAILABLE  = 0x0020L,
    RADIUS_EVENT_ALL = 0xFFFFL
};

typedef enum
{
    RADIUS_MGR_MONITOR_COMMAND_WATCH = 0,   /* watch the specified socket(s) in specified time */
    RADIUS_MGR_MONITOR_COMMAND_STOP_MONITOR,
    RADIUS_MGR_MONITOR_COMMAND_MAX,
} RADIUS_MGR_MONITOR_COMMAND_T;

typedef enum
{
    RADIUS_MGR_MONITOR_RESULT_NONE = 0,
    RADIUS_MGR_MONITOR_RESULT_COMMAND,
    RADIUS_MGR_MONITOR_RESULT_TIMEOUT,
    RADIUS_MGR_MONITOR_RESULT_AUTH_SOCKET_AVAILABLE,
    RADIUS_MGR_MONITOR_RESULT_ACCT_SOCKET_AVAILABLE,
} RADIUS_MGR_MONITOR_RESULT_T;

typedef struct
{
    union
    {
        struct
        {
            int     auth_socket_id;
            int     acct_socket_id;
            UI32_T  timeout; /* system time in seconds */
        } watch;
    } u;
} RADIUS_MGR_MonitorParameter_T;

typedef struct
{
    RADIUS_MGR_MONITOR_COMMAND_T    command;
    RADIUS_MGR_MonitorParameter_T   data;
} RADIUS_MGR_MonitorCommandMsg_T;

typedef struct RADIUS_MGR_MonitorContext_S
{
    RADIUS_MGR_MonitorCommandMsg_T command;
    UI32_T  command_len;

    int     auth_socket_id;
    int     acct_socket_id;
    UI32_T  timeout;
    BOOL_T  is_to_stop_monitor;
} RADIUS_MGR_MonitorContext_T;

#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)

typedef struct RADACC_AccClientIdentifier_S
{
    UI8_T   identifier[MAXSIZE_sysName + 1]; /* MAXSIZE_sysName is defined in leaf.h, leaf_1907.h, leaf_1213.h */
} RADACC_AccClientIdentifier_T;

#endif /* SYS_CPNT_RADIUS_ACCOUNTING == TRUE */


/* Message declarations for IPC.
 */
typedef struct
{
    UI32_T timeval;
} RADIUS_MGR_IPCMsg_RequestTimeout_T;

typedef struct
{
    UI32_T serverport;
} RADIUS_MGR_IPCMsg_ServerPort_T;

typedef struct
{
    UI8_T  serversecret[MAXSIZE_radiusServerGlobalKey + 1];
} RADIUS_MGR_IPCMsg_ServerSecret_T;

typedef struct
{
    UI32_T retryval;
} RADIUS_MGR_IPCMsg_RetransmitTimes_T;

typedef struct
{
    UI32_T serverip;
} RADIUS_MGR_IPCMsg_ServerIP_T;

#if 0 /*maggie liu for RADIUS authentication ansync*/
typedef struct
{
    UI8_T  username[RADIUS_USER_NAME_LEN+1];
    UI8_T  password[AUTH_PASS_LEN + 1];
    I32_T  privilege;
} RADIUS_MGR_IPCMsg_AuthCheck_T;
#endif

/*maggie liu for RADIUS authentication ansync*/
typedef struct RADIUS_MGR_IPCMsg_AuthCheck_S
{
    char   username[SYS_ADPT_MAX_USER_NAME_LEN +1];
    char   password  [SYS_ADPT_MAX_PASSWORD_LEN +1];
    I32_T  privilege;
}RADIUS_MGR_IPCMsg_AuthCheck_T;


#define RADIUS_MGR_ASYNC_MAX_COOKIE_SIZE 256

typedef struct
{
    UI8_T   cookie[RADIUS_MGR_ASYNC_MAX_COOKIE_SIZE];
    UI32_T  cookie_size;
    char    username[SYS_ADPT_MAX_USER_NAME_LEN +1];
    char    password[SYS_ADPT_MAX_PASSWORD_LEN +1];
}RADIUS_MGR_IPCMsg_AsyncLoginAuth_2_T;

typedef struct
{
    UI8_T   cookie[RADIUS_MGR_ASYNC_MAX_COOKIE_SIZE];
    UI32_T  cookie_size;
    char    username[SYS_ADPT_MAX_USER_NAME_LEN +1];
    char    password[SYS_ADPT_MAX_PASSWORD_LEN +1];
}RADIUS_MGR_IPCMsg_AsyncEnablePasswordAuth_T;

typedef struct
{
    char   username[RADIUS_USER_NAME_LEN+1];
    char   password[AUTH_PASS_LEN + 1];
    I32_T  privilege;
} RADIUS_MGR_IPCMsg_SyncAuthCheck_T;

typedef struct
{
    UI32_T  radius_id;
    UI32_T  eap_datalen;
    UI32_T  state_datalen;
    UI32_T  src_port;
    UI32_T  src_vid;
    UI32_T  service_type;
    UI32_T  cookie;         /* MSGQ_ID for return reslt */
    UI8_T   eap_data[EAP_MESSAGE_SIZE];
    UI8_T   state_data[STATE_MESSAGE_SIZE];
    UI8_T   src_mac[SYS_ADPT_MAC_ADDR_LEN];
    UI32_T  server_ip;
    char    username[DOT1X_USERNAME_LENGTH + 1];
    RADIUS_AsyncRequestControlFlag_T    flag;
} RADIUS_MGR_IPCMsg_AsyncAuthCheck_T;

typedef struct
{
    RADIUS_MGR_RequestContext_T request_p;
}RADIUS_MGR_IPCMsg_SubmitRequest_T;

/* format:
 *   rada_username - 00-00-00-00-00-00
 *   rada_password - 00-00-00-00-00-00
 */
typedef struct
{
    UI32_T  src_lport;
    UI32_T  cookie;         /* MSGQ_ID for return result */
    UI8_T   src_mac      [SYS_ADPT_MAC_ADDR_LEN];
    char    rada_username[RADIUS_MAX_MAC_STRING_LENGTH+1];
    char    rada_password[RADIUS_MAX_MAC_STRING_LENGTH+1];
} RADIUS_MGR_IPCMsg_RadaAuthCheck_T;

typedef struct
{
    UI32_T server_index;
    RADIUS_Server_Host_T server_host;
} RADIUS_MGR_IPCMsg_SetServerHost_T;

typedef struct
{
    UI32_T server_index;
} RADIUS_MGR_IPCMsg_DestroyServerHost_T;

typedef struct
{
    UI32_T acct_port;
} RADIUS_MGR_IPCMsg_ServerAcctPort_T;

typedef union
{
    UI32_T cmd;    /* for sending IPC request. CSCA_MGR_IPC_CMD1 ... */
    UI32_T result; /* for response */
} RADIUS_MGR_IPCMsg_Type_T;

typedef union
{
    RADIUS_MGR_IPCMsg_RequestTimeout_T      requesttimeout_data;
    RADIUS_MGR_IPCMsg_ServerPort_T          serverport_data;
    RADIUS_MGR_IPCMsg_ServerSecret_T        serversecret_data;
    RADIUS_MGR_IPCMsg_RetransmitTimes_T     retransmittimes_data;
    RADIUS_MGR_IPCMsg_ServerIP_T            serverip_data;
    RADIUS_MGR_IPCMsg_AuthCheck_T           authcheck_data;
    RADIUS_MGR_IPCMsg_AsyncAuthCheck_T      asyncauthcheck_data;
    RADIUS_MGR_IPCMsg_SetServerHost_T       setserverhost_data;
    RADIUS_MGR_IPCMsg_DestroyServerHost_T   destroyserverhost_data;
    RADIUS_MGR_IPCMsg_ServerAcctPort_T      server_acct_port;
    RADIUS_MGR_IPCMsg_RadaAuthCheck_T       radaauthcheck_data;

    RADIUS_MGR_IPCMsg_AsyncLoginAuth_2_T         authen_check_data;
    RADIUS_MGR_IPCMsg_AsyncEnablePasswordAuth_T  authen_enable_data;

    RADIUS_MGR_IPCMsg_SubmitRequest_T       request_data;
} RADIUS_MGR_IPCMsg_Data_T;

typedef struct
{
    RADIUS_MGR_IPCMsg_Type_T type;
    RADIUS_MGR_IPCMsg_Data_T data;
} RADIUS_MGR_IPCMsg_T;



/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_MGR_SetConfigSettingToDefault
 *---------------------------------------------------------------------------
 * PURPOSE: This function will set all the config values which are belongs
 *          to RADIUS_MGR to default value.
 * INPUT : None
 * OUTPUT: None
 * RETURN: TRUE/FALSE
 * NOTES : This is local function and only will be implemented by
 *        RADIUS_MGR_EnterMasterMode()
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_MGR_SetConfigSettingToDefault();

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_MGR_CurrentOperationMode
 *---------------------------------------------------------------------------
 * PURPOSE:  This function return the current opearion mode of RADIUS 's task
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   Radius_operation_mode
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
 SYS_TYPE_Stacking_Mode_T RADIUS_MGR_CurrentOperationMode();

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_MGR_EnterMasterMode
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will make the Radius client enter the master mode.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
 void RADIUS_MGR_EnterMasterMode();

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_MGR_EnterSlaveMode
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will make the Radius client enter the Slave mode.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
void RADIUS_MGR_EnterSlaveMode();

/* FUNCTION NAME : RADIUS_MGR_SetTransitionMode
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
void RADIUS_MGR_SetTransitionMode(void);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_MGR_EnterTransition Mode
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will make the Radius client enter the transition mode.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
void RADIUS_MGR_EnterTransitionMode();



BOOL_T RADIUS_MGR_InitiateProcessResources(void);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_MGR_Create_InterCSC_Relation
 *---------------------------------------------------------------------------
 * PURPOSE:  This function initializes all function pointer registration operations.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
void RADIUS_MGR_Create_InterCSC_Relation(void);

/* use RADIUS_OM/RADIUS_POM instead.
 */
#if 0
/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_MGR_GetRunningRequestTimeout
 *---------------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default RADIUS request timeout is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: timeout
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 *---------------------------------------------------------------------------
 */
 UI32_T RADIUS_MGR_GetRunningRequestTimeout(UI32_T *timeout);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_MGR_Get_Request_Timeout
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the number of times the RADIUS client
 *           waits for a reply from RADIUS server
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   timeout value.
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
 UI32_T RADIUS_MGR_Get_Request_Timeout(void);
#endif

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_MGR_Set_Request_Timeout
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set the number of times the RADIUS client
 *           waits for a reply from RADIUS server
 * INPUT:    timeout value.
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
 BOOL_T RADIUS_MGR_Set_Request_Timeout(UI32_T timeval);

/* use RADIUS_OM/RADIUS_POM instead.
 */
#if 0
/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_MGR_GetRunningServerPort
 *---------------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default RADIUS server port is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * RETURN: serverport
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 *---------------------------------------------------------------------------
 */
 UI32_T RADIUS_MGR_GetRunningServerPort(UI32_T *serverport);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_MGR_Get_Server_Port
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the UDP port number of the remote RADIUS server
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   Prot number
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
 UI32_T RADIUS_MGR_Get_Server_Port(void);
#endif

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_MGR_Set_Server_Port
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set the UDP port number of the remote RADIUS server
 * INPUT:    Prot number
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
 BOOL_T RADIUS_MGR_Set_Server_Port(UI32_T serverport);



#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_MGR_GetRunningServerAcctPort
 *---------------------------------------------------------------------------
 * PURPOSE  : This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *            the non-default RADIUS accounting port is successfully retrieved.
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
 *---------------------------------------------------------------------------
 */
UI32_T RADIUS_MGR_GetRunningServerAcctPort(UI32_T *acct_port);

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_MGR_GetServerAcctPort
 *---------------------------------------------------------------------------
 * PURPOSE  : get global server accounting port
 * INPUT    : none
 * OUTPUT   : acct_port
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_MGR_GetServerAcctPort(UI32_T *acct_port);

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_MGR_SetServerAcctPort
 *---------------------------------------------------------------------------
 * PURPOSE  : set global server accounting port
 * INPUT    : acct_port
 * OUTPUT   : none
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_MGR_SetServerAcctPort(UI32_T acct_port);

#endif /* SYS_CPNT_RADIUS_ACCOUNTING == TRUE */



/* use RADIUS_OM/RADIUS_POM instead.
 */
#if 0
/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_MGR_GetRunningServerSecret
 *---------------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default RADIUS server secret is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: serversecret[MAX_SECRET_LENGTH + 1]
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 *---------------------------------------------------------------------------
 */
UI32_T RADIUS_MGR_GetRunningServerSecret(UI8_T serversecret[]);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_MGR_Get_Server_Secret
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the shared secret text string used between
 *           the RADIUS client and server.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   secret text string pointer
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
UI8_T * RADIUS_MGR_Get_Server_Secret(void);
#endif

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_MGR_Set_Server_Secret
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set the shared secret text string used between
 *           the RADIUS client and server.
 * INPUT:    secret text string
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
 BOOL_T RADIUS_MGR_Set_Server_Secret(UI8_T *serversecret);

/* use RADIUS_OM/RADIUS_POM instead.
 */
#if 0
/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_MGR_GetRunningRetransmitTimes
 *---------------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default RADIUS retransmit times is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: retimes
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 *---------------------------------------------------------------------------
 */
 UI32_T RADIUS_MGR_GetRunningRetransmitTimes(UI32_T *retimes);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_MGR_Get_Retransmit_Times
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the number of times the RADIUS client
 *           transmits each RADIUS request to the server before giving up
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   retransmit times
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
 UI32_T RADIUS_MGR_Get_Retransmit_Times(void);
#endif

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_MGR_Set_Retransmit_Times
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set the number of times the RADIUS client
 *           transmits each RADIUS request to the server before giving up
 * INPUT:    retransmit times
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
 BOOL_T RADIUS_MGR_Set_Retransmit_Times(UI32_T retryval);

/* use RADIUS_OM/RADIUS_POM instead.
 */
#if 0
/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_MGR_GetRunningServerIP
 *---------------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default RADIUS server IP is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: serverip
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 *---------------------------------------------------------------------------
 */
 UI32_T RADIUS_MGR_GetRunningServerIP(UI32_T *serverip);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_MGR_Get_Server_IP
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the IP address of the remote RADIUS server
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   RADIUS server IP address
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
 UI32_T RADIUS_MGR_Get_Server_IP(void);
#endif

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_MGR_Set_Server_IP
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set the IP address of the remote RADIUS server
 * INPUT:    RADIUS server IP address
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
 BOOL_T RADIUS_MGR_Set_Server_IP(UI32_T serverip);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_Auth_Check
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will call RADIUS client to do authentication
 * INPUT:    RADIUS Authentication username  and password
 * OUTPUT:   RADIUS Authentication privilege
 *           privilege: RADIUS client service type
 *            15 = AUTH_ADMINISTRATIVE
 *             0 = AUTH_LOGIN
 * RETURN:   RADIUS Authentication result
 *        -3 = not in master mode
 *            -2 = receive illegal packet
 *            -1 = Authentication failure
 *             0 = Authentication OK
 *             1 = Authentication timeout
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
 I32_T RADIUS_Auth_Check(const char *username, const char *password,I32_T *privilege);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_EAP_Auth_Check
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will call RADIUS client to do authentication that
 *           support EAP protocol.
 * INPUT:    RADIUS Authentication username  and password
 * OUTPUT:   RADIUS Authentication privilege
 *           privilege: RADIUS client service type
 *            15 = AUTH_ADMINISTRATIVE
 *             0 = AUTH_LOGIN
 * RETURN:   RADIUS Authentication result
 *        -3 = not in master mode
 *            -2 = receive illegal packet
 *            -1 = Authentication failure
 *             0 = Authentication OK
 *             1 = Authentication timeout
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
I32_T RADIUS_MGR_EAP_Auth_Check(
    UI8_T *username,
    UI8_T *password,
    UI32_T src_port,
    UI8_T *src_mac,
    UI8_T *eap_data,
    UI32_T eap_datalen,
    UI8_T *recv_data,
    UI32_T *recv_datalen,
    char *authorized_vlan_list,
    char *authorized_qos_list,
    UI32_T *session_timeout,
    R_STATE_DATA *state_message,
    UI32_T *server_ip
);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_MGR_SyncAuthCheck
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will call RADIUS client to do authentication
 * INPUT:    RADIUS Authentication username  and password
 * OUTPUT:   RADIUS Authentication privilege
 *           privilege: RADIUS client service type
 *            15 = AUTH_ADMINISTRATIVE
 *             0 = AUTH_LOGIN
 * RETURN:   RADIUS Authentication result
 *        -3 = not in master mode
 *            -2 = receive illegal packet
 *            -1 = Authentication failure
 *             0 = Authentication OK
 *             1 = Authentication timeout
 * NOTE:     The same as RADIUS_Auth_Check
 *---------------------------------------------------------------------------
 */
I32_T RADIUS_MGR_SyncAuthCheck(const char *username, const char *password,I32_T *privilege);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_MGR_AsyncEapAuthCheck
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will call RADIUS client to do EAP authentication
 * INPUT:    eap_data     --- EAP packet data
 *           eap_datalen  --- EAP packet data length
 *           radius_id    --- RADIUS sequent ID
 *           state_data   --- RADIUS STATE type packet data
 *           state_datale --- RADIUS STATE type packet data length
 *           src_port     --- source port
 *           src_mac      --- source mac address
 *           src_vid      --- source vid
 *           cookie       --- MSGQ_ID for return result
 *           service_type --- which component need to be service
 *           server_ip    --- Use this server IP address first. 0 means not
 *                            specified
 *           username_p   --- Username
 *           flag         --- Control flag
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE
 * NOTE:     All asynchornous request will be enqueued and be proccessed by
 *           RADIUS task.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_MGR_AsyncEapAuthCheck(
    UI8_T   *eap_data,
    UI32_T  eap_datalen,
    UI32_T  radius_id,
    UI8_T   *state_data,
    UI32_T  state_datalen,
    UI32_T  src_port,
    UI8_T   *src_mac,
    UI32_T  src_vid,
    UI32_T  cookie,
    UI32_T  service_type,
    UI32_T  server_ip,
    char    *username_p,
    RADIUS_AsyncRequestControlFlag_T flag);

/*  For MIB */

/* use RADIUS_OM/RADIUS_POM instead.
 */
#if 0
/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_MGR_Get_UnknowAddress_Packets
 *---------------------------------------------------------------------------
 * PURPOSE:  Get the number of RADIUS Access-Response packets
 *           received from unknown addresses.
 * INPUT:    None
 * OUTPUT:   None.
 * RETURN:   Number of RADIUS Access-Response packets received from unknown addresses
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
 UI32_T  RADIUS_MGR_Get_UnknowAddress_Packets(void);
/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_MGR_Get_NAS_ID
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
 BOOL_T RADIUS_MGR_Get_NAS_ID(UI8_T *nas_id);
/*---------------------------------------------------------------------------
 * ROUTINE NAME - GetAuthServerTable
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will call RADIUS client MIB to get radiusAuthServerTable
 * INPUT:    adiusAuthServerIndex
 * OUTPUT:   AuthServerEntry pointer
 * RETURN:   Get table TRUE/FALSE
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
 BOOL_T GetAuthServerTable(UI32_T Index,AuthServerEntry *ServerEntry);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - GetNextAuthServerTable
 *---------------------------------------------------------------------------
 * PURPOSE:  This funtion returns true if the next available table entry info
 *           can be successfully retrieved. Otherwise, false is returned.
 * INPUT:    RadiusAuthServerIndex ,
 * OUTPUT:   NextAuthServerEntry pointer
 * RETURN:   Get next table TRUE/FALSE
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
 BOOL_T GetNextAuthServerTable(UI32_T *Index,AuthServerEntry *ServerEntry);
#endif

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_MGR_Set_Server_Host
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set the RADIUS server host
 * INPUT:    server_index,server_host
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE
 * NOTE:        server_ip;
 *      server_port (1-65535)  - set 0 will use the global radius configuration
 *          timeout     (1-65535)  - set 0 will use the global radius configuration
 *          retransmit  (1-65535)  - set 0 will use the global radius configuration
 *          secret      (length < RADIUS_MAX_SECRET_LENGTH)  - set NULL will use the global radius configuration
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_MGR_Set_Server_Host(UI32_T server_index,RADIUS_Server_Host_T *server_host);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_MGR_Destroy_Server_Host
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will destroy the RADIUS server host
 * INPUT:    server_index
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_MGR_Destroy_Server_Host(UI32_T server_index);

/* use RADIUS_OM/RADIUS_POM instead.
 */
#if 0
/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_MGR_GetNext_Server_Host
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the RADIUS server host configuration
 * INPUT:    server_index
 * OUTPUT:   server_host.
 * RETURN:   TRUE/FALSE
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_MGR_GetNext_Server_Host(UI32_T *index,RADIUS_Server_Host_T *server_host);

/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_MGR_GetNextRunning_Server_Host
 *---------------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default RADIUS server host is successfully retrieved.
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
UI32_T RADIUS_MGR_GetNextRunning_Server_Host(UI32_T *index,RADIUS_Server_Host_T *server_host);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_MGR_Get_Server_Host
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the RADIUS server host configuration
 * INPUT:    server_index
 * OUTPUT:   server_host.
 * RETURN:   TRUE/FALSE
 * NOTE:     The index range is from 1 to SYS_ADPT_RADIUS_MAX_NUMBER_OF_SERVERS
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_MGR_Get_Server_Host(UI32_T index,RADIUS_Server_Host_T *server_host);
#endif

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_MGR_GetServerHostMaxRetransmissionTimeout
 *---------------------------------------------------------------------------
 * PURPOSE  : query the max of retransmission timeout of server hosts
 * INPUT    : none
 * OUTPUT   : max_retransmission_timeout
 * RETURN   : TRUE - succeeded; FALSE - failed
 * NOTES    : none
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_MGR_GetServerHostMaxRetransmissionTimeout(UI32_T *max_retransmission_timeout);

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_MGR_IsServerHostValid
 *---------------------------------------------------------------------------
 * PURPOSE  : query whether this server host is valid or not
 * INPUT    : server_index (1-based)
 * OUTPUT   : none
 * RETURN   : TRUE - valid; FALSE - invalid
 * NOTES    : none
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_MGR_IsServerHostValid(UI32_T server_index);

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_MGR_LookupServerIndexByIpAddress
 *---------------------------------------------------------------------------
 * PURPOSE  : lookup server host by ip_address
 * INPUT    : ip_address
 * OUTPUT   : server_index (1-based)
 * RETURN   : TRUE - found; FALSE - not found
 * NOTES    : none
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_MGR_LookupServerIndexByIpAddress(UI32_T ip_address, UI32_T *server_index);

/* FUNCTION NAME - RADIUS_MGR_HandleHotInsertion
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
void RADIUS_MGR_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default);



/* FUNCTION NAME - RADIUS_MGR_HandleHotRemoval
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
void RADIUS_MGR_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port);

/* FUNCTION NAME - RADIUS_MGR_RadaAuthCheck
 * PURPOSE  : for rada mode authentication
 * INPUT    : src_port, src_mac, rada_username, rada_passwd,
 *            cookie (MSGQ_ID to return the result)
 * OUTPUT   : None
 * RETURN   : TRUE -- succeeded / FALSE -- failed
 * NOTE     : none
 */
BOOL_T RADIUS_MGR_RadaAuthCheck(
    UI32_T  src_port,       UI8_T   *src_mac,
    char    *rada_username, char    *rada_passwd,   UI32_T  cookie);

#if(SYS_ADPT_RADIUS_USE_FIELD_ID_AS_PRIVILEGE == TRUE)
/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_MGR_GetRunningAuthCheckingServiceTypeEnabled
 *---------------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default RADIUS server IP is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: mode
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 *---------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T RADIUS_MGR_GetRunningAuthCheckingServiceTypeEnabled(BOOL_T *mode);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_MGR_SetAuthCheckingServiceTypeEnabled
 *---------------------------------------------------------------------------
 * PURPOSE  : Set whether to check service-type when authorizing from radius server
 * INPUT    : mode
 * OUTPUT   : None
 * RETURN   : TRUE - success; FALSE - fail
 * NOTE:    : None
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_MGR_SetAuthCheckingServiceTypeEnabled(BOOL_T mode);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_MGR_GetAuthCheckingServiceTypeEnabled
 *---------------------------------------------------------------------------
 * PURPOSE  : Get whether to check service-type when authorizing from radius server
 * INPUT    : None
 * OUTPUT   : mode
 * RETURN   : TRUE - success; FALSE - fail
 * NOTE:    : None
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_MGR_GetAuthCheckingServiceTypeEnabled(BOOL_T *mode);
#endif

#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_MGR_GetAccClientInvalidServerAddresses
 *---------------------------------------------------------------------------
 * PURPOSE: This function to get the number of RADIUS Accounting-Response packets
 *          received from unknown addresses.
 * INPUT:   none.
 * OUTPUT:  UI32_T *invalid_server_address_counter  --  The number of RADIUS Accounting-Response
 *                                                      packets received from unknown addresses.
 * RETURN:  TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:   none.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_MGR_GetAccClientInvalidServerAddresses(UI32_T *invalid_server_address_counter);

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_MGR_GetAccClientIdentifier
 *---------------------------------------------------------------------------
 * PURPOSE  : This function to get the NAS-Identifier of the RADIUS accounting client.
 * INPUT    : none.
 * OUTPUT   : client_identifier  --  the NAS-Identifier of the RADIUS accounting client.
 * RETURN   : TRUE to indicate successful and FALSE to indicate failure.
 * NOTES    : none.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_MGR_GetAccClientIdentifier(RADACC_AccClientIdentifier_T *client_identifier);


/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_MGR_HookAccRequestIfNeed
 *---------------------------------------------------------------------------
 * PURPOSE  : hook accounting update request if need
 * INPUT    : none.
 * OUTPUT   : none.
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_MGR_HookAccRequestIfNeed();

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_MGR_SendNextAccRequest
 *---------------------------------------------------------------------------
 * PURPOSE  : This function will sent accounting request to accounting server.
 * INPUT    : none.
 * OUTPUT   : none.
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_MGR_SendNextAccRequest();

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_MGR_RecvAccResponse
 *---------------------------------------------------------------------------
 * PURPOSE  : try to receive accounting request from accounting server.
 * INPUT    : none.
 * OUTPUT   : none.
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_MGR_RecvAccResponse();

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_MGR_GetAccUserEntryQty
 *---------------------------------------------------------------------------
 * PURPOSE  : get the number of user entries
 * INPUT    : none.
 * OUTPUT   : qty.
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : exclude "dead" user which connect_status ==
 *            AAA_ACC_CNET_DORMANT, AAA_ACC_CNET_IDLE, AAA_ACC_CNET_FAILED
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_MGR_GetAccUserEntryQty(UI32_T *qty);

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_MGR_GetAccUserEntryQtyFilterByNameAndType
 *---------------------------------------------------------------------------
 * PURPOSE  : get the number of user with specified name and client_type
 * INPUT    : name, client_type
 * OUTPUT   : qty.
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : exclude "dead" user which connect_status ==
 *            AAA_ACC_CNET_DORMANT, AAA_ACC_CNET_IDLE, AAA_ACC_CNET_FAILED
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_MGR_GetAccUserEntryQtyFilterByNameAndType(char *name, AAA_ClientType_T client_type, UI32_T *qty);

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_MGR_GetAccUserEntryQtyFilterByType
 *---------------------------------------------------------------------------
 * PURPOSE  : get the number of user with specified client_type
 * INPUT    : client_type
 * OUTPUT   : qty.
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : exclude "dead" user which connect_status ==
 *            AAA_ACC_CNET_DORMANT, AAA_ACC_CNET_IDLE, AAA_ACC_CNET_FAILED
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_MGR_GetAccUserEntryQtyFilterByType(AAA_ClientType_T client_type, UI32_T *qty);

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_MGR_GetAccUserEntryQtyFilterByPort
 *---------------------------------------------------------------------------
 * PURPOSE  : get the number of user from specified port
 * INPUT    : ifindex
 * OUTPUT   : qty.
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : exclude "dead" user which connect_status ==
 *            AAA_ACC_CNET_DORMANT, AAA_ACC_CNET_IDLE, AAA_ACC_CNET_FAILED
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_MGR_GetAccUserEntryQtyFilterByPort(UI32_T ifindex, UI32_T *qty);

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_MGR_GetAccUserRunningInfo
 *---------------------------------------------------------------------------
 * PURPOSE  : get running_info by ifindex, user name, client type
 * INPUT    : ifindex, name, client_type
 * OUTPUT   : running_info
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : suppose (ifindex + name + client_type) is unique
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_MGR_GetAccUserRunningInfo(UI32_T ifindex, const char *name, AAA_ClientType_T client_type, AAA_AccUserRunningInfo_T *running_info);


#endif /* SYS_CPNT_RADIUS_ACCOUNTING == TRUE */

/*---------------------------------------------------------------------------
 * FUNCTION NAME - RADIUS_MGR_HandleIPCReqMsg
 *---------------------------------------------------------------------------
 * PURPOSE : Handle the ipc request message for RADIUS mgr.
 * INPUT   : ipcmsg_p  --  input request ipc message buffer
 * OUTPUT  : ipcmsg_p  --  output response ipc message buffer
 * RETUEN  : TRUE  - need to send response.
 *           FALSE - not need to send response.
 * NOTE    : None
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_MGR_HandleIPCReqMsg(SYSFUN_Msg_T *ipcmsg_p);

#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)
/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_MGR_Backdoor_ShowAccUser
 *---------------------------------------------------------------------------
 * PURPOSE  : show accounting user information
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : for radius backdoor
 *---------------------------------------------------------------------------
 */
void RADIUS_MGR_Backdoor_ShowAccUser(void);

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_MGR_Reload_CallBack
 *---------------------------------------------------------------------------
 * PURPOSE  : system reload callback
 * INPUT    : none
 * OUTPUT   : none.
 * RETURN   : none
 * NOTES    : none.
 *---------------------------------------------------------------------------
 */
void RADIUS_MGR_Reload_CallBack();

#if 0
/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_MGR_TCN_CallBack
 *---------------------------------------------------------------------------
 * PURPOSE  : system TCN callback
 * INPUT    : none
 * OUTPUT   : none.
 * RETURN   : none
 * NOTES    : none.
 *---------------------------------------------------------------------------
 */
static void RADIUS_MGR_TCN_CallBack();
#endif /* #if 0 */
#endif /* #if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE) */

/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_MGR_SubmitRequest
 *---------------------------------------------------------------------------
 * PURPOSE  : Submit a request, call RADIUS_OM_SubmitRequest to create a
 *            request in request queue.
 * INPUT    : request_p - Request
 * OUTPUT   : None
 * RETURN   : RADIUS_RETURN_FAIL
 *            RADIUS_RETURN_SUCCESS
 * NOTES    : None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_MGR_SubmitRequest(
    RADIUS_MGR_RequestContext_T *request_p);

/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_MGR_CreateAuthReqSocket
 *---------------------------------------------------------------------------
 * PURPOSE  : Create the socket to send and receive authentcation request(s).
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : RADIUS_RETURN_FAIL
 *            RADIUS_RETURN_SUCCESS
 * NOTES    : None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_MGR_CreateAuthReqSocket();

/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_MGR_DestroyAuthReqSocket
 *---------------------------------------------------------------------------
 * PURPOSE  : Destroy the socket to send and receive authentcation request(s).
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : RADIUS_RETURN_FAIL
 *            RADIUS_RETURN_SUCCESS
 * NOTES    : None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_MGR_DestroyAuthReqSocket();

/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_MGR_RunRequestStateMachine
 *---------------------------------------------------------------------------
 * PURPOSE  : Process state machine of a specified request.
 * INPUT    : request_index - Request index
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 *---------------------------------------------------------------------------
 */
void RADIUS_MGR_RunRequestStateMachine(UI32_T request_index);

/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_MGR_NotifyMonitorTaskWatchSockets
 *---------------------------------------------------------------------------
 * PURPOSE  : Notify monitor task with sockets of authentication and
 *            accounting and specified time (notifies when expires).
 * INPUT    : socket_id  - Socket to send command for notify
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    : None
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_MGR_NotifyMonitorTaskWatchSockets(int socket_id);

/*---------------------------------------------------------------------------
 * FUNCTION NAME - RADIUS_MGR_NotifyMonitorTaskStopMonitor
 *---------------------------------------------------------------------------
 * PURPOSE : Notify monitor task to stop monitor.
 * INPUT   : socket_id  - Socket to send command for notify
 * OUTPUT  : None
 * RETUEN  : TRUE/FALSE
 * NOTE    : None
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_MGR_NotifyMonitorTaskStopMonitor(int socket_id);

/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_MGR_SelectMonitorSockets
 *---------------------------------------------------------------------------
 * PURPOSE  : Select on RADIUS monitor sockets (control, authentication and
 *            accounting) with specified timeout.
 * INPUT    : auth_socket_id            - Authentication socket ID
 *            acct_socket_id            - Accounting socket ID
 *            monitor_socket_id         - Monitor socket ID
 *            auth_and_acct_timeout     - Timeout in seconds
 * OUTPUT   : result_p                  - Result of monitor
 * RETURN   : TRUE/FALSE
 * NOTES    : None
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_MGR_SelectMonitorSockets(int auth_socket_id,
    int acct_socket_id, int monitor_socket_id, UI32_T auth_and_acct_timeout,
    RADIUS_MGR_MONITOR_RESULT_T *result_p);

/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_MGR_ReceiveMonitorCommand
 *---------------------------------------------------------------------------
 * PURPOSE  : Receive monitor command from the specified socket.
 * INPUT    : socket_id     - Socket ID
 * OUTPUT   : context_p - Monitor context
 * RETURN   : TRUE/FALSE
 * NOTES    : None
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_MGR_ReceiveMonitorCommand(int socket_id,
    RADIUS_MGR_MonitorContext_T *context_p);

/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_MGR_ProcessMonitorCommand
 *---------------------------------------------------------------------------
 * PURPOSE  : Process monitor command and extract it into context.
 * INPUT    : context_p - Monitor context
 * OUTPUT   : context_p - Monitor context
 * RETURN   : TRUE/FALSE
 * NOTES    : None
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_MGR_ProcessMonitorCommand(
    RADIUS_MGR_MonitorContext_T *context_p);

/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_MGR_ProcessTimeoutRequests
 *---------------------------------------------------------------------------
 * PURPOSE  : Process timeout requests.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 *---------------------------------------------------------------------------
 */
void RADIUS_MGR_ProcessTimeoutRequests();

#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)
/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_MGR_ProcessTimeoutAccUsers
 *---------------------------------------------------------------------------
 * PURPOSE  : Process timeout accounting users.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 *---------------------------------------------------------------------------
 */
void RADIUS_MGR_ProcessTimeoutAccUsers();
#endif /* #if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE) */

/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_MGR_ProcessMonitorTimeoutEvent
 *---------------------------------------------------------------------------
 * PURPOSE  : Process monitor timeout event.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 *---------------------------------------------------------------------------
 */
void RADIUS_MGR_ProcessMonitorTimeoutEvent();

/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_MGR_ProcessMonitorAuthSocketAvailableEvent
 *---------------------------------------------------------------------------
 * PURPOSE  : Process monitor event indicates that authentication socket is
 *            available now.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 *---------------------------------------------------------------------------
 */
void RADIUS_MGR_ProcessMonitorAuthSocketAvailableEvent();

/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_MGR_ProcessMonitorAcctSocketAvailableEvent
 *---------------------------------------------------------------------------
 * PURPOSE  : Process monitor event indicates that accounting socket is
 *            available now.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 *---------------------------------------------------------------------------
 */
void RADIUS_MGR_ProcessMonitorAcctSocketAvailableEvent();

/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_MGR_ProcessRequestFromWaitingQueue
 *---------------------------------------------------------------------------
 * PURPOSE  : Process new submitted RADIUS requests from waiting queue.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    :
 *---------------------------------------------------------------------------
 */
void RADIUS_MGR_ProcessRequestFromWaitingQueue();

#endif /* RADIUS_MGR_H */

