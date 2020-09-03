/* MODULE NAME:  swctrl_group.c
 * PURPOSE:
 *     This file is a demonstration code for implementations of swctrl group.
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
#include "pri_mgr.h"
#include "swctrl_group.h"

#if (SYS_CPNT_SWCTRL == TRUE)
    #include "swctrl_init.h"
    #include "swctrl_task.h"
    #include "swctrl.h"
    #include "trk_init.h"
    #include "trk_mgr.h"
#endif

#if (SYS_CPNT_VLAN == TRUE)
    #include "vlan_init.h"
    #include "vlan_mgr.h"
#endif

#if (SYS_CPNT_LEDMGMT == TRUE)
    #include "ledmgmt_init.h"
    #include "led_mgr.h"
#endif

#if (SYS_CPNT_RSPAN == TRUE)
    #include "rspan_init.h"
    #include "rspan_mgr.h"
#endif

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
#include "sw_watchdog_mgr.h"
#endif

#if 1
#define DBG_PRINT(format,...) printf("%s()L%d "format"\r\n",__FUNCTION__,__LINE__,##__VA_ARGS__);
#else
#define DBG_PRINT(format,...)
#endif

/* NAMING CONSTANT DECLARATIONS
 */
/* union all data type used for CSCGroup1 MGR IPC message to get the maximum
 * required ipc message buffer
 */
typedef union SWCTRL_GROUP_MGR_MSG_U
{
    PRI_MGR_IpcMsg_T    pri_mgr_ipcmsg;
#if (SYS_CPNT_SWCTRL == TRUE)
    SWCTRL_MGR_IPCMsg_T swctrl_mgr_ipcmsg;
    TRK_MGR_IpcMsg_T    trk_mgr_ipcmsg;
#endif
#if (SYS_CPNT_VLAN == TRUE)
    VLAN_MGR_IpcMsg_T   vlan_mgr_ipcmsg;
#endif
#if (SYS_CPNT_LEDMGMT == TRUE)
    LED_MGR_IPCMsg_T    led_mgr_ipcmsg;
#endif
#if (SYS_CPNT_RSPAN == TRUE)
    RSPAN_MGR_IPCMsg_T    rspan_mgr_ipcmsg;
#endif
    BACKDOOR_MGR_Msg_T  backdoor_mgr_ipcmsg;
} SWCTRL_GROUP_MGR_MSG_T;

#define SWCTRL_GROUP_PMGR_MSGBUF_MAX_REQUIRED_SIZE sizeof(SWCTRL_GROUP_MGR_MSG_T)

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void SWCTRL_GROUP_Mgr_Thread_Function_Entry(void* arg);
static void SWCTRL_GROUP_SetTransitionMode(void);
static void SWCTRL_GROUP_EnterTransitionMode(void);
static void SWCTRL_GROUP_EnterMasterMode(void);
static void SWCTRL_GROUP_EnterSlaveMode(void);
static void SWCTRL_GROUP_ProvisionComplete(void);
static void SWCTRL_GROUP_PreProvisionComplete(void);
static void SWCTRL_GROUP_HandleSysCallbackIPCMsg(SYSFUN_Msg_T *msgbuf_p);
static void SWCTRL_GROUP_LoopbackPacketCallbackHandler(L_MM_Mref_Handle_T *mref_handle_p,
                                                      UI8_T          dst_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                      UI8_T          src_mac[SYS_ADPT_MAC_ADDR_LEN],
                                                      UI16_T         tag_info,
                                                      UI16_T         type,
                                                      UI32_T         pkt_length,
                                                      UI32_T         src_unit,
                                                      UI32_T         src_port);
/* STATIC VARIABLE DECLARATIONS
 */
/* the buffer for retrieving ipc request for swctrl_group mgr thread
 */
static UI8_T swctrl_group_mgrtd_ipc_buf[SYSFUN_SIZE_OF_MSG(SWCTRL_GROUP_PMGR_MSGBUF_MAX_REQUIRED_SIZE)];
static SYSFUN_MsgQ_T   ipc_msgq_handle;

#define SWCTRL_INTERNAL_LOOPBACK_TEST_TIMEOUT            (3 * SYS_BLD_TICKS_PER_SECOND)
#define SWCTRL_INTERNAL_LOOPBACK_TEST_TIMEOUT_EVENT      BIT_0
static UI32_T internal_loopback_resp_tid;
static void     *timer_id;
#if (SYS_CPNT_REFINE_IPC_MSG == TRUE)
static void* swctrl_group_timer_id;
#define SWCTRL_GROUP_TIMER_EVENT_INTERVAL 100
#define SWCTRL_GROUP_EVENT_PERIODIC_TIMER BIT_2
#endif
#if (SYS_CPNT_LEDMGMT == TRUE)
#define SWCTRL_GROUP_LED_TIMER_INTERVAL     10
#define SWCTRL_GROUP_LED_TIMER_EVENT        BIT_3
#endif

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
/*------------------------------------------------------------------------------
 * ROUTINE NAME : SWCTRL_GROUP_HandleHotInertion
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
static void SWCTRL_GROUP_HandleHotInertion(SYSFUN_Msg_T* msgbuf_p);

static void SWCTRL_GROUP_HandleHotRemoval(SYSFUN_Msg_T* msgbuf_p);
#endif

/* EXPORTED SUBPROGRAM BODIES
 */

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SWCTRL_GROUP_InitiateProcessResource
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will init  CSC Group.
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
 *------------------------------------------------------------------------------
 */
void SWCTRL_GROUP_InitiateProcessResource(void)
{
#if (SYS_CPNT_SWCTRL == TRUE)
    SWCTRL_INIT_Initiate_System_Resources();
    TRK_INIT_InitiateSystemResources();
#endif

#if (SYS_CPNT_VLAN == TRUE)
    VLAN_INIT_InitiateSystemResources();
#endif

    PRI_MGR_InitiateSystemResources();

#if (SYS_CPNT_LEDMGMT == TRUE)
    LEDMGMT_INIT_Initiate_System_Resources();
#endif

#if (SYS_CPNT_RSPAN == TRUE)
    RSPAN_INIT_InitiateSystemResources();
#endif

#if (SYS_CPNT_REFINE_IPC_MSG == TRUE)

    SYS_CALLBACK_REFINED_OM_Init();
#endif

}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SWCTRL_GROUP_Create_InterCSC_Relation
 *-----------------------------------------------------------------------------
 * PURPOSE : Create inter-CSC relationships for SWCTRL Group.
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
void SWCTRL_GROUP_Create_InterCSC_Relation(void)
{
#if (SYS_CPNT_SWCTRL == TRUE)
    SWCTRL_INIT_Create_InterCSC_Relation();
    TRK_INIT_Create_InterCSC_Relation();
#endif

#if (SYS_CPNT_VLAN == TRUE)
    VLAN_INIT_Create_InterCSC_Relation();
#endif

    PRI_MGR_Create_InterCSC_Relation();
#if (SYS_CPNT_LEDMGMT == TRUE)
    LEDMGMT_INIT_Create_InterCSC_Relation();
#endif
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SWCTRL_GROUP_Create_All_Threads
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
void SWCTRL_GROUP_Create_All_Threads(void)
{
    UI32_T thread_id;

#if (SYS_CPNT_SWCTRL == TRUE)
    SWCTRL_INIT_Create_Tasks();
#endif

#if (SYS_CPNT_POE == TRUE)
    /* Joeanne add for LED
     */
    LEDMGMT_INIT_Create_Tasks();
#endif

    if(SYSFUN_SpawnThread(SYS_BLD_SWCTRL_GROUP_MGR_THREAD_PRIORITY,
                          SYS_BLD_SWCTRL_GROUP_MGR_SCHED_POLICY,
                          SYS_BLD_SWCTRL_GROUP_MGR_THREAD_NAME,
                          SYS_BLD_TASK_COMMON_STACK_SIZE,
                          SYSFUN_TASK_FP,
                          SWCTRL_GROUP_Mgr_Thread_Function_Entry,
                          NULL, &thread_id)!=SYSFUN_OK)
    {
        printf("%s:Spawn CSCGroup1 MGR thread fail.\n", __FUNCTION__);
    }

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
    SW_WATCHDOG_MGR_RegisterMonitorThread(SW_WATCHDOG_SWCTRL_GROUP, thread_id, SYS_ADPT_SWCTRL_GROUP_SW_WATCHDOG_TIMER);
#endif

    VLAN_MGR_SetGroupThreadId(thread_id);
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SWCTRL_GROUP_AddStaticTrunkMemberCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Call call-back function, when UIs add static trunk member.
 *
 * INPUT   : trunk_ifindex --- Trunk member is deleted from which trunk
 *           tm_ifindex    --- Which trunk member is deleted from trunk
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
void SWCTRL_GROUP_AddStaticTrunkMemberCallbackHandler(UI32_T vid, UI32_T ifindex)
{
    return;
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SWCTRL_GROUP_DelStaticTrunkMemberCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Call call-back function, when UIs delete static trunk member.
 *
 * INPUT   : trunk_ifindex --- Trunk member is deleted from which trunk
 *           tm_ifindex    --- Which trunk member is deleted from trunk
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
void SWCTRL_GROUP_DelStaticTrunkMemberCallbackHandler(UI32_T vid, UI32_T ifindex)
{
    return;
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SWCTRL_GROUP_VlanActiveToSuspendCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Call call-back function when VLAN state is changed from active to
 *           suspended.
 *
 * INPUT   : vid_ifindex - vlan ifindex whose state is changed
 *           vlan_status - vlan status which changes the state of the VLAN
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
void SWCTRL_GROUP_VlanActiveToSuspendCallbackHandler(UI32_T vid_ifindex, UI32_T vlan_status)
{
#if (SYS_CPNT_RSPAN == TRUE)
    RSPAN_MGR_VlanActiveToSuspendCallback(vid_ifindex, vlan_status);
#endif
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SWCTRL_GROUP_VlanSuspendToActiveCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Call call-back function when VLAN state is changed from suspended
 *           to active.
 *
 * INPUT   : vid_ifindex - vlan ifindex whose state is changed
 *           vlan_status - vlan status which changes the state of the VLAN
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
void SWCTRL_GROUP_VlanSuspendToActiveCallbackHandler(UI32_T vid_ifindex, UI32_T vlan_status)
{
#if (SYS_CPNT_RSPAN == TRUE)
    RSPAN_MGR_VlanSuspendToActiveCallback(vid_ifindex, vlan_status);
#endif
}


/* LOCAL SUBPROGRAM BODIES
 */
/*------------------------------------------------------------------------------
 * ROUTINE NAME : SWCTRL_GROUP_Mgr_Thread_Function_Entry
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
static void SWCTRL_GROUP_Mgr_Thread_Function_Entry(void* arg)
{

    L_THREADGRP_Handle_T tg_handle = L2_L4_PROC_COMM_GetSwctrlGroupTGHandle();
    UI32_T               member_id,received_events,local_events=SYSFUN_SYSTEM_EVENT_IPCMSG;
    SYSFUN_Msg_T         *msgbuf_p=(SYSFUN_Msg_T*)swctrl_group_mgrtd_ipc_buf;
    BOOL_T               need_resp;
    UI32_T               last_process_vid = 0;
#if (SYS_CPNT_LEDMGMT == TRUE)
    void                 *led_timer_id;
#endif


#if (SYS_CPNT_INTERNAL_LOOPBACK_TEST)
    internal_loopback_resp_tid = 0;
    timer_id = SYSFUN_PeriodicTimer_Create();
#endif

    /* join the thread group of CSCGroup1
     */
    if(L_THREADGRP_Join(tg_handle, SYS_BLD_SWCTRL_GROUP_MGR_THREAD_PRIORITY, &member_id)==FALSE)
    {
        printf("%s: L_THREADGRP_Join fail.\n", __FUNCTION__);
        return;
    }

    /* create mgr ipc message queue handle
     */
    if(SYSFUN_CreateMsgQ(SYS_BLD_SWCTRL_GROUP_IPCMSGQ_KEY, SYSFUN_MSGQ_BIDIRECTIONAL, &ipc_msgq_handle)!=SYSFUN_OK)
    {
        printf("%s: SYSFUN_CreateMsgQ fail.\n", __FUNCTION__);
        return;
    }

#if (SYS_CPNT_REFINE_IPC_MSG == TRUE)
    swctrl_group_timer_id = SYSFUN_PeriodicTimer_Create();
    if(FALSE==SYSFUN_PeriodicTimer_Start(swctrl_group_timer_id, SWCTRL_GROUP_TIMER_EVENT_INTERVAL, SWCTRL_GROUP_EVENT_PERIODIC_TIMER))
        printf("\r\n%s: SYSFUN_PeriodicTimer_Start fail", __FUNCTION__);
#endif

#if (SYS_CPNT_LEDMGMT == TRUE)
    led_timer_id = SYSFUN_PeriodicTimer_Create();
    if(FALSE==SYSFUN_PeriodicTimer_Start(led_timer_id, SWCTRL_GROUP_LED_TIMER_INTERVAL, SWCTRL_GROUP_LED_TIMER_EVENT))
        printf("\r\n%s: SYSFUN_PeriodicTimer_Start fail", __FUNCTION__);
#endif

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
#if (SYS_CPNT_LEDMGMT == TRUE)
                             SWCTRL_GROUP_LED_TIMER_EVENT | /* BIT_3 */
#endif
                             SWCTRL_INTERNAL_LOOPBACK_TEST_TIMEOUT_EVENT |  /* BIT_0 */
                             VLAN_TYPE_PORTSTATE_EVENT |                    /* BIT_1 */
#if (SYS_CPNT_REFINE_IPC_MSG == TRUE)
                             SWCTRL_GROUP_EVENT_PERIODIC_TIMER |
#endif
                             SYSFUN_SYSTEM_EVENT_SET_TRANSITION_MODE,
                             SYSFUN_EVENT_WAIT_ANY,
                             (local_events==0)?SYSFUN_TIMEOUT_WAIT_FOREVER:SYSFUN_TIMEOUT_NOWAIT,
                             &received_events);
        local_events |= received_events;

/*EPR:ES3628BT-FLF-ZZ-00179
    Problem:system:DUT ip interface is down even the vlan is up and ip is configured
    RootCause: 1 vlan_operstatus_changed is used to check if the vlan state is change or not
                          2, vlan will send event to notify vlan state change.And swctrl_group will process 5 vlan step by step
                          3, when the second portstate event  of vlan1,and  swctrl_group is process vlan 100, swctrl_group will
                             never process vlan 1 again,it just process vlan after 100
   Solution:when receive portstate event,it will to check vlan 1 to last vlan again
   File:Swctrl_group.c
*/
        if (received_events & VLAN_TYPE_PORTSTATE_EVENT)
        {
           last_process_vid = 0;
        }

        if(local_events & SYSFUN_SYSTEM_EVENT_SET_TRANSITION_MODE)
        {
            SWCTRL_GROUP_SetTransitionMode();
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
                SWCTRL_GROUP_PMGR_MSGBUF_MAX_REQUIRED_SIZE, msgbuf_p)==SYSFUN_OK)
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
                    case SYS_MODULE_PRIMGMT:
                        need_resp=PRI_MGR_HandleIPCReqMsg(msgbuf_p);
                        break;

#if (SYS_CPNT_SWCTRL == TRUE)
                    case SYS_MODULE_SWCTRL:
                        need_resp=SWCTRL_MGR_HandleIPCReqMsg(msgbuf_p);
                        break;

                    case SYS_MODULE_TRUNK:
                        need_resp=TRK_MGR_HandleIPCReqMsg(msgbuf_p);
                        break;
#endif

#if (SYS_CPNT_VLAN == TRUE)
                    case SYS_MODULE_VLAN:
                        need_resp=VLAN_MGR_HandleIPCReqMsg(msgbuf_p);
                        break;
#endif

#if (SYS_CPNT_LEDMGMT == TRUE)
                    case SYS_MODULE_LEDMGMT:
                        need_resp=LED_MGR_HandleIPCReqMsg(msgbuf_p);
                        break;
#endif

#if (SYS_CPNT_RSPAN == TRUE)
                    case SYS_MODULE_RSPAN:
                        need_resp=RSPAN_MGR_HandleIPCReqMsg(msgbuf_p);
                        break;
#endif

                    case SYS_MODULE_SYS_CALLBACK:
                        /* SYS_CALLBACK ipc message can only be uni-direction
                         * just set need_resp as FALSE
                         */
                        need_resp=FALSE;
                        SWCTRL_GROUP_HandleSysCallbackIPCMsg(msgbuf_p);
                        break;

                    case SYS_MODULE_BACKDOOR:
                        need_resp=BACKDOOR_MGR_HandleIPCMsg(msgbuf_p);
                        break;
                    /* global cmd
                     */
                    case SYS_TYPE_CMD_ENTER_MASTER_MODE:
                        SWCTRL_GROUP_EnterMasterMode();

                        /* SYS_TYPE_CMD_ENTER_MASTER_MODE
                         * need a response which contains nothing
                         */
                        msgbuf_p->msg_size=0;
                        need_resp=TRUE;

                        /* IPCFAIL might happen when a upper layer swctrl group has
                         * entered transition mode but lower layer swctrl groups still
                         * in master mode so async callback msg will keep sending
                         * from lower layer swctrl group. In this case, the IPCFAIL
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
                        SWCTRL_GROUP_EnterSlaveMode();

                        /* SYS_TYPE_CMD_ENTER_SLAVE_MODE
                         * need a response which contains nothing
                         */
                        msgbuf_p->msg_size=0;
                        need_resp=TRUE;

                        /* IPCFAIL might happen when a upper layer swctrl group has
                         * entered transition mode but lower layer swctrl group still
                         * in master mode so async callback msg will keep sending
                         * from lower layer swctrl group. In this case, the IPCFAIL
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
                        SWCTRL_GROUP_EnterTransitionMode();
                        last_process_vid = 0;

                        /* SYS_TYPE_CMD_ENTER_TRANSITION_MODE
                         * need a response which contains nothing
                         */
                        msgbuf_p->msg_size=0;
                        need_resp=TRUE;
                        break;

                    case SYS_TYPE_CMD_PROVISION_COMPLETE:
                    case SYS_TYPE_CMD_MODULE_PROVISION_COMPLETE:
                        SWCTRL_GROUP_ProvisionComplete();

                        /* SYS_TYPE_CMD_PROVISION_COMPLETE
                         * need a response which contains nothing
                         */
                        msgbuf_p->msg_size=0;
                        need_resp=TRUE;
                        break;

                    case SYS_TYPE_CMD_PREPROVISION_COMPLETE:
                        SWCTRL_GROUP_PreProvisionComplete();

                        /* SYS_TYPE_CMD_PROVISION_COMPLETE
                         * need a response which contains nothing
                         */
                        msgbuf_p->msg_size=0;
                        need_resp=TRUE;
                        break;
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
                     case     SYS_TYPE_CMD_HANDLE_HOT_INSERTION:
                       SWCTRL_GROUP_HandleHotInertion(msgbuf_p);

                        msgbuf_p->msg_size=0;
                        need_resp=TRUE;
                        break;

                     case SYS_TYPE_CMD_HANDLE_HOT_REMOVAL:
                       SWCTRL_GROUP_HandleHotRemoval(msgbuf_p);

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

#if (SYS_CPNT_REFINE_IPC_MSG == TRUE)
        if(local_events & SWCTRL_GROUP_EVENT_PERIODIC_TIMER)
        {
           SYS_CALLBACK_MGR_ProcessRefineOmDB();
            local_events ^= SWCTRL_GROUP_EVENT_PERIODIC_TIMER;
        }
#endif
        /* handle vlan event
         */
        if (local_events & VLAN_TYPE_PORTSTATE_EVENT)
        {
            BOOL_T is_clean_event = FALSE;
            /* request thread group execution permission
             */
            if (L_THREADGRP_Execution_Request(tg_handle, member_id) != TRUE)
            {
                printf("\n%s: L_THREADGRP_Execution_Request fail.\n",
                    __FUNCTION__);
            }

            is_clean_event = VLAN_MGR_ProcessForwardingSignal(&last_process_vid);

            /* release thread group execution permission
             */
            if (L_THREADGRP_Execution_Release(tg_handle, member_id) != TRUE)
            {
                printf("\n%s: L_THREADGRP_Execution_Release fail.\n",
                    __FUNCTION__);
            }

            if (is_clean_event == TRUE)
                local_events ^= VLAN_TYPE_PORTSTATE_EVENT;
        }

#if (SYS_CPNT_INTERNAL_LOOPBACK_TEST == TRUE)
        if(local_events & SWCTRL_INTERNAL_LOOPBACK_TEST_TIMEOUT_EVENT)
        {
            BOOL_T resp_flag = FALSE;
            if (L_THREADGRP_Execution_Request(tg_handle, member_id) != TRUE)
            {
                printf("\n%s: L_THREADGRP_Execution_Request fail.\n",
                    __FUNCTION__);
            }
            resp_flag = SWCTRL_InternalLoopbackTimeout(msgbuf_p);

            if(resp_flag == TRUE && SYSFUN_SendResponseMsg(ipc_msgq_handle, msgbuf_p) != SYSFUN_OK)
            {

                printf("%s: SYSFUN_SendResponseMsg fail.\n", __FUNCTION__);fflush(stdout);
            }

            /* release thread group execution permission
             */
            if (L_THREADGRP_Execution_Release(tg_handle, member_id) != TRUE)
            {
                printf("\n%s: L_THREADGRP_Execution_Release fail.\n",
                    __FUNCTION__);
            }
            local_events ^= SWCTRL_INTERNAL_LOOPBACK_TEST_TIMEOUT_EVENT;
        }
#endif

#if (SYS_CPNT_LEDMGMT == TRUE)
        if (local_events & SWCTRL_GROUP_LED_TIMER_EVENT)
        {
            LED_MGR_Display();

            local_events ^= SWCTRL_GROUP_LED_TIMER_EVENT;
        }
#endif

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

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
        if(local_events & SYSFUN_SYSTEM_EVENT_SW_WATCHDOG)
        {
            SW_WATCHDOG_MGR_ResetTimer(SW_WATCHDOG_SWCTRL_GROUP);
            local_events ^= SYSFUN_SYSTEM_EVENT_SW_WATCHDOG;
        }
#endif
    } /* end of while(1) */
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SWCTRL_GROUP_SetTransitionMode
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
static void SWCTRL_GROUP_SetTransitionMode(void)
{
#if (SYS_CPNT_SWCTRL == TRUE)
    SWCTRL_INIT_SetTransitionMode();
    TRK_INIT_SetTransitionMode();
#endif

#if (SYS_CPNT_VLAN == TRUE)
    VLAN_INIT_SetTransitionMode();
#endif

    PRI_MGR_SetTransitionMode();

#if (SYS_CPNT_LEDMGMT == TRUE)
    LED_MGR_Set_TransitionMode();
#endif

#if (SYS_CPNT_RSPAN == TRUE)
    RSPAN_INIT_SetTransitionMode();
#endif

}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SWCTRL_GROUP_EnterTransitionMode
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
static void SWCTRL_GROUP_EnterTransitionMode(void)
{
#if (SYS_CPNT_SWCTRL == TRUE)
    SWCTRL_INIT_EnterTransitionMode();
    TRK_INIT_EnterTransitionMode();
#endif

#if (SYS_CPNT_VLAN == TRUE)
    VLAN_INIT_EnterTransitionMode();
#endif

    PRI_MGR_EnterTransitionMode();

#if (SYS_CPNT_LEDMGMT == TRUE)
    LED_MGR_EnterTransitionMode();
#endif

#if (SYS_CPNT_RSPAN == TRUE)
    RSPAN_INIT_EnterTransitionMode();
#endif
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SWCTRL_GROUP_EnterMasterMode
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
static void SWCTRL_GROUP_EnterMasterMode(void)
{
#if (SYS_CPNT_SWCTRL == TRUE)
    SWCTRL_INIT_EnterMasterMode();
    TRK_INIT_EnterMasterMode();
#endif

#if (SYS_CPNT_VLAN == TRUE)
    VLAN_INIT_EnterMasterMode();
#endif

    PRI_MGR_EnterMasterMode();

#if (SYS_CPNT_LEDMGMT == TRUE)
    LED_MGR_EnterMasterMode();
#endif

#if (SYS_CPNT_RSPAN == TRUE)
    RSPAN_INIT_EnterMasterMode();
#endif

}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SWCTRL_GROUP_EnterSlaveMode
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
static void SWCTRL_GROUP_EnterSlaveMode(void)
{
#if (SYS_CPNT_SWCTRL == TRUE)
    SWCTRL_INIT_EnterSlaveMode();
    TRK_INIT_EnterSlaveMode();
#endif

#if (SYS_CPNT_VLAN == TRUE)
    VLAN_INIT_EnterSlaveMode();
#endif

    PRI_MGR_EnterSlaveMode();
#if (SYS_CPNT_LEDMGMT == TRUE)
    LED_MGR_EnterSlaveMode();
#endif

#if (SYS_CPNT_RSPAN == TRUE)
    RSPAN_INIT_EnterSlaveMode();
#endif

}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SWCTRL_GROUP_ProvisionComplete
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will invoke all provision complete function in SWCTRL group.
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
static void SWCTRL_GROUP_ProvisionComplete(void)
{
#if (SYS_CPNT_SWCTRL == TRUE)
    SWCTRL_ProvisionComplete();
#endif

#if (SYS_CPNT_VLAN == TRUE)
    VLAN_INIT_ProvisionComplete();
#endif

#if (SYS_CPNT_LEDMGMT == TRUE)
    LED_MGR_Provision_Complete();
#endif
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SWCTRL_GROUP_PreprovisionComplete
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will invoke all preprovision complete function in SWCTRL group.
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
static void SWCTRL_GROUP_PreProvisionComplete(void)
{
#if (SYS_CPNT_SWCTRL == TRUE)
    SWCTRL_PreProvisionComplete();
#endif

#if (SYS_CPNT_VLAN == TRUE)
    VLAN_INIT_PreProvisionComplete();
#endif
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GROUP_SwdrvPortLinkUpCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: Port link up callback function, register to swdrv
 * INPUT   : unit -- in which unit
 *           port -- which port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_GROUP_SwdrvPortLinkUpCallbackHandler(UI32_T unit,
                                            UI32_T port)
{
    #if (SYS_CPNT_SWCTRL == TRUE)
        SWCTRL_TASK_PortLinkUp_CallBack(unit,port);
    #endif
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GROUP_SwdrvPortLinkDownCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: Port link down callback function, register to swdrv
 * INPUT   : unit -- in which unit
 *           port -- which port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_GROUP_SwdrvPortLinkDownCallbackHandler(UI32_T unit,
                                            UI32_T port)
{
    #if (SYS_CPNT_SWCTRL == TRUE)
        SWCTRL_TASK_PortLinkDown_CallBack(unit,port);
    #endif
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GROUP_SwdrvCraftPortLinkUpCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: Port link up callback function, register to swdrv
 * INPUT   : unit -- in which unit
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_GROUP_SwdrvCraftPortLinkUpCallbackHandler(UI32_T unit)
{
    #if (SYS_CPNT_SWCTRL == TRUE)
        SWCTRL_TASK_CraftPortLinkUp_CallBack(unit);
    #endif
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GROUP_SwdrvCraftPortLinkDownCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: Port link down callback function, register to swdrv
 * INPUT   : unit -- in which unit
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_GROUP_SwdrvCraftPortLinkDownCallbackHandler(UI32_T unit)
{
    #if (SYS_CPNT_SWCTRL == TRUE)
        SWCTRL_TASK_CraftPortLinkDown_CallBack(unit);
    #endif
}
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GROUP_SwdrvPortTypeChangedCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: Port link down callback function, register to swdrv
 * INPUT   : unit -- in which unit
 *           port -- which port
 *           module_id
 *           port_type -- port type to set
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_GROUP_SwdrvPortTypeChangedCallbackHandler(UI32_T unit,
                                                  UI32_T port,
                                                  UI32_T module_id,
                                                  UI32_T port_type)
{
    #if (SYS_CPNT_SWCTRL == TRUE)
        SWCTRL_TASK_PortTypeChanged_CallBack(unit,port,module_id,port_type);
    #endif
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GROUP_SwdrvPortSpeedDuplexCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: Port speed/duplex callback function, register to swdrv
 * INPUT   : unit         -- in which unit
 *           port         -- which port
 *           speed_duplex -- the speed/duplex status
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_GROUP_SwdrvPortSpeedDuplexCallbackHandler( UI32_T unit,
                                                  UI32_T port,
                                                  UI32_T port_type)
{
    #if (SYS_CPNT_SWCTRL == TRUE)
        SWCTRL_TASK_PortSpeedDuplex_CallBack(unit,port,port_type);
    #endif
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GROUP_SwdrvPortFlowCtrlCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: Port speed/duplex callback function, register to swdrv
 * INPUT   : unit      -- in which unit
 *           port      -- which port
 *           flow_ctrl -- the flow control status
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_GROUP_SwdrvPortFlowCtrlCallbackHandler(UI32_T unit,
                                              UI32_T port,
                                              UI32_T flow_ctrl)
{
    #if (SYS_CPNT_SWCTRL == TRUE)
        SWCTRL_TASK_PortFlowCtrl_CallBack(unit,port,flow_ctrl);
    #endif
}

#if (SYS_CPNT_SWDRV_MONITOR_SFP_DDM == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GROUP_SwdrvPortSfpPresentCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: Port sfp present callback function, register to swdrv
 * INPUT   : unit -- in which unit
 *           port -- which port
 *           is_present -- present or not
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
void SWCTRL_GROUP_SwdrvPortSfpPresentCallbackHandler(UI32_T unit,
                                                      UI32_T port,
                                                      BOOL_T is_present)
{
#if (SYS_CPNT_SWCTRL == TRUE)
    SWCTRL_TASK_PortSfpPresent_CallBack(unit, port, is_present);
#endif
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GROUP_SwdrvPortSfpInfoCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: Port sfp info callback function, register to swdrv
 * INPUT   : unit -- in which unit
 *           sfp_index -- which sfp_index
 *           sfp_info_p  -- sfp eeprom info
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
void SWCTRL_GROUP_SwdrvPortSfpInfoCallbackHandler(UI32_T unit,
                                                  UI32_T sfp_index,
                                                  SWDRV_TYPE_SfpInfo_T *sfp_info_p)
{
#if (SYS_CPNT_SWCTRL == TRUE)
    SWCTRL_TASK_PortSfpInfo_CallBack(unit, sfp_index, sfp_info_p);
#endif
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GROUP_SwdrvPortSfpDdmInfoCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: Port sfp ddm info callback function, register to swdrv
 * INPUT   : unit -- in which unit
 *           sfp_index -- which sfp_index
 *           sfp_ddm_info_p -- sfp DDM eeprom info
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
void SWCTRL_GROUP_SwdrvPortSfpDdmInfoCallbackHandler(UI32_T unit,
                                                     UI32_T sfp_index,
                                                     SWDRV_TYPE_SfpDdmInfo_T *sfp_ddm_info_p)
{
#if (SYS_CPNT_SWCTRL == TRUE)
    SWCTRL_TASK_PortSfpDdmInfo_CallBack(unit, sfp_index, sfp_ddm_info_p);
#endif
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GROUP_SwdrvPortSfpDdmInfoMeasuredCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: Port sfp ddm info callback function, register to swdrv
 * INPUT   : unit -- in which unit
 *           sfp_index -- which sfp_index
 *           sfp_ddm_info_measured_p -- sfp DDM measured eeprom info
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
void SWCTRL_GROUP_SwdrvPortSfpDdmInfoMeasuredCallbackHandler(UI32_T unit,
                                                      UI32_T sfp_index,
                                                      SWDRV_TYPE_SfpDdmInfoMeasured_T *sfp_ddm_info_measured_p)
{
#if (SYS_CPNT_SWCTRL == TRUE)
    SWCTRL_TASK_PortSfpDdmInfoMeasured_CallBack(unit, sfp_index, sfp_ddm_info_measured_p);
#endif
}
#endif /* #if (SYS_CPNT_SWDRV_MONITOR_SFP_DDM == TRUE) */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GROUP_LPortTypeChangedCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when a hot swap module is removed.
 * INPUT   : ifindex   -- which logical port.
 *           port_type -- changed to which port type.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_GROUP_LPortTypeChangedCallbackHandler(
    UI32_T ifindex, UI32_T port_type)
{
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GROUP_UPortTypeChangedCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when a hot swap module is removed.
 * INPUT   : ifindex   -- which logical port.
 *           port_type -- changed to which port type.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_GROUP_UPortTypeChangedCallbackHandler(
    UI32_T unit, UI32_T port, UI32_T port_type)
{
#if (SYS_CPNT_LEDMGMT == TRUE)
    LED_MGR_PortTypeChanged(unit, port, port_type);
#endif
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GROUP_TrunkMemberAdd1stCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when a logical port is added to a
 *           trunk
 * INPUT   : trunk_ifindex  -- which trunk port
 *           member_ifindex -- which member port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_GROUP_TrunkMemberAdd1stCallbackHandler(
    UI32_T trunk_ifindex,UI32_T member_ifindex)
{
    PRI_MGR_AddFirstTrunkMember_CallBack(trunk_ifindex, member_ifindex);

#if (SYS_CPNT_VLAN == TRUE)
    VLAN_MGR_AddFirstTrunkMember_CallBack(trunk_ifindex, member_ifindex);
#endif
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GROUP_TrunkMemberAddCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when a logical port is added to a
 *           trunk
 * INPUT   : trunk_ifindex  -- which trunk port
 *           member_ifindex -- which member port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_GROUP_TrunkMemberAddCallbackHandler(
    UI32_T trunk_ifindex,UI32_T member_ifindex)
{
    PRI_MGR_AddTrunkMember_CallBack(trunk_ifindex, member_ifindex);

#if (SYS_CPNT_VLAN == TRUE)
    VLAN_MGR_AddTrunkMember_CallBack(trunk_ifindex, member_ifindex);
#endif
}

 /* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GROUP_TrunkMemberDeleteCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when a logical port is deleted from a
 *           trunk
 * INPUT   : trunk_ifindex  -- which trunk port
 *           member_ifindex -- which member port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_GROUP_TrunkMemberDeleteCallbackHandler(
    UI32_T trunk_ifindex,UI32_T member_ifindex)
{
    PRI_MGR_DeleteTrunkMember_CallBack(trunk_ifindex, member_ifindex);

#if (SYS_CPNT_VLAN == TRUE)
    VLAN_MGR_DeleteTrunkMember_CallBack(trunk_ifindex, member_ifindex);
#endif
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GROUP_TrunkMemberDeleteLstCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when a logical port is deleted from a
 *           trunk
 * INPUT   : trunk_ifindex  -- which trunk port
 *           member_ifindex -- which member port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_GROUP_TrunkMemberDeleteLstCallbackHandler(
    UI32_T trunk_ifindex,UI32_T member_ifindex)
{
    PRI_MGR_DeleteLastTrunkMember_CallBack(trunk_ifindex, member_ifindex);

#if (SYS_CPNT_VLAN == TRUE)
    VLAN_MGR_DeleteLastTrunkMember_CallBack(trunk_ifindex, member_ifindex);
#endif
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GROUP_PortLinkUpCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when the link is up
 * INPUT   : ifindex -- which logical port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_GROUP_PortLinkUpCallbackHandler(
    UI32_T ifindex)
{
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GROUP_uPortLinkUpCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when the link is up
 * INPUT   : unit -- in which unit
 *           port -- which logical port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_GROUP_uPortLinkUpCallbackHandler(
    UI32_T unit,UI32_T port)
{
    #if (SYS_CPNT_LEDMGMT == TRUE)
        LED_MGR_Linkup(unit, port);
    #endif
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GROUP_uPortFastLinkUpCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when the link is up
 * INPUT   : unit -- in which unit
 *           port -- which logical port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/

void SWCTRL_GROUP_uPortFastLinkUpCallbackHandler(
    UI32_T unit,UI32_T port)
{
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GROUP_PortLinkDownCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when the link is down
 * INPUT   : ifindex -- which logical port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_GROUP_PortLinkDownCallbackHandler(
    UI32_T ifindex)
{
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GROUP_uPortLinkDownCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when the link is down
 * INPUT   : unit -- in which unit
 *           port -- which logical port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_GROUP_uPortLinkDownCallbackHandler(
    UI32_T unit,UI32_T port)
{
    #if (SYS_CPNT_LEDMGMT == TRUE)
        LED_MGR_Linkdown(unit, port);
    #endif
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GROUP_uPortFastLinkDownCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when the link is down
 * INPUT   : unit -- in which unit
 *           port -- which logical port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_GROUP_uPortFastLinkDownCallbackHandler(
    UI32_T unit,UI32_T port)
{
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GROUP_PortOperUpCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when the oper status is up
 * INPUT   : ifindex -- which logical port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_GROUP_PortOperUpCallbackHandler(
    UI32_T ifindex)
{
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GROUP_PortNotOperUpCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when the oper status is not up
 * INPUT   : ifindex -- which logical port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_GROUP_PortNotOperUpCallbackHandler(
    UI32_T ifindex)
{
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GROUP_TrunkMemberPortOperUpCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when the oper status is up
 * INPUT   : ifindex -- which logical port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_GROUP_TrunkMemberPortOperUpCallbackHandler(
    UI32_T trunk_ifindex,UI32_T trunk_member_port_ifindex)
{
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GROUP_TrunkMemberPortNotOperUpCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when the oper status is down
 * INPUT   : ifindex -- which logical port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_GROUP_TrunkMemberPortNotOperUpCallbackHandler(
    UI32_T trunk_ifindex,UI32_T trunk_member_port_ifindex)
{
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GROUP_TrunkMemberActiveCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when member port is active
 * INPUT   : ifindex -- which logical port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_GROUP_TrunkMemberActiveCallbackHandler(
    UI32_T trunk_ifindex,UI32_T trunk_member_port_ifindex)
{
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GROUP_TrunkMemberInactiveCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when member port is inactive
 * INPUT   : ifindex -- which logical port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_GROUP_TrunkMemberInactiveCallbackHandler(
    UI32_T trunk_ifindex,UI32_T trunk_member_port_ifindex)
{
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GROUP_PortAdminEnableCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when the port is enabled
 * INPUT   : ifindex -- which logical port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_GROUP_PortAdminEnableCallbackHandler(
    UI32_T ifindex)
{
}

 /* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_Notify_PortAdminDisable
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when the port is disabled
 * INPUT   : ifindex -- which logical port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_GROUP_PortAdminDisableCallbackHandler (
    UI32_T ifindex)
{
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_Notify_uPortAdminEnable
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when the port is enabled
 * INPUT   : unit -- in which unit
 *           port -- which logical port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_GROUP_uPortAdminEnableCallbackHandler(
    UI32_T unit,UI32_T port)
{
#if (SYS_CPNT_LEDMGMT == TRUE)
    LED_MGR_AdminEnable(unit, port);
#endif
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GROUP_PortAdminDisableBeforeCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when the port is disabled
 * INPUT   : ifindex -- which logical port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : for notify LLDP before doing shutdown port
 * -------------------------------------------------------------------------*/

void SWCTRL_GROUP_PortAdminDisableBeforeCallbackHandler(
    UI32_T ifindex)
{
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GROUP_uPortAdminDisableCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when the port is disabled
 * INPUT   : unit -- in which unit
 *           port -- which logical port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_GROUP_uPortAdminDisableCallbackHandler(
    UI32_T unit,UI32_T port)
{
#if (SYS_CPNT_LEDMGMT == TRUE)
    LED_MGR_AdminDisable(unit, port);
#endif
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GROUP_PortStatusChangedPassivelyCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when the port status is changed passively
 * INPUT   : ifindex -- which logical port
 *           status
 *           changed_bmp
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_GROUP_PortStatusChangedPassivelyCallbackHandler(UI32_T ifindex, BOOL_T status, UI32_T changed_bmp)
{
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GROUP_PortSpeedDuplexCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when the speed/duplex status of a port
 *           is changed
 * INPUT   : ifindex -- which logical port
 *           speed_duplex -- new status of speed/duplex
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_GROUP_PortSpeedDuplexCallbackHandler(
    UI32_T ifindex,UI32_T speed_duplex)
{
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GROUP_uPortSpeedDuplexCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when the speed/duplex status of a port
 *           is changed
 * INPUT   : unit -- in which unit
 *           port -- which logical port
 *           speed_duplex -- new status of speed/duplex
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_GROUP_uPortSpeedDuplexCallbackHandler(
    UI32_T unit,UI32_T port,UI32_T speed_duplex)
{
#if (SYS_CPNT_LEDMGMT == TRUE)
    LED_MGR_SpeedDuplexChange(unit, port, speed_duplex);
#endif
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GROUP_uPortLacpEffectiveOperStatusChangedCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when the effective oper status change
 *           for LACP.
 *           is changed
 * INPUT   : unit           --- which unit
 *           port           --- which port
 *           pre_status     --- status before change
 *           current_status --- status after change
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/

void SWCTRL_GROUP_uPortLacpEffectiveOperStatusChangedCallbackHandler(
    UI32_T unit,UI32_T port,UI32_T pre_status, UI32_T current_status )
{
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GROUP_uPortDot1xEffectiveOperStatusChangedCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when the effective oper status change
 *           for DOT1x.
 * INPUT   : unit           --- which unit
 *           port           --- which port
 *           pre_status     --- status before change
 *           current_status --- status after change
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_GROUP_uPortDot1xEffectiveOperStatusChangedCallbackHandler(
    UI32_T unit, UI32_T port,UI32_T pre_status, UI32_T current_status )
{
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GROUP_PortEffectiveOperStatusChangedCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when the effective oper status change.
 * INPUT   : ifindex        --- which ifindex
 *           pre_status     --- status before change
 *           current_status --- status after change
 *           level          --- see SWCTRL_OperDormantLevel_T
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : When to callback:
 *           1) Oper status becomes effecitve.
 *              Oper status is changed from lower status to specified dormant
 *              status.
 *           2) Oper status becomes ineffecitve.
 *              Oper status is changed from specified dormant status or upper
 *              status to lower status.
 * -------------------------------------------------------------------------*/
void SWCTRL_GROUP_PortEffectiveOperStatusChangedCallbackHandler(
    UI32_T ifindex,
    UI32_T pre_status,
    UI32_T current_status,
    SWCTRL_OperDormantLevel_T level)
{
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GROUP_ForwardingUPortAddToTrunkCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when forwarding uport added to trunk.
 * INPUT   : trunk_ifindex  - Which trunk.
 *           member_ifindex - Which trunk member.
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTE    : None.
 * -------------------------------------------------------------------------*/
void SWCTRL_GROUP_ForwardingUPortAddToTrunkCallbackHandler(
    UI32_T trunk_ifindex, UI32_T member_ifindex)
{
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GROUP_ForwardingTrunkMemberDeleteCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when forwarding trunk member deleted.
 * INPUT   : trunk_ifindex  - Which trunk.
 *           member_ifindex - Which trunk member.
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTE    : None.
 * -------------------------------------------------------------------------*/
void SWCTRL_GROUP_ForwardingTrunkMemberDeleteCallbackHandler(
    UI32_T trunk_ifindex, UI32_T member_ifindex)
{
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GROUP_ForwardingTrunkMemberToNonForwardingCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when forwarding trunk member become non-forwarding.
 * INPUT   : trunk_ifindex  - Which trunk.
 *           member_ifindex - Which trunk member.
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTE    : None.
 * -------------------------------------------------------------------------*/
void SWCTRL_GROUP_ForwardingTrunkMemberToNonForwardingCallbackHandler(
    UI32_T trunk_ifindex, UI32_T member_ifindex)
{
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GROUP_PortLearningStatusChangedCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when learning status changed.
 * INPUT   : lport
 *           learning
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTE    : None.
 * -------------------------------------------------------------------------*/
void SWCTRL_GROUP_PortLearningStatusChangedCallbackHandler(
    UI32_T lport, BOOL_T learning)
{
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SWCTRL_GROUP_HandleSysCallbackIPCMsg
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
static void SWCTRL_GROUP_HandleSysCallbackIPCMsg(SYSFUN_Msg_T *msgbuf_p)
{
    SYS_CALLBACK_MGR_Msg_T *sys_cbmsg_p = (SYS_CALLBACK_MGR_Msg_T*)msgbuf_p->msg_buf;

    switch(sys_cbmsg_p->callback_event_id)
    {
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_SWDRV_PORT_LINK_UP:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &SWCTRL_GROUP_SwdrvPortLinkUpCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_SWDRV_PORT_LINK_DOWN:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &SWCTRL_GROUP_SwdrvPortLinkDownCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_SWDRV_CRAFT_PORT_LINK_UP:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &SWCTRL_GROUP_SwdrvCraftPortLinkUpCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_SWDRV_CRAFT_PORT_LINK_DOWN:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &SWCTRL_GROUP_SwdrvCraftPortLinkDownCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_SWDRV_PORT_TYPE_CHANGED:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &SWCTRL_GROUP_SwdrvPortTypeChangedCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_SWDRV_PORT_SPEED_DUPLEX:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &SWCTRL_GROUP_SwdrvPortSpeedDuplexCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_SWDRV_PORT_FLOW_CTRL:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &SWCTRL_GROUP_SwdrvPortFlowCtrlCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_LOOPBACK_PACKET:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &SWCTRL_GROUP_LoopbackPacketCallbackHandler);
            break;

    #if (SYS_CPNT_SWDRV_MONITOR_SFP_DDM == TRUE)
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_SWDRV_PORT_SFP_PRESENT:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &SWCTRL_GROUP_SwdrvPortSfpPresentCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_SWDRV_PORT_SFP_INFO:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &SWCTRL_GROUP_SwdrvPortSfpInfoCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_SWDRV_PORT_SFP_DDM_INFO:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &SWCTRL_GROUP_SwdrvPortSfpDdmInfoCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_SWDRV_PORT_SFP_DDM_INFO_MEASURED:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &SWCTRL_GROUP_SwdrvPortSfpDdmInfoMeasuredCallbackHandler);
            break;
    #endif /* #if (SYS_CPNT_SWDRV_MONITOR_SFP_DDM == TRUE) */

        default:
            SYSFUN_Debug_Printf("%s: received callback_event that is not handled(%d)\n",
                __FUNCTION__, sys_cbmsg_p->callback_event_id);
    }
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SWCTRL_GROUP_StartInternalLoopbackTimeoutTimer
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will be call to start the loopback timout timer.
 *
 * INPUT:
 *    None
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
static void SWCTRL_GROUP_LoopbackPacketCallbackHandler(L_MM_Mref_Handle_T *mref_handle_p,
                                              UI8_T          dst_mac[SYS_ADPT_MAC_ADDR_LEN],
                                              UI8_T          src_mac[SYS_ADPT_MAC_ADDR_LEN],
                                              UI16_T         tag_info,
                                              UI16_T         type,
                                              UI32_T         pkt_length,
                                              UI32_T         src_unit,
                                              UI32_T         src_port)
{
#if (SYS_CPNT_INTERNAL_LOOPBACK_TEST == TRUE)
    BOOL_T need_resp = FALSE;
    SYSFUN_Msg_T* msgbuf_p = (SYSFUN_Msg_T*)swctrl_group_mgrtd_ipc_buf;

    /* 1. check the packet */
    need_resp = SWCTRL_PacketHandler(dst_mac, src_mac, type, msgbuf_p);

    if((need_resp==TRUE) && (SYSFUN_SendResponseMsg(ipc_msgq_handle, msgbuf_p)!=SYSFUN_OK))
                    printf("%s: SYSFUN_SendResponseMsg fail.\n", __FUNCTION__);
#endif

    /* 2. always release packet */
    L_MM_Mref_Release(&mref_handle_p);
}

#if (SYS_CPNT_INTERNAL_LOOPBACK_TEST == TRUE)
/*------------------------------------------------------------------------------
 * ROUTINE NAME : SWCTRL_GROUP_StartInternalLoopbackTimeoutTimer
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will be call to start the loopback timout timer.
 *
 * INPUT:
 *    None
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
BOOL_T SWCTRL_GROUP_StartInternalLoopbackTimeoutTimer()
{
    if (SYSFUN_PeriodicTimer_Start(timer_id, SWCTRL_INTERNAL_LOOPBACK_TEST_TIMEOUT,
            SWCTRL_INTERNAL_LOOPBACK_TEST_TIMEOUT_EVENT) == FALSE)
    {
        printf("\r\n%s: Start timer failed!\r\n", __FUNCTION__);
        return FALSE;
    }

    return TRUE;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SWCTRL_GROUP_StopInternalLoopbackTimeoutTimer
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will be call to stop the loopback timout timer.
 *
 * INPUT:
 *    None
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
BOOL_T SWCTRL_GROUP_StopInternalLoopbackTimeoutTimer()
{
    if (SYSFUN_PeriodicTimer_Stop(timer_id) == FALSE)
    {
        printf("\r\n%s: Stop timer failed!\r\n", __FUNCTION__);
        return FALSE;
    }
    return TRUE;
}

#endif
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
/*------------------------------------------------------------------------------
 * ROUTINE NAME : SWCTRL_GROUP_HandleHotInertion
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
static void SWCTRL_GROUP_HandleHotInertion(SYSFUN_Msg_T* msgbuf_p)
{
 SYS_TYPE_HandleHotSwapArg_T *msg_p;

 if (msgbuf_p == NULL)
    return;

    msg_p = (SYS_TYPE_HandleHotSwapArg_T*)msgbuf_p->msg_buf;

    #if (SYS_CPNT_SWCTRL == TRUE)
    SWCTRL_INIT_HandleHotInsertion(msg_p->starting_port_ifindex, msg_p->number_of_port, msg_p->use_default);
    TRK_INIT_HandleHotInsertion(msg_p->starting_port_ifindex, msg_p->number_of_port, msg_p->use_default);
    #endif

    #if (SYS_CPNT_VLAN == TRUE)
    VLAN_INIT_HandleHotInsertion(msg_p->starting_port_ifindex, msg_p->number_of_port, msg_p->use_default);
    #endif
    PRI_MGR_HandleHotInsertion(msg_p->starting_port_ifindex, msg_p->number_of_port, msg_p->use_default);

    #if (SYS_CPNT_LEDMGMT == TRUE)
    LED_MGR_HandleHotInsertion(msg_p->starting_port_ifindex, msg_p->number_of_port, msg_p->use_default);
    #endif
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SWCTRL_GROUP_HandleHotRemoval
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
static void SWCTRL_GROUP_HandleHotRemoval(SYSFUN_Msg_T* msgbuf_p)
{
 SYS_TYPE_HandleHotSwapArg_T *msg_p;

 if (msgbuf_p == NULL)
    return;

    msg_p = (SYS_TYPE_HandleHotSwapArg_T*)msgbuf_p->msg_buf;

    #if (SYS_CPNT_SWCTRL == TRUE)
    SWCTRL_INIT_HandleHotRemoval(msg_p->starting_port_ifindex, msg_p->number_of_port);
    TRK_INIT_HandleHotRemoval(msg_p->starting_port_ifindex, msg_p->number_of_port);
    #endif

    #if (SYS_CPNT_VLAN == TRUE)
    VLAN_INIT_HandleHotRemoval(msg_p->starting_port_ifindex, msg_p->number_of_port);
    #endif
    PRI_MGR_HandleHotRemoval(msg_p->starting_port_ifindex, msg_p->number_of_port);

    #if (SYS_CPNT_LEDMGMT == TRUE)
    LED_MGR_HandleHotRemoval(msg_p->starting_port_ifindex, msg_p->number_of_port);
    #endif
}
#endif


