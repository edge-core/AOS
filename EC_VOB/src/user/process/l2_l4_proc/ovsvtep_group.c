/*-----------------------------------------------------------------------------
 * MODULE NAME: OVSVTEP_GROUP.C
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *    The module implements the fucntionality of OVSVTEP group.
 *
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    2016/12/20     --- Squid Ro, Create for Linux platform
 *
 * Copyright(C)      Accton Corporation, 2016
 *-----------------------------------------------------------------------------
 */


/* INCLUDE FILE DECLARATIONS
 */
#include "sys_cpnt.h"

#if (SYS_CPNT_OVSVTEP == TRUE)

#include <string.h>

#include "sys_bld.h"
#include "sys_module.h"
#include "sys_type.h"
#include "l_threadgrp.h"
#include "sysfun.h"
#include "backdoor_mgr.h"
#include "l2_l4_proc_comm.h"
#include "ovsvtep_group.h"
#include "ovsvtep_init.h"
#include "ovsvtep_mgr.h"
#include "ovsvtep_type.h"
#include "sys_callback_mgr.h"

#if 0
#include "util.h"
#include "ovsdb-idl.h"
#include "vswitch-idl.h"
#include "vtep-idl.h"
#endif

/* NAMING CONSTANT DECLARATIONS
 */
#if 0
#define VLOG_INFO_ONCE      VLOG_INFO

#define VLOG_INFO(args...)  do{static int cnt; if (cnt++%60 == 0) BACKDOOR_MGR_Printf(args);} while(0)

#define VLOG_ERR_RL(a,b)        BACKDOOR_MGR_Printf(b)
#define VLOG_ERR                BACKDOOR_MGR_Printf
#define OVSVTEP_DBG(args...)    BACKDOOR_MGR_Printf(args)
#endif

/* the size of the ipc msg buf for the MGR thread of OVSVTEP group to receive and
 * response PMGR ipc. The size of this buffer should pick the maximum of size
 * required for PMGR ipc request and response in all CSCs handled in this
 * process. (this size does not include IPC msg header)
 */
#define OVSVTEP_GROUP_PMGR_MSGBUF_MAX_REQUIRED_SIZE sizeof(OVSVTEP_GROUP_MgrMsg_T)


/* MACRO FUNCTION DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */

/* union all data type used for OVSVTEP group MGR IPC message to get the maximum
 * required ipc message buffer
 */
typedef union {
//    SYS_CALLBACK_MGR_LPort_CBData_T                 sys_callback_mgr_lport_cbdata;
//    SYS_CALLBACK_MGR_L2muxReceivePacket_CBData_T    sys_callback_mgr_l2mux_receive_packet_cbdata;
    SYS_CALLBACK_MGR_IPCFG_NsmRouteChange_CBData_T    sys_callback_mgr_nsm_route_change_cbdata;
    SYS_CALLBACK_MGR_RifActive_CBData_T               sys_callback_mgr_rif_active_cbdata;
    SYS_CALLBACK_MGR_RifDown_CBData_T                 sys_callback_mgr_rif_down_cbdata;
}   OVSVTEP_GROUP_Syscallback_CBData_T;

typedef struct
{
    UI8_T buffer[SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(OVSVTEP_GROUP_Syscallback_CBData_T))];
}   OVSVTEP_GROUP_Syscallback_Msg_T;

typedef union OVSVTEP_GROUP_MgrMsg_U
{
//    OVSVTEP_MGR_IpcMsg_T               OVSVTEP_mgr_ipcmsg;
    OVSVTEP_GROUP_Syscallback_Msg_T    OVSVTEP_syscb_ipcmsg;
} OVSVTEP_GROUP_MgrMsg_T;


/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void OVSVTEP_GROUP_Mgr_Thread_Function_Entry(void* arg);
static void OVSVTEP_GROUP_SetTransitionMode(void);
static void OVSVTEP_GROUP_EnterTransitionMode(void);
static void OVSVTEP_GROUP_EnterMasterMode(void);
static void OVSVTEP_GROUP_EnterSlaveMode(void);
static void OVSVTEP_GROUP_ProvisionComplete(void);
static void OVSVTEP_GROUP_HandleSysCallbackIPCMsg(
    SYSFUN_Msg_T    *msgbuf_p);
#if 0
static void OVSVTEP_GROUP_LPortOperUpCallbackHandler(
    UI32_T  lport);
static void OVSVTEP_GROUP_LPortNotOperUpCallbackHandler(
    UI32_T  lport);
static void OVSVTEP_GROUP_TrunkMemberAdd1stCallbackHandler(
    UI32_T  trunk_ifindex,
    UI32_T  member_ifindex);
static void OVSVTEP_GROUP_TrunkMemberAddCallbackHandler(
    UI32_T  trunk_ifindex,
    UI32_T  member_ifindex);
static void OVSVTEP_GROUP_TrunkMemberDeleteCallbackHandler(
    UI32_T  trunk_ifindex,
    UI32_T  member_ifindex);
static void OVSVTEP_GROUP_TrunkMemberDeleteLstCallbackHandler(
    UI32_T  trunk_ifindex,
    UI32_T  member_ifindex);
#endif
static void OVSVTEP_GROUP_RifActiveCallbackHandler(
    UI32_T ifindex,
    L_INET_AddrIp_T *addr_p);
static void OVSVTEP_GROUP_RifDownCallbackHandler(
    UI32_T ifindex,
    L_INET_AddrIp_T *addr_p);
static void OVSVTEP_GROUP_RouteChangeCallbackHandler(
    UI32_T address_family);
static void OVSVTEP_GROUP_OvsMacUpdateCallbackHandler(
    UI32_T ifindex, UI32_T vid, UI8_T *mac_p, BOOL_T added);

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
static void OVSVTEP_GROUP_HandleHotInsertion(SYSFUN_Msg_T* msgbuf_p);
static void OVSVTEP_GROUP_HandleHotRemoval(SYSFUN_Msg_T* msgbuf_p);
#endif /* #if (SYS_CPNT_UNIT_HOT_SWAP == TRUE) */


/* STATIC VARIABLE DEFINITIONS
 */

/* the buffer for retrieving ipc request for OVSVTEP group mgr thread
 */
static UI8_T OVSVTEP_mgrtd_ipc_buf[SYSFUN_SIZE_OF_MSG(OVSVTEP_GROUP_PMGR_MSGBUF_MAX_REQUIRED_SIZE)];

#if 0
static struct ovsdb_idl *vtep_idl;
static struct ovsdb_idl *ovsdb_idl;
static unsigned int vtep_idl_seqno;
static unsigned int ovsdb_idl_seqno;
static struct hmap all_tunnels;
#endif

/* EXPORTED SUBPROGRAM BODIES
 */
/*-----------------------------------------------------------------------------
 * FUNCTION NAME - OVSVTEP_GROUP_InitiateProcessResources
 *-----------------------------------------------------------------------------
 * PURPOSE: Initiate process resource for OVSVTEP group.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *-----------------------------------------------------------------------------
 */
void OVSVTEP_GROUP_InitiateProcessResources(void)
{
    OVSVTEP_INIT_InitiateSystemResources();
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - OVSVTEP_GROUP_Create_InterCSC_Relation
 *-----------------------------------------------------------------------------
 * PURPOSE: Create inter-CSC relationships for OVSVTEP group.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *-----------------------------------------------------------------------------
 */
void OVSVTEP_GROUP_Create_InterCSC_Relation(void)
{
    OVSVTEP_INIT_Create_InterCSC_Relation();
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - OVSVTEP_GROUP_Create_All_Threads
 *-----------------------------------------------------------------------------
 * PURPOSE: This function will spawn all threads in OVSVTEP group.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : All threads in the same CSC group will join the same thread group.
 *-----------------------------------------------------------------------------
 */
void OVSVTEP_GROUP_Create_All_Threads(void)
{
    UI32_T thread_id;

    if (SYSFUN_SpawnThread(SYS_BLD_OVSVTEP_GROUP_MGR_THREAD_PRIORITY,
                           SYS_BLD_OVSVTEP_GROUP_MGR_SCHED_POLICY,
                           SYS_BLD_OVSVTEP_GROUP_MGR_THREAD_NAME,
                           SYS_BLD_TASK_COMMON_STACK_SIZE,
                           SYSFUN_TASK_FP,
                           OVSVTEP_GROUP_Mgr_Thread_Function_Entry,
                           NULL,
                           &thread_id) != SYSFUN_OK)
    {
        printf("\n%s: Spawn OVSVTEP Group MGR thread fail.\n", __FUNCTION__);
    }
}


/* LOCAL SUBPROGRAM BODIES
 */
/*-----------------------------------------------------------------------------
 * FUNCTION NAME - OVSVTEP_GROUP_Mgr_Thread_Function_Entry
 *-----------------------------------------------------------------------------
 * PURPOSE: This is the entry function for the MGR thread of OVSVTEP group.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *-----------------------------------------------------------------------------
 */
static void OVSVTEP_GROUP_Mgr_Thread_Function_Entry(void* arg)
{
#if 1
    UI32_T                  ipc_msgq_handle;
    L_THREADGRP_Handle_T    tg_handle = L2_L4_PROC_COMM_GetOvsvtepGroupTGHandle();
    UI32_T                  member_id, received_events, local_events = SYSFUN_SYSTEM_EVENT_IPCMSG;
    SYSFUN_Msg_T            *msgbuf_p = (SYSFUN_Msg_T*)OVSVTEP_mgrtd_ipc_buf;
    UI32_T                  all_events =
                                SYSFUN_SYSTEM_EVENT_SET_TRANSITION_MODE |
                                SYSFUN_SYSTEM_EVENT_IPCMSG  |
                                SYSFUN_SYSTEM_EVENT_IPCFAIL |
                                OVSVTEP_TYPE_EVENT_TIMER_1000MS;

    void                    *OVSVTEP_timer_id;
    BOOL_T                  need_resp;

    /* join the thread group of OVSVTEP Group
     */
    if (L_THREADGRP_Join(tg_handle, SYS_BLD_OVSVTEP_GROUP_MGR_THREAD_PRIORITY,
            &member_id) == FALSE)
    {
        printf("\n%s: L_THREADGRP_Join fail.\n", __FUNCTION__);
        return;
    }

    /* create mgr ipc message queue handle
     */
    if (SYSFUN_CreateMsgQ(SYS_BLD_OVSVTEP_GROUP_IPCMSGQ_KEY,
            SYSFUN_MSGQ_BIDIRECTIONAL, &ipc_msgq_handle) != SYSFUN_OK)
    {
        printf("\n%s: SYSFUN_CreateMsgQ fail.\n", __FUNCTION__);
        return;
    }

    OVSVTEP_timer_id = SYSFUN_PeriodicTimer_Create();
    SYSFUN_PeriodicTimer_Start(OVSVTEP_timer_id, OVSVTEP_TYPE_TIMER_TICKS1000MS, OVSVTEP_TYPE_EVENT_TIMER_1000MS);

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
            OVSVTEP_GROUP_SetTransitionMode();
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
                    OVSVTEP_GROUP_PMGR_MSGBUF_MAX_REQUIRED_SIZE, msgbuf_p);

            if (ret == SYSFUN_OK)
            {
                /* request thread group execution permission
                 */
                if (L_THREADGRP_Execution_Request(tg_handle, member_id) != TRUE)
                {
                    printf("\n%s: L_THREADGRP_Execution_Request fail.\n",
                        __FUNCTION__);
                }

                /* handle request message based on cmd
                 */
                switch (msgbuf_p->cmd)
                {
                /* global cmd
                 */
                case SYS_TYPE_CMD_ENTER_TRANSITION_MODE:
                    OVSVTEP_GROUP_EnterTransitionMode();

                    /* SYS_TYPE_CMD_ENTER_TRANSITION_MODE
                     * need a response which contains nothing
                     */
                    msgbuf_p->msg_size = 0;
                    need_resp = TRUE;
                    break;

                case SYS_TYPE_CMD_ENTER_MASTER_MODE:
                    OVSVTEP_GROUP_EnterMasterMode();

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
                    OVSVTEP_GROUP_EnterSlaveMode();

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
                case SYS_TYPE_CMD_MODULE_PROVISION_COMPLETE:
                    OVSVTEP_GROUP_ProvisionComplete();
                    msgbuf_p->msg_size = 0;
                    need_resp = TRUE;
                    break;

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
                case SYS_TYPE_CMD_HANDLE_HOT_INSERTION:
                    OVSVTEP_GROUP_HandleHotInsertion(msgbuf_p);
                    msgbuf_p->msg_size=0;
                    need_resp=TRUE;
                    break;

                case SYS_TYPE_CMD_HANDLE_HOT_REMOVAL:
                    OVSVTEP_GROUP_HandleHotRemoval(msgbuf_p);
                    msgbuf_p->msg_size=0;
                    need_resp=TRUE;
                    break;
#endif /* #if (SYS_CPNT_UNIT_HOT_SWAP == TRUE) */

                /* module id cmd
                 */
                case SYS_MODULE_OVSVTEP:
                    need_resp = OVSVTEP_MGR_HandleIPCReqMsg(msgbuf_p);
                    break;

                case SYS_MODULE_BACKDOOR:
                    need_resp = BACKDOOR_MGR_HandleIPCMsg(msgbuf_p);
                    break;

                case SYS_MODULE_SYS_CALLBACK:
                    OVSVTEP_GROUP_HandleSysCallbackIPCMsg(msgbuf_p);
                    /* SYS_CALLBACK ipc message can only be uni-direction
                     * just set need_resp as FALSE
                     */
                    need_resp = FALSE;
                    break;

                case SYS_TYPE_CMD_RELOAD_SYSTEM:
                    msgbuf_p->msg_size=0;
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
                    printf("\n%s: Invalid IPC req cmd: %u.\n", __FUNCTION__, msgbuf_p->cmd);
                    need_resp = FALSE;
                }

                /* release thread group execution permission
                 */
                if (L_THREADGRP_Execution_Release(tg_handle, member_id) != TRUE)
                {
                    printf("\n%s: L_THREADGRP_Execution_Release fail.\n",
                        __FUNCTION__);
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

        /* handle timer evnet
         */
        {
            UI32_T  tmp_timer_event;

            tmp_timer_event = local_events & (OVSVTEP_TYPE_EVENT_TIMER_1000MS);
            if (tmp_timer_event)
            {
                if (L_THREADGRP_Execution_Request(tg_handle, member_id)!=TRUE)
                    printf("\r\n%s: L_THREADGRP_Execution_Request fail.", __FUNCTION__);

                {
                    OVSVTEP_MGR_ProcessVtep();
                }
                //OVSVTEP_MGR_ProcessTimerEvent(tmp_timer_event);

                if (L_THREADGRP_Execution_Release(tg_handle, member_id)!=TRUE)
                   printf("\r\n%s: L_THREADGRP_Execution_Release fail.", __FUNCTION__);

                local_events ^= tmp_timer_event;
            }
        }

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
    } /* end of while (1) */
#endif
} /* End of OVSVTEP_GROUP_Mgr_Thread_Function_Entry */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - OVSVTEP_GROUP_SetTransitionMode
 *-----------------------------------------------------------------------------
 * PURPOSE: This function will invoke all set transition mode function in
 *          OVSVTEP group.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *-----------------------------------------------------------------------------
 */
static void OVSVTEP_GROUP_SetTransitionMode(void)
{
    OVSVTEP_INIT_SetTransitionMode();
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - OVSVTEP_GROUP_EnterTransitionMode
 *-----------------------------------------------------------------------------
 * PURPOSE: This function will invoke all enter transition mode function in
 *          OVSVTEP group.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *-----------------------------------------------------------------------------
 */
static void OVSVTEP_GROUP_EnterTransitionMode(void)
{
    OVSVTEP_INIT_EnterTransitionMode();
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - OVSVTEP_GROUP_EnterMasterMode
 *-----------------------------------------------------------------------------
 * PURPOSE: This function will invoke all set master mode function in
 *          OVSVTEP group.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *-----------------------------------------------------------------------------
 */
static void OVSVTEP_GROUP_EnterMasterMode(void)
{
    OVSVTEP_INIT_EnterMasterMode();
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - OVSVTEP_GROUP_EnterSlaveMode
 *-----------------------------------------------------------------------------
 * PURPOSE: This function will invoke all enter slave mode function in
 *          OVSVTEP group.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *-----------------------------------------------------------------------------
 */
static void OVSVTEP_GROUP_EnterSlaveMode(void)
{
    OVSVTEP_INIT_EnterSlaveMode();
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - OVSVTEP_GROUP_ProvisionComplete
 *-----------------------------------------------------------------------------
 * PURPOSE: This function will invoke all provision complete function in
 *          OVSVTEP group.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *-----------------------------------------------------------------------------
 */
static void OVSVTEP_GROUP_ProvisionComplete(void)
{
    OVSVTEP_INIT_ProvisionComplete();
}

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
/*-----------------------------------------------------------------------------
 * FUNCTION NAME - OVSVTEP_GROUP_HandleHotInsertion
 *-----------------------------------------------------------------------------
 * PURPOSE: This function will invoke a new dut insertion in OVSVTEP Group.
 * INPUT  : msgbuf_p -- SYS_CALLBACK IPC message
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *-----------------------------------------------------------------------------
 */
static void OVSVTEP_GROUP_HandleHotInsertion(SYSFUN_Msg_T* msgbuf_p)
{
#if 0
    SYS_TYPE_HandleHotSwapArg_T     *msg_p;

    if (NULL != msgbuf_p)
    {
        msg_p = (SYS_TYPE_HandleHotSwapArg_T*) msgbuf_p->msg_buf;

        OVSVTEP_INIT_HandleHotInsertion(msg_p->starting_port_ifindex, msg_p->number_of_port, msg_p->use_default);
    }
#endif
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - OVSVTEP_GROUP_HandleHotRemoval
 *-----------------------------------------------------------------------------
 * PURPOSE: This function will invoke a dut removal in OVSVTEP Group.
 * INPUT  : msgbuf_p -- SYS_CALLBACK IPC message
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *-----------------------------------------------------------------------------
 */
static void OVSVTEP_GROUP_HandleHotRemoval(SYSFUN_Msg_T* msgbuf_p)
{
#if 0
    SYS_TYPE_HandleHotSwapArg_T     *msg_p;

    if (NULL !=  msgbuf_p)
    {
        msg_p = (SYS_TYPE_HandleHotSwapArg_T*) msgbuf_p->msg_buf;

        OVSVTEP_INIT_HandleHotRemoval(msg_p->starting_port_ifindex, msg_p->number_of_port);
    }
#endif
}
#endif /* #if (SYS_CPNT_UNIT_HOT_SWAP == TRUE) */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - OVSVTEP_GROUP_HandleSysCallbackIPCMsg
 *-----------------------------------------------------------------------------
 * PURPOSE: This function will handle all callbacks from IPC messages in
 *          OVSVTEP group.
 * INPUT  : msgbuf_p -- SYS_CALLBACK IPC message
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *-----------------------------------------------------------------------------
 */
static void OVSVTEP_GROUP_HandleSysCallbackIPCMsg(SYSFUN_Msg_T *msgbuf_p)
{
    SYS_CALLBACK_MGR_Msg_T *sys_cbmsg_p =
        (SYS_CALLBACK_MGR_Msg_T*)msgbuf_p->msg_buf;

    switch (sys_cbmsg_p->callback_event_id)
    {
    case SYS_CALLBACK_MGR_CALLBACKEVENTID_OVS_MAC_UPDATE:
        SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
            OVSVTEP_GROUP_OvsMacUpdateCallbackHandler);
        break;
    case SYS_CALLBACK_MGR_CALLBACKEVENTID_IPCFG_NSM_ROUTE_CHANGE:
        SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
            OVSVTEP_GROUP_RouteChangeCallbackHandler);
        break;
    case SYS_CALLBACK_MGR_CALLBACKEVENTID_RIF_ACTIVE:
        SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
            OVSVTEP_GROUP_RifActiveCallbackHandler);
        break;
    case SYS_CALLBACK_MGR_CALLBACKEVENTID_RIF_DOWN:
        SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
            OVSVTEP_GROUP_RifDownCallbackHandler);
        break;

    default:
        SYSFUN_Debug_Printf("%s: received callback_event that is not handled(%d)\n",
            __FUNCTION__, sys_cbmsg_p->callback_event_id);
        break;
    }
}

#if 0
/*-----------------------------------------------------------------------------
 * FUNCTION NAME - OVSVTEP_GROUP_LPortOperUpCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE: CallBack function
 * INPUT  : lport - lport to oper up
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *-----------------------------------------------------------------------------
 */
static void OVSVTEP_GROUP_LPortOperUpCallbackHandler(
    UI32_T  lport)
{
//    OVSVTEP_MGR_LPortLinkUp_CallBack(lport);
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - OVSVTEP_GROUP_LPortNotOperUpCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE: CallBack function
 * INPUT  : lport - lport to not oper up
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *-----------------------------------------------------------------------------
 */
static void OVSVTEP_GROUP_LPortNotOperUpCallbackHandler(
    UI32_T  lport)
{
//    OVSVTEP_MGR_LPortLinkDown_CallBack(lport);
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - OVSVTEP_GROUP_TrunkMemberAdd1stCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the callback event happening when the first port is added
 *           to a trunk.
 * INPUT   : trunk_ifindex  -- which trunk port
 *           member_ifindex -- which member port
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
static void OVSVTEP_GROUP_TrunkMemberAdd1stCallbackHandler(
    UI32_T  trunk_ifindex,
    UI32_T  member_ifindex)
{
//    OVSVTEP_MGR_AddTrkMbr_CallBack(trunk_ifindex, member_ifindex);
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - OVSVTEP_GROUP_TrunkMemberAddCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the callback event happening when a logical port is added
 *           to a trunk.
 * INPUT   : trunk_ifindex  -- which trunk port
 *           member_ifindex -- which member port
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
static void OVSVTEP_GROUP_TrunkMemberAddCallbackHandler(
    UI32_T  trunk_ifindex,
    UI32_T  member_ifindex)
{
//    OVSVTEP_MGR_AddTrkMbr_CallBack(trunk_ifindex, member_ifindex);
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - OVSVTEP_GROUP_TrunkMemberDeleteCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the callback event happening when a logical port is deleted
 *           from a trunk.
 * INPUT   : trunk_ifindex  -- which trunk port
 *           member_ifindex -- which member port
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
static void OVSVTEP_GROUP_TrunkMemberDeleteCallbackHandler(
    UI32_T  trunk_ifindex,
    UI32_T  member_ifindex)
{
//    OVSVTEP_MGR_DelTrkMbr_CallBack(trunk_ifindex, member_ifindex);
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - OVSVTEP_GROUP_TrunkMemberDeleteLstCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the callback event happening when the last port is deleted
 *           from a trunk.
 * INPUT   : trunk_ifindex  -- which trunk port
 *           member_ifindex -- which member port
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
static void OVSVTEP_GROUP_TrunkMemberDeleteLstCallbackHandler(
    UI32_T  trunk_ifindex,
    UI32_T  member_ifindex)
{
//    OVSVTEP_MGR_DelTrkMbr_CallBack(trunk_ifindex, member_ifindex);
}
#endif

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - OVSVTEP_GROUP_RifActiveCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Signal to OVSVTEP_MGR to indicate that rifActive.
 * INPUT   : ifindex  -- the ifindex  of active rif
 *           addr_p  --  the addr_p of active rif
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
static void OVSVTEP_GROUP_RifActiveCallbackHandler(UI32_T ifindex, L_INET_AddrIp_T *addr_p)
{
    OVSVTEP_MGR_RifActive_Callback(ifindex, addr_p);
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - OVSVTEP_GROUP_RifDownCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Signal to OVSVTEP_MGR to indicate that rifDown.
 * INPUT   : ifindex  -- the ifindex of down rif
 *           addr_p  --  the addr_p of down rif
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
static void OVSVTEP_GROUP_RifDownCallbackHandler(UI32_T ifindex, L_INET_AddrIp_T *addr_p)
{
    OVSVTEP_MGR_RifDown_Callback(ifindex, addr_p);
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - OVSVTEP_GROUP_RouteChangeCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the callback event happening when nsm rib route change.
 * INPUT   : address_family -- specify which address family has route change (IPv4/IPv6)
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
static void OVSVTEP_GROUP_RouteChangeCallbackHandler(
    UI32_T address_family)
{
    OVSVTEP_MGR_RouteChange_CallBack(address_family);
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - OVSVTEP_GROUP_OvsMacUpdateCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the callback event happening when nsm rib route change.
 * INPUT   : address_family -- specify which address family has route change (IPv4/IPv6)
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
static void OVSVTEP_GROUP_OvsMacUpdateCallbackHandler(
    UI32_T ifindex, UI32_T vid, UI8_T *mac_p, BOOL_T added)
{
    OVSVTEP_MGR_OvsMacUpdate_CallBack(ifindex, vid, mac_p, added);
}
#endif /* #if (SYS_CPNT_OVSVTEP == TRUE) */

