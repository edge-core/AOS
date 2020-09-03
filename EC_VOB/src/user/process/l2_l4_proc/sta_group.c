/*-----------------------------------------------------------------------------
 * MODULE NAME: STA_GROUP.C
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *    The module implements the fucntionality of STA group.
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

#include <stdio.h>
#include "sys_bld.h"
#include "sys_module.h"
#include "sys_type.h"
#include "sys_cpnt.h"
#include "l_hold_timer.h"
#include "l_threadgrp.h"
#include "sysfun.h"
#include "backdoor_mgr.h"
#include "bridge_init.h"
#include "extbrg_mgr.h"
#include "l2_l4_proc_comm.h"
#if (SYS_CPNT_L2MUX == TRUE)
#include "l2mux_mgr.h"
#endif
#if (SYS_CPNT_SNMP == TRUE)
#include "snmp_pmgr.h"
#endif
#include "sta_group.h"
#include "sys_callback_mgr.h"
#if (SYS_CPNT_SYSLOG == TRUE)
#include "syslog_pmgr.h"
#endif
#include "xstp_mgr.h"
#include "xstp_type.h"
#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
#include "sw_watchdog_mgr.h"
#endif
#if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE)
#include "cmgr.h"
#endif


/* NAMING CONSTANT DECLARATIONS
 */

/* the size of the ipc msg buf for the MGR thread of STA group to receive and
 * response PMGR ipc. The size of this buffer should pick the maximum of size
 * required for PMGR ipc request and response in all CSCs handled in this
 * process. (this size does not include IPC msg header)
 */
#define PMGR_MSGBUF_MAX_REQUIRED_SIZE sizeof(STA_GROUP_MgrMsg_T)


/* MACRO FUNCTION DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */

/* union all data type used for STA group MGR IPC message to get the maximum
 * required ipc message buffer
 */
typedef union STA_GROUP_MgrMsg_U
{
    EXTBRG_MGR_IpcMsg_T extbrg_mgr_ipcmsg;
    XSTP_MGR_IpcMsg_T   xstp_mgr_ipcmsg;
#if (SYS_CPNT_REFINE_IPC_MSG == TRUE)
    struct
    {
        SYS_CALLBACK_MGR_Msg_T                       header;
        union
        {
            SYS_CALLBACK_MGR_REFINEList_CBData_T  sys_callback_mgr_refinelist_cbdata;
        } sta_group_sys_callback_payload;
    } sta_group_sys_callback_msg;
#endif
    BACKDOOR_MGR_Msg_T  backdoor_mgr_ipcmsg;
#if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE)
    CMGR_IpcMsg_T       cmgr_ipcmsg;
#endif
} STA_GROUP_MgrMsg_T;


/* LOCAL SUBPROGRAM DECLARATIONS
 */

static void STA_GROUP_Mgr_Thread_Function_Entry(void* arg);
static void STA_GROUP_SetTransitionMode(void);
static void STA_GROUP_EnterTransitionMode(void);
static void STA_GROUP_EnterMasterMode(void);
static void STA_GROUP_EnterSlaveMode(void);
static void STA_GROUP_ProvisionComplete(void);
static void STA_GROUP_HandleSysCallbackIPCMsg(SYSFUN_Msg_T *msgbuf_p);

static void STA_GROUP_LportOperUpCallbackHandler(UI32_T ifindex);
static void STA_GROUP_LportNotOperUpCallbackHandler(UI32_T ifindex);
static void STA_GROUP_LportAdminEnableCallbackHandler(UI32_T ifindex);
static void STA_GROUP_LportAdminDisableCallbackHandler(UI32_T ifindex);
static void STA_GROUP_LportSpeedDuplexCallbackHandler(UI32_T ifindex,
                                                      UI32_T speed_duplex);
static void STA_GROUP_TrunkMemberAdd1stCallbackHandler(UI32_T trunk_ifindex,
                                                       UI32_T member_ifindex);
static void STA_GROUP_TrunkMemberAddCallbackHandler(UI32_T trunk_ifindex,
                                                    UI32_T member_ifindex);
static void STA_GROUP_TrunkMemberDeleteCallbackHandler(UI32_T trunk_ifindex,
                                                       UI32_T member_ifindex);
static void STA_GROUP_TrunkMemberDeleteLstCallbackHandler(UI32_T trunk_ifindex,
                                                          UI32_T member_ifindex);
static void STA_GROUP_TrunkMemberPortOperUpCallbackHandler(UI32_T trunk_ifindex,
                                                           UI32_T member_ifindex);
static void STA_GROUP_TrunkMemberPortNotOperUpCallbackHandler(UI32_T trunk_ifindex,
                                                              UI32_T member_ifindex);
static void STA_GROUP_VlanCreateCallbackHandler(UI32_T vid_ifindex,
                                                UI32_T vlan_status);
static void STA_GROUP_VlanDestroyCallbackHandler(UI32_T vid_ifindex,
                                                 UI32_T vlan_status);
static void STA_GROUP_VlanMemberAddCallbackHandler(UI32_T vid_ifindex,
                                                   UI32_T lport_ifindex,
                                                   UI32_T vlan_status);
static void STA_GROUP_VlanMemberDeleteCallbackHandler(UI32_T vid_ifindex,
                                                      UI32_T lport_ifindex,
                                                      UI32_T vlan_status);
#if (SYS_CPNT_REFINE_IPC_MSG == TRUE)
static void STA_GROUP_VlanListCallbackHandler(UI32_T subid,SYS_CALLBACK_MGR_REFINEList_CBData_T*msg);
#endif
static void STA_GROUP_VlanFinishAddFirstTrunkMemberCallbackHandler(
    UI32_T trunk_ifindex, UI32_T member_ifindex);
static void STA_GROUP_VlanFinishAddTrunkMemberCallbackHandler(
    UI32_T trunk_ifindex, UI32_T member_ifindex);
static void STA_GROUP_VlanFinishDeleteTrunkMemberCallbackHandler(
    UI32_T trunk_ifindex, UI32_T member_ifindex);
static void STA_GROUP_VlanFinishDeleteLastTrunkMemberCallbackHandler(
    UI32_T trunk_ifindex, UI32_T member_ifindex);
static void STA_GROUP_VlanMemberTagChangedCallbackHandler(UI32_T vid_ifindex,
                                                          UI32_T lport_ifindex);
static void STA_GROUP_PvidChangeCallbackHandler(UI32_T lport_ifindex,
                                                UI32_T old_pvid,
                                                UI32_T new_pvid);
static void STA_GROUP_PortStatusChangedPassivelyCallbackHandler(UI32_T ifindex,
                                                                BOOL_T status,
                                                                UI32_T changed_bmp);
static void STA_GROUP_StaPacketCallbackHandler(L_MM_Mref_Handle_T *mref_handle_p,
                                               UI8_T              *dst_mac,
                                               UI8_T              *src_mac,
                                               UI16_T             tag_info,
                                               UI16_T             type,
                                               UI32_T             pkt_length,
                                               UI32_T             src_unit,
                                               UI32_T             src_port,
                                               UI32_T             packet_class);

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
static void STA_GROUP_HandleHotInertion(SYSFUN_Msg_T* msgbuf_p);

static void STA_GROUP_HandleHotRemoval(SYSFUN_Msg_T* msgbuf_p);
#endif
#if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE)
static void STA_GROUP_HandleCmgrIpcMsg(SYSFUN_Msg_T *ipc_msg_p);
#endif


/* STATIC VARIABLE DEFINITIONS
 */

/* the buffer for retrieving ipc request for stagroup mgr thread
 */
static UI8_T mgrtd_ipc_buf[SYSFUN_SIZE_OF_MSG(PMGR_MSGBUF_MAX_REQUIRED_SIZE)];


/* EXPORTED SUBPROGRAM BODIES
 */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - STA_GROUP_InitiateProcessResources
 *-----------------------------------------------------------------------------
 * PURPOSE : Initiate process resource for STA group.
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
void STA_GROUP_InitiateProcessResources(void)
{
    BRIDGE_INIT_InitiateSystemResources();
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - STA_GROUP_Create_InterCSC_Relation
 *-----------------------------------------------------------------------------
 * PURPOSE : Create inter-CSC relationships for STA group.
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
void STA_GROUP_Create_InterCSC_Relation(void)
{
    BRIDGE_INIT_Create_InterCSC_Relation();

}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - STA_GROUP_Create_All_Threads
 *-----------------------------------------------------------------------------
 * PURPOSE : This function will spawn all threads in STA group.
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
void STA_GROUP_Create_All_Threads(void)
{
    UI32_T thread_id;

    if (SYSFUN_SpawnThread(SYS_BLD_STA_GROUP_MGR_THREAD_PRIORITY,
                           SYS_BLD_STA_GROUP_MGR_SCHED_POLICY,
                           SYS_BLD_STA_GROUP_MGR_THREAD_NAME,
                           SYS_BLD_TASK_COMMON_STACK_SIZE,
                           SYSFUN_TASK_FP,
                           STA_GROUP_Mgr_Thread_Function_Entry,
                           NULL,
                           &thread_id) != SYSFUN_OK)
    {
        printf("\r\n%s: Spawn STAGroup MGR thread fail.\r\n", __FUNCTION__);
    }

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
    SW_WATCHDOG_MGR_RegisterMonitorThread(SW_WATCHDOG_STA_GROUP, thread_id, SYS_ADPT_STA_GROUP_SW_WATCHDOG_TIMER);
#endif

    return;
}


/* LOCAL SUBPROGRAM BODIES
 */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - STA_GROUP_Mgr_Thread_Function_Entry
 *-----------------------------------------------------------------------------
 * PURPOSE : This is the entry function for the MGR thread of STA group.
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
static void STA_GROUP_Mgr_Thread_Function_Entry(void* arg)
{
    SYSFUN_MsgQ_T           ipc_msgq_handle;
    L_THREADGRP_Handle_T    tg_handle = L2_L4_PROC_COMM_GetStaGroupTGHandle();
    UI32_T                  member_id, received_events, local_events = SYSFUN_SYSTEM_EVENT_IPCMSG;
    SYSFUN_Msg_T            *msgbuf_p = (SYSFUN_Msg_T*)mgrtd_ipc_buf;
    BOOL_T                  need_resp;
    UI32_T                  all_events =
                                SYSFUN_SYSTEM_EVENT_SET_TRANSITION_MODE |
                                SYSFUN_SYSTEM_EVENT_IPCMSG              |
                                XSTP_TYPE_EVENT_TIMER                   |
                                XSTP_TYPE_EVENT_HDTIMER                 |
#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
                                SYSFUN_SYSTEM_EVENT_SW_WATCHDOG         |
#endif
                                SYSFUN_SYSTEM_EVENT_IPCFAIL;

    /* join the thread group of STA Group
     */
    if (L_THREADGRP_Join(tg_handle, SYS_BLD_STA_GROUP_MGR_THREAD_PRIORITY,
            &member_id) == FALSE)
    {
        printf("\r\n%s: L_THREADGRP_Join fail.\r\n", __FUNCTION__);
        return;
    }

    /* create mgr ipc message queue handle
     */
    if (SYSFUN_CreateMsgQ(SYS_BLD_STA_GROUP_IPCMSGQ_KEY,
            SYSFUN_MSGQ_BIDIRECTIONAL, &ipc_msgq_handle) != SYSFUN_OK)
    {
        printf("\r\n%s: SYSFUN_CreateMsgQ fail.\r\n", __FUNCTION__);
        return;
    }

    XSTP_TASK_StartTimerEvent();
    /* Trap */
#if (SYS_CPNT_SNMP == TRUE)
    SNMP_PMGR_NotifyStaTplgChanged();
#endif
#if (SYS_CPNT_SYSLOG==TRUE)
    SYSLOG_PMGR_NotifyStaTplgChanged();
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
            STA_GROUP_SetTransitionMode();
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
                    PMGR_MSGBUF_MAX_REQUIRED_SIZE, msgbuf_p) == SYSFUN_OK)
            {
                /* request thread group execution permission
                 */
                if (L_THREADGRP_Execution_Request(tg_handle, member_id) != TRUE)
                {
                    printf("\r\n%s: L_THREADGRP_Execution_Request fail.\r\n",
                        __FUNCTION__);
                }

                /* handle request message based on cmd
                 */
                switch (msgbuf_p->cmd)
                {
                    /* global cmd
                     */

                    case SYS_TYPE_CMD_ENTER_TRANSITION_MODE:
                        STA_GROUP_EnterTransitionMode();
                        local_events ^= XSTP_TYPE_EVENT_TIMER;
                        local_events ^= XSTP_TYPE_EVENT_HDTIMER;

                        /* SYS_TYPE_CMD_ENTER_TRANSITION_MODE
                         * need a response which contains nothing
                         */
                        msgbuf_p->msg_size = 0;
                        need_resp = TRUE;
                        break;

                    case SYS_TYPE_CMD_ENTER_MASTER_MODE:
                        STA_GROUP_EnterMasterMode();

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
                        STA_GROUP_EnterSlaveMode();
                        local_events ^= XSTP_TYPE_EVENT_TIMER;
                        local_events ^= XSTP_TYPE_EVENT_HDTIMER;

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
                        STA_GROUP_ProvisionComplete();
                        msgbuf_p->msg_size = 0;
                        need_resp = TRUE;
                        break;

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
                    case SYS_TYPE_CMD_HANDLE_HOT_INSERTION:
                        STA_GROUP_HandleHotInertion(msgbuf_p);
                        msgbuf_p->msg_size=0;
                        need_resp=TRUE;
                        break;

                    case SYS_TYPE_CMD_HANDLE_HOT_REMOVAL:
                        STA_GROUP_HandleHotRemoval(msgbuf_p);
                        msgbuf_p->msg_size=0;
                        need_resp=TRUE;
                         break;
#endif

                    /*add by fen.wang ,to process system reload msg,it is sent by stkctrl task*/
                    case SYS_TYPE_CMD_RELOAD_SYSTEM:
                        msgbuf_p->msg_size=0;
                        need_resp=TRUE;
                        break;

                    /* module id cmd
                     */

                    case SYS_MODULE_EXTBRG:
                        need_resp = EXTBRG_MGR_HandleIPCReqMsg(msgbuf_p);
                        break;

                    case SYS_MODULE_XSTP:
                        need_resp = XSTP_MGR_HandleIPCReqMsg(msgbuf_p);
                        break;

                    case SYS_MODULE_BACKDOOR:
                        need_resp = BACKDOOR_MGR_HandleIPCMsg(msgbuf_p);
                        break;

                    case SYS_MODULE_SYS_CALLBACK:
                        STA_GROUP_HandleSysCallbackIPCMsg(msgbuf_p);
                        /* SYS_CALLBACK ipc message can only be uni-direction
                         * just set need_resp as FALSE
                         */
                        need_resp = FALSE;
                        break;

#if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE)
                    case SYS_MODULE_CMGR:
                        STA_GROUP_HandleCmgrIpcMsg(msgbuf_p);
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
                        printf("\r\n%s: Invalid IPC req cmd.\r\n", __FUNCTION__);
                        need_resp = FALSE;
                }

                /* release thread group execution permission
                 */
                if (L_THREADGRP_Execution_Release(tg_handle, member_id) != TRUE)
                {
                    printf("\r\n%s: L_THREADGRP_Execution_Release fail.\r\n",
                        __FUNCTION__);
                }

                if ((need_resp == TRUE) && (SYSFUN_SendResponseMsg(
                        ipc_msgq_handle, msgbuf_p) != SYSFUN_OK))
                {
                    printf("\r\n%s: SYSFUN_SendResponseMsg fail.\r\n",
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
        if (local_events & XSTP_TYPE_EVENT_TIMER)
        {
            /* request thread group execution permission
             */
            if (L_THREADGRP_Execution_Request(tg_handle, member_id) != TRUE)
            {
                printf("\r\n%s: L_THREADGRP_Execution_Request fail.\r\n",
                    __FUNCTION__);
            }

            XSTP_MGR_Decrease();

            XSTP_MGR_ProcessTimerEvent();

            /* release thread group execution permission
             */
            if (L_THREADGRP_Execution_Release(tg_handle, member_id) != TRUE)
            {
                printf("\r\n%s: L_THREADGRP_Execution_Release fail.\r\n",
                    __FUNCTION__);
            }

            local_events ^= XSTP_TYPE_EVENT_TIMER;
        }

        /* handle hold timer event
         */
        if (local_events & XSTP_TYPE_EVENT_HDTIMER)
        {
            /* request thread group execution permission
             */
            if (L_THREADGRP_Execution_Request(tg_handle, member_id) != TRUE)
            {
                printf("\r\n%s: L_THREADGRP_Execution_Request fail.\r\n",
                    __FUNCTION__);
            }

            XSTP_TASK_ProcessHdTimer(L_HOLD_TIMER_PERIODIC_UPDATE_EV);

            /* release thread group execution permission
             */
            if (L_THREADGRP_Execution_Release(tg_handle, member_id) != TRUE)
            {
                printf("\r\n%s: L_THREADGRP_Execution_Release fail.\r\n",
                    __FUNCTION__);
            }

            local_events ^= XSTP_TYPE_EVENT_HDTIMER;
        }

        /* request thread group execution permission
         */
        if (L_THREADGRP_Execution_Request(tg_handle, member_id) != TRUE)
        {
            printf("\r\n%s: L_THREADGRP_Execution_Request fail.\r\n",
                __FUNCTION__);
        }
        XSTP_MGR_NotifyLportChangeState();
        /* release thread group execution permission
         */
        if (L_THREADGRP_Execution_Release(tg_handle, member_id) != TRUE)
        {
            printf("\r\n%s: L_THREADGRP_Execution_Release fail.\r\n",
                __FUNCTION__);
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

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
        if(local_events & SYSFUN_SYSTEM_EVENT_SW_WATCHDOG)
        {
            SW_WATCHDOG_MGR_ResetTimer(SW_WATCHDOG_STA_GROUP);
            local_events ^= SYSFUN_SYSTEM_EVENT_SW_WATCHDOG;
        }
#endif

    } /* end of while (1) */
} /* End of STA_GROUP_Mgr_Thread_Function_Entry */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - STA_GROUP_SetTransitionMode
 *-----------------------------------------------------------------------------
 * PURPOSE : This function will invoke all set transition mode function in
 *           STA group.
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
static void STA_GROUP_SetTransitionMode(void)
{
#if (SYS_CPNT_BRIDGE == TRUE)
    BRIDGE_INIT_SetTransitionMode();
#endif
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - STA_GROUP_EnterTransitionMode
 *-----------------------------------------------------------------------------
 * PURPOSE : This function will invoke all enter transition mode function in
 *           STA group.
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
static void STA_GROUP_EnterTransitionMode(void)
{
#if (SYS_CPNT_BRIDGE == TRUE)
    BRIDGE_INIT_EnterTransitionMode();
#endif
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - STA_GROUP_EnterMasterMode
 *-----------------------------------------------------------------------------
 * PURPOSE : This function will invoke all set master mode function in
 *           STA group.
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
static void STA_GROUP_EnterMasterMode(void)
{
#if (SYS_CPNT_BRIDGE == TRUE)
    BRIDGE_INIT_EnterMasterMode();
#endif
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - STA_GROUP_EnterSlaveMode
 *-----------------------------------------------------------------------------
 * PURPOSE : This function will invoke all enter slave mode function in
 *           STA group.
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
static void STA_GROUP_EnterSlaveMode(void)
{
#if (SYS_CPNT_BRIDGE == TRUE)
    BRIDGE_INIT_EnterSlaveMode();
#endif
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - STA_GROUP_ProvisionComplete
 *-----------------------------------------------------------------------------
 * PURPOSE : This function will invoke all provision complete function in
 *           STA group.
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
static void STA_GROUP_ProvisionComplete(void)
{
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - STA_GROUP_HandleSysCallbackIPCMsg
 *-----------------------------------------------------------------------------
 * PURPOSE : This function will handle all callbacks from IPC messages in
 *           STA group.
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
static void STA_GROUP_HandleSysCallbackIPCMsg(SYSFUN_Msg_T *msgbuf_p)
{
    SYS_CALLBACK_MGR_Msg_T *sys_cbmsg_p =
        (SYS_CALLBACK_MGR_Msg_T*)msgbuf_p->msg_buf;

    switch (sys_cbmsg_p->callback_event_id)
    {
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_PORT_OPER_UP:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                STA_GROUP_LportOperUpCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_PORT_NOT_OPER_UP:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                STA_GROUP_LportNotOperUpCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_PORT_ADMIN_ENABLE:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                STA_GROUP_LportAdminEnableCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_PORT_ADMIN_DISABLE:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                STA_GROUP_LportAdminDisableCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_PORT_SPEED_DUPLEX:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                STA_GROUP_LportSpeedDuplexCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_TRUNK_MEMBER_ADD_1ST:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                STA_GROUP_TrunkMemberAdd1stCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_TRUNK_MEMBER_ADD:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                STA_GROUP_TrunkMemberAddCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_TRUNK_MEMBER_DELETE:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                STA_GROUP_TrunkMemberDeleteCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_TRUNK_MEMBER_DELETE_LST:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                STA_GROUP_TrunkMemberDeleteLstCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_TRUNK_MEMBER_ACTIVE:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                STA_GROUP_TrunkMemberPortOperUpCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_TRUNK_MEMBER_INACTIVE:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                STA_GROUP_TrunkMemberPortNotOperUpCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_CREATE:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                STA_GROUP_VlanCreateCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_DESTROY:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                STA_GROUP_VlanDestroyCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_MEMBER_ADD:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                STA_GROUP_VlanMemberAddCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_MEMBER_DELETE:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                STA_GROUP_VlanMemberDeleteCallbackHandler);
            break;

#if (SYS_CPNT_REFINE_IPC_MSG == TRUE)
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_LIST:
          SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                      STA_GROUP_VlanListCallbackHandler);
            break;
#endif

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_FINISH_ADD_FIRST_TRUNK_MEMBER:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                STA_GROUP_VlanFinishAddFirstTrunkMemberCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_FINISH_ADD_TRUNK_MEMBER:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                STA_GROUP_VlanFinishAddTrunkMemberCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_FINISH_DELETE_TRUNK_MEMBER:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                STA_GROUP_VlanFinishDeleteTrunkMemberCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_FINISH_DELETE_LAST_TRUNK_MEMBER:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                STA_GROUP_VlanFinishDeleteLastTrunkMemberCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_MEMBER_TAG_CHANGED:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                STA_GROUP_VlanMemberTagChangedCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_PVID_CHANGE:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                STA_GROUP_PvidChangeCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_PORT_STATUS_CHANGED_PASSIVELY:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                STA_GROUP_PortStatusChangedPassivelyCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_STA_PACKET:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                STA_GROUP_StaPacketCallbackHandler);
            break;

        default:
            SYSFUN_Debug_Printf(
                "\n%s: received callback_event that is not handled(%d)\n",
                __FUNCTION__, sys_cbmsg_p->callback_event_id);
    }
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - STA_GROUP_LportOperUpCallbackHandler
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
static void STA_GROUP_LportOperUpCallbackHandler(UI32_T ifindex)
{
#if (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_RSTP) || (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_MSTP)
    XSTP_MGR_LportLinkup_CallBack(ifindex);
#endif
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - STA_GROUP_LportNotOperUpCallbackHandler
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
static void STA_GROUP_LportNotOperUpCallbackHandler(UI32_T ifindex)
{
#if (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_RSTP) || (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_MSTP)
    XSTP_MGR_LportLinkdown_CallBack(ifindex);
#endif
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - STA_GROUP_LportAdminEnableCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the callback event happening when port is enabled.
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
static void STA_GROUP_LportAdminEnableCallbackHandler(UI32_T ifindex)
{
#if (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_RSTP) || (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_MSTP)
    XSTP_MGR_PortAdminEnable_CallBack(ifindex);
#endif
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - STA_GROUP_LportAdminDisableCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the callback event happening when port is disabled.
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
static void STA_GROUP_LportAdminDisableCallbackHandler(UI32_T ifindex)
{
#if (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_RSTP) || (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_MSTP)
    XSTP_MGR_PortAdminDisable_CallBack(ifindex);
#endif
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - STA_GROUP_LportSpeedDuplexCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the callback event happening when the speed or duplex of a
 *           port is changed.
 *
 * INPUT   : ifindex      -- which logical port
 *           speed_duplex -- new status of speed/duplex
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
static void STA_GROUP_LportSpeedDuplexCallbackHandler(UI32_T ifindex,
                                                      UI32_T speed_duplex)
{
#if (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_RSTP) || (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_MSTP)
    XSTP_MGR_PortSpeedDuplex_CallBack(ifindex, speed_duplex);
#endif
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - STA_GROUP_TrunkMemberAdd1stCallbackHandler
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
static void STA_GROUP_TrunkMemberAdd1stCallbackHandler(UI32_T trunk_ifindex,
                                                       UI32_T member_ifindex)
{
#if (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_RSTP) || (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_MSTP)
    XSTP_MGR_TrunkMemberAdd1st_CallBack(trunk_ifindex, member_ifindex);
#endif
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - STA_GROUP_TrunkMemberAddCallbackHandler
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
static void STA_GROUP_TrunkMemberAddCallbackHandler(UI32_T trunk_ifindex,
                                                    UI32_T member_ifindex)
{
#if (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_RSTP) || (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_MSTP)
    XSTP_MGR_TrunkMemberAdd_CallBack(trunk_ifindex, member_ifindex);
#endif
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - STA_GROUP_TrunkMemberDeleteCallbackHandler
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
static void STA_GROUP_TrunkMemberDeleteCallbackHandler(UI32_T trunk_ifindex,
                                                       UI32_T member_ifindex)
{
#if (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_RSTP) || (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_MSTP)
    XSTP_MGR_TrunkMemberDelete_CallBack(trunk_ifindex, member_ifindex);
#endif
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - STA_GROUP_TrunkMemberDeleteLstCallbackHandler
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
static void STA_GROUP_TrunkMemberDeleteLstCallbackHandler(UI32_T trunk_ifindex,
                                                          UI32_T member_ifindex)
{
#if (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_RSTP) || (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_MSTP)
    XSTP_MGR_TrunkMemberDeleteLst_CallBack(trunk_ifindex, member_ifindex);
#endif
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - STA_GROUP_TrunkMemberPortOperUpCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the callback event happening when a trunk member port's
 *           oper status is up.
 *
 * INPUT   : trunk_ifindex             -- which trunk port
 *           trunk_member_port_ifindex -- which member port
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
static void STA_GROUP_TrunkMemberPortOperUpCallbackHandler(UI32_T trunk_ifindex,
                                                           UI32_T trunk_member_port_ifindex)
{
#if (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_RSTP) || (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_MSTP)
    XSTP_MGR_TrunkMemberPortOperUp_CallBack(trunk_ifindex, trunk_member_port_ifindex);
#endif
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - STA_GROUP_TrunkMemberPortNotOperUpCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the callback event happening when a trunk member port's
 *           oper status is not up.
 *
 * INPUT   : trunk_ifindex             -- which trunk port
 *           trunk_member_port_ifindex -- which member port
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
static void STA_GROUP_TrunkMemberPortNotOperUpCallbackHandler(UI32_T trunk_ifindex,
                                                              UI32_T trunk_member_port_ifindex)
{
#if (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_RSTP) || (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_MSTP)
    XSTP_MGR_TrunkMemberPortNotOperUp_CallBack(trunk_ifindex, trunk_member_port_ifindex);
#endif
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - STA_GROUP_VlanCreateCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the callback event happening when a vlan is created.
 *
 * INPUT   : vid_ifindex -- specify which vlan has just been created
 *           vlan_status -- VAL_dot1qVlanStatus_other
 *                          VAL_dot1qVlanStatus_permanent
 *                          VAL_dot1qVlanStatus_dynamicGvrp
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
static void STA_GROUP_VlanCreateCallbackHandler(UI32_T vid_ifindex,
                                                UI32_T vlan_status)
{
#if (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_RSTP) || (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_MSTP)
    XSTP_MGR_VlanCreated_CallBack(vid_ifindex, vlan_status);
#endif
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - STA_GROUP_VlanDestroyCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the callback event happening when a vlan is deleted.
 *
 * INPUT   : vid_ifindex -- specify which vlan has just been deleted
 *           vlan_status -- VAL_dot1qVlanStatus_other
 *                          VAL_dot1qVlanStatus_permanent
 *                          VAL_dot1qVlanStatus_dynamicGvrp
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
static void STA_GROUP_VlanDestroyCallbackHandler(UI32_T vid_ifindex,
                                                 UI32_T vlan_status)
{
#if (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_RSTP) || (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_MSTP)
    XSTP_MGR_VlanDestroy_CallBack(vid_ifindex, vlan_status);
#endif
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - STA_GROUP_VlanMemberAddCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the callback event happening when a lport is added to a
 *           vlan's member set.
 *
 * INPUT   : vid_ifindex   -- specify which vlan member set to join
 *           lport_ifindex -- sepcify which lport to join to the member set
 *           vlan_status   -- VAL_dot1qVlanStatus_other
 *                            VAL_dot1qVlanStatus_permanent
 *                            VAL_dot1qVlanStatus_dynamicGvrp
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
static void STA_GROUP_VlanMemberAddCallbackHandler(UI32_T vid_ifindex,
                                                   UI32_T lport_ifindex,
                                                   UI32_T vlan_status)
{
#if (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_RSTP) || (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_MSTP)
    XSTP_MGR_VlanMemberAdd_CallBack(vid_ifindex, lport_ifindex, vlan_status);
#endif
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - STA_GROUP_VlanMemberDeleteCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the callback event happening when a port is remove from
 *           vlan's member set.
 *
 * INPUT   : vid_ifindex   -- specify which vlan's member set to disjoin from
 *           lport_ifindex -- sepcify which lport to disjoin from the member set
 *           vlan_status   -- VAL_dot1qVlanStatus_other
 *                            VAL_dot1qVlanStatus_permanent
 *                            VAL_dot1qVlanStatus_dynamicGvrp
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
static void STA_GROUP_VlanMemberDeleteCallbackHandler(UI32_T vid_ifindex,
                                                      UI32_T lport_ifindex,
                                                      UI32_T vlan_status)
{
#if (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_RSTP) || (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_MSTP)
    XSTP_MGR_VlanMemberDelete_CallBack(vid_ifindex, lport_ifindex, vlan_status);
#endif
}

#if (SYS_CPNT_REFINE_IPC_MSG == TRUE)
/*-----------------------------------------------------------------------------
 * FUNCTION NAME - STA_GROUP_VlanListCallbackHandler
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
static void STA_GROUP_VlanListCallbackHandler(UI32_T subid,SYS_CALLBACK_MGR_REFINEList_CBData_T*msg)
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
         STA_GROUP_VlanCreateCallbackHandler(vid_ifindex,vlan_status);

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
         STA_GROUP_VlanMemberAddCallbackHandler(vid_ifindex,lport_index,vlan_status);

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
            STA_GROUP_VlanMemberDeleteCallbackHandler(vid_ifindex,lport_index,vlan_status);

            }
          }
     break;
     default:
        break;
   }

}
#endif

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - STA_GROUP_VlanFinishAddFirstTrunkMemberCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the callback event happening when adding the first trunk
 *           member to a VLAN is finished.
 *
 * INPUT   : trunk_ifindex  -- ifindex of the trunk port
 *           member_ifindex -- ifindex of the trunk member port
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
static void STA_GROUP_VlanFinishAddFirstTrunkMemberCallbackHandler(
    UI32_T trunk_ifindex, UI32_T member_ifindex)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

} /* End of STA_GROUP_VlanFinishAddFirstTrunkMemberCallbackHandler */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - STA_GROUP_VlanFinishAddTrunkMemberCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the callback event happening when adding a trunk member
 (           except the first) to a VLAN  is finished.
 *
 * INPUT   : trunk_ifindex  -- ifindex of the trunk port
 *           member_ifindex -- ifindex of the trunk member port
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
static void STA_GROUP_VlanFinishAddTrunkMemberCallbackHandler(
    UI32_T trunk_ifindex, UI32_T member_ifindex)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

} /* End of STA_GROUP_VlanFinishAddTrunkMemberCallbackHandler */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - STA_GROUP_VlanFinishDeleteTrunkMemberCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the callback event happening when deleting a trunk member
 *           (except the last) from a VLAN is finished.
 *
 * INPUT   : trunk_ifindex  -- ifindex of the trunk port
 *           member_ifindex -- ifindex of the trunk member port
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
static void STA_GROUP_VlanFinishDeleteTrunkMemberCallbackHandler(
    UI32_T trunk_ifindex, UI32_T member_ifindex)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */
} /* End of STA_GROUP_VlanFinishDeleteTrunkMemberCallbackHandler */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - STA_GROUP_VlanFinishDeleteLastTrunkMemberCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the callback event happening when deleting the last trunk
 *           member from a VLAN is finished.
 *
 * INPUT   : trunk_ifindex  -- ifindex of the trunk port
 *           member_ifindex -- ifindex of the trunk member port
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
static void STA_GROUP_VlanFinishDeleteLastTrunkMemberCallbackHandler(
    UI32_T trunk_ifindex, UI32_T member_ifindex)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */
} /* End of STA_GROUP_VlanFinishDeleteLastTrunkMemberCallbackHandler */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - STA_GROUP_VlanMemberTagChangedCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the callback event happening when tag type of a port member
 *           for a VLAN is changed.
 *
 * INPUT   : vid_ifindex   -- the ifindex of the VLAN
 *         : lport_ifindex -- the ifindex the port
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : Untagged member -> Tagged member / Tagged member -> Untagged member
 *-----------------------------------------------------------------------------
 */
static void STA_GROUP_VlanMemberTagChangedCallbackHandler(UI32_T vid_ifindex,
                                                          UI32_T lport_ifindex)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */
} /* End of STA_GROUP_VlanMemberTagChangedCallbackHandler */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - STA_GROUP_PvidChangeCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the callback event happening when member type of a port for
 *           a VLAN has been changed.
 *
 * INPUT   : lport_ifindex -- the ifindex the port
 *           old_pvid      -- the PVID before modification
 *           new_pvid      -- the PVID after modification
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
static void STA_GROUP_PvidChangeCallbackHandler(UI32_T lport_ifindex,
                                                UI32_T old_pvid,
                                                UI32_T new_pvid)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */
} /* End of STA_GROUP_PvidChangeCallbackHandler */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - STA_GROUP_PortStatusChangedPassivelyCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the callback event happening when port status is changed
 *           passively.
 *
 * INPUT   : ifindex     -- which logical port
 *           status      -- port status after changed
 *           changed_bmp -- the bitmap indicating who changed port status
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
static void STA_GROUP_PortStatusChangedPassivelyCallbackHandler(UI32_T ifindex,
                                                                BOOL_T status,
                                                                UI32_T changed_bmp)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */
} /* End of STA_GROUP_PortStatusChangedPassivelyCallbackHandler */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - STA_GROUP_StaPacketCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the callback event happening when a network interface
 *           received a BPDU packet.
 *
 * INPUT   : mref_handle_p -- memory reference of receive packet
 *           dst_mac       -- destination address
 *           src_mac       -- source address
 *           tag_info      -- raw tagged info of the packet
 *           type          -- packet type
 *           pkt_length    -- pdu length
 *           src_unit      -- source unit
 *           src_port      -- source port
 *           packet_class  -- class to identify which kind of packet
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
static void STA_GROUP_StaPacketCallbackHandler(L_MM_Mref_Handle_T *mref_handle_p,
                                               UI8_T              *dst_mac,
                                               UI8_T              *src_mac,
                                               UI16_T             tag_info,
                                               UI16_T             type,
                                               UI32_T             pkt_length,
                                               UI32_T             src_unit,
                                               UI32_T             src_port,
                                               UI32_T             packet_class)
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

#if (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_RSTP) || (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_MSTP)
    XSTP_MGR_BpduRcvd_Callback(mref_handle_p,
                               dst_mac,
                               src_mac,
                               tag_info,
                               type,
                               pkt_length,
                               src_unit,
                               src_port);
#else
    L_MM_Mref_Release(&mref_handle_p);
#endif
}

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
static void STA_GROUP_HandleHotInertion(SYSFUN_Msg_T* msgbuf_p)
{
    SYS_TYPE_HandleHotSwapArg_T *msg_p;

    if (msgbuf_p == NULL)
        return;

    msg_p = (SYS_TYPE_HandleHotSwapArg_T*)msgbuf_p->msg_buf;

    BRIDGE_INIT_HandleHotInsertion(msg_p->starting_port_ifindex, msg_p->number_of_port, msg_p->use_default);
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
static void STA_GROUP_HandleHotRemoval(SYSFUN_Msg_T* msgbuf_p)
{
    SYS_TYPE_HandleHotSwapArg_T *msg_p;

    if (msgbuf_p == NULL)
        return;

    msg_p = (SYS_TYPE_HandleHotSwapArg_T*)msgbuf_p->msg_buf;

    BRIDGE_INIT_HandleHotRemoval(msg_p->starting_port_ifindex, msg_p->number_of_port);
}
#endif /* #if (SYS_CPNT_UNIT_HOT_SWAP == TRUE) */

#if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE)
static void STA_GROUP_HandleCmgrIpcMsg(SYSFUN_Msg_T *ipc_msg_p)
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
                STA_GROUP_VlanDestroyCallbackHandler(
                    cmgr_msg_p->data.arg_ui32_ui32_bool_bool.arg_ui32_1,
                    cmgr_msg_p->data.arg_ui32_ui32_bool_bool.arg_ui32_2);
            }
            STA_GROUP_VlanCreateCallbackHandler(
                cmgr_msg_p->data.arg_ui32_ui32_bool_bool.arg_ui32_1,
                cmgr_msg_p->data.arg_ui32_ui32_bool_bool.arg_ui32_2);
        }
        else
        {
            STA_GROUP_VlanDestroyCallbackHandler(
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
                STA_GROUP_VlanMemberDeleteCallbackHandler(
                    cmgr_msg_p->data.arg_ui32_ui32_ui32_bool_bool.arg_ui32_1,
                    cmgr_msg_p->data.arg_ui32_ui32_ui32_bool_bool.arg_ui32_2,
                    cmgr_msg_p->data.arg_ui32_ui32_ui32_bool_bool.arg_ui32_3);
            }
            STA_GROUP_VlanMemberAddCallbackHandler(
                cmgr_msg_p->data.arg_ui32_ui32_ui32_bool_bool.arg_ui32_1,
                cmgr_msg_p->data.arg_ui32_ui32_ui32_bool_bool.arg_ui32_2,
                cmgr_msg_p->data.arg_ui32_ui32_ui32_bool_bool.arg_ui32_3);
        }
        else
        {
            STA_GROUP_VlanMemberDeleteCallbackHandler(
                cmgr_msg_p->data.arg_ui32_ui32_ui32_bool_bool.arg_ui32_1,
                cmgr_msg_p->data.arg_ui32_ui32_ui32_bool_bool.arg_ui32_2,
                cmgr_msg_p->data.arg_ui32_ui32_ui32_bool_bool.arg_ui32_3);
        }
        break;

    case CMGR_IPC_PVID_CHANGE:
        STA_GROUP_PvidChangeCallbackHandler(
            cmgr_msg_p->data.arg_ui32_ui32_ui32.arg_ui32_1,
            cmgr_msg_p->data.arg_ui32_ui32_ui32.arg_ui32_2,
            cmgr_msg_p->data.arg_ui32_ui32_ui32.arg_ui32_3);
        break;

    case CMGR_IPC_VLAN_MEMBER_TAG_CHANGE:
        STA_GROUP_VlanMemberTagChangedCallbackHandler(
            cmgr_msg_p->data.arg_ui32_ui32.arg_ui32_1,
            cmgr_msg_p->data.arg_ui32_ui32.arg_ui32_2);
        break;
    } /* end switch */

    return;
}
#endif /* #if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE) */
