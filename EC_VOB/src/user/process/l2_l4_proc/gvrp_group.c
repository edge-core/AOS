/*-----------------------------------------------------------------------------
 * MODULE NAME: GVRP_GROUP.C
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *    Implements the APIs of GVRP group.
 *
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    2007/06/23     --- Timon, Create
 *
 * Copyright(C)      Accton Corporation, 2007
 *-----------------------------------------------------------------------------
 */


/* INCLUDE FILE DECLARATIONS
 */

#include "sys_bld.h"
#include "sys_module.h"
#include "sys_type.h"
#include "sys_cpnt.h"
#include "l_threadgrp.h"
#include "sysfun.h"
#include "backdoor_mgr.h"
#if (SYS_CPNT_L2MUX == TRUE)
#include "l2mux_mgr.h"
#endif
#if (SYS_CPNT_LLDP == TRUE)
#include "lldp_init.h"
#include "lldp_mgr.h"
#include "lldp_type.h"
#endif
#include "gvrp_group.h"
#include "l2_l4_proc_comm.h"
#include "sys_callback_mgr.h"
#if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE)
#include "cmgr.h"
#endif


/* NAMING CONSTANT DECLARATIONS
 */

/* the size of the ipc msg buf for the MGR thread of GVRP group to receive and
 * response PMGR ipc. The size of this buffer should pick the maximum of size
 * required for PMGR ipc request and response in all CSCs handled in this
 * process. (this size does not include IPC msg header)
 */
#define GVRP_GROUP_PMGR_MSGBUF_MAX_REQUIRED_SIZE sizeof(GVRP_GROUP_MgrMsg_T)


/* MACRO FUNCTION DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */

/* union all data type used for GVRP group MGR IPC message to get the maximum
 * required ipc message buffer
 */
typedef union GVRP_GROUP_MgrMsg_U
{
#if (SYS_CPNT_LLDP == TRUE)
    LLDP_MGR_IpcMsg_T lldp_mgr_ipcmsg;
#endif
    struct gvrp_sys_callback_msg
    {
        SYS_CALLBACK_MGR_Msg_T                       header;
        union
        {
        SYS_CALLBACK_MGR_L2muxReceivePacket_CBData_T payload;
#if (SYS_CPNT_REFINE_IPC_MSG == TRUE)
        SYS_CALLBACK_MGR_REFINEList_CBData_T  sys_callback_mgr_refinelist_cbdata;
#endif
        }sys_callback_payload;
    }receive_packet_ipcmsg;

    BACKDOOR_MGR_Msg_T  backdoor_mgr_ipcmsg;
#if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE)
    CMGR_IpcMsg_T       cmgr_ipcmsg;
#endif
} GVRP_GROUP_MgrMsg_T;


/* LOCAL SUBPROGRAM DECLARATIONS
 */

static void GVRP_GROUP_Mgr_Thread_Function_Entry(void* arg);
static void GVRP_GROUP_SetTransitionMode(void);
static void GVRP_GROUP_EnterTransitionMode(void);
static void GVRP_GROUP_EnterMasterMode(void);
static void GVRP_GROUP_EnterSlaveMode(void);
static void GVRP_GROUP_ProvisionComplete(void);
static void GVRP_GROUP_HandleSysCallbackIPCMsg(SYSFUN_Msg_T *msgbuf_p);

static void GVRP_GROUP_VlanMemberAddCallbackHandler(UI32_T vid_ifindex,
                                                    UI32_T lport_ifindex,
                                                    UI32_T vlan_status);
static void GVRP_GROUP_VlanMemberDeleteCallbackHandler(UI32_T vid_ifindex,
                                                       UI32_T lport_ifindex,
                                                       UI32_T vlan_status);
static void GVRP_GROUP_VlanCreateForGvrpCallbackHandler(UI32_T vid_ifindex,
                                                        UI32_T vlan_status);
static void GVRP_GROUP_VlanDestroyForGvrpCallbackHandler(UI32_T vid_ifindex,
                                                         UI32_T vlan_status);
static void GVRP_GROUP_VlanMemberAddForGvrpCallbackHandler(UI32_T vid_ifindex,
                                                           UI32_T lport_ifindex,
                                                           UI32_T vlan_status);
static void GVRP_GROUP_VlanMemberDeleteForGvrpCallbackHandler(UI32_T vid_ifindex,
                                                              UI32_T lport_ifindex,
                                                              UI32_T vlan_status);
static void GVRP_GROUP_VlanPortModeCallbackHandler(UI32_T lport_ifindex,
                                                   UI32_T vlan_port_mode);
static void GVRP_GROUP_LportChangeStateCallbackHandler(void);
static void GVRP_GROUP_LportOperUpCallbackHandler(UI32_T ifindex);
static void GVRP_GROUP_LportNotOperUpCallbackHandler(UI32_T ifindex);
static void GVRP_GROUP_FinishAddFirstTrunkMemberCallbackHandler(UI32_T trunk_ifindex,
                                                                UI32_T member_ifindex);
static void GVRP_GROUP_FinishAddTrunkMemberCallbackHandler(UI32_T trunk_ifindex,
                                                           UI32_T member_ifindex);
static void GVRP_GROUP_FinishDeleteTrunkMemberCallbackHandler(UI32_T trunk_ifindex,
                                                              UI32_T member_ifindex);
static void GVRP_GROUP_FinishDeleteLastTrunkMemberCallbackHandler(UI32_T trunk_ifindex,
                                                                  UI32_T member_ifindex);
static void GVRP_GROUP_VlanMemberDeleteByTrunkCallbackHandler(UI32_T vid_ifindex,
                                                              UI32_T lport_ifindex,
                                                              UI32_T vlan_status);
static void GVRP_GROUP_TrunkMemberAdd1stCallbackHandler(UI32_T trunk_ifindex,
                                                        UI32_T member_ifindex);
static void GVRP_GROUP_TrunkMemberAddCallbackHandler(UI32_T trunk_ifindex,
                                                     UI32_T member_ifindex);
static void GVRP_GROUP_TrunkMemberDeleteCallbackHandler(UI32_T trunk_ifindex,
                                                        UI32_T member_ifindex);
static void GVRP_GROUP_TrunkMemberDeleteLstCallbackHandler(UI32_T trunk_ifindex,
                                                           UI32_T member_ifindex);
static void GVRP_GROUP_LPortAdminDisableBeforeCallbackHandler(UI32_T ifindex);
static void GVRP_GROUP_PvidChangedCallbackHandler(UI32_T lport_ifindex,
                                                  UI32_T old_pvid,
                                                  UI32_T new_pvid);
#if 0
static void GVRP_GROUP_GvrpPacketCallbackHandler(L_MM_Mref_Handle_T *mref_handle_p,
                                                 UI8_T *dst_mac,
                                                 UI8_T *src_mac,
                                                 UI16_T tag_info,
                                                 UI16_T type,
                                                 UI32_T length,
                                                 UI32_T lport);
#else
static void GVRP_GROUP_GvrpPacketCallbackHandler(L_MM_Mref_Handle_T *mref_handle_p,
                                                        UI8_T *dst_mac,
                                                        UI8_T *src_mac,
                                                        UI16_T tag_info,
                                                        UI16_T type,
                                                        UI32_T pkt_length,
                                                        UI32_T  src_unit,
                                                        UI32_T  src_port,
                                                        UI32_T  packet_class);

#endif

#if 0
static void GVRP_GROUP_LldpPacketCallbackHandler(L_MM_Mref_Handle_T *mref_handle_p,
                                                 UI8_T *dst_mac,
                                                 UI8_T *src_mac,
                                                 UI16_T tag_info,
                                                 UI16_T type,
                                                 UI32_T length,
                                                 UI32_T lport);
#else
static void GVRP_GROUP_LldpPacketCallbackHandler(L_MM_Mref_Handle_T *mref_handle_p,
                                                        UI8_T *dst_mac,
                                                        UI8_T *src_mac,
                                                        UI16_T tag_info,
                                                        UI16_T type,
                                                        UI32_T pkt_length,
                                                        UI32_T  src_unit,
                                                        UI32_T  src_port,
                                                        UI32_T  packet_class);

#endif

#if (SYS_CPNT_LLDP == TRUE)
static void GVRP_GROUP_VlanNameChangedCallbackHandler(UI32_T vid);
static void GVRP_GROUP_ProtovlanGidBindingChangedCallbackHandler(UI32_T lport);
static void GVRP_GROUP_IfMauChangedCallbackHandler(UI32_T lport);
#endif /* end of #if (SYS_CPNT_LLDP == TRUE) */
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
static void GVRP_GROUP_HandleHotInertion(SYSFUN_Msg_T* msgbuf_p);
static void GVRP_GROUP_HandleHotRemoval(SYSFUN_Msg_T* msgbuf_p);
#endif
#if (SYS_CPNT_REFINE_IPC_MSG == TRUE)
static void GVRP_GROUP_VlanListCallbackHandler(UI32_T subid,SYS_CALLBACK_MGR_REFINEList_CBData_T*msg);
#endif
#if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE)
static void GVRP_GROUP_HandleCmgrIpcMsg(SYSFUN_Msg_T *ipc_msg_p);
#endif


/* STATIC VARIABLE DEFINITIONS
 */

/* the buffer for retrieving ipc request for stagroup mgr thread
 */
static UI8_T mgrtd_ipc_buf[SYSFUN_SIZE_OF_MSG(GVRP_GROUP_PMGR_MSGBUF_MAX_REQUIRED_SIZE)];


/* EXPORTED SUBPROGRAM BODIES
 */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - GVRP_GROUP_InitiateProcessResources
 *-----------------------------------------------------------------------------
 * PURPOSE : Initiate process resource for GVRP group.
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
void GVRP_GROUP_InitiateProcessResources(void)
{
#if (SYS_CPNT_LLDP == TRUE)
    LLDP_INIT_InitiateSystemResources();
#endif

    return;
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - GVRP_GROUP_Create_InterCSC_Relation
 *-----------------------------------------------------------------------------
 * PURPOSE : Create inter-CSC relationships for GVRP group.
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
void GVRP_GROUP_Create_InterCSC_Relation(void)
{
#if (SYS_CPNT_LLDP == TRUE)
    LLDP_INIT_Create_InterCSC_Relation();
#endif

    return;
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - GVRP_GROUP_Create_All_Threads
 *-----------------------------------------------------------------------------
 * PURPOSE : This function will spawn all threads in GVRP group.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : All threads in the same CSC group will join the same thread group.
 *-----------------------------------------------------------------------------
 */
void GVRP_GROUP_Create_All_Threads(void)
{
    UI32_T thread_id;

    if (SYSFUN_SpawnThread(SYS_BLD_GVRP_GROUP_MGR_THREAD_PRIORITY,
                           SYS_BLD_GVRP_GROUP_MGR_SCHED_POLICY,
                           SYS_BLD_GVRP_GROUP_MGR_THREAD_NAME,
                           SYS_BLD_TASK_COMMON_STACK_SIZE,
                           SYSFUN_TASK_FP,
                           GVRP_GROUP_Mgr_Thread_Function_Entry,
                           NULL,
                           &thread_id) != SYSFUN_OK)
    {
        printf("\n%s: Spawn STAGroup MGR thread fail.\n", __FUNCTION__);
    }

    return;
} /* End of GVRP_GROUP_Create_All_Threads */


/*-----------------------------------------------------------------------------
 * FUNCTION NAME - GVRP_GROUP_MACTableDeleteByVIDnPortCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Notify other CSC when detecting a new neighbor.
 *
 * INPUT   : lport                --
 *           mac_addr             --
 *           network_addr_subtype --
 *           network_addr         --
 *           network_addr_len     --
 *           network_addr_ifindex --
 *           tel_exist            --
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
void GVRP_GROUP_TelephoneDetectCallbackHandler(UI32_T lport,
                                               UI8_T *mac_addr,
                                               UI8_T network_addr_subtype,
                                               UI8_T *network_addr,
                                               UI8_T network_addr_len,
                                               UI32_T network_addr_ifindex,
                                               BOOL_T tel_exist)
{
    return;
}


/* LOCAL SUBPROGRAM BODIES
 */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - GVRP_GROUP_Mgr_Thread_Function_Entry
 *-----------------------------------------------------------------------------
 * PURPOSE : This is the entry function for the MGR thread of GVRP group.
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
static void GVRP_GROUP_Mgr_Thread_Function_Entry(void* arg)
{
    SYSFUN_MsgQ_T           ipc_msgq_handle;
    L_THREADGRP_Handle_T    tg_handle = L2_L4_PROC_COMM_GetGvrpGroupTGHandle();
    UI32_T                  member_id, received_events, local_events = SYSFUN_SYSTEM_EVENT_IPCMSG;
    SYSFUN_Msg_T            *msgbuf_p = (SYSFUN_Msg_T*)mgrtd_ipc_buf;
    BOOL_T                  need_resp;
#if (SYS_CPNT_LLDP == TRUE)
    void                    *lldp_timer_id;
#endif
    UI32_T                  all_events =
                                SYSFUN_SYSTEM_EVENT_SET_TRANSITION_MODE |
                                SYSFUN_SYSTEM_EVENT_IPCMSG |
#if (SYS_CPNT_LLDP == TRUE)
                                LLDP_TYPE_EVENT_TIMER |
#endif
                                SYSFUN_SYSTEM_EVENT_IPCFAIL;

    /* join the thread group of GVRP Group
     */
    if (L_THREADGRP_Join(tg_handle, SYS_BLD_GVRP_GROUP_MGR_THREAD_PRIORITY,
            &member_id) == FALSE)
    {
        printf("\n%s: L_THREADGRP_Join fail.\n", __FUNCTION__);
        return;
    }

    /* create mgr ipc message queue handle
     */
    if (SYSFUN_CreateMsgQ(SYS_BLD_GVRP_GROUP_IPCMSGQ_KEY,
            SYSFUN_MSGQ_BIDIRECTIONAL, &ipc_msgq_handle) != SYSFUN_OK)
    {
        printf("\n%s: SYSFUN_CreateMsgQ fail.\n", __FUNCTION__);
        return;
    }

#if (SYS_CPNT_LLDP == TRUE)
    lldp_timer_id = SYSFUN_PeriodicTimer_Create();
    SYSFUN_PeriodicTimer_Start(lldp_timer_id, LLDP_TYPE_TIMER_TICKS2SEC, LLDP_TYPE_EVENT_TIMER);
#endif

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
            GVRP_GROUP_SetTransitionMode();
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
            if (SYSFUN_ReceiveMsg(ipc_msgq_handle, 0, SYSFUN_TIMEOUT_NOWAIT,
                    GVRP_GROUP_PMGR_MSGBUF_MAX_REQUIRED_SIZE, msgbuf_p) == SYSFUN_OK)
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
                        GVRP_GROUP_EnterTransitionMode();

                        /* SYS_TYPE_CMD_ENTER_TRANSITION_MODE
                         * need a response which contains nothing
                         */
                        msgbuf_p->msg_size = 0;
                        need_resp = TRUE;
                        break;

                    case SYS_TYPE_CMD_ENTER_MASTER_MODE:
                        GVRP_GROUP_EnterMasterMode();

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
                        GVRP_GROUP_EnterSlaveMode();

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
                        GVRP_GROUP_ProvisionComplete();
                        msgbuf_p->msg_size = 0;
                        need_resp = TRUE;
                        break;

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
                     case  SYS_TYPE_CMD_HANDLE_HOT_INSERTION:
                       GVRP_GROUP_HandleHotInertion(msgbuf_p);

                        msgbuf_p->msg_size=0;
                        need_resp=TRUE;
                        break;

                     case SYS_TYPE_CMD_HANDLE_HOT_REMOVAL:
                       GVRP_GROUP_HandleHotRemoval(msgbuf_p);

                        msgbuf_p->msg_size=0;
                        need_resp=TRUE;
                         break;
#endif
                   /*add by fen.wang ,to process system reload msg,it is sent by stkctrl task*/
                   case  SYS_TYPE_CMD_RELOAD_SYSTEM:

                       msgbuf_p->msg_size=0;
                       need_resp=TRUE;
                       break;

                    /* module id cmd
                     */

#if (SYS_CPNT_LLDP == TRUE)
                    case SYS_MODULE_LLDP:
                        need_resp = LLDP_MGR_HandleIPCReqMsg(msgbuf_p);
                        break;
#endif

                    case SYS_MODULE_BACKDOOR:
                        need_resp = BACKDOOR_MGR_HandleIPCMsg(msgbuf_p);
                        break;

                    case SYS_MODULE_SYS_CALLBACK:
                        GVRP_GROUP_HandleSysCallbackIPCMsg(msgbuf_p);
                        /* SYS_CALLBACK ipc message can only be uni-direction
                         * just set need_resp as FALSE
                         */
                        need_resp = FALSE;
                        break;

#if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE)
                    case SYS_MODULE_CMGR:
                        GVRP_GROUP_HandleCmgrIpcMsg(msgbuf_p);
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

#if (SYS_CPNT_LLDP == TRUE)
        /* handle LLDP timer evnet
         */
        if (local_events & LLDP_TYPE_EVENT_TIMER)
        {
            /* request thread group execution permission
             */
            if (L_THREADGRP_Execution_Request(tg_handle, member_id) != TRUE)
            {
                printf("\r\n%s: L_THREADGRP_Execution_Request fail.\r\n",
                    __FUNCTION__);
            }

            LLDP_MGR_ProcessTimerEvent();

            /* release thread group execution permission
             */
            if (L_THREADGRP_Execution_Release(tg_handle, member_id) != TRUE)
            {
                printf("\r\n%s: L_THREADGRP_Execution_Release fail.\r\n",
                    __FUNCTION__);
            }

            local_events ^= LLDP_TYPE_EVENT_TIMER;
        }
#endif /* end of #if (SYS_CPNT_LLDP == TRUE) */

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
} /* End of GVRP_GROUP_Mgr_Thread_Function_Entry */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - GVRP_GROUP_SetTransitionMode
 *-----------------------------------------------------------------------------
 * PURPOSE : This function will invoke all set transition mode function in
 *           GVRP group.
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
static void GVRP_GROUP_SetTransitionMode(void)
{
#if (SYS_CPNT_LLDP == TRUE)
    LLDP_INIT_SetTransitionMode();
#endif

    return;
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - GVRP_GROUP_EnterTransitionMode
 *-----------------------------------------------------------------------------
 * PURPOSE : This function will invoke all enter transition mode function in
 *           GVRP group.
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
static void GVRP_GROUP_EnterTransitionMode(void)
{
#if (SYS_CPNT_LLDP == TRUE)
    LLDP_INIT_EnterTransitionMode();
#endif

    return;
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - GVRP_GROUP_EnterMasterMode
 *-----------------------------------------------------------------------------
 * PURPOSE : This function will invoke all set master mode function in
 *           GVRP group.
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
static void GVRP_GROUP_EnterMasterMode(void)
{
#if (SYS_CPNT_LLDP == TRUE)
    LLDP_INIT_EnterMasterMode();
#endif

    return;
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - GVRP_GROUP_EnterSlaveMode
 *-----------------------------------------------------------------------------
 * PURPOSE : This function will invoke all enter slave mode function in
 *           GVRP group.
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
static void GVRP_GROUP_EnterSlaveMode(void)
{
#if (SYS_CPNT_LLDP == TRUE)
    LLDP_INIT_EnterSlaveMode();
#endif

    return;
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - GVRP_GROUP_ProvisionComplete
 *-----------------------------------------------------------------------------
 * PURPOSE : This function will invoke all provision complete function in
 *           GVRP group.
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
static void GVRP_GROUP_ProvisionComplete(void)
{
#if (SYS_CPNT_LLDP == TRUE)
    LLDP_INIT_ProvisionComplete();
#endif

    return;
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - GVRP_GROUP_HandleSysCallbackIPCMsg
 *-----------------------------------------------------------------------------
 * PURPOSE : This function will handle all callbacks from IPC messages in
 *           GVRP group.
 *
 * INPUT   : msgbuf_p -- SYS_CALLBACK IPC message
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
static void GVRP_GROUP_HandleSysCallbackIPCMsg(SYSFUN_Msg_T *msgbuf_p)
{
    SYS_CALLBACK_MGR_Msg_T *sys_cbmsg_p =
        (SYS_CALLBACK_MGR_Msg_T*)msgbuf_p->msg_buf;

    switch (sys_cbmsg_p->callback_event_id)
    {
#if (SYS_CPNT_REFINE_IPC_MSG == TRUE)
     case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_LIST:
       SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                   GVRP_GROUP_VlanListCallbackHandler);
               break;
#endif

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_MEMBER_ADD:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                GVRP_GROUP_VlanMemberAddCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_MEMBER_DELETE:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                GVRP_GROUP_VlanMemberDeleteCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_CREATE_FOR_GVRP:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                GVRP_GROUP_VlanCreateForGvrpCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_DESTROY_FOR_GVRP:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                GVRP_GROUP_VlanDestroyForGvrpCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_MEMBER_ADD_FOR_GVRP:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                GVRP_GROUP_VlanMemberAddForGvrpCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_MEMBER_DELETE_FOR_GVRP:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                GVRP_GROUP_VlanMemberDeleteForGvrpCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_PORT_MODE:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                GVRP_GROUP_VlanPortModeCallbackHandler);
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_LPORT_CHANGE_STATE:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                GVRP_GROUP_LportChangeStateCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_PORT_OPER_UP:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                GVRP_GROUP_LportOperUpCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_PORT_NOT_OPER_UP:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                GVRP_GROUP_LportNotOperUpCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_FINISH_ADD_FIRST_TRUNK_MEMBER:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                GVRP_GROUP_FinishAddFirstTrunkMemberCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_FINISH_ADD_TRUNK_MEMBER:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                GVRP_GROUP_FinishAddTrunkMemberCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_FINISH_DELETE_TRUNK_MEMBER:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                GVRP_GROUP_FinishDeleteTrunkMemberCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_FINISH_DELETE_LAST_TRUNK_MEMBER:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                GVRP_GROUP_FinishDeleteLastTrunkMemberCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_MEMBER_DELETE_BY_TRUNK:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                GVRP_GROUP_VlanMemberDeleteByTrunkCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_TRUNK_MEMBER_ADD_1ST:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                GVRP_GROUP_TrunkMemberAdd1stCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_TRUNK_MEMBER_ADD:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                GVRP_GROUP_TrunkMemberAddCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_TRUNK_MEMBER_DELETE:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                GVRP_GROUP_TrunkMemberDeleteCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_TRUNK_MEMBER_DELETE_LST:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                GVRP_GROUP_TrunkMemberDeleteLstCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_PORT_ADMIN_DISABLE_BEFORE:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                GVRP_GROUP_LPortAdminDisableBeforeCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_PVID_CHANGE:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                GVRP_GROUP_PvidChangedCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_GVRP_PACKET:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                GVRP_GROUP_GvrpPacketCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_LLDP_PACKET:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                GVRP_GROUP_LldpPacketCallbackHandler);
            break;

#if (SYS_CPNT_LLDP == TRUE)
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_NAME_CHANGED:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                GVRP_GROUP_VlanNameChangedCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_PROTOVLAN_GID_BINDING_CHANGED:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                GVRP_GROUP_ProtovlanGidBindingChangedCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_IF_MAU_CHANGED:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                GVRP_GROUP_IfMauChangedCallbackHandler);
            break;
#endif /* end of #if (SYS_CPNT_LLDP == TRUE) */

        default:
            SYSFUN_Debug_Printf(
                "\n%s: received callback_event that is not handled(%d)\n",
                __FUNCTION__, sys_cbmsg_p->callback_event_id);
    }
} /* End of GVRP_GROUP_HandleSysCallbackIPCMsg */

#if (SYS_CPNT_REFINE_IPC_MSG == TRUE)
/*-----------------------------------------------------------------------------
 * FUNCTION NAME - GVRP_GROUP_VlanListCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the callback event happening when vlans are  created/ports added to vlan/ports removed from vlans.
 *
 * INPUT   : subid -- the detail action to do
 *                  msg--- msg data
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
static void GVRP_GROUP_VlanListCallbackHandler(UI32_T subid,SYS_CALLBACK_MGR_REFINEList_CBData_T*msg)
{
    UI32_T vid_ifindex,lport_index,vlan_status,vid;

    switch(subid)
    {
     case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_CREATE:
        vlan_status = msg->arg.arg1.value;
        for(vid= 1; vid<=SYS_ADPT_MAX_NBR_OF_VLAN;vid++)
        {
         if(SYS_CALLBACK_MGR_IS_MEMBER(msg->list.vlanlist,vid))
         {

           VLAN_VID_CONVERTTO_IFINDEX(vid, vid_ifindex);

           GVRP_GROUP_VlanCreateCallbackHandler(vid_ifindex,vlan_status);

         }
       }
     break;

      case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_MEMBER_ADD:
       lport_index = msg->arg.arg2.value[0];
       vlan_status = msg->arg.arg2.value[1];
        for(vid= 1; vid<=SYS_ADPT_MAX_NBR_OF_VLAN;vid++)
        {
         if(SYS_CALLBACK_MGR_IS_MEMBER(msg->list.vlanlist,vid))
         {
            VLAN_VID_CONVERTTO_IFINDEX(vid, vid_ifindex);

            GVRP_GROUP_VlanMemberAddCallbackHandler(vid_ifindex,lport_index,vlan_status);

         }
       }
    break;

      case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_MEMBER_DELETE:
        lport_index = msg->arg.arg2.value[0];
        vlan_status = msg->arg.arg2.value[1];
        for(vid= 1; vid<=SYS_ADPT_MAX_NBR_OF_VLAN;vid++)
        {
         if(SYS_CALLBACK_MGR_IS_MEMBER(msg->list.vlanlist,vid))
         {

           VLAN_VID_CONVERTTO_IFINDEX(vid, vid_ifindex);

           GVRP_GROUP_VlanMemberDeleteCallbackHandler(vid_ifindex,lport_index,vlan_status);

         }
       }
    break;
      default:
      break;
   }
}
#endif /* #if (SYS_CPNT_REFINE_IPC_MSG == TRUE) */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - GVRP_GROUP_VlanMemberAddCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the callback event happening when a lport is added to a
 *           vlan's member set.
 *
 * INPUT   : vid_ifindex    -- specify which vlan has just been deleted
 *           lport_ifindex  -- sepcify which lport to be deleted
 *           vlan_status    -- VAL_dot1qVlanStatus_other \
 *                             VAL_dot1qVlanStatus_permanent \
 *                             VAL_dot1qVlanStatus_dynamicGvrp
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
static void GVRP_GROUP_VlanMemberAddCallbackHandler(UI32_T vid_ifindex,
                                                    UI32_T lport_ifindex,
                                                    UI32_T vlan_status)
{
#if (SYS_CPNT_LLDP == TRUE)
    LLDP_MGR_VlanMemberChanged_CallBack(lport_ifindex);
#endif
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - GVRP_GROUP_VlanMemberDeleteCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the callback event happening when a port is removed from
 *           vlan's member set.
 *
 * INPUT   : vid_ifindex    -- specify which vlan has just been deleted
 *           lport_ifindex  -- sepcify which lport to be deleted
 *           vlan_status    -- VAL_dot1qVlanStatus_other \
 *                             VAL_dot1qVlanStatus_permanent \
 *                             VAL_dot1qVlanStatus_dynamicGvrp
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
static void GVRP_GROUP_VlanMemberDeleteCallbackHandler(UI32_T vid_ifindex,
                                                       UI32_T lport_ifindex,
                                                       UI32_T vlan_status)
{
#if (SYS_CPNT_LLDP == TRUE)
    LLDP_MGR_VlanMemberChanged_CallBack(lport_ifindex);
#endif
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - GVRP_GROUP_VlanCreateForGvrpCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the callback event happening when a vlan is created.
 *
 * INPUT   : vid_ifindex -- specify which vlan has just been deleted
 *           vlan_status -- VAL_dot1qVlanStatus_other \
 *                          VAL_dot1qVlanStatus_permanent \
 *                          VAL_dot1qVlanStatus_dynamicGvrp
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
static void GVRP_GROUP_VlanCreateForGvrpCallbackHandler(UI32_T vid_ifindex,
                                                        UI32_T vlan_status)
{
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - GVRP_GROUP_VlanDestroyForGvrpCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the callback event happening when a vlan is deleted.
 *
 * INPUT   : vid_ifindex -- specify which vlan has just been deleted
 *           vlan_status -- VAL_dot1qVlanStatus_other \
 *                          VAL_dot1qVlanStatus_permanent \
 *                          VAL_dot1qVlanStatus_dynamicGvrp
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
static void GVRP_GROUP_VlanDestroyForGvrpCallbackHandler(UI32_T vid_ifindex,
                                                         UI32_T vlan_status)
{
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - GVRP_GROUP_VlanMemberAddForGvrpCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the callback event happening when a lport is added to a
 *           vlan's member set.
 *
 * INPUT   : vid_ifindex    -- specify which vlan has just been deleted
 *           lport_ifindex  -- sepcify which lport to be deleted
 *           vlan_status    -- VAL_dot1qVlanStatus_other \
 *                             VAL_dot1qVlanStatus_permanent \
 *                             VAL_dot1qVlanStatus_dynamicGvrp
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
static void GVRP_GROUP_VlanMemberAddForGvrpCallbackHandler(UI32_T vid_ifindex,
                                                           UI32_T lport_ifindex,
                                                           UI32_T vlan_status)
{
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - GVRP_GROUP_VlanMemberDeleteForGvrpCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the callback event happening when a port is removed from
 *           vlan's member set.
 *
 * INPUT   : vid_ifindex    -- specify which vlan has just been deleted
 *           lport_ifindex  -- sepcify which lport to be deleted
 *           vlan_status    -- VAL_dot1qVlanStatus_other \
 *                             VAL_dot1qVlanStatus_permanent \
 *                             VAL_dot1qVlanStatus_dynamicGvrp
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
static void GVRP_GROUP_VlanMemberDeleteForGvrpCallbackHandler(
                UI32_T vid_ifindex, UI32_T lport_ifindex, UI32_T vlan_status)
{
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - GVRP_GROUP_VlanPortModeCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the callback event happening when vlan port mode has been
 *           changed to access mode.
 *
 * INPUT   : lport_ifindex  -- the specific lport to be notify
 *           vlan_port_mode -- value of this field after modification
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
static void GVRP_GROUP_VlanPortModeCallbackHandler(UI32_T lport_ifindex,
                                                   UI32_T vlan_port_mode)
{
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - GVRP_GROUP_LportChangeStateCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the callback event happening when the port enters/leaves
 *           the forwarding state.
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
static void GVRP_GROUP_LportChangeStateCallbackHandler(void)
{
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - GVRP_GROUP_LportOperUpCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the callback event happening when port's oper status is up.
 *
 * INPUT   : ifindex -- which logical port
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
static void GVRP_GROUP_LportOperUpCallbackHandler(UI32_T ifindex)
{
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - GVRP_GROUP_LportNotOperUpCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the callback event happening when port's oper status is not
 *           up.
 *
 * INPUT   : ifindex -- which logical port
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
static void GVRP_GROUP_LportNotOperUpCallbackHandler(UI32_T ifindex)
{
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - GVRP_GROUP_FinishAddFirstTrunkMemberCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the callback event happening when the first member port is
 *           added to a trunk port.
 *
 * INPUT   : trunk_ifindex   -- specify the trunk port ifindex
 *           member_ifindex  -- specify the member port that joined trunking
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
static void GVRP_GROUP_FinishAddFirstTrunkMemberCallbackHandler(UI32_T trunk_ifindex,
                                                                UI32_T member_ifindex)
{
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - GVRP_GROUP_FinishAddTrunkMemberCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the callback event happening when member port is added to
 *           a trunk port.
 *
 * INPUT   : trunk_ifindex   -- specify the trunk port ifindex
 *           member_ifindex  -- specify the member port that joined trunking
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
static void GVRP_GROUP_FinishAddTrunkMemberCallbackHandler(UI32_T trunk_ifindex,
                                                           UI32_T member_ifindex)
{
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - GVRP_GROUP_FinishDeleteTrunkMemberCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the callback event happening when a member port is deleted
 *           from a trunk port.
 *
 * INPUT   : trunk_ifindex   -- specify the trunk port ifindex
 *           member_ifindex  -- specify the member port that is going to be
 *                              removed from the trunk
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
static void GVRP_GROUP_FinishDeleteTrunkMemberCallbackHandler(UI32_T trunk_ifindex,
                                                              UI32_T member_ifindex)
{
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - GVRP_GROUP_FinishDeleteLastTrunkMemberCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the callback event happening when the last member port is
 *           removed from the trunk port.
 *
 * INPUT   : trunk_ifindex   -- specify the trunk port ifindex
 *           member_ifindex  -- specify the member port that is going to be
 *                              removed from the trunk
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
static void GVRP_GROUP_FinishDeleteLastTrunkMemberCallbackHandler(UI32_T trunk_ifindex,
                                                                  UI32_T member_ifindex)
{
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - GVRP_GROUP_VlanMemberDeleteByTrunkCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the callback event happening when a port disjoins a
 *           VLAN by the reason that it joins a trunk.
 *
 * INPUT   : vid_ifindex -- specify which vlan's member set to be deleted
 *           lport_ifidx -- sepcify which lport to be deleted
 *           vlan_status -- VAL_dot1qVlanStatus_other \
 *                          VAL_dot1qVlanStatus_permanent \
 *                          VAL_dot1qVlanStatus_dynamicGvrp
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
static void GVRP_GROUP_VlanMemberDeleteByTrunkCallbackHandler(UI32_T vid_ifindex,
                                                              UI32_T lport_ifindex,
                                                              UI32_T vlan_status)
{
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - GVRP_GROUP_TrunkMemberAdd1stCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the callback event happening when the first port is added
 *           to a trunk.
 *
 * INPUT   : trunk_ifindex  -- which trunk port
 *           member_ifindex -- which member port
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
static void GVRP_GROUP_TrunkMemberAdd1stCallbackHandler(UI32_T trunk_ifindex,
                                                        UI32_T member_ifindex)
{
#if (SYS_CPNT_LLDP == TRUE)
    LLDP_MGR_TrunkMemberAdd1st_CallBack(trunk_ifindex, member_ifindex);
#endif
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - GVRP_GROUP_TrunkMemberAddCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the callback event happening when a logical port is added
 *           to a trunk.
 *
 * INPUT   : trunk_ifindex  -- which trunk port
 *           member_ifindex -- which member port
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
static void GVRP_GROUP_TrunkMemberAddCallbackHandler(UI32_T trunk_ifindex,
                                                     UI32_T member_ifindex)
{
#if (SYS_CPNT_LLDP == TRUE)
    LLDP_MGR_TrunkMemberAdd_CallBack(trunk_ifindex, member_ifindex);
#endif
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - GVRP_GROUP_TrunkMemberDeleteCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the callback event happening when a logical port is deleted
 *           from a trunk.
 *
 * INPUT   : trunk_ifindex  -- which trunk port
 *           member_ifindex -- which member port
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
static void GVRP_GROUP_TrunkMemberDeleteCallbackHandler(UI32_T trunk_ifindex,
                                                        UI32_T member_ifindex)
{
#if (SYS_CPNT_LLDP == TRUE)
    LLDP_MGR_TrunkMemberDelete_CallBack(trunk_ifindex, member_ifindex);
#endif
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - GVRP_GROUP_TrunkMemberDeleteLstCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the callback event happening when the last port is deleted
 *           from a trunk.
 *
 * INPUT   : trunk_ifindex  -- which trunk port
 *           member_ifindex -- which member port
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
static void GVRP_GROUP_TrunkMemberDeleteLstCallbackHandler(UI32_T trunk_ifindex,
                                                           UI32_T member_ifindex)
{
#if (SYS_CPNT_LLDP == TRUE)
    LLDP_MGR_TrunkMemberDeleteLst_CallBack(trunk_ifindex, member_ifindex);
#endif
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - GVRP_GROUP_LPortAdminDisableBeforeCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the callback event happening before a port is admin
 *           disabled.
 *
 * INPUT   : ifindex -- which logical port
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
static void GVRP_GROUP_LPortAdminDisableBeforeCallbackHandler(UI32_T ifindex)
{

}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - GVRP_GROUP_PvidChangedCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the callback event happening when the pvid of a port
 *           changes.
 *
 * INPUT   : lport_ifindex -- the specific port this modification is of
 *           old_pvid      -- previous pvid before modification
 *           new_pvid      -- new and current pvid after modification
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
static void GVRP_GROUP_PvidChangedCallbackHandler(UI32_T lport_ifindex,
                                                  UI32_T old_pvid,
                                                  UI32_T new_pvid)
{
#if (SYS_CPNT_LLDP == TRUE)
    LLDP_MGR_PvidChanged_CallBack(lport_ifindex, old_pvid, new_pvid);
#endif
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - GVRP_GROUP_GvrpPacketCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the callback event happening when a network interface
 *           received a GVRP PDU packet.
 *
 * INPUT   : mref_handle_p -- packet buffer and return buffer function pointer.
 *           dst_mac       -- the destination MAC address of this packet.
 *           src_mac       -- the source MAC address of this packet.
 *           tag_info      -- tag information
 *           type          -- packet type
 *           pkt_length    -- pdu length
 *           lport         -- source lport
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
#if 0
static void GVRP_GROUP_GvrpPacketCallbackHandler(L_MM_Mref_Handle_T *mref_handle_p,
                                                 UI8_T *dst_mac,
                                                 UI8_T *src_mac,
                                                 UI16_T tag_info,
                                                 UI16_T type,
                                                 UI32_T length,
                                                 UI32_T lport)
{
    GARP_MGR_GarpPduRcvd_Callback(mref_handle_p,
                                  dst_mac,
                                  src_mac,
                                  tag_info,
                                  type,
                                  length,
                                  lport);
}
#else
/*EPR: N/A
    Problem: 1, a gvrp pkt received ,it will sent to l2mut by msg in order to change the port from user port to logic port
                     2 l2mux will send the pkt to csc
                     3 the pkt received by csc will slow
    Solution: the pkt will directly sent from driver to csc without l2mux

*/
static void GVRP_GROUP_GvrpPacketCallbackHandler(L_MM_Mref_Handle_T *mref_handle_p,
                                                        UI8_T *dst_mac,
                                                        UI8_T *src_mac,
                                                        UI16_T tag_info,
                                                        UI16_T type,
                                                        UI32_T pkt_length,
                                                        UI32_T  src_unit,
                                                        UI32_T  src_port,
                                                        UI32_T  packet_class)
{
#if (SYS_CPNT_L2MUX == TRUE)
    if (L2MUX_MGR_PreprocessPkt_CallbackFunc(mref_handle_p,dst_mac,src_mac,tag_info,type,
                                             pkt_length,src_unit,src_port,packet_class))
    {
        /* MREF had been handled and released
         */
        return;
    }
#endif

    L_MM_Mref_Release(&mref_handle_p);
}

#endif

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - GVRP_GROUP_LldpPacketCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the callback event happening when a network interface
 *           received a LLDPDU packet.
 *
 * INPUT   : mref_handle_p -- packet buffer and return buffer function pointer.
 *           dst_mac       -- the destination MAC address of this packet.
 *           src_mac       -- the source MAC address of this packet.
 *           tag_info      -- tag information
 *           type          -- packet type
 *           pkt_length    -- pdu length
 *           lport         -- source lport
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
#if 0
static void GVRP_GROUP_LldpPacketCallbackHandler(L_MM_Mref_Handle_T *mref_handle_p,
                                                 UI8_T *dst_mac,
                                                 UI8_T *src_mac,
                                                 UI16_T tag_info,
                                                 UI16_T type,
                                                 UI32_T length,
                                                 UI32_T lport)
{
    LLDP_MGR_LldpduRcvd_Callback(mref_handle_p,
                                 dst_mac,
                                 src_mac,
                                 tag_info,
                                 type,
                                 length,
                                 lport);
}
#else
/*EPR: N/A
    Problem: 1, a gvrp pkt received ,it will sent to l2mut by msg in order to change the port from user port to logic port
                     2 l2mux will send the pkt to csc
                     3 the pkt received by csc will slow
    Solution: the pkt will directly sent from driver to csc without l2mux

*/

static void GVRP_GROUP_LldpPacketCallbackHandler(L_MM_Mref_Handle_T *mref_handle_p,
                                                        UI8_T *dst_mac,
                                                        UI8_T *src_mac,
                                                        UI16_T tag_info,
                                                        UI16_T type,
                                                        UI32_T pkt_length,
                                                        UI32_T  src_unit,
                                                        UI32_T  src_port,
                                                        UI32_T  packet_class)
{
#if (SYS_CPNT_L2MUX == TRUE)
    if (L2MUX_MGR_PreprocessPkt_CallbackFunc(mref_handle_p,dst_mac,src_mac,tag_info,type,
                                             pkt_length,src_unit,src_port,packet_class))
    {
        /* MREF had been handled and released
         */
        return;
    }
#endif

#if (SYS_CPNT_LLDP == TRUE)
    LLDP_MGR_LldpduRcvd_Callback(mref_handle_p,
                                 dst_mac,
                                 src_mac,
                                 tag_info,
                                 type,
                                 pkt_length,
                                 src_unit,
                                 src_port,
                                 packet_class);
#else
    L_MM_Mref_Release(&mref_handle_p);
#endif
}

#endif

#if (SYS_CPNT_LLDP == TRUE)
/*-----------------------------------------------------------------------------
 * FUNCTION NAME - GVRP_GROUP_VlanNameChangedCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the callback event happening when a vlan's name has
 *           been changed.
 *
 * INPUT   : vid -- specify the id of vlan whose name has been changed
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
static void GVRP_GROUP_VlanNameChangedCallbackHandler(UI32_T vid)
{
    LLDP_MGR_NotifyVlanNameChanged(vid);
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - GVRP_GROUP_ProtovlanGidBindingChangedCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the callback event happening when the protocol vlan
 *           group id binding for a port has been changed.
 *
 * INPUT   : lport -- the logical port whose protocol group id binding has
 *                    been changed
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
static void GVRP_GROUP_ProtovlanGidBindingChangedCallbackHandler(UI32_T lport)
{
    LLDP_MGR_NotifyProtoVlanGroupIdBindingChanged(lport);
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - GVRP_GROUP_IfMauChangedCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the callback event happening when the MAU of a port has
 *           been changed.
 *
 * INPUT   : lport -- which logical port
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
static void GVRP_GROUP_IfMauChangedCallbackHandler(UI32_T lport)
{
    LLDP_MGR_NotifyIfMauChanged(lport);
}
#endif /* end of #if (SYS_CPNT_LLDP == TRUE) */

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
/*------------------------------------------------------------------------------
 * ROUTINE NAME : GVRP_GROUP_HandleHotInertion
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
static void GVRP_GROUP_HandleHotInertion(SYSFUN_Msg_T* msgbuf_p)
{
    SYS_TYPE_HandleHotSwapArg_T *msg_p;

    if (msgbuf_p == NULL)
        return;

    msg_p = (SYS_TYPE_HandleHotSwapArg_T*)msgbuf_p->msg_buf;
#if (SYS_CPNT_LLDP == TRUE)
    LLDP_INIT_HandleHotInsertion(msg_p->starting_port_ifindex, msg_p->number_of_port, msg_p->use_default);
#endif
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : GVRP_GROUP_ HandleHotRemoval
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
static void GVRP_GROUP_HandleHotRemoval(SYSFUN_Msg_T* msgbuf_p)
{
    SYS_TYPE_HandleHotSwapArg_T *msg_p;

    if (msgbuf_p == NULL)
        return;

    msg_p = (SYS_TYPE_HandleHotSwapArg_T*)msgbuf_p->msg_buf;

#if (SYS_CPNT_LLDP == TRUE)
    LLDP_INIT_HandleHotRemoval(msg_p->starting_port_ifindex, msg_p->number_of_port);
#endif
}
#endif

#if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE)
static void GVRP_GROUP_HandleCmgrIpcMsg(SYSFUN_Msg_T *ipc_msg_p)
{
    CMGR_IpcMsg_T *cmgr_msg_p;

    if (ipc_msg_p == NULL)
    {
        return;
    }

    cmgr_msg_p = (CMGR_IpcMsg_T*)ipc_msg_p->msg_buf;
    switch (cmgr_msg_p->type.cmd)
    {
    case CMGR_IPC_GVRP_VLAN_CHANGE:
        if (cmgr_msg_p->data.arg_ui32_ui32_bool_bool.arg_bool_1 == TRUE)
        {
            if (cmgr_msg_p->data.arg_ui32_ui32_bool_bool.arg_bool_2 == TRUE)
            {
                GVRP_GROUP_VlanDestroyForGvrpCallbackHandler(
                    cmgr_msg_p->data.arg_ui32_ui32_bool_bool.arg_ui32_1,
                    cmgr_msg_p->data.arg_ui32_ui32_bool_bool.arg_ui32_2);
            }
            GVRP_GROUP_VlanCreateForGvrpCallbackHandler(
                cmgr_msg_p->data.arg_ui32_ui32_bool_bool.arg_ui32_1,
                cmgr_msg_p->data.arg_ui32_ui32_bool_bool.arg_ui32_2);
        }
        else
        {
            GVRP_GROUP_VlanDestroyForGvrpCallbackHandler(
                cmgr_msg_p->data.arg_ui32_ui32_bool_bool.arg_ui32_1,
                cmgr_msg_p->data.arg_ui32_ui32_bool_bool.arg_ui32_2);
        }
        break;

    case CMGR_IPC_GVRP_VLAN_MEMBER_CHANGE:
        if (cmgr_msg_p->data.arg_ui32_ui32_ui32_bool_bool.arg_bool_1 == TRUE)
        {
            /* do not care merged, because only process state transition */
            GVRP_GROUP_VlanMemberAddForGvrpCallbackHandler(
                cmgr_msg_p->data.arg_ui32_ui32_ui32_bool_bool.arg_ui32_1,
                cmgr_msg_p->data.arg_ui32_ui32_ui32_bool_bool.arg_ui32_2,
                cmgr_msg_p->data.arg_ui32_ui32_ui32_bool_bool.arg_ui32_3);
        }
        else
        {
            GVRP_GROUP_VlanMemberDeleteForGvrpCallbackHandler(
                cmgr_msg_p->data.arg_ui32_ui32_ui32_bool_bool.arg_ui32_1,
                cmgr_msg_p->data.arg_ui32_ui32_ui32_bool_bool.arg_ui32_2,
                cmgr_msg_p->data.arg_ui32_ui32_ui32_bool_bool.arg_ui32_3);
        }
        break;

    case CMGR_IPC_PORT_VLAN_CHANGE:
        LLDP_MGR_VlanMemberChanged_CallBack(cmgr_msg_p->data.arg_ui32);
        break;

    case CMGR_IPC_PORT_VLAN_MODE_CHANGE:
        GVRP_GROUP_VlanPortModeCallbackHandler(
            cmgr_msg_p->data.arg_ui32_ui32.arg_ui32_1,
            cmgr_msg_p->data.arg_ui32_ui32.arg_ui32_2);
        break;

    case CMGR_IPC_PVID_CHANGE:
        GVRP_GROUP_PvidChangedCallbackHandler(
            cmgr_msg_p->data.arg_ui32_ui32_ui32.arg_ui32_1,
            cmgr_msg_p->data.arg_ui32_ui32_ui32.arg_ui32_2,
            cmgr_msg_p->data.arg_ui32_ui32_ui32.arg_ui32_3);
        break;

    case CMGR_IPC_VLAN_NAME_CHANGE:
        GVRP_GROUP_VlanNameChangedCallbackHandler(cmgr_msg_p->data.arg_ui32);
        break;

    case CMGR_IPC_PROTOCOL_VLAN_CHANGE:
        GVRP_GROUP_ProtovlanGidBindingChangedCallbackHandler(
            cmgr_msg_p->data.arg_ui32);
        break;
    } /* end switch */

    return;
}
#endif /* #if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE) */
