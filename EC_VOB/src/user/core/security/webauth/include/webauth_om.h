/* MODULE NAME: WEBAUTH_OM.H
 * PURPOSE:
 *      Declarations for the WEBAUTH OM
 * NOTES:
 *
 *
 * HISTORY:
 *    01/29/07 --  Rich Lee, Create
 *
 * Copyright(C)      Accton Corporation, 2007
 */


#ifndef _WEBAUTH_OM_H
#define _WEBAUTH_OM_H


/* INCLUDE FILE DECLARATTIONS
 */
#include <sys_type.h>
#include "webauth_type.h"
#include "sysrsc_mgr.h"

/* NAMING CONSTANT DECLARATIONS
 */


/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */


/* FUNCTION NAME: WEBAUTH_OM_InitWebAuthOm
 * PURPOSE: This function will init all data of OM
 * INPUT:   none
 * OUTPUT:  none
 * RETURN:  none
 * NOTES:   none
 */
void WEBAUTH_OM_InitWebAuthOm(void);

/* FUNCTION NAME: WEBAUTH_OM_AttachSystemResources
 * PURPOSE: This function will init all share memory data
 * INPUT:   none
 * OUTPUT:  none
 * RETURN:  none
 * NOTES:   none
 */
void WEBAUTH_OM_AttachSystemResources(void);

/* FUNCTION NAME: WEBAUTH_OM_GetShMemInfo
 * PURPOSE: This function will provide shared memory information for SYSRSC.
 * INPUT:   none
 * OUTPUT:  segid_p  -  shared memory segment id
 *          seglen_p -  length of the shared memroy segment
 * RETURN:  none
 * NOTES:   none
 */
void WEBAUTH_OM_GetShMemInfo(SYSRSC_MGR_SEGID_T *segid_p, UI32_T *seglen_p);

/* FUNCTION NAME: WEBAUTH_OM_ClearWebAuthOm
 * PURPOSE: This function will init all data of OM
 * INPUT:   none
 * OUTPUT:  none
 * RETURN:  none
 * NOTES:   this function mainly for disable webauth, then free list and
 *          temp data
 */
void WEBAUTH_OM_ClearWebAuthOm(void);

/* FUNCTION NAME: WEBAUTH_OM_GetSystemInfoPointer
 * PURPOSE: This function will return the point of webauth system info.
 * INPUT:   none
 * OUTPUT:  none
 * RETURN:  WEBAUTH_TYPE_System_Info_T*
 * NOTES:   none
 */
WEBAUTH_TYPE_System_Info_T* WEBAUTH_OM_GetSystemInfoPointer(void);

/* FUNCTION NAME: WEBAUTH_OM_SetSystemStatus
 * PURPOSE: this function will set webauth status
 * INPUT:   VAL_webauthEnable_enabled / VAL_webauthEnable_disabled
 * OUTPUT:  None
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTE:    None
 */
UI32_T WEBAUTH_OM_SetSystemStatus(UI8_T status);

/* FUNCTION NAME: WEBAUTH_OM_GetSystemStatus
 * PURPOSE: This function will return the status of webath
 * INPUT:   none
 * OUTPUT:  none
 * RETURN:  VAL_webauthEnable_enabled/VAL_webauthEnable_disabled
 * NOTES:   none
 */
UI8_T WEBAUTH_OM_GetSystemStatus(void);

/* FUNCTION NAME: WEBAUTH_OM_GetLPortInfoPTR
 * PURPOSE: This function will get port ptr by lport
 * INPUT:   lport
 * OUTPUT:  none
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none
 */
WEBAUTH_TYPE_Port_Info_T* WEBAUTH_OM_GetLPortInfoPTR(UI32_T lport);

/* FUNCTION NAME: WEBAUTH_OM_SetQuietPeriod
 * PURPOSE: this function will set quiet period
 * INPUT:   quiet_period
 * OUTPUT:  None
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTE:    None
 */
UI32_T WEBAUTH_OM_SetQuietPeriod(UI16_T quiet_period);

/* FUNCTION NAME: WEBAUTH_OM_GetQuietPeriod
 * PURPOSE: This function will return the quiet period value
 * INPUT:   *quiet_period_p
 * OUTPUT:  quiet period value
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none
 */
UI32_T WEBAUTH_OM_GetQuietPeriod(UI16_T *quiet_period_p);

/* FUNCTION NAME: WEBAUTH_OM_SetMaxLoginAttempts
 * PURPOSE: this function will set max login attempts
 * INPUT:   max_login_attempt value
 * OUTPUT:  None
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTE:    None
 */
UI32_T WEBAUTH_OM_SetMaxLoginAttempts(UI8_T max_login_attempt);

/* FUNCTION NAME: WEBAUTH_OM_GetMaxLoginAttempts
 * PURPOSE: This function will return global value of max login attempt
 * INPUT:   *max_login_attempt_p
 * OUTPUT:  *max login attempt_p value
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none
 */
UI32_T WEBAUTH_OM_GetMaxLoginAttempts(UI8_T *max_login_attempt_p);

/* FUNCTION NAME: WEBAUTH_OM_SetSystemSessionTimeout
 * PURPOSE: This function will set global session timeout value
 * INPUT:   session_timeout
 * OUTPUT:  None
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTE:    None
 */
UI32_T WEBAUTH_OM_SetSessionTimeout(UI16_T session_timeout);

/* FUNCTION NAME: WEBAUTH_OM_GetSystemSessionTimeout
 * PURPOSE: This function will get global session timeout value
 * INPUT:   *session_timeout_p
 * OUTPUT:  sesstion timeout value
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none
 */
UI32_T WEBAUTH_OM_GetSessionTimeout(UI16_T *session_timeout_p);

/* FUNCTION NAME: WEBAUTH_OM_GetExternalLoginURL
 * PURPOSE: This function get external login URL
 * INPUT:   *url_p,
 *          url type:
 *             {WEBAUTH_EXTERNAL_URL_Login
 *              WEBAUTH_EXTERNAL_URL_Login_fail
 *              WEBAUTH_EXTERNAL_URL_Login_Success}
 * OUTPUT:  webauth_system_info.external_login_url
 * RETURN:  WEBAUTH_TYPE_RETURN_OK
 * NOTES:   none
 */
UI32_T WEBAUTH_OM_GetExternalLoginURL(char *url_p);

/* FUNCTION NAME: WEBAUTH_OM_SetExternalLoginURL
 * PURPOSE: This function set external login URL
 * INPUT:   *url_p,
 *          url type:
 *             {WEBAUTH_EXTERNAL_URL_Login
 *              WEBAUTH_EXTERNAL_URL_Login_fail
 *              WEBAUTH_EXTERNAL_URL_Login_Success}
 * OUTPUT:  none
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none
 */
UI32_T WEBAUTH_OM_SetExternalLoginURL(char *url_p);

/* FUNCTION NAME: WEBAUTH_OM_GetExternalLoginFailURL
 * PURPOSE: This function will get login fail URL
 * INPUT:   *url_p
 * OUTPUT:  *url_p
 * RETURN:  WEBAUTH_TYPE_RETURN_OK
 * NOTES:   none
 */
UI32_T WEBAUTH_OM_GetExternalLoginFailURL(char *url_p);

/* FUNCTION NAME: WEBAUTH_OM_SetExternalLoginFailURL
 * PURPOSE: This function will set login fail URL
 * INPUT:   *url_p
 * OUTPUT:  none
 * RETURN:  WEBAUTH_TYPE_RETURN_OK
 * NOTES:   none
 */
UI32_T WEBAUTH_OM_SetExternalLoginFailURL(char *url_p);

/* FUNCTION NAME: WEBAUTH_OM_GetExternalLoginSuccessURL
 * PURPOSE: This function will get external login success URL
 * INPUT:   *url_p
 * OUTPUT:  *url_p
 * RETURN:  WEBAUTH_TYPE_RETURN_OK
 * NOTES:   none
 */
UI32_T WEBAUTH_OM_GetExternalLoginSuccessURL(char *url_p);

/* FUNCTION NAME: WEBAUTH_OM_SetExternalLoginSuccessURL
 * PURPOSE: This function will set login success URL
 * INPUT:   *url_p
 * OUTPUT:  none
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none
 */
UI32_T WEBAUTH_OM_SetExternalLoginSuccessURL(char *url_p);

/* FUNCTION NAME: WEBAUTH_OM_GetBlackListHead
 * PURPOSE: This function will get black list head
 * INPUT:   none
 * OUTPUT:  none
 * RETURN:  webauth_om_black_list_head_p
 * NOTES:   none
 */
WEBAUTH_TYPE_Host_Info_T* WEBAUTH_OM_GetBlackListHead(void);

/* FUNCTION NAME: WEBAUTH_OM_GetTryingListHead
 * PURPOSE: This function will get trying list head
 * INPUT:   none
 * OUTPUT:  none
 * RETURN:  webauth_om_host_trying_list_p
 * NOTES:   none
 */
WEBAUTH_TYPE_Host_Trying_T * WEBAUTH_OM_GetTryingListHead(void);

/* FUNCTION NAME: WEBAUTH_OM_GetTotalTryingCount
 * PURPOSE: This function will get total trying count
 * INPUT:   *count_p
 * OUTPUT:  *count_p
 * RETURN:  WEBAUTH_TYPE_RETURN_OK
 * NOTES:   none
 */
UI32_T WEBAUTH_OM_GetTotalTryingCount(UI16_T *count_p);

/* FUNCTION NAME: WEBAUTH_OM_SetSuccessListHead
 * PURPOSE: This function will set scuuess list head
 * INPUT:   *success_list_p
 * OUTPUT:  none
 * RETURN:  WEBAUTH_TYPE_RETURN_OK
 * NOTES:   none
 */
UI32_T WEBAUTH_OM_SetSuccessListHead(WEBAUTH_TYPE_Host_Info_T *success_list_p);

/* FUNCTION NAME: WEBAUTH_OM_GetSuccessListHead
 * PURPOSE: This function will get success list head
 * INPUT:   none
 * OUTPUT:  none
 * RETURN:  webauth_om_success_list_head_p
 * NOTES:   none
 */
WEBAUTH_TYPE_Host_Info_T *WEBAUTH_OM_GetSuccessListHead(void);

/* FUNCTION NAME: WEBAUTH_OM_SetBlackListHead
 * PURPOSE: This function will set black list head
 * INPUT:   *black_list_p
 * OUTPUT:  none
 * RETURN:  WEBAUTH_TYPE_RETURN_OK
 * NOTES:   none
 */
UI32_T WEBAUTH_OM_SetBlackListHead(WEBAUTH_TYPE_Host_Info_T *black_list_p);

/* FUNCTION NAME: WEBAUTH_OM_SetTryingListHead
 * PURPOSE: This function will set ptr to trying list head
 * INPUT:   *trying_list_p
 * OUTPUT:  none
 * RETURN:  WEBAUTH_TYPE_RETURN_OK
 * NOTES:   none
 */
UI32_T WEBAUTH_OM_SetTryingListHead(WEBAUTH_TYPE_Host_Trying_T *trying_list_p);

/* FUNCTION NAME: WEBAUTH_OM_SetTryingListTail
 * PURPOSE: This function will set ptr to trying list tail
 * INPUT:   *trying_list_tail_p
 * OUTPUT:  none
 * RETURN:  WEBAUTH_TYPE_RETURN_OK
 * NOTES:   none
 */
UI32_T WEBAUTH_OM_SetTryingListTail(WEBAUTH_TYPE_Host_Trying_T *trying_list_tail_p);

/* FUNCTION NAME: WEBAUTH_OM_SetBlackListTail
 * PURPOSE: This function will set black list tail
 * INPUT:   lport
 * OUTPUT:  none
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none
 */
UI32_T WEBAUTH_OM_SetBlackListTail(WEBAUTH_TYPE_Host_Info_T *black_list_p);

/* FUNCTION NAME: WEBAUTH_OM_SetSuccessListTail
 * PURPOSE: This function will set scuuess list tail
 * INPUT:   *success_list_p
 * OUTPUT:  none
 * RETURN:  WEBAUTH_TYPE_RETURN_OK
 * NOTES:   none
 */
UI32_T WEBAUTH_OM_SetSuccessListTail(WEBAUTH_TYPE_Host_Info_T *success_list_p);

/* FUNCTION NAME: WEBAUTH_OM_CreateSuccessHostByLPort
 * PURPOSE: This function will create success host data by lport
 * INPUT:   ip_addr
 *          lport
 * OUTPUT:  none
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none
 */
UI32_T WEBAUTH_OM_CreateSuccessHostByLPort(UI32_T ip_addr, UI32_T lport);

/* FUNCTION NAME: WEBAUTH_OM_CreateBlackHostByLPort
 * PURPOSE: This function will create black host data by lport
 * INPUT:   ip_addr
 *          lport
 * OUTPUT:  none
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none
 */
UI32_T WEBAUTH_OM_CreateBlackHostByLPort(UI32_T ip_addr, UI32_T lport);

/* FUNCTION NAME: WEBAUTH_OM_CreateTryingHostByLPort
 * PURPOSE: This function will create trying host data by lport
 * INPUT:   ip_addr
 *          lport
 * OUTPUT:  none
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ;
 *          WEBAUTH_TYPE_RETURN_ERROR;
 *          WEBAUTH_TYPE_RETURN_HOST_Trying, this is for cgi to check type
 * NOTES:   none
 */
UI32_T WEBAUTH_OM_CreateTryingHostByLPort(UI32_T ip_addr, UI32_T lport);

/* FUNCTION NAME: WEBAUTH_OM_DeleteSuccessHostArrayByIP
 * PURPOSE: This function will delete success host data by lport
 * INPUT:   lport
 *          ip_addr
 * OUTPUT:  none
 * RETURN:  WEBAUTH_TYPE_RETURN_OK
 * NOTES:   none
 */
UI32_T WEBAUTH_OM_DeleteSuccessHostArrayByIP(UI32_T lport, UI32_T ip_addr);

/* FUNCTION NAME: WEBAUTH_OM_DeleteBlackListHostByIP
 * PURPOSE: This function will delete black host data in list
 * INPUT:   ip_addr
 * OUTPUT:  none
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none
 */
UI32_T WEBAUTH_OM_DeleteBlackListHostByIP(UI32_T ip_addr);

/* FUNCTION NAME: WEBAUTH_OM_DeleteTryingHost
 * PURPOSE: This function will delete trying host data in list
 * INPUT:   ip_addr
 *          lport
 * OUTPUT:  none
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none
 */
UI32_T WEBAUTH_OM_DeleteTryingHost(UI32_T ip_addr, UI32_T lport);

/* FUNCTION NAME: WEBAUTH_OM_DeleteSuccessListHost
 * PURPOSE: This function will delete success host data in list
 * INPUT:   ip_addr
 *          lport
 * OUTPUT:  none
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none
 */
UI32_T WEBAUTH_OM_DeleteSuccessListHost(UI32_T ip_addr, UI32_T lport);

/* FUNCTION NAME: WEBAUTH_OM_InitArrayByLPort
 * PURPOSE: This function do init port array by lport
 * INPUT:   lport
 * OUTPUT:  none
 * RETURN:  WEBAUTH_TYPE_RETURN_OK
 * NOTES:   none
 */
UI32_T WEBAUTH_OM_InitArrayByLPort(UI32_T lport);

/* FUNCTION NAME: WEBAUTH_OM_DeleteBlackListHost
 * PURPOSE: This function will delete black host data in list
 * INPUT:   ip_addr
            lport
 * OUTPUT:  none
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none
 */
UI32_T WEBAUTH_OM_DeleteBlackListHost(UI32_T ip_addr, UI32_T lport );

/* FUNCTION NAME: WEBAUTH_OM_SetDebugFlag
 * PURPOSE: This function to set debug flag
 * INPUT:   debug_flag
 * OUTPUT:  none
 * RETURN:  none
 * NOTES:   none
 */
void WEBAUTH_OM_SetDebugFlag(UI32_T debug_flag);

/* FUNCTION NAME: WEBAUTH_OM_IsIPValidByLPort
 * PURPOSE: This function will check ip is valid by lport
 * INPUT:   ip_addr
 *          lport
 * OUTPUT:  none
 * RETURN:  TRUE: this ip is valid for this port
 *          FALSE: this ip is invalid for this port
 * NOTES:   none
 */
BOOL_T WEBAUTH_OM_IsIPValidByLPort(UI32_T ip_addr, UI32_T lport);

#endif  /* End of webauth_om_h */
