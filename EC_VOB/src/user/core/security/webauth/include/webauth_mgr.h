/* MODULE NAME:  WEBAUTH_MGR.H
 * PURPOSE:  This package provides the webauth mgr functions .
 *
 * NOTE:
 *
 * HISTORY:
 * 02/05/2007     --  Rich Lee , Create
 *
 *
 * Copyright(C)       Accton Corporation, 2007
 */

#ifndef _WEBAUTH_MGR_H
#define _WEBAUTH_MGR_H


/* INCLUDE FILE DECLARATTIONS
 */
#include <stddef.h>
#include <sys_type.h>
#include "sys_adpt.h"
#include "sys_cpnt.h"
#include "sysfun.h"
#include "webauth_type.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* The key to get networkaccess mgr msgq.
 */
#define WEBAUTH_MGR_IPCMSG_KEY  SYS_BLD_WEB_GROUP_IPCMSGQ_KEY

/* The commands for IPC message.
 */
enum {
    WEBAUTH_MGR_IPC_CMD_SET_SYSTEM_STATUS,
    WEBAUTH_MGR_IPC_CMD_GET_SYSTEM_STATUS,
    WEBAUTH_MGR_IPC_CMD_SET_STATUS,
    WEBAUTH_MGR_IPC_CMD_GET_STATUS,
    WEBAUTH_MGR_IPC_CMD_SET_QUIET_PERIOD,
    WEBAUTH_MGR_IPC_CMD_GET_QUIET_PERIOD,
    WEBAUTH_MGR_IPC_CMD_SET_MAX_LOGIN_ATTEMPTS,
    WEBAUTH_MGR_IPC_CMD_GET_MAX_LOGIN_ATTEMPTS,
    WEBAUTH_MGR_IPC_CMD_SET_SYSTEM_SESSION_TIMEOUT,
    WEBAUTH_MGR_IPC_CMD_GET_SYSTEM_SESSION_TIMEOUT,
    WEBAUTH_MGR_IPC_CMD_SET_EXTERNAL_URL,
    WEBAUTH_MGR_IPC_CMD_GET_EXTERNAL_URL,
    WEBAUTH_MGR_IPC_CMD_REAUTH_BY_LPORT,
    WEBAUTH_MGR_IPC_CMD_REAUTH_HOST_BY_LPORT,
    WEBAUTH_MGR_IPC_CMD_GET_RUNNING_SESSION_TIMEOUT,
    WEBAUTH_MGR_IPC_CMD_GET_RUNNING_QUIET_PERIOD,
    WEBAUTH_MGR_IPC_CMD_GET_RUNNING_MAX_LOGIN_ATTEMPT,
    WEBAUTH_MGR_IPC_CMD_GET_RUNNING_SYSTEM_STATUS,
    WEBAUTH_MGR_IPC_CMD_GET_RUNNING_STATUS,
    WEBAUTH_MGR_IPC_CMD_GET_RUNNING_LOGIN_URL,
    WEBAUTH_MGR_IPC_CMD_GET_RUNNING_LOGIN_FAIL_URL,
    WEBAUTH_MGR_IPC_CMD_GET_RUNNING_LOGIN_SUCCESS_URL,
    WEBAUTH_MGR_IPC_CMD_GETNEXT_SUCCESS_HOST_BY_LPORT,
    WEBAUTH_MGR_IPC_CMD_CREATE_SUCCESS_HOST_BY_LPORT,
    WEBAUTH_MGR_IPC_CMD_DELETE_SUCCESS_LIST_BY_HOST_IP,
    WEBAUTH_MGR_IPC_CMD_CREATE_BLACK_HOST_BY_LPORT,
    WEBAUTH_MGR_IPC_CMD_CHECK_IP_IS_BLACK_BY_LPORT,
    WEBAUTH_MGR_IPC_CMD_CREATE_TRYING_HOST,
    WEBAUTH_MGR_IPC_CMD_DELETE_BLACK_LIST_BY_HOST_IP,
    WEBAUTH_MGR_IPC_CMD_DELETE_TRYING_HOST,
    WEBAUTH_MGR_IPC_CMD_GET_PORT_INFO_BY_LPORT,
    WEBAUTH_MGR_IPC_CMD_GETNEXT_PORT_INFO_BY_LPORT,
    WEBAUTH_MGR_IPC_CMD_GET_SUCCESS_HOST_BY_LPORT_AND_IP,
    WEBAUTH_MGR_IPC_CMD_PROCESS_AUTH,
    WEBAUTH_MGR_IPC_CMD_PROCESS_TIMER_EVENT,
    WEBAUTH_MGR_IPC_CMD_GETNEXT_SUCCESS_HOST_BY_LPORT_AND_IP,
    WEBAUTH_MGR_IPC_CMD_GET_SUCCESS_HOST_BY_LPORT,
    WEBAUTH_MGR_IPC_CMD_GET_SUCCESS_COUNT_BY_LPORT,
    WEBAUTH_MGR_IPC_CMD_GET_TRYING_HOST_COUNT,
    WEBAUTH_MGR_IPC_CMD_SET_DEBUG_FLAG,
    WEBAUTH_MGR_IPC_CMD_GET_DEBUG_FLAG,
    WEBAUTH_MGR_IPC_CMD_GET_WEBAUTH_INFO,
    WEBAUTH_MGR_IPC_CMD_IS_IP_VALID_BY_LPORT,
    WEBAUTH_MGR_IPC_CMD_GETNEXT_SUCCESS_HOST_BY_LPORT_AND_SUCC_INDEX,
};

/* MACRO FUNCTION DECLARATIONS
 */
/*-------------------------------------------------------------------------
 * MACRO NAME - WEBAUTH_MGR_GET_MSGBUFSIZE
 *-------------------------------------------------------------------------
 * PURPOSE : Get the size of WEBAUTH message that contains the specified
 *           type of data.
 * INPUT   : type_name - the type name of data for the message.
 * OUTPUT  : None
 * RETURN  : The size of WEBAUTH message.
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
#define WEBAUTH_MGR_GET_MSGBUFSIZE(type_name) \
    (offsetof(WEBAUTH_MGR_IPCMsg_T, data)+ sizeof(type_name))

/*-------------------------------------------------------------------------
 * MACRO NAME - WEBAUTH_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA
 *-------------------------------------------------------------------------
 * PURPOSE : Get the size of WEBAUTH message that has no data block.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : The size of WEBAUTH message.
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
#define WEBAUTH_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA() \
    sizeof(WEBAUTH_MGR_IPCMsg_Type_T)

/*-------------------------------------------------------------------------
 * MACRO NAME - WEBAUTH_MGR_MSG_CMD
 *              WEBAUTH_MGR_MSG_RETVAL
 *-------------------------------------------------------------------------
 * PURPOSE : Get the WEBAUTH command/return value of an IPC message.
 * INPUT   : msg_p - the IPC message.
 * OUTPUT  : None
 * RETURN  : The WEBAUTH command/return value; it's allowed to be used as lvalue.
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
#define WEBAUTH_MGR_MSG_CMD(msg_p)    (((WEBAUTH_MGR_IPCMsg_T *)msg_p->msg_buf)->type.cmd)
#define WEBAUTH_MGR_MSG_RETVAL(msg_p) (((WEBAUTH_MGR_IPCMsg_T *)msg_p->msg_buf)->type.result)

/*-------------------------------------------------------------------------
 * MACRO NAME - WEBAUTH_MGR_MSG_DATA
 *-------------------------------------------------------------------------
 * PURPOSE : Get the data block of an IPC message.
 * INPUT   : msg_p - the IPC message.
 * OUTPUT  : None
 * RETURN  : The pointer of the data block.
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
#define WEBAUTH_MGR_MSG_DATA(msg_p)   ((void *)&((WEBAUTH_MGR_IPCMsg_T *)msg_p->msg_buf)->data)

/* DATA TYPE DECLARATIONS
 */

 typedef struct
{
    UI32_T u32;
} WEBAUTH_MGR_IPCMsg_U32_Data_T;

typedef struct
{
    UI32_T u32_1;
    UI32_T u32_2;
} WEBAUTH_MGR_IPCMsg_U32_U32_Data_T;

typedef struct
{
    UI32_T u32_1;
    UI32_T u32_2;
    UI32_T u32_3;
} WEBAUTH_MGR_IPCMsg_U32_U32_U32_Data_T;

typedef struct
{
    char url[WEBAUTH_TYPE_MAX_URL_LENGTH + 1];
    WEBAUTH_TYPE_EXTERNAL_URL_T url_type;
} WEBAUTH_MGR_IPCMsg_ExternelUrl_T;

typedef struct
{
    WEBAUTH_TYPE_Host_Info_T host_info;
    UI32_T u32_1;
    UI32_T u32_2;
} WEBAUTH_MGR_IPCMsg_HostInfo_U32_U32_T;

typedef struct
{
    WEBAUTH_TYPE_Port_Info_T port_info;
    UI32_T u32;
} WEBAUTH_MGR_IPCMsg_PortInfo_U32_T;

typedef struct
{
    char username[SYS_ADPT_MAX_USER_NAME_LEN + 1];
    char password[SYS_ADPT_MAX_PASSWORD_LEN + 1];
    UI32_T u32_1;
    UI32_T u32_2;
} WEBAUTH_MGR_IPCMsg_Username_Password_U32_U32_T;

typedef struct
{
    WEBAUTH_TYPE_Host_Info_T host_info;
} WEBAUTH_MGR_IPCMsg_HostInfo_T;

typedef struct
{
    WEBAUTH_TYPE_System_Info_T system_info;
} WEBAUTH_MGR_IPCMsg_SystemInfo_T;

typedef union
{
    WEBAUTH_MGR_IPCMsg_U32_Data_T u32;

    WEBAUTH_MGR_IPCMsg_U32_U32_Data_T u32_u32;

    WEBAUTH_MGR_IPCMsg_U32_U32_U32_Data_T u32_u32_u32;

    WEBAUTH_MGR_IPCMsg_ExternelUrl_T external_url;

    WEBAUTH_MGR_IPCMsg_HostInfo_U32_U32_T host_info_u32_u32;

    WEBAUTH_MGR_IPCMsg_PortInfo_U32_T port_info_u32;

    WEBAUTH_MGR_IPCMsg_Username_Password_U32_U32_T username_password_u32_u32;

    WEBAUTH_MGR_IPCMsg_HostInfo_T host_info;

    WEBAUTH_MGR_IPCMsg_SystemInfo_T system_info;
} WEBAUTH_MGR_IPCMsg_Data_T;

/* Message declarations for IPC.
 */
typedef union
{
    UI32_T  cmd;    /* for sending IPC request. CSCA_MGR_IPC_CMD1 ... */
    UI32_T  result; /* for response */
} WEBAUTH_MGR_IPCMsg_Type_T;

typedef struct
{
    WEBAUTH_MGR_IPCMsg_Type_T type;
    WEBAUTH_MGR_IPCMsg_Data_T data;
} WEBAUTH_MGR_IPCMsg_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/* FUNCTION NAME: WEBAUTH_MGR_EnterMasterMode
 * PURPOSE: Enable WEBAUTH operation while in master mode.
 * INPUT:   none
 * OUTPUT:  none
 * RETURN:  none
 * NOTES:   none
 */
void WEBAUTH_MGR_EnterMasterMode(void);

/* FUNCTION NAME: WEBAUTH_MGR_EnterTransitionMode
 * PURPOSE: It's the temporary transition mode between system into master
 *          mode.
 * INPUT:   none
 * OUTPUT:  none
 * RETURN:  none
 * NOTES:   This function will call WEBAUTH_MGR_InitiateSystemResources
 *          to clear OM.
 */
void WEBAUTH_MGR_EnterTransitionMode(void);

/* FUNCTION NAME: WEBAUTH_MGR_EnterSlaveMode
 * PURPOSE: Disable the WEBAUTH operation while in slave mode.
 * INPUT:   none
 * OUTPUT:  none
 * RETURN:  none
 * NOTES:   none
 */
void WEBAUTH_MGR_EnterSlaveMode(void);

/* FUNCTION NAME: WEBAUTH_MGR_SetTransitionMode
 * PURPOSE: This function sets the component to temporary transition mode
 * INPUT:   none
 * OUTPUT:  none
 * RETURN:  none
 * NOTES:   none
 */
void  WEBAUTH_MGR_SetTransitionMode(void);

/* FUNCTION NAME - WEBAUTH_MGR_Initiate_System_Resources
 * PURPOSE: This function will clear OM of WEBAUTH
 * INPUT:   none
 * OUTPUT:  none
 * RETURN:  none
 * NOTES:   none
 */
void WEBAUTH_MGR_Initiate_System_Resources(void);

/* FUNCTION NAME:  WEBAUTH_MGR_ProvisionComplete
 * PURPOSE: This function will tell webauth that provision is completed
 * INPUT:   notify_flag
 * OUTPUT:  None
 * RETURN:  None
 * NOTE:    None
 */
void  WEBAUTH_MGR_ProvisionComplete(BOOL_T notify_flag);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - WEBAUTH_MGR_HandleHotInsertion
 * ------------------------------------------------------------------------
 * PURPOSE  : This function will initialize the port OM of the module ports
 *            when the option module is inserted.
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port
 *                                     inserted
 *            number_of_port        -- the number of ports on the inserted
 *                                     module
 *            use_default           -- the flag indicating the default
 *                                     configuration is used without further
 *                                     provision applied; TRUE if a new module
 *                                     different from the original one is
 *                                     inserted
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Only one module is inserted at a time.
 * ------------------------------------------------------------------------
 */
void WEBAUTH_MGR_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - WEBAUTH_MGR_HandleHotRemoval
 * ------------------------------------------------------------------------
 * PURPOSE  : This function will clear the port OM of the module ports when
 *            the option module is removed.
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port
 *                                     removed
 *            number_of_port        -- the number of ports on the removed
 *                                     module
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Only one module is removed at a time.
 * ------------------------------------------------------------------------
 */
void WEBAUTH_MGR_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port);

/* FUNCTION	NAME: WEBAUTH_MGR_GetOperationMode
 * PURPOSE: Get current webauth operation mode (master / slave / transition).
 * INPUT:   None.
 * OUTPUT:  None.
 * RETURN:  SYS_TYPE_Stacking_Mode_T - opmode.
 * NOTES:   None.
 */
SYS_TYPE_Stacking_Mode_T WEBAUTH_MGR_GetOperationMode(void);

/* FUNCTION NAME: WEBAUTH_MGR_SetSystemStatus
 * PURPOSE: This function will set global status of webauth
 * INPUT:   status
 * OUTPUT:  VAL_webauthEnable_enabled/VAL_webauthEnable_disabled
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none
 */
UI32_T WEBAUTH_MGR_SetSystemStatus(UI8_T status);

/* FUNCTION NAME: WEBAUTH_MGR_GetSystemStatus
 * PURPOSE: This function will return the status of webath
 * INPUT:   *status
 * OUTPUT:  VAL_webauthEnable_enabled/VAL_webauthEnable_disabled
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none
 */
UI32_T WEBAUTH_MGR_GetSystemStatus(UI8_T *status);

/* FUNCTION NAME: WEBAUTH_MGR_SetStatusByLPort
 * PURPOSE: this function will set webauth per port status
 * INPUT:   lport
 *          status
 * OUTPUT:  VAL_webAuthPortConfigStatus_enabled/VAL_webAuthPortConfigStatus_disabled
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTE:    none
 */
UI32_T WEBAUTH_MGR_SetStatusByLPort(UI32_T lport, UI8_T status);

/* FUNCTION NAME: WEBAUTH_MGR_GetStatusByLPort
 * PURPOSE: This function will return webauth per port status
 * INPUT:   lport
 * OUTPUT:  *status -- VAL_webAuthPortConfigStatus_enabled/VAL_webAuthPortConfigStatus_disabled
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none
 */
UI32_T WEBAUTH_MGR_GetStatusByLPort(UI32_T lport, UI8_T *status);

/* FUNCTION NAME: WEBAUTH_MGR_SetQuietPeriod
 * PURPOSE: this function will set quiet period
 * INPUT:   quiet_period
 * OUTPUT:  None
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTE:    None
 */
UI32_T WEBAUTH_MGR_SetQuietPeriod(UI16_T quiet_period);

/* FUNCTION NAME: WEBAUTH_MGR_GetQuietPeriod
 * PURPOSE: This function will return the quiet period value
 * INPUT:   none.
 * OUTPUT:  *quiet_period
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none
 */
UI32_T WEBAUTH_MGR_GetQuietPeriod(UI16_T *quiet_period);

/* FUNCTION NAME: WEBAUTH_MGR_SetMaxLoginAttempts
 * PURPOSE: this function will set max login attempts
 * INPUT:   max_login_attempt
 * OUTPUT:  None
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTE:    None
 */
UI32_T WEBAUTH_MGR_SetMaxLoginAttempts(UI8_T max_login_attempt);

/* FUNCTION NAME: WEBAUTH_MGR_GetMaxLoginAttempts
 * PURPOSE: This function will return global value of max login attempt
 * INPUT:   none
 * OUTPUT:  *max login attempt
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none
 */
UI32_T WEBAUTH_MGR_GetMaxLoginAttempts(UI8_T *max_login_attempt);

/* FUNCTION NAME: WEBAUTH_MGR_SetSystemSessionTimeout
 * PURPOSE: This function will set global session timeout value
 * INPUT:   session_timeout
 * OUTPUT:  None
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTE:    None
 */
UI32_T WEBAUTH_MGR_SetSystemSessionTimeout(UI16_T session_timeout);

/* FUNCTION NAME: WEBAUTH_MGR_GetSystemSessionTimeout
 * PURPOSE: This function will get global session timeout value
 * INPUT:   none.
 * OUTPUT:  *session_timeout
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none
 */
UI32_T WEBAUTH_MGR_GetSystemSessionTimeout(UI16_T *session_timeout);

/* FUNCTION NAME: WEBAUTH_MGR_SetExternalURL
 * PURPOSE: This function set external URL by it's type
 * INPUT:   *url,
 *          url type:
 *             {WEBAUTH_EXTERNAL_URL_Login
 *              WEBAUTH_EXTERNAL_URL_Login_fail
 *              WEBAUTH_EXTERNAL_URL_Login_Success}
 * OUTPUT:  none
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none
 */
UI32_T WEBAUTH_MGR_SetExternalURL(
        char *url, WEBAUTH_TYPE_EXTERNAL_URL_T url_type);

/* FUNCTION NAME: WEBAUTH_MGR_GetExternalURL
 * PURPOSE: This function get external URL by it's type
 * INPUT:   url type:
 *             {WEBAUTH_EXTERNAL_URL_Login
 *              WEBAUTH_EXTERNAL_URL_Login_fail
 *              WEBAUTH_EXTERNAL_URL_Login_Success}
 * OUTPUT:  *url
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none
 */
UI32_T WEBAUTH_MGR_GetExternalURL(
        char *url, WEBAUTH_TYPE_EXTERNAL_URL_T url_type);

/* FUNCTION NAME:  WEBAUTH_MGR_GetLoginAttemptByLPort
 * PURPOSE: This function get hosts login attempts in specified logical port.
 * INPUT:   ip_addr
 *          lport
 * OUTPUT:  *login_attempt
 * RETURN:  WEBAUTH_TYPE_RETURN_OK, WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:
 */
BOOL_T WEBAUTH_MGR_GetLoginAttemptByLPort(
        UI32_T ip_addr, UI32_T lport, UI8_T *login_attempt);

/* FUNCTION NAME: WEBAUTH_MGR_ReAuthByLPort
 * PURPOSE: This function will reinit all hosts in this lport
 * INPUT:   lport
 * OUTPUT:  none
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR,
 *          WEBAUTH_TYPE_RETURN_NO_EFFECT
 * NOTES:   none
 */
UI32_T WEBAUTH_MGR_ReAuthByLPort(UI32_T lport);

/* FUNCTION NAME: WEBAUTH_MGR_ReAuthByLPort
 * PURPOSE: This function will reinit specify host in this lport
 * INPUT:   lport
 *          ip_addr
 * OUTPUT:  none
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR,
 *          WEBAUTH_TYPE_RETURN_NO_EFFECT
 * NOTES:   none
 */
UI32_T WEBAUTH_MGR_ReAuthHostByLPort(UI32_T lport, UI32_T ip_addr);

/* FUNCTION NAME:  WEBAUTH_MGR_GetRunningSessionTimeout
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
 *          specific session timeout with non-default values
 *          can be retrieved successfully. Otherwise,
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL or
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT:   *session_timeout
 * OUTPUT:  session timeout value .
 * RETURN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES:   1. This function shall only be invoked by CLI to save the
 *             "running configuration" to local or remote files.
 *          2. Since only non-default configuration will be saved, this
 *             function shall return non-default structure for each field
 *             for the device.
 */
SYS_TYPE_Get_Running_Cfg_T WEBAUTH_MGR_GetRunningSessionTimeout(
                            UI16_T *session_timeout);

/* FUNCTION NAME:  WEBAUTH_MGR_GetRunningQuietPeriod
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
 *          specific quiet period with non-default values
 *          can be retrieved successfully. Otherwise,
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL or
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT:   *quiet period .
 * OUTPUT:  *quiet period value.
 * RETURN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES:   1. This function shall only be invoked by CLI to save the
 *             "running configuration" to local or remote files.
 *          2. Since only non-default configuration will be saved, this
 *             function shall return non-default structure for each field
 *             for the device.
 */
SYS_TYPE_Get_Running_Cfg_T WEBAUTH_MGR_GetRunningQuietPeriod(
                            UI16_T *quiet_period);

/* FUNCTION NAME:  WEBAUTH_MGR_GetRunningMaxLoginAttempt
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
 *          specific max login attempt with non-default values
 *          can be retrieved successfully. Otherwise,
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL or
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT:   *login_attempt
 * OUTPUT:  max login attempt value.
 * RETURN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES:   1. This function shall only be invoked by CLI to save the
 *             "running configuration" to local or remote files.
 *          2. Since only non-default configuration will be saved, this
 *             function shall return non-default structure for each field
 *             for the device.
 */
SYS_TYPE_Get_Running_Cfg_T WEBAUTH_MGR_GetRunningMaxLoginAttempt(
                            UI8_T *login_attempt);

/* FUNCTION NAME:  WEBAUTH_MGR_GetRunningSystemStatus
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
 *          system status with non-default values
 *          can be retrieved successfully. Otherwise,
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL
 *          or SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT:   *status
 * OUTPUT:  status
 * RETURN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES:   1. This function shall only be invoked by CLI to save the
 *             "running configuration" to local or remote files.
 *          2. Since only non-default configuration will be saved, this
 *             function shall return non-default structure for each field for
 *             the device.
 */
SYS_TYPE_Get_Running_Cfg_T WEBAUTH_MGR_GetRunningSystemStatus(UI8_T *status);

/* FUNCTION NAME:  WEBAUTH_MGR_GetRunningStatusByLPort
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
 *          logical port status with non-default values
 *          can be retrieved successfully. Otherwise,
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL or
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT:   lport  -- logical port
 *          *status
 * OUTPUT:  none
 * RETURN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES:   1. This function shall only be invoked by CLI to save the
 *             "running configuration" to local or remote files.
 *          2. Since only non-default configuration will be saved, this
 *             function shall return non-default structure for each field
 *             for the device.
 */
SYS_TYPE_Get_Running_Cfg_T WEBAUTH_MGR_GetRunningStatusByLPort(
                            UI32_T lport,UI8_T *status);

/* FUNCTION NAME:  WEBAUTH_MGR_GetRunningLoginURL
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
 *          login URL with non-default values
 *          can be retrieved successfully. Otherwise,
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL
 *          or SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT:   *url
 * OUTPUT:  *url
 * RETURN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES:   1. This function shall only be invoked by CLI to save the
 *             "running configuration" to local or remote files.
 *          2. Since only non-default configuration will be saved, this
 *             function shall return non-default structure for each field for
 *             the device.
 */
SYS_TYPE_Get_Running_Cfg_T  WEBAUTH_MGR_GetRunningLoginURL(char *url);

/* FUNCTION NAME:  WEBAUTH_MGR_GetRunningLoginFailURL
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
 *          login fail URL with non-default values
 *          can be retrieved successfully. Otherwise,
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL
 *          or SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT:   *url
 * OUTPUT:  *url
 * RETURN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES:   1. This function shall only be invoked by CLI to save the
 *             "running configuration" to local or remote files.
 *          2. Since only non-default configuration will be saved, this
 *             function shall return non-default structure for each field for
 *             the device.
 */
SYS_TYPE_Get_Running_Cfg_T  WEBAUTH_MGR_GetRunningLoginFailURL(char *url);

/* FUNCTION NAME:  WEBAUTH_MGR_GetRunningLoginSuccessURL
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
 *          login success URL with non-default values
 *          can be retrieved successfully. Otherwise,
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL
 *          or SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT:   *url
 * OUTPUT:  *url
 * RETURN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES:   1. This function shall only be invoked by CLI to save the
 *             "running configuration" to local or remote files.
 *          2. Since only non-default configuration will be saved, this
 *             function shall return non-default structure for each field for
 *             the device.
 */
SYS_TYPE_Get_Running_Cfg_T  WEBAUTH_MGR_GetRunningLoginSuccessURL(char *url);

/* FUNCTION NAME: WEBAUTH_MGR_GetNextSuccessHostByLPort
 * PURPOSE: This function will get next success host by lport
 * INPUT:   lport    -- key 1
 *          *index_p -- key 2
 * OUTPUT:  host_info_p
 *          *index_p
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   for ecah logical port, it will have at most
 *          SYS_ADPT_MAX_NBR_OF_WEBAUTH_HOST_PER_PORT success hosts.
 */
UI32_T WEBAUTH_MGR_GetNextSuccessHostByLPort(
       WEBAUTH_TYPE_Host_Info_T  *host_info_p, UI32_T lport, UI8_T *index_p);

/* FUNCTION NAME: WEBAUTH_MGR_CreateSuccessHostByLPort
 * PURPOSE: create success host data by lport
 * INPUT:   ip_addr
 *          lport
 * OUTPUT:  none
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none
 */
UI32_T WEBAUTH_MGR_CreateSuccessHostByLPort(UI32_T ip_addr, UI32_T lport);

/* FUNCTION NAME: WEBAUTH_MGR_DeleteSuccessListByHostIP
 * PURPOSE: delete success host data by lport
 * INPUT:   ip_addr
 *          lport
 * OUTPUT:  none
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none
 */
UI32_T WEBAUTH_MGR_DeleteSuccessListByHostIP(UI32_T ip_addr, UI32_T lport);

/* FUNCTION NAME: WEBAUTH_MGR_CreateBlackHostByLPort
 * PURPOSE: This function will create black hostby lport
 * INPUT:   ip_addr
 *          lport
 * OUTPUT:  none
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   for each logical port, it will have at most
 *          SYS_ADPT_MAX_NBR_OF_WEBAUTH_HOST_PER_PORT, if full kill
 *          oldest and add to tail.
 */
UI32_T WEBAUTH_MGR_CreateBlackHostByLPort(UI32_T ip_addr, UI32_T lport);

/* FUNCTION NAME: WEBAUTH_MGR_CheckIPIsBlackByLPort
 * PURPOSE: This function will check whether host is in black
 * INPUT:   ip_addr
 *          lport
 * OUTPUT:  none
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   if is black , return WEBAUTH_TYPE_RETURN_OK
 *          else return WEBAUTH_TYPE_RETURN_ERROR
 */
UI32_T WEBAUTH_MGR_CheckIPIsBlackByLPort(UI32_T ip_addr, UI32_T lport);

/* FUNCTION NAME: WEBAUTH_MGR_CreateTryingHost
 * PURPOSE: This function will create trying host to trying list
 * INPUT:   ip_addr
 *          lport
 * OUTPUT:  none
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   for trying list , it has length
 *          WEBAUTH_TYPE_MAX_TRYING_HOST_COUNT, if reach max count, delete
 *          oldest and add to tail of list.
 */
UI32_T WEBAUTH_MGR_CreateTryingHost(UI32_T ip_addr, UI32_T lport);

/* FUNCTION NAME: WEBAUTH_MGR_DeleteBlackListByHostIP
 * PURPOSE: delete black host data by lport
 * INPUT:   ip_addr
 *          lport
 * OUTPUT:  none
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none
 */
UI32_T WEBAUTH_MGR_DeleteBlackListByHostIP(UI32_T ip_addr, UI32_T lport);

/* FUNCTION NAME: WEBAUTH_MGR_DeleteTryingHost
 * PURPOSE: delete trying host data by lport
 * INPUT:   ip_addr
 *          lport
 * OUTPUT:  none
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none
 */
UI32_T WEBAUTH_MGR_DeleteTryingHost(UI32_T ip_addr,UI32_T lport);


/* FUNCTION NAME: WEBAUTH_MGR_GetPortInfoByLPort
 * PURPOSE: This function will get port info by lport
 * INPUT:   lport
 * OUTPUT:  *lport_info_p
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none
 */
UI32_T WEBAUTH_MGR_GetPortInfoByLPort(UI32_T lport, WEBAUTH_TYPE_Port_Info_T *lport_info_p);

/* FUNCTION NAME: WEBAUTH_MGR_GetNextPortInfoByLPort
 * PURPOSE: this function will get next port info
 * INPUT:   *index
 * OUTPUT:  *index
 *          *lport_info_p
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTE:    key is index; index=0: get initial
 */
UI32_T WEBAUTH_MGR_GetNextPortInfoByLPort(UI32_T *index, WEBAUTH_TYPE_Port_Info_T *lport_info_p);

/* FUNCTION NAME: WEBAUTH_MGR_GetSuccessHostByLPortAndIP
 * PURPOSE: This function will get success host by lport and ip
 * INPUT:   host_info_p
 * OUTPUT:  host_info_p
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none
 */
UI32_T WEBAUTH_MGR_GetSuccessHostByLPortAndIP(
        WEBAUTH_TYPE_Host_Info_T  *host_info_p);

/* FUNCTION NAME: WEBAUTH_MGR_ProcessAuth
 * PURPOSE: This function do auth job
 * INPUT:   *username_p
 *          *paassword_p
 *          ip_addr
 *          lport
 * OUTPUT:  none
 * RETURN:  WEBAUTH_TYPE_RETURN_HOST_Success;WEBAUTH_TYPE_RETURN_HOST_BLACK
 *          WEBAUTH_TYPE_RETURN_HOST_TRYING
 * NOTES:   1. for now auth is done by radius(username and pwd)
 *          2. auth success -> set rule (return WEBAUTH_TYPE_RETURN_OK)
 *          3. auth fail
 *              3.1 if new challenge -> add to try list
 *              3.2 if trying before, check trying count
 *                  3.2.1 if trying count < max -> add trying count++
 *                  3.2.2 trying count = max-1 -> add block list,
 *                        delete host in trying list
 */
UI32_T WEBAUTH_MGR_ProcessAuth(
        char *username_p, char *password_p, UI32_T ip_addr, UI32_T lport);

/* FUNCTION NAME: WEBAUTH_MGR_ProcessTimerEvent
 * PURPOSE: this function will process timer event
 * INPUT:   none
 * OUTPUT:  None
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTE:    process three list
 *          1.success: check from head, if expire
 *                     delete: host,list,rule
 *          2.black  : check from head, if expire
 *                     delete: host,list,rule
 *          3.trying : check from head, if expire
 *                     delete: host,list
 */
void WEBAUTH_MGR_ProcessTimerEvent(void);

/* FUNCTION NAME: WEBAUTH_MGR_GetNextSuccessHostByLPortAndIndex
 * PURPOSE: This function will get next success host by lport and index
 *          index means success count.
 * INPUT:   host_info_p->lport -- key 1
 *          host_info_p->ip    -- key 2
 * OUTPUT:  host_info_p
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   for ecah logical port, it will have at most
 *          SYS_ADPT_MAX_NBR_OF_WEBAUTH_HOST_PER_PORT success hosts.
 *          loop end: lport not exist, or got one
 */
UI32_T WEBAUTH_MGR_GetNextSuccessHostByLPortAndIP(
        WEBAUTH_TYPE_Host_Info_T  *host_info_p);

/* FUNCTION NAME: WEBAUTH_MGR_GetSuccessHostByLPortAndIP
 * PURPOSE: This function will get success host by lport and ip
 * INPUT:   host_info_p->lport -- key 1
 *          host_info_p->ip    -- key 2
 * OUTPUT:  host_info_p
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none
 */
UI32_T WEBAUTH_MGR_GetSuccessHostByLPortAndIP(
        WEBAUTH_TYPE_Host_Info_T  *host_info_p);

/* FUNCTION NAME: WEBAUTH_MGR_GetSuccessHostByLPort
 * PURPOSE: This function will get success host by lport and index
 * INPUT:   lport
 *          index
 * OUTPUT:  *host_info_p
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   for ecah logical port, it will have at most
 *          SYS_ADPT_MAX_NBR_OF_WEBAUTH_HOST_PER_PORT success hosts.
 */
UI32_T WEBAUTH_MGR_GetSuccessHostByLPort(
        WEBAUTH_TYPE_Host_Info_T  *host_info_p, UI32_T lport, UI8_T index);

/* FUNCTION NAME: WEBAUTH_MGR_GetSuccessCountByLPort
 * PURPOSE: This function will get success count by lport
 * INPUT:   lport
 * OUTPUT:  *success_count_p
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none
 */
UI32_T WEBAUTH_MGR_GetSuccessCountByLPort(UI32_T lport, UI16_T *success_count_p);

/* FUNCTION NAME: WEBAUTH_MGR_GetTryingHostCount
 * PURPOSE: This function will get trying host login attempt by ip
 * INPUT:   ip_addr
 *          lport
 *          *login_attempt_p
 * OUTPUT:  *login_attempt_p
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none
 */
UI32_T WEBAUTH_MGR_GetTryingHostCount(
       UI32_T ip_addr, UI32_T lport,UI8_T *login_attempt_p );

/* FUNCTION NAME: WEBAUTH_MGR_SetDebugFlag
 * PURPOSE: This function will set debug flag
 * INPUT:   debug_flag
 * OUTPUT:  none
 * RETURN:  none.
 * NOTES:   none.
 */
void WEBAUTH_MGR_SetDebugFlag(UI32_T debug_flag);

/* FUNCTION NAME: WEBAUTH_MGR_GetDebugFlag
 * PURPOSE: This function will set debug flag
 * INPUT:   void
 * OUTPUT:  none
 * RETURN:  webauth_mgr_debug_flag
 * NOTES:   none.
 */
UI32_T WEBAUTH_MGR_GetDebugFlag(void);

/* FUNCTION NAME: WEBAUTH_MGR_GetWebAuthInfo
 * PURPOSE: This function will get webauth global status
 * INPUT:   none
 * OUTPUT:  *sys_info_p
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none
 */
UI32_T WEBAUTH_MGR_GetWebAuthInfo(WEBAUTH_TYPE_System_Info_T *sys_info_p);

/* FUNCTION NAME: WEBAUTH_MGR_IsIPValidByLPort
 * PURPOSE: This function will check ip is valid by lport
 * INPUT:   ip_addr
 *          lport
 * OUTPUT:  none
 * RETURN:  TRUE: this ip is valid for this port
 *          FALSE: this ip is in valid for this port, or global/port
 *                 status for webauth is disabled.
 * NOTES:   none.
 */
BOOL_T WEBAUTH_MGR_IsIPValidByLPort(UI32_T ip_addr, UI32_T lport);

/* FUNCTION NAME: WEBAUTH_MGR_PortLinkDown_CallBack
 * PURPOSE: this function will process port link down
 * INPUT:   unit
 *          port
 * OUTPUT:  none
 * RETURN:  none
 * NOTE:    when port link down, reset all hosts in this port
 */
void WEBAUTH_MGR_PortLinkDown_CallBack(UI32_T unit, UI32_T port);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - WEBAUTH_MGR_HandleIPCReqMsg
 *------------------------------------------------------------------------------
 * PURPOSE  : Handle the ipc request message for netaccess mgr.
 * INPUT    : ipcmsg_p  --  input request ipc message buffer
 * OUTPUT   : ipcmsg_p  --  input request ipc message buffer
 * RETURN   : TRUE  - There is a response need to send.
 *            FALSE - There is no response to send.
 * NOTES    : none
 *------------------------------------------------------------------------------
 */
BOOL_T WEBAUTH_MGR_HandleIPCReqMsg(SYSFUN_Msg_T *ipcmsg_p);

#endif  /* End of webauth_mgr_h */

