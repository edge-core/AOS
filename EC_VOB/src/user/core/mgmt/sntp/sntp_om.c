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


 /* INCLUDE FILE DECLARATIONS
 */
#include <string.h>
#include "sys_type.h"
#include "sys_bld.h"
#include "sysfun.h"
#include "sntp_type.h"
#include "sntp_om.h"

/*
 *    NAMING CONSTANT DECLARATIONS
 */

/*
 *    DATA TYPE DECLARATIONS
 */
typedef struct
{
    UI32_T                  polling_inteval;
    UI32_T                  sync_method;                    /* reserved */
    UI32_T                  ip_count;
    SNTP_OPERATION_MODE_E   config_mode;
    SNTP_SERVICE_E          service_status;                 /* show if SNTP ON or OFF */
    SNTP_SERVER_ENTRY_T     server_entry[SNTP_MAX_SERVER];  /* SNTP server ip entry */
    SNTP_SERVER_STATUS_E    status[SNTP_MAX_SERVER];        /* internal view of server status */
} SNTP_OM_MIB_ENTRY_T;

/*
 * LOCAL SUBPROGRAM DECLARATIONS
 */

/*
 * STATIC VARIABLE DECLARATIONS
 */
static UI32_T               sntp_om_sem_id;
static UI32_T               orig_priority;

/* Data base for SNTP */
static SNTP_OM_MIB_ENTRY_T  SNTP_TABLE;
static SNTP_OM_VAR_STATE_T  SNTP_VAR_STATE;

/*
 *    MACRO FUNCTIONS DECLARACTION
 */
#define SNTP_OM_LOCK()    orig_priority=SYSFUN_OM_ENTER_CRITICAL_SECTION(sntp_om_sem_id);
#define SNTP_OM_UNLOCK()  SYSFUN_OM_LEAVE_CRITICAL_SECTION(sntp_om_sem_id,orig_priority);

/* EXPORTED SUBPROGRAM BODIES
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
BOOL_T SNTP_OM_CreatSem(void)
{
    /* create semaphore */
    if (SYSFUN_OK != SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_SNTP_OM, &sntp_om_sem_id))
    {
        return FALSE;
    }

    return TRUE;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNTP_OM_Initiate_System_Resources
 *---------------------------------------------------------------------------
 * PURPOSE:  This function is used to initialize the SNTP database.
 * INPUT:    None
 * OUTPUT:   None
 * RETURN:   None
 * NOTE:     None
 *---------------------------------------------------------------------------*/
void SNTP_OM_Initiate_System_Resources(void)
{
    SNTP_OM_ClearSntpDatabase();
}

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
BOOL_T SNTP_OM_AddServerIp(UI32_T index, L_INET_AddrIp_T *ipaddress)
{
    UI32_T  i;
    BOOL_T  ret = TRUE;

    SNTP_OM_LOCK();

    for (i = 0; i < (index - 1); i++)
    {
        /* prevent the same ip address problem */
        if (L_INET_CompareInetAddr((L_INET_Addr_T *)ipaddress,
            (L_INET_Addr_T *)&SNTP_TABLE.server_entry[i].ipaddress,
            L_INET_COMPARE_FLAG_INCLUDE_ADDRRESS_LENGTH) == 0)
        {
            ret = FALSE;
            break;
        }

        /* reslove hole problem */
        if ((index - 1) >= 1)
        {
            if (0 == SNTP_TABLE.server_entry[i].ipaddress.addrlen)
            {
                ret = FALSE;
                break;
            }
        }
    }

    if (TRUE == ret)
    {
        memcpy(&SNTP_TABLE.server_entry[index - 1].ipaddress, ipaddress, sizeof(L_INET_AddrIp_T));
        SNTP_TABLE.ip_count++;
    }

    SNTP_OM_UNLOCK();

    return ret;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNTP_OM_ClearSntpDatabase
 *---------------------------------------------------------------------------
 * PURPOSE:  This function is used to clear the SNTP database.
 * INPUT:    None
 * OUTPUT:   None
 * RETURN:   None
 * NOTE:     None
 *---------------------------------------------------------------------------*/
void SNTP_OM_ClearSntpDatabase(void)
{
    SNTP_OM_LOCK();
    memset(&SNTP_TABLE, 0, sizeof(SNTP_TABLE));
    memset(&SNTP_VAR_STATE, 0, sizeof(SNTP_VAR_STATE));
    SNTP_OM_UNLOCK();
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNTP_OM_InitSntpDatabaseToDefaultValue
 *---------------------------------------------------------------------------
 * PURPOSE:  This function is used to set the SNTP database to default value.
 * INPUT:    None
 * OUTPUT:   None
 * RETURN:   None
 * NOTE:     None
 *---------------------------------------------------------------------------*/
void SNTP_OM_InitSntpDatabaseToDefaultValue(void)
{
    SNTP_OM_LOCK();

#if (SYS_CPNT_A3COM515_SNTP_MIB == TRUE)
    SNTP_TABLE.polling_inteval = SNTP_DEFAULT_POLLINGTIME * 60;
#else
    SNTP_TABLE.polling_inteval = SNTP_DEFAULT_POLLINGTIME;
#endif
    SNTP_TABLE.config_mode = SNTP_DEFAULT_OPERATIONMODE;
    SNTP_TABLE.service_status = SNTP_DEFAULT_STATUS;

    SNTP_OM_UNLOCK();
}

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
BOOL_T SNTP_OM_DeleteServerIp(UI32_T index)
{
    int i;

    SNTP_OM_LOCK();

    if (SNTP_TABLE.server_entry[index - 1].ipaddress.addrlen != 0)
    {
        SNTP_TABLE.ip_count--;

        memset(&SNTP_TABLE.server_entry[index - 1].ipaddress, 0, sizeof(L_INET_AddrIp_T));

        /*
         * If deleted ip is not the last data in om, sort them and make
         * 0.0.0.0 to final
         */
        if (index != MAX_sntpServerIndex)
        {
            for (i = index - 1; i < MAX_sntpServerIndex; i++)
            {
                memcpy(&SNTP_TABLE.server_entry[i].ipaddress, &SNTP_TABLE.server_entry[i + 1].ipaddress, sizeof(L_INET_AddrIp_T));
            }

            memset(&SNTP_TABLE.server_entry[MAX_sntpServerIndex - 1].ipaddress, 0, sizeof(L_INET_AddrIp_T));
        }
    }

    SNTP_OM_UNLOCK();

    return TRUE;
}

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
BOOL_T SNTP_OM_GetCurrentServer(L_INET_AddrIp_T *ipaddress)
{
    BOOL_T ret = FALSE;

    SNTP_OM_LOCK();

    if(0 != SNTP_VAR_STATE.current_server.addrlen)
    {
        memcpy(ipaddress, &SNTP_VAR_STATE.current_server, sizeof(L_INET_AddrIp_T));
        ret = TRUE;
    }

    SNTP_OM_UNLOCK();

    return ret;
}

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
BOOL_T SNTP_OM_GetCurrentUTCTime(UI32_T *time)
{
    SNTP_OM_LOCK();
    *time = SNTP_VAR_STATE.current_time;
    SNTP_OM_UNLOCK();

    return TRUE;
}

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
UI32_T SNTP_OM_GetServerIpCount(void)
{
    UI32_T  ret;

    SNTP_OM_LOCK();
    ret = SNTP_TABLE.ip_count;
    SNTP_OM_UNLOCK();

    return ret;
}

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
BOOL_T SNTP_OM_GetLastUpdateUTCTime(UI32_T *time)
{
    SNTP_OM_LOCK();
    *time = SNTP_VAR_STATE.last_update_time;
    SNTP_OM_UNLOCK();

    return TRUE;
}

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
BOOL_T SNTP_OM_GetPollTime(UI32_T *polltime)
{
    SNTP_OM_LOCK();
    *polltime = SNTP_TABLE.polling_inteval;
    SNTP_OM_UNLOCK();

    return TRUE;
}

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
BOOL_T SNTP_OM_GetServerIp(UI32_T index, L_INET_AddrIp_T *ipaddress)
{
    SNTP_OM_LOCK();
    if(SNTP_TABLE.server_entry[index - 1].ipaddress.addrlen != 0)
    {
        memcpy(ipaddress, &SNTP_TABLE.server_entry[index - 1].ipaddress, sizeof(L_INET_AddrIp_T));
    SNTP_OM_UNLOCK();
    return TRUE;
}
    else
    { 
        SNTP_OM_UNLOCK();
        return FALSE;
    }

}

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
BOOL_T SNTP_OM_GetServiceOperationMode(UI32_T *mode)
{
    SNTP_OM_LOCK();
    *mode = SNTP_TABLE.config_mode;
    SNTP_OM_UNLOCK();

    return TRUE;
}

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
BOOL_T SNTP_OM_GetStatus(UI32_T *status)
{
    SNTP_OM_LOCK();
    *status = SNTP_TABLE.service_status;
    SNTP_OM_UNLOCK();

    return TRUE;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_OM_GetVarStateTable
 *------------------------------------------------------------------------------
 * PURPOSE  : Get SNTP_VAR_STATE table from OM.
 * INPUT    : buffer to get SNTP_VAR_STATE table
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    :
 *------------------------------------------------------------------------------*/
void SNTP_OM_GetVarStateTable(SNTP_OM_VAR_STATE_T *dst)
{
    SNTP_OM_LOCK();

    memcpy(dst, &SNTP_VAR_STATE, sizeof(SNTP_VAR_STATE));

    SNTP_OM_UNLOCK();
}

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
BOOL_T SNTP_OM_SearchIpEntry(L_INET_AddrIp_T *ipaddress, UI32_T *ipindex)
{
    UI32_T  index;
    BOOL_T  ret = FALSE;

    SNTP_OM_LOCK();

    for (index = 0; index < SNTP_MAX_SERVER; index++)
    {
        if ( (SNTP_TABLE.server_entry[index].ipaddress.type == ipaddress->type)&& 
             (memcmp(SNTP_TABLE.server_entry[index].ipaddress.addr, ipaddress->addr, ipaddress->addrlen)==0))
        {
            *ipindex = index;
            ret = TRUE;
            break;
        }
    }

    SNTP_OM_UNLOCK();

    return ret;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_OM_SetCurrentServer
 *------------------------------------------------------------------------------
 * PURPOSE  : Set current server of getting current time
 * INPUT    : buffer to get server
 * OUTPUT   : ip address
 * RETURN   : none
 * NOTES    :
 *------------------------------------------------------------------------------*/
void SNTP_OM_SetCurrentServer(L_INET_AddrIp_T *ipaddress)
{
    SNTP_OM_LOCK();
    memcpy(&SNTP_VAR_STATE.current_server, ipaddress, sizeof(L_INET_AddrIp_T));
    SNTP_OM_UNLOCK();
}

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
BOOL_T SNTP_OM_SetPollTime(UI32_T polltime)
{
    SNTP_OM_LOCK();
    SNTP_TABLE.polling_inteval = polltime;
    SNTP_OM_UNLOCK();

    return TRUE;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_OM_SetServerStatusByIndex
 *------------------------------------------------------------------------------
 * PURPOSE  : Set server status by index
 * INPUT    : index, status
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    :
 *------------------------------------------------------------------------------*/
void SNTP_OM_SetServerStatusByIndex(UI32_T index, SNTP_SERVER_STATUS_E status)
{
    SNTP_OM_LOCK();
    SNTP_TABLE.status[index] = status;
    SNTP_OM_UNLOCK();
}

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
BOOL_T SNTP_OM_SetServiceOperationMode(UI32_T mode)
{
    SNTP_OM_LOCK();
    SNTP_TABLE.config_mode = mode;
    SNTP_OM_UNLOCK();

    return TRUE;
}

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
BOOL_T SNTP_OM_SetStatus(UI32_T status)
{
    SNTP_OM_LOCK();
    SNTP_TABLE.service_status = status;
    SNTP_OM_UNLOCK();

    return TRUE;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_OM_GetVarStateTable
 *------------------------------------------------------------------------------
 * PURPOSE  : Set SNTP_VAR_STATE table from OM.
 * INPUT    : buffer to set SNTP_VAR_STATE table
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    :
 *------------------------------------------------------------------------------*/
void SNTP_OM_SetVarStateTable(SNTP_OM_VAR_STATE_T *src)
{
    SNTP_OM_LOCK();

    memcpy(&SNTP_VAR_STATE, src, sizeof(SNTP_VAR_STATE));

    SNTP_OM_UNLOCK();
}

