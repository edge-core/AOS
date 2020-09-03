/*-----------------------------------------------------------------------------
 * Module   : ntp_pmgr.h
 *-----------------------------------------------------------------------------
 * PURPOSE  : Provide APIs to access ntp control functions.
 *-----------------------------------------------------------------------------
 * NOTES    :
 *
 *-----------------------------------------------------------------------------
 * HISTORY  : 11/13/2007 - Squid Ro, Create
 *
 *-----------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2007
 *-----------------------------------------------------------------------------
 */

#ifndef NTP_PMGR_H
#define NTP_PMGR_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "ntp_mgr.h"

/* NAME CONSTANT DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - NTP_PMGR_InitiateProcessResources
 *-------------------------------------------------------------------------
 * PURPOSE : Initiate resource used in the calling process.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE  - success
 *           FALSE - failure
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
BOOL_T NTP_PMGR_InitiateProcessResources(void);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_PMGR_SetStatus
 *------------------------------------------------------------------------------
 * PURPOSE  : Set NTP status : enable/disable
 * INPUT    : status you want to set. 1: VAL_ntpStatus_enabled,
 *                                    2: VAL_ntpStatus_disabled
 * OUTPUT   : none
 * RETURN   : TRUE : If success
 *            FALSE:
 * NOTES    : none
 *------------------------------------------------------------------------------*/
BOOL_T NTP_PMGR_SetStatus(UI32_T status);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_PMGR_GetStatus
 *------------------------------------------------------------------------------
 * PURPOSE  : Get NTP status  : enable/disable
 * INPUT    : none
 * OUTPUT   : current NTP status. 1: VAL_ntpStatus_enabled,
 *                                 2: VAL_ntpStatus_disabled
 * RETURN   : TRUE : If success
 *            FALSE:
 * NOTES    : none
 *------------------------------------------------------------------------------*/
BOOL_T NTP_PMGR_GetStatus(UI32_T *status);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - NTP_PMGR_GetRunningStatus
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the status of disable/enable mapping of system
 * INPUT:    None
 * OUTPUT:   status -- VAL_ntpStatus_enabled/VAL_ntpStatus_disabled
 * RETURN:   SYS_TYPE_GET_RUNNING_CFG_FAIL -- error (system is not in MASTER mode)
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 *           SYS_TYPE_GET_RUNNING_CFG_SUCCESS -- different from default value
 * NOTE:     default value is disable  1:enable, 2:disable
 *---------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T NTP_PMGR_GetRunningStatus(UI32_T *status);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_PMGR_GetLastUpdateTime
 *------------------------------------------------------------------------------
 * PURPOSE  : Return the last update time for show ntp
 * INPUT    :
 * OUTPUT   :
 * RETURN   :
 * NOTES    :
 *------------------------------------------------------------------------------*/
BOOL_T NTP_PMGR_GetLastUpdateTime(I32_T *time);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_PMGR_GetPollTime
 *------------------------------------------------------------------------------
 * PURPOSE  : Get the polling interval
 * INPUT    : A buffer is used to be put data
 * OUTPUT   : polling interval used in unicast mode
 * RETURN   : TRUE : If success
 *            FALSE:
 * NOTES    : none
 *------------------------------------------------------------------------------*/
BOOL_T NTP_PMGR_GetPollTime(UI32_T *polltime);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_PMGR_SetServiceOperationMode
 *------------------------------------------------------------------------------
 * PURPOSE  : Set ntp service operation mode , unicast/brocast/anycast/disable
 * INPUT    : VAL_ntpServiceMode_unicast = 1
 *            VAL_ntpServiceMode_broadcast = 2
 *            VAL_ntpServiceMode_anycast = 3
 *
 * OUTPUT   : none
 * RETURN   : TRUE : If success
 *            FALSE:
 * NOTES    : none
 *------------------------------------------------------------------------------*/
BOOL_T NTP_PMGR_SetServiceOperationMode(UI32_T mode);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_PMGR_GetServiceOperationMode
 *------------------------------------------------------------------------------
 * PURPOSE  : Set ntp service operation mode , Unicast/brocast/anycast
 * INPUT    : VAL_ntpServiceMode_unicast = 1
 *            VAL_ntpServiceMode_broadcast = 2
 *            VAL_ntpServiceMode_anycast = 3
 * OUTPUT   : none
 * RETURN   : TRUE : If success
 *            FALSE:
 * NOTES    : none
 *------------------------------------------------------------------------------*/
BOOL_T NTP_PMGR_GetServiceOperationMode(UI32_T *mode);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - NTP_PMGR_GetRunningServiceMode
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the operation mode mapping of system
 * INPUT:    None
 * OUTPUT:   VAL_ntpServiceMode_unicast = 1
 *           VAL_ntpServiceMode_broadcast = 2
 *           VAL_ntpServiceMode_anycast = 3
 * RETURN:   SYS_TYPE_GET_RUNNING_CFG_FAIL -- error (system is not in MASTER mode)
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 *           SYS_TYPE_GET_RUNNING_CFG_SUCCESS -- different from default value
 * NOTE: default value is unicast
 *---------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T NTP_PMGR_GetRunningServiceMode(UI32_T *servicemode);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_PMGR_AddServerIp
 *------------------------------------------------------------------------------
 * PURPOSE  : Add a ntp server ip to OM
 * INPUT    : 1.index 2. ip address
 * OUTPUT   : none
 * RETURN   : TRUE : If success
 *            FALSE:
 * NOTES    : Ex: index1=10.1.1.1 index2=NULL index3=NULL
 *            if we want to add an IP to index3, we can not add it
 *            becasue index2=NULL
 *------------------------------------------------------------------------------*/
BOOL_T NTP_PMGR_AddServerIp(UI32_T ipaddress, UI32_T version, UI32_T keyid);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_PMGR_DeleteServerIp
 *------------------------------------------------------------------------------
 * PURPOSE  : Delete a server ip from OM
 * INPUT    : index (MIN_ntpServerIndex <= index <= MAX_ntpServerIndex)
 * OUTPUT   : none
 * RETURN   : TRUE : If success
 *            FALSE:
 * NOTES    : 1.Delete one server ip will cause the ip behind deleted-ip was deleted.
 *            2.The ip is zero if it is deleted
 *            3. If ntp srv ip are 192.168.1.1, 192.168.1.2, 192.168.1.3.
 *               If delete 192.168.1.2, sort om to 192.168.1.1, 192.168.1.3, 0.0.0.0.
 *------------------------------------------------------------------------------*/
BOOL_T NTP_PMGR_DeleteServerIp(UI32_T ipaddress);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_PMGR_DeleteAllServerIp
 *------------------------------------------------------------------------------
 * PURPOSE  : Delete all servers ip from OM
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE : If success
 *            FALSE:
 * NOTES    :
 *------------------------------------------------------------------------------*/
BOOL_T NTP_PMGR_DeleteAllServerIp(void);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_PMGR_GetNextServer
 *------------------------------------------------------------------------------
 * PURPOSE  : Get a next server entry  from OM using ip address as index
 * INPUT    : 1.point to the server list.
 * OUTPUT   : buffer contain information
 * RETURN   : TRUE : If success
 *            FALSE: If get failed, or the assigned ip was cleared.
 * NOTES    : none
 *------------------------------------------------------------------------------*/
BOOL_T NTP_PMGR_GetNextServer(UI32_T *ipaddress, UI32_T *version, UI32_T *keyid);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_PMGR_GetLastUpdateServer
 *------------------------------------------------------------------------------
 * PURPOSE  : Return the last update server for show ntp
 * INPUT    :
 * OUTPUT   :
 * RETURN   :
 * NOTES    :
 *------------------------------------------------------------------------------*/
BOOL_T NTP_PMGR_GetLastUpdateServer(NTP_MGR_SERVER_T *serv);

/*------------------------------------------------------------------------------
  * FUNCTION NAME - NTP_PMGR_FindServer
  *------------------------------------------------------------------------------
  * PURPOSE  : Get a server entry  from OM using ip address as index
  * INPUT    : ip address
  * OUTPUT   : buffer contain information
  * RETURN   : TRUE : If find
  *            FALSE: If not found
  * NOTES    : This is only used in cli
  *------------------------------------------------------------------------------*/
BOOL_T  NTP_PMGR_FindServer(UI32_T ipaddress, NTP_MGR_SERVER_T *server);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_PMGR_FindNextServer
 *------------------------------------------------------------------------------
 * PURPOSE  : Get a next server entry  from OM using ip address as index
 * INPUT    : 1.point to the server list.
 * OUTPUT   : buffer contain information
 * RETURN   : TRUE : If success
 *            FALSE: If get failed, or the assigned ip was cleared.
 * NOTES    : none
 *------------------------------------------------------------------------------*/
BOOL_T NTP_PMGR_FindNextServer(UI32_T ipadd, NTP_MGR_SERVER_T *serv);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_PMGR_SetAuthStatus
 *------------------------------------------------------------------------------
 * PURPOSE  : Set NTP authenticate status : enable/disable
 * INPUT    : status you want to set. 1 :VAL_ntpAuthenticateStatus_enabled, 2 :VAL_ntpAuthenticateStatus_disabled
 * OUTPUT   : none
 * RETURN   : TRUE : If success
 *            FALSE:
 * NOTES    : none
 *------------------------------------------------------------------------------*/
BOOL_T NTP_PMGR_SetAuthStatus(UI32_T status);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_PMGR_GetAuthStatus
 *------------------------------------------------------------------------------
 * PURPOSE  : Get NTP authenticate status  : enable/disable
 * INPUT    : none
 * OUTPUT   : current NTP status.1 :VAL_ntpAuthenticateStatus_enabled, 2: VAL_ntpAuthenticateStatus_disabled
 * RETURN   : TRUE : If success
 *            FALSE:
 * NOTES    : none
 *------------------------------------------------------------------------------*/
BOOL_T NTP_PMGR_GetAuthStatus(UI32_T *status);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - NTP_PMGR_GetRunningAuthStatus
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the authenticate status of disable/enable mapping of system
 * INPUT:    None
 * OUTPUT:   status -- VAL_ntpAuthenticateStatus_enabled/VAL_ntpAuthenticateStatus_disabled
 * RETURN:   SYS_TYPE_GET_RUNNING_CFG_FAIL -- error (system is not in MASTER mode)
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 *           SYS_TYPE_GET_RUNNING_CFG_SUCCESS -- different from default value
 * NOTE:     default value is disable  1:enable, 2:disable
 *---------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T  NTP_PMGR_GetRunningAuthStatus(UI32_T *status);

/*------------------------------------------------------------------------------
* FUNCTION NAME - NTP_PMGR_AddAuthKey
*------------------------------------------------------------------------------
* PURPOSE  : Add an authentication key to the list
* INPUT    : 1.index 2. md5_key
* OUTPUT   : none
* RETURN   : TRUE : If success
*            FALSE:
* NOTES    : none
*------------------------------------------------------------------------------*/
BOOL_T  NTP_PMGR_AddAuthKey(UI32_T index, char *encryptedkey);

/*------------------------------------------------------------------------------
* FUNCTION NAME - NTP_PMGR_AddAuthKey_Encrypted
*------------------------------------------------------------------------------
* PURPOSE  : Add an authentication encrypted key to the list ,use in provison
* INPUT    : 1.index 2. encryptedkey
* OUTPUT   : none
* RETURN   : TRUE : If success
*            FALSE:
* NOTES    : none
*------------------------------------------------------------------------------*/
BOOL_T  NTP_PMGR_AddAuthKey_Encrypted(UI32_T index, char *encryptedkey);

/*------------------------------------------------------------------------------
* FUNCTION NAME - NTP_PMGR_SetAuthKeyStatus
*------------------------------------------------------------------------------
* PURPOSE  : Set an authentication key status
* INPUT    : 1.index 2. status : VAL_ntpAuthKeyStatus_valid/VAL_ntpAuthKeyStatus_invalid
* OUTPUT   : none
* RETURN   : TRUE : If success
*            FALSE:
* NOTES    : only used for snmp do set authentication key status
*------------------------------------------------------------------------------*/
BOOL_T  NTP_PMGR_SetAuthKeyStatus(UI32_T index, UI32_T status);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_PMGR_DeleteAuthKey
 *------------------------------------------------------------------------------
 * PURPOSE  : Delete a designed Authentication key
 * INPUT    : keyid
 * OUTPUT   : none
 * RETURN   : TRUE : If success
 *            FALSE:
 * NOTES    :
 *------------------------------------------------------------------------------*/
BOOL_T  NTP_PMGR_DeleteAuthKey(UI32_T keyid);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_PMGR_DeleteAllAuthKey
 *------------------------------------------------------------------------------
 * PURPOSE  : Delete all Authenticaion key
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE : If success
 *            FALSE:
 * NOTES    :
 *------------------------------------------------------------------------------*/
BOOL_T  NTP_PMGR_DeleteAllAuthKey(void);

/*------------------------------------------------------------------------------
* FUNCTION NAME - NTP_PMGR_GetNextKey
*------------------------------------------------------------------------------
* PURPOSE  : Get next key
* INPUT    :
* OUTPUT   :
* RETURN   : TRUE : If find
*            FALSE: If not found
* NOTES    : none
*------------------------------------------------------------------------------*/
BOOL_T  NTP_PMGR_GetNextKey(NTP_MGR_AUTHKEY_T *authkey);

/*------------------------------------------------------------------------------
* FUNCTION NAME - NTP_PMGR_FindKey
*------------------------------------------------------------------------------
* PURPOSE  : check whether the key exist
* INPUT    : keyid
* OUTPUT   :
* RETURN   : TRUE : If find
*            FALSE: If not found
* NOTES    : none
*------------------------------------------------------------------------------*/
BOOL_T  NTP_PMGR_FindKey(UI32_T keyid, NTP_MGR_AUTHKEY_T *authkey);

#endif /* NTP_PMGR_H */

