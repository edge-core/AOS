/*-----------------------------------------------------------------------------
 * Module   : sntp_pmgr.h
 *-----------------------------------------------------------------------------
 * PURPOSE  : Provide APIs to access sntp control functions.
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

#ifndef SNTP_PMGR_H
#define SNTP_PMGR_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sntp_mgr.h"

/* NAME CONSTANT DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_PMGR_InitiateProcessResources
 *-------------------------------------------------------------------------
 * PURPOSE : Initiate resource used in the calling process.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE  - success
 *           FALSE - failure
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
BOOL_T SNTP_PMGR_InitiateProcessResources(void);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_PMGR_AddServerIp
 *------------------------------------------------------------------------------
 * PURPOSE  : Add a sntp server ip to OM
 * INPUT    : 1.index 2. ip address
 * OUTPUT   : none
 * RETURN   : TRUE : If success
 *            FALSE:
 * NOTES    : Ex: index1=10.1.1.1 index2=NULL index3=NULL
 *            if we want to add an IP to index3, we can not add it
 *            becasue index2=NULL
 *------------------------------------------------------------------------------*/
BOOL_T SNTP_PMGR_AddServerIp(UI32_T index, L_INET_AddrIp_T *ipaddress);

 /*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_PMGR_AddServerIpForCLI
 *------------------------------------------------------------------------------
 * PURPOSE  : Add a sntp server ip to OM
 * INPUT    : ipaddress
 * OUTPUT   : none
 * RETURN   : TRUE : If success
 *            FALSE:
 * NOTES    : none
 *------------------------------------------------------------------------------*/
BOOL_T  SNTP_PMGR_AddServerIpForCLI(L_INET_AddrIp_T *ipaddress);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_PMGR_DeleteAllServerIp
 *------------------------------------------------------------------------------
 * PURPOSE  : Delete all servers ip from OM
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE : If success
 *            FALSE:
 * NOTES    :
 *------------------------------------------------------------------------------*/
BOOL_T  SNTP_PMGR_DeleteAllServerIp(void);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_PMGR_DeleteServerIp
 *------------------------------------------------------------------------------
 * PURPOSE  : Delete a server ip from OM
 * INPUT    : index (MIN_sntpServerIndex <= index <= MAX_sntpServerIndex)
 * OUTPUT   : none
 * RETURN   : TRUE : If success
 *            FALSE:
 * NOTES    : 1.Delete one server ip will cause the ip behind deleted-ip was deleted.
 *            2.The ip is zero if it is deleted
 *            3. If sntp srv ip are 192.168.1.1, 192.168.1.2, 192.168.1.3.
 *               If delete 192.168.1.2, sort om to 192.168.1.1, 192.168.1.3, 0.0.0.0.
 *------------------------------------------------------------------------------*/
BOOL_T SNTP_PMGR_DeleteServerIp(UI32_T index);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_PMGR_DeleteServerIpForCLI
 *------------------------------------------------------------------------------
 * PURPOSE  : Delete a server ip from OM
 * INPUT    : ipaddress
 * OUTPUT   : none
 * RETURN   : TRUE : If success
 *            FALSE:
 * NOTES    : Delete one server ip will cause the SNTP server address entry be relocated,
 *            such as: server entry  : 10.1.1.1, 10.1.1.2, 10.1.1.3 remove 10.1.1.2
 *                                 --> 10.1.1.1, 10.1.1.3
 *------------------------------------------------------------------------------*/
BOOL_T  SNTP_PMGR_DeleteServerIpForCLI(L_INET_AddrIp_T * ipaddress);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_PMGR_GetCurrentServer
 *------------------------------------------------------------------------------
 * PURPOSE  : Get current server of getting current time
 * INPUT    : buffer to get server
 * OUTPUT   : ip address
 * RETURN   : TRUE : If success
 *            FALSE: If failed
 * NOTES    :
 *------------------------------------------------------------------------------*/
BOOL_T SNTP_PMGR_GetCurrentServer(L_INET_AddrIp_T *ipaddress);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_PMGR_GetPollTime
 *------------------------------------------------------------------------------
 * PURPOSE  : Get the polling interval
 * INPUT    : A buffer is used to be put data
 * OUTPUT   : polling interval used in unicast mode
 * RETURN   : TRUE : If success
 *            FALSE:
 * NOTES    : none
 *------------------------------------------------------------------------------*/
BOOL_T  SNTP_PMGR_GetPollTime(UI32_T *polltime);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNTP_PMGR_GetRunningPollTime
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the status of disable/enable mapping of system
 * INPUT:    None
 * OUTPUT:   polling time
 * RETURN:   SYS_TYPE_GET_RUNNING_CFG_FAIL -- error (system is not in MASTER mode)
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 *           SYS_TYPE_GET_RUNNING_CFG_SUCCESS -- different from default value
 * NOTE:    default value is 16 secconds
 *---------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T  SNTP_PMGR_GetRunningPollTime(UI32_T *polltime);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNTP_PMGR_GetRunningServiceMode
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the operation mode mapping of system
 * INPUT:    None
 * OUTPUT:   VAL_sntpServiceMode_unicast = 1
 *           VAL_sntpServiceMode_broadcast = 2
 *           VAL_sntpServiceMode_anycast = 3
 * RETURN:   SYS_TYPE_GET_RUNNING_CFG_FAIL -- error (system is not in MASTER mode)
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 *           SYS_TYPE_GET_RUNNING_CFG_SUCCESS -- different from default value
 * NOTE: default value is unicast
 *---------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T  SNTP_PMGR_GetRunningServiceMode(UI32_T *servicemode);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNTP_PMGR_GetRunningStatus
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the status of disable/enable mapping of system
 * INPUT:    None
 * OUTPUT:   status -- VAL_sntpStatus_enabled/VAL_sntpStatus_disabled
 * RETURN:   SYS_TYPE_GET_RUNNING_CFG_FAIL -- error (system is not in MASTER mode)
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 *           SYS_TYPE_GET_RUNNING_CFG_SUCCESS -- different from default value
 * NOTE:     default value is disable  1:enable, 2:disable
 *---------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T  SNTP_PMGR_GetRunningStatus(UI32_T *status);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_PMGR_GetServerIp
 *------------------------------------------------------------------------------
 * PURPOSE  : Get a server entry  from OM using ip address as index
 * INPUT    : 1.index (MIN_sntpServerIndex <= index <= MAX_sntpServerIndex)
 *            2.point to buffer that can contain information of a server
 * OUTPUT   : buffer contain information
 * RETURN   : TRUE : If success
 *            FALSE: If get failed.
 * NOTES    : none
 *------------------------------------------------------------------------------*/
BOOL_T  SNTP_PMGR_GetServerIp(UI32_T index,L_INET_AddrIp_T *ipaddress);

 /*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_PMGR_GetServiceOperationMode
 *------------------------------------------------------------------------------
 * PURPOSE  : Set sntp service operation mode , Unicast/brocast/anycast
 * INPUT    : VAL_sntpServiceMode_unicast = 1
 *            VAL_sntpServiceMode_broadcast = 2
 *            VAL_sntpServiceMode_anycast = 3
 * OUTPUT   : none
 * RETURN   : TRUE : If success
 *            FALSE:
 * NOTES    : none
 *------------------------------------------------------------------------------*/
BOOL_T  SNTP_PMGR_GetServiceOperationMode(UI32_T *mode);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_PMGR_GetStatus
 *------------------------------------------------------------------------------
 * PURPOSE  : Get SNTP status  : enable/disable
 * INPUT    : none
 * OUTPUT   : current SNTP status. 1: VAL_sntpStatus_enabled,
 *                                 2: VAL_sntpStatus_disabled
 * RETURN   : TRUE : If success
 *            FALSE:
 * NOTES    : none
 *------------------------------------------------------------------------------*/
BOOL_T  SNTP_PMGR_GetStatus(UI32_T *status);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_PMGR_SetPollTime
 *------------------------------------------------------------------------------
 * PURPOSE  : Set the polling interval
 * INPUT    : polling interval used in unicast mode
 * OUTPUT   : none
 * RETURN   : TRUE : If success
 *            FALSE:
 * NOTES    :
 *------------------------------------------------------------------------------*/
BOOL_T  SNTP_PMGR_SetPollTime(UI32_T polltime);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_PMGR_SetServiceOperationMode
 *------------------------------------------------------------------------------
 * PURPOSE  : Set sntp service operation mode , unicast/brocast/anycast/disable
 * INPUT    : VAL_sntpServiceMode_unicast = 1
 *            VAL_sntpServiceMode_broadcast = 2
 *            VAL_sntpServiceMode_anycast = 3
 *
 * OUTPUT   : none
 * RETURN   : TRUE : If success
 *            FALSE:
 * NOTES    : none
 *------------------------------------------------------------------------------*/
BOOL_T SNTP_PMGR_SetServiceOperationMode(UI32_T mode);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_PMGR_SetStatus
 *------------------------------------------------------------------------------
 * PURPOSE  : Set SNTP status : enable/disable
 * INPUT    : status you want to set. 1: VAL_sntpStatus_enabled,
 *                                    2: VAL_sntpStatus_disabled
 * OUTPUT   : none
 * RETURN   : TRUE : If success
 *            FALSE:
 * NOTES    : none
 *------------------------------------------------------------------------------*/
BOOL_T SNTP_PMGR_SetStatus(UI32_T status);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_PMGR_GetNextServerIp
 *------------------------------------------------------------------------------
 * PURPOSE  : Get next server entry using ip address as index
 * INPUT    : 1.index (start from 0)
 *            2.point to buffer that can contain server ip address
 * OUTPUT   : buffer contain information
 * RETURN   : TRUE : If success
 *            FALSE:
 * NOTES    : if input index is 0, then it will find first entry in table.
 *------------------------------------------------------------------------------*/
BOOL_T SNTP_PMGR_GetNextServerIp(UI32_T *index,L_INET_AddrIp_T *ipaddress);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_PMGR_DeleteServerIpForSNMP
 *------------------------------------------------------------------------------
 * PURPOSE  : Delete a server ip from OM
 * INPUT      : index
 * OUTPUT   : none
 * RETURN   : TRUE : If success
 *            FALSE:
 * NOTES    : Delete one server ip will cause the SNTP server address entry be relocated,
 *            such as: server entry  : 10.1.1.1, 10.1.1.2, 10.1.1.3 remove 10.1.1.2
 *                                 --> 10.1.1.1, 10.1.1.3
 *------------------------------------------------------------------------------*/
BOOL_T SNTP_PMGR_DeleteServerIpForSNMP(UI32_T index);

#endif /* SNTP_PMGR_H */

