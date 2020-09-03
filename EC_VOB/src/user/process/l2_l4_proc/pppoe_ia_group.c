/*-----------------------------------------------------------------------------
 * MODULE NAME: PPPOE_IA_GROUP.C
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *    None.
 *
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    2009/11/26     --- Squid Ro, Create for Linux platform
 *
 * Copyright(C)      Accton Corporation, 2009
 *-----------------------------------------------------------------------------
 */


/* INCLUDE FILE DECLARATIONS
 */
#include <string.h>
#include "sys_cpnt.h"

#if (SYS_CPNT_PPPOE_IA == TRUE)

#include "sys_bld.h"
#include "sys_module.h"
#include "sys_type.h"
#include "l_threadgrp.h"
#include "sysfun.h"
#include "backdoor_mgr.h"
#include "l2_l4_proc_comm.h"
#include "pppoe_ia_group.h"
#include "pppoe_ia_init.h"
#include "pppoe_ia_mgr.h"
#include "pppoe_ia_type.h"
#include "sys_callback_mgr.h"


/* NAMING CONSTANT DECLARATIONS
 */

/* the size of the ipc msg buf for the MGR thread of PPPOE_IA group to receive and
 * response PMGR ipc. The size of this buffer should pick the maximum of size
 * required for PMGR ipc request and response in all CSCs handled in this
 * process. (this size does not include IPC msg header)
 */
#define PPPOE_IA_GROUP_PMGR_MSGBUF_MAX_REQUIRED_SIZE sizeof(PPPOE_IA_GROUP_MgrMsg_T)


/* MACRO FUNCTION DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */

/* union all data type used for PPPOE_IA group MGR IPC message to get the maximum
 * required ipc message buffer
 */
typedef union PPPOE_IA_GROUP_MgrMsg_U
{
    PPPOE_IA_MGR_IpcMsg_T   PPPOE_IA_mgr_ipcmsg;
} PPPOE_IA_GROUP_MgrMsg_T;


/* LOCAL SUBPROGRAM DECLARATIONS
 */

static void PPPOE_IA_GROUP_Mgr_Thread_Function_Entry(
    void* arg);
static void PPPOE_IA_GROUP_SetTransitionMode(void);
static void PPPOE_IA_GROUP_EnterTransitionMode(void);
static void PPPOE_IA_GROUP_EnterMasterMode(void);
static void PPPOE_IA_GROUP_EnterSlaveMode(void);
static void PPPOE_IA_GROUP_ProvisionComplete(void);
static void PPPOE_IA_GROUP_HandleSysCallbackIPCMsg(
    SYSFUN_Msg_T    *msgbuf_p);
static void PPPOE_IA_GROUP_TrunkMemberAdd1stCallbackHandler(
    UI32_T  trunk_ifindex,
    UI32_T  member_ifindex);
static void PPPOE_IA_GROUP_TrunkMemberAddCallbackHandler(
    UI32_T  trunk_ifindex,
    UI32_T  member_ifindex);
static void PPPOE_IA_GROUP_TrunkMemberDeleteCallbackHandler(
    UI32_T  trunk_ifindex,
    UI32_T  member_ifindex);
static void PPPOE_IA_GROUP_TrunkMemberDeleteLstCallbackHandler(
    UI32_T  trunk_ifindex,
    UI32_T  member_ifindex);
static void PPPOE_IA_GROUP_PppoedPacketCallbackHandler(
    L_MM_Mref_Handle_T  *mref_handle_p,
    UI8_T               dst_mac[SYS_ADPT_MAC_ADDR_LEN],
    UI8_T               src_mac[SYS_ADPT_MAC_ADDR_LEN],
    UI16_T              tag_info,
    UI16_T              type,
    UI32_T              pkt_length,
    UI32_T              src_lport);

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
static void PPPOE_IA_GROUP_HandleHotInsertion(SYSFUN_Msg_T* msgbuf_p);
static void PPPOE_IA_GROUP_HandleHotRemoval(SYSFUN_Msg_T* msgbuf_p);
#endif /* #if (SYS_CPNT_UNIT_HOT_SWAP == TRUE) */


/* STATIC VARIABLE DEFINITIONS
 */

/* the buffer for retrieving ipc request for pppoe_ia group mgr thread
 */
static UI8_T mgrtd_ipc_buf[SYSFUN_SIZE_OF_MSG(PPPOE_IA_GROUP_PMGR_MSGBUF_MAX_REQUIRED_SIZE)];


/* EXPORTED SUBPROGRAM BODIES
 */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_GROUP_InitiateProcessResources
 *-----------------------------------------------------------------------------
 * PURPOSE : Initiate process resource for PPPOE_IA group.
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
void PPPOE_IA_GROUP_InitiateProcessResources(void)
{
    PPPOE_IA_INIT_InitiateSystemResources();
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_GROUP_Create_InterCSC_Relation
 *-----------------------------------------------------------------------------
 * PURPOSE : Create inter-CSC relationships for PPPOE_IA group.
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
void PPPOE_IA_GROUP_Create_InterCSC_Relation(void)
{
    PPPOE_IA_INIT_Create_InterCSC_Relation();
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_GROUP_Create_All_Threads
 *-----------------------------------------------------------------------------
 * PURPOSE : This function will spawn all threads in PPPOE_IA group.
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
void PPPOE_IA_GROUP_Create_All_Threads(void)
{
    UI32_T thread_id;

    if (SYSFUN_SpawnThread(SYS_BLD_PPPOE_IA_GROUP_MGR_THREAD_PRIORITY,
                           SYS_BLD_PPPOE_IA_GROUP_MGR_SCHED_POLICY,
                           SYS_BLD_PPPOE_IA_GROUP_MGR_THREAD_NAME,
                           SYS_BLD_TASK_COMMON_STACK_SIZE,
                           SYSFUN_TASK_FP,
                           PPPOE_IA_GROUP_Mgr_Thread_Function_Entry,
                           NULL,
                           &thread_id) != SYSFUN_OK)
    {
        printf("\n%s: Spawn PPPOE IA Group MGR thread fail.\n", __FUNCTION__);
    }

    return;
}


/* LOCAL SUBPROGRAM BODIES
 */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_GROUP_Mgr_Thread_Function_Entry
 *-----------------------------------------------------------------------------
 * PURPOSE : This is the entry function for the MGR thread of PPPOE_IA group.
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
static void PPPOE_IA_GROUP_Mgr_Thread_Function_Entry(void* arg)
{
    SYSFUN_MsgQ_T           ipc_msgq_handle;
    L_THREADGRP_Handle_T    tg_handle = L2_L4_PROC_COMM_GetPppoeiaGroupTGHandle();
    UI32_T                  member_id, received_events, local_events = SYSFUN_SYSTEM_EVENT_IPCMSG;
    SYSFUN_Msg_T            *msgbuf_p = (SYSFUN_Msg_T*)mgrtd_ipc_buf;
    BOOL_T                  need_resp;
    UI32_T                  all_events =
                                SYSFUN_SYSTEM_EVENT_SET_TRANSITION_MODE |
                                SYSFUN_SYSTEM_EVENT_IPCMSG |
                                SYSFUN_SYSTEM_EVENT_IPCFAIL;

    /* join the thread group of PPPOE_IA Group
     */
    if (L_THREADGRP_Join(tg_handle, SYS_BLD_PPPOE_IA_GROUP_MGR_THREAD_PRIORITY,
            &member_id) == FALSE)
    {
        printf("\n%s: L_THREADGRP_Join fail.\n", __FUNCTION__);
        return;
    }

    /* create mgr ipc message queue handle
     */
    if (SYSFUN_CreateMsgQ(SYS_BLD_PPPOE_IA_GROUP_IPCMSGQ_KEY,
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
            PPPOE_IA_GROUP_SetTransitionMode();
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
                    PPPOE_IA_GROUP_PMGR_MSGBUF_MAX_REQUIRED_SIZE, msgbuf_p);

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
                    PPPOE_IA_GROUP_EnterTransitionMode();

                    /* SYS_TYPE_CMD_ENTER_TRANSITION_MODE
                     * need a response which contains nothing
                     */
                    msgbuf_p->msg_size = 0;
                    need_resp = TRUE;
                    break;

                case SYS_TYPE_CMD_ENTER_MASTER_MODE:
                    PPPOE_IA_GROUP_EnterMasterMode();

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
                    PPPOE_IA_GROUP_EnterSlaveMode();

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
                    PPPOE_IA_GROUP_ProvisionComplete();
                    msgbuf_p->msg_size = 0;
                    need_resp = TRUE;
                    break;

                /* module id cmd
                 */
                case SYS_MODULE_PPPOE_IA:
                    need_resp = PPPOE_IA_MGR_HandleIPCReqMsg(msgbuf_p);
                    break;

                case SYS_MODULE_BACKDOOR:
                    need_resp = BACKDOOR_MGR_HandleIPCMsg(msgbuf_p);
                    break;

                case SYS_MODULE_SYS_CALLBACK:
                    PPPOE_IA_GROUP_HandleSysCallbackIPCMsg(msgbuf_p);
                    /* SYS_CALLBACK ipc message can only be uni-direction
                     * just set need_resp as FALSE
                     */
                    need_resp = FALSE;
                    break;

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
                 case SYS_TYPE_CMD_HANDLE_HOT_INSERTION:
                    PPPOE_IA_GROUP_HandleHotInsertion(msgbuf_p);

                    msgbuf_p->msg_size=0;
                    need_resp=TRUE;
                    break;

                 case SYS_TYPE_CMD_HANDLE_HOT_REMOVAL:
                    PPPOE_IA_GROUP_HandleHotRemoval(msgbuf_p);

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
} /* End of PPPOE_IA_GROUP_Mgr_Thread_Function_Entry */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_GROUP_SetTransitionMode
 *-----------------------------------------------------------------------------
 * PURPOSE : This function will invoke all set transition mode function in
 *           PPPOE_IA group.
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
static void PPPOE_IA_GROUP_SetTransitionMode(void)
{
    PPPOE_IA_INIT_SetTransitionMode();
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_GROUP_EnterTransitionMode
 *-----------------------------------------------------------------------------
 * PURPOSE : This function will invoke all enter transition mode function in
 *           PPPOE_IA group.
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
static void PPPOE_IA_GROUP_EnterTransitionMode(void)
{
    PPPOE_IA_INIT_EnterTransitionMode();
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_GROUP_EnterMasterMode
 *-----------------------------------------------------------------------------
 * PURPOSE : This function will invoke all set master mode function in
 *           PPPOE_IA group.
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
static void PPPOE_IA_GROUP_EnterMasterMode(void)
{
    PPPOE_IA_INIT_EnterMasterMode();
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_GROUP_EnterSlaveMode
 *-----------------------------------------------------------------------------
 * PURPOSE : This function will invoke all enter slave mode function in
 *           PPPOE_IA group.
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
static void PPPOE_IA_GROUP_EnterSlaveMode(void)
{
    PPPOE_IA_INIT_EnterSlaveMode();
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_GROUP_ProvisionComplete
 *-----------------------------------------------------------------------------
 * PURPOSE : This function will invoke all provision complete function in
 *           PPPOE_IA group.
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
static void PPPOE_IA_GROUP_ProvisionComplete(void)
{
    return;
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_GROUP_HandleSysCallbackIPCMsg
 *-----------------------------------------------------------------------------
 * PURPOSE : This function will handle all callbacks from IPC messages in
 *           PPPOE_IA group.
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
static void PPPOE_IA_GROUP_HandleSysCallbackIPCMsg(SYSFUN_Msg_T *msgbuf_p)
{
    SYS_CALLBACK_MGR_Msg_T *sys_cbmsg_p =
        (SYS_CALLBACK_MGR_Msg_T*)msgbuf_p->msg_buf;

    switch (sys_cbmsg_p->callback_event_id)
    {
    case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_PPPOED_PACKET:
        SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
            PPPOE_IA_GROUP_PppoedPacketCallbackHandler);
        break;

    case SYS_CALLBACK_MGR_CALLBACKEVENTID_TRUNK_MEMBER_ADD_1ST:
        SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
            PPPOE_IA_GROUP_TrunkMemberAdd1stCallbackHandler);
        break;

    case SYS_CALLBACK_MGR_CALLBACKEVENTID_TRUNK_MEMBER_ADD:
        SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
            PPPOE_IA_GROUP_TrunkMemberAddCallbackHandler);
        break;

    case SYS_CALLBACK_MGR_CALLBACKEVENTID_TRUNK_MEMBER_DELETE:
        SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
            PPPOE_IA_GROUP_TrunkMemberDeleteCallbackHandler);
        break;

    case SYS_CALLBACK_MGR_CALLBACKEVENTID_TRUNK_MEMBER_DELETE_LST:
        SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
            PPPOE_IA_GROUP_TrunkMemberDeleteLstCallbackHandler);
        break;

    default:
        SYSFUN_Debug_Printf("%s: received callback_event that is not handled(%d)\n",
            __FUNCTION__, sys_cbmsg_p->callback_event_id);
        break;
    }
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_GROUP_TrunkMemberAdd1stCallbackHandler
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
static void PPPOE_IA_GROUP_TrunkMemberAdd1stCallbackHandler(
    UI32_T  trunk_ifindex,
    UI32_T  member_ifindex)
{
    PPPOE_IA_MGR_AddFstTrkMbr_CallBack(trunk_ifindex, member_ifindex);
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_GROUP_TrunkMemberAddCallbackHandler
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
static void PPPOE_IA_GROUP_TrunkMemberAddCallbackHandler(
    UI32_T  trunk_ifindex,
    UI32_T  member_ifindex)
{
    PPPOE_IA_MGR_AddTrkMbr_CallBack(trunk_ifindex, member_ifindex);
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_GROUP_TrunkMemberDeleteCallbackHandler
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
static void PPPOE_IA_GROUP_TrunkMemberDeleteCallbackHandler(
    UI32_T  trunk_ifindex,
    UI32_T  member_ifindex)
{
    PPPOE_IA_MGR_DelTrkMbr_CallBack(trunk_ifindex, member_ifindex);
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_GROUP_TrunkMemberDeleteLstCallbackHandler
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
static void PPPOE_IA_GROUP_TrunkMemberDeleteLstCallbackHandler(
    UI32_T  trunk_ifindex,
    UI32_T  member_ifindex)
{
    PPPOE_IA_MGR_DelLstTrkMbr_CallBack(trunk_ifindex, member_ifindex);
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_GROUP_PppoedPacketCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the callback event happening when a network interface
 *           received a PPPOE Discovery packet.
 *
 * INPUT   : mref_handle_p -- packet buffer and return buffer function pointer.
 *           dst_mac       -- the destination MAC address of this packet.
 *           src_mac       -- the source MAC address of this packet.
 *           tag_info      -- tag information
 *           type          -- packet type
 *           pkt_length    -- pdu length
 *           src_lport     -- source lport
 *           packet_class  -- class to identify which kind of packet (not used)
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
static void PPPOE_IA_GROUP_PppoedPacketCallbackHandler(
    L_MM_Mref_Handle_T  *mref_handle_p,
    UI8_T               dst_mac[SYS_ADPT_MAC_ADDR_LEN],
    UI8_T               src_mac[SYS_ADPT_MAC_ADDR_LEN],
    UI16_T              tag_info,
    UI16_T              type,
    UI32_T              pkt_length,
    UI32_T              src_lport)
{
    PPPOE_IA_TYPE_PktHdr_T  pppoe_ia_phdr;
    PPPOE_IA_TYPE_Msg_T     pppoe_ia_msg;

    if (NULL != mref_handle_p)
    {
        memcpy (pppoe_ia_phdr.dst_mac, dst_mac, SYS_ADPT_MAC_ADDR_LEN);
        memcpy (pppoe_ia_phdr.src_mac, src_mac, SYS_ADPT_MAC_ADDR_LEN);
        pppoe_ia_phdr.lport     = src_lport;
        pppoe_ia_phdr.tag_info  = tag_info;

        pppoe_ia_msg.mem_ref_p = mref_handle_p;
        pppoe_ia_msg.pkt_hdr_p = &pppoe_ia_phdr;

        PPPOE_IA_MGR_ProcessRcvdPDU(&pppoe_ia_msg);
        L_MM_Mref_Release(&mref_handle_p);
    }
}

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
/*------------------------------------------------------------------------------
 * ROUTINE NAME : PPPOE_IA_GROUP_HandleHotInsertion
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will invoke a new dut insertion in PPPOE_IA_GROUP.
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
static void PPPOE_IA_GROUP_HandleHotInsertion(SYSFUN_Msg_T* msgbuf_p)
{
    SYS_TYPE_HandleHotSwapArg_T *msg_p;

    if (msgbuf_p == NULL)
        return;

    msg_p = (SYS_TYPE_HandleHotSwapArg_T*)msgbuf_p->msg_buf;

    PPPOE_IA_INIT_HandleHotInsertion(msg_p->starting_port_ifindex, msg_p->number_of_port, msg_p->use_default);
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : PPPOE_IA_GROUP_HandleHotRemoval
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will invoke a new dut removal in PPPOE_IA_GROUP.
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
static void PPPOE_IA_GROUP_HandleHotRemoval(SYSFUN_Msg_T* msgbuf_p)
{
    SYS_TYPE_HandleHotSwapArg_T *msg_p;

    if (msgbuf_p == NULL)
        return;

    msg_p = (SYS_TYPE_HandleHotSwapArg_T*)msgbuf_p->msg_buf;

    PPPOE_IA_INIT_HandleHotRemoval(msg_p->starting_port_ifindex, msg_p->number_of_port);
}
#endif /* #if (SYS_CPNT_UNIT_HOT_SWAP == TRUE) */

#endif /* #if (SYS_CPNT_PPPOE_IA == TRUE) */

