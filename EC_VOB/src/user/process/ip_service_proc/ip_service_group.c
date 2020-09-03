/*-----------------------------------------------------------------------------
 * FILE NAME: IP_SERVICE_GROUP.C
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *    Implements the APIs of IP_SERVICE group.
 *
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    2007/08/01     --- Timon, Create
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
#include "l_threadgrp.h"
#include "sysfun.h"
#include "backdoor_mgr.h"
#if (SYS_CPNT_DHCP == TRUE)
#include "dhcp_init.h"
#include "dhcp_mgr.h"
#include "dhcp_type.h"
#endif
#if (SYS_CPNT_DHCPV6 == TRUE)
#include "dhcpv6_init.h"
#include "dhcpv6_mgr.h"
#endif
#include "ip_service_proc_comm.h"
#include "sys_callback_mgr.h"

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
#include "sw_watchdog_mgr.h"
#endif

/* NAMING CONSTANT DECLARATIONS
 */

/* the size of the ipc msg buf for the MGR thread of IP_SERVICE group to
 * receive and response PMGR ipc. The size of this buffer should pick the
 * maximum of size required for PMGR ipc request and response in all CSCs
 * handled in this process. (this size does not include IPC msg header)
 */
#define IP_SERVICE_GROUP_PMGR_MSGBUF_MAX_REQUIRED_SIZE sizeof(IP_SERVICE_GROUP_MgrMsg_T)


/* MACRO FUNCTION DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */
typedef union {
    SYS_CALLBACK_MGR_IfOperStatusChanged_CBData_T     sys_callback_mgr_if_oper_status_changed_cbdata;
    SYS_CALLBACK_MGR_VlanCreate_CBData_T              sys_callback_mgr_vlan_create_cbdata;
    SYS_CALLBACK_MGR_VlanDestroy_CBData_T             sys_callback_mgr_vlan_destroy_cbdata;
#if (SYS_CPNT_DHCPV6 == TRUE)    
    SYS_CALLBACK_MGR_IPV6_SetManualAddr_CBData_T      sys_callback_mgr_ipv6_set_maunal_addr_cbdata;
    SYS_CALLBACK_MGR_IPV6_AddrAutoconfig_CBData_T     sys_callback_mgr_ipv6_addr_autoconfig_cbdata;
    SYS_CALLBACK_MGR_L3IF_Create_CBData_T             sys_callback_mgr_l3if_create_cbdata;
    SYS_CALLBACK_MGR_L3IF_Destroy_CBData_T            sys_callback_mgr_l3if_destroy_cbdata;
    SYS_CALLBACK_MGR_L3IF_OperStatusUp_CBData_T       sys_callback_mgr_l3if_oper_status_up_cbdata;
    SYS_CALLBACK_MGR_L3IF_OperStatusDown_CBData_T     sys_callback_mgr_l3if_oper_status_down_cbdata;
#endif    

    SYS_CALLBACK_MGR_RifCreated_CBData_T              sys_callback_mgr_rif_create_cbdata;
    SYS_CALLBACK_MGR_RifDestroyed_CBData_T            sys_callback_mgr_rif_destroy_cbdata;
    SYS_CALLBACK_MGR_RifActive_CBData_T               sys_callback_mgr_rif_active_cbdata;

} IP_SERVICE_GROUP_Syscallback_CBData_T;

typedef struct
{
    UI8_T buffer[SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(IP_SERVICE_GROUP_Syscallback_CBData_T))];
} IP_SERVICE_GROUP_Syscallback_Msg_T;

/* union all data type used for IP_SERVICE group MGR IPC message to get the
 * maximum required ipc message buffer
 */
typedef union IP_SERVICE_GROUP_MgrMsg_U
{
#if (SYS_CPNT_DHCP == TRUE)
    DHCP_MGR_IpcMsg_T   dhcp_mgr_ipcmsg;
#endif
#if (SYS_CPNT_DHCPV6 == TRUE)
    DHCPv6_MGR_IpcMsg_T   dhcpv6_mgr_ipcmsg;
#endif

    BACKDOOR_MGR_Msg_T  backdoor_mgr_ipcmsg;
    
    IP_SERVICE_GROUP_Syscallback_Msg_T   sys_callback_ipcmsg;
    
} IP_SERVICE_GROUP_MgrMsg_T;


/* LOCAL SUBPROGRAM DECLARATIONS
 */

static void IP_SERVICE_GROUP_Mgr_Thread_Function_Entry(void* arg);
static void IP_SERVICE_GROUP_SetTransitionMode(void);
static void IP_SERVICE_GROUP_EnterTransitionMode(void);
static void IP_SERVICE_GROUP_EnterMasterMode(void);
static void IP_SERVICE_GROUP_EnterSlaveMode(void);
static void IP_SERVICE_GROUP_ProvisionComplete(void);
static void IP_SERVICE_GROUP_HandleSysCallbackIPCMsg(SYSFUN_Msg_T *msgbuf_p);
static void IP_SERVICE_GROUP_ReceiveBootpPacketCallbackHandler(
                L_MM_Mref_Handle_T *mref_handle_p,
                UI32_T packet_length,
                UI32_T rxRifNum,
                UI8_T *dst_mac,
                UI8_T *src_mac,
                UI32_T vid,
                UI32_T src_lport_ifIndex);

#if (SYS_CPNT_DHCPV6 == TRUE)
static UI32_T IP_SERVICE_GROUP_SignalAddrModeChangedCallbackHandler(UI32_T vid_ifIndex, BOOL_T auto_conf_status);
static void IP_SERVICE_GROUP_IfCreateCallbackHandler(UI32_T vid_ifIndex, BOOL_T auto_conf_status);
static void IP_SERVICE_GROUP_IfOperStatusUpCallbackHandler(UI32_T vid_ifIndex, BOOL_T auto_conf_status);
static void IP_SERVICE_GROUP_IfOperStatusDownCallbackHandler(UI32_T vid_ifIndex);
#endif

static void IP_SERVICE_GROUP_IfDeleteCallbackHandler(UI32_T vid_ifIndex);
static void IP_SERVICE_GROUP_RifDestroyCallbackHandler(UI32_T vid_ifindex, L_INET_AddrIp_T *addr);
static void IP_SERVICE_GROUP_RifCreateCallbackHandler(UI32_T vid_ifindex, L_INET_AddrIp_T *addr);
static void IP_SERVICE_GROUP_RifActiveCallbackHandler(UI32_T vid_ifindex, L_INET_AddrIp_T *addr);


/* STATIC VARIABLE DEFINITIONS
 */

/* the buffer for retrieving ipc request for IP_SERVICE group mgr thread
 */
static UI8_T ip_service_group_mgrtd_ipc_buf[SYSFUN_SIZE_OF_MSG(IP_SERVICE_GROUP_PMGR_MSGBUF_MAX_REQUIRED_SIZE)];


/* EXPORTED SUBPROGRAM BODIES
 */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - IP_SERVICE_GROUP_InitiateProcessResources
 *-----------------------------------------------------------------------------
 * PURPOSE : Initiate process resource for IP_SERVICE group.
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
void IP_SERVICE_GROUP_InitiateProcessResources(void)
{
#if (SYS_CPNT_DHCP == TRUE)
    DHCP_INIT_InitiateSystemResources();
#endif
#if (SYS_CPNT_DHCPV6 == TRUE)
    DHCPv6_INIT_InitiateSystemResources();
#endif

    return;
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - IP_SERVICE_GROUP_Create_InterCSC_Relation
 *-----------------------------------------------------------------------------
 * PURPOSE : Create inter-CSC relationships for IP_SERVICE group.
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
void IP_SERVICE_GROUP_Create_InterCSC_Relation(void)
{
#if (SYS_CPNT_DHCP == TRUE)
    DHCP_INIT_Create_InterCSC_Relation();
#endif
#if (SYS_CPNT_DHCPV6 == TRUE)
    DHCPv6_INIT_Create_InterCSC_Relation();
#endif

    return;
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - STA_GROUP_Create_All_Threads
 *-----------------------------------------------------------------------------
 * PURPOSE : This function will spawn all threads in IP_SERVICE group.
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
void IP_SERVICE_GROUP_Create_All_Threads(void)
{
    UI32_T thread_id;

    if (SYSFUN_SpawnThread(SYS_BLD_IP_SERVICE_GROUP_MGR_THREAD_PRIORITY,
                           SYS_BLD_IP_SERVICE_GROUP_MGR_SCHED_POLICY,
                           SYS_BLD_IP_SERVICE_GROUP_MGR_THREAD_NAME,
                           SYS_BLD_TASK_LARGE_STACK_SIZE,
                           SYSFUN_TASK_FP,
                           IP_SERVICE_GROUP_Mgr_Thread_Function_Entry,
                           NULL,
                           &thread_id) != SYSFUN_OK)
    {
        printf("\r\n%s: Spawn IP_SERVICE Group MGR thread fail.\r\n", __FUNCTION__);
    }

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
    SW_WATCHDOG_MGR_RegisterMonitorThread(SW_WATCHDOG_IP_SERVICE_GROUP, thread_id, SYS_ADPT_IP_SERVICE_GROUP_SW_WATCHDOG_TIMER);
#endif
    return;
}


/* LOCAL SUBPROGRAM BODIES
 */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - IP_SERVICE_GROUP_Mgr_Thread_Function_Entry
 *-----------------------------------------------------------------------------
 * PURPOSE : This is the entry function for the MGR thread of IP_SERVICE group.
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
static void IP_SERVICE_GROUP_Mgr_Thread_Function_Entry(void* arg)
{
    SYSFUN_MsgQ_T           ipc_msgq_handle;
    L_THREADGRP_Handle_T    tg_handle = IP_SERVICE_PROC_COMM_GetIpServiceGroupTGHandle();
    UI32_T                  member_id, received_events, local_events = SYSFUN_SYSTEM_EVENT_IPCMSG;
    SYSFUN_Msg_T            *msgbuf_p = (SYSFUN_Msg_T*)ip_service_group_mgrtd_ipc_buf;
    BOOL_T                  need_resp;
	UI32_T timeout;
#if (SYS_CPNT_DHCP == TRUE)
    void                    *dhcp_timer_id;
#endif
    UI32_T                  all_events =
                                SYSFUN_SYSTEM_EVENT_SET_TRANSITION_MODE |
                                SYSFUN_SYSTEM_EVENT_IPCMSG |
#if (SYS_CPNT_DHCP == TRUE)
                                DHCP_TYPE_EVENT_TIMER_1_SEC |
#endif
#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
                                SYSFUN_SYSTEM_EVENT_SW_WATCHDOG |
#endif
                                SYSFUN_SYSTEM_EVENT_IPCFAIL;

    /* join the thread group of IP_SERVICE Group
     */
    if (L_THREADGRP_Join(tg_handle, SYS_BLD_IP_SERVICE_GROUP_MGR_THREAD_PRIORITY,
            &member_id) == FALSE)
    {
        printf("\r\n%s: L_THREADGRP_Join fail.\r\n", __FUNCTION__);
        return;
    }

    /* create mgr ipc message queue handle
     */
    if (SYSFUN_CreateMsgQ(SYS_BLD_IP_SERVICE_GROUP_IPCMSGQ_KEY,
            SYSFUN_MSGQ_BIDIRECTIONAL, &ipc_msgq_handle) != SYSFUN_OK)
    {
        printf("\r\n%s: SYSFUN_CreateMsgQ fail.\r\n", __FUNCTION__);
        return;
    }

#if (SYS_CPNT_DHCP == TRUE)
    dhcp_timer_id = SYSFUN_PeriodicTimer_Create();
    SYSFUN_PeriodicTimer_Start(dhcp_timer_id, SYS_BLD_TICKS_PER_SECOND, DHCP_TYPE_EVENT_TIMER_1_SEC);
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
        if (local_events == 0 && !DHCP_MGR_IsSocketDirty()
#if (SYS_CPNT_DHCPV6 == TRUE)
             &&!DHCPv6_MGR_IsSocketDirty()
#endif /* #if (SYS_CPNT_DHCPV6 == TRUE) */
           )
        {
           timeout=SYSFUN_TIMEOUT_WAIT_FOREVER;
        }
        else
        {
            timeout = SYSFUN_TIMEOUT_NOWAIT;
        }

        SYSFUN_ReceiveEvent(all_events,
                            SYSFUN_EVENT_WAIT_ANY,
                            timeout,
                            &received_events);

        local_events |= received_events;

        if (local_events & SYSFUN_SYSTEM_EVENT_SET_TRANSITION_MODE)
        {
            IP_SERVICE_GROUP_SetTransitionMode();
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
                    IP_SERVICE_GROUP_PMGR_MSGBUF_MAX_REQUIRED_SIZE, msgbuf_p)
                    == SYSFUN_OK)
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
                        IP_SERVICE_GROUP_EnterTransitionMode();

                        /* SYS_TYPE_CMD_ENTER_TRANSITION_MODE
                         * need a response which contains nothing
                         */
                        msgbuf_p->msg_size = 0;
                        need_resp = TRUE;
                        break;

                    case SYS_TYPE_CMD_ENTER_MASTER_MODE:
                        IP_SERVICE_GROUP_EnterMasterMode();

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
                        IP_SERVICE_GROUP_EnterSlaveMode();

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
                        IP_SERVICE_GROUP_ProvisionComplete();
                        msgbuf_p->msg_size = 0;
                        need_resp = TRUE;
                        break;                 

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
                     case  SYS_TYPE_CMD_HANDLE_HOT_INSERTION:
                       
                        msgbuf_p->msg_size=0;
                        need_resp=TRUE;
                        break;
                        
                     case SYS_TYPE_CMD_HANDLE_HOT_REMOVAL:
                      
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

#if (SYS_CPNT_DHCP == TRUE)
                    case SYS_MODULE_DHCP:
                        need_resp = DHCP_MGR_HandleIPCReqMsg(msgbuf_p);
                        break;
#endif
#if (SYS_CPNT_DHCPV6 == TRUE)
                    case SYS_MODULE_DHCPv6:
                        need_resp = DHCPv6_MGR_HandleIPCReqMsg(msgbuf_p);
                        break;
#endif

                    case SYS_MODULE_BACKDOOR:
                        need_resp = BACKDOOR_MGR_HandleIPCMsg(msgbuf_p);
                        break;

                    case SYS_MODULE_SYS_CALLBACK:
                        IP_SERVICE_GROUP_HandleSysCallbackIPCMsg(msgbuf_p);
                        /* SYS_CALLBACK ipc message can only be uni-direction
                         * just set need_resp as FALSE
                         */
                        need_resp = FALSE;
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

#if (SYS_CPNT_DHCP == TRUE)
        DHCP_MGR_Dispatch();

        /* handle DHCP timer event
         */
        if (local_events & DHCP_TYPE_EVENT_TIMER_1_SEC)
        {
            /* request thread group execution permission
             */
            if (L_THREADGRP_Execution_Request(tg_handle, member_id) != TRUE)
            {
                printf("\r\n%s: L_THREADGRP_Execution_Request fail.\r\n",
                    __FUNCTION__);
            }

            DHCP_MGR_CheckTimeout(SYSFUN_GetSysTick() / SYS_BLD_TICKS_PER_SECOND);

            /* release thread group execution permission
             */
            if (L_THREADGRP_Execution_Release(tg_handle, member_id) != TRUE)
            {
                printf("\r\n%s: L_THREADGRP_Execution_Release fail.\r\n",
                    __FUNCTION__);
            }

            local_events ^= DHCP_TYPE_EVENT_TIMER_1_SEC;
        }
#endif
#if (SYS_CPNT_DHCPV6 == TRUE)
        DHCPv6_MGR_Dispatch();
#endif

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
       	    SW_WATCHDOG_MGR_ResetTimer(SW_WATCHDOG_IP_SERVICE_GROUP);
            local_events ^= SYSFUN_SYSTEM_EVENT_SW_WATCHDOG;
        }
#endif

    } /* end of while (1) */
} /* End of IP_SERVICE_GROUP_Mgr_Thread_Function_Entry */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - IP_SERVICE_GROUP_SetTransitionMode
 *-----------------------------------------------------------------------------
 * PURPOSE : This function will invoke all set transition mode function in
 *           IP_SERVICE group.
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
static void IP_SERVICE_GROUP_SetTransitionMode(void)
{
#if (SYS_CPNT_DHCP == TRUE)
    DHCP_INIT_SetTransitionMode();
#endif
#if (SYS_CPNT_DHCPV6 == TRUE)
    DHCPv6_INIT_SetTransitionMode();
#endif

    return;
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - IP_SERVICE_GROUP_EnterTransitionMode
 *-----------------------------------------------------------------------------
 * PURPOSE : This function will invoke all enter transition mode function in
 *           IP_SERVICE group.
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
static void IP_SERVICE_GROUP_EnterTransitionMode(void)
{
#if (SYS_CPNT_DHCP == TRUE)
    DHCP_INIT_EnterTransitionMode();
#endif
#if (SYS_CPNT_DHCPV6 == TRUE)
    DHCPv6_INIT_EnterTransitionMode();
#endif

    return;
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - IP_SERVICE_GROUP_EnterMasterMode
 *-----------------------------------------------------------------------------
 * PURPOSE : This function will invoke all set master mode function in
 *           IP_SERVICE group.
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
static void IP_SERVICE_GROUP_EnterMasterMode(void)
{
#if (SYS_CPNT_DHCP == TRUE)
    DHCP_INIT_EnterMasterMode();
#endif
#if (SYS_CPNT_DHCPV6 == TRUE)
    DHCPv6_INIT_EnterMasterMode();
#endif

    return;
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - IP_SERVICE_GROUP_EnterSlaveMode
 *-----------------------------------------------------------------------------
 * PURPOSE : This function will invoke all enter slave mode function in
 *           IP_SERVICE group.
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
static void IP_SERVICE_GROUP_EnterSlaveMode(void)
{
#if (SYS_CPNT_DHCP == TRUE)
    DHCP_INIT_EnterSlaveMode();
#endif
#if (SYS_CPNT_DHCPV6 == TRUE)
    DHCPv6_INIT_EnterSlaveMode();
#endif

    return;
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - IP_SERVICE_GROUP_ProvisionComplete
 *-----------------------------------------------------------------------------
 * PURPOSE : This function will invoke all provision complete function in
 *           IP_SERVICE group.
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
static void IP_SERVICE_GROUP_ProvisionComplete(void)
{
#if (SYS_CPNT_DHCP == TRUE)
    DHCP_MGR_ProvisionComplete();
#endif
#if (SYS_CPNT_DHCPV6 == TRUE)
    DHCPv6_MGR_ProvisionComplete();
#endif

    return;
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - IP_SERVICE_GROUP_HandleSysCallbackIPCMsg
 *-----------------------------------------------------------------------------
 * PURPOSE : This function will handle all callbacks from IPC messages in
 *           IP_SERVICE group.
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
static void IP_SERVICE_GROUP_HandleSysCallbackIPCMsg(SYSFUN_Msg_T *msgbuf_p)
{
    SYS_CALLBACK_MGR_Msg_T *sys_cbmsg_p =
        (SYS_CALLBACK_MGR_Msg_T*)msgbuf_p->msg_buf;

    switch (sys_cbmsg_p->callback_event_id)
    {
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_BOOTP_PACKET:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                IP_SERVICE_GROUP_ReceiveBootpPacketCallbackHandler);
			break;

         /* L3 Interface Delete */
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_NETCFG_L3IF_DESTROY:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &IP_SERVICE_GROUP_IfDeleteCallbackHandler);
            break;          
            
#if (SYS_CPNT_DHCP == TRUE)
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_DHCP_RESTART3:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                DHCP_MGR_Restart3);
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_DHCP_SET_IF_ROLE:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                DHCP_MGR_SetIfRole);
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_DHCP_SET_IF_STATUS:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, DHCP_MGR_SetIfStatus);
            break;
#endif                    

#if (SYS_CPNT_DHCPV6 == TRUE)
         /* Manual set global IP  */
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_NETCFG_IPV6_SET_MANUAL_ADDR:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &IP_SERVICE_GROUP_SignalAddrModeChangedCallbackHandler);
            break;

        /* Autoconfig */
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_NETCFG_IPV6_ADDRAUTOCONFIG:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &IP_SERVICE_GROUP_SignalAddrModeChangedCallbackHandler);
            break;
            
        /* L3 Interface Create */
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_NETCFG_L3IF_CREATE:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &IP_SERVICE_GROUP_IfCreateCallbackHandler);
            break;
            
        /* L3 Interface Oper Status UP */
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_NETCFG_L3IF_OPER_STATUS_UP:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &IP_SERVICE_GROUP_IfOperStatusUpCallbackHandler);
            break;
            
        /* L3 Interface Oper Status Down */
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_NETCFG_L3IF_OPER_STATUS_DOWN:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &IP_SERVICE_GROUP_IfOperStatusDownCallbackHandler);
            break;
#endif


        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RIF_ACTIVE:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &IP_SERVICE_GROUP_RifActiveCallbackHandler);
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RIF_DESTROYED:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &IP_SERVICE_GROUP_RifDestroyCallbackHandler);
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RIF_CREATED:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p, &IP_SERVICE_GROUP_RifCreateCallbackHandler);
            break;

        default:
            SYSFUN_Debug_Printf(
                "\n%s: received callback_event that is not handled(%d)\n",
                __FUNCTION__, sys_cbmsg_p->callback_event_id);
    }
} /* End of IP_SERVICE_GROUP_HandleSysCallbackIPCMsg */

static void IP_SERVICE_GROUP_IfDeleteCallbackHandler(UI32_T vid_ifIndex)
{
#if (SYS_CPNT_DHCP == TRUE)
    DHCP_MGR_Destroy_If(vid_ifIndex);
#endif

#if (SYS_CPNT_DHCPV6 == TRUE)
    DHCPv6_MGR_IfDelete(vid_ifIndex);
#endif
}

#if (SYS_CPNT_DHCPV6 == TRUE)

static UI32_T IP_SERVICE_GROUP_SignalAddrModeChangedCallbackHandler(UI32_T vid_ifIndex, BOOL_T auto_conf_status)
{
    /* notify by CMGR */
#if 0
    DHCPv6_MGR_SignalAddrModeChanged(vid_ifIndex, auto_conf_status);
#endif
    return 0;
}

static void IP_SERVICE_GROUP_IfCreateCallbackHandler(UI32_T vid_ifIndex, BOOL_T auto_conf_status)
{
    if (auto_conf_status == TRUE)
        DHCPv6_MGR_IfCreate(vid_ifIndex, DHCPv6_TYPE_INTERFACE_MODE_AUTO);
    else
        DHCPv6_MGR_IfCreate(vid_ifIndex, DHCPv6_TYPE_INTERFACE_MODE_USER_DEFINE);
}

static void IP_SERVICE_GROUP_IfOperStatusUpCallbackHandler(UI32_T vid_ifIndex, BOOL_T auto_conf_status)
{
    DHCPv6_MGR_IfOperStatusUp(vid_ifIndex, auto_conf_status);
}

static void IP_SERVICE_GROUP_IfOperStatusDownCallbackHandler(UI32_T vid_ifIndex)
{
    DHCPv6_MGR_IfOperStatusDown(vid_ifIndex);
}
#endif


static void IP_SERVICE_GROUP_RifDestroyCallbackHandler(UI32_T vid_ifindex, L_INET_AddrIp_T *addr)
{

#if (SYS_CPNT_CRAFT_PORT == TRUE)
    if (vid_ifindex == SYS_ADPT_CRAFT_INTERFACE_IFINDEX)
    {
        return;
    }
#endif

    DHCP_MGR_SignalRifDestroy(vid_ifindex,addr);

#if (SYS_CPNT_DHCPV6_RELAY == TRUE)    
    DHCPv6_MGR_SignalRifChange(vid_ifindex);
#endif

}

static void IP_SERVICE_GROUP_RifCreateCallbackHandler(UI32_T vid_ifindex, L_INET_AddrIp_T *addr)
{

#if (SYS_CPNT_CRAFT_PORT == TRUE)
    if (vid_ifindex == SYS_ADPT_CRAFT_INTERFACE_IFINDEX)
    {
        return;
    }
#endif

#if (SYS_CPNT_DHCPV6_RELAY == TRUE)
    DHCPv6_MGR_SignalRifChange(vid_ifindex);
#endif

}

static void IP_SERVICE_GROUP_RifActiveCallbackHandler(UI32_T vid_ifindex, L_INET_AddrIp_T *addr)
{

    DHCP_MGR_SignalRifUp(vid_ifindex, addr);

#if (SYS_CPNT_DHCPV6_RELAY == TRUE)
    DHCPv6_MGR_SignalRifUp(vid_ifindex);
#endif

}


/*-----------------------------------------------------------------------------
 * FUNCTION NAME - IP_SERVICE_GROUP_ReceiveBootpPacketCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the callback event happening when a bootp packet is
 *           received.
 *
 * INPUT   : mref_handle_p     --
 *           packet_length     --
 *           rxRifNum          --
 *           dst_mac           --
 *           src_mac           --
 *           vid               --
 *           src_lport_ifIndex --
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
static void IP_SERVICE_GROUP_ReceiveBootpPacketCallbackHandler(
                L_MM_Mref_Handle_T *mref_handle_p,
                UI32_T packet_length,
                UI32_T ifindex,
                UI8_T *dst_mac,
                UI8_T *src_mac,
                UI32_T vid,
                UI32_T src_lport_ifIndex)
{
#if (SYS_CPNT_DHCP == TRUE)
    DHCP_MGR_do_packet(mref_handle_p, packet_length, ifindex,
                       dst_mac, src_mac, vid, src_lport_ifIndex);
#else
    L_MM_Mref_Release(&mref_handle_p);
#endif
}


