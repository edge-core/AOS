#ifndef	WEBAUTH_PMGR_H
#define	WEBAUTH_PMGR_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "webauth_type.h"

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
BOOL_T WEBAUTH_PMGR_InitiateProcessResources(void);

/* FUNCTION NAME: WEBAUTH_PMGR_SetSystemStatus
 * PURPOSE: This function will set global status of webauth
 * INPUT:   status
 * OUTPUT:  VAL_webauthEnable_enabled/VAL_webauthEnable_disabled
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none 
 */
UI32_T WEBAUTH_PMGR_SetSystemStatus(UI8_T status);

/* FUNCTION NAME: WEBAUTH_PMGR_GetSystemStatus
 * PURPOSE: This function will return the status of webath
 * INPUT:   *status
 * OUTPUT:  VAL_webauthEnable_enabled/VAL_webauthEnable_disabled
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none 
 */
UI32_T WEBAUTH_PMGR_GetSystemStatus(UI8_T *status);

/* FUNCTION NAME: WEBAUTH_PMGR_SetStatusByLPort 
 * PURPOSE: this function will set webauth per port status
 * INPUT:   lport
 *          status
 * OUTPUT:  VAL_webAuthPortConfigStatus_enabled/VAL_webAuthPortConfigStatus_disabled
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTE:    none 
 */
UI32_T WEBAUTH_PMGR_SetStatusByLPort(UI32_T lport, UI8_T status);

/* FUNCTION NAME: WEBAUTH_PMGR_GetStatusByLPort
 * PURPOSE: This function will return webauth per port status 
 * INPUT:   lport
 * OUTPUT:  *status -- VAL_webAuthPortConfigStatus_enabled/VAL_webAuthPortConfigStatus_disabled
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none 
 */
UI32_T WEBAUTH_PMGR_GetStatusByLPort(UI32_T lport, UI8_T *status);

/* FUNCTION NAME: WEBAUTH_PMGR_SetQuietPeriod
 * PURPOSE: this function will set quiet period
 * INPUT:   quiet_period
 * OUTPUT:  None
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTE:    None 
 */
UI32_T WEBAUTH_PMGR_SetQuietPeriod(UI16_T quiet_period);

/* FUNCTION NAME: WEBAUTH_PMGR_GetQuietPeriod
 * PURPOSE: This function will return the quiet period value
 * INPUT:   none.
 * OUTPUT:  *quiet_period
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none 
 */
UI32_T WEBAUTH_PMGR_GetQuietPeriod(UI16_T *quiet_period);

/* FUNCTION NAME: WEBAUTH_PMGR_SetMaxLoginAttempts
 * PURPOSE: this function will set max login attempts
 * INPUT:   max_login_attempt
 * OUTPUT:  None
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTE:    None
 */
UI32_T WEBAUTH_PMGR_SetMaxLoginAttempts(UI8_T max_login_attempt);

/* FUNCTION NAME: WEBAUTH_PMGR_GetMaxLoginAttempts
 * PURPOSE: This function will return global value of max login attempt
 * INPUT:   none
 * OUTPUT:  *max login attempt
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none 
 */
UI32_T WEBAUTH_PMGR_GetMaxLoginAttempts(UI8_T *max_login_attempt);

/* FUNCTION NAME: WEBAUTH_PMGR_SetSystemSessionTimeout
 * PURPOSE: This function will set global session timeout value
 * INPUT:   session_timeout
 * OUTPUT:  None
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTE:    None
 */
UI32_T WEBAUTH_PMGR_SetSystemSessionTimeout(UI16_T session_timeout);

/* FUNCTION NAME: WEBAUTH_PMGR_GetSystemSessionTimeout
 * PURPOSE: This function will get global session timeout value
 * INPUT:   none.
 * OUTPUT:  *session_timeout
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none
 */
UI32_T WEBAUTH_PMGR_GetSystemSessionTimeout(UI16_T *session_timeout);

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
        char *url, WEBAUTH_TYPE_EXTERNAL_URL_T url_type);

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
        char *url, WEBAUTH_TYPE_EXTERNAL_URL_T url_type);

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
        UI32_T ip_addr, UI32_T lport, UI8_T *login_attempt);
#endif

/* FUNCTION NAME: WEBAUTH_PMGR_ReAuthByLPort
 * PURPOSE: This function will reinit all hosts in this lport
 * INPUT:   lport
 * OUTPUT:  none
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR,
 *          WEBAUTH_TYPE_RETURN_NO_EFFECT
 * NOTES:   none
 */
UI32_T WEBAUTH_PMGR_ReAuthByLPort(UI32_T lport);

/* FUNCTION NAME: WEBAUTH_PMGR_ReAuthByLPort
 * PURPOSE: This function will reinit specify host in this lport
 * INPUT:   lport
 *          ip_addr
 * OUTPUT:  none
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR,
 *          WEBAUTH_TYPE_RETURN_NO_EFFECT
 * NOTES:   none
 */
UI32_T WEBAUTH_PMGR_ReAuthHostByLPort(UI32_T lport, UI32_T ip_addr);

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
                            UI16_T *session_timeout);

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
                            UI16_T *quiet_period);

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
                            UI8_T *login_attempt);

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
SYS_TYPE_Get_Running_Cfg_T WEBAUTH_PMGR_GetRunningSystemStatus(UI8_T *status);

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
                            UI32_T lport,UI8_T *status);

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
SYS_TYPE_Get_Running_Cfg_T  WEBAUTH_PMGR_GetRunningLoginURL(char *url);

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
SYS_TYPE_Get_Running_Cfg_T  WEBAUTH_PMGR_GetRunningLoginFailURL(char *url);

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
SYS_TYPE_Get_Running_Cfg_T  WEBAUTH_PMGR_GetRunningLoginSuccessURL(char *url);

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
       WEBAUTH_TYPE_Host_Info_T  *host_info_p, UI32_T lport, UI8_T *index_p);

/* FUNCTION NAME: WEBAUTH_PMGR_CreateSuccessHostByLPort
 * PURPOSE: create success host data by lport
 * INPUT:   ip_addr
 *          lport
 * OUTPUT:  none
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none
 */
UI32_T WEBAUTH_PMGR_CreateSuccessHostByLPort(UI32_T ip_addr, UI32_T lport);

/* FUNCTION NAME: WEBAUTH_PMGR_DeleteSuccessListByHostIP
 * PURPOSE: delete success host data by lport
 * INPUT:   ip_addr
 *          lport
 * OUTPUT:  none
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none
 */
UI32_T WEBAUTH_PMGR_DeleteSuccessListByHostIP(UI32_T ip_addr, UI32_T lport);

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
UI32_T WEBAUTH_PMGR_CreateBlackHostByLPort(UI32_T ip_addr, UI32_T lport);

/* FUNCTION NAME: WEBAUTH_PMGR_CheckIPIsBlackByLPort
 * PURPOSE: This function will check whether host is in black
 * INPUT:   ip_addr
 *          lport
 * OUTPUT:  none
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   if is black , return WEBAUTH_TYPE_RETURN_OK
 *          else return WEBAUTH_TYPE_RETURN_ERROR
 */
UI32_T WEBAUTH_PMGR_CheckIPIsBlackByLPort(UI32_T ip_addr, UI32_T lport);

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
UI32_T WEBAUTH_PMGR_CreateTryingHost(UI32_T ip_addr, UI32_T lport);

/* FUNCTION NAME: WEBAUTH_PMGR_DeleteBlackListByHostIP
 * PURPOSE: delete black host data by lport
 * INPUT:   ip_addr
 *          lport
 * OUTPUT:  none
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none
 */
UI32_T WEBAUTH_PMGR_DeleteBlackListByHostIP(UI32_T ip_addr, UI32_T lport);

/* FUNCTION NAME: WEBAUTH_PMGR_DeleteTryingHost
 * PURPOSE: delete trying host data by lport
 * INPUT:   ip_addr
 *          lport
 * OUTPUT:  none
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none
 */
UI32_T WEBAUTH_PMGR_DeleteTryingHost(UI32_T ip_addr,UI32_T lport);


/* FUNCTION NAME: WEBAUTH_PMGR_GetPortInfoByLPort
 * PURPOSE: This function will get port info by lport
 * INPUT:   lport 
 * OUTPUT:  *lport_info_p
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none 
 */
UI32_T WEBAUTH_PMGR_GetPortInfoByLPort(UI32_T lport, WEBAUTH_TYPE_Port_Info_T *lport_info_p);

/* FUNCTION NAME: WEBAUTH_PMGR_GetNextPortInfoByLPort
 * PURPOSE: this function will get next port info
 * INPUT:   *index 
 * OUTPUT:  *index 
 *          *lport_info_p
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTE:    key is index; index=0: get initial
 */
UI32_T WEBAUTH_PMGR_GetNextPortInfoByLPort(UI32_T *index, WEBAUTH_TYPE_Port_Info_T *lport_info_p);

/* FUNCTION NAME: WEBAUTH_PMGR_GetSuccessHostByLPortAndIP
 * PURPOSE: This function will get success host by lport and ip
 * INPUT:   host_info_p
 * OUTPUT:  host_info_p
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none
 */
UI32_T WEBAUTH_PMGR_GetSuccessHostByLPortAndIP(
        WEBAUTH_TYPE_Host_Info_T  *host_info_p);
        
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
        char *username_p, char *password_p, UI32_T ip_addr, UI32_T lport);

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
void WEBAUTH_PMGR_ProcessTimerEvent(void);

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
        WEBAUTH_TYPE_Host_Info_T  *host_info_p);
        
/* FUNCTION NAME: WEBAUTH_PMGR_GetSuccessHostByLPortAndIP
 * PURPOSE: This function will get success host by lport and ip
 * INPUT:   host_info_p->lport -- key 1
 *          host_info_p->ip    -- key 2
 * OUTPUT:  host_info_p
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none
 */
UI32_T WEBAUTH_PMGR_GetSuccessHostByLPortAndIP(
        WEBAUTH_TYPE_Host_Info_T  *host_info_p);

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
        WEBAUTH_TYPE_Host_Info_T  *host_info_p, UI32_T lport, UI8_T index);

/* FUNCTION NAME: WEBAUTH_PMGR_GetSuccessCountByLPort
 * PURPOSE: This function will get success count by lport 
 * INPUT:   lport          
 * OUTPUT:  *success_count_p
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none
 */
UI32_T WEBAUTH_PMGR_GetSuccessCountByLPort(UI32_T lport, UI16_T *success_count_p);

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
       UI32_T ip_addr, UI32_T lport,UI8_T *login_attempt_p );
              
/* FUNCTION NAME: WEBAUTH_PMGR_SetDebugFlag
 * PURPOSE: This function will set debug flag
 * INPUT:   debug_flag
 * OUTPUT:  none
 * RETURN:  none.
 * NOTES:   none.
 */
void WEBAUTH_PMGR_SetDebugFlag(UI32_T debug_flag);

/* FUNCTION NAME: WEBAUTH_PMGR_GetDebugFlag
 * PURPOSE: This function will set debug flag
 * INPUT:   void
 * OUTPUT:  none
 * RETURN:  WEBAUTH_PMGR_debug_flag
 * NOTES:   none.
 */
UI32_T WEBAUTH_PMGR_GetDebugFlag(void);

/* FUNCTION NAME: WEBAUTH_PMGR_GetWebAuthInfo
 * PURPOSE: This function will get webauth global status
 * INPUT:   none
 * OUTPUT:  *sys_info_p
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none
 */
UI32_T WEBAUTH_PMGR_GetWebAuthInfo(WEBAUTH_TYPE_System_Info_T *sys_info_p);

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
BOOL_T WEBAUTH_PMGR_IsIPValidByLPort(UI32_T ip_addr, UI32_T lport);

/* FUNCTION NAME: WEBAUTH_PMGR_GetNextSuccessHostByLPortAndSuccIndex
 * PURPOSE: This function will get success host by lport and success index
 * INPUT:   lport_p, succ_index_p
 * OUTPUT:  host_info_p, lport_p, succ_index_p
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none
 */
UI32_T WEBAUTH_PMGR_GetNextSuccessHostByLPortAndSuccIndex(WEBAUTH_TYPE_Host_Info_T *host_info_p, UI32_T *lport_p, UI8_T *succ_index_p);

#endif /* WEBAUTH_PMGR_H */
