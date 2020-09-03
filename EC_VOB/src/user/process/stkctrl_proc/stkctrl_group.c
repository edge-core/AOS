/* MODULE NAME:  stkctrl_group.c
 * PURPOSE:
 *     Implementations of stkctrl group.
 *
 * NOTES:
 *
 * HISTORY
 *    7/11/2007 - Charlie Chen, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_bld.h"
#include "sys_module.h"
#include "sysfun.h"
#include "l_threadgrp.h"
#include "sys_callback_mgr.h"
#include "backdoor_mgr.h"
#include "leddrv.h"

#include "stkctrl_group.h"
#include "stkctrl_proc_comm.h"
#include "stkctrl_init.h"
#include "stkctrl_task.h"

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
#include "sw_watchdog_mgr.h"
#endif

/* NAMING CONSTANT DECLARATIONS
 */
#define STKCTRL_GROUP_PMGR_MSGBUF_MAX_REQUIRED_SIZE sizeof(STKCTRL_GROUP_MgrMsg_T)

/* MACRO FUNCTION DECLARATIONS
 */
#ifdef STKCTRL_GROUP_DEBUG
#define DBG(x...) printf(x)
#else
#define DBG(x...) 
#endif

/* DATA TYPE DECLARATIONS
 */

typedef struct STKCTRL_GROUP_Syscallback_Msg_S
{
    UI8_T stack_state[SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_StackState_CBData_T))];
    UI8_T module_state_changed[SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_ModuleStateChanged_CBData_T))];
} STKCTRL_GROUP_Syscallback_Msg_T;

/* union all data type used for stkctrl group MGR IPC message to get the maximum
 * required ipc message buffer
 */
typedef union STKCTRL_GROUP_MGR_Msg_U
{
    STKCTRL_GROUP_Syscallback_Msg_T syscallback_ipcmsg;
    BACKDOOR_MGR_Msg_T              backdoor_mgr_ipcmsg;
} STKCTRL_GROUP_MgrMsg_T;

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void STKCTRL_GROUP_HandleSysCallbackIPCMsg(SYSFUN_Msg_T *msgbuf_p, UI32_T *events_p,UI32_T *unit);

/* STATIC VARIABLE DECLARATIONS
 */
/* the buffer for retrieving ipc request for stkctrl group mgr thread
 */
static UI8_T stkctrl_group_mgrtd_ipc_buf[SYSFUN_SIZE_OF_MSG(STKCTRL_GROUP_PMGR_MSGBUF_MAX_REQUIRED_SIZE)];

/* EXPORTED SUBPROGRAM BODIES
 */
/*------------------------------------------------------------------------------
 * ROUTINE NAME : STKCTRL_GROUP_Create_All_Threads
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will spawn all threads in STKCTRL GROUP.
 *
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    None.
 *
 * NOTES:
 *    All threads in the same CSC group will join the same thread group.
 *------------------------------------------------------------------------------
 */
void STKCTRL_GROUP_Create_All_Threads(void)
{
    UI32_T thread_id;
    if(SYSFUN_SpawnThread(SYS_BLD_STKCTRL_GROUP_MGR_THREAD_PRIORITY,
		SYS_BLD_STKCTRL_GROUP_MGR_THREAD_SCHED_POLICY,
		SYS_BLD_STKCTRL_GROUP_MGR_THREAD_NAME,
		SYS_BLD_TASK_COMMON_STACK_SIZE,
		SYSFUN_TASK_FP,
		STKCTRL_GROUP_Main_Thread_Function_Entry,
		NULL, &thread_id)!=SYSFUN_OK)
    {
	printf("%s:Spawn STKCTRL_GROUP MGR thread fail.\n", __FUNCTION__);
    }

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
    SW_WATCHDOG_MGR_RegisterMonitorThread(SW_WATCHDOG_STKCTRL_GROUP, thread_id, SYS_ADPT_STKCTRL_GROUP_SW_WATCHDOG_TIMER);
#endif

}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : STKCTRL_GROUP_Main_Thread_Function_Entry
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will be the function entry of main thread of the calling
 *    process.
 *
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    None.
 *
 * NOTES:
 *    1.All threads in the same CSC group will join the same thread group.
 *    2.Because the process STKCTRL_PROC conains no OM, STKCTRL_GROUP MGR thread
 *      will become main thread.
 *------------------------------------------------------------------------------
 */
void STKCTRL_GROUP_Main_Thread_Function_Entry(void)
{
    SYSFUN_MsgQ_T        ipc_msgq_handle;
    UI32_T               ret;
    L_THREADGRP_Handle_T tg_handle = STKCTRL_PROC_COMM_GetStkctrlGroupTGHandle();
    UI32_T               member_id,received_events,local_events;
    SYSFUN_Msg_T         *msgbuf_p=(SYSFUN_Msg_T*)stkctrl_group_mgrtd_ipc_buf;
    BOOL_T               need_resp;
    UI32_T  unit=0;

    /* Set proper priority
     */
    if(SYSFUN_OK!=SYSFUN_SetTaskPriority(SYS_BLD_STKCTRL_GROUP_MGR_THREAD_SCHED_POLICY,
        SYSFUN_TaskIdSelf(), SYS_BLD_STKCTRL_GROUP_MGR_THREAD_PRIORITY))
    {
        printf("\r\n%s:SYSFUN_SetTaskPriority fail", __FUNCTION__);
    }

    /* join the thread group of STKCTRL_GROUP
     */
    if(L_THREADGRP_Join(tg_handle, SYS_BLD_STKCTRL_GROUP_MGR_THREAD_PRIORITY, &member_id)==FALSE)
    {
        printf("\r\n%s: L_THREADGRP_Join fail.", __FUNCTION__);
        return;
    }

    /* create mgr ipc message queue handle
     */
    if(SYSFUN_CreateMsgQ(SYS_BLD_STKCTRL_GROUP_IPCMSGQ_KEY, SYSFUN_MSGQ_BIDIRECTIONAL, &ipc_msgq_handle)!=SYSFUN_OK)
    {
        printf("\r\n%s: SYSFUN_CreateMsgQ fail.", __FUNCTION__);
        return;
    }
#if 0 /*BT hardware spc need when init led should be LEDDRV_STACK_INIT ,and it will be done in driver proc*/
    STKTPLG_PMGR_SetStackRoleLed(LEDDRV_STACK_ARBITRATION);
#endif

    /* main loop:
     *    while(1)
     *    {
     *
     *        Wait event
     *            Handle SYSFUN_SYSTEM_EVENT_SET_TRANSITION_MODE event if any
     *            Handle SYSFUN_SYSTEM_EVENT_IPCMSG if any
     *            Handle SYSFUN_SYSTEM_EVENT_IPCFAIL if any
     *    }
     */
    /* Messages might have been enqueued before this thread gets started.
     * So it is necessary to turn SYSFUN_SYSTEM_EVENT_IPCMSG on to check this.
     */
    local_events=SYSFUN_SYSTEM_EVENT_IPCMSG;
    while(1)
    {

        SYSFUN_ReceiveEvent (SYSFUN_SYSTEM_EVENT_IPCFAIL|
                             SYSFUN_SYSTEM_EVENT_IPCMSG |
#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
                             SYSFUN_SYSTEM_EVENT_SW_WATCHDOG |
#endif
                             STKCTRL_TASK_EVENT_ALL,
                             SYSFUN_EVENT_WAIT_ANY,
                             (local_events==0)?SYSFUN_TIMEOUT_WAIT_FOREVER:SYSFUN_TIMEOUT_NOWAIT,
                             &received_events);
        local_events |= received_events;

        DBG("%s: receive event = %lu\n",__FUNCTION__,received_events);

        /* handle IPCMSG
         */
        if(local_events & SYSFUN_SYSTEM_EVENT_IPCMSG)
        {
            if((ret=SYSFUN_ReceiveMsg(ipc_msgq_handle, 0, SYSFUN_TIMEOUT_NOWAIT,
                STKCTRL_GROUP_PMGR_MSGBUF_MAX_REQUIRED_SIZE, msgbuf_p))==SYSFUN_OK)
            {
                /* request thread group execution permission
                 */
                if(L_THREADGRP_Execution_Request(tg_handle, member_id)!=TRUE)
                    printf("\r\n%s: L_THREADGRP_Execution_Request fail.", __FUNCTION__);

                DBG("%s: msgbuf_p->cmd = %lu\n",__FUNCTION__,msgbuf_p->cmd);

                /* handle request message based on cmd
                 */
                switch(msgbuf_p->cmd)
                {
                    case SYS_MODULE_SYS_CALLBACK:
                    {
                        UI32_T output_events;
                        /* SYS_CALLBACK ipc message can only be uni-direction
                         * just set need_resp as FALSE
                         */
                        need_resp=FALSE;
                        STKCTRL_GROUP_HandleSysCallbackIPCMsg(msgbuf_p, &output_events,&unit);
                        local_events |= output_events;
                    }
                        break;

                    case SYS_MODULE_BACKDOOR:
                        need_resp=BACKDOOR_MGR_HandleIPCMsg(msgbuf_p);
                        break;

                    default:
                        /* Unknow command. There is no way to idntify whether this
                         * ipc message need or not need a response. If we response to
                         * a asynchronous msg, then all following synchronous msg will
                         * get wrong responses and that might not easy to debug.
                         * If we do not response to a synchronous msg, the requester
                         * will be blocked forever. It should be easy to debug that
                         * error.
                         */
                        printf("\r\n%s: Invalid IPC req cmd(=%d).", __FUNCTION__, (int)(msgbuf_p->cmd));
                        need_resp=FALSE;
                }

                /* release thread group execution permission
                 */            
                if(L_THREADGRP_Execution_Release(tg_handle, member_id)!=TRUE)
                    printf("\r\n%s: L_THREADGRP_Execution_Release fail.", __FUNCTION__);

                if((need_resp==TRUE) && (SYSFUN_SendResponseMsg(ipc_msgq_handle, msgbuf_p)!=SYSFUN_OK))
                    printf("\r\n%s: SYSFUN_SendResponseMsg fail.", __FUNCTION__);

            } /* end of if(SYSFUN_ReceiveMsg... */
            else
            {
                if(ret==SYSFUN_RESULT_NO_MESSAGE)
                    local_events ^= SYSFUN_SYSTEM_EVENT_IPCMSG;
                else if(ret!=SYSFUN_RESULT_EINTR)
                    printf("\r\n%s(): Error on SYSFUN_ReceiveMsg(ret=%d)", __FUNCTION__, (int)ret);
            }
        } /* end of if(local_events & SYSFUN_SYSTEM_EVENT_IPCMSG) */

        /* handle STKCTRL TASK events */
        if(local_events & STKCTRL_TASK_EVENT_ALL)
        {
            UI32_T stkctrl_events;

            stkctrl_events = local_events & STKCTRL_TASK_EVENT_ALL;
            local_events &= ~STKCTRL_TASK_EVENT_ALL;

            /* request thread group execution permission
             */
            if(L_THREADGRP_Execution_Request(tg_handle, member_id)!=TRUE)
                printf("\r\n%s: L_THREADGRP_Execution_Request fail.", __FUNCTION__);

            /* In STKCTRL_TASK_HandleEvents(), it is possible to clear all events
             * to prevent from clearing events that does not belong to stkctrl
             * another local variable is passed to STKCTRL_TASK_HandleEvents()
             */
            DBG("%s: stkctrl_events = %lu\n",__FUNCTION__, stkctrl_events);
            STKCTRL_TASK_HandleEvents(&stkctrl_events,&unit);

            /* release thread group execution permission
             */            
            if(L_THREADGRP_Execution_Release(tg_handle, member_id)!=TRUE)
                printf("\r\n%s: L_THREADGRP_Execution_Release fail.", __FUNCTION__);

            local_events |= stkctrl_events;
        }

        /* handle IPC Async Callback fail when IPC Msgq is empty
         */
        if((local_events & SYSFUN_SYSTEM_EVENT_IPCFAIL) &&
            !(local_events & SYSFUN_SYSTEM_EVENT_IPCMSG))
        {
            /* read fail info from IPCFAIL
             */

            /* do recovery action
             */
            local_events ^= SYSFUN_SYSTEM_EVENT_IPCFAIL;
        }

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
       if (local_events & SYSFUN_SYSTEM_EVENT_SW_WATCHDOG)
       {
       	   SW_WATCHDOG_MGR_ResetTimer(SW_WATCHDOG_STKCTRL_GROUP);
           local_events ^= SYSFUN_SYSTEM_EVENT_SW_WATCHDOG;
       }
#endif

    } /* end of while(1) */
}
/* LOCAL SUBPROGRAM BODIES
 */

/*------------------------------------------------------------------------------
 * ROUTINE NAME : STKCTRL_GROUP_HandleSysCallbackIPCMsg
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will handle all callbacks from IPC messages in stkctrl group.
 *
 * INPUT:
 *    msgbuf_p  --  SYS_CALLBACK IPC message
 *
 * OUTPUT:
 *    events_p  --  The events to be handled after this function is called.
 *
 * RETURN:
 *    None.
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
static void STKCTRL_GROUP_HandleSysCallbackIPCMsg(SYSFUN_Msg_T *msgbuf_p, UI32_T* events_p,UI32_T *unit)
{
    SYS_CALLBACK_MGR_Msg_T *sys_cbmsg_p = (SYS_CALLBACK_MGR_Msg_T*)msgbuf_p->msg_buf;

    *events_p=0;

    DBG("%s: sys_cbmsg_p->callback_event_id = %lu\n",__FUNCTION__,sys_cbmsg_p->callback_event_id);
    DBG("%s: %d,%d,%d,%d,%d,%d\n",__FUNCTION__,SYS_CALLBACK_MGR_CALLBACKEVENTID_STACK_STATE,SYS_CALLBACK_MGR_CALLBACKEVENTID_MODULE_STATE_CHANGED,
    SYS_CALLBACK_MGR_CALLBACKEVENTID_UnitHotInsertRemove,SYS_CALLBACK_MGR_CALLBACKEVENTID_ENTER_TRANSITION_MODE,
    SYS_CALLBACK_MGR_CALLBACKEVENTID_PROVISION_COMPLETE,SYS_CALLBACK_MGR_CALLBACKEVENTID_MODULE_PROVISION_COMPLETE);

    switch(sys_cbmsg_p->callback_event_id)
    {
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_STACK_STATE:
        {
            SYS_CALLBACK_MGR_StackState_CBData_T *cbdata_p = (SYS_CALLBACK_MGR_StackState_CBData_T*)(sys_cbmsg_p->callback_data);

            STKCTRL_TASK_StackState_CallBack(cbdata_p->msg, events_p);
        }
            break;            
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_MODULE_STATE_CHANGED:
        {
            SYS_CALLBACK_MGR_ModuleStateChanged_CBData_T *cbdata_p = (SYS_CALLBACK_MGR_ModuleStateChanged_CBData_T*)(sys_cbmsg_p->callback_data);
            *unit = cbdata_p->unit_id;
            STKCTRL_TASK_ModuleStateChanged_CallBack(unit, events_p);
        }

            break;
   #if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_UnitHotInsertRemove:
           STKCTRL_TASK_UnitHotInsertRemove_CallBack(events_p,unit);
           break;  
   #endif
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_ENTER_TRANSITION_MODE:
            STKCTRL_TASK_EnterTransitionMode_CallBack(events_p);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_PROVISION_COMPLETE:
            STKCTRL_TASK_ProvisionComplete_CallBack(events_p);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_MODULE_PROVISION_COMPLETE:
           {
            SYS_CALLBACK_MGR_ModuleStateChanged_CBData_T         *cbdata_msg_p;
            cbdata_msg_p = (SYS_CALLBACK_MGR_ModuleStateChanged_CBData_T*)sys_cbmsg_p->callback_data;
            *unit = cbdata_msg_p->unit_id;
            STKCTRL_TASK_ModuleProvisionComplete_CallBack(events_p,unit);
           }
           break;
        default:
            SYSFUN_Debug_Printf("\r\n%s: received callback_event that is not handled(%d)",
                __FUNCTION__, sys_cbmsg_p->callback_event_id);
    }
}

