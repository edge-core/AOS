#include <stdio.h>
#include <string.h>
#include "sys_type.h"
#include "sys_module.h"
#include "sys_bld.h"
#include "sysfun.h"
#include "webauth_mgr.h"
#include "webauth_pmgr.h"

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void WEBAUTH_PMGR_SendMsg(UI32_T cmd, SYSFUN_Msg_T *msg_p, UI32_T req_size, UI32_T res_size, UI32_T ret_val);

static UI32_T WEBAUTH_PMGR_GetUI32Data(UI32_T ipc_cmd, UI32_T *out_p);
static UI32_T WEBAUTH_PMGR_GetUI16Data(UI32_T ipc_cmd, UI16_T *out_p);
static UI32_T WEBAUTH_PMGR_GetUI8Data(UI32_T ipc_cmd, UI8_T *out_p);

static UI32_T WEBAUTH_PMGR_SetUI32Data(UI32_T ipc_cmd, UI32_T data);

static UI32_T WEBAUTH_PMGR_GetUI32DataByLport(UI32_T ipc_cmd, UI32_T lport, UI32_T *out_p);
static UI32_T WEBAUTH_PMGR_GetUI8DataByLport(UI32_T ipc_cmd, UI32_T lport, UI8_T *out_p);

static UI32_T WEBAUTH_PMGR_SetUI32DataByLport(UI32_T ipc_cmd, UI32_T lport, UI32_T data);

static SYS_TYPE_Get_Running_Cfg_T WEBAUTH_PMGR_GetRunningUI32DataByLport(UI32_T ipc_cmd, UI32_T lport, UI32_T *out_p);
static SYS_TYPE_Get_Running_Cfg_T WEBAUTH_PMGR_GetRunningUI8DataByLport(UI32_T ipc_cmd, UI32_T lport, UI8_T *out_p);

static SYS_TYPE_Get_Running_Cfg_T WEBAUTH_PMGR_GetRunningUI32Data(UI32_T ipc_cmd, UI32_T *out_p);
static SYS_TYPE_Get_Running_Cfg_T WEBAUTH_PMGR_GetRunningUI16Data(UI32_T ipc_cmd, UI16_T *out_p);
static SYS_TYPE_Get_Running_Cfg_T WEBAUTH_PMGR_GetRunningUI8Data(UI32_T ipc_cmd, UI8_T *out_p);

/* STATIC VARIABLE DECLARATIONS
 */
static SYSFUN_MsgQ_T ipcmsgq_handle;

/* EXPORTED SUBPROGRAM BODIES
 */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - WEBAUTH_PMGR_InitiateProcessResources
 *-------------------------------------------------------------------------
 * PURPOSE: Initiate resource used in the calling process.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : TRUE  - success
 *          FALSE - failure
 * NOTE   : None
 *-------------------------------------------------------------------------
 */
BOOL_T WEBAUTH_PMGR_InitiateProcessResources(void)
{
    if (SYSFUN_GetMsgQ(WEBAUTH_MGR_IPCMSG_KEY, SYSFUN_MSGQ_BIDIRECTIONAL, &ipcmsgq_handle) != SYSFUN_OK)
    {
        printf("%s(): SYSFUN_GetMsgQ fail.\n", __FUNCTION__);
        return FALSE;
    }

    return TRUE;
}

/* FUNCTION NAME: WEBAUTH_PMGR_SetSystemStatus
 * PURPOSE: This function will set global status of webauth
 * INPUT:   status
 * OUTPUT:  VAL_webauthEnable_enabled/VAL_webauthEnable_disabled
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none 
 */
UI32_T WEBAUTH_PMGR_SetSystemStatus(UI8_T status)
{
    return WEBAUTH_PMGR_SetUI32Data(WEBAUTH_MGR_IPC_CMD_SET_SYSTEM_STATUS, status);
}

/* FUNCTION NAME: WEBAUTH_PMGR_GetSystemStatus
 * PURPOSE: This function will return the status of webath
 * INPUT:   *status
 * OUTPUT:  VAL_webauthEnable_enabled/VAL_webauthEnable_disabled
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none 
 */
UI32_T WEBAUTH_PMGR_GetSystemStatus(UI8_T *status)
{
    return WEBAUTH_PMGR_GetUI8Data(WEBAUTH_MGR_IPC_CMD_GET_SYSTEM_STATUS, status);
}

/* FUNCTION NAME: WEBAUTH_PMGR_SetStatusByLPort 
 * PURPOSE: this function will set webauth per port status
 * INPUT:   lport
 *          status
 * OUTPUT:  VAL_webAuthPortConfigStatus_enabled/VAL_webAuthPortConfigStatus_disabled
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTE:    none 
 */
UI32_T WEBAUTH_PMGR_SetStatusByLPort(UI32_T lport, UI8_T status)
{
    return WEBAUTH_PMGR_SetUI32DataByLport(WEBAUTH_MGR_IPC_CMD_SET_STATUS, lport, status);
}

/* FUNCTION NAME: WEBAUTH_PMGR_GetStatusByLPort
 * PURPOSE: This function will return webauth per port status 
 * INPUT:   lport
 * OUTPUT:  *status -- VAL_webAuthPortConfigStatus_enabled/VAL_webAuthPortConfigStatus_disabled
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none 
 */
UI32_T WEBAUTH_PMGR_GetStatusByLPort(UI32_T lport, UI8_T *status)
{
    return WEBAUTH_PMGR_GetUI8DataByLport(WEBAUTH_MGR_IPC_CMD_GET_STATUS, lport, status);
}

/* FUNCTION NAME: WEBAUTH_PMGR_SetQuietPeriod
 * PURPOSE: this function will set quiet period
 * INPUT:   quiet_period
 * OUTPUT:  None
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTE:    None 
 */
UI32_T WEBAUTH_PMGR_SetQuietPeriod(UI16_T quiet_period)
{
    return WEBAUTH_PMGR_SetUI32Data(WEBAUTH_MGR_IPC_CMD_SET_QUIET_PERIOD, quiet_period);
}

/* FUNCTION NAME: WEBAUTH_PMGR_GetQuietPeriod
 * PURPOSE: This function will return the quiet period value
 * INPUT:   none.
 * OUTPUT:  *quiet_period
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none 
 */
UI32_T WEBAUTH_PMGR_GetQuietPeriod(UI16_T *quiet_period)
{
    return WEBAUTH_PMGR_GetUI16Data(WEBAUTH_MGR_IPC_CMD_GET_QUIET_PERIOD, quiet_period);
}

/* FUNCTION NAME: WEBAUTH_PMGR_SetMaxLoginAttempts
 * PURPOSE: this function will set max login attempts
 * INPUT:   max_login_attempt
 * OUTPUT:  None
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTE:    None
 */
UI32_T WEBAUTH_PMGR_SetMaxLoginAttempts(UI8_T max_login_attempt)
{
    return WEBAUTH_PMGR_SetUI32Data(WEBAUTH_MGR_IPC_CMD_SET_MAX_LOGIN_ATTEMPTS, max_login_attempt);
}

/* FUNCTION NAME: WEBAUTH_PMGR_GetMaxLoginAttempts
 * PURPOSE: This function will return global value of max login attempt
 * INPUT:   none
 * OUTPUT:  *max login attempt
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none 
 */
UI32_T WEBAUTH_PMGR_GetMaxLoginAttempts(UI8_T *max_login_attempt)
{
    return WEBAUTH_PMGR_GetUI8Data(WEBAUTH_MGR_IPC_CMD_GET_MAX_LOGIN_ATTEMPTS, max_login_attempt);
}

/* FUNCTION NAME: WEBAUTH_PMGR_SetSystemSessionTimeout
 * PURPOSE: This function will set global session timeout value
 * INPUT:   session_timeout
 * OUTPUT:  None
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTE:    None
 */
UI32_T WEBAUTH_PMGR_SetSystemSessionTimeout(UI16_T session_timeout)
{
    return WEBAUTH_PMGR_SetUI32Data(WEBAUTH_MGR_IPC_CMD_SET_SYSTEM_SESSION_TIMEOUT, session_timeout);
}

/* FUNCTION NAME: WEBAUTH_PMGR_GetSystemSessionTimeout
 * PURPOSE: This function will get global session timeout value
 * INPUT:   none.
 * OUTPUT:  *session_timeout
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none
 */
UI32_T WEBAUTH_PMGR_GetSystemSessionTimeout(UI16_T *session_timeout)
{
    return WEBAUTH_PMGR_GetUI16Data(WEBAUTH_MGR_IPC_CMD_GET_SYSTEM_SESSION_TIMEOUT, session_timeout);
}

/* FUNCTION NAME: WEBAUTH_PMGR_SetExternalURL
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
UI32_T WEBAUTH_PMGR_SetExternalURL(
        char *url, WEBAUTH_TYPE_EXTERNAL_URL_T url_type)
{
    UI8_T msgbuf[SYSFUN_SIZE_OF_MSG(WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_ExternelUrl_T))];
    SYSFUN_Msg_T *msg_p = (SYSFUN_Msg_T *)msgbuf;
    WEBAUTH_MGR_IPCMsg_ExternelUrl_T *data_p;

    if (NULL == url)
    {
        return WEBAUTH_TYPE_RETURN_ERROR;
    }

    data_p = WEBAUTH_MGR_MSG_DATA(msg_p);
    strncpy(data_p->url, url, sizeof(data_p->url));
    data_p->url_type = url_type;

    WEBAUTH_PMGR_SendMsg(WEBAUTH_MGR_IPC_CMD_SET_EXTERNAL_URL,
        msg_p,
        WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_ExternelUrl_T),
        WEBAUTH_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
        WEBAUTH_TYPE_RETURN_ERROR);

    return WEBAUTH_MGR_MSG_RETVAL(msg_p);
}

/* FUNCTION NAME: WEBAUTH_PMGR_GetExternalURL
 * PURPOSE: This function get external URL by it's type
 * INPUT:   url type:
 *             {WEBAUTH_EXTERNAL_URL_Login
 *              WEBAUTH_EXTERNAL_URL_Login_fail
 *              WEBAUTH_EXTERNAL_URL_Login_Success} 
 * OUTPUT:  *url
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none
 */
UI32_T WEBAUTH_PMGR_GetExternalURL(
        char *url, WEBAUTH_TYPE_EXTERNAL_URL_T url_type)
{
    UI8_T msgbuf[SYSFUN_SIZE_OF_MSG(WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_ExternelUrl_T))];
    SYSFUN_Msg_T *msg_p = (SYSFUN_Msg_T *)msgbuf;
    WEBAUTH_MGR_IPCMsg_ExternelUrl_T *data_p;

    if (NULL == url)
    {
        return WEBAUTH_TYPE_RETURN_ERROR;
    }

    data_p = WEBAUTH_MGR_MSG_DATA(msg_p);
    data_p->url_type = url_type;

    WEBAUTH_PMGR_SendMsg(WEBAUTH_MGR_IPC_CMD_GET_EXTERNAL_URL,
        msg_p,
        WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_ExternelUrl_T),
        WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_ExternelUrl_T),
        WEBAUTH_TYPE_RETURN_ERROR);

    if (WEBAUTH_TYPE_RETURN_OK == WEBAUTH_MGR_MSG_RETVAL(msg_p))
    {
        strncpy(url, data_p->url, sizeof(data_p->url));
        url_type = data_p->url_type;
    }

    return WEBAUTH_MGR_MSG_RETVAL(msg_p);
}

#if 0
/* FUNCTION NAME:  WEBAUTH_PMGR_GetLoginAttemptByLPort
 * PURPOSE: This function get hosts login attempts in specified logical port.
 * INPUT:   ip_addr  
 *          lport
 * OUTPUT:  *login_attempt
 * RETURN:  WEBAUTH_TYPE_RETURN_OK, WEBAUTH_TYPE_RETURN_ERROR
 * NOTES: 
 */
BOOL_T WEBAUTH_PMGR_GetLoginAttemptByLPort(
        UI32_T ip_addr, UI32_T lport, UI8_T *login_attempt)
{
    UI8_T msgbuf[SYSFUN_SIZE_OF_MSG(WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_ExternalUrl_T))];
    SYSFUN_Msg_T *msg_p = (SYSFUN_Msg_T *)msgbuf;
    WEBAUTH_MGR_IPCMsg_ExternalUrl_T *data_p;

    if (NULL == login_attempt)
    {
        return WEBAUTH_TYPE_RETURN_ERROR;
    }

    data_p = WEBAUTH_MGR_MSG_DATA(msg_p);
    data_p->url_type = url_type;

    WEBAUTH_PMGR_SendMsg(WEBAUTH_MGR_IPC_CMD_GET_EXTERNAL_URL,
        msg_p,
        WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_ExternalUrl_T),
        WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_ExternalUrl_T),
        (BOOL_T) FALSE);

    if (FALSE == WEBAUTH_MGR_MSG_RETVAL(msg_p))
    {
        strncpy(url, data_p->url, sizeof(data_p->url));
        url_type = data_p->url_type;
    }

    return WEBAUTH_MGR_MSG_RETVAL(msg_p);
}
#endif

/* FUNCTION NAME: WEBAUTH_PMGR_ReAuthByLPort
 * PURPOSE: This function will reinit all hosts in this lport
 * INPUT:   lport
 * OUTPUT:  none
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR,
 *          WEBAUTH_TYPE_RETURN_NO_EFFECT
 * NOTES:   none
 */
UI32_T WEBAUTH_PMGR_ReAuthByLPort(UI32_T lport)
{
    return WEBAUTH_PMGR_SetUI32Data(WEBAUTH_MGR_IPC_CMD_REAUTH_BY_LPORT, lport);
}

/* FUNCTION NAME: WEBAUTH_PMGR_ReAuthByLPort
 * PURPOSE: This function will reinit specify host in this lport
 * INPUT:   lport
 *          ip_addr
 * OUTPUT:  none
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR,
 *          WEBAUTH_TYPE_RETURN_NO_EFFECT
 * NOTES:   none
 */
UI32_T WEBAUTH_PMGR_ReAuthHostByLPort(UI32_T lport, UI32_T ip_addr)
{
    return WEBAUTH_PMGR_SetUI32DataByLport(WEBAUTH_MGR_IPC_CMD_REAUTH_HOST_BY_LPORT, lport, ip_addr);
}

/* FUNCTION NAME:  WEBAUTH_PMGR_GetRunningSessionTimeout
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
SYS_TYPE_Get_Running_Cfg_T WEBAUTH_PMGR_GetRunningSessionTimeout(
                            UI16_T *session_timeout)
{
    return WEBAUTH_PMGR_GetRunningUI16Data(WEBAUTH_MGR_IPC_CMD_GET_RUNNING_SESSION_TIMEOUT, session_timeout);
}

/* FUNCTION NAME:  WEBAUTH_PMGR_GetRunningQuietPeriod
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
SYS_TYPE_Get_Running_Cfg_T WEBAUTH_PMGR_GetRunningQuietPeriod(
                            UI16_T *quiet_period)
{
    return WEBAUTH_PMGR_GetRunningUI16Data(WEBAUTH_MGR_IPC_CMD_GET_RUNNING_QUIET_PERIOD, quiet_period);
}

/* FUNCTION NAME:  WEBAUTH_PMGR_GetRunningMaxLoginAttempt
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
SYS_TYPE_Get_Running_Cfg_T WEBAUTH_PMGR_GetRunningMaxLoginAttempt(
                            UI8_T *login_attempt)
{
    return WEBAUTH_PMGR_GetRunningUI8Data(WEBAUTH_MGR_IPC_CMD_GET_RUNNING_MAX_LOGIN_ATTEMPT, login_attempt);
}

/* FUNCTION NAME:  WEBAUTH_PMGR_GetRunningSystemStatus
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
SYS_TYPE_Get_Running_Cfg_T WEBAUTH_PMGR_GetRunningSystemStatus(UI8_T *status)
{
    return WEBAUTH_PMGR_GetRunningUI8Data(WEBAUTH_MGR_IPC_CMD_GET_RUNNING_SYSTEM_STATUS, status);
}

/* FUNCTION NAME:  WEBAUTH_PMGR_GetRunningStatusByLPort
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
SYS_TYPE_Get_Running_Cfg_T WEBAUTH_PMGR_GetRunningStatusByLPort(
                            UI32_T lport,UI8_T *status)
{
    return WEBAUTH_PMGR_GetRunningUI8DataByLport(WEBAUTH_MGR_IPC_CMD_GET_RUNNING_STATUS, lport, status);
}

/* FUNCTION NAME:  WEBAUTH_PMGR_GetRunningLoginURL
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
SYS_TYPE_Get_Running_Cfg_T  WEBAUTH_PMGR_GetRunningLoginURL(char *url)
{
    UI8_T msgbuf[SYSFUN_SIZE_OF_MSG(WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_ExternelUrl_T))];
    SYSFUN_Msg_T *msg_p = (SYSFUN_Msg_T *)msgbuf;
    WEBAUTH_MGR_IPCMsg_ExternelUrl_T *data_p;

    if (NULL == url)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    data_p = WEBAUTH_MGR_MSG_DATA(msg_p);

    WEBAUTH_PMGR_SendMsg(WEBAUTH_MGR_IPC_CMD_GET_RUNNING_LOGIN_URL,
        msg_p,
        WEBAUTH_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
        WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_ExternelUrl_T),
        SYS_TYPE_GET_RUNNING_CFG_FAIL);

    strncpy(url, data_p->url, sizeof(data_p->url));

    return WEBAUTH_MGR_MSG_RETVAL(msg_p);
}

/* FUNCTION NAME:  WEBAUTH_PMGR_GetRunningLoginFailURL
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
SYS_TYPE_Get_Running_Cfg_T  WEBAUTH_PMGR_GetRunningLoginFailURL(char *url)
{
    UI8_T msgbuf[SYSFUN_SIZE_OF_MSG(WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_ExternelUrl_T))];
    SYSFUN_Msg_T *msg_p = (SYSFUN_Msg_T *)msgbuf;
    WEBAUTH_MGR_IPCMsg_ExternelUrl_T *data_p;

    if (NULL == url)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    data_p = WEBAUTH_MGR_MSG_DATA(msg_p);

    WEBAUTH_PMGR_SendMsg(WEBAUTH_MGR_IPC_CMD_GET_RUNNING_LOGIN_FAIL_URL,
        msg_p,
        WEBAUTH_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
        WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_ExternelUrl_T),
        SYS_TYPE_GET_RUNNING_CFG_FAIL);

    strncpy(url, data_p->url, sizeof(data_p->url));

    return WEBAUTH_MGR_MSG_RETVAL(msg_p);
}

/* FUNCTION NAME:  WEBAUTH_PMGR_GetRunningLoginSuccessURL
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
SYS_TYPE_Get_Running_Cfg_T  WEBAUTH_PMGR_GetRunningLoginSuccessURL(char *url)
{
    UI8_T msgbuf[SYSFUN_SIZE_OF_MSG(WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_ExternelUrl_T))];
    SYSFUN_Msg_T *msg_p = (SYSFUN_Msg_T *)msgbuf;
    WEBAUTH_MGR_IPCMsg_ExternelUrl_T *data_p;

    if (NULL == url)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    data_p = WEBAUTH_MGR_MSG_DATA(msg_p);

    WEBAUTH_PMGR_SendMsg(WEBAUTH_MGR_IPC_CMD_GET_RUNNING_LOGIN_SUCCESS_URL,
        msg_p,
        WEBAUTH_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
        WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_ExternelUrl_T),
        SYS_TYPE_GET_RUNNING_CFG_FAIL);

    strncpy(url, data_p->url, sizeof(data_p->url));

    return WEBAUTH_MGR_MSG_RETVAL(msg_p);
}

/* FUNCTION NAME: WEBAUTH_PMGR_GetNextSuccessHostByLPort
 * PURPOSE: This function will get next success host by lport
 * INPUT:   lport    -- key 1
 *          *index_p -- key 2
 * OUTPUT:  host_info_p
 *          *index_p
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   for ecah logical port, it will have at most 
 *          SYS_ADPT_MAX_NBR_OF_WEBAUTH_HOST_PER_PORT success hosts.
 */
UI32_T WEBAUTH_PMGR_GetNextSuccessHostByLPort(
       WEBAUTH_TYPE_Host_Info_T  *host_info_p, UI32_T lport, UI8_T *index_p)
{
    UI8_T msgbuf[SYSFUN_SIZE_OF_MSG(WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_HostInfo_U32_U32_T))];
    SYSFUN_Msg_T *msg_p = (SYSFUN_Msg_T *)msgbuf;
    WEBAUTH_MGR_IPCMsg_HostInfo_U32_U32_T *data_p;

    if (   (NULL == host_info_p)
        || (NULL == index_p))
    {
        return WEBAUTH_TYPE_RETURN_ERROR;
    }

    data_p = WEBAUTH_MGR_MSG_DATA(msg_p);
    memcpy(&data_p->host_info, host_info_p, sizeof(data_p->host_info));
    data_p->u32_1 = lport;
    data_p->u32_2 = *index_p;

    WEBAUTH_PMGR_SendMsg(WEBAUTH_MGR_IPC_CMD_GETNEXT_SUCCESS_HOST_BY_LPORT,
        msg_p,
        WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_HostInfo_U32_U32_T),
        WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_HostInfo_U32_U32_T),
        WEBAUTH_TYPE_RETURN_ERROR);

    if (WEBAUTH_TYPE_RETURN_OK == WEBAUTH_MGR_MSG_RETVAL(msg_p))
    {
        memcpy(host_info_p, &data_p->host_info, sizeof(data_p->host_info));
        *index_p = data_p->u32_2;
    }

    return WEBAUTH_MGR_MSG_RETVAL(msg_p);
}

/* FUNCTION NAME: WEBAUTH_PMGR_CreateSuccessHostByLPort
 * PURPOSE: create success host data by lport
 * INPUT:   ip_addr
 *          lport
 * OUTPUT:  none
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none
 */
UI32_T WEBAUTH_PMGR_CreateSuccessHostByLPort(UI32_T ip_addr, UI32_T lport)
{
    UI8_T msgbuf[SYSFUN_SIZE_OF_MSG(WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_U32_U32_Data_T))];
    SYSFUN_Msg_T *msg_p = (SYSFUN_Msg_T *)msgbuf;
    WEBAUTH_MGR_IPCMsg_U32_U32_Data_T *data_p;

    data_p = WEBAUTH_MGR_MSG_DATA(msg_p);
    data_p->u32_1 = ip_addr;
    data_p->u32_2 = lport;

    WEBAUTH_PMGR_SendMsg(WEBAUTH_MGR_IPC_CMD_CREATE_SUCCESS_HOST_BY_LPORT,
        msg_p,
        WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_U32_U32_Data_T),
        WEBAUTH_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
        WEBAUTH_TYPE_RETURN_ERROR);

    return WEBAUTH_MGR_MSG_RETVAL(msg_p);
}

/* FUNCTION NAME: WEBAUTH_PMGR_DeleteSuccessListByHostIP
 * PURPOSE: delete success host data by lport
 * INPUT:   ip_addr
 *          lport
 * OUTPUT:  none
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none
 */
UI32_T WEBAUTH_PMGR_DeleteSuccessListByHostIP(UI32_T ip_addr, UI32_T lport)
{
    UI8_T msgbuf[SYSFUN_SIZE_OF_MSG(WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_U32_U32_Data_T))];
    SYSFUN_Msg_T *msg_p = (SYSFUN_Msg_T *)msgbuf;
    WEBAUTH_MGR_IPCMsg_U32_U32_Data_T *data_p;

    data_p = WEBAUTH_MGR_MSG_DATA(msg_p);
    data_p->u32_1 = ip_addr;
    data_p->u32_2 = lport;

    WEBAUTH_PMGR_SendMsg(WEBAUTH_MGR_IPC_CMD_DELETE_SUCCESS_LIST_BY_HOST_IP,
        msg_p,
        WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_U32_U32_Data_T),
        WEBAUTH_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
        WEBAUTH_TYPE_RETURN_ERROR);

    return WEBAUTH_MGR_MSG_RETVAL(msg_p);
}

/* FUNCTION NAME: WEBAUTH_PMGR_CreateBlackHostByLPort
 * PURPOSE: This function will create black hostby lport
 * INPUT:   ip_addr
 *          lport
 * OUTPUT:  none
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   for each logical port, it will have at most 
 *          SYS_ADPT_MAX_NBR_OF_WEBAUTH_HOST_PER_PORT, if full kill
 *          oldest and add to tail.
 */
UI32_T WEBAUTH_PMGR_CreateBlackHostByLPort(UI32_T ip_addr, UI32_T lport)
{
    UI8_T msgbuf[SYSFUN_SIZE_OF_MSG(WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_U32_U32_Data_T))];
    SYSFUN_Msg_T *msg_p = (SYSFUN_Msg_T *)msgbuf;
    WEBAUTH_MGR_IPCMsg_U32_U32_Data_T *data_p;

    data_p = WEBAUTH_MGR_MSG_DATA(msg_p);
    data_p->u32_1 = ip_addr;
    data_p->u32_2 = lport;

    WEBAUTH_PMGR_SendMsg(WEBAUTH_MGR_IPC_CMD_CREATE_BLACK_HOST_BY_LPORT,
        msg_p,
        WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_U32_U32_Data_T),
        WEBAUTH_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
        WEBAUTH_TYPE_RETURN_ERROR);

    return WEBAUTH_MGR_MSG_RETVAL(msg_p);
}

/* FUNCTION NAME: WEBAUTH_PMGR_CheckIPIsBlackByLPort
 * PURPOSE: This function will check whether host is in black
 * INPUT:   ip_addr
 *          lport
 * OUTPUT:  none
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   if is black , return WEBAUTH_TYPE_RETURN_OK
 *          else return WEBAUTH_TYPE_RETURN_ERROR
 */
UI32_T WEBAUTH_PMGR_CheckIPIsBlackByLPort(UI32_T ip_addr, UI32_T lport)
{
    UI8_T msgbuf[SYSFUN_SIZE_OF_MSG(WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_U32_U32_Data_T))];
    SYSFUN_Msg_T *msg_p = (SYSFUN_Msg_T *)msgbuf;
    WEBAUTH_MGR_IPCMsg_U32_U32_Data_T *data_p;

    data_p = WEBAUTH_MGR_MSG_DATA(msg_p);
    data_p->u32_1 = ip_addr;
    data_p->u32_2 = lport;

    WEBAUTH_PMGR_SendMsg(WEBAUTH_MGR_IPC_CMD_CHECK_IP_IS_BLACK_BY_LPORT,
        msg_p,
        WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_U32_U32_Data_T),
        WEBAUTH_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
        WEBAUTH_TYPE_RETURN_ERROR);

    return WEBAUTH_MGR_MSG_RETVAL(msg_p);
}

/* FUNCTION NAME: WEBAUTH_PMGR_CreateTryingHost
 * PURPOSE: This function will create trying host to trying list
 * INPUT:   ip_addr
 *          lport
 * OUTPUT:  none
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   for trying list , it has length
 *          WEBAUTH_TYPE_MAX_TRYING_HOST_COUNT, if reach max count, delete
 *          oldest and add to tail of list.
 */
UI32_T WEBAUTH_PMGR_CreateTryingHost(UI32_T ip_addr, UI32_T lport)
{
    UI8_T msgbuf[SYSFUN_SIZE_OF_MSG(WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_U32_U32_Data_T))];
    SYSFUN_Msg_T *msg_p = (SYSFUN_Msg_T *)msgbuf;
    WEBAUTH_MGR_IPCMsg_U32_U32_Data_T *data_p;

    data_p = WEBAUTH_MGR_MSG_DATA(msg_p);
    data_p->u32_1 = ip_addr;
    data_p->u32_2 = lport;

    WEBAUTH_PMGR_SendMsg(WEBAUTH_MGR_IPC_CMD_CREATE_TRYING_HOST,
        msg_p,
        WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_U32_U32_Data_T),
        WEBAUTH_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
        WEBAUTH_TYPE_RETURN_ERROR);

    return WEBAUTH_MGR_MSG_RETVAL(msg_p);
}

/* FUNCTION NAME: WEBAUTH_PMGR_DeleteBlackListByHostIP
 * PURPOSE: delete black host data by lport
 * INPUT:   ip_addr
 *          lport
 * OUTPUT:  none
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none
 */
UI32_T WEBAUTH_PMGR_DeleteBlackListByHostIP(UI32_T ip_addr, UI32_T lport)
{
    UI8_T msgbuf[SYSFUN_SIZE_OF_MSG(WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_U32_U32_Data_T))];
    SYSFUN_Msg_T *msg_p = (SYSFUN_Msg_T *)msgbuf;
    WEBAUTH_MGR_IPCMsg_U32_U32_Data_T *data_p;

    data_p = WEBAUTH_MGR_MSG_DATA(msg_p);
    data_p->u32_1 = ip_addr;
    data_p->u32_2 = lport;

    WEBAUTH_PMGR_SendMsg(WEBAUTH_MGR_IPC_CMD_DELETE_BLACK_LIST_BY_HOST_IP,
        msg_p,
        WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_U32_U32_Data_T),
        WEBAUTH_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
        WEBAUTH_TYPE_RETURN_ERROR);

    return WEBAUTH_MGR_MSG_RETVAL(msg_p);
}

/* FUNCTION NAME: WEBAUTH_PMGR_DeleteTryingHost
 * PURPOSE: delete trying host data by lport
 * INPUT:   ip_addr
 *          lport
 * OUTPUT:  none
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none
 */
UI32_T WEBAUTH_PMGR_DeleteTryingHost(UI32_T ip_addr,UI32_T lport)
{
    UI8_T msgbuf[SYSFUN_SIZE_OF_MSG(WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_U32_U32_Data_T))];
    SYSFUN_Msg_T *msg_p = (SYSFUN_Msg_T *)msgbuf;
    WEBAUTH_MGR_IPCMsg_U32_U32_Data_T *data_p;

    data_p = WEBAUTH_MGR_MSG_DATA(msg_p);
    data_p->u32_1 = ip_addr;
    data_p->u32_2 = lport;

    WEBAUTH_PMGR_SendMsg(WEBAUTH_MGR_IPC_CMD_DELETE_TRYING_HOST,
        msg_p,
        WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_U32_U32_Data_T),
        WEBAUTH_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
        WEBAUTH_TYPE_RETURN_ERROR);

    return WEBAUTH_MGR_MSG_RETVAL(msg_p);
}


/* FUNCTION NAME: WEBAUTH_PMGR_GetPortInfoByLPort
 * PURPOSE: This function will get port info by lport
 * INPUT:   lport 
 * OUTPUT:  *lport_info_p
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none 
 */
UI32_T WEBAUTH_PMGR_GetPortInfoByLPort(UI32_T lport, WEBAUTH_TYPE_Port_Info_T *lport_info_p)
{
    UI8_T msgbuf[SYSFUN_SIZE_OF_MSG(WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_PortInfo_U32_T))];
    SYSFUN_Msg_T *msg_p = (SYSFUN_Msg_T *)msgbuf;
    WEBAUTH_MGR_IPCMsg_PortInfo_U32_T *data_p;

    if (NULL == lport_info_p)
    {
        return WEBAUTH_TYPE_RETURN_ERROR;
    }

    data_p = WEBAUTH_MGR_MSG_DATA(msg_p);
    data_p->u32 = lport;

    WEBAUTH_PMGR_SendMsg(WEBAUTH_MGR_IPC_CMD_GET_PORT_INFO_BY_LPORT,
        msg_p,
        WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_PortInfo_U32_T),
        WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_PortInfo_U32_T),
        WEBAUTH_TYPE_RETURN_ERROR);

    if (WEBAUTH_TYPE_RETURN_OK == WEBAUTH_MGR_MSG_RETVAL(msg_p))
    {
        memcpy(lport_info_p, &data_p->port_info, sizeof(data_p->port_info));
    }

    return WEBAUTH_MGR_MSG_RETVAL(msg_p);
}

/* FUNCTION NAME: WEBAUTH_PMGR_GetNextPortInfoByLPort
 * PURPOSE: this function will get next port info
 * INPUT:   *index 
 * OUTPUT:  *index 
 *          *lport_info_p
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTE:    key is index; index=0: get initial
 */
UI32_T WEBAUTH_PMGR_GetNextPortInfoByLPort(UI32_T *index, WEBAUTH_TYPE_Port_Info_T *lport_info_p)
{
    UI8_T msgbuf[SYSFUN_SIZE_OF_MSG(WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_PortInfo_U32_T))];
    SYSFUN_Msg_T *msg_p = (SYSFUN_Msg_T *)msgbuf;
    WEBAUTH_MGR_IPCMsg_PortInfo_U32_T *data_p;

    if (   (NULL == index)
        || (NULL == lport_info_p))
    {
        return WEBAUTH_TYPE_RETURN_ERROR;
    }

    data_p = WEBAUTH_MGR_MSG_DATA(msg_p);
    data_p->u32 = *index;
    memcpy(&data_p->port_info, lport_info_p, sizeof(data_p->port_info));

    WEBAUTH_PMGR_SendMsg(WEBAUTH_MGR_IPC_CMD_GETNEXT_PORT_INFO_BY_LPORT,
        msg_p,
        WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_PortInfo_U32_T),
        WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_PortInfo_U32_T),
        WEBAUTH_TYPE_RETURN_ERROR);

    if (WEBAUTH_TYPE_RETURN_OK == WEBAUTH_MGR_MSG_RETVAL(msg_p))
    {
        *index = data_p->u32;
        memcpy(lport_info_p, &data_p->port_info, sizeof(data_p->port_info));
    }

    return WEBAUTH_MGR_MSG_RETVAL(msg_p);
}

/* FUNCTION NAME: WEBAUTH_PMGR_GetSuccessHostByLPortAndIP
 * PURPOSE: This function will get success host by lport and ip
 * INPUT:   host_info_p
 * OUTPUT:  host_info_p
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none
 */
UI32_T WEBAUTH_PMGR_GetSuccessHostByLPortAndIP(
        WEBAUTH_TYPE_Host_Info_T  *host_info_p)
{
    UI8_T msgbuf[SYSFUN_SIZE_OF_MSG(WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_HostInfo_T))];
    SYSFUN_Msg_T *msg_p = (SYSFUN_Msg_T *)msgbuf;
    WEBAUTH_MGR_IPCMsg_HostInfo_T *data_p;

    if (NULL == host_info_p)
    {
        return WEBAUTH_TYPE_RETURN_ERROR;
    }

    data_p = WEBAUTH_MGR_MSG_DATA(msg_p);
    memcpy(&data_p->host_info, host_info_p, sizeof(data_p->host_info));

    WEBAUTH_PMGR_SendMsg(WEBAUTH_MGR_IPC_CMD_GET_SUCCESS_HOST_BY_LPORT_AND_IP,
        msg_p,
        WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_HostInfo_T),
        WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_HostInfo_T),
        WEBAUTH_TYPE_RETURN_ERROR);

    if (WEBAUTH_TYPE_RETURN_OK == WEBAUTH_MGR_MSG_RETVAL(msg_p))
    {
        memcpy(host_info_p, &data_p->host_info, sizeof(data_p->host_info));
    }

    return WEBAUTH_MGR_MSG_RETVAL(msg_p);
}
        
/* FUNCTION NAME: WEBAUTH_PMGR_ProcessAuth
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
UI32_T WEBAUTH_PMGR_ProcessAuth(
        char *username_p, char *password_p, UI32_T ip_addr, UI32_T lport)
{
    UI8_T msgbuf[SYSFUN_SIZE_OF_MSG(WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_Username_Password_U32_U32_T))];
    SYSFUN_Msg_T *msg_p = (SYSFUN_Msg_T *)msgbuf;
    WEBAUTH_MGR_IPCMsg_Username_Password_U32_U32_T *data_p;

    if (   (NULL == username_p)
        || (NULL == password_p))
    {
        return WEBAUTH_TYPE_RETURN_ERROR;
    }

    data_p = WEBAUTH_MGR_MSG_DATA(msg_p);
    strncpy(data_p->username, username_p, sizeof(data_p->username));
    strncpy(data_p->password, password_p, sizeof(data_p->password));
    data_p->u32_1 = ip_addr;
    data_p->u32_2 = lport;

    WEBAUTH_PMGR_SendMsg(WEBAUTH_MGR_IPC_CMD_PROCESS_AUTH,
        msg_p,
        WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_Username_Password_U32_U32_T),
        WEBAUTH_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
        WEBAUTH_TYPE_RETURN_ERROR);

    return WEBAUTH_MGR_MSG_RETVAL(msg_p);
}

/* FUNCTION NAME: WEBAUTH_PMGR_ProcessTimerEvent
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
void WEBAUTH_PMGR_ProcessTimerEvent(void)
{
    UI8_T msgbuf[SYSFUN_SIZE_OF_MSG(WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_Type_T))];
    SYSFUN_Msg_T *msg_p = (SYSFUN_Msg_T *)msgbuf;

    WEBAUTH_PMGR_SendMsg(WEBAUTH_MGR_IPC_CMD_PROCESS_TIMER_EVENT,
        msg_p,
        WEBAUTH_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
        WEBAUTH_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
        WEBAUTH_TYPE_RETURN_ERROR);
}

/* FUNCTION NAME: WEBAUTH_PMGR_GetNextSuccessHostByLPortAndIndex
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
UI32_T WEBAUTH_PMGR_GetNextSuccessHostByLPortAndIP(
        WEBAUTH_TYPE_Host_Info_T  *host_info_p)
{
    UI8_T msgbuf[SYSFUN_SIZE_OF_MSG(WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_HostInfo_T))];
    SYSFUN_Msg_T *msg_p = (SYSFUN_Msg_T *)msgbuf;
    WEBAUTH_MGR_IPCMsg_HostInfo_T *data_p;

    if (NULL == host_info_p)
    {
        return WEBAUTH_TYPE_RETURN_ERROR;
    }

    data_p = WEBAUTH_MGR_MSG_DATA(msg_p);
    memcpy(&data_p->host_info, host_info_p, sizeof(data_p->host_info));

    WEBAUTH_PMGR_SendMsg(WEBAUTH_MGR_IPC_CMD_GETNEXT_SUCCESS_HOST_BY_LPORT_AND_IP,
        msg_p,
        WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_HostInfo_T),
        WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_HostInfo_T),
        WEBAUTH_TYPE_RETURN_ERROR);

    if (WEBAUTH_TYPE_RETURN_OK == WEBAUTH_MGR_MSG_RETVAL(msg_p))
    {
        memcpy(host_info_p, &data_p->host_info, sizeof(data_p->host_info));
    }

    return WEBAUTH_MGR_MSG_RETVAL(msg_p);
}

/* FUNCTION NAME: WEBAUTH_PMGR_GetNextSuccessHostByLPortAndSuccIndex
 * PURPOSE: This function will get success host by lport and success index
 * INPUT:   lport_p, succ_index_p
 * OUTPUT:  host_info_p, lport_p, succ_index_p
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none
 */
UI32_T WEBAUTH_PMGR_GetNextSuccessHostByLPortAndSuccIndex(WEBAUTH_TYPE_Host_Info_T *host_info_p, UI32_T *lport_p, UI8_T *succ_index_p)
{
    UI8_T msgbuf[SYSFUN_SIZE_OF_MSG(WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_HostInfo_U32_U32_T))];
    SYSFUN_Msg_T *msg_p = (SYSFUN_Msg_T *)msgbuf;
    WEBAUTH_MGR_IPCMsg_HostInfo_U32_U32_T *data_p;

    if (NULL == host_info_p)
    {
        return WEBAUTH_TYPE_RETURN_ERROR;
    }

    data_p = WEBAUTH_MGR_MSG_DATA(msg_p);
    data_p->u32_1 = *lport_p;
    data_p->u32_2 = *succ_index_p;

    WEBAUTH_PMGR_SendMsg(WEBAUTH_MGR_IPC_CMD_GETNEXT_SUCCESS_HOST_BY_LPORT_AND_SUCC_INDEX,
        msg_p,
        WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_HostInfo_U32_U32_T),
        WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_HostInfo_U32_U32_T),
        WEBAUTH_TYPE_RETURN_ERROR);

    if (WEBAUTH_TYPE_RETURN_OK == WEBAUTH_MGR_MSG_RETVAL(msg_p))
    {
        *lport_p = data_p->u32_1;
        *succ_index_p = data_p->u32_2;
        memcpy(host_info_p, &data_p->host_info, sizeof(data_p->host_info));
    }

    return WEBAUTH_MGR_MSG_RETVAL(msg_p);
}
        
/* FUNCTION NAME: WEBAUTH_PMGR_GetSuccessHostByLPort
 * PURPOSE: This function will get success host by lport and index
 * INPUT:   lport
 *          index
 * OUTPUT:  *host_info_p
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   for ecah logical port, it will have at most 
 *          SYS_ADPT_MAX_NBR_OF_WEBAUTH_HOST_PER_PORT success hosts.
 */
UI32_T WEBAUTH_PMGR_GetSuccessHostByLPort(
        WEBAUTH_TYPE_Host_Info_T  *host_info_p, UI32_T lport, UI8_T index)
{
    UI8_T msgbuf[SYSFUN_SIZE_OF_MSG(WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_HostInfo_U32_U32_T))];
    SYSFUN_Msg_T *msg_p = (SYSFUN_Msg_T *)msgbuf;
    WEBAUTH_MGR_IPCMsg_HostInfo_U32_U32_T *data_p;

    if (NULL == host_info_p)
    {
        return WEBAUTH_TYPE_RETURN_ERROR;
    }

    data_p = WEBAUTH_MGR_MSG_DATA(msg_p);
    memcpy(&data_p->host_info, host_info_p, sizeof(data_p->host_info));
    data_p->u32_1 = lport;
    data_p->u32_2 = index;

    WEBAUTH_PMGR_SendMsg(WEBAUTH_MGR_IPC_CMD_GET_SUCCESS_HOST_BY_LPORT,
        msg_p,
        WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_HostInfo_U32_U32_T),
        WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_HostInfo_U32_U32_T),
        WEBAUTH_TYPE_RETURN_ERROR);

    if (WEBAUTH_TYPE_RETURN_OK == WEBAUTH_MGR_MSG_RETVAL(msg_p))
    {
        memcpy(host_info_p, &data_p->host_info, sizeof(data_p->host_info));
    }

    return WEBAUTH_MGR_MSG_RETVAL(msg_p);
}

/* FUNCTION NAME: WEBAUTH_PMGR_GetSuccessCountByLPort
 * PURPOSE: This function will get success count by lport 
 * INPUT:   lport          
 * OUTPUT:  *success_count_p
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none
 */
UI32_T WEBAUTH_PMGR_GetSuccessCountByLPort(UI32_T lport, UI16_T *success_count_p)
{
    UI8_T msgbuf[SYSFUN_SIZE_OF_MSG(WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_U32_U32_Data_T))];
    SYSFUN_Msg_T *msg_p = (SYSFUN_Msg_T *)msgbuf;
    WEBAUTH_MGR_IPCMsg_U32_U32_Data_T *data_p;

    if (NULL == success_count_p)
    {
        return WEBAUTH_TYPE_RETURN_ERROR;
    }

    data_p = WEBAUTH_MGR_MSG_DATA(msg_p);
    data_p->u32_1 = lport;

    WEBAUTH_PMGR_SendMsg(WEBAUTH_MGR_IPC_CMD_GET_SUCCESS_COUNT_BY_LPORT,
        msg_p,
        WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_U32_U32_Data_T),
        WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_U32_U32_Data_T),
        WEBAUTH_TYPE_RETURN_ERROR);

    if (WEBAUTH_TYPE_RETURN_OK == WEBAUTH_MGR_MSG_RETVAL(msg_p))
    {
        *success_count_p = data_p->u32_2;
    }

    return WEBAUTH_MGR_MSG_RETVAL(msg_p);
}

/* FUNCTION NAME: WEBAUTH_PMGR_GetTryingHostCount
 * PURPOSE: This function will get trying host login attempt by ip
 * INPUT:   ip_addr
 *          lport
 *          *login_attempt_p 
 * OUTPUT:  *login_attempt_p 
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none
 */
UI32_T WEBAUTH_PMGR_GetTryingHostCount(
       UI32_T ip_addr, UI32_T lport,UI8_T *login_attempt_p )
{
    UI8_T msgbuf[SYSFUN_SIZE_OF_MSG(WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_U32_U32_U32_Data_T))];
    SYSFUN_Msg_T *msg_p = (SYSFUN_Msg_T *)msgbuf;
    WEBAUTH_MGR_IPCMsg_U32_U32_U32_Data_T *data_p;

    if (NULL == login_attempt_p)
    {
        return WEBAUTH_TYPE_RETURN_ERROR;
    }

    data_p = WEBAUTH_MGR_MSG_DATA(msg_p);
    data_p->u32_1 = ip_addr;
    data_p->u32_2 = lport;

    WEBAUTH_PMGR_SendMsg(WEBAUTH_MGR_IPC_CMD_GET_TRYING_HOST_COUNT,
        msg_p,
        WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_U32_U32_U32_Data_T),
        WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_U32_U32_U32_Data_T),
        WEBAUTH_TYPE_RETURN_ERROR);

    if (WEBAUTH_TYPE_RETURN_OK == WEBAUTH_MGR_MSG_RETVAL(msg_p))
    {
        *login_attempt_p = data_p->u32_3;
    }

    return WEBAUTH_MGR_MSG_RETVAL(msg_p);
}
              
/* FUNCTION NAME: WEBAUTH_PMGR_SetDebugFlag
 * PURPOSE: This function will set debug flag
 * INPUT:   debug_flag
 * OUTPUT:  none
 * RETURN:  none.
 * NOTES:   none.
 */
void WEBAUTH_PMGR_SetDebugFlag(UI32_T debug_flag)
{
    WEBAUTH_PMGR_SetUI32Data(WEBAUTH_MGR_IPC_CMD_SET_DEBUG_FLAG, debug_flag);
}

/* FUNCTION NAME: WEBAUTH_PMGR_GetDebugFlag
 * PURPOSE: This function will set debug flag
 * INPUT:   void
 * OUTPUT:  none
 * RETURN:  WEBAUTH_PMGR_debug_flag
 * NOTES:   none.
 */
UI32_T WEBAUTH_PMGR_GetDebugFlag(void)
{
    UI32_T debug_flag;

    WEBAUTH_PMGR_GetUI32Data(WEBAUTH_MGR_IPC_CMD_GET_DEBUG_FLAG, &debug_flag);

    return debug_flag;
}

/* FUNCTION NAME: WEBAUTH_PMGR_GetWebAuthInfo
 * PURPOSE: This function will get webauth global status
 * INPUT:   none
 * OUTPUT:  *sys_info_p
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none
 */
UI32_T WEBAUTH_PMGR_GetWebAuthInfo(WEBAUTH_TYPE_System_Info_T *sys_info_p)
{
    UI8_T msgbuf[SYSFUN_SIZE_OF_MSG(WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_SystemInfo_T))];
    SYSFUN_Msg_T *msg_p = (SYSFUN_Msg_T *)msgbuf;
    WEBAUTH_MGR_IPCMsg_SystemInfo_T *data_p;

    if (NULL == sys_info_p)
    {
        return WEBAUTH_TYPE_RETURN_ERROR;
    }

    data_p = WEBAUTH_MGR_MSG_DATA(msg_p);

    WEBAUTH_PMGR_SendMsg(WEBAUTH_MGR_IPC_CMD_GET_WEBAUTH_INFO,
        msg_p,
        WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_SystemInfo_T),
        WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_SystemInfo_T),
        WEBAUTH_TYPE_RETURN_ERROR);

    if (WEBAUTH_TYPE_RETURN_OK == WEBAUTH_MGR_MSG_RETVAL(msg_p))
    {
        memcpy(sys_info_p, &data_p->system_info, sizeof(data_p->system_info));
    }

    return WEBAUTH_MGR_MSG_RETVAL(msg_p);
}

/* FUNCTION NAME: WEBAUTH_PMGR_IsIPValidByLPort
 * PURPOSE: This function will check ip is valid by lport
 * INPUT:   ip_addr
 *          lport
 * OUTPUT:  none
 * RETURN:  TRUE: this ip is valid for this port
 *          FALSE: this ip is in valid for this port, or global/port
 *                 status for webauth is disabled.
 * NOTES:   none.
 */
BOOL_T WEBAUTH_PMGR_IsIPValidByLPort(UI32_T ip_addr, UI32_T lport)
{
    UI8_T msgbuf[SYSFUN_SIZE_OF_MSG(WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_U32_U32_Data_T))];
    SYSFUN_Msg_T *msg_p = (SYSFUN_Msg_T *)msgbuf;
    WEBAUTH_MGR_IPCMsg_U32_U32_Data_T *data_p;

    data_p = WEBAUTH_MGR_MSG_DATA(msg_p);
    data_p->u32_1 = ip_addr;
    data_p->u32_2 = lport;

    WEBAUTH_PMGR_SendMsg(WEBAUTH_MGR_IPC_CMD_IS_IP_VALID_BY_LPORT,
        msg_p,
        WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_U32_U32_Data_T),
        WEBAUTH_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
        (BOOL_T) FALSE);

    return WEBAUTH_MGR_MSG_RETVAL(msg_p);
}

/* LOCAL SUBPROGRAM BODIES
 */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - WEBAUTH_PMGR_SendMsg
 *-------------------------------------------------------------------------
 * PURPOSE: To send an IPC message.
 * INPUT  : cmd       - the WEBAUTH message command.
 *          msg_p     - the buffer of the IPC message.
 *          req_size  - the size of WEBAUTH request message.
 *          res_size  - the size of WEBAUTH response message.
 *          ret_val   - the return value to set when IPC is failed.
 * OUTPUT : msg_p     - the response message.
 * RETURN : None
 * NOTE   : None
 *-------------------------------------------------------------------------
 */
static void WEBAUTH_PMGR_SendMsg(UI32_T cmd, SYSFUN_Msg_T *msg_p, UI32_T req_size, UI32_T res_size, UI32_T ret_val)
{
    UI32_T ret;

    msg_p->cmd = SYS_MODULE_WEBAUTH;
    msg_p->msg_size = req_size;

    WEBAUTH_MGR_MSG_CMD(msg_p) = cmd;

    ret = SYSFUN_SendRequestMsg(ipcmsgq_handle,
        msg_p,
        SYSFUN_TIMEOUT_WAIT_FOREVER,
        SYSFUN_SYSTEM_EVENT_IPCMSG,
        res_size,
        msg_p);

    /* If IPC is failed, set return value as ret_val.
     */
    if (ret != SYSFUN_OK)
    {
        WEBAUTH_MGR_MSG_RETVAL(msg_p) = ret_val;
    }
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - WEBAUTH_PMGR_GetUI32Data
 *-------------------------------------------------------------------------
 * PURPOSE: To get a required data by specified lport and ipc_cmd.
 * INPUT  : ipc_cmd   - the WEBAUTH IPC message command
 *          lport     - lport
 * OUTPUT : out_p     - the output buffer of required data
 * RETURN : WEBAUTH_TYPE_RETURN_OK/WEBAUTH_TYPE_RETURN_OK
 * NOTE   : None
 *-------------------------------------------------------------------------
 */
static UI32_T WEBAUTH_PMGR_GetUI32Data(UI32_T ipc_cmd, UI32_T *out_p)
{
    UI8_T msgbuf[SYSFUN_SIZE_OF_MSG(WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_U32_Data_T))];
    SYSFUN_Msg_T *msg_p = (SYSFUN_Msg_T *)msgbuf;
    WEBAUTH_MGR_IPCMsg_U32_Data_T *data_p;

    if (NULL == out_p)
    {
        return WEBAUTH_TYPE_RETURN_ERROR;
    }

    data_p = WEBAUTH_MGR_MSG_DATA(msg_p);

    WEBAUTH_PMGR_SendMsg(ipc_cmd,
        msg_p,
        WEBAUTH_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
        WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_U32_Data_T),
        WEBAUTH_TYPE_RETURN_ERROR);

    if (WEBAUTH_TYPE_RETURN_OK == WEBAUTH_MGR_MSG_RETVAL(msg_p))
    {
        *out_p = data_p->u32;
    }

    return WEBAUTH_MGR_MSG_RETVAL(msg_p);
}

static UI32_T WEBAUTH_PMGR_GetUI16Data(UI32_T ipc_cmd, UI16_T *out_p)
{
    UI32_T value;
    UI32_T ret;

    ret = WEBAUTH_PMGR_GetUI32Data(ipc_cmd, &value);
    *out_p = value;

    return ret;
}

static UI32_T WEBAUTH_PMGR_GetUI8Data(UI32_T ipc_cmd, UI8_T *out_p)
{
    UI32_T value;
    UI32_T ret;

    ret = WEBAUTH_PMGR_GetUI32Data(ipc_cmd, &value);
    *out_p = value;

    return ret;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - WEBAUTH_PMGR_SetUI32Data
 *-------------------------------------------------------------------------
 * PURPOSE: To set a required data by specified lport and ipc_cmd.
 * INPUT  : ipc_cmd   - the WEBAUTH IPC message command
 *          lport     - lport
 *          data      - the data to set
 * OUTPUT : None
 * RETURN : WEBAUTH_TYPE_RETURN_OK/WEBAUTH_TYPE_RETURN_ERROR
 * NOTE   : None
 *-------------------------------------------------------------------------
 */
static UI32_T WEBAUTH_PMGR_SetUI32Data(UI32_T ipc_cmd, UI32_T data)
{
    UI8_T msgbuf[SYSFUN_SIZE_OF_MSG(WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_U32_Data_T))];
    SYSFUN_Msg_T *msg_p = (SYSFUN_Msg_T *)msgbuf;
    WEBAUTH_MGR_IPCMsg_U32_Data_T *data_p;

    data_p = WEBAUTH_MGR_MSG_DATA(msg_p);
    data_p->u32 = data;

    WEBAUTH_PMGR_SendMsg(ipc_cmd,
        msg_p,
        WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_U32_Data_T),
        WEBAUTH_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
        WEBAUTH_TYPE_RETURN_ERROR);

    return WEBAUTH_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - WEBAUTH_PMGR_GetUI32DataByLport
 *-------------------------------------------------------------------------
 * PURPOSE: To get a required data by specified lport and ipc_cmd.
 * INPUT  : ipc_cmd   - the WEBAUTH IPC message command
 *          lport     - lport
 * OUTPUT : out_p     - the output buffer of required data
 * RETURN : WEBAUTH_TYPE_RETURN_OK/WEBAUTH_TYPE_RETURN_ERROR
 * NOTE   : None
 *-------------------------------------------------------------------------
 */
static UI32_T WEBAUTH_PMGR_GetUI32DataByLport(UI32_T ipc_cmd, UI32_T lport, UI32_T *out_p)
{
    UI8_T msgbuf[SYSFUN_SIZE_OF_MSG(WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_U32_U32_Data_T))];
    SYSFUN_Msg_T *msg_p = (SYSFUN_Msg_T *)msgbuf;
    WEBAUTH_MGR_IPCMsg_U32_U32_Data_T *data_p;

    if (NULL == out_p)
    {
        return WEBAUTH_TYPE_RETURN_ERROR;
    }

    data_p = WEBAUTH_MGR_MSG_DATA(msg_p);
    data_p->u32_1 = lport;

    WEBAUTH_PMGR_SendMsg(ipc_cmd,
        msg_p,
        WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_U32_U32_Data_T),
        WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_U32_U32_Data_T),
        WEBAUTH_TYPE_RETURN_ERROR);

    if (WEBAUTH_TYPE_RETURN_OK == WEBAUTH_MGR_MSG_RETVAL(msg_p))
    {
        *out_p = data_p->u32_2;
    }

    return WEBAUTH_MGR_MSG_RETVAL(msg_p);
}

static UI32_T WEBAUTH_PMGR_GetUI8DataByLport(UI32_T ipc_cmd, UI32_T lport, UI8_T *out_p)
{
    UI32_T value = 0;
    UI32_T ret;

    ret = WEBAUTH_PMGR_GetUI32DataByLport(ipc_cmd, lport, &value);
    *out_p = value;

    return ret;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - WEBAUTH_PMGR_SetUI32DataByLport
 *-------------------------------------------------------------------------
 * PURPOSE: To set a required data by specified lport and ipc_cmd.
 * INPUT  : ipc_cmd   - the WEBAUTH IPC message command
 *          lport     - lport
 *          data      - the data to set
 * OUTPUT : None
 * RETURN : WEBAUTH_TYPE_RETURN_OK/WEBAUTH_TYPE_RETURN_ERROR
 * NOTE   : None
 *-------------------------------------------------------------------------
 */
static UI32_T WEBAUTH_PMGR_SetUI32DataByLport(UI32_T ipc_cmd, UI32_T lport, UI32_T data)
{
    UI8_T msgbuf[SYSFUN_SIZE_OF_MSG(WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_U32_U32_Data_T))];
    SYSFUN_Msg_T *msg_p = (SYSFUN_Msg_T *)msgbuf;
    WEBAUTH_MGR_IPCMsg_U32_U32_Data_T *data_p;

    data_p = WEBAUTH_MGR_MSG_DATA(msg_p);
    data_p->u32_1 = lport;
    data_p->u32_2 = data;

    WEBAUTH_PMGR_SendMsg(ipc_cmd,
        msg_p,
        WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_U32_U32_Data_T),
        WEBAUTH_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
        WEBAUTH_TYPE_RETURN_ERROR);

    return WEBAUTH_MGR_MSG_RETVAL(msg_p);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - WEBAUTH_PMGR_GetRunningUI32Data
 *-------------------------------------------------------------------------
 * PURPOSE: To get a required data by specified lport and ipc_cmd.
 * INPUT  : ipc_cmd   - the WEBAUTH IPC message command
 *          lport     - lport
 * OUTPUT : out_p     - the output buffer of required data
 * RETURN : SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE   : None
 *-------------------------------------------------------------------------
 */
static SYS_TYPE_Get_Running_Cfg_T WEBAUTH_PMGR_GetRunningUI32Data(UI32_T ipc_cmd, UI32_T *out_p)
{
    UI8_T msgbuf[SYSFUN_SIZE_OF_MSG(WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_U32_Data_T))];
    SYSFUN_Msg_T *msg_p = (SYSFUN_Msg_T *)msgbuf;
    WEBAUTH_MGR_IPCMsg_U32_Data_T *data_p;

    if (NULL == out_p)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    data_p = WEBAUTH_MGR_MSG_DATA(msg_p);

    WEBAUTH_PMGR_SendMsg(ipc_cmd,
        msg_p,
        WEBAUTH_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
        WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_U32_Data_T),
        SYS_TYPE_GET_RUNNING_CFG_FAIL);

    *out_p = data_p->u32;

    return (SYS_TYPE_Get_Running_Cfg_T) WEBAUTH_MGR_MSG_RETVAL(msg_p);
}

static SYS_TYPE_Get_Running_Cfg_T WEBAUTH_PMGR_GetRunningUI16Data(UI32_T ipc_cmd, UI16_T *out_p)
{
    UI32_T value;
    SYS_TYPE_Get_Running_Cfg_T ret;

    ret = WEBAUTH_PMGR_GetRunningUI32Data(ipc_cmd, &value);
    *out_p = value;

    return ret;
}

static SYS_TYPE_Get_Running_Cfg_T WEBAUTH_PMGR_GetRunningUI8Data(UI32_T ipc_cmd, UI8_T *out_p)
{
    UI32_T value;
    SYS_TYPE_Get_Running_Cfg_T ret;

    ret = WEBAUTH_PMGR_GetRunningUI32Data(ipc_cmd, &value);
    *out_p = value;

    return ret;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - WEBAUTH_PMGR_GetRunningUI32DataByLport
 *-------------------------------------------------------------------------
 * PURPOSE: To get a required data by specified lport and ipc_cmd.
 * INPUT  : ipc_cmd   - the WEBAUTH IPC message command
 *          lport     - lport
 * OUTPUT : out_p     - the output buffer of required data
 * RETURN : SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE   : None
 *-------------------------------------------------------------------------
 */
static SYS_TYPE_Get_Running_Cfg_T WEBAUTH_PMGR_GetRunningUI32DataByLport(UI32_T ipc_cmd, UI32_T lport, UI32_T *out_p)
{
    UI8_T msgbuf[SYSFUN_SIZE_OF_MSG(WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_U32_U32_Data_T))];
    SYSFUN_Msg_T *msg_p = (SYSFUN_Msg_T *)msgbuf;
    WEBAUTH_MGR_IPCMsg_U32_U32_Data_T *data_p;

    if (NULL == out_p)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    data_p = WEBAUTH_MGR_MSG_DATA(msg_p);
    data_p->u32_1 = lport;

    WEBAUTH_PMGR_SendMsg(ipc_cmd,
        msg_p,
        WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_U32_U32_Data_T),
        WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_U32_U32_Data_T),
        SYS_TYPE_GET_RUNNING_CFG_FAIL);

    *out_p = data_p->u32_2;

    return (SYS_TYPE_Get_Running_Cfg_T) WEBAUTH_MGR_MSG_RETVAL(msg_p);
}

static SYS_TYPE_Get_Running_Cfg_T WEBAUTH_PMGR_GetRunningUI8DataByLport(UI32_T ipc_cmd, UI32_T lport, UI8_T *out_p)
{
    UI32_T value;
    SYS_TYPE_Get_Running_Cfg_T ret;

    ret = WEBAUTH_PMGR_GetRunningUI32DataByLport(ipc_cmd, lport, &value);
    *out_p = value;

    return ret;
}
