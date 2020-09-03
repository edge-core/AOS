/* Project Name: New Feature
 * File_Name : Tacacs_task.h
 * Purpose     : Tacacs initiation and Tacacs task creation
 *
 * 2004/06/08    : Ricky Lin     Create this file
 *
 * Copyright(C)      Accton Corporation, 2001, 2002
 *
 * Note    : Designed for new platform (Mercury)
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <string.h>
#include "sys_cpnt.h"
#include "sysfun.h"
#include "l_threadgrp.h"
#include "tacacs_task.h"
#if (SYS_CPNT_TACACS_PLUS_ACCOUNTING == TRUE)
#include "tacacs_mgr.h"
/*#include "socket.h"*/
#include "sysfun.h"
#include "sys_bld.h"
#include "libtacacs.h"
#include "sys_module.h"
#include "l_mm.h"
#include "sys_time.h"
#include "aaa_mgr.h"
#include "aaa_om.h"
#include "tacacs_om.h"
#include "tacacs_om_private.h"
#include "auth_protocol_proc_comm.h"

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
#include "sw_watchdog_mgr.h"
#endif

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void TACACS_TASK_Body(void);
static void TACACS_TASK_ChildTaskBody(void *arg);
static void TACACS_TASK_MainRoutine(UI16_T acc_user_index);
extern BOOL_T tacacs_acc_main(
    UI32_T server_ip,
    UI8_T *secret,
    UI32_T server_port,
    UI32_T retransmit,
    UI32_T timeout,
    int flag,
    TACACS_AccUserInfo_T *user_info,
    UI32_T current_sys_time);

#if (SYS_CPNT_TACACS_PLUS_ACCOUNTING_COMMAND == TRUE)
static void TACACS_TASK_ProcessAccCmdMessage(UI16_T acc_user_index,
                                             TPACC_AccCommandmdMessage_T *acc_cmd_msg);
extern BOOL_T TAC_ACCOUNT_C_AcctCmdMain(UI32_T server_ip,
                                        UI8_T *secret,
                                        UI32_T server_port,
                                        UI32_T timeout,
                                        UI32_T retransmit,
                                        int    flag,
                                        const TPACC_AccCommandmdMessage_T *cmd_entry_p,
                                        UI32_T current_sys_time);
#endif /*#if (SYS_CPNT_TACACS_PLUS_ACCOUNTING_COMMAND == TRUE)*/

/* STATIC VARIABLE DECLARATIONS
 */

static BOOL_T is_transition_done;
//static UI32_T tacacs_task_id;

/* EXPORTED SUBPROGRAM BODIES
 */


/* FUNCTION NAME: TACACS_TASK_Initiate_System_Resources
 * PURPOSE: This function is used to initialize the stack control module.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure, system error.
 * NOTES:
 *
 */
BOOL_T TACACS_TASK_Initiate_System_Resources(void)
{
    TACACS_TASK_Init();
    return TRUE;
} /* End of TACACS_TASK_Initiate_System_Resources */


/* ------------------------------------------------------------------------
 *  ROUTINE NAME  - TACACS_TASK_Body
 * ------------------------------------------------------------------------
 *  FUNCTION : TBD
 *  INPUT    : None
 *  OUTPUT   : None.
 *  RETURN   : None
 *  NOTE     : TBD
 * ------------------------------------------------------------------------*/
static void TACACS_TASK_Body(void)
{
    UI32_T	event_var;
    UI32_T	wait_events;
    UI32_T	rcv_events;
    UI32_T	timeout;
    UI32_T	ret_value;
    SYS_TYPE_Stacking_Mode_T current_mode;
    UI32_T  queue_id = 0;
    TACACS_Acc_Queue_T         msg;
    L_THREADGRP_Handle_T tg_handle =(L_THREADGRP_Handle_T) AUTH_PROTOCOL_PROC_COMM_GetAuthProtocolGroupTGHandle();
    UI32_T member_id;
    UI8_T msg_space_p[sizeof(SYSFUN_Msg_T)+sizeof(TACACS_Acc_Queue_T)];
    SYSFUN_Msg_T *req_msg_p;
    req_msg_p=(SYSFUN_Msg_T *)msg_space_p;

    TACACS_MGR_CreateTaskMsgQ();

    /* join the thread group
     */
    if(L_THREADGRP_Join(tg_handle, SYS_BLD_AUTH_PROTOCOL_GROUP_MGR_THREAD_PRIORITY, &member_id)==FALSE)
    {
        printf("%s: L_THREADGRP_Join fail.\n", __FUNCTION__);
        return;
    }

    /*	Prepare waiting event and init. event var.	*/
    wait_events = TACACS_TASK_EVENT_ENTER_TRANSITION |
                  TACACS_TASK_EVENT_ACC_REQ;

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
    wait_events |= SYSFUN_SYSTEM_EVENT_SW_WATCHDOG;
#endif

    event_var = 0;

    TACACS_MGR_GetAccMsgQId(&queue_id);
    while (1)
    {
        /*	Check timer event and message event	*/
        if (event_var != 0)
        {    /* There is some message in message queue  */
            timeout = SYSFUN_TIMEOUT_NOWAIT;
        }
        else
            timeout = SYSFUN_TIMEOUT_WAIT_FOREVER;

        if ((ret_value = SYSFUN_ReceiveEvent (wait_events, SYSFUN_EVENT_WAIT_ANY,
                                 timeout, &rcv_events))!=SYSFUN_OK)
        {
        	if (ret_value != SYSFUN_RESULT_NO_EVENT_SET)
        	{
        		/*	Log to system : unexpect return value	*/
        		;
        	}
        }
        event_var |= rcv_events;

        if (event_var==0)
        {
        	/*	Log to system : ERR--Receive Event Failure */
            continue;
        }

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
        if (event_var & SYSFUN_SYSTEM_EVENT_SW_WATCHDOG)
        {
       	    SW_WATCHDOG_MGR_ResetTimer(SW_WATCHDOG_TACACS);
            event_var ^= SYSFUN_SYSTEM_EVENT_SW_WATCHDOG;
        }
#endif

        /* Get the system operation mode from MGR */
    	current_mode =  TACACS_MGR_CurrentOperationMode();

        if (current_mode == SYS_TYPE_STACKING_TRANSITION_MODE)
        {
            /* Set flag to Transition done, make STKCTRL task turn-back */
            if (event_var & TACACS_TASK_EVENT_ENTER_TRANSITION )
                is_transition_done = TRUE;

            event_var = 0;

            continue;
        }
        else if (current_mode == SYS_TYPE_STACKING_SLAVE_MODE)
        {
            event_var = 0;
            continue;
        }
        else if (event_var & TACACS_TASK_EVENT_ACC_REQ)
        {
            if(queue_id == 0)
            {
                TACACS_MGR_GetAccMsgQId(&queue_id);
            }

            while(SYSFUN_ReceiveMsg(queue_id, 0,SYSFUN_TIMEOUT_NOWAIT,
                        sizeof(TACACS_Acc_Queue_T), req_msg_p)== SYSFUN_OK)
            {
                memcpy(&msg,req_msg_p->msg_buf,sizeof(TACACS_Acc_Queue_T));
                /* request thread group execution permission
                 */
                if(L_THREADGRP_Execution_Request(tg_handle, member_id)!=TRUE)
                {
                    SYSFUN_Debug_Printf("%s: L_THREADGRP_Execution_Request fail.\n", __FUNCTION__);
                }

                if (NULL == msg.msg_data_p)
                {
                    TACACS_TASK_MainRoutine(msg.user_index);
                }
                else
                {
                #if (SYS_CPNT_TACACS_PLUS_ACCOUNTING_COMMAND == TRUE)
                    TACACS_TASK_ProcessAccCmdMessage(
                        msg.user_index, (TPACC_AccCommandmdMessage_T *) msg.msg_data_p);
                #endif
                    L_MM_Free((void *)msg.msg_data_p);
                    msg.msg_data_p = NULL;
                }

                /* release thread group execution permission
                 */
                if(L_THREADGRP_Execution_Release(tg_handle, member_id)!=TRUE)
                {
                    SYSFUN_Debug_Printf("%s: L_THREADGRP_Execution_Release fail.\n", __FUNCTION__);
                }

                SYSFUN_Sleep(20);
            }
            event_var ^= TACACS_TASK_EVENT_ACC_REQ;
        }

    } /* End of while */

} /* End of TACACS_TASK_Body() */

/*---------------------------------------------------------------------------+
 * Routine Name : TACACS_TASK_Init()                                           +
 *---------------------------------------------------------------------------+
 * Function : Initialize TACACS 's Task .	                                     +
 * Input    : None                                                           +
 * Output   :                                                                +
 * Return   : never returns                                                  +
 * Note     :                                                                +
 *---------------------------------------------------------------------------*/
void TACACS_TASK_Init(void)
{

    is_transition_done = FALSE;
}

/*---------------------------------------------------------------------------+
 * Routine Name : TACACS_TASK_CreateTACACSTask()                                 +
 *---------------------------------------------------------------------------+
 * Function : Create and start TACACS task	                             +
 * Input    : None						             +
 * Output   :                                                                +
 * Return   : never returns                                                  +
 * Note     :                                                                +
 *---------------------------------------------------------------------------*/
void TACACS_TASK_CreateTACACSTask(void)
{
    UI32_T thread_id;

   if (SYSFUN_SpawnThread (SYS_BLD_TACACS_CSC_THREAD_PRIORITY,
                          SYS_BLD_TACACS_CSC_THREAD_SCHED_POLICY,
                          SYS_BLD_TACACS_CSC_THREAD_NAME,
                          SYS_BLD_TASK_COMMON_STACK_SIZE,
                          SYSFUN_TASK_FP,
                          TACACS_TASK_Body,
                          NULL,
                          &thread_id)!=SYSFUN_OK)
    {
        printf("%s:SYSFUN_SpawnThread fail.\n", __FUNCTION__);
    } /* End of if */

    TACACS_MGR_SetMainTaskId(thread_id);
#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
    SW_WATCHDOG_MGR_RegisterMonitorThread(SW_WATCHDOG_TACACS, thread_id, SYS_ADPT_TACACS_SW_WATCHDOG_TIMER );
#endif
} /* End of TACACS_TASK_CreateTACACSTask() */



/* FUNCTION NAME : TACACS_TASK_SetTransitionMode
 * PURPOSE:
 *		Sending enter transition event to task calling by stkctrl.
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
void TACACS_TASK_SetTransitionMode()
{
    UI32_T tacacs_task_id = 0;
	/* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */

    /* BODY */
    is_transition_done = FALSE;
    TACACS_MGR_GetMainTaskId(&tacacs_task_id);
    SYSFUN_SendEvent (tacacs_task_id, TACACS_TASK_EVENT_ENTER_TRANSITION);
} /* end of TACACS_TASK_SetTransitionMode */

/* FUNCTION NAME : TACACS_TASK_EnterTransitionMode
 * PURPOSE:
 *		Sending enter transition event to task calling by stkctrl.
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
void 	TACACS_TASK_EnterTransitionMode()
{
	/* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */

    /* BODY */
    /*	want task release all resources	*/
	SYSFUN_TASK_ENTER_TRANSITION_MODE(is_transition_done);
}   /* end of TACACS_TASK_EnterTransitionMode */

/* FUNCTION NAME : TACACS_TASK_EnterMasterMode
 * PURPOSE:
 *		Enter Master mode calling by stkctrl.
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
void TACACS_TASK_EnterMasterMode()
{
	/* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */

    /* BODY */
    ;
}   /* end of TACACS_TASK_EnterMasterMode */

/* FUNCTION NAME : TACACS_TASK_EnterSlaveMode
 * PURPOSE:
 *		Enter Slave mode calling by stkctrl.
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
void TACACS_TASK_EnterSlaveMode()
{
	/* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */

    /* BODY */
    ;
}   /* end of TACACS_TASK_EnterSlaveMode */

/* ------------------------------------------------------------------------
 *  ROUTINE NAME  - TACACS_TASK_MainRoutine
 * ------------------------------------------------------------------------
 *  FUNCTION : TBD
 *  INPUT    : None
 *  OUTPUT   : None.
 *  RETURN   : None
 *  NOTE     : TBD
 * ------------------------------------------------------------------------*/
static void TACACS_TASK_MainRoutine(UI16_T acc_user_index)
{
   UI32_T   i;
   UI32_T   tid;
   char     name[16] = {0};
   int      id = 0;
   UI32_T   arg;

   /* create a task name for the new task */
   for (i=0; i < 100; i++)
   {
      if (++id > 99)
         id = 1;

      sprintf(name, "%s%2.2d", "tacacs", id);
      if (SYSFUN_TaskNameToID(name, &tid))
         break;
   }
   if (i >= 100)
   {
      printf ("\r\n TACACS_TASK_MainRoutine: spawn TACACS child task error\n");
      return;
   }

    arg = acc_user_index;
    if (SYSFUN_SpawnThread (SYS_BLD_TACACS_CSC_THREAD_PRIORITY+1,
                          SYS_BLD_TACACS_CSC_THREAD_SCHED_POLICY,
                          name,
                          SYS_BLD_TASK_COMMON_STACK_SIZE,
                          SYSFUN_TASK_FP,
                          TACACS_TASK_ChildTaskBody,
                          (void*)arg,
                          &tid)!=SYSFUN_OK)
    {
        printf("%s:SYSFUN_SpawnThread fail.\n", __FUNCTION__);
    } /* End of if */

   /* set task id to database */
   TACACS_MGR_SetAccUserEntryTaskId(acc_user_index, tid);

   SYSFUN_SendEvent(tid, TACACS_TASK_EVENT_ACC_START);
   return;
}

/* ------------------------------------------------------------------------
 *  ROUTINE NAME  - TACACS_TASK_ChildTaskBody
 * ------------------------------------------------------------------------
 *  FUNCTION : TBD
 *  INPUT    : None
 *  OUTPUT   : None.
 *  RETURN   : None
 *  NOTE     : TBD
 * ------------------------------------------------------------------------*/
static void TACACS_TASK_ChildTaskBody(void *arg)
{
    UI32_T	event_var;
    UI32_T	wait_events;
    UI32_T	rcv_events;
    UI32_T	timeout;
    UI32_T	ret_value;
    SYS_TYPE_Stacking_Mode_T current_mode;
    TACACS_AccUserInfo_T   user_info;
    TACACS_Server_Host_T server_host;
    UI32_T  current_sys_time;
    void*   timer_id = NULL;
    UI32_T  retry_count;
    UI16_T  acc_user_id;

    /*	Prepare waiting event and init. event var.	*/
    wait_events = TACACS_TASK_EVENT_TIMER |
                  TACACS_TASK_EVENT_ENTER_TRANSITION |
                  TACACS_TASK_EVENT_ACC_START |
                  TACACS_TASK_EVENT_ACC_STOP;


    event_var = 0;
    retry_count = 0;

    acc_user_id = (UI16_T)((UI32_T)arg);

    while (1)
    {
        /*	Check timer event and message event	*/
        if (event_var != 0)
        {    /* There is some message in message queue  */
            timeout = SYSFUN_TIMEOUT_NOWAIT;
        }
        else
            timeout = SYSFUN_TIMEOUT_WAIT_FOREVER;

        if ((ret_value = SYSFUN_ReceiveEvent (wait_events, SYSFUN_EVENT_WAIT_ANY,
                                 timeout, &rcv_events))!=SYSFUN_OK)
        {
        	if (ret_value != SYSFUN_RESULT_NO_EVENT_SET)
        	{
        		/*	Log to system : unexpect return value	*/
        		;
        	}
        }
        event_var |= rcv_events;

        if (event_var==0)
        {
        	/*	Log to system : ERR--Receive Event Failure */
            continue;
        }

        /* Get the system operation mode from MGR */
    	current_mode =  TACACS_MGR_CurrentOperationMode();

        if (current_mode != SYS_TYPE_STACKING_MASTER_MODE)
        {
            event_var = 0;

            continue;
        }
        else if (event_var & TACACS_TASK_EVENT_ACC_START)
        {
            BOOL_T  return_val = FALSE;

            user_info.user_index = acc_user_id;
            SYS_TIME_GetRealTimeBySec(&current_sys_time);
            if(TRUE == TACACS_OM_GetAccUserEntry(&user_info))
            {
                if (0 == user_info.tacacs_entry_info.active_server_index)
                {
                    TACACS_AccObjectProcessResult_T result;

                    result = TACACS_MGR_ChangeAccUserServer(&user_info);
                    if(TPACC_OPROCESS_DELETED == result)
                    {
                        TACACS_OM_SetAccUserEntryConnectStatus(user_info.user_index, AAA_ACC_CNET_FAILED);
                        SYSFUN_DelSelfThread();
                    }
                    else if(TPACC_OPROCESS_FAILURE == result)
                    {
                        /* setting not ready,just remove this event */
                        TACACS_OM_SetAccUserEntryConnectStatus(user_info.user_index, AAA_ACC_CNET_FAILED);
                        event_var ^= TACACS_TASK_EVENT_ACC_START;
                        SYSFUN_Sleep(50);
                        continue;
                    }
                }
                if(FALSE == TACACS_OM_Get_Server_Host(user_info.tacacs_entry_info.active_server_index, &server_host))
                {
                    SYSFUN_Sleep(20);
                    event_var ^= TACACS_TASK_EVENT_ACC_START;
                    continue;
                }
                return_val = tacacs_acc_main(server_host.server_ip,
                                server_host.secret,
                                server_host.server_port,
                                server_host.retransmit,
                                server_host.timeout,
                                TAC_PLUS_ACCT_FLAG_START,
                                &user_info,
                                current_sys_time);
                if(return_val == TRUE)
                {
                    TACACS_MGR_SetAccUserEntryLastUpdateTime(user_info.user_index, current_sys_time);

                    TACACS_OM_SetAccUserEntryConnectStatus(user_info.user_index, AAA_ACC_CNET_CONNECTED);
                    retry_count = 0;
                    event_var ^= TACACS_TASK_EVENT_ACC_START;
                    timer_id = SYSFUN_PeriodicTimer_Create();
                    SYSFUN_PeriodicTimer_Start(timer_id, TACACS_TASK_TIMER_PERIOD, TACACS_TASK_EVENT_TIMER);
                }
                else
                {
                    retry_count++;
                    if(retry_count >= 10)
                    {
                        TACACS_OM_SetAccUserEntryConnectStatus(user_info.user_index, AAA_ACC_CNET_FAILED);
                        if (0) {
                            printf("%lu kill-self. The server is unreachable. The accounting data is lost.\r\n", SYSFUN_TaskIdSelf());
                        }
                        SYSFUN_DelSelfThread();
                    }
                    else
                    {
                        TACACS_OM_SetAccUserEntryConnectStatus(user_info.user_index, AAA_ACC_CNET_CONNECTING);
                        SYSFUN_Sleep(100);
                    }
                }
            }
            else
            {
                /* User leave when task is connecting to TACACS server.
                 * About accounting data. This may be a security issue.
                 */
                if (event_var & TACACS_TASK_EVENT_ACC_STOP)
                {
                    if (0) {
                        printf("%lu kill-self. The server is unreachable. The accounting data is lost.\r\n", SYSFUN_TaskIdSelf());
                    }
                    SYSFUN_DelSelfThread();
                }

                SYSFUN_Sleep(20);
            }
        }
        else if (event_var & TACACS_TASK_EVENT_TIMER)
        {
            UI32_T  update_interval;
            BOOL_T  return_val = FALSE;

            if (FALSE == AAA_OM_GetAccUpdateInterval(&update_interval))
            {
                event_var ^= TACACS_TASK_EVENT_TIMER;
                update_interval = 0;
                continue;
            }

            if (update_interval == 0)
            {
                event_var ^= TACACS_TASK_EVENT_TIMER;
                continue;
            }

            update_interval *= 60; /* translate to seconds */
            SYS_TIME_GetRealTimeBySec(&current_sys_time);
            user_info.user_index = acc_user_id;
            if(TRUE == TACACS_OM_GetAccUserEntry(&user_info))
            {
                if (1 == user_info.ctrl_bitmap.stop_packet_sent)
                {
                    event_var ^= TACACS_TASK_EVENT_TIMER;
                    continue;
                }
                if ((current_sys_time - user_info.last_update_time) < update_interval)
                {
                    event_var ^= TACACS_TASK_EVENT_TIMER;
                    continue;
                }
                if(FALSE == TACACS_OM_Get_Server_Host(user_info.tacacs_entry_info.active_server_index, &server_host))
                {
                    event_var ^= TACACS_TASK_EVENT_TIMER;
                    continue;
                }
                return_val = tacacs_acc_main(server_host.server_ip,
                                server_host.secret,
                                server_host.server_port,
                                server_host.retransmit,
                                server_host.timeout,
                                TAC_PLUS_ACCT_FLAG_WATCHDOG,
                                &user_info,
                                current_sys_time);
                if(return_val == TRUE)
                {
                    TACACS_MGR_SetAccUserEntryLastUpdateTime(user_info.user_index, current_sys_time);
                    TACACS_OM_SetAccUserEntryConnectStatus(user_info.user_index, AAA_ACC_CNET_CONNECTED);
                    retry_count = 0;
                }
                else
                {
                    retry_count++;
                    if(retry_count >= 10)
                    {
                        TACACS_OM_SetAccUserEntryConnectStatus(user_info.user_index, AAA_ACC_CNET_FAILED);
                        SYSFUN_PeriodicTimer_Destroy(timer_id);
                        SYSFUN_DelSelfThread();
                    }
                    else
                    {
                        TACACS_OM_SetAccUserEntryConnectStatus(user_info.user_index, AAA_ACC_CNET_CONNECTING);
                    }
                }

                event_var ^= TACACS_TASK_EVENT_TIMER;
            }
            else
            {
                event_var ^= TACACS_TASK_EVENT_TIMER;
                continue;
            }
        }
        else if (event_var & TACACS_TASK_EVENT_ACC_STOP)
        {
            event_var ^= TACACS_TASK_EVENT_ACC_STOP;
            SYS_TIME_GetRealTimeBySec(&current_sys_time);
            tacacs_acc_main(server_host.server_ip,
                            server_host.secret,
                            server_host.server_port,
                            server_host.retransmit,
                            server_host.timeout,
                            TAC_PLUS_ACCT_FLAG_STOP,
                            &user_info,
                            current_sys_time);
            SYSFUN_PeriodicTimer_Destroy(timer_id);
            SYSFUN_DelSelfThread();
        }
    } /* End of while */
}

#if (SYS_CPNT_TACACS_PLUS_ACCOUNTING_COMMAND == TRUE)
static void TACACS_TASK_ProcessAccCmdMessage(UI16_T acc_user_index, TPACC_AccCommandmdMessage_T *acc_cmd_msg)
{
    UI32_T                          current_sys_time;
    BOOL_T                          return_val;
    AAA_TacacsPlusEntryInterface_T  tacacs_entry;
    TACACS_Server_Host_T            server_host;

    memset(&server_host, 0, sizeof(TACACS_Server_Host_T));

    tacacs_entry.tacacs_index = 0;
    while (TRUE == AAA_OM_GetNextTacacsPlusEntry(acc_cmd_msg->tacacs_entry_info.aaa_group_index, &tacacs_entry))
    {
        if(    (FALSE == TACACS_OM_Get_Server_Host(tacacs_entry.tacacs_server_index, &server_host))
            || (FALSE == server_host.used_flag))
            continue;

        SYS_TIME_GetRealTimeBySec(&current_sys_time);
        return_val = TAC_ACCOUNT_C_AcctCmdMain(server_host.server_ip,
                        server_host.secret,
                        server_host.server_port,
                        server_host.timeout,
                        server_host.retransmit,
                        acc_cmd_msg->acct_flag,
                        acc_cmd_msg,
                        current_sys_time);

        if(return_val == TRUE)
                break;
    }
}
#endif /*#if (SYS_CPNT_TACACS_PLUS_ACCOUNTING_COMMAND == TRUE)*/

#endif

