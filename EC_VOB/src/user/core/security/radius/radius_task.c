/* Project Name: New Feature
 * File_Name : Radius_task.c
 * Purpose     : Radius initiation and Radius task creation
 *
 * 2001/11/22    : Kevin Cheng     Create this file
 *
 * Copyright(C)      Accton Corporation, 2001, 2002
 *
 * Note    : Designed for new platform (Mercury)
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "unistd.h"
#include "radius_callback.h"
#include "radius_mgr.h"
#include "radius_task.h"
#include "radiusclient.h"
#include "sys_cpnt.h"
#include "sys_module.h"
#if (SYS_CPNT_EH == TRUE)
#include "syslog_type.h"
#include "eh_type.h"
#include "eh_mgr.h"
#endif
#include "l_threadgrp.h"
#include "auth_protocol_proc_comm.h"
#include "radius_om.h"
#include "sys_callback_mgr.h"

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
#include "sw_watchdog_mgr.h"
#endif

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */
#define s_close(fd) close(fd)

/* DATA TYPE DECLARATIONS
 */
typedef struct  RADIUS_TASK_LCB_S               /* LCB -- Local Control Block */
{
    BOOL_T      init_flag;                      /* TRUE: RADIUS_TASK initialized */
    UI32_T      radius_task_id;                 /* RADIUS_TASK ID                */
    SYSFUN_MsgQ_T msg_id;
}   RADIUS_TASK_LCB_T, *RADIUS_TASK_LCB_P;

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void RADIUS_TASK_Body(void);
static BOOL_T RADIUS_TASK_ResetMonitorContext(
    RADIUS_MGR_MonitorContext_T *context_p);
static void RADIUS_TASK_MonitorBody(void *arg_p);
static BOOL_T RADIUS_TASK_CreateRadiusMonitorTask(int socket_id);

/* STATIC VARIABLE DECLARATIONS
 */
static BOOL_T is_transition_done;

static  RADIUS_TASK_LCB_P     radius_task_lcb;
static  RADIUS_TASK_LCB_T     task_lcb_t;

/* FUNCTION NAME: RADIUS_TASK_InitiateProcessResources
 * PURPOSE: This function is used to initialize the stack control module.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure, system error.
 * NOTES:
 *
 */
BOOL_T RADIUS_TASK_InitiateProcessResources(void)
{
    RADIUS_TASK_Init();
    return TRUE;
} /* End of RADIUS_TASK_InitiateProcessResources */

/* FUNCTION NAME: RADIUS_TASK_Create_InterCSC_Relation
 * PURPOSE: This function initializes all function pointer registration operations.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:
 *
 */
void RADIUS_TASK_Create_InterCSC_Relation(void)
{
    return;
}

/*---------------------------------------------------------------------------
 * Routine Name : RADIUS_TASK_Init
 *---------------------------------------------------------------------------
 * Function : Initialize Radius 's Task
 * Input    : None
 * Output   : None
 * Return   : None
 * Note     : None
 *---------------------------------------------------------------------------
 */
void RADIUS_TASK_Init(void)
{
    radius_task_lcb = &task_lcb_t;
    radius_task_lcb->init_flag = TRUE;

    is_transition_done = FALSE;
}

/*---------------------------------------------------------------------------
 * Routine Name : RADIUS_TASK_CreateRadiusTask
 *---------------------------------------------------------------------------
 * Function : Create and start RADIUS task
 * Input    : None
 * Output   : None
 * Return   : None
 * Note     : None
 *---------------------------------------------------------------------------
 */
void RADIUS_TASK_CreateRadiusTask(void)
{
    /* suger, 05-04-2004, temporary set the task stack size to 16K
     * after the precise size is calculated, will change later.
     */
    if (SYSFUN_SpawnThread(SYS_BLD_RADIUS_THREAD_PRIORITY,
                           SYS_BLD_RADIUS_THREAD_SCHED_POLICY,
                           SYS_BLD_RADIUS_THREAD_NAME,
#if ( SYS_CPNT_DOT1X_TTLS_TLS_PEAP == TRUE)
#ifdef ES3526MA_POE_7LF_LN
                           (SYS_BLD_TASK_COMMON_STACK_SIZE * 8),
#else
                           SYS_BLD_RADIUS_TASK_LARGE_STACK_SIZE,
#endif
#else
                           SYS_BLD_TASK_LARGE_STACK_SIZE,
#endif
                           SYSFUN_TASK_NO_FP,
                           RADIUS_TASK_Body,
                           NULL,
                           &radius_task_lcb->radius_task_id) != SYSFUN_OK)
    {
        /*DBG_PrintText("Radius task creation error\n");*/
#if (SYS_CPNT_EH == TRUE)
        EH_MGR_Handle_Exception(SYS_MODULE_RADIUS, RADIUS_TASK_CreateRadiusTask_FUNC_NO, EH_TYPE_MSG_TASK_CREATE, (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_CRIT));/*Mercury_V2-00030*/
#endif
        radius_task_lcb->radius_task_id = -1;
    } /* End of if */
    RADIUS_OM_Set_TaskId(radius_task_lcb->radius_task_id);

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
    SW_WATCHDOG_MGR_RegisterMonitorThread(SW_WATCHDOG_RADIUS, radius_task_lcb->radius_task_id, SYS_ADPT_RADIUS_SW_WATCHDOG_TIMER);
#endif

} /* End of RADIUS_TASK_CreateRadiusTask() */

/* FUNCTION NAME : RADIUS_TASK_SetTransitionMode
 * PURPOSE:
 *      Sending enter transition event to task calling by stkctrl.
 * INPUT:
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *     None.
 *
 */
void RADIUS_TASK_SetTransitionMode()
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */

    /* BODY */
    is_transition_done = FALSE;
    SYSFUN_SendEvent (radius_task_lcb->radius_task_id, RADIUS_TASK_EVENT_ENTER_TRANSITION);
} /* end of RADIUS_TASK_SetTransitionMode */

/* FUNCTION NAME : RADIUS_TASK_EnterTransitionMode
 * PURPOSE:
 *      Sending enter transition event to task calling by stkctrl.
 * INPUT:
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *     None.
 *
 */
void    RADIUS_TASK_EnterTransitionMode()
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */

    /* BODY */
    /*  want task release all resources */
    SYSFUN_TASK_ENTER_TRANSITION_MODE(is_transition_done);
}   /* end of RADIUS_TASK_EnterTransitionMode */

/* FUNCTION NAME : RADIUS_TASK_EnterMasterMode
 * PURPOSE:
 *      Enter Master mode calling by stkctrl.
 * INPUT:
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *     None.
 *
 */
void RADIUS_TASK_EnterMasterMode()
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */

    /* BODY */
    ;
}   /* end of RADIUS_TASK_EnterMasterMode */

/* FUNCTION NAME : RADIUS_TASK_EnterSlaveMode
 * PURPOSE:
 *      Enter Slave mode calling by stkctrl.
 * INPUT:
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *     None.
 *
 */
void RADIUS_TASK_EnterSlaveMode()
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */

    /* BODY */
    ;
}   /* end of RADIUS_TASK_EnterSlaveMode */

/* LOCAL SUBPROGRAM BODIES
 */

/*---------------------------------------------------------------------------
 *  ROUTINE NAME  - RADIUS_TASK_Body
 *---------------------------------------------------------------------------
 *  FUNCTION : TBD
 *  INPUT    : None
 *  OUTPUT   : None
 *  RETURN   : None
 *  NOTE     : TBD
 *---------------------------------------------------------------------------
 */
static void RADIUS_TASK_Body()
{
    typedef union {
        RADIUS_TASK_MSGQ_T m2;
        RADIUS_ANSYNC_AUTH_MSG_T m3;
    } RADIUS_TASK_Msg_T;

#define RADIUS_TASK_MSG_SIZE  sizeof(RADIUS_TASK_Msg_T)

    UI8_T                  msg_buf[SYSFUN_SIZE_OF_MSG(RADIUS_TASK_MSG_SIZE)];
    SYSFUN_Msg_T           *msg_p = (SYSFUN_Msg_T *)msg_buf;
    RADIUS_TASK_MSGQ_T     *rada_msg_p = (RADIUS_TASK_MSGQ_T *)msg_p->msg_buf;
    RADIUS_RADA_AUTH_MSG_T *rada_auth_msg_p;
    RADIUS_ANSYNC_AUTH_MSG_T *radius_auth_msg_p = (RADIUS_ANSYNC_AUTH_MSG_T *)msg_p->msg_buf; /*maggie liu for RADIUS authentication ansync*/

    I32_T   result;
    UI32_T  event_var;
    UI32_T  wait_events, rcv_events, timeout, member_id;
    UI32_T  ret_value;

    SYS_TYPE_Stacking_Mode_T current_mode;

    L_THREADGRP_Handle_T      tg_handle;

    int     monitor_command_pipe[2] = { -1, -1 };
    BOOL_T  is_monitor_task_created = FALSE;

    /* Join the thread group
     */
    tg_handle = AUTH_PROTOCOL_PROC_COMM_GetAuthProtocolGroupTGHandle();
    if (L_THREADGRP_Join(tg_handle, SYS_BLD_RADIUS_THREAD_PRIORITY, &member_id) != TRUE)
    {
        printf("\r\n%s: L_THREADGRP_Join fail.", __FUNCTION__);
        return;
    }

    wait_events = RADIUS_TASK_EVENT_ENTER_TRANSITION | RADIUS_EVENT_NEW_REQ |
        RADIUS_EVENT_TIMEOUT | RADIUS_EVENT_AUTH_SOCKET_AVAILABLE |
        RADIUS_EVENT_ACCT_SOCKET_AVAILABLE;

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
    wait_events |= SYSFUN_SYSTEM_EVENT_SW_WATCHDOG;
#endif

    event_var = 0;

    while (1)
    {
        /*  Check timer event and message event */
        if (event_var != 0)
        {    /* There is some message in message queue  */
            timeout = SYSFUN_TIMEOUT_NOWAIT;
        }
        else
            timeout = SYSFUN_TIMEOUT_WAIT_FOREVER;

        if ((ret_value=SYSFUN_ReceiveEvent (wait_events, SYSFUN_EVENT_WAIT_ANY,
                 timeout, &rcv_events))!=SYSFUN_OK)
        {
            if (ret_value != SYSFUN_RESULT_NO_EVENT_SET)
            {
                /*  Log to system : unexpect return value   */
                ;
            }
        }
        event_var |= rcv_events;

        if (event_var==0)
        {
#ifndef UNIT_TEST
            /*  Log to system : ERR--Receive Event Failure */
            continue;
#else
            break;
#endif /* #ifndef UNIT_TEST */
        }

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
        if (event_var & SYSFUN_SYSTEM_EVENT_SW_WATCHDOG)
        {
            SW_WATCHDOG_MGR_ResetTimer(SW_WATCHDOG_RADIUS);
            event_var ^= SYSFUN_SYSTEM_EVENT_SW_WATCHDOG;
        }
#endif

        /* Get the system operation mode from MGR */
        current_mode =  RADIUS_MGR_CurrentOperationMode();

        if (current_mode == SYS_TYPE_STACKING_TRANSITION_MODE)
        {
            /* Set flag to Transition done, make STKCTRL task turn-back */
            if (event_var & RADIUS_TASK_EVENT_ENTER_TRANSITION)
            {
                if (TRUE == is_monitor_task_created)
                {
                    if (-1 != monitor_command_pipe[1])
                    {
                        RADIUS_MGR_NotifyMonitorTaskStopMonitor(monitor_command_pipe[1]);

                        s_close(monitor_command_pipe[1]);
                        monitor_command_pipe[1] = -1;
                    }

                    is_monitor_task_created = FALSE;
                }

                is_transition_done = TRUE;
            }

            event_var = 0;
            continue;
        }
        else if (current_mode == SYS_TYPE_STACKING_SLAVE_MODE)
        {
            event_var = 0;
            continue;
        }
        else if (SYS_TYPE_STACKING_MASTER_MODE == current_mode)
        {
            if (FALSE == is_monitor_task_created)
            {
                if (0 == pipe(monitor_command_pipe))
                {
                    if (TRUE == RADIUS_TASK_CreateRadiusMonitorTask(
                        monitor_command_pipe[0]))
                    {
                        is_monitor_task_created = TRUE;
                    }
                    else
                    {
                        s_close(monitor_command_pipe[0]);
                        monitor_command_pipe[0] = -1;

                        s_close(monitor_command_pipe[1]);
                        monitor_command_pipe[1] = -1;
                    }
                }
            }
        }

        if (event_var & RADIUS_EVENT_NEW_REQ)
        {
            RADIUS_MGR_ProcessRequestFromWaitingQueue();
            RADIUS_MGR_NotifyMonitorTaskWatchSockets(monitor_command_pipe[1]);

            event_var ^= RADIUS_EVENT_NEW_REQ;
        }
        else if (event_var & RADIUS_EVENT_TIMEOUT)
        {
            RADIUS_MGR_ProcessMonitorTimeoutEvent();
            RADIUS_MGR_NotifyMonitorTaskWatchSockets(monitor_command_pipe[1]);

            event_var ^= RADIUS_EVENT_TIMEOUT;
        }
        else if (event_var & RADIUS_EVENT_AUTH_SOCKET_AVAILABLE)
        {
            RADIUS_MGR_ProcessMonitorAuthSocketAvailableEvent();
            RADIUS_MGR_NotifyMonitorTaskWatchSockets(monitor_command_pipe[1]);

            event_var ^= RADIUS_EVENT_AUTH_SOCKET_AVAILABLE;
        }
        else if (event_var & RADIUS_EVENT_ACCT_SOCKET_AVAILABLE)
        {
            RADIUS_MGR_ProcessMonitorAcctSocketAvailableEvent();
            RADIUS_MGR_NotifyMonitorTaskWatchSockets(monitor_command_pipe[1]);

            event_var ^= RADIUS_EVENT_ACCT_SOCKET_AVAILABLE;
        }
    } /* End of while */

#undef RADIUS_TASK_MSG_SIZE
} /* End of RADIUS_TASK_Body() */

/*---------------------------------------------------------------------------
 *  ROUTINE NAME  - RADIUS_TASK_ResetMonitorContext
 *---------------------------------------------------------------------------
 *  FUNCTION : Reset monitor context.
 *  INPUT    : None
 *  OUTPUT   : context_p    - Monitor context
 *  RETURN   : None
 *  NOTE     : None
 *---------------------------------------------------------------------------
 */
static BOOL_T RADIUS_TASK_ResetMonitorContext(
    RADIUS_MGR_MonitorContext_T *context_p)
{
    if (NULL == context_p)
    {
        return FALSE;
    }

    memset(context_p, 0, sizeof(*context_p));

    context_p->is_to_stop_monitor = FALSE;
    context_p->auth_socket_id = -1;
    context_p->acct_socket_id = -1;
    context_p->timeout = 0;

    context_p->command_len = 0;

    return TRUE;
}

/*---------------------------------------------------------------------------
 *  ROUTINE NAME  - RADIUS_TASK_MonitorBody
 *---------------------------------------------------------------------------
 *  FUNCTION : Body of RADIUS montir task.
 *  INPUT    : monitor_socket_id    - Socket to receive command for monitor
 *  OUTPUT   : None
 *  RETURN   : None
 *  NOTE     : None
 *---------------------------------------------------------------------------
 */
static void RADIUS_TASK_MonitorBody(void *arg_p)
{
    int monitor_socket_id;
    RADIUS_MGR_MonitorContext_T context;

    RADIUS_TASK_ResetMonitorContext(&context);

    if (NULL == arg_p)
    {
        return;
    }

    monitor_socket_id = (intptr_t)arg_p;

    while (FALSE == context.is_to_stop_monitor)
    {
        RADIUS_MGR_MONITOR_RESULT_T result;

        result = RADIUS_MGR_MONITOR_RESULT_NONE;

        if (TRUE == RADIUS_MGR_SelectMonitorSockets(context.auth_socket_id,
            context.acct_socket_id, monitor_socket_id,
            context.timeout, &result))
        {
            switch (result)
            {
            case RADIUS_MGR_MONITOR_RESULT_COMMAND:
            {
                /* Reset the complete command received previously.
                 */
                if (sizeof(context.command) == context.command_len)
                {
                    RADIUS_TASK_ResetMonitorContext(&context);
                }

                if (   (TRUE == RADIUS_MGR_ReceiveMonitorCommand(
                        monitor_socket_id, &context))
                    && (sizeof(context.command) == context.command_len)
                    )
                {
                    RADIUS_MGR_ProcessMonitorCommand(&context);
                }

                break;
            }

            case RADIUS_MGR_MONITOR_RESULT_AUTH_SOCKET_AVAILABLE:
                SYSFUN_SendEvent(RADIUS_OM_Get_TaskId(),
                    RADIUS_EVENT_AUTH_SOCKET_AVAILABLE);

                RADIUS_TASK_ResetMonitorContext(&context);
                break;

            case RADIUS_MGR_MONITOR_RESULT_ACCT_SOCKET_AVAILABLE:
                SYSFUN_SendEvent(RADIUS_OM_Get_TaskId(),
                    RADIUS_EVENT_ACCT_SOCKET_AVAILABLE);

                RADIUS_TASK_ResetMonitorContext(&context);
                break;

            case RADIUS_MGR_MONITOR_RESULT_TIMEOUT:
                SYSFUN_SendEvent(RADIUS_OM_Get_TaskId(), RADIUS_EVENT_TIMEOUT);

                RADIUS_TASK_ResetMonitorContext(&context);
                break;

            case RADIUS_MGR_MONITOR_RESULT_NONE:
            default:
                RADIUS_TASK_ResetMonitorContext(&context);
                break;
            }
        }
    }

    s_close(monitor_socket_id);
    monitor_socket_id = -1;
}

/*---------------------------------------------------------------------------
 * Routine Name : RADIUS_TASK_CreateRadiusTask
 *---------------------------------------------------------------------------
 * Function : Create and start RADIUS task
 * Input    : None
 * Output   : None
 * Return   : None
 * Note     : None
 *---------------------------------------------------------------------------
 */
static BOOL_T RADIUS_TASK_CreateRadiusMonitorTask(int socket_id)
{
    UI32_T tid;

    if (SYSFUN_OK != SYSFUN_SpawnThread(SYS_BLD_RADIUS_THREAD_PRIORITY,
        SYS_BLD_RADIUS_THREAD_SCHED_POLICY,
        SYS_BLD_RADIUS_MONITOR_THREAD_NAME,
        SYS_BLD_RADIUS_TASK_LARGE_STACK_SIZE,
        SYSFUN_TASK_NO_FP,
        RADIUS_TASK_MonitorBody,
        (void *)(intptr_t)socket_id,
        &tid))
    {
#if (SYS_CPNT_EH == TRUE)
        EH_MGR_Handle_Exception(SYS_MODULE_RADIUS, RADIUS_TASK_CreateRadiusTask_FUNC_NO, EH_TYPE_MSG_TASK_CREATE, (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_CRIT));
#endif
        return FALSE;
    }

    return TRUE;
}
