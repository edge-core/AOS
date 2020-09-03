/* Module Name: SYSLOG_TASK.C
 * Purpose: Initialize the resources and create task for the system log module.
 *
 * Notes:
 *
 * History:
 *    11/14/01       -- Aaron Chuang, Create
 *
 * Copyright(C)      Accton Corporation, 1999 - 2001
 *
 */


/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include "sys_type.h"
#include "sys_bld.h"
#include "sys_cpnt.h"
#include "sysfun.h"
#include "syslog_om.h"
#include "syslog_mgr.h"
#include "backdoor_mgr.h"
#include "l_threadgrp.h"
#include "core_util_proc_comm.h"

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
#include "sw_watchdog_mgr.h"
#endif

/* NAMING CONSTANT DECLARATIONS
 */
#if 0
#define SYSLOG_TASK_EVENT_30_SEC            0x0001L
#define SYSLOG_TASK_EVENT_ENTER_TRANSITION  0x0002L
#define SYSLOG_TASK_TIMER_TICKS_0_5_SEC     50
#endif

/* DATA TYPE DECLARATIONS
 */


/* LOCAL SUBPROGRAM DECLARATIONS
 */


/* STATIC VARIABLE DECLARATIONS
 */
static  UI32_T  syslog_task_id;
static  void*   syslog_timer_id; /* periodic timer id */
#if 0 /* old un-stacking code */
static  SYS_TYPE_Stacking_Mode_T    syslog_task_stacking_mode;
#endif

/* for stacking using */
static  BOOL_T  is_transition_done;


#if (SYS_CPNT_REMOTELOG == TRUE)
static BOOL_T   not_check_rif_cnt = FALSE;
static UI32_T   up_rif_cnt = 0;
#endif

/* MACRO FUNCTIONS DECLARACTION
 */


/* EXPORTED SUBPROGRAM BODIES
 */


/* FUNCTION NAME: SYSLOG_TASK_Initiate_System_Resources
 * PURPOSE: This function is used to initialize the system log module.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure, system error.
 * NOTES:   Initialize the system log module.
 *
 */
BOOL_T SYSLOG_TASK_Initiate_System_Resources(void)
{
#if 0 /* old un-stacking code */
   syslog_task_stacking_mode = SYS_TYPE_STACKING_TRANSITION_MODE;
#endif
    is_transition_done = FALSE;
    return(TRUE);
} /* SYSLOG_TASK_Initiate_System_Resources() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - SYSLOG_TASK_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void SYSLOG_TASK_Create_InterCSC_Relation(void)
{
    return;
} /* end of SYSLOG_TASK_Create_InterCSC_Relation */

/* FUNCTION NAME: SYSLOG_TASK_Main
 * PURPOSE: SYSLOG task body.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:   1. Set the periodic timer, interval value is 30 seconds.
 *          2. When following events happen, the UC flash DB will be logged to file system.
 *             -- UC flash DB is full.
 *             -- Excess 12 hours not log to file system.
 *
 */
void SYSLOG_TASK_Main(void)
{
    /* LOCAL VARIABLES DEFINITION */
    static  UI32_T  ui_event_var;      /* keep track events not be handled. */
    UI32_T  ui_rcv_event;
    UI32_T  timeout;
    UI32_T  member_id;
    UI32_T  ui_return_value;
    SYS_TYPE_Stacking_Mode_T current_state;
    L_THREADGRP_Handle_T tg_handle = CORE_UTIL_PROC_COMM_GetUTILITY_GROUPTGHandle();

#if (SYS_CPNT_REMOTELOG == TRUE)
    UI32_T  spanning_tree_state;
    BOOL_T  remotelog_queue_is_empty;
    void    *sta_timer_id;
    BOOL_T is_need_sleep = FALSE;
#endif

    /* BODY */
    ui_event_var = 0;   /* clear tracking event variable */
    syslog_timer_id = SYSFUN_PeriodicTimer_Create();
    SYSFUN_PeriodicTimer_Start(syslog_timer_id, SYS_BLD_SYSLOG_LOOKUP_INTERVAL_TICKS, SYSLOG_ADPT_EVENT_30_SEC);

#if (SYS_CPNT_REMOTELOG == TRUE)
	sta_timer_id = SYSFUN_PeriodicTimer_Create();
#endif
    /* join the thread group
     */
    if(L_THREADGRP_Join(tg_handle, SYS_BLD_UTILITY_GROUP_MGR_THREAD_PRIORITY, &member_id)==FALSE)
    {
        printf("%s: L_THREADGRP_Join fail.\n", __FUNCTION__);
        return;
    }

    while(1)
    {
    	/*	Check timer event and message event	*/
        if (ui_event_var != 0)
        {    /* There is some message in message queue  */
            timeout = SYSFUN_TIMEOUT_NOWAIT;
        }
        else
            timeout = SYSFUN_TIMEOUT_WAIT_FOREVER;

        if ((ui_return_value=SYSFUN_ReceiveEvent(
                                                 SYSLOG_ADPT_EVENT_30_SEC            | 
#if (SYS_CPNT_REMOTELOG == TRUE)                                                
                                                 SYSLOG_ADPT_EVENT_STA_STATE_CHANGED |
                                                 SYSLOG_ADPT_EVENT_RIF_UP            |
                                                 SYSLOG_ADPT_EVENT_RIF_DOWN          |
                                                 SYSLOG_ADPT_EVENT_35_SEC            | 
                                                 SYSLOG_ADPT_EVENT_TRAP_ARRIVAL      |
#endif                                                
#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
                                                 SYSFUN_SYSTEM_EVENT_SW_WATCHDOG     |
#endif
                                                 SYSLOG_ADPT_EVENT_ENTER_TRANSITION,
                                                 SYSFUN_EVENT_WAIT_ANY,
                                                 timeout,
                                                 &ui_rcv_event))!=SYSFUN_OK)
        {
        	if (ui_return_value != SYSFUN_RESULT_NO_EVENT_SET)
        	{
        		/*	Log to system : unexpect return value	*/
        		;
			}
        }

        ui_event_var |= ui_rcv_event;

        if ( ui_event_var == 0)
            continue;

        /* Get the system operation mode from MGR */
    	current_state =  SYSLOG_MGR_GetOperationMode();

        if (current_state == SYS_TYPE_STACKING_TRANSITION_MODE)
        {
	        /* Set flag to Transition done, make STKCTRL task turn-back */
	        if (ui_event_var & SYSLOG_ADPT_EVENT_ENTER_TRANSITION )
	        	is_transition_done = TRUE;

			ui_event_var = 0;
			continue;
		}

		/* request thread group execution permission
         */
        if(L_THREADGRP_Execution_Request(tg_handle, member_id)!=TRUE)
        {
            SYSFUN_Debug_Printf("%s: L_THREADGRP_Execution_Request fail.\n", __FUNCTION__);
        }

        /*  Check timer event occurs ?  */
        if (ui_event_var & SYSLOG_ADPT_EVENT_30_SEC)
        {
            SYSLOG_MGR_TimerExpiry();
            ui_event_var ^= SYSLOG_ADPT_EVENT_30_SEC;
        }   /*  End of timer event handling */

#if (SYS_CPNT_REMOTELOG == TRUE)
        SYSLOG_MGR_GetStaState(&spanning_tree_state);
        if (ui_event_var & SYSLOG_ADPT_EVENT_STA_STATE_CHANGED)
        {
            if (spanning_tree_state == SYSLOG_ADPT_STA_BECOME_STABLED_STATE)
            {   
				SYSFUN_PeriodicTimer_Start(sta_timer_id, 35*SYS_BLD_TICKS_PER_SECOND, SYSLOG_ADPT_EVENT_35_SEC);
            }
            ui_event_var ^= SYSLOG_ADPT_EVENT_STA_STATE_CHANGED;
        }
		if (ui_event_var & SYSLOG_ADPT_EVENT_35_SEC)
		{
			SYSFUN_PeriodicTimer_Stop(sta_timer_id);
			spanning_tree_state = SYSLOG_ADPT_STA_STABLED_STATE;
			SYSLOG_MGR_SetStaState(spanning_tree_state);
			ui_event_var ^= SYSLOG_ADPT_EVENT_35_SEC;
		}
        if (ui_event_var & SYSLOG_ADPT_EVENT_RIF_DOWN)
        {
            up_rif_cnt--;
            ui_event_var ^= SYSLOG_ADPT_EVENT_RIF_DOWN;
        }
        if (ui_event_var & SYSLOG_ADPT_EVENT_RIF_UP)
        {
            up_rif_cnt++;
            ui_event_var ^= SYSLOG_ADPT_EVENT_RIF_UP;
        }
        remotelog_queue_is_empty = SYSLOG_MGR_IsQueueEmpty();
        if ((ui_event_var & SYSLOG_ADPT_EVENT_TRAP_ARRIVAL) ||
            (remotelog_queue_is_empty == FALSE))
        {
            /* Note: The trap messages cant be sent out only if the netwrok interface is available and
             * spanning tree stablized.
             */
            if (not_check_rif_cnt)
            {
            	if (spanning_tree_state == SYSLOG_ADPT_STA_STABLED_STATE)
                {
	                SYSLOG_MGR_HandleTrapQueue();
	                ui_event_var ^= SYSLOG_ADPT_EVENT_TRAP_ARRIVAL;
                }
                else
                {
                    is_need_sleep = TRUE;
                }
            }
            else
            {
                if ((spanning_tree_state == SYSLOG_ADPT_STA_STABLED_STATE) && (up_rif_cnt > 0))
                {
                    SYSLOG_MGR_HandleTrapQueue();
                	ui_event_var ^= SYSLOG_ADPT_EVENT_TRAP_ARRIVAL;
                }
                else
                {
                    is_need_sleep = TRUE;
                }
            }
        }
#endif /* #if (SYS_CPNT_REMOTELOG == TRUE) */

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
        if (ui_event_var & SYSFUN_SYSTEM_EVENT_SW_WATCHDOG)
        {
       	    SW_WATCHDOG_MGR_ResetTimer(SW_WATCHDOG_UTILITY_SYSLOG);
            ui_event_var ^= SYSFUN_SYSTEM_EVENT_SW_WATCHDOG;
        }
#endif

        /* release thread group execution permission
         */
        if(L_THREADGRP_Execution_Release(tg_handle, member_id)!=TRUE)
        {
            SYSFUN_Debug_Printf("%s: L_THREADGRP_Execution_Release fail.\n", __FUNCTION__);
        }

#if (SYS_CPNT_REMOTELOG == TRUE)
        if (is_need_sleep == TRUE)
        {
            /* Add delay time or lower priority task cannot wake up.
             */
            SYSFUN_Sleep(30);

            is_need_sleep = FALSE;
        }
#endif /* #if (SYS_CPNT_REMOTELOG == TRUE) */

    }   /*  End of while(1) */
}/* End of SYSLOG_TASK_Main */


/* FUNCTION NAME: SYSLOG_TASK_Create_Tasks
 * PURPOSE: Create and start SYSLOG task.
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:
 *
 */
BOOL_T SYSLOG_TASK_Create_Tasks(void)
{
    if( SYSFUN_SpawnThread(   SYS_BLD_SYSLOG_THREAD_PRIORITY,
                               SYS_BLD_SYSLOG_THREAD_SCHED_POLICY,
                               SYS_BLD_SYSLOG_CSC_THREAD_NAME,
                               SYS_BLD_TASK_COMMON_STACK_SIZE,
                               SYSFUN_TASK_NO_FP,
                               SYSLOG_TASK_Main,
                               0,
                               &syslog_task_id) != SYSFUN_OK )
    {
        perror("Syslog Task Spawn error");
        return FALSE;
    }

#if (SYS_CPNT_REMOTELOG == TRUE)
    SYSLOG_MGR_SetTaskId(syslog_task_id);
#endif

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
    SW_WATCHDOG_MGR_RegisterMonitorThread(SW_WATCHDOG_UTILITY_SYSLOG, syslog_task_id, SYS_ADPT_SYSLOG_SW_WATCHDOG_TIMER);
#endif

    return TRUE;
}/* End of SYSLOG_TASK_Create_Tasks */


/* FUNCTION NAME: SYSLOG_TASK_EnterTransitionMode
 * PURPOSE: The function enter transition mode and free all SYSLOG resources
 *          and reset database to factory default.
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:   1. The function MUST be called before calling the
 *             SYSLOG_TASK_EnterMasterMode function.
 */
BOOL_T SYSLOG_TASK_EnterTransitionMode (void)
{
	SYSFUN_TASK_ENTER_TRANSITION_MODE(is_transition_done);
#if 0 /* old un-stacking code */
    /* Need to set to FALSE when re-stacking */
    syslog_task_stacking_mode = SYS_TYPE_STACKING_TRANSITION_MODE;

    if (SYSLOG_MGR_LoadDefaultOM() != TRUE)
    {
        return FALSE;
    }
#endif
    return TRUE;
} /* End of SYSLOG_TASK_EnterTransitionMode */


/* FUNCTION NAME: SYSLOG_TASK_EnterMasterMode
 * PURPOSE: The function enter master mode.
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:
 */
BOOL_T SYSLOG_TASK_EnterMasterMode(void)
{
#if 0 /* old un-stacking code */
    syslog_task_stacking_mode = SYS_TYPE_STACKING_MASTER_MODE;
#endif
    return TRUE;
} /* End of SYSLOG_TASK_EnterMasterMode */


/* FUNCTION NAME: SYSLOG_TASK_EnterSlaveMode
 * PURPOSE: The function enter slave mode.
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:
 */
BOOL_T SYSLOG_TASK_EnterSlaveMode(void)
{
#if 0 /* old un-stacking code */
    syslog_task_stacking_mode = SYS_TYPE_STACKING_SLAVE_MODE;
#endif
    return TRUE;
} /* End of SYSLOG_TASK_EnterMasterMode */


/* FUNCTION NAME: SYSLOG_TASK_SetTransitionMode
 * PURPOSE: The function set transition mode.
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:
 */
BOOL_T SYSLOG_TASK_SetTransitionMode (void)
{
    is_transition_done = FALSE;
    SYSFUN_SendEvent (syslog_task_id, SYSLOG_ADPT_EVENT_ENTER_TRANSITION);
    return TRUE;
} /* End of SYSLOG_TASK_SetTransitionMode */


