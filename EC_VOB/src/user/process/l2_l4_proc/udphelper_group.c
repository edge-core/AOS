/* MODULE NAME:  udphelper_group.c
 * PURPOSE:
 *     This file is implemented for udp helper group.
 *
 * NOTES:
 *
 * HISTORY
 *    3/30/2009 - LinLi, Created
 *
 * Copyright(C)      Accton Corporation, 2009
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "sysfun.h"
#include "sys_cpnt.h"
#if (SYS_CPNT_UDP_HELPER == TRUE)

#include "sys_bld.h"
#include "sys_module.h"
#include "sysrsc_mgr.h"
#include "sys_callback_mgr.h"
#include "l_threadgrp.h"
#include "backdoor_mgr.h"
#include "l2_l4_proc_comm.h"
#include "udphelper_group.h"

#include "udphelper_init.h"
#include "udphelper_mgr.h"
#include "udphelper_type.h"
/* NAMING CONSTANT DECLARATIONS
 */
/* union all data type used for UDPHELPER group MGR IPC message to get the maximum
 * required ipc message buffer
 */

#define UDPHELPER_GROUP_PMGR_MSGBUF_MAX_REQUIRED_SIZE sizeof(UDPHELPER_GROUP_MgrMsg_T)

/* Union the SYS CALLBACK Message size used in UDPHELPER Group for
 * calculating the maximum buffer size for receiving message
 * from Queue
 */


/* union all data type used for MGR IPC message to get the maximum required
 * ipc message buffer
 */

typedef union UDPHELPER_GROUP_MgrMsg_U
{
    UDPHELPER_MGR_IPCMsg_T                      udphelper_mgr_ipcmsg;

    SYS_CALLBACK_MGR_ImlReceivePacket_CBData_T  sys_callback_ipcmsg;
    BACKDOOR_MGR_Msg_T                          backdoor_mgr_ipcmsg;
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    SYS_TYPE_HandleHotSwapArg_T                 HandleHotInsertionArg_ipcmsg;
#endif
} UDPHELPER_GROUP_MgrMsg_T;

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void UDPHELPER_GROUP_Mgr_Thread_Function_Entry(void* arg);
static void UDPHELPER_GROUP_SetTransitionMode(void);
static void UDPHELPER_GROUP_EnterTransitionMode(void);
static void UDPHELPER_GROUP_EnterMasterMode(void);
static void UDPHELPER_GROUP_EnterSlaveMode(void);
static void UDPHELPER_GROUP_HandleSysCallbackIPCMsg(SYSFUN_Msg_T *msgbuf_p);
static void UDPHELPER_GROUP_HandleHotInertion(SYSFUN_Msg_T* msgbuf_p);
static void UDPHELPER_GROUP_HandleHotRemoval(SYSFUN_Msg_T* msgbuf_p);

/* STATIC VARIABLE DECLARATIONS
 */
/* the buffer for retrieving ipc request for udphelper_group mgr thread
 */
static UI8_T udphelper_group_mgrtd_ipc_buf[SYSFUN_SIZE_OF_MSG(UDPHELPER_GROUP_PMGR_MSGBUF_MAX_REQUIRED_SIZE)];

/* EXPORTED SUBPROGRAM BODIES
 */


/*-----------------------------------------------------------------------------
 * FUNCTION NAME - UDPHELPER_GROUP_InitiateProcessResources
 *-----------------------------------------------------------------------------
 * PURPOSE : Initiate process resource for UDPHELPER group.
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
void UDPHELPER_GROUP_InitiateProcessResources(void)
{
    UDPHELPER_INIT_Initiate_System_Resources();
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - UDPHELPER_GROUP__Create_InterCSC_Relation
 *-----------------------------------------------------------------------------
 * PURPOSE : Create inter-CSC relationships for UDPHELPER group.
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
void UDPHELPER_GROUP_Create_InterCSC_Relation(void)
{
    UDPHELPER_MGR_Create_InterCSC_Relation();
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : UDPHELPER_GROUP_Create_All_Threads
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will spawn all threads in UDPHELPER group.
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
void UDPHELPER_GROUP_Create_All_Threads(void)
{
    UI32_T thread_id;

    if(SYSFUN_SpawnThread(SYS_BLD_UDPHELPER_GROUP_MGR_THREAD_PRIORITY,
                          SYS_BLD_UDPHELPER_GROUP_MGR_SCHED_POLICY,
                          SYS_BLD_UDPHELPER_GROUP_MGR_THREAD_NAME,
                          SYS_BLD_TASK_COMMON_STACK_SIZE,
                          SYSFUN_TASK_NO_FP,
                          UDPHELPER_GROUP_Mgr_Thread_Function_Entry,
                          NULL, &thread_id)!=SYSFUN_OK)
    {
        printf("%s:Spawn UDPHELPER MGR thread fail.\n", __FUNCTION__);
    }

}

/* LOCAL SUBPROGRAM BODIES
 */
/*------------------------------------------------------------------------------
 * ROUTINE NAME : UDPHELPER_GROUP_Mgr_Thread_Function_Entry
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This is the entry function for the MGR thread of UDPHELPER Group.
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
static void UDPHELPER_GROUP_Mgr_Thread_Function_Entry(void* arg)
{
    SYSFUN_MsgQ_T        ipc_msgq_handle;
    L_THREADGRP_Handle_T tg_handle = L2_L4_PROC_COMM_GetUdphelperGroupTGHandle();
    UI32_T               member_id,received_events,local_events = SYSFUN_SYSTEM_EVENT_IPCMSG;
    SYSFUN_Msg_T         *msgbuf_p = (SYSFUN_Msg_T*)udphelper_group_mgrtd_ipc_buf;
    BOOL_T               need_resp;

    /* join the thread group of UDPHELPER Group
     */
    if(L_THREADGRP_Join(tg_handle, SYS_BLD_UDPHELPER_GROUP_MGR_THREAD_PRIORITY, &member_id)==FALSE)
    {
        printf("%s: L_THREADGRP_Join fail.\n", __FUNCTION__);
        return;
    }

    /* create mgr ipc message queue handle
     */
    if(SYSFUN_CreateMsgQ(SYS_BLD_UDPHELPER_GROUP_IPCMSGQ_KEY, SYSFUN_MSGQ_BIDIRECTIONAL, &ipc_msgq_handle) != SYSFUN_OK)
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
            UDPHELPER_GROUP_SetTransitionMode();
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
                UDPHELPER_GROUP_PMGR_MSGBUF_MAX_REQUIRED_SIZE, msgbuf_p) == SYSFUN_OK)
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
                    case SYS_MODULE_UDPHELPER:
                        need_resp = UDPHELPER_MGR_HandleIPCReqMsg(msgbuf_p);
                        break;


                    case SYS_MODULE_BACKDOOR:
                        need_resp = BACKDOOR_MGR_HandleIPCMsg(msgbuf_p);
                        break;

                    case SYS_MODULE_SYS_CALLBACK:
                        /* SYS_CALLBACK ipc message can only be uni-direction
                         * just set need_resp as FALSE */                         
                        need_resp = FALSE;
                        UDPHELPER_GROUP_HandleSysCallbackIPCMsg(msgbuf_p);
                        break;

                    /* global cmd*/
                    case SYS_TYPE_CMD_ENTER_MASTER_MODE:
                        UDPHELPER_GROUP_EnterMasterMode();
        
                        /* SYS_TYPE_CMD_ENTER_MASTER_MODE
                         * need a response which contains nothing
                         */
                        msgbuf_p->msg_size = 0;
                        need_resp = TRUE;

                        /* IPCFAIL might happen when a upper layer udphelper group has
                         * entered transition mode but lower layer udphelper groups still
                         * in master mode so async callback msg will keep sending
                         * from lower layer udphelper group. In this case, the IPCFAIL
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
                        UDPHELPER_GROUP_EnterSlaveMode();

                        /* SYS_TYPE_CMD_ENTER_SLAVE_MODE
                         * need a response which contains nothing
                         */
                        msgbuf_p->msg_size = 0;
                        need_resp = TRUE;

                        /* IPCFAIL might happen when a upper layer  group has
                         * entered transition mode but lower layer UDPHELPER group still
                         * in master mode so async callback msg will keep sending
                         * from lower layer UDPHELPER group. In this case, the IPCFAIL
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
                        UDPHELPER_GROUP_EnterTransitionMode();

                        /* SYS_TYPE_CMD_ENTER_TRANSITION_MODE
                         * need a response which contains nothing
                         */
                        msgbuf_p->msg_size = 0;
                        need_resp = TRUE;
                        break;
						
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)

					case  SYS_TYPE_CMD_HANDLE_HOT_INSERTION:
                        UDPHELPER_GROUP_HandleHotInertion(msgbuf_p);
                        
						msgbuf_p->msg_size=0;
						need_resp=TRUE;
						break;
												
					case SYS_TYPE_CMD_HANDLE_HOT_REMOVAL:
                        UDPHELPER_GROUP_HandleHotRemoval(msgbuf_p);
						
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
 * ROUTINE NAME : UDPHELPER_GROUP_SetTransitionMode
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will invoke all set transition mode function in UDPHELPER Group.
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
static void UDPHELPER_GROUP_SetTransitionMode(void)
{
    UDPHELPER_INIT_SetTransitionMode();
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : UDPHELPER_GROUP_EnterTransitionMode
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will invoke all enter transition mode function in UDPHELPER Group.
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
static void UDPHELPER_GROUP_EnterTransitionMode(void)
{
    UDPHELPER_INIT_EnterTransitionMode();
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : UDPHELPER_GROUP_EnterMasterMode
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will invoke all set master mode function in UDPHELPER Group.
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
static void UDPHELPER_GROUP_EnterMasterMode(void)
{
    UDPHELPER_INIT_EnterMasterMode();
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : UDPHELPER_GROUP_EnterSlaveMode
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will invoke all enter slave mode function in UDPHELPER Group.
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
static void UDPHELPER_GROUP_EnterSlaveMode(void)
{
    UDPHELPER_INIT_EnterSlaveMode();
}

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
/*------------------------------------------------------------------------------
 * ROUTINE NAME : UDPHELPER_GROUP_HandleHotInertion
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
static void UDPHELPER_GROUP_HandleHotInertion(SYSFUN_Msg_T* msgbuf_p)
{
    SYS_TYPE_HandleHotSwapArg_T *msg_p;

    if (msgbuf_p == NULL)
        return;

    msg_p = (SYS_TYPE_HandleHotSwapArg_T*)msgbuf_p->msg_buf;
    return;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : UDPHELPER_GROUP_HandleHotRemoval
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
static void UDPHELPER_GROUP_HandleHotRemoval(SYSFUN_Msg_T* msgbuf_p)
{
    SYS_TYPE_HandleHotSwapArg_T *msg_p;

    if (msgbuf_p == NULL)
        return;

    msg_p = (SYS_TYPE_HandleHotSwapArg_T*)msgbuf_p->msg_buf;
    return;
}
#endif

/*------------------------------------------------------------------------
 * ROUTINE NAME - UDPHELPER_GROUP_RecvPacket_CallBack
 *------------------------------------------------------------------------
 * FUNCTION: This is the callback function for udphelper receive packet.
 *
 * INPUT   : 
 *
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : 
 *------------------------------------------------------------------------*/  
static void UDPHELPER_GROUP_RecvPacket_CallBack(L_MM_Mref_Handle_T *mref_handle_p,
                                                       UI32_T packet_length,
                                                       UI32_T ifindex,
                                                       UI8_T *dst_mac,
                                                       UI8_T *src_mac,
                                                       UI32_T vid,
                                                       UI32_T src_port)
{ 
    UDPHELPER_MGR_do_packet(mref_handle_p, packet_length, ifindex,
                            dst_mac, src_mac, vid, src_port);
} 

/*------------------------------------------------------------------------------
 * ROUTINE NAME : UDPHELPER_GROUP_HandleSysCallbackIPCMsg
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will handle all callbacks from IPC messages in UDPHELPER Group.
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
static void UDPHELPER_GROUP_HandleSysCallbackIPCMsg(SYSFUN_Msg_T *msgbuf_p)
{
    SYS_CALLBACK_MGR_Msg_T *sys_cbmsg_p = (SYS_CALLBACK_MGR_Msg_T*)msgbuf_p->msg_buf;

    switch(sys_cbmsg_p->callback_event_id)
    {
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_UDP_HELPER_PACKET:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &UDPHELPER_GROUP_RecvPacket_CallBack);
            break;

        default:
            SYSFUN_Debug_Printf("%s: received callback_event that is not handled(%d)\n",
                                   __FUNCTION__, sys_cbmsg_p->callback_event_id);
            break;
    }
}
#endif

