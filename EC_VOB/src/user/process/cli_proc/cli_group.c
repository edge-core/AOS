/* MODULE NAME:  cli_group.c
 * PURPOSE:
 *     For CLI group
 *
 * NOTES:
 *
 * HISTORY
 *    7/07/2007 - Rich Lee, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "sysfun.h"
#include "sys_module.h"
#include "sys_cpnt.h"
#include "sys_bld.h"
#include "cli_group.h"
#include "cli_proc_comm.h"
#include "l_ipcio.h"
#include "sys_callback_mgr.h"
#include "backdoor_mgr.h"

#include "cli_task.h"
#include "cli_main.h"
#if (SYS_CPNT_CLI == TRUE)
#include "cli_mgr.h"
#include "cli_init.h"
#endif

#if (SYS_CPNT_CLI_ADD_TO_RUNNING_CONFIG == TRUE)
#include "cli_addrunconfig_task.h"
#endif

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
#include "sw_watchdog_mgr.h"
#endif

/* NAMING CONSTANT DECLARATIONS
 */
#define CLI_GROUP_PMGR_MSGBUF_MAX_REQUIRED_SIZE sizeof(CLI_GROUP_MGR_MSG_T)
/* union all data type used for CLI Group MGR IPC message to get the maximum
 * required ipc message buffer
 */
typedef union CLI_GROUP_MGR_MSG_U
{
#if (SYS_CPNT_CLI == TRUE)
    CLI_MGR_IPCMsg_T cli_mgr_ipcmsg;
#endif

#if (SYS_CPNT_CLI_DYNAMIC_PROVISION_VIA_DHCP== TRUE)
	struct
    {
        SYS_CALLBACK_MGR_Msg_T sys_callback_msg_header;
        union
        {
            SYS_CALLBACK_MGR_DHCP_RxOptionConfig_CBData_T dhcp_rx_option;
        } sys_callback_msg_data;
    } sys_callback_msg;
#endif

    BACKDOOR_MGR_Msg_T   backdoor_mgr_ipcmsg;
    L_IPCIO_Msg_T        l_ipcio_ipcmsg;
} CLI_GROUP_MGR_MSG_T;

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void CLI_GROUP_Mgr_Thread_Function_Entry(void* arg);
#if 0 /* XXX steven.jiang for warnings */
static void CLI_GROUP_SetTransitionMode(void);
static void CLI_GROUP_EnterTransitionMode(void);
static void CLI_GROUP_EnterMasterMode(void);
static void CLI_GROUP_EnterSlaveMode(void);
static void CLI_GROUP_HandleSysCallbackIPCMsg(SYSFUN_Msg_T *msgbuf_p);
#endif /* 0 */

/* STATIC VARIABLE DECLARATIONS
 */
/* the buffer for retrieving ipc request for CLI_GROUP mgr thread
 */
static UI8_T CLI_GROUP_mgrtd_ipc_buf[SYSFUN_SIZE_OF_MSG(CLI_GROUP_PMGR_MSGBUF_MAX_REQUIRED_SIZE)];

/* EXPORTED SUBPROGRAM BODIES
 */
/*------------------------------------------------------------------------------
 * ROUTINE NAME : CLI_GROUP_Create_All_Threads
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will spawn all threads in CLI_GROUP.
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
void CLI_GROUP_Create_All_Threads(void)
{
    UI32_T thread_id;

#if (SYS_CPNT_CLI == TRUE)
    CLI_INIT_Create_Tasks();
#endif

/* temporarily defintion
 */
#define SYS_BLD_CLI_GROUP_MGR_THREAD_PRIORITY   SYS_BLD_CLI_THREAD_PRIORITY
#define SYS_BLD_CLI_GROUP_MGR_SCHED_POLICY      SYS_BLD_CLI_THREAD_SCHED_POLICY
#define SYS_BLD_CLI_GROUP_MGR_THREAD            "CLI_GROUP"

    if(SYSFUN_SpawnThread(SYS_BLD_CLI_GROUP_MGR_THREAD_PRIORITY,
                          SYS_BLD_CLI_GROUP_MGR_SCHED_POLICY,
                          SYS_BLD_CLI_GROUP_MGR_THREAD,
                          SYS_BLD_CLI_TASK_STACK_SIZE,
                          SYSFUN_TASK_FP,
                          CLI_GROUP_Mgr_Thread_Function_Entry,
                          NULL, &thread_id)!=SYSFUN_OK)
    {
        printf("%s:Spawn CLI_GROUP MGR thread fail.\n", __FUNCTION__);
    }

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
    SW_WATCHDOG_MGR_RegisterMonitorThread(SW_WATCHDOG_CLI_GROUP, thread_id, SYS_ADPT_CLI_GROUP_SW_WATCHDOG_TIMER);
#endif

}

/* LOCAL SUBPROGRAM BODIES
 */
static void CLI_GROUP_Mgr_Thread_Function_Entry(void* arg)
{
    SYSFUN_Msg_T    *msgbuf_p=(SYSFUN_Msg_T*)CLI_GROUP_mgrtd_ipc_buf;
    static UI32_T   local_events  = SYSFUN_SYSTEM_EVENT_IPCMSG;
    UI32_T          received_events,sysfun_ret;
    SYSFUN_MsgQ_T   ipc_msgq_handle;
    BOOL_T          need_resp;


    /* create mgr ipc message queue handle
     */
    if(SYSFUN_CreateMsgQ(SYS_BLD_CLI_GROUP_IPCMSGQ_KEY, SYSFUN_MSGQ_BIDIRECTIONAL, &ipc_msgq_handle)!=SYSFUN_OK)
    {
        printf("%s: SYSFUN_CreateMsgQ fail.\n", __FUNCTION__);
        return;
    }

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
#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
                             SYSFUN_SYSTEM_EVENT_SW_WATCHDOG |
#endif
                             SYSFUN_SYSTEM_EVENT_SET_TRANSITION_MODE,
                             SYSFUN_EVENT_WAIT_ANY,
                             (local_events==0)?SYSFUN_TIMEOUT_WAIT_FOREVER:SYSFUN_TIMEOUT_NOWAIT,
                             &received_events);
        local_events |= received_events;

        if(local_events & SYSFUN_SYSTEM_EVENT_SET_TRANSITION_MODE)
        {
            CLI_INIT_SetTransitionMode();
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
            sysfun_ret = SYSFUN_ReceiveMsg(ipc_msgq_handle, 0, SYSFUN_TIMEOUT_NOWAIT,
                CLI_GROUP_PMGR_MSGBUF_MAX_REQUIRED_SIZE, msgbuf_p);
            if(sysfun_ret == SYSFUN_OK)
            {

                /* handle request message based on cmd
                 */
                switch(msgbuf_p->cmd)
                {
                    /* module id cmd
                     */
#if (SYS_CPNT_CLI == TRUE)
                    case SYS_MODULE_CLI:
                        need_resp=CLI_MGR_HandleIPCReqMsg(msgbuf_p);
                        break;
#endif
                    case SYS_MODULE_SYS_CALLBACK:
                        /* SYS_CALLBACK ipc message can only be uni-direction
                         * just set need_resp as FALSE
                         */
                        CLI_MGR_HandleSysCallbackIPCMsg(msgbuf_p);
                        need_resp=FALSE;
                        break;

                    case SYS_MODULE_BACKDOOR:
                        need_resp = BACKDOOR_MGR_HandleIPCMsg(msgbuf_p);
                        break;

                    case SYS_TYPE_CMD_IPCIO:
                        need_resp = CLI_MAIN_HandleIPCIOMsg(msgbuf_p);
                        break;

                    case SYS_TYPE_CMD_PROVISION_COMPLETE:
                    case SYS_TYPE_CMD_MODULE_PROVISION_COMPLETE:
                        msgbuf_p->msg_size = 0;
                        need_resp = TRUE;
                        break;

                    case SYS_TYPE_CMD_ENTER_MASTER_MODE:
                        CLI_INIT_EnterMasterMode();
                        SYSFUN_Debug_Printf("\nCLI enter master mode\n");
                        /* SYS_TYPE_CMD_ENTER_MASTER_MODE
                         * need a response which contains nothing
                         */
                        msgbuf_p->msg_size=0;
                        need_resp=TRUE;

#if (SYS_CPNT_CLI_ADD_TO_RUNNING_CONFIG == TRUE)
                        CLI_ADDRUNCONFIG_CreateTask();
#endif

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
                        CLI_INIT_EnterSlaveMode();

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
                        CLI_INIT_EnterTransitionMode();

                        /* SYS_TYPE_CMD_ENTER_TRANSITION_MODE
                         * need a response which contains nothing
                         */
                        msgbuf_p->msg_size=0;
                        need_resp=TRUE;
                        break;


#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
                    case SYS_TYPE_CMD_HANDLE_HOT_INSERTION:
                        CLI_MAIN_HandleHotInertion(msgbuf_p);
                        msgbuf_p->msg_size=0;
                        need_resp=TRUE;
                        break;

                    case SYS_TYPE_CMD_HANDLE_HOT_REMOVAL:
                        CLI_MAIN_HandleHotRemoval(msgbuf_p);
                        msgbuf_p->msg_size=0;
                        need_resp=TRUE;
                        break;
#endif
/*add by fen.wang,to process reload */
                    case  SYS_TYPE_CMD_RELOAD_SYSTEM:
                        msgbuf_p->msg_size=0;
                        need_resp=TRUE;
/*jingyan zheng add for defect ES3628BT-FLF-ZZ-00275*/
#if (SYS_CPNT_TELNET == TRUE)
                        CLI_TASK_CloseTelnetSessions(NULL);
#endif
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
                        SYSFUN_Debug_Printf("%s: Invalid IPC req cmd.\n", __FUNCTION__);
                        need_resp=FALSE;
                }

                if((need_resp==TRUE) && (SYSFUN_SendResponseMsg(ipc_msgq_handle, msgbuf_p)!=SYSFUN_OK))
                    SYSFUN_Debug_Printf("%s: SYSFUN_SendResponseMsg fail.\n", __FUNCTION__);

            } /* end of if(SYSFUN_ReceiveMsg... */
            else
                local_events ^= SYSFUN_SYSTEM_EVENT_IPCMSG;
        } /* end of if(local_events & SYSFUN_SYSTEM_EVENT_IPCMSG) */

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
       	    SW_WATCHDOG_MGR_ResetTimer(SW_WATCHDOG_CLI_GROUP);
            local_events ^= SYSFUN_SYSTEM_EVENT_SW_WATCHDOG;
        }
#endif
    }
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : CLI_GROUP_HandleSysCallbackIPCMsg
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will handle all callbacks from IPC messages in CLI_GROUP.
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
#if 0 /* XXX steven.jiang for warnings */
static void CLI_GROUP_HandleSysCallbackIPCMsg(SYSFUN_Msg_T *msgbuf_p)
{
}
#endif /* 0 */

