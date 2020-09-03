/* ==================================================================================
 * FILE NAME: SFLOW_TASK.c
 *
 * ABSTRACT:
 * MODIFICATION HISOTRY:
 *
 * MODIFIER                DATE            DESCRIPTION
 * ---------------------------------------------------------------------------------
 * Joeanne               10-25-2007         First created
 * Nelson Dai            09-13-2009         Porting from vxWorks to Linux platform
 * Nelson Dai            09-25-2009         sFlow over IPv6
 * ---------------------------------------------------------------------------------
 * Copyright(C)            Accton Techonology Corporation 2007
 * ================================================================================*/

#include <string.h>
#include "sflow_mgr.h"
#include "sysfun.h"
#include "sys_bld.h"
#include "l_threadgrp.h"
#include "sflow_proc_comm.h"
//#include "syslog_pmgr.h"

#if (SYS_CPNT_SFLOW == TRUE)

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATION
 */
static void SFLOW_TASK_TaskMain (void);

static BOOL_T SFLOW_TASK_IsTransitionDone = FALSE; /* flag for finishing clear all msg or allocation */
static UI32_T SFLOW_TASK_TaskId;
//static SFLOW_TASK_Lcb_P_T sflow_task_lcb;

/* external variable */
//#define SFLOW_TASK_MAX_MSGQ_LEN     1024

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_TASK_Initiate_System_Resources
 *------------------------------------------------------------------------------
 * PURPOSE  : Initialize MsgQ and SFLOW_MGR
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    :
 *------------------------------------------------------------------------------*/
void SFLOW_TASK_Initiate_System_Resources(void)
{
    SFLOW_TASK_IsTransitionDone = FALSE;
    return;
}   /* end of SFLOW_TASK_Initiate_System_Resources() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_TASK_Create_Tasks
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will create task.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void SFLOW_TASK_Create_Tasks(void)
{
	UI32_T thread_id;

    if (SYSFUN_SpawnThread(SYS_BLD_SFLOW_CSC_THREAD_PRIORITY,
                           SYS_BLD_SFLOW_CSC_SCHED_POLICY,
                           SYS_BLD_SFLOW_CSC_THREAD_NAME,
                           SYS_BLD_TASK_COMMON_STACK_SIZE,
                           SYSFUN_TASK_FP,
                           SFLOW_TASK_TaskMain,
                           NULL,
                           &thread_id) != SYSFUN_OK)
    {
        printf("%s: Create sFlow task fail!!\n", __FUNCTION__);
        return;
    }

	SFLOW_TASK_TaskId = thread_id;
    return;
}   /* end of SFLOW_TASK_Create_Tasks() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_TASK_EnterMasterMode
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will call sflow task into master mode.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void SFLOW_TASK_EnterMasterMode(void)
{
    return;
} /* end of SFLOW_TASK_EnterMasterMode() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_TASK_EnterSlaveMode
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will call set sflow task into slave mode.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void SFLOW_TASK_EnterSlaveMode(void)
{
    return;
} /* end of SFLOW_TASK_EnterSlaveMode() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_TASK_EnterTransitionMode
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will call set sflow task into transition mode.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void SFLOW_TASK_EnterTransitionMode(void)
{
    SYSFUN_TASK_ENTER_TRANSITION_MODE(SFLOW_TASK_IsTransitionDone);
    return;
} /* end of SFLOW_TASK_EnterTransitionMode() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_TASK_SetTransitionMode
 *--------------------------------------------------------------------------
 * PURPOSE  : This function sets the component to temporary transition mode
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------
 */
void  SFLOW_TASK_SetTransitionMode(void)
{
    SFLOW_TASK_IsTransitionDone = FALSE;
    SYSFUN_SendEvent(SFLOW_TASK_TaskId, SFLOW_MGR_EVENT_ENTER_TRANSITION);
    return;
} /* end of SFLOW_TASK_SetTransitionMode() */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_TASK_StartTimerEvent
 * ------------------------------------------------------------------------
 * FUNCTION : Service routine to start the periodic timer event for the sFlow
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : The periodic timer event is sent to the task which creates
 *            the timer. Hence we have to set the timer by the sFlow task itself.
 * ------------------------------------------------------------------------
 */
static void SFLOW_TASK_StartTimerEvent(void)
{
    void* timer_id = SYSFUN_PeriodicTimer_Create();
    SYSFUN_PeriodicTimer_Start(timer_id, SFLOW_TIMER_TICKS2SEC, SFLOW_MGR_EVENT_TIMER);
    return;
} /* End of SFLOW_TASK_StartTimerEvent */

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_TASK_TaskMain
 *------------------------------------------------------------------------------
 * PURPOSE  : This function receive event and message from callback function.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : Send a message to task
 *------------------------------------------------------------------------------*/
static void SFLOW_TASK_TaskMain(void)
{
    L_THREADGRP_Handle_T tg_handle = SFLOW_PROC_COMM_GetSflowTGHandle();
    UI32_T events, all_events;
    UI32_T member_id, timeout, current_mode;
    UI32_T receiver_index = 0;
    UI32_T ifindex = 0;
    UI32_T instance_id = 0;

    SFLOW_TASK_StartTimerEvent();

    all_events = SFLOW_MGR_EVENT_NONE;

    /* join the thread group of CSCGroup1
     */
    if (L_THREADGRP_Join(tg_handle, SYS_BLD_SFLOW_CSC_THREAD_PRIORITY, &member_id) == FALSE)
    {
        printf("%s: L_THREADGRP_Join fail.\n", __FUNCTION__);
        return;
    }

    while (1)
    {
        /* Three events will be handled: 1.Timer, 2.Enter Transition, 3.Timeout */
        if (all_events)
        {
            timeout = (UI32_T)SYSFUN_TIMEOUT_NOWAIT;
        }
        else
        {
            timeout = (UI32_T)SYSFUN_TIMEOUT_WAIT_FOREVER;
        }

        SYSFUN_ReceiveEvent(SFLOW_MGR_EVENT_ALL,
                            SYSFUN_EVENT_WAIT_ANY,
                            timeout,
                            &events);

        all_events |= events;
        all_events &= (SFLOW_MGR_EVENT_TIMER   |
                       SFLOW_MGR_EVENT_TIMEOUT |
                       SFLOW_MGR_EVENT_ENTER_TRANSITION);

        /* Get the system operation mode from MGR */
        current_mode = SFLOW_MGR_GetOperationMode();

        if (current_mode == SYS_TYPE_STACKING_TRANSITION_MODE)
        {
            /* Set flag to Transition done, make STKCTRL task turn-back */
            if (all_events & SFLOW_MGR_EVENT_ENTER_TRANSITION)
            {
                SFLOW_TASK_IsTransitionDone = TRUE;
            }
            all_events = SFLOW_MGR_EVENT_NONE;

//            SYSLOG_PMGR_NotifyStaTplgChanged();
//            SYSFUN_Sleep(200);
            continue;
        }
        else if (current_mode == SYS_TYPE_STACKING_SLAVE_MODE)
        {
            all_events = SFLOW_MGR_EVENT_NONE;
            SYSFUN_Sleep(200);
            continue;
        }
        else /* Master mode */
        {
	        /* request thread group execution permission
	         */
	        if (L_THREADGRP_Execution_Request(tg_handle, member_id) != TRUE)
	        {
	            SYSFUN_Debug_Printf("%s: L_THREADGRP_Execution_Request fail.\n", __FUNCTION__);
	        }
            if (all_events)
            {
                /* 1. Handle Timer Event */
                if (all_events & SFLOW_MGR_EVENT_TIMER)
                {
                    for (receiver_index = SFLOW_MGR_MIN_RECEIVER_INDEX; receiver_index <= SFLOW_MGR_MAX_RECEIVER_INDEX; ++receiver_index)
                    {
                        SFLOW_MGR_ProcessReceiverTimeoutCountdown(receiver_index);
                    }

                    for (ifindex = SFLOW_MGR_MIN_IFINDEX; ifindex <= SFLOW_MGR_MAX_IFINDEX; ++ifindex)
                    {
                        for (instance_id = SFLOW_MGR_MIN_INSTANCE_ID; instance_id <= SFLOW_MGR_MAX_INSTANCE_ID; ++instance_id)
                        {
                            SFLOW_MGR_ProcessPolling(ifindex, instance_id);
                        }
                    }

                    all_events &= ~SFLOW_MGR_EVENT_TIMER;
                } /* End of timer event*/
            } /* End of if (all_events) */

	        /* release thread group execution permission
	         */
	        if (L_THREADGRP_Execution_Release(tg_handle, member_id) != TRUE)
	        {
	            SYSFUN_Debug_Printf("%s: L_THREADGRP_Execution_Release fail.\n", __FUNCTION__);
	        }
        }
    } /* End of while */

    L_THREADGRP_Leave(tg_handle, member_id);
}


#endif /* #if (SYS_CPNT_SFLOW == TRUE) */

