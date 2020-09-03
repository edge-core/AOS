/* MODULE NAME:  amtrl3_group.c
 * PURPOSE:
 *     This file is implemented for amtrl3 group.
 *
 * NOTES:
 *
 * HISTORY
 *    1/18/2008 - djd, Created
 *
 * Copyright(C)      Accton Corporation, 2008
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "sysfun.h"
#include "sys_adpt.h"
#include "sys_cpnt.h"
#include "sys_bld.h"
#include "sys_module.h"
#include "sysrsc_mgr.h"
#include "sys_callback_mgr.h"
#include "l_threadgrp.h"
#include "backdoor_mgr.h"
#include "l2_l4_proc_comm.h"
#include "amtrl3_group.h"

#include "amtrl3_init.h"
#include "amtrl3_mgr.h"
#include "amtrl3_task.h"
#include "amtrl3_type.h"
#include "amtr_type.h"
#if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE)
#include "cmgr.h"
#endif

/* NAMING CONSTANT DECLARATIONS
 */
/* union all data type used for AMTRL3 group MGR IPC message to get the maximum
 * required ipc message buffer
 */

#define AMTRL3_GROUP_PMGR_MSGBUF_MAX_REQUIRED_SIZE sizeof(AMTRL3_GROUP_MgrMsg_T)

/* Union the SYS CALLBACK Message size used in AMTRL3 Group for
 * calculating the maximum buffer size for receiving message
 * from Queue
 */
typedef struct AMTRL3_GROUP_Syscallback_Msg_S
{
    SYS_CALLBACK_MGR_Msg_T sys_callback_msg_header;
    union
    {
        SYS_CALLBACK_MGR_TrunkMember_CBData_T   trunk_member_data;
        struct AMTRDRV_ADDR_CBData_S {
                UI32_T num_of_entries;
#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == TRUE)
                AMTR_TYPE_AddrEntry_T               addr_buf[SYS_ADPT_AMTR_NBR_OF_ADDR_TO_SYNC_IN_ONE_SCAN];
#else
                AMTR_TYPE_AddrEntry_T               addr_buf[AMTRDRV_TYPE_MAX_ENTRIES_IN_ONE_PACKET];
#endif
            } mac_address_data;
        struct AMTR_PortMove_CBData_S {
            UI32_T num_of_entries;
#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == TRUE)
            AMTR_TYPE_PortMoveEntry_T port_move_buf[SYS_ADPT_AMTR_NBR_OF_ADDR_TO_SYNC_IN_ONE_SCAN];
#else
            AMTR_TYPE_PortMoveEntry_T port_move_buf[AMTRDRV_TYPE_MAX_ENTRIES_IN_ONE_PACKET];
#endif
            } port_move_data;
        SYS_CALLBACK_MGR_MACTableDeleteByPort_CBData_T  mac_delete_by_port_data;
        SYS_CALLBACK_MGR_MACTableDeleteByVIDnPort_CBData_T mac_delete_by_vid_port_data;
        SYS_CALLBACK_MGR_VlanDestroy_CBData_T   vlan_destroy_data;
        SYS_CALLBACK_MGR_IfOperStatusChanged_CBData_T   if_oper_status_data;
    } sys_callback_msg_data;

}AMTRL3_GROUP_Syscallback_Msg_T;

/* union all data type used for MGR IPC message to get the maximum required
 * ipc message buffer
 */

typedef union AMTRL3_GROUP_MgrMsg_U
{
    AMTRL3_MGR_IPCMsg_T             amtrl3_mgr_ipcmsg;

    AMTRL3_GROUP_Syscallback_Msg_T  sys_callback_ipcmsg;
    BACKDOOR_MGR_Msg_T              backdoor_mgr_ipcmsg;
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    SYS_TYPE_HandleHotSwapArg_T     HandleHotInsertionArg_ipcmsg;
#endif
#if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE)
    CMGR_IpcMsg_T                   cmgr_ipcmsg;
#endif
} AMTRL3_GROUP_MgrMsg_T;

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void AMTRL3_GROUP_Mgr_Thread_Function_Entry(void* arg);
static void AMTRL3_GROUP_SetTransitionMode(void);
static void AMTRL3_GROUP_EnterTransitionMode(void);
static void AMTRL3_GROUP_EnterMasterMode(void);
static void AMTRL3_GROUP_EnterSlaveMode(void);
static void AMTRL3_GROUP_HandleSysCallbackIPCMsg(SYSFUN_Msg_T *msgbuf_p);
static void AMTRL3_GROUP_HandleHotInertion(SYSFUN_Msg_T* msgbuf_p);
static void AMTRL3_GROUP_HandleHotRemoval(SYSFUN_Msg_T* msgbuf_p);
#if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE)
static void AMTRL3_GROUP_HandleCmgrIpcMsg(SYSFUN_Msg_T *ipc_msg_p);
#endif

/* STATIC VARIABLE DECLARATIONS
 */
/* the buffer for retrieving ipc request for amtrl3_group mgr thread
 */
static UI8_T amtrl3_group_mgrtd_ipc_buf[SYSFUN_SIZE_OF_MSG(AMTRL3_GROUP_PMGR_MSGBUF_MAX_REQUIRED_SIZE)];

/* EXPORTED SUBPROGRAM BODIES
 */


/*-----------------------------------------------------------------------------
 * FUNCTION NAME - AMTRL3_GROUP_InitiateProcessResources
 *-----------------------------------------------------------------------------
 * PURPOSE : Initiate process resource for AMTRL3 group.
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
void AMTRL3_GROUP_InitiateProcessResources(void)
{
    AMTRL3_INIT_Initiate_System_Resources();
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - AMTRL34_GROUP__Create_InterCSC_Relation
 *-----------------------------------------------------------------------------
 * PURPOSE : Create inter-CSC relationships for AMTRL3 group.
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
void AMTRL3_GROUP_Create_InterCSC_Relation(void)
{
    AMTRL3_MGR_Create_InterCSC_Relation();
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : AMTRL3_GROUP_Create_All_Threads
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will spawn all threads in AMTRL3 group.
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
void AMTRL3_GROUP_Create_All_Threads(void)
{
    UI32_T thread_id;

    AMTRL3_INIT_Create_Tasks();

    if(SYSFUN_SpawnThread(SYS_BLD_AMTRL3_GROUP_MGR_THREAD_PRIORITY,
                          SYS_BLD_AMTRL3_GROUP_MGR_SCHED_POLICY,
                          SYS_BLD_AMTRL3_GROUP_MGR_THREAD_NAME,
                          SYS_BLD_TASK_COMMON_STACK_SIZE,
                          SYSFUN_TASK_NO_FP,
                          AMTRL3_GROUP_Mgr_Thread_Function_Entry,
                          NULL, &thread_id)!=SYSFUN_OK)
    {
        printf("%s:Spawn AMTRL3 MGR thread fail.\n", __FUNCTION__);
    }

}

/* LOCAL SUBPROGRAM BODIES
 */
/*------------------------------------------------------------------------------
 * ROUTINE NAME : AMTRL3_GROUP_Mgr_Thread_Function_Entry
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This is the entry function for the MGR thread of AMTRL3 Group.
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
static void AMTRL3_GROUP_Mgr_Thread_Function_Entry(void* arg)
{
    SYSFUN_MsgQ_T        ipc_msgq_handle;
    L_THREADGRP_Handle_T tg_handle = L2_L4_PROC_COMM_GetAmtrl3GroupTGHandle();
    UI32_T               member_id,received_events,local_events = SYSFUN_SYSTEM_EVENT_IPCMSG;
    SYSFUN_Msg_T         *msgbuf_p = (SYSFUN_Msg_T*)amtrl3_group_mgrtd_ipc_buf;
    BOOL_T               need_resp;

    /* join the thread group of AMTRL3 Group
     */
    if(L_THREADGRP_Join(tg_handle, SYS_BLD_AMTRL3_GROUP_MGR_THREAD_PRIORITY, &member_id)==FALSE)
    {
        printf("%s: L_THREADGRP_Join fail.\n", __FUNCTION__);
        return;
    }

    /* create mgr ipc message queue handle
     */
    if(SYSFUN_CreateMsgQ(SYS_BLD_AMTRL3_GROUP_IPCMSGQ_KEY, SYSFUN_MSGQ_BIDIRECTIONAL, &ipc_msgq_handle) != SYSFUN_OK)
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
                             SYSFUN_SYSTEM_EVENT_SET_TRANSITION_MODE,
                             SYSFUN_EVENT_WAIT_ANY,
                             (local_events==0)?SYSFUN_TIMEOUT_WAIT_FOREVER:SYSFUN_TIMEOUT_NOWAIT,
                             &received_events);
        local_events |= received_events;

        if(local_events & SYSFUN_SYSTEM_EVENT_SET_TRANSITION_MODE)
        {
            AMTRL3_GROUP_SetTransitionMode();
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
                AMTRL3_GROUP_PMGR_MSGBUF_MAX_REQUIRED_SIZE, msgbuf_p) == SYSFUN_OK)
            {
                /* request thread group execution permission
                 */
                if(L_THREADGRP_Execution_Request(tg_handle, member_id) != TRUE)
                    printf("%s: L_THREADGRP_Execution_Request fail.\n", __FUNCTION__);

                /* handle request message based on cmd
                 */
                switch(msgbuf_p->cmd)
                {
                    /* module id cmd*/
                    case SYS_MODULE_AMTRL3:
                        need_resp = AMTRL3_MGR_HandleIPCReqMsg(msgbuf_p);
                        break;

                    case SYS_MODULE_SYS_CALLBACK:
                        /* SYS_CALLBACK ipc message can only be uni-direction
                         * just set need_resp as FALSE */
                        need_resp = FALSE;
                        AMTRL3_GROUP_HandleSysCallbackIPCMsg(msgbuf_p);
                        break;

                    case SYS_MODULE_BACKDOOR:
                        need_resp = BACKDOOR_MGR_HandleIPCMsg(msgbuf_p);
                        break;

#if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE)
                    case SYS_MODULE_CMGR:
                        AMTRL3_GROUP_HandleCmgrIpcMsg(msgbuf_p);
                        msgbuf_p->msg_size = 0;
                        need_resp = TRUE;
                        break;
#endif

                    /* global cmd
                     */
                    case SYS_TYPE_CMD_ENTER_MASTER_MODE:
                        AMTRL3_GROUP_EnterMasterMode();

                        /* SYS_TYPE_CMD_ENTER_MASTER_MODE
                         * need a response which contains nothing
                         */
                        msgbuf_p->msg_size = 0;
                        need_resp = TRUE;

                        /* IPCFAIL might happen when a upper layer amtrl3 group has
                         * entered transition mode but lower layer amtrl3 groups still
                         * in master mode so async callback msg will keep sending
                         * from lower layer amtrl3 group. In this case, the IPCFAIL
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
                        AMTRL3_GROUP_EnterSlaveMode();

                        /* SYS_TYPE_CMD_ENTER_SLAVE_MODE
                         * need a response which contains nothing
                         */
                        msgbuf_p->msg_size = 0;
                        need_resp = TRUE;

                        /* IPCFAIL might happen when a upper layer  group has
                         * entered transition mode but lower layer AMTRL3 group still
                         * in master mode so async callback msg will keep sending
                         * from lower layer AMTRL3 group. In this case, the IPCFAIL
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
                        AMTRL3_GROUP_EnterTransitionMode();

                        /* SYS_TYPE_CMD_ENTER_TRANSITION_MODE
                         * need a response which contains nothing
                         */
                        msgbuf_p->msg_size = 0;
                        need_resp = TRUE;
                        break;

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)

                    case  SYS_TYPE_CMD_HANDLE_HOT_INSERTION:
                        AMTRL3_GROUP_HandleHotInertion(msgbuf_p);

                        msgbuf_p->msg_size=0;
                        need_resp=TRUE;
                        break;

                    case SYS_TYPE_CMD_HANDLE_HOT_REMOVAL:
                        AMTRL3_GROUP_HandleHotRemoval(msgbuf_p);

                        msgbuf_p->msg_size=0;
                        need_resp=TRUE;
                        break;
#endif
                    case  SYS_TYPE_CMD_RELOAD_SYSTEM:

                        msgbuf_p->msg_size=0;
                        need_resp=TRUE;
                        break;

                    case SYS_TYPE_CMD_PROVISION_COMPLETE:
                        /* SYS_TYPE_CMD_PROVISION_COMPLETE
                         * need a response which contains nothing
                         */
                        msgbuf_p->msg_size = 0;
                        need_resp = TRUE;
                        break;
                   case SYS_TYPE_CMD_MODULE_PROVISION_COMPLETE:
                        /*  need a response which contains nothing     */
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
                        need_resp = FALSE;
                        break;
                }

                /* release thread group execution permission
                 */
                if(L_THREADGRP_Execution_Release(tg_handle, member_id) != TRUE)
                    printf("%s: L_THREADGRP_Execution_Release fail.\n", __FUNCTION__);

                if((need_resp == TRUE) && (SYSFUN_SendResponseMsg(ipc_msgq_handle, msgbuf_p) != SYSFUN_OK))
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
    } /* end of while(1) */
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : AMTRL3_GROUP_SetTransitionMode
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will invoke all set transition mode function in AMTRL3 Group.
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
static void AMTRL3_GROUP_SetTransitionMode(void)
{
    AMTRL3_INIT_SetTransitionMode();
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : AMTRL3_GROUP_EnterTransitionMode
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will invoke all enter transition mode function in AMTRL3 Group.
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
static void AMTRL3_GROUP_EnterTransitionMode(void)
{
    AMTRL3_INIT_EnterTransitionMode();
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : AMTRL3_GROUP_EnterMasterMode
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will invoke all set master mode function in AMTRL3 Group.
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
static void AMTRL3_GROUP_EnterMasterMode(void)
{
    AMTRL3_INIT_EnterMasterMode();
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : AMTRL3_GROUP_EnterSlaveMode
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will invoke all enter slave mode function in AMTRL3 Group.
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
static void AMTRL3_GROUP_EnterSlaveMode(void)
{
    AMTRL3_INIT_EnterSlaveMode();
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : AMTRL3_GROUP_HandleSysCallbackIPCMsg
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will handle all callbacks from IPC messages in AMTRL3 Group.
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
static void AMTRL3_GROUP_HandleSysCallbackIPCMsg(SYSFUN_Msg_T *msgbuf_p)
{
    SYS_CALLBACK_MGR_Msg_T *sys_cbmsg_p = (SYS_CALLBACK_MGR_Msg_T*)msgbuf_p->msg_buf;

    switch(sys_cbmsg_p->callback_event_id)
    {
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_TRUNK_MEMBER_ADD_1ST:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &AMTRL3_MGR_ForwardingUPortAddToTrunk_CallBack);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_TRUNK_MEMBER_ADD:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &AMTRL3_MGR_ForwardingUPortAddToTrunk_CallBack);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_TRUNK_MEMBER_DELETE:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &AMTRL3_MGR_ForwardingTrunkMemberDelete_CallBack);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_TRUNK_MEMBER_DELETE_LST:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &AMTRL3_MGR_ForwardingTrunkMemberDelete_CallBack);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_TRUNK_MEMBER_INACTIVE:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &AMTRL3_MGR_ForwardingTrunkMemberToNonForwarding_CallBack);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_MAC_AGING_OUT:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &AMTRL3_MGR_MacAgingOut_CallBack);
            break;
            
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_MAC_ADDR_UPDATE:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &AMTRL3_MGR_MacAddrUpdateCallbackHandler);
            break;
            
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_PORT_MOVE:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &AMTRL3_MGR_PortMove_CallBack);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_MAC_TABLE_DELETE_BY_PORT:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &AMTRL3_MGR_MACTableDeleteByPort_CallBack);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_MAC_TABLE_DELETE_BY_VID_AND_PORT:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &AMTRL3_MGR_MACTableDeleteByVIDnPort_CallBack);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_MAC_TABLE_DELETE_BY_LIFE_TIME:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &AMTRL3_TASK_MacDeleteByLifeTimeCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_L3VLAN_DESTROY:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                AMTRL3_MGR_VlanDestroy_CallBack);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_L3IF_OPER_STATUS_CHANGED:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                AMTRL3_MGR_IfOperStatusChanged_CallBack);
            break;
#if (SYS_CPNT_IP_TUNNEL == TRUE)
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_TUNNEL_NET_ROUTE_HIT_BIT_CHANGE:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                AMTRL3_MGR_TunnelNetRouteHitBitChange_CallBack);
            break;
#endif
        default:
            SYSFUN_Debug_Printf("%s: received callback_event that is not handled(%d)\n",
                                   __FUNCTION__, sys_cbmsg_p->callback_event_id);
            break;
    }
}


#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
/*------------------------------------------------------------------------------
 * ROUTINE NAME : AMTRL3_GROUP_HandleHotInertion
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
static void AMTRL3_GROUP_HandleHotInertion(SYSFUN_Msg_T* msgbuf_p)
{
    SYS_TYPE_HandleHotSwapArg_T *msg_p;

    if (msgbuf_p == NULL)
        return;

    msg_p = (SYS_TYPE_HandleHotSwapArg_T*)msgbuf_p->msg_buf;
    AMTRL3_INIT_HandleHotInsertion(msg_p->starting_port_ifindex, msg_p->number_of_port, msg_p->use_default);
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : AMTRL3_GROUP_HandleHotRemoval
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
static void AMTRL3_GROUP_HandleHotRemoval(SYSFUN_Msg_T* msgbuf_p)
{
    SYS_TYPE_HandleHotSwapArg_T *msg_p;

    if (msgbuf_p == NULL)
        return;

    msg_p = (SYS_TYPE_HandleHotSwapArg_T*)msgbuf_p->msg_buf;
    AMTRL3_INIT_HandleHotRemoval(msg_p->starting_port_ifindex, msg_p->number_of_port);
}
#endif

#if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE)
static void AMTRL3_GROUP_HandleCmgrIpcMsg(SYSFUN_Msg_T *ipc_msg_p)
{
    CMGR_IpcMsg_T *cmgr_msg_p;

    if (ipc_msg_p == NULL)
    {
        return;
    }

    cmgr_msg_p = (CMGR_IpcMsg_T*)ipc_msg_p->msg_buf;
    switch (cmgr_msg_p->type.cmd)
    {
    case CMGR_IPC_L3_VLAN_DESTROY:
        AMTRL3_MGR_VlanDestroy_CallBack(
            cmgr_msg_p->data.arg_ui32_ui32.arg_ui32_1,
            cmgr_msg_p->data.arg_ui32_ui32.arg_ui32_2);
        break;

    case CMGR_IPC_L3_IF_OPER_STATUS_CHANGE:
        AMTRL3_MGR_IfOperStatusChanged_CallBack(
            cmgr_msg_p->data.arg_ui32_ui32.arg_ui32_1,
            cmgr_msg_p->data.arg_ui32_ui32.arg_ui32_2);
        break;
    } /* end switch */

    return;
}
#endif /* #if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE) */
