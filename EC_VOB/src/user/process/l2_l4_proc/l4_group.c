/* MODULE NAME:  l4_group.c
 * PURPOSE:
 *     This file is a demonstration code for implementations of l4 group.
 *
 * NOTES:
 *
 * HISTORY
 *    6/12/2007 - Eli Lin, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "sysfun.h"
#include "sys_cpnt.h"
#include "sys_bld.h"
#include "sys_module.h"
#include "sysrsc_mgr.h"
#include "sys_callback_mgr.h"
#include "l_threadgrp.h"
#include "backdoor_mgr.h"
#include "l2_l4_proc_comm.h"
#include "l4_group.h"

#if (SYS_CPNT_L4 == TRUE)
#include "l4_init.h"
#include "l4_mgr.h"
#endif

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
#include "sw_watchdog_mgr.h"
#endif

/* NAMING CONSTANT DECLARATIONS
 */
/* union all data type used for CSCGroup1 MGR IPC message to get the maximum
 * required ipc message buffer
 */
typedef union L4_GROUP_MGR_MSG_U
{
#if (SYS_CPNT_L4 == TRUE)
    L4_MGR_IPCMsg_T     l4_mgr_ipcmsg;
#endif

    BACKDOOR_MGR_Msg_T  backdoor_mgr_ipcmsg;
} L4_GROUP_MGR_MSG_T;

#define L4_GROUP_PMGR_MSGBUF_MAX_REQUIRED_SIZE sizeof(L4_GROUP_MGR_MSG_T)

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void L4_GROUP_Mgr_Thread_Function_Entry(void* arg);
static void L4_GROUP_SetTransitionMode(void);
static void L4_GROUP_EnterTransitionMode(void);
static void L4_GROUP_EnterMasterMode(void);
static void L4_GROUP_EnterSlaveMode(void);
static void L4_GROUP_HandleSysCallbackIPCMsg(SYSFUN_Msg_T *msgbuf_p);
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
/*------------------------------------------------------------------------------
 * ROUTINE NAME : L4_GROUP_HandleHotInertion
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
static void L4_GROUP_HandleHotInertion(SYSFUN_Msg_T* msgbuf_p);

static void L4_GROUP_HandleHotRemoval(SYSFUN_Msg_T* msgbuf_p);
#endif
/* STATIC VARIABLE DECLARATIONS
 */
/* the buffer for retrieving ipc request for l4_group mgr thread
 */
static UI8_T l4_group_mgrtd_ipc_buf[SYSFUN_SIZE_OF_MSG(L4_GROUP_PMGR_MSGBUF_MAX_REQUIRED_SIZE)];

/* EXPORTED SUBPROGRAM BODIES
 */


/*-----------------------------------------------------------------------------
 * FUNCTION NAME - L4_GROUP_InitiateProcessResources
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
void L4_GROUP_InitiateProcessResources(void)
{
#if (SYS_CPNT_L4 == TRUE)
    L4_INIT_Initiate_System_Resources();
#endif
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - L4_GROUP__Create_InterCSC_Relation
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
void L4_GROUP_Create_InterCSC_Relation(void)
{
#if (SYS_CPNT_L4 == TRUE)
    L4_INIT_Create_InterCSC_Relation();
#endif
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : L4_GROUP_Create_All_Threads
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will spawn all threads in CSCGroup1.
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
void L4_GROUP_Create_All_Threads(void)
{
    UI32_T thread_id;

    if(SYSFUN_SpawnThread(SYS_BLD_L4_GROUP_MGR_THREAD_PRIORITY,
                          SYS_BLD_L4_GROUP_MGR_SCHED_POLICY,
                          SYS_BLD_L4_GROUP_MGR_THREAD_NAME,
                          SYS_BLD_TASK_COMMON_STACK_SIZE,
                          SYSFUN_TASK_FP,
                          L4_GROUP_Mgr_Thread_Function_Entry,
                          NULL, &thread_id)!=SYSFUN_OK)
    {
        printf("%s:Spawn L4 Group MGR thread fail.\n", __FUNCTION__);
    }

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
    SW_WATCHDOG_MGR_RegisterMonitorThread(SW_WATCHDOG_L4_GROUP, thread_id, SYS_ADPT_L4_GROUP_SW_WATCHDOG_TIMER);
#endif

}

/* LOCAL SUBPROGRAM BODIES
 */
/*------------------------------------------------------------------------------
 * ROUTINE NAME : L4_GROUP_Mgr_Thread_Function_Entry
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This is the entry function for the MGR thread of CSCGroup1.
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
static void L4_GROUP_Mgr_Thread_Function_Entry(void* arg)
{
    SYSFUN_MsgQ_T        ipc_msgq_handle;
    L_THREADGRP_Handle_T tg_handle =L2_L4_PROC_COMM_GetL4GroupTGHandle();
    UI32_T               member_id,received_events,local_events=SYSFUN_SYSTEM_EVENT_IPCMSG;
    SYSFUN_Msg_T         *msgbuf_p=(SYSFUN_Msg_T*)l4_group_mgrtd_ipc_buf;
    BOOL_T               need_resp;
    void*                   timer_id;

    /* join the thread group of CSCGroup1
     */
    if(L_THREADGRP_Join(tg_handle, SYS_BLD_L4_GROUP_MGR_THREAD_PRIORITY, &member_id)==FALSE)
    {
        printf("%s: L_THREADGRP_Join fail.\n", __FUNCTION__);
        return;
    }

    /* create mgr ipc message queue handle
     */
    if(SYSFUN_CreateMsgQ(SYS_BLD_L4_GROUP_IPCMSGQ_KEY, SYSFUN_MSGQ_BIDIRECTIONAL, &ipc_msgq_handle)!=SYSFUN_OK)
    {
        printf("%s: SYSFUN_CreateMsgQ fail.\n", __FUNCTION__);
        return;
    }

    /* start the periodic timer event
     */
    timer_id = SYSFUN_PeriodicTimer_Create();
    if (SYSFUN_PeriodicTimer_Start(timer_id, RULE_TYPE_TIMER,
            RULE_TYPE_EVENT_TIMER) == FALSE)
    {
        printf("\r\n%s: Start timer failed!\r\n", __FUNCTION__);
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
#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
                             SYSFUN_SYSTEM_EVENT_SW_WATCHDOG |
#endif
                             SYSFUN_SYSTEM_EVENT_SET_TRANSITION_MODE | RULE_TYPE_EVENT_TIMER,
                             SYSFUN_EVENT_WAIT_ANY,
                             (local_events==0)?SYSFUN_TIMEOUT_WAIT_FOREVER:SYSFUN_TIMEOUT_NOWAIT,
                             &received_events);
        local_events |= received_events;

        if(local_events & SYSFUN_SYSTEM_EVENT_SET_TRANSITION_MODE)
        {
            L4_GROUP_SetTransitionMode();
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
                L4_GROUP_PMGR_MSGBUF_MAX_REQUIRED_SIZE, msgbuf_p)==SYSFUN_OK)
            {
                /* request thread group execution permission
                 */
                if(L_THREADGRP_Execution_Request(tg_handle, member_id)!=TRUE)
                    printf("%s: L_THREADGRP_Execution_Request fail.\n", __FUNCTION__);

                /* handle request message based on cmd
                 */
                switch(msgbuf_p->cmd)
                {
                    /* module id cmd
                     */
#if (SYS_CPNT_L4 == TRUE)
                    case SYS_MODULE_L4:
                        need_resp=L4_MGR_HandleIPCReqMsg(msgbuf_p);
                        break;
#endif

                    case SYS_MODULE_SYS_CALLBACK:
                        /* SYS_CALLBACK ipc message can only be uni-direction
                         * just set need_resp as FALSE
                         */
                        need_resp=FALSE;
                        L4_GROUP_HandleSysCallbackIPCMsg(msgbuf_p);
                        break;

                    case SYS_MODULE_BACKDOOR:
                        need_resp=BACKDOOR_MGR_HandleIPCMsg(msgbuf_p);
                        break;
                    /* global cmd
                     */
                    case SYS_TYPE_CMD_ENTER_MASTER_MODE:
                        L4_GROUP_EnterMasterMode();

                        /* SYS_TYPE_CMD_ENTER_MASTER_MODE
                         * need a response which contains nothing
                         */
                        msgbuf_p->msg_size=0;
                        need_resp=TRUE;

                        /* IPCFAIL might happen when a upper layer l4 group has
                         * entered transition mode but lower layer l4 groups still
                         * in master mode so async callback msg will keep sending
                         * from lower layer l4 group. In this case, the IPCFAIL
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
                        L4_GROUP_EnterSlaveMode();

                        /* SYS_TYPE_CMD_ENTER_SLAVE_MODE
                         * need a response which contains nothing
                         */
                        msgbuf_p->msg_size=0;
                        need_resp=TRUE;

                        /* IPCFAIL might happen when a upper layer l4 group has
                         * entered transition mode but lower layer l4 group still
                         * in master mode so async callback msg will keep sending
                         * from lower layer l4 group. In this case, the IPCFAIL
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
                        L4_GROUP_EnterTransitionMode();

                        /* SYS_TYPE_CMD_ENTER_TRANSITION_MODE
                         * need a response which contains nothing
                         */
                        msgbuf_p->msg_size=0;
                        need_resp=TRUE;
                        break;

                    case SYS_TYPE_CMD_PROVISION_COMPLETE:
                        /* SYS_TYPE_CMD_PROVISION_COMPLETE
                         * need a response which contains nothing
                         */
                        msgbuf_p->msg_size=0;
                        need_resp=TRUE;
                        break;
                        
                   case SYS_TYPE_CMD_MODULE_PROVISION_COMPLETE:
                        /*  need a response which contains nothing     */
                        msgbuf_p->msg_size = 0;
                        need_resp = TRUE;
                        break;
                        
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
                     case SYS_TYPE_CMD_HANDLE_HOT_INSERTION:
                       L4_GROUP_HandleHotInertion(msgbuf_p);
                       
                        msgbuf_p->msg_size=0;
                        need_resp=TRUE;
                        break;
                        
                     case SYS_TYPE_CMD_HANDLE_HOT_REMOVAL:
                       L4_GROUP_HandleHotRemoval(msgbuf_p);
                       
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
                        printf("%s: Invalid IPC req cmd.\n", __FUNCTION__);
                        need_resp=FALSE;
                        break;
                }

                /* release thread group execution permission
                 */
                if(L_THREADGRP_Execution_Release(tg_handle, member_id)!=TRUE)
                    printf("%s: L_THREADGRP_Execution_Release fail.\n", __FUNCTION__);

                if((need_resp==TRUE) && (SYSFUN_SendResponseMsg(ipc_msgq_handle, msgbuf_p)!=SYSFUN_OK))
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
		
        /* handle timer evnet
         */
        if (local_events & RULE_TYPE_EVENT_TIMER)
        {
            
            /* request thread group execution permission
             */
            if (L_THREADGRP_Execution_Request(tg_handle, member_id) != TRUE)
            {
                printf("\r\n%s: L_THREADGRP_Execution_Request fail.\r\n",
                    __FUNCTION__);
            }           

#if (SYS_CPNT_TIME_BASED_ACL == TRUE)
            L4_MGR_ACL_ProcessTimerEvent();
#endif

            /* release thread group execution permission
             */
            if (L_THREADGRP_Execution_Release(tg_handle, member_id) != TRUE)
            {
                printf("\r\n%s: L_THREADGRP_Execution_Release fail.\r\n",
                    __FUNCTION__);
            }

            local_events ^= RULE_TYPE_EVENT_TIMER;
        }
		
#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
        if (local_events & SYSFUN_SYSTEM_EVENT_SW_WATCHDOG)
        {
            SW_WATCHDOG_MGR_ResetTimer(SW_WATCHDOG_L4_GROUP);
            local_events ^= SYSFUN_SYSTEM_EVENT_SW_WATCHDOG;
        }
#endif

    } /* end of while(1) */
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : L4_GROUP_SetTransitionMode
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will invoke all set transition mode function in CSCGroup1.
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
static void L4_GROUP_SetTransitionMode(void)
{
#if (SYS_CPNT_L4 == TRUE)
    L4_INIT_SetTransitionMode();
#endif
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : L4_GROUP_EnterTransitionMode
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will invoke all enter transition mode function in CSCGroup1.
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
static void L4_GROUP_EnterTransitionMode(void)
{
#if (SYS_CPNT_L4 == TRUE)
    L4_INIT_EnterTransitionMode();
#endif
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : L4_GROUP_EnterMasterMode
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will invoke all set master mode function in CSCGroup1.
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
static void L4_GROUP_EnterMasterMode(void)
{
#if (SYS_CPNT_L4 == TRUE)
    L4_INIT_EnterMasterMode();
#endif
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : L4_GROUP_EnterSlaveMode
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will invoke all enter slave mode function in CSCGroup1.
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
static void L4_GROUP_EnterSlaveMode(void)
{
#if (SYS_CPNT_L4 == TRUE)
    L4_INIT_EnterSlaveMode();
#endif
}


/* -------------------------------------------------------------------------
 * ROUTINE NAME - L4_GROUP_TrunkMemberAdd1stCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when a logical port is added to a
 *           trunk
 * INPUT   : trunk_ifindex  -- which trunk port
 *           member_ifindex -- which member port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void  L4_GROUP_TrunkMemberAdd1stCallbackHandler(
    UI32_T trunk_ifindex,UI32_T member_ifindex)
{
#if (SYS_CPNT_L4 == TRUE)
    L4_MGR_COS_AddFirstTrunkMember(trunk_ifindex,member_ifindex);
#endif
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - L4_GROUP_TrunkMemberAddCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when a logical port is added to a
 *           trunk
 * INPUT   : trunk_ifindex  -- which trunk port
 *           member_ifindex -- which member port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void  L4_GROUP_TrunkMemberAddCallbackHandler(
    UI32_T trunk_ifindex,UI32_T member_ifindex)
{
#if (SYS_CPNT_L4 == TRUE)
    L4_MGR_COS_AddTrunkMember(trunk_ifindex,member_ifindex);
#endif
}

 /* -------------------------------------------------------------------------
 * ROUTINE NAME - L4_GROUP_TrunkMemberDeleteCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when a logical port is deleted from a
 *           trunk
 * INPUT   : trunk_ifindex  -- which trunk port
 *           member_ifindex -- which member port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void  L4_GROUP_TrunkMemberDeleteCallbackHandler(
    UI32_T trunk_ifindex,UI32_T member_ifindex)
{
#if (SYS_CPNT_L4 == TRUE)
    L4_MGR_COS_DelTrunkMember(trunk_ifindex,member_ifindex);
#endif
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - L4_GROUP_TrunkMemberDeleteLstCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when a logical port is deleted from a
 *           trunk
 * INPUT   : trunk_ifindex  -- which trunk port
 *           member_ifindex -- which member port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void  L4_GROUP_TrunkMemberDeleteLstCallbackHandler(
    UI32_T trunk_ifindex,UI32_T member_ifindex)
{
#if (SYS_CPNT_L4 == TRUE)
    L4_MGR_COS_DelLastTrunkMember(trunk_ifindex,member_ifindex);
#endif
}


/*------------------------------------------------------------------------------
 * FUNCTION NAME - PRI_MGR_Notify_CosChanged
 *------------------------------------------------------------------------------
 * PURPOSE  : Call CallBack function when cos mapping changed.
 * INPUT    : lport_ifindex - specify which port the event occured.
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *------------------------------------------------------------------------------*/
void L4_GROUP_CosChangedCallbackHandler(UI32_T lport_ifindex)
{
#if (SYS_CPNT_L4 == TRUE)
    L4_MGR_COS_CosLportConfigAsic(lport_ifindex);
#endif
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : L4_GROUP_HandleSysCallbackIPCMsg
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will handle all callbacks from IPC messages in CSCGroup1.
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
static void L4_GROUP_HandleSysCallbackIPCMsg(SYSFUN_Msg_T *msgbuf_p)
{
    SYS_CALLBACK_MGR_Msg_T *sys_cbmsg_p = (SYS_CALLBACK_MGR_Msg_T*)msgbuf_p->msg_buf;

    switch(sys_cbmsg_p->callback_event_id)
    {

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_TRUNK_MEMBER_ADD_1ST:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &L4_GROUP_TrunkMemberAdd1stCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_TRUNK_MEMBER_ADD:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &L4_GROUP_TrunkMemberAddCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_TRUNK_MEMBER_DELETE:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &L4_GROUP_TrunkMemberDeleteCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_TRUNK_MEMBER_DELETE_LST:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &L4_GROUP_TrunkMemberDeleteLstCallbackHandler);
            break;
/*
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_PRI_MGR_RegisterCosChanged_CallBack:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &L4_GROUP_CosChangedCallbackHandler);
            break;
*/
        default:
            SYSFUN_Debug_Printf("%s: received callback_event that is not handled(%d)\n",
                __FUNCTION__, sys_cbmsg_p->callback_event_id);
            break;
    }
}
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
/*------------------------------------------------------------------------------
 * ROUTINE NAME : L4_GROUP_HandleHotInertion
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
static void L4_GROUP_HandleHotInertion(SYSFUN_Msg_T* msgbuf_p)
{
 SYS_TYPE_HandleHotSwapArg_T *msg_p;

 if (msgbuf_p == NULL)
    return;

    msg_p = (SYS_TYPE_HandleHotSwapArg_T*)msgbuf_p->msg_buf;
  
    L4_INIT_HandleHotInsertion(msg_p->starting_port_ifindex, msg_p->number_of_port, msg_p->use_default);      
   
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
static void L4_GROUP_HandleHotRemoval(SYSFUN_Msg_T* msgbuf_p)
{
 SYS_TYPE_HandleHotSwapArg_T *msg_p;

 if (msgbuf_p == NULL)
    return;

    msg_p = (SYS_TYPE_HandleHotSwapArg_T*)msgbuf_p->msg_buf;
    
    L4_INIT_HandleHotRemoval(msg_p->starting_port_ifindex, msg_p->number_of_port);      
    
}
#endif

