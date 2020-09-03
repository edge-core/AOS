/* Module Name: SYSLOG_TASK.C
 * Purpose: Initialize the resources and create task for the smtp module.
 *
 * Notes:
 *
 * History:
 *    01/22/03       -- Ricky Lin, Create
 *
 * Copyright(C)      Accton Corporation, 1999 - 2001
 *
 */


/* INCLUDE FILE DECLARATIONS
 */
#include "stdio.h"
#include "sys_type.h"
#include "sys_bld.h"
#include "sysfun.h"
#include "smtp_mgr.h"
#include "smtp_task.h"
#include "l_threadgrp.h"

#if (SYS_CPNT_EH == TRUE)
#include "sys_module.h"
#include "syslog_type.h"
#include "eh_type.h"
#include "eh_mgr.h"
#endif

/* NAMING CONSTANT DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */


/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void SMTP_TASK_Main(void);

/* STATIC VARIABLE DECLARATIONS
 */
static  UI32_T  smtp_task_id;
static  void*   smtp_timer_id;

/* for stacking using */
static  BOOL_T  is_transition_done;

/* MACRO FUNCTIONS DECLARACTION
 */


/* EXPORTED SUBPROGRAM BODIES
 */


/* FUNCTION NAME: SMTP_TASK_Initiate_System_Resources
 * PURPOSE: This function is used to initialize the smtp task.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:   None.
 *
 */
void SMTP_TASK_Initiate_System_Resources(void)
{
    is_transition_done = FALSE;
    return;
} /* End of SMTP_TASK_Initiate_System_Resources() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - SMTP_TASK_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void SMTP_TASK_Create_InterCSC_Relation(void)
{
    return;
} /* end of SMTP_TASK_Create_InterCSC_Relation */

/* FUNCTION NAME: SMTP_TASK_Create_Task
 * PURPOSE: This function will create and start SMTP task.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:   None.
 *
 */
void SMTP_TASK_Create_Task(void)
{
    if(SYSFUN_SpawnThread(SYS_BLD_SMTP_CSC_THREAD_PRIORITY,
                          SYS_BLD_SMTP_CSC_THREAD_SCHED_POLICY,
                          SYS_BLD_SMTP_CSC_THREAD_NAME,
                          SYS_BLD_TASK_COMMON_STACK_SIZE,
                          SYSFUN_TASK_NO_FP,
                          SMTP_TASK_Main,
                          NULL,
                          &smtp_task_id)!=SYSFUN_OK)
    {
        printf("%s:SYSFUN_SpawnThread fail.\n", __FUNCTION__);
    }

    SMTP_OM_SetTaskId(smtp_task_id);
}/* End of SMTP_TASK_Create_Task() */


/* FUNCTION NAME: SMTP_TASK_EnterTransitionMode
 * PURPOSE: The function will enter transition mode.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:   1. The function MUST be called before calling the
 *             SMTP_TASK_EnterMasterMode function.
 */
void SMTP_TASK_EnterTransitionMode(void)
{
    SYSFUN_TASK_ENTER_TRANSITION_MODE(is_transition_done);
    return;
} /* End of SMTP_TASK_EnterTransitionMode() */

/* FUNCTION NAME: SMTP_TASK_SetTransitionMode
 * PURPOSE: The function will set transition mode.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:   None.
 *
 */
void SMTP_TASK_SetTransitionMode(void)
{
    is_transition_done = FALSE;
    SYSFUN_SendEvent (smtp_task_id, SMTP_TASK_EVENT_ENTER_TRANSITION);
    return;
} /* End of SMTP_TASK_SetTransitionMode() */


/* LOCAL SUBPROGRAM BODIES
 */

/* FUNCTION NAME: SMTP_TASK_Main
 * PURPOSE: This function is SMTP task body.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:   1.Set the periodic timer, interval value is SYS_BLD_SMTP_LOOKUP_INTERVAL_TICKS.
 *          2.When periodic timer event occurs,call SMTP_MGR_Timer_Handler()
 *
 */
static void SMTP_TASK_Main(void)
{
    UI32_T                      smtp_event_var;      /* keep track events not be handled. */
    UI32_T                      smtp_rcv_event;
    UI32_T                      timeout;
    UI32_T                      smtp_return_value;
    SYS_TYPE_Stacking_Mode_T    current_state;

    smtp_event_var = 0;   /* clear tracking event variable */
    smtp_timer_id = SYSFUN_PeriodicTimer_Create();
    SYSFUN_PeriodicTimer_Start(smtp_timer_id, SYS_BLD_SMTP_LOOKUP_INTERVAL_TICKS, SMTP_TASK_EVENT_PERIODIC_TIMER);

    while (1)
    {
        if (smtp_event_var != 0)
        {
            timeout = SYSFUN_TIMEOUT_NOWAIT;
        }
        else
        {
            timeout = SYSFUN_TIMEOUT_WAIT_FOREVER;
        }

        if ((smtp_return_value = SYSFUN_ReceiveEvent (
            SMTP_TASK_EVENT_PERIODIC_TIMER | 
            SMTP_TASK_EVENT_ENTER_TRANSITION |
            SMTP_TASK_EVENT_SEND_MAIL,
            SYSFUN_EVENT_WAIT_ANY,
            timeout,
            &smtp_rcv_event))!=SYSFUN_OK)
        {
            if (smtp_return_value != SYSFUN_RESULT_NO_EVENT_SET)
            {
                /* Log to system : unexpect return value
                 */
                ;
            }
        }

        smtp_event_var |= smtp_rcv_event;

        if (smtp_event_var == 0)
        {
            continue;
        }

        /* Get the system operation mode from MGR
         */
        current_state =  SMTP_MGR_GetOperationMode();

        if (current_state == SYS_TYPE_STACKING_TRANSITION_MODE)
        {
            /* Set flag to Transition don
             */
            if (smtp_event_var & SMTP_TASK_EVENT_ENTER_TRANSITION)
            {
                is_transition_done = TRUE;
            }

            smtp_event_var = 0;
            continue;
        }

        if (smtp_event_var & SMTP_TASK_EVENT_SEND_MAIL)
        {
            SMTP_MGR_RETURN_T ret;

            ret = SMTP_MGR_HandleTrapQueue();

            /* OM queue is empty or error(ex: server connect fail).
             */
            if (ret != SMTP_MGR_RETURN_SUCCESS)
            {
                smtp_event_var ^= SMTP_TASK_EVENT_SEND_MAIL;
            }
        }

        if (smtp_event_var & SMTP_TASK_EVENT_PERIODIC_TIMER)
        {
            smtp_event_var ^= SMTP_TASK_EVENT_PERIODIC_TIMER;
            smtp_event_var |= SMTP_TASK_EVENT_SEND_MAIL;
        }
    }   /*  End of while(1) */
}/* End of SMTP_TASK_Main() */
