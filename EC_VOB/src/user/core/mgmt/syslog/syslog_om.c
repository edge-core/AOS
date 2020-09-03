/* Module Name: SYSLOG_OM.C
 * Purpose: Initialize the database resources and provide some Get/Set function
 *          for accessing the system log database.
 *
 * Notes:
 *
 * History:
 *    10/29/01       -- Aaron Chuang, Create
 *                   -- Separete parts from syslog_mgr.c, and generate the syslog_om.c
 *
 *    07/17/07       -- Rich Lee, Porting to Linux
 *
 * Copyright(C)      Accton Corporation, 1999 - 2001
 *
 */


/* INCLUDE FILE DECLARATIONS
 */
#include "string.h"
#include "sys_type.h"
#include "sys_dflt.h"
#include "sys_bld.h"
#include "sysfun.h"
#include "syslog_type.h"
#include "syslog_om.h"
#include "uc_mgr.h"
#include "sys_adpt.h"
#include "sys_cpnt.h"
#include "ip_lib.h"
#if (SYS_CPNT_EH == TRUE)   /* Aaron add for EH component, 2002/02/18 */
#include "sys_module.h"
#include "eh_type.h"
#include "eh_pmgr.h"
#endif


/* NAMING CONSTANT DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */
typedef struct
{
    I32_T                         que_elements_cnt;
    SYSLOG_EVENT_SyslogData_T     *front;
    SYSLOG_EVENT_SyslogData_T     *rear;
} SYSLOG_OM_Queue_T;


/* LOCAL SUBPROGRAM DECLARATIONS
 */


/* STATIC VARIABLE DECLARATIONS
 */
static SYSLOG_OM_Config_T       syslog_config;
static SYSLOG_UcNormalDb_T      *uc_normal_db; /*  uc_normal_db = 0x01fd-fff4 */
static SYSLOG_UcFlashDb_T       *uc_flash_db;  /*  uc_flash_db = 0x01fd-7fe8 */

#if (SYS_CPNT_REMOTELOG == TRUE)
static SYSLOG_OM_Remote_Config_T remotelog_config;
static UI8_T   message[SYS_ADPT_MAX_LENGTH_OF_REMOTELOG_MESSAGE];
static SYSLOG_OM_Queue_T        remotelog_queue;
#endif

static UI32_T syslog_om_semid;

static UI32_T syslog_debug_flag;

static void
SYSLOG_OM_SortRemoteLogServerTableByIPAddress(
);

static BOOL_T
SYSLOG_OM_IsValidIPAddress(
    const L_INET_AddrIp_T *ip_addr
);

/* MACRO FUNCTIONS DECLARACTION
 */
#define NEXT_UC_NORMAL_INDEX(v)     (v=(v+1) & (SYSLOG_ADPT_MAX_ENTRIES_OF_UC_NORMAL_DB-1))
#define NEXT_UC_FLASH_INDEX(v)      (v=(v+1) & (SYSLOG_ADPT_MAX_ENTRIES_OF_UC_FLASH_DB-1))

#ifndef _countof
#define _countof(_Ary)  (sizeof(_Ary) / sizeof(*_Ary))
#endif /* _countof */

#ifndef ASSERT
#define ASSERT(eq)
#endif


#define SYSLOG_OM_IS_DEBUG_ERROR_ON(flag)            (flag & SYSLOG_OM_DEBUG_OM_ERR)

#define SYSLOG_OM_DEBUG_LOG(fmt, ...)                           \
    {                                                           \
        UI32_T debug_flag = SYSLOG_OM_GetDebugFlag();           \
        if (SYSLOG_OM_IS_DEBUG_ERROR_ON(debug_flag))            \
        {                                                       \
            printf("%s:%d ", __FUNCTION__, __LINE__);           \
            printf(fmt,__VA_ARGS__);                            \
            printf("\r\n");                                     \
        }                                                       \
    }


/* EXPORTED SUBPROGRAM BODIES
 */

/* FUNCTION NAME:  SYSLOG_OM_SetDebugFlag
 * PURPOSE  : set backdoor debug flag
 * INPUT    : debug_flag
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 */
void
SYSLOG_OM_SetDebugFlag(
    UI32_T debug_flag)
{
    syslog_debug_flag = debug_flag;
}

/* FUNCTION NAME:  SYSLOG_OM_GetDebugFlag
 * PURPOSE  : get backdoor debug flag
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : debug flag
 * NOTES    : none
 */
UI32_T
SYSLOG_OM_GetDebugFlag()
{
    return syslog_debug_flag;
}

/* FUNCTION NAME: SYSLOG_OM_Init
 * PURPOSE: This function is used to initialize the system log database.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:   Initialize the system log database.
 *
 */
void
SYSLOG_OM_Init(void)
{
#if (SYS_CPNT_REMOTELOG == TRUE)
    UI32_T i;
    char   str_rem_ip_addr[L_INET_MAX_IPADDR_STR_LEN + 1]={0};
#endif

    /* Initialize the syslog config
     */
    syslog_config.syslog_status = SYS_DFLT_SYSLOG_STATUS;
    syslog_config.uc_log_level = SYS_DFLT_SYSLOG_UC_LOG_LEVEL;
    syslog_config.flash_log_level = SYS_DFLT_SYSLOG_FLASH_LOG_LEVEL;

#if (SYS_CPNT_REMOTELOG == TRUE)
    /* following for remotelog, Aaron Chuang 2002/07/22 */
    for(i = 0; i < _countof(remotelog_config.server_config); i ++)
    {

        memset(&remotelog_config, 0, sizeof(remotelog_config));

#if(SYS_CPNT_REMOTELOG_FACILITY_LEVEL_FOR_EVERY_SERVER == TRUE)
        remotelog_config.server_config[i].facility = SYS_DFLT_REMOTELOG_FACILITY_TYPE;
        remotelog_config.server_config[i].level = SYS_DFLT_REMOTELOG_LEVEL;
#endif
    }

#if(SYS_CPNT_REMOTELOG_FACILITY_LEVEL_FOR_EVERY_SERVER == FALSE)
    remotelog_config.facility = SYS_DFLT_REMOTELOG_FACILITY_TYPE;
    remotelog_config.level = SYS_DFLT_REMOTELOG_LEVEL;
#endif /* #if(SYS_CPNT_REMOTELOG_FACILITY_LEVEL_FOR_EVERY_SERVER == FALSE) */

    remotelog_config.status = SYS_DFLT_REMOTELOG_LEVEL;


    /* Initialize the remotelog queue
     */
    remotelog_queue.front = (SYSLOG_EVENT_SyslogData_T *)NULL;
    remotelog_queue.rear  = (SYSLOG_EVENT_SyslogData_T *)NULL;
    remotelog_queue.que_elements_cnt = 0;

    for (i = 0; i < _countof(remotelog_config.server_config); i ++)
    {
        L_INET_InaddrToString((L_INET_Addr_T *)&remotelog_config.server_config[i].ipaddr,
                              str_rem_ip_addr, sizeof(str_rem_ip_addr));
        SYSLOG_OM_DEBUG_LOG("\r\nRemotelog server ip %lu : %s", (unsigned long)i, str_rem_ip_addr);
    }
#if(SYS_CPNT_REMOTELOG_FACILITY_LEVEL_FOR_EVERY_SERVER == TRUE)
    SYSLOG_OM_DEBUG_LOG("\r\nRemotelog facility : %ld",(long)remotelog_config.server_config[0].facility);
    SYSLOG_OM_DEBUG_LOG("\r\nRemotelog facility : %ld",(long)remotelog_config.server_config[1].facility);
    SYSLOG_OM_DEBUG_LOG("\r\nRemotelog facility : %ld",(long)remotelog_config.server_config[2].facility);
    SYSLOG_OM_DEBUG_LOG("\r\nRemotelog facility : %ld",(long)remotelog_config.server_config[3].facility);
    SYSLOG_OM_DEBUG_LOG("\r\nRemotelog facility : %ld",(long)remotelog_config.server_config[4].facility);

    SYSLOG_OM_DEBUG_LOG("\r\nRemotelog level : %ld",(long)remotelog_config.server_config[0].level);
    SYSLOG_OM_DEBUG_LOG("\r\nRemotelog level : %ld",(long)remotelog_config.server_config[1].level);
    SYSLOG_OM_DEBUG_LOG("\r\nRemotelog level : %ld",(long)remotelog_config.server_config[2].level);
    SYSLOG_OM_DEBUG_LOG("\r\nRemotelog level : %ld",(long)remotelog_config.server_config[3].level);
    SYSLOG_OM_DEBUG_LOG("\r\nRemotelog level : %ld",(long)remotelog_config.server_config[4].level);
#else
    SYSLOG_OM_DEBUG_LOG("\r\nRemotelog facility : %ld",(long)remotelog_config.facility);
    SYSLOG_OM_DEBUG_LOG("\r\nRemotelog level : %ld",(long)remotelog_config.level);
#endif /* #if(SYS_CPNT_REMOTELOG_FACILITY_LEVEL_FOR_EVERY_SERVER == TRUE) */
    SYSLOG_OM_DEBUG_LOG("\r\nRemotelog status : %ld",(long)remotelog_config.status);
#endif

    return;
}

/* FUNCTION NAME: SYSLOG_OM_Initiate_System_Resources
 * PURPOSE: This function is used to initialize the system log database.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure, system error.
 * NOTES:   Initialize the system log database.
 *
 */
BOOL_T
SYSLOG_OM_Initiate_System_Resources(
    void)
{
    if (SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_SYSLOG_OM, &syslog_om_semid) != SYSFUN_OK)
    {
        printf("\n%s:get om sem id fail.\n", __FUNCTION__);
    }

    SYSLOG_OM_Init();

    return(TRUE);
} /* SYSLOG_OM_Initiate_System_Resources() */

/* FUNCTION NAME: SYSLOG_OM_Clear_UcDatabaseOldVersionRecord
 * PURPOSE: This function is used to check code version.
 *          If the code version is old version, clear UC detabase.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  TRUE    -- successful.
 * NOTES:   1. Only need to check version when switch warmstart.
 *          2. Switch will clear UC detabase when update code from
 *             old version to new version.
 *
 */
BOOL_T
SYSLOG_OM_Clear_UcDatabaseOldVersionRecord(
    void)
{
    int i, count;

    count = 0;

    uc_normal_db = (SYSLOG_UcNormalDb_T *) UC_MGR_Allocate(UC_MGR_SYSLOG_UC_NORMAL_INDEX, sizeof(SYSLOG_UcNormalDb_T), 4);
    uc_flash_db =  (SYSLOG_UcFlashDb_T *) UC_MGR_Allocate(UC_MGR_SYSLOG_UC_FLASH_INDEX, sizeof(SYSLOG_UcFlashDb_T), 4);

    for(i = 0; i < _countof(uc_normal_db->header.stat); i ++)
    {
        count = count + uc_normal_db->header.stat[i].count;
    }

    if(count != uc_normal_db->header.count)
    {
        memset(uc_normal_db, 0, sizeof(SYSLOG_UcNormalDb_T));
    }

    count = 0;

    for(i = 0; i < _countof(uc_flash_db->header.stat); i ++)
    {
        count = count + uc_flash_db->header.stat[i].count;
    }

    if(count != uc_flash_db->header.count)
    {
        memset(uc_flash_db, 0, sizeof(SYSLOG_UcFlashDb_T));
    }

    return TRUE;
} /* End of SYSLOG_OM_Clear_UcDatabaseOldVersionRecord */

/* FUNCTION NAME: SYSLOG_OM_Initiate_UcDatabase
 * PURPOSE: This function is used to initialize the UC database.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure, system error.
 * NOTES:   Initialize the UC normal database.
 *
 */
BOOL_T SYSLOG_OM_Initiate_UcDatabase(void)
{
    /* Initialize UC normal database
     */
    uc_normal_db = (SYSLOG_UcNormalDb_T *) UC_MGR_Allocate(UC_MGR_SYSLOG_UC_NORMAL_INDEX, sizeof(SYSLOG_UcNormalDb_T), 4);
    uc_flash_db =  (SYSLOG_UcFlashDb_T *) UC_MGR_Allocate(UC_MGR_SYSLOG_UC_FLASH_INDEX, sizeof(SYSLOG_UcFlashDb_T), 4);

    /* Initial log entry in UC-memory only when coldstart
     */
    memset(uc_normal_db, 0, sizeof(SYSLOG_UcNormalDb_T));
    memset(uc_flash_db, 0, sizeof(SYSLOG_UcFlashDb_T));

    return TRUE;
} /* SYSLOG_OM_Initiate_UcDatabase() */

/* FUNCTION NAME: SYSLOG_OM_GetSyslogStatus
 * PURPOSE: This function is used to get the system log status.
 * INPUT:   *syslog_status -- output buffer of system log status value.
 * OUTPUT:  *syslog_status -- system log status value.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure, system error.
 * NOTES:
 *
 */
BOOL_T SYSLOG_OM_GetSyslogStatus(UI32_T *syslog_status)
{
    UI32_T orig_priority;

    orig_priority=SYSLOG_OM_EnterCriticalSection(syslog_om_semid);
    *syslog_status = syslog_config.syslog_status;
    SYSLOG_OM_LeaveCriticalSection(syslog_om_semid,orig_priority);

    return TRUE;
} /* End of SYSLOG_OM_GetSyslogStatus */


/* FUNCTION NAME: SYSLOG_OM_SetSyslogStatus
 * PURPOSE: This function is used to set the system log status.
 * INPUT:   syslog_status -- setting value of system log status.
 * OUTPUT:  None.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure, system error.
 * NOTES:
 *
 */
BOOL_T SYSLOG_OM_SetSyslogStatus(UI32_T syslog_status)
{
    UI32_T orig_priority;

    orig_priority=SYSLOG_OM_EnterCriticalSection(syslog_om_semid);
    syslog_config.syslog_status = syslog_status;
    SYSLOG_OM_LeaveCriticalSection(syslog_om_semid,orig_priority);
    return TRUE;
} /* End of SYSLOG_OM_SetSyslogStatus */


/* FUNCTION NAME: SYSLOG_OM_GetUcLogLevel
 * PURPOSE: This function is used to get the un-cleared memory log level.
 * INPUT:   *uc_log_level -- output buffer of un-cleared memory log level value.
 * OUTPUT:  *uc_log_level -- un-cleared memory log level value.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure, system error.
 * NOTES:
 *
 */
BOOL_T SYSLOG_OM_GetUcLogLevel(UI32_T *uc_log_level)
{
    UI32_T orig_priority;

    orig_priority=SYSLOG_OM_EnterCriticalSection(syslog_om_semid);
    *uc_log_level = syslog_config.uc_log_level;
    SYSLOG_OM_LeaveCriticalSection(syslog_om_semid,orig_priority);
    return TRUE;
} /* End of SYSLOG_OM_GetUcLogLevel */


/* FUNCTION NAME: SYSLOG_OM_SetUcLogLevel
 * PURPOSE: This function is used to set the un-cleared memory log level.
 * INPUT:   uc_log_level -- setting value of un-cleared memory log level.
 * RETUEN:  SYSLOG_RETURN_OK                                --  OK, Successful, Without any Error
 *          SYSLOG_NORMAL_LEVEL_SMALLER_THAN_FLASH_LEVEL    --  Normal level is smaller than flash level
 *          SYSLOG_FLASH_LEVEL_BIGGER_THAN_NORMAL_LEVEL     --  Flash level is bigger than normal level
 *          SYSLOG_LEVEL_VLAUE_INVALID                      --  Syslog level is invalid
 *
 * NOTES:
 *
 */
UI32_T SYSLOG_OM_SetUcLogLevel(UI32_T uc_log_level)
{
    UI32_T orig_priority;

    orig_priority=SYSLOG_OM_EnterCriticalSection(syslog_om_semid);
    syslog_config.uc_log_level = uc_log_level;
    SYSLOG_OM_LeaveCriticalSection(syslog_om_semid,orig_priority);
    return SYSLOG_RETURN_OK;
} /* End of SYSLOG_OM_SetUcLogLevel */


/* FUNCTION NAME: SYSLOG_OM_GetFlashLogLevel
 * PURPOSE: This function is used to get the flash log level.
 * INPUT:   *flash_log_level -- output buffer of flash log level value.
 * OUTPUT:  *flash_log_level -- flash log level value.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure, system error.
 * NOTES:
 *
 */
BOOL_T SYSLOG_OM_GetFlashLogLevel(UI32_T *flash_log_level)
{
    UI32_T orig_priority;

    orig_priority=SYSLOG_OM_EnterCriticalSection(syslog_om_semid);
    *flash_log_level = syslog_config.flash_log_level;
    SYSLOG_OM_LeaveCriticalSection(syslog_om_semid,orig_priority);
    return TRUE;
} /* End of SYSLOG_OM_GetFlashLogLevel */


/* FUNCTION NAME: SYSLOG_OM_SetFlashLogLevel
 * PURPOSE: This function is used to set the flash log level.
 * INPUT:   flash_log_level -- setting value of flash log level.
 * OUTPUT:  None.
 * RETUEN:  SYSLOG_RETURN_OK                                --  OK, Successful, Without any Error
 *          SYSLOG_NORMAL_LEVEL_SMALLER_THAN_FLASH_LEVEL    --  Normal level is smaller than flash level
 *          SYSLOG_FLASH_LEVEL_BIGGER_THAN_NORMAL_LEVEL     --  Flash level is bigger than normal level
 *          SYSLOG_LEVEL_VLAUE_INVALID                      --  Syslog level is invalid
 *
 * NOTES:
 *
 */
UI32_T SYSLOG_OM_SetFlashLogLevel(UI32_T flash_log_level)
{
    UI32_T orig_priority;

    orig_priority=SYSLOG_OM_EnterCriticalSection(syslog_om_semid);
    syslog_config.flash_log_level = flash_log_level;
    SYSLOG_OM_LeaveCriticalSection(syslog_om_semid,orig_priority);
    return SYSLOG_RETURN_OK;
} /* End of SYSLOG_OM_SetFlashLogLevel */


/* FUNCTION NAME: SYSLOG_OM_GetSyslogConfig
 * PURPOSE: This function is used to get the system log config.
 * INPUT:   *config -- output buffer of system log config structure.
 * OUTPUT:  None.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure, system error.
 * NOTES:
 *
 */
BOOL_T SYSLOG_OM_GetSyslogConfig(SYSLOG_OM_Config_T *config)
{
    UI32_T orig_priority;

    orig_priority=SYSLOG_OM_EnterCriticalSection(syslog_om_semid);
    memcpy(config, &syslog_config, sizeof(SYSLOG_OM_Config_T));
    SYSLOG_OM_LeaveCriticalSection(syslog_om_semid,orig_priority);
    //*config = syslog_config;
    return TRUE;
} /* End of SYSLOG_OM_GetSyslogConfig */


/* FUNCTION NAME: SYSLOG_OM_AddUcNormalEntry
 * PURPOSE: Add a log message to un-cleared memory normal log DB.
 * INPUT:   *syslog_entry   -- add this syslog entry
 * OUTPUT:  None.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure, system error.
 * NOTES:
 *
 */
BOOL_T
SYSLOG_OM_AddUcNormalEntry(
    SYSLOG_OM_Record_T *syslog_entry)
{
    UI32_T orig_priority;

    orig_priority=SYSLOG_OM_EnterCriticalSection(syslog_om_semid);
    uc_normal_db = (SYSLOG_UcNormalDb_T *) UC_MGR_Allocate(UC_MGR_SYSLOG_UC_NORMAL_INDEX, sizeof(SYSLOG_UcNormalDb_T), 4);
    memcpy(&(uc_normal_db->entry[uc_normal_db->header.rear]), syslog_entry, sizeof(SYSLOG_OM_Record_T));
    //uc_normal_db->entry[uc_normal_db->header.rear] = *syslog_entry;
    NEXT_UC_NORMAL_INDEX(uc_normal_db->header.rear);

    if (uc_normal_db->header.count >= SYSLOG_ADPT_MAX_ENTRIES_OF_UC_NORMAL_DB)
    {
        /* queue full
         */
        NEXT_UC_NORMAL_INDEX(uc_normal_db->header.front);
    }
    else
    {
        ASSERT(syslog_entry->profile_id < _countof(uc_normal_db->header.stat));
        uc_normal_db->header.stat[syslog_entry->profile_id].count++;

        /* queue not full
         */
        uc_normal_db->header.count++;
    }

    SYSLOG_OM_LeaveCriticalSection(syslog_om_semid,orig_priority);

    return TRUE;
} /* End of SYSLOG_OM_AddUcNormalEntry */


/* FUNCTION NAME: SYSLOG_OM_AddUcFlashEntry
 * PURPOSE: Add a log message to un-cleared memory flash log DB.
 * INPUT:   *syslog_entry   -- add this syslog entry
 * OUTPUT:  None.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure, system error.
 * NOTES:
 *
 */
BOOL_T SYSLOG_OM_AddUcFlashEntry(SYSLOG_OM_Record_T *syslog_entry)
{
    UI32_T orig_priority;

    orig_priority=SYSLOG_OM_EnterCriticalSection(syslog_om_semid);
    uc_flash_db =  (SYSLOG_UcFlashDb_T *) UC_MGR_Allocate(UC_MGR_SYSLOG_UC_FLASH_INDEX, sizeof(SYSLOG_UcFlashDb_T), 4);
    memcpy(&(uc_flash_db->entry[uc_flash_db->header.rear]), syslog_entry, sizeof(SYSLOG_OM_Record_T));
    //uc_flash_db->entry[uc_flash_db->header.rear] = *syslog_entry;
    NEXT_UC_FLASH_INDEX(uc_flash_db->header.rear);
    if (uc_flash_db->header.count >= SYSLOG_ADPT_MAX_ENTRIES_OF_UC_FLASH_DB)
    {   /* queue full */
        NEXT_UC_FLASH_INDEX(uc_flash_db->header.front);
    }
    else
    {
        ASSERT(syslog_entry->profile_id < _countof(uc_flash_db->header.stat));
        uc_flash_db->header.stat[syslog_entry->profile_id].count++;

        /* queue not full */
        uc_flash_db->header.count++;
    }
    SYSLOG_OM_LeaveCriticalSection(syslog_om_semid,orig_priority);

    return TRUE;
} /* End of SYSLOG_OM_AddUcFlashEntry */

/* FUNCTION NAME: SYSLOG_OM_ClearUcRamEntries
 * PURPOSE: Clear log message from system log module in UC RAM memory.
 * INPUT:
 * OUTPUT:  None.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure, system error.
 * NOTES:
 *
 */
BOOL_T
SYSLOG_OM_ClearUcRamEntries(
    void)
{
    UI32_T orig_priority;

    orig_priority = SYSLOG_OM_EnterCriticalSection(syslog_om_semid);
    uc_normal_db = (SYSLOG_UcNormalDb_T *) UC_MGR_Allocate(UC_MGR_SYSLOG_UC_NORMAL_INDEX, sizeof(SYSLOG_UcNormalDb_T), 4);

    if (NULL == uc_normal_db)
    {
        SYSLOG_OM_LeaveCriticalSection(syslog_om_semid,orig_priority);
        return FALSE;
    }

    memset(uc_normal_db, 0, sizeof(SYSLOG_UcNormalDb_T));
    SYSLOG_OM_LeaveCriticalSection(syslog_om_semid,orig_priority);

    return TRUE;
} /* End of SYSLOG_OM_ClearUcRamEntries */

/* FUNCTION NAME: SYSLOG_OM_ClearUcFlashEntries
 * PURPOSE: Clear log message from system log module in UC Flash memory.
 * INPUT:
 * OUTPUT:  None.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure, system error.
 * NOTES:
 *
 */
BOOL_T
SYSLOG_OM_ClearUcFlashEntries(
    void)
{
    UI32_T orig_priority;

    orig_priority=SYSLOG_OM_EnterCriticalSection(syslog_om_semid);
    uc_flash_db =  (SYSLOG_UcFlashDb_T *) UC_MGR_Allocate(UC_MGR_SYSLOG_UC_FLASH_INDEX, sizeof(SYSLOG_UcFlashDb_T), 4);

    if (NULL == uc_flash_db)
    {
        SYSLOG_OM_LeaveCriticalSection(syslog_om_semid,orig_priority);
        return FALSE;
    }

    memset(uc_flash_db, 0, sizeof(SYSLOG_UcFlashDb_T));
    SYSLOG_OM_LeaveCriticalSection(syslog_om_semid,orig_priority);

    return TRUE;
} /* End of SYSLOG_OM_ClearUcFlashEntries */


/* FUNCTION NAME: SYSLOG_OM_GetUcNormalHeader
 * PURPOSE: Get header from un-cleared memory normal log DB.
 * INPUT:
 * OUTPUT:  None.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure, system error.
 * NOTES:
 *
 */
BOOL_T SYSLOG_OM_GetUcNormalHeader(SYSLOG_OM_Header_T *header)
{
    UI32_T orig_priority;

    orig_priority=SYSLOG_OM_EnterCriticalSection(syslog_om_semid);
    uc_normal_db = (SYSLOG_UcNormalDb_T *) UC_MGR_Allocate(UC_MGR_SYSLOG_UC_NORMAL_INDEX, sizeof(SYSLOG_UcNormalDb_T), 4);
    memcpy( header, &(uc_normal_db->header),sizeof(SYSLOG_OM_Header_T));
    //*header = uc_normal_db->header;
    SYSLOG_OM_LeaveCriticalSection(syslog_om_semid,orig_priority);

    return TRUE;
} /* End of SYSLOG_OM_GetUcNormalHeader */


/* FUNCTION NAME: SYSLOG_OM_GetUcFlashHeader
 * PURPOSE: Get header from un-cleared memory flash log DB.
 * INPUT:
 * OUTPUT:  None.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure, system error.
 * NOTES:
 *
 */
BOOL_T SYSLOG_OM_GetUcFlashHeader(SYSLOG_OM_Header_T *header)
{
    UI32_T orig_priority;

    orig_priority=SYSLOG_OM_EnterCriticalSection(syslog_om_semid);
    uc_flash_db =  (SYSLOG_UcFlashDb_T *) UC_MGR_Allocate(UC_MGR_SYSLOG_UC_FLASH_INDEX, sizeof(SYSLOG_UcFlashDb_T), 4);
    memcpy( header, &(uc_flash_db->header),sizeof(SYSLOG_OM_Header_T));
    //*header = uc_flash_db->header;

    SYSLOG_OM_LeaveCriticalSection(syslog_om_semid,orig_priority);
    return TRUE;
} /* End of SYSLOG_OM_GetUcFlashHeader */


/* FUNCTION NAME: SYSLOG_OM_SetUcFlashHeader
 * PURPOSE: Set header from un-cleared memory flash log DB.
 * INPUT:   header  -- un-cleared flash db header.
 * OUTPUT:  None.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure, system error.
 * NOTES:
 *
 */
BOOL_T SYSLOG_OM_SetUcFlashHeader(SYSLOG_OM_Header_T *header)
{
    UI32_T orig_priority;

    orig_priority=SYSLOG_OM_EnterCriticalSection(syslog_om_semid);
    uc_flash_db =  (SYSLOG_UcFlashDb_T *) UC_MGR_Allocate(UC_MGR_SYSLOG_UC_FLASH_INDEX, sizeof(SYSLOG_UcFlashDb_T), 4);
    memcpy( &(uc_flash_db->header), header, sizeof(SYSLOG_OM_Header_T));
    //uc_flash_db->header = header;
    SYSLOG_OM_LeaveCriticalSection(syslog_om_semid,orig_priority);
    return TRUE;
} /* End of SYSLOG_OM_SetUcFlashHeader */


/* FUNCTION NAME: SYSLOG_OM_GetUcNormalEntry
 * PURPOSE: Get a log message from un-cleared memory normal log DB.
 * INPUT:
 * OUTPUT:  None.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure, system error.
 * NOTES:
 *
 */
BOOL_T SYSLOG_OM_GetUcNormalEntry(UI32_T syslog_index, SYSLOG_OM_Record_T *syslog_entry)
{
    UI32_T orig_priority;

    orig_priority=SYSLOG_OM_EnterCriticalSection(syslog_om_semid);
    uc_normal_db = (SYSLOG_UcNormalDb_T *) UC_MGR_Allocate(UC_MGR_SYSLOG_UC_NORMAL_INDEX, sizeof(SYSLOG_UcNormalDb_T), 4);
    memcpy(syslog_entry, &uc_normal_db->entry[syslog_index], sizeof(SYSLOG_OM_Record_T));
    SYSLOG_OM_LeaveCriticalSection(syslog_om_semid,orig_priority);
    return TRUE;
} /* End of SYSLOG_OM_GetUcNormalEntry */


/* FUNCTION NAME: SYSLOG_OM_GetUcFlashEntry
 * PURPOSE: Get a log message from un-cleared memory flash log DB.
 * INPUT:
 * OUTPUT:  None.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure, system error.
 * NOTES:
 *
 */
BOOL_T SYSLOG_OM_GetUcFlashEntry(UI32_T syslog_index, SYSLOG_OM_Record_T *syslog_entry)
{
    UI32_T orig_priority;

    orig_priority=SYSLOG_OM_EnterCriticalSection(syslog_om_semid);
    uc_flash_db =  (SYSLOG_UcFlashDb_T *) UC_MGR_Allocate(UC_MGR_SYSLOG_UC_FLASH_INDEX, sizeof(SYSLOG_UcFlashDb_T), 4);
    memcpy(syslog_entry, &uc_flash_db->entry[syslog_index], sizeof(SYSLOG_OM_Record_T));
    SYSLOG_OM_LeaveCriticalSection(syslog_om_semid,orig_priority);
    return TRUE;
} /* End of SYSLOG_OM_GetUcFlashEntry */


/* FUNCTION NAME: SYSLOG_OM_CheckUcFlashFull
 * PURPOSE: Check un-cleared memory flash log DB is full or not.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  TRUE    -- Full.
 *          FALSE   -- Not full.
 * NOTES:
 *
 */
BOOL_T SYSLOG_OM_CheckUcFlashFull(void)
{
    UI32_T orig_priority;
    BOOL_T  ret;

    orig_priority=SYSLOG_OM_EnterCriticalSection(syslog_om_semid);
    uc_flash_db =  (SYSLOG_UcFlashDb_T *) UC_MGR_Allocate(UC_MGR_SYSLOG_UC_FLASH_INDEX, sizeof(SYSLOG_UcFlashDb_T), 4);
    if (uc_flash_db->header.count == SYSLOG_ADPT_MAX_ENTRIES_OF_UC_FLASH_DB)
        ret = TRUE;
    else
        ret = FALSE;
    SYSLOG_OM_LeaveCriticalSection(syslog_om_semid,orig_priority);
    return ret;
} /* End of SYSLOG_OM_CheckUcFlashFull */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SYSLOG_OM_HandleIPCReqMsg
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the ipc request message for SYSLOG OM.
 *
 * INPUT   : msgbuf_p -- input request ipc message buffer
 *
 * OUTPUT  : msgbuf_p -- output response ipc message buffer
 *
 * RETURN  : TRUE  - there is a response required to be sent
 *           FALSE - there is no response required to be sent
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T
SYSLOG_OM_HandleIPCReqMsg(
    SYSFUN_Msg_T* msgbuf_p)
{
    SYSLOG_OM_IPCMsg_T  *msg_p;
    BOOL_T  need_resp = TRUE;

    if (msgbuf_p == NULL)
    {
        return FALSE;
    }
    msg_p = (SYSLOG_OM_IPCMsg_T*)msgbuf_p->msg_buf;

    /* dispatch IPC message and call the corresponding VLAN_OM function
     */
    switch (msg_p->type.cmd)
    {
#if (SYS_CPNT_REMOTELOG_FACILITY_LEVEL_FOR_EVERY_SERVER == FALSE)
        case SYSLOG_OM_IPC_GET_REMOTE_LOG_LEVEL:
            msg_p->type.ret_ui32 =
                SYSLOG_OM_GetRemoteLogLevel(&msg_p->data.syslog_remote_config.level);
            msgbuf_p->msg_size = SYSLOG_OM_GET_MSGBUFSIZE(syslog_remote_config);
            break;
#endif /* #if (SYS_CPNT_REMOTELOG_FACILITY_LEVEL_FOR_EVERY_SERVER == TRUE) */

        case SYSLOG_OM_IPC_GET_REMOTE_LOG_STATUS:
            msg_p->type.ret_ui32 =
                SYSLOG_OM_GetRemoteLogStatus(&msg_p->data.syslog_remote_config.status);
            msgbuf_p->msg_size = SYSLOG_OM_GET_MSGBUFSIZE(syslog_remote_config);
            break;

        case SYSLOG_OM_IPC_GET_SERVER_IPADDR:
            msg_p->type.ret_ui32 =
                 SYSLOG_OM_GetRemoteLogServerIPAddr((UI8_T)msg_p->data.syslog_remote_config.server_index,
                                                 &msg_p->data.syslog_remote_config.server_config.ipaddr);
            msgbuf_p->msg_size = SYSLOG_OM_GET_MSGBUFSIZE(syslog_remote_config);
            break;

        case SYSLOG_OM_IPC_GET_SYSLOG_STATUS:
            msg_p->type.ret_b = SYSLOG_OM_GetSyslogStatus(&msg_p->data.syslog_om_data.syslog_status);
            msgbuf_p->msg_size = SYSLOG_OM_GET_MSGBUFSIZE(syslog_om_data);
            break;

        case SYSLOG_OM_IPC_GET_UC_LOG_LEVEL:
            msg_p->type.ret_b = SYSLOG_OM_GetUcLogLevel(&msg_p->data.syslog_om_data.uc_log_level);
            msgbuf_p->msg_size = SYSLOG_OM_GET_MSGBUFSIZE(syslog_om_data);
            break;

        default:
            printf("%s(): Invalid cmd.\n", __FUNCTION__);
            /* Unknow command. There is no way to idntify whether this
             * ipc message need or not need a response. If we response to
             * a asynchronous msg, then all following synchronous msg will
             * get wrong responses and that might not easy to debug.
             * If we do not response to a synchronous msg, the requester
             * will be blocked forever. It should be easy to debug that
             * error.
             */
            need_resp=FALSE;
    }

    return need_resp;
} /* End of SYSLOG_OM_HandleIPCReqMsg */


#if (SYS_CPNT_REMOTELOG == TRUE)
/* FUNCTION NAME: SYSLOG_OM_GetRemoteLogServerIPAddr
 * PURPOSE: This function is used to get the remote server IP address.
 * INPUT:   index -- index of server ip address
 *          *ipaddr -- buffer of server ip address.
 * OUTPUT:  *ipaddr -- value of server ip address.
 * RETUEN:  SYSLOG_REMOTE_SUCCESS    --  OK, Successful, Without any Error
 *          SYSLOG_REMOTE_FAIL   --  fail
 * NOTES:
 */
UI32_T
SYSLOG_OM_GetRemoteLogServerIPAddr(
    UI8_T index,
    L_INET_AddrIp_T *ipaddr)
{
    UI32_T orig_priority;
    char   str_rem_ip_addr[L_INET_MAX_IPADDR_STR_LEN+1]={0};

    orig_priority = SYSLOG_OM_EnterCriticalSection(syslog_om_semid);

    if (index >= SYS_ADPT_MAX_NUM_OF_REMOTELOG_SERVER)
    {
        SYSLOG_OM_LeaveCriticalSection(syslog_om_semid, orig_priority);

        return SYSLOG_REMOTE_INVALID_BUFFER;
    }

    if (ipaddr == NULL)
    {
#if (SYS_CPNT_EH == TRUE)   /* Aaron add for EH component, 2002/02/18 */
        EH_MGR_Handle_Exception(SYS_MODULE_SYSLOG,
                                SYSLOG_OM_GET_SERVER_IP_ADDR_FUNC_NO,
                                EH_TYPE_MSG_NULL_POINTER,
                                (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_CRIT));
#endif /* #if (SYS_CPNT_EH == TRUE) */
        SYSLOG_OM_LeaveCriticalSection(syslog_om_semid, orig_priority);

        return SYSLOG_REMOTE_INVALID_BUFFER;
    }

    if(remotelog_config.server_config[index].ipaddr.addrlen == 0)
    {
        SYSLOG_OM_LeaveCriticalSection(syslog_om_semid, orig_priority);

        return SYSLOG_REMOTE_FAIL;
    }

    memcpy(ipaddr, &remotelog_config.server_config[index].ipaddr, sizeof(*ipaddr));

    L_INET_InaddrToString((L_INET_Addr_T *) ipaddr, str_rem_ip_addr, sizeof(str_rem_ip_addr));
    SYSLOG_OM_DEBUG_LOG("\r\nRemotelog get server ip %d : %s", index,str_rem_ip_addr);

    SYSLOG_OM_LeaveCriticalSection(syslog_om_semid, orig_priority);

    return SYSLOG_REMOTE_SUCCESS;
} /* End of SYSLOG_OM_GetRemoteLogServerIPAddr */

/* FUNCTION NAME: SYSLOG_OM_SetRemoteLogStatus
 * PURPOSE: This function is used to enable/disable remotelog status.
 * INPUT:   remotelog_status -- status of remotelog.
 * RETUEN:  SYSLOG_REMOTE_SUCCESS    --  OK, Successful, Without any Error
 *          SYSLOG_REMOTE_INVALID   --  input value is invalid
 * NOTES:
 */
UI32_T
SYSLOG_OM_SetRemoteLogStatus(
    UI32_T remotelog_status)
{
    UI32_T orig_priority;

    orig_priority = SYSLOG_OM_EnterCriticalSection(syslog_om_semid);

    if (((int)remotelog_status != SYSLOG_STATUS_ENABLE) &&
        ((int)remotelog_status != SYSLOG_STATUS_DISABLE))
    {
#if (SYS_CPNT_EH == TRUE)   /* Aaron add for EH component, 2002/02/18 */
        UI8_T   buff1[32] = {0};

        sprintf(buff1, "Remote Log status");
        EH_MGR_Handle_Exception1(SYS_MODULE_SYSLOG,
                                 SYSLOG_OM_SET_REMOTELOG_STATUS_FUNC_NO,
                                 EH_TYPE_MSG_IS_INVALID,
                                 (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR),
                                 buff1);
#endif
        SYSLOG_OM_LeaveCriticalSection(syslog_om_semid, orig_priority);

        return SYSLOG_REMOTE_INVALID;
    }

    remotelog_config.status = remotelog_status;
    SYSLOG_OM_LeaveCriticalSection(syslog_om_semid, orig_priority);

    return SYSLOG_REMOTE_SUCCESS;
} /* End of SYSLOG_OM_SetRemoteLogStatus */

/* FUNCTION NAME: SYSLOG_OM_GetRemoteLogStatus
 * PURPOSE: This function is used to get remotelog status.
 * INPUT:   *remotelog_status -- output buffer of remotelog status
 * OUTPUT:  *remotelog_status -- value of remotelog status.
 * RETUEN:  SYSLOG_REMOTE_SUCCESS    --  OK, Successful, Without any Error
 *          SYSLOG_REMOTE_INVALID_BUFFER   --  output buffer is invalid
 * NOTES:
 */
UI32_T
SYSLOG_OM_GetRemoteLogStatus(
    UI32_T *remotelog_status)
{
    UI32_T orig_priority;

    orig_priority = SYSLOG_OM_EnterCriticalSection(syslog_om_semid);

    if (remotelog_status == NULL)
    {
#if (SYS_CPNT_EH == TRUE)   /* Aaron add for EH component, 2002/02/18 */
        EH_MGR_Handle_Exception(SYS_MODULE_SYSLOG,
                                SYSLOG_OM_GET_REMOTELOG_STATUS_FUNC_NO,
                                EH_TYPE_MSG_NULL_POINTER,
                                (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_CRIT));
#endif
        SYSLOG_OM_LeaveCriticalSection(syslog_om_semid, orig_priority);

        return SYSLOG_REMOTE_INVALID_BUFFER;
    }

    *remotelog_status = remotelog_config.status;

    SYSLOG_OM_DEBUG_LOG("\r\nRemotelog get status : %ld", (long)*remotelog_status);

    SYSLOG_OM_LeaveCriticalSection(syslog_om_semid, orig_priority);

    return SYSLOG_REMOTE_SUCCESS;
} /* End of SYSLOG_OM_GetRemoteLogStatus */

/* FUNCTION NAME: SYSLOG_OM_GetRemoteLogConfiguration
 * PURPOSE: This function is used to get the remote log config.
 * INPUT:   *config -- output buffer of remote log config structure.
 * OUTPUT:  None.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure, system error.
 * NOTES:
 */
BOOL_T
SYSLOG_OM_GetRemoteLogConfiguration(
    SYSLOG_OM_Remote_Config_T *config)
{
    UI32_T orig_priority;

    orig_priority = SYSLOG_OM_EnterCriticalSection(syslog_om_semid);

    if(config == NULL)
    {
        SYSLOG_OM_LeaveCriticalSection(syslog_om_semid, orig_priority);

        return FALSE;
    }

    memcpy( config, &remotelog_config, sizeof(*config));
    SYSLOG_OM_LeaveCriticalSection(syslog_om_semid, orig_priority);

    return TRUE;
}/* End of SYSLOG_OM_GetRemoteLogConfiguration */

/* FUNCTION NAME: SYSLOG_OM_GetRemoteLogMessage
 * PURPOSE: This function is used to set the remote log message.
 * INPUT:   *config -- output buffer of remote log message.
 * OUTPUT:  None.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure, system error.
 * NOTES:  For backdoor use only
 */
BOOL_T
SYSLOG_OM_GetRemoteLogMessage(
    UI8_T *msg)
{
    UI32_T orig_priority;

    if(msg == NULL)
    {
        SYSLOG_OM_LeaveCriticalSection(syslog_om_semid, orig_priority);

        return FALSE;
    }

    orig_priority = SYSLOG_OM_EnterCriticalSection(syslog_om_semid);
    memcpy(msg, message, sizeof(UI8_T)*strlen((char *)message));
    SYSLOG_OM_LeaveCriticalSection(syslog_om_semid, orig_priority);

    return TRUE;
}/* End of SYSLOG_OM_GetRemoteLogMessage */

/* FUNCTION NAME: SYSLOG_OM_SetRemoteLogMessage
 * PURPOSE: This function is used to set the remote log message.
 * INPUT:   msg -- remote log message.
 * OUTPUT:  None.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure, system error.
 * NOTES:   Called by mgr only for save remotelog message string
 */
BOOL_T
SYSLOG_OM_SetRemoteLogMessage(
    UI8_T *msg)
{
    UI32_T orig_priority;

    if((strlen((char *)msg) > SYS_ADPT_MAX_LENGTH_OF_REMOTELOG_MESSAGE)||
       (msg == NULL))
    {
        SYSLOG_OM_LeaveCriticalSection(syslog_om_semid, orig_priority);

        return FALSE;
    }

    orig_priority = SYSLOG_OM_EnterCriticalSection(syslog_om_semid);
    memcpy(&message, msg, sizeof(UI8_T)*strlen((char *)msg));

    SYSLOG_OM_DEBUG_LOG("\r\nRemotelog OM set message: %s",message);
    SYSLOG_OM_DEBUG_LOG("\r\nRemotelog OM set message length: %u",(unsigned)sizeof(message));

    SYSLOG_OM_LeaveCriticalSection(syslog_om_semid, orig_priority);

    return TRUE;
}/* End of SYSLOG_OM_SetRemoteLogMessage */

/* FUNCTION NAME: SYSLOG_OM_GetRemoteLogServerPortByIndex
 * PURPOSE: This function is used to get udp port.
 * INPUT:   *udp_port -- udp port.
 *          index -- index of server ip address
 * OUTPUT:  *udp_port -- udp port.
 * RETUEN:  REMOTELOG_SUCCESS    --  OK, Successful, Without any Error
 *          REMOTELOG_FAIL   --  fail
 * NOTES:
 */
UI32_T
SYSLOG_OM_GetRemoteLogServerPortByIndex(
    UI32_T index,
    UI32_T *udp_port)
{
    UI32_T orig_priority;

    orig_priority = SYSLOG_OM_EnterCriticalSection(syslog_om_semid);

    if ((index < 0) ||
        (index >= SYS_ADPT_MAX_NUM_OF_REMOTELOG_SERVER) ||
        (udp_port == NULL))
    {
        SYSLOG_OM_LeaveCriticalSection(syslog_om_semid, orig_priority);

        return SYSLOG_REMOTE_FAIL;
    }

    *udp_port = remotelog_config.server_config[index].udp_port;

    if (*udp_port == 0)
    {
#if (SYS_CPNT_EH == TRUE)   /* Aaron add for EH component, 2002/02/18 */
        EH_MGR_Handle_Exception(SYS_MODULE_SYSLOG,
                                SYSLOG_OM_GET_SERVER_IP_ADDR_FUNC_NO,
                                EH_TYPE_MSG_NULL_POINTER,
                                (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_CRIT));
#endif
        SYSLOG_OM_LeaveCriticalSection(syslog_om_semid, orig_priority);

        return SYSLOG_REMOTE_FAIL;
    }

    SYSLOG_OM_DEBUG_LOG("\r\nRemotelog get server index %lu udp port: %lx", (long)index, (unsigned long)*udp_port);

    SYSLOG_OM_LeaveCriticalSection(syslog_om_semid, orig_priority);

    return SYSLOG_REMOTE_SUCCESS;
} /* End of SYSLOG_OM_GetRemoteLogServerPortByIndex */

/* FUNCTION NAME: SYSLOG_OM_QueueEnqueue
 * PURPOSE: This function is used to enqueue remotelog entry.
 * INPUT:   new_blk -- remotelog event data pointer.
 * OUTPUT:  None.
 * RETUEN:  Error code.
 * NOTES:
 */
UI32_T
SYSLOG_OM_QueueEnqueue(
    SYSLOG_EVENT_SyslogData_T *new_blk)
{
    UI32_T orig_priority;

    orig_priority = SYSLOG_OM_EnterCriticalSection(syslog_om_semid);

    if (SYSLOG_TYPE_MAX_QUE_NBR <= remotelog_queue.que_elements_cnt)
    {
        SYSLOG_OM_LeaveCriticalSection(syslog_om_semid, orig_priority);
        return SYSLOG_REMOTE_FAIL;
    }

    new_blk->next = NULL;
    if (remotelog_queue.rear == (SYSLOG_EVENT_SyslogData_T *)NULL )   /* empty queue */
    {
        remotelog_queue.rear = new_blk;
        remotelog_queue.front = new_blk;
    }
    else
    {
        remotelog_queue.rear->next = new_blk;
        remotelog_queue.rear = new_blk;
    }

    remotelog_queue.que_elements_cnt++;

    SYSLOG_OM_LeaveCriticalSection(syslog_om_semid, orig_priority);
    return SYSLOG_REMOTE_SUCCESS;

}

/* FUNCTION NAME: SYSLOG_OM_QueueDequeue
 * PURPOSE: This function is used to dequeue remotelog entry.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  remotelog event data
 * NOTES:   None.
 */
SYSLOG_EVENT_SyslogData_T *
SYSLOG_OM_QueueDequeue()
{
    UI32_T orig_priority;
    SYSLOG_EVENT_SyslogData_T    *remotelog_data;

    orig_priority=SYSLOG_OM_EnterCriticalSection(syslog_om_semid);

    if (remotelog_queue.front == NULL)
    {
        SYSLOG_OM_LeaveCriticalSection(syslog_om_semid, orig_priority);
        return NULL;
    }

    /* More items in the queue.
     */
    remotelog_data = remotelog_queue.front;            /* Return the first element */
    remotelog_queue.front = remotelog_data->next;      /* Move queue head to next element  */

    if (remotelog_queue.front == NULL)
    {
        remotelog_queue.rear = NULL;                   /*  queue is empty */
    }

    remotelog_queue.que_elements_cnt --;

    SYSLOG_OM_LeaveCriticalSection(syslog_om_semid, orig_priority);
    return remotelog_data;
}

/* FUNCTION NAME: SYSLOG_MGR_IsQueueEmpty
 * PURPOSE: This function is used to check if queue is empty.
 * INPUT:   None
 * OUTPUT:  None.
 * RETUEN:  TRUE -- Queue is empty
 *          FALSE -- Queue is not empty
 * NOTES:   None.
 */
BOOL_T
SYSLOG_OM_IsQueueEmpty()
{
    UI32_T orig_priority;
    BOOL_T ret;

    orig_priority = SYSLOG_OM_EnterCriticalSection(syslog_om_semid);

    ret = (remotelog_queue.que_elements_cnt <= 0) ? TRUE : FALSE;

    SYSLOG_OM_LeaveCriticalSection(syslog_om_semid, orig_priority);
    return ret;
}

/* FUNCTION NAME: SYSLOG_OM_GetRemoteLogServerByIndex
 * PURPOSE: This function is used to get the server config data.
 * INPUT:   *server_config -- buffer of server config.
 *          index -- index of remote server
 * OUTPUT:  *server_config -- server config data.
 * RETUEN:  SYSLOG_REMOTE_SUCCESS    --  OK, Successful, Without any Error
 *          SYSLOG_REMOTE_FAIL   --  fail
 * NOTES:
 */
UI32_T
SYSLOG_OM_GetRemoteLogServerByIndex(
    SYSLOG_OM_Remote_Server_Config_T *server_config,
    UI32_T index)
{
    UI32_T orig_priority;

    orig_priority = SYSLOG_OM_EnterCriticalSection(syslog_om_semid);

    if ((server_config == NULL)||
        (index < 0) ||
        (index >= _countof(remotelog_config.server_config)))
    {
        SYSLOG_OM_LeaveCriticalSection(syslog_om_semid, orig_priority);

        return SYSLOG_REMOTE_INVALID_BUFFER;
    }

    memcpy(server_config,
           &(remotelog_config.server_config[index]),
           sizeof(SYSLOG_OM_Remote_Server_Config_T));

    SYSLOG_OM_LeaveCriticalSection(syslog_om_semid, orig_priority);

    return SYSLOG_REMOTE_SUCCESS;
} /* End of SYSLOG_OM_GetRemoteLogServerByIndex */

/* FUNCTION NAME: SYSLOG_OM_GetNextRemoteLogServer
 * PURPOSE: This function is used to get the server_config data
 * INPUT:   *server_config -- buffer of server config
 * OUTPUT:  *server_config -- server config data.
 * RETUEN:  SYSLOG_REMOTE_SUCCESS    --  OK, Successful, Without any Error
 *          SYSLOG_REMOTE_FAIL   --  fail
 * NOTES:
 */
UI32_T
SYSLOG_OM_GetNextRemoteLogServer(
    SYSLOG_OM_Remote_Server_Config_T *server_config)
{
    UI32_T  orig_priority;
    UI32_T  i;

    orig_priority = SYSLOG_OM_EnterCriticalSection(syslog_om_semid);

    if ((remotelog_config.server_config[0].ipaddr.addrlen == 0) ||
        (server_config == NULL))
    {
        SYSLOG_OM_LeaveCriticalSection(syslog_om_semid, orig_priority);

        return SYSLOG_REMOTE_FAIL;
    }

    for (i = 0; i < _countof(remotelog_config.server_config); i++)
    {
        if (0 < L_INET_CompareInetAddr(
            (L_INET_Addr_T *)&remotelog_config.server_config[i].ipaddr,
            (L_INET_Addr_T *)&server_config->ipaddr,
            L_INET_COMPARE_FLAG_INCLUDE_ADDRRESS_LENGTH))
        {
            if (remotelog_config.server_config[i].ipaddr.addrlen != 0)
            {
                memcpy(server_config, &(remotelog_config.server_config[i]),
                       sizeof(SYSLOG_OM_Remote_Server_Config_T));

                SYSLOG_OM_LeaveCriticalSection(syslog_om_semid, orig_priority);
                return SYSLOG_REMOTE_SUCCESS;
            }
        }
    }

    SYSLOG_OM_LeaveCriticalSection(syslog_om_semid, orig_priority);

    return SYSLOG_REMOTE_FAIL;
}  /* End of SYSLOG_OM_GetNextRemoteLogServer */

/* FUNCTION NAME: SYSLOG_OM_GetRemoteLogServer
 * PURPOSE: This function is used to get the server_config data by IP address.
 * INPUT:   *server_config -- buffer of server config
 * OUTPUT:  *server_config -- server config data.
 * RETUEN:  SYSLOG_REMOTE_SUCCESS    --  OK, Successful, Without any Error
 *          SYSLOG_REMOTE_FAIL   --  fail
 * NOTES:
 */
UI32_T
SYSLOG_OM_GetRemoteLogServer(
    SYSLOG_OM_Remote_Server_Config_T *server_config,
    UI32_T *index)
{
    UI32_T  i;

    if(server_config == NULL)
    {
        return SYSLOG_REMOTE_FAIL;
    }

    for (i = 0; i < _countof(remotelog_config.server_config); i ++)
    {
        if(L_INET_CompareInetAddr((L_INET_Addr_T *)&server_config->ipaddr,
            (L_INET_Addr_T *)&remotelog_config.server_config[i].ipaddr,
            L_INET_COMPARE_FLAG_INCLUDE_ADDRRESS_LENGTH) == 0)
        {
            memcpy(server_config,
                   &remotelog_config.server_config[i],
                   sizeof(SYSLOG_OM_Remote_Server_Config_T));
            *index = i;

            return SYSLOG_REMOTE_SUCCESS;
        }
    }

    return SYSLOG_REMOTE_FAIL;

} /* End of SYSLOG_OM_GetRemoteLogServer */

/* FUNCTION NAME: SYSLOG_OM_CreateRemoteLogServer
 * PURPOSE: This function is used to set the remote server IP address.
 * INPUT:   ipaddr -- value of server ip.
 * OUTPUT:  NONE.
 * RETUEN:  SYSLOG_REMOTE_SUCCESS    --  OK, Successful, Without any Error
 *          SYSLOG_REMOTE_FAIL   --  set fail
 *          SYSLOG_REMOTE_INPUT_EXIST    --  input address have existed
 * NOTES:
 */
UI32_T
SYSLOG_OM_CreateRemoteLogServer(
    L_INET_AddrIp_T *ipaddr)
{
    UI32_T orig_priority;
    UI32_T index;
    SYSLOG_OM_Remote_Server_Config_T server_config;

    orig_priority = SYSLOG_OM_EnterCriticalSection(syslog_om_semid);

    if(ipaddr == NULL)
    {
        SYSLOG_OM_LeaveCriticalSection(syslog_om_semid, orig_priority);

        return SYSLOG_REMOTE_FAIL;
    }

    memset(&server_config,0,sizeof(server_config));
    memcpy(&server_config.ipaddr, ipaddr, sizeof(server_config.ipaddr));

    /* IP address already exists
     */
    if (SYSLOG_OM_GetRemoteLogServer(&server_config, &index) == SYSLOG_REMOTE_SUCCESS)
    {
        SYSLOG_OM_LeaveCriticalSection(syslog_om_semid, orig_priority);

        return SYSLOG_REMOTE_SUCCESS;
    }
    /* New IP address
     */
    for (index = 0; index < _countof(remotelog_config.server_config); index ++)
    {
        if(remotelog_config.server_config[index].ipaddr.addrlen == 0)
        {
            memcpy(&remotelog_config.server_config[index].ipaddr,
                   ipaddr,
                   sizeof(remotelog_config.server_config[index].ipaddr));

            /* Set udp port to default port
             */
            remotelog_config.server_config[index].udp_port = SYS_DFLT_SYSLOG_HOST_PORT;
            remotelog_config.server_config[index].index = index;

#if (SYS_CPNT_REMOTELOG_FACILITY_LEVEL_FOR_EVERY_SERVER == TRUE)
            /* Set facility type and trap level to default value
             */
            remotelog_config.server_config[index].facility = SYS_DFLT_REMOTELOG_FACILITY_TYPE;
            remotelog_config.server_config[index].level = SYS_DFLT_REMOTELOG_LEVEL;
#endif/*SYS_CPNT_REMOTELOG_FACILITY_LEVEL_FOR_EVERY_SERVER*/

            SYSLOG_OM_SortRemoteLogServerTableByIPAddress();
            SYSLOG_OM_LeaveCriticalSection(syslog_om_semid, orig_priority);

            return SYSLOG_REMOTE_SUCCESS;
        }
    }

    SYSLOG_OM_LeaveCriticalSection(syslog_om_semid, orig_priority);

    return SYSLOG_REMOTE_FAIL;
}/* End of SYSLOG_OM_CreateRemoteLogServer */

/* FUNCTION NAME: SYSLOG_OM_DeleteRemoteLogServer
 * PURPOSE: This function is used to delete server ip address.
 * INPUT:   ipaddr -- deleted server ip.
 * OUTPUT:  NONE
 * RETUEN:  SYSLOG_REMOTE_SUCCESS    --  OK, Successful, Without any Error
 *          SYSLOG_REMOTE_FAIL   --  set fail
 *          REMOTELOG_EXIST    --  input address have existed
 * NOTES:
 */
UI32_T
SYSLOG_OM_DeleteRemoteLogServer(
    L_INET_AddrIp_T *ipaddr)
{
#if (SYS_CPNT_EH == TRUE)   /* Aaron add for EH component, 2002/02/18 */
    UI8_T   buff1[32] = {0};
#endif
    UI32_T orig_priority;
    UI32_T index;
    SYSLOG_OM_Remote_Server_Config_T server_config;

    orig_priority = SYSLOG_OM_EnterCriticalSection(syslog_om_semid);

    if(ipaddr == NULL)
    {
        SYSLOG_OM_LeaveCriticalSection(syslog_om_semid, orig_priority);

        return SYSLOG_REMOTE_FAIL;
    }

    memset(&server_config,0,sizeof(server_config));
    memcpy(&server_config.ipaddr, ipaddr, sizeof(server_config.ipaddr));

    if (SYSLOG_OM_GetRemoteLogServer(&server_config, &index) == SYSLOG_REMOTE_SUCCESS)
    {
        memset(&remotelog_config.server_config[index].ipaddr, 0, sizeof(L_INET_AddrIp_T));
        SYSLOG_OM_LeaveCriticalSection(syslog_om_semid, orig_priority);
        SYSLOG_OM_SortRemoteLogServerTableByIPAddress();

        return SYSLOG_REMOTE_SUCCESS;
    }

#if (SYS_CPNT_EH == TRUE)   /* Aaron add for EH component, 2002/02/18 */
    sprintf(buff1, "Remote log server IP address");
    EH_PMGR_Handle_Exception1(SYS_MODULE_SYSLOG,
                              SYSLOG_OM_DELETE_SERVER_IP_ADDR_FUNC_NO,
                              EH_TYPE_MSG_NOT_EXIST,
                              (SYSLOG_LEVEL_INFO),
                              buff1);
#endif
    SYSLOG_OM_LeaveCriticalSection(syslog_om_semid, orig_priority);

    return SYSLOG_REMOTE_FAIL;
} /* End of SYSLOG_OM_DeleteRemoteLogServer */

/* FUNCTION NAME: SYSLOG_OM_SetRemoteLogServerPort
 * PURPOSE: This function is used to set the remote server port.
 * INPUT:   ipaddr -- value of server ip.
 *          port   -- server port.
 * OUTPUT:  NONE.
 * RETUEN:  SYSLOG_REMOTE_SUCCESS    --  OK, Successful, Without any Error
 *          SYSLOG_REMOTE_FAIL   --  set fail
 *          SYSLOG_REMOTE_INPUT_EXIST    --  input address have existed
 *
 * NOTES:
 *
 */
UI32_T
SYSLOG_OM_SetRemoteLogServerPort(
    L_INET_AddrIp_T *ipaddr,
    UI32_T port)
{
    UI32_T orig_priority;
    UI32_T index;
    SYSLOG_OM_Remote_Server_Config_T server_config;

    orig_priority = SYSLOG_OM_EnterCriticalSection(syslog_om_semid);

    if(ipaddr == NULL)
    {
        SYSLOG_OM_LeaveCriticalSection(syslog_om_semid, orig_priority);

        return SYSLOG_REMOTE_FAIL;
    }

    memset(&server_config,0,sizeof(server_config));
    memcpy(&server_config.ipaddr, ipaddr, sizeof(server_config.ipaddr));

    if (SYSLOG_OM_GetRemoteLogServer(&server_config, &index) == SYSLOG_REMOTE_SUCCESS)
    {
        remotelog_config.server_config[index].udp_port = port;
        SYSLOG_OM_LeaveCriticalSection(syslog_om_semid, orig_priority);

        return SYSLOG_REMOTE_SUCCESS;
    }

    SYSLOG_OM_LeaveCriticalSection(syslog_om_semid, orig_priority);

    return SYSLOG_REMOTE_FAIL;
} /* End of SetRemoteLogServerPort */

/* FUNCTION NAME: SYSLOG_OM_GetRemoteLogServerPort
 * PURPOSE: This function is used to get the remote server port by IP address.
 * INPUT:   *ipaddr -- value of server ip.
 *          *port_p -- value of udp_port
 * OUTPUT:  *port_p -- value of udp_port
 * RETUEN:  SYSLOG_REMOTE_SUCCESS    --  OK, Successful, Without any Error
 *          SYSLOG_REMOTE_FAIL   --  set fail
 *          SYSLOG_REMOTE_INPUT_EXIST    --  input address have existed
 * NOTES:
 */
UI32_T
SYSLOG_OM_GetRemoteLogServerPort(
    L_INET_AddrIp_T *ipaddr,
    UI32_T *port_p)
{
    UI32_T orig_priority;
    UI32_T index;
    SYSLOG_OM_Remote_Server_Config_T server_config;

    orig_priority = SYSLOG_OM_EnterCriticalSection(syslog_om_semid);

    if(ipaddr == NULL)
    {
        SYSLOG_OM_LeaveCriticalSection(syslog_om_semid, orig_priority);

        return SYSLOG_REMOTE_FAIL;
    }

    memset(&server_config,0,sizeof(server_config));
    memcpy(&server_config.ipaddr, ipaddr, sizeof(server_config.ipaddr));

    if (SYSLOG_OM_GetRemoteLogServer(&server_config, &index) == SYSLOG_REMOTE_SUCCESS)
    {
        *port_p = remotelog_config.server_config[index].udp_port;
        SYSLOG_OM_LeaveCriticalSection(syslog_om_semid, orig_priority);

        return SYSLOG_REMOTE_SUCCESS;
    }

    SYSLOG_OM_LeaveCriticalSection(syslog_om_semid, orig_priority);

    return SYSLOG_REMOTE_FAIL;
}/* End of SYSLOG_OM_GetRemoteLogServerPort */

/* FUNCTION NAME: SYSLOG_OM_DeleteAllRemoteLogServer
 * PURPOSE: This function is used to delete all server ip address.
 * INPUT:   NONE.
 * OUTPUT:  NONE.
 * RETUEN:  SYSLOG_REMOTE_SUCCESS    --  OK, Successful, Without any Error
 *          SYSLOG_REMOTE_FAIL   --  set fail
 *          REMOTELOG_EXIST    --  input address have existed
 * NOTES:
 */
UI32_T
SYSLOG_OM_DeleteAllRemoteLogServer()
{
    UI8_T  i;
    UI32_T orig_priority;

    orig_priority = SYSLOG_OM_EnterCriticalSection(syslog_om_semid);

    for (i = 0; i < _countof(remotelog_config.server_config); i ++)
    {
        memset(&remotelog_config.server_config[i].ipaddr, 0, sizeof(L_INET_AddrIp_T));
#if(SYS_CPNT_REMOTELOG_FACILITY_LEVEL_FOR_EVERY_SERVER == TRUE)
        remotelog_config.server_config[i].facility = SYS_DFLT_REMOTELOG_FACILITY_TYPE;
        remotelog_config.server_config[i].level = SYS_DFLT_REMOTELOG_LEVEL;
#endif
    }

    SYSLOG_OM_LeaveCriticalSection(syslog_om_semid, orig_priority);

    return SYSLOG_REMOTE_SUCCESS;
} /* End of SYSLOG_OM_DeleteAllRemoteLogServer */

static void
SYSLOG_OM_SortRemoteLogServerTableByIPAddress()
{
    I32_T i = 0;
    UI32_T j = 0;

    SYSLOG_OM_Remote_Server_Config_T *entry;
    SYSLOG_OM_Remote_Server_Config_T temp_entry;

    entry = remotelog_config.server_config;

    if (entry == NULL)
    {

        return;
    }

    /* use bubble sort
     */
    for (i = _countof(remotelog_config.server_config) - 1; i > 0; i--)
    {
        for (j = 0; j < i ; j ++)
        {
            /* start address = 0 treat as the biggest value, so do not swap
             * for endian issue, use memcmp to do comparing.
             */
            if ((0 < L_INET_CompareInetAddr((L_INET_Addr_T *)&entry[j].ipaddr,
                (L_INET_Addr_T *)&entry[j + 1].ipaddr,
                L_INET_COMPARE_FLAG_INCLUDE_ADDRRESS_LENGTH) &&
                 TRUE == SYSLOG_OM_IsValidIPAddress(&entry[j + 1].ipaddr)) ||
                (FALSE == SYSLOG_OM_IsValidIPAddress(&entry[j].ipaddr) &&
                 TRUE == SYSLOG_OM_IsValidIPAddress(&entry[j + 1].ipaddr)))
            {
                memcpy(&temp_entry, &(entry[j]), sizeof(temp_entry));
                memcpy(&(entry[j]), &(entry[j + 1]), sizeof(entry[j]));
                memcpy(&(entry[j + 1]), &temp_entry, sizeof(entry[j + 1]));
                entry[j + 1].index = entry[j].index;
                entry[j].index = temp_entry.index;
            }
        }
    }

    return;
}/* End of SYSLOG_OM_SortRemoteLogServerTableByIPAddress */

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYSLOG_OM_IsValidIPAddress
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns TRUE if the IP is valid.
 *          Otherwise, FALSE is returned.
 * INPUT:   ip_addr: to specify a unique ip filter address.
 * OUTPUT:  NONE.
 * RETURN:  TRUE or FALSE
 * NOTES:
 * ---------------------------------------------------------------------
 */
static BOOL_T
SYSLOG_OM_IsValidIPAddress(
   const L_INET_AddrIp_T *ip_addr)
{
    if(NULL == ip_addr)
    {
        return FALSE;
    }

    /* only support v4 and v6
     */
    if((L_INET_ADDR_TYPE_IPV4 != ip_addr->type) && (L_INET_ADDR_TYPE_IPV6 != ip_addr->type) &&
       (L_INET_ADDR_TYPE_IPV4Z != ip_addr->type) && (L_INET_ADDR_TYPE_IPV6Z != ip_addr->type))
    {
        return FALSE;
    }

    if (L_INET_ADDR_TYPE_IPV4 == ip_addr->type || L_INET_ADDR_TYPE_IPV4Z == ip_addr->type)
    {
        if (IP_LIB_OK != IP_LIB_IsValidForRemoteIp((UI8_T *)ip_addr->addr))
        {
            return FALSE;
        }
    }
    else if (L_INET_ADDR_TYPE_IPV6 == ip_addr->type || L_INET_ADDR_TYPE_IPV6Z == ip_addr->type)
    {
        if (IP_LIB_OK != IP_LIB_CheckIPv6PrefixForInterface((UI8_T *)ip_addr->addr, 128))
        {
            return FALSE;
        }
    }

    if (L_INET_ADDR_TYPE_IPV6 == ip_addr->type)
    {
        static UI8_T zero_v6ip[SYS_ADPT_IPV6_ADDR_LEN] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

        if (0 == memcmp(zero_v6ip, &ip_addr->addr, sizeof( zero_v6ip)))
        {
            return FALSE;
        }
    }

    return TRUE;
}/* End of SYSLOG_OM_IsValidIPAddress */

#if (SYS_CPNT_REMOTELOG_FACILITY_LEVEL_FOR_EVERY_SERVER == TRUE)

/* FUNCTION NAME: SYSLOG_OM_SetRemoteLogServerFacility
 * PURPOSE: This function is used to set the remote log server facility.
 * INPUT:   ipaddr -- value of server ip.
 *          facility -- value of facility
 * OUTPUT:  NONE.
 * RETUEN:  SYSLOG_REMOTE_SUCCESS    --  OK, Successful, Without any Error
 *          SYSLOG_REMOTE_FAIL   --  set fail
 *          SYSLOG_REMOTE_INPUT_EXIST    --  input address have existed
 * NOTES:
 */
UI32_T
SYSLOG_OM_SetRemoteLogServerFacility(
    L_INET_AddrIp_T *ipaddr,
    UI32_T facility)
{
    UI32_T orig_priority;
    UI32_T index;
    UI8_T  temp_index = 0;
    UI8_T  first = 1;
    SYSLOG_OM_Remote_Server_Config_T server_config

    orig_priority = SYSLOG_OM_EnterCriticalSection(syslog_om_semid);

    if((ipaddr == NULL) ||
       (facility < SYSLOG_TYPE_MIN_FACILITY) ||
       (facility > SYSLOG_TYPE_MAX_FACILITY))
    {
        SYSLOG_OM_LeaveCriticalSection(syslog_om_semid, orig_priority);

        return SYSLOG_REMOTE_FAIL;
    }

    memcpy(server_config.ipaddr, ipaddr, sizeof(server_config.ipaddr));

    if (SYSLOG_OM_GetRemoteLogServer(&server_config, &index) == SYSLOG_REMOTE_SUCCESS)
    {
        remotelog_config.server_config[index].facility = facility;
        SYSLOG_OM_LeaveCriticalSection(syslog_om_semid, orig_priority);

        return SYSLOG_REMOTE_SUCCESS;
    }

    for (index = 0; index < _countof(remotelog_config.server_config); index ++)
    {
        if (remotelog_config.server_config[index].ipaddr.addrlen == 0)
        {
            if (first)
            {
                temp_index = index;
            }

            first = 0;
        }
    }

    if (first)
    {
        SYSLOG_OM_LeaveCriticalSection(syslog_om_semid, orig_priority);

        return SYSLOG_REMOTE_FAIL;
    }

    memcpy(&remotelog_config.server_config[temp_index].ipaddr,
           ipaddr,
           sizeof(remotelog_config.server_config[temp_index].ipaddr));
    remotelog_config.server_config[temp_index].facility = facility;
    remotelog_config.server_config[temp_index].udp_port = SYS_DFLT_SYSLOG_HOST_PORT;

    SYSLOG_OM_LeaveCriticalSection(syslog_om_semid, orig_priority);

    return SYSLOG_REMOTE_SUCCESS;
}/* End of SYSLOG_OM_SetRemoteLogServerFacility */

/* FUNCTION NAME: SYSLOG_OM_SetRemoteLogServerLevel
 * PURPOSE: This function is used to set the remote log server trap level.
 * INPUT:   ipaddr -- value of server ip.
 *          level  -- value of trap level
 * OUTPUT:  NONE.
 * RETUEN:  SYSLOG_REMOTE_SUCCESS     --  OK, Successful, Without any Error
 *          SYSLOG_REMOTE_FAIL        --  set fail
 *          SYSLOG_REMOTE_INPUT_EXIST --  input address have existed
 *
 * NOTES:
 */
UI32_T
SYSLOG_OM_SetRemoteLogServerLevel(
    L_INET_AddrIp_T *ipaddr,
    UI32_T level)
{
    UI32_T orig_priority;
    UI32_T index;
    UI8_T  temp_index = 0;
    UI8_T  first = 1;
    SYSLOG_OM_Remote_Server_Config_T server_config

    orig_priority = SYSLOG_OM_EnterCriticalSection(syslog_om_semid);

    if(ipaddr == NULL) ||
       (level < SYSLOG_TYPE_MIN_LEVEL) ||
       (level > SYSLOG_TYPE_MAX_LEVEL))
    {
        SYSLOG_OM_LeaveCriticalSection(syslog_om_semid, orig_priority);

        return SYSLOG_REMOTE_FAIL;
    }

    memcpy(server_config.ipaddr, ipaddr, sizeof(server_config.ipaddr));

    if (SYSLOG_OM_GetRemoteLogServer(&server_config, &index) == SYSLOG_REMOTE_SUCCESS)
    {
        remotelog_config.server_config[index].level = level;
        SYSLOG_OM_LeaveCriticalSection(syslog_om_semid, orig_priority);

        return SYSLOG_REMOTE_SUCCESS;
    }

    for (index = 0; index < _countof(remotelog_config.server_config); index ++)
    {
        if (remotelog_config.server_config[index].ipaddr.addrlen == 0)
        {
            if (first)
            {
                temp_index = index;
            }

            first = 0;
        }
    }

    if (first)
    {
        SYSLOG_OM_LeaveCriticalSection(syslog_om_semid, orig_priority);

        return SYSLOG_REMOTE_FAIL;
    }

    memcpy(&remotelog_config.server_config[temp_index].ipaddr,
           ipaddr,
           sizeof(remotelog_config.server_config[temp_index].ipaddr));
    remotelog_config.server_config[temp_index].level = level;
    remotelog_config.server_config[temp_index].udp_port = SYS_DFLT_SYSLOG_HOST_PORT;

    SYSLOG_OM_LeaveCriticalSection(syslog_om_semid, orig_priority);

    return SYSLOG_REMOTE_SUCCESS;
}/* End of SYSLOG_OM_SetRemoteLogServerLevel */

#else
/* FUNCTION NAME: SYSLOG_OM_SetRemoteLogFacility
 * PURPOSE: This function is used to set the remote log server facility.
 * INPUT:   facility -- value of facility
 * OUTPUT:  NONE.
 * RETUEN:  SYSLOG_REMOTE_SUCCESS    --  OK, Successful, Without any Error
 *          SYSLOG_REMOTE_FAIL   --  set fail
 *          SYSLOG_REMOTE_INPUT_EXIST    --  input address have existed
 * NOTES:
 */
UI32_T
SYSLOG_OM_SetRemoteLogFacility(
    UI32_T facility)
{
    UI32_T orig_priority;

    orig_priority = SYSLOG_OM_EnterCriticalSection(syslog_om_semid);

    if ((facility < SYSLOG_REMOTE_FACILITY_KERN) || (facility > SYSLOG_REMOTE_FACILITY_LOCAL7))
    {
#if (SYS_CPNT_EH == TRUE)   /* Aaron add for EH component, 2002/02/18 */
        UI8_T   buff1[32] = {0};

        sprintf(buff1, "The input facility type: %lu", facility);
        EH_PMGR_Handle_Exception1(SYS_MODULE_SYSLOG,
                                  SYSLOG_OM_SET_FACILITY_TYPE_FUNC_NO,
                                  EH_TYPE_MSG_IS_INVALID,
                                  (SYSLOG_LEVEL_INFO),
                                  buff1);
#endif
        SYSLOG_OM_LeaveCriticalSection(syslog_om_semid, orig_priority);

        return SYSLOG_REMOTE_INVALID;
    }

    remotelog_config.facility = facility;

    SYSLOG_OM_LeaveCriticalSection(syslog_om_semid, orig_priority);

    return SYSLOG_REMOTE_SUCCESS;
}/* End of SYSLOG_OM_SetRemoteLogFacility */

/* FUNCTION NAME: SYSLOG_OM_GetRemoteLogFacility
 * PURPOSE: This function is used to get the log facility.
 * INPUT:   *facility_p -- output buffer of remotelog facility.
 * OUTPUT:  *facility_p -- value of remotelog facility.
 * RETUEN:  SYSLOG_REMOTE_SUCCESS    --  OK, Successful, Without any Error
 *          SYSLOG_REMOTE_INVALID   --  remotelog facility is invalid
 * NOTES:
 */
UI32_T
SYSLOG_OM_GetRemoteLogFacility(
    UI32_T *facility_p)
{
    UI32_T orig_priority;

    orig_priority = SYSLOG_OM_EnterCriticalSection(syslog_om_semid);

    if (facility_p == NULL)
    {
#if (SYS_CPNT_EH == TRUE)   /* Aaron add for EH component, 2002/02/18 */
        EH_PMGR_Handle_Exception(SYS_MODULE_SYSLOG,
                                 SYSLOG_OM_GET_FACILITY_TYPE_FUNC_NO,
                                 EH_TYPE_MSG_NULL_POINTER,
                                 (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_CRIT));
#endif
        SYSLOG_OM_LeaveCriticalSection(syslog_om_semid, orig_priority);

        return SYSLOG_REMOTE_INVALID_BUFFER;
    }

    *facility_p = remotelog_config.facility;

    SYSLOG_OM_DEBUG_LOG("\r\nRemotelog get facility type : %ld",(long)*facility_p);

    SYSLOG_OM_LeaveCriticalSection(syslog_om_semid, orig_priority);

    return SYSLOG_REMOTE_SUCCESS;
} /* End of SYSLOG_OM_GetRemoteLogFacility */

/* FUNCTION NAME: SYSLOG_OM_SetRemoteLogLevel
 * PURPOSE: This function is used to set remotelog level.
 * INPUT:   level -- value of remotelog level.
 * RETUEN:  SYSLOG_REMOTE_SUCCESS   --  OK, Successful, Without any Error
 *          SYSLOG_REMOTE_INVALID   --  input value is invalid
 * NOTES:
 */
UI32_T
SYSLOG_OM_SetRemoteLogLevel(
    UI32_T level)
{
    UI32_T orig_priority;

    orig_priority = SYSLOG_OM_EnterCriticalSection(syslog_om_semid);
    if ((level < SYSLOG_LEVEL_EMERG) || (level > SYSLOG_LEVEL_DEBUG))
    {
#if (SYS_CPNT_EH == TRUE)   /* Aaron add for EH component, 2002/02/18 */
        UI8_T   buff1[32] = {0};

        sprintf(buff1, "Remote log level (0-7): ");
        EH_MGR_Handle_Exception1(SYS_MODULE_SYSLOG,
                                 SYSLOG_OM_SET_REMOTELOG_LEVEL_FUNC_NO,
                                 EH_TYPE_MSG_VALUE_OUT_OF_RANGE,
                                 (SYSLOG_LEVEL_INFO),
                                 buff1);
#endif
        SYSLOG_OM_LeaveCriticalSection(syslog_om_semid, orig_priority);

        return SYSLOG_REMOTE_INVALID;
    }

    remotelog_config.level = level;

    SYSLOG_OM_LeaveCriticalSection(syslog_om_semid, orig_priority);

    return SYSLOG_REMOTE_SUCCESS;
}/* End of SYSLOG_OM_SetRemoteLogLevel */

/* FUNCTION NAME: SYSLOG_OM_GetRemoteLogLevel
 * PURPOSE: This function is used to get remotelog level.
 * INPUT:   *remotelog -- output buffer of remotelog level.
 * OUTPUT:  *remotelog -- value of remotelog level.
 * RETUEN:  SYSLOG_REMOTE_SUCCESS    --  OK, Successful, Without any Error
 *          SYSLOG_REMOTE_INVALID   --  input value is invalid
 * NOTES:
 */
UI32_T
SYSLOG_OM_GetRemoteLogLevel(
    UI32_T *level_p)
{
    UI32_T orig_priority;

    orig_priority = SYSLOG_OM_EnterCriticalSection(syslog_om_semid);

    if (level_p == NULL)
    {
#if (SYS_CPNT_EH == TRUE)   /* Aaron add for EH component, 2002/02/18 */
        EH_MGR_Handle_Exception(SYS_MODULE_SYSLOG,
                                SYSLOG_OM_GET_REMOTELOG_LEVEL_FUNC_NO,
                                EH_TYPE_MSG_NULL_POINTER,
                                (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_CRIT));
#endif
        SYSLOG_OM_LeaveCriticalSection(syslog_om_semid, orig_priority);

        return SYSLOG_REMOTE_INVALID_BUFFER;
    }

    *level_p = remotelog_config.level;

    SYSLOG_OM_DEBUG_LOG("\r\nRemotelog get level : %ld", (long)*level_p);

    SYSLOG_OM_LeaveCriticalSection(syslog_om_semid, orig_priority);

    return SYSLOG_REMOTE_SUCCESS;
} /* End of SYSLOG_OM_GetRemoteLogLevel */

#endif /*endif (SYS_CPNT_REMOTELOG_FACILITY_LEVEL_FOR_EVERY_SERVER == TRUE)*/
#endif /*endif (SYS_CPNT_REMOTELOG == TRUE)*/

/* FUNCTION NAME: SYSLOG_OM_GetUcLogPointers
 * PURPOSE: This function is used to save logs on RAM to Flash before reload.
 * INPUT:   None.
 * OUTPUT:  ret_sys_log_ram_db_pp       -- Sys log on RAM.
 *          ret_sys_log_flash_db_pp     -- Sys log on RAM buffer.
 * RETUEN:  TRUE/FALSE.
 * NOTES:   The returned pointer should be used only to save or restore
 *          into/from Flash only.
 */
BOOL_T SYSLOG_OM_GetUcLogPointers(SYSLOG_UcNormalDb_T **ret_sys_log_ram_db_pp,
    SYSLOG_UcFlashDb_T **ret_sys_log_flash_db_pp)
{
    UI32_T orig_priority;

    if (   (NULL == ret_sys_log_ram_db_pp)
        || (NULL == ret_sys_log_flash_db_pp)
        )
    {
        return FALSE;
    }


    orig_priority = SYSLOG_OM_EnterCriticalSection(syslog_om_semid);

    *ret_sys_log_ram_db_pp = (SYSLOG_UcNormalDb_T *) UC_MGR_Allocate(UC_MGR_SYSLOG_UC_NORMAL_INDEX, sizeof(SYSLOG_UcNormalDb_T), 4);;
    *ret_sys_log_flash_db_pp = (SYSLOG_UcFlashDb_T *) UC_MGR_Allocate(UC_MGR_SYSLOG_UC_FLASH_INDEX, sizeof(SYSLOG_UcFlashDb_T), 4);;

    SYSLOG_OM_LeaveCriticalSection(syslog_om_semid, orig_priority);

    return TRUE;
}