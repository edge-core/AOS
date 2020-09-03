/* MODULE NAME:  ssh_group.c
 * PURPOSE:
 *     For SSH group
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
#include "sys_cpnt.h"
#include "sys_bld.h"
#include "sys_module.h"
#include "ssh_group.h"
#include "cli_proc_comm.h"
#include "sys_callback_mgr.h"
#include "backdoor_mgr.h"

#if (SYS_CPNT_SSH2 == TRUE)
    #include "sshd_mgr.h"
    #include "sshd_init.h"
#endif

#if (SYS_CPNT_KEYGEN == TRUE)
    #include "keygen_mgr.h"
    #include "keygen_init.h"
#endif

#if (SYS_CPNT_XFER == TRUE)
    #include "xfer_pmgr.h"
#endif

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
#include "sw_watchdog_mgr.h"
#endif

/* NAMING CONSTANT DECLARATIONS
 */
#define SSH_GROUP_PMGR_MSGBUF_MAX_REQUIRED_SIZE sizeof(SSH_GROUP_MGR_MSG_T)
/* union all data type used for ssh Group MGR IPC message to get the maximum
 * required ipc message buffer
 */
typedef union SSH_GROUP_MGR_MSG_U
{
#if (SYS_CPNT_SSH2 == TRUE)
    SSHD_MGR_IPCMsg_T   sshd_mgr_ipcmsg;
#endif

#if (SYS_CPNT_KEYGEN == TRUE)
    KEYGEN_MGR_IPCMsg_T keygen_mgr_ipcmsg;
#endif

    BACKDOOR_MGR_Msg_T  backdoor_mgr_ipcmsg;
} SSH_GROUP_MGR_MSG_T;

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void SSH_GROUP_Mgr_Thread_Function_Entry(void* arg);
static void SSH_GROUP_SetTransitionMode(void);
static void SSH_GROUP_EnterTransitionMode(void);
static void SSH_GROUP_EnterMasterMode(void);
static void SSH_GROUP_EnterSlaveMode(void);
static void SSH_GROUP_ProvisionComplete(void);



/* STATIC VARIABLE DECLARATIONS
 */
/* the buffer for retrieving ipc request for SSH_GROUP mgr thread
 */
static UI8_T SSH_GROUP_mgrtd_ipc_buf[SYSFUN_SIZE_OF_MSG(SSH_GROUP_PMGR_MSGBUF_MAX_REQUIRED_SIZE)];

/* EXPORTED SUBPROGRAM BODIES
 */
/* XXX steven.jiang for warnings */
void   SSHD_INIT_InitiateSystemResources(void);
void   KEYGEN_INIT_InitiateSystemResources(void);
BOOL_T KEYGEN_MGR_HandleIPCReqMsg(SYSFUN_Msg_T *ipcmsg_p);

/*------------------------------------------------------------------------
 * ROUTINE NAME - SSH_GROUP_InitiateProcessResources
 *------------------------------------------------------------------------
 * FUNCTION: This function will initialize kernel resources
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : Invoked by root.c()
 *------------------------------------------------------------------------*/
void SSH_GROUP_InitiateProcessResources (void)
{
#if (SYS_CPNT_SSH2 == TRUE)
    SSHD_INIT_InitiateSystemResources();
#endif

#if (SYS_CPNT_KEYGEN == TRUE)
    KEYGEN_INIT_InitiateSystemResources();
#endif

    XFER_PMGR_InitiateProcessResource();
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SSH_GROUP_Create_InterCSC_Relation
 *-----------------------------------------------------------------------------
 * PURPOSE : Create inter-CSC relationships for SSH_Group.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
void SSH_GROUP_Create_InterCSC_Relation(void)
{
#if (SYS_CPNT_SSH2 == TRUE)
    SSHD_INIT_Create_InterCSC_Relation();
#endif

#if (SYS_CPNT_KEYGEN == TRUE)
    KEYGEN_INIT_Create_InterCSC_Relation();
#endif

}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SSH_GROUP_Create_All_Threads
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will spawn all threads in SSH_GROUP.
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
void SSH_GROUP_Create_All_Threads(void)
{
    UI32_T thread_id;

#if (SYS_CPNT_SSH2 == TRUE)
    SSHD_INIT_Create_Tasks();
#endif

#if (SYS_CPNT_KEYGEN == TRUE)
    KEYGEN_INIT_Create_Tasks();
#endif

    if(SYSFUN_SpawnThread(SYS_BLD_SSH_GROUP_MGR_THREAD_PRIORITY,
                          SYS_BLD_SSH_GROUP_MGR_SCHED_POLICY,
                          (char *)SYS_BLD_SSH_GROUP_MGR_THREAD_NAME,
#ifdef ES3526MA_POE_7LF_LN
                          (SYS_BLD_TASK_COMMON_STACK_SIZE * 5),
#else
                          SYS_BLD_TASK_COMMON_STACK_SIZE,
#endif
                          SYSFUN_TASK_FP,
                          SSH_GROUP_Mgr_Thread_Function_Entry,
                          NULL, &thread_id)!=SYSFUN_OK)
    {
        printf("%s:Spawn SSH_GROUP MGR thread fail.\n", __FUNCTION__);
    }

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
    SW_WATCHDOG_MGR_RegisterMonitorThread(SW_WATCHDOG_SSH_GROUP, thread_id, SYS_ADPT_SSH_GROUP_SW_WATCHDOG_TIMER);
#endif

}

/* LOCAL SUBPROGRAM BODIES
 */
/*------------------------------------------------------------------------------
 * ROUTINE NAME : SSH_GROUP_Mgr_Thread_Function_Entry
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This is the entry function for the MGR thread of SSH_GROUP.
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
static void SSH_GROUP_Mgr_Thread_Function_Entry(void* arg)
{
    SYSFUN_MsgQ_T           ipc_msgq_handle;
    UI32_T                  member_id,received_events,local_events=SYSFUN_SYSTEM_EVENT_IPCMSG;
    L_THREADGRP_Handle_T    tg_handle = CLI_PROC_COMM_GetSSH_GROUPTGHandle();
    SYSFUN_Msg_T            *msgbuf_p=(SYSFUN_Msg_T*)SSH_GROUP_mgrtd_ipc_buf;
    BOOL_T                  need_resp;

    /* join the thread group of SSH_GROUP
     */
    if(L_THREADGRP_Join(tg_handle, SYS_BLD_SSH_GROUP_MGR_THREAD_PRIORITY, &member_id)==FALSE)
    {
        printf("%s: L_THREADGRP_Join fail.\n", __FUNCTION__);
        return;
    }

    /* create mgr ipc message queue handle
     */
    if(SYSFUN_CreateMsgQ(SYS_BLD_SSH_GROUP_IPCMSGQ_KEY, SYSFUN_MSGQ_BIDIRECTIONAL, &ipc_msgq_handle)!=SYSFUN_OK)
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
            SSH_GROUP_SetTransitionMode();
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
                SSH_GROUP_PMGR_MSGBUF_MAX_REQUIRED_SIZE, msgbuf_p)==SYSFUN_OK)
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
#if (SYS_CPNT_SSH2 == TRUE)
                    case SYS_MODULE_SSH:
                        need_resp=SSHD_MGR_HandleIPCReqMsg(msgbuf_p);
                        break;
#endif

#if (SYS_CPNT_KEYGEN == TRUE)
                    case SYS_MODULE_KEYGEN:
                        need_resp=KEYGEN_MGR_HandleIPCReqMsg(msgbuf_p);
                        break;
#endif

                    case SYS_MODULE_SYS_CALLBACK:
                        /* SYS_CALLBACK ipc message can only be uni-direction
                         * just set need_resp as FALSE
                         */
                        need_resp=FALSE;

                        /* doesn't need any callback handler, yet.
                         * SSH_GROUP_HandleSysCallbackIPCMsg(msgbuf_p);
                         */
                        break;

                    case SYS_MODULE_BACKDOOR:
                        need_resp=BACKDOOR_MGR_HandleIPCMsg(msgbuf_p);
                        break;

                    /* global cmd
                     */
                    case SYS_TYPE_CMD_ENTER_MASTER_MODE:
                        SSH_GROUP_EnterMasterMode();

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
                        SSH_GROUP_EnterSlaveMode();

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
                        SSH_GROUP_EnterTransitionMode();

                        /* SYS_TYPE_CMD_ENTER_TRANSITION_MODE
                         * need a response which contains nothing
                         */
                        msgbuf_p->msg_size=0;
                        need_resp=TRUE;
                        break;

                    case SYS_TYPE_CMD_PROVISION_COMPLETE:
                        /* SYS_TYPE_CMD_PROVISION_COMPLETE
                         * need a response which contains nothing
                         */
                        SSH_GROUP_ProvisionComplete();
                        msgbuf_p->msg_size=0;
                        need_resp=TRUE;
                        break;
                        
                    case SYS_TYPE_CMD_MODULE_PROVISION_COMPLETE:
 #if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
                    case SYS_TYPE_CMD_HANDLE_HOT_INSERTION:
                    case SYS_TYPE_CMD_HANDLE_HOT_REMOVAL:
#endif
                    case  SYS_TYPE_CMD_RELOAD_SYSTEM:
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
       	    SW_WATCHDOG_MGR_ResetTimer(SW_WATCHDOG_SSH_GROUP);
            local_events ^= SYSFUN_SYSTEM_EVENT_SW_WATCHDOG;
        }
#endif


    } /* end of while(1) */
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SSH_GROUP_SetTransitionMode
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will invoke all set transition mode function in SSH_GROUP.
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
static void SSH_GROUP_SetTransitionMode(void)
{
#if (SYS_CPNT_SSH2 == TRUE)
    SSHD_INIT_SetTransitionMode();
#endif

#if (SYS_CPNT_KEYGEN == TRUE)
    KEYGEN_INIT_SetTransitionMode();
#endif
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SSH_GROUP_EnterTransitionMode
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will invoke all enter transition mode function in SSH_GROUP.
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
static void SSH_GROUP_EnterTransitionMode(void)
{
#if (SYS_CPNT_SSH2 == TRUE)
    SSHD_INIT_EnterTransitionMode();
#endif

#if (SYS_CPNT_KEYGEN == TRUE)
    KEYGEN_INIT_EnterTransitionMode();
#endif
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SSH_GROUP_EnterMasterMode
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will invoke all set master mode function in SSH_GROUP.
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
static void SSH_GROUP_EnterMasterMode(void)
{
#if (SYS_CPNT_SSH2 == TRUE)
    SSHD_INIT_EnterMasterMode();
#endif

#if (SYS_CPNT_KEYGEN == TRUE)
    KEYGEN_INIT_EnterMasterMode();
#endif
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SSH_GROUP_EnterSlaveMode
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will invoke all enter slave mode function in SSH_GROUP.
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
static void SSH_GROUP_EnterSlaveMode(void)
{
#if (SYS_CPNT_SSH2 == TRUE)
    SSHD_INIT_EnterSlaveMode();
#endif

#if (SYS_CPNT_KEYGEN == TRUE)
    KEYGEN_INIT_EnterSlaveMode();
#endif
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SSH_GROUP_ProvisionComplete
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will invoke all provision complete function in CSCGroup1.
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
static void SSH_GROUP_ProvisionComplete(void)
{
#if (SYS_CPNT_SSH2 == TRUE)
    SSHD_INIT_ProvisionComplete();
#endif

#if (SYS_CPNT_KEYGEN == TRUE)
    KEYGEN_INIT_ProvisionComplete();
#endif
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SSH_GROUP_HandleSysCallbackIPCMsg
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will handle all callbacks from IPC messages in SSH_GROUP.
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
static void SSH_GROUP_HandleSysCallbackIPCMsg(SYSFUN_Msg_T *msgbuf_p)
{
}
#endif /* 0 */

