/* MODULE NAME:  watchdog_proc.c
 * PURPOSE:
 *    Watchdog process
 *
 * NOTES:
 *
 * REASON:
 * Description:
 * HISTORY
 *    2/8/2011 - Charlie Chen, Created
 *
 * Copyright(C)      Edge-Core Networks, 2011
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>

#include "sys_type.h"
#include "sys_cpnt.h"
#include "sys_bld.h"
#include "sys_adpt.h"
#include "sys_hwcfg.h"

#include "sysfun.h"
#include "sysrsc_mgr.h"

#include "l_cmnlib_init.h"

#if (SYS_CPNT_WATCHDOG_TIMER == TRUE)
#include "sys_time.h"
    #if (SYS_HWCFG_WATCHDOG_TYPE == SYS_HWCFG_WATCHDOG_OS6450)
    #include "dev_swdrv_pmgr.h"
    #endif
#endif

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
#include "uc_mgr.h"
#include "sw_watchdog_mgr.h"
    #if (SYS_CPNT_SYSLOG == TRUE)
    #include "syslog_pmgr.h"
    #endif /* #if (SYS_CPNT_SYSLOG == TRUE) */
#endif /* #if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE) */

/* NAMING CONSTANT DECLARATIONS
 */
#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
#define SW_WATCHDOG_EVENT_TIMER           BIT_1
#define SW_WATCHDOG_MONITOR_TIMER_TICKS   (SYS_BLD_TICKS_PER_SECOND*60)
#endif /* #if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE) */

#if (SYS_CPNT_WATCHDOG_TIMER == TRUE)
#define HW_WATCHDOG_EVENT_TIMER           BIT_2
#define HW_WATCHDOG_MONITOR_TIMER_TICKS   (SYS_ADPT_HW_WATCHDOG_PERIODIC_TIMER_TICKS)
#endif

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void   WTDOG_PROC_Daemonize_Entry(void);
static BOOL_T WTDOG_PROC_InitiateProcessResources(void);
#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
static void   SW_WTDOG_TASK_Main(void);
#endif
#if (SYS_CPNT_WATCHDOG_TIMER == TRUE)
static void   HW_WTDOG_TASK_Main(void);
#endif

/* STATIC VARIABLE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM BODIES
 */
/*------------------------------------------------------------------------------
 * ROUTINE NAME : main
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    the main entry for watchdog process
 *
 * INPUT:
 *    argc     --  the size of the argv array
 *    argv     --  the array of arguments
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    0 -- Success
 *   <0 -- Error
 * NOTES:
 *    This function is the entry point for watchdog process.
 *------------------------------------------------------------------------------
 */
int main(int argc, char* argv[])
{
    UI32_T process_id;
#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE) && (SYS_CPNT_WATCHDOG_TIMER==TRUE)
    /* On ASF4612MMS, it is found that if main thread sched_policy is SYSFUN_SCHED_DEFAULT
     * and the spawned child thread for hw watchdog adpot SYSFUN_SCHED_RR as sched_policy,
     * hw watchdog will timeout and reboot when doing fimware upgrade.
     * After changing main thread sched_policy as SYSFUN_SCHED_RR, no hw watchdog timeout
     * occur when doing firmware upgrade. It seems that the real effect of sched_policy
     * of child thread depends on main thread's sched_policy. So when both sw watchdog
     * and hardware watchdog are enabled, main thread(handling sw watchdog) adopts
     * SYSFUN_SCHED_RR and set priority as lowest priority of realtime priority.
     */
    UI32_T sched_policy = SYSFUN_SCHED_RR;
    UI32_T process_priority = SYS_BLD_REALTIMEPROCESS_LOWEST_PRIORITY;
#elif (SYS_CPNT_SW_WATCHDOG_TIMER != TRUE) && (SYS_CPNT_WATCHDOG_TIMER == TRUE)
    UI32_T sched_policy = SYSFUN_SCHED_RR;
    UI32_T process_priority = SYS_BLD_WTDOG_PROCESS_PRIORITY;
#elif (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE) && (SYS_CPNT_WATCHDOG_TIMER != TRUE)
    UI32_T sched_policy = SYSFUN_SCHED_DEFAULT;
    UI32_T process_priority = SYS_BLD_PROCESS_DEFAULT_PRIORITY;
#endif

#if (SYS_CPNT_WATCHDOG_TIMER == TRUE) || \
        (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
    if (SYSFUN_SpawnProcess(process_priority,
                            sched_policy,
                            (char *)SYS_BLD_WTDOG_PROCESS_NAME,
                            WTDOG_PROC_Daemonize_Entry,
                            NULL,
                            &process_id) != SYSFUN_OK)
    {
        printf("WTDOG_Process SYSFUN_SpawnProcess error.\r\n");
        return -1;
    }
#endif
    return 0;
}

/* LOCAL SUBPROGRAM BODIES
 */

/*------------------------------------------------------------------------------
 * FUNCTION NAME - WTDOG_PROC_Daemonize_Entry
 *------------------------------------------------------------------------------
 * PURPOSE : After the process has been daemonized, the main thread of the
 *           process will call this function to start the main thread.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *------------------------------------------------------------------------------
 */
static void WTDOG_PROC_Daemonize_Entry(void)
{
    UI32_T thread_id;
    if(FALSE==SYSRSC_MGR_AttachSystemResources())
    {
        printf("%s: SYSRSC_MGR_AttachSystemResources fail\n", __FUNCTION__);
        return;
    }
    if (WTDOG_PROC_InitiateProcessResources() == FALSE)
    {
        printf("WTDOG_PROC_InitiateProcessResources fail.\r\n");
        return ;
    }
#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE) && \
        (SYS_CPNT_WATCHDOG_TIMER == TRUE)
    if(SYSFUN_SpawnThread(SYS_BLD_WTDOG_PROCESS_PRIORITY,
                          SYSFUN_SCHED_RR,
                          SYS_BLD_HW_WTDOG_CSC_THREAD_NAME,
                          SYS_BLD_TASK_COMMON_STACK_SIZE,
                          SYSFUN_TASK_FP,
                          HW_WTDOG_TASK_Main,
                          NULL, &thread_id)!=SYSFUN_OK)
    {
        printf("%s:Spawn HW_WTDOG_TASK_Main thread fail.\n", __FUNCTION__);
    }
#endif

#if (SYS_CPNT_SW_WATCHDOG_TIMER != TRUE) && \
    (SYS_CPNT_WATCHDOG_TIMER == TRUE)
    HW_WTDOG_TASK_Main();
#elif (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
    SW_WTDOG_TASK_Main();
#endif

    return;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - WTDOG_PROC_InitiateProcessResources
 *---------------------------------------------------------------------------
 * PURPOSE:  Initiate Process Resources for watchdog process
 * INPUT:    None
 *
 * OUTPUT:   None.
 * RETURN:   TRUE  -- Success
 *           FALSE -- Error
 * NOTE:
 *           This function is the entry point for watchdog process.
 *---------------------------------------------------------------------------
 */
static BOOL_T WTDOG_PROC_InitiateProcessResources(void)
{

    L_CMNLIB_INIT_InitiateProcessResources();

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
    if(UC_MGR_InitiateProcessResources() == FALSE)
    {
       printf("%s: UC_MGR_InitiateProcessResources fail.\r\n", __FUNCTION__);
       return FALSE;
    }
#endif

#if (SYS_CPNT_WATCHDOG_TIMER == TRUE) && \
    (SYS_HWCFG_WATCHDOG_TYPE == SYS_HWCFG_WATCHDOG_OS6450)
    DEV_SWDRV_PMGR_InitiateProcessResource();
#endif

#if (SYS_CPNT_SYSLOG==TRUE)
    SYSLOG_PMGR_InitiateProcessResource();
#endif

   return TRUE;
}
#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
/*--------------------------------------------------------------------------
 * ROUTINE NAME - SW_WTDOG_TASK_Main
 *---------------------------------------------------------------------------
 * PURPOSE:  The main function for sw watchdog
 * INPUT:    None
 * OUTPUT:   None
 * RETURN:   None
 * NOTE:     None
 *---------------------------------------------------------------------------
 */
static void SW_WTDOG_TASK_Main(void)
{
    UI32_T  wtdog_rcv_event;
    UI32_T  local_events=0,received_events;
    void    *sw_wtdog_timer_id;

    sw_wtdog_timer_id = SYSFUN_PeriodicTimer_Create();
    /* register software watchdog timer event
     */
    if (SYSFUN_PeriodicTimer_Start(sw_wtdog_timer_id, SW_WATCHDOG_MONITOR_TIMER_TICKS, SW_WATCHDOG_EVENT_TIMER) == FALSE)
    {
        printf("%s: Start timer failed!\r\n", __FUNCTION__);
    }
    wtdog_rcv_event = SW_WATCHDOG_EVENT_TIMER;

    while(TRUE)
    {
        SYSFUN_ReceiveEvent (wtdog_rcv_event,
                             SYSFUN_EVENT_WAIT_ANY,
                             (local_events==0)?SYSFUN_TIMEOUT_WAIT_FOREVER:SYSFUN_TIMEOUT_NOWAIT,
                             &received_events);
        local_events |= received_events;

        if((local_events & SW_WATCHDOG_EVENT_TIMER))
        {
            SW_WATCHDOG_MGR_HandleTimerEvents();
            local_events ^= SW_WATCHDOG_EVENT_TIMER;
        }
    }
}
#endif

#if (SYS_CPNT_WATCHDOG_TIMER == TRUE)
/*--------------------------------------------------------------------------
 * ROUTINE NAME - HW_WTDOG_TASK_Main
 *---------------------------------------------------------------------------
 * PURPOSE:  The main function for hw watchdog
 * INPUT:    None
 * OUTPUT:   None
 * RETURN:   None
 * NOTE:     None
 *---------------------------------------------------------------------------
 */
static void HW_WTDOG_TASK_Main(void)
{
    UI32_T  wtdog_rcv_event;
    UI32_T  local_events=0,received_events;
    void    *hw_wtdog_timer_id;

    hw_wtdog_timer_id = SYSFUN_PeriodicTimer_Create();
    /* register hardware watchdog timer event
     */
    if (SYSFUN_PeriodicTimer_Start(hw_wtdog_timer_id, HW_WATCHDOG_MONITOR_TIMER_TICKS, HW_WATCHDOG_EVENT_TIMER) == FALSE)
    {
        printf("%s: Start timer failed!\r\n", __FUNCTION__);
    }
    wtdog_rcv_event = HW_WATCHDOG_EVENT_TIMER;


    /* enable hardware watchdog*/
    SYS_TIME_EnableWatchDogTimer();


    while(TRUE)
    {
        SYSFUN_ReceiveEvent (wtdog_rcv_event,
                             SYSFUN_EVENT_WAIT_ANY,
                             (local_events==0)?SYSFUN_TIMEOUT_WAIT_FOREVER:SYSFUN_TIMEOUT_NOWAIT,
                             &received_events);
        local_events |= received_events;

        if((local_events & HW_WATCHDOG_EVENT_TIMER))
        {
            /* Check if H/W watch dog time-out happened or not,
             * if yes, this function will generate a device log.
             * SYS_TIME_CheckWatchDogTimeOut() not support yet
            SYS_TIME_CheckWatchDogTimeOut();
             */
            SYS_TIME_KickWatchDogTimer();
            local_events ^= HW_WATCHDOG_EVENT_TIMER;
        }

    }
}
#endif
