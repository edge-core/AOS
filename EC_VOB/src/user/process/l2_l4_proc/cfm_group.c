/* MODULE NAME: cfm_group.c
* PURPOSE:
*    {1. What is covered in this file - function and scope}
*    {2. Related documents or hardware information}
* NOTES:
*    {Something must be known or noticed}
*    {1. How to use these functions - Give an example}
*    {2. Sequence of messages if applicable}
*    {3. Any design limitation}
*    {4. Any performance limitation}
*    {5. Is it a reusable component}
*
* HISTORY:
*    mm/dd/yy (A.D.)
*    1/21/2008    Macauley_Cheng Create
* Copyright(C)      Accton Corporation, 2008
*/

/* INCLUDE FILE DECLARATIONS
*/
#include "sys_cpnt.h"
#if (SYS_CPNT_CFM == TRUE)

#include "sys_adpt.h"
#include "sysfun.h"
#include "sys_bld.h"
#include "sys_module.h"
#include "sysrsc_mgr.h"
#include "sys_callback_mgr.h"
#include "l_threadgrp.h"
#include "l_mm.h"
#include "backdoor_mgr.h"
#include "l2_l4_proc_comm.h"
#include "cfm_group.h"
#include "cfm_init.h"
#include "cfm_mgr.h"
#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
#include "sw_watchdog_mgr.h"
#endif
#if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE)
#include "cmgr.h"
#endif

/* NAMING CONSTANT DECLARATIONS
*/
#define CFM_GROUP_TIMER_TICKS_1_SEC SYS_BLD_TICKS_PER_SECOND
#define CFM_GROUP_TIMER_TICKS_100MS (SYS_BLD_TICKS_PER_SECOND/10)
#define CFM_GROUP_EVENT_1_SEC       BIT_1
#define CFM_GROUP_EVENT_100_MS      BIT_2 /*100ms*/
#define CFM_GROUP_PMGR_MSGBUF_MAX_REQUIRED_SIZE  sizeof(CFM_GROUP_MGR_MSG_T)

/* MACRO FUNCTION DECLARATIONS
*/

/* DATA TYPE DECLARATIONS
*/
typedef union {
    SYS_CALLBACK_MGR_LPort_CBData_T                   sys_callback_mgr_lport_cbdata;
    SYS_CALLBACK_MGR_TrunkMember_CBData_T             sys_callback_mgr_trunk_member_cbdata;
    SYS_CALLBACK_MGR_VlanMemberDeleteByTrunk_CBData_T sys_callback_mgr_vlan_member_delete_by_trunk_cbdata;
    SYS_CALLBACK_MGR_IfOperStatusChanged_CBData_T     sys_callback_mgr_if_oper_status_changed_cbdata;
    SYS_CALLBACK_MGR_VlanDestroy_CBData_T             sys_callback_mgr_vlan_destroy_cbdata;
    SYS_CALLBACK_MGR_VlanMemberAdd_CBData_T           sys_callback_mgr_vlan_member_add_cbdata;
    SYS_CALLBACK_MGR_L2muxReceivePacket_CBData_T      sys_callback_mgr_l2mux_receive_packet_cbdata;
} CFM_GROUP_Syscallback_CBData_T;

typedef struct
{
    UI8_T buffer[SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(CFM_GROUP_Syscallback_CBData_T))];
} CFM_GROUP_Syscallback_Msg_T;

typedef union CFM_GROUP_MGR_MSG_U
{
    CFM_MGR_IPCMsg_T              cfm_mgr_ipcmsg;
    BACKDOOR_MGR_Msg_T            backdoor_mgr_ipcmsg;
    CFM_GROUP_Syscallback_Msg_T   sys_callback_ipcmsg;
#if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE)
    CMGR_IpcMsg_T                 cmgr_ipcmsg;
#endif
} CFM_GROUP_MGR_MSG_T;

/* LOCAL SUBPROGRAM BODIES
*/
static void CFM_GROUP_Mgr_Thread_Function_Entry(void* arg);
static void CFM_GROUP_SetTransitionMode(void);
static void CFM_GROUP_EnterTransitionMode(void);
static void CFM_GROUP_EnterMasterMode(void);
static void CFM_GROUP_EnterSlaveMode(void);
static void CFM_GROUP_ProvisionComplete(void);
static void CFM_GROUP_HandleSysCallbackIPCMsg(SYSFUN_Msg_T *msgbuf_p);

static void CFM_GROUP_LPortOperUpCallbackHandler(UI32_T lport_ifindex);
static void CFM_GROUP_LPortNotOperUpCallbackHandler(UI32_T lport_ifindex);
static void CFM_GROUP_TrunkMemberAdd1stCallbackHandler(UI32_T trunk_ifindex, UI32_T lport_ifindex);
static void CFM_GROUP_TrunkMemberAddCallbackHandler(UI32_T trunk_ifindex, UI32_T lport_ifindex);
static void CFM_GROUP_TrunkMemberDeleteCallbackHandler(UI32_T trunk_ifindex, UI32_T lport_ifindex);
static void CFM_GROUP_TrunkMemberDeleteLstCallbackHandler(UI32_T trunk_ifindex, UI32_T lport_ifindex);
static void CFM_GROUP_VlanCreateCallbackHandler(UI32_T vlan_ifindex, UI32_T vlan_status);
static void CFM_GROUP_VlanDestroyCallbackHandler(UI32_T vlan_ifindex, UI32_T vlan_status);
static void CFM_GROUP_VlanMemberAddCallbackHandler(UI32_T vid_ifindex,UI32_T lport_ifindex,UI32_T vlan_status);
static void CFM_GROUP_VlanMemberDeleteCallbackHandler(UI32_T vid_ifindex,UI32_T lport_ifindex,UI32_T vlan_status);
static void CFM_GROUP_PortLeaveForwardingCallbackHandler(UI32_T xstpid, UI32_T lport);
static void CFM_GROUP_PortEnterForwardingCallbackHandler(UI32_T xstpid, UI32_T lport);


static void CFM_GROUP_ReceivePacketCallback(L_MM_Mref_Handle_T  *mref_handle_p,
                                                   UI8_T            dst_mac[6],
                                                   UI8_T            src_mac[6],
                                                   UI16_T           tag_info,
                                                   UI16_T           type,
                                                   UI32_T           pkt_length,
                                                   UI32_T           lport);

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
static void CFM_GROUP_HandleHotInsertion(SYSFUN_Msg_T* msgbuf_p);
static void CFM_GROUP_HandleHotRemoval(SYSFUN_Msg_T* msgbuf_p);
#endif /* #if (SYS_CPNT_UNIT_HOT_SWAP == TRUE) */

#if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE)
static void CFM_GROUP_HandleCmgrIpcMsg(SYSFUN_Msg_T *ipc_msg_p);
#endif

/* STATIC VARIABLE DEFINITIONS
 */
static UI8_T cfm_group_mgrtd_ipc_buf[SYSFUN_SIZE_OF_MSG(CFM_GROUP_PMGR_MSGBUF_MAX_REQUIRED_SIZE)];

/* EXPORTED SUBPROGRAM SPECIFICATIONS
*/
/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_GROUP_InitiateProcessResource
 *-------------------------------------------------------------------------
 * PURPOSE : initial the resource
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void CFM_GROUP_InitiateProcessResource(void)
{
    CFM_INIT_Initiate_System_Resources();
}
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_BACKDOOR_MainMenu
 *-------------------------------------------------------------------------
 * PURPOSE : create the inter csc relationship
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void CFM_GROUP_Create_InterCSC_Relation(void)
{
    CFM_INIT_Create_InterCSC_Relation();
}

/*-------------------------------------------------------------------------
* FUNCTION NAME - CFM_GROUP_Create_All_Threads
*-------------------------------------------------------------------------
* PURPOSE : This function will spawn all threads in CFM
* INPUT   : None
* OUTPUT  : None
* RETURN  : None
* NOTE    : None
*-------------------------------------------------------------------------
*/
void CFM_GROUP_Create_All_Threads(void)
{
    UI32_T thread_id;

    if (SYSFUN_SpawnThread(SYS_BLD_CFM_GROUP_MGR_THREAD_PRIORITY,
                           SYS_BLD_CFM_GROUP_MGR_SCHED_POLICY,
                           SYS_BLD_CFM_GROUP_MGR_THREAD_NAME,
                           SYS_BLD_TASK_COMMON_STACK_SIZE,
                           SYSFUN_TASK_FP,
                           CFM_GROUP_Mgr_Thread_Function_Entry,
                           NULL,
                           &thread_id) != SYSFUN_OK)
    {
        printf("\r\n%s: Spawn CFM Group MGR thread fail.\r\n", __FUNCTION__);
    }

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
    SW_WATCHDOG_MGR_RegisterMonitorThread(SW_WATCHDOG_CFM_GROUP, thread_id, SYS_ADPT_CFM_GROUP_SW_WATCHDOG_TIMER);
#endif

    return;
}


/* Local SUBPROGRAM SPECIFICATIONS
*/
/*-------------------------------------------------------------------------
* FUNCTION NAME - CFM_GROUP_Mgr_Thread_Function_Entry
*-------------------------------------------------------------------------
* PURPOSE : This is the entry function for the MGR thread of CFM group.
* INPUT   : None
* OUTPUT  : None
* RETURN  : None
* NOTE    : None
*-------------------------------------------------------------------------
*/
static void CFM_GROUP_Mgr_Thread_Function_Entry(void* arg)
{
    SYSFUN_MsgQ_T           ipc_msgq_handle;
    L_THREADGRP_Handle_T    tg_handle = L2_L4_PROC_COMM_GetCfmGroupTGHandle();
    UI32_T                  member_id, received_events, local_events = SYSFUN_SYSTEM_EVENT_IPCMSG;
    SYSFUN_Msg_T            *msgbuf_p = (SYSFUN_Msg_T*)cfm_group_mgrtd_ipc_buf;
    BOOL_T                  need_resp;
    UI32_T                  all_events =
                                SYSFUN_SYSTEM_EVENT_SET_TRANSITION_MODE |
                                SYSFUN_SYSTEM_EVENT_IPCMSG |
                                CFM_GROUP_EVENT_1_SEC |
                                CFM_GROUP_EVENT_100_MS |
#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
                                SYSFUN_SYSTEM_EVENT_SW_WATCHDOG |
#endif
                                SYSFUN_SYSTEM_EVENT_IPCFAIL;
    void                 *cfm_timer_id;

    /* join the thread group of CFM Group
     */
    if (L_THREADGRP_Join(tg_handle, SYS_BLD_CFM_GROUP_MGR_THREAD_PRIORITY,
            &member_id) == FALSE)
    {
        printf("\r\n%s: L_THREADGRP_Join fail.\r\n", __FUNCTION__);
        return;
    }

    /* create mgr ipc message queue handle
     */
    if (SYSFUN_CreateMsgQ(SYS_BLD_CFM_GROUP_IPCMSGQ_KEY,
            SYSFUN_MSGQ_BIDIRECTIONAL, &ipc_msgq_handle) != SYSFUN_OK)
    {
        printf("\r\n%s: SYSFUN_CreateMsgQ fail.\r\n", __FUNCTION__);
        return;
    }

    /*create timer tick*/
    cfm_timer_id = SYSFUN_PeriodicTimer_Create();

    SYSFUN_PeriodicTimer_Start(cfm_timer_id, CFM_GROUP_TIMER_TICKS_1_SEC, CFM_GROUP_EVENT_1_SEC);


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
            CFM_GROUP_SetTransitionMode();
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
                CFM_GROUP_PMGR_MSGBUF_MAX_REQUIRED_SIZE, msgbuf_p)==SYSFUN_OK)
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
                    /* module id cmd
                     */
                    case SYS_MODULE_CFM:
                        need_resp=CFM_MGR_HandleIPCReqMsg(msgbuf_p);
                        break;
                    case SYS_MODULE_BACKDOOR:
                        need_resp = BACKDOOR_MGR_HandleIPCMsg(msgbuf_p);
                        break;

                    case SYS_MODULE_SYS_CALLBACK:
                        CFM_GROUP_HandleSysCallbackIPCMsg(msgbuf_p);
                        /* SYS_CALLBACK ipc message can only be uni-direction
                         * just set need_resp as FALSE
                         */
                        need_resp = FALSE;
                        break;

#if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE)
                    case SYS_MODULE_CMGR:
                        CFM_GROUP_HandleCmgrIpcMsg(msgbuf_p);
                        msgbuf_p->msg_size = 0;
                        need_resp = TRUE;
                        break;
#endif

                    /* global cmd
                     */
                    case SYS_TYPE_CMD_ENTER_TRANSITION_MODE:
                        CFM_GROUP_EnterTransitionMode();

                        /* SYS_TYPE_CMD_ENTER_TRANSITION_MODE
                         * need a response which contains nothing
                         */
                        msgbuf_p->msg_size = 0;
                        need_resp = TRUE;
                        break;

                    case SYS_TYPE_CMD_ENTER_MASTER_MODE:
                        CFM_GROUP_EnterMasterMode();

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
                        CFM_GROUP_EnterSlaveMode();

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
                        CFM_GROUP_ProvisionComplete();
                        msgbuf_p->msg_size = 0;
                        need_resp = TRUE;
                        break;

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
                    case SYS_TYPE_CMD_HANDLE_HOT_INSERTION:
                        CFM_GROUP_HandleHotInsertion(msgbuf_p);
                        msgbuf_p->msg_size=0;
                        need_resp=TRUE;
                        break;

                    case SYS_TYPE_CMD_HANDLE_HOT_REMOVAL:
                        CFM_GROUP_HandleHotRemoval(msgbuf_p);
                        msgbuf_p->msg_size=0;
                        need_resp=TRUE;
                        break;
#endif /* #if (SYS_CPNT_UNIT_HOT_SWAP == TRUE) */

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
                        printf("\r\n%s: Invalid IPC req cmd %d.\r\n", __FUNCTION__, msgbuf_p->cmd);
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
        if (local_events & CFM_GROUP_EVENT_1_SEC)
        {
            if (L_THREADGRP_Execution_Request(tg_handle, member_id)!=TRUE)
                printf("\r\n%s: L_THREADGRP_Execution_Request fail.", __FUNCTION__);

            CFM_MGR_ProcessTimerEvent();

            if (L_THREADGRP_Execution_Release(tg_handle, member_id)!=TRUE)
               printf("\r\n%s: L_THREADGRP_Execution_Release fail.", __FUNCTION__);

            local_events ^= CFM_GROUP_EVENT_1_SEC;
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
       	   SW_WATCHDOG_MGR_ResetTimer(SW_WATCHDOG_CFM_GROUP);
           local_events ^= SYSFUN_SYSTEM_EVENT_SW_WATCHDOG;
       }
#endif
    } /* end of while (1) */
}/*End of CFM_GROUP_Mgr_Thread_Function_Entry*/


/*------------------------------------------------------------------------------
* ROUTINE NAME : CFM_GROUP_SetTransitionMode
*-----------------------------------------------------------------------------
* PURPOSE: This function will invoke all set transition mode function in CSCGroup1.
* INPUT   : None
* OUTPUT  : None
* RETURN  : None
* NOTE    : None
*-------------------------------------------------------------------------
*/
static void CFM_GROUP_SetTransitionMode(void)
{
    CFM_INIT_SetTransitionMode();
}

/*------------------------------------------------------------------------------
* ROUTINE NAME : CFM_GROUP_EnterTransitionMode
*-----------------------------------------------------------------------------
* PURPOSE: This function will invoke all enter transition mode function in CSCGroup1.
* INPUT   : None
* OUTPUT  : None
* RETURN  : None
* NOTE    : None
*-------------------------------------------------------------------------
*/
static void CFM_GROUP_EnterTransitionMode(void)
{
    CFM_INIT_EnterTransitionMode();
}

/*------------------------------------------------------------------------------
* ROUTINE NAME : CFM_GROUP_EnterMasterMode
*-----------------------------------------------------------------------------
* PURPOSE: This function will invoke all set master mode function in CSCGroup1.
* INPUT   : None
* OUTPUT  : None
* RETURN  : None
* NOTE    : None
*-------------------------------------------------------------------------
*/
static void CFM_GROUP_EnterMasterMode(void)
{
    CFM_INIT_EnterMasterMode();
}

/*------------------------------------------------------------------------------
* ROUTINE NAME : CFM_GROUP_EnterSlaveMode
*-----------------------------------------------------------------------------
* PURPOSE : This function will invoke all enter slave mode function in CSCGroup1.
* INPUT   : None
* OUTPUT  : None
* RETURN  : None
* NOTE    : None
*-------------------------------------------------------------------------
*/
static void CFM_GROUP_EnterSlaveMode(void)
{
    CFM_INIT_EnterSlaveMode();
}

/*------------------------------------------------------------------------------
* ROUTINE NAME : CFM_GROUP_ProvisionComplete
*-----------------------------------------------------------------------------
* PURPOSE: This function will invoke all provision complete function in CSCGroup1.
* INPUT   : None
* OUTPUT  : None
* RETURN  : None
* NOTE    : None
*-------------------------------------------------------------------------
*/
static void CFM_GROUP_ProvisionComplete(void)
{
    CFM_INIT_ProvisionComplete();
    return;
}

/*------------------------------------------------------------------------------
* ROUTINE NAME : CFM_GROUP_HandleSysCallbackIPCMsg
*-----------------------------------------------------------------------------
* PURPOSE:  This function will handle all callbacks from IPC messages in CSCGroup1.
* INPUT   : None
* OUTPUT  : None
* RETURN  : None
* NOTE    : None
*-------------------------------------------------------------------------
*/
static void CFM_GROUP_HandleSysCallbackIPCMsg(SYSFUN_Msg_T *msgbuf_p)
{
    SYS_CALLBACK_MGR_Msg_T *sys_cbmsg_p = (SYS_CALLBACK_MGR_Msg_T*)msgbuf_p->msg_buf;

    switch(sys_cbmsg_p->callback_event_id)
    {
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_CFM_PACKET:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &CFM_GROUP_ReceivePacketCallback);
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_PORT_OPER_UP:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &CFM_GROUP_LPortOperUpCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_PORT_NOT_OPER_UP:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &CFM_GROUP_LPortNotOperUpCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_TRUNK_MEMBER_ADD_1ST:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &CFM_GROUP_TrunkMemberAdd1stCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_TRUNK_MEMBER_ADD:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &CFM_GROUP_TrunkMemberAddCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_TRUNK_MEMBER_DELETE:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &CFM_GROUP_TrunkMemberDeleteCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_TRUNK_MEMBER_DELETE_LST:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &CFM_GROUP_TrunkMemberDeleteLstCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_CREATE:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &CFM_GROUP_VlanCreateCallbackHandler);
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_DESTROY:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &CFM_GROUP_VlanDestroyCallbackHandler);
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_MEMBER_ADD:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &CFM_GROUP_VlanMemberAddCallbackHandler);
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_MEMBER_DELETE:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &CFM_GROUP_VlanMemberDeleteCallbackHandler);
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_LPORT_LEAVE_FORWARDING:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &CFM_GROUP_PortLeaveForwardingCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_LPORT_ENTER_FORWARDING:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &CFM_GROUP_PortEnterForwardingCallbackHandler);
            break;

        default:
            SYSFUN_Debug_Printf("\r\n%s: received callback_event that is not handled(%d)",
                __FUNCTION__, sys_cbmsg_p->callback_event_id);
    }
}

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
/*------------------------------------------------------------------------------
 * ROUTINE NAME : CFM_GROUP_HandleHotInsertion
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will invoke a new dut insertion in CFM_GROUP.
 *
 * INPUT:
 *    starting_port_ifindex.
 *    number_of_port
 *    use_default
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
static void CFM_GROUP_HandleHotInsertion(SYSFUN_Msg_T* msgbuf_p)
{
    SYS_TYPE_HandleHotSwapArg_T *msg_p;

    if (msgbuf_p == NULL)
        return;

    msg_p = (SYS_TYPE_HandleHotSwapArg_T*)msgbuf_p->msg_buf;

    CFM_INIT_HandleHotInsertion(msg_p->starting_port_ifindex, msg_p->number_of_port, msg_p->use_default);
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : CFM_GROUP_HandleHotRemoval
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will invoke a new dut removal in CFM_GROUP.
 *
 * INPUT:
 *    starting_port_ifindex.
 *    number_of_port
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
static void CFM_GROUP_HandleHotRemoval(SYSFUN_Msg_T* msgbuf_p)
{
    SYS_TYPE_HandleHotSwapArg_T *msg_p;

    if (msgbuf_p == NULL)
        return;

    msg_p = (SYS_TYPE_HandleHotSwapArg_T*)msgbuf_p->msg_buf;

    CFM_INIT_HandleHotRemoval(msg_p->starting_port_ifindex, msg_p->number_of_port);
}
#endif /* #if (SYS_CPNT_UNIT_HOT_SWAP == TRUE) */


static void CFM_GROUP_LPortOperUpCallbackHandler(UI32_T lport_ifindex)
{
    CFM_MGR_ProcessInterfaceStatusChange_Callback(lport_ifindex);
}

static void CFM_GROUP_LPortNotOperUpCallbackHandler(UI32_T lport_ifindex)
{
    CFM_MGR_ProcessInterfaceStatusChange_Callback(lport_ifindex);
}

static void CFM_GROUP_TrunkMemberAdd1stCallbackHandler(UI32_T trunk_ifindex, UI32_T lport_ifindex)
{
    CFM_MGR_ProcessTrunkAdd1stMember_Callback(trunk_ifindex, lport_ifindex);
}

static void CFM_GROUP_TrunkMemberAddCallbackHandler(UI32_T trunk_ifindex, UI32_T lport_ifindex)
{
    CFM_MGR_ProcessTrunkAddMember_Callback(trunk_ifindex, lport_ifindex);
}

static void CFM_GROUP_TrunkMemberDeleteCallbackHandler(UI32_T trunk_ifindex, UI32_T lport_ifindex)
{
    CFM_MGR_ProcessTrunkMemberDelete_Callback(trunk_ifindex, lport_ifindex);
}

static void CFM_GROUP_TrunkMemberDeleteLstCallbackHandler(UI32_T trunk_ifindex, UI32_T lport_ifindex)
{
    CFM_MGR_ProcessTrunkDeleteLastMember_Callback(trunk_ifindex, lport_ifindex);
}

static void CFM_GROUP_VlanCreateCallbackHandler(UI32_T vlan_ifindex, UI32_T vlan_status)
{
    CFM_MGR_ProcessVlanCreate_Callback(vlan_ifindex, vlan_status);
}
static void CFM_GROUP_VlanDestroyCallbackHandler(UI32_T vlan_ifindex, UI32_T vlan_status)
{
    CFM_MGR_ProcessVlanDestory_Callback(vlan_ifindex,vlan_status);
}
static void CFM_GROUP_VlanMemberAddCallbackHandler(UI32_T vid_ifindex,UI32_T lport_ifindex,UI32_T vlan_status)
{
   CFM_MGR_ProcessVlanMemberAdd_Callback(vid_ifindex,lport_ifindex,vlan_status);
}
static void CFM_GROUP_VlanMemberDeleteCallbackHandler(UI32_T vid_ifindex,UI32_T lport_ifindex,UI32_T vlan_status)
{
   CFM_MGR_ProcessVlanMemberDelete_Callback(vid_ifindex,lport_ifindex,vlan_status);
}
static void CFM_GROUP_PortLeaveForwardingCallbackHandler(UI32_T xstpid, UI32_T lport)
{
    CFM_MGR_ProcessPortLeaveForwarding(xstpid, lport);
}

static void CFM_GROUP_PortEnterForwardingCallbackHandler(UI32_T xstpid, UI32_T lport)
{
    CFM_MGR_ProcessPortEnterForwarding(xstpid, lport);
}

static void CFM_GROUP_ReceivePacketCallback(L_MM_Mref_Handle_T  *mref_handle_p,
                                                   UI8_T            dst_mac[6],
                                                   UI8_T            src_mac[6],
                                                   UI16_T           tag_info,
                                                   UI16_T           type,
                                                   UI32_T           pkt_length,
                                                   UI32_T           lport)
{
    CFM_MGR_ProcessRcvdPDU(mref_handle_p, dst_mac, src_mac, tag_info, type, pkt_length, lport);
}

#if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE)
static void CFM_GROUP_HandleCmgrIpcMsg(SYSFUN_Msg_T *ipc_msg_p)
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
                CFM_GROUP_VlanDestroyCallbackHandler(
                    cmgr_msg_p->data.arg_ui32_ui32_bool_bool.arg_ui32_1,
                    cmgr_msg_p->data.arg_ui32_ui32_bool_bool.arg_ui32_2);
            }
            CFM_GROUP_VlanCreateCallbackHandler(
                cmgr_msg_p->data.arg_ui32_ui32_bool_bool.arg_ui32_1,
                cmgr_msg_p->data.arg_ui32_ui32_bool_bool.arg_ui32_2);
        }
        else
        {
            CFM_GROUP_VlanDestroyCallbackHandler(
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
                CFM_GROUP_VlanMemberDeleteCallbackHandler(
                    cmgr_msg_p->data.arg_ui32_ui32_ui32_bool_bool.arg_ui32_1,
                    cmgr_msg_p->data.arg_ui32_ui32_ui32_bool_bool.arg_ui32_2,
                    cmgr_msg_p->data.arg_ui32_ui32_ui32_bool_bool.arg_ui32_3);
            }
            CFM_GROUP_VlanMemberAddCallbackHandler(
                cmgr_msg_p->data.arg_ui32_ui32_ui32_bool_bool.arg_ui32_1,
                cmgr_msg_p->data.arg_ui32_ui32_ui32_bool_bool.arg_ui32_2,
                cmgr_msg_p->data.arg_ui32_ui32_ui32_bool_bool.arg_ui32_3);
        }
        else
        {
            CFM_GROUP_VlanMemberDeleteCallbackHandler(
                cmgr_msg_p->data.arg_ui32_ui32_ui32_bool_bool.arg_ui32_1,
                cmgr_msg_p->data.arg_ui32_ui32_ui32_bool_bool.arg_ui32_2,
                cmgr_msg_p->data.arg_ui32_ui32_ui32_bool_bool.arg_ui32_3);
        }
        break;

    case CMGR_IPC_XSTP_PORT_STATE_CHANGE:
        if (cmgr_msg_p->data.arg_ui32_ui32_bool_bool.arg_bool_1 == TRUE)
        {
            if (cmgr_msg_p->data.arg_ui32_ui32_bool_bool.arg_bool_2 == TRUE)
            {
                CFM_GROUP_PortLeaveForwardingCallbackHandler(
                    cmgr_msg_p->data.arg_ui32_ui32_bool_bool.arg_ui32_1,
                    cmgr_msg_p->data.arg_ui32_ui32_bool_bool.arg_ui32_2);
            }
            CFM_GROUP_PortEnterForwardingCallbackHandler(
                cmgr_msg_p->data.arg_ui32_ui32_bool_bool.arg_ui32_1,
                cmgr_msg_p->data.arg_ui32_ui32_bool_bool.arg_ui32_2);
        }
        else
        {
            CFM_GROUP_PortLeaveForwardingCallbackHandler(
                cmgr_msg_p->data.arg_ui32_ui32_bool_bool.arg_ui32_1,
                cmgr_msg_p->data.arg_ui32_ui32_bool_bool.arg_ui32_2);
        }
        break;
    } /* end switch */

    return;
}
#endif /* #if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE) */

#endif /*#if (SYS_CPNT_CFM == TRUE)*/
