/*-----------------------------------------------------------------------------
 * MODULE NAME: LACP_GROUP.C
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *    None.
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
#include "l2_l4_proc_comm.h"
#include "lacp_group.h"
#include "lacp_init.h"
#include "lacp_mgr.h"
#include "lacp_type.h"
#include "sys_callback_mgr.h"
#if (SYS_CPNT_L2MUX == TRUE)
#include "l2mux_mgr.h"
#endif

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
#include "sw_watchdog_mgr.h"
#endif

#if (SYS_CPNT_SYNCE == TRUE)
#include "sync_e_mgr.h"
#include "sync_e_type.h"
#endif

/* NAMING CONSTANT DECLARATIONS
 */

/* the size of the ipc msg buf for the MGR thread of LACP group to receive and
 * response PMGR ipc. The size of this buffer should pick the maximum of size
 * required for PMGR ipc request and response in all CSCs handled in this
 * process. (this size does not include IPC msg header)
 */
#define PMGR_MSGBUF_MAX_REQUIRED_SIZE sizeof(LACP_GROUP_MgrMsg_T)


/* MACRO FUNCTION DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */

/* union all data type used for LACP group MGR IPC message to get the maximum
 * required ipc message buffer
 */
typedef union LACP_GROUP_MgrMsg_U
{
    LACP_MGR_IpcMsg_T   lacp_mgr_ipcmsg;
#if (SYS_CPNT_SYNCE == TRUE)
    SYNC_E_MGR_IpcMsg_T sync_e_mgr_ipcmsg;
#endif
    BACKDOOR_MGR_Msg_T  backdoor_mgr_ipcmsg;
} LACP_GROUP_MgrMsg_T;


/* LOCAL SUBPROGRAM DECLARATIONS
 */

static void LACP_GROUP_Mgr_Thread_Function_Entry(void* arg);
static void LACP_GROUP_SetTransitionMode(void);
static void LACP_GROUP_EnterTransitionMode(void);
static void LACP_GROUP_EnterMasterMode(void);
static void LACP_GROUP_EnterSlaveMode(void);
static void LACP_GROUP_ProvisionComplete(void);
static void LACP_GROUP_HandleSysCallbackIPCMsg(SYSFUN_Msg_T *msgbuf_p);

static void LACP_GROUP_UportSpeedDuplexCallbackHandler(UI32_T unit,
                                                       UI32_T port,
                                                       UI32_T speed_duplex);
static void LACP_GROUP_UportLinkUpCallbackHandler(UI32_T unit, UI32_T port);
static void LACP_GROUP_UportLinkDownCallbackHandler(UI32_T unit, UI32_T port);
static void LACP_GROUP_UportAdminEnableCallbackHandler(UI32_T unit, UI32_T port);
static void LACP_GROUP_UportAdminDisableCallbackHandler(UI32_T unit, UI32_T port);
#if (SYS_CPNT_STATIC_TRUNK_CONFIG_ALLOWED_ON_LACP_PORT == TRUE)
static void LACP_GROUP_AddStaticTrunkMemberCallbackHandler(UI32_T trunk_ifindex,
                                                           UI32_T tm_ifindex);
static void LACP_GROUP_DelStaticTrunkMemberCallbackHandler(UI32_T trunk_ifindex,
                                                           UI32_T tm_ifindex);
#endif
static void LACP_GROUP_LacpPacketCallbackHandler(L_MM_Mref_Handle_T *mref_handle_p,
                                                 UI8_T   dst_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                 UI8_T   src_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                 UI16_T  tag_info,
                                                 UI16_T  type,
                                                 UI32_T  pkt_length,
                                                 UI32_T  src_unit,
                                                 UI32_T  src_port,
                                                 UI32_T  packet_class);

#if (SYS_CPNT_SYNCE == TRUE)
static void LACP_GROUP_ESMCPacketCallbackHandler(L_MM_Mref_Handle_T *mref_handle_p,
                                                  UI8_T *dst_mac,
                                                  UI8_T *src_mac,
                                                  UI16_T tag_info,
                                                  UI16_T type,
                                                  UI32_T pkt_length,
                                                  UI32_T  src_unit,
                                                  UI32_T  src_port,
                                                  UI32_T  packet_class);
#endif

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
/*------------------------------------------------------------------------------
 * ROUTINE NAME : LACP_GROUP_HandleHotInertion
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
static void LACP_GROUP_HandleHotInertion(SYSFUN_Msg_T* msgbuf_p);

static void LACP_GROUP_HandleHotRemoval(SYSFUN_Msg_T* msgbuf_p);
#endif


/* STATIC VARIABLE DEFINITIONS
 */

/* the buffer for retrieving ipc request for stagroup mgr thread
 */
static UI8_T mgrtd_ipc_buf[SYSFUN_SIZE_OF_MSG(PMGR_MSGBUF_MAX_REQUIRED_SIZE)];


/* EXPORTED SUBPROGRAM BODIES
 */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - LACP_GROUP_InitiateProcessResources
 *-----------------------------------------------------------------------------
 * PURPOSE : Initiate process resource for LACP group.
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
void LACP_GROUP_InitiateProcessResources(void)
{
    LACP_INIT_InitiateSystemResources();
#if (SYS_CPNT_SYNCE == TRUE)
    SYNC_E_MGR_InitialSystemResources();
#endif
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - LACP_GROUP_Create_InterCSC_Relation
 *-----------------------------------------------------------------------------
 * PURPOSE : Create inter-CSC relationships for LACP group.
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
void LACP_GROUP_Create_InterCSC_Relation(void)
{
    LACP_INIT_Create_InterCSC_Relation();
#if (SYS_CPNT_SYNCE == TRUE)
    SYNC_E_MGR_Create_InterCSC_Relation();
#endif
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - LACP_GROUP_Create_All_Threads
 *-----------------------------------------------------------------------------
 * PURPOSE : This function will spawn all threads in LACP group.
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
void LACP_GROUP_Create_All_Threads(void)
{
    UI32_T thread_id;

    if (SYSFUN_SpawnThread(SYS_BLD_LACP_GROUP_MGR_THREAD_PRIORITY,
                           SYS_BLD_LACP_GROUP_MGR_SCHED_POLICY,
                           SYS_BLD_LACP_GROUP_MGR_THREAD_NAME,
                           SYS_BLD_TASK_COMMON_STACK_SIZE,
                           SYSFUN_TASK_FP,
                           LACP_GROUP_Mgr_Thread_Function_Entry,
                           NULL,
                           &thread_id) != SYSFUN_OK)
    {
        printf("\n%s: Spawn STAGroup MGR thread fail.\n", __FUNCTION__);
    }

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
    SW_WATCHDOG_MGR_RegisterMonitorThread(SW_WATCHDOG_LACP_GROUP, thread_id, SYS_ADPT_LACP_GROUP_SW_WATCHDOG_TIMER);
#endif

    return;
}


/* LOCAL SUBPROGRAM BODIES
 */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - LACP_GROUP_Mgr_Thread_Function_Entry
 *-----------------------------------------------------------------------------
 * PURPOSE : This is the entry function for the MGR thread of LACP group.
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
static void LACP_GROUP_Mgr_Thread_Function_Entry(void* arg)
{
    SYSFUN_MsgQ_T           ipc_msgq_handle;
    L_THREADGRP_Handle_T    tg_handle = L2_L4_PROC_COMM_GetLacpGroupTGHandle();
    UI32_T                  member_id, received_events, local_events = SYSFUN_SYSTEM_EVENT_IPCMSG;
    SYSFUN_Msg_T            *msgbuf_p = (SYSFUN_Msg_T*)mgrtd_ipc_buf;
    BOOL_T                  need_resp;
    void                    *lacp_timer_id;
    UI32_T                  all_events =
                                SYSFUN_SYSTEM_EVENT_SET_TRANSITION_MODE |
                                SYSFUN_SYSTEM_EVENT_IPCMSG |
                                LACP_EVENT_TIMER |
#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
                                SYSFUN_SYSTEM_EVENT_SW_WATCHDOG |
#endif
                                SYSFUN_SYSTEM_EVENT_IPCFAIL;

    /* join the thread group of LACP Group
     */
    if (L_THREADGRP_Join(tg_handle, SYS_BLD_LACP_GROUP_MGR_THREAD_PRIORITY,
            &member_id) == FALSE)
    {
        printf("\n%s: L_THREADGRP_Join fail.\n", __FUNCTION__);
        return;
    }

    /* create mgr ipc message queue handle
     */
    if (SYSFUN_CreateMsgQ(SYS_BLD_LACP_GROUP_IPCMSGQ_KEY,
            SYSFUN_MSGQ_BIDIRECTIONAL, &ipc_msgq_handle) != SYSFUN_OK)
    {
        printf("\n%s: SYSFUN_CreateMsgQ fail.\n", __FUNCTION__);
        return;
    }

    lacp_timer_id = SYSFUN_PeriodicTimer_Create();
    SYSFUN_PeriodicTimer_Start(lacp_timer_id, LACPDUD_TIMER_TICKS1SEC, LACP_EVENT_TIMER);

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
            LACP_GROUP_SetTransitionMode();
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
                        LACP_GROUP_EnterTransitionMode();

                        /* SYS_TYPE_CMD_ENTER_TRANSITION_MODE
                         * need a response which contains nothing
                         */
                        msgbuf_p->msg_size = 0;
                        need_resp = TRUE;
                        break;

                    case SYS_TYPE_CMD_ENTER_MASTER_MODE:
                        LACP_GROUP_EnterMasterMode();

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
                        LACP_GROUP_EnterSlaveMode();

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
                        LACP_GROUP_ProvisionComplete();
                        msgbuf_p->msg_size = 0;
                        need_resp = TRUE;
                        break;
                    /* module id cmd
                     */

                    case SYS_MODULE_LACP:
                        need_resp = LACP_MGR_HandleIPCReqMsg(msgbuf_p);
                        break;
#if (SYS_CPNT_SYNCE == TRUE)
                    case SYS_MODULE_SYNCE:
                        need_resp = SYNC_E_MGR_HandleIPCReqMsg(msgbuf_p);
                        break;
#endif

                    case SYS_MODULE_BACKDOOR:
                        need_resp = BACKDOOR_MGR_HandleIPCMsg(msgbuf_p);
                        break;

                    case SYS_MODULE_SYS_CALLBACK:
                        LACP_GROUP_HandleSysCallbackIPCMsg(msgbuf_p);
                        /* SYS_CALLBACK ipc message can only be uni-direction
                         * just set need_resp as FALSE
                         */
                        need_resp = FALSE;
                        break;
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
                     case     SYS_TYPE_CMD_HANDLE_HOT_INSERTION:
                       LACP_GROUP_HandleHotInertion(msgbuf_p);
                       
                        msgbuf_p->msg_size=0;
                        need_resp=TRUE;
                        break;
                     case SYS_TYPE_CMD_HANDLE_HOT_REMOVAL:
                       LACP_GROUP_HandleHotRemoval(msgbuf_p);
                       
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

        /* handle LACP timer event
         */
        if (local_events & LACP_EVENT_TIMER)
        {
            /* request thread group execution permission
             */
            if (L_THREADGRP_Execution_Request(tg_handle, member_id) != TRUE)
            {
                printf("\n%s: L_THREADGRP_Execution_Request fail.\n",
                    __FUNCTION__);
            }

            LACP_TASK_TimerTick();
#if (SYS_CPNT_SYNCE == TRUE)
            SYNC_E_MGR_TimerEvent();
#endif

            /* release thread group execution permission
             */
            if (L_THREADGRP_Execution_Release(tg_handle, member_id) != TRUE)
            {
                printf("\n%s: L_THREADGRP_Execution_Release fail.\n",
                    __FUNCTION__);
            }

            local_events ^= LACP_EVENT_TIMER;
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
        if (local_events & SYSFUN_SYSTEM_EVENT_SW_WATCHDOG)
        {
       	    SW_WATCHDOG_MGR_ResetTimer(SW_WATCHDOG_LACP_GROUP);
            local_events ^= SYSFUN_SYSTEM_EVENT_SW_WATCHDOG;
        }
#endif
    } /* end of while (1) */
} /* End of LACP_GROUP_Mgr_Thread_Function_Entry */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - LACP_GROUP_SetTransitionMode
 *-----------------------------------------------------------------------------
 * PURPOSE : This function will invoke all set transition mode function in
 *           LACP group.
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
static void LACP_GROUP_SetTransitionMode(void)
{
    LACP_INIT_SetTransitionMode();
#if (SYS_CPNT_SYNCE == TRUE)
    SYNC_E_MGR_SetTransitionMode();
#endif
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - LACP_GROUP_EnterTransitionMode
 *-----------------------------------------------------------------------------
 * PURPOSE : This function will invoke all enter transition mode function in
 *           LACP group.
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
static void LACP_GROUP_EnterTransitionMode(void)
{
    LACP_INIT_EnterTransitionMode();
#if (SYS_CPNT_SYNCE == TRUE)
    SYNC_E_MGR_EnterTransitionMode();
#endif
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - LACP_GROUP_EnterMasterMode
 *-----------------------------------------------------------------------------
 * PURPOSE : This function will invoke all set master mode function in
 *           LACP group.
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
static void LACP_GROUP_EnterMasterMode(void)
{
    LACP_INIT_EnterMasterMode();
#if (SYS_CPNT_SYNCE == TRUE)
    SYNC_E_MGR_EnterMasterMode();
#endif
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - LACP_GROUP_EnterSlaveMode
 *-----------------------------------------------------------------------------
 * PURPOSE : This function will invoke all enter slave mode function in
 *           LACP group.
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
static void LACP_GROUP_EnterSlaveMode(void)
{
    LACP_INIT_EnterSlaveMode();
#if (SYS_CPNT_SYNCE == TRUE)
    SYNC_E_MGR_EnterSlaveMode();
#endif
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - LACP_GROUP_ProvisionComplete
 *-----------------------------------------------------------------------------
 * PURPOSE : This function will invoke all provision complete function in
 *           LACP group.
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
static void LACP_GROUP_ProvisionComplete(void)
{
#if (SYS_CPNT_SYNCE == TRUE)
    SYNC_E_MGR_ProvisionComplete();
#endif
    return;
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - LACP_GROUP_HandleSysCallbackIPCMsg
 *-----------------------------------------------------------------------------
 * PURPOSE : This function will handle all callbacks from IPC messages in
 *           LACP group.
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
static void LACP_GROUP_HandleSysCallbackIPCMsg(SYSFUN_Msg_T *msgbuf_p)
{
    SYS_CALLBACK_MGR_Msg_T *sys_cbmsg_p =
        (SYS_CALLBACK_MGR_Msg_T*)msgbuf_p->msg_buf;

    switch (sys_cbmsg_p->callback_event_id)
    {
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_UPORT_SPEED_DUPLEX:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                LACP_GROUP_UportSpeedDuplexCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_UPORT_LINK_UP:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                LACP_GROUP_UportLinkUpCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_UPORT_LINK_DOWN:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                LACP_GROUP_UportLinkDownCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_UPORT_ADMIN_ENABLE:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                LACP_GROUP_UportAdminEnableCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_UPORT_ADMIN_DISABLE:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                LACP_GROUP_UportAdminDisableCallbackHandler);
            break;

#if (SYS_CPNT_STATIC_TRUNK_CONFIG_ALLOWED_ON_LACP_PORT == TRUE)
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_ADD_STATIC_TRUNK_MEMBER:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                LACP_GROUP_AddStaticTrunkMemberCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_DEL_STATIC_TRUNK_MEMBER:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                LACP_GROUP_DelStaticTrunkMemberCallbackHandler);
            break;
#endif /* #if (SYS_CPNT_STATIC_TRUNK_CONFIG_ALLOWED_ON_LACP_PORT == TRUE) */

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_LACP_PACKET:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                LACP_GROUP_LacpPacketCallbackHandler);
            break;
#if (SYS_CPNT_SYNCE == TRUE)
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_ESMC_PACKET:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                LACP_GROUP_ESMCPacketCallbackHandler);
            break;
#endif
        default:
            SYSFUN_Debug_Printf(
                "\n%s: received callback_event that is not handled(%d)\n",
                __FUNCTION__, sys_cbmsg_p->callback_event_id);
    }
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - LACP_GROUP_UportSpeedDuplexCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the callback event happening when the speed/duplex status
 *           of a port is changed.
 *
 * INPUT   : unit         -- in which unit
 *           port         -- which logical port
 *           speed_duplex -- new status of speed/duplex
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
static void LACP_GROUP_UportSpeedDuplexCallbackHandler(UI32_T unit,
                                                       UI32_T port,
                                                       UI32_T speed_duplex)
{
    LACP_MGR_PortDuplexChange_CallBack(unit, port, speed_duplex);
#if (SYS_CPNT_SYNCE == TRUE)
    SYNC_E_MGR_PortSpeedDuplexChange_Callback(unit, port, speed_duplex);
#endif
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - LACP_GROUP_UportLinkUpCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the callback event happening when the link is up.
 *
 * INPUT   : unit -- in which unit
 *           port -- which logical port
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
static void LACP_GROUP_UportLinkUpCallbackHandler(UI32_T unit, UI32_T port)
{
    LACP_MGR_PortLinkup_CallBack(unit, port);
#if (SYS_CPNT_SYNCE == TRUE)
    SYNC_E_MGR_UportLinkUpCallBack(unit, port);
#endif
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - LACP_GROUP_UportLinkDownCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the callback event happening when the link is down.
 *
 * INPUT   : unit -- in which unit
 *           port -- which logical port
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
static void LACP_GROUP_UportLinkDownCallbackHandler(UI32_T unit, UI32_T port)
{
    LACP_MGR_PortLinkdown_CallBack(unit, port);
#if (SYS_CPNT_SYNCE == TRUE)
    SYNC_E_MGR_UportLinkDownCallBack(unit, port);
#endif
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - LACP_GROUP_UportAdminEnableCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the callback event happening when the port is enabled.
 *
 * INPUT   : unit -- in which unit
 *           port -- which logical port
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
static void LACP_GROUP_UportAdminEnableCallbackHandler(UI32_T unit, UI32_T port)
{
    LACP_MGR_PortAdminEnable_CallBack(unit, port);
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - LACP_GROUP_UportAdminDisableCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the callback event happening when the port is disabled.
 *
 * INPUT   : unit -- in which unit
 *           port -- which logical port
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
static void LACP_GROUP_UportAdminDisableCallbackHandler(UI32_T unit, UI32_T port)
{
    LACP_MGR_PortAdminDisable_CallBack(unit, port);
}

#if (SYS_CPNT_STATIC_TRUNK_CONFIG_ALLOWED_ON_LACP_PORT == TRUE)
/*-----------------------------------------------------------------------------
 * FUNCTION NAME - LACP_GROUP_AddStaticTrunkMemberCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the callback event happening when UIs add static trunk
 *           member.
 *
 * INPUT   : trunk_ifindex --- trunk member is added to which trunk
 *           tm_ifindex    --- which trunk member is added to trunk
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
static void LACP_GROUP_AddStaticTrunkMemberCallbackHandler(UI32_T trunk_ifindex,
                                                           UI32_T tm_ifindex)
{
#if (SYS_CPNT_STATIC_TRUNK_CONFIG_ALLOWED_ON_LACP_PORT == TRUE)
    LACP_MGR_AddStaticTrunkMember_CallBack(trunk_ifindex, tm_ifindex);
#endif
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - LACP_GROUP_DelStaticTrunkMemberCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the callback event happening when UIs delete static trunk
 *           member.
 *
 * INPUT   : trunk_ifindex --- trunk member is added to which trunk
 *           tm_ifindex    --- which trunk member is added to trunk
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
static void LACP_GROUP_DelStaticTrunkMemberCallbackHandler(UI32_T trunk_ifindex,
                                                           UI32_T tm_ifindex)
{
#if (SYS_CPNT_STATIC_TRUNK_CONFIG_ALLOWED_ON_LACP_PORT == TRUE)
    LACP_MGR_DelStaticTrunkMember_CallBack(trunk_ifindex, tm_ifindex);
#endif
}
#endif /* #if (SYS_CPNT_STATIC_TRUNK_CONFIG_ALLOWED_ON_LACP_PORT == TRUE) */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - LACP_GROUP_LacpPacketCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the callback event happening when a network interface
 *           received a LACPDU packet.
 *
 * INPUT   : mref_handle_p -- packet buffer and return buffer function pointer.
 *           dst_mac       -- the destination MAC address of this packet.
 *           src_mac       -- the source MAC address of this packet.
 *           tag_info      -- tag information
 *           type          -- packet type
 *           pkt_length    -- pdu length
 *           src_unit      -- user view unit number
 *           src_port      -- user view port number
 *           packet_class  -- class to identify which kind of packet (not used)
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
static void LACP_GROUP_LacpPacketCallbackHandler(L_MM_Mref_Handle_T *mref_handle_p,
                                                 UI8_T   dst_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                 UI8_T   src_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                 UI16_T  tag_info,
                                                 UI16_T  type,
                                                 UI32_T  pkt_length,
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

    LACP_MGR_LacpduRcvd_Callback(mref_handle_p,
                                 dst_mac,
                                 src_mac,
                                 tag_info,
                                 type,
                                 pkt_length,
                                 src_unit,
                                 src_port);
    return;
}

#if (SYS_CPNT_SYNCE == TRUE)
static void LACP_GROUP_ESMCPacketCallbackHandler(L_MM_Mref_Handle_T *mref_handle_p,
                                                  UI8_T *dst_mac,
                                                  UI8_T *src_mac,
                                                  UI16_T tag_info,
                                                  UI16_T type,
                                                  UI32_T pkt_length,
                                                  UI32_T  src_unit,
                                                  UI32_T  src_port,
                                                  UI32_T  packet_class)
{

    SYNC_E_MGR_ESMCPduRcvd_Callback(mref_handle_p,
                                    dst_mac,
                                    src_mac,
                                    tag_info,
                                    type,
                                    pkt_length,
                                    src_unit,
                                    src_port,
                                    packet_class);
	return;
}
#endif /*#if (SYS_CPNT_SYNCE == TRUE)*/

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
/*------------------------------------------------------------------------------
 * ROUTINE NAME : LACP_GROUP_HandleHotInertion
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
static void LACP_GROUP_HandleHotInertion(SYSFUN_Msg_T* msgbuf_p)
{
 SYS_TYPE_HandleHotSwapArg_T *msg_p;

 if (msgbuf_p == NULL)
    return;

    msg_p = (SYS_TYPE_HandleHotSwapArg_T*)msgbuf_p->msg_buf;
  
    LACP_INIT_HandleHotInsertion(msg_p->starting_port_ifindex, msg_p->number_of_port, msg_p->use_default);      

#if (SYS_CPNT_SYNCE == TRUE)
    SYNC_E_MGR_HotInsert(msg_p->starting_port_ifindex, msg_p->number_of_port, msg_p->use_default);
#endif

}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : LACP_GROUP_HandleHotRemoval
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
static void LACP_GROUP_HandleHotRemoval(SYSFUN_Msg_T* msgbuf_p)
{
 SYS_TYPE_HandleHotSwapArg_T *msg_p;

 if (msgbuf_p == NULL)
    return;

    msg_p = (SYS_TYPE_HandleHotSwapArg_T*)msgbuf_p->msg_buf;
    
    LACP_INIT_HandleHotRemoval(msg_p->starting_port_ifindex, msg_p->number_of_port);      

#if (SYS_CPNT_SYNCE == TRUE)
    SYNC_E_MGR_HotRemove(msg_p->starting_port_ifindex, msg_p->number_of_port);
#endif

}
#endif

