/*-----------------------------------------------------------------------------
 * MODULE NAME: SYS_CALLBACK_GROUP.C
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *    Implements the APIs of SYS_CALLBACK group.
 *
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    2013/05/21     --- Jimi, Create
 *
 * Copyright(C)      EdgeCore Corporation, 2013
 *-----------------------------------------------------------------------------
 */


/* INCLUDE FILE DECLARATIONS
 */

#include "sys_bld.h"
#include "sys_module.h"
#include "sys_type.h"
#include "sys_cpnt.h"
#include "l_threadgrp.h"
#include "l_ipcmem.h"
#include "sysfun.h"
#include "backdoor_mgr.h"
#include "sys_callback_mgr.h"
#include "sys_callback_proc_comm.h"
#include "swctrl_pom.h"
#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
#include "sw_watchdog_mgr.h"
#endif
#if (SYS_CPNT_DHCPSNP == TRUE)
#include "dhcpsnp_pmgr.h"
#endif
#if (SYS_CPNT_IP_SOURCE_GUARD == TRUE)
#include "ipsg_pom.h"
#endif
#if (SYS_CPNT_DHCP_RELAY_OPTION82 == TRUE)
#include "dhcp_pom.h"
#endif
#if (SYS_CPNT_PORT_SECURITY == TRUE)
#include "psec_pom.h"
#endif
#include "amtr_pmgr.h"

#if (SYS_CPNT_NETACCESS == TRUE)
#include "netaccess_pmgr.h"
#endif

#if (SYS_CPNT_IPSG_MAC_MODE == TRUE)
#include "ipsg_pmgr.h"
#include "amtrdrv_pom.h"
#endif

/* NAMING CONSTANT DECLARATIONS
 */

/* the size of the ipc msg buf for the MGR thread of SYS_CALLBACK group to receive and
 * response PMGR ipc. The size of this buffer should pick the maximum of size
 * required for PMGR ipc request and response in all CSCs handled in this
 * process. (this size does not include IPC msg header)
 */
#define SYS_CALLBACK_GROUP_PMGR_MSGBUF_MAX_REQUIRED_SIZE sizeof(SYS_CALLBACK_GROUP_MgrMsg_T)


/* MACRO FUNCTION DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */
typedef union {
    /* need add sys_callback msg */
    SYS_CALLBACK_MGR_L2muxReceivePacket_CBData_T	sys_callback_mgr_l2mux_rx_snoop_dhcp_pkt_cbdata;
    SYS_CALLBACK_MGR_AuthenticatePacket_CBData_T    sys_callback_mgr_authenticate_pkt_cbdata;
} SYS_CALLBACK_GROUP_Syscallback_CBData_T;

typedef struct
{
    UI8_T buffer[SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(SYS_CALLBACK_GROUP_Syscallback_CBData_T))];
} SYS_CALLBACK_GROUP_Syscallback_Msg_T;

/* union all data type used for SYS_CALLBACK group MGR IPC message to get the maximum
 * required ipc message buffer
 */
typedef union SYS_CALLBACK_GROUP_MgrMsg_U
{
    BACKDOOR_MGR_Msg_T                              backdoor_mgr_ipcmsg;
    SYS_CALLBACK_GROUP_Syscallback_Msg_T            sys_callback_ipcmsg;
} SYS_CALLBACK_GROUP_MgrMsg_T;


/* LOCAL SUBPROGRAM DECLARATIONS
 */

static void SYS_CALLBACK_GROUP_Mgr_Thread_Function_Entry(void* arg);
static void SYS_CALLBACK_GROUP_SetTransitionMode(void);
static void SYS_CALLBACK_GROUP_EnterTransitionMode(void);
static void SYS_CALLBACK_GROUP_EnterMasterMode(void);
static void SYS_CALLBACK_GROUP_EnterSlaveMode(void);
static void SYS_CALLBACK_GROUP_HandleSysCallbackIPCMsg(SYSFUN_Msg_T *msgbuf_p);
static void SYS_CALLBACK_GROUP_ProvisionComplete(void);
static void SYS_CALLBACK(SYSFUN_Msg_T *msgbuf_p);
static void SYS_CALLBACK_GROUP_RxSnoopDhcpPacketCallbackHandler(
    L_MM_Mref_Handle_T *mref_handle_p,
    UI8_T   dst_mac[SYS_ADPT_MAC_ADDR_LEN],
    UI8_T   src_mac[SYS_ADPT_MAC_ADDR_LEN],
    UI16_T  tag_info,
    UI16_T  ether_type,
    UI32_T  pkt_len,
    UI32_T  ing_lport
);

static void SYS_CALLBACK_GROUP_AuthenticatePacketCallbackHandler(
    L_MM_Mref_Handle_T *mref_handle_p,
    UI8_T   dst_mac[SYS_ADPT_MAC_ADDR_LEN],
    UI8_T   src_mac[SYS_ADPT_MAC_ADDR_LEN],
    UI16_T  tag_info,
    UI16_T  ether_type,
    UI32_T  pkt_len,
    UI32_T  src_unit,
    UI32_T  src_port,
    SYS_CALLBACK_MGR_AuthenticatePacket_Result_T    auth_result,
    void    *cookie
);


/* STATIC VARIABLE DEFINITIONS
 */

/* the buffer for retrieving ipc request for sys_callbackgrp mgr thread
 */
static UI8_T mgrtd_ipc_buf[SYSFUN_SIZE_OF_MSG(SYS_CALLBACK_GROUP_PMGR_MSGBUF_MAX_REQUIRED_SIZE)];


/* EXPORTED SUBPROGRAM BODIES
 */

/* ------------------------------------------------------------------------
 * FUNCTION NAME : SYS_CALLBACK_GROUP_InitiateProcessResources
 * ------------------------------------------------------------------------ 
 * PURPOSE: Initiate process resource for SYS_CALLBACK group.
 * INPUT  : none
 * OUTPUT : none
 * RETURN : none
 * NOTES  : none
 * ------------------------------------------------------------------------ 
 */
void SYS_CALLBACK_GROUP_InitiateProcessResources(void)
{
    return;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME : SYS_CALLBACK_GROUP_Create_InterCSC_Relation
 * ------------------------------------------------------------------------ 
 * PURPOSE: Create inter-CSC relationships for SYS_CALLBACK group.
 * INPUT  : none
 * OUTPUT : none
 * RETURN : none
 * NOTES  : none
 * ------------------------------------------------------------------------ 
 */
void SYS_CALLBACK_GROUP_Create_InterCSC_Relation(void)
{
    return;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME : SYS_CALLBACK_GROUP_Create_All_Threads
 * ------------------------------------------------------------------------ 
 * PURPOSE: This function will spawn all threads in SYS_CALLBACK group.
 * INPUT  : none
 * OUTPUT : none
 * RETURN : none
 * NOTES  : All threads in the same CSC group will join the same thread group.
 * ------------------------------------------------------------------------ 
 */
void SYS_CALLBACK_GROUP_Create_All_Threads(void)
{
    UI32_T thread_id;

    if (SYSFUN_SpawnThread(SYS_BLD_SYS_CALLBACK_GROUP_MGR_THREAD_PRIORITY,    
                           SYS_BLD_SYS_CALLBACK_GROUP_MGR_SCHED_POLICY,
                           SYS_BLD_SYS_CALLBACK_GROUP_MGR_THREAD_NAME,
                           SYS_BLD_TASK_COMMON_STACK_SIZE,
                           SYSFUN_TASK_FP,
                           SYS_CALLBACK_GROUP_Mgr_Thread_Function_Entry,
                           NULL,
                           &thread_id) != SYSFUN_OK)
    {
        printf("\n%s: Spawn SYS_CALLBACK Group MGR thread fail.\n", __FUNCTION__);
    }

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
    SW_WATCHDOG_MGR_RegisterMonitorThread(SW_WATCHDOG_SYS_CALLBACK_GROUP, thread_id, SYS_ADPT_SYS_CALLBACK_GROUP_SW_WATCHDOG_TIMER);
#endif
    return;
} /* End of SYS_CALLBACK_GROUP_Create_All_Threads */


/* LOCAL SUBPROGRAM BODIES
 */

/* ------------------------------------------------------------------------
 * FUNCTION NAME : SYS_CALLBACK_GROUP_Mgr_Thread_Function_Entry
 * ------------------------------------------------------------------------ 
 * PURPOSE: This is the entry function for the MGR thread of SYS_CALLBACK group.
 * INPUT  : none
 * OUTPUT : none
 * RETURN : none
 * NOTES  : none
 * ------------------------------------------------------------------------ 
 */
static void SYS_CALLBACK_GROUP_Mgr_Thread_Function_Entry(void* arg)
{
    SYSFUN_MsgQ_T           ipc_msgq_handle;
    L_THREADGRP_Handle_T    tg_handle = SYS_CALLBACK_PROC_COMM_GetSyscallbackGroupTGHandle();
    UI32_T                  member_id, received_events, local_events = SYSFUN_SYSTEM_EVENT_IPCMSG;
    SYSFUN_Msg_T            *msgbuf_p = (SYSFUN_Msg_T*)mgrtd_ipc_buf;
    BOOL_T                  need_resp;
    UI32_T                  all_events =
                                SYSFUN_SYSTEM_EVENT_SET_TRANSITION_MODE |
                                SYSFUN_SYSTEM_EVENT_IPCMSG |
#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
                                SYSFUN_SYSTEM_EVENT_SW_WATCHDOG |
#endif
                                SYSFUN_SYSTEM_EVENT_IPCFAIL;


    /* join the thread group of SYS_CALLBACK Group
     */
    if (L_THREADGRP_Join(tg_handle, SYS_BLD_SYS_CALLBACK_GROUP_MGR_THREAD_PRIORITY,
            &member_id) == FALSE)
    {
        printf("\n%s: L_THREADGRP_Join fail.\n", __FUNCTION__);
        return;
    }


    /* create mgr ipc message queue handle
     */
    if (SYSFUN_CreateMsgQ(SYS_BLD_SYS_CALLBACK_GROUP_IPCMSGQ_KEY,
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

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
        if (local_events & SYSFUN_SYSTEM_EVENT_SW_WATCHDOG)
        {
            SW_WATCHDOG_MGR_ResetTimer(SW_WATCHDOG_SYS_CALLBACK_GROUP);
            local_events ^= SYSFUN_SYSTEM_EVENT_SW_WATCHDOG;
        }
#endif

        if (local_events & SYSFUN_SYSTEM_EVENT_SET_TRANSITION_MODE)
        {
        
            SYS_CALLBACK_GROUP_SetTransitionMode();
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
                    SYS_CALLBACK_GROUP_PMGR_MSGBUF_MAX_REQUIRED_SIZE, msgbuf_p) == SYSFUN_OK)
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
                        SYS_CALLBACK_GROUP_EnterTransitionMode();

                        /* SYS_TYPE_CMD_ENTER_TRANSITION_MODE
                         * need a response which contains nothing
                         */
                        msgbuf_p->msg_size = 0;
                        need_resp = TRUE;
                        break;

                    case SYS_TYPE_CMD_ENTER_MASTER_MODE:
                        SYS_CALLBACK_GROUP_EnterMasterMode();

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
                        SYS_CALLBACK_GROUP_EnterSlaveMode();

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
                        SYS_CALLBACK_GROUP_ProvisionComplete();
                        msgbuf_p->msg_size = 0;
                        need_resp = TRUE;
                        break;

                    /* module id cmd
                     */
                    case SYS_MODULE_BACKDOOR:
                        need_resp = BACKDOOR_MGR_HandleIPCMsg(msgbuf_p);
                        break;

                    case SYS_MODULE_SYS_CALLBACK:
                        SYS_CALLBACK_GROUP_HandleSysCallbackIPCMsg(msgbuf_p);
                        /* SYS_CALLBACK ipc message can only be uni-direction
                         * just set need_resp as FALSE
                         */
                        need_resp = FALSE;
                        break;

                    case SYS_TYPE_CMD_RELOAD_SYSTEM:

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
} /* End of SYS_CALLBACK_GROUP_Mgr_Thread_Function_Entry */

/* ------------------------------------------------------------------------
 * FUNCTION NAME : SYS_CALLBACK_GROUP_SetTransitionMode
 * ------------------------------------------------------------------------ 
 * PURPOSE: This function will invoke all set transition mode function in
 *          SYS_CALLBACK group.
 * INPUT  : none
 * OUTPUT : none
 * RETURN : none
 * NOTES  : none
 * ------------------------------------------------------------------------ 
 */
static void SYS_CALLBACK_GROUP_SetTransitionMode(void)
{
    return;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME : SYS_CALLBACK_GROUP_EnterTransitionMode
 * ------------------------------------------------------------------------ 
 * PURPOSE: This function will invoke all enter transition mode function in
 *          SYS_CALLBACK group.
 * INPUT  : none
 * OUTPUT : none
 * RETURN : none
 * NOTES  : none
 * ------------------------------------------------------------------------ 
 */ 
static void SYS_CALLBACK_GROUP_EnterTransitionMode(void)
{
    return;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME : SYS_CALLBACK_GROUP_EnterMasterMode
 * ------------------------------------------------------------------------ 
 * PURPOSE: This function will invoke all set master mode function in
 *          SYS_CALLBACK group.
 * INPUT  : none
 * OUTPUT : none
 * RETURN : none
 * NOTES  : none
 * ------------------------------------------------------------------------ 
 */  
static void SYS_CALLBACK_GROUP_EnterMasterMode(void)
{
    return;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME : SYS_CALLBACK_GROUP_EnterSlaveMode
 * ------------------------------------------------------------------------ 
 * PURPOSE: This function will invoke all enter slave mode function in
 *          SYS_CALLBACK group.
 * INPUT  : none
 * OUTPUT : none
 * RETURN : none
 * NOTES  : none
 * ------------------------------------------------------------------------ 
 */   
static void SYS_CALLBACK_GROUP_EnterSlaveMode(void)
{
    return;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME : SYS_CALLBACK_GROUP_ProvisionComplete
 * ------------------------------------------------------------------------ 
 * PURPOSE: This function will invoke all provision complete function in
 *          SYS_CALLBACK group.
 * INPUT  : none
 * OUTPUT : none
 * RETURN : none
 * NOTES  : none
 * ------------------------------------------------------------------------ 
 */   
static void SYS_CALLBACK_GROUP_ProvisionComplete(void)
{
    return;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME : SYS_CALLBACK_GROUP_HandleSysCallbackIPCMsg
 * ------------------------------------------------------------------------ 
 * PURPOSE: This function will handle all callbacks from IPC messages in
 *          SYS_CALLBACK group.
 * INPUT  : msgbuf_p -- SYS_CALLBACK IPC message
 * OUTPUT : none
 * RETURN : none
 * NOTES  : none
 * ------------------------------------------------------------------------ 
 */ 
static void SYS_CALLBACK_GROUP_HandleSysCallbackIPCMsg(SYSFUN_Msg_T *msgbuf_p)
{
    SYS_CALLBACK_MGR_Msg_T *sys_cbmsg_p =
        (SYS_CALLBACK_MGR_Msg_T*)msgbuf_p->msg_buf;

    switch (sys_cbmsg_p->callback_event_id)
    { 
    	
    	case SYS_CALLBACK_MGR_CALLBACKEVENTID_RX_SNOOP_DHCP_PACKET:
			SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
				&SYS_CALLBACK_GROUP_RxSnoopDhcpPacketCallbackHandler);
			break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_AUTHENTICATE_PACKET:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                &SYS_CALLBACK_GROUP_AuthenticatePacketCallbackHandler);
            break;

        default:
            SYSFUN_Debug_Printf(
                "\n%s: received callback_event that is not handled(%d)\n",
                __FUNCTION__, sys_cbmsg_p->callback_event_id);
    }
} /* End of SYS_CALLBACK_GROUP_HandleSysCallbackIPCMsg */


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_CALLBACK_GROUP_RxSnoopDhcpPacketCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, to authenticate a packet.
 * INPUT   : mref_handle_p  --  L_MM_Mref descriptor
 *           dst_mac        --  destination mac address
 *           src_mac        --  source mac address
 *           tag_info       --  vlan tag information
 *			 ether_type		--	ethernet type
 *           pkt_len        --  packet length
 *           ing_lport      --  ingress logical port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : Handle DHCP packet between DHCPSNP and DHCP relay/client/server
 * -------------------------------------------------------------------------*/
static void SYS_CALLBACK_GROUP_RxSnoopDhcpPacketCallbackHandler(
    L_MM_Mref_Handle_T *mref_handle_p,
    UI8_T   dst_mac[SYS_ADPT_MAC_ADDR_LEN],
    UI8_T   src_mac[SYS_ADPT_MAC_ADDR_LEN],
    UI16_T  tag_info,
    UI16_T  ether_type,
    UI32_T  pkt_len,
    UI32_T  ing_lport)
{	
    L_MM_Mref_Handle_T *new_mref_p=mref_handle_p;
    UI32_T              new_pkt_len=pkt_len;
    BOOL_T              annouce=TRUE;
#if (SYS_CPNT_DHCPSNP == TRUE)
    {
        UI32_T ing_vid = tag_info & 0xfff;
        
        /* for DHCPSNP processing
         */
        annouce = DHCPSNP_PMGR_RxProcessPacket(
                    mref_handle_p,
                    pkt_len,
                    dst_mac,
                    src_mac,
                    ing_vid,
                    ing_lport,
                    &new_mref_p);
    }
#endif /* SYS_CPNT_DHCPSNP */

    if(annouce)
    {   
        /* if new allocated mref existed, use it;
         * otherwise, use original mref to annouce
         */
        if(NULL!=new_mref_p)
        {
            L_MM_Mref_Release(&mref_handle_p);
            L_MM_Mref_GetPdu(new_mref_p,&new_pkt_len);
    	}
        else
        {	
            new_mref_p = mref_handle_p;
            new_pkt_len = pkt_len;
        }
    }
    else
    {   
        /* don't annouce packet, free mref
         */
        if(NULL!=new_mref_p)
        {	
            L_MM_Mref_Release(&new_mref_p);
        }

        L_MM_Mref_Release(&mref_handle_p);
        return;
    }

    /* callback to L2MUX_GROUP for upper layer packet process(client/relay/server)
     */
    SYS_CALLBACK_MGR_RxDhcpPacket(
    	SYS_MODULE_SYS_CALLBACK,
    	new_mref_p,
    	dst_mac,
    	src_mac,
    	tag_info,
    	ether_type,
    	new_pkt_len,
    	ing_lport);

}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_CALLBACK_GROUP_AuthenticatePacketCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, to authenticate a packet.
 * INPUT   : mref_handle_p  - MREF handle for packet
 *           dst_mac        - destination mac address
 *           src_mac        - source mac address
 *           tag_info       - vlan tag infomation
 *           ether_type     - ethernet type
 *           pkt_len        - packet length
 *           src_unit       - source unit
 *           src_port       - source port
 *           auth_result    - last auth result
 *           cookie         - shall be passed to next CSC via
 *                            SYS_CALLBACK_MGR_AuthenticatePacket_AsyncReturn
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
static void SYS_CALLBACK_GROUP_AuthenticatePacketCallbackHandler(
    L_MM_Mref_Handle_T *mref_handle_p,
    UI8_T   dst_mac[SYS_ADPT_MAC_ADDR_LEN],
    UI8_T   src_mac[SYS_ADPT_MAC_ADDR_LEN],
    UI16_T  tag_info,
    UI16_T  ether_type,
    UI32_T  pkt_len,
    UI32_T  src_unit,
    UI32_T  src_port,
    SYS_CALLBACK_MGR_AuthenticatePacket_Result_T    auth_result,
    void    *cookie)
{
    SYS_CALLBACK_MGR_AuthenticatePacket_CBData_T *org_msg_p = cookie;
    SYS_CALLBACK_MGR_AuthenticatePacket_Result_T auth_ret;
    UI32_T  src_lport=0;
    UI32_T  learn_cnt=0;
    UI32_T  vid = 0;

    SWCTRL_POM_UserPortToLogicalPort(src_unit, src_port, &src_lport);
    vid = tag_info & 0xfff;

#if (SYS_CPNT_NETACCESS == TRUE)
    /* if the flag is off, means it's first time enter authentication process
     */
    if (0 == org_msg_p->flag)
    {
        /* check NETACCESS validation by asynchronous call
         */
        NETACCESS_PMGR_AuthenticatePacket(cookie);
        return;
    }
#endif /* end of #if (SYS_CPNT_NETACCESS == TRUE) */

#if (SYS_CPNT_IPSG_MAC_MODE == TRUE)
    /* check IPSG validation
     */
    auth_ret = IPSG_PMGR_AuthenticatePacket(mref_handle_p,src_mac, vid, src_lport);

    if(auth_ret == SYS_CALLBACK_MGR_AUTH_FAILED)
    {
        goto free_mref;
    }

    if(auth_result < auth_ret)
    {
        auth_result = auth_ret;
    }

    learn_cnt = AMTRDRV_OM_GetSecurityCounterByport(src_lport);

    /* check port maximum count
     */
    {
        UI32_T limit=0;
        UI8_T  filter_mode=0;
        UI8_T  table_mode=0;

        if(IPSG_TYPE_OK != IPSG_POM_GetPortFilterMode(src_lport, &filter_mode))
        {
            goto free_mref;
        }

        if(IPSG_TYPE_FILTER_MODE_DISABLED != filter_mode)
        {
            if((IPSG_TYPE_OK != IPSG_POM_GetPortFilterMode(src_lport, &table_mode))||
               (IPSG_TYPE_OK != IPSG_POM_GetPortEntryLimit (src_lport, table_mode, &limit)))
            {
                goto free_mref;
            }

            if((IPSG_TYPE_TABLE_MODE_MAC == table_mode)&&
               (learn_cnt >= limit))
            {
                goto free_mref;
            }
        }
    }
#endif /* end of #if (SYS_CPNT_IPSG_MAC_MODE == TRUE) */

#if (SYS_CPNT_PORT_SECURITY == TRUE)
    {
        UI32_T max_count=0;
        UI32_T port_security_enabled_by_who=0;

        if ((TRUE == SWCTRL_POM_IsSecurityPort(src_lport, &port_security_enabled_by_who))&&
            (SWCTRL_PORT_SECURITY_ENABLED_BY_PSEC == port_security_enabled_by_who))
        {
            if((FALSE == PSEC_POM_GetMaxMacCount(src_lport, &max_count))||
               (learn_cnt >= max_count))
            {
                goto free_mref;
            }
        }
    }
#endif  /* end of #if (SYS_CPNT_PORT_SECURITY == TRUE) */

    /* AMTR learn mac address
     */
    if(FALSE == AMTR_PMGR_AuthenticatePacket(src_mac, vid, src_lport, auth_result))
    {
        goto free_mref;
    }

    /* sys_callback to LAN to dispatch packet
     */
    SYS_CALLBACK_MGR_AuthenticationDispatchPacketCallback(
        SYS_MODULE_SYS_CALLBACK,
        L_IPCMEM_GetPtr(org_msg_p->lan_cbdata.mref_handle_offset),
        org_msg_p->lan_cbdata.dst_mac,
        org_msg_p->lan_cbdata.src_mac,
        org_msg_p->lan_cbdata.tag_info,
        org_msg_p->lan_cbdata.type,
        org_msg_p->lan_cbdata.pkt_length,
        org_msg_p->lan_cbdata.src_unit,
        org_msg_p->lan_cbdata.src_port,
        org_msg_p->lan_cbdata.packet_class);

    /* cookie buffer is allocated when NETACESS asynchonrous return, we should free it
     */
    L_MM_Free(cookie);
    return;

    /* drop packet and free mref
     */
free_mref:
    L_MM_Free(cookie);
    L_MM_Mref_Release(&mref_handle_p);
    return;
}


