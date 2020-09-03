/* MODULE NAME: WEBAUTH_TASK.C 
 * PURPOSE: 
 *      Definitions for WEBAUTH task 
 * NOTES:
 *
 *
 * HISTORY:
 *    01/29/07 --  Rich Lee, Create
 * 
 * Copyright(C)      Accton Corporation, 2007 
 */

/* INCLUDE FILE DECLARATION 
 */
#include <stdio.h>
#include <string.h>
#include "sys_type.h"
#include "sys_bld.h"
#include "sysfun.h"
#include "sys_module.h"
#include "l_hold_timer.h"

#include "webauth_type.h"
#include "webauth_mgr.h"
#include "webauth_om.h"
#include "webauth_init.h"
#include "webauth_task.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */
 
/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static  void    WEBAUTH_TASK_Main ();
static  void    WEBAUTH_TASK_StartTimerEvent(void);

/* STATIC VARIABLE DEFINITIONS
 */ 

static  BOOL_T  webauth_task_transitiondone;  /* flag for finishing clear all msg or allocation */
static  UI32_T  webauth_task_id;
static  void  *webauth_task_timerid;
/* EXPORTED SUBPROGRAM BODIES
 */

/* FUNCTION NAME:  WEBAUTH_TASK_Create_Task
 * PURPOSE: This function creates the WEBAUTH task for network
 * INPUT:   none
 * OUTPUT:  none
 * RETURN:  none
 * NOTES:   none
 */
void WEBAUTH_TASK_Create_Task (void)
{
    UI32_T debug_flag = WEBAUTH_MGR_GetDebugFlag();   
     
    if (SYSFUN_SpawnThread (SYS_BLD_WEBAUTH_CSC_THREAD_PRIORITY,
                        	SYS_BLD_WEBAUTH_CSC_SCHED_POLICY,
                            SYS_BLD_WEBAUTH_CSC_THREAD_NAME,
                        	SYS_BLD_TASK_COMMON_STACK_SIZE,
                        	SYSFUN_TASK_NO_FP,
                        	WEBAUTH_TASK_Main,
                        	NULL,
                        	&webauth_task_id) != SYSFUN_OK)
    {
        
        if(debug_flag)
            printf("\n webauth create task fail\n");
      
    }

} /* end of WEBAUTH_TASK_CreateTask () */

/* FUNCTION NAME:  WEBAUTH_TASK_Initiate_System_Resources
 * PURPOSE: init task parameters
 * INPUT:   none
 * OUTPUT:  none
 * RETURN:  none
 * NOTES:   none
 */
void WEBAUTH_TASK_Initiate_System_Resources(void)
{   
    webauth_task_transitiondone  =   FALSE;
    webauth_task_id = 0;
    webauth_task_timerid = NULL;
} /* End of WEBAUTH_TASK_Initiate_System_Resources() */

/* FUNCTION NAME:  WEBAUTH_TASK_EnterMasterMode
 * PURPOSE: would be called from webauth_init.c 
 * INPUT:   none
 * OUTPUT:  none
 * RETURN:  none
 * NOTES:   none
 */
void WEBAUTH_TASK_EnterMasterMode(void)
{
    
} /* End of WEBAUTH_TASK_EnterMasterMode */

/* FUNCTION NAME:  WEBAUTH_TASK_EnterTransitionMode
 * PURPOSE: This function forces this subsystem enter the Transition 
 *          Operation mode.
 * INPUT:   none.
 * OUTPUT:  none.
 * RETURN:  none.
 * NOTES:   none.
 */
void WEBAUTH_TASK_EnterTransitionMode(void)
{
   
} /*End of WEBAUTH_TASK_EnterTransitionMode */

/* FUNCTION NAME:  WEBAUTH_TASK_EnterSlaveMode
 * PURPOSE: This function forces this subsystem enter the Slave Operation mode.
 * INPUT:   none.
 * OUTPUT:  none.
 * RETURN:  none.
 * NOTES:
 */
void WEBAUTH_TASK_EnterSlaveMode(void)
{

} /* End of WEBAUTH_TASK_EnterSlaveMode */

/* FUNCTION NAME : WEBAUTH_TASK_SetTransitionMode
 * PURPOSE: Set transition mode.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETURN:  None.
 * NOTES:   None.
 */
void WEBAUTH_TASK_SetTransitionMode(void)
{    
    webauth_task_transitiondone = FALSE;
    SYSFUN_SendEvent (webauth_task_id, WEBAUTH_TYPE_EVENT_ENTER_TRANSITION);
    
} /* End of WEBAUTH_TASK_SetTransitionMode */

/* FUNCTION NAME:  WEBAUTH_TASK_ProvisionComplete
 * PURPOSE: This function will tell webauth that provision is completed
 * INPUT:   None
 * OUTPUT:  None
 * RETURN:  None
 * NOTE:    None
 */
void WEBAUTH_TASK_ProvisionComplete(void)
{
}

/* LOCAL SUBPROGRAM BODIES
 */

/* FUNCTION NAME:  WEBAUTH_TASK_Main
 * PURPOSE: start timer routine
 * INPUT:   none.
 * OUTPUT:  None
 * RETURN:  None
 * NOTE:    None
 */

static void WEBAUTH_TASK_Main ()
{
    UI32_T                          events, all_events;
    UI32_T                          timeout;
    UI32_T                          current_mode;


    WEBAUTH_TASK_StartTimerEvent();

    all_events = WEBAUTH_TYPE_EVENT_NONE;

    while(1)
    {
        
        if (all_events)
            timeout = (UI32_T)SYSFUN_TIMEOUT_NOWAIT;
        else
            timeout = (UI32_T)SYSFUN_TIMEOUT_WAIT_FOREVER;

        SYSFUN_ReceiveEvent(WEBAUTH_TYPE_EVENT_ALL,
                            SYSFUN_EVENT_WAIT_ANY,
                            timeout,
                            &events);

        all_events |= events;

        /* Get the system operation mode from MGR 
         */
        current_mode = WEBAUTH_MGR_GetOperationMode();

        if (current_mode == SYS_TYPE_STACKING_TRANSITION_MODE)
        {
            /* Set flag to Transition done, make STKCTRL task turn-back 
             */
            if (all_events & WEBAUTH_TYPE_EVENT_ENTER_TRANSITION )
                webauth_task_transitiondone = TRUE;
                
            all_events = WEBAUTH_TYPE_EVENT_NONE;
        }
        else if (current_mode == SYS_TYPE_STACKING_SLAVE_MODE)
        {
            all_events = WEBAUTH_TYPE_EVENT_NONE;
        }
        else /* MASTER MODE */
        {
            if (all_events & WEBAUTH_TYPE_EVENT_TIMER)
            {
                WEBAUTH_MGR_ProcessTimerEvent();
                all_events &= ~WEBAUTH_TYPE_EVENT_TIMER;
            } /* End of if*/
        }

    } /* End of while */
	
} /* End of HTTP_TASK_Main () */

/* FUNCTION NAME:  WEBAUTH_TASK_StartTimerEvent
 * PURPOSE: Service routine to start the periodic timer event 
 * INPUT:   none.
 * OUTPUT:  None
 * RETURN:  None
 * NOTE:    None
 */
#define WEBAUTH_TASK_TIMER_SEC      1
static  void WEBAUTH_TASK_StartTimerEvent(void)
{
    webauth_task_timerid = SYSFUN_PeriodicTimer_Create();
    SYSFUN_PeriodicTimer_Start(webauth_task_timerid,
                               WEBAUTH_TASK_TIMER_SEC * SYS_BLD_TICKS_PER_SECOND,
                               WEBAUTH_TYPE_EVENT_TIMER);

}/* End of WEBAUTH_TASK_StartTimerEvent */

