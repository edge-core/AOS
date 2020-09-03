/* FUNCTION NAME: add_task.h
 * PURPOSE:
 *	1. ADD initiation and task creation
 * NOTES:
 *
 * REASON:
 *    DESCRIPTION:
 *    CREATOR:	Junying
 *    Date       2006/04/26
 *
 * Copyright(C)      Accton Corporation, 2006
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include "sys_bld.h"
#include "sys_type.h"
#include "sysfun.h"
#include "l_mem.h"
#include "l_mpool.h"
#include "backdoor_mgr.h"
#include "add_backdoor.h"
#include "sys_cpnt.h"
#include "sys_module.h"
#include "syslog_type.h"
#include "amtr_mgr.h"
#include "add_om.h"
#include "add_mgr.h"
#include "add_task.h"
#include "l2mux_mgr.h"

/* NAMEING CONSTANT */
#define ADD_TASK_MSG_NO               64    /* SYS_BLD_DEFAULT_MSG_NBR            */

#define _ADD_TASK_DEBUG

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void ADD_TASK_Body();
static void ADD_TASK_Timer_Alarm();

/* TYPE DECLARATIONS
 */
typedef struct  ADD_TASK_LCB_S              /* LCB -- Local Control Block  */
{
    BOOL_T  init_flag;                      /* TRUE: ADD_TASK initialized  */
    UI32_T  add_task_id;                    /* ADD_TASK ID                 */
    UI32_T  msg_id;                         /* MSG ID                      */
}   ADD_TASK_LCB_T, *ADD_TASK_LCB_P;

/* STATIC VARIABLE DECLARATIONS
 */
//static UI32_T add_task_sem_id;
static  ADD_TASK_LCB_P      add_task_lcb;
static  ADD_TASK_LCB_T      add_task_lcb_t;
static  BOOL_T              add_task_is_transition_done;

/* EXPORTED SUBPROGRAM BODIES
 */
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_TASK_Initiate_System_Resources
 * ---------------------------------------------------------------------
 * PURPOSE: Init system resource.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETURN:  None.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_TASK_Initiate_System_Resources()
{
    add_task_lcb = &add_task_lcb_t;
    add_task_lcb-> init_flag = TRUE;

    /* Create Queue */
    if(SYSFUN_CreateMsgQ(ADD_TASK_MSG_NO, SYSFUN_MSG_FIFO, &(add_task_lcb->msg_id)) != SYSFUN_OK)
    {
        SYSFUN_Debug_Printf("\r\n[%s:%d] Fail to create message queue.", __FUNCTION__, __LINE__);
        return FALSE;
    }
    ADD_OM_SetMessageQueueId(add_task_lcb->msg_id);
    BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack("add", ADD_Backdoor_Main);

    add_task_is_transition_done = FALSE;
    return TRUE;
} /* End of ADD_TASK_Initiate_System_Resources */

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_TASK_CreateTask
 * ---------------------------------------------------------------------
 * PURPOSE: Create and start ADD task.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETURN:  None.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
void ADD_TASK_CreateTask()
{
    if(SYSFUN_SpawnTask (SYS_BLD_ADD_TASK,
                         SYS_BLD_ADD_TASK_PRIORITY,
                         SYS_BLD_TASK_COMMON_STACK_SIZE,
                         0,
                         ADD_TASK_Body,
                         0,
                         &add_task_lcb->add_task_id) != SYSFUN_OK )
    {
        SYSFUN_Debug_Printf("\r\n[%s:%d] Fail to create task.", __FUNCTION__, __LINE__);
        add_task_lcb->add_task_id = 0;
    } /* End of if */
    ADD_OM_SetTaskId(add_task_lcb->add_task_id);
} /* End of ADD_TASK_CreateTask() */

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_TASK_SetTransitionMode
 * ---------------------------------------------------------------------
 * PURPOSE: Sending enter transition event to task calling by stkctrl.
 * INPUT:  None.
 * OUTPUT: None.
 * RETURN: None
 * NOTES:  None
 * ---------------------------------------------------------------------
 */
void ADD_TASK_SetTransitionMode()
{
    add_task_is_transition_done = FALSE;
    SYSFUN_SendEvent (ADD_OM_GetTaskId(), ADD_TYPE_EVENT_ENTER_TRANSITION);
} /* end of ADD_TASK_SetTransitionMode */

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_TASK_EnterTransitionMode
 * ---------------------------------------------------------------------
 * PURPOSE: Sending enter transition event to task calling by stkctrl.
 * INPUT:  None.
 * OUTPUT: None.
 * RETURN: None
 * NOTES:  None
 * ---------------------------------------------------------------------
 */
void ADD_TASK_EnterTransitionMode()
{
    /*  want task release all resources */
    SYSFUN_TASK_ENTER_TRANSITION_MODE(add_task_is_transition_done);
}   /* end of ADD_TASK_EnterTransitionMode */

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_TASK_EnterMasterMode
 * ---------------------------------------------------------------------
 * PURPOSE: Enter Master mode calling by stkctrl.
 * INPUT:  None.
 * OUTPUT: None.
 * RETURN: None
 * NOTES:  None
 * ---------------------------------------------------------------------
 */
void ADD_TASK_EnterMasterMode()
{
}   /* end of ADD_TASK_EnterMasterMode */

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_TASK_EnterSlaveMode
 * ---------------------------------------------------------------------
 * PURPOSE: Enter Slave mode calling by stkctrl.
 * INPUT:  None.
 * OUTPUT: None.
 * RETURN: None
 * NOTES:  None
 * ---------------------------------------------------------------------
 */
void ADD_TASK_EnterSlaveMode()
{
}   /* end of ADD_TASK_EnterSlaveMode */

/* ------------------------------------------------------------------------
 *  ROUTINE NAME  - ADD_TASK_Body
 * ------------------------------------------------------------------------
 *  FUNCTION : TBD
 *  INPUT    : None
 *  OUTPUT   : None.
 *  RETURN   : None
 *  NOTE     : TBD
 * ------------------------------------------------------------------------
 */
static void ADD_TASK_Body()
{
    SYS_TYPE_Stacking_Mode_T   current_mode;
    ADD_MessageQueue_T         msg;
    UI32_T  timer_id;
    UI32_T  events, all_events;
    UI32_T  timeout;

    all_events = ADD_TYPE_EVENT_NONE;
    SYSFUN_StartPeriodicTimerEvent(ADD_TYPE_TIMER_EVENT_OF_SEC*ADD_TYPE_TIMER_1SEC_OF_TICKS, ADD_TYPE_EVENT_TIMER, 0, &timer_id);

    while (1)
    {

        if (all_events)
            timeout = SYSFUN_TIMEOUT_NOWAIT;
        else
            timeout = SYSFUN_TIMEOUT_WAIT_FOREVER;
        SYSFUN_ReceiveEvent(ADD_TYPE_EVENT_ALL,SYSFUN_EVENT_WAIT_ANY,timeout,&events);
        all_events |= events;

        if (0 == all_events)
        {
            /* Log to system : ERR--Receive Event Failure */
            continue;
        }

        /* Get the system operation mode from MGR */
        current_mode =  ADD_MGR_CurrentOperationMode();
        if (SYS_TYPE_STACKING_TRANSITION_MODE == current_mode)
        {
            /* task in transition mode, should clear resource (msgQ) in task */
            while(SYSFUN_OK == SYSFUN_ReceiveMsgQ(add_task_lcb->msg_id, (UI32_T *)&msg, SYSFUN_TIMEOUT_NOWAIT))
            {   /* free buffer saved in message  */
                if(NULL != msg.m_data)
                {
                    L_MEM_Free((void *) msg.m_data);
                }
            }

            /* Set flag to Transition done, make STKCTRL task turn-back */
            if (all_events & ADD_TYPE_EVENT_ENTER_TRANSITION )
                add_task_is_transition_done = TRUE;
            all_events = 0;
            continue;
        }
        else if (SYS_TYPE_STACKING_SLAVE_MODE == current_mode)
        {
            all_events = 0;
            continue;
        }

        all_events &= (ADD_TYPE_EVENT_TIMER|ADD_TYPE_EVENT_NEW_MAC);
        if (all_events)
        {
            if (all_events & ADD_TYPE_EVENT_TIMER)
            {
                ADD_TASK_Timer_Alarm();
                all_events &= ~ADD_TYPE_EVENT_TIMER;
            }
         }/*End of if (all_events)*/
    } /* End of while */
} /* End of ADD_TASK_Body() */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - ADD_TASK_Timer_Alarm
 * ------------------------------------------------------------------------
 * PURPOSE  : ADD task timer
 * INPUT    :   none
 * RETURN   :   none
 * NOTE: The timer count one time every second
 * ------------------------------------------------------------------------
 */
static void ADD_TASK_Timer_Alarm()
{
    UI32_T lport;

    for(lport=1; lport <= SYS_ADPT_TOTAL_NBR_OF_LPORT; ++lport)
    {
        ADD_MGR_ProcessTimeoutMessage(lport);
    }
}

