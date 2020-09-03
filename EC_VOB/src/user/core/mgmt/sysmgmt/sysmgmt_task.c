/* ------------------------------------------------------------------------
 *  FILE NAME  -  SYSMGMT_TASK.C
 * ------------------------------------------------------------------------
 * PURPOSE:  SYSMGMT TASK
 *
 * Notes: API List
 *
 * HISTORY:
 *      Date        --  Modifier,    Reason
 *      -----------------------------------------------------------------------
 *      12-24-2007  --  Andy_Chang,  Create
 *
 *
 * ------------------------------------------------------------------------
 * Copyright(C)							  	ACCTON Technology Corp. , 2007
 * ------------------------------------------------------------------------
 */


/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include "sys_bld.h"
#include "sysfun.h"
#include "l_threadgrp.h"
#include "sys_mgmt_proc_comm.h"
#include "sys_reload_mgr.h"
#include "sys_mgr.h"

/* NAMING CONSTANT DECLARATIONS
 */

typedef enum STKCTRL_TASK_Event_E
{
    /* no event
     */
    SYSMGMT_TASK_EVENT_NONE                     = 0x0000L,

    SYSMGMT_TASK_EVENT_ENTER_TRANSITION         = 0x0001L,

    SYSMGMT_TASK_EVENT_RELOAD_SYSTEM_COUNTDOWN  = 0x0002L,

#if (SYS_CPNT_MONITORING_PROCESS_CPU == TRUE) || (SYS_CPNT_SYSMGMT_MONITORING_MEMORY_UTILIZATION == TRUE)
    SYSMGMT_TASK_EVENT_SYSTEM_RESOURCE_REFRESH_EVENT = 0x0004L,
#endif

    SYSMGMT_TASK_EVENT_ALL                      = 0xFFFFL
} SYSMGMT_TASK_EVENT_T; ;



#define SYSMGMT_TASK_TIMER_SEC          1    /* 1 seconds */


#if (SYS_CPNT_MONITORING_PROCESS_CPU == TRUE) || (SYS_CPNT_SYSMGMT_MONITORING_MEMORY_UTILIZATION == TRUE)
#define SYSMGMT_TASK_SYSTEM_RESOURCE_REFRESH_TICKS  (5 * SYS_BLD_TICKS_PER_SECOND)
#endif

/* DATA TYPE DECLARATIONS
 */


/* LOCAL SUBPROGRAM DECLARATIONS
 */

#if (SYS_CPNT_SYSMGMT_DEFERRED_RELOAD == TRUE)
static void SYSMGMT_TASK_Main(void);
#endif

/* STATIC VARIABLE DECLARATIONS
 */
static  BOOL_T                  is_provision_complete;
static  BOOL_T                  is_transition_done;
static  UI32_T                  sysmgmt_task_id;
static  void*                   sysmgmt_task_reloadid;

/* MACRO FUNCTIONS DECLARACTION
 */


/* EXPORTED SUBPROGRAM BODIES
 */

/*------------------------------------------------------------------------
 * FUNCTION NAME - SYSMGMT_TASK_Init
 *------------------------------------------------------------------------
 * PURPOSE : This function will initialize kernel resources
 * INPUT   : None.
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : None.
 *------------------------------------------------------------------------*/
void SYSMGMT_TASK_Init(void)
{
    sysmgmt_task_reloadid = 0;
    is_provision_complete = FALSE;

	return;
} /* End of SYSMGMT_TASK_Init() */


/*------------------------------------------------------------------------
 * FUNCTION NAME - SYSMGMT_TASK_CreateTask
 *------------------------------------------------------------------------
 * PURPOSE : This function will create SYSMGMT task
 * INPUT   : None.
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTES   : None.
 *------------------------------------------------------------------------*/
BOOL_T SYSMGMT_TASK_CreateTask(void)
{
#if (SYS_CPNT_SYSMGMT_DEFERRED_RELOAD == TRUE)

    void* NO_AGRS = NULL;

    if (SYSFUN_SpawnThread(SYS_BLD_SYSMGMT_THREAD_PRIORITY,
                           SYS_BLD_SYSMGMT_THREAD_SCHED_POLICY,
                           SYS_BLD_SYS_MGMT_THREAD_NAME,
                           SYS_BLD_TASK_COMMON_STACK_SIZE,
                           SYSFUN_TASK_NO_FP,
                           SYSMGMT_TASK_Main,
                           NO_AGRS,
                           &sysmgmt_task_id) != SYSFUN_OK)
    {
        perror("\r\nSYSMGMT Task Spawn error!");
        return FALSE;
    }

#endif /* SYS_CPNT_SYSMGMT_DEFERRED_RELOAD */
    return TRUE;

}/* End of SYSMGMT_TASK_CreateTask() */

/* -------------------------------------------------------------------------
 * FUNCTION NAME: SYSMGMT_TASK_EnterMasterMode
 * -------------------------------------------------------------------------
 * PURPOSE : This function will enter SYSMGMT Task master mode.
 * INPUT   : None.
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : None.
 * -------------------------------------------------------------------------*/
void SYSMGMT_TASK_EnterMasterMode(void)
{
     return;
}/* End of SYSMGMT_TASK_EnterMasterMode() */

/*------------------------------------------------------------------------
 * FUNCTION NAME - SYSMGMT_TASK_EnterTransitionMode
 *------------------------------------------------------------------------
 * PURPOSE : The function enter transition mode
 * INPUT   : None.
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : None.
 *------------------------------------------------------------------------*/
void SYSMGMT_TASK_EnterTransitionMode(void)
{
    SYSFUN_TASK_ENTER_TRANSITION_MODE(is_transition_done);
    return;
}/* End of SYSMGMT_TASK_EnterTransitionMode() */

/*------------------------------------------------------------------------
 * FUNCTION NAME - SYSMGMT_TASK_EnterSlaveMode
 *------------------------------------------------------------------------
 * PURPOSE : This function will enter slave mode
 * INPUT   : None.
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : None.
 *------------------------------------------------------------------------*/
void SYSMGMT_TASK_EnterSlaveMode(void)
{
   return;
}/* End of SYSMGMT_TASK_EnterSlaveMode() */

/*------------------------------------------------------------------------
 * FUNCTION NAME - SYSMGMT_TASK_SetTransitionMode
 *------------------------------------------------------------------------
 * PURPOSE : This function will set transition state flag
 * INPUT   : None.
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : None.
 *------------------------------------------------------------------------*/
void SYSMGMT_TASK_SetTransitionMode(void)
{
   is_transition_done = FALSE;
   SYSFUN_SendEvent(sysmgmt_task_id, SYSMGMT_TASK_EVENT_ENTER_TRANSITION);
   return;
} /* end of SYSMGMT_TASK_SetTransitionMode() */


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYSMGMT_TASK_ProvisionComplete
 * ---------------------------------------------------------------------
 * PURPOSE: The function notify us provision is completed
 * INPUT: None
 * OUTPUT:
 * RETURN: none
 * NOTES: None
 * ---------------------------------------------------------------------
 */
void SYSMGMT_TASK_ProvisionComplete(void)
{
    is_provision_complete = TRUE;
    return;
}

/* LOCAL SUBPROGRAM BODIES
 */

#if (SYS_CPNT_SYSMGMT_DEFERRED_RELOAD == TRUE)
/*------------------------------------------------------------------------
 * FUNCTION NAME: SYSMGMT_TASK_Main
 *------------------------------------------------------------------------
 * PURPOSE : Wait on for message from discp_input, and handle the
 *           message appropriately.
 * INPUT   : None.
 * OUTPUT  : None.
 * RETUEN  : None.
 * NOTES   : None.
 *------------------------------------------------------------------------*/
static void SYSMGMT_TASK_Main(void)
{
    L_THREADGRP_Handle_T tg_handle = SYS_MGMT_PROC_COMM_Get_SYSMGMT_MGR_GROUPTGHandle();
    UI32_T  member_id;
    UI32_T  event_var;      /* keep track events not be handled. */
    UI32_T  rcv_event;
    UI32_T  timeout;
    UI32_T  current_state;

    if (L_THREADGRP_Join(tg_handle, SYS_BLD_SYSMGMT_THREAD_PRIORITY, &member_id) != TRUE)
    {
        printf("\r\n%s: L_THREADGRP_Join fail.", __FUNCTION__);
        return;
    }

    /* clear events variable
     */
    event_var = SYSMGMT_TASK_EVENT_NONE;

    /* create periodic timer event
     */
    sysmgmt_task_reloadid = SYSFUN_PeriodicTimer_Create();
    if (sysmgmt_task_reloadid != NULL)
    {
        SYSFUN_PeriodicTimer_Start(sysmgmt_task_reloadid,
            SYSMGMT_TASK_TIMER_SEC*SYS_BLD_TICKS_PER_SECOND,
            SYSMGMT_TASK_EVENT_RELOAD_SYSTEM_COUNTDOWN
            );
    }
    else
    {
        SYSFUN_Debug_Printf("\r\nSYSFUN_StartPeriodicTimerEvent timer fail");
    }

    while (1)
    {
        if (event_var != SYSMGMT_TASK_EVENT_NONE)
        {
            timeout = SYSFUN_TIMEOUT_NOWAIT;
        }
        else
        {
            timeout = SYSFUN_TIMEOUT_WAIT_FOREVER;
        }

        /* wait for event_var forever
         */
        SYSFUN_ReceiveEvent(SYSMGMT_TASK_EVENT_ALL, SYSFUN_EVENT_WAIT_ANY,
                            timeout, &rcv_event);

        event_var |= rcv_event;

    	current_state =  SYS_RELOAD_MGR_GetOperationMode();

        if (current_state == SYS_TYPE_STACKING_TRANSITION_MODE)
        {
            if (event_var & SYSMGMT_TASK_EVENT_ENTER_TRANSITION )
                is_transition_done = TRUE;  /* Turn on the transition done flag */

            event_var = 0;
            continue;
        }

        /* When receive timer event, stack ctrl task will decrease the remaing time to reload
         */
        if (event_var & SYSMGMT_TASK_EVENT_RELOAD_SYSTEM_COUNTDOWN)
        {
            SYS_RELOAD_MGR_TimeHandler(SYSMGMT_TASK_TIMER_SEC);
            event_var &= ~SYSMGMT_TASK_EVENT_RELOAD_SYSTEM_COUNTDOWN;
        }
    }   /*  End of while(1) */
    return;
} /* End of SYSMGMT_TASK_Main */
#endif /* SYS_CPNT_SYSMGMT_DEFERRED_RELOAD */
