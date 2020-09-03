/* Module Name: PING_TASK.C
 * Purpose:
 *      This component implements the standard Ping-MIB functionality (rfc2925).
 *
 * Notes:
 *      1. This design references the TRACEROUTE module.
 *
 * History:
 *      Date        --  Modifier,   Reason
 *      2007/3      --  peter_yu    Create to support partial standard Ping-MIB.
 *
 * Copyright(C)      Accton Corporation, 2007
 */


/* INCLUDE FILE DECLARATIONS
 */
#include <stdlib.h>
#include "sys_bld.h"
#include "sys_dflt.h"
#include "sys_type.h"
#include "sys_adpt.h"
#include "sysfun.h"
#include "l_threadgrp.h"
#include "ping_mgr.h"
#include "ping_task.h"
#include "app_protocol_proc_comm.h"
#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
#include "sw_watchdog_mgr.h"
#endif

/* NAMING CONSTANT DECLARATIONS
 */
#define PING_TASK_EVENT_ENTER_TRANSITION      0x0001L
#define PING_TASK_TIMER_EVENT                 0x0002L
#define PING_TASK_START_TIMER_EVENT           0x0004L
#define PING_TASK_PROVISION_EVENT             0x0008L

/* DATA TYPE DECLARATIONS
 */

typedef struct  PING_TASK_Lcb_S
{
    BOOL_T      init_flag;                  /* TRUE: PING_TASK initialized */
    UI32_T      ping_task_id;               /* PING_TASK ID */
}  PING_TASK_Lcb_T, *PING_TASK_Lcb_P_T;

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void PING_TASK_TaskMain (void);


/* STATIC VARIABLE DECLARATIONS
 */
static  PING_TASK_Lcb_P_T     ping_task_lcb;
static  BOOL_T                ping_task_is_transition_done = FALSE;


/* EXPORTED SUBPROGRAM BODIES
 */

/* FUNCTION NAME - PING_TASK_Initiate_System_Resources
 * PURPOSE  : This function allocates and initiates the system resource for
 *            PING database.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 */
void PING_TASK_Initiate_System_Resources(void)
{
    ping_task_lcb = (PING_TASK_Lcb_P_T) malloc (sizeof(PING_TASK_Lcb_T));

    if (!ping_task_lcb)
    {
        return;
    }

    ping_task_lcb->init_flag   = TRUE;

    ping_task_is_transition_done = FALSE;

    return;
} /* end of PING_TASK_Initiate_System_Resources() */

/* FUNCTION NAME - PING_TASK_EnterMasterMode
 * PURPOSE  : This function will configured PING to enter master mode
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 */
void PING_TASK_EnterMasterMode(void)
{

    return;
} /* end of PING_TASK_EnterMasterMode() */


/* FUNCTION NAME - PING_TASK_EnterTransitionMode
 * PURPOSE  : This function will configured PING to enter transition mode
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 */
void PING_TASK_EnterTransitionMode(void)
{
    SYSFUN_TASK_ENTER_TRANSITION_MODE(ping_task_is_transition_done);
     return;
} /* end of PING_TASK_EnterTransitionMode() */


/* FUNCTION NAME - PING_TASK_EnterSlaveMode
 * PURPOSE  : This function will configured PING to enter master mode
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 */
void PING_TASK_EnterSlaveMode(void)
{
    return;
} /* end of PING_TASK_EnterSlaveMode() */


/* FUNCTION NAME - PING_TASK_SetTransitionMode
 * PURPOSE  : This function sets the component to temporary transition mode
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 */
void  PING_TASK_SetTransitionMode(void)
{
    ping_task_is_transition_done = FALSE;
    SYSFUN_SendEvent (ping_task_lcb->ping_task_id, PING_TASK_EVENT_ENTER_TRANSITION);

} /* end of PING_TASK_SetTransitionMode() */


/* FUNCTION NAME - PING_TASK_CreateTask
 * PURPOSE  : This function will create PING task.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 */
void PING_TASK_CreateTask(void)
{
    /* BODY */

    if (SYSFUN_SpawnThread(SYS_BLD_PING_TASK_PRIORITY,
                           SYS_BLD_APP_PROTOCOL_GROUP_MGR_SCHED_POLICY,
                           SYS_BLD_PING_TASK,
                           SYS_BLD_TASK_COMMON_STACK_SIZE,
                           SYSFUN_TASK_NO_FP,
                           PING_TASK_TaskMain,
                           NULL,
                           &ping_task_lcb->ping_task_id) != SYSFUN_OK)
    {

    } /* end of if */

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
    SW_WATCHDOG_MGR_RegisterMonitorThread ( SW_WATCHDOG_PING, ping_task_lcb->ping_task_id, SYS_ADPT_PING_SW_WATCHDOG_TIMER);
#endif

    return;

} /* end of PING_TASK_CreateTask() */


/* FUNCTION NAME - PING_TASK_ProvisionComplete
 * PURPOSE  : This function will create socket after provision complete
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 */
void PING_TASK_ProvisionComplete(void)
{
    SYSFUN_SendEvent (ping_task_lcb->ping_task_id, PING_TASK_PROVISION_EVENT);

} /* end of PING_TASK_ProvisionComplete() */


/* FUNCTION NAME - PING_TASK_PeriodicTimerStart_Callback
 * PURPOSE  : This function will send event to ping task in order to 
 *            start periodic timer
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 */
void PING_TASK_PeriodicTimerStart_Callback(void)
{
    SYSFUN_SendEvent (ping_task_lcb->ping_task_id, PING_TASK_START_TIMER_EVENT);
}


/* LOCAL SUBPROGRAM BODIES
 */
/* FUNCTION NAME - PING_TASK_TaskMain
 * PURPOSE  : Ping task body
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 */
static void PING_TASK_TaskMain (void)
{
    L_THREADGRP_Handle_T tg_handle = APP_PROTOCOL_PROC_COMM_GetAppProtocolGroupHandle();
    UI32_T  member_id, wait_event,event, all_event = 0;
    void    *timer_id;
    I32_T   count_down = 0;
    BOOL_T  is_timer_start = FALSE;

    if (!ping_task_lcb->init_flag)
        return;

    if(L_THREADGRP_Join(tg_handle, SYS_BLD_APP_PROTOCOL_GROUP_MGR_THREAD_PRIORITY, &member_id)==FALSE)
    {
        printf("%s: L_THREADGRP_Join fail.\n", __FUNCTION__);
        return;
    }

    timer_id = SYSFUN_PeriodicTimer_Create();

    wait_event = (PING_TASK_EVENT_ENTER_TRANSITION |
                  PING_TASK_TIMER_EVENT |
                  PING_TASK_START_TIMER_EVENT |
#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
                  SYSFUN_SYSTEM_EVENT_SW_WATCHDOG |
#endif
                  PING_TASK_PROVISION_EVENT);
    while(1)
    {
        SYSFUN_ReceiveEvent (wait_event, SYSFUN_EVENT_WAIT_ANY, SYSFUN_TIMEOUT_WAIT_FOREVER, &event);
        all_event |= event;

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
        if (all_event & SYSFUN_SYSTEM_EVENT_SW_WATCHDOG)
        {
       	    SW_WATCHDOG_MGR_ResetTimer(SW_WATCHDOG_PING);
            all_event ^= SYSFUN_SYSTEM_EVENT_SW_WATCHDOG;
        }
#endif

        if (all_event & PING_TASK_EVENT_ENTER_TRANSITION)
        {
            all_event ^= PING_TASK_EVENT_ENTER_TRANSITION;
            PING_MGR_CloseSocket();
            ping_task_is_transition_done = TRUE;
            continue;
        }

        if (all_event & PING_TASK_PROVISION_EVENT)
        {
            all_event ^= PING_TASK_PROVISION_EVENT;
            SYSFUN_Sleep(30 *SYS_BLD_TICKS_PER_SECOND);
            PING_MGR_CreateSocket();
        }

        if (all_event & PING_TASK_TIMER_EVENT)
        {
            all_event ^= PING_TASK_TIMER_EVENT;

            if(L_THREADGRP_Execution_Request(tg_handle, member_id)!=TRUE)
                printf("%s: L_THREADGRP_Execution_Request fail.\r\n", __FUNCTION__);

            if (PING_MGR_TriggerPing() != PING_TYPE_NO_MORE_PROBE_TO_SEND)
                count_down = 2*SYS_DFLT_PING_CTL_TIME_OUT/SYS_BLD_TICKS_PER_SECOND;

            if (L_THREADGRP_Execution_Release(tg_handle, member_id) != TRUE)
                printf("\r\n%s: L_THREADGRP_Execution_Release fail.\r\n", __FUNCTION__);

            if (--count_down <= 0)
            {
                SYSFUN_PeriodicTimer_Stop(timer_id);
                is_timer_start = FALSE;
            }
        }

        if (all_event & PING_TASK_START_TIMER_EVENT)
        {
            all_event ^= PING_TASK_START_TIMER_EVENT;
            if (!is_timer_start)
            {
                if (SYSFUN_PeriodicTimer_Start(timer_id, SYS_BLD_TICKS_PER_SECOND, PING_TASK_TIMER_EVENT))
                    is_timer_start = TRUE;
            }
        }
    }

    SYSFUN_PeriodicTimer_Destroy(timer_id);
    L_THREADGRP_Leave(tg_handle, member_id);

    return;
}

