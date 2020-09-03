/* MODULE NAME:  stktplg_group.c
 * PURPOSE:
 *     Implementations of stktplg group.
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

#include "stktplg_group.h"
#include "stktplg_proc_comm.h"
#include "stktplg_mgr.h"
#include "stktplg_task.h"
#include "stktplg_engine.h"

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
#include "sw_watchdog_mgr.h"
#endif

/* NAMING CONSTANT DECLARATIONS
 */
#define STKTPLG_GROUP_PMGR_MSGBUF_MAX_REQUIRED_SIZE sizeof(STKTPLG_GROUP_MgrMsg_T)

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

typedef struct STKTPLG_GROUP_Syscallback_Msg_S
{
    UI8_T ReceiveStktplgPacket[SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_MGR_IscReceiveStktplgPacket_CBData_T))];
} STKTPLG_GROUP_Syscallback_Msg_T;

/* union all data type used for stktplg group MGR IPC message to get the maximum
 * required ipc message buffer
 */
typedef union STKTPLG_GROUP_MgrMsg_U
{
    STKTPLG_GROUP_Syscallback_Msg_T syscallback_ipcmsg;
    STKTPLG_MGR_IPCMsg_T            stktplg_mgr_ipcmsg;
    BACKDOOR_MGR_Msg_T              backdoor_mgr_ipcmsg;
} STKTPLG_GROUP_MgrMsg_T;

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void STKTPLG_GROUP_Mgr_Thread_Function_Entry(void* arg);
static void STKTPLG_GROUP_HandleSysCallbackIPCMsg(SYSFUN_Msg_T *msgbuf_p);

/* STATIC VARIABLE DECLARATIONS
 */
/* the buffer for retrieving ipc request for stktplg group mgr thread
 */
static UI8_T stktplg_group_mgrtd_ipc_buf[SYSFUN_SIZE_OF_MSG(STKTPLG_GROUP_PMGR_MSGBUF_MAX_REQUIRED_SIZE)];

static void* stktplg_timer_id;

/* EXPORTED SUBPROGRAM BODIES
 */
/*------------------------------------------------------------------------------
 * ROUTINE NAME : STKTPLG_GROUP_Create_All_Threads
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will spawn all threads in STKTPLG GROUP.
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
void STKTPLG_GROUP_Create_All_Threads(void)
{
    UI32_T thread_id;

    if(SYSFUN_SpawnThread(SYS_BLD_STKTPLG_GROUP_MGR_THREAD_PRIORITY,
                          SYS_BLD_STKTPLG_GROUP_MGR_THREAD_SCHED_POLICY,
                          SYS_BLD_STKTPLG_GROUP_MGR_THREAD_NAME,
                          SYS_BLD_STKTPLG_GROUP_TASK_STACK_SIZE,
                          SYSFUN_TASK_FP,
                          STKTPLG_GROUP_Mgr_Thread_Function_Entry,
                          NULL, &thread_id)!=SYSFUN_OK)
    {
        printf("%s:Spawn STKTPLG Group MGR thread fail.\r\n", __FUNCTION__);
    }

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
    SW_WATCHDOG_MGR_RegisterMonitorThread(SW_WATCHDOG_STKTPLG_GROUP, thread_id, SYS_ADPT_STKTPLG_GROUP_SW_WATCHDOG_TIMER);
#endif

}

/* LOCAL SUBPROGRAM BODIES
 */
/*------------------------------------------------------------------------------
 * ROUTINE NAME : STKTPLG_GROUP_Mgr_Thread_Function_Entry
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This is the entry function for the MGR thread of stktplg group.
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
 *    None.
 *------------------------------------------------------------------------------
 */
static void STKTPLG_GROUP_Mgr_Thread_Function_Entry(void* arg)
{
#define STKTPLG_GROUP_IPC_MSGQ_NUM   2

    SYSFUN_MsgQ_T        ipc_msgq_handle_ar[STKTPLG_GROUP_IPC_MSGQ_NUM];
    UI32_T    i;
    SYSFUN_MsgQ_T        ipc_msgq_handle;
    UI32_T               ret;
    L_THREADGRP_Handle_T tg_handle = STKTPLG_PROC_COMM_GetStktplgGroupTGHandle();
    UI32_T               member_id,received_events,local_events;
    SYSFUN_Msg_T         *msgbuf_p=(SYSFUN_Msg_T*)stktplg_group_mgrtd_ipc_buf;
    BOOL_T               need_resp;

    /* join the thread group of STKTPLG GROUP
     */
    if(L_THREADGRP_Join(tg_handle, SYS_BLD_STKTPLG_GROUP_MGR_THREAD_PRIORITY, &member_id)==FALSE)
    {
        printf("\r\n%s: L_THREADGRP_Join fail.\r\n", __FUNCTION__);
        return;
    }

    /* create mgr ipc message queue handle
     */
    if(SYSFUN_CreateMsgQ(SYS_BLD_CSC_STKTPLG_TASK_MSGK_KEY, SYSFUN_MSGQ_BIDIRECTIONAL, &ipc_msgq_handle)!=SYSFUN_OK)
    {
        printf("\r\n%s: SYSFUN_CreateMsgQ fail.", __FUNCTION__);
        return;
    }
    ipc_msgq_handle_ar[0] = ipc_msgq_handle;

    if(SYSFUN_CreateMsgQ(SYS_BLD_STKTPLG_GROUP_IPCMSGQ_KEY, SYSFUN_MSGQ_BIDIRECTIONAL, &ipc_msgq_handle)!=SYSFUN_OK)
    {
        printf("\r\n%s: SYSFUN_CreateMsgQ fail.", __FUNCTION__);
        return;
    }
    ipc_msgq_handle_ar[1] = ipc_msgq_handle;

    /* create periodic timer event which is original in stktplg_task
     */
    stktplg_timer_id = SYSFUN_PeriodicTimer_Create();
    if(FALSE==SYSFUN_PeriodicTimer_Start(stktplg_timer_id, STKTPLG_TASK_TIMER_EVENT_INTERVAL, STKTPLG_TASK_EVENT_PERIODIC_TIMER))
        printf("\r\n%s: SYSFUN_PeriodicTimer_Start fail", __FUNCTION__);

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
                             SYSFUN_SYSTEM_EVENT_IPCMSG|
#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
                             SYSFUN_SYSTEM_EVENT_SW_WATCHDOG |
#endif
                             STKTPLG_TASK_EVENT_PERIODIC_TIMER,
                             SYSFUN_EVENT_WAIT_ANY,
                             (local_events==0)?SYSFUN_TIMEOUT_WAIT_FOREVER:SYSFUN_TIMEOUT_NOWAIT,
                             &received_events);
        local_events |= received_events;

        /* handle IPCMSG
         */
        if(local_events & SYSFUN_SYSTEM_EVENT_IPCMSG)
        {
            for (i = 0; i < STKTPLG_GROUP_IPC_MSGQ_NUM; i++)
            {
                ipc_msgq_handle = ipc_msgq_handle_ar[i];

                if((ret=SYSFUN_ReceiveMsg(ipc_msgq_handle, 0, SYSFUN_TIMEOUT_NOWAIT,
                    STKTPLG_GROUP_PMGR_MSGBUF_MAX_REQUIRED_SIZE, msgbuf_p))==SYSFUN_OK)
                {
                    /* request thread group execution permission
                     */
                    if(L_THREADGRP_Execution_Request(tg_handle, member_id)!=TRUE)
                        printf("%s: L_THREADGRP_Execution_Request fail.\r\n", __FUNCTION__);

                    /* handle request message based on cmd
                     */
                    switch(msgbuf_p->cmd)
                    {
                        case SYS_MODULE_SYS_CALLBACK:
                            /* SYS_CALLBACK ipc message can only be uni-direction
                             * just set need_resp as FALSE
                             */
                            need_resp=FALSE;
                            STKTPLG_GROUP_HandleSysCallbackIPCMsg(msgbuf_p);
                            break;

                        case SYS_MODULE_BACKDOOR:
                            need_resp=BACKDOOR_MGR_HandleIPCMsg(msgbuf_p);
                            break;

                        case SYS_MODULE_STKTPLG:
                            need_resp=STKTPLG_MGR_HandleIPCReqMsg(msgbuf_p);
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
                            printf("%s: Invalid IPC req cmd(=%d).\r\n", __FUNCTION__, (int)(msgbuf_p->cmd));
                            need_resp=FALSE;
                    }

                    /* release thread group execution permission
                     */            
                    if(L_THREADGRP_Execution_Release(tg_handle, member_id)!=TRUE)
                        printf("%s: L_THREADGRP_Execution_Release fail.\r\n", __FUNCTION__);

                    if((need_resp==TRUE) && (SYSFUN_SendResponseMsg(ipc_msgq_handle, msgbuf_p)!=SYSFUN_OK))
                        printf("%s: SYSFUN_SendResponseMsg fail.\r\n", __FUNCTION__);

                    /* only process a message for every iteration of while().
                     */
                    break;
                } /* end of if(SYSFUN_ReceiveMsg... */
                else
                {
                    /* if no message in this msgq, process next one.
                     * if other error occurs, break.
                     */
                    if(ret==SYSFUN_RESULT_NO_MESSAGE)
                        continue;
                    else if(ret!=SYSFUN_RESULT_EINTR)
                        printf("%s(): Error on SYSFUN_ReceiveMsg(ret=%d)\r\n", __FUNCTION__, (int)ret);
                    break;
                }
            } /* end of for (i) */

            /* if all msgq are empty, clear event.
             */
            if (i >= STKTPLG_GROUP_IPC_MSGQ_NUM)
            {
                if(ret==SYSFUN_RESULT_NO_MESSAGE)
                    local_events ^= SYSFUN_SYSTEM_EVENT_IPCMSG;
            }
        } /* end of if(local_events & SYSFUN_SYSTEM_EVENT_IPCMSG) */

        /* handle timer event */
        if(local_events & STKTPLG_TASK_EVENT_PERIODIC_TIMER)
        {
            /* request thread group execution permission
             */
            if(L_THREADGRP_Execution_Request(tg_handle, member_id)!=TRUE)
                printf("%s: L_THREADGRP_Execution_Request fail.\r\n", __FUNCTION__);

            STKTPLG_TASK_HandleEvents(STKTPLG_TASK_EVENT_PERIODIC_TIMER);

            /* release thread group execution permission
             */            
            if(L_THREADGRP_Execution_Release(tg_handle, member_id)!=TRUE)
                printf("%s: L_THREADGRP_Execution_Release fail.\r\n", __FUNCTION__);

            local_events ^= STKTPLG_TASK_EVENT_PERIODIC_TIMER;
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
        if(local_events & SYSFUN_SYSTEM_EVENT_SW_WATCHDOG)
        {
       	    SW_WATCHDOG_MGR_ResetTimer(SW_WATCHDOG_STKTPLG_GROUP);
            local_events ^= SYSFUN_SYSTEM_EVENT_SW_WATCHDOG;
        }
#endif
    } /* end of while(1) */
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : STKTPLG_GROUP_HandleSysCallbackIPCMsg
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will handle all callbacks from IPC messages in stktplg group.
 *
 * INPUT:
 *    msgbuf_p  --  SYS_CALLBACK IPC message
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    None.
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
static void STKTPLG_GROUP_HandleSysCallbackIPCMsg(SYSFUN_Msg_T *msgbuf_p)
{
    SYS_CALLBACK_MGR_Msg_T *sys_cbmsg_p = (SYS_CALLBACK_MGR_Msg_T*)msgbuf_p->msg_buf;

    switch(sys_cbmsg_p->callback_event_id)
    {
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_STKTPLG_PACKET:
            if(FALSE==SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, (void*)&STKTPLG_TASK_ReceivePackets))
                SYSFUN_Debug_Printf("\r\n%s:Handle event id %lu fail.", __FUNCTION__, sys_cbmsg_p->callback_event_id);
            break;
        default:
            SYSFUN_Debug_Printf("%s: received callback_event that is not handled(%d)\r\n",
                __FUNCTION__, sys_cbmsg_p->callback_event_id);
    }
}

