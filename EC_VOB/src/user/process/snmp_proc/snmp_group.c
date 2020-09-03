/* MODULE NAME:  snmp_group.c
 * PURPOSE:
 *     This file is a demonstration code for implementations of csc group.
 *
 * NOTES:
 *
 * HISTORY
 *    6/12/2007 - Eli Lin, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */
 
/* INCLUDE FILE DECLARATIONS
 */
#include "sysfun.h"
#include "sys_cpnt.h"
#include "sys_bld.h"
#include "snmp_group.h"
#include "snmp_proc_comm.h"
#include "sys_callback_mgr.h"
#include "sys_module.h"
#include "backdoor_mgr.h"

#if (SYS_CPNT_SNMP == TRUE)
#include "snmp_mgr.h"
#include "snmp_init.h"
#endif

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
#include "sw_watchdog_mgr.h"
#endif

/* NAMING CONSTANT DECLARATIONS
 */
/* union all data type used for snmp group MGR IPC message to get the maximum
 * required ipc message buffer
 */
typedef union SNMP_GROUP_MGR_MSG_U
{
#if (SYS_CPNT_SNMP == TRUE)
    SNMP_MGR_IPCMsg_T   snmp_mgr_ipcmsg;
#endif

    BACKDOOR_MGR_Msg_T  backdoor_mgr_ipcmsg;
} SNMP_GROUP_MGR_MSG_T;

/* MACRO FUNCTION DECLARATIONS
 */
#define SNMP_GROUP_PMGR_MSGBUF_MAX_REQUIRED_SIZE sizeof(SNMP_GROUP_MGR_MSG_T)

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void SNMP_GROUP_Mgr_Thread_Function_Entry(void* arg);
static void SNMP_GROUP_SetTransitionMode(void);
static void SNMP_GROUP_EnterTransitionMode(void);
static void SNMP_GROUP_EnterMasterMode(void);
static void SNMP_GROUP_EnterSlaveMode(void);
static void SNMP_GROUP_HandleSysCallbackIPCMsg(SYSFUN_Msg_T *msgbuf_p);
#if (SYS_CPNT_CLUSTER == TRUE)
static void SNMP_GROUP_ClusterChangeRoleCallbackHandler(UI32_T role);
#endif /* #if (SYS_CPNT_CLUSTER == TRUE) */
/* STATIC VARIABLE DECLARATIONS
 */
/* the buffer for retrieving ipc request for SNMP_GROUP mgr thread
 */
static UI8_T snmpgroup_mgrtd_ipc_buf[SYSFUN_SIZE_OF_MSG(SNMP_GROUP_PMGR_MSGBUF_MAX_REQUIRED_SIZE)];

/* EXPORTED SUBPROGRAM BODIES
 */
/*------------------------------------------------------------------------------
 * ROUTINE NAME : SNMP_GROUP_Create_All_Threads
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will spawn all threads in SNMP_GROUP.
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
void SNMP_GROUP_Create_All_Threads(void)
{
    UI32_T thread_id;

/* SNMP belongs to SNMP_GROUP
 */
#if (SYS_CPNT_SNMP == TRUE)
    SNMP_INIT_Create_Tasks(); 
#endif

if(SYSFUN_SpawnThread(SYS_BLD_SNMP_GROUP_MGR_THREAD_PRIORITY,
                          SYS_BLD_SNMP_GROUP_MGR_SCHED_POLICY,
                          SYS_BLD_SNMP_GROUP_MGR_THREAD_NAME,
                          SYS_BLD_TASK_COMMON_STACK_SIZE,
                          SYSFUN_TASK_FP,
                          SNMP_GROUP_Mgr_Thread_Function_Entry,
                          NULL, &thread_id)!=SYSFUN_OK)
    {
        printf("%s:Spawn SNMP_GROUP MGR thread fail.\n", __FUNCTION__);
    }

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
    SW_WATCHDOG_MGR_RegisterMonitorThread(SW_WATCHDOG_SNMP_GROUP, thread_id, SYS_ADPT_SNMP_GROUP_SW_WATCHDOG_TIMER);
#endif

}

/* LOCAL SUBPROGRAM BODIES
 */
/*------------------------------------------------------------------------------
 * ROUTINE NAME : SNMP_GROUP_Mgr_Thread_Function_Entry
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This is the entry function for the MGR thread of SNMP_GROUP.
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
static void SNMP_GROUP_Mgr_Thread_Function_Entry(void* arg)
{
    SYSFUN_MsgQ_T        ipc_msgq_handle;
    L_THREADGRP_Handle_T tg_handle = SNMP_PROC_COMM_GetSnmpGroupTGHandle();
    UI32_T               member_id,received_events,local_events=SYSFUN_SYSTEM_EVENT_IPCMSG;
    SYSFUN_Msg_T         *msgbuf_p=(SYSFUN_Msg_T*)snmpgroup_mgrtd_ipc_buf;
    BOOL_T               need_resp;

    /* Set proper priority
     */
    if(SYSFUN_OK!=SYSFUN_SetTaskPriority(SYS_BLD_SNMP_GROUP_MGR_SCHED_POLICY,
        SYSFUN_TaskIdSelf(), SYS_BLD_SNMP_GROUP_MGR_THREAD_PRIORITY))
    {
        printf("\r\n%s:SYSFUN_SetTaskPriority fail", __FUNCTION__);
    }

    /* join the thread group of SNMP_GROUP
     */
    if(L_THREADGRP_Join(tg_handle, SYS_BLD_SNMP_GROUP_MGR_THREAD_PRIORITY, &member_id)==FALSE)
    {
        printf("%s: L_THREADGRP_Join fail.\n", __FUNCTION__);
        return;
    }

    /* create mgr ipc message queue handle
     */
    if(SYSFUN_CreateMsgQ(SYS_BLD_SNMP_GROUP_IPCMSGQ_KEY, SYSFUN_MSGQ_BIDIRECTIONAL, &ipc_msgq_handle)!=SYSFUN_OK)
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
            SNMP_GROUP_SetTransitionMode();
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
                SNMP_GROUP_PMGR_MSGBUF_MAX_REQUIRED_SIZE, msgbuf_p)==SYSFUN_OK)
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
#if (SYS_CPNT_SNMP == TRUE)
                    case SYS_MODULE_SNMP:
                        need_resp=SNMP_MGR_HandleIPCReqMsg(msgbuf_p);
                        break;
#endif

                    case SYS_MODULE_SYS_CALLBACK:
                        /* SYS_CALLBACK ipc message can only be uni-direction
                         * just set need_resp as FALSE
                         */
                        need_resp=FALSE;
                        SNMP_GROUP_HandleSysCallbackIPCMsg(msgbuf_p);
                        break;

                    case SYS_MODULE_BACKDOOR:
                        need_resp=BACKDOOR_MGR_HandleIPCMsg(msgbuf_p);
                        break;
                        
                    /* global cmd
                     */
                    case SYS_TYPE_CMD_ENTER_MASTER_MODE:
                        SNMP_GROUP_EnterMasterMode();

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
                        SNMP_GROUP_EnterSlaveMode();

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
                        SNMP_GROUP_EnterTransitionMode();

                        /* SYS_TYPE_CMD_ENTER_TRANSITION_MODE
                         * need a response which contains nothing
                         */
                        msgbuf_p->msg_size=0;
                        need_resp=TRUE;
                        break;

                    case SYS_TYPE_CMD_PROVISION_COMPLETE:
                    case SYS_TYPE_CMD_MODULE_PROVISION_COMPLETE:
                        /* SYS_TYPE_CMD_PROVISION_COMPLETE
                         * need a response which contains nothing
                         */
                        SNMP_INIT_ProvisionComplete();
                        msgbuf_p->msg_size=0;
                        need_resp=TRUE;
                        break;

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
                     case  SYS_TYPE_CMD_HANDLE_HOT_INSERTION:
                         msgbuf_p->msg_size=0;
                        need_resp=TRUE;
                        break;
                        
                     case SYS_TYPE_CMD_HANDLE_HOT_REMOVAL:                      
                        msgbuf_p->msg_size=0;
                        need_resp=TRUE;
                         break;
#endif
                    /*add by fen.wang ,to process system reload msg,it is sent by stkctrl task*/
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
       if (local_events & SYSFUN_SYSTEM_EVENT_SW_WATCHDOG)
       {
       	   SW_WATCHDOG_MGR_ResetTimer(SW_WATCHDOG_SNMP_GROUP);
           local_events ^= SYSFUN_SYSTEM_EVENT_SW_WATCHDOG;
       }
#endif

    } /* end of while(1) */
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SNMP_GROUP_SetTransitionMode
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will invoke all set transition mode function in SNMP_GROUP.
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
static void SNMP_GROUP_SetTransitionMode(void)
{
#if (SYS_CPNT_SNMP == TRUE)
   SNMP_INIT_SetTransitionMode();
#endif
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SNMP_GROUP_EnterTransitionMode
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will invoke all enter transition mode function in SNMPGroup.
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
static void SNMP_GROUP_EnterTransitionMode(void)
{
#if (SYS_CPNT_SNMP == TRUE)
    SNMP_INIT_EnterTransitionMode();
#endif
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SNMP_GROUP_EnterMasterMode
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will invoke all set master mode function in SNMPGroup.
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
static void SNMP_GROUP_EnterMasterMode(void)
{
#if (SYS_CPNT_SNMP == TRUE)
    SNMP_INIT_EnterMasterMode();
#endif
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SNMPRGROUP_EnterSlaveMode
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will invoke all enter slave mode function in SNMPGroup.
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
static void SNMP_GROUP_EnterSlaveMode(void)
{
#if (SYS_CPNT_SNMP == TRUE)
    SNMP_INIT_EnterSlaveMode();
#endif
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SNMP_GROUP_HandleSysCallbackIPCMsg
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will handle all callbacks from IPC messages in SNMP_GROUP.
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
static void SNMP_GROUP_HandleSysCallbackIPCMsg(SYSFUN_Msg_T *msgbuf_p)
{
    SYS_CALLBACK_MGR_Msg_T *sys_cbmsg_p = (SYS_CALLBACK_MGR_Msg_T*)msgbuf_p->msg_buf;

    switch(sys_cbmsg_p->callback_event_id)
    {
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_INTRUSION_MAC:
#if (SYS_CPNT_CSCA == TRUE)
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &CSCGROUP1_IntrusionMacCallbackHandler);
#endif
            break;

#if (SYS_CPNT_CLUSTER == TRUE)
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_CLUSTER_CHANGEROLE:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &SNMP_GROUP_ClusterChangeRoleCallbackHandler);
            break;
#endif /* #if (SYS_CPNT_CLUSTER == TRUE) */

        default:
            SYSFUN_Debug_Printf("%s: received callback_event that is not handled(%d)\n",
                __FUNCTION__, sys_cbmsg_p->callback_event_id);
    }
}

#if (SYS_CPNT_CLUSTER == TRUE)
/* FUNCTION NAME:  SNMP_GROUP_ClusterChangeRoleCallBackHandler
 * PURPOSE : Handle the callback event happening when a cluster role is changed.
 *
 * INPUT   : role -- specify the role cluster has changed
 *           
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *
 */
static void SNMP_GROUP_ClusterChangeRoleCallbackHandler(UI32_T role)
{
    SNMP_MGR_SetClusterRole(role);
    return;
}  
#endif /* #if (SYS_CPNT_CLUSTER == TRUE) */
