/* MODULE NAME:  CFGDB_group.c
 * PURPOSE:
 *     Files for EH group.
 *
 * NOTES:
 *
 * HISTORY
 *    7/06/2007 - Rich Lee, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */
 
/* INCLUDE FILE DECLARATIONS
 */
#include "sysfun.h"
#include "sys_cpnt.h"
#include "sys_bld.h"
#include "sys_module.h"
#include "cfgdb_group.h"
#include "core_util_proc_comm.h"
#include "sys_callback_mgr.h"
#include "l_threadgrp.h"
#include "cfgdb_init.h"
#include "cfgdb_mgr.h"
#include "cfgdb_type.h"
#include "backdoor_mgr.h"

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
#include "sw_watchdog_mgr.h"
#endif

/* NAMING CONSTANT DECLARATIONS
 */
#define CFGDB_GROUP_PMGR_MSGBUF_MAX_REQUIRED_SIZE sizeof(CFGDB_GROUP_MGR_MSG_T)

/* union all data type used for EH Group MGR IPC message to get the maximum
 * required ipc message buffer
 */
typedef union CFGDB_GROUP_MGR_MSG_U
{
    BACKDOOR_MGR_Msg_T  backdoor_mgr_ipcmsg;
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    SYS_TYPE_HandleHotSwapArg_T    HandleHotInsertionArg_ipcmsg;
#endif
} CFGDB_GROUP_MGR_MSG_T;

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void CFGDB_GROUP_Mgr_Thread_Function_Entry(void* arg);
static void CFGDB_GROUP_SetTransitionMode(void);
static void CFGDB_GROUP_EnterTransitionMode(void);
static void CFGDB_GROUP_EnterMasterMode(void);
static void CFGDB_GROUP_EnterSlaveMode(void);
static void CFGDB_GROUP_ProvisionComplete(void);
static void CFGDB_GROUP_HandleSysCallbackIPCMsg(SYSFUN_Msg_T *msgbuf_p);


/* STATIC VARIABLE DECLARATIONS
 */
/* the buffer for retrieving ipc request for CFGDB_GROUP mgr thread
 */
static UI8_T cfgdb_group_mgrtd_ipc_buf[SYSFUN_SIZE_OF_MSG(CFGDB_GROUP_PMGR_MSGBUF_MAX_REQUIRED_SIZE)];

/* EXPORTED SUBPROGRAM BODIES
 */

/*------------------------------------------------------------------------------
 * ROUTINE NAME : CFGDB_GROUP_Create_All_Threads
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will spawn all threads in CFGDB_GROUP.
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
 *   
 *------------------------------------------------------------------------------
 */
void CFGDB_GROUP_Create_All_Threads(void)
{
    UI32_T thread_id;

    if(SYSFUN_SpawnThread(SYS_BLD_CFGDB_GROUP_MGR_THREAD_PRIORITY,
                          SYS_BLD_CFGDB_GROUP_MGR_SCHED_POLICY,
                          SYS_BLD_CFGDB_GROUP_MGR_THREAD_NAME,
                          SYS_BLD_TASK_COMMON_STACK_SIZE,
                          SYSFUN_TASK_FP,
                          CFGDB_GROUP_Mgr_Thread_Function_Entry,
                          NULL, 
                          &thread_id)!=SYSFUN_OK)
    {
        printf("%s:Spawn CFGDB_GROUP MGR thread fail.\n", __FUNCTION__);
    }

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
    SW_WATCHDOG_MGR_RegisterMonitorThread(SW_WATCHDOG_CFGDB_GROUP, thread_id, SYS_ADPT_CFGDB_GROUP_SW_WATCHDOG_TIMER);
#endif

    CFGDB_MGR_SetTaskId(thread_id);
}

/* LOCAL SUBPROGRAM BODIES
 */
/*------------------------------------------------------------------------------
 * ROUTINE NAME : CFGDB_GROUP_Mgr_Thread_Function_Entry
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This is the entry function for the MGR thread of CFGDB_GROUP.
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
static void CFGDB_GROUP_Mgr_Thread_Function_Entry(void* arg)
{
    SYSFUN_MsgQ_T           ipc_msgq_handle;
    L_THREADGRP_Handle_T    tg_handle = CORE_UTIL_PROC_COMM_GetCFGDB_GROUPTGHandle();
    UI32_T                  member_id,received_events,local_events=SYSFUN_SYSTEM_EVENT_IPCMSG;
    SYSFUN_Msg_T            *msgbuf_p=(SYSFUN_Msg_T*)cfgdb_group_mgrtd_ipc_buf;
    BOOL_T                  need_resp;
    void*                   timer_id;

    /* join the thread group of CFGDB_GROUP
     */
    if(L_THREADGRP_Join(tg_handle, SYS_BLD_CFGDB_GROUP_MGR_THREAD_PRIORITY, &member_id)==FALSE)
    {
        printf("%s: L_THREADGRP_Join fail.\n", __FUNCTION__);
        return;
    }

    /* create mgr ipc message queue handle
     */
    if(SYSFUN_CreateMsgQ(SYS_BLD_CFGDB_GROUP_IPCMSGQ_KEY, SYSFUN_MSGQ_BIDIRECTIONAL, &ipc_msgq_handle)!=SYSFUN_OK)
    {
        printf("%s: SYSFUN_CreateMsgQ fail.\n", __FUNCTION__);
        return;
    }

    timer_id = SYSFUN_PeriodicTimer_Create();
    SYSFUN_PeriodicTimer_Start(timer_id, SYS_BLD_CFGDB_PERIODIC_POLLING_TICKS, CFGDB_TYPE_EVENT_TIMER);

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
    while(1)
    {
        SYSFUN_ReceiveEvent (SYSFUN_SYSTEM_EVENT_IPCFAIL|
                             SYSFUN_SYSTEM_EVENT_IPCMSG |
                             SYSFUN_SYSTEM_EVENT_SET_TRANSITION_MODE |
#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
                             SYSFUN_SYSTEM_EVENT_SW_WATCHDOG |
#endif
                             CFGDB_TYPE_EVENT_TIMER |
                             CFGDB_TYPE_EVENT_SHUTDOWN,
                             SYSFUN_EVENT_WAIT_ANY,
                             (local_events==0)?SYSFUN_TIMEOUT_WAIT_FOREVER:SYSFUN_TIMEOUT_NOWAIT,
                             &received_events);
        local_events |= received_events;

        if(local_events & SYSFUN_SYSTEM_EVENT_SET_TRANSITION_MODE)
        {
            CFGDB_GROUP_SetTransitionMode();
            local_events ^= SYSFUN_SYSTEM_EVENT_SET_TRANSITION_MODE;
            /* need not to do IPCFAIL recovery in transition mode
             */
            if(local_events & SYSFUN_SYSTEM_EVENT_IPCFAIL)
            {
                /* clear fail info in IPCFAIL
                 */
                local_events ^= SYSFUN_SYSTEM_EVENT_IPCFAIL;
            }

        }

        /* handle IPCMSG
         */
        if(local_events & SYSFUN_SYSTEM_EVENT_IPCMSG)
        {
            if(SYSFUN_ReceiveMsg(ipc_msgq_handle, 0, SYSFUN_TIMEOUT_NOWAIT,
                CFGDB_GROUP_PMGR_MSGBUF_MAX_REQUIRED_SIZE, msgbuf_p)==SYSFUN_OK)
            {
                /* request thread group execution permission
                 */
                if(L_THREADGRP_Execution_Request(tg_handle, member_id)!=TRUE)
                    printf("%s: L_THREADGRP_Execution_Request fail.\n", __FUNCTION__);

                /* handle request message based on cmd
                 */
                switch(msgbuf_p->cmd)
                {
                    /* module id cmd
                     */
                    case SYS_MODULE_SYS_CALLBACK:
                        /* SYS_CALLBACK ipc message can only be uni-direction
                         * just set need_resp as FALSE
                         */
                        need_resp=FALSE;
                        CFGDB_GROUP_HandleSysCallbackIPCMsg(msgbuf_p);
                        break;

                    case SYS_MODULE_BACKDOOR:
                        need_resp=BACKDOOR_MGR_HandleIPCMsg(msgbuf_p);
                        break;

                    /* global cmd
                     */
                    case SYS_TYPE_CMD_ENTER_MASTER_MODE:
                        CFGDB_GROUP_EnterMasterMode();

                        /* SYS_TYPE_CMD_ENTER_MASTER_MODE
                         * need a response which contains nothing
                         */
                        msgbuf_p->msg_size=0;
                        need_resp=TRUE;

                        /* IPCFAIL might happen when a upper layer cscgroup has
                         * entered transition mode but lower layer cscgroups still
                         * in master mode so async callback msg will keep sending
                         * from lower layer cscgroups. In this case, the IPCFAIL
                         * event can be ignored.
                         */
                        if(local_events & SYSFUN_SYSTEM_EVENT_IPCFAIL)
                        {
                            /* clear fail info in IPCFAIL
                             */
                            local_events ^= SYSFUN_SYSTEM_EVENT_IPCFAIL;
                        }

                        break;

                    case SYS_TYPE_CMD_ENTER_SLAVE_MODE:
                       CFGDB_GROUP_EnterSlaveMode();

                        /* SYS_TYPE_CMD_ENTER_SLAVE_MODE
                         * need a response which contains nothing
                         */
                        msgbuf_p->msg_size=0;
                        need_resp=TRUE;

                        /* IPCFAIL might happen when a upper layer cscgroup has
                         * entered transition mode but lower layer cscgroups still
                         * in master mode so async callback msg will keep sending
                         * from lower layer cscgroups. In this case, the IPCFAIL
                         * event can be ignored.
                         */
                        if(local_events & SYSFUN_SYSTEM_EVENT_IPCFAIL)
                        {
                            /* clear fail info in IPCFAIL
                             */
                            local_events ^= SYSFUN_SYSTEM_EVENT_IPCFAIL;
                        }
                        break;

                    case SYS_TYPE_CMD_ENTER_TRANSITION_MODE:

                        CFGDB_GROUP_EnterTransitionMode();

                        /* SYS_TYPE_CMD_ENTER_TRANSITION_MODE
                         * need a response which contains nothing
                         */
                        msgbuf_p->msg_size=0;
                        need_resp=TRUE;
                        break;
                    
                    case SYS_TYPE_CMD_PROVISION_COMPLETE:
                        CFGDB_GROUP_ProvisionComplete();
                        msgbuf_p->msg_size=0;
                        need_resp=TRUE;
                        break; 

                    case SYS_TYPE_CMD_MODULE_PROVISION_COMPLETE:
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
                    case SYS_TYPE_CMD_HANDLE_HOT_INSERTION:
                    case SYS_TYPE_CMD_HANDLE_HOT_REMOVAL:
#endif
                    case SYS_TYPE_CMD_RELOAD_SYSTEM:
                        msgbuf_p->msg_size=0;
                        need_resp=TRUE;
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
                        printf("%s: Invalid IPC req cmd.\n", __FUNCTION__);
                        need_resp=FALSE;
                }


                /* release thread group execution permission
                 */            
                if(L_THREADGRP_Execution_Release(tg_handle, member_id)!=TRUE)
                    printf("%s: L_THREADGRP_Execution_Release fail.\n", __FUNCTION__);


                if((need_resp==TRUE) && (SYSFUN_SendResponseMsg(ipc_msgq_handle, msgbuf_p)!=SYSFUN_OK))
                    printf("%s: SYSFUN_SendResponseMsg fail.\n", __FUNCTION__);

            } /* end of if(SYSFUN_ReceiveMsg... */
            else
                local_events ^= SYSFUN_SYSTEM_EVENT_IPCMSG;
        } /* end of if(local_events & SYSFUN_SYSTEM_EVENT_IPCMSG) */

        /* shutdown event
         */
        if (local_events & CFGDB_TYPE_EVENT_SHUTDOWN)
        {

            /* request thread group execution permission
             */
            if(L_THREADGRP_Execution_Request(tg_handle, member_id)!=TRUE)
                printf("%s: L_THREADGRP_Execution_Request fail.\n", __FUNCTION__);
            
            CFGDB_MGR_ShutdownHandler();
            
            /* release thread group execution permission
             */            
            if(L_THREADGRP_Execution_Release(tg_handle, member_id)!=TRUE)
                printf("%s: L_THREADGRP_Execution_Release fail.\n", __FUNCTION__);
            
            local_events ^= CFGDB_TYPE_EVENT_SHUTDOWN;

        }


        /* timer event
         */
        if (local_events & CFGDB_TYPE_EVENT_TIMER)
        {

            /* request thread group execution permission
             */
            if(L_THREADGRP_Execution_Request(tg_handle, member_id)!=TRUE)
                printf("%s: L_THREADGRP_Execution_Request fail.\n", __FUNCTION__);

            CFGDB_MGR_TimerHandler();
            
            /* release thread group execution permission
             */            
            if(L_THREADGRP_Execution_Release(tg_handle, member_id)!=TRUE)
                printf("%s: L_THREADGRP_Execution_Release fail.\n", __FUNCTION__);

            local_events ^= CFGDB_TYPE_EVENT_TIMER;
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
       	    SW_WATCHDOG_MGR_ResetTimer(SW_WATCHDOG_CFGDB_GROUP);
            local_events ^= SYSFUN_SYSTEM_EVENT_SW_WATCHDOG;
        }
#endif
    } /* end of while(1) */
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : CFGDB_GROUP_SetTransitionMode
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will invoke all set transition mode function in CFGDB_GROUP.
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
static void CFGDB_GROUP_SetTransitionMode(void)
{
    CFGDB_INIT_SetTransitionMode();
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : CFGDB_GROUP_EnterTransitionMode
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will invoke all enter transition mode function in CFGDB_GROUP.
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
static void CFGDB_GROUP_EnterTransitionMode(void)
{
    CFGDB_INIT_EnterTransitionMode();
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : CFGDB_GROUP_EnterMasterMode
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will invoke all set master mode function in CFGDB_GROUP.
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
static void CFGDB_GROUP_EnterMasterMode(void)
{    
    CFGDB_MGR_EnterMasterMode();
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : CFGDB_GROUP_EnterSlaveMode
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will invoke all enter slave mode function in CFGDB_GROUP.
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
static void CFGDB_GROUP_EnterSlaveMode(void)
{
    CFGDB_INIT_EnterSlaveMode();
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : CFGDB_GROUP_ProvisionComplete
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will invoke all ProvisionComplete function in CFGDB_GROUP.
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
static void CFGDB_GROUP_ProvisionComplete(void)
{
    CFGDB_INIT_ProvisionComplete();
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : CFGDB_GROUP_HandleSysCallbackIPCMsg
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will handle all callbacks from IPC messages in CFGDB_GROUP.
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
static void CFGDB_GROUP_HandleSysCallbackIPCMsg(SYSFUN_Msg_T *msgbuf_p)
{
    SYS_CALLBACK_MGR_Msg_T *sys_cbmsg_p = (SYS_CALLBACK_MGR_Msg_T*)msgbuf_p->msg_buf;

    switch(sys_cbmsg_p->callback_event_id)
    {
        /* Need to handle ISC_Register_Service_CallBack(ISC_CFGDB_SID, CFGDB_MGR_ReceivePacket_CallBack)
         */
        
        default:
            SYSFUN_Debug_Printf("%s: received callback_event that is not handled(%d)\n",
                __FUNCTION__, sys_cbmsg_p->callback_event_id);
    }
}
