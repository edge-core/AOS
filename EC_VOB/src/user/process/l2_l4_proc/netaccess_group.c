/* MODULE NAME:  netaccess_group.c
 * PURPOSE:
 *     This file is a demonstration code for implementations of netaccess group.
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
#include "netaccess_group.h"
#include "swctrl.h"

#if (SYS_CPNT_DOT1X == TRUE)
#include "dot1x_vm.h"
#endif

#if (SYS_CPNT_AMTR == TRUE)
#include "amtr_init.h"
#include "amtr_mgr.h"
#include "amtr_type.h"
#endif
#if (SYS_CPNT_NMTR == TRUE)
#include "nmtr_init.h"
#include "nmtr_mgr.h"
#include "nmtr_type.h"
#endif

#if (SYS_CPNT_ADD == TRUE)
#include "add_init.h"
#include "add_mgr.h"
#endif

#include "amtrdrv_mgr.h"
#include "amtrdrv_pom.h"

#if (SYS_CPNT_PORT_SECURITY == TRUE)&& (SYS_CPNT_NETWORK_ACCESS == FALSE)
#include "psec_init.h"
#include "psec_mgr.h"
#include "psec_task.h"
#endif

#if (SYS_CPNT_NETACCESS == TRUE)
#include "netaccess_init.h"
#include "netaccess_mgr.h"
#include "l2mux_mgr.h"
#endif

#if (SYS_CPNT_DOS == TRUE)
#include "dos_init.h"
#include "dos_mgr.h"
#endif

#if (SYS_CPNT_APP_FILTER == TRUE)
#include "af_mgr.h"
#endif /* #if (SYS_CPNT_APP_FILTER == TRUE) */

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
#include "sw_watchdog_mgr.h"
#endif

#if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE)
#include "cmgr.h"
#endif

/* NAMING CONSTANT DECLARATIONS
 */
typedef union {
    /* need add sys_callback msg */
#if (SYS_CPNT_AMTRDRV == TRUE)
    struct AMTRDRV_ADDR_CBData_S {
        UI32_T num_of_entries;
#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == TRUE)
        AMTR_TYPE_AddrEntry_T               addr_buf[SYS_ADPT_AMTR_NBR_OF_ADDR_TO_SYNC_IN_ONE_SCAN];
#else
        AMTR_TYPE_AddrEntry_T               addr_buf[AMTRDRV_TYPE_MAX_ENTRIES_IN_ONE_PACKET];
#endif
    } AMTRDRV_ADDR_CBData_T;
#endif
    SYS_CALLBACK_MGR_AnnounceRADIUSPacket_CBData_T    sys_callback_mgr_announce_radius_packet_cbdata;
#if (SYS_CPNT_REFINE_IPC_MSG == TRUE)
    SYS_CALLBACK_MGR_REFINEList_CBData_T  sys_callback_mgr_refinelist_cbdata;
#endif
    SYS_CALLBACK_MGR_AuthenticatePacket_CBData_T    sys_callback_mgr_authpkt_cbdata;
    SYS_CALLBACK_MGR_PortLearningStatusChanged_CBData_T sys_callback_mgr_port_learning_status_cbdata;
} NETACCESS_GROUP_Syscallback_CBData_T;

typedef struct
{
    UI8_T buffer[SYS_CALLBACK_MGR_SIZE_OF_MSG(sizeof(NETACCESS_GROUP_Syscallback_CBData_T))];
} NETACCESS_GROUP_Syscallback_Msg_T;

/* union all data type used for CSCGroup1 MGR IPC message to get the maximum
 * required ipc message buffer
 */
typedef union NETACCESS_GROUP_MGR_MSG_U
{
#if (SYS_CPNT_AMTRDRV == TRUE)
    struct AMTRDRV_ADDR_IpcMsg_S {
        UI32_T num_of_entries;
#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == TRUE)
        AMTR_TYPE_AddrEntry_T               addr_buf[SYS_ADPT_AMTR_NBR_OF_ADDR_TO_SYNC_IN_ONE_SCAN];
#else
        AMTR_TYPE_AddrEntry_T               addr_buf[AMTRDRV_TYPE_MAX_ENTRIES_IN_ONE_PACKET];
#endif
    } AMTRDRV_ADDR_IpcMsg_T;
#endif

#if (SYS_CPNT_AMTR == TRUE)
    AMTR_MGR_IpcMsg_T amtr_mgr_ipcmsg;
#endif

#if (SYS_CPNT_PORT_SECURITY == TRUE)&& (SYS_CPNT_NETWORK_ACCESS == FALSE)
    PSEC_MGR_IPCMsg_T psec_mgr_ipcmsg;
#endif

#if (SYS_CPNT_NMTR == TRUE)
    NMTR_MGR_IPCMsg_T nmtr_mgr_ipcmsg;
#endif

#if (SYS_CPNT_NETACCESS == TRUE)
//NETACCESS_MGR_HandleIPCReqMsg(msgbuf_p);
#endif

#if (SYS_CPNT_DOS == TRUE)
    DOS_MGR_IpcMsg_T dos_mgr_ipcmsg;
#endif

#if (SYS_CPNT_APP_FILTER == TRUE)
    AF_MGR_IpcMsg_T af_mgr_ipcmsg;
#endif /* #if (SYS_CPNT_APP_FILTER == TRUE) */

    BACKDOOR_MGR_Msg_T                  backdoor_mgr_ipcmsg;
    NETACCESS_GROUP_Syscallback_Msg_T   sys_callback_ipcmsg;
#if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE)
    CMGR_IpcMsg_T                       cmgr_ipcmsg;
#endif
} NETACCESS_GROUP_MGR_MSG_T;

static UI32_T netaccess_group1_id = 0;
#define NETACCESS_GROUP_PMGR_MSGBUF_MAX_REQUIRED_SIZE sizeof(NETACCESS_GROUP_MGR_MSG_T)

#define NETACCESS_TIMER_TICKS1SEC   100
#define L2_L4_EVENT_TIMER   BIT_1 /* timer event */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void NETACCESS_GROUP_Mgr_Thread_Function_Entry(void* arg);
static void NETACCESS_GROUP_Mgr_Thread_Hash2hisam_Function_Entry(void* arg);
static void NETACCESS_NMTR_Thread_Function_Entry(void* arg);
static void NETACCESS_GROUP_SetTransitionMode(void);
static void NETACCESS_GROUP_EnterTransitionMode(void);
static void NETACCESS_GROUP_EnterMasterMode(void);
static void NETACCESS_GROUP_EnterSlaveMode(void);
static void NETACCESS_GROUP_ProvisionComplete(void);
static void NETACCESS_HandleNMTRSysCallbackIPCMsg(SYSFUN_Msg_T *msgbuf_p);
static void NETACCESS_GROUP_HandleSysCallbackIPCMsg(SYSFUN_Msg_T *msgbuf_p);

static void NETACCESS_GROUP_LportLinkDownCallbackHandler(UI32_T ifindex);
static void NETACCESS_GROUP_TrunkMemberAdd1stCallbackHandler(
    UI32_T trunk_ifindex, UI32_T member_ifindex);
static void NETACCESS_GROUP_TrunkMemberAddCallbackHandler(
    UI32_T trunk_ifindex, UI32_T member_ifindex);
static void NETACCESS_GROUP_TrunkMemberDelCallbackHandler(
    UI32_T trunk_ifindex, UI32_T member_ifindex);
static void NETACCESS_GROUP_TrunkMemberDeleteLstCallbackHandler(
    UI32_T trunk_ifindex, UI32_T member_ifindex);
static void NETACCESS_GROUP_TrunkMemberActiveCallbackHandler(UI32_T trunk_ifindex,
                                                             UI32_T member_ifindex);
static void NETACCESS_GROUP_LportAdminEnableCallbackHandler(UI32_T ifindex);
static void NETACCESS_GROUP_LportAdminDisableCallbackHandler(UI32_T ifindex);
static void NETACCESS_GROUP_VlanDestroyCallbackHandler(
    UI32_T vid_ifindex, UI32_T vlan_status);
static void NETACCESS_GROUP_VlanMemberAddCallbackHandler(
    UI32_T vid_ifindex, UI32_T lport_ifindex, UI32_T vlan_status);
static void NETACCESS_GROUP_VlanMemberDeleteCallbackHandler(
    UI32_T vid_ifindex, UI32_T lport_ifindex, UI32_T vlan_status);
static void NETACCESS_GROUP_NewAddressCallbackHandler(
    UI32_T num_of_entries, AMTR_TYPE_AddrEntry_T addr_buf[]);
static void NETACCESS_GROUP_AgingOutCallbackHandler(
    UI32_T num_of_entries, AMTR_TYPE_AddrEntry_T addr_buf[]);
static void NETACCESS_GROUP_SecurityCheckCallbackHandler(
    UI32_T src_lport, UI16_T vid, UI8_T *src_mac, UI8_T *dst_mac, UI16_T ether_type);
static void NETACCESS_GROUP_UPortLinkUpCallbackHandler(
    UI32_T unit, UI32_T port);
static void NETACCESS_GROUP_UPortLinkDownCallbackHandler(
    UI32_T unit, UI32_T port);
static void NETACCESS_GROUP_UPortAdminEnableCallbackHandler(
    UI32_T unit, UI32_T port);
static void NETACCESS_GROUP_UPortAdminDisableCallbackHandler(
    UI32_T unit, UI32_T port);
static void NETACCESS_GROUP_PortLearningStatusChangedCallbackHandler(
    UI32_T lport,
    BOOL_T learning);
static void NETACCESS_GROUP_UPortDot1xEffectiveOperStatusChangedCallbackHandler(
    UI32_T unit, UI32_T port, UI32_T pre_status, UI32_T current_status);

static void NETACCESS_GROUP_UpdateNmtrdrvStatsCallbackHandler(
    UI32_T update_type, UI32_T unit,UI32_T start_port, UI32_T port_amount);

static void NETACCESS_GROUP_Dot1xPacketCallbackHandler(
    L_MM_Mref_Handle_T *mref_handle_p, UI8_T   dst_mac[SYS_ADPT_MAC_ADDR_LEN],
    UI8_T   src_mac[SYS_ADPT_MAC_ADDR_LEN], UI16_T  tag_info, UI16_T  type,
    UI32_T  pkt_length, UI32_T  src_unit, UI32_T  src_port, UI32_T  packet_class);

static void NETACCESS_GROUP_AnnounceRadiusPacketCallbackHandler(
    UI32_T  result,                 UI8_T   *data_buf,
    UI32_T  data_len,               UI32_T  src_port,
    UI8_T   *src_mac,               UI32_T  src_vid,
    char   *authorized_vlan_list,  char   *authorized_qos_list,
    UI32_T   session_timeout,       UI32_T  server_ip);

static void NETACCESS_GROUP_AnnounceRadaAuthPacketCallbackHandler(
    UI32_T  lport,                  UI8_T   *mac,
    int     identifier,             BOOL_T  authorized_result,
    char   *authorized_vlan_list,  char   *authorized_qos_list,
    UI32_T  session_time,           UI32_T  server_ip);

static void NETACCESS_GROUP_AnnounceDot1xAuthPacketCallbackHandler(
    UI32_T  lport,                  UI8_T   *mac,
    int     eap_identifier,         BOOL_T  authorized_result,
    char   *authorized_vlan_list,  char   *authorized_qos_list,
    UI32_T  session_time,           UI32_T  server_ip);

#if (SYS_CPNT_ADD == TRUE)
static void NETACCESS_GROUP_TelephoneDetectCallbackHandler(
    UI32_T  lport,
    UI8_T  *mac_addr,
    UI8_T   network_addr_subtype,
    UI8_T  *network_addr,
    UI8_T   network_addr_len,
    UI32_T  network_addr_ifindex,
    BOOL_T  tel_exist);
#endif

static void NETACCESS_GROUP_AnnounceACLDeletedHandler(
    char    *acl_name,
    UI32_T  acl_type,
    UI8_T   *dynamic_port_list);

static
void NETACCESS_GROUP_AnnouncePolicyMapDeletedHandler(
    char    *policy_map_name,
    UI8_T   *dynamic_port_list);

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
static void NETACCESS_GROUP_HandleHotInertion(SYSFUN_Msg_T* msgbuf_p);

static void NETACCESS_GROUP_HandleHotRemoval(SYSFUN_Msg_T* msgbuf_p);
#endif
#if (SYS_CPNT_REFINE_IPC_MSG == TRUE)
static void NETACCESS_GROUP_VlanListCallbackHandler(UI32_T subid,SYS_CALLBACK_MGR_REFINEList_CBData_T*msg);
#endif

#if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE)
static void NETACCESS_GROUP_HandleCmgrIpcMsg(SYSFUN_Msg_T *ipc_msg_p);
#endif

/* STATIC VARIABLE DECLARATIONS
 */
/* the buffer for retrieving ipc request for netaccess_group mgr thread
 */
static UI8_T netaccess_group_mgrtd_ipc_buf[SYSFUN_SIZE_OF_MSG(NETACCESS_GROUP_PMGR_MSGBUF_MAX_REQUIRED_SIZE)];
static UI8_T netaccess_nmtr_ipc_buf[SYSFUN_SIZE_OF_MSG(NETACCESS_GROUP_PMGR_MSGBUF_MAX_REQUIRED_SIZE)];

/* EXPORTED SUBPROGRAM BODIES
 */
/*-----------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_GROUP_InitiateProcessResources
 *-----------------------------------------------------------------------------
 * PURPOSE : Initiate process resource for NETACCESS_Group.
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
void NETACCESS_GROUP_InitiateProcessResources(void)
{
#if (SYS_CPNT_AMTR == TRUE)
    AMTR_INIT_InitiateSystemResources();
#endif
#if (SYS_CPNT_NMTR == TRUE)
    NMTR_INIT_InitiateSystemResources();
#endif

#if (SYS_CPNT_ADD == TRUE)
    ADD_INIT_Initiate_System_Resources();
#endif

#if (SYS_CPNT_PORT_SECURITY == TRUE)
    PSEC_INIT_InitiateProcessResources();
#endif

#if (SYS_CPNT_NETACCESS == TRUE)
    NETACCESS_INIT_InitiateSystemResources();
#endif

#if (SYS_CPNT_DOS == TRUE)
    DOS_INIT_InitiateSystemResources();
#endif

#if (SYS_CPNT_APP_FILTER == TRUE)
    AF_MGR_InitiateSystemResources();
#endif /* #if (SYS_CPNT_APP_FILTER == TRUE)*/
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_GROUP_Create_InterCSC_Relation
 *-----------------------------------------------------------------------------
 * PURPOSE : Create inter-CSC relationships for NETACCESS_Group.
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
void NETACCESS_GROUP_Create_InterCSC_Relation(void)
{
#if (SYS_CPNT_AMTR == TRUE)
    AMTR_INIT_Create_InterCSC_Relation();
#endif
#if (SYS_CPNT_NMTR == TRUE)
    NMTR_INIT_Create_InterCSC_Relation();
#endif

#if (SYS_CPNT_ADD == TRUE)
    ADD_INIT_Create_InterCSC_Relation();
#endif

#if (SYS_CPNT_PORT_SECURITY == TRUE)
    PSEC_INIT_Create_InterCSC_Relation();
#endif

#if (SYS_CPNT_NETACCESS == TRUE)
    NETACCESS_INIT_Create_InterCSC_Relation();
#endif

#if (SYS_CPNT_DOS == TRUE)
    DOS_INIT_Create_InterCSC_Relation();
#endif

#if (SYS_CPNT_APP_FILTER == TRUE)
    AF_MGR_Create_InterCSC_Relation();
#endif /* #if (SYS_CPNT_APP_FILTER == TRUE)*/
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : NETACCESS_GROUP_Create_All_Threads
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will spawn all threads in NETACCESS_Group.
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
void NETACCESS_GROUP_Create_All_Threads(void)
{
    UI32_T thread_id;

    if(SYSFUN_SpawnThread(SYS_BLD_NETACCESS_GROUP_MGR_THREAD_PRIORITY,
                          SYS_BLD_NETACCESS_GROUP_MGR_SCHED_POLICY,
                          SYS_BLD_NETACCESS_GROUP_MGR_THREAD_NAME,
                          SYS_BLD_TASK_COMMON_STACK_SIZE,
                          SYSFUN_TASK_FP,
                          NETACCESS_GROUP_Mgr_Thread_Function_Entry,
                          NULL, &thread_id)!=SYSFUN_OK)
    {
        printf("%s:Spawn CSCGroup1 MGR thread fail.\n", __FUNCTION__);
    }

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
    SW_WATCHDOG_MGR_RegisterMonitorThread(SW_WATCHDOG_NETACCESS_GROUP, thread_id, SYS_ADPT_NETACCESS_GROUP_SW_WATCHDOG_TIMER);
#endif

    AMTRDRV_MGR_SetAmtrID(thread_id);
    if(SYSFUN_SpawnThread(SYS_BLD_NETACCESS_GROUP_MGR_THREAD_PRIORITY,
                          SYS_BLD_NETACCESS_GROUP_MGR_SCHED_POLICY,
                          SYS_BLD_NETACCESS_NMTR_THREAD_NAME,
                          SYS_BLD_TASK_COMMON_STACK_SIZE,
                          SYSFUN_TASK_FP,
                          NETACCESS_NMTR_Thread_Function_Entry,
                          NULL, &thread_id)!=SYSFUN_OK)
    {
        printf("%s:Spawn CSCGroup1 MGR thread fail.\n", __FUNCTION__);
    }

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
    SW_WATCHDOG_MGR_RegisterMonitorThread(SW_WATCHDOG_NETACCESS_NMTR, thread_id, SYS_ADPT_NETACCESS_NMTR_SW_WATCHDOG_TIMER);
#endif
    /*Fix ERP:ES4827G-FLF-ZZ-00464*/
    if(SYSFUN_SpawnThread(SYS_BLD_NETACCESS_GROUP_MGR_THREAD_HASH2HSIAM_PRIORITY,
                          SYS_BLD_NETACCESS_GROUP_MGR_HASH2HSIAM_SCHED_POLICY,
                          SYS_BLD_NETACCESS_GROUP_MGR_THREAD_HASH2HSIAM,
                          SYS_BLD_TASK_COMMON_STACK_SIZE,
                          SYSFUN_TASK_FP,
                          NETACCESS_GROUP_Mgr_Thread_Hash2hisam_Function_Entry,
                          NULL, &thread_id)!=SYSFUN_OK)
    {
        printf("%s:Spawn %s thread fail.\n", __FUNCTION__,SYS_BLD_NETACCESS_GROUP_MGR_THREAD_HASH2HSIAM);
    }

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
    SW_WATCHDOG_MGR_RegisterMonitorThread(SW_WATCHDOG_NETACCESS_NMTR_HASH2HISAM, thread_id, SYS_ADPT_NETACCESS_NMTR_HASH2HISAM_SW_WATCHDOG_TIMER);
#endif

#if (SYS_CPNT_PORT_SECURITY == TRUE)
    PSEC_INIT_Create_Tasks();
#endif

#if (SYS_CPNT_NETACCESS == TRUE)
    NETACCESS_INIT_CreateTasks();
#endif

}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - STA_GROUP_InitiateProcessResources
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

#if (SYS_CPNT_INTRUSION_MSG_TRAP == TRUE)
/*-----------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_GROUP_IntrusionMacCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : When detecting intrusion mac, AMTR will notify other CSC groups
 *           by this function.
 *
 * INPUT   : src_lport  -- which lport
 *           vid        -- which vlan id
 *           src_mac    -- source mac address
 *           dst_mac    -- destination mac address
 *           ether_type -- ether type
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE   -- Is intrusion
 *           FALSE  -- Is not intrusion
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T NETACCESS_GROUP_IntrusionMacCallbackHandler(UI32_T src_lport, UI16_T vid,
                                                 UI8_T *src_mac, UI8_T *dst_mac,
                                                 UI16_T ether_type)
{
/* even if netacess is TRUE,
 *   psec need to handle intrusion callback by itself,
 *   bcz the whole work is still controlled by psec, not by netaccess.
 */
#if (SYS_CPNT_PORT_SECURITY == TRUE)&& (SYS_CPNT_NETWORK_ACCESS == FALSE)
    /* intrusion packet was processed by PSEC, need not go on.
     */
    if (TRUE == PSEC_TASK_IntrusionMac_CallBack(
                    src_lport, vid, src_mac, dst_mac, ether_type, NULL))
    {
        return TRUE;
    }
#endif

#if (SYS_CPNT_NETACCESS == TRUE)
    if (TRUE == NETACCESS_MGR_AnnounceNewMac_CallBack(
        src_lport, vid, src_mac,
        dst_mac, ether_type, L2MUX_MGR_RECV_REASON_INTRUDER, NULL))
    {
        return TRUE;
    }
#endif

    return FALSE;
}
#endif /* #if (SYS_CPNT_INTRUSION_MSG_TRAP == TRUE) */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_GROUP_SecurityPortMoveCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : When port move, AMTR will notify other CSC groups by this
 *           function.
 *
 * INPUT   : ifindex          -- port whcih the mac is learnt now
 *           vid              -- which vlan id
 *           mac              -- mac address
 *           original_ifindex -- original port which the mac was learnt before
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
void NETACCESS_GROUP_SecurityPortMoveCallbackHandler(UI32_T ifindex,
                                                     UI32_T vid,
                                                     UI8_T  *mac,
                                                     UI32_T original_ifindex)
{
/* even if netacess is TRUE,
 *   psec need to handle port-move callback by itself,
 *   bcz the whole work is still controlled by psec, not by netaccess.
 */
#if (SYS_CPNT_PORT_SECURITY == TRUE)&& (SYS_CPNT_NETWORK_ACCESS == FALSE)

    /* port move packet was processed by PSEC, need not go on.
     */
    if (TRUE == PSEC_TASK_PortMove_CallBack(
                    ifindex, vid, mac, original_ifindex))
        return;

#endif

#if (SYS_CPNT_NETACCESS == TRUE)
{
    NETACCESS_MGR_AnnounceNewMac_CallBack(
        ifindex, vid, mac,
        NULL, 0, L2MUX_MGR_RECV_REASON_STATION_MOVE, NULL);
}
#endif

    return;
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_GROUP_AutoLearnCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : When port move, AMTR will notify other CSC groups by this
 *           function.
 *
 * INPUT   : ifindex        --
 *           portsec_status --
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
void NETACCESS_GROUP_AutoLearnCallbackHandler(UI32_T ifindex, UI32_T portsec_status)
{
/* even if netacess is TRUE,
 *   psec need to handle auto-learn callback by itself,
 *   bcz the mac learning is still controlled by AMTR, not by netaccess.
 */
#if (SYS_CPNT_PORT_SECURITY == TRUE)&& (SYS_CPNT_NETWORK_ACCESS == FALSE)
    PSEC_MGR_SetPortSecurityStatusOperation_Callback(ifindex, portsec_status);
#endif

/* if netacess is TRUE,
 *   dot1x op multihost is handled by netaccess directly,
 *   it's not necessary to call dot1x_mgr to handle auto-learn callback.
 */
#if (SYS_CPNT_NETACCESS == FALSE)
#if (SYS_CPNT_DOT1X == TRUE)
    DOT1X_MGR_SetPortSecurityStatus(ifindex, portsec_status);
#endif /* #if (SYS_CPNT_DOT1X == TRUE) */
#endif /* #if (SYS_CPNT_NETACCESS == TRUE) */
    return;
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_GROUP_MacAgeOutCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Call call-back function, when one mac is aged out
 *
 * INPUT   : vid, mac, ifindex
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
void NETACCESS_GROUP_MacAgeOutCallbackHandler(
    UI32_T vid,     UI8_T  *mac,    UI32_T ifindex)
{

}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_GROUP_MACTableDeleteByPortCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Call call-back function, when delete by port.
 *
 * INPUT   : ifindex --
 *           reason  --
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
void NETACCESS_GROUP_MACTableDeleteByPortCallbackHandler(UI32_T ifindex, UI32_T reason)
{
    return;
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_GROUP_MACTableDeleteByVidCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Call call-back function, when delete by vid.
 *
 * INPUT   : vid -- which vlan id
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
void NETACCESS_GROUP_MACTableDeleteByVidCallbackHandler(UI32_T vid)
{
    return;
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_GROUP_MACTableDeleteByVIDnPortCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Call call-back function, when delete by vid+port.
 *
 * INPUT   : vid     -- which vlan id
 *           ifindex --
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
void NETACCESS_GROUP_MACTableDeleteByVIDnPortCallbackHandler(UI32_T vid, UI32_T ifindex)
{
    return;
}


BOOL_T NETACCESS_GROUP_SetStaticMacCheckCallbackHandler(UI32_T vid, UI8_T *mac, UI32_T lport)
{
    BOOL_T ret = TRUE;

#if (SYS_CPNT_ADD == TRUE)
    ret &= ADD_MGR_AMTR_SetStaticMacCheck_CallBack(vid, mac, lport);
#endif
    return ret;
}

void NETACCESS_GROUP_EditAddrEntryCallbackHandler(UI32_T vid, UI8_T *mac, UI32_T lport, BOOL_T is_age)
{
#if (SYS_CPNT_ADD == TRUE)
    ADD_MGR_AMTR_EditAddrNotify_CallBack(vid, mac, lport, is_age);
#endif
}

/* LOCAL SUBPROGRAM BODIES
 */
/*------------------------------------------------------------------------------
 * ROUTINE NAME : NETACCESS_GROUP_Mgr_Thread_Function_Entry
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
static void NETACCESS_GROUP_Mgr_Thread_Function_Entry(void* arg)
{
    SYSFUN_MsgQ_T           ipc_msgq_handle;
    L_THREADGRP_Handle_T    tg_handle = L2_L4_PROC_COMM_GetNetaccessGroupTGHandle();
    UI32_T                  member_id,received_events,local_events=SYSFUN_SYSTEM_EVENT_IPCMSG;
    SYSFUN_Msg_T            *msgbuf_p=(SYSFUN_Msg_T*)netaccess_group_mgrtd_ipc_buf;
    BOOL_T                  need_resp;
    AMTR_TYPE_AddrEntry_T   addr_buf [AMTRDRV_TYPE_MAX_ENTRIES_IN_ONE_PACKET+1];
    UI32_T  num_of_entries;
    void*   tmid;
#define NETACCESS_GROUP_PASS_COUNT  5
    static int pass_count = 0;
    /* join the thread group of CSCGroup1
     */
    if(L_THREADGRP_Join(tg_handle, SYS_BLD_NETACCESS_GROUP_MGR_THREAD_PRIORITY, &member_id)==FALSE)
    {
        printf("%s: L_THREADGRP_Join fail.\n", __FUNCTION__);
        return;
    }

    /* create mgr ipc message queue handle
     */
    if(SYSFUN_CreateMsgQ(SYS_BLD_NETACCESS_GROUP_IPCMSGQ_KEY, SYSFUN_MSGQ_BIDIRECTIONAL, &ipc_msgq_handle)!=SYSFUN_OK)
    {
        printf("%s: SYSFUN_CreateMsgQ fail.\n", __FUNCTION__);
        return;
    }

    tmid = SYSFUN_PeriodicTimer_Create();
    SYSFUN_PeriodicTimer_Start(tmid, NETACCESS_TIMER_TICKS1SEC, L2_L4_EVENT_TIMER);

    while(1){

        SYSFUN_ReceiveEvent (SYSFUN_SYSTEM_EVENT_IPCFAIL|
                             SYSFUN_SYSTEM_EVENT_IPCMSG |
                             SYSFUN_SYSTEM_EVENT_SET_TRANSITION_MODE|
#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
                             SYSFUN_SYSTEM_EVENT_SW_WATCHDOG |
#endif
                             L2_L4_EVENT_TIMER |
                             AMTR_TYPE_EVENT_ADDRESS_OPERATION|
                             AMTR_TYPE_EVENT_ADDRESS_AGINGOUT_OPERATION,
                             SYSFUN_EVENT_WAIT_ANY,
                             (local_events==0)?SYSFUN_TIMEOUT_WAIT_FOREVER:SYSFUN_TIMEOUT_NOWAIT,
                             &received_events);
        local_events |= received_events;

        if(local_events & SYSFUN_SYSTEM_EVENT_SET_TRANSITION_MODE){

            NETACCESS_GROUP_SetTransitionMode();
            local_events ^= SYSFUN_SYSTEM_EVENT_SET_TRANSITION_MODE;

            if(local_events & SYSFUN_SYSTEM_EVENT_IPCFAIL)
                local_events ^= SYSFUN_SYSTEM_EVENT_IPCFAIL;
        }


        /* handle IPCMSG
         */
        if(local_events & SYSFUN_SYSTEM_EVENT_IPCMSG){

            if(SYSFUN_ReceiveMsg(ipc_msgq_handle, 0, SYSFUN_TIMEOUT_NOWAIT,
                NETACCESS_GROUP_PMGR_MSGBUF_MAX_REQUIRED_SIZE, msgbuf_p)==SYSFUN_OK){

                if(L_THREADGRP_Execution_Request(tg_handle, member_id)!=TRUE)
                    printf("%s: L_THREADGRP_Execution_Request fail.\n", __FUNCTION__);

                /* handle request message based on cmd
                 */
                switch(msgbuf_p->cmd){
                    /* module id cmd
                     */
#if (SYS_CPNT_AMTR == TRUE)
                    case SYS_MODULE_AMTR:
                        need_resp=AMTR_MGR_HandleIPCReqMsg(msgbuf_p);
                        break;
#endif

#if (SYS_CPNT_PORT_SECURITY == TRUE)&& (SYS_CPNT_NETWORK_ACCESS == FALSE)
                    case SYS_MODULE_PSEC:
                        need_resp=PSEC_MGR_HandleIPCReqMsg(msgbuf_p);
                        break;
#endif

#if (SYS_CPNT_ADD == TRUE)
                    case SYS_MODULE_ADD:
                        need_resp = ADD_MGR_HandleIPCReqMsg(msgbuf_p);
                        break;
#endif

#if (SYS_CPNT_NETACCESS == TRUE)
                    case SYS_MODULE_NETACCESS:
                        need_resp=NETACCESS_MGR_HandleIPCReqMsg(msgbuf_p);
                        break;
#endif

#if (SYS_CPNT_DOS == TRUE)
                    case SYS_MODULE_DOS:
                        need_resp = DOS_MGR_HandleIPCReqMsg(msgbuf_p);
                        break;
#endif

#if (SYS_CPNT_APP_FILTER == TRUE)
                    case SYS_MODULE_AF:
                        need_resp = AF_MGR_HandleIPCReqMsg(msgbuf_p);
                        break;
#endif /* #if (SYS_CPNT_APP_FILTER == TRUE) */

                    case SYS_MODULE_SYS_CALLBACK:
                        /* SYS_CALLBACK ipc message can only be uni-direction
                         * just set need_resp as FALSE
                         */
                        need_resp=FALSE;
                        NETACCESS_GROUP_HandleSysCallbackIPCMsg(msgbuf_p);
                        break;

                    case SYS_MODULE_BACKDOOR:
                        need_resp=BACKDOOR_MGR_HandleIPCMsg(msgbuf_p);
                        break;

#if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE)
                    case SYS_MODULE_CMGR:
                        NETACCESS_GROUP_HandleCmgrIpcMsg(msgbuf_p);
                        msgbuf_p->msg_size = 0;
                        need_resp = TRUE;
                        break;
#endif

                    /* global cmd
                     */
                    case SYS_TYPE_CMD_ENTER_MASTER_MODE:
                        NETACCESS_GROUP_EnterMasterMode();

                        /* SYS_TYPE_CMD_ENTER_MASTER_MODE
                         * need a response which contains nothing
                         */
                        msgbuf_p->msg_size=0;
                        need_resp=TRUE;

                        /* IPCFAIL might happen when a upper layer netaccess group has
                         * entered transition mode but lower layer netaccess groups still
                         * in master mode so async callback msg will keep sending
                         * from lower layer netaccess group. In this case, the IPCFAIL
                         * event can be ignored.
                         */
                        if(local_events & SYSFUN_SYSTEM_EVENT_IPCFAIL)
                            local_events ^= SYSFUN_SYSTEM_EVENT_IPCFAIL;

                        break;

                    case SYS_TYPE_CMD_ENTER_SLAVE_MODE:
                        NETACCESS_GROUP_EnterSlaveMode();

                        /* SYS_TYPE_CMD_ENTER_SLAVE_MODE
                         * need a response which contains nothing
                         */
                        msgbuf_p->msg_size=0;
                        need_resp=TRUE;

                        /* IPCFAIL might happen when a upper layer netaccess group has
                         * entered transition mode but lower layer netaccess group still
                         * in master mode so async callback msg will keep sending
                         * from lower layer netaccess group. In this case, the IPCFAIL
                         * event can be ignored.
                         */
                        if(local_events & SYSFUN_SYSTEM_EVENT_IPCFAIL)
                            local_events ^= SYSFUN_SYSTEM_EVENT_IPCFAIL;
                        break;

                    case SYS_TYPE_CMD_ENTER_TRANSITION_MODE:
                        NETACCESS_GROUP_EnterTransitionMode();

                        /* SYS_TYPE_CMD_ENTER_TRANSITION_MODE
                         * need a response which contains nothing
                         */
                        msgbuf_p->msg_size=0;
                        need_resp=TRUE;
                        break;

                    case SYS_TYPE_CMD_PROVISION_COMPLETE:
                    case SYS_TYPE_CMD_MODULE_PROVISION_COMPLETE:
                        NETACCESS_GROUP_ProvisionComplete();

                        /* SYS_TYPE_CMD_ENTER_TRANSITION_MODE
                         * need a response which contains nothing
                         */
                        msgbuf_p->msg_size=0;
                        need_resp=TRUE;
                        break;

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
                     case     SYS_TYPE_CMD_HANDLE_HOT_INSERTION:
                       NETACCESS_GROUP_HandleHotInertion(msgbuf_p);

                        msgbuf_p->msg_size=0;
                        need_resp=TRUE;
                        break;

                     case SYS_TYPE_CMD_HANDLE_HOT_REMOVAL:
                       NETACCESS_GROUP_HandleHotRemoval(msgbuf_p);

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
                        printf("%s: Invalid IPC req cmd.\n", __FUNCTION__);
                        need_resp=FALSE;
                }

                if(L_THREADGRP_Execution_Release(tg_handle, member_id)!=TRUE)
                    printf("%s: L_THREADGRP_Execution_Release fail.\n", __FUNCTION__);

                if((need_resp==TRUE) && (SYSFUN_SendResponseMsg(ipc_msgq_handle, msgbuf_p)!=SYSFUN_OK))
                    printf("%s: SYSFUN_SendResponseMsg fail.\n", __FUNCTION__);

            }else{
                local_events ^= SYSFUN_SYSTEM_EVENT_IPCMSG;
                pass_count = 4;
            }
        } /* end of if(local_events & SYSFUN_SYSTEM_EVENT_IPCMSG) */

        if(local_events&(AMTR_TYPE_EVENT_ADDRESS_OPERATION|AMTR_TYPE_EVENT_ADDRESS_AGINGOUT_OPERATION)){
            /* the time cost of learning is heavy. it will delay the speed of dealing message
                      * wil create a new thread for get message . all actions of modifing om  must be in one thread.
                      */
            pass_count ++;
            if(pass_count == NETACCESS_GROUP_PASS_COUNT){

                if(local_events&AMTR_TYPE_EVENT_ADDRESS_OPERATION){
#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == FALSE)
                    num_of_entries = AMTRDRV_MGR_ProcessMasterNABuffer(addr_buf,AMTRDRV_TYPE_MAX_ENTRIES_IN_ONE_PACKET);

                    if(num_of_entries)
                        AMTR_MGR_AnnounceNewAddress_CallBack(num_of_entries, addr_buf);
                    else{
                        local_events ^= AMTR_TYPE_EVENT_ADDRESS_OPERATION;
                    }
#else
    #ifdef NETACCESS_GROUP_DEBUG
                    printf("%s(): Shouldn't receive this event @ line %d\r\n", __FUNCTION__, __LINE__);
    #endif
                    local_events ^= AMTR_TYPE_EVENT_ADDRESS_AGINGOUT_OPERATION;
#endif
                }

                if(local_events&AMTR_TYPE_EVENT_ADDRESS_AGINGOUT_OPERATION){
#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == FALSE)
                    num_of_entries = AMTRDRV_MGR_ProcessAgingOutEntries(addr_buf,AMTRDRV_TYPE_MAX_ENTRIES_IN_ONE_PACKET);

                    if(num_of_entries)
                        AMTR_MGR_AgingOut_CallBack(num_of_entries,addr_buf);
                    else{
                        local_events ^= AMTR_TYPE_EVENT_ADDRESS_AGINGOUT_OPERATION;
                    }
#else
    #ifdef NETACCESS_GROUP_DEBUG
                    printf("%s(): Shouldn't receive this event @ line %d\r\n", __FUNCTION__, __LINE__);
    #endif
                    local_events ^= AMTR_TYPE_EVENT_ADDRESS_AGINGOUT_OPERATION;
#endif
                }
                pass_count = 0;
            }
        }

        if (local_events & L2_L4_EVENT_TIMER){

            /* request thread group execution permission
             */
            if (L_THREADGRP_Execution_Request(tg_handle, member_id) != TRUE){
                printf("\n%s: L_THREADGRP_Execution_Request fail.\n",
                    __FUNCTION__);
            }

#if (SYS_CPNT_NETACCESS == TRUE)
            NETACCESS_MGR_ProcessTimeoutEvent();
#endif /* #if (SYS_CPNT_NETACCESS == TRUE) */

        #if (SYS_CPNT_ADD == TRUE)
            ADD_MGR_ProcessTimeoutEvent();
        #endif

            /* release thread group execution permission
             */
            if (L_THREADGRP_Execution_Release(tg_handle, member_id) != TRUE)
            {
                printf("\n%s: L_THREADGRP_Execution_Release fail.\n",
                    __FUNCTION__);
            }

            local_events ^= L2_L4_EVENT_TIMER;
        }

        if((local_events & SYSFUN_SYSTEM_EVENT_IPCFAIL) &&
            !(local_events & SYSFUN_SYSTEM_EVENT_IPCMSG))
            local_events ^= SYSFUN_SYSTEM_EVENT_IPCFAIL;

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
        if (local_events & SYSFUN_SYSTEM_EVENT_SW_WATCHDOG)
        {
            SW_WATCHDOG_MGR_ResetTimer(SW_WATCHDOG_NETACCESS_GROUP);
            local_events ^= SYSFUN_SYSTEM_EVENT_SW_WATCHDOG;
        }
#endif

    } /* end of while(1) */
    L_THREADGRP_Leave(tg_handle, member_id);
}

/* LOCAL SUBPROGRAM BODIES
 */


/* LOCAL SUBPROGRAM BODIES
 */
/*------------------------------------------------------------------------------
 * ROUTINE NAME : NETACCESS_GROUP_Mgr_Thread_Hash2hisam_Function_Entry
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    AMTR_MGR_SyncHashToHisam : when a mac that is  00-ab-cd-ef-gh-rh and a*b*c*d*e*f*g*h*r*h != 0,
 *    the time cost of this function is heavy.So remove this function from NETACCESS_GROUP_Mgr_Thread_Function_Entry,
 *    to share the loadbalance . you can read the bug discriptiion (ES4827G-FLF-ZZ-00464)
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
static void NETACCESS_GROUP_Mgr_Thread_Hash2hisam_Function_Entry(void* arg)
{
    UI32_T  received_events;
    void *  amtr_tmid;
    int count;
    /* AMTR timer event
     */
    amtr_tmid = SYSFUN_PeriodicTimer_Create();

    SYSFUN_PeriodicTimer_Start(amtr_tmid,SYS_BLD_AMTR_MGR_SYNC_HASH2HISAM_TICKS,AMTR_MGR_EVENT_TIMER);

    while(1){

        SYSFUN_ReceiveEvent (AMTR_MGR_EVENT_TIMER
#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
                            |SYSFUN_SYSTEM_EVENT_SW_WATCHDOG
#endif
                             ,SYSFUN_EVENT_WAIT_ANY,
                             SYSFUN_TIMEOUT_WAIT_FOREVER,
                             &received_events);

        if(received_events&AMTR_MGR_EVENT_TIMER){

            SYSFUN_PeriodicTimer_Stop(amtr_tmid);

            count = AMTR_MGR_SyncHashToHisam();

            if(count < AMTR_TYPE_SYNC2HISAM_NUM)
                SYSFUN_PeriodicTimer_Restart(amtr_tmid,2*SYS_BLD_AMTR_MGR_SYNC_HASH2HISAM_TICKS);
            else
                SYSFUN_PeriodicTimer_Restart(amtr_tmid,SYS_BLD_AMTR_MGR_SYNC_HASH2HISAM_TICKS);
        #if (SYS_CPNT_AMTR_MAC_NOTIFY == TRUE)
            AMTR_MGR_MacNotifyProcessQueue(SYSFUN_GetSysTick());
        #endif
        #if (SYS_CPNT_MLAG == TRUE)
            AMTR_MGR_MlagMacNotifyProcessQueue(SYSFUN_GetSysTick());
        #endif
        #if (SYS_CPNT_OVSVTEP == TRUE)
            AMTR_MGR_OvsMacNotifyProcessQueue(SYSFUN_GetSysTick());
        #endif

            received_events = 0;
        }

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
        if (received_events & SYSFUN_SYSTEM_EVENT_SW_WATCHDOG)
        {
            SW_WATCHDOG_MGR_ResetTimer(SW_WATCHDOG_NETACCESS_NMTR_HASH2HISAM);
            received_events ^= SYSFUN_SYSTEM_EVENT_SW_WATCHDOG;
        }
#endif

    } /* while(1) */
}

/* LOCAL SUBPROGRAM BODIES
 */
/*------------------------------------------------------------------------------
 * ROUTINE NAME : NETACCESS_NMTR_Thread_Function_Entry
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This is the entry function for the NMTR thread of CSCGroup1.
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
static void NETACCESS_NMTR_Thread_Function_Entry(void* arg)
{
    SYSFUN_MsgQ_T           ipc_msgq_handle;
    UI32_T                  received_events,local_events=SYSFUN_SYSTEM_EVENT_IPCMSG;
    SYSFUN_Msg_T            *msgbuf_p=(SYSFUN_Msg_T*)netaccess_nmtr_ipc_buf;
    BOOL_T                  need_resp;

    /* create mgr ipc message queue handle*/
    if(SYSFUN_CreateMsgQ(SYS_BLD_NETACCESS_NMTR_IPCMSGQ_KEY, SYSFUN_MSGQ_BIDIRECTIONAL, &ipc_msgq_handle)!=SYSFUN_OK)
    {
        printf("%s: SYSFUN_CreateMsgQ fail.\n", __FUNCTION__);
        return;
    }

    while(1)
    {
        SYSFUN_ReceiveEvent (SYSFUN_SYSTEM_EVENT_IPCFAIL |
#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
                             SYSFUN_SYSTEM_EVENT_SW_WATCHDOG |
#endif
                             SYSFUN_SYSTEM_EVENT_IPCMSG ,
                             SYSFUN_EVENT_WAIT_ANY,
                             (local_events==0)?SYSFUN_TIMEOUT_WAIT_FOREVER:SYSFUN_TIMEOUT_NOWAIT,
                             &received_events);
        local_events |= received_events;

        if(local_events & SYSFUN_SYSTEM_EVENT_IPCMSG)
        {
            if(SYSFUN_ReceiveMsg(ipc_msgq_handle, 0, SYSFUN_TIMEOUT_NOWAIT,
                NETACCESS_GROUP_PMGR_MSGBUF_MAX_REQUIRED_SIZE, msgbuf_p)==SYSFUN_OK)
            {
                /* handle request message based on cmd  */
                switch(msgbuf_p->cmd)
                {
                    /* module id cmd    */
#if (SYS_CPNT_NMTR == TRUE)
                    case SYS_MODULE_NMTR:
                        need_resp=NMTR_MGR_HandleIPCReqMsg(msgbuf_p);
                        break;
#endif
                    case SYS_MODULE_SYS_CALLBACK:
                        /* SYS_CALLBACK ipc message can only be uni-direction
                                            * just set need_resp as FALSE    */
                        need_resp=FALSE;
                        NETACCESS_HandleNMTRSysCallbackIPCMsg(msgbuf_p);
                        break;

                    case SYS_MODULE_BACKDOOR:
                        need_resp=BACKDOOR_MGR_HandleIPCMsg(msgbuf_p);
                        break;

                    default:
                        printf("%s: Invalid IPC req cmd %d.\n", __FUNCTION__,msgbuf_p->cmd);
                        need_resp=FALSE;
                }
                if((need_resp==TRUE) && (SYSFUN_SendResponseMsg(ipc_msgq_handle, msgbuf_p)!=SYSFUN_OK))
                    printf("%s: SYSFUN_SendResponseMsg fail.\n", __FUNCTION__);

            } /* end of if(SYSFUN_ReceiveMsg... */
            else
            {
                local_events ^= SYSFUN_SYSTEM_EVENT_IPCMSG;
            }
        } /* end of if(local_events & SYSFUN_SYSTEM_EVENT_IPCMSG) */

        /* handle IPC Async Callback fail when IPC Msgq is empty */
        if((local_events & SYSFUN_SYSTEM_EVENT_IPCFAIL) &&
            !(local_events & SYSFUN_SYSTEM_EVENT_IPCMSG))
        {
            /* read fail info from IPCFAIL   */

            /* do recovery action  */
            local_events ^= SYSFUN_SYSTEM_EVENT_IPCFAIL;
        }

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
        if(local_events & SYSFUN_SYSTEM_EVENT_SW_WATCHDOG)
        {
            SW_WATCHDOG_MGR_ResetTimer(SW_WATCHDOG_NETACCESS_NMTR);
            local_events ^= SYSFUN_SYSTEM_EVENT_SW_WATCHDOG;
        }
#endif

    } /* end of while(1) */
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : NETACCESS_GROUP_SetTransitionMode
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
static void NETACCESS_GROUP_SetTransitionMode(void)
{
#if (SYS_CPNT_AMTR == TRUE)
    AMTR_INIT_SetTransitionMode();
#endif

#if (SYS_CPNT_NMTR == TRUE)
    NMTR_INIT_SetTransitionMode();
#endif

#if (SYS_CPNT_ADD == TRUE)
    ADD_INIT_SetTransitionMode();
#endif

#if (SYS_CPNT_PORT_SECURITY == TRUE)
    PSEC_INIT_SetTransitionMode();
#endif

#if (SYS_CPNT_NETACCESS == TRUE)
    NETACCESS_INIT_SetTransitionMode();
#endif

#if (SYS_CPNT_DOS == TRUE)
    DOS_INIT_SetTransitionMode();
#endif

#if (SYS_CPNT_APP_FILTER == TRUE)
    AF_MGR_SetTransitionMode();
#endif /* #if (SYS_CPNT_APP_FILTER == TRUE)*/
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : NETACCESS_GROUP_EnterTransitionMode
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
static void NETACCESS_GROUP_EnterTransitionMode(void)
{
#if (SYS_CPNT_AMTR == TRUE)

    AMTR_INIT_EnterTransitionMode();
#endif

#if (SYS_CPNT_NMTR == TRUE)
    NMTR_INIT_EnterTransitionMode();
#endif

#if (SYS_CPNT_ADD == TRUE)
    ADD_INIT_EnterTransitionMode();
#endif

#if (SYS_CPNT_PORT_SECURITY == TRUE)
    PSEC_INIT_EnterTransitionMode();
#endif

#if (SYS_CPNT_NETACCESS == TRUE)
    NETACCESS_INIT_EnterTransitionMode();
#endif

#if (SYS_CPNT_DOS == TRUE)
    DOS_INIT_EnterTransitionMode();
#endif

#if (SYS_CPNT_APP_FILTER == TRUE)
    AF_MGR_EnterTransitionMode();
#endif /* #if (SYS_CPNT_APP_FILTER == TRUE)*/
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : NETACCESS_GROUP_EnterMasterMode
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
static void NETACCESS_GROUP_EnterMasterMode(void)
{
#if (SYS_CPNT_AMTR == TRUE)
    AMTR_INIT_EnterMasterMode();
#endif

#if (SYS_CPNT_NMTR == TRUE)
    NMTR_INIT_EnterMasterMode();
#endif

#if (SYS_CPNT_ADD == TRUE)
    ADD_INIT_EnterMasterMode();
#endif

#if (SYS_CPNT_PORT_SECURITY == TRUE)
    PSEC_INIT_EnterMasterMode();
#endif

#if (SYS_CPNT_NETACCESS == TRUE)
    NETACCESS_INIT_EnterMasterMode();
#endif

#if (SYS_CPNT_DOS == TRUE)
    DOS_INIT_EnterMasterMode();
#endif

#if (SYS_CPNT_APP_FILTER == TRUE)
    AF_MGR_EnterMasterMode();
#endif /* #if (SYS_CPNT_APP_FILTER == TRUE)*/
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : NETACCESS_GROUP_EnterSlaveMode
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
static void NETACCESS_GROUP_EnterSlaveMode(void)
{
#if (SYS_CPNT_AMTR == TRUE)
    AMTR_INIT_EnterSlaveMode();
#endif

#if (SYS_CPNT_NMTR == TRUE)
    NMTR_INIT_EnterSlaveMode();
#endif

#if (SYS_CPNT_ADD == TRUE)
    ADD_INIT_EnterSlaveMode();
#endif

#if (SYS_CPNT_PORT_SECURITY == TRUE)
    PSEC_INIT_EnterSlaveMode();
#endif

#if (SYS_CPNT_NETACCESS == TRUE)
    NETACCESS_INIT_EnterSlaveMode();
#endif

#if (SYS_CPNT_DOS == TRUE)
    DOS_INIT_EnterSlaveMode();
#endif

#if (SYS_CPNT_APP_FILTER == TRUE)
    AF_MGR_EnterSlaveMode();
#endif /* #if (SYS_CPNT_APP_FILTER == TRUE)*/
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : NETACCESS_GROUP_ProvisionComplete
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will invoke all provision complete function in CSCGroup1.
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
static void NETACCESS_GROUP_ProvisionComplete(void)
{
#if (SYS_CPNT_ADD == TRUE)
    ADD_MGR_CLI_ProvisionComplete_CallBack();
#endif

#if (SYS_CPNT_DOS == TRUE)
    DOS_INIT_ProvisionComplete();
#endif

   return;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : NETACCESS_HandleNMTRSysCallbackIPCMsg
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will handle all NMTR callbacks from IPC messages in CSCGroup1.
 *
 * INPUT:
 *    msgbuf_p  -- NMTR  SYS_CALLBACK IPC message
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
static void NETACCESS_HandleNMTRSysCallbackIPCMsg(SYSFUN_Msg_T *msgbuf_p)
{
    SYS_CALLBACK_MGR_Msg_T *sys_cbmsg_p = (SYS_CALLBACK_MGR_Msg_T*)msgbuf_p->msg_buf;

    switch(sys_cbmsg_p->callback_event_id)
    {
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_UPDATE_LOCAL_NMTRDRV_STATS:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                NETACCESS_GROUP_UpdateNmtrdrvStatsCallbackHandler);
            break;

        default:
            SYSFUN_Debug_Printf("%s: received callback_event that is not handled(%d)\n",
                __FUNCTION__, sys_cbmsg_p->callback_event_id);
            break;
    }

}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : NETACCESS_GROUP_HandleSysCallbackIPCMsg
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
static void NETACCESS_GROUP_HandleSysCallbackIPCMsg(SYSFUN_Msg_T *msgbuf_p)
{
    SYS_CALLBACK_MGR_Msg_T *sys_cbmsg_p = (SYS_CALLBACK_MGR_Msg_T*)msgbuf_p->msg_buf;

    switch(sys_cbmsg_p->callback_event_id)
    {
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_PORT_LINK_DOWN:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                NETACCESS_GROUP_LportLinkDownCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_TRUNK_MEMBER_ADD_1ST:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                NETACCESS_GROUP_TrunkMemberAdd1stCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_TRUNK_MEMBER_ADD:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                NETACCESS_GROUP_TrunkMemberAddCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_TRUNK_MEMBER_DELETE:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                NETACCESS_GROUP_TrunkMemberDelCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_TRUNK_MEMBER_DELETE_LST:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                NETACCESS_GROUP_TrunkMemberDeleteLstCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_TRUNK_MEMBER_ACTIVE:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                NETACCESS_GROUP_TrunkMemberActiveCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_PORT_ADMIN_ENABLE:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                NETACCESS_GROUP_LportAdminEnableCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_PORT_ADMIN_DISABLE:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                NETACCESS_GROUP_LportAdminDisableCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_DESTROY:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                NETACCESS_GROUP_VlanDestroyCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_MEMBER_ADD:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                NETACCESS_GROUP_VlanMemberAddCallbackHandler);
            break;
#if (SYS_CPNT_REFINE_IPC_MSG == TRUE)
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_LIST:
          SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                      NETACCESS_GROUP_VlanListCallbackHandler);
                  break;
#endif

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_MEMBER_DELETE:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                NETACCESS_GROUP_VlanMemberDeleteCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_NEW_MAC_ADDRESS:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                NETACCESS_GROUP_NewAddressCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_MAC_AGING_OUT:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                NETACCESS_GROUP_AgingOutCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_SECURITY_CHECK:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                NETACCESS_GROUP_SecurityCheckCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_UPORT_LINK_UP:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                NETACCESS_GROUP_UPortLinkUpCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_UPORT_LINK_DOWN:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                NETACCESS_GROUP_UPortLinkDownCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_UPORT_ADMIN_ENABLE   :
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                NETACCESS_GROUP_UPortAdminEnableCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_UPORT_ADMIN_DISABLE:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                NETACCESS_GROUP_UPortAdminDisableCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_PORT_LEARNING_STATUS_CHANGED:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                NETACCESS_GROUP_PortLearningStatusChangedCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_RECEIVE_DOT1X_PACKET:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                NETACCESS_GROUP_Dot1xPacketCallbackHandler);
            break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_UPORT_DOT1X_EFFECTIVE_OPER_STATUS_CHANGED:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                NETACCESS_GROUP_UPortDot1xEffectiveOperStatusChangedCallbackHandler);
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_ANNOUNCE_RADIUS_PACKET:
            /* should be handled by DOT1X_MGR_AnnounceRADIUSPacket
             */
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                NETACCESS_GROUP_AnnounceRadiusPacketCallbackHandler);
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_ANNOUNCE_RADA_AUTH_RESULT:
            /* should be handled by NETACCESS_MGR_AnnounceRadiusAuthorizedResult
             */
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                NETACCESS_GROUP_AnnounceRadaAuthPacketCallbackHandler);
            break;
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_ANNOUNCE_DOT1X_AUTH_RESULT:
            /* should be handled by NETACCESS_MGR_AnnounceDot1xAuthorizedResult
             */
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                NETACCESS_GROUP_AnnounceDot1xAuthPacketCallbackHandler);
            break;

#if (SYS_CPNT_ADD == TRUE)
        case SYS_CALLBACK_MGR_CALLBACKEVENTID_TELEPHONE_DETECT:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                NETACCESS_GROUP_TelephoneDetectCallbackHandler);
        break;
#endif

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_ACL_DELETED:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                NETACCESS_GROUP_AnnounceACLDeletedHandler);
        break;

        case SYS_CALLBACK_MGR_CALLBACKEVENTID_POLICY_MAP_DELETED:
            SYS_CALLBACK_MGR_HandleIPCMsgAndCallback(msgbuf_p,
                NETACCESS_GROUP_AnnouncePolicyMapDeletedHandler);
        break;
        default:
            SYSFUN_Debug_Printf("%s: received callback_event that is not handled(%d)\n",
                __FUNCTION__, sys_cbmsg_p->callback_event_id);
            break;
    }
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_GROUP_LportLinkDownCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the callback event happening when the link is down.
 *
 * INPUT   : ifindex -- which logical port
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
static void NETACCESS_GROUP_LportLinkDownCallbackHandler(UI32_T ifindex)
{
#if (SYS_CPNT_AMTR == TRUE)
    AMTR_MGR_PortLinkDown_CallBack(ifindex);
#endif
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_GROUP_TrunkMemberAdd1stCallbackHandler
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
static void NETACCESS_GROUP_TrunkMemberAdd1stCallbackHandler(UI32_T trunk_ifindex,
                                                             UI32_T member_ifindex)
{
#if (SYS_CPNT_AMTR == TRUE)
    AMTR_MGR_PortAddIntoTrunk_CallBack(trunk_ifindex, member_ifindex,TRUE);
#if (SYS_CPNT_AMTR_MAC_NOTIFY == TRUE)
    AMTR_MGR_MacNotifyAddFstTrkMbr_CallBack(trunk_ifindex, member_ifindex);
#endif
#endif
#if (SYS_CPNT_NMTR == TRUE)
    NMTR_MGR_TrunkMemberAdd1st_Callback(trunk_ifindex, member_ifindex);
#endif
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_GROUP_TrunkMemberAddCallbackHandler
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
static void NETACCESS_GROUP_TrunkMemberAddCallbackHandler(UI32_T trunk_ifindex,
                                                          UI32_T member_ifindex)
{
#if (SYS_CPNT_AMTR == TRUE)
    AMTR_MGR_PortAddIntoTrunk_CallBack(trunk_ifindex, member_ifindex,FALSE);
#endif
#if (SYS_CPNT_NMTR == TRUE)
    NMTR_MGR_TrunkMemberAdd_Callback(trunk_ifindex, member_ifindex);
#endif
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_GROUP_TrunkMemberDelCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the callback event happening when a logical port is
 *           removed from a trunk.
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
static void NETACCESS_GROUP_TrunkMemberDelCallbackHandler(UI32_T trunk_ifindex,
                                                          UI32_T member_ifindex)
{
#if (SYS_CPNT_AMTR_MAC_NOTIFY == TRUE)
    AMTR_MGR_MacNotifyDelTrkMbr_CallBack(trunk_ifindex, member_ifindex);
#endif
}
/*-----------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_GROUP_TrunkMemberDeleteLstCallbackHandler
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
static void NETACCESS_GROUP_TrunkMemberDeleteLstCallbackHandler(UI32_T trunk_ifindex,
                                                                UI32_T member_ifindex)
{
#if (SYS_CPNT_AMTR == TRUE)
    AMTR_MGR_DestroyTrunk_CallBack(trunk_ifindex, member_ifindex);
#if (SYS_CPNT_AMTR_MAC_NOTIFY == TRUE)
    AMTR_MGR_MacNotifyDelLstTrkMbr_CallBack(trunk_ifindex, member_ifindex);
#endif
#endif
#if (SYS_CPNT_NMTR == TRUE)
    NMTR_MGR_TrunkMemberDeleteLst_Callback(trunk_ifindex, member_ifindex);
#endif
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_GROUP_TrunkMemberActiveCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the callback event happening when member port is active
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
static void NETACCESS_GROUP_TrunkMemberActiveCallbackHandler(UI32_T trunk_ifindex,
                                                             UI32_T member_ifindex)
{
#if (SYS_CPNT_AMTR == TRUE)
    AMTR_MGR_PortAddIntoTrunk_CallBack(trunk_ifindex, member_ifindex,FALSE);
#endif
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_GROUP_LportAdminEnableCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the callback event happening when port is enabled.
 *
 * INPUT   : ifindex -- which logical port
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
static void NETACCESS_GROUP_LportAdminEnableCallbackHandler(UI32_T ifindex)
{
#if (SYS_CPNT_PORT_SECURITY == TRUE)&& (SYS_CPNT_NETWORK_ACCESS == FALSE)
    PSEC_MGR_NoShutdown_CallBack(ifindex);
#endif
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_GROUP_LportAdminDisableCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the callback event happening when port is disabled.
 *
 * INPUT   : ifindex -- which logical port
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
static void NETACCESS_GROUP_LportAdminDisableCallbackHandler(UI32_T ifindex)
{
#if (SYS_CPNT_AMTR == TRUE)
    AMTR_MGR_PortAdminDisable_CallBack(ifindex);
#endif
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_GROUP_VlanDestroyCallbackHandler
 *-----------------------------------------------------------------------------
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
 *-----------------------------------------------------------------------------
 */
static void NETACCESS_GROUP_VlanDestroyCallbackHandler(UI32_T vid_ifindex,
                                                       UI32_T vlan_status)
{
#if (SYS_CPNT_AMTR == TRUE)
    AMTR_MGR_DestroyVlan_CallBack(vid_ifindex, vlan_status);
#endif
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_GROUP_VlanMemberAddCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the callback event happening when a vlan is deleted.
 *
 * INPUT   : vid_ifindex   -- specify which vlan's member set to disjoin from
 *           lport_ifindex -- sepcify which lport to disjoin from the member set
 *           vlan_status   -- VAL_dot1qVlanStatus_other
 *                            VAL_dot1qVlanStatus_permanent
 *                            VAL_dot1qVlanStatus_dynamicGvrp
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
static void NETACCESS_GROUP_VlanMemberAddCallbackHandler(
    UI32_T vid_ifindex, UI32_T lport_ifindex, UI32_T vlan_status)
{
#if (SYS_CPNT_NETACCESS == TRUE)
    NETACCESS_MGR_VlanMemberAdd_CallBack(vid_ifindex, lport_ifindex, vlan_status);
#endif
}

#if (SYS_CPNT_REFINE_IPC_MSG == TRUE)
/*-----------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_GROUP_VlanListCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the callback event happening when ports added to vlan/ports removed from vlans.
 *
 * INPUT   : subid -- the detail action to do
 *                  msg--- msg data
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
static void NETACCESS_GROUP_VlanListCallbackHandler(UI32_T subid,SYS_CALLBACK_MGR_REFINEList_CBData_T*msg)
{


    switch(subid)
    {
      case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_MEMBER_ADD:

       NETACCESS_MGR_VlanList_CallBack((UI8_T*)msg,NETACCESS_PORT_ADDED);
     break;

      case SYS_CALLBACK_MGR_CALLBACKEVENTID_VLAN_MEMBER_DELETE:

         NETACCESS_MGR_VlanList_CallBack((UI8_T*)msg,NETACCESS_PORT_REMOVED);
      break;

      default:
      break;
   }

}


#endif
/*-----------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_GROUP_VlanMemberDeleteCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the callback event happening when a vlan is deleted.
 *
 * INPUT   : vid_ifindex   -- specify which vlan's member set to disjoin from
 *           lport_ifindex -- sepcify which lport to disjoin from the member set
 *           vlan_status   -- VAL_dot1qVlanStatus_other
 *                            VAL_dot1qVlanStatus_permanent
 *                            VAL_dot1qVlanStatus_dynamicGvrp
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
static void NETACCESS_GROUP_VlanMemberDeleteCallbackHandler(UI32_T vid_ifindex,
                                                            UI32_T lport_ifindex,
                                                            UI32_T vlan_status)
{
#if (SYS_CPNT_AMTR == TRUE)
    AMTR_MGR_VlanMemberDelete_CallBack(vid_ifindex, lport_ifindex, vlan_status);
#endif

#if (SYS_CPNT_NETACCESS == TRUE)
    NETACCESS_MGR_VlanMemberDelete_CallBack(vid_ifindex, lport_ifindex, vlan_status);
#endif

#if (SYS_CPNT_ADD == TRUE)
    ADD_MGR_VLAN_VlanMemberDelete_CallBack(vid_ifindex, lport_ifindex, vlan_status);
#endif
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_GROUP_NewAddressCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the callback event happening when there is any new address.
 *
 * INPUT   : num_of_entries -- how many records in the buffer
 *           addr_entry     -- addresses buffer
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
static void NETACCESS_GROUP_NewAddressCallbackHandler(UI32_T num_of_entries,
                                                      AMTR_TYPE_AddrEntry_T addr_buf[])
{
#if (SYS_CPNT_AMTR == TRUE) && (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == TRUE)
    AMTR_MGR_AnnounceNewAddress_CallBack(num_of_entries, addr_buf);
#endif
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_GROUP_AgingOutCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the callback event happening when any address is aged out.
 *
 * INPUT   : num_of_entries -- how many records in the buffer
 *           addr_entry     -- addresses buffer
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
static void NETACCESS_GROUP_AgingOutCallbackHandler(UI32_T num_of_entries,
                                                    AMTR_TYPE_AddrEntry_T addr_buf[])
{
#if (SYS_CPNT_AMTR == TRUE) && (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == TRUE)
    AMTR_MGR_AgingOut_CallBack(num_of_entries, addr_buf);
#endif
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_GROUP_SecurityCheckCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the callback event happening when network interface has
 *           received a NA packet.
 *
 * INPUT   : src_lport  --
 *           vid        --
 *           src_mac    --
 *           dst_mac    --
 *           ether_type --
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
static void NETACCESS_GROUP_SecurityCheckCallbackHandler(UI32_T src_lport,
                                                         UI16_T vid,
                                                         UI8_T *src_mac,
                                                         UI8_T *dst_mac,
                                                         UI16_T ether_type)
{
#if 0 /* move to sys_callback */
#if (SYS_CPNT_AMTR == TRUE)
    AMTR_MGR_SecurityCheck_Callback(src_lport, vid, src_mac, dst_mac, ether_type);
#endif
#endif
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_GROUP_UPortLinkUpCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE: CallBack function
 * INPUT:   unit,port
 * OUTPUT: None.
 * RETURN: None.
 * NOTES:  None.
 *-----------------------------------------------------------------------------
 */
static void NETACCESS_GROUP_UPortLinkUpCallbackHandler(UI32_T unit, UI32_T port)
{
#if (SYS_CPNT_NETACCESS == TRUE)
    NETACCESS_MGR_PortLinkUp_CallBack(unit, port);
#else
    #if (SYS_CPNT_DOT1X == TRUE)
        DOT1X_MGR_PortLinkUp_CallBack(unit, port);
    #endif
#endif

#if (SYS_CPNT_ADD == TRUE)
    ADD_MGR_PortLinkUp_CallBack(unit, port);
#endif

}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_GROUP_UPortLinkDownCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE: CallBack function
 * INPUT:   unit,port
 * OUTPUT: None.
 * RETURN: None.
 * NOTES:  None.
 *-----------------------------------------------------------------------------
 */
static void NETACCESS_GROUP_UPortLinkDownCallbackHandler(UI32_T unit, UI32_T port)
{
#if (SYS_CPNT_NETACCESS == TRUE)
    NETACCESS_MGR_PortLinkDown_CallBack(unit, port);
#endif

#if (SYS_CPNT_ADD == TRUE)
    ADD_MGR_PortLinkDown_CallBack(unit, port);
#endif

}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_GROUP_UPortAdminEnableCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE: CallBack function
 * INPUT:   unit,port
 * OUTPUT: None.
 * RETURN: None.
 * NOTES:  None.
 *-----------------------------------------------------------------------------
 */
static void NETACCESS_GROUP_UPortAdminEnableCallbackHandler(UI32_T unit, UI32_T port)
{
#if (SYS_CPNT_NETACCESS == TRUE)
    NETACCESS_MGR_UPortAdminEnable_CallBack(unit, port);
#else
    #if (SYS_CPNT_DOT1X == TRUE)
        DOT1X_MGR_UPortAdminEnable_CallBack(unit, port);
    #endif
#endif
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_GROUP_UPortAdminDisableCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE:  CallBack function
 * INPUT:   unit,port
 * OUTPUT: None.
 * RETURN: None.
 * NOTES:  None.
 *-----------------------------------------------------------------------------
 */
static void NETACCESS_GROUP_UPortAdminDisableCallbackHandler(UI32_T unit, UI32_T port)
{
#if (SYS_CPNT_NETACCESS == TRUE)
    NETACCESS_MGR_UPortAdminDisable_CallBack(unit, port);
#else
    #if (SYS_CPNT_DOT1X == TRUE)
        DOT1X_MGR_UPortAdminDisable_CallBack(unit, port);
    #endif
#endif
}


/* -------------------------------------------------------------------------
 * ROUTINE NAME - NETACCESS_GROUP_PortLearningStatusChangedCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, to handle learning status changed.
 * INPUT   : lport
 *           learning
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
static void NETACCESS_GROUP_PortLearningStatusChangedCallbackHandler(
    UI32_T lport,
    BOOL_T learning)
{
#if (SYS_CPNT_AMTR == TRUE)
    AMTR_MGR_PortLearningStatusChanged_Callback(lport, learning);
#endif
}


/*-----------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_GROUP_UPortDot1xEffectiveOperStatusChangedCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE: When effective oper status of some user port was changed for
            dot1x the function will be called.
 * INPUT:  pre_status --- The status before change.
 *         current_status --- The status after change.
 *                    Status                                    Precedence
 *                 ----------------------------------------     ----------
 *                 1) VAL_ifOperStatus_up                       0
 *                 2) SWCTRL_PORT_DORMANT_STATUS_TYPE_LACP      1
 *                 3) SWCTRL_PORT_DORMANT_STATUS_TYPE_DOT1X     2
 *                 4) VAL_ifOperStatus_down                     3
 *                 5) VAL_ifOperStatus_lowerLayerDown           3
 *                 6) VAL_ifOperStatus_notPresent
 * OUTPUT: None.
 * RETURN: None.
 * NOTES:  None.
 *-----------------------------------------------------------------------------
 */
static void NETACCESS_GROUP_UPortDot1xEffectiveOperStatusChangedCallbackHandler(
    UI32_T unit,
    UI32_T port,
    UI32_T pre_status,
    UI32_T current_status)
{
#if (SYS_CPNT_DOT1X == TRUE)
    /* if netacess is TRUE, dot1x is handled by netaccess directly,
     *   it's not necessary to call dot1x_mgr to handle port-op-status-changed callback.
     */
    #if (SYS_CPNT_NETACCESS == FALSE)
        DOT1X_MGR_PortOperStatusChanged_CallBack(unit, port, pre_status, current_status);
    #endif
#endif
}


/* -------------------------------------------------------------------------
 * ROUTINE NAME - NETACCESS_GROUP_UpdateLocalNmtrdrvStatsCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: NMTRDRV announce update command callback to NMTR
 * INPUT   : update_type -- Indicate the type of update
 *           start_port  -- Start port number
 *           port_amount -- Total amount of update ports
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
static void NETACCESS_GROUP_UpdateNmtrdrvStatsCallbackHandler(UI32_T update_type,UI32_T unit, UI32_T start_port, UI32_T port_amount)
{
    NMTR_MGR_HandleUpdateNmtrdrvStats(update_type,unit,start_port,port_amount);
    return;
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_GROUP_Dot1xPacketCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the callback event happening when a network interface
 *           received a DOT1X packet.
 *
 * INPUT   : mref_handle_p -- packet buffer and return buffer function pointer.
 *           dst_mac       -- the destination MAC address of this packet.
 *           src_mac       -- the source MAC address of this packet.
 *           tag_info      -- tag information
 *           type          -- packet type
 *           pkt_length    -- pdu length
 *           src_unit      -- user view unit number
 *           src_port      -- user view port number
 *           packet_class  -- class to identify which kind of packet (not used)
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
static void NETACCESS_GROUP_Dot1xPacketCallbackHandler(
    L_MM_Mref_Handle_T  *mref_handle_p,
    UI8_T   dst_mac[SYS_ADPT_MAC_ADDR_LEN], UI8_T   src_mac[SYS_ADPT_MAC_ADDR_LEN],
    UI16_T  tag_info,                       UI16_T  type,
    UI32_T  pkt_length,                     UI32_T  src_unit,
    UI32_T  src_port,                       UI32_T  packet_class)
{

#if (SYS_CPNT_NETACCESS == TRUE)
    UI32_T  lport;

    SWCTRL_UserPortToIfindex(src_unit, src_port, &lport);

    NETACCESS_MGR_AnnounceEapPacket_CallBack(
        mref_handle_p,  dst_mac,    src_mac,
        tag_info, type, pkt_length, lport, NULL);
#else
#if (SYS_CPNT_DOT1X == TRUE)
    DOT1X_TASK_AnnounceDOT1XPacket(
        mref_handle_p,  dst_mac,    src_mac,
        tag_info, type, pkt_length, src_unit,  src_port);
#endif /* #if (SYS_CPNT_DOT1X == TRUE) */
#endif /* #if (SYS_CPNT_NETACCESS == TRUE) */

    L_MM_Mref_Release(&mref_handle_p);

    return;

}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - NETACCESS_GROUP_AnnounceRadiusPacketCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: NMTRDRV announce update command callback to NMTR
 * INPUT   : update_type -- Indicate the type of update
 *           start_port  -- Start port number
 *           port_amount -- Total amount of update ports
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
static void NETACCESS_GROUP_AnnounceRadiusPacketCallbackHandler(
    UI32_T  result,                 UI8_T   *data_buf,
    UI32_T  data_len,               UI32_T  src_port,
    UI8_T   *src_mac,               UI32_T  src_vid,
    char   *authorized_vlan_list,  char   *authorized_qos_list,
    UI32_T   session_timeout,       UI32_T  server_ip)
{
#if (SYS_CPNT_DOT1X == TRUE)
    DOT1X_VM_AnnounceRadiusPacket(
        result, data_buf, data_len, src_port, src_mac, src_vid,
        authorized_vlan_list, authorized_qos_list, session_timeout, server_ip);
#endif /* #if (SYS_CPNT_DOT1X == TRUE) */
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - NETACCESS_GROUP_AnnounceRadaAuthPacketCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: NMTRDRV announce rada auth packet to NETACCESS_MGR
 * INPUT   : update_type -- Indicate the type of update
 *           start_port  -- Start port number
 *           port_amount -- Total amount of update ports
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
static void NETACCESS_GROUP_AnnounceRadaAuthPacketCallbackHandler(
    UI32_T  lport,                  UI8_T   *mac,
    int     identifier,             BOOL_T  authorized_result,
    char   *authorized_vlan_list,  char   *authorized_qos_list,
    UI32_T  session_time,           UI32_T  server_ip)
{
#if (SYS_CPNT_NETACCESS == TRUE)
    NETACCESS_MGR_AnnounceRadiusAuthorizedResult(
        lport, mac, identifier, authorized_result, authorized_vlan_list,
        authorized_qos_list, session_time, server_ip);
#endif /* #if (SYS_CPNT_NETACCESS == TRUE) */
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - NETACCESS_GROUP_AnnounceDot1xAuthPacketCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: NMTRDRV announce rada auth packet to NETACCESS_MGR
 * INPUT   : update_type -- Indicate the type of update
 *           start_port  -- Start port number
 *           port_amount -- Total amount of update ports
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
static void NETACCESS_GROUP_AnnounceDot1xAuthPacketCallbackHandler(
    UI32_T  lport,                  UI8_T   *mac,
    int     eap_identifier,         BOOL_T  authorized_result,
    char   *authorized_vlan_list,  char   *authorized_qos_list,
    UI32_T  session_time,           UI32_T  server_ip)
{
#if (SYS_CPNT_NETACCESS == TRUE)
    NETACCESS_MGR_AnnounceDot1xAuthorizedResult(
        lport, mac, eap_identifier, authorized_result, authorized_vlan_list,
        authorized_qos_list, session_time, server_ip);
#endif /* #if (SYS_CPNT_NETACCESS == TRUE) */
}


#if (SYS_CPNT_ADD == TRUE)
static void NETACCESS_GROUP_TelephoneDetectCallbackHandler(
    UI32_T  lport,
    UI8_T  *mac_addr,
    UI8_T   network_addr_subtype,
    UI8_T  *network_addr,
    UI8_T   network_addr_len,
    UI32_T  network_addr_ifindex,
    BOOL_T  tel_exist)
{
    ADD_MGR_LLDP_TelephoneDetect_CallBack(
        lport,
        mac_addr,
        network_addr_subtype,
        network_addr,
        network_addr_len,
        network_addr_ifindex,
        tel_exist
    );
}
#endif

static
void NETACCESS_GROUP_AnnounceACLDeletedHandler(
    char    *acl_name,
    UI32_T  acl_type,
    UI8_T   *dynamic_port_list)
{
#if (SYS_CPNT_NETACCESS_DYNAMIC_QOS == TRUE)
    NETACCESS_MGR_DelAcl_CallBack(acl_name, acl_type, dynamic_port_list);
#endif
}

static
void NETACCESS_GROUP_AnnouncePolicyMapDeletedHandler(
    char    *policy_map_name,
    UI8_T   *dynamic_port_list)
{
#if (SYS_CPNT_NETACCESS_DYNAMIC_QOS == TRUE)
    NETACCESS_MGR_DelPolicyMap_CallBack(policy_map_name, dynamic_port_list);
#endif
}

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
/*------------------------------------------------------------------------------
 * ROUTINE NAME : NETACCESS_GROUP_HandleHotInertion
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
static void NETACCESS_GROUP_HandleHotInertion(SYSFUN_Msg_T* msgbuf_p)
{
    SYS_TYPE_HandleHotSwapArg_T *msg_p;

    if (msgbuf_p == NULL)
        return;

    msg_p = (SYS_TYPE_HandleHotSwapArg_T*)msgbuf_p->msg_buf;

#if (SYS_CPNT_AMTR == TRUE)
    AMTR_INIT_HandleHotInsertion(msg_p->starting_port_ifindex, msg_p->number_of_port, msg_p->use_default);
#endif

#if (SYS_CPNT_NMTR == TRUE)
    NMTR_INIT_HandleHotInsertion(msg_p->starting_port_ifindex, msg_p->number_of_port, msg_p->use_default);
#endif

#if (SYS_CPNT_ADD == TRUE)
    ADD_INIT_HandleHotInsertion(msg_p->starting_port_ifindex, msg_p->number_of_port, msg_p->use_default);
#endif

#if (SYS_CPNT_PORT_SECURITY == TRUE)
    PSEC_INIT_HandleHotInsertion(msg_p->starting_port_ifindex, msg_p->number_of_port, msg_p->use_default);
#endif

#if (SYS_CPNT_NETACCESS == TRUE)
    NETACCESS_INIT_HandleHotInsertion(msg_p->starting_port_ifindex, msg_p->number_of_port, msg_p->use_default);
#endif

#if (SYS_CPNT_DOS == TRUE)
    DOS_INIT_HandleHotInsertion(msg_p->starting_port_ifindex, msg_p->number_of_port, msg_p->use_default);
#endif

#if (SYS_CPNT_APP_FILTER == TRUE)
    AF_MGR_HandleHotInsertion(msg_p->starting_port_ifindex, msg_p->number_of_port, msg_p->use_default);
#endif

}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : NETACCESS_GROUP_ HandleHotRemoval
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
static void NETACCESS_GROUP_HandleHotRemoval(SYSFUN_Msg_T* msgbuf_p)
{
    SYS_TYPE_HandleHotSwapArg_T *msg_p;

    if (msgbuf_p == NULL)
        return;

    msg_p = (SYS_TYPE_HandleHotSwapArg_T*)msgbuf_p->msg_buf;

#if (SYS_CPNT_AMTR == TRUE)
    AMTR_INIT_HandleHotRemoval(msg_p->starting_port_ifindex, msg_p->number_of_port);
#endif

#if (SYS_CPNT_NMTR == TRUE)
    NMTR_INIT_HandleHotRemoval(msg_p->starting_port_ifindex, msg_p->number_of_port);
#endif

#if (SYS_CPNT_ADD == TRUE)
    ADD_INIT_HandleHotRemoval(msg_p->starting_port_ifindex, msg_p->number_of_port);
#endif

#if (SYS_CPNT_PORT_SECURITY == TRUE)
    PSEC_INIT_HandleHotRemoval(msg_p->starting_port_ifindex, msg_p->number_of_port);
#endif

#if (SYS_CPNT_DOS == TRUE)
    DOS_INIT_HandleHotRemoval(msg_p->starting_port_ifindex, msg_p->number_of_port);
#endif

#if (SYS_CPNT_APP_FILTER == TRUE)
    AF_MGR_HandleHotRemoval(msg_p->starting_port_ifindex, msg_p->number_of_port);
#endif
}
#endif

#if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE)
static void NETACCESS_GROUP_HandleCmgrIpcMsg(SYSFUN_Msg_T *ipc_msg_p)
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
        if (    (cmgr_msg_p->data.arg_ui32_ui32_bool_bool.arg_bool_1 == FALSE)
             || (cmgr_msg_p->data.arg_ui32_ui32_bool_bool.arg_bool_2 == TRUE)
           )
        {
            NETACCESS_GROUP_VlanDestroyCallbackHandler(
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
                NETACCESS_GROUP_VlanMemberDeleteCallbackHandler(
                    cmgr_msg_p->data.arg_ui32_ui32_ui32_bool_bool.arg_ui32_1,
                    cmgr_msg_p->data.arg_ui32_ui32_ui32_bool_bool.arg_ui32_2,
                    cmgr_msg_p->data.arg_ui32_ui32_ui32_bool_bool.arg_ui32_3);
            }
            NETACCESS_GROUP_VlanMemberAddCallbackHandler(
                cmgr_msg_p->data.arg_ui32_ui32_ui32_bool_bool.arg_ui32_1,
                cmgr_msg_p->data.arg_ui32_ui32_ui32_bool_bool.arg_ui32_2,
                cmgr_msg_p->data.arg_ui32_ui32_ui32_bool_bool.arg_ui32_3);
        }
        else
        {
            NETACCESS_GROUP_VlanMemberDeleteCallbackHandler(
                cmgr_msg_p->data.arg_ui32_ui32_ui32_bool_bool.arg_ui32_1,
                cmgr_msg_p->data.arg_ui32_ui32_ui32_bool_bool.arg_ui32_2,
                cmgr_msg_p->data.arg_ui32_ui32_ui32_bool_bool.arg_ui32_3);
        }
        break;
    }

    return;
}
#endif /* #if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE) */
