/* MODULE NAME: vxlan_group.c
 * PURPOSE:
 *   The module implements the fucntionality of VXLAN group.
 * NOTES:
 *   None
 *
 * HISTORY:
 *    mm/dd/yy
 *    04/21/15 -- Kelly, Create
 *
 * Copyright(C) Accton Corporation, 2015
 */


/* INCLUDE FILE DECLARATIONS
 */
#include "sys_cpnt.h"
#include <string.h>
#include "sys_bld.h"
#include "sys_module.h"
#include "sys_type.h"
#include "l_threadgrp.h"
#include "sysfun.h"
#include "backdoor_mgr.h"
#include "l2_l4_proc_comm.h"
#include "sys_callback_mgr.h"
#include "l_inet.h"
#include "vxlan_group.h"
#include "vxlan_mgr.h"

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
#include "sw_watchdog_mgr.h"
#endif
#if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE)
#include "cmgr.h"
#endif

/* NAMING CONSTANT DECLARATIONS
 */

/* the size of the ipc msg buf for the MGR thread to receive and
 * response PMGR ipc. The size of this buffer should pick the maximum of size
 * required for PMGR ipc request and response in all CSCs handled in this
 * process. (this size does not include IPC msg header)
 */
#define PMGR_MSGBUF_MAX_REQUIRED_SIZE sizeof(VXLAN_GROUP_MgrMsg_T)


/* MACRO FUNCTION DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */

/* union all data type used for group MGR IPC message to get the maximum
 * required ipc message buffer
 */
typedef union {
    SYS_CALLBACK_MGR_LPort_CBData_T                   sys_callback_mgr_lport_cbdata;
}   VXLAN_GROUP_Syscallback_CBData_T;

typedef struct
{
    UI8_T buffer[SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(VXLAN_GROUP_Syscallback_CBData_T))];
}   VXLAN_GROUP_Syscallback_Msg_T;

typedef union VXLAN_GROUP_MgrMsg_U
{
    BACKDOOR_MGR_Msg_T              backdoor_mgr_ipcmsg;
    VXLAN_GROUP_Syscallback_Msg_T   syscb_ipcmsg;
#if (SYS_CPNT_VXLAN == TRUE)
    VXLAN_MGR_IpcMsg_T              vxlan_mgr_ipcmsg;
#endif
#if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE)
    CMGR_IpcMsg_T                   cmgr_ipcmsg;
#endif
} VXLAN_GROUP_MgrMsg_T;


/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void VXLAN_GROUP_Mgr_Thread_Function_Entry(void* arg);
static void VXLAN_GROUP_SetTransitionMode(void);
static void VXLAN_GROUP_EnterTransitionMode(void);
static void VXLAN_GROUP_EnterMasterMode(void);
static void VXLAN_GROUP_EnterSlaveMode(void);
static void VXLAN_GROUP_ProvisionComplete(void);
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
static void VXLAN_GROUP_HandleHotInsertion(SYSFUN_Msg_T* msgbuf_p);
static void VXLAN_GROUP_HandleHotRemoval(SYSFUN_Msg_T* msgbuf_p);
#endif /* #if (SYS_CPNT_UNIT_HOT_SWAP == TRUE) */
static void VXLAN_GROUP_HandleSysCallbackIPCMsg(SYSFUN_Msg_T *msgbuf_p);
static void VXLAN_GROUP_RouteChangeCallbackHandler(UI32_T address_family);
static void VXLAN_GROUP_VlanCreateCallbackHandler(UI32_T vid_ifindex, UI32_T vlan_status);
static void VXLAN_GROUP_VlanDestroyCallbackHandler(UI32_T vid_ifindex, UI32_T vlan_status);
static void VXLAN_GROUP_VlanMemberAddCallbackHandler(UI32_T vid_ifindex,UI32_T lport_ifindex,UI32_T vlan_status);
static void VXLAN_GROUP_VlanMemberDeleteCallbackHandler(UI32_T vid_ifindex,UI32_T lport_ifindex,UI32_T vlan_status);
static void VXLAN_GROUP_RifActiveCallbackHandler(UI32_T ifindex, L_INET_AddrIp_T *addr_p);
static void VXLAN_GROUP_RifDownCallbackHandler(UI32_T ifindex, L_INET_AddrIp_T *addr_p);
static void VXLAN_GROUP_TrunkMemberAdd1stCallbackHandler(UI32_T trunk_ifindex, UI32_T member_ifindex);
static void VXLAN_GROUP_TrunkMemberAddCallbackHandler(UI32_T trunk_ifindex, UI32_T member_ifindex);
static void VXLAN_GROUP_TrunkMemberDeleteCallbackHandler(UI32_T trunk_ifindex, UI32_T member_ifindex);
static void VXLAN_GROUP_TrunkMemberDeleteLstCallbackHandler(UI32_T trunk_ifindex, UI32_T member_ifindex);
#if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE)
static void VXLAN_GROUP_HandleCmgrIpcMsg(SYSFUN_Msg_T *ipc_msg_p);
#endif
static void VXLAN_GROUP_RifCreatedCallbackHandler(UI32_T ifindex, L_INET_AddrIp_T *addr_p);
static void VXLAN_GROUP_RifDestroyedCallbackHandler(UI32_T ifindex, L_INET_AddrIp_T *addr_p);
static void VXLAN_GROUP_LportLinkDownCallbackHandler(UI32_T ifindex);
#if (SYS_CPNT_VXLAN_ACCESS_PORT_MODE == SYS_CPNT_VXLAN_PER_PORT_AND_PER_PORT_PER_VLAN)
static void VXLAN_GROUP_PvidChangeCallbackHandler(UI32_T lport_ifindex, UI32_T old_pvid, UI32_T new_pvid);
#endif

/* STATIC VARIABLE DEFINITIONS
 */

/* the buffer for retrieving ipc request for group mgr thread
 */
static UI8_T mgrtd_ipc_buf[SYSFUN_SIZE_OF_MSG(PMGR_MSGBUF_MAX_REQUIRED_SIZE)];

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME - VXLAN_GROUP_InitiateProcessResources
 * PURPOSE : Initiate process resource.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */
void VXLAN_GROUP_InitiateProcessResources(void)
{
#if (SYS_CPNT_VXLAN == TRUE)
    VXLAN_MGR_InitiateSystemResources();
#endif
}

/* FUNCTION NAME - VXLAN_GROUP_Create_InterCSC_Relation
 * PURPOSE : Create inter-CSC relationships.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */
void VXLAN_GROUP_Create_InterCSC_Relation(void)
{
#if (SYS_CPNT_VXLAN == TRUE)
    VXLAN_MGR_Create_InterCSC_Relation();
#endif
}

/* FUNCTION NAME - VXLAN_GROUP_Create_All_Threads
 * PURPOSE : This function will spawn all threads.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */
void VXLAN_GROUP_Create_All_Threads(void)
{
    UI32_T thread_id;

    if (SYSFUN_SpawnThread(SYS_BLD_VXLAN_GROUP_MGR_THREAD_PRIORITY,
                           SYS_BLD_VXLAN_GROUP_MGR_SCHED_POLICY,
                           SYS_BLD_VXLAN_GROUP_MGR_THREAD_NAME,
                           SYS_BLD_TASK_COMMON_STACK_SIZE,
                           SYSFUN_TASK_FP,
                           VXLAN_GROUP_Mgr_Thread_Function_Entry,
                           NULL,
                           &thread_id) != SYSFUN_OK)
    {
        printf("\n%s: Spawn CMGR Group MGR thread fail.\n", __FUNCTION__);
    }

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
    SW_WATCHDOG_MGR_RegisterMonitorThread(SW_WATCHDOG_VXLAN_GROUP, thread_id, SYS_ADPT_VXLAN_GROUP_SW_WATCHDOG_TIMER);
#endif

}


/* LOCAL SUBPROGRAM BODIES
 */

/* FUNCTION NAME - VXLAN_GROUP_Mgr_Thread_Function_Entry
 * PURPOSE : This is the entry function for the MGR thread.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */
static void VXLAN_GROUP_Mgr_Thread_Function_Entry(void* arg)
{
    SYSFUN_MsgQ_T           ipc_msgq_handle;
    L_THREADGRP_Handle_T    tg_handle = L2_L4_PROC_COMM_GetVxlanGroupTGHandle();
    UI32_T                  member_id, received_events, local_events = SYSFUN_SYSTEM_EVENT_IPCMSG;
    SYSFUN_Msg_T            *msgbuf_p = (SYSFUN_Msg_T*)mgrtd_ipc_buf;
    UI32_T                  all_events =
                                SYSFUN_SYSTEM_EVENT_SET_TRANSITION_MODE |
                                SYSFUN_SYSTEM_EVENT_IPCMSG |
#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
                                SYSFUN_SYSTEM_EVENT_SW_WATCHDOG |
#endif
                                SYSFUN_SYSTEM_EVENT_IPCFAIL;
    BOOL_T                  need_resp;

    /* join the thread group
     */
    if (tg_handle != NULL)
    {
        if (L_THREADGRP_Join(tg_handle, SYS_BLD_VXLAN_GROUP_MGR_THREAD_PRIORITY,
                &member_id) == FALSE)
        {
            printf("\n%s: L_THREADGRP_Join fail.\n", __FUNCTION__);
            return;
        }
    }

    /* create mgr ipc message queue handle
     */
    if (SYSFUN_CreateMsgQ(SYS_BLD_VXLAN_GROUP_IPCMSGQ_KEY,
            SYSFUN_MSGQ_BIDIRECTIONAL, &ipc_msgq_handle) != SYSFUN_OK)
    {
        printf("\n%s: SYSFUN_CreateMsgQ fail.\n", __FUNCTION__);
        return;
    }

    /* main loop:
     *    while(1)
     *    {
     *        Wait event
     *            Handle SYSFUN_SYSTEM_EVENT_SET_TRANSITION_MODE event if any
     *            Handle SYSFUN_SYSTEM_EVENT_IPCMSG if any
     *            Handle SYSFUN_SYSTEM_EVENT_IPCFAIL if any
     *    }
     */
    while (1)
    {
        SYSFUN_ReceiveEvent(all_events,
                            SYSFUN_EVENT_WAIT_ANY,
                            (local_events == 0) ? SYSFUN_TIMEOUT_WAIT_FOREVER
                                                : SYSFUN_TIMEOUT_NOWAIT,
                            &received_events);
        local_events |= received_events;

        if (local_events & SYSFUN_SYSTEM_EVENT_SET_TRANSITION_MODE)
        {
            VXLAN_GROUP_SetTransitionMode();
            local_events ^= SYSFUN_SYSTEM_EVENT_SET_TRANSITION_MODE;

            /* need not to do IPCFAIL recovery in transition mode
             */
            if (local_events & SYSFUN_SYSTEM_EVENT_IPCFAIL)
            {
                /* clear fail info in IPCFAIL
                 */
                local_events ^= SYSFUN_SYSTEM_EVENT_IPCFAIL;
            }
        }

        /* handle IPCMSG
         */
        if (local_events & SYSFUN_SYSTEM_EVENT_IPCMSG)
        {
            UI32_T  ret;

            ret = SYSFUN_ReceiveMsg(ipc_msgq_handle, 0, SYSFUN_TIMEOUT_NOWAIT,
                    PMGR_MSGBUF_MAX_REQUIRED_SIZE, msgbuf_p);

            if (ret == SYSFUN_OK)
            {
                /* request thread group execution permission
                 */
                if (tg_handle != NULL)
                {
                    if (L_THREADGRP_Execution_Request(tg_handle, member_id) != TRUE)
                    {
                        printf("\n%s: L_THREADGRP_Execution_Request fail.\n",
                            __FUNCTION__);
                    }
                }

                /* handle request message based on cmd
                 */
                switch (msgbuf_p->cmd)
                {
                    /* global cmd
                     */
                    case SYS_TYPE_CMD_ENTER_TRANSITION_MODE:
                        VXLAN_GROUP_EnterTransitionMode();

                        /* SYS_TYPE_CMD_ENTER_TRANSITION_MODE
                         * need a response which contains nothing
                         */
                        msgbuf_p->msg_size = 0;
                        need_resp = TRUE;
                        break;

                    case SYS_TYPE_CMD_ENTER_MASTER_MODE:
                        VXLAN_GROUP_EnterMasterMode();

                        /* SYS_TYPE_CMD_ENTER_MASTER_MODE
                         * need a response which contains nothing
                         */
                        msgbuf_p->msg_size = 0;
                        need_resp = TRUE;

                        /* IPCFAIL might happen when a upper layer cscgroup has
                         * entered transition mode but lower layer cscgroups still
                         * in master mode so async callback msg will keep sending
                         * from lower layer cscgroups. In this case, the IPCFAIL
                         * event can be ignored.
                         */
                        if (local_events & SYSFUN_SYSTEM_EVENT_IPCFAIL)
                        {
                            /* clear fail info in IPCFAIL
                             */
                            local_events ^= SYSFUN_SYSTEM_EVENT_IPCFAIL;
                        }
                        break;

                    case SYS_TYPE_CMD_ENTER_SLAVE_MODE:
                        VXLAN_GROUP_EnterSlaveMode();

                        /* SYS_TYPE_CMD_ENTER_SLAVE_MODE
                         * need a response which contains nothing
                         */
                        msgbuf_p->msg_size = 0;
                        need_resp = TRUE;

                        /* IPCFAIL might happen when a upper layer cscgroup has
                         * entered transition mode but lower layer cscgroups still
                         * in master mode so async callback msg will keep sending
                         * from lower layer cscgroups. In this case, the IPCFAIL
                         * event can be ignored.
                         */
                        if (local_events & SYSFUN_SYSTEM_EVENT_IPCFAIL)
                        {
                            /* clear fail info in IPCFAIL
                             */
                            local_events ^= SYSFUN_SYSTEM_EVENT_IPCFAIL;
                        }
                        break;

                    case SYS_TYPE_CMD_PROVISION_COMPLETE:
                        VXLAN_GROUP_ProvisionComplete();
                        msgbuf_p->msg_size = 0;
                        need_resp = TRUE;
                        break;

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
                    case SYS_TYPE_CMD_HANDLE_HOT_INSERTION:
                        VXLAN_GROUP_HandleHotInsertion(msgbuf_p);
                        msgbuf_p->msg_size=0;
                        need_resp=TRUE;
                        break;

                    case SYS_TYPE_CMD_HANDLE_HOT_REMOVAL:
                        VXLAN_GROUP_HandleHotRemoval(msgbuf_p);
                        msgbuf_p->msg_size=0;
                        need_resp=TRUE;
                        break;
#endif /* #if (SYS_CPNT_UNIT_HOT_SWAP == TRUE) */

                    case SYS_TYPE_CMD_RELOAD_SYSTEM:
                        msgbuf_p->msg_size=0;
                        need_resp = TRUE;
                        break;

                    /* module id cmd
                     */
                    case SYS_MODULE_VXLAN:
                        need_resp = VXLAN_MGR_HandleIPCReqMsg(msgbuf_p);
                        break;

                    case SYS_MODULE_BACKDOOR:
                        need_resp = BACKDOOR_MGR_HandleIPCMsg(msgbuf_p);
                        break;

                    case SYS_MODULE_SYS_CALLBACK:
                        VXLAN_GROUP_HandleSysCallbackIPCMsg(msgbuf_p);
                        /* SYS_CALLBACK ipc message can only be uni-direction
                         * just set need_resp as FALSE
                         */
                        need_resp = FALSE;
                        break;

#if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE)
                    case SYS_MODULE_CMGR:
                        VXLAN_GROUP_HandleCmgrIpcMsg(msgbuf_p);
                        msgbuf_p->msg_size = 0;
                        need_resp = TRUE;
                        break;
#endif

                    default:
                        /* Unknow command. There is no way to idntify whether this
                         * ipc message need or not need a response. If we response to
                         * a asynchronous msg, then all following synchronous msg will
                         * get wrong responses and that might not easy to debug.
                         * If we do not response to a synchronous msg, the requester
                         * will be blocked forever. It should be easy to debug that
                         * error.
                         */
                        printf("\n%s: Invalid IPC req cmd.\n", __FUNCTION__);
                        need_resp = FALSE;
                } /* end of switch (msgbuf_p->cmd) */

                /* release thread group execution permission
                 */
                if (tg_handle != NULL)
                {
                    if (L_THREADGRP_Execution_Release(tg_handle, member_id) != TRUE)
                    {
                        printf("\n%s: L_THREADGRP_Execution_Release fail.\n",
                            __FUNCTION__);
                    }
                }

                if ((need_resp == TRUE) && (SYSFUN_SendResponseMsg(
                        ipc_msgq_handle, msgbuf_p) != SYSFUN_OK))
                {
                    printf("\n%s: SYSFUN_SendResponseMsg fail.\n",
                        __FUNCTION__);
                }
            } /* end of if (SYSFUN_ReceiveMsg... */
            else
            {
                local_events ^= SYSFUN_SYSTEM_EVENT_IPCMSG;
            }
        } /* end of if (local_events & SYSFUN_SYSTEM_EVENT_IPCMSG) */

        /* handle IPC Async Callback fail when IPC Msgq is empty
         */
        if ((local_events & SYSFUN_SYSTEM_EVENT_IPCFAIL) &&
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
            SW_WATCHDOG_MGR_ResetTimer(SW_WATCHDOG_VXLAN_GROUP);
            local_events ^= SYSFUN_SYSTEM_EVENT_SW_WATCHDOG;
        }
#endif

    } /* end of while (1) */
} /* End of VXLAN_GROUP_Mgr_Thread_Function_Entry */


/* FUNCTION NAME - VXLAN_GROUP_SetTransitionMode
 * PURPOSE : This function will invoke all set transition mode function.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */
static void VXLAN_GROUP_SetTransitionMode(void)
{
#if (SYS_CPNT_VXLAN == TRUE)
    VXLAN_MGR_SetTransitionMode();
#endif
}


/* FUNCTION NAME - VXLAN_GROUP_EnterTransitionMode
 * PURPOSE : This function will invoke all enter transition mode function.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */
static void VXLAN_GROUP_EnterTransitionMode(void)
{
#if (SYS_CPNT_VXLAN == TRUE)
    VXLAN_MGR_EnterTransitionMode();
#endif
}


/* FUNCTION NAME - VXLAN_GROUP_EnterMasterMode
 * PURPOSE : This function will invoke all set master mode function.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */
static void VXLAN_GROUP_EnterMasterMode(void)
{
#if (SYS_CPNT_VXLAN == TRUE)
    VXLAN_MGR_EnterMasterMode();
#endif
}


/* FUNCTION NAME - VXLAN_GROUP_EnterSlaveMode
 * PURPOSE : This function will invoke all enter slave mode function.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */
static void VXLAN_GROUP_EnterSlaveMode(void)
{
#if (SYS_CPNT_VXLAN == TRUE)
    VXLAN_MGR_EnterSlaveMode();
#endif
}


/* FUNCTION NAME - VXLAN_GROUP_ProvisionComplete
 * PURPOSE : This function will invoke all provision complete function.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */
static void VXLAN_GROUP_ProvisionComplete(void)
{
}

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
/* FUNCTION NAME - VXLAN_GROUP_HandleHotInsertion
 * PURPOSE : This function will invoke a new dut insertion.
 * INPUT   : msgbuf_p -- SYS_CALLBACK IPC message
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */
static void VXLAN_GROUP_HandleHotInsertion(SYSFUN_Msg_T* msgbuf_p)
{
    SYS_TYPE_HandleHotSwapArg_T     *msg_p;

    if (msgbuf_p == NULL)
        return;

    msg_p = (SYS_TYPE_HandleHotSwapArg_T*) msgbuf_p->msg_buf;

    /* XXX_HandleHotInsertion(msg_p->starting_port_ifindex, msg_p->number_of_port, msg_p->use_default); */
}


/* FUNCTION NAME - VXLAN_GROUP_HandleHotRemoval
 * PURPOSE : This function will invoke a dut removal.
 * INPUT   : msgbuf_p -- SYS_CALLBACK IPC message
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */
static void VXLAN_GROUP_HandleHotRemoval(SYSFUN_Msg_T* msgbuf_p)
{
    SYS_TYPE_HandleHotSwapArg_T     *msg_p;

    if (msgbuf_p == NULL)
        return;

    msg_p = (SYS_TYPE_HandleHotSwapArg_T*) msgbuf_p->msg_buf;

    /* XXX_HandleHotInsertion(msg_p->starting_port_ifindex, msg_p->number_of_port, msg_p->use_default); */
}
#endif /* #if (SYS_CPNT_UNIT_HOT_SWAP == TRUE) */

/* FUNCTION NAME - VXLAN_GROUP_HandleSysCallbackIPCMsg
 * PURPOSE : This function will handle all callbacks from IPC messages.
 * INPUT   : msgbuf_p -- SYS_CALLBACK IPC message
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */
static void VXLAN_GROUP_HandleSysCallbackIPCMsg(SYSFUN_Msg_T *msgbuf_p)
{
    SYS_CALLBACK_MGR_Msg_T *sys_cbmsg_p =
        (SYS_CALLBACK_MGR_Msg_T*)msgbuf_p->msg_buf;

    switch (sys_cbmsg_p->callback_event_id)
    {
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_IPCFG_NSM_ROUTE_CHANGE:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                VXLAN_GROUP_RouteChangeCallbackHandler);
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_CREATE:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                VXLAN_GROUP_VlanCreateCallbackHandler);
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_DESTROY:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                VXLAN_GROUP_VlanDestroyCallbackHandler);
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_MEMBER_ADD:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                VXLAN_GROUP_VlanMemberAddCallbackHandler);
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_MEMBER_DELETE:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                VXLAN_GROUP_VlanMemberDeleteCallbackHandler);
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RIF_ACTIVE:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                VXLAN_GROUP_RifActiveCallbackHandler);
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RIF_DOWN:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                VXLAN_GROUP_RifDownCallbackHandler);
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_TRUNK_MEMBER_ADD_1ST:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                VXLAN_GROUP_TrunkMemberAdd1stCallbackHandler);
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_TRUNK_MEMBER_ADD:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                VXLAN_GROUP_TrunkMemberAddCallbackHandler);
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_TRUNK_MEMBER_DELETE:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                VXLAN_GROUP_TrunkMemberDeleteCallbackHandler);
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_TRUNK_MEMBER_DELETE_LST:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                VXLAN_GROUP_TrunkMemberDeleteLstCallbackHandler);
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RIF_CREATED:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                VXLAN_GROUP_RifCreatedCallbackHandler);
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RIF_DESTROYED:
             SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                 VXLAN_GROUP_RifDestroyedCallbackHandler);
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_PORT_LINK_DOWN:
             SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                 VXLAN_GROUP_LportLinkDownCallbackHandler);
            break;
#if (SYS_CPNT_VXLAN_ACCESS_PORT_MODE == SYS_CPNT_VXLAN_PER_PORT_AND_PER_PORT_PER_VLAN)
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_PVID_CHANGE:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                VXLAN_GROUP_PvidChangeCallbackHandler);
            break;
#endif

        default:
            SYSFUN_Debug_Printf("%s: received callback_event that is not handled(%d)\n",
                __FUNCTION__, sys_cbmsg_p->callback_event_id);
            break;
    }
}

/* FUNCTION NAME:  VXLAN_GROUP_RouteChangeCallbackHandler
 * PURPOSE : Handle the callback event happening when nsm rib route change.
 * INPUT   : address_family -- specify which address family has route change (IPv4/IPv6)
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : None.
 */
static void VXLAN_GROUP_RouteChangeCallbackHandler(UI32_T address_family)
{

    VXLAN_MGR_RouteChange_CallBack(address_family);

    return;
}   /*  end of NECTCFG_GROUP_NsmRouteChangeCallbackHandler  */

/* FUNCTION NAME:  VXLAN_GROUP_VlanCreateCallbackHandler
 * PURPOSE :  Handle the callback event happening when a vlan is created.
 * INPUT   : vid_ifindex -- specify which vlan has just been created
 *           vlan_status -- VAL_dot1qVlanStatus_other
 *                          VAL_dot1qVlanStatus_permanent
 *                          VAL_dot1qVlanStatus_dynamicGvrp
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : None.
 */
static void VXLAN_GROUP_VlanCreateCallbackHandler(UI32_T vid_ifindex, UI32_T vlan_status)
{
   VXLAN_MGR_VlanCreate_Callback(vid_ifindex, vlan_status);
   return;
}

/* FUNCTION NAME:  VXLAN_GROUP_VlanDestroyCallbackHandler
 * PURPOSE :  Handle the callback event happening when a vlan is destroyed.
 * INPUT   : vid_ifindex -- specify which vlan has just been destroyed
 *           vlan_status -- VAL_dot1qVlanStatus_other
 *                          VAL_dot1qVlanStatus_permanent
 *                          VAL_dot1qVlanStatus_dynamicGvrp
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : None.
 */
static void VXLAN_GROUP_VlanDestroyCallbackHandler(UI32_T vid_ifindex, UI32_T vlan_status)
{
   VXLAN_MGR_VlanDestroy_Callback(vid_ifindex, vlan_status);
   return;
}

/* FUNCTION NAME:  VXLAN_GROUP_VlanMemberAddCallbackHandler
 * PURPOSE : Handle the callback event happening when a lport is added to a
 *           vlan's member set.
 * INPUT   : vid_ifindex   -- specify which vlan member set to join
 *           lport_ifindex -- sepcify which lport to join to the member set
 *           vlan_status   -- VAL_dot1qVlanStatus_other
 *                            VAL_dot1qVlanStatus_permanent
 *                            VAL_dot1qVlanStatus_dynamicGvrp
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : None.
 */
static void VXLAN_GROUP_VlanMemberAddCallbackHandler(UI32_T vid_ifindex,UI32_T lport_ifindex,UI32_T vlan_status)
{
#if (SYS_CPNT_VXLAN_ACCESS_PORT_MODE == SYS_CPNT_VXLAN_SYSTEM_WISE_VLAN_VNI_MAPPING)
   VXLAN_MGR_VlanMemberAdd_Callback(vid_ifindex,lport_ifindex,vlan_status);
#endif
   return;
}

/* FUNCTION NAME:  VXLAN_GROUP_VlanMemberDeleteCallbackHandler
 * PURPOSE : Handle the callback event happening when a port is remove from
 *           vlan's member set.
 * INPUT   :vid_ifindex   -- specify which vlan's member set to disjoin from
 *           lport_ifindex -- sepcify which lport to disjoin from the member set
 *           vlan_status   -- VAL_dot1qVlanStatus_other
 *                            VAL_dot1qVlanStatus_permanent
 *                            VAL_dot1qVlanStatus_dynamicGvrp
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : None.
 */
static void VXLAN_GROUP_VlanMemberDeleteCallbackHandler(UI32_T vid_ifindex,UI32_T lport_ifindex,UI32_T vlan_status)
{
#if (SYS_CPNT_VXLAN_ACCESS_PORT_MODE == SYS_CPNT_VXLAN_SYSTEM_WISE_VLAN_VNI_MAPPING)
   VXLAN_MGR_VlanMemberDelete_Callback(vid_ifindex,lport_ifindex,vlan_status);
#endif
   return;
}


/* FUNCTION NAME:  VXLAN_GROUP_RifActiveCallbackHandler
 * PURPOSE : Signal to VXLAN_MGR to indicate that rifActive.
 * INPUT   : ifindex  -- the ifindex  of active rif
 *           addr_p  --  the addr_p of active rif
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : None.
 */
static void VXLAN_GROUP_RifActiveCallbackHandler(UI32_T ifindex, L_INET_AddrIp_T *addr_p)
{
    VXLAN_MGR_RifActive_Callback(ifindex, addr_p);
    return;
}

/* FUNCTION NAME:  VXLAN_GROUP_RifDownCallbackHandler
 * PURPOSE : Signal to VXLAN_MGR to indicate that rifDown.
 * INPUT   : ifindex  -- the ifindex  of active rif
 *           addr_p  --  the addr_p of active rif
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : None.
 */
static void VXLAN_GROUP_RifDownCallbackHandler(UI32_T ifindex, L_INET_AddrIp_T *addr_p)
{
    VXLAN_MGR_RifDown_Callback(ifindex, addr_p);
    return;
}

/* FUNCTION NAME:  VXLAN_GROUP_TrunkMemberAdd1stCallbackHandler
 * PURPOSE : Handle the callback event happening when the first port is added
 *           to a trunk.
 * INPUT   : trunk_ifindex  -- which trunk port
 *           member_ifindex -- which member port
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : None.
 */
static void VXLAN_GROUP_TrunkMemberAdd1stCallbackHandler(UI32_T trunk_ifindex, UI32_T member_ifindex)
{
    VXLAN_MGR_TrunkMemberAdd1st_CallBack(trunk_ifindex, member_ifindex);
}

/* FUNCTION NAME:  VXLAN_GROUP_TrunkMemberAddCallbackHandler
 * PURPOSE : Handle the callback event happening when a logical port is added
 *           to a trunk.
 * INPUT   : trunk_ifindex  -- which trunk port
 *           member_ifindex -- which member port
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : None.
 */
static void VXLAN_GROUP_TrunkMemberAddCallbackHandler(UI32_T trunk_ifindex, UI32_T member_ifindex)
{
    VXLAN_MGR_TrunkMemberAdd_CallBack(trunk_ifindex, member_ifindex);
}

/* FUNCTION NAME:  VXLAN_GROUP_TrunkMemberDeleteCallbackHandler
 * PURPOSE : Handle the callback event happening when a logical port is deleted
 *           from a trunk.
 * INPUT   : trunk_ifindex  -- which trunk port
 *           member_ifindex -- which member port
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : None.
 */
static void VXLAN_GROUP_TrunkMemberDeleteCallbackHandler(UI32_T trunk_ifindex, UI32_T member_ifindex)
{
    VXLAN_MGR_TrunkMemberDelete_CallBack(trunk_ifindex, member_ifindex);
}

/* FUNCTION NAME:  VXLAN_GROUP_TrunkMemberDeleteLstCallbackHandler
 * PURPOSE : Handle the callback event happening when the last port is deleted
 *           from a trunk.
 * INPUT   : trunk_ifindex  -- which trunk port
 *           member_ifindex -- which member port
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : None.
 */
static void VXLAN_GROUP_TrunkMemberDeleteLstCallbackHandler(UI32_T trunk_ifindex, UI32_T member_ifindex)
{
    VXLAN_MGR_TrunkMemberDeleteLst_CallBack(trunk_ifindex, member_ifindex);
}

#if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE)
static void VXLAN_GROUP_HandleCmgrIpcMsg(SYSFUN_Msg_T *ipc_msg_p)
{
    CMGR_IpcMsg_T *cmgr_msg_p;

    if (ipc_msg_p == NULL)
    {
        return;
    }

    cmgr_msg_p = (CMGR_IpcMsg_T*)ipc_msg_p->msg_buf;
    switch (cmgr_msg_p->type.cmd)
    {
    case CMGR_IPC_VLAN_CHANGE:
        if (cmgr_msg_p->data.arg_ui32_ui32_bool_bool.arg_bool_1 == TRUE)
        {
            if (cmgr_msg_p->data.arg_ui32_ui32_bool_bool.arg_bool_2 == TRUE)
            {
                VXLAN_GROUP_VlanDestroyCallbackHandler(
                    cmgr_msg_p->data.arg_ui32_ui32_bool_bool.arg_ui32_1,
                    cmgr_msg_p->data.arg_ui32_ui32_bool_bool.arg_ui32_2);
            }
            VXLAN_GROUP_VlanCreateCallbackHandler(
                cmgr_msg_p->data.arg_ui32_ui32_bool_bool.arg_ui32_1,
                cmgr_msg_p->data.arg_ui32_ui32_bool_bool.arg_ui32_2);
        }
        else
        {
            VXLAN_GROUP_VlanDestroyCallbackHandler(
                cmgr_msg_p->data.arg_ui32_ui32_bool_bool.arg_ui32_1,
                cmgr_msg_p->data.arg_ui32_ui32_bool_bool.arg_ui32_2);
        }
        break;

    case CMGR_IPC_VLAN_MEMBER_CHANGE:
        if (cmgr_msg_p->data.arg_ui32_ui32_ui32_bool_bool.arg_bool_1 == TRUE)
        {
            if (cmgr_msg_p->data.arg_ui32_ui32_ui32_bool_bool.arg_bool_2 ==
                    TRUE)
            {
                VXLAN_GROUP_VlanMemberDeleteCallbackHandler(
                    cmgr_msg_p->data.arg_ui32_ui32_ui32_bool_bool.arg_ui32_1,
                    cmgr_msg_p->data.arg_ui32_ui32_ui32_bool_bool.arg_ui32_2,
                    cmgr_msg_p->data.arg_ui32_ui32_ui32_bool_bool.arg_ui32_3);
            }
            VXLAN_GROUP_VlanMemberAddCallbackHandler(
                cmgr_msg_p->data.arg_ui32_ui32_ui32_bool_bool.arg_ui32_1,
                cmgr_msg_p->data.arg_ui32_ui32_ui32_bool_bool.arg_ui32_2,
                cmgr_msg_p->data.arg_ui32_ui32_ui32_bool_bool.arg_ui32_3);
        }
        else
        {
            VXLAN_GROUP_VlanMemberDeleteCallbackHandler(
                cmgr_msg_p->data.arg_ui32_ui32_ui32_bool_bool.arg_ui32_1,
                cmgr_msg_p->data.arg_ui32_ui32_ui32_bool_bool.arg_ui32_2,
                cmgr_msg_p->data.arg_ui32_ui32_ui32_bool_bool.arg_ui32_3);
        }
        break;
    } /* end switch */

    return;
}
#endif /* #if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE) */

/* FUNCTION NAME:  VXLAN_GROUP_RifCreatedCallbackHandler
 * PURPOSE : Signal to VXLAN_MGR to indicate that rifCreated.
 * INPUT   : ifindex  -- the ifindex  of active rif
 *           addr_p  --  the addr_p of active rif
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : None.
 */
static void VXLAN_GROUP_RifCreatedCallbackHandler(UI32_T ifindex, L_INET_AddrIp_T *addr_p)
{
    VXLAN_MGR_RifCreated_Callback(ifindex, addr_p);
    return;
}

/* FUNCTION NAME:  VXLAN_GROUP_RifDestroyedCallbackHandler
 * PURPOSE : Signal to VXLAN_MGR to indicate that rifDestroyed.
 * INPUT   : ifindex  -- the ifindex  of active rif
 *           addr_p  --  the addr_p of active rif
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : None.
 */
static void VXLAN_GROUP_RifDestroyedCallbackHandler(UI32_T ifindex, L_INET_AddrIp_T *addr_p)
{
    VXLAN_MGR_RifDestroyed_Callback(ifindex, addr_p);
    return;
}

/* FUNCTION NAME:  VXLAN_GROUP_LportLinkDownCallbackHandler
 * PURPOSE : Handle the callback event happening when the link is down.
 * INPUT   : ifindex -- which logical port
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : None.
 */
static void VXLAN_GROUP_LportLinkDownCallbackHandler(UI32_T ifindex)
{
    VXLAN_MGR_PortLinkDown_CallBack(ifindex);
    return;
}

#if (SYS_CPNT_VXLAN_ACCESS_PORT_MODE == SYS_CPNT_VXLAN_PER_PORT_AND_PER_PORT_PER_VLAN)
/* FUNCTION NAME:  VXLAN_GROUP_PvidChangeCallbackHandler
 * PURPOSE : Handle the callback event happening when pvid of port change
 * INPUT   : lport_ifindex -- which logical port
 *           old_pvid      -- old pvid
 *           new_pvid      -- new pvid
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : None.
 */
static void VXLAN_GROUP_PvidChangeCallbackHandler(UI32_T lport_ifindex, UI32_T old_pvid, UI32_T new_pvid)
{
    VXLAN_MGR_PvidChange_CallBack(lport_ifindex, old_pvid, new_pvid);
    return;
}
#endif

