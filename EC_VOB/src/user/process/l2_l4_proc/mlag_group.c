/* =============================================================================
 * MODULE NAME : MLAG_GROUP.C
 * PURPOSE     : Provide definitions for MLAG CSC group functions.
 * NOTE        : None.
 * HISTORY     :
 *     2014/04/18 -- Timon Chang, Create.
 *
 * Copyright(C)      Accton Corporation, 2014
 * =============================================================================
 */

#include "sys_cpnt.h"
#if (SYS_CPNT_MLAG == TRUE)

/* -----------------------------------------------------------------------------
 * INCLUDE FILE DECLARATIONS
 * -----------------------------------------------------------------------------
 */

#include "sys_bld.h"
#include "sys_module.h"
#include "sys_type.h"
#include "sys_adpt.h"
#include "l_mm.h"
#include "l_threadgrp.h"
#include "sysfun.h"
#include "backdoor_mgr.h"
#include "sys_callback_mgr.h"
#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
#include "sw_watchdog_mgr.h"
#endif
#include "l2_l4_proc_comm.h"
#include "mlag_mgr.h"
#include "mlag_group.h"

#if (SYS_CPNT_L2MUX == TRUE)
#include "l2mux_mgr.h"
#endif
/* -----------------------------------------------------------------------------
 * NAMING CONSTANT DECLARATIONS
 * -----------------------------------------------------------------------------
 */

/* The size of this buffer should pick the maximum size required for PMGR IPC
 * request and response of all CSCs handled in this CSC group.
 * (this size does not include IPC message header)
 */
#define PMGR_MSGBUF_MAX_REQUIRED_SIZE sizeof(MLAG_GROUP_MgrMsg_T)

/* -----------------------------------------------------------------------------
 * MACRO FUNCTION DECLARATIONS
 * -----------------------------------------------------------------------------
 */

/* -----------------------------------------------------------------------------
 * DATA TYPE DECLARATIONS
 * -----------------------------------------------------------------------------
 */

typedef union
{
    SYS_CALLBACK_MGR_LPort_CBData_T                     lport_cbdata;
    SYS_CALLBACK_MGR_PortEffectiveOperStatus_CBData_T   port_eff_status_cbadta;
    SYS_CALLBACK_MGR_MacNotify_CBData_T                 mac_notify_cbdata;
    SYS_CALLBACK_MGR_LanReceivePacket_CBData_T          lan_rx_packet_cbdata;
} MLAG_GROUP_CBData_T;

typedef union MLAG_GROUP_MgrMsg_U
{
    MLAG_MGR_IpcMsg_T   mlag_mgr_ipcmsg;
    BACKDOOR_MGR_Msg_T  backdoor_mgr_ipcmsg;
    UI8_T   buffer[SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(MLAG_GROUP_CBData_T))];
} MLAG_GROUP_MgrMsg_T;

/* -----------------------------------------------------------------------------
 * LOCAL SUBPROGRAM DECLARATIONS
 * -----------------------------------------------------------------------------
 */

static void MLAG_GROUP_Mgr_Thread_Function_Entry(void* arg);
static void MLAG_GROUP_SetTransitionMode(void);
static void MLAG_GROUP_EnterTransitionMode(void);
static void MLAG_GROUP_EnterMasterMode(void);
static void MLAG_GROUP_EnterSlaveMode(void);
static void MLAG_GROUP_ProvisionComplete(void);
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
static void MLAG_GROUP_HandleHotInsertion(SYSFUN_Msg_T *msgbuf_p);
static void MLAG_GROUP_HandleHotRemoval(SYSFUN_Msg_T *msgbuf_p);
#endif
static void MLAG_GROUP_HandleSysCallbackIPCMsg(SYSFUN_Msg_T *msgbuf_p);
static void MLAG_GROUP_PortOperUpCallbackHandler(UI32_T ifindex);
static void MLAG_GROUP_PortNotOperUpCallbackHandler(UI32_T ifindex);
static void MLAG_GROUP_PortEffectiveOperStatusChangedCallbackHandler(
    UI32_T ifindex, UI32_T pre_status, UI32_T current_status, UI32_T level);
static void MLAG_GROUP_MlagMacUpdateCallbackHandler(UI32_T ifindex, UI32_T vid,
    UI8_T *mac_p, BOOL_T added);
static void MLAG_GROUP_ReceiveMlagPacketCallbackHandler(
    L_MM_Mref_Handle_T *mref_handle_p,
    UI8_T              dst_mac[SYS_ADPT_MAC_ADDR_LEN],
    UI8_T              src_mac[SYS_ADPT_MAC_ADDR_LEN],
    UI16_T             tag_info,
    UI16_T             type,
    UI32_T             pkt_length,
    UI32_T             src_unit,
    UI32_T             src_port,
    UI32_T             packet_class);

/* -----------------------------------------------------------------------------
 * STATIC VARIABLE DEFINITIONS
 * -----------------------------------------------------------------------------
 */

/* the buffer for retrieving ipc request for stagroup mgr thread
 */
static UI8_T mgrtd_ipc_buf[SYSFUN_SIZE_OF_MSG(PMGR_MSGBUF_MAX_REQUIRED_SIZE)];

/* -----------------------------------------------------------------------------
 * EXPORTED SUBPROGRAM BODIES
 * -----------------------------------------------------------------------------
 */

/* FUNCTION NAME - MLAG_GROUP_InitiateProcessResources
 * PURPOSE : Initiate process resources for the CSC group.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 */
void MLAG_GROUP_InitiateProcessResources(void)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    MLAG_MGR_InitiateProcessResources();
    return;
} /* End of MLAG_GROUP_InitiateProcessResources */

/* FUNCTION NAME - MLAG_GROUP_Create_InterCSC_Relation
 * PURPOSE : Create inter-CSC relations for the CSC group.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 */
void MLAG_GROUP_Create_InterCSC_Relation(void)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    MLAG_MGR_CreateInterCscRelation();
    return;
} /* End of MLAG_GROUP_Create_InterCSC_Relation */

/* FUNCTION NAME - MLAG_GROUP_Create_All_Threads
 * PURPOSE : Spawn all threads for the CSC group.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 */
void MLAG_GROUP_Create_All_Threads(void)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    UI32_T thread_id;

    /* BODY
     */

    if (SYSFUN_SpawnThread(SYS_BLD_MLAG_GROUP_MGR_THREAD_PRIORITY,
                           SYS_BLD_MLAG_GROUP_MGR_SCHED_POLICY,
                           SYS_BLD_MLAG_GROUP_MGR_THREAD_NAME,
                           SYS_BLD_TASK_COMMON_STACK_SIZE,
                           SYSFUN_TASK_FP,
                           MLAG_GROUP_Mgr_Thread_Function_Entry,
                           NULL,
                           &thread_id) != SYSFUN_OK)
    {
        printf("\r\n%s: failed to spawn MLAG GROUP MGR thread.\r\n", __func__);
    }

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
    SW_WATCHDOG_MGR_RegisterMonitorThread(SW_WATCHDOG_MLAG_GROUP, thread_id,
        SYS_ADPT_MLAG_GROUP_SW_WATCHDOG_TIMER);
#endif

    return;
} /* End of MLAG_GROUP_Create_All_Threads */

/* -----------------------------------------------------------------------------
 * LOCAL SUBPROGRAM BODIES
 * -----------------------------------------------------------------------------
 */

/* FUNCTION NAME - MLAG_GROUP_Mgr_Thread_Function_Entry
 * PURPOSE : Provide entry point for the CSC group MGR thread.
 * INPUT   : arg -- pointer to argument list; NULL if no argument
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 */
static void MLAG_GROUP_Mgr_Thread_Function_Entry(void* arg)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    L_THREADGRP_Handle_T    tg_handle;
    UI32_T                  member_id;
    SYSFUN_MsgQ_T           ipc_msgq_handle;
    UI32_T                  local_events, received_events;
    SYSFUN_Msg_T            *msgbuf_p = (SYSFUN_Msg_T*)mgrtd_ipc_buf;
    BOOL_T                  need_resp;
    UI32_T                  all_events =
                                SYSFUN_SYSTEM_EVENT_SET_TRANSITION_MODE |
                                SYSFUN_SYSTEM_EVENT_IPCMSG              |
                                MLAG_TYPE_EVENT_TIMER                   |
#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
                                SYSFUN_SYSTEM_EVENT_SW_WATCHDOG         |
#endif
                                SYSFUN_SYSTEM_EVENT_IPCFAIL;
    void                    *timer_id;

    /* BODY
     */

    /* join thread group
     */
    tg_handle = L2_L4_PROC_COMM_GetMlagGroupTGHandle();
    if (L_THREADGRP_Join(tg_handle, SYS_BLD_MLAG_GROUP_MGR_THREAD_PRIORITY,
            &member_id) == FALSE)
    {
        printf("\r\n%s: failed to join thread group.\r\n", __func__);
        return;
    }

    /* create MGR IPC message queue handle
     */
    if (SYSFUN_CreateMsgQ(SYS_BLD_MLAG_GROUP_IPCMSGQ_KEY,
            SYSFUN_MSGQ_BIDIRECTIONAL, &ipc_msgq_handle) != SYSFUN_OK)
    {
        printf("\r\n%s: failed to create message queue.\r\n", __func__);
        return;
    }

    timer_id = SYSFUN_PeriodicTimer_Create();
    if (SYSFUN_PeriodicTimer_Start(timer_id, SYS_BLD_TICKS_PER_SECOND,
            MLAG_TYPE_EVENT_TIMER) == FALSE)
    {
        printf("\r\n %s: failed to start MLAG periodic timer \r\n", __func__);
        return;
    }

    /* main loop:
     *    while(1)
     *    {
     *        Wait event ...
     *    }
     */
    local_events = SYSFUN_SYSTEM_EVENT_IPCMSG;
    received_events = 0;
    while (1)
    {
        SYSFUN_ReceiveEvent(all_events,
                            SYSFUN_EVENT_WAIT_ANY,
                            (local_events == 0) ? SYSFUN_TIMEOUT_WAIT_FOREVER
                                                : SYSFUN_TIMEOUT_NOWAIT,
                            &received_events);

        local_events |= received_events;

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
        if (local_events & SYSFUN_SYSTEM_EVENT_SW_WATCHDOG)
        {
            SW_WATCHDOG_MGR_ResetTimer(SW_WATCHDOG_MLAG_GROUP);
            local_events ^= SYSFUN_SYSTEM_EVENT_SW_WATCHDOG;
        }
#endif

        if (local_events & SYSFUN_SYSTEM_EVENT_SET_TRANSITION_MODE)
        {
            MLAG_GROUP_SetTransitionMode();
            local_events ^= SYSFUN_SYSTEM_EVENT_SET_TRANSITION_MODE;

            /* need not to do IPCFAIL recovery in transition mode */
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
                /* request thread group execution permission */
                if (L_THREADGRP_Execution_Request(tg_handle, member_id) != TRUE)
                {
                    printf("\r\n%s: failed to request exec permission of thread"
                        " group.\r\n", __func__);
                }

                /* handle request message based on cmd */
                switch (msgbuf_p->cmd)
                {
                    /* system cmd
                     */

                    case SYS_TYPE_CMD_ENTER_TRANSITION_MODE:
                        MLAG_GROUP_EnterTransitionMode();
                        /* SYS_TYPE_CMD_ENTER_TRANSITION_MODE
                         * need a response which contains nothing
                         */
                        msgbuf_p->msg_size = 0;
                        need_resp = TRUE;

                        /* need not to do IPCFAIL recovery in transition mode */
                        if (local_events & SYSFUN_SYSTEM_EVENT_IPCFAIL)
                        {
                            /* clear fail info in IPCFAIL
                             */
                            local_events ^= SYSFUN_SYSTEM_EVENT_IPCFAIL;
                        }
                        break;

                    case SYS_TYPE_CMD_ENTER_MASTER_MODE:
                        MLAG_GROUP_EnterMasterMode();
                        /* SYS_TYPE_CMD_ENTER_MASTER_MODE
                         * need a response which contains nothing
                         */
                        msgbuf_p->msg_size = 0;
                        need_resp = TRUE;
                        break;

                    case SYS_TYPE_CMD_ENTER_SLAVE_MODE:
                        MLAG_GROUP_EnterSlaveMode();
                        /* SYS_TYPE_CMD_ENTER_SLAVE_MODE
                         * need a response which contains nothing
                         */
                        msgbuf_p->msg_size = 0;
                        need_resp = TRUE;

                        /* need not to do IPCFAIL recovery in slave mode */
                        if (local_events & SYSFUN_SYSTEM_EVENT_IPCFAIL)
                        {
                            /* clear fail info in IPCFAIL
                             */
                            local_events ^= SYSFUN_SYSTEM_EVENT_IPCFAIL;
                        }
                        break;

                    case SYS_TYPE_CMD_PROVISION_COMPLETE:
                    case SYS_TYPE_CMD_MODULE_PROVISION_COMPLETE:
                        MLAG_GROUP_ProvisionComplete();
                        msgbuf_p->msg_size = 0;
                        need_resp = TRUE;
                        break;

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
                    case SYS_TYPE_CMD_HANDLE_HOT_INSERTION:
                        MLAG_GROUP_HandleHotInsertion(msgbuf_p);
                        msgbuf_p->msg_size = 0;
                        need_resp = TRUE;
                        break;

                    case SYS_TYPE_CMD_HANDLE_HOT_REMOVAL:
                        MLAG_GROUP_HandleHotRemoval(msgbuf_p);
                        msgbuf_p->msg_size = 0;
                        need_resp = TRUE;
                         break;
#endif

                    case SYS_TYPE_CMD_RELOAD_SYSTEM:
                        /* process system reload message sent by stkctrl task
                         */
                        msgbuf_p->msg_size = 0;
                        need_resp = TRUE;
                        break;

                    /* module cmd
                     */

                    case SYS_MODULE_SYS_CALLBACK:
                        MLAG_GROUP_HandleSysCallbackIPCMsg(msgbuf_p);
                        /* SYS_CALLBACK ipc message is uni-directional only
                         */
                        need_resp = FALSE;
                        break;

                    case SYS_MODULE_BACKDOOR:
                        need_resp = BACKDOOR_MGR_HandleIPCMsg(msgbuf_p);
                        break;

                    case SYS_MODULE_MLAG:
                        need_resp = MLAG_MGR_HandleIPCReqMsg(msgbuf_p);
                        break;

                    default:
                        /* Unknown command. There is no way to identify whether
                         * this ipc message need a response or not. If we
                         * response to a asynchronous msg, then all following
                         * synchronous msg will get wrong responses and that
                         * might not be easy to debug. If we do not response to
                         * a synchronous msg, the requester will be blocked
                         * forever, which should be easy to debug.
                         */
                        printf("\r\n%s: Invalid IPC cmd.\r\n", __func__);
                        need_resp = FALSE;
                } /* end switch (msgbuf_p->cmd) */

                /* release thread group execution permission */
                if (L_THREADGRP_Execution_Release(tg_handle, member_id) != TRUE)
                {
                    printf("\r\n%s: failed to release exec permission of thread"
                        " group.\r\n", __func__);
                }

                if ((need_resp == TRUE) &&
                    (SYSFUN_SendResponseMsg(ipc_msgq_handle, msgbuf_p)
                        != SYSFUN_OK))
                {
                    printf("\r\n%s: failed to send response message.\r\n",
                        __func__);
                }
            } /* end if (SYSFUN_ReceiveMsg...) */
            else
            {
                local_events ^= SYSFUN_SYSTEM_EVENT_IPCMSG;
            }
        } /* end if (local_events & SYSFUN_SYSTEM_EVENT_IPCMSG) */

        /* handle MLAG timer event
         */
        if (local_events & MLAG_TYPE_EVENT_TIMER)
        {
            /* request thread group execution permission
             */
            if (L_THREADGRP_Execution_Request(tg_handle, member_id) != TRUE)
            {
                printf("\r\n%s: L_THREADGRP_Execution_Request fail.\r\n",
                    __func__);
            }

            MLAG_MGR_ProcessTimerEvent();

            /* release thread group execution permission
             */
            if (L_THREADGRP_Execution_Release(tg_handle, member_id) != TRUE)
            {
                printf("\r\n%s: L_THREADGRP_Execution_Release fail.\r\n",
                    __func__);
            }

            local_events ^= MLAG_TYPE_EVENT_TIMER;
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
    } /* end while (1) */
} /* End of MLAG_GROUP_Mgr_Thread_Function_Entry */

/* FUNCTION NAME - MLAG_GROUP_SetTransitionMode
 * PURPOSE : Invoke all SetTransitionMode functions in the CSC group.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 */
static void MLAG_GROUP_SetTransitionMode(void)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

   MLAG_MGR_SetTransitionMode();
   return;
} /* End of MLAG_GROUP_SetTransitionMode */

/* FUNCTION NAME - MLAG_GROUP_EnterTransitionMode
 * PURPOSE : Invoke all EnterTransitionMode functions in the CSC group.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */
static void MLAG_GROUP_EnterTransitionMode(void)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    MLAG_MGR_EnterTransitionMode();
    return;
} /* End of MLAG_GROUP_EnterTransitionMode */

/* FUNCTION NAME - MLAG_GROUP_EnterMasterMode
 * PURPOSE : Invoke all EnterMasterMode functions in the CSC group.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */
static void MLAG_GROUP_EnterMasterMode(void)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    MLAG_MGR_EnterMasterMode();
    return;
} /* End of MLAG_GROUP_EnterMasterMode */

/* FUNCTION NAME - MLAG_GROUP_EnterSlaveMode
 * PURPOSE : Invoke all EnterSlaveMode functions in the CSC group.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */
static void MLAG_GROUP_EnterSlaveMode(void)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    MLAG_MGR_EnterSlaveMode();
    return;
} /* End of MLAG_GROUP_EnterSlaveMode */

/* FUNCTION NAME - MLAG_GROUP_ProvisionComplete
 * PURPOSE : Invoke all ProvisionComplete functions in the CSC group.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */
static void MLAG_GROUP_ProvisionComplete(void)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    MLAG_MGR_ProvisionComplete();
    return;
} /* End of MLAG_GROUP_ProvisionComplete */

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
/* FUNCTION NAME - MLAG_GROUP_HandleHotInsertion
 * PURPOSE : Invoke all HandleHotInsertion functions in the CSC group.
 * INPUT   : msgbuf_p -- pointer to message containing related arguments
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 */
static void MLAG_GROUP_HandleHotInsertion(SYSFUN_Msg_T *msgbuf_p)
{
    SYS_TYPE_HandleHotSwapArg_T *msg_p;

    if (msgbuf_p == NULL)
    {
        return;
    }

    msg_p = (SYS_TYPE_HandleHotSwapArg_T*)msgbuf_p->msg_buf;

    MLAG_MGR_HandleHotInsertion(msg_p->starting_port_ifindex,
        msg_p->number_of_port, msg_p->use_default);

    return;
} /* End of MLAG_GROUP_HandleHotInsertion */

/* FUNCTION NAME - MLAG_GROUP_HandleHotRemoval
 * PURPOSE : Invoke all HandleHotRemoval functions in the CSC group.
 * INPUT   : msgbuf_p -- pointer to message containing related arguments
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 */
static void MLAG_GROUP_HandleHotRemoval(SYSFUN_Msg_T *msgbuf_p)
{
    SYS_TYPE_HandleHotSwapArg_T *msg_p;

    if (msgbuf_p == NULL)
    {
        return;
    }

    msg_p = (SYS_TYPE_HandleHotSwapArg_T*)msgbuf_p->msg_buf;

    MLAG_MGR_HandleHotRemoval(msg_p->starting_port_ifindex,
        msg_p->number_of_port);

    return;
} /* End of MLAG_GROUP_HandleHotRemoval */
#endif /* #if (SYS_CPNT_UNIT_HOT_SWAP == TRUE) */

/* FUNCTION NAME - MLAG_GROUP_HandleSysCallbackIPCMsg
 * PURPOSE : Handle system callback messages to DCB group.
 * INPUT   : msgbuf_p -- pointer to message containing related arguments
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */
static void MLAG_GROUP_HandleSysCallbackIPCMsg(SYSFUN_Msg_T *msgbuf_p)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    SYS_CALLBACK_MGR_Msg_T *sys_cbmsg_p;

    /* BODY
     */

    sys_cbmsg_p = (SYS_CALLBACK_MGR_Msg_T*)msgbuf_p->msg_buf;

    switch (sys_cbmsg_p->callback_event_id)
    {
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_PORT_OPER_UP:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                MLAG_GROUP_PortOperUpCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_PORT_NOT_OPER_UP:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                MLAG_GROUP_PortNotOperUpCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_PORT_EFFECTIVE_OPER_STATUS_CHANGED:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                MLAG_GROUP_PortEffectiveOperStatusChangedCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_MLAG_MAC_UPDATE:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                MLAG_GROUP_MlagMacUpdateCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_MLAG_PACKET:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                MLAG_GROUP_ReceiveMlagPacketCallbackHandler);
            break;

        default:
            SYSFUN_Debug_Printf("\r\n%s: invalid callback event (%lu)\r\n",
                __func__, sys_cbmsg_p->callback_event_id);
    }

    return;
} /* End of MLAG_GROUP_HandleSysCallbackIPCMsg */

/* FUNCTION NAME - MLAG_GROUP_PortOperUpCallbackHandler
 * PURPOSE : Handle callback when port oper status is up.
 * INPUT   : ifindex -- which logical port
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */
static void MLAG_GROUP_PortOperUpCallbackHandler(UI32_T ifindex)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    MLAG_MGR_PortOperUp_CallBack(ifindex);
    return;
} /* End of MLAG_GROUP_PortOperUpCallbackHandler */

/* FUNCTION NAME - MLAG_GROUP_PortNotOperUpCallbackHandler
 * PURPOSE : Handle callback when port oper status is not up.
 * INPUT   : ifindex -- interface on which change happens
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */
static void MLAG_GROUP_PortNotOperUpCallbackHandler(UI32_T ifindex)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    MLAG_MGR_PortNotOperUp_CallBack(ifindex);
    return;
} /* End of MLAG_GROUP_PortNotOperUpCallbackHandler */

/* FUNCTION NAME - MLAG_GROUP_PortEffectiveOperStatusChangedCallbackHandler
 * PURPOSE : Handle callback when port oper status is not up.
 * INPUT   : ifindex        -- which logical port
 *           pre_status     -- status before change
 *           current_status -- status after change
 *           level          -- SWCTRL_OperDormantLevel_T
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */
static void MLAG_GROUP_PortEffectiveOperStatusChangedCallbackHandler(
    UI32_T ifindex, UI32_T pre_status, UI32_T current_status, UI32_T level)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    MLAG_MGR_PortEffectiveOperStatusChanged_CallBack(ifindex, pre_status,
        current_status, level);
    return;
} /* End of MLAG_GROUP_PortEffectiveOperStatusChangedCallbackHandler */

/* FUNCTION NAME - MLAG_GROUP_MlagMacUpdateCallbackHandler
 * PURPOSE : Handle callback when port oper status is not up.
 * INPUT   : ifindex -- port on which MAC address is updated
 *           vid     -- VLAN ID
 *           mac_p   -- MAC address
 *           added   -- MAC address is added or removed
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */
static void MLAG_GROUP_MlagMacUpdateCallbackHandler(UI32_T ifindex, UI32_T vid,
    UI8_T *mac_p, BOOL_T added)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    MLAG_MGR_MacUpdate_CallBack(ifindex, vid, mac_p, added);
    return;
} /* End of MLAG_GROUP_MlagMacUpdateCallbackHandler */

/* FUNCTION NAME - MLAG_GROUP_ReceiveMlagPacketCallbackHandler
 * PURPOSE : Handle callback when MLAG packet is received.
 * INPUT   : mref_handle_p  --- pointer to memory reference handle
 *           dst_mac        --- destination mac address
 *           src_mac        --- source mac address
 *           tag_info       --- packet tag info
 *           type           --- packet type
 *           pkt_length     --- packet length
 *           src_unit       --- source unit
 *           src_port       --- source port
 *           packet_class   --- packet classified by LAN
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */
static void MLAG_GROUP_ReceiveMlagPacketCallbackHandler(
    L_MM_Mref_Handle_T *mref_handle_p,
    UI8_T              dst_mac[SYS_ADPT_MAC_ADDR_LEN],
    UI8_T              src_mac[SYS_ADPT_MAC_ADDR_LEN],
    UI16_T             tag_info,
    UI16_T             type,
    UI32_T             pkt_length,
    UI32_T             src_unit,
    UI32_T             src_port,
    UI32_T             packet_class)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    if (mref_handle_p == NULL)
    {
        return;
    }

#if (SYS_CPNT_L2MUX == TRUE)
    if (L2MUX_MGR_PreprocessPkt_CallbackFunc(mref_handle_p, dst_mac, src_mac,
            tag_info, type, pkt_length, src_unit, src_port, packet_class))
    {
        /* mref_handle_p had been handled and released */
        return;
    }
#endif

    MLAG_MGR_ProcessReceivedPacket(mref_handle_p, src_mac, tag_info, src_unit,
        src_port);
    L_MM_Mref_Release(&mref_handle_p);
    return;
} /* End of MLAG_GROUP_MlagMacUpdateCallbackHandler */

#endif /* #if (SYS_CPNT_MLAG == TRUE) */
