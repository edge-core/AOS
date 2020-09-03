/* Module Name: WTDOG_MGR.C
 * Purpose: This moudle is responsible for (software) watchdog monitor procedure.
 * Notes:
 * History:
 *      31/May/2010    -- First Draft created by Aken Liu
 *       9/Feb/2011    -- Porting to EC platform by Charlie Chen
 *
 * Copyright(C)      Accton Corporation, 2001
 * Copyright(C)      Edge-Core Networks, 2011
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "string.h"
#include "sys_cpnt.h"
#include "sys_bld.h"
#include "sys_module.h"
#include "sysfun.h"
#include "sysrsc_mgr.h"
#include "sw_watchdog_mgr.h"
#include "sys_time.h"
#include "fs.h"
#include "uc_mgr.h"

#if (SYS_CPNT_SYSLOG == TRUE)
#include "syslog_type.h"
#include "syslog_mgr.h"
#include "syslog_pmgr.h"
#endif

/* NAMING CONSTANT DECLARATIONS
 */
/* max number = static spawn + dynamic spawn (ex:telnet's parent + telnet's child)
 * Dynamic spawn id allcation:
 *     Range 1: SW_WATCHDOG_MAX_MONITOR_ID ~ SW_WATCHDOG_MAX_MONITOR_ID+SYS_ADPT_MAX_TELNET_NUM are for server part.
 *     Range 2: SW_WATCHDOG_MAX_MONITOR_ID+SYS_ADPT_MAX_TELNET_NUM ~ SW_WATCHDOG_MAX_MONITOR_ID+SYS_ADPT_MAX_TELNET_NUM*2 are for cli part
 */
#define  SW_WATCHDOG_MAX_THREAD_NUM   (SW_WATCHDOG_MAX_MONITOR_ID + SYS_ADPT_MAX_TELNET_NUM*2)

/* enum function number
 */
enum {
    SW_WATCHDOG_MGR_FUNCTION_NO_REGISTER_MONITOR_THREAD = 0,
    SW_WATCHDOG_MGR_FUNCTION_NO_UNREGISTER_MONITOR_THREAD,
    SW_WATCHDOG_MGR_FUNCTION_NO_RESET_TIMER,
    SW_WATCHDOG_MGR_FUNCTION_NO_HANDLE_TIMER_EVENT,
};

/* enum error number
 */
enum {
    SW_WATCHDOG_MGR_ERROR_NO_INVALID_ARG = 0,
    SW_WATCHDOG_MGR_ERROR_NO_MONITOR_THREAD_TIMEOUT
};

/* MACRO FUNCTION DECLARATIONS
 */
/* macro function to add a debug log entry to syslog
 */
#if (SYS_CPNT_SYSLOG == TRUE)
#define ADD_DEBUG_LOG_ENTRY_TO_SYSLOG(function_num, error_num, fmtstr, ...) \
{ \
    SYSLOG_OM_Record_T syslog_entry; \
\
    memset((UI8_T *)&syslog_entry, 0, sizeof(SYSLOG_OM_Record_T)); \
    syslog_entry.owner_info.level = SYSLOG_LEVEL_DEBUG; \
    syslog_entry.owner_info.module_no = SYS_MODULE_WDT; \
    syslog_entry.owner_info.function_no = (function_num); \
    syslog_entry.owner_info.error_no = (error_num); \
    snprintf(((char*)syslog_entry.message), sizeof(syslog_entry.message), (fmtstr), ##__VA_ARGS__); \
    SYSLOG_PMGR_AddEntry(&syslog_entry); \
}
#else
#define ADD_DEBUG_LOG_ENTRY_TO_SYSLOG(...)
#endif

/* macro function to print debug message to console according to
 * msg_level.
 * debug message is shown only when shmem_data_p->show_watchdog_debug_msg_level
 * does not equal to SW_WATCHDOG_MGR_DEBUG_MSG_LEVEL_NO_MSG and msg_level
 * is less than or equal to shmem_data_p->show_watchdog_debug_msg_level.
 */
#define SHOW_DEBUG_MSG(msg_level, fmtstr, ...) \
    if(shmem_data_p->show_watchdog_debug_msg_level!=SW_WATCHDOG_MGR_DEBUG_MSG_LEVEL_NO_MSG && \
        msg_level<=shmem_data_p->show_watchdog_debug_msg_level) \
        {printf(fmtstr, ##__VA_ARGS__);}

#define SW_WATCHDOG_MGR_ENTER_CRITICAL_SECTION(sem_id) SYSFUN_ENTER_CRITICAL_SECTION(sem_id, SYSFUN_TIMEOUT_WAIT_FOREVER)
#define SW_WATCHDOG_MGR_LEAVE_CRITICAL_SECTION(sem_id) SYSFUN_LEAVE_CRITICAL_SECTION(sem_id)

/* DATA TYPE DECLARATIONS
 */
typedef struct
{
    /* thread id of the thread to be monitored by software watchdog
     */
    UI32_T   thread_id;
    /* this value will be set to monitor_timer when the timer is reset
     * this thread will not be monitored if expired_timer is zero.
     */
    UI32_T   expired_timer;
    /* this value will be decreased by 1 when a timer event is received
     * and the value is not zero. When the value is decreased to
     * zero will trigger software watchdog to reboot the system
     */
    UI32_T   monitor_timer;
    char     thread_name[SYSFUN_TASK_NAME_LENGTH + 1];
} SW_WatchDogEntry_T;

typedef struct
{
    UI32_T                    sw_watchdog_mgr_sem_id;
    /* sofwate watchdog is enabled if the value is SW_WATCHDOG_STATUS_ENABLE
     * and is disabled if the value is SW_WATCHDOG_STATUS_DISABLE
     */
    BOOL_T                    sw_watchdog_status;
    /* debug message level
     */
    UI8_T                     show_watchdog_debug_msg_level;
    /* stop reboot even if sw watchdog timeout occurs if stop_reboot == TRUE
     * this is for debug only
     */
    BOOL_T                    stop_reboot;
    /* the table contains the entries of threads to be monitored
     */
    SW_WatchDogEntry_T        sw_watchdog_monitor_table[SW_WATCHDOG_MAX_THREAD_NUM];
} SW_WATCHDOG_Shmem_Data_T;


/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void SW_WATCHDOG_MGR_AddSysLog(UI32_T sw_watchdog_id);

/* STATIC VARIABLE DECLARATIONS
 */
static SW_WATCHDOG_Shmem_Data_T   *shmem_data_p;
#define sw_watchdog_mgr_sem_id     shmem_data_p->sw_watchdog_mgr_sem_id


/* EXPORTED SUBPROGRAM BODIES
 */

/* FUNCTION NAME - SW_WATCHDOG_MGR_Initiate_System_Resources
 * PURPOSE  : This function is used to initiate the system resources
 *            of software watchdog
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 */
void SW_WATCHDOG_MGR_InitiateSystemResources(void)
{
    shmem_data_p = (SW_WATCHDOG_Shmem_Data_T*)SYSRSC_MGR_GetShMem(SYSRSC_MGR_SW_WATCHDOG_SHMEM_SEGID);
    shmem_data_p->sw_watchdog_status = SW_WATCHDOG_STATUS_ENABLE;
    shmem_data_p->stop_reboot = FALSE;
    shmem_data_p->show_watchdog_debug_msg_level = SW_WATCHDOG_MGR_DEBUG_MSG_LEVEL_NO_MSG;

    memset(shmem_data_p->sw_watchdog_monitor_table, 0, sizeof(SW_WatchDogEntry_T) * SW_WATCHDOG_MAX_MONITOR_ID);

    if(SYSFUN_CreateSem(SYSFUN_SEMKEY_PRIVATE, 1, SYSFUN_SEM_FIFO, &sw_watchdog_mgr_sem_id) !=SYSFUN_OK)
    {
        printf("\r\n SW_WATCHDOG_MGR: Failed to create semaphore.");
        return;
    }

    return;
}

void SW_WATCHDOG_MGR_AttachSystemResources(void)
{
    shmem_data_p = (SW_WATCHDOG_Shmem_Data_T*)SYSRSC_MGR_GetShMem(SYSRSC_MGR_SW_WATCHDOG_SHMEM_SEGID);
}

void SW_WATCHDOG_MGR_GetShMemInfo(SYSRSC_MGR_SEGID_T *segid_p, UI32_T *seglen_p)
{
    *segid_p = SYSRSC_MGR_SW_WATCHDOG_SHMEM_SEGID;
    *seglen_p = sizeof(SW_WATCHDOG_Shmem_Data_T);
}

/* FUNCTION NAME - SW_WATCHDOG_MGR_RegisterMonitorThread
 * PURPOSE  : This function is for each monitored thread to register
 * INPUT    : monitor_id -- the id of each monitored task
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : if the expired_time equal to 0, it means disable to monitor this thread
 */
void SW_WATCHDOG_MGR_RegisterMonitorThread(UI32_T monitor_id, UI32_T thread_id, UI32_T expired_timer)
{
    if (monitor_id < SW_WATCHDOG_MAX_THREAD_NUM)
    {
        shmem_data_p->sw_watchdog_monitor_table[monitor_id].thread_id = thread_id;
        shmem_data_p->sw_watchdog_monitor_table[monitor_id].expired_timer = expired_timer;
        /* initial the default value to expired_timer */
        SW_WATCHDOG_MGR_ENTER_CRITICAL_SECTION(sw_watchdog_mgr_sem_id);
        shmem_data_p->sw_watchdog_monitor_table[monitor_id].monitor_timer = expired_timer;
        SW_WATCHDOG_MGR_LEAVE_CRITICAL_SECTION(sw_watchdog_mgr_sem_id);
        SYSFUN_TaskIDToName(thread_id, shmem_data_p->sw_watchdog_monitor_table[monitor_id].thread_name, sizeof(shmem_data_p->sw_watchdog_monitor_table[monitor_id].thread_name));
        SHOW_DEBUG_MSG(SW_WATCHDOG_MGR_DEBUG_MSG_LEVEL_VERBOSE_MSG, "<%lu> %s register done.... \n",(unsigned long)monitor_id,shmem_data_p->sw_watchdog_monitor_table[monitor_id].thread_name);
    }
    else
    {
        ADD_DEBUG_LOG_ENTRY_TO_SYSLOG(SW_WATCHDOG_MGR_FUNCTION_NO_REGISTER_MONITOR_THREAD,
            SW_WATCHDOG_MGR_ERROR_NO_INVALID_ARG, "Invalid thread_id:%lu", (unsigned long)thread_id);
        SHOW_DEBUG_MSG(SW_WATCHDOG_MGR_DEBUG_MSG_LEVEL_CRITICAL_MSG, "\r\n%s:Invalid thread_id=%lu\r\n", __FUNCTION__, (unsigned long)thread_id);
    }
}

/* FUNCTION NAME - SW_WATCHDOG_MGR_UnregisterMonitorThread
 * PURPOSE  : This function is for unregist the monitored thread
 * INPUT    : monitor_id -- the id of each monitored task
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    :
 */
void SW_WATCHDOG_MGR_UnregisterMonitorThread(UI32_T monitor_id)
{
    if (monitor_id < SW_WATCHDOG_MAX_THREAD_NUM)
    {
        SHOW_DEBUG_MSG(SW_WATCHDOG_MGR_DEBUG_MSG_LEVEL_VERBOSE_MSG, "<%lu> %s unregist done.... \n",(unsigned long)monitor_id ,shmem_data_p->sw_watchdog_monitor_table[monitor_id].thread_name);

        shmem_data_p->sw_watchdog_monitor_table[monitor_id].thread_id = 0;
        shmem_data_p->sw_watchdog_monitor_table[monitor_id].expired_timer = 0;
        SW_WATCHDOG_MGR_ENTER_CRITICAL_SECTION(sw_watchdog_mgr_sem_id);
        shmem_data_p->sw_watchdog_monitor_table[monitor_id].monitor_timer = 0;
        SW_WATCHDOG_MGR_LEAVE_CRITICAL_SECTION(sw_watchdog_mgr_sem_id);
        memset(shmem_data_p->sw_watchdog_monitor_table[monitor_id].thread_name,0,sizeof(shmem_data_p->sw_watchdog_monitor_table[monitor_id].thread_name));
    }
    else
    {
        ADD_DEBUG_LOG_ENTRY_TO_SYSLOG(SW_WATCHDOG_MGR_FUNCTION_NO_UNREGISTER_MONITOR_THREAD,
            SW_WATCHDOG_MGR_ERROR_NO_INVALID_ARG, "Invalid thread_id:%lu", (unsigned long)monitor_id);
        SHOW_DEBUG_MSG(SW_WATCHDOG_MGR_DEBUG_MSG_LEVEL_CRITICAL_MSG, "\r\n%s:Invalid thread_id=%lu\r\n", __FUNCTION__, (unsigned long)monitor_id);
    }
}

/* FUNCTION NAME - SW_WATCHDOG_MGR_ResetTimer
 * PURPOSE  : This function is for each monitored thread to reset its monitor timer
 * INPUT    : monitor_id -- the id of each monitored task
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 */
void SW_WATCHDOG_MGR_ResetTimer(UI32_T monitor_id)
{
    if (monitor_id >= 0 && monitor_id < SW_WATCHDOG_MAX_THREAD_NUM)
    {
        SW_WATCHDOG_MGR_ENTER_CRITICAL_SECTION(sw_watchdog_mgr_sem_id);
        shmem_data_p->sw_watchdog_monitor_table[monitor_id].monitor_timer = shmem_data_p->sw_watchdog_monitor_table[monitor_id].expired_timer;
        SW_WATCHDOG_MGR_LEAVE_CRITICAL_SECTION(sw_watchdog_mgr_sem_id);
        SHOW_DEBUG_MSG(SW_WATCHDOG_MGR_DEBUG_MSG_LEVEL_VERBOSE_MSG, "[%s] <%lu> reset.... \n",__FUNCTION__,(unsigned long)monitor_id);
    }
    else
    {
        ADD_DEBUG_LOG_ENTRY_TO_SYSLOG(SW_WATCHDOG_MGR_FUNCTION_NO_RESET_TIMER,
            SW_WATCHDOG_MGR_ERROR_NO_INVALID_ARG, "Invalid thread_id:%lu", (unsigned long)monitor_id);
        SHOW_DEBUG_MSG(SW_WATCHDOG_MGR_DEBUG_MSG_LEVEL_CRITICAL_MSG, "\r\n%s:Invalid thread_id=%lu\r\n", __FUNCTION__, (unsigned long)monitor_id);
    }
}


/* FUNCTION NAME - SW_WATCHDOG_MGR_HandleTimerEvents
 * PURPOSE  : This function is software watchdog main routine.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 */
void SW_WATCHDOG_MGR_HandleTimerEvents(void)
{
    UI32_T  sw_watchdog_id=0;
    UI8_T   command[64]={0};
    UC_MGR_Sys_Info_T sys_info;
    BOOL_T  restart_flag=FALSE, is_syslog_not_work=FALSE;

    if (shmem_data_p->sw_watchdog_status == SW_WATCHDOG_STATUS_DISABLE)
    {
        return;
    }

    for (sw_watchdog_id=0; sw_watchdog_id<SW_WATCHDOG_MAX_THREAD_NUM ; sw_watchdog_id++)
    {
        if (shmem_data_p->sw_watchdog_monitor_table[sw_watchdog_id].thread_id == 0)
        {
            continue;
        }
        else
        {
            SHOW_DEBUG_MSG(SW_WATCHDOG_MGR_DEBUG_MSG_LEVEL_VERBOSE_MSG, "<%lu> Name: %s M_timer: %lu E_timer: %lu \n",(unsigned long)sw_watchdog_id,shmem_data_p->sw_watchdog_monitor_table[sw_watchdog_id].thread_name,(unsigned long)shmem_data_p->sw_watchdog_monitor_table[sw_watchdog_id].monitor_timer,(unsigned long)shmem_data_p->sw_watchdog_monitor_table[sw_watchdog_id].expired_timer);
        }

        /* if the expired_timer = 0, we don't monitor this thread.*/
        if (shmem_data_p->sw_watchdog_monitor_table[sw_watchdog_id].expired_timer == 0)
        {
            SHOW_DEBUG_MSG(SW_WATCHDOG_MGR_DEBUG_MSG_LEVEL_VERBOSE_MSG, "<%lu> Name: %s skip... \n",(unsigned long)sw_watchdog_id,shmem_data_p->sw_watchdog_monitor_table[sw_watchdog_id].thread_name);
            continue;
        }

        if (shmem_data_p->sw_watchdog_monitor_table[sw_watchdog_id].monitor_timer <= (shmem_data_p->sw_watchdog_monitor_table[sw_watchdog_id].expired_timer/2))
        {
            SHOW_DEBUG_MSG(SW_WATCHDOG_MGR_DEBUG_MSG_LEVEL_IMPORTANT_MSG, "<%lu> Name:%s monitor_timer=%lu.\n",(unsigned long)sw_watchdog_id,shmem_data_p->sw_watchdog_monitor_table[sw_watchdog_id].thread_name, (unsigned long)shmem_data_p->sw_watchdog_monitor_table[sw_watchdog_id].monitor_timer);

        }

        if (shmem_data_p->sw_watchdog_monitor_table[sw_watchdog_id].monitor_timer == 0)
        {
            char shell_cmd[80];

            SHOW_DEBUG_MSG(SW_WATCHDOG_MGR_DEBUG_MSG_LEVEL_CRITICAL_MSG, "<%lu> Name:%s time-out.\n",(unsigned long)sw_watchdog_id,shmem_data_p->sw_watchdog_monitor_table[sw_watchdog_id].thread_name);

            /* execute /etc/sw_wtd_log.sh to log timeout thread stackframe under /flash
             */
            snprintf(shell_cmd, sizeof(shell_cmd), "/etc/sw_wtd_log.sh %lu %s", (unsigned long)shmem_data_p->sw_watchdog_monitor_table[sw_watchdog_id].thread_id, shmem_data_p->sw_watchdog_monitor_table[sw_watchdog_id].thread_name);
            shell_cmd[79]=0;
            system(shell_cmd);

            restart_flag = TRUE;

            if (sw_watchdog_id != SW_WATCHDOG_UTILITY_SYSLOG && is_syslog_not_work == FALSE)
            {
                SW_WATCHDOG_MGR_AddSysLog(sw_watchdog_id);
            }
            else
            {
                /* syslog thread has something wrong!! Should not log,
                 * otherwise, sw watchdog will wait syslog_pmgr forever
                 */
                is_syslog_not_work = TRUE;
                SHOW_DEBUG_MSG(SW_WATCHDOG_MGR_DEBUG_MSG_LEVEL_CRITICAL_MSG, "SYSLOG does not work. This software watchdog timeout will not be written to syslog\r\n");
            }

        }
        else
        {
            SW_WATCHDOG_MGR_ENTER_CRITICAL_SECTION(sw_watchdog_mgr_sem_id);
            shmem_data_p->sw_watchdog_monitor_table[sw_watchdog_id].monitor_timer--;
            SW_WATCHDOG_MGR_LEAVE_CRITICAL_SECTION(sw_watchdog_mgr_sem_id);
            SYSFUN_SendEvent(shmem_data_p->sw_watchdog_monitor_table[sw_watchdog_id].thread_id, SYSFUN_SYSTEM_EVENT_SW_WATCHDOG);
        }
    }

    if (restart_flag == TRUE && shmem_data_p->stop_reboot==FALSE)
    {
        SHOW_DEBUG_MSG(SW_WATCHDOG_MGR_DEBUG_MSG_LEVEL_CRITICAL_MSG, "System reboot due to software watchdog timeout.\r\n");

        /* disable hardware watchdog timer for syslog and reload procedure. Avoid WDT is trigged again.
         */
#if (SYS_CPNT_WATCHDOG_TIMER == TRUE)
        SYS_TIME_DisableWatchDogTimer();
#endif

        /* wait for any CSC writes files complete.*/
        FS_Shutdown();

        /* reboot via linux kernel */
        if (UC_MGR_GetSysInfo(&sys_info) != TRUE)
        {
            /* Should not happen!!!
             * If happen, re-enable hw watchdog and enter while-loop to trig hw watchdog restart the device.
             */
#if (SYS_CPNT_WATCHDOG_TIMER == TRUE)
            SYS_TIME_EnableWatchDogTimer();
#endif
            while (1)
            {
                /* Critical issue!! continue show error message in the console.*/
                printf("\r\nFailed to get system infomation from UC\r\n");
                SYSFUN_Sleep(100);
            }
            return;
        }
        sprintf((char *)command, "type=3,startaddr=%p,bootreasonaddr=%p", sys_info.warm_start_entry, sys_info.boot_reason_addr);

        if (shmem_data_p->show_watchdog_debug_msg_level == SW_WATCHDOG_MGR_DEBUG_MSG_LEVEL_VERBOSE_MSG)
        {
            printf ("\nRestart now.\n");
            SYSFUN_Sleep(100);
        }

#if (SYS_CPNT_ONIE_SUPPORT==TRUE)
        /* On projects that support ONIE, the system is started and stopped by
         * a set of rc scripts. Execute "/sbin/reboot" will trigger the system
         * to execute the scripts to do required operations(e.g. unmount all mounted
         * file systems) before rebooting the system. It is better to do the
         * gracefully shutdown procedure whenever possible. Failed to shutdown
         * gracefully might lead to corrupted files on flash.
         */
        if(SYSFUN_ExecuteSystemShell("/sbin/reboot")==SYSFUN_OK)
        {
            SYSFUN_Sleep(60*SYS_BLD_TICKS_PER_SECOND); /* if the system cannot reboot in 60 seconds, continue to call SYSFUN_Reboot() */
        }
#endif

        SYSFUN_Reboot((char *)command);
        return;
    }
}

/* FUNCTION NAME - SW_WATCHDOG_MGR_AddSysLog
 * PURPOSE  : This function will add an entry about sw watchdog timeout
 *            to syslog.
 * INPUT    : sw_watchdog_id -- software watchdog id
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : 1. This function can only be called when syslog works properly.
 *            2. This function will call SYSLOG_PMGR_AddEntrySync() and
 *               will trigger syslog to sync log from RAM to FLASH.
 */
static void SW_WATCHDOG_MGR_AddSysLog(UI32_T sw_watchdog_id)
{
#if (SYS_CPNT_SYSLOG == TRUE)
    SYSLOG_OM_Record_T syslog_entry;

    memset((UI8_T *)&syslog_entry, 0, sizeof(SYSLOG_OM_Record_T));
    syslog_entry.owner_info.level = SYSLOG_LEVEL_ERR;
    syslog_entry.owner_info.module_no = SYS_MODULE_WDT;
    syslog_entry.owner_info.function_no = SW_WATCHDOG_MGR_FUNCTION_NO_HANDLE_TIMER_EVENT;
    syslog_entry.owner_info.error_no = SW_WATCHDOG_MGR_ERROR_NO_MONITOR_THREAD_TIMEOUT;
    snprintf((char*)syslog_entry.message, sizeof(syslog_entry.message),
             "SW Watchdog Timeout due to thread %s",
             shmem_data_p->sw_watchdog_monitor_table[sw_watchdog_id].thread_name);

    if(SYSLOG_PMGR_AddEntrySync(&syslog_entry)==FALSE)
    {
        printf("\r\n%s(%d): Failed to add an entry to syslog synchronous.\r\n",
            __FUNCTION__, __LINE__);
    }
#endif
    return;
}


/* FUNCTION NAME - SW_WATCHDOG_MGR_SetMonitorStatus
 * PURPOSE  : This function is for superuser to enable/disable software watchdog
 * INPUT    : monitor_status -- enable/disable
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 */
BOOL_T SW_WATCHDOG_MGR_SetMonitorStatus(BOOL_T monitor_status)
{
    shmem_data_p->sw_watchdog_status = monitor_status;
    return TRUE;
}

/* FUNCTION NAME - SW_WATCHDOG_MGR_GetMonitorStatus
 * PURPOSE  : This function is for superuser get the status of software watchdog
 * INPUT    : monitor_status -- enable/disable
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 */
BOOL_T SW_WATCHDOG_MGR_GetMonitorStatus(BOOL_T *monitor_status)
{
    *monitor_status = shmem_data_p->sw_watchdog_status;
    return TRUE;
}

/* FUNCTION NAME - SW_WATCHDOG_MGR_SetDebugMsgLevel
 * PURPOSE  : Set the status of software watchdog debug message level
 * INPUT    : debug_msg_level -- SW_WATCHDOG_MGR_DEBUG_MSG_LEVEL_XXX
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : When debug_msg_level == SW_WATCHDOG_MGR_DEBUG_MSG_LEVEL_NO_MSG
 *            no debug message will be output to console.
 */
void SW_WATCHDOG_MGR_SetDebugMsgLevel(UI8_T debug_msg_level)
{
    shmem_data_p->show_watchdog_debug_msg_level = debug_msg_level;
    return;
}

/* FUNCTION NAME - SW_WATCHDOG_MGR_GetDebugMsgLevel
 * PURPOSE  : Get the status of software watchdog debug message level
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : debug message level
 * NOTES    : When debug_msg_level == SW_WATCHDOG_MGR_DEBUG_MSG_LEVEL_NO_MSG
 *            no debug message will be output to console.
 */
UI8_T SW_WATCHDOG_MGR_GetDebugMsgLevel(void)
{
    return shmem_data_p->show_watchdog_debug_msg_level;
}

/* FUNCTION NAME - SW_WATCHDOG_MGR_SetStopRebootStatus
 * PURPOSE  : Set the status of stop reboot. If TRUE is set,
 *            software watchdog will not reboot when a software watchdog
 *            timeout occurs.
 * INPUT    : is_stop_reboot - the value of stop_reboot
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : This is for debug only
 */
void SW_WATCHDOG_MGR_SetStopRebootStatus(BOOL_T is_stop_reboot)
{
    shmem_data_p->stop_reboot=is_stop_reboot;
}

/* FUNCTION NAME - SW_WATCHDOG_MGR_GetStopRebootStatus
 * PURPOSE  : Get the status of stop reboot.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : stop_reboot status
 * NOTES    : This is for debug only
 */
BOOL_T SW_WATCHDOG_MGR_GetStopRebootStatus(void)
{
    return shmem_data_p->stop_reboot;
}

/* FUNCTION NAME - SW_WATCHDOG_MGR_DumpMonitoredThread
 * PURPOSE  : List the status of all monitored thread
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 */
void SW_WATCHDOG_MGR_DumpMonitoredThread(void)
{
    UI32_T sw_watchdog_id = 0;

    for (sw_watchdog_id=0; sw_watchdog_id<SW_WATCHDOG_MAX_THREAD_NUM ; sw_watchdog_id++)
    {
        if (shmem_data_p->sw_watchdog_monitor_table[sw_watchdog_id].thread_id == 0)
        {
            continue;
        }
        else
        {
            printf ("<%lu> Name: %s T_id: %lu M_timer: %lu E_timer: %lu \n",(unsigned long)sw_watchdog_id,shmem_data_p->sw_watchdog_monitor_table[sw_watchdog_id].thread_name,(unsigned long)shmem_data_p->sw_watchdog_monitor_table[sw_watchdog_id].thread_id, (unsigned long)shmem_data_p->sw_watchdog_monitor_table[sw_watchdog_id].monitor_timer,(unsigned long)shmem_data_p->sw_watchdog_monitor_table[sw_watchdog_id].expired_timer);
        }
    }
    return;
}

