/*-----------------------------------------------------------------------------
 * Module   : sntp_om.h
 *-----------------------------------------------------------------------------
 * PURPOSE  : Initialize the database resources and provide some Get/Set
 *            function for accessing the SNTP database.
 *-----------------------------------------------------------------------------
 * NOTES    :
 *
 *-----------------------------------------------------------------------------
 * HISTORY  : 11/13/2007 - Squid Ro, Create
 *                       - Separete parts from sntp_mgr.c, and generate the sntp_om.c
 *-----------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2007
 *-----------------------------------------------------------------------------
 */

#ifndef SNTP_OM_H
#define SNTP_OM_H

/* INCLUDE FILE DECLARATIONS */
#include "sys_type.h"
#include "sys_cpnt.h"
#include "sntp_type.h"

/* NAME CONSTANT DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
typedef struct
{
    UI32_T                  last_update_time;
    UI32_T                  current_time;
    UI32_T                  current_tick;
    L_INET_AddrIp_T         last_update_server;
    L_INET_AddrIp_T         current_server;
    SNTP_SERVER_STATUS_E    current_server_status;
} SNTP_OM_VAR_STATE_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNTP_OM_CreatSem
 *---------------------------------------------------------------------------
 * PURPOSE:  Initiate the semaphore for SNTP objects.
 * INPUT:    None
 * OUTPUT:   None
 * RETURN:   None
 * NOTE:     None
 *---------------------------------------------------------------------------*/
void SNTP_OM_Initiate_System_Resources(void);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNTP_OM_Initiate_System_Resources
 *---------------------------------------------------------------------------
 * PURPOSE:  This function is used to initialize the SNTP database.
 * INPUT:    None
 * OUTPUT:   None
 * RETURN:   None
 * NOTE:     None
 *---------------------------------------------------------------------------*/
BOOL_T SNTP_OM_CreatSem(void);

 /*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_OM_AddServerIp
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
BOOL_T SNTP_OM_AddServerIp(UI32_T index, L_INET_AddrIp_T *ipaddress);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNTP_OM_ClearSntpDatabase
 *---------------------------------------------------------------------------
 * PURPOSE:  This function is used to clear the SNTP database.
 * INPUT:    None
 * OUTPUT:   None
 * RETURN:   None
 * NOTE:     None
 *---------------------------------------------------------------------------*/
void SNTP_OM_ClearSntpDatabase(void);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNTP_OM_InitSntpDatabaseToDefaultValue
 *---------------------------------------------------------------------------
 * PURPOSE:  This function is used to set the SNTP database to default value.
 * INPUT:    None
 * OUTPUT:   None
 * RETURN:   None
 * NOTE:     None
 *---------------------------------------------------------------------------*/
void SNTP_OM_InitSntpDatabaseToDefaultValue(void);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_OM_DeleteServerIp
 *------------------------------------------------------------------------------
 * PURPOSE  : Delete a server ip from OM
 * INPUT    : index (MIN_sntpServerIndex <= index <= MAX_sntpServerIndex)
 * OUTPUT   : none
 * RETURN   : TRUE : If success
 *            FALSE:
 * NOTES    : 1. If sntp srv ip are 192.168.1.1, 192.168.1.2, 192.168.1.3.
                 If delete 192.168.1.2, sort om to 192.168.1.1, 192.168.1.3
 *------------------------------------------------------------------------------*/
BOOL_T SNTP_OM_DeleteServerIp(UI32_T index);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_OM_GetCurrentServer
 *------------------------------------------------------------------------------
 * PURPOSE  : Get current server of getting current time
 * INPUT    : buffer to get server
 * OUTPUT   : UI32_T ip address
 * RETURN   : TRUE : If success
 *            FALSE: If failed
 * NOTES    :
 *------------------------------------------------------------------------------*/
BOOL_T SNTP_OM_GetCurrentServer(L_INET_AddrIp_T *ipaddress);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_OM_GetCurrentUTCTime
 *------------------------------------------------------------------------------
 * PURPOSE  : Get time information of GMT
 * INPUT    : Buffer of  UTC time
 * OUTPUT   : 1.time in seconds from 2001/01/01 00:00:00
 * RETURN   : TRUE : If success
 *            FALSE: If failed
 * NOTES    :
 *------------------------------------------------------------------------------*/
BOOL_T SNTP_OM_GetCurrentUTCTime(UI32_T *time);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_OM_GetServerIpCount
 *------------------------------------------------------------------------------
 * PURPOSE  : Get number of server ip
 * INPUT    :
 * OUTPUT   : UI32_T ip address
 * RETURN   : TRUE : If success
 *            FALSE: If failed
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T SNTP_OM_GetServerIpCount(void);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_OM_GetLastUpdateUTCTime
 *------------------------------------------------------------------------------
 * PURPOSE  : Get time information of last update-time
 * INPUT    : buffer pointer stored time information
 * OUTPUT   : time in seconds from 2001/01/01 00:00:00
 * RETURN   : TRUE : If success
 *            FALSE: SNTP never get time from server.
 * NOTES    :
 *------------------------------------------------------------------------------*/
BOOL_T SNTP_OM_GetLastUpdateUTCTime(UI32_T *time);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_OM_GetPollTime
 *------------------------------------------------------------------------------
 * PURPOSE  : Get the polling interval
 * INPUT    : A buffer is used to be put data
 * OUTPUT   : polling interval used in unicast mode
 * RETURN   : TRUE : If success
 *            FALSE:
 * NOTES    : none
 *------------------------------------------------------------------------------*/
BOOL_T SNTP_OM_GetPollTime(UI32_T *polltime);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_OM_GetServerIp
 *------------------------------------------------------------------------------
 * PURPOSE  : Get a server entry  from OM using ip address as index
 * INPUT    : 1.index (MIN_sntpServerIndex <= index <= MAX_sntpServerIndex)
 *            2.point to buffer that can contain information of a server
 * OUTPUT   : buffer contain information
 * RETURN   : TRUE : If success
 *            FALSE: If get failed.
 * NOTES    : none
 *------------------------------------------------------------------------------*/
BOOL_T SNTP_OM_GetServerIp(UI32_T index, L_INET_AddrIp_T *ipaddress);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_OM_SetServerStatusByIndex
 *------------------------------------------------------------------------------
 * PURPOSE  : Set server status by index
 * INPUT    : index, status
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    :
 *------------------------------------------------------------------------------*/
void SNTP_OM_SetServerStatusByIndex(UI32_T index, SNTP_SERVER_STATUS_E status);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_OM_GetServiceOperationMode
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
BOOL_T SNTP_OM_GetServiceOperationMode(UI32_T *mode);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_OM_GetStatus
 *------------------------------------------------------------------------------
 * PURPOSE  : Get SNTP status  : enable/disable
 * INPUT    : none
 * OUTPUT   : current SNTP status.1 :VAL_sntpStatus_enabled, 2: VAL_sntpStatus_disabled
 * RETURN   : TRUE : If success
 *            FALSE:
 * NOTES    : none
 *------------------------------------------------------------------------------*/
BOOL_T SNTP_OM_GetStatus(UI32_T *status);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_OM_GetVarStateTable
 *------------------------------------------------------------------------------
 * PURPOSE  : Get SNTP_VAR_STATE table from OM.
 * INPUT    : buffer to get SNTP_VAR_STATE table
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    :
 *------------------------------------------------------------------------------*/
void SNTP_OM_GetVarStateTable(SNTP_OM_VAR_STATE_T *dst);

/*------------------------------------------------------------------------------
 * FUNCTION NAME -  SNTP_OM_SearchIpEntry
 *------------------------------------------------------------------------------
 * PURPOSE  : Search Ip entry in OM
 * INPUT    : ip address
 * OUTPUT   : ip index in OM structure
 * RETURN   : TRUE : If success
 *            FALSE: If failed
 * NOTES    : MAX server defined in sntp_type.h should not exceed 2^32
 *------------------------------------------------------------------------------*/
BOOL_T SNTP_OM_SearchIpEntry(L_INET_AddrIp_T *ipaddress, UI32_T *ipindex);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_OM_SetCurrentServer
 *------------------------------------------------------------------------------
 * PURPOSE  : Set current server of getting current time
 * INPUT    : buffer to get server
 * OUTPUT   : ip address
 * RETURN   : none
 * NOTES    :
 *------------------------------------------------------------------------------*/
void SNTP_OM_SetCurrentServer(L_INET_AddrIp_T *ipaddress);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_OM_SetPollTime
 *------------------------------------------------------------------------------
 * PURPOSE  : Set the polling interval
 * INPUT    : polling interval used in unicast mode
 * OUTPUT   : none
 * RETURN   : TRUE : If success
 *            FALSE:
 * NOTES    :
 *------------------------------------------------------------------------------*/
BOOL_T SNTP_OM_SetPollTime(UI32_T polltime);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_OM_SetServiceOperationMode
 *------------------------------------------------------------------------------
 * PURPOSE  : Set sntp service operation mode , unicast/brocast/anycast/disable
 * INPUT    : VAL_sntpServiceMode_unicast   = 1
 *            VAL_sntpServiceMode_broadcast = 2
 *            VAL_sntpServiceMode_anycast   = 3
 *
 * OUTPUT   : none
 * RETURN   : TRUE : If success
 *            FALSE:
 * NOTES    : none
 *------------------------------------------------------------------------------*/
BOOL_T SNTP_OM_SetServiceOperationMode(UI32_T mode);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_OM_SetStatus
 *------------------------------------------------------------------------------
 * PURPOSE  : Set SNTP status : enable/disable
 * INPUT    : status you want to set. 1 :VAL_sntpStatus_enabled, 2 :VAL_sntpStatus_disabled
 * OUTPUT   : none
 * RETURN   : TRUE : If success
 *            FALSE:
 * NOTES    : none
 *------------------------------------------------------------------------------*/
BOOL_T SNTP_OM_SetStatus(UI32_T status);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_OM_GetVarStateTable
 *------------------------------------------------------------------------------
 * PURPOSE  : Set SNTP_VAR_STATE table from OM.
 * INPUT    : buffer to set SNTP_VAR_STATE table
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    :
 *------------------------------------------------------------------------------*/
void SNTP_OM_SetVarStateTable(SNTP_OM_VAR_STATE_T *src);

#endif /* SNTP_OM_H */

