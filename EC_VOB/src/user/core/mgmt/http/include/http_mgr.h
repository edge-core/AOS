/* MODULE NAME: http_mgr.h
 * PURPOSE:
 *   Initialize the resource and provide some functions for the http module.
 *
 * NOTES:
 *
 * History:
 *       Date          -- Modifier,  Reason
 *     2002-02         --Isiah , created.
 *
 * Copyright(C)      Accton Corporation, 2002
 */

#ifndef HTTP_MGR_H
#define HTTP_MGR_H


/* INCLUDE FILE DECLARATIONS
 */
#include "sys_cpnt.h"
#include "sys_dflt.h"
#include "http_type.h"
#include "sysfun.h"

#if __cplusplus
extern "C" {
#endif

/* NAMING CONSTANT DECLARATIONS
 */
/* The key to get HTTP mgr msgq.
 */
/* The commands for IPC message.
 */
enum {
    HTTP_MGR_IPC_CMD_GET_SECURE_HTTP_STATUS,        /* 0 */
    HTTP_MGR_IPC_CMD_GET_SECURE_HTTP_PORT,          /* 1 */
    HTTP_MGR_IPC_CMD_SET_SECURE_HTTP_STATUS,        /* 2 */
    HTTP_MGR_IPC_CMD_SET_SECURE_HTTP_PORT,          /* 3 */
    HTTP_MGR_IPC_CMD_GET_RUNNING_HTTP_SECURE_PORT,  /* 4 */
    HTTP_MGR_IPC_CMD_GET_RUNNING_HTTP_SECURE_STATUS,/* 5 */
    HTTP_MGR_IPC_CMD_GET_RUNNING_HTTP_PORT,         /* 6 */
    HTTP_MGR_IPC_CMD_GET_RUNNING_HTTP_STATUS,       /* 7 */
    HTTP_MGR_IPC_CMD_GET_HTTP_PORT,                 /* 8 */
    HTTP_MGR_IPC_CMD_GET_HTTP_STATUS,               /* 9 */
    HTTP_MGR_IPC_CMD_SET_HTTP_PORT,                 /* 10 */
    HTTP_MGR_IPC_CMD_SET_HTTP_STATUS,               /* 11 */
    HTTP_MGR_IPC_CMD_GET_RUNNING_HTTP_DEBUG_INFO,   /* 12 */
    HTTP_MGR_IPC_CMD_GET_RUNNING_HTTP_PARAMETERS,   /* 13 */
    HTTP_MGR_IPC_CMD_GETNEXT_USER_CONNECT_INFO,
    HTTP_MGR_IPC_CMD_SET_USER_CONNECT_SENDLOGFLAG,
    HTTP_MGR_IPC_CMD_SET_TFTP_DOWNLOAD_CERTIFICATE_FLAG,
    HTTP_MGR_IPC_CMD_SET_TFTP_IP,
    HTTP_MGR_IPC_CMD_SET_TFTP_CERTIFICATE_FILENAME,
    HTTP_MGR_IPC_CMD_SET_PRIVATEKEY_FILENAME,
    HTTP_MGR_IPC_CMD_SET_PRIVATEKEY_PASSWORD,
    HTTP_MGR_IPC_CMD_GET_CERTIFICATE_FROM_TFTP,
};


/* MACRO DEFINITIONS
 */
/*-------------------------------------------------------------------------
 * MACRO NAME - HTTP_MGR_GET_MSGBUFSIZE
 *-------------------------------------------------------------------------
 * PURPOSE : Get the size of HTTP message that contains the specified
 *           type of data.
 * INPUT   : type_name - the type name of data for the message.
 * OUTPUT  : None
 * RETURN  : The size of HTTP message.
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
#define HTTP_MGR_GET_MSGBUFSIZE(type_name) \
    ((uintptr_t)&((HTTP_MGR_IPCMsg_T *)0)->data + sizeof(type_name))

/*-------------------------------------------------------------------------
 * MACRO NAME - HTTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA
 *-------------------------------------------------------------------------
 * PURPOSE : Get the size of HTTP message that has no data block.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : The size of HTTP message.
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
#define HTTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA() \
    sizeof(HTTP_MGR_IPCMsg_Type_T)

/*-------------------------------------------------------------------------
 * MACRO NAME - HTTP_MGR_MSG_CMD
 *              HTTP_MGR_MSG_RETVAL
 *-------------------------------------------------------------------------
 * PURPOSE : Get the HTTP command/return value of an IPC message.
 * INPUT   : msg_p - the IPC message.
 * OUTPUT  : None
 * RETURN  : The HTTP command/return value; it's allowed to be used as lvalue.
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
#define HTTP_MGR_MSG_CMD(msg_p)    (((HTTP_MGR_IPCMsg_T *)(msg_p)->msg_buf)->type.cmd)
#define HTTP_MGR_MSG_RETVAL(msg_p) (((HTTP_MGR_IPCMsg_T *)(msg_p)->msg_buf)->type.result)

/*-------------------------------------------------------------------------
 * MACRO NAME - HTTP_MGR_MSG_DATA
 *-------------------------------------------------------------------------
 * PURPOSE : Get the data block of an IPC message.
 * INPUT   : msg_p - the IPC message.
 * OUTPUT  : None
 * RETURN  : The pointer of the data block.
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
#define HTTP_MGR_MSG_DATA(msg_p)   ((void *)&((HTTP_MGR_IPCMsg_T *)(msg_p)->msg_buf)->data)


/* DATA TYPE DECLARATIONS
 */
typedef struct
{
    UI32_T              port;
    HTTP_STATE_T    	state;
    BOOL_T              port_changed;
    BOOL_T              state_changed;
}HTTP_MGR_RunningCfg_T;

/* Message declarations for IPC.
 */
typedef union
{
    UI32_T  cmd;    /* for sending IPC request. CSCA_MGR_IPC_CMD1 ... */
    UI32_T  result; /* for response */
} HTTP_MGR_IPCMsg_Type_T;

typedef struct
{
    HTTP_STATE_T     status;
    UI32_T           port;
} HTTP_MGR_IPCMsg_HTTP_STATUS_Type_T;

#if (SYS_CPNT_HTTPS == TRUE)

typedef struct
{
    SECURE_HTTP_STATE_T     status;
    UI32_T                  port;
} HTTP_MGR_IPCMsg_HTTPS_STATUS_Type_T;

typedef struct
{
    L_INET_AddrIp_T  server_ip;
} HTTP_MGR_IPCMsg_TFTP_ADDR_Type_T;

typedef struct
{
    UI8_T  tftp_src_filename[MAXSIZE_tftpSrcFile+1];
} HTTP_MGR_IPCMsg_TFTP_SRC_FILENAME_Type_T;

typedef struct
{
    UI8_T  passwd[20+1];
}HTTP_MGR_IPCMsg_SERVER_PRIVATE_PWD_Type_T;

#endif  /* #if (SYS_CPNT_HTTPS == TRUE) */


typedef struct
{
    UI32_T http_debug;
}HTTP_MGR_IPCMsg_HTTP_DEBUG_Type_T;

typedef struct
{
    UI32_T running_status;
}HTTP_MGR_IPCMsg_RUNNING_STATUS_Type_T;


typedef struct
{
    HTTP_MGR_RunningCfg_T   http_runcfg_data;
}HTTP_MGR_IPCMsg_HTTP_RUNNING_CFG_Type_T;

typedef struct
{
    HTTP_Session_T    session;
}HTTP_MGR_IPCMsg_USER_CONNECTION_INFO_T;

typedef struct
{
    HTTP_DownloadCertificateEntry_T    certificate_entry;
}HTTP_MGR_IPCMsg_DOWNLOAD_CERTIFICATE_T;

typedef union
{
#if (SYS_CPNT_HTTPS == TRUE)
    HTTP_MGR_IPCMsg_HTTPS_STATUS_Type_T      https;
    HTTP_MGR_IPCMsg_TFTP_ADDR_Type_T         server_ip;
    HTTP_MGR_IPCMsg_TFTP_SRC_FILENAME_Type_T tftp_src_filename;
    HTTP_MGR_IPCMsg_SERVER_PRIVATE_PWD_Type_T private_pwd;
#endif
    HTTP_MGR_IPCMsg_HTTP_STATUS_Type_T      http;
    HTTP_MGR_IPCMsg_HTTP_DEBUG_Type_T       httpdebug;
    HTTP_MGR_IPCMsg_HTTP_RUNNING_CFG_Type_T http_runcfg;
    HTTP_MGR_IPCMsg_RUNNING_STATUS_Type_T    runnning_status;
    HTTP_MGR_IPCMsg_DOWNLOAD_CERTIFICATE_T      download_certificate;
    HTTP_MGR_IPCMsg_USER_CONNECTION_INFO_T      user_conn_info;
} HTTP_MGR_IPCMsg_Data_T;

typedef struct
{
    HTTP_MGR_IPCMsg_Type_T    type;
    HTTP_MGR_IPCMsg_Data_T    data;
} HTTP_MGR_IPCMsg_T;

/* NAMING CONSTANT DECLARATIONS
 */
#define HTTP_MGR_STATE_T    HTTP_STATE_T
#define HTTP_MGR_DEFPORT    HTTP_DEFAULT_PORT_NUMBER
#define HTTP_MGR_DEFSTATE   HTTP_DEFAULT_STATE
#define HTTP_MGR_OPMODE_T   SYS_TYPE_Stacking_Mode_T
#define HTTP_MGR_OPMODE_MASTER      SYS_TYPE_STACKING_MASTER_MODE
#define HTTP_MGR_OPMODE_TRANSITION  SYS_TYPE_STACKING_TRANSITION_MODE
#define HTTP_MGR_OPMODE_SLAVE       SYS_TYPE_STACKING_SLAVE_MODE
#define	HTTP_MGR_STATE_ENABLED  HTTP_STATE_ENABLED
#define	HTTP_MGR_STATE_DISABLED HTTP_STATE_DISABLED

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

//#ifdef SUPPORT_DEBUG_IP_HTTP_SECURE
typedef enum HTTP_MGR_DebugType_E
{
    HTTP_MGR_DEBUG_NONE = 0,
    HTTP_MGR_DEBUG_SESSION,
    HTTP_MGR_DEBUG_STATE,
    HTTP_MGR_DEBUG_ALL
} HTTP_MGR_DebugType_T;
//#endif /* ifdef SUPPORT_DEBUG_IP_HTTP_SECURE */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
void HTTP_MGR_InitiateSystemResources(void);

/* FUNCTION NAME:  HTTP_MGR_Create_InterCSC_Relation
 * PURPOSE:
 *          This function initializes all function pointer registration operations.
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
 *          none.
 */
void HTTP_MGR_Create_InterCSC_Relation(void);

/* FUNCTION NAME:  HTTP_MGR_Enter_Critical_Section
 * PURPOSE:
 *          Enter critical section before a task invokes the http_mgr objects.
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
 *          .
 */
BOOL_T HTTP_MGR_Enter_Critical_Section(void);



/* FUNCTION NAME:  HTTP_MGR_Leave_Critical_Section
 * PURPOSE:
 *          Leave critical section after a task invokes the http_mgr objects.
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
 *          .
 */
BOOL_T HTTP_MGR_Leave_Critical_Section(void);



/* FUNCTION NAME:  HTTP_MGR_Enter_Master_Mode
 * PURPOSE:
 *          This function initiates all the system database, and also configures
 *          the switch to the initiation state based on the specified "System Boot
 *          Configruation File". After that, the HTTP subsystem will enter the
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
 *			2. HTTP will handle network requests only when this subsystem
 *				is in the Master Operation mode
 *			3. This function is invoked in HTTP_INIT_EnterMasterMode.
 */
BOOL_T HTTP_MGR_Enter_Master_Mode(void);



/* FUNCTION NAME:  HTTP_MGR_Enter_Transition_Mode
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
BOOL_T HTTP_MGR_Enter_Transition_Mode(void);



/* FUNCTION NAME:  HTTP_MGR_Enter_Slave_Mode
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
void HTTP_MGR_Enter_Slave_Mode(void);



/* FUNCTION	NAME : HTTP_MGR_SetTransitionMode
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
void HTTP_MGR_SetTransitionMode(void);


#if (SYS_CPNT_MGMT_IP_FLT == TRUE)
/* FUNCTION NAME : HTTP_MGR_MgmtIPFltChanged_Callback
 * PURPOSE:
 *      Process when the database of management IP filter was changed
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
void HTTP_MGR_MgmtIPFltChanged_Callback();
#endif /* #if (SYS_CPNT_MGMT_IP_FLT == TRUE) */

/* FUNCTION NAME:  HTTP_MGR_Set_Root_Dir
 * PURPOSE:
 *          Set web root directory.
 *
 * INPUT:
 *          dir - the root directory
 *
 * OUTPUT:
 *          None
 *
 * RETURN:
 *          None
 *
 * NOTES:
 *          None
 */
BOOL_T HTTP_MGR_Set_Root_Dir(const char *dir);

/* FUNCTION NAME:  HTTP_MGR_Get_Root_Dir
 * PURPOSE:
 *          Get web root directory.
 *
 * INPUT:
 *          buf - buffer pointer
 *          bufsz - buffer size
 *
 * OUTPUT:
 *          the root directory
 *
 * RETURN:
 *          None
 *
 * NOTES:
 *          None
 */
BOOL_T HTTP_MGR_Get_Root_Dir(char *buf, size_t bufsz);

/* FUNCTION NAME:  HTTP_MGR_Set_Http_Status
 * PURPOSE:
 *          This function set http state.
 *
 * INPUT:
 *          HTTP_STATE_T - HTTP status.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          none.
 * NOTES:
 *          .
 */
BOOL_T HTTP_MGR_Set_Http_Status (HTTP_STATE_T state);



/* FUNCTION NAME:  HTTP_MGR_Get_Http_Status
 * PURPOSE:
 *          This function get http state.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          HTTP_STATE_T - HTTP status.
 * NOTES:
 *          .
 */
HTTP_STATE_T HTTP_MGR_Get_Http_Status();



/* FUNCTION NAME:  HTTP_MGR_Set_Http_Port
 * PURPOSE:
 *          This function set http port number.
 *
 * INPUT:
 *          UI32_T - HTTP port number.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          none.
 * NOTES:
 *          .
 */
BOOL_T HTTP_MGR_Set_Http_Port (UI32_T port);



/* FUNCTION NAME:  HTTP_MGR_Get_Http_Port
 * PURPOSE:
 *			This function get http port number.
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          UI32_T - HTTP port value.
 * NOTES:
 *          default is tcp/80.
 */
UI32_T HTTP_MGR_Get_Http_Port();



/* FUNCTION	NAME : HTTP_MGR_GetOperationMode
 * PURPOSE:
 *		Get current http operation mode (master / slave / transition).
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
SYS_TYPE_Stacking_Mode_T HTTP_MGR_GetOperationMode(void);



/* FUNCTION NAME:  HTTP_MGR_Set_OpMode
 * PURPOSE:
 *          This function set http operation mode.
 *
 * INPUT:
 *          SYS_TYPE_Stacking_Mode_T - opmode.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          none.
 * NOTES:
 *          .
 */
void HTTP_MGR_Set_OpMode (SYS_TYPE_Stacking_Mode_T opmode);



/* FUNCTION NAME:  HTTP_MGR_Get_OpMode
 * PURPOSE:
 *          This function get http operation mode.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          SYS_TYPE_Stacking_Mode_T - opmode.
 * NOTES:
 *          .
 */
SYS_TYPE_Stacking_Mode_T HTTP_MGR_Get_OpMode ();



/* FUNCTION NAME:  HTTP_MGR_GetRunningHttpParameters
 * PURPOSE:
 *			This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
 *          specific http parameters with non-default values
 *          can be retrieved successfully. Otherwise, SYS_TYPE_GET_RUNNING_CFG_FAIL
 *          or SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          http_cfg - structure containing changed of status and non-defalut value.
 *
 * RETURN:
 *          SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES:
 *          1. This function shall only be invoked by CLI to save the
 *             "running configuration" to local or remote files.
 *          2. Since only non-default configuration will be saved, this
 *             function shall return non-default structure for each field for the device.
 */
UI32_T HTTP_MGR_GetRunningHttpParameters(HTTP_MGR_RunningCfg_T *http_cfg);



/* FUNCTION NAME:  HTTP_MGR_Get_Running_Http_Port
 * PURPOSE:
 *			This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
 *          specific http port with non-default values
 *          can be retrieved successfully. Otherwise, SYS_TYPE_GET_RUNNING_CFG_FAIL
 *          or SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          UI32_T * - HTTP port number.
 *
 * RETURN:
 *          SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES:
 *          1. This function shall only be invoked by CLI to save the
 *             "running configuration" to local or remote files.
 *          2. Since only non-default configuration will be saved, this
 *             function shall return non-default structure for each field for the device.
 */
SYS_TYPE_Get_Running_Cfg_T  HTTP_MGR_Get_Running_Http_Port(UI32_T *port);



/* FUNCTION NAME:  HTTP_MGR_Get_Running_Http_Status
 * PURPOSE:
 *			This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
 *          specific http state with non-default values
 *          can be retrieved successfully. Otherwise, SYS_TYPE_GET_RUNNING_CFG_FAIL
 *          or SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          UI32_T * - HTTP state.
 *
 * RETURN:
 *          SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES:
 *          1. This function shall only be invoked by CLI to save the
 *             "running configuration" to local or remote files.
 *          2. Since only non-default configuration will be saved, this
 *             function shall return non-default structure for each field for the device.
 */
SYS_TYPE_Get_Running_Cfg_T  HTTP_MGR_Get_Running_Http_Status(UI32_T *status);



/* FUNCTION NAME:  HTTP_MGR_AllocateConnectionObject
 * PURPOSE:
 *          Allocate a connection object
 *
 * INPUT:
 *          sockfd  -- socket ID
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          Connection object
 * NOTES:
 *          .
 */
void * HTTP_MGR_AllocateConnectionObject(int sockfd);



/* FUNCTION NAME:  HTTP_MGR_FreeConnectionObject
 * PURPOSE:
 *          Free a connection object
 *
 * INPUT:
 *          conn_obj_p  -- A connection object
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE    -- successed;
 *          FALSE   -- connection object is invalid
 * NOTES:
 *          .
 */
BOOL_T HTTP_MGR_FreeConnectionObject(HTTP_Connection_T *conn_obj_p);


/* FUNCTION NAME:  HTTP_MGR_GetNextConnectionObjectAtIndex
 * PURPOSE:
 *          Enumerates all connection objects
 *
 * INPUT:
 *          index  -- index. Use 0xffffffff to get first object.
 *
 * OUTPUT:
 *          http_connection -- connection object
 *
 * RETURN:
 *          TRUE/FALSE
 * NOTES:
 *          .
 */
BOOL_T HTTP_MGR_GetNextConnectionObjectAtIndex(UI32_T *index, HTTP_Connection_T *http_connection);



/*-----------------------------------------------------------------------*/





/*-----------------------------------------------------------------------*/
#if (SYS_CPNT_HTTPS == TRUE)

/* FUNCTION NAME:  HTTP_MGR_Set_Secure_Port
 * PURPOSE:
 *          This function set security port number.
 *
 * INPUT:
 *          UI32_T - HTTPS port number.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate changed an FALSE to indicate not changed.
 * NOTES:
 *          .
 */
BOOL_T HTTP_MGR_Set_Secure_Port (UI32_T port);



/* FUNCTION NAME:  HTTP_MGR_Get_Secure_Port
 * PURPOSE:
 *			This function get security port number.
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          UI32_T * - HTTPS port value.
 *
 * RETURN:
 *          TRUE.
 * NOTES:
 *          default is tcp/443.
 */
BOOL_T HTTP_MGR_Get_Secure_Port(UI32_T *port);



/* FUNCTION NAME:  HTTP_MGR_Set_Secure_Http_Status
 * PURPOSE:
 *          This function set https state.
 *
 * INPUT:
 *          SECURE_HTTP_STATE_T - HTTPS status.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE.
 * NOTES:
 *          .
 */
BOOL_T HTTP_MGR_Set_Secure_Http_Status (SECURE_HTTP_STATE_T state);



/* FUNCTION NAME:  HTTP_MGR_Get_Secure_Http_Status
 * PURPOSE:
 *          This function get https state.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          SECURE_HTTP_STATE_T - HTTPS status.
 * NOTES:
 *          .
 */
SECURE_HTTP_STATE_T HTTP_MGR_Get_Secure_Http_Status();



/* FUNCTION NAME:  HTTP_MGR_Get_SSL_CTX
 * PURPOSE:
 *          Get SSL Context object pointer.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          void * - SSL_CTX object.
 * NOTES:
 *          .
 */
void *HTTP_MGR_Get_SSL_CTX();




#if 0
/* FUNCTION NAME:  HTTP_MGR_Set_SSL_Session_Cache_Timeout
 * PURPOSE:
 *			Set SSl Session cache timeout .
 *
 * INPUT:
 *          UI32_T  -- Timeout value.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE.
 * NOTES:
 *          .
 */
BOOL_T HTTP_MGR_Set_SSL_Session_Cache_Timeout(UI32_T t);
#endif

/* FUNCTION NAME:  HTTP_MGR_Get_Running_Http_Secure_Port
 * PURPOSE:
 *			This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
 *          specific http secure port with non-default values
 *          can be retrieved successfully. Otherwise, SYS_TYPE_GET_RUNNING_CFG_FAIL
 *          or SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          UI32_T * - HTTPS port number.
 *
 * RETURN:
 *          SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES:
 *          1. This function shall only be invoked by CLI to save the
 *             "running configuration" to local or remote files.
 *          2. Since only non-default configuration will be saved, this
 *             function shall return non-default structure for each field for the device.
 */
SYS_TYPE_Get_Running_Cfg_T  HTTP_MGR_Get_Running_Http_Secure_Port(UI32_T *port);



/* FUNCTION NAME:  HTTP_MGR_Get_Running_Secure_Http_Status
 * PURPOSE:
 *			This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
 *          specific https state with non-default values
 *          can be retrieved successfully. Otherwise, SYS_TYPE_GET_RUNNING_CFG_FAIL
 *          or SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          UI32_T * - HTTPS state.
 *
 * RETURN:
 *          SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES:
 *          1. This function shall only be invoked by CLI to save the
 *             "running configuration" to local or remote files.
 *          2. Since only non-default configuration will be saved, this
 *             function shall return non-default structure for each field for the device.
 */
SYS_TYPE_Get_Running_Cfg_T  HTTP_MGR_Get_Running_Secure_Http_Status(UI32_T *status);



/* FUNCTION NAME:  HTTP_MGR_Get_Running_SSL_Session_Cache_Timeout
 * PURPOSE:
 *			This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
 *          specific SSL session cache timeout with non-default values
 *          can be retrieved successfully. Otherwise, SYS_TYPE_GET_RUNNING_CFG_FAIL
 *          or SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          UI32_T * - SSL session cache timeout.
 *
 * RETURN:
 *          SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES:
 *          1. This function shall only be invoked by CLI to save the
 *             "running configuration" to local or remote files.
 *          2. Since only non-default configuration will be saved, this
 *             function shall return non-default structure for each field for the device.
 */
SYS_TYPE_Get_Running_Cfg_T  HTTP_MGR_Get_Running_SSL_Session_Cache_Timeout(UI32_T *timeout);




/* FUNCTION NAME:  HTTP_MGR_Get_Certificate_Status
 * PURPOSE:
 *          This function to get flag of certificate status.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate certificate have changed;
 *          FALSE to indicate not changed.
 * NOTES:
 *          .
 */
BOOL_T HTTP_MGR_Get_Certificate_Status();



/* FUNCTION NAME:  HTTP_MGR_Get_Certificate
 * PURPOSE:
 *          This function to read server certificate and private key to ssl_ctx.
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
 *          .
 */
BOOL_T HTTP_MGR_Get_Certificate();



/* FUNCTION NAME:  HTTP_MGR_Enable_Debug_Information
 * PURPOSE:
 *			This function to enable generate debug information .
 *
 * INPUT:
 *          HTTP_MGR_DebugType_T - Debug type.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T HTTP_MGR_Enable_Debug_Information(HTTP_MGR_DebugType_T debug_type);



/* FUNCTION NAME:  HTTP_MGR_Disable_Debug_Information
 * PURPOSE:
 *			This function to disable generate debug information .
 *
 * INPUT:
 *          HTTP_MGR_DebugType_T - Debug type.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T HTTP_MGR_Disable_Debug_Information(HTTP_MGR_DebugType_T debug_type);



/* FUNCTION NAME:  HTTP_MGR_Get_Running_Debug_Information_Status
 * PURPOSE:
 *			This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
 *          specific Debug type with non-disable values
 *          can be retrieved successfully. Otherwise, SYS_TYPE_GET_RUNNING_CFG_FAIL
 *          or SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          HTTP_MGR_DebugType_T * - debug type.
 *
 * RETURN:
 *          SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES:
 *          1. This function shall only be invoked by CLI to save the
 *             "running configuration" to local or remote files.
 *          2. Since only non-default configuration will be saved, this
 *             function shall return non-default structure for each field for the device.
 */
SYS_TYPE_Get_Running_Cfg_T  HTTP_MGR_Get_Running_Debug_Information_Status(HTTP_MGR_DebugType_T *debug_type);



/* FUNCTION NAME:  HTTP_MGR_Get_Debug_Session_Status
 * PURPOSE:
 *			This function to get debug session status .
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          HTTPS_DEBUG_SESSION_STATE_T - Debug Session status.
 * NOTES:
 *          .
 */
HTTPS_DEBUG_SESSION_STATE_T HTTP_MGR_Get_Debug_Session_Status();



/* FUNCTION NAME:  HTTP_MGR_Get_Debug_State_Status
 * PURPOSE:
 *			This function to get debug state status .
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          HTTPS_DEBUG_STATE_STATE_T - Debug State status.
 * NOTES:
 *          .
 */
HTTPS_DEBUG_STATE_STATE_T HTTP_MGR_Get_Debug_State_Status();



/* FUNCTION NAME : HTTP_MGR_Get_Certificate_Info
 * PURPOSE:
 *      Get HTTPS Certificate Info.
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      HTTP_CertificateInfo_T  * -- HTTPS certificate info.
 *
 * RETURN:
 *      TRUE  - The output value is https certifiacte info.
 *      FALSE - The output value is invalid.
 *
 * NOTES:
 *      This function invoked in UI.
 */
BOOL_T HTTP_MGR_Get_Certificate_Info(HTTP_CertificateInfo_T *info);



/* FUNCTION NAME:  HTTP_MGR_Set_Auto_Redirect_To_Https_Status
 * PURPOSE:
 *          This function set state of http redirect to https.
 *
 * INPUT:
 *          HTTP_Redirect_Status_T status    --  http redirect to https status.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE.
 * NOTES:
 *          .
 */
BOOL_T HTTP_MGR_Set_Auto_Redirect_To_Https_Status(HTTP_Redirect_Status_T status);



/* FUNCTION NAME:  HTTP_MGR_Get_Auto_Redirect_To_Https_Status
 * PURPOSE:
 *          This function get state of http redirect to https.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          HTTP_Redirect_Status_T *status   --  http redirect to https status.
 *
 * RETURN:
 *          TRUE.
 * NOTES:
 *          .
 */
BOOL_T HTTP_MGR_Get_Auto_Redirect_To_Https_Status(HTTP_Redirect_Status_T *status);



/* FUNCTION NAME:  HTTP_MGR_Get_Running_Auto_Redirect_To_Https_Status
 * PURPOSE:
 *			This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
 *          specific https state with non-default values
 *          can be retrieved successfully. Otherwise, SYS_TYPE_GET_RUNNING_CFG_FAIL
 *          or SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          HTTP_Redirect_Status_T *status   --  http redirect to https status.
 *
 * RETURN:
 *          SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES:
 *          1. This function shall only be invoked by CLI to save the
 *             "running configuration" to local or remote files.
 *          2. Since only non-default configuration will be saved, this
 *             function shall return non-default structure for each field for the device.
 */
SYS_TYPE_Get_Running_Cfg_T  HTTP_MGR_Get_Running_Auto_Redirect_To_Https_Status(HTTP_Redirect_Status_T *status);



/* FUNCTION NAME:  HTTP_MGR_Set_Tftp_Ip
 * PURPOSE:
 *          This function set tftp server to download certificate.
 *
 * INPUT:
 *          L_INET_AddrIp_T  *tftp_server --  tftp server.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE    --  successful.
 *          FALSE   --  failure.
 * NOTES:
 *          .
 */
BOOL_T HTTP_MGR_Set_Tftp_Ip(L_INET_AddrIp_T *tftp_server);



/* FUNCTION NAME:  HTTP_MGR_Get_Tftp_Ip
 * PURPOSE:
 *          This function get tftp server will be download certificate.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          L_INET_AddrIp_T  *tftp_server    --  tftp server.
 *
 * RETURN:
 *          TRUE    --  successful.
 *          FALSE   --  failure.
 * NOTES:
 *          .
 */
BOOL_T HTTP_MGR_Get_Tftp_Ip(L_INET_AddrIp_T *tftp_server);



/* FUNCTION NAME:  HTTP_MGR_Set_Tftp_Certificate_Filename
 * PURPOSE:
 *          This function set certificate flename to be download from tftp server.
 *
 * INPUT:
 *          char   *tftp_cert_file --  certificate filename.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE    --  successful.
 *          FALSE   --  failure.
 * NOTES:
 *          .
 */
BOOL_T HTTP_MGR_Set_Tftp_Certificate_Filename(const char *tftp_cert_file);



/* FUNCTION NAME:  HTTP_MGR_Get_Tftp_Certificate_Filename
 * PURPOSE:
 *          This function get certificate flename to be download from tftp server.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          char   *tftp_cert_file --  certificate filename.
 *
 * RETURN:
 *          TRUE    --  successful.
 *          FALSE   --  failure.
 * NOTES:
 *          .
 */
BOOL_T HTTP_MGR_Get_Tftp_Certificate_Filename(char *tftp_cert_file);



/* FUNCTION NAME:  HTTP_MGR_Set_Tftp_Privatekey_Filename
 * PURPOSE:
 *          This function set privatekey flename to be download from tftp server.
 *
 * INPUT:
 *          const char *tftp_private_file  --  privatekey filename.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE    --  successful.
 *          FALSE   --  failure.
 * NOTES:
 *          .
 */
BOOL_T HTTP_MGR_Set_Tftp_Privatekey_Filename(const char *tftp_private_file);



/* FUNCTION NAME:  HTTP_MGR_Get_Tftp_Privatekey_Filename
 * PURPOSE:
 *          This function get privatekey flename to be download from tftp server.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          char   *tftp_private_file  --  privatekey filename.
 *
 * RETURN:
 *          TRUE    --  successful.
 *          FALSE   --  failure.
 * NOTES:
 *          .
 */
BOOL_T HTTP_MGR_Get_Tftp_Privatekey_Filename(char *tftp_private_file);



/* FUNCTION NAME:  HTTP_MGR_Set_Tftp_Privatekey_Password
 * PURPOSE:
 *          This function set privatekey password to be download from tftp server.
 *
 * INPUT:
 *          const char *tftp_private_password  --  privatekey password.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE    --  successful.
 *          FALSE   --  failure.
 * NOTES:
 *          .
 */
BOOL_T HTTP_MGR_Set_Tftp_Privatekey_Password(const char *tftp_private_password);



/* FUNCTION NAME:  HTTP_MGR_Get_Tftp_Privatekey_Password
 * PURPOSE:
 *          This function get privatekey password to be download from tftp server.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          char   *tftp_private_password  --  privatekey password.
 *
 * RETURN:
 *          TRUE    --  successful.
 *          FALSE   --  failure.
 * NOTES:
 *          .
 */
BOOL_T HTTP_MGR_Get_Tftp_Privatekey_Password(char *tftp_private_password);



/* FUNCTION NAME:  HTTP_MGR_Set_Tftp_Download_Certificate_Flag
 * PURPOSE:
 *          This function set a flag to protect only one web user seting parameter
 *          for download certificate file.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE    --  Indicate user can go on to set parameters.
 *          FALSE   --  Indicate have another user keep those parameters.
 * NOTES:
 *          .
 */
BOOL_T HTTP_MGR_Set_Tftp_Download_Certificate_Flag();



/* FUNCTION NAME:  HTTP_MGR_Reset_Tftp_Download_Certificate_Flag
 * PURPOSE:
 *          This function reset a flag to allow other web user seting parameter
 *          for download certificate file.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE.
 * NOTES:
 *          .
 */
BOOL_T HTTP_MGR_Reset_Tftp_Download_Certificate_Flag();



/* FUNCTION NAME:  HTTP_MGR_Set_Certificate_Download_Status
 * PURPOSE:
 *          This function set certificate download status.
 *
 * INPUT:
 *          HTTP_GetCertificateStatus_T status  --  certificate download status.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE    --  successful.
 *          FALSE   --  failure.
 * NOTES:
 *          .
 */
BOOL_T HTTP_MGR_Set_Certificate_Download_Status(HTTP_GetCertificateStatus_T status);



/* FUNCTION NAME:  HTTP_MGR_Get_Certificate_Download_Status
 * PURPOSE:
 *          This function get certificate download status.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          HTTP_GetCertificateStatus_T *status --  certificate download status.
 *
 * RETURN:
 *          TRUE    --  successful.
 *          FALSE   --  failure.
 * NOTES:
 *          .
 */
BOOL_T HTTP_MGR_Get_Certificate_Download_Status(HTTP_GetCertificateStatus_T *status);



/* FUNCTION NAME:  HTTP_MGR_Get_Certificate_From_Tftp
 * PURPOSE:
 *          A server certificate authenticates the server to the client.
 *			This function to save server certificate ,
 *          when you got a server certificate from a certification authority(CA).
 *
 * INPUT:
 *          certificate_entry_p  -- certificate entry
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          User can use TFTP or XModem to upload server certificate
 *
 *          This function is invoked in WEB or SNMP.
 */
BOOL_T
HTTP_MGR_Get_Certificate_From_Tftp(
    HTTP_DownloadCertificateEntry_T *certificate_entry_p
);

/* FUNCTION NAME:  HTTP_MGR_XferCopy_Callback
 * PURPOSE:
 *          This function is callback from XFER.
 *			Note xfer result to HTTP.
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
 *          This function is callback from XFER.
 */
void HTTP_MGR_XferCopy_Callback(void *cookie, UI32_T status);



/* FUNCTION NAME:  HTTP_MGR_Set_XferProgressStatus
 * PURPOSE:
 *          This function set status of xfer in progress.
 *
 * INPUT:
 *          BOOL_T  status  --  status of xfer in progress.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          none.
 * NOTES:
 *          .
 */
void HTTP_MGR_Set_XferProgressStatus(BOOL_T status);



/* FUNCTION NAME:  HTTP_MGR_Get_XferProgressStatus
 * PURPOSE:
 *          This function get status of xfer in progress.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          BOOL_T  *status --  status of xfer in progress.
 *
 * RETURN:
 *          none.
 * NOTES:
 *          .
 */
void HTTP_MGR_Get_XferProgressStatus(BOOL_T *status);



/* FUNCTION NAME:  HTTP_MGR_Set_XferProgressResult
 * PURPOSE:
 *          This function set xfer process result.
 *
 * INPUT:
 *          BOOL_T  result  --  xfer process result.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          none.
 * NOTES:
 *          .
 */
void HTTP_MGR_Set_XferProgressResult(BOOL_T result);



/* FUNCTION NAME:  HTTP_MGR_Get_XferProgressResult
 * PURPOSE:
 *          This function get xfer process result.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          BOOL_T  *result --  status of xfer in progress.
 *
 * RETURN:
 *          none.
 * NOTES:
 *          .
 */
void HTTP_MGR_Get_XferProgressResult(BOOL_T *result);



















//#endif /* ifdef SUPPORT_DEBUG_IP_HTTP_SECURE */
/*-----------------------------------------------------------------------*/

#endif /* if (SYS_CPNT_HTTPS == TRUE) */
/*-----------------------------------------------------------------------*/



/*isiah.2003-06-12. add for show web connection.*/
/* FUNCTION NAME:  HTTP_MGR_Set_User_Connection_Info
 * PURPOSE:
 *          This function set user connection info.
 *
 * INPUT:
 *          void *req   --  request struct of http connection.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE.
 * NOTES:
 *          .
 */
BOOL_T HTTP_MGR_Set_User_Connection_Info(void *req);


/* FUNCTION NAME:  HTTP_MGR_Get_User_Connection_Info
 * PURPOSE:
 *          This function get user connection info.
 *
 * INPUT:
 *          void *req   --  request struct of http connection.
 *
 * OUTPUT:
 *          user_conn_info_p   -- user connection information
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T HTTP_MGR_Get_User_Connection_Info(void *req, HTTP_Session_T *session);


/* FUNCTION NAME:  HTTP_MGR_Check_User_Connection_Info
 * PURPOSE:
 *          This function check current connection info.
 *
 * INPUT:
 *          void *req   --  request struct of http connection.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T HTTP_MGR_Check_User_Connection_Info(void *req);



/* FUNCTION NAME:  HTTP_MGR_Delete_User_Connection_Info
 * PURPOSE:
 *          This function delete current connection info.
 *
 * INPUT:
 *          void *req   --  request struct of http connection.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T HTTP_MGR_Delete_User_Connection_Info(void *req);

/* FUNCTION NAME:  HTTP_MGR_Set_User_Connection_Send_Log
 * PURPOSE:
 *          This function set current connection info.
 *
 * INPUT:
 *          L_INET_AddrIp_T  ip_addr     --  remote ip address.
 *          const char      *username    --  remote user.
 *          BOOL_T           is_send_log -- send log flag.
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T HTTP_MGR_Set_User_Connection_Send_Log(L_INET_AddrIp_T ip_addr, const char *username, BOOL_T is_send_log);


/* FUNCTION NAME:  HTTP_MGR_Get_Next_User_Connection_Info
 * PURPOSE:
 *          This function get next connection info.
 *
 * INPUT:
 *          HTTP_Session_T   *connection_info    --  previous active connection information.
 *
 * OUTPUT:
 *          HTTP_Session_T   *connection_info    --  current active connection information.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *      This function invoked in CLI command "show user".
 *      Initial input value is zero.
 */
BOOL_T HTTP_MGR_Get_Next_User_Connection_Info(HTTP_Session_T *session);


/*------------------------------------------------------------------------
 * ROUTINE NAME - HTTP_MGR_SendLoginLogoutTrap
 *------------------------------------------------------------------------
 * FUNCTION: Send login/logout trap
 * INPUT   : trap_type        -- trap type is login or logout
 *           http_connection  -- http connection struct
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 *------------------------------------------------------------------------
 */
BOOL_T
HTTP_MGR_SendLoginLogoutTrap(
    UI32_T trap_type,
    HTTP_Connection_T *http_connection
);


/* FUNCTION NAME - HTTP_MGR_HandleHotInsertion
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
void HTTP_MGR_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default);



/* FUNCTION NAME - HTTP_MGR_HandleHotRemoval
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
void HTTP_MGR_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port);

/* FUNCTION NAME : HTTP_MGR_RifDestroyedCallbackHandler
 * PURPOSE:
 *      Process when the rif destroy (IP address was changed)
 *
 * INPUT:
 *      ip_addr_p  -- the ip address which is down
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
void HTTP_MGR_RifDestroyedCallbackHandler(L_INET_AddrIp_T *ip_addr_p);

#if (SYS_CPNT_CLUSTER == TRUE)
/* FUNCTION NAME:  HTTP_MGR_Set_Cluster_Port
 * PURPOSE:
 *          This function set cluster port number.
 *
 * INPUT:
 *          UI32_T - Cluster port number.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          none.
 * NOTES:
 *          .
 */
BOOL_T HTTP_MGR_Set_Cluster_Port(UI32_T port);


/* FUNCTION NAME:  HTTP_MGR_Get_Cluster_Port
 * PURPOSE:
 *          This function get cluster port number.
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          UI32_T - Cluster port value.
 * NOTES:
 *          default is tcp/80.
 */
UI32_T HTTP_MGR_Get_Cluster_Port();
#endif /* SYS_CPNT_CLUSTER */


/* FUNCTION NAME:  HTTP_Init_SSL_Resources
 * PURPOSE:
 *          Initiate SSL Resources -- Registers the available ciphers and digests,
 *          read certificate files from file system, create a new SSL_CTX object
 *          read server certificate and private key into SSL_CTX, initialize session cache,
 *          and hook function into SSL_CTX.
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
 *          .
 */
BOOL_T HTTP_Init_SSL_Resources();

/*------------------------------------------------------------------------------
 * ROUTINE NAME : HTTP_MGR_HandleIPCReqMsg
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Handle the ipc request message for http mgr.
 * INPUT:
 *    ipcmsg_p  --  input request ipc message buffer
 *
 * OUTPUT:
 *    ipcmsg_p  --  output response ipc message buffer
 *
 * RETURN:
 *    TRUE  - There is a response need to send.
 *    FALSE - There is no response to send.
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T HTTP_MGR_HandleIPCReqMsg(SYSFUN_Msg_T* ipcmsg_p);

/* FUNCTION NAME : HTTP_MGR_RifDestroyCallbackHandler
 * PURPOSE:
 *      Process when the rif destroy (IP address was changed)
 *
 * INPUT:
 *      ip_addr_p  -- the ip address which is down
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
void HTTP_MGR_RifDestroyCallbackHandler(L_INET_AddrIp_T *ip_addr_p);

/* FUNCTION NAME:  HTTP_MGR_MapUriToPath
 * PURPOSE:
 *          Map uri to file path.
 *
 * INPUT:
 *          const char *uri -- URI
 *
 * OUTPUT:
 *          None
 *
 * RETURN:
 *          Return file path that allocated by malloc() or
 *          return NULL if operation failed.
 *
 * NOTES:
 *          1. The path for REST API should not be change by alias.
 *          2. All alias path will be replaced.
 *          3. Nonmatch URI will be replaced by root.
 */
char *HTTP_MGR_MapUriToPath(const char *uri);

/* FUNCTION NAME:  HTTP_MGR_LoadConfig
 * PURPOSE:
 *          Load config file from file_path.
 *
 * INPUT:
 *          file_path -- the config file path
 *
 * OUTPUT:
 *          None
 *
 * RETURN:
 *          TRUE / FALSE
 *
 * NOTES:
 *          None
 */
BOOL_T HTTP_MGR_LoadConfig(const char *file_path);

/* FUNCTION NAME:  HTTP_MGR_GetConfigValue
 * PURPOSE:
 *          Get config by key.
 *
 * INPUT:
 *          key
 *
 * OUTPUT:
 *          None
 *
 * RETURN:
 *          config value
 *
 * NOTES:
 *          None
 */
void *HTTP_MGR_GetConfigValue(const char *key);

#if __cplusplus
}
#endif

#endif /* end HTTP_MGR_H */
