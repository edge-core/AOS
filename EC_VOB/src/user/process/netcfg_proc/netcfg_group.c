/* MODULE NAME:  netcfg_group.c
 * PURPOSE:
 *     This file handles the related work for NETCFG csc group.
 *
 * NOTES:
 *
 * HISTORY
 *    06/12/2007 - Charlie Chen , Created
 *    07/10/2007 - Max Chen     , Modified for porting NETCFG
 *
 * Copyright(C)      Accton Corporation, 2007
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "sysfun.h"
#include "sys_adpt.h"
#include "sys_cpnt.h"
#include "sys_bld.h"
#include "sys_module.h"
#include "l_threadgrp.h"
#include "sys_callback_mgr.h"

#include "netcfg_group.h"
#include "netcfg_proc_comm.h"
#include "backdoor_mgr.h"
#include "netcfg_mgr_main.h"
#include "netcfg_mgr_route.h"
#include "netcfg_mgr_ip.h"
#include "netcfg_mgr_nd.h"

#if (SYS_CPNT_RIP == TRUE)
#include "rip_pmgr.h"
#include "netcfg_mgr_rip.h"/*Lin.Li, for RIP porting*/
#endif
#if (SYS_CPNT_OSPF == TRUE)
#include "netcfg_mgr_ospf.h"/*Lin.Li, for OSPF porting*/
#include "ospf_pmgr.h"
#endif

/* added by steven.gao for OSPFv3 */
#if (SYS_CPNT_OSPF6 == TRUE)
#include "ospf6_pmgr.h"
#endif
#if (SYS_CPNT_BGP == TRUE)
#include "bgp_pmgr.h"
#endif

#if (SYS_CPNT_VRRP  == TRUE )
#include "vrrp_pmgr.h"
#endif

/*For VLAN signal */
#include "leaf_2863.h"

/*For PMGR/POM used in NETCFG Group */
#include "amtr_pmgr.h"
#include "amtr_pom.h"
#include "vlan_pmgr.h"
#include "vlan_pom.h"
#if (SYS_CPNT_NSM == TRUE)
#include "nsm_pmgr.h"
#endif
#include "stktplg_pom.h"
#include "iml_pmgr.h"
#if (SYS_CPNT_LLDP == TRUE)
#include "lldp_pmgr.h"
#endif
#include "dhcp_pmgr.h"
#if (SYS_CPNT_DHCPV6 == TRUE)
#include "dhcpv6_pmgr.h"
#endif
#if (SYS_CPNT_AMTRL3 == TRUE)
#include "amtrl3_pmgr.h"/*Lin.Li, for ARP porting*/
#include "amtrl3_pom.h"/*Lin.Li, for ARP porting*/
#endif
#include "ipal_types.h"
#include "ipal_debug.h" /* for ipal backdoor */
#include "ipal_kernel.h"
#include "ipal_reflect.h"

#if (SYS_CPNT_IPV6_RA_GUARD == TRUE)
#include "trk_pmgr.h"
#include "swctrl_pom.h"
#include "l2mux_pmgr.h"
#endif

#if (SYS_CPNT_PBR == TRUE)
#include "netcfg_mgr_pbr.h"
#endif

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
#include "sw_watchdog_mgr.h"
#endif

#include "dev_swdrvl3_pmgr.h"
#include "dev_nicdrv_pmgr.h"
#include "l4_pmgr.h"
#include "swctrl_pmgr.h"

#if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE)
#include "cmgr.h"
#endif

/* NAMING CONSTANT DECLARATIONS
 */
/* the size of the ipc msg buf for CSCGroup2 MGR thread to receive and response
 * PMGR ipc. The size of this buffer should pick the maximum of size required
 * for PMGR ipc request and response in all CSCs handled in this process.
 * (this size does not include IPC msg header)
 */
#define NETCFG_PMGR_MSGBUF_MAX_REQUIRED_SIZE sizeof(NETCFG_GROUP_MgrMsg_T)


/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
/* union all data type used for MGR IPC message to get the maximum required
 * ipc message buffer
 */

typedef union NETCFG_GROUP_MgrMsg_U
{
    /* Union the SYS CALLBACK Message size used in NETCFG Group for
     * calculating the maximum buffer size for receiving message
     * from Queue
     */
    struct
    {
        SYS_CALLBACK_MGR_Msg_T sys_callback_msg_header;
        union
        {
            SYS_CALLBACK_MGR_VlanDestroy_CBData_T   vlan_destroy_data;
            SYS_CALLBACK_MGR_IfOperStatusChanged_CBData_T   if_oper_status_data;
        } sys_callback_msg_data;
    } sys_callback_msg;

    NETCFG_MGR_MAIN_IPCMsg_T        netcfg_mgr_main_ipcmsg;
    NETCFG_MGR_ROUTE_IPCMsg_T       netcfg_mgr_route_ipcmsg;
    NETCFG_MGR_IP_IPCMsg_T          netcfg_mgr_ip_ipcmsg;
    BACKDOOR_MGR_Msg_T              backdoor_mgr_ipcmsg;
    NETCFG_MGR_ND_IPCMsg_T          netcfg_mgr_nd_ipcmsg;
#if (SYS_CPNT_RIP == TRUE)
    NETCFG_MGR_RIP_IPCMsg_T         netcfg_mgr_rip_ipcmsg;
#endif
#if (SYS_CPNT_OSPF == TRUE)
    NETCFG_MGR_OSPF_IPCMsg_T        netcfg_mgr_ospf_ipcmsg;
#endif
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    SYS_TYPE_HandleHotSwapArg_T     HandleHotInsertionArg_ipcmsg;
#endif
#if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE)
    CMGR_IpcMsg_T                   cmgr_ipcmsg;
#endif
} NETCFG_GROUP_MgrMsg_T;

/* LOCAL SUBPROGRAM DECLARATIONS
 */
/* the buffer for retrieving ipc request for main thread
 */
static UI8_T netcfg_mgrtd_ipc_buf[SYSFUN_SIZE_OF_MSG(NETCFG_PMGR_MSGBUF_MAX_REQUIRED_SIZE)];

/* STATIC VARIABLE DECLARATIONS
 */
static void   NETCFG_GROUP_Mgr_Thread_Function_Entry(void* arg);
static void   NETCFG_GROUP_EnterTransitionMode(void);
static void   NETCFG_GROUP_SetTransitionMode(void);
static void   NETCFG_GROUP_EnterMasterMode(void);
static void   NETCFG_GROUP_EnterSlaveMode(void);
static void   NETCFG_GROUP_ProvisionComplete(void);
static void   NETCFG_GROUP_HandleSysCallbackIPCMsg(SYSFUN_Msg_T *msgbuf_p);
static void   NETCFG_GROUP_VlanDestroyCallbackHandler(UI32_T vid_ifindex, UI32_T vlan_status);
static void   NETCFG_GROUP_IfOperStatusChangedCallbackHandler(UI32_T vid_ifindex, UI32_T oper_status);
static void NECTCFG_GROUP_NsmRouteChangeCallbackHandler(UI32_T address_family);

#if (SYS_CPNT_IPV6_RA_GUARD == TRUE)
static void NECTCFG_GROUP_TrunkMemberAdd1stCallbackHandler(
    UI32_T  trunk_ifindex,
    UI32_T  member_ifindex);
static void NECTCFG_GROUP_TrunkMemberAddCallbackHandler(
    UI32_T  trunk_ifindex,
    UI32_T  member_ifindex);
static void NECTCFG_GROUP_TrunkMemberDeleteCallbackHandler(
    UI32_T  trunk_ifindex,
    UI32_T  member_ifindex);
static void NECTCFG_GROUP_TrunkMemberDeleteLstCallbackHandler(
    UI32_T  trunk_ifindex,
    UI32_T  member_ifindex);

#if (SYS_CPNT_IPV6_RA_GUARD_SW_RELAY == TRUE)
static void NECTCFG_GROUP_RaGuardPacketCallbackHandler(
    L_MM_Mref_Handle_T  *mref_handle_p,
    UI8_T               dst_mac[SYS_ADPT_MAC_ADDR_LEN],
    UI8_T               src_mac[SYS_ADPT_MAC_ADDR_LEN],
    UI16_T              ingress_vid,
    UI8_T               ing_cos,
    UI8_T               pkt_type,
    UI32_T              pkt_length,
    UI32_T              src_lport);
#endif /* #if (SYS_CPNT_IPV6_RA_GUARD_SW_RELAY == TRUE) */
#endif /* #if (SYS_CPNT_IPV6_RA_GUARD == TRUE) */

#if (SYS_CPNT_PBR == TRUE)
static void NETCFG_GROUP_HostRouteChangedCallBackHandler(L_INET_AddrIp_T *addr_p, BOOL_T is_unresolved);
static void NETCFG_GROUP_AclChangedCallBackHandler(UI32_T acl_index, char *acl_name, UI8_T type);
static void NETCFG_GROUP_RoutemapChangedCallBackHandler(char *rmap_name, UI32_T seq_num, BOOL_T is_deleted);
#endif

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
static void   NETCFG_GROUP_HandleHotInsertion(SYSFUN_Msg_T* msgbuf_p);
static void   NETCFG_GROUP_HandleHotRemoval(SYSFUN_Msg_T* msgbuf_p);
#endif

#if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE)
static void NETCFG_GROUP_HandleCmgrIpcMsg(SYSFUN_Msg_T *ipc_msg_p);
#endif

/* EXPORTED SUBPROGRAM BODIES
 */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - NETCFG_GROUP_InitiateProcessResources
 *-----------------------------------------------------------------------------
 * PURPOSE : Initiate process resource for NETCFG group.
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
void NETCFG_GROUP_InitiateProcessResources(void)
{
    /* Initial PMGR used in NETCFG Group */
#if (SYS_CPNT_AMTR == TRUE)
    AMTR_PMGR_InitiateProcessResource();
    AMTR_POM_InitiateProcessResource();
#endif
    VLAN_PMGR_InitiateProcessResource();
    VLAN_POM_InitiateProcessResource();
#if (SYS_CPNT_NSM == TRUE)
    NSM_PMGR_InitiateProcessResource();
#endif
    STKTPLG_POM_InitiateProcessResources();
    SWCTRL_PMGR_Init();
#if (SYS_CPNT_AMTRL3 == TRUE)
    AMTRL3_PMGR_InitiateProcessResource();/*Lin.Li, for ARP porting*/
    AMTRL3_POM_InitiateProcessResource();/*Lin.Li, for ARP porting*/
#endif
#if (SYS_CPNT_RIP == TRUE)
    RIP_PMGR_InitiateProcessResource();/*Lin.Li, for RIP porting*/
#endif
#if (SYS_CPNT_BGP == TRUE)
    BGP_PMGR_InitiateProcessResource();
#endif
#if (SYS_CPNT_OSPF == TRUE)
    OSPF_PMGR_InitiateProcessResource();/*Lin.Li, for OSPF porting*/
#endif
    /* added by steven.gao for OSPFv3 */
#if (SYS_CPNT_OSPF6 == TRUE)
    OSPF6_PMGR_InitiateProcessResource();
#endif

#if (SYS_CPNT_UDP_HELPER == TRUE)
    if(UDPHELPER_PMGR_InitiateProcessResource()==FALSE)
        return FALSE;
#endif

    IML_PMGR_InitiateProcessResource();
#if (SYS_CPNT_LLDP == TRUE)
    LLDP_PMGR_InitiateProcessResources();
#endif

#if (SYS_CPNT_VRRP==TRUE)
     VRRP_PMGR_InitiateProcessResource();
#endif

    DHCP_PMGR_InitiateProcessResources();

#if (SYS_CPNT_DHCPV6 == TRUE)
    DHCPv6_PMGR_InitiateProcessResources();
#endif

#if (SYS_CPNT_SWDRVL3 == TRUE)
    DEV_SWDRVL3_PMGR_InitiateProcessResource();
#endif

#if (SYS_CPNT_NICDRV == TRUE)
    DEV_NICDRV_PMGR_InitiateProcessResource();
#endif

    /*
     * L4
     */
	L4_PMGR_Init();

    /* Initial NETCFG Group */
    NETCFG_MGR_MAIN_InitiateProcessResources();

#if (SYS_CPNT_IPV6_RA_GUARD == TRUE)
    SWCTRL_POM_Init();
    TRK_PMGR_InitiateProcessResource();
    L2MUX_PMGR_InitiateProcessResource();
#endif

    IPAL_Kernel_Init();
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - NETCFG_GROUP_Create_InterCSC_Relation
 *-----------------------------------------------------------------------------
 * PURPOSE : Create inter-CSC relationships for NETCFG group.
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
void NETCFG_GROUP_Create_InterCSC_Relation(void)
{
    NETCFG_MGR_MAIN_Create_InterCSC_Relation();
    IPAL_BACKDOOR_Create_InterCSC_Relation();
}


/* FUNCTION NAME:  NETCFG_GROUP_Create_All_Threads
 * PURPOSE:
 *    This function will spawn all threads in CSCGroup2.
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
 *
 */
void NETCFG_GROUP_Create_All_Threads(void)
{
    UI32_T thread_id;

    IPAL_REFLECT_CreateTask();

 /*
#if (SYS_CPNT_CSCB == TRUE)
    CSCB_INIT_Create_Tasks();
#endif */
    /* Create Thread to handle the MGR message */
    if(SYSFUN_SpawnThread(SYS_BLD_NETCFG_GROUP_MGR_THREAD_PRIORITY,
                          SYS_BLD_NETCFG_GROUP_MGR_SCHED_POLICY,
                          SYS_BLD_NETCFG_GROUP_MGR_THREAD_NAME,
                          SYS_BLD_TASK_COMMON_STACK_SIZE,
                          SYSFUN_TASK_FP,
                          NETCFG_GROUP_Mgr_Thread_Function_Entry,
                          NULL, &thread_id)!=SYSFUN_OK)
    {
        printf("%s:Spawn NETCFG Group MGR thread fail.\n", __FUNCTION__);
    }

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
    SW_WATCHDOG_MGR_RegisterMonitorThread(SW_WATCHDOG_NETCFG_GROUP, thread_id, SYS_ADPT_NETCFG_GROUP_SW_WATCHDOG_TIMER);
#endif

}

/* LOCAL SUBPROGRAM BODIES
 */

/* FUNCTION NAME:  NETCFG_GROUP_Mgr_Thread_Function_Entry
 * PURPOSE:
 *    This is the entry function for the MGR thread of NETCFG GROUP.
 *
 * INPUT:
 *    arg   --  not used.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    None.
 *
 * NOTES:
 *    None.
 *
 */
static void NETCFG_GROUP_Mgr_Thread_Function_Entry(void* arg)
{
    SYSFUN_MsgQ_T        ipcmsgq_handle;
    L_THREADGRP_Handle_T tg_handle = NETCFG_PROC_COMM_GetNetcfgTGHandle();
    UI32_T               member_id,received_events,local_events=SYSFUN_SYSTEM_EVENT_IPCMSG;
    SYSFUN_Msg_T         *msgbuf_p=(SYSFUN_Msg_T*)netcfg_mgrtd_ipc_buf;
    BOOL_T               need_resp;

    /* join the thread group of CSCGroup1
     */
    if(L_THREADGRP_Join(tg_handle, SYS_BLD_NETCFG_GROUP_MGR_THREAD_PRIORITY, &member_id)==FALSE)
    {
        printf("%s: L_THREADGRP_Join fail.\n", __FUNCTION__);
        return;
    }

    /* create mgr ipc message queue handle
     */
    if(SYSFUN_CreateMsgQ(SYS_BLD_NETCFG_GROUP_IPCMSGQ_KEY, SYSFUN_MSGQ_BIDIRECTIONAL, &ipcmsgq_handle)!=SYSFUN_OK)
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
#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
                             SYSFUN_SYSTEM_EVENT_SW_WATCHDOG |
#endif
                             SYSFUN_SYSTEM_EVENT_SET_TRANSITION_MODE,
                             SYSFUN_EVENT_WAIT_ANY,
                             (local_events==0)?SYSFUN_TIMEOUT_WAIT_FOREVER:SYSFUN_TIMEOUT_NOWAIT,
                             &received_events);
        local_events |= received_events;

        if(local_events & SYSFUN_SYSTEM_EVENT_SET_TRANSITION_MODE)
        {
            NETCFG_GROUP_SetTransitionMode();
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
            if(SYSFUN_ReceiveMsg(ipcmsgq_handle, 0, SYSFUN_TIMEOUT_NOWAIT,
                NETCFG_PMGR_MSGBUF_MAX_REQUIRED_SIZE, msgbuf_p)==SYSFUN_OK)
            {
                /* request thread group execution permission
                 */
                if(L_THREADGRP_Execution_Request(tg_handle, member_id)!=TRUE)
                {
                    SYSFUN_Debug_Printf("%s: L_THREADGRP_Execution_Request fail.\n", __FUNCTION__);
                }

                /* handle request message based on cmd
                 */
                switch(msgbuf_p->cmd)
                {
                    /* module id cmd
                     */
                    case SYS_MODULE_NETCFG:
                        need_resp=NETCFG_MGR_MAIN_HandleIPCReqMsg(msgbuf_p);
                        break;

                    case SYS_MODULE_ROUTECFG:
                        need_resp=NETCFG_MGR_ROUTE_HandleIPCReqMsg(msgbuf_p);
                        break;

                    case SYS_MODULE_IPCFG:
                        need_resp=NETCFG_MGR_IP_HandleIPCReqMsg(msgbuf_p);
                        break;

                    case SYS_MODULE_BACKDOOR:
                        need_resp=BACKDOOR_MGR_HandleIPCMsg(msgbuf_p);
                        break;

                    /*Lin.Li, for ARP porting, modify start*/
                    case SYS_MODULE_ARPCFG:
                    case SYS_MODULE_NDCFG:
                        need_resp = NETCFG_MGR_ND_HandleIPCReqMsg(msgbuf_p);
                        break;
                   /*Lin.Li, for ARP porting, modify end*/

#if (SYS_CPNT_RIP == TRUE)
                    /*Lin.Li, for RIP porting, modify start*/
                    case SYS_MODULE_RIPCFG:
                        need_resp=NETCFG_MGR_RIP_HandleIPCReqMsg(msgbuf_p);
                        break;
                    /*Lin.Li, for RIP porting, modify end*/
#endif
#if (SYS_CPNT_OSPF == TRUE)
                    /*Lin.Li, for OSPF porting, modify start*/
                    case SYS_MODULE_OSPFCFG:
                        need_resp=NETCFG_MGR_OSPF_HandleIPCReqMsg(msgbuf_p);
                        break;
                    /*Lin.Li, for OSPF porting, modify end*/
#endif
#if (SYS_CPNT_PBR == TRUE)
                    case SYS_MODULE_PBRCFG:
                        need_resp=NETCFG_MGR_PBR_HandleIPCReqMsg(msgbuf_p);
                        break;
#endif

#if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE)
                    case SYS_MODULE_CMGR:
                        NETCFG_GROUP_HandleCmgrIpcMsg(msgbuf_p);
                        msgbuf_p->msg_size = 0;
                        need_resp = TRUE;
                        break;
#endif

                    /* global cmd
                     */
                    case SYS_TYPE_CMD_ENTER_MASTER_MODE:
                        NETCFG_GROUP_EnterMasterMode();

                        /* SYS_TYPE_CMD_ENTER_MASTER_MODE
                         * need a response which contains nothing
                         */
                        msgbuf_p->msg_size=0;
                        need_resp=TRUE;

                        /* IPCFAIL might happen when a upper layer cscgroup has
                         * entered transition mode but lower layer cscgroups still
                         * in master mode so async callback msg will keep sending
                         * from lower layer cscgroups. In this case, the IPCFAIL
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
                        NETCFG_GROUP_EnterSlaveMode();

                        /* SYS_TYPE_CMD_ENTER_SLAVE_MODE
                         * need a response which contains nothing
                         */
                        msgbuf_p->msg_size=0;
                        need_resp=TRUE;

                        /* IPCFAIL might happen when a upper layer cscgroup has
                         * entered transition mode but lower layer cscgroups still
                         * in master mode so async callback msg will keep sending
                         * from lower layer cscgroups. In this case, the IPCFAIL
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
                        NETCFG_GROUP_EnterTransitionMode();

                        /* SYS_TYPE_CMD_ENTER_TRANSITION_MODE
                         * need a response which contains nothing
                         */
                        msgbuf_p->msg_size=0;
                        need_resp=TRUE;
                        break;
              /*Donny.li modify for ARP stacking.2008.08.07 */
                    case SYS_TYPE_CMD_PREPROVISION_COMPLETE:
                        msgbuf_p->msg_size=0;
                        need_resp=TRUE;
                        break;
              /*Donny.li end modify for ARP stacking.2008.08.07 */


              /*Donny.li modify for ARP stacking.2008.08.07 */
                    case SYS_TYPE_CMD_PROVISION_COMPLETE:
                    case SYS_TYPE_CMD_MODULE_PROVISION_COMPLETE:
                        NETCFG_GROUP_ProvisionComplete();

                        /* SYS_TYPE_CMD_PROVISION_COMPLETE
                         * need a response which contains nothing
                         */
                        msgbuf_p->msg_size=0;
                        need_resp=TRUE;
                        break;

                    case SYS_MODULE_SYS_CALLBACK:
                        NETCFG_GROUP_HandleSysCallbackIPCMsg(msgbuf_p);
                        /* SYS_CALLBACK ipc message can only be uni-direction
                         * just set need_resp as FALSE
                         */
                        need_resp = FALSE;
                        break;

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
                     case     SYS_TYPE_CMD_HANDLE_HOT_INSERTION:
                        NETCFG_GROUP_HandleHotInsertion(msgbuf_p);
                        msgbuf_p->msg_size=0;
                        need_resp=TRUE;
                        break;

                     case SYS_TYPE_CMD_HANDLE_HOT_REMOVAL:
                       NETCFG_GROUP_HandleHotRemoval(msgbuf_p);

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
                        need_resp=FALSE;
                        printf("%s: Invalid IPC req cmd.\n", __FUNCTION__);
                }

                /* release thread group execution permission
                 */
                if(L_THREADGRP_Execution_Release(tg_handle, member_id)!=TRUE)
                {
                    SYSFUN_Debug_Printf("%s: L_THREADGRP_Execution_Release fail.\n", __FUNCTION__);
                }

                if((need_resp==TRUE) && (SYSFUN_SendResponseMsg(ipcmsgq_handle, msgbuf_p)!=SYSFUN_OK))
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

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
       if(local_events & SYSFUN_SYSTEM_EVENT_SW_WATCHDOG)
       {
       	   SW_WATCHDOG_MGR_ResetTimer(SW_WATCHDOG_NETCFG_GROUP);
           local_events ^= SYSFUN_SYSTEM_EVENT_SW_WATCHDOG;
       }
#endif

    } /* end of while(1) */
}

/* FUNCTION NAME:  NETCFG_GROUP_EnterTransitionMode
 * PURPOSE:
 *    This function will invoke all enter transition mode function in NETCFG GROUP.
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
 *
 */
static void NETCFG_GROUP_EnterTransitionMode(void)
{
    NETCFG_MGR_MAIN_EnterTransitionMode();

}
/*Donny.li modify for ARP stacking.2008.08.07 */
/* FUNCTION NAME:  NETCFG_GROUP_ProvisionComplete
 * PURPOSE:
 *     Let default gateway CFGDB into route when provision complete.
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
 *
 */
static void NETCFG_GROUP_ProvisionComplete(void)
{
    NETCFG_MGR_MAIN_ProvisionComplete();
}
/*Donny.li end modify for ARP stacking.2008.08.07 */


/* FUNCTION NAME:  NETCFG_GROUP_SetTransitionMode
 * PURPOSE:
 *    This function will invoke all set transition mode function in NETCFG GROUP.
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
 *
 */
static void NETCFG_GROUP_SetTransitionMode(void)
{
    NETCFG_MGR_MAIN_SetTransitionMode();
}

/* FUNCTION NAME:  NETCFG_GROUP_EnterMasterMode
 * PURPOSE:
 *    This function will invoke all enter master mode function in NETCFG GROUP.
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
 *
 */
static void NETCFG_GROUP_EnterMasterMode(void)
{
    NETCFG_MGR_MAIN_EnterMasterMode();
}

/* FUNCTION NAME:  NETCFG_GROUP_EnterSlaveMode
 * PURPOSE:
 *    This function will invoke all enter slave mode function in CSCGroup2.
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
 *
 */
static void NETCFG_GROUP_EnterSlaveMode(void)
{

    NETCFG_MGR_MAIN_EnterSlaveMode();

}


/* FUNCTION NAME:  NETCFG_GROUP_HandleSysCallbackIPCMsg
 * PURPOSE : This function will handle all callbacks from IPC messages in
 *           NETCFG group.
 *
 * INPUT   : msgbuf_p -- SYS_CALLBACK IPC message
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *
 */
static void NETCFG_GROUP_HandleSysCallbackIPCMsg(SYSFUN_Msg_T *msgbuf_p)
{
    SYS_CALLBACK_MGR_Msg_T *sys_cbmsg_p = (SYS_CALLBACK_MGR_Msg_T*)msgbuf_p->msg_buf;

    switch (sys_cbmsg_p->callback_event_id)
    {
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_L3VLAN_DESTROY:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                NETCFG_GROUP_VlanDestroyCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_L3IF_OPER_STATUS_CHANGED:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                NETCFG_GROUP_IfOperStatusChangedCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_IPCFG_NSM_ROUTE_CHANGE:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                NECTCFG_GROUP_NsmRouteChangeCallbackHandler);
            break;

#if (SYS_CPNT_IPV6_RA_GUARD == TRUE)
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_TRUNK_MEMBER_ADD_1ST:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                NECTCFG_GROUP_TrunkMemberAdd1stCallbackHandler);
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_TRUNK_MEMBER_ADD:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                NECTCFG_GROUP_TrunkMemberAddCallbackHandler);
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_TRUNK_MEMBER_DELETE:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                NECTCFG_GROUP_TrunkMemberDeleteCallbackHandler);
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_TRUNK_MEMBER_DELETE_LST:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                NECTCFG_GROUP_TrunkMemberDeleteLstCallbackHandler);
            break;
    #if (SYS_CPNT_IPV6_RA_GUARD_SW_RELAY == TRUE)
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_RAGUARD_PACKET:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                NECTCFG_GROUP_RaGuardPacketCallbackHandler);
            break;
    #endif /* #if (SYS_CPNT_IPV6_RA_GUARD_SW_RELAY == TRUE) */
#endif /* #if (SYS_CPNT_IPV6_RA_GUARD == TRUE)*/

#if (SYS_CPNT_PBR == TRUE)
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_HOST_ROUTE_CHANGED:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                NETCFG_GROUP_HostRouteChangedCallBackHandler);            
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_ACL_CHANGED:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                NETCFG_GROUP_AclChangedCallBackHandler);            
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_ROUTEMAP_CHANGED:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                NETCFG_GROUP_RoutemapChangedCallBackHandler);            
            break;
#endif
        default:
            SYSFUN_Debug_Printf(
                "\n%s: received callback_event that is not handled(%d)\n",
                __FUNCTION__, sys_cbmsg_p->callback_event_id);
    }
}


/* FUNCTION NAME:  NETCFG_GROUP_VlanDestroyCallbackHandler
 * PURPOSE : Handle the callback event happening when a vlan is deleted.
 *
 * INPUT   : vid_ifindex -- specify which vlan has just been deleted
 *           vlan_status -- VAL_dot1qVlanStatus_other
 *                          VAL_dot1qVlanStatus_permanent
 *                          VAL_dot1qVlanStatus_dynamicGvrp
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *
 */
static void NETCFG_GROUP_VlanDestroyCallbackHandler(UI32_T vid_ifindex, UI32_T vlan_status)
{
    /*  LOCAL VARIABLE DECLARATION
     */

    /*  BODY
     */
    NETCFG_MGR_IP_L3InterfaceDestory_CallBack(vid_ifindex);

    return;
}   /*  end of NETCFG_MGR_VlanDestroy_CallBack  */

/* FUNCTION NAME:  NECTCFG_GROUP_NsmRouteChangeCallbackHandler
 * PURPOSE : Handle the callback event happening when nsm rib route change.
 *
 * INPUT   : address_family -- specify which address family has route change (IPv4/IPv6)
 *
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *
 */
static void NECTCFG_GROUP_NsmRouteChangeCallbackHandler(UI32_T address_family)

{

    NETCFG_MGR_IP_NsmRouteChange_CallBack(address_family);

    return;
}   /*  end of NECTCFG_GROUP_NsmRouteChangeCallbackHandler  */

/* FUNCTION NAME:  NETCFG_GROUP_IfOperStatusChangedCallbackHandler
 * PURPOSE : Handle the callback event happening when a vlan operation status is changed.
 *
 * INPUT   : vid_ifindex -- specify which vlan has just been deleted
 *           oper_status -- VAL_ifOperStatus_up, interface up.
 *                          VAL_ifOperStatus_down, interface down.
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *
 */
static void NETCFG_GROUP_IfOperStatusChangedCallbackHandler(UI32_T vid_ifindex,
                                                    UI32_T oper_status)
{
    /*  LOCAL VARIABLE DECLARATION
     */
    /*  BODY
     */
    NETCFG_MGR_IP_L3IfOperStatusChanged_CallBack(vid_ifindex, oper_status);

#if (SYS_CPNT_NSM != TRUE)
    /* If NSM is not included, we need to add/delete default gateway to/from
     * Linux kernel ourself (for L2 products)
     */
    NETCFG_MGR_ROUTE_L3IfOperStatusChanged_CallBack(vid_ifindex, oper_status);
#endif
    return;
}   /*  end of NETCFG_MGR_IfOperStatusChanged_CallBack  */

#if (SYS_CPNT_IPV6_RA_GUARD == TRUE)
/* FUNCTION NAME - NECTCFG_GROUP_TrunkMemberAdd1stCallbackHandler
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
 */
static void NECTCFG_GROUP_TrunkMemberAdd1stCallbackHandler(
    UI32_T  trunk_ifindex,
    UI32_T  member_ifindex)
{
    NETCFG_MGR_ND_RAGUARD_AddFstTrkMbr_CallBack(trunk_ifindex, member_ifindex);
}

/* FUNCTION NAME - NECTCFG_GROUP_TrunkMemberAddCallbackHandler
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
 */
static void NECTCFG_GROUP_TrunkMemberAddCallbackHandler(
    UI32_T  trunk_ifindex,
    UI32_T  member_ifindex)
{
    NETCFG_MGR_ND_RAGUARD_AddTrkMbr_CallBack(trunk_ifindex, member_ifindex);
}

/* FUNCTION NAME - NECTCFG_GROUP_TrunkMemberDeleteCallbackHandler
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
 */
static void NECTCFG_GROUP_TrunkMemberDeleteCallbackHandler(
    UI32_T  trunk_ifindex,
    UI32_T  member_ifindex)
{
    NETCFG_MGR_ND_RAGUARD_DelTrkMbr_CallBack(trunk_ifindex, member_ifindex);
}

/* FUNCTION NAME - NECTCFG_GROUP_TrunkMemberDeleteLstCallbackHandler
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
 */
static void NECTCFG_GROUP_TrunkMemberDeleteLstCallbackHandler(
    UI32_T  trunk_ifindex,
    UI32_T  member_ifindex)
{
    NETCFG_MGR_ND_RAGUARD_DelLstTrkMbr_CallBack(trunk_ifindex, member_ifindex);
}

#if (SYS_CPNT_IPV6_RA_GUARD_SW_RELAY == TRUE)
/* FUNCTION NAME - NECTCFG_GROUP_RaGuardPacketCallbackHandler
 * PURPOSE : Handle the callback event happening when a network interface
 *           received a RA Guard packet.
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
 */
static void NECTCFG_GROUP_RaGuardPacketCallbackHandler(
    L_MM_Mref_Handle_T  *mref_handle_p,
    UI8_T               dst_mac[SYS_ADPT_MAC_ADDR_LEN],
    UI8_T               src_mac[SYS_ADPT_MAC_ADDR_LEN],
    UI16_T              ingress_vid,
    UI8_T               ing_cos,
    UI8_T               pkt_type,
    UI32_T              pkt_length,
    UI32_T              src_lport)
{
    NETCFG_MGR_ND_RAGUARD_ProcessRcvdPDU(
        mref_handle_p,
        dst_mac,
        src_mac,
        ingress_vid,
        ing_cos,
        pkt_type,
        pkt_length,
        src_lport);
}
#endif /* #if (SYS_CPNT_IPV6_RA_GUARD_SW_RELAY == TRUE) */
#endif /* #if (SYS_CPNT_IPV6_RA_GUARD == TRUE) */

#if (SYS_CPNT_PBR == TRUE)
static void NETCFG_GROUP_HostRouteChangedCallBackHandler(L_INET_AddrIp_T *addr_p, BOOL_T is_unresolved)
{
    NETCFG_MGR_PBR_HostRouteChangedCallbackHandler(addr_p, is_unresolved);
}

static void NETCFG_GROUP_AclChangedCallBackHandler(UI32_T acl_index, char *acl_name, UI8_T type)
{
    NETCFG_MGR_PBR_AclChangedCallbackHandler(acl_index, acl_name, type);
}

static void NETCFG_GROUP_RoutemapChangedCallBackHandler(char *rmap_name, UI32_T seq_num, BOOL_T is_deleted)
{
    NETCFG_MGR_PBR_RoutemapChangedCallbackHandler(rmap_name, seq_num, is_deleted);
}
#endif

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
/*------------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_GROUP_HandleHotInsertion
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will invoke a new dut insertion in NETCFG.
 *
 * INPUT:
 *    msgbuf_p->starting_port_ifindex   -- starting port ifindex
 *    msgbuf_p->number_of_port          -- number of ports
 *    msgbuf_p->use_default             -- whether use default setting
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
static void NETCFG_GROUP_HandleHotInsertion(SYSFUN_Msg_T* msgbuf_p)
{
     SYS_TYPE_HandleHotSwapArg_T *msg_p;

     if (msgbuf_p == NULL)
        return ;

    msg_p = (SYS_TYPE_HandleHotSwapArg_T*)msgbuf_p->msg_buf;
    if (msg_p->csc_subset_identifier == SYS_TYPE_CMD_CSC_SUBSET_IDENTIFIER_L3IF)
    {
        NETCFG_MGR_IP_HandleHotInsertionForL3If(msg_p->starting_port_ifindex, msg_p->number_of_port, msg_p->use_default);
    }

#if (SYS_CPNT_IPV6_RA_GUARD == TRUE)
    NETCFG_MGR_ND_RAGUARD_HandleHotInsertion(
        msg_p->starting_port_ifindex, msg_p->number_of_port, msg_p->use_default);
#endif

    return;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_GROUP_ HandleHotRemoval
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will invoke a  dut removal in NETCFG.
 *
 * INPUT:
 *    msgbuf_p->starting_port_ifindex  -- starting port ifindex
 *    msgbuf_p->number_of_port         -- number of ports
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
static void NETCFG_GROUP_HandleHotRemoval(SYSFUN_Msg_T* msgbuf_p)
{
    SYS_TYPE_HandleHotSwapArg_T *msg_p;

    if (msgbuf_p == NULL)
        return;

    msg_p = (SYS_TYPE_HandleHotSwapArg_T*)msgbuf_p->msg_buf;

#if (SYS_CPNT_IPV6_RA_GUARD == TRUE)
    NETCFG_MGR_ND_RAGUARD_HandleHotRemoval(
        msg_p->starting_port_ifindex, msg_p->number_of_port);
#endif /* #if (SYS_CPNT_IPV6_RA_GUARD == TRUE) */

//    NETCFG_MGR_HandleHotRemoval(msg_p->starting_port_ifindex, msg_p->number_of_port);

}
#endif

#if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE)
static void NETCFG_GROUP_HandleCmgrIpcMsg(SYSFUN_Msg_T *ipc_msg_p)
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
        NETCFG_GROUP_VlanDestroyCallbackHandler(
            cmgr_msg_p->data.arg_ui32_ui32.arg_ui32_1,
            cmgr_msg_p->data.arg_ui32_ui32.arg_ui32_2);
        break;

    case CMGR_IPC_L3_IF_OPER_STATUS_CHANGE:
        NETCFG_GROUP_IfOperStatusChangedCallbackHandler(
            cmgr_msg_p->data.arg_ui32_ui32.arg_ui32_1,
            cmgr_msg_p->data.arg_ui32_ui32.arg_ui32_2);
        break;
    } /* end switch */

    return;
}
#endif /* #if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE) */
