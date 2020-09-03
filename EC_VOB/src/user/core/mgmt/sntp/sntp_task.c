/* static char SccsId[] = "+-<>?!SNTP_TASK.C  22.1  5/05/02  08:00:00";
 * ------------------------------------------------------------------------
 *  FILE NAME  -  _SNTP_TASK.C
 * ------------------------------------------------------------------------
 *  ABSTRACT:
 *		This module handles all events and signal handling functions
 *
 *  Notes:
 *		None.
 *  Modification History:
 *  Modifier           Date        Description
 *  -----------------------------------------------------------------------
 *  S.K.Yang		  03-05-2002  Created
 * ------------------------------------------------------------------------
 *  Copyright(C)				Accton Corporation, 1999
 * ------------------------------------------------------------------------
 */



/* INCLUDE FILE	DECLARATIONS
 */
#include "sys_type.h"
#include "sys_bld.h"
#include "sysfun.h"
#include "sntp_task.h"
#include "sntp_mgr.h"
#include "sntp_type.h"
#include "sntp_dbg.h"
#include "assert.h"

/* For Exceptional Handler */
#include "sys_module.h"
#include "syslog_type.h"
#include "eh_type.h"
#include "eh_mgr.h"
/* end For Exceptional Handler */

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
#include "sw_watchdog_mgr.h"
#endif

/*
 * NAME CONSTANT DECLARATIONS
 */
#define SNTP_SLEEP_TICK                     100 /* tickTime sntp slept */
#define SNTP_TASK_ONE_SEC                   SYS_BLD_TICKS_PER_SECOND
#define SNTP_TASK_EVENT_TIMER_1_SEC         BIT_2
#define SNTP_TASK_EVENT_ENTER_TRANSITION    BIT_4

/* MACRO FUNCTION DECLARATIONS
 */


/* DATA	TYPE DECLARATIONS
 */

/* For Exceptional Handler */
enum SNTP_TASK_FUN_NO_E
{
    SNTP_TASK_TASK_INIT_FUNC_NO     = 1,
    SNTP_TASK_TASK_CREATE_FUNC_NO
};

typedef struct SNTP_TASK_LCB_S
{
    UI32_T  task_id;                            /* SNTP task id */
    BOOL_T  task_created_flag;                  /* TRUE-task created, FALSE-no */
    BOOL_T  is_transition_done;                 /* flag for finishing clear all
                                                 * msg or allocation */
} SNTP_TASK_LCB_T;

/* LOCAL SUBPROGRAM DECLARATIONS */
static void             SNTP_TASK_TaskMain(void);
static BOOL_T           SNTP_TASK_IsProvisionComplete(void);

/* STATIC VARIABLE DECLARATIONS */
static SNTP_TASK_LCB_T  sntp_task_lcb;
static BOOL_T           is_provision_complete = FALSE;

/*
 * EXPORTED SUBPROGRAM SPECIFICATIONS ;
 */
/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_TASK_Init
 *------------------------------------------------------------------------------
 * PURPOSE  : SInit system resource required by SNTP_TASK; including message queue,
 *			  memory.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *------------------------------------------------------------------------------*/
void SNTP_TASK_Init(void)
{
    sntp_task_lcb.is_transition_done = FALSE;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_TASK_CreateTask
 *------------------------------------------------------------------------------
 * PURPOSE  : Create task in SNTP
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : 1. All tasks created, then call MGR, perform proper function.
 *------------------------------------------------------------------------------*/
void SNTP_TASK_CreateTask(void)
{
    /*
     * If SNTP task is never created, Create the task and set task_created
     * flag. else keep log in system : SNTP_TASK is created twice.
     */
    if (sntp_task_lcb.task_created_flag)
    {
        /* log message to system : SNTP_TASK create twice */
        return;
    }

    if(SYSFUN_SpawnThread(SYS_BLD_SNTP_CSC_THREAD_PRIORITY,
                          SYS_BLD_SNTP_CSC_THREAD_SCHED_POLICY,
                          SYS_BLD_SNTP_CSC_THREAD_NAME,
                          SYS_BLD_TASK_COMMON_STACK_SIZE,
                          SYSFUN_TASK_NO_FP,
                          SNTP_TASK_TaskMain,
                          NULL,
                          &(sntp_task_lcb.task_id))!=SYSFUN_OK)
    {
        sntp_task_lcb.task_id = 0;
        sntp_task_lcb.task_created_flag = FALSE;
        EH_MGR_Handle_Exception (SYS_MODULE_SNTP, SNTP_TASK_TASK_CREATE_FUNC_NO, EH_TYPE_MSG_TASK_CREATE, SYSLOG_LEVEL_CRIT);

        printf("%s:SYSFUN_SpawnThread fail.\n", __FUNCTION__);
    }
    else
    {
        /* Set flag in LCB, the IP_TASK resource is created */
        sntp_task_lcb.task_created_flag = TRUE;
    }

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
    SW_WATCHDOG_MGR_RegisterMonitorThread ( SW_WATCHDOG_APP_PROTOCOL_SNTP, sntp_task_lcb.task_id, SYS_ADPT_APP_PROTOCOL_SNTP_SW_WATCHDOG_TIMER );
#endif
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_TASK_ProvisionComplete
 *------------------------------------------------------------------------------
 * PURPOSE  : This function will tell the SNTP module to start.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : This function is invoked in SNTP_INIT_ProvisionComplete().
 *------------------------------------------------------------------------------*/
void SNTP_TASK_ProvisionComplete(void)
{
    is_provision_complete = TRUE;
}


/*===========================
 * LOCAL SUBPROGRAM BODIES
 *===========================
 */
/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_TASK_TaskMain
 *------------------------------------------------------------------------------
 * PURPOSE  :
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    :
 *------------------------------------------------------------------------------*/
static void SNTP_TASK_TaskMain(void)
{
    UI32_T  event_var;
    UI32_T  wait_events;
    UI32_T  rcv_events;
    UI32_T  timeout;
    UI32_T  ret_value;
    void    *one_sec_timer_id;

    /* Prepare waiting event and init. event var. */
    wait_events = SNTP_TASK_EVENT_TIMER_1_SEC     | 
#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
                  SYSFUN_SYSTEM_EVENT_SW_WATCHDOG |
#endif
                  SNTP_TASK_EVENT_ENTER_TRANSITION;
    event_var = 0;

    /* Start one sec timer */
    one_sec_timer_id = SYSFUN_PeriodicTimer_Create();
    SYSFUN_PeriodicTimer_Start(one_sec_timer_id, SNTP_TASK_ONE_SEC, SNTP_TASK_EVENT_TIMER_1_SEC);

    while (1)
    {
        /* Check timer event and message event */
        timeout = SYSFUN_TIMEOUT_WAIT_FOREVER;

        if ((ret_value=SYSFUN_ReceiveEvent (wait_events, SYSFUN_EVENT_WAIT_ANY,
                                 timeout, &rcv_events))!=SYSFUN_OK)
        {
            if (ret_value != SYSFUN_RESULT_NO_EVENT_SET)
            {
                /* Log to system : unexpect return value */
                ;
            }
        }

        event_var |= rcv_events;

        if (event_var == 0)
        {
            /* Log to system : ERR--Receive Event Failure */
            continue;
        }

        SYSFUN_Sleep(SNTP_SLEEP_TICK);

        switch (SNTP_MGR_GetOperationMode())
        {
            case SNTP_TYPE_SYSTEM_STATE_TRANSITION:
                if (event_var & SNTP_TASK_EVENT_ENTER_TRANSITION)
                {
                    sntp_task_lcb.is_transition_done = TRUE;
                }

                event_var = 0;
                is_provision_complete = FALSE;
                break;

            case SNTP_TYPE_SYSTEM_STATE_MASTER:
                if (SNTP_TASK_IsProvisionComplete() == FALSE)
                {
                    SYSFUN_Sleep(10);
                    break;
                }

                if (sntp_task_lcb.is_transition_done != FALSE)
                {
                    SNTP_MGR_InTimeServiceMode();
                }
                break;

            case SNTP_TYPE_SYSTEM_STATE_SLAVE:
#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
                event_var &= SYSFUN_SYSTEM_EVENT_SW_WATCHDOG;
#else
                event_var = 0;
#endif
                is_provision_complete = FALSE;
                SYSFUN_Sleep(10);
                break;

            default:
                break;
        }

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
       if (event_var & SYSFUN_SYSTEM_EVENT_SW_WATCHDOG)
       {
       	   SW_WATCHDOG_MGR_ResetTimer(SW_WATCHDOG_APP_PROTOCOL_SNTP);
           event_var ^= SYSFUN_SYSTEM_EVENT_SW_WATCHDOG;
       }
#endif
    }
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_TASK_EnterMasterMode
 *------------------------------------------------------------------------------
 * PURPOSE  : Enter Master mode calling by stkctrl.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    :
 *------------------------------------------------------------------------------*/

void SNTP_TASK_EnterMasterMode(void)
{
    return;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_TASK_EnterSlaveMode
 *------------------------------------------------------------------------------
 * PURPOSE  : Enter Slave mode calling by stkctrl.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    :
 *------------------------------------------------------------------------------*/
void SNTP_TASK_EnterSlaveMode(void)
{
    return;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_TASK_EnterTransitionMode
 *------------------------------------------------------------------------------
 * PURPOSE  : Sending enter transition event to task calling by stkctrl.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    :
 *------------------------------------------------------------------------------*/
void SNTP_TASK_EnterTransitionMode(void)
{
    /* want task release all resources */
    SYSFUN_TASK_ENTER_TRANSITION_MODE(sntp_task_lcb.is_transition_done);
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_TASK_SetTransitionMode
 *------------------------------------------------------------------------------
 * PURPOSE  :
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    :
 *------------------------------------------------------------------------------*/
void SNTP_TASK_SetTransitionMode(void)
{
    sntp_task_lcb.is_transition_done = FALSE;
    SYSFUN_SendEvent(sntp_task_lcb.task_id, SNTP_TASK_EVENT_ENTER_TRANSITION);
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SNTP_TASK_IsProvisionComplete
 *------------------------------------------------------------------------------
 * PURPOSE  : This function will check the SNTP module can start.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    :
 *------------------------------------------------------------------------------*/
static BOOL_T SNTP_TASK_IsProvisionComplete(void)
{
    return (is_provision_complete);
}
