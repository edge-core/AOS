/* MODULE NAME:  sys_time_task.c
 * PURPOSE:
 *   Implementation of SYS_TIME task.
 *
 * NOTES:
 *   In order to keep the information of the accumulated system up time on a
 *   device and provides this information to ALU(Accton License) to verify that
 *   whether the license had been used for the time period more than the period
 *   offered by the license.
 *
 *   SYS_TIME task is responsible to count the system up time periodically and
 *   write the information of the accumulated system up time to a non-volatile
 *   storage device to ensure the information can be kept among every power
 *   cycle on the device. This vital information will be kept through the
 *   data storage service provided by FS.
 *
 *   Only needs to create SYS_TIME task when SYS_CPNT_SYS_TIME_ACCUMULATED_SYSTEM_UP_TIME
 *   is defined as TRUE.
 *
 * HISTORY
 *    9/8/2015 - Charlie Chen, Created
 *
 * Copyright(C)      Accton Corporation, 2015
 */
 
/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_bld.h"
#include "sys_cpnt.h"

#if (SYS_CPNT_SYS_TIME_ACCUMULATED_SYSTEM_UP_TIME == TRUE)
#include "sys_time.h"
#include "sys_time_task.h"
#include "sw_watchdog_mgr.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define SYS_TIME_PERIODIC_TICKS (60*SYS_BLD_TICKS_PER_SECOND)
#define SYS_TIME_EVENT_TIMER BIT_0

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */
static void SYS_TIME_TASK_TaskMain(void);

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME: SYS_TIME_TASK_CreateTask
 *-----------------------------------------------------------------------------
 * PURPOSE: Create task in SYS_TIME
 *-----------------------------------------------------------------------------
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 *-----------------------------------------------------------------------------
 * NOTES:
 */
void SYS_TIME_TASK_CreateTask(void)
{
    UI32_T ret;
    UI32_T thread_id=0;

    if(SYSFUN_SpawnThread(SYS_BLD_SYS_TIME_THREAD_PRIORITY,
                          SYS_BLD_SYS_TIME_THREAD_SCHED_POLICY,
                          SYS_BLD_SYS_TIME_THREAD_NAME,
                          SYS_BLD_TASK_COMMON_STACK_SIZE,
                          SYSFUN_TASK_FP,
                          SYS_TIME_TASK_TaskMain,
                          NULL, &thread_id)!=SYSFUN_OK)
    {
        printf("\r\n%s:Spawn SYSTIME thread fail.", __FUNCTION__);
    }

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
#ifndef SYS_ADPT_SYS_TIME_SW_WATCHDOG_TIMER
#define SYS_ADPT_SYS_TIME_SW_WATCHDOG_TIMER SYS_ADPT_SW_WATCHDOG_TIMER_DEFAULT
#endif
    SW_WATCHDOG_MGR_RegisterMonitorThread(SW_WATCHDOG_SYS_TIME, thread_id, SYS_ADPT_SYS_TIME_SW_WATCHDOG_TIMER);
#endif

}

/* LOCAL SUBPROGRAM BODIES
 */
/* FUNCTION NAME: SYS_TIME_TASK_TaskMain
 *-----------------------------------------------------------------------------
 * PURPOSE: Entry function for SYS_TIME task
 *-----------------------------------------------------------------------------
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 *-----------------------------------------------------------------------------
 * NOTES:
 */
static void SYS_TIME_TASK_TaskMain(void)
{
    void*  timer_id;
    UI32_T local_event=0;
    UI32_T event=0;


    timer_id = SYSFUN_PeriodicTimer_Create();
    if (SYSFUN_PeriodicTimer_Start(timer_id, SYS_TIME_PERIODIC_TICKS, SYS_TIME_EVENT_TIMER)==FALSE)
    {
        printf("%s(%d)SYSFUN_PeriodicTimer_Start error!\r\n", __FUNCTION__, __LINE__);
    }

    /* call SYS_TIME_BumpUpAccumulatedSysUpTime as early as possible to update
     * the accumulated system up time precisely
     */
    SYS_TIME_BumpUpAccumulatedSysUpTime();

    while(TRUE)
    {
        SYSFUN_ReceiveEvent(SYS_TIME_EVENT_TIMER|SYSFUN_SYSTEM_EVENT_SW_WATCHDOG,
            SYSFUN_EVENT_WAIT_ANY,
            (local_event==0)?SYSFUN_TIMEOUT_WAIT_FOREVER:SYSFUN_TIMEOUT_NOWAIT,
            &event);

        local_event |= event;
#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
        if(local_event & SYSFUN_SYSTEM_EVENT_SW_WATCHDOG)
        {
       	    SW_WATCHDOG_MGR_ResetTimer(SW_WATCHDOG_SYS_TIME);
            local_event ^= SYSFUN_SYSTEM_EVENT_SW_WATCHDOG;
        }
#endif

        if (local_event & SYS_TIME_EVENT_TIMER)
        {
            SYS_TIME_BumpUpAccumulatedSysUpTime();
            local_event ^= SYS_TIME_EVENT_TIMER;
        }
    }
}
#endif /* end of #if (SYS_CPNT_SYS_TIME_ACCUMULATED_SYSTEM_UP_TIME == TRUE) */
