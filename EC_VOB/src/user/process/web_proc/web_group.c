/* MODULE NAME:  webgroup.c
 * PURPOSE:
 *     For WEB group
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
#include "web_group.h"
#include "web_proc_comm.h"
#include "sys_callback_mgr.h"
#include "backdoor_mgr.h"

#if (SYS_CPNT_HTTP == TRUE)
#include "http_mgr.h"
#include "http_init.h"
#endif

#if (SYS_CPNT_WEBAUTH == TRUE)
#include "webauth_mgr.h"
#include "webauth_init.h"
#endif

#if (SYS_CPNT_MGMT_IP_FLT == TRUE)
#include "mgmt_ip_flt.h"
#endif

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
#include "sw_watchdog_mgr.h"
#endif

#include "cgi_init.h"

/* NAMING CONSTANT DECLARATIONS
 */
/* the size of the ipc msg buf for CSCGroup2 MGR thread to receive and response
 * PMGR ipc. The size of this buffer should pick the maximum of size required
 * for PMGR ipc request and response in all CSCs handled in this process.
 * (this size does not include IPC msg header)
 */
#define WEBGROUP_PMGR_MSGBUF_MAX_REQUIRED_SIZE sizeof(WEBGROUP_MGR_MSG_T)
/* union all data type used for WEBGroup MGR IPC message to get the maximum
 * required ipc message buffer
 */
typedef union WEBGROUP_MGR_MSG_U
{
#if (SYS_CPNT_HTTP == TRUE)
    HTTP_MGR_IPCMsg_T         http_mgr_ipcmsg;
#endif

#if (SYS_CPNT_WEBAUTH == TRUE)
    WEBAUTH_MGR_IPCMsg_T      webauth_mgr_ipcmsg;
#endif

    BACKDOOR_MGR_Msg_T        backdoor_mgr_ipcmsg;

} WEBGROUP_MGR_MSG_T;

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void WEBGROUP_Mgr_Thread_Function_Entry(void* arg);
static void WEBGROUP_SetTransitionMode(void);
static void WEBGROUP_EnterTransitionMode(void);
static void WEBGROUP_EnterMasterMode(void);
static void WEBGROUP_EnterSlaveMode(void);
static void WEBGROUP_HandleSysCallbackIPCMsg(SYSFUN_Msg_T *msgbuf_p);

#if (SYS_CPNT_MGMT_IP_FLT == TRUE)
static void WEB_GROUP_MgmtIPFltChangedCallbackHandler(UI32_T mode);
#endif

static void WEB_GROUP_RifDestroyedCallbackHandler(UI32_T ifindex, L_INET_AddrIp_T *addr_p);

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
static void WEB_GROUP_HandleHotInertion(SYSFUN_Msg_T* msgbuf_p);
static void WEB_GROUP_HandleHotRemoval(SYSFUN_Msg_T* msgbuf_p);
#endif /* SYS_CPNT_UNIT_HOT_SWAP */

/* STATIC VARIABLE DECLARATIONS
 */
/* the buffer for retrieving ipc request for WEBGROUP mgr thread
 */
static UI8_T WEBGROUP_mgrtd_ipc_buf[SYSFUN_SIZE_OF_MSG(WEBGROUP_PMGR_MSGBUF_MAX_REQUIRED_SIZE)];

/* EXPORTED SUBPROGRAM BODIES
 */
/*-----------------------------------------------------------------------------
 * FUNCTION NAME - WEB_GROUP_Create_InterCSC_Relation
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *    Create inter-CSC relationships for WEBGROUP.
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
 *-----------------------------------------------------------------------------
 */
void WEB_GROUP_Create_InterCSC_Relation(void)
{
    HTTP_INIT_Create_InterCSC_Relation();
    CGI_INIT_Create_InterCSC_Relation();
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : WEBGROUP_Create_All_Threads
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will spawn all threads in WEBGROUP.
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
void WEBGROUP_Create_All_Threads(void)
{
    UI32_T thread_id;

#if (SYS_CPNT_HTTP == TRUE)
    HTTP_INIT_Create_Tasks();
#endif

#if (SYS_CPNT_WEBAUTH == TRUE)
    WEBAUTH_INIT_CreateTasks();
#endif

    CGI_INIT_CreateTasks();

    if(SYSFUN_SpawnThread(SYS_BLD_WEBGROUP_MGR_THREAD_PRIORITY,
                          SYS_BLD_WEBGROUP_MGR_SCHED_POLICY,
                          SYS_BLD_WEB_GROUP_MGR_THREAD_NAME,/*SYS_BLD_WEBGROUP_MGR_THREAD_NAME,*/
                          SYS_BLD_TASK_COMMON_STACK_SIZE,
                          SYSFUN_TASK_FP,
                          WEBGROUP_Mgr_Thread_Function_Entry,
                          NULL, &thread_id)!=SYSFUN_OK)
    {
        printf("%s:Spawn WEBGROUP MGR thread fail.\n", __FUNCTION__);
    }

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
    SW_WATCHDOG_MGR_RegisterMonitorThread(SW_WATCHDOG_WEB_GROUP, thread_id, SYS_ADPT_WEB_GROUP_SW_WATCHDOG_TIMER);
#endif

}


void WEB_GROUP_ProvisionComplete(void)
{
    HTTP_INIT_ProvisionComplete();

#if (SYS_CPNT_WEBAUTH == TRUE)
    WEBAUTH_INIT_ProvisionComplete();
#endif
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : WEBGROUP_SetTransitionMode
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will invoke all set transition mode function in WEBGROUP.
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
static void WEBGROUP_SetTransitionMode(void)
{

#if (SYS_CPNT_HTTP == TRUE)
    HTTP_INIT_SetTransitionMode();
#endif

#if (SYS_CPNT_WEBAUTH == TRUE)
    WEBAUTH_INIT_SetTransitionMode();
#endif

    CGI_INIT_SetTransitionMode();
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : WEBGROUP_EnterTransitionMode
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will invoke all enter transition mode function in WEBGROUP.
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
static void WEBGROUP_EnterTransitionMode(void)
{

#if (SYS_CPNT_HTTP == TRUE)
    HTTP_INIT_EnterTransitionMode();
#endif

#if (SYS_CPNT_WEBAUTH == TRUE)
    WEBAUTH_INIT_EnterTransitionMode();
#endif

    CGI_INIT_EnterTransitionMode();
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : WEBGROUP_EnterMasterMode
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will invoke all set master mode function in WEBGROUP.
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
static void WEBGROUP_EnterMasterMode(void)
{

#if (SYS_CPNT_HTTP == TRUE)
    HTTP_INIT_EnterMasterMode();
#endif

#if (SYS_CPNT_WEBAUTH == TRUE)
    WEBAUTH_INIT_EnterMasterMode();
#endif

    CGI_INIT_EnterMasterMode();
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : WEBGROUP_EnterSlaveMode
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will invoke all enter slave mode function in WEBGROUP.
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
static void WEBGROUP_EnterSlaveMode(void)
{

#if (SYS_CPNT_HTTP == TRUE)
    HTTP_INIT_EnterSlaveMode();
#endif

#if (SYS_CPNT_WEBAUTH == TRUE)
    WEBAUTH_INIT_EnterSlaveMode();
#endif

    CGI_INIT_EnterSlaveMode();
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : WEBGROUP_HandleSysCallbackIPCMsg
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will handle all callbacks from IPC messages in WEBGROUP.
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
static void WEBGROUP_HandleSysCallbackIPCMsg(SYSFUN_Msg_T *msgbuf_p)
{
    SYS_CALLBACK_MGR_Msg_T *sys_cbmsg_p = (SYS_CALLBACK_MGR_Msg_T*)msgbuf_p->msg_buf;

    switch(sys_cbmsg_p->callback_event_id)
    {
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_ANNOUNCE_XFER_RESULT:
        {
            SYS_CALLBACK_MGR_AnnounceXferResult_CBData_T  *cbdata_msg_p = (SYS_CALLBACK_MGR_AnnounceXferResult_CBData_T *)sys_cbmsg_p->callback_data;
            HTTP_MGR_XferCopy_Callback(cbdata_msg_p->cookie, cbdata_msg_p->status);
        }
        break;

#if(SYS_CPNT_WEBAUTH == TRUE)
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_UPORT_LINK_DOWN:
        {
            SYS_CALLBACK_MGR_UPort_CBData_T  *cbdata_msg_p = (SYS_CALLBACK_MGR_UPort_CBData_T *)sys_cbmsg_p->callback_data;
            WEBAUTH_MGR_PortLinkDown_CallBack(cbdata_msg_p->unit, cbdata_msg_p->port);
        }
        break;
#endif  /* (SYS_CPNT_WEBAUTH == TRUE) */

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RIF_DESTROYED:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, WEB_GROUP_RifDestroyedCallbackHandler);
            break;

#if (SYS_CPNT_MGMT_IP_FLT == TRUE)
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_MGMT_IP_FLT_CHANGED:
        SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
            WEB_GROUP_MgmtIPFltChangedCallbackHandler);
        break;
#endif

        default:
            SYSFUN_Debug_Printf("\r\n%s: received callback_event that is not handled(%d)",
                __FUNCTION__, sys_cbmsg_p->callback_event_id);
    }
}

#if (SYS_CPNT_MGMT_IP_FLT == TRUE)
/*------------------------------------------------------------------------------
 * ROUTINE NAME : WEB_GROUP_MgmtIPFltChangedCallbackHandler
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Callback to HTTP if the database of mgmt IP filter was changed
 *
 * INPUT:
 *    mode  --  which mode of mgmt IP filter was changed
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
static void WEB_GROUP_MgmtIPFltChangedCallbackHandler(UI32_T mode)
{
    if (MGMT_IP_FLT_WEB == mode)
    {
        HTTP_MGR_MgmtIPFltChanged_Callback();
    }
}
#endif /* #if (SYS_CPNT_MGMT_IP_FLT == TRUE) */

/* LOCAL SUBPROGRAM BODIES
 */
/*------------------------------------------------------------------------------
 * ROUTINE NAME : WEBGROUP_Mgr_Thread_Function_Entry
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This is the entry function for the MGR thread of WEBGROUP.
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
static void WEBGROUP_Mgr_Thread_Function_Entry(void* arg)
{
    SYSFUN_MsgQ_T        ipc_msgq_handle;
    L_THREADGRP_Handle_T tg_handle = WEB_PROC_COMM_GetWEB_GROUPTGHandle();
    UI32_T               member_id,received_events,local_events=SYSFUN_SYSTEM_EVENT_IPCMSG;
    SYSFUN_Msg_T         *msgbuf_p=(SYSFUN_Msg_T*)WEBGROUP_mgrtd_ipc_buf;
    BOOL_T               need_resp=FALSE;

    /* join the thread group of WEBGROUP
     */
    if(L_THREADGRP_Join(tg_handle, SYS_BLD_WEBGROUP_MGR_THREAD_PRIORITY, &member_id)==FALSE)
    {
        printf("%s: L_THREADGRP_Join fail.\n", __FUNCTION__);
        return;
    }

    /* create mgr ipc message queue handle
     */
    if(SYSFUN_CreateMsgQ(SYS_BLD_WEB_GROUP_IPCMSGQ_KEY, SYSFUN_MSGQ_BIDIRECTIONAL, &ipc_msgq_handle)!=SYSFUN_OK)
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
            WEBGROUP_SetTransitionMode();
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
                WEBGROUP_PMGR_MSGBUF_MAX_REQUIRED_SIZE, msgbuf_p)==SYSFUN_OK)
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
#if (SYS_CPNT_HTTP == TRUE)
                    case SYS_MODULE_HTTP:
                        need_resp=HTTP_MGR_HandleIPCReqMsg(msgbuf_p);
                        break;
#endif

#if (SYS_CPNT_WEBAUTH == TRUE)
                    case SYS_MODULE_WEBAUTH:
                        need_resp = WEBAUTH_MGR_HandleIPCReqMsg(msgbuf_p);
                        break;
#endif /* #if (SYS_CPNT_WEBAUTH == TRUE) */

                    case SYS_MODULE_SYS_CALLBACK:
                        /* SYS_CALLBACK ipc message can only be uni-direction
                         * just set need_resp as FALSE
                         */
                        need_resp=FALSE;
                        WEBGROUP_HandleSysCallbackIPCMsg(msgbuf_p);
                        break;

                    case SYS_MODULE_BACKDOOR:
                        need_resp=BACKDOOR_MGR_HandleIPCMsg(msgbuf_p);
                        break;
                    /* global cmd
                     */
                    case SYS_TYPE_CMD_ENTER_MASTER_MODE:
                        WEBGROUP_EnterMasterMode();

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
                        WEBGROUP_EnterSlaveMode();

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
                        WEBGROUP_EnterTransitionMode();

                        /* SYS_TYPE_CMD_ENTER_TRANSITION_MODE
                         * need a response which contains nothing
                         */
                        msgbuf_p->msg_size=0;
                        need_resp=TRUE;
                        break;

                   case SYS_TYPE_CMD_PROVISION_COMPLETE:
                   case SYS_TYPE_CMD_MODULE_PROVISION_COMPLETE:
                        WEB_GROUP_ProvisionComplete();
                        msgbuf_p->msg_size = 0;
                        need_resp = TRUE;
                        break;

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
                   case SYS_TYPE_CMD_HANDLE_HOT_INSERTION:
                        WEB_GROUP_HandleHotInertion(msgbuf_p);
                        msgbuf_p->msg_size = 0;
                        need_resp = TRUE;
                        break;

                   case SYS_TYPE_CMD_HANDLE_HOT_REMOVAL:
                        WEB_GROUP_HandleHotRemoval(msgbuf_p);
                        msgbuf_p->msg_size = 0;
                        need_resp = TRUE;
                        break;
#endif /* SYS_CPNT_UNIT_HOT_SWAP */

                   case SYS_TYPE_CMD_RELOAD_SYSTEM:
                   /*need to check if need to add SYS_TYPE_CMD_SYNC_SYS_TIME*/
                   case SYS_TYPE_CMD_SYNC_SYS_TIME:
                        msgbuf_p->msg_size = 0;
                        need_resp = TRUE;
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
       	    SW_WATCHDOG_MGR_ResetTimer(SW_WATCHDOG_WEB_GROUP);
            local_events ^= SYSFUN_SYSTEM_EVENT_SW_WATCHDOG;
        }
#endif
    } /* end of while(1) */
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : WEB_GROUP_RifDestroyedCallbackHandler
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Callback to web the rif destroy (IP address was changed)
 *
 * INPUT:
 *    ifindex -- ifindex
 *    addr_p -- address
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
static void WEB_GROUP_RifDestroyedCallbackHandler(UI32_T ifindex, L_INET_AddrIp_T *addr_p)
{
    HTTP_MGR_RifDestroyedCallbackHandler(addr_p);
}

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
/*------------------------------------------------------------------------------
 * ROUTINE NAME : WEB_GROUP_HandleHotInertion
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will invoke a new dut insertion in CSCGroup1.
 *
 * INPUT:
 *    starting_port_ifindex.
      number_of_port
      use_default
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
static void WEB_GROUP_HandleHotInertion(SYSFUN_Msg_T* msgbuf_p)
{
    SYS_TYPE_HandleHotSwapArg_T *msg_p;

    if (msgbuf_p == NULL)
        return;

    msg_p = (SYS_TYPE_HandleHotSwapArg_T*)msgbuf_p->msg_buf;

#if (SYS_CPNT_WEBAUTH == TRUE)
    WEBAUTH_INIT_HandleHotInsertion(msg_p->starting_port_ifindex, msg_p->number_of_port, msg_p->use_default);
#endif /* SYS_CPNT_WEBAUTH */

    CGI_INIT_HandleHotInsertion(msg_p->starting_port_ifindex, msg_p->number_of_port, msg_p->use_default);
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : WEB_GROUP_HandleHotRemoval
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will invoke a new dut removal in CSCGroup1.
 *
 * INPUT:
 *    starting_port_ifindex.
      number_of_port
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
static void WEB_GROUP_HandleHotRemoval(SYSFUN_Msg_T* msgbuf_p)
{
    SYS_TYPE_HandleHotSwapArg_T *msg_p;

    if (msgbuf_p == NULL)
        return;

    msg_p = (SYS_TYPE_HandleHotSwapArg_T*)msgbuf_p->msg_buf;

#if (SYS_CPNT_WEBAUTH == TRUE)
    WEBAUTH_INIT_HandleHotRemoval(msg_p->starting_port_ifindex, msg_p->number_of_port);
#endif /* SYS_CPNT_WEBAUTH */

    CGI_INIT_HandleHotRemoval(msg_p->starting_port_ifindex, msg_p->number_of_port);
}
#endif

