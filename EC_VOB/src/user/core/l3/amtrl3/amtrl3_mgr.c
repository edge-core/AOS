/* =====================================================================================
 *  Module Name: AMTRL3_MGR.C
 *  Purpose :
 *      Interface between Phase2 (Protocol stack) and SWDRVL3, manage routing table, arp table
 *      and pass configuration to AMTRL3 setting chip which has Layer-3 capability.
 *
 *  Notes:
 *      1. AMTRL3 is one of module in IML layer, handle all configuration commands,
 *         maintains Net Route Table and ARP table which provide MIB information.
 *      2. AMTRL3 also handle all lower layer event, eg. port link up/down, trunking
 *         maintain host route table.
 *      3. AMTRL3 does not support the following criteria:
 *         No Replacement  - New host entry will be discard if max num of host entry
 *                           is reached.
 *         No ARP age out  - AMTRL3 does not support automatic arp ageout mechanism.
 *                           Host entry will only be remove base on ARP Ageout request
 *                           from Phase 2.
 *         No Supernetting - AMTRL3 does not support supernetting for Net routes
 *
 *  API List # =
 *          -------------------------------------------------------------------
 *          AMTRL3_MGR_Initiate_System_Resources()
 *          AMTRL3_MGR_EnterMasterMode()
 *          AMTRL3_MGR_EnterSlaveMode()
 *          AMTRL3_MGR_EnterTransitionMode()
 *          AMTRL3_MGR_AddIpCidrRouteEntry()
 *          AMTRL3_MGR_AddHostRoute()
 *          AMTRL3_MGR_DeleteIpCidrRouteEntry()
 *          AMTRL3_MGR_DeleteIpNetToMediaEntry()
 *          AMTRL3_MGR_SetIpForwarding()
 *          AMTRL3_MGR_SetIpNotForwarding()
 *          AMTRL3_MGR_AddVlanMac()
 *          AMTRL3_MGR_DeleteVlanMac()
 *          AMTRL3_MGR_GetIpCidrRouteEntry()
 *          AMTRL3_MGR_GetNextIpCidrRouteEntry()
 *          AMTRL3_MGR_GetIpNetToMediaEntry()
 *          AMTRL3_MGR_GetNextIpNetToMediaEntry()
 *          AMTRL3_MGR_ClearAllHostRoute()
 *          AMTRL3_MGR_DeleteDynamicNetRoute()
 *          AMTRL3_MGR_DeleteDynamicIpNetToMediaEntryBySubnet()
 *          AMTRL3_MGR_ReplaceExistHostRoute()
 *
 *  History :
 *      Date            Modifier    Reason
 *  -----------------------------------------------------------------------
 *      04-28-2003        hyliao    First Created
 *      01-12-2004        amytu     Redesign
 *      09-09-2004        krlin     Remove HOST_ROUTE_ERROR status
 *      01-14-2008        djd       Linux Porting
 * -------------------------------------------------------------------------------------
 * Copyright(C)        Accton Techonology Corporation, 2002,2003,2004,2008
 * =====================================================================================
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include "sys_type.h"
#include "sys_adpt.h"
#include <sysfun.h>
#include "sys_dflt.h"
#include "sys_bld.h"
#include "sys_module.h"
#include "sysfun.h"
#include "l_sort_lst.h"
#include "l_stdlib.h"
#include "l_inet.h"
//#include "l_mem.h"
#include "leaf_2096.h"
#include "leaf_1213.h"
#include "leaf_4001.h"
#include "leaf_es3626a.h"
#include "string.h"
#include "stktplg_pom.h"
#include "vlan_lib.h"
#include "vlan_pmgr.h"
#include "vlan_pom.h"
#include "swctrl.h"
#include "trk_mgr.h"
#include "ip_lib.h"
#include "xstp_pom.h"
#include "swdrvl3.h"
#include "swctrl.h"
#include "swctrl_pom.h"
//#include "arp_mgr.h"
//#include "netif_mgr.h"
#include "amtr_type.h"
#include "amtr_mgr.h"
#include "amtr_pmgr.h"
#include "amtrl3_mgr.h"
#include "amtrl3_om.h"
#include "amtrl3_type.h"
#include "amtrl3_backdoor.h"
#include "backdoor_mgr.h"
//#include "xmalloc.h"
#include "stdlib.h"
#include "l_cvrt.h"

#include "l_prefix.h"
#include "ipal_types.h"
#include "ipal_kernel.h"
#include "ipal_neigh.h"
#include "ipal_route.h"

#include "netcfg_type.h"
#include "netcfg_pom_ip.h"
#include "k_amtrl3_mgr.h" /* K_AMTRL3_MGR_SYSCALL_CMD_HIT */

#if (SYS_CPNT_VXLAN == TRUE)
#include "swdrv.h"
#include "vxlan_type.h"
#include "amtrdrv_pom.h"
#endif

/* NAME CONSTANT DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */


enum AMTRL3_MGR_HostRouteEvent_E
{
    HOST_ROUTE_PARTIAL_EVENT = 0,
    HOST_ROUTE_READY_EVENT,
    HOST_ROUTE_UNREFERENCE_EVENT,
    HOST_ROUTE_REMOVE_EVENT,
    HOST_ROUTE_SYNC_COMPLETE_EVENT,

    /* INTERNAL HOST ROUTE EVENT NOT LISTED
     * IN FSM
     */
    HOST_ROUTE_DROP_EVENT
};

enum AMTRL3_MGR_ReturnCode_E
{
    AMTRL3_MGR_SYNC_COMPLETE = 0,
    AMTRL3_MGR_SYNC_INCOMPLETE,
    AMTRL3_MGR_NO_MORE_ENTRY,
    AMTRL3_MGR_ADD_HOST_ERROR,
    AMTRL3_MGR_INSUFFICIENT_HOST_INFO,
    AMTRL3_MGR_ADD_HOST_OK
};

enum AMTRL3_MGR_ARP_ACTION_E
{
    AMTRL3_MGR_NORMAL_OPERATION = 1,
    AMTRL3_MGR_DISABLE_ARP_UNRESOLVED,
    AMTRL3_MGR_DISABLE_ARP_GATEWAY,
    AMTRL3_MGR_DISALBE_ARP_ALL
};

enum HOST_ROUTE_INTERNAL_STATE_E
{
    /* Internal State must start from the last state of
     * host route entry status defined in AMTRL3_OM.h
     */
    HOST_ROUTE_UNRESOLVED_1 = HOST_ROUTE_LAST_STATE,
    HOST_ROUTE_UNREFERENCE_1,
    HOST_ROUTE_UNREFERENCE_2,
    HOST_ROUTE_READY_1,
    HOST_ROUTE_READY_2,
    HOST_ROUTE_READY_3,
    HOST_ROUTE_REMOVE_1,
    HOST_ROUTE_REMOVE_2
};


/*  Funtion macro definitions
 */

#define AMTRL3_MGR_CHECK_OPERATION_MODE(RETVAL)               \
                if(SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE) \
                    return (RETVAL);

#define AMTRL3_MGR_CHECK_OPERATION_MODE_WITHOUT_RETURN_VALUE()  \
                if(SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)  \
                    return;


#define AMTRL3_MGR_MARCO_FILL_INET_CIDR_ROUTE_ENTRY(inet_cidr_route_entry)           \
                        inet_cidr_route_entry->inet_cidr_route_metric2 = 0;                     \
                        inet_cidr_route_entry->inet_cidr_route_metric3 = 0;                     \
                        inet_cidr_route_entry->inet_cidr_route_metric4 = 0;                     \
                        inet_cidr_route_entry->inet_cidr_route_metric5 = 0;                     \
                        inet_cidr_route_entry->inet_cidr_route_status = VAL_ipCidrRouteStatus_active;

#define AMTRL3_MGR_IS_VALID_HW_INFO(hw_info) (hw_info != NULL && hw_info != L_CVRT_UINT_TO_PTR(AMTRL3_OM_HW_INFO_INVALID))

#define AMTRL3_MGR_VXLAN_DBG_MSG(fmt, ...)                      \
            if (amtrl3_mgr_debug_tunnel)                        \
            {                                                   \
                BACKDOOR_MGR_Printf("(%5d):%s, " fmt "\r\n",    \
                    __LINE__, __FUNCTION__, ##__VA_ARGS__ );    \
            }

/*Simon's dump memory  function*/
#define DUMP_MEMORY_BYTES_PER_LINE 32
#define DUMP_MEMORY(mptr,size) do{\
    int i;\
    unsigned char * ptr=(unsigned char *)mptr;\
    for(i=0;i<size;i++){\
        if(i%DUMP_MEMORY_BYTES_PER_LINE ==0){\
            if(i>0)BACKDOOR_MGR_Printf("\r\n");\
            BACKDOOR_MGR_Printf("%0.4xH\t",i);\
        }\
        BACKDOOR_MGR_Printf("%0.2x", *ptr++);\
    }\
    BACKDOOR_MGR_Printf("\r\n");\
}while(0)

/*END Simon's debug function*/
/* LOCAL SUBPROGRAM DECLARATIONS
 */
static AMTRL3_MGR_FIB_T *AMTRL3_MGR_GetFIBByID(UI32_T fib_id);
static void AMTRL3_MGR_TrunkMemberAddForChipWithLoadBalance(UI32_T fib_id, UI32_T trunk_ifindex,  UI32_T member_ifindex,  AMTRL3_OM_HostRouteEntry_T  *host_entry);
static void AMTRL3_MGR_TrunkMemberAddForChipWithoutLoadBalance(UI32_T fib_id,  UI32_T trunk_ifindex, UI32_T member_ifindex,  AMTRL3_OM_HostRouteEntry_T  *host_entry);
static void AMTRL3_MGR_ClearGlobalDatabase(void);
static void AMTRL3_MGR_DeleteHostRouteEntryByLport(UI32_T fib_id, UI32_T lport);
static BOOL_T AMTRL3_MGR_CheckHostCounterOverSpec(UI32_T action_flags,  UI32_T fib_id, UI32_T type);
static UI32_T AMTRL3_MGR_CheckHostEntryForReplacement(UI32_T action_flags, UI32_T fib_id, AMTRL3_OM_HostRouteEntry_T *old_entry, AMTRL3_OM_HostRouteEntry_T *new_entry);
static void AMTRL3_MGR_UpdateCounterForReplacement(UI32_T action_flags, UI32_T fib_id, UI32_T old_type, UI32_T new_type);
static BOOL_T AMTRL3_MGR_CheckHostTypeForReplacement(AMTRL3_OM_HostRouteEntry_T *old_entry, AMTRL3_OM_HostRouteEntry_T *new_entry);
static BOOL_T AMTRL3_MGR_DeleteDefaultRoute(UI32_T action_flags, UI32_T fib_id, AMTRL3_OM_NetRouteEntry_T *net_route_entry, AMTRL3_OM_HostRouteEntry_T *nhop_entry);
static void AMTRL3_MGR_LoadBalanceHostRouteForTrunkMember(UI32_T fib_id, UI32_T trunk_ifindex,  UI32_T member_ifindex);
static BOOL_T  AMTRL3_MGR_IsFirstPathOfECMPRouteInChip(UI32_T action_flags, UI32_T fib_id, AMTRL3_OM_NetRouteEntry_T  *net_entry);
static BOOL_T  AMTRL3_MGR_IsLastPathOfECMPRouteInOm(UI32_T action_flags, UI32_T fib_id, AMTRL3_OM_NetRouteEntry_T  *net_entry);
static UI32_T AMTRL3_MGR_AddHostRouteToChip(UI32_T action_flags, UI32_T fib_id, AMTRL3_OM_HostRouteEntry_T  *host_entry);
static UI32_T AMTRL3_MGR_AddNonTunnelHostRouteToChip(UI32_T action_flags, UI32_T fib_id, AMTRL3_OM_HostRouteEntry_T  *host_entry);

#if (SYS_CPNT_IP_TUNNEL == TRUE)
static UI32_T AMTRL3_MGR_AddTunnelHostRouteToChip(UI32_T action_flags, UI32_T fib_id, AMTRL3_OM_HostRouteEntry_T  *host_entry);
static BOOL_T AMTRL3_MGR_DeleteTunnelHostRouteFromChip(UI32_T action_flags, UI32_T fib_id, AMTRL3_OM_HostRouteEntry_T  *host_route_entry);
static BOOL_T AMTRL3_MGR_AddTunnelNetRouteToChipByGateway(UI32_T action_flags, UI32_T fib_id, AMTRL3_OM_HostRouteEntry_T *gateway);
static BOOL_T AMTRL3_MGR_RemoveTunnelNetRouteFromChipByGateway(UI32_T action_flags, UI32_T fib_id, AMTRL3_OM_HostRouteEntry_T *gateway);
static BOOL_T AMTRL3_MGR_GetNextNetRouteByTunnelID(UI32_T action_flags, UI32_T fib_id, AMTRL3_OM_NetRouteEntry_T  *net_entry);
static BOOL_T AMTRL3_MGR_HandleTunnelNetRouteToChip(UI32_T action_flags, UI32_T fib_id,
                                                    AMTRL3_OM_NetRouteEntry_T        *tunnel_local_net_route_entry,
                                                    AMTRL3_TYPE_InetCidrRouteEntry_T *tunnel_net_route_entry);
static void AMTRL3_MGR_CheckAndRemoveExpiredTunnelEntry(
                                        UI32_T fib_id,
                                        AMTRL3_OM_NetRouteEntry_T *tunnel_net_entry,
                                        UI32_T current_time);
static UI32_T AMTRL3_MGR_HotInsertAddTunnelHostRouteToModule(UI32_T action_flags, UI32_T fib_id, AMTRL3_OM_HostRouteEntry_T  *host_entry);
static BOOL_T AMTRL3_MGR_TunnelUpdateTtl(AMTRL3_TYPE_InetHostRouteEntry_T * al3_tunnel_host, UI32_T tunnel_ttl);
static void   AMTRL3_MGR_PrintSwdrvl3HostTunnelInfo(SWDRVL3_HostTunnel_T *entry);
static void   AMTRL3_MGR_SetTunnelHostRouteEventByNexthopEntry( UI32_T fib_id, AMTRL3_OM_HostRouteEntry_T* host_route_entry , UI32_T event);
#endif /*SYS_CPNT_IP_TUNNEL*/

#if (SYS_CPNT_VXLAN == TRUE)
static void AMTRL3_MGR_SetVxlanTunnelByNexthopEvent(UI32_T fib_id, AMTRL3_OM_HostRouteEntry_T *host_route_entry ,UI32_T event);
static UI32_T AMTRL3_MGR_HotInsertAddVxlanHostRouteToModule(UI32_T fib_id, AMTRL3_OM_HostRouteEntry_T *host_entry);
#endif

static BOOL_T AMTRL3_MGR_DeleteHostRouteFromChip(UI32_T action_flags, UI32_T fib_id, AMTRL3_OM_HostRouteEntry_T  *host_route_entry);
static BOOL_T AMTRL3_MGR_DeleteNonTunnelHostRouteFromChip(UI32_T action_flags, UI32_T fib_id, AMTRL3_OM_HostRouteEntry_T  *host_route_entry);

static BOOL_T AMTRL3_MGR_AddNetRouteToChip(UI32_T action_flags,  UI32_T fib_id,   AMTRL3_OM_NetRouteEntry_T  *net_entry, AMTRL3_OM_HostRouteEntry_T *host_entry);
static BOOL_T AMTRL3_MGR_DeleteNetRouteFromChip(UI32_T action_flags, UI32_T fib_id, AMTRL3_OM_NetRouteEntry_T  *net_entry, AMTRL3_OM_HostRouteEntry_T *nhop_entry);
static void AMTRL3_MGR_CheckAndRemoveExpiredHostEntry( UI32_T action_flags, UI32_T fib_id,  AMTRL3_OM_HostRouteEntry_T *local_host_entry, UI32_T current_time);
static BOOL_T AMTRL3_MGR_HostRouteEventHandler(UI32_T action_flags, UI32_T fib_id,  UI32_T host_route_event,  AMTRL3_OM_HostRouteEntry_T *host_route_entry);
static UI32_T AMTRL3_MGR_CompensateNetRouteByGateway(UI32_T action_flags, UI32_T fib_id, AMTRL3_OM_HostRouteEntry_T   *gateway_entry_p,   UI32_T   *num_of_entry_processed);
static BOOL_T AMTRL3_MGR_CompensateResolvedNetRouteEntry(UI32_T action_flags, UI32_T fib_id,AMTRL3_OM_ResolvedNetRouteEntry_T   *resolved_net_entry,AMTRL3_OM_NetRouteStatus_T   *net_route_result);
static BOOL_T AMTRL3_MGR_CompensateSuperNetRoutes(UI32_T action_flags, UI32_T fib_id, AMTRL3_OM_NetRouteEntry_T  *net_entry);
static void AMTRL3_MGR_RefreshNetTable(UI32_T action_flags, UI32_T fib_id, AMTRL3_OM_ResolvedNetRouteEntry_T *child_entry);
static BOOL_T AMTRL3_MGR_ModifySuperNetRouteEntry(UI32_T action_flags, UI32_T fib_id, AMTRL3_OM_NetRouteEntry_T *net_entry);
static BOOL_T AMTRL3_MGR_DeriveDetailPortInfo(UI32_T vid_ifindex, UI32_T lport, UI32_T *vid, UI32_T *unit, UI32_T *port, UI32_T *trunk_id,  BOOL_T *is_tagged, BOOL_T *is_trunk);
static void AMTRL3_MGR_DeriveProperHostRouteStatus(UI32_T event_type, AMTRL3_OM_HostRouteEntry_T *host_route_entry);
static void AMTRL3_MGR_DeleteAllRelatedNetRouteFromChip(UI32_T action_flags, UI32_T fib_id, AMTRL3_OM_HostRouteEntry_T *host_entry);
static void AMTRL3_MGR_DeleteAllRelatedEcmpRouteFromChip(UI32_T action_flags, UI32_T fib_id, AMTRL3_OM_HostRouteEntry_T *host_entry);
static void AMTRL3_MGR_DebugFSM(UI32_T action_flags, UI32_T *host_route_event, L_INET_AddrIp_T dst_ip, UI32_T old_status, UI32_T new_status);
static void AMTRL3_MGR_InitChipCapacity(void);
static UI32_T AMTRL3_MGR_SetMyIpHostRouteToChip(UI32_T action_flags, UI32_T fib_id, AMTRL3_OM_HostRouteEntry_T  *host_entry);
static BOOL_T AMTRL3_MGR_DeleteMyIpHostRouteFromChip(UI32_T action_flags, UI32_T fib_id, AMTRL3_OM_HostRouteEntry_T  *host_route_entry);
static UI32_T AMTRL3_MGR_UpdateHostRouteToChip(UI32_T action_flags, UI32_T fib_id, AMTRL3_OM_HostRouteEntry_T  *host_entry);
static UI32_T AMTRL3_MGR_UpdateNonTunnelHostRouteToChip(UI32_T action_flags, UI32_T fib_id, AMTRL3_OM_HostRouteEntry_T  *host_entry);
static UI32_T AMTRL3_MGR_UpdateTunnelHostRouteToChip(UI32_T action_flags, UI32_T fib_id, AMTRL3_OM_HostRouteEntry_T  *host_entry);

static BOOL_T AMTRL3_MGR_AddECMPRouteOnePathToChip(UI32_T action_flags,
                                                          UI32_T fib_id,
                                                          AMTRL3_OM_NetRouteEntry_T  *net_entry,
                                                          AMTRL3_OM_HostRouteEntry_T *host_entry,
                                                          BOOL_T is_first);
static BOOL_T AMTRL3_MGR_DeleteECMPRouteOnePathFromChip(UI32_T action_flags,
                                                                UI32_T fib_id,
                                                                AMTRL3_OM_NetRouteEntry_T  *net_entry,
                                                                AMTRL3_OM_HostRouteEntry_T *nhop_entry,
                                                                BOOL_T is_last);
static BOOL_T AMTRL3_MGR_AddECMPRouteMultiPathToChip(UI32_T action_flags,
                                                          UI32_T fib_id,
                                                          AMTRL3_OM_NetRouteEntry_T  *net_entry,
                                                          AMTRL3_OM_HostRouteEntry_T *host_entry,
                                                          UI32_T count);
static BOOL_T AMTRL3_MGR_DeleteECMPRouteFromChip(UI32_T action_flags,
                                                          UI32_T fib_id,
                                                          AMTRL3_OM_NetRouteEntry_T *net_entry,
                                                          UI32_T active_count);
static void AMTRL3_MGR_SyncHWInfoToAllECMPPath(UI32_T action_flags,
                                                    UI32_T fib_id,
                                                    AMTRL3_OM_NetRouteEntry_T *net_entry);
static void AMTRL3_MGR_ClearHostRouteHWInfo(UI32_T action_flags, UI32_T fib_id, AMTRL3_OM_HostRouteEntry_T *host_entry);
static void AMTRL3_MGR_ClearNetRouteHWInfo(UI32_T action_flags, UI32_T fib_id, AMTRL3_OM_NetRouteEntry_T *net_entry);
static void AMTRL3_MGR_HostRouteEventForNetRoute(UI32_T action_flags,
                                                  UI32_T fib_id,
                                                  AMTRL3_OM_HostRouteEntry_T *host_entry);
static void AMTRL3_MGR_HostRouteEventForECMPRoute(UI32_T action_flags,
                                                                UI32_T fib_id,
                                                                AMTRL3_OM_HostRouteEntry_T *host_entry);

static void AMTRL3_MGR_HotInsertProcessNetRoute(UI32_T fib_id);
static void AMTRL3_MGR_HotInsertProcessHostRoute(UI32_T fib_id);
/* static void AMTRL3_MGR_HotInsertProcessL3Interface(UI32_T fib_id); */
static void AMTRL3_MGR_HotRemovalProcessHostRoute(UI32_T fib_id, UI32_T lport);
static UI32_T AMTRL3_MGR_HotInsertAddHostRouteToModule(UI32_T action_flags, UI32_T fib_id, AMTRL3_OM_HostRouteEntry_T  *host_entry);
static UI32_T AMTRL3_MGR_HotInsertAddMyIpHostRouteToChip(UI32_T action_flags, UI32_T fib_id, AMTRL3_OM_HostRouteEntry_T  *host_entry);
static BOOL_T AMTRL3_MGR_HotInsertAddNetRouteToModule(UI32_T action_flags,
                                                      UI32_T fib_id,
                                                      AMTRL3_OM_NetRouteEntry_T  *net_entry,
                                                      AMTRL3_OM_HostRouteEntry_T *host_entry);
static BOOL_T AMTRL3_MGR_HotInsertAddECMPRouteMultiPathToModule(UI32_T action_flags,
                                                          UI32_T fib_id,
                                                          AMTRL3_OM_NetRouteEntry_T  *net_entry,
                                                          AMTRL3_OM_HostRouteEntry_T *host_entry,
                                                          UI32_T count);
static void AMTRL3_MGR_HotRemoveProcessNetRoute(UI32_T fib_id, UI32_T action_flags,
                            AMTRL3_OM_HostRouteEntry_T *nh_entry);

static void AMTRL3_MGR_UpdateHostRouteRefCountInChip(UI32_T action_flags, UI32_T fib_id,
                            AMTRL3_OM_HostRouteEntry_T *host_entry_p, I32_T offset);
static BOOL_T AMTRL3_MGR_GetAMTRExactMAC(AMTR_TYPE_AddrEntry_T *amtr_addr_entry_p);

/* STATIC VARIABLE DECLARATIONS
 */
static SYS_TYPE_Stacking_Mode_T  ___csc_mgr_operating_mode = SYS_TYPE_STACKING_TRANSITION_MODE;
static UI32_T   amtrl3_mgr_hostroute_scanning_mode;
static UI32_T   amtrl3_mgr_hostroute_delete_mode;
static BOOL_T   amtrl3_mgr_netroute_performance_flag;
static BOOL_T   amtrl3_mgr_hostroute_performance_flag;
static UI8_T    amtrl3_mgr_debug_flag;
static UI8_T    amtrl3_mgr_arp_action;
static UI16_T   amtrl3_mgr_callback_debug_flag;
static UI8_T    amtrl3_mgr_debug_task;
static BOOL_T   amtrl3_mgr_debug_tunnel;
static UI8_T    amtrl3_mgr_zero_mac[SYS_ADPT_MAC_ADDR_LEN] = {0};
static UI32_T   amtrl3_chip_capability_flags;
static BOOL_T chip_full_flag = FALSE; /* Only for unit test */

static AMTRL3_MGR_FIB_T *amtrl3_mgr_fib_p[SYS_ADPT_MAX_NUMBER_OF_FIB] = {0};
static AMTRL3_MGR_FIB_T  amtrl3_mgr_default_fib;
static UI32_T            amtrl3_my_unit_id;

const static UI32_T gateway_retransmit_time[AMTRL3_MGR_GATEWAY_ENTRY_DELAY_INTERVAL] =
{0,3,10,20};

const static UI32_T static_host_retransmit_time[AMTRL3_MGR_STATIC_ENTRY_DELAY_INTERVAL] =
{2,6,10,20};

const static UI32_T host_route_FSM[6] [5]  =
{
    /* not_exist
     */
    {
    HOST_ROUTE_UNRESOLVED_1,
    HOST_ROUTE_READY_1,
    HOST_ROUTE_NOT_EXIST,
    HOST_ROUTE_NOT_EXIST,
    HOST_ROUTE_NOT_EXIST
    },

    /* Unresolved
     */
    {
    HOST_ROUTE_UNRESOLVED,
    HOST_ROUTE_READY_1,
    HOST_ROUTE_UNREFERENCE_1,
    HOST_ROUTE_REMOVE_1,
    HOST_ROUTE_UNRESOLVED
    },

    /* Unreference
     */
    {
    HOST_ROUTE_UNRESOLVED_1,
    HOST_ROUTE_READY_1,
    HOST_ROUTE_UNREFERENCE,
    HOST_ROUTE_REMOVE_1,
    HOST_ROUTE_UNREFERENCE
    },

    /* Ready_not_sync
     */
    {
    HOST_ROUTE_READY_NOT_SYNC,
    HOST_ROUTE_READY_2,
    HOST_ROUTE_UNREFERENCE_2,
    HOST_ROUTE_REMOVE_2,
    HOST_ROUTE_READY_3
    },

    /* Host_Ready
     */
    {
    HOST_ROUTE_READY_3,
    HOST_ROUTE_READY_2,
    HOST_ROUTE_UNREFERENCE_2,
    HOST_ROUTE_REMOVE_2,
    HOST_ROUTE_HOST_READY
    },

    /* Gateway_Ready
     */
    {
    HOST_ROUTE_READY_3,
    HOST_ROUTE_READY_2,
    HOST_ROUTE_UNREFERENCE_2,
    HOST_ROUTE_REMOVE_2,
    HOST_ROUTE_GATEWAY_READY
    }
};

/* -------------------------------------------------------------------------
* FUNCTION NAME: AMTRL3_MGR_InitChipCapacity
* -------------------------------------------------------------------------
* PURPOSE:  Set the chip capacity when AMTRL3 init.
* INPUT:
* OUTPUT:   none.
* RETURN:
* NOTES:
* -------------------------------------------------------------------------*/
static void AMTRL3_MGR_InitChipCapacity(void)
{
    amtrl3_chip_capability_flags = 0;

#if (SYS_ADPT_CHIP_SUPPORT_EGRESS_OBJECT == TRUE)
    SET_FLAG(amtrl3_chip_capability_flags, AMTRL3_CHIP_SUPPORT_EGRESS_OBJECT);
#endif

#if (SYS_ADPT_CHIP_SUPPORT_SAME_ECMP_EGRESS == TRUE)
    SET_FLAG(amtrl3_chip_capability_flags, AMTRL3_CHIP_SUPPORT_SAME_HW_INFO_FOR_ECMP_PATH);
#endif

#if (SYS_ADPT_CHIP_SUPPORT_L3_TRUNK_LOAD_BALANCE == TRUE)
    SET_FLAG(amtrl3_chip_capability_flags, AMTRL3_CHIP_SUPPORT_L3_TRUNK_LOAD_BALANCE);
#endif

#if (SYS_ADPT_CHIP_SUPPORT_LOCAL_HOSE == TRUE)
    SET_FLAG(amtrl3_chip_capability_flags, AMTRL3_CHIP_USE_LOCAL_HOST_TO_TRAP_MY_IP_PKTS);
#endif

#if (SYS_ADPT_CHIP_USE_DEFAULT_ROUTE_TRAP_TO_CPU == TRUE)
    SET_FLAG(amtrl3_chip_capability_flags, AMTRL3_CHIP_USE_DEFAULT_ROUTE_TRAP_TO_CPU);
#endif

#if (SYS_ADPT_CHIP_USE_TCAM_FOR_ROUTE_TABLE == TRUE)
    SET_FLAG(amtrl3_chip_capability_flags, AMTRL3_CHIP_USE_TCAM_FOR_ROUTE_TABLE);
#endif

#if (SYS_ADPT_CHIP_KEEP_TRUNK_ID_IN_HOST_ROUTE == TRUE)
    SET_FLAG(amtrl3_chip_capability_flags, AMTRL3_CHIP_KEEP_TRUNK_ID_IN_HOST_ROUTE);
#endif

    return;
}

 /* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_Initiate_System_Resources
 * -------------------------------------------------------------------------
 * PURPOSE:  Initialize amtrl3_arp amtrl3_netroute amtrl3_hostroute to manage.
 * INPUT:    none.
 * OUTPUT:   none.
 * RETURN:   none.
 * NOTES:
 * -------------------------------------------------------------------------*/
void AMTRL3_MGR_Initiate_System_Resources(void)
{
    AMTRL3_OM_Initiate_System_Resources();

    AMTRL3_MGR_InitChipCapacity();
    AMTRL3_MGR_ClearGlobalDatabase();
    AMTRL3_MGR_CreateFIB(SYS_ADPT_DEFAULT_FIB);

    IPAL_Kernel_Init();

    return;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_EnterMasterMode
 * -------------------------------------------------------------------------
 * PURPOSE:  This function will set amtrl3_mgr into master mode.
 * INPUT:    none.
 * OUTPUT:   none.
 * RETURN:   none.
 * NOTES:
 * -------------------------------------------------------------------------*/
void AMTRL3_MGR_EnterMasterMode(void)
{
    STKTPLG_POM_GetMyUnitID(&amtrl3_my_unit_id);
    SYSFUN_ENTER_MASTER_MODE();
    return;
}


/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_EnterSlaveMode
 * -------------------------------------------------------------------------
 * PURPOSE:  This function will set amtrl3_mgr into slave mode.
 * INPUT:    none.
 * OUTPUT:   none.
 * RETURN:   none.
 * NOTES:
 * -------------------------------------------------------------------------*/
void AMTRL3_MGR_EnterSlaveMode(void)
{
    STKTPLG_POM_GetMyUnitID(&amtrl3_my_unit_id);
    SYSFUN_ENTER_SLAVE_MODE();
    return;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_EnterTransitionMode
 * -------------------------------------------------------------------------
 * PURPOSE:  This function will set amtrl3_mgr into transition mode.
 * INPUT:    none.
 * OUTPUT:   none.
 * RETURN:   none.
 * NOTES:
 * -------------------------------------------------------------------------*/
void AMTRL3_MGR_EnterTransitionMode(void)
{
    UI32_T  fib_id;

    for(fib_id = 0; fib_id < SYS_ADPT_MAX_NUMBER_OF_FIB; fib_id++)
    {
        if (fib_id == SYS_ADPT_DEFAULT_FIB)
            AMTRL3_MGR_ClearFIB(fib_id);
        else
            AMTRL3_MGR_DeleteFIB(fib_id);
    }

    AMTRL3_MGR_ClearGlobalDatabase();
    amtrl3_my_unit_id = 0;

    SYSFUN_ENTER_TRANSITION_MODE();

    return;
}

/*------------------------------------------------------------------------------
 * Function : AMTRL3_MGR_SetTransitionMode()
 *------------------------------------------------------------------------------
 * Purpose  : This function will set the operation mode to transition mode
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-----------------------------------------------------------------------------*/
void AMTRL3_MGR_SetTransitionMode(void)
{
    SYSFUN_SET_TRANSITION_MODE();
    return;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTRL3_MGR_GetOperationMode
 *------------------------------------------------------------------------
 * FUNCTION: This function will return present opertaion mode
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : Current operation mode.
 *          1). SYS_TYPE_STACKING_TRANSITION_MODE
 *          2). SYS_TYPE_STACKING_MASTER_MODE
 *          3). SYS_TYPE_STACKING_SLAVE_MODE
 * NOTE    : None
 *------------------------------------------------------------------------*/
SYS_TYPE_Stacking_Mode_T AMTRL3_MGR_GetOperationMode(void)
{
    return SYSFUN_GET_CSC_OPERATING_MODE();
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_GetFIBByID
 * -------------------------------------------------------------------------
 * PURPOSE:  To get FIB by its id.
 * INPUT:    fib_id - FIB id (0 ~ 254).
 * OUTPUT:   none.
 * RETURN:   a pointer to the FIB just found
 * NOTES:
 * -------------------------------------------------------------------------*/
static AMTRL3_MGR_FIB_T *AMTRL3_MGR_GetFIBByID(UI32_T fib_id)
{
    if (fib_id >= SYS_ADPT_MAX_NUMBER_OF_FIB)
        return NULL;

    return  amtrl3_mgr_fib_p[SYS_ADPT_DEFAULT_FIB];
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_CreateFIB
 * -------------------------------------------------------------------------
 * PURPOSE:  create a FIB instance
 * INPUT:    fib_id       --  FIB id (1 ~ 255).
 * OUTPUT:   none.
 * RETURN:   AMTRL3_TYPE_FIB_SUCCESS,
 *           AMTRL3_TYPE_FIB_ALREADY_EXIST,
 *           AMTRL3_TYPE_FIB_OM_ERROR,
 *           AMTRL3_TYPE_FIB_FAIL.
 * NOTES:
 * -------------------------------------------------------------------------*/
UI32_T AMTRL3_MGR_CreateFIB(UI32_T fib_id)
{
    UI32_T            ret;
    AMTRL3_MGR_FIB_T  *mgr_fib;

    /* bcz SYS_ADPT_DEFAULT_FIB's om is shared for all fib_id
     */
    if(SYS_ADPT_DEFAULT_FIB != fib_id)
        return AMTRL3_TYPE_FIB_SUCCESS;

    ret = AMTRL3_OM_CreateFIB(fib_id);

    /* Create mgr fib */
    if(ret == AMTRL3_TYPE_FIB_SUCCESS)
    {
        if(SYS_ADPT_DEFAULT_FIB == fib_id)
        {
            mgr_fib = &amtrl3_mgr_default_fib;
            memset(mgr_fib, 0, sizeof(AMTRL3_MGR_FIB_T));
        }
        else
        {
            if((mgr_fib = malloc(sizeof(AMTRL3_MGR_FIB_T))) == NULL)
            {
                AMTRL3_OM_DeleteFIB(fib_id);
                return AMTRL3_TYPE_FIB_FAIL;
            }
            memset(mgr_fib, 0, sizeof(AMTRL3_MGR_FIB_T));
        }

        amtrl3_mgr_fib_p[fib_id] = mgr_fib;
        mgr_fib->fib_id = fib_id;
    }
    return ret;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_DeleteFIB()
 * -------------------------------------------------------------------------
 * PURPOSE:  Delete a FIB
 * INPUT:    fib_id  --  FIB id (1 ~ 255).
 * OUTPUT:   none.
 * RETURN:   AMTRL3_TYPE_FIB_SUCCESS,
 *           AMTRL3_TYPE_FIB_NOT_EXIST,
 *           AMTRL3_TYPE_FIB_OM_ERROR,
 *           AMTRL3_TYPE_FIB_FAIL.
 * NOTES:
 * -------------------------------------------------------------------------*/
UI32_T AMTRL3_MGR_DeleteFIB(UI32_T fib_id)
{
    UI32_T ret;

    /* Don't check master mode for this function, because AMTRL3_MGR_EnterTransitionMode
     * will call into this function to clean all FIB database
     */
    /*AMTRL3_MGR_CHECK_OPERATION_MODE(FALSE);*/

    /* bcz SYS_ADPT_DEFAULT_FIB's om is shared for all fib_id
     */
    if(SYS_ADPT_DEFAULT_FIB != fib_id)
        return AMTRL3_TYPE_FIB_SUCCESS;

    ret = AMTRL3_OM_DeleteFIB(fib_id);

    if(ret == AMTRL3_TYPE_FIB_SUCCESS)
    {
        if(SYS_ADPT_DEFAULT_FIB == fib_id)
            memset(amtrl3_mgr_fib_p[fib_id], 0 , sizeof(AMTRL3_MGR_FIB_T));
        else
            free(amtrl3_mgr_fib_p[fib_id]);
        amtrl3_mgr_fib_p[fib_id] = NULL;
    }

    return ret;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_ClearFIB()
 * -------------------------------------------------------------------------
 * PURPOSE:  Clear a FIB
 * INPUT:    fib_id  --  FIB id (1 ~ 255).
 * OUTPUT:   none
 * RETURN:   AMTRL3_TYPE_FIB_SUCCESS,
 *           AMTRL3_TYPE_FIB_NOT_EXIST,
 *           AMTRL3_TYPE_FIB_OM_ERROR,
 *           AMTRL3_TYPE_FIB_FAIL.
 * NOTES:
 * -------------------------------------------------------------------------*/
UI32_T AMTRL3_MGR_ClearFIB(UI32_T fib_id)
{
    UI32_T ret;

    /* Don't check master mode for this function, because AMTRL3_MGR_EnterTransitionMode
     * will call into this function to clean all FIB database
     */
    if(AMTRL3_TYPE_FIB_SUCCESS == (ret = AMTRL3_OM_ClearFIB(fib_id)))
    {
        if (amtrl3_mgr_fib_p[fib_id])
            memset(amtrl3_mgr_fib_p[fib_id], 0 , sizeof(AMTRL3_MGR_FIB_T));
    }

    return ret;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_GetNextFIBID
 * -------------------------------------------------------------------------
 * PURPOSE:  Get next FIB id(0 ~ 254).
 * INPUT:    fib_id - AMTRL3_OM_GET_FIRST_FIB_ID to get first
 * OUTPUT:   fib_id
 * RETURN:   TRUE/FALSE
 * NOTES:
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_MGR_GetNextFIBID(UI32_T *fib_id)
{
    AMTRL3_MGR_CHECK_OPERATION_MODE(FALSE);
    return AMTRL3_OM_GetNextFIBID(fib_id);
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_ClearGlobalDatabase()
 * -------------------------------------------------------------------------
 * PURPOSE:  Clear all global database.
 * INPUT:    none.
 * OUTPUT:   none.
 * RETURN:   none.
 * NOTES:
 * -------------------------------------------------------------------------*/
static void AMTRL3_MGR_ClearGlobalDatabase(void)
{
    /* BODY
     */
    amtrl3_mgr_hostroute_scanning_mode  = 0;
    amtrl3_mgr_hostroute_delete_mode = 0;
    amtrl3_mgr_callback_debug_flag   = 0;
    amtrl3_mgr_arp_action = 0;
    amtrl3_mgr_debug_flag = 0;
    amtrl3_mgr_debug_task = 0;
    amtrl3_mgr_debug_tunnel = FALSE;
    /* Initialize amtrl3 debug counters
     */
    amtrl3_mgr_netroute_performance_flag   = FALSE;
    amtrl3_mgr_hostroute_performance_flag  = FALSE;

    return;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_EnableIpForwarding
 * -------------------------------------------------------------------------
 * PURPOSE:  Enable IPv4/v6 forwarding
 * INPUT:    action_flags    --  AMTRL3_TYPE_FLAGS_IPV4 \
 *                               AMTRL3_TYPE_FLAGS_IPV6
 *           vr_id          --  Virtual Router id
 * OUTPUT:   none.
 * RETURN:   TRUE / FALSE
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_MGR_EnableIpForwarding(UI32_T action_flags, UI32_T vr_id)
{
    UI32_T result;

    if(action_flags == AMTRL3_TYPE_FLAGS_IPV4)
        result = SWDRVL3_EnableIpForwarding(SWDRVL3_FLAG_IPV4, vr_id);
    else if(action_flags == AMTRL3_TYPE_FLAGS_IPV6)
        result = SWDRVL3_EnableIpForwarding(SWDRVL3_FLAG_IPV6, vr_id);
    else
        return FALSE;

    if(result == SWDRVL3_L3_NO_ERROR)
        return TRUE;
    else
        return FALSE;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_DisableIpForwarding
 * -------------------------------------------------------------------------
 * PURPOSE:  Disable IPv4/v6 forwarding
 * INPUT:    action_flags    --  AMTRL3_TYPE_FLAGS_IPV4 \
 *                               AMTRL3_TYPE_FLAGS_IPV6
 *           vr_id          --  Virtual Router id
 * OUTPUT:   none.
 * RETURN:   TRUE / FALSE
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_MGR_DisableIpForwarding(UI32_T action_flags, UI32_T vr_id)
{
    UI32_T result;

    if(action_flags == AMTRL3_TYPE_FLAGS_IPV4)
        result = SWDRVL3_DisableIpForwarding(SWDRVL3_FLAG_IPV4, vr_id);
    else if(action_flags == AMTRL3_TYPE_FLAGS_IPV6)
        result = SWDRVL3_DisableIpForwarding(SWDRVL3_FLAG_IPV6, vr_id);
    else
        return FALSE;

    if(result == SWDRVL3_L3_NO_ERROR)
        return TRUE;
    else
        return FALSE;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_SetHostRoute
 * -------------------------------------------------------------------------
 * PURPOSE:  Add one ipv4 or ipv6 host route to Host Route database
 *           and host route table in chip.
 * INPUT:    action_flags    --  AMTRL3_TYPE_FLAGS_IPV4 \
 *                               AMTRL3_TYPE_FLAGS_IPV6
 *           fib_id          --  FIB id
 *           host_entry      --  Inet Host Route Entry
 *                               all structure fileds must be set
 *           type            --  VAL_ipNetToPhysicalExtType_other     \
 *                               VAL_ipNetToPhysicalExtType_invalid   \
 *                               VAL_ipNetToPhysicalExtType_dynamic   \
 *                               VAL_ipNetToPhysicalExtType_static    \
 *                               VAL_ipNetToPhysicalExtType_local     \
 *                               VAL_ipNetToPhysicalExtType_broadcast \
 *                               VAL_ipNetToPhysicalExtType_vrrp      \
 * OUTPUT:   none.
 * RETURN:   TRUE / FALSE
 * NOTES:    1. IML should get the return value to decide whether the ARP pkt
 *              should be sent to TCP/IP stack, if it is TRUE, need send.
 *           2. For link local ipv6 address, address type and zone index must
 *              be correctly set.
 *           3. only one flag can be set in one time
 *           4. If first add (add from config), old_host_entry should be NULL
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_MGR_SetHostRoute(UI32_T action_flags,
                               UI32_T fib_id,
                               AMTRL3_TYPE_InetHostRouteEntry_T *host_entry,
                               AMTRL3_OM_HostRouteEntry_T *old_host_entry,
                               UI32_T type)
{
    UI32_T                              event_type;
    UI32_T                              begin_timetick = 0, end_timetick = 0;
    UI32_T                              unit = 0, port = 0, vid;
    AMTRL3_OM_HostRouteEntry_T          new_host_route_entry;
    AMTRL3_OM_HostRouteEntry_T          old_host_route_entry;
    UI8_T                               ip_str[L_INET_MAX_IPADDR_STR_LEN] = {0};
    BOOL_T                              ret = FALSE;

    /* TODO: get fib from host_entry->dst_vid_ifindex
     */
    if(AMTRL3_MGR_GetFIBByID(fib_id) == NULL)
        return FALSE;

    VLAN_IFINDEX_CONVERTTO_VID(host_entry->dst_vid_ifindex, vid);
    fib_id = AMTRL3_OM_GetFibIdByVid(vid);

    if(AMTRL3_OM_IsAddressEqualZero(&(host_entry->dst_inet_addr)))
        return FALSE;

    AMTRL3_MGR_CHECK_OPERATION_MODE(FALSE);

    if (amtrl3_mgr_hostroute_performance_flag)
        begin_timetick = SYSFUN_GetSysTick();
    if(host_entry->dst_inet_addr.addrlen ==0)
    {
        if(L_INET_ADDR_TYPE_IPV6 ==host_entry->dst_inet_addr.type
                ||L_INET_ADDR_TYPE_IPV6Z ==host_entry->dst_inet_addr.type)
            host_entry->dst_inet_addr.addrlen=SYS_ADPT_IPV6_ADDR_LEN;
        else
            host_entry->dst_inet_addr.addrlen=SYS_ADPT_IPV4_ADDR_LEN;
    }
    memset(&new_host_route_entry, 0, sizeof(AMTRL3_OM_HostRouteEntry_T));

    if(old_host_entry == NULL)
        memset(&old_host_route_entry, 0, sizeof(AMTRL3_OM_HostRouteEntry_T));
    else
        memcpy(&old_host_route_entry, old_host_entry, sizeof(AMTRL3_OM_HostRouteEntry_T));

    /* If lport == 0 or SYS_ADPT_DESTINATION_PORT_UNKNOWN, try to find
     * lport by dst_mac from AMTR
     */
    if (host_entry->lport == 0 ||
        host_entry->lport == SYS_ADPT_DESTINATION_PORT_UNKNOWN)
    {
        AMTR_TYPE_AddrEntry_T addr_entry;

        VLAN_IFINDEX_CONVERTTO_VID(host_entry->dst_vid_ifindex, addr_entry.vid);
        memcpy(addr_entry.mac, host_entry->dst_mac, SYS_ADPT_MAC_ADDR_LEN);

        if (AMTR_PMGR_GetExactAddrEntry(&addr_entry) == TRUE)
        {
            host_entry->lport = addr_entry.ifindex;
        }
        else
        {
            /* if can't find mac, we set lport to unkown*/
            host_entry->lport = SYS_ADPT_DESTINATION_PORT_UNKNOWN;
        }
    }

    if (amtrl3_mgr_debug_flag & DEBUG_LEVEL_TRACE_HOST)
    {
        AMTRL3_MGR_Ntoa(&(host_entry->dst_inet_addr), ip_str);

        BACKDOOR_MGR_Printf("AMTRL3_MGR_SetHostRoute IP: %s, port %d, type: %d, ",
                ip_str, (int)host_entry->lport, (int)type);
        BACKDOOR_MGR_Printf("MAC = %02x %02x %02x %02x %02x %02x\n",
                host_entry->dst_mac[0], host_entry->dst_mac[1], host_entry->dst_mac[2],
                host_entry->dst_mac[3], host_entry->dst_mac[4], host_entry->dst_mac[5]);
        BACKDOOR_MGR_Printf("(dst_inet_addr) type = %u, zone id = %lu,preflen=%u,addrlen=%u\r\n",
            host_entry->dst_inet_addr.type, (unsigned long)host_entry->dst_inet_addr.zoneid,
            host_entry->dst_inet_addr.preflen,host_entry->dst_inet_addr.addrlen);
    }

    /* Fill-in host route entry for new host entry
     */
    memset(&new_host_route_entry, 0, sizeof(AMTRL3_OM_HostRouteEntry_T));

    new_host_route_entry.key_fields = *host_entry;
    new_host_route_entry.uport = host_entry->lport;
    new_host_route_entry.entry_type = type;
    new_host_route_entry.arp_interval_index = 0;
    new_host_route_entry.hit_timestamp = (SYSFUN_GetSysTick() / SYS_BLD_TICKS_PER_SECOND);

    if(CHECK_FLAG(amtrl3_chip_capability_flags, AMTRL3_CHIP_KEEP_TRUNK_ID_IN_HOST_ROUTE))
        SWCTRL_POM_LogicalPortToUserPort(host_entry->lport, &unit, &port, &(new_host_route_entry.key_fields.trunk_id));

    /* Assign key for old host entry
     */
    if(old_host_entry == NULL)
    {
        old_host_route_entry.key_fields.dst_vid_ifindex = host_entry->dst_vid_ifindex;
        old_host_route_entry.key_fields.dst_inet_addr = host_entry->dst_inet_addr;
    }

    /* Reference existing host route entry for current host route entry status
     */
    if((old_host_entry != NULL) ||
        AMTRL3_OM_GetHostRouteEntry(action_flags, fib_id, &old_host_route_entry))
    {
        /* If AMTRL3_TYPE_FLAGS_NOT_OVERRIDE flag is on and old entry is different, just return TRUE
         */
        if (action_flags & AMTRL3_TYPE_FLAGS_NOT_OVERRIDE &&
            0 != memcmp(host_entry->dst_mac, old_host_route_entry.key_fields.dst_mac, SYS_ADPT_MAC_ADDR_LEN))
        {
            return TRUE;
        }

        if((new_host_route_entry.entry_type != old_host_route_entry.entry_type) &&
           (old_host_route_entry.entry_type != VAL_ipNetToPhysicalExtType_static) &&
           (AMTRL3_MGR_CheckHostCounterOverSpec(action_flags, fib_id, new_host_route_entry.entry_type) == TRUE))
        {
            return ret;
        }

        event_type = AMTRL3_MGR_CheckHostEntryForReplacement(action_flags, fib_id, &old_host_route_entry, &new_host_route_entry);

        if(event_type != HOST_ROUTE_DROP_EVENT)
        {
            new_host_route_entry.in_chip_status = old_host_route_entry.in_chip_status;
            new_host_route_entry.ref_count      = old_host_route_entry.ref_count;
            new_host_route_entry.ref_count_in_chip = old_host_route_entry.ref_count_in_chip;
            new_host_route_entry.status         = old_host_route_entry.status;
            new_host_route_entry.ecmp_ref_count = old_host_route_entry.ecmp_ref_count;
#if (SYS_CPNT_PBR == TRUE)
            new_host_route_entry.pbr_ref_count  = old_host_route_entry.pbr_ref_count;
#endif
#if (SYS_CPNT_VXLAN == TRUE)
            new_host_route_entry.vxlan_uc_ref_count = old_host_route_entry.vxlan_uc_ref_count;
            new_host_route_entry.vxlan_mc_ref_count = old_host_route_entry.vxlan_mc_ref_count;
#endif
            AMTRL3_MGR_UpdateCounterForReplacement(action_flags, fib_id, old_host_route_entry.entry_type, new_host_route_entry.entry_type);
        }
    }
    else
    {
        if((CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4) &&
            (AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_IPV4_TOTAL_IPNET2PHYSICAL_COUNTS) >= SYS_ADPT_MAX_NBR_OF_TOTAL_IPNET2PHYSICAL_IPV4_CACHE_ENTRY)) ||
           (CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6) &&
            (AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_IPV6_TOTAL_IPNET2PHYSICAL_COUNTS) >= SYS_ADPT_MAX_NBR_OF_TOTAL_IPNET2PHYSICAL_IPV6_CACHE_ENTRY)))
        {
            return ret;
        }

#if (SYS_CPNT_IP_TUNNEL == TRUE)
        {
            UI32_T  count=0;
            /* check if dynamic 6to4 host route reach the limit */
            if (host_entry->tunnel_entry_type == AMTRL3_TYPE_INETTOPHYSICALEXTTYPE_TUNNEL_6TO4 &&
                IS_TUNNEL_IFINDEX(host_entry->u.ip_tunnel.nexthop_vidifindex))
            {

                count = AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_IPV6_6to4_TUNNEL_HOST_ROUTE_COUNT);

                if(count >= SYS_ADPT_MAX_NBR_OF_TUNNEL_DYNAMIC_6to4_HOST_CACHE_ENTRY)
                {
                    if(amtrl3_mgr_debug_tunnel)
                        BACKDOOR_MGR_Printf("%s[%d]:dynamic host route reach the limit\r\n",__FUNCTION__,__LINE__);
                    return FALSE;
                }
            }
        }
#endif

        new_host_route_entry.hw_info = L_CVRT_UINT_TO_PTR(AMTRL3_OM_HW_INFO_INVALID);
#if (SYS_CPNT_VXLAN == TRUE)
        new_host_route_entry.vxlan_uc_hw_info = L_CVRT_UINT_TO_PTR(AMTRL3_OM_HW_INFO_INVALID);
        new_host_route_entry.vxlan_mc_hw_info = L_CVRT_UINT_TO_PTR(AMTRL3_OM_HW_INFO_INVALID);
#endif
        if (AMTRL3_MGR_CheckHostCounterOverSpec(action_flags, fib_id, new_host_route_entry.entry_type) == TRUE)
        {
            return ret;
        }

        /* Determine event_type base on input host route info
         */
        if(host_entry->lport == 0 ||
           host_entry->lport == SYS_ADPT_DESTINATION_PORT_UNKNOWN)
        {
            if((type  == VAL_ipNetToPhysicalExtType_local) ||
               (type  == VAL_ipNetToPhysicalExtType_vrrp))
            {
                event_type = HOST_ROUTE_READY_EVENT;
            }
#if (SYS_CPNT_IP_TUNNEL == TRUE)
 /*If tunnel nexthop resolved, change to ready*/
            else if((host_entry->tunnel_entry_type == AMTRL3_TYPE_INETTOPHYSICALEXTTYPE_TUNNEL_MANUAL ||
                     host_entry->tunnel_entry_type == AMTRL3_TYPE_INETTOPHYSICALEXTTYPE_TUNNEL_6TO4 ||
                     host_entry->tunnel_entry_type == AMTRL3_TYPE_INETTOPHYSICALEXTTYPE_TUNNEL_ISATAP) &&
                    (host_entry->u.ip_tunnel.nexthop_vidifindex != 0))
            {
                AMTRL3_OM_HostRouteEntry_T nexthop;
                memset(&nexthop,0,sizeof(nexthop));
                nexthop.key_fields.dst_inet_addr = host_entry->u.ip_tunnel.nexthop_inet_addr;
                nexthop.key_fields.dst_vid_ifindex = host_entry->u.ip_tunnel.nexthop_vidifindex;

                if (AMTRL3_OM_GetHostRouteEntry(AMTRL3_TYPE_FLAGS_IPV4, fib_id, &nexthop) == TRUE )
                {
                    if(nexthop.status ==HOST_ROUTE_HOST_READY ||
                        nexthop.status ==HOST_ROUTE_GATEWAY_READY ||
                        nexthop.status == HOST_ROUTE_READY_NOT_SYNC)
                    {
                        new_host_route_entry.key_fields.lport = nexthop.key_fields.lport;
                        new_host_route_entry.uport = nexthop.uport;
                        memcpy(new_host_route_entry.key_fields.dst_mac, nexthop.key_fields.dst_mac,SYS_ADPT_MAC_ADDR_LEN);
                        event_type = HOST_ROUTE_READY_EVENT;
                    }
                    else
                    {
                        event_type = HOST_ROUTE_PARTIAL_EVENT;
                    }

                }
                else
                {
                    //create new nexthop entry
                    nexthop.hw_info = L_CVRT_UINT_TO_PTR(AMTRL3_OM_HW_INFO_INVALID);
                    nexthop.entry_type = VAL_ipNetToPhysicalExtType_dynamic;
                    event_type = HOST_ROUTE_PARTIAL_EVENT;
                    AMTRL3_MGR_HostRouteEventHandler(AMTRL3_TYPE_FLAGS_IPV4, fib_id, HOST_ROUTE_PARTIAL_EVENT, &nexthop);
                }

                /*AMTRL3_MGR_HostRouteEventHandler(AMTRL3_TYPE_FLAGS_IPV4, fib_id, HOST_ROUTE_PARTIAL_EVENT, &nexthop);*/
            }
#endif /*SYS_CPNT_IP_TUNNEL*/
            else
            {
                event_type = HOST_ROUTE_PARTIAL_EVENT;
            }
        }
        else if ((memcmp(amtrl3_mgr_zero_mac, host_entry->dst_mac, SYS_ADPT_MAC_ADDR_LEN)== 0) ||
                 (host_entry->lport == 0) || (type == 0) || (host_entry->dst_vid_ifindex == 0))
        {
            event_type = HOST_ROUTE_PARTIAL_EVENT;
        }
        else
        {
            event_type = HOST_ROUTE_READY_EVENT;
        }
    } /* end of else */

    if (event_type == HOST_ROUTE_DROP_EVENT)
    {
        return ret;
    }

    ret = AMTRL3_MGR_HostRouteEventHandler(action_flags, fib_id, event_type, &new_host_route_entry);

    if (amtrl3_mgr_hostroute_performance_flag)
    {
        end_timetick = SYSFUN_GetSysTick();
        BACKDOOR_MGR_Printf("AMTRL3_MGR_SetHostRoute takes %d ticks\n", (int)(end_timetick - begin_timetick));
    }

    return ret;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME : AMTRL3_MGR_DeleteHostRoute
 * -------------------------------------------------------------------------
 * PURPOSE:
 *      Remove IPv4 or IPv6 neighbor entry base on given IpNetToPhsical entry information
 *
 * INPUT:
 *      action_flags    -- AMTRL3_TYPE_FLAGS_IPV4 \
 *                         AMTRL3_TYPE_FLAGS_IPV6
 *      fib_id          -- FIB id
 *      inet_host_entry -- delete the entry match the key fields in this structure
 *                         KEY: dst_vid_ifindex + dst_inet_addr
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE : AMTRL3_MGR_OK   - Successfully delete the entry.
 *      FALSE: AMTRL3_MGR_CAN_NOT_FIND   -- the host entry do not exist.
 *             AMTRL3_MGR_CAN_NOT_DELETE -- Error in removing specific entry
 *
 * NOTES:
 *      1. action flag: AMTRL3_TYPE_FLAGS_IPV4/IPV6 must consistent with
 *         address type in ip_net_to_physical_entry structure.
 -------------------------------------------------------------------------*/
BOOL_T  AMTRL3_MGR_DeleteHostRoute(UI32_T action_flags,
                                   UI32_T fib_id,
                                   AMTRL3_TYPE_InetHostRouteEntry_T *inet_host_entry)
{
    AMTRL3_OM_HostRouteEntry_T host_entry;
    UI8_T  ip_str[L_INET_MAX_IPADDR_STR_LEN] = {0};
    UI32_T address_type;

    AMTRL3_MGR_CHECK_OPERATION_MODE(FALSE);

    if(AMTRL3_MGR_GetFIBByID(fib_id) == NULL)
        return FALSE;

    address_type = inet_host_entry->dst_inet_addr.type;
    if((action_flags == AMTRL3_TYPE_FLAGS_IPV4) &&
       (address_type != L_INET_ADDR_TYPE_IPV4) &&
       (address_type != L_INET_ADDR_TYPE_IPV4Z))
        return FALSE;
    if((action_flags == AMTRL3_TYPE_FLAGS_IPV6) &&
       (address_type != L_INET_ADDR_TYPE_IPV6) &&
       (address_type != L_INET_ADDR_TYPE_IPV6Z))
        return FALSE;

    memset(&host_entry, 0, sizeof(AMTRL3_OM_HostRouteEntry_T));
    host_entry.key_fields.dst_vid_ifindex = inet_host_entry->dst_vid_ifindex;
    host_entry.key_fields.dst_inet_addr = inet_host_entry->dst_inet_addr;

    if (amtrl3_mgr_debug_flag & DEBUG_LEVEL_TRACE_HOST)
    {
        AMTRL3_MGR_Ntoa(&(inet_host_entry->dst_inet_addr), ip_str);
        BACKDOOR_MGR_Printf("AMTRL3_MGR_DeleteHostRoute: IP: %s, if_index: %d\n",ip_str, (int)inet_host_entry->dst_vid_ifindex);
    }

    if (AMTRL3_OM_GetHostRouteEntry(action_flags, fib_id, &host_entry) != TRUE)
    {
        return TRUE;
    }

    /* Gateway entry shall not be removed
     */
    if (host_entry.ref_count > 0)
    {
        if(host_entry.entry_type == VAL_ipNetToMediaExtType_static)
        {
            host_entry.entry_type = VAL_ipNetToMediaExtType_dynamic;

            if(action_flags == AMTRL3_TYPE_FLAGS_IPV4)
            {
                AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_IPV4_STATIC_IPNET2PHYSICAL_COUNTS, 1);
                AMTRL3_OM_IncreaseDatabaseValue(fib_id, AMTRL3_OM_IPV4_DYNAMIC_IPNET2PHYSICAL_COUNTS, 1);
            }
            else if(action_flags == AMTRL3_TYPE_FLAGS_IPV6)
            {
                AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_IPV6_STATIC_IPNET2PHYSICAL_COUNTS, 1);
                AMTRL3_OM_IncreaseDatabaseValue(fib_id, AMTRL3_OM_IPV6_DYNAMIC_IPNET2PHYSICAL_COUNTS, 1);
            }
        }

        return AMTRL3_MGR_HostRouteEventHandler(action_flags, fib_id, HOST_ROUTE_UNREFERENCE_EVENT, &host_entry);
    }
    else
    {
        /* Appropriate host route status should be determined in MGR thru the
         * Finite State Machine before passing to OM for data storage.
         */
        return AMTRL3_MGR_HostRouteEventHandler(action_flags, fib_id, HOST_ROUTE_REMOVE_EVENT, &host_entry);
    }
}

#if (SYS_CPNT_PBR == TRUE)
/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_SetHostRouteForPbr
 * -------------------------------------------------------------------------
 * PURPOSE:  Add a host route entry in OM if it doesn't exist yet
 * INPUT:    action_flags   -- AMTRL3_TYPE_FLAGS_IPV4
 *           fib_id         -- FIB id
 *           host_entry     -- host route entry
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE
 * NOTES:    1. dst_mac is ignored.
 *           2. do address resolution on dst_vid_ifindex.
 *           3. increase ref_count & pbr_ref_count, and do address resolution(ARP)
 * -------------------------------------------------------------------------
 */
BOOL_T AMTRL3_MGR_SetHostRouteForPbr(UI32_T action_flags,
                                     UI32_T fib_id,
                                     AMTRL3_TYPE_InetHostRouteEntry_T *host_entry)
{
    AMTRL3_OM_HostRouteEntry_T host_route_entry;

    if (host_entry == NULL)
        return FALSE;

    if (amtrl3_mgr_debug_flag & DEBUG_LEVEL_TRACE_HOST)
    {
        UI8_T ip_str[L_INET_MAX_IPADDR_STR_LEN] = {};
        AMTRL3_MGR_Ntoa(&(host_entry->dst_inet_addr), ip_str);
        BACKDOOR_MGR_Printf("%s: ifindex: %ld, IP %s, ", __FUNCTION__, (long)host_entry->dst_vid_ifindex, ip_str);
    }

    AMTRL3_MGR_CHECK_OPERATION_MODE(FALSE);

    if (AMTRL3_MGR_GetFIBByID(fib_id) == NULL)
        return FALSE;

    if (AMTRL3_OM_IsAddressEqualZero(&host_entry->dst_inet_addr))
        return FALSE;

    memset(&host_route_entry, 0, sizeof(host_route_entry));
    host_route_entry.key_fields = *host_entry;

    /* If the nexthop host entry already exist in OM, get it out.
     * If not exist, threat it as dynamic and set it to OM. */
    if (AMTRL3_OM_GetHostRouteEntry(action_flags, fib_id, &host_route_entry) == FALSE)
    {
        host_route_entry.entry_type = VAL_ipNetToPhysicalExtType_dynamic;
        host_route_entry.hw_info = L_CVRT_UINT_TO_PTR(AMTRL3_OM_HW_INFO_INVALID);
    }

    host_route_entry.ref_count++;
    host_route_entry.pbr_ref_count++;

    if (amtrl3_mgr_debug_flag & DEBUG_LEVEL_TRACE_HOST)
    {
        BACKDOOR_MGR_Printf("%s[%d], pbr_ref_count: %ld\r\n", __FUNCTION__, __LINE__, (long)host_route_entry.pbr_ref_count);
    }

    return AMTRL3_MGR_HostRouteEventHandler(action_flags, fib_id, HOST_ROUTE_PARTIAL_EVENT, &host_route_entry);
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_DeleteHostRouteForPbr
 * -------------------------------------------------------------------------
 * PURPOSE:  Unreference host route entry.
 * INPUT:    action_flags   -- AMTRL3_TYPE_FLAGS_IPV4
 *           fib_id         -- FIB id
 *           host_entry     -- host route entry
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE
 * NOTES:    1. decrease the ref_count & pbr_ref_count
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_MGR_DeleteHostRouteForPbr(UI32_T action_flags,
                                        UI32_T fib_id,
                                        AMTRL3_TYPE_InetHostRouteEntry_T *host_entry)
{
    AMTRL3_OM_HostRouteEntry_T host_route_entry;
    UI32_T event;

    if (host_entry == NULL)
        return FALSE;

    if (amtrl3_mgr_debug_flag & DEBUG_LEVEL_TRACE_HOST)
    {
        UI8_T ip_str[L_INET_MAX_IPADDR_STR_LEN] = {};
        AMTRL3_MGR_Ntoa(&(host_entry->dst_inet_addr), ip_str);
        BACKDOOR_MGR_Printf("%s: ifindex: %ld, IP %s, ", __FUNCTION__, (long)host_entry->dst_vid_ifindex, ip_str);
    }

    AMTRL3_MGR_CHECK_OPERATION_MODE(FALSE);

    if (AMTRL3_MGR_GetFIBByID(fib_id) == NULL)
        return FALSE;

    if (AMTRL3_OM_IsAddressEqualZero(&host_entry->dst_inet_addr))
        return FALSE;

    memset(&host_route_entry, 0, sizeof(host_route_entry));
    host_route_entry.key_fields = *host_entry;

    if (AMTRL3_OM_GetHostRouteEntry(action_flags, fib_id, &host_route_entry) == FALSE)
    {
        return FALSE;
    }

    if (host_route_entry.pbr_ref_count > 0)
    {
        host_route_entry.pbr_ref_count--;
        if (host_route_entry.ref_count > 0)
            host_route_entry.ref_count--;
    }

    if (amtrl3_mgr_debug_flag & DEBUG_LEVEL_TRACE_HOST)
    {
        BACKDOOR_MGR_Printf("%s[%d], pbr_ref_count: %ld\r\n", __FUNCTION__, __LINE__, (long)host_route_entry.pbr_ref_count);
    }

    if (host_route_entry.pbr_ref_count == 0 && host_route_entry.ref_count == 0)
        return AMTRL3_MGR_HostRouteEventHandler(action_flags, fib_id, HOST_ROUTE_UNREFERENCE_EVENT, &host_route_entry);
    else
        return AMTRL3_MGR_HostRouteEventHandler(action_flags, fib_id, HOST_ROUTE_PARTIAL_EVENT, &host_route_entry);
}

#endif

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_DeleteHostRouteEntryByLport
 * -------------------------------------------------------------------------
 * PURPOSE:  This function removes all host route entry with destination
 *           port to the designated port
 * INPUT:    fib_id - point to the database of a FIB
 *           lport  - lport_ifindex
 * OUTPUT:   none.
 * RETURN:   TRUE  - This net entry is in the same direction with default route.
 *           FALSE - This net entry is not in the same direction with default route.
 * NOTES:    1. Delete all host entry including IPv4 and IPv6 entries.
 *           2. If device learn a lot of host entry from this lport, shall we consider
 *              only delete part of them in one action.
 * -------------------------------------------------------------------------*/
static void AMTRL3_MGR_DeleteHostRouteEntryByLport(UI32_T fib_id, UI32_T lport)
{
    /* Local Variable Declaration
     */
    AMTRL3_OM_HostRouteEntry_T   host_route_entry;
    UI32_T      process_count = 0;
    UI32_T      action_flags = 0;
    UI32_T      address_type;
    AMTR_TYPE_AddrEntry_T amtr_addr_entry;
    UI32_T      tmp_vid;

    /* BODY
     */
    if(AMTRL3_MGR_GetFIBByID(fib_id) == NULL)
        return;

    memset(&host_route_entry, 0, sizeof(AMTRL3_OM_HostRouteEntry_T));
    host_route_entry.key_fields.lport = lport;

    if ((amtrl3_mgr_callback_debug_flag & CALLBACK_DEBUG_LPORT_DOWN) &&
        (amtrl3_mgr_callback_debug_flag & CALLBACK_DEBUG_DETAIL))
        BACKDOOR_MGR_Printf("AMTRL3_MGR_DeleteHostRouteEntryByLport %d\n", (int)lport);

    while(AMTRL3_OM_GetNextHostRouteEntryByLport(AMTRL3_TYPE_FLAGS_IPV4 | AMTRL3_TYPE_FLAGS_IPV6,
                                                 fib_id,
                                                 &host_route_entry))
    {
        if(host_route_entry.key_fields.lport != lport)
            break;

        memset(&amtr_addr_entry, 0, sizeof(AMTR_TYPE_AddrEntry_T));
        memcpy(amtr_addr_entry.mac,host_route_entry.key_fields.dst_mac,SYS_ADPT_MAC_ADDR_LEN);
        VLAN_OM_ConvertFromIfindex(host_route_entry.key_fields.dst_vid_ifindex, &tmp_vid);
        amtr_addr_entry.vid = (UI16_T)tmp_vid;
        if(AMTRL3_MGR_GetAMTRExactMAC(&amtr_addr_entry))
        {
            if (amtr_addr_entry.life_time == AMTR_TYPE_ADDRESS_LIFETIME_PERMANENT ||
                amtr_addr_entry.life_time == AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_RESET)
            {
                continue;
            }
        }

        address_type = host_route_entry.key_fields.dst_inet_addr.type;
        if((address_type == L_INET_ADDR_TYPE_IPV4) || (address_type == L_INET_ADDR_TYPE_IPV4Z))
            action_flags = AMTRL3_TYPE_FLAGS_IPV4;
        else if((address_type == L_INET_ADDR_TYPE_IPV6) || (address_type == L_INET_ADDR_TYPE_IPV6Z))
            action_flags = AMTRL3_TYPE_FLAGS_IPV6;

        /* 1. Remove host route entry from chip
         * 2. Update host route status to Unreference or unresolved
         * 3. Removal of host route entry should dependeds on ARP age-out process
         */
        if (!AMTRL3_MGR_HostRouteEventHandler(action_flags, fib_id, HOST_ROUTE_UNREFERENCE_EVENT, &host_route_entry))
        {
            if (amtrl3_mgr_debug_flag & DEBUG_LEVEL_ERROR)
            {
                UI8_T ip_str[L_INET_MAX_IPADDR_STR_LEN] = {0};

                AMTRL3_MGR_Ntoa(&(host_route_entry.key_fields.dst_inet_addr), ip_str);
                BACKDOOR_MGR_Printf("AMTRL3_MGR_HostRouteEventHandler Fails %s\n", ip_str);
            }
        }
        process_count++;
    } /* end of while */

    if ((amtrl3_mgr_callback_debug_flag & CALLBACK_DEBUG_LPORT_DOWN) &&
        (amtrl3_mgr_callback_debug_flag & CALLBACK_DEBUG_DETAIL))
        BACKDOOR_MGR_Printf("AMTRL3_MGR_DeleteHostRouteEntryByLport deletes %d entries\n", (int)process_count);

    return;
} /* end of AMTRL3_MGR_DeleteHostRouteEntryByLport() */

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_ReplaceExistHostRoute
 * -------------------------------------------------------------------------
 * PURPOSE:  Modify an existing host route, this function will search the host
 *           table, if the IP existed, then update the entry based on the new
 *           information, if the IP is not exist in the table then do nothing.
 * INPUT:    action_flags    --  AMTRL3_TYPE_FLAGS_IPV4 \
 *                               AMTRL3_TYPE_FLAGS_IPV6
 *           fib_id       --  FIB id
 *           host_entry      --  host route entry
 *           type            --  VAL_ipNetToPhysicalExtType_other     \
 *                               VAL_ipNetToPhysicalExtType_invalid   \
 *                               VAL_ipNetToPhysicalExtType_dynamic   \
 *                               VAL_ipNetToPhysicalExtType_static    \
 *                               VAL_ipNetToPhysicalExtType_local     \
 *                               VAL_ipNetToPhysicalExtType_broadcast \
 *                               VAL_ipNetToPhysicalExtType_vrrp      \
 * OUTPUT:   none.
 * RETURN:   TRUE / FALSE
 * NOTE:
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_MGR_ReplaceExistHostRoute(UI32_T action_flags,
                                        UI32_T fib_id,
                                        AMTRL3_TYPE_InetHostRouteEntry_T *host_entry,
                                        UI32_T type)
{
     /* Local Variable Declaration
     */
    AMTRL3_OM_HostRouteEntry_T          old_host_route_entry;

    /* BODY
     */
    if(AMTRL3_MGR_GetFIBByID(fib_id) == NULL)
        return FALSE;

    AMTRL3_MGR_CHECK_OPERATION_MODE(FALSE);

    memset(&old_host_route_entry, 0, sizeof(AMTRL3_OM_HostRouteEntry_T));

    /* Assign key for old host entry
     */
    old_host_route_entry.key_fields.dst_vid_ifindex = host_entry->dst_vid_ifindex;
    old_host_route_entry.key_fields.dst_inet_addr = host_entry->dst_inet_addr;

    /* Reference existing host route entry for current host route entry status
     */
    if (TRUE == AMTRL3_OM_GetHostRouteEntry(action_flags, fib_id, &old_host_route_entry))
    {
        return AMTRL3_MGR_SetHostRoute(action_flags, fib_id, host_entry, &old_host_route_entry, type);
    }
    else
    {
        return FALSE;
    }
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_CheckHostCounterOverSpec
 * -------------------------------------------------------------------------
 * PURPOSE:  This function checks host entry type to validate whether
 *           new entry shall over spec. or not.
 * INPUT:    action_flags - AMTRL3_TYPE_FLAGS_IPV4 \
 *                          AMTRL3_TYPE_FLAGS_IPV6
 *           fib_id - point to the database of a FIB
 *           type - the type of new host route entry
 * OUTPUT:   None.
 * RETURN:   TRUE - over spec.
 *           FALSE - under spec.
 * NOTES:    None
 * -------------------------------------------------------------------------*/
static BOOL_T AMTRL3_MGR_CheckHostCounterOverSpec(UI32_T action_flags,
                                                  UI32_T fib_id,
                                                  UI32_T type)
{
    BOOL_T ret = FALSE;

    if(AMTRL3_MGR_GetFIBByID(fib_id) == NULL)
        return FALSE;

    switch (type)
    {
        case VAL_ipNetToPhysicalExtType_static:
            if ((CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4) &&
                 (AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_IPV4_STATIC_IPNET2PHYSICAL_COUNTS) >= SYS_ADPT_MAX_NBR_OF_STATIC_IPNET2PHYSICAL_IPV4_CACHE_ENTRY)) ||
                (CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6) &&
                 (AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_IPV6_STATIC_IPNET2PHYSICAL_COUNTS) >= SYS_ADPT_MAX_NBR_OF_STATIC_IPNET2PHYSICAL_IPV6_CACHE_ENTRY)))
            {
                ret = TRUE;
            }
            break;
        case VAL_ipNetToPhysicalExtType_dynamic:
            if ((CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4) &&
                (AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_IPV4_DYNAMIC_IPNET2PHYSICAL_COUNTS) >= SYS_ADPT_MAX_NBR_OF_DYNAMIC_IPNET2PHYSICAL_IPV4_CACHE_ENTRY)) ||
                (CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6) &&
                (AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_IPV6_DYNAMIC_IPNET2PHYSICAL_COUNTS) >= SYS_ADPT_MAX_NBR_OF_DYNAMIC_IPNET2PHYSICAL_IPV6_CACHE_ENTRY)))
            {
                ret = TRUE;
            }
            break;
        default:
            break;
    }

    return (ret);
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_CheckHostEntryForReplacement
 * -------------------------------------------------------------------------
 * PURPOSE:  This function checks host entry attributes to validate whether
 *           new entry shall replace current entry or not.
 * INPUT:    action_flags - AMTRL3_TYPE_FLAGS_IPV4 \
 *                          AMTRL3_TYPE_FLAGS_IPV6
 *           fib_id - point to the database of a FIB
 *           old_entry - Current host route entry
 *           new_entry - new host route entry
 * OUTPUT:   none.
 * RETURN:   HOST_ROUTE_READY_EVENT   \
 *           HOST_ROUTE_PARTIAL_EVENT \
 *           HOST_ROUTE_DROP_EVENT
 * NOTES:    None
 * -------------------------------------------------------------------------*/
static UI32_T AMTRL3_MGR_CheckHostEntryForReplacement(UI32_T action_flags,
                                                      UI32_T fib_id,
                                                      AMTRL3_OM_HostRouteEntry_T *old_entry,
                                                      AMTRL3_OM_HostRouteEntry_T *new_entry)
{
    /* LOCAL VARIABLE DECLARATION
     */
    /* BODY
     */
    if(AMTRL3_MGR_GetFIBByID(fib_id) == NULL)
        return HOST_ROUTE_DROP_EVENT;

    if ((old_entry == NULL) || (new_entry == NULL))
        return HOST_ROUTE_DROP_EVENT;

    /* case 1: 1. Handle IP Data Packet, ARP in software but not in chip,
     *            currently, this case not happen in Linux platform
     *         2. Configure static ARP, lport unknown.
     */
    if ((new_entry->key_fields.lport == SYS_ADPT_DESTINATION_PORT_UNKNOWN) &&
        (new_entry->entry_type  != VAL_ipNetToPhysicalExtType_local))
    {
        if(old_entry->entry_type != new_entry->entry_type)
        {
            /* Static arp has age out, now need forward ip date to that host.
             * So need resolve this entry and add to chip again
             */
            /* Currently, in Linux platform,
             * 1. Gateway entry does not been removed from chip when age out,
             * 2. No such dynamic ARP (lport unknown) comes in
             */
            #if 0
            if(old_entry->entry_type == VAL_ipNetToPhysicalExtType_static &&
               new_entry->entry_type == VAL_ipNetToPhysicalExtType_dynamic)
            {
                /* To avoid update entry which has already in chip with status of
                 * HOST_READY or GATEWAY_READY.
                 * Added by Vai.Wang 2/25/2005
                 */
                if((old_entry->status == HOST_ROUTE_HOST_READY) ||
                    (old_entry->status == HOST_ROUTE_GATEWAY_READY) ||
                    (old_entry->status == HOST_ROUTE_READY_NOT_SYNC))
                {
                    return HOST_ROUTE_DROP_EVENT;
                }
                else
                {
                    new_entry->entry_type = VAL_ipNetToPhysicalExtType_static;
                    return HOST_ROUTE_PARTIAL_EVENT;
                }
            }
            #endif

            /* Add static ARP and original dynamic ARP is not in READY status.*/
            if(old_entry->entry_type == VAL_ipNetToPhysicalExtType_dynamic &&
               new_entry->entry_type == VAL_ipNetToPhysicalExtType_static)
            {
                if(memcmp(old_entry->key_fields.dst_mac, new_entry->key_fields.dst_mac, SYS_ADPT_MAC_ADDR_LEN) == 0)
                {
                    /* For the entry that its status is READY, inherit information from
                     * old host entry and only need change entry type to STATIC*/
                    if((old_entry->status == HOST_ROUTE_HOST_READY)    ||
                       (old_entry->status == HOST_ROUTE_GATEWAY_READY) ||
                       (old_entry->status == HOST_ROUTE_READY_NOT_SYNC))
                    {
                        memcpy(new_entry, old_entry, sizeof(AMTRL3_OM_HostRouteEntry_T));
                        new_entry->entry_type = VAL_ipNetToPhysicalExtType_static;
                    }
                    new_entry->hw_info = old_entry->hw_info;
#if (SYS_CPNT_VXLAN == TRUE)
                    new_entry->vxlan_uc_hw_info = old_entry->vxlan_uc_hw_info;
                    new_entry->vxlan_mc_hw_info = old_entry->vxlan_mc_hw_info;
#endif
                    return HOST_ROUTE_PARTIAL_EVENT;
                }
                else
                {
                    /* Add a static entry with different MAC with the original dynamic one.*/
                    AMTRL3_MGR_HostRouteEventHandler(action_flags, fib_id, HOST_ROUTE_UNREFERENCE_EVENT, old_entry);
                    new_entry->hw_info = old_entry->hw_info;
#if (SYS_CPNT_VXLAN == TRUE)
                    new_entry->vxlan_uc_hw_info = old_entry->vxlan_uc_hw_info;
                    new_entry->vxlan_mc_hw_info = old_entry->vxlan_mc_hw_info;
#endif
                    return HOST_ROUTE_PARTIAL_EVENT;
                }
            }
        }
        else /* same type */
        {
            /* Currently, in Linux platform no such case happen */
            #if 0
            if(new_entry->entry_type != VAL_ipNetToPhysicalExtType_static)
            {
               /* Need to allow new host entry to inherit information from
                * old host entry if new entry carries invalid fields.
                */
                if ((old_entry->status == HOST_ROUTE_HOST_READY)    ||
                    (old_entry->status == HOST_ROUTE_GATEWAY_READY) ||
                    (old_entry->status == HOST_ROUTE_READY_NOT_SYNC))
                {
                    new_entry->key_fields.lport= old_entry->key_fields.lport;
                    new_entry->uport = old_entry->uport;
                    new_entry->in_chip_status = old_entry->in_chip_status;
                    memcpy(new_entry->key_fields.dst_mac,
                           old_entry->key_fields.dst_mac,
                           (sizeof(UI8_T)*SYS_ADPT_MAC_ADDR_LEN));
                } /* end of if */
                return HOST_ROUTE_PARTIAL_EVENT;
            }
            else
            #endif

#if (SYS_CPNT_IP_TUNNEL == TRUE)
            if (new_entry->key_fields.tunnel_entry_type == AMTRL3_TYPE_INETTOPHYSICALEXTTYPE_TUNNEL_MANUAL ||
                new_entry->key_fields.tunnel_entry_type == AMTRL3_TYPE_INETTOPHYSICALEXTTYPE_TUNNEL_ISATAP ||
                new_entry->key_fields.tunnel_entry_type == AMTRL3_TYPE_INETTOPHYSICALEXTTYPE_TUNNEL_6TO4)
            {
                /* tunnel's patch:
                 * copy lport&uport, destination mac, hwinfo from old entry,
                 * or some information will be overwited by new host entry
                 */
                new_entry->key_fields.lport = old_entry->key_fields.lport;
                new_entry->uport = old_entry->uport;
                new_entry->hw_info = old_entry->hw_info;
                new_entry->hw_tunnel_index = old_entry->hw_tunnel_index;

                memcpy(new_entry->key_fields.dst_mac, old_entry->key_fields.dst_mac, SYS_ADPT_MAC_ADDR_LEN);

                if(amtrl3_mgr_debug_tunnel)
                    BACKDOOR_MGR_Printf("%s[%d],return HOST_ROUTE_PARTIAL_EVENT\r\n",__FUNCTION__,__LINE__);

                return HOST_ROUTE_PARTIAL_EVENT;
            }
#endif /* #if (SYS_CPNT_IP_TUNNEL == TRUE) */
#if (SYS_CPNT_VXLAN == TRUE)
       // TODO, add something here KH_SHI
#endif

            if(new_entry->entry_type == VAL_ipNetToPhysicalExtType_static)
            {
                /* Config a static entry just same with an exist static entry*/
                if(memcmp(old_entry->key_fields.dst_mac, new_entry->key_fields.dst_mac, SYS_ADPT_MAC_ADDR_LEN) == 0)
                {
                    return HOST_ROUTE_DROP_EVENT;
                }
                /* Config a static entry with different mac with old static entry*/
                else
                {
                    AMTRL3_MGR_HostRouteEventHandler(action_flags, fib_id, HOST_ROUTE_UNREFERENCE_EVENT, old_entry);
                    new_entry->hw_info = old_entry->hw_info;
#if (SYS_CPNT_VXLAN == TRUE)
                    new_entry->vxlan_uc_hw_info = old_entry->vxlan_uc_hw_info;
                    new_entry->vxlan_mc_hw_info = old_entry->vxlan_mc_hw_info;
#endif
                    return HOST_ROUTE_PARTIAL_EVENT;
                }
            }
        }
    } /* end of if */

    /* case 2: Duplicate Event
     */
    if (AMTRL3_OM_IsAddressEqual(&(old_entry->key_fields.dst_inet_addr), &(new_entry->key_fields.dst_inet_addr)) &&
        (old_entry->key_fields.dst_vid_ifindex == new_entry->key_fields.dst_vid_ifindex) &&
        (old_entry->key_fields.lport == new_entry->key_fields.lport) &&
        (old_entry->entry_type      == new_entry->entry_type)      &&
        ((old_entry->status         == HOST_ROUTE_HOST_READY)      ||
         (old_entry->status         == HOST_ROUTE_GATEWAY_READY)   ||
         (old_entry->status         == HOST_ROUTE_READY_NOT_SYNC)) &&
        (memcmp(old_entry->key_fields.dst_mac, new_entry->key_fields.dst_mac, SYS_ADPT_MAC_ADDR_LEN) == 0))
    {
        /* CHIP does not refresh host entry hitbit upon ARP Reply because
         * only destination IP is used to search L3_entry for unicast packet.
         * In order to guarantee that gateway does not reach ageout time,
         * timestamp of gateway entry shall be updated by software when add
         * a host route.
         */
        if ((old_entry->status == HOST_ROUTE_GATEWAY_READY) ||
            (old_entry->status == HOST_ROUTE_READY_NOT_SYNC))
        {
            new_entry->hw_info = old_entry->hw_info;
#if (SYS_CPNT_VXLAN == TRUE)
            new_entry->vxlan_uc_hw_info = old_entry->vxlan_uc_hw_info;
            new_entry->vxlan_mc_hw_info = old_entry->vxlan_mc_hw_info;
#endif
            return HOST_ROUTE_PARTIAL_EVENT;
        }
        /*If recieve duplicate event,also need send to TCP/IP stack for send ARP reply.change HOST_ROUTE_DROP_EVENT to HOST_ROUTE_READY_EVENT --xiongyu 20081105*/
        else
        {
            /* added by steven.gao */
            new_entry->hw_info = old_entry->hw_info;
#if (SYS_CPNT_VXLAN == TRUE)
            new_entry->vxlan_uc_hw_info = old_entry->vxlan_uc_hw_info;
            new_entry->vxlan_mc_hw_info = old_entry->vxlan_mc_hw_info;
#endif
            return HOST_ROUTE_READY_EVENT;
        }
    } /* end of if */

    /* case 3: Host route entry type replacement priority
     */
    if (!AMTRL3_MGR_CheckHostTypeForReplacement(old_entry, new_entry))
    {
        return HOST_ROUTE_DROP_EVENT;
    }

    new_entry->hw_info = old_entry->hw_info;
#if (SYS_CPNT_VXLAN == TRUE)
    new_entry->vxlan_uc_hw_info = old_entry->vxlan_uc_hw_info;
    new_entry->vxlan_mc_hw_info = old_entry->vxlan_mc_hw_info;
#endif

    return HOST_ROUTE_READY_EVENT;
} /* end of AMTRL3_MGR_CheckHostEntryForReplacement() */

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_UpdateCounterForReplacement
 * -------------------------------------------------------------------------
 * PURPOSE:  This function updates dynamic and static counter for successful
 *           replacement.
 * INPUT:    action_flags - AMTRL3_TYPE_FLAGS_IPV4 \
 *                          AMTRL3_TYPE_FLAGS_IPV6
 *           fib_id - FIB id
 *           old_type - entry type of original host route entry.
 *           new_type - entry type of new host route entry.
 * OUTPUT:   none.
 * RETURN:   none.
 * NOTES:    2 flags can not be set at the same time
 * -------------------------------------------------------------------------*/
static void AMTRL3_MGR_UpdateCounterForReplacement(UI32_T action_flags, UI32_T fib_id, UI32_T old_type, UI32_T new_type)
{
    if(AMTRL3_MGR_GetFIBByID(fib_id) == NULL)
        return;

    switch (old_type)
    {
        case VAL_ipNetToPhysicalExtType_static:
            if(action_flags == AMTRL3_TYPE_FLAGS_IPV4)
            {
                AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_IPV4_STATIC_IPNET2PHYSICAL_COUNTS, 1);
                AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_IPV4_TOTAL_IPNET2PHYSICAL_COUNTS, 1);
            }
            else if(action_flags == AMTRL3_TYPE_FLAGS_IPV6)
            {
                AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_IPV6_STATIC_IPNET2PHYSICAL_COUNTS, 1);
                AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_IPV6_TOTAL_IPNET2PHYSICAL_COUNTS, 1);
            }
            break;
        case VAL_ipNetToPhysicalExtType_dynamic:
            if(action_flags == AMTRL3_TYPE_FLAGS_IPV4)
            {
                AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_IPV4_DYNAMIC_IPNET2PHYSICAL_COUNTS, 1);
                AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_IPV4_TOTAL_IPNET2PHYSICAL_COUNTS, 1);
            }
            else if(action_flags == AMTRL3_TYPE_FLAGS_IPV6)
            {
                AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_IPV6_DYNAMIC_IPNET2PHYSICAL_COUNTS, 1);
                AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_IPV6_TOTAL_IPNET2PHYSICAL_COUNTS, 1);
            }

            break;
        default:
            break;
    } /* end of switch */

    switch (new_type)
    {
        case VAL_ipNetToPhysicalExtType_static:
            if(action_flags == AMTRL3_TYPE_FLAGS_IPV4)
            {
                AMTRL3_OM_IncreaseDatabaseValue(fib_id, AMTRL3_OM_IPV4_STATIC_IPNET2PHYSICAL_COUNTS, 1);
                AMTRL3_OM_IncreaseDatabaseValue(fib_id, AMTRL3_OM_IPV4_TOTAL_IPNET2PHYSICAL_COUNTS, 1);
            }
            else if(action_flags == AMTRL3_TYPE_FLAGS_IPV6)
            {
                AMTRL3_OM_IncreaseDatabaseValue(fib_id, AMTRL3_OM_IPV6_STATIC_IPNET2PHYSICAL_COUNTS, 1);
                AMTRL3_OM_IncreaseDatabaseValue(fib_id, AMTRL3_OM_IPV6_TOTAL_IPNET2PHYSICAL_COUNTS, 1);
            }
            break;
        case VAL_ipNetToPhysicalExtType_dynamic:
            if(action_flags == AMTRL3_TYPE_FLAGS_IPV4)
            {
                AMTRL3_OM_IncreaseDatabaseValue(fib_id, AMTRL3_OM_IPV4_DYNAMIC_IPNET2PHYSICAL_COUNTS, 1);
                AMTRL3_OM_IncreaseDatabaseValue(fib_id, AMTRL3_OM_IPV4_TOTAL_IPNET2PHYSICAL_COUNTS, 1);
            }
            else if(action_flags == AMTRL3_TYPE_FLAGS_IPV6)
            {
                AMTRL3_OM_IncreaseDatabaseValue(fib_id, AMTRL3_OM_IPV6_DYNAMIC_IPNET2PHYSICAL_COUNTS, 1);
                AMTRL3_OM_IncreaseDatabaseValue(fib_id, AMTRL3_OM_IPV6_TOTAL_IPNET2PHYSICAL_COUNTS, 1);
            }
            break;
        default:
            break;
    } /* end of switch */
    return;
} /* end of AMTRL3_MGR_UpdateCounterForReplacement() */

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_CheckHostTypeForReplacement
 * -------------------------------------------------------------------------
 * PURPOSE:  This function checks type for host entry to ensure entry type of
 *           lower priority does not override entry type of higher priority.
 * INPUT:    fib_id - point to the database of a FIB
 *           old_type - entry type of original host route entry.
 *           new_type - entry type of new host route entry.
 * OUTPUT:   none.
 * RETURN:   TRUE  - Replaceable
 *           FALSE - Ir-replaceable
 * NOTES:    The priority of ARP entry type is as follows:
 *           local > FIBRP > static / Dynamic
 *           Hence the value of new ARP entry shall be check against
 *           existing ARP entry
 * -------------------------------------------------------------------------*/
static BOOL_T AMTRL3_MGR_CheckHostTypeForReplacement(AMTRL3_OM_HostRouteEntry_T *old_entry, AMTRL3_OM_HostRouteEntry_T *new_entry)
{

    /* Check to see if this should add check tunnel type */
    switch (old_entry->entry_type)
    {
        case VAL_ipNetToPhysicalExtType_local:
            if (new_entry->entry_type == VAL_ipNetToPhysicalExtType_vrrp)
            {
                new_entry->entry_type = VAL_ipNetToPhysicalExtType_local;
                return TRUE;
            }
            else if (new_entry->entry_type == VAL_ipNetToPhysicalExtType_local)
                return TRUE;
#if 0  /* local host entry will not have tunnel type */
#if (SYS_CPNT_IP_TUNNEL == TRUE)
            else if(new_entry->key_fields.tunnel_entry_type == AMTRL3_TYPE_INETTOPHYSICALEXTTYPE_TUNNEL_ISATAP ||
                    new_entry->key_fields.tunnel_entry_type == AMTRL3_TYPE_INETTOPHYSICALEXTTYPE_TUNNEL_6TO4 ||
                    new_entry->key_fields.tunnel_entry_type == AMTRL3_TYPE_INETTOPHYSICALEXTTYPE_TUNNEL_MANUAL
                    )
                return TRUE;
#endif /*SYS_CPNT_IP_TUNNEL*/
#endif
            break;
        case VAL_ipNetToPhysicalExtType_vrrp:
            if (new_entry->entry_type == VAL_ipNetToPhysicalExtType_local)
                return TRUE;
            else if (new_entry->entry_type == VAL_ipNetToPhysicalExtType_vrrp)
                return TRUE;
            break;
        case VAL_ipNetToPhysicalExtType_static:
            if (new_entry->entry_type == VAL_ipNetToPhysicalExtType_local)
                return TRUE;
            else if (new_entry->entry_type == VAL_ipNetToPhysicalExtType_static)
                return TRUE;
            else if (new_entry->entry_type == VAL_ipNetToPhysicalExtType_vrrp)
                return TRUE;
            else if (new_entry->entry_type == VAL_ipNetToPhysicalExtType_dynamic)
            {
                if ((old_entry->key_fields.dst_vid_ifindex == new_entry->key_fields.dst_vid_ifindex) &&
                    (memcmp(old_entry->key_fields.dst_mac, new_entry->key_fields.dst_mac, SYS_ADPT_MAC_ADDR_LEN) == 0))
                {
                   /* This maybe an ARP reply, so the original ARP entry type
                    * shall not be replace
                    */
                    new_entry->entry_type = VAL_ipNetToPhysicalExtType_static;
                    return TRUE;
                }
            }
            break;
        case VAL_ipNetToPhysicalExtType_dynamic:
            if (new_entry->entry_type == VAL_ipNetToPhysicalExtType_local)
                return TRUE;
            else if (new_entry->entry_type == VAL_ipNetToPhysicalExtType_static)
                return TRUE;
            else if (new_entry->entry_type == VAL_ipNetToPhysicalExtType_dynamic)
                return TRUE;
            else if (new_entry->entry_type == VAL_ipNetToPhysicalExtType_vrrp)
                return TRUE;
#if 0   /* dynamic host entry will not have tunnel type */
#if (SYS_CPNT_IP_TUNNEL == TRUE)
            else if(new_entry->key_fields.tunnel_entry_type == AMTRL3_TYPE_INETTOPHYSICALEXTTYPE_TUNNEL_ISATAP)
                return TRUE;
            else if(new_entry->key_fields.tunnel_entry_type == AMTRL3_TYPE_INETTOPHYSICALEXTTYPE_TUNNEL_6TO4)
                return TRUE;
            else if(new_entry->key_fields.tunnel_entry_type == AMTRL3_TYPE_INETTOPHYSICALEXTTYPE_TUNNEL_MANUAL)
                return TRUE;
#endif
#endif
            break;
#if 0
#if (SYS_CPNT_IP_TUNNEL == TRUE)
        case AMTRL3_TYPE_INETTOPHYSICALEXTTYPE_TUNNEL_ISATAP:
            if(amtrl3_mgr_debug_tunnel)
                BACKDOOR_MGR_Printf("get AMTRL3_TYPE_INETTOPHYSICALEXTTYPE_TUNNEL_ISATAP");
            return TRUE;
            break;
        case AMTRL3_TYPE_INETTOPHYSICALEXTTYPE_TUNNEL_6TO4:
            if(amtrl3_mgr_debug_tunnel)
                BACKDOOR_MGR_Printf("get AMTRL3_TYPE_INETTOPHYSICALEXTTYPE_TUNNEL_6TO4");
            return TRUE;
            break;
        case AMTRL3_TYPE_INETTOPHYSICALEXTTYPE_TUNNEL_MANUAL:
            if(amtrl3_mgr_debug_tunnel)
                BACKDOOR_MGR_Printf("get AMTRL3_TYPE_INETTOPHYSICALEXTTYPE_TUNNEL_MANUAL");
            return TRUE;
            break;
#endif /*SYS_CPNT_IP_TUNNEL*/
#endif
        default:
            /* new entry
             */
            return TRUE;
            break;
    } /* end of switch */
    return FALSE;
} /* end of AMTRL3_MGR_CheckHostTypeForReplacement() */

/* -------------------------------------------------------------------------
 *  FUNCTION NAME: AMTRL3_MGR_SetSuperNettingStatus
 * -------------------------------------------------------------------------
 * PURPOSE:  Disable / Enable ipv4 and ipv6 SuperNetting Status
 * INPUT:    action_flags - AMTRL3_TYPE_FLAGS_IPV4 \
 *                          AMTRL3_TYPE_FLAGS_IPV6
 *           fib_id       --  FIB id
 *           v4_super_netting_status - AMTRL3_TYPE_SUPER_NET_ENABLE \
 *                                     AMTRL3_TYPE_SUPER_NET_DISABLE
 *           v6_super_netting_status - AMTRL3_TYPE_SUPER_NET_ENABLE \
 *                                     AMTRL3_TYPE_SUPER_NET_DISABLE
 * OUTPUT:   none.
 * RETURN:   TRUE/FALSE
 * NOTES:    None
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_MGR_SetSuperNettingStatus(UI32_T action_flags,
                                        UI32_T fib_id,
                                        UI8_T v4_super_netting_status,
                                        UI8_T v6_super_netting_status)
{
    /* BODY
     */
    if (((v4_super_netting_status != AMTRL3_TYPE_SUPER_NET_ENABLE) && (v4_super_netting_status != AMTRL3_TYPE_SUPER_NET_DISABLE)) ||
        ((v6_super_netting_status != AMTRL3_TYPE_SUPER_NET_ENABLE) && (v6_super_netting_status != AMTRL3_TYPE_SUPER_NET_DISABLE)))
        return FALSE;
    if(AMTRL3_MGR_GetFIBByID(fib_id) == NULL)
        return FALSE;

    AMTRL3_MGR_CHECK_OPERATION_MODE(FALSE);

    if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
        AMTRL3_OM_SetDatabaseValue(fib_id, AMTRL3_OM_IPV4_SUPER_NETTING_STATUS, v4_super_netting_status);
    if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
        AMTRL3_OM_SetDatabaseValue(fib_id, AMTRL3_OM_IPV6_SUPER_NETTING_STATUS, v6_super_netting_status);

    return TRUE;
} /* end of AMTRL3_MGR_SetSuperNettingStatus() */

/* -------------------------------------------------------------------------
 *  FUNCTION NAME: AMTRL3_MGR_GetSuperNettingStatus
 * -------------------------------------------------------------------------
 * PURPOSE:  Get ipv4 and ipv6 SuperNetting Status
 * INPUT:    action_flags - AMTRL3_TYPE_FLAGS_IPV4 \
 *                          AMTRL3_TYPE_FLAGS_IPV6
 *           fib_id       --  FIB id
 * OUTPUT:   v4_super_netting_status - AMTRL3_TYPE_SUPER_NET_ENABLE \
 *                                    AMTRL3_TYPE_SUPER_NET_DISABLE
 *           v6_super_netting_status - AMTRL3_TYPE_SUPER_NET_ENABLE \
 *                                   AMTRL3_TYPE_SUPER_NET_DISABLE
 * RETURN:   TRUE/FALSE
 * NOTES:    None
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_MGR_GetSuperNettingStatus(UI32_T action_flags,
                                        UI32_T fib_id,
                                        UI8_T *v4_super_netting_status,
                                        UI8_T *v6_super_netting_status)
{
    /* BODY
     */
    if (v4_super_netting_status == NULL || v6_super_netting_status == NULL)
        return FALSE;
    if(AMTRL3_MGR_GetFIBByID(fib_id) == NULL)
        return FALSE;

    AMTRL3_MGR_CHECK_OPERATION_MODE(FALSE);

    if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
        *v4_super_netting_status = AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_IPV4_SUPER_NETTING_STATUS);
    if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
        *v6_super_netting_status = AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_IPV6_SUPER_NETTING_STATUS);

    return TRUE;

} /* end of AMTRL3_MGR_GetSuperNettingStatus() */

/* -------------------------------------------------------------------------
 *  FUNCTION NAME: AMTRL3_MGR_LocalUpdateVid2Fib
 * -------------------------------------------------------------------------
 * PURPOSE:  To update vid2fib table
 * INPUT:    new_fib_id   -- new FIB id
 *           vlan_ifindex -- vlan_ifindex to update
 * OUTPUT:
 * RETURN:   TRUE/FALSE
 * NOTES:    None
 * -------------------------------------------------------------------------*/
static BOOL_T AMTRL3_MGR_LocalUpdateVid2Fib(UI32_T new_fib_id, UI32_T vlan_ifindex)
{
    AMTRL3_OM_HostRouteEntry_T  host_route_entry;
    UI32_T                      action_flags = 0, address_type;
    UI32_T                      old_fib_id, tmp_vid;
    BOOL_T                      ret = FALSE;

    VLAN_OM_ConvertFromIfindex(vlan_ifindex, &tmp_vid);

    old_fib_id = AMTRL3_OM_GetFibIdByVid(tmp_vid);
    if (old_fib_id != new_fib_id)
    {
        if (amtrl3_mgr_debug_flag & DEBUG_LEVEL_TRACE_NET)
        {
            BACKDOOR_MGR_Printf("Move ifindex/old fib/new fib -%d/%d/%d\n",
                vlan_ifindex, old_fib_id, new_fib_id);
        }

        /* if old fib != new fib, delete old host route
         */
        memset(&host_route_entry, 0, sizeof(AMTRL3_OM_HostRouteEntry_T));
        host_route_entry.key_fields.dst_vid_ifindex = vlan_ifindex;

        while(AMTRL3_OM_GetNextHostRouteEntryByVlanMac(AMTRL3_TYPE_FLAGS_IPV4 | AMTRL3_TYPE_FLAGS_IPV6, old_fib_id, &host_route_entry) == TRUE)
        {
            if(host_route_entry.key_fields.dst_vid_ifindex != vlan_ifindex)
                break;

            address_type = host_route_entry.key_fields.dst_inet_addr.type;
            if((address_type == L_INET_ADDR_TYPE_IPV4) ||
               (address_type == L_INET_ADDR_TYPE_IPV4Z))
            {
                action_flags = AMTRL3_TYPE_FLAGS_IPV4;
            }
            else if((address_type == L_INET_ADDR_TYPE_IPV6) ||
                    (address_type == L_INET_ADDR_TYPE_IPV6Z))
            {
                action_flags = AMTRL3_TYPE_FLAGS_IPV6;
            }

            AMTRL3_MGR_HostRouteEventHandler(action_flags, old_fib_id, HOST_ROUTE_REMOVE_EVENT, &host_route_entry);
        }

        AMTRL3_OM_SetVid2Fib(tmp_vid, new_fib_id);
        ret = TRUE;
    }

    return ret;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME : AMTRL3_MGR_SetInetCidrRouteEntry
 * -------------------------------------------------------------------------
 * PURPOSE: Set one record to ipv4 or ipv6 net route table
 *          and set configuration to driver
 *
 * INPUT:
 *      action_flags    -- AMTRL3_TYPE_FLAGS_IPV4 \
 *                         AMTRL3_TYPE_FLAGS_IPV6
 *      fib_id       -- FIB id
 *      net_route_entry -- record of forwarding table.
 *                         key :
 *                           inet_cidr_route_dest          -- destination inet address
 *                           inet_cidr_route_pfxlen        -- prefix length
 *                           inet_cidr_route_policy        -- serve as an additional index which
 *                                                            may delineate between multiple entries to
 *                                                            the same destination.
 *                                                            Default is {0,0}
 *                           inet_cidr_route_next_hop      -- next hop inet address.
 *
 * OUTPUT: none.
 *
 * RETURN:
 *      TRUE : AMTRL3_MGR_OK              -- Successfully insert to database
 *      FALSE: AMTRL3_MGR_ALREADY_EXIST   -- this entry existed in chip/database.
 *             AMTRL3_MGR_TABLE_FULL      -- over reserved space
 *
 * NOTES: 1. If the entry with same key value already exist, this function will
 *        update this entry except those key fields value.
 *        2. This function is only for non-ECMP routes, so AMTRL3_TYPE_FLAGS_ECMP
 *         / WCMP flag should not set.
 *        3. action flag: AMTRL3_TYPE_FLAGS_IPV4/IPV6 must consistent with
 *        address type in net_route_entry structure.
 *        4. for dest and nexthop address, unused bytes must be zero.
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_MGR_SetInetCidrRouteEntry(UI32_T action_flags,
                                        UI32_T fib_id,
                                        AMTRL3_TYPE_InetCidrRouteEntry_T  *net_route_entry)
{
    AMTRL3_OM_NetRouteEntry_T   local_net_entry;
    AMTRL3_OM_HostRouteEntry_T  local_host_entry;
    UI32_T                      begin_timetick = 0, end_timetick = 0;
    UI32_T                      host_begin = 0, host_end = 0;
    BOOL_T                      ret = FALSE, is_next_hop_zero;
    UI8_T                       ip_str[L_INET_MAX_IPADDR_STR_LEN] = {0};


    if(AMTRL3_MGR_GetFIBByID(fib_id) == NULL)
        return FALSE;
    if (net_route_entry->partial_entry.inet_cidr_route_metric1 < 0)
        return FALSE;
    /* only for non-ECMP set */
    if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_ECMP) ||
       CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_WCMP))
        return FALSE;

    if (amtrl3_mgr_netroute_performance_flag)
        begin_timetick = SYSFUN_GetSysTick();

    memset(&local_net_entry, 0, sizeof(AMTRL3_OM_NetRouteEntry_T));
    memset(&local_host_entry, 0, sizeof(AMTRL3_OM_HostRouteEntry_T));

    AMTRL3_MGR_CHECK_OPERATION_MODE(FALSE);
    /* TODO, move vid to FIB ID (only update vid2fib when process directly connted route)
     * 1. if old fib != new fib, delete old host route
     *    (net route should be deleted by DeleteInetCidrRouteEntry)
     * 2. set host route on fib
     */
    is_next_hop_zero = AMTRL3_OM_IsAddressEqualZero(&(net_route_entry->partial_entry.inet_cidr_route_next_hop));
    if (TRUE == is_next_hop_zero)
        AMTRL3_MGR_LocalUpdateVid2Fib(fib_id, net_route_entry->partial_entry.inet_cidr_route_if_index);

    net_route_entry->partial_entry.inet_cidr_route_dest.preflen = net_route_entry->partial_entry.inet_cidr_route_pfxlen;
    L_INET_ApplyMask(&net_route_entry->partial_entry.inet_cidr_route_dest);

    if (amtrl3_mgr_debug_flag & DEBUG_LEVEL_TRACE_NET)
    {
        AMTRL3_MGR_Ntoa(&(net_route_entry->partial_entry.inet_cidr_route_dest), ip_str);
        BACKDOOR_MGR_Printf("AL3_AddInetCidrRouteEntry IP: %s, ", ip_str);

        BACKDOOR_MGR_Printf("Prefix-length: %d, ", (int)net_route_entry->partial_entry.inet_cidr_route_pfxlen);

        AMTRL3_MGR_Ntoa(&(net_route_entry->partial_entry.inet_cidr_route_next_hop), ip_str);
        BACKDOOR_MGR_Printf("nhop: %s, if_index %d\n", ip_str, (int)net_route_entry->partial_entry.inet_cidr_route_if_index);
        BACKDOOR_MGR_Printf("nexthop zoneid: %d\n", net_route_entry->partial_entry.inet_cidr_route_next_hop.zoneid);
    } /* end of if */

    memcpy(&local_net_entry.inet_cidr_route_entry,
                &net_route_entry->partial_entry,
                sizeof(AMTRL3_TYPE_InetCidrRoutePartialEntry_T));
    local_net_entry.hw_info = L_CVRT_UINT_TO_PTR(AMTRL3_OM_HW_INFO_INVALID);

    /* Check if Net entry already exist */
    if(AMTRL3_OM_GetNetRouteEntry(action_flags, fib_id, &local_net_entry))
    {
        /* 1. If the new net route has ECMP attribute, but the old one is not ECMP route, return FALSE.
         * 2. If the new net route is not ECMP route, but the old one has ECMP attribute, return FALSE.
         *
         * Upper layer(NSM) should delete old net route at first then re add it with new attribute
         */
        if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_ECMP) != CHECK_FLAG(local_net_entry.flags, AMTRL3_TYPE_FLAGS_ECMP))
        {
            return FALSE;
        }

#if (SYS_CPNT_NSM == FALSE)
        if (local_net_entry.inet_cidr_route_entry.inet_cidr_route_if_index != net_route_entry->partial_entry.inet_cidr_route_if_index)
            SYS_CALLBACK_MGR_IPCFG_NsmRouteChange(SYS_MODULE_AMTRL3, AFI_IP);
#endif

        /* If the entry already exist, update it's content except
         * four key fields value: dest, prefix_len, nexthop, policy.
         * Do not need write chip again, because those information are not needed
         * by chip or will rewrite to chip in other procedure (like if_index changed)
         */
        local_net_entry.inet_cidr_route_entry = net_route_entry->partial_entry;
        //local_net_entry.flags = ((action_flags & AMTRL3_TYPE_FLAGS_ECMP) | (action_flags & AMTRL3_TYPE_FLAGS_WCMP));
        ret = AMTRL3_OM_SetNetRouteEntry(action_flags, fib_id, &local_net_entry);
        return ret;
    } /* end of if */

    /* If designated gateway entry alreaedy exist in OM, increment its reference count,
       Otherwise, create a gateway entry whose Ip address is next hop of this net
       route entry */
    if(is_next_hop_zero == FALSE)
    {
        if(amtrl3_mgr_netroute_performance_flag)
            host_begin = SYSFUN_GetSysTick();

        local_host_entry.key_fields.dst_inet_addr = net_route_entry->partial_entry.inet_cidr_route_next_hop;
        local_host_entry.key_fields.dst_vid_ifindex = net_route_entry->partial_entry.inet_cidr_route_if_index;

        /* If the nexthop host entry already exist in OM, get it out.
         * If not exist, threat it as dynamic and set it to OM. */
        if(AMTRL3_OM_GetHostRouteEntry(action_flags, fib_id, &local_host_entry) == FALSE)
        {
            local_host_entry.entry_type = VAL_ipNetToPhysicalExtType_dynamic;
            local_host_entry.hw_info = L_CVRT_UINT_TO_PTR(AMTRL3_OM_HW_INFO_INVALID);
        }
        local_host_entry.ref_count ++;

        if(!AMTRL3_MGR_HostRouteEventHandler(action_flags, fib_id, HOST_ROUTE_PARTIAL_EVENT, &local_host_entry))
        {
            return ret;
        }

        if(amtrl3_mgr_netroute_performance_flag)
        {
            host_end = SYSFUN_GetSysTick();
            BACKDOOR_MGR_Printf("Host route takes %d ticks\n", (int)(host_end - host_begin));
        }
    }
    else
    {
        /* Connected Route */
        if (net_route_entry->partial_entry.inet_cidr_route_proto == VAL_ipCidrRouteProto_local)
        {
#if (SYS_CPNT_IP_TUNNEL == TRUE)
            {
                NETCFG_TYPE_L3_Interface_T l3_if;
                /* check if this local route belongs to static tunnel,
                 * if yes, we don't set local route to chip, because it needs to be tunneled by initiator in chip
                 */
                memset(&l3_if, 0, sizeof(l3_if));
                l3_if.ifindex = net_route_entry->partial_entry.inet_cidr_route_if_index;
                if(NETCFG_TYPE_FAIL == NETCFG_POM_IP_GetL3Interface(&l3_if))
                {
                    if (amtrl3_mgr_debug_flag & DEBUG_LEVEL_ERROR)
                    {
                        BACKDOOR_MGR_Printf("AL3: Get tunnel interface %lu failed\n", (unsigned long)l3_if.ifindex);
                    }
                    return FALSE;
                }


                if(NETCFG_TYPE_TUNNEL_MODE_CONFIGURED == l3_if.u.tunnel_intf.tunnel_mode)
                {
                    local_net_entry.net_route_status = AMTRL3_OM_NET_ROUTE_UNRESOLVED;
                    goto store_connected_route;
                }
            }
#endif
            if (AMTRL3_MGR_AddNetRouteToChip(action_flags, fib_id, &local_net_entry, NULL) == FALSE)
            {
                if (amtrl3_mgr_debug_flag & DEBUG_LEVEL_ERROR)
                {
                    AMTRL3_MGR_Ntoa(&(net_route_entry->partial_entry.inet_cidr_route_dest), ip_str);
                    BACKDOOR_MGR_Printf("AL3: Add Connected Route failed: %s\n", ip_str);
                }
                return FALSE;
            }
            goto store_connected_route;
        }
#if (SYS_CPNT_IP_TUNNEL == TRUE)
        else if(IS_TUNNEL_IFINDEX(net_route_entry->partial_entry.inet_cidr_route_if_index) && AMTRL3_OM_IsAddressEqualZero(&net_route_entry->partial_entry.inet_cidr_route_next_hop))
        {
            if(TRUE == AMTRL3_MGR_HandleTunnelNetRouteToChip(action_flags,fib_id,&local_net_entry,net_route_entry))
            {
                if(amtrl3_mgr_debug_tunnel)
                    BACKDOOR_MGR_Printf("%s[%d],store tunnel net route\r\n",__FUNCTION__,__LINE__);
                        goto store_connected_route;
            }
            else
                return FALSE;
        }
#endif /*SYS_CPNT_IP_TUNNEL*/
        else
        {
            /* added by steven.gao
             *
             * OSPFv3 may add the connected route, so we treat it as local.
             */
            local_net_entry.inet_cidr_route_entry.inet_cidr_route_proto = VAL_ipCidrRouteProto_local;
            if (AMTRL3_MGR_AddNetRouteToChip(action_flags, fib_id, &local_net_entry, NULL) == FALSE)
            {
                if (amtrl3_mgr_debug_flag & DEBUG_LEVEL_ERROR)
                {
                    AMTRL3_MGR_Ntoa(&(net_route_entry->partial_entry.inet_cidr_route_dest), ip_str);
                    BACKDOOR_MGR_Printf("AL3: Add Connected Route failed: %s\n", ip_str);
                }
                return FALSE;
            }
            goto store_connected_route;
        }
    }

    if (local_host_entry.in_chip_status)
    {
        if (!AMTRL3_MGR_AddNetRouteToChip(action_flags, fib_id, &local_net_entry, &local_host_entry))
        {
            local_net_entry.net_route_status = AMTRL3_OM_NET_ROUTE_UNRESOLVED;
            if (amtrl3_mgr_debug_flag & DEBUG_LEVEL_TRACE_NET)
                BACKDOOR_MGR_Printf("AL3_AddInetCidrRouteEntry :AMTRL3_OM_NET_ROUTE_UNRESOLVED fail to add chip case 1\n");
        }
    }
    else
    {
        local_net_entry.net_route_status = AMTRL3_OM_NET_ROUTE_UNRESOLVED;
        if (amtrl3_mgr_debug_flag & DEBUG_LEVEL_TRACE_NET)
            BACKDOOR_MGR_Printf("AL3_AddInetCidrRouteEntry :AMTRL3_OM_NET_ROUTE_UNRESOLVED not in chip case\n");
    }

store_connected_route:
    ret = AMTRL3_OM_SetNetRouteEntry(action_flags, fib_id, &local_net_entry);
    if((ret == FALSE) &&
       (local_net_entry.net_route_status == AMTRL3_OM_NET_ROUTE_READY))
    {
        AMTRL3_MGR_DeleteNetRouteFromChip(action_flags, fib_id, &local_net_entry, &local_host_entry);
        return FALSE;
    }

    if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
        AMTRL3_OM_IncreaseDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV4_NET_ROUTE_IN_OM, 1);
    else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
        AMTRL3_OM_IncreaseDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV6_NET_ROUTE_IN_OM, 1);

    if (amtrl3_mgr_netroute_performance_flag)
    {
        end_timetick = SYSFUN_GetSysTick();
        BACKDOOR_MGR_Printf("AMTRL3_MGR_AddInetCidrRouteEntry takes %d ticks\n", (int)(end_timetick - begin_timetick));
    }

#if (SYS_CPNT_NSM == FALSE)
    {
        UI32_T af = CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4) ? AFI_IP : AFI_IP6;
        SYS_CALLBACK_MGR_IPCFG_NsmRouteChange(SYS_MODULE_AMTRL3, af);
    }
#endif

    return ret;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME : AMTRL3_MGR_DeleteInetCidrRouteEntry
 * -------------------------------------------------------------------------
 * PURPOSE: Delete specified ipv4 or ipv6 record with the key from database and chip.
 *
 * INPUT:
 *      action_flags    -- AMTRL3_TYPE_FLAGS_IPV4 \
 *                         AMTRL3_TYPE_FLAGS_IPV6
 *      fib_id       -- FIB id
 *      net_route_entry -- record of forwarding table, only key fields are useful
 *                         in this function.
 *                         KEY :
 *                           inet_cidr_route_dest          -- destination inet address
 *                           inet_cidr_route_pfxlen        -- prefix length
 *                           inet_cidr_route_policy        -- serve as an additional index which
 *                                                            may delineate between multiple entries to
 *                                                            the same destination.
 *                                                            Default is {0,0}
 *                           inet_cidr_route_next_hop      -- next hop inet address.

 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE    - AMTRL3_MGR_OK                    -- Successfully remove from database
 *      FALSE   - AMTRL3_MGR_CAN_NOT_FIND          -- This entry does not exist in chip/database.
 *                AMTRL3_MGR_CAN_NOT_DELETE        -- Cannot Delete
 *
 * NOTES:
 *      1. Only for non-ECMP routes.
 *      2. action flag: AMTRL3_TYPE_FLAGS_IPV4/IPV6 must consistent with
 *         address type in net_route_entry structure.
 *      3. Only key fields in net_route_entry are used in this function.
 *         Other fields can have random value.
 * -------------------------------------------------------------------------*/
BOOL_T  AMTRL3_MGR_DeleteInetCidrRouteEntry(UI32_T action_flags,
                                            UI32_T fib_id,
                                            AMTRL3_TYPE_InetCidrRouteEntry_T  *net_route_entry)
{
    UI32_T                              event_type;
    AMTRL3_OM_NetRouteEntry_T           local_net_entry;
    AMTRL3_OM_HostRouteEntry_T          local_host_entry, *nhop_entry = NULL;
    AMTRL3_OM_ResolvedNetRouteEntry_T   resolved_net_entry;
    UI8_T                               ip_str[L_INET_MAX_IPADDR_STR_LEN] = {0};
    BOOL_T                              ret = FALSE;

    /* only for non-ECMP routes */
    if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_ECMP) ||
       CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_WCMP))
        return FALSE;
    if(AMTRL3_MGR_GetFIBByID(fib_id) == NULL)
        return FALSE;

    net_route_entry->partial_entry.inet_cidr_route_dest.preflen = net_route_entry->partial_entry.inet_cidr_route_pfxlen;
    L_INET_ApplyMask(&net_route_entry->partial_entry.inet_cidr_route_dest);

    memset(&local_net_entry, 0, sizeof(AMTRL3_OM_NetRouteEntry_T));
    memset(&local_host_entry, 0, sizeof(AMTRL3_OM_HostRouteEntry_T));
    memset(&resolved_net_entry, 0, sizeof(AMTRL3_OM_ResolvedNetRouteEntry_T));

    local_net_entry.inet_cidr_route_entry = net_route_entry->partial_entry;

    if(amtrl3_mgr_debug_flag & DEBUG_LEVEL_TRACE_NET)
    {
        AMTRL3_MGR_Ntoa(&(net_route_entry->partial_entry.inet_cidr_route_dest), ip_str);
        BACKDOOR_MGR_Printf("AL3_DeleteInetCidrRouteEntry IP: %s, ", ip_str);
        BACKDOOR_MGR_Printf("Prefix-length: %d, ", (int)net_route_entry->partial_entry.inet_cidr_route_pfxlen);
        AMTRL3_MGR_Ntoa(&(net_route_entry->partial_entry.inet_cidr_route_next_hop), ip_str);
        BACKDOOR_MGR_Printf("nhop: %s\n", ip_str);
    } /* end of if */

    AMTRL3_MGR_CHECK_OPERATION_MODE(FALSE);

    /* if net route entry does not already exist in OM, do
       not continue to process it
     */
    if (TRUE != AMTRL3_OM_GetNetRouteEntry(action_flags, fib_id, &local_net_entry))
    {
        /* for frr/fpm case, delete net route without nexthop
         */
        if (  (TRUE == AMTRL3_OM_IsAddressEqualZero(&local_net_entry.inet_cidr_route_entry.inet_cidr_route_next_hop))
            &&(TRUE != AMTRL3_OM_GetNextNetRouteEntry(action_flags, fib_id, &local_net_entry))
           )
            return FALSE;
    }

#if (SYS_CPNT_IP_TUNNEL == TRUE)
    {
        AMTRL3_OM_HostRouteEntry_T       tunnel_host_entry;
        AMTRL3_OM_NetRouteEntry_T        tunnel_net_entry;
        AMTRL3_TYPE_InetCidrRouteEntry_T delete_net_entry;
        L_INET_AddrIp_T                  masked_host_entry;

        if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
        {
            /* we must search if dynamic 6to4 host route's next hop is covered with the deleted net route,
             * if yes, we will delete corresponding dynamic 6to4 net route and host route
             */
            memset(&tunnel_host_entry, 0, sizeof(AMTRL3_OM_HostRouteEntry_T));

            tunnel_host_entry.key_fields.dst_vid_ifindex = SYS_ADPT_TUNNEL_1_IF_INDEX_NUMBER;
            while(AMTRL3_OM_GetNextHostRouteEntryByVlanMac(AMTRL3_TYPE_FLAGS_IPV6, fib_id, &tunnel_host_entry))
            {
                memset(&masked_host_entry, 0, sizeof(L_INET_AddrIp_T));
                masked_host_entry = tunnel_host_entry.key_fields.u.ip_tunnel.dest_inet_addr;
                masked_host_entry.preflen = net_route_entry->partial_entry.inet_cidr_route_pfxlen;
                L_INET_ApplyMask(&masked_host_entry);

                /* check if tunnel's destination is the same as delete net route */
                if(!memcmp(masked_host_entry.addr, net_route_entry->partial_entry.inet_cidr_route_dest.addr, SYS_ADPT_IPV4_ADDR_LEN))
                {

                    /* delete dynamic 6to4 host route,
                     * delete dynamic 6to4 net route will done by AMTRL3_MGR_RemoveTunnelNetRouteFromChipByGateway()
                     */
                    if(amtrl3_mgr_debug_tunnel)
                        BACKDOOR_MGR_Printf("%s[%d],remove tunnel host route\r\n",__FUNCTION__,__LINE__);
                    if(FALSE == AMTRL3_MGR_HostRouteEventHandler(AMTRL3_TYPE_FLAGS_IPV6, fib_id, HOST_ROUTE_REMOVE_EVENT, &tunnel_host_entry))
                    {
                        if(amtrl3_mgr_debug_tunnel)
                            BACKDOOR_MGR_Printf("%s[%d],failed to delete dynamic tunnel host route\r\n",__FUNCTION__,__LINE__);
                        return FALSE;
                    }
                }
            }
        }
    }
#endif

    if(AMTRL3_OM_IsAddressEqualZero(&(net_route_entry->partial_entry.inet_cidr_route_next_hop)) == TRUE)
    {
        /* Connected Route */
        if (net_route_entry->partial_entry.inet_cidr_route_proto == VAL_ipCidrRouteProto_local)
        {
#if (SYS_CPNT_IP_TUNNEL == TRUE)
            if (IS_TUNNEL_IFINDEX(net_route_entry->partial_entry.inet_cidr_route_if_index))
            {
                memset(&local_host_entry, 0, sizeof(local_host_entry));
                local_host_entry.key_fields.dst_vid_ifindex = net_route_entry->partial_entry.inet_cidr_route_if_index;
                //simon: Note! this time, om may already delete host route entry!

                if (AMTRL3_MGR_DeleteNetRouteFromChip(action_flags, fib_id, &local_net_entry, &local_host_entry) == FALSE)
                {
                    if(amtrl3_mgr_debug_tunnel)
                        BACKDOOR_MGR_Printf("Fail to remove netroute for tunnel %ld", (long)local_host_entry.key_fields.dst_vid_ifindex);
                    return FALSE;
                }
            }
            else
#endif /*SYS_CPNT_IP_TUNNEL*/
            {
                if (AMTRL3_MGR_DeleteNetRouteFromChip(action_flags, fib_id, &local_net_entry, NULL) == FALSE)
                {
                    if (amtrl3_mgr_debug_flag & DEBUG_LEVEL_ERROR)
                    {
                        AMTRL3_MGR_Ntoa(&(net_route_entry->partial_entry.inet_cidr_route_dest), ip_str);
                        BACKDOOR_MGR_Printf("AL3: Delete Connected Route failed: %s\n", ip_str);
                    }
                    return FALSE;
                }
            }
            goto remove_connected_route;
        }
        else
        {

#if (SYS_CPNT_IP_TUNNEL == TRUE)
            {
                if((local_net_entry.tunnel_entry_type == AMTRL3_TYPE_INETTOPHYSICALEXTTYPE_TUNNEL_6TO4))
                {
                    memset(&local_host_entry, 0, sizeof(AMTRL3_OM_HostRouteEntry_T));
                    local_host_entry.key_fields.dst_vid_ifindex = local_net_entry.inet_cidr_route_entry.inet_cidr_route_if_index;
                    local_host_entry.key_fields.dst_inet_addr = local_net_entry.tunnel_nexthop_inet_addr;
                    /* Decrease next hop host route's reference count by 1*/
                    if(AMTRL3_OM_GetHostRouteEntry(AMTRL3_TYPE_FLAGS_IPV4, fib_id, &local_host_entry))
                    {
                        local_host_entry.ref_count--;
                        event_type = HOST_ROUTE_PARTIAL_EVENT;

                        /* Appropriate host route status should be determined in MGR thru the
                           Finite State Machine before passing to OM for data storage.
                         */
                        if(amtrl3_mgr_debug_tunnel)
                        {
                            BACKDOOR_MGR_Printf("%s[%d],destination addr:%lx:%lx:%lx:%lx(%lu)\r\n",__FUNCTION__,__LINE__,
                                L_INET_EXPAND_IPV6(local_host_entry.key_fields.dst_inet_addr.addr),
                                (unsigned long)local_host_entry.key_fields.dst_inet_addr.addrlen);
                        }

                        if (!AMTRL3_MGR_HostRouteEventHandler(AMTRL3_TYPE_FLAGS_IPV4, fib_id, event_type, &local_host_entry))
                        {
                            BACKDOOR_MGR_Printf("failed to decrease next hop reference count\r\n");//Error set host route entry
                        }

                        nhop_entry = &local_host_entry;
                    }

                    /* Decrease 6to4 tunnel net route's database value by 1 */
                    AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_IPV6_6to4_TUNNEL_NET_ROUTE_COUNT, 1);
                    if(amtrl3_mgr_debug_tunnel)
                    {
                        BACKDOOR_MGR_Printf("%s[%d],delete net route %lx:%lx:%lx:%lx/%lu from chip\r\n",__FUNCTION__,__LINE__,
                            L_INET_EXPAND_IPV6(local_net_entry.inet_cidr_route_entry.inet_cidr_route_dest.addr),
                            (unsigned long)local_net_entry.inet_cidr_route_entry.inet_cidr_route_dest.preflen);
                    }

                    local_net_entry.inet_cidr_route_entry.inet_cidr_route_proto = VAL_ipCidrRouteProto_netmgmt;
                    if (AMTRL3_MGR_DeleteNetRouteFromChip(action_flags, fib_id, &local_net_entry, nhop_entry) == FALSE)
                    {
                        if (amtrl3_mgr_debug_flag & DEBUG_LEVEL_ERROR)
                        {
                            AMTRL3_MGR_Ntoa(&(net_route_entry->partial_entry.inet_cidr_route_dest), ip_str);
                            BACKDOOR_MGR_Printf("AL3: Delete Connected Route failed: %s\n", ip_str);
                        }
                        return FALSE;
                    }
                    goto remove_connected_route;
                }
            }
#endif

            /* added by Steven.Gao for OSPFv3 connected route */
            local_net_entry.inet_cidr_route_entry.inet_cidr_route_proto = VAL_ipCidrRouteProto_local;
            if (AMTRL3_MGR_DeleteNetRouteFromChip(action_flags, fib_id, &local_net_entry, NULL) == FALSE)
            {
                if (amtrl3_mgr_debug_flag & DEBUG_LEVEL_ERROR)
                {
                    AMTRL3_MGR_Ntoa(&(net_route_entry->partial_entry.inet_cidr_route_dest), ip_str);
                    BACKDOOR_MGR_Printf("AL3: Delete Connected Route failed: %s\n", ip_str);
                }
                return FALSE;
            }
            goto remove_connected_route;
        }
    }

    /* Update reference count of the designated gateway entry
     */
    local_host_entry.key_fields.dst_vid_ifindex = net_route_entry->partial_entry.inet_cidr_route_if_index;
    local_host_entry.key_fields.dst_inet_addr = net_route_entry->partial_entry.inet_cidr_route_next_hop;
    if(AMTRL3_OM_GetHostRouteEntry(action_flags, fib_id, &local_host_entry))
    {
        local_host_entry.ref_count--;
        event_type = HOST_ROUTE_PARTIAL_EVENT;

        /* Appropriate host route status should be determined in MGR thru the
           Finite State Machine before passing to OM for data storage.
         */
        if (!AMTRL3_MGR_HostRouteEventHandler(action_flags, fib_id, event_type, &local_host_entry))
        {
            //Error set host route entry
        }
        nhop_entry = &local_host_entry;
    } /* end of if */

    if((AMTRL3_OM_IsAddressEqualZero(&(net_route_entry->partial_entry.inet_cidr_route_dest)) == TRUE) &&
        (net_route_entry->partial_entry.inet_cidr_route_pfxlen == 0))
    {
        ret = AMTRL3_MGR_DeleteDefaultRoute(action_flags, fib_id, &local_net_entry, nhop_entry);
        return ret;
    }

    if(local_net_entry.net_route_status == AMTRL3_OM_NET_ROUTE_READY)
    {
        if(AMTRL3_MGR_DeleteNetRouteFromChip(action_flags, fib_id, &local_net_entry, nhop_entry) == FALSE)
        {
            return FALSE;
        }

        if (nhop_entry->key_fields.dst_inet_addr.type == L_INET_ADDR_TYPE_IPV6Z &&
            nhop_entry->ref_count_in_chip == 0)
        {
            AMTRL3_MGR_HostRouteEventHandler(action_flags, fib_id, HOST_ROUTE_UNREFERENCE_EVENT, nhop_entry);
        }
    }
    else if (local_net_entry.net_route_status == AMTRL3_OM_NET_ROUTE_RESOLVED)
    {
        if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
            resolved_net_entry.inverse_prefix_length = (AMTRL3_MGR_IPV4_MAXIMUM_PREFIX_LENGTH - local_net_entry.inet_cidr_route_entry.inet_cidr_route_pfxlen);
        else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
            resolved_net_entry.inverse_prefix_length = (AMTRL3_MGR_IPV6_MAXIMUM_PREFIX_LENGTH - local_net_entry.inet_cidr_route_entry.inet_cidr_route_pfxlen);
        resolved_net_entry.inet_cidr_route_dest = net_route_entry->partial_entry.inet_cidr_route_dest;
        resolved_net_entry.inet_cidr_route_next_hop = net_route_entry->partial_entry.inet_cidr_route_next_hop;

        if(AMTRL3_OM_DeleteResolvedNetEntry(action_flags, fib_id, &resolved_net_entry))
        {
            if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
                AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_IPV4_RESOLVED_NET_ROUTE_COUNT, 1);
            else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
                AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_IPV6_RESOLVED_NET_ROUTE_COUNT, 1);
        }
        else
        {
            if(amtrl3_mgr_debug_flag & DEBUG_LEVEL_ERROR)
                BACKDOOR_MGR_Printf("AMTRL3_OM_DeleteResolvedNetEntry Fails\n");
            return FALSE;
        }
    }

remove_connected_route:
    ret = AMTRL3_OM_DeleteNetRouteEntry(action_flags, fib_id, &local_net_entry);
    if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
        AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV4_NET_ROUTE_IN_OM, 1);
    else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
        AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV6_NET_ROUTE_IN_OM, 1);

    AMTRL3_MGR_ClearNetRouteHWInfo(action_flags, fib_id, &local_net_entry);

#if (SYS_CPNT_NSM == FALSE)
    {
        UI32_T af = CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4) ? AFI_IP : AFI_IP6;
        SYS_CALLBACK_MGR_IPCFG_NsmRouteChange(SYS_MODULE_AMTRL3, af);
    }
#endif

    return ret;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_ClearAllNetRoute
 * -------------------------------------------------------------------------
 * PURPOSE:  Delete all net route entries in AMTRL3 net Route table
 * INPUT:
 *      action_flags    -- AMTRL3_TYPE_FLAGS_IPV4 \
 *                         AMTRL3_TYPE_FLAGS_IPV6
 *      fib_id       -- FIB id
 * OUTPUT:   none.
 * RETURN:   TRUE
 *           FALSE
 * NOTES:
 *      1. If only AMTRL3_TYPE_FLAGS_IPV4 is set, only delete all ipv4 entries.
 *      2. If only AMTRL3_TYPE_FLAGS_IPV6 is set, only delete all ipv6 entries.
 *      3. If both IPV4 and IPV6 flag are set, delete ipv4 and ipv6 entries.
 * -------------------------------------------------------------------------*/
BOOL_T  AMTRL3_MGR_ClearAllNetRoute(UI32_T action_flags, UI32_T fib_id)
{
    return TRUE;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_DeleteDefaultRoute
 * -------------------------------------------------------------------------
 * PURPOSE:  This function provides seperate remove operation for default route
 *           entry. (om and chip)
 * INPUT:    action_flags - AMTRL3_TYPE_FLAGS_IPV4 \
 *                          AMTRL3_TYPE_FLAGS_IPV6
 *           fib_id - FIB id
 *           net_entry - contains net route information to set to OM and chip
 *              ip_cidr_route_dest      -- subnet ip address.
 *              ip_cidr_route_mask      -- subnet mask
 *              ip_cidr_route_tos       -- default is 0.
 *              ip_cidr_route_next_hop  -- next hop ip address.
 *
 * OUTPUT:   none.
 * RETURN:   TRUE  - Default Route set successfully
 *           FALSE - Fail to set default Route
 * NOTES:    None
 * -------------------------------------------------------------------------*/
static BOOL_T AMTRL3_MGR_DeleteDefaultRoute(UI32_T action_flags,
                                            UI32_T fib_id,
                                            AMTRL3_OM_NetRouteEntry_T *net_route_entry,
                                            AMTRL3_OM_HostRouteEntry_T *nhop_entry)
{
    BOOL_T is_last_in_om = FALSE, ret = FALSE;

    /* BODY
     */
    if(AMTRL3_MGR_GetFIBByID(fib_id) == NULL)
        return FALSE;

    if(AMTRL3_MGR_IsLastPathOfECMPRouteInOm(action_flags, fib_id, net_route_entry))
        is_last_in_om = TRUE;

    switch(net_route_entry->net_route_status)
    {
        case AMTRL3_OM_NET_ROUTE_UNRESOLVED:
        case AMTRL3_OM_NET_ROUTE_RESOLVED: /* for default route resolved route only in om */
            if(AMTRL3_OM_DeleteNetRouteEntry(action_flags, fib_id, net_route_entry) == FALSE)
                return FALSE;
            if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
            {
                if(CHECK_FLAG(net_route_entry->flags, AMTRL3_TYPE_FLAGS_ECMP))
                {
                    AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_IPV4_DEFAULT_ROUTE_MULTIPATH_COUNTER, 1);
                    AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV4_ECMP_NH_IN_OM, 1);
                    if(is_last_in_om)
                    {
                        AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV4_ECMP_ROUTE_IN_OM, 1);
                        AMTRL3_MGR_ClearNetRouteHWInfo(action_flags | AMTRL3_TYPE_FLAGS_ECMP, fib_id, net_route_entry);
                    }
                }
                else
                    AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV4_NET_ROUTE_IN_OM, 1);
            }
            else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
            {
                if(CHECK_FLAG(net_route_entry->flags, AMTRL3_TYPE_FLAGS_ECMP))
                {
                    AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_IPV6_DEFAULT_ROUTE_MULTIPATH_COUNTER, 1);
                    AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV6_ECMP_NH_IN_OM, 1);
                    if(is_last_in_om)
                    {
                        AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV6_ECMP_ROUTE_IN_OM, 1);
                        AMTRL3_MGR_ClearNetRouteHWInfo(action_flags | AMTRL3_TYPE_FLAGS_ECMP, fib_id, net_route_entry);
                    }
                }
                else
                    AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV6_NET_ROUTE_IN_OM, 1);
            }
            break;
        case AMTRL3_OM_NET_ROUTE_READY:
            if(CHECK_FLAG(net_route_entry->flags, AMTRL3_TYPE_FLAGS_ECMP))
                ret = AMTRL3_MGR_DeleteECMPRouteOnePathFromChip(action_flags, fib_id, net_route_entry, nhop_entry, is_last_in_om);
            else
                ret = AMTRL3_MGR_DeleteNetRouteFromChip(action_flags, fib_id, net_route_entry, nhop_entry);

            if(ret == FALSE)
                return FALSE;

            if (nhop_entry->key_fields.dst_inet_addr.type == L_INET_ADDR_TYPE_IPV6Z &&
                nhop_entry->ref_count_in_chip == 0)
            {
                AMTRL3_MGR_HostRouteEventHandler(action_flags, fib_id, HOST_ROUTE_UNREFERENCE_EVENT, nhop_entry);
            }

            if(AMTRL3_OM_DeleteNetRouteEntry(action_flags, fib_id, net_route_entry) == FALSE)
                return FALSE;

            if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
            {
                if(CHECK_FLAG(net_route_entry->flags, AMTRL3_TYPE_FLAGS_ECMP))
                {
                    AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_IPV4_DEFAULT_ROUTE_MULTIPATH_COUNTER, 1);
                    AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV4_ECMP_NH_IN_OM, 1);
                    if(is_last_in_om)
                    {
                        AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV4_ECMP_ROUTE_IN_OM, 1);
                        AMTRL3_MGR_ClearNetRouteHWInfo(action_flags | AMTRL3_TYPE_FLAGS_ECMP, fib_id, net_route_entry);
                    }
                }
                else
                    AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV4_NET_ROUTE_IN_OM, 1);
            }
            else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
            {
                if(CHECK_FLAG(net_route_entry->flags, AMTRL3_TYPE_FLAGS_ECMP))
                {
                    AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_IPV6_DEFAULT_ROUTE_MULTIPATH_COUNTER, 1);
                    AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV6_ECMP_NH_IN_OM, 1);
                    if(is_last_in_om)
                    {
                        AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV6_ECMP_ROUTE_IN_OM, 1);
                        AMTRL3_MGR_ClearNetRouteHWInfo(action_flags | AMTRL3_TYPE_FLAGS_ECMP, fib_id, net_route_entry);
                    }
                }
                else
                    AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV6_NET_ROUTE_IN_OM, 1);
            }
            break;
        default:
            break;
    }

    return TRUE;
} /* end of AMTRL3_MGR_DeleteDefaultRoute() */

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_GetDefaultRouteInfo
 * -------------------------------------------------------------------------
 * PURPOSE:  To get default route action type information.
 * INPUT:    action_flags   --  AMTRL3_TYPE_FLAGS_IPV4 \
 *                              AMTRL3_TYPE_FLAGS_IPV6
 *           fib_id     --   FIB id
 *
 * OUTPUT:   action_type    --  ROUTE / TRAP2CPU / DROP
 *           setted         --  Default route is setted or not? (not use)
 * RETURN:
 * NOTES:    Currently used in backdoor, to provide information.
 * -------------------------------------------------------------------------*/
void AMTRL3_MGR_GetDefaultRouteInfo(UI32_T action_flags,
                                    UI32_T fib_id,
                                    UI32_T *action_type,
                                    UI32_T *multipath_num)
{
    /* BODY
     */
    AMTRL3_MGR_CHECK_OPERATION_MODE_WITHOUT_RETURN_VALUE();
    if(AMTRL3_MGR_GetFIBByID(fib_id) == NULL)
        return;

    if(action_flags == AMTRL3_TYPE_FLAGS_IPV4)
    {
        *action_type = AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_IPV4_DEFAULT_ROUTE_ACTION_TYPE);
        *multipath_num = AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_IPV4_DEFAULT_ROUTE_MULTIPATH_COUNTER);
    }
    else if(action_flags == AMTRL3_TYPE_FLAGS_IPV6)
    {
        *action_type = AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_IPV6_DEFAULT_ROUTE_ACTION_TYPE);
        *multipath_num = AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_IPV6_DEFAULT_ROUTE_MULTIPATH_COUNTER);
    }
    return;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_SetIpNetToPhysicalEntryTimeout
 * -------------------------------------------------------------------------
 * PURPOSE:  Set host route table ageout timer
 * INPUT:    action_flags    --  AMTRL3_TYPE_FLAGS_IPV4 \
 *                               AMTRL3_TYPE_FLAGS_IPV6
 *           fib_id       --  FIB id
 *           v4_timeout      --  ageout time in seconds for ipv4 entries
 *           v6_timeout      --  ageout time in seconds for ipv6 entries
 * OUTPUT:   none.
 * RETURN:   TRUE / FALSE
 * NOTES:    Only set the age timer with the corresponding flag set.
 *           If only AMTRL3_TYPE_FLAGS_IPV4 was set, only ipv4 entries
 *           ageout timer will be set.
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_MGR_SetIpNetToPhysicalEntryTimeout(UI32_T action_flags,
                                                 UI32_T fib_id,
                                                 UI32_T v4_timeout,
                                                 UI32_T v6_timeout)
{
    if(AMTRL3_MGR_GetFIBByID(fib_id) == NULL)
        return FALSE;

    AMTRL3_MGR_CHECK_OPERATION_MODE(FALSE);

    if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
        AMTRL3_OM_SetDatabaseValue(fib_id, AMTRL3_OM_IPV4_HOST_ENTRY_AGEOUT_TIME, v4_timeout);
    if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
        AMTRL3_OM_SetDatabaseValue(fib_id, AMTRL3_OM_IPV6_HOST_ENTRY_AGEOUT_TIME, v6_timeout);

    return TRUE;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME : AMTRL3_PMGR_ClearAllDynamicARP
 * -------------------------------------------------------------------------
 * PURPOSE: Clear all dynamic ARP from chip and TCP/IP stack
 *
 * INPUT:
 *      action_flags    -- AMTRL3_TYPE_FLAGS_IPV4 \
 *                         AMTRL3_TYPE_FLAGS_IPV6
 *      fib_id          -- FIB id
 *
 * OUTPUT: none.
 *
 * RETURN:
 *          TRUE        -- Delete successfully
 *          FALSE:      -- Delete fail
 * NOTES:  If set AMTRL3_TYPE_FLAGS_IPV4 flag, only delete all ipv4 dynamic ARP.
 *         If set AMTRL3_TYPE_FLAGS_IPV6 flag, only delete all ipv6 dynamic ARP.
 *         If all flag set, delete all ipv4 & ipv6 dynamic ARP.
 * -------------------------------------------------------------------------*/
BOOL_T  AMTRL3_MGR_ClearAllDynamicARP(UI32_T action_flags, UI32_T fib_id)
{
    AMTRL3_OM_HostRouteEntry_T          host_route_entry;
    UI32_T                              begin_timetick = 0, end_timetick = 0;
    BOOL_T ret = FALSE;

    if(AMTRL3_MGR_GetFIBByID(fib_id) == NULL)
        return FALSE;

    AMTRL3_MGR_CHECK_OPERATION_MODE(FALSE);

    memset(&host_route_entry, 0, sizeof(AMTRL3_OM_HostRouteEntry_T));

    if (CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4) &&
        CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
    {
        host_route_entry.key_fields.dst_inet_addr.type= L_INET_ADDR_TYPE_IPV4;
    }
    else if (CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
    {
        host_route_entry.key_fields.dst_inet_addr.type= L_INET_ADDR_TYPE_IPV4;
    }
    else if (CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
    {
        host_route_entry.key_fields.dst_inet_addr.type= L_INET_ADDR_TYPE_IPV6;
    }
    else
    {
        return FALSE;
    }

    if (amtrl3_mgr_hostroute_performance_flag)
        begin_timetick = SYSFUN_GetSysTick();

    while (AMTRL3_OM_GetNextHostRouteEntry(action_flags, fib_id, &host_route_entry))
    {
        /* Only process the learnt dynamic ARP entry */
        if (host_route_entry.entry_type != VAL_ipNetToPhysicalExtType_dynamic)
            continue;

        if (amtrl3_mgr_debug_flag & DEBUG_LEVEL_TRACE_HOST)
        {
            UI8_T       ip_str[L_INET_MAX_IPADDR_STR_LEN] = {0};

            AMTRL3_MGR_Ntoa(&(host_route_entry.key_fields.dst_inet_addr), ip_str);
            BACKDOOR_MGR_Printf("%s deleting: %s\n",__FUNCTION__, ip_str);
        }

        if (host_route_entry.ref_count > 0)
            ret = AMTRL3_MGR_HostRouteEventHandler(action_flags, fib_id,
                                HOST_ROUTE_UNREFERENCE_EVENT, &host_route_entry);
        else
            ret = AMTRL3_MGR_HostRouteEventHandler(action_flags, fib_id,
                                HOST_ROUTE_REMOVE_EVENT, &host_route_entry);
        if (ret == FALSE)
        {
            if (amtrl3_mgr_debug_flag & DEBUG_LEVEL_ERROR)
            {
                BACKDOOR_MGR_Printf("%s[%d]: Error while deleting all ARP\n",__FUNCTION__, __LINE__);
            }

            return ret;
        }
    }

    if (amtrl3_mgr_hostroute_performance_flag)
    {
        end_timetick = SYSFUN_GetSysTick();
        BACKDOOR_MGR_Printf("%s delete all dynamic ARP entries takes %d ticks\n", __FUNCTION__, (int)(end_timetick - begin_timetick));
    }

    return TRUE;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_DeleteDynamicNetRoute
 * -------------------------------------------------------------------------
 * PURPOSE:  Delete dynamic net route entries, but not deleting AMTRL3 vm structure
 *           This is a patch for BCM driver due to chip 's deleting behavior. Here is to
 *           inverse the order of adding sequence to delete.
 * INPUT:    ip_address
 *           prefix_length
 * OUTPUT:   none.
 * RETURN:   TRUE / FALSE
 * NOTES:
 *           1. A patch for BCM driver , to delete all related net routes by inverse order of
 *              adding.
 *           2. Key is (ip_address , prefix_length )
 *           3. If key is (0,0) means delete all dynamic net route
 *
 * -------------------------------------------------------------------------*/
BOOL_T  AMTRL3_MGR_DeleteDynamicNetRoute(L_INET_AddrIp_T ip_address , UI32_T prefix_length)
{
    return TRUE;
} /* end of AMTRL3_MGR_DeleteDynamicNetRoute() */

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_IsFirstPathOfECMPRouteInChip
 * -------------------------------------------------------------------------
 * PURPOSE:  If it is the first path of ECMP route
 * INPUT:
 *
 * OUTPUT:   none.
 * RETURN:   TRUE / FALSE
 * NOTES:
 *
 * -------------------------------------------------------------------------*/
static BOOL_T  AMTRL3_MGR_IsFirstPathOfECMPRouteInChip(UI32_T action_flags, UI32_T fib_id, AMTRL3_OM_NetRouteEntry_T  *net_entry)
{
    if(net_entry->hw_info == L_CVRT_UINT_TO_PTR(AMTRL3_OM_HW_INFO_INVALID))
        return TRUE;
    else
        return FALSE;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_IsLastPathOfECMPRouteInOm
 * -------------------------------------------------------------------------
 * PURPOSE:  If it is the last path of ECMP route (include unresolved routes)
 * INPUT:
 *
 * OUTPUT:   none.
 * RETURN:   TRUE / FALSE
 * NOTES:
 *
 * -------------------------------------------------------------------------*/
static BOOL_T  AMTRL3_MGR_IsLastPathOfECMPRouteInOm(UI32_T action_flags, UI32_T fib_id, AMTRL3_OM_NetRouteEntry_T  *net_entry)
{
    UI32_T      num_of_entry = 0;
    AMTRL3_OM_NetRouteEntry_T  net_route_entry_block[SYS_ADPT_MAX_NBR_OF_ECMP_ENTRY_PER_ROUTE];
    AMTRL3_OM_NetRouteEntry_T  *local_net_entry;
    int ready_ecmp_routes = 0;
    UI32_T i;

    memset(net_route_entry_block, 0, sizeof(AMTRL3_OM_NetRouteEntry_T) * SYS_ADPT_MAX_NBR_OF_ECMP_ENTRY_PER_ROUTE);
    local_net_entry = (AMTRL3_OM_NetRouteEntry_T*)net_route_entry_block;
    local_net_entry->inet_cidr_route_entry.inet_cidr_route_dest = net_entry->inet_cidr_route_entry.inet_cidr_route_dest;
    local_net_entry->inet_cidr_route_entry.inet_cidr_route_pfxlen = net_entry->inet_cidr_route_entry.inet_cidr_route_pfxlen;

    AMTRL3_OM_GetNextNNetRouteEntryByDstIp(action_flags,
                                           fib_id,
                                           SYS_ADPT_MAX_NBR_OF_ECMP_ENTRY_PER_ROUTE,
                                           &num_of_entry,
                                           net_route_entry_block);

    for (i = 0; i < num_of_entry; i++)
    {
        if (net_route_entry_block[i].net_route_status == AMTRL3_OM_NET_ROUTE_READY)
            ready_ecmp_routes++;
    }

    if(ready_ecmp_routes == 1)
        return TRUE;
    else
        return FALSE;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME : AMTRL3_MGR_AddECMPRouteMultiPath
 * -------------------------------------------------------------------------
 * PURPOSE: Set ECMP route with multiple paths
 *
 * INPUT:
 *      action_flags    -- AMTRL3_TYPE_FLAGS_IPV4 \
 *                         AMTRL3_TYPE_FLAGS_IPV6
 *      fib_id       -- FIB id
 *      net_route_entry -- array records of forwarding table, only key fields are useful
 *                          in this function
 *                         key :
 *                           inet_cidr_route_dest          -- destination inet address
 *                           inet_cidr_route_pfxlen        -- prefix length
 *                           inet_cidr_route_policy        -- serve as an additional index which
 *                                                            may delineate between multiple entries to
 *                                                            the same destination.
 *                                                            Default is {0,0}
 *                           inet_cidr_route_next_hop      -- next hop inet address.
 *
 * OUTPUT: none.
 *
 * RETURN: TRUE : AMTRL3_MGR_OK       -- Successfully insert to database
 *         FALSE: AMTRL3_MGR_TABLE_FULL      -- over reserved space
 * NOTES: 1. The key fields (dest_type, dest, pfxlen, policy) must be
 *           the same in all records of the array. Upper layer (NSM) must confirm.
 *        2. There should not exist any same route before.
 *        3. Path number should <= AMTRL3_MGR_MAXIMUM_ECMP_PATH.
 * -------------------------------------------------------------------------*/
BOOL_T  AMTRL3_MGR_AddECMPRouteMultiPath(UI32_T action_flags,
                                         UI32_T fib_id,
                                         AMTRL3_TYPE_InetCidrRouteEntry_T  *net_route_entry,
                                         UI32_T num)
{
    /* Local Variable Declaration
     */
    AMTRL3_OM_NetRouteEntry_T       local_net_entry;
    AMTRL3_OM_NetRouteEntry_T       active_net_entry_block[AMTRL3_MGR_MAXIMUM_ECMP_PATH];
    AMTRL3_OM_NetRouteEntry_T       inactive_net_entry_block[AMTRL3_MGR_MAXIMUM_ECMP_PATH];
    AMTRL3_OM_HostRouteEntry_T      local_host_entry;
    AMTRL3_OM_HostRouteEntry_T      active_host_entry_block[AMTRL3_MGR_MAXIMUM_ECMP_PATH];
    UI32_T      begin_timetick = 0, end_timetick = 0;
    UI32_T      host_begin = 0, host_end = 0;
    BOOL_T      ret = FALSE, is_default_route = FALSE;
    UI8_T       ip_str[L_INET_MAX_IPADDR_STR_LEN] = {0};
    UI32_T      i = 0, active_count = 0, inactive_count = 0;

    /* BODY
     */
    if (amtrl3_mgr_debug_flag & DEBUG_LEVEL_TRACE_NET)
    {
        BACKDOOR_MGR_Printf("%s, %d\r\n", __FUNCTION__, __LINE__);
    }

    if(AMTRL3_MGR_GetFIBByID(fib_id) == NULL)
        return FALSE;

    if(amtrl3_mgr_netroute_performance_flag)
        begin_timetick = SYSFUN_GetSysTick();

    memset(&local_net_entry, 0, sizeof(AMTRL3_OM_NetRouteEntry_T));
    memset(active_net_entry_block, 0, sizeof(AMTRL3_OM_NetRouteEntry_T) * AMTRL3_MGR_MAXIMUM_ECMP_PATH);
    memset(inactive_net_entry_block, 0, sizeof(AMTRL3_OM_NetRouteEntry_T) * AMTRL3_MGR_MAXIMUM_ECMP_PATH);
    memset(active_host_entry_block, 0, sizeof(AMTRL3_OM_HostRouteEntry_T) * AMTRL3_MGR_MAXIMUM_ECMP_PATH);

    AMTRL3_MGR_CHECK_OPERATION_MODE(FALSE);

    if((num < 1) || (num > AMTRL3_MGR_MAXIMUM_ECMP_PATH))
        return FALSE;

    net_route_entry[0].partial_entry.inet_cidr_route_dest.preflen = net_route_entry[0].partial_entry.inet_cidr_route_pfxlen;
    L_INET_ApplyMask(&net_route_entry[0].partial_entry.inet_cidr_route_dest);

    local_net_entry.inet_cidr_route_entry = net_route_entry[0].partial_entry;

    /* Ensure that the route should not exist in OM before add multi path */
    if(AMTRL3_OM_GetOneNetRouteEntryByDstIp(action_flags, fib_id, &local_net_entry))
    {
        if (amtrl3_mgr_debug_flag & DEBUG_LEVEL_TRACE_NET)
        {
            BACKDOOR_MGR_Printf("%s, %d, route exists in om, failed.\r\n", __FUNCTION__, __LINE__);
        }
        return FALSE;
    }

    if (amtrl3_mgr_debug_flag & DEBUG_LEVEL_TRACE_NET)
    {
        BACKDOOR_MGR_Printf("%s, %d, keep going...\r\n", __FUNCTION__, __LINE__);
    }

    for(i = 0; i < num; i++)
    {
        if(net_route_entry[i].partial_entry.inet_cidr_route_metric1 < 0)
            return FALSE;
        if(AMTRL3_OM_IsAddressEqualZero(&(net_route_entry[i].partial_entry.inet_cidr_route_next_hop)) == TRUE)
            return FALSE;

        /* get exact dest for all paths */
        if(i > 0)
            net_route_entry[i].partial_entry.inet_cidr_route_dest = net_route_entry[0].partial_entry.inet_cidr_route_dest;
    }

    if((AMTRL3_OM_IsAddressEqualZero(&(net_route_entry[0].partial_entry.inet_cidr_route_dest)) == TRUE) &&
       (net_route_entry[0].partial_entry.inet_cidr_route_pfxlen == 0))
        is_default_route = TRUE;

    /* add */
    for(i = 0; i < num; i++)
    {
        if(amtrl3_mgr_debug_flag & DEBUG_LEVEL_TRACE_NET)
        {
            AMTRL3_MGR_Ntoa(&(net_route_entry[i].partial_entry.inet_cidr_route_dest), ip_str);
            BACKDOOR_MGR_Printf("%s IP: %s, ", __FUNCTION__, ip_str);

            BACKDOOR_MGR_Printf("Prefix-length: %d, ", (int)net_route_entry[i].partial_entry.inet_cidr_route_pfxlen);

            AMTRL3_MGR_Ntoa(&(net_route_entry[i].partial_entry.inet_cidr_route_next_hop), ip_str);
            BACKDOOR_MGR_Printf("nhop: %s, if_index %d\n", ip_str, (int)net_route_entry[i].partial_entry.inet_cidr_route_if_index);
        }

        /* If designated gateway entry alreaedy exist in OM, increment its reference count,
           Otherwise, create a gateway entry whose Ip address is next hop of this net
           route entry */
        if(amtrl3_mgr_netroute_performance_flag)
            host_begin = SYSFUN_GetSysTick();

        memset(&local_host_entry, 0, sizeof(AMTRL3_OM_HostRouteEntry_T));
        local_host_entry.key_fields.dst_inet_addr = net_route_entry[i].partial_entry.inet_cidr_route_next_hop;
        local_host_entry.key_fields.dst_vid_ifindex = net_route_entry[i].partial_entry.inet_cidr_route_if_index;

        /* If the nexthop host entry already exist in OM, get it out.
         * If not exist, threat it as dynamic and set it to OM. */
        if(AMTRL3_OM_GetHostRouteEntry(action_flags, fib_id, &local_host_entry) == FALSE)
        {
            local_host_entry.entry_type = VAL_ipNetToPhysicalExtType_dynamic;
            local_host_entry.hw_info = L_CVRT_UINT_TO_PTR(AMTRL3_OM_HW_INFO_INVALID);
        }
        local_host_entry.ref_count++;
        local_host_entry.ecmp_ref_count++;

        if(!AMTRL3_MGR_HostRouteEventHandler(action_flags, fib_id, HOST_ROUTE_PARTIAL_EVENT, &local_host_entry))
            return ret;

        if(amtrl3_mgr_netroute_performance_flag)
        {
            host_end = SYSFUN_GetSysTick();
            BACKDOOR_MGR_Printf("Host route takes %d ticks\n", (int)(host_end - host_begin));
        }

        if(local_host_entry.in_chip_status)
        {
            active_net_entry_block[active_count].inet_cidr_route_entry = net_route_entry[i].partial_entry;
            SET_FLAG(active_net_entry_block[active_count].flags, AMTRL3_TYPE_FLAGS_ECMP);
            active_net_entry_block[active_count].hw_info = L_CVRT_UINT_TO_PTR(AMTRL3_OM_HW_INFO_INVALID);
            memcpy(&active_host_entry_block[active_count], &local_host_entry, sizeof(AMTRL3_OM_HostRouteEntry_T));
            active_count++;
        }
        else
        {
            inactive_net_entry_block[inactive_count].inet_cidr_route_entry = net_route_entry[i].partial_entry;
            SET_FLAG(inactive_net_entry_block[inactive_count].flags, AMTRL3_TYPE_FLAGS_ECMP);
            inactive_net_entry_block[inactive_count].hw_info = L_CVRT_UINT_TO_PTR(AMTRL3_OM_HW_INFO_INVALID);
            inactive_count++;
        }
    }

    /* add ecmp route to chip */
    if(active_count)
    {
    /* removed by steven.gao */
#if 0
        /* Set default route entry specially if chip use default route to trap to cpu */
        if(is_default_route)
        {
            active_net_entry_block[0].hw_info = AMTRL3_OM_HW_INFO_INVALID;
            for(i = 0; i < active_count; i++)
            {
                /* sync hw_info to all active paths if add default route one by one */
                if(CHECK_FLAG(amtrl3_chip_capability_flags, AMTRL3_CHIP_SUPPORT_SAME_HW_INFO_FOR_ECMP_PATH))
                    active_net_entry_block[i].hw_info = active_net_entry_block[0].hw_info;
                if(!AMTRL3_MGR_SetDefaultRouteEntry(action_flags | AMTRL3_TYPE_FLAGS_ECMP, fib_id, &active_net_entry_block[i], &active_host_entry_block[i]))
                {
                    active_net_entry_block[i].net_route_status = AMTRL3_OM_NET_ROUTE_UNRESOLVED;
                    if(amtrl3_mgr_debug_flag & DEBUG_LEVEL_ERROR)
                        BACKDOOR_MGR_Printf("Set default Route Entry Fails\n");
                }
            }
        }
        else
#endif
        ret = AMTRL3_MGR_AddECMPRouteMultiPathToChip(action_flags, fib_id, active_net_entry_block, active_host_entry_block, active_count);

        if (amtrl3_mgr_debug_flag & DEBUG_LEVEL_TRACE_NET)
        {
            BACKDOOR_MGR_Printf("%s, %d, ret: %d\r\n", __FUNCTION__, __LINE__, ret);
        }
    }

    if(CHECK_FLAG(amtrl3_chip_capability_flags, AMTRL3_CHIP_SUPPORT_SAME_HW_INFO_FOR_ECMP_PATH))
    {
        if(active_count)
        {
            /* sync hw_info to all inactive paths */
            for(i = 0; i < inactive_count; i++)
                inactive_net_entry_block[i].hw_info = active_net_entry_block[0].hw_info;
        }
    }

    /* set to OM database */
    for(i = 0; i < active_count; i++)
    {
        ret = AMTRL3_OM_SetNetRouteEntry(action_flags, fib_id, &active_net_entry_block[i]);
    }
    for(i = 0; i < inactive_count; i++)
    {
        inactive_net_entry_block[i].net_route_status = AMTRL3_OM_NET_ROUTE_UNRESOLVED;
        ret = AMTRL3_OM_SetNetRouteEntry(action_flags, fib_id, &inactive_net_entry_block[i]);
    }

    if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
    {
        AMTRL3_OM_IncreaseDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV4_ECMP_NH_IN_OM, num);
        AMTRL3_OM_IncreaseDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV4_ECMP_ROUTE_IN_OM, 1);
        if(is_default_route)
        {
            AMTRL3_OM_IncreaseDatabaseValue(fib_id, AMTRL3_OM_IPV4_DEFAULT_ROUTE_MULTIPATH_COUNTER, num);
            AMTRL3_OM_SetDatabaseValue(fib_id, AMTRL3_OM_IPV4_DEFAULT_ROUTE_ACTION_TYPE, SYS_CPNT_DEFAULT_ROUTE_ACTION_ROUTE);
        }
    }
    else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
    {
        AMTRL3_OM_IncreaseDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV6_ECMP_NH_IN_OM, num);
        AMTRL3_OM_IncreaseDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV6_ECMP_ROUTE_IN_OM, 1);
        if(is_default_route)
        {
            AMTRL3_OM_IncreaseDatabaseValue(fib_id, AMTRL3_OM_IPV6_DEFAULT_ROUTE_MULTIPATH_COUNTER, num);
            AMTRL3_OM_SetDatabaseValue(fib_id, AMTRL3_OM_IPV6_DEFAULT_ROUTE_ACTION_TYPE, SYS_CPNT_DEFAULT_ROUTE_ACTION_ROUTE);
        }
    }

    if(amtrl3_mgr_netroute_performance_flag)
    {
        end_timetick = SYSFUN_GetSysTick();
        BACKDOOR_MGR_Printf("AMTRL3_MGR_AddECMPRouteMultiPath takes %d ticks\n", (int)(end_timetick - begin_timetick));
    }

    return ret;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME : AMTRL3_MGR_DeleteECMPRoute
 * -------------------------------------------------------------------------
 * PURPOSE: Delete the ECMP route with all paths.
 *
 * INPUT:
 *      action_flags    -- AMTRL3_TYPE_FLAGS_IPV4 \
 *                         AMTRL3_TYPE_FLAGS_IPV6
 *      fib_id       -- FIB id
 *      net_route_entry -- only key fields (dest and pfxlen) are useful for deleting
 *                         key :
 *                           inet_cidr_route_dest          -- destination inet address
 *                           inet_cidr_route_pfxlen        -- prefix length
 *
 * OUTPUT: none.
 *
 * RETURN: TRUE : AMTRL3_MGR_OK      -- Successfully remove from database
 *         FALSE:
 *                -AMTRL3_MGR_CAN_NOT_FIND -- This entry does not exist in chip/database.
 *                -AMTRL3_MGR_CAN_NOT_DELETE   -- Cannot Delete
 * NOTES: 1. If route exists, it must set the flag AMTRL3_PMGR_FLAGS_ECMP.
 *        2. action_flags: AMTRL3_PMGR_FLAGS_IPV4/IPV6 must consistent with address type in dest
 * -------------------------------------------------------------------------*/
BOOL_T  AMTRL3_MGR_DeleteECMPRoute(UI32_T action_flags,
                                   UI32_T fib_id,
                                   AMTRL3_TYPE_InetCidrRouteEntry_T  *net_route_entry)
{
    /* Local Variable Declaration
     */
    AMTRL3_OM_NetRouteEntry_T       *local_net_entry;
    AMTRL3_OM_NetRouteEntry_T       net_route_entry_block[AMTRL3_MGR_MAXIMUM_ECMP_PATH];
    AMTRL3_OM_HostRouteEntry_T      local_host_entry, *nhop_entry = NULL;
    AMTRL3_OM_ResolvedNetRouteEntry_T   resolved_net_entry;
    BOOL_T      ret = FALSE, delete_flag = FALSE, is_default_route = FALSE;
    UI8_T       ip_str[L_INET_MAX_IPADDR_STR_LEN] = {0};
    UI32_T      event_type;
    UI32_T      num_of_entry = 0, i = 0, active_count = 0;

    /* BODY
     */
    if(AMTRL3_MGR_GetFIBByID(fib_id) == NULL)
        return FALSE;

    AMTRL3_MGR_CHECK_OPERATION_MODE(FALSE);

    net_route_entry->partial_entry.inet_cidr_route_dest.preflen = net_route_entry->partial_entry.inet_cidr_route_pfxlen;
    L_INET_ApplyMask(&net_route_entry->partial_entry.inet_cidr_route_dest);


    memset(&resolved_net_entry, 0, sizeof(AMTRL3_OM_ResolvedNetRouteEntry_T));
    memset(net_route_entry_block, 0, sizeof(AMTRL3_OM_NetRouteEntry_T) * AMTRL3_MGR_MAXIMUM_ECMP_PATH);

    local_net_entry = (AMTRL3_OM_NetRouteEntry_T*)net_route_entry_block;
    local_net_entry->inet_cidr_route_entry.inet_cidr_route_dest = net_route_entry->partial_entry.inet_cidr_route_dest;
    local_net_entry->inet_cidr_route_entry.inet_cidr_route_pfxlen = net_route_entry->partial_entry.inet_cidr_route_pfxlen;

    ret = AMTRL3_OM_GetNextNNetRouteEntryByDstIp(action_flags,
                                               fib_id,
                                               AMTRL3_MGR_MAXIMUM_ECMP_PATH,
                                               &num_of_entry,
                                               net_route_entry_block);

    if((ret == FALSE) || (num_of_entry == 0))
    {
        return AMTRL3_MGR_NO_MORE_ENTRY;
    }

    if((AMTRL3_OM_IsAddressEqualZero(&(local_net_entry->inet_cidr_route_entry.inet_cidr_route_dest)) == TRUE) &&
        (local_net_entry->inet_cidr_route_entry.inet_cidr_route_pfxlen == 0))
        is_default_route = TRUE;

    if(is_default_route)
    {
        for(i = 0; i < num_of_entry; i++)
        {
            memset(&local_host_entry, 0, sizeof(AMTRL3_OM_HostRouteEntry_T));
            local_net_entry = (AMTRL3_OM_NetRouteEntry_T *)(&net_route_entry_block[i]);
            local_host_entry.key_fields.dst_vid_ifindex = local_net_entry->inet_cidr_route_entry.inet_cidr_route_if_index;
            local_host_entry.key_fields.dst_inet_addr = local_net_entry->inet_cidr_route_entry.inet_cidr_route_next_hop;
            if(AMTRL3_OM_GetHostRouteEntry(action_flags, fib_id, &local_host_entry))
            {
                local_host_entry.ref_count--;
                local_host_entry.ecmp_ref_count--;

                event_type = HOST_ROUTE_PARTIAL_EVENT;

                if (!AMTRL3_MGR_HostRouteEventHandler(action_flags, fib_id, event_type, &local_host_entry))
                {
                    //Error set host route entry
                }
                nhop_entry = &local_host_entry;
            }
            AMTRL3_MGR_DeleteDefaultRoute(action_flags, fib_id, &net_route_entry_block[i], nhop_entry);
        }
        return TRUE;
    }

    for(i = 0; i < num_of_entry; i++)
    {
        local_net_entry = (AMTRL3_OM_NetRouteEntry_T *)(&net_route_entry_block[i]);
        /* check ecmp flag first */
        if(!CHECK_FLAG(local_net_entry->flags, AMTRL3_TYPE_FLAGS_ECMP))
            return FALSE;

        if(local_net_entry->net_route_status == AMTRL3_OM_NET_ROUTE_READY)
            active_count++;
    }

    for(i = 0; i < num_of_entry; i++)
    {
        local_net_entry = (AMTRL3_OM_NetRouteEntry_T *)(&net_route_entry_block[i]);

        if(amtrl3_mgr_debug_flag & DEBUG_LEVEL_TRACE_NET)
        {
            AMTRL3_MGR_Ntoa(&(local_net_entry->inet_cidr_route_entry.inet_cidr_route_dest), ip_str);
            BACKDOOR_MGR_Printf("AL3_DeleteECMPRouteEntry IP: %s, ", ip_str);
            BACKDOOR_MGR_Printf("Prefix-length: %d, ", (int)local_net_entry->inet_cidr_route_entry.inet_cidr_route_pfxlen);
            AMTRL3_MGR_Ntoa(&(local_net_entry->inet_cidr_route_entry.inet_cidr_route_next_hop), ip_str);
            BACKDOOR_MGR_Printf("nhop: %s\n", ip_str);
        }

        /* Update reference count of the designated gateway entry  */
        memset(&local_host_entry, 0, sizeof(AMTRL3_OM_HostRouteEntry_T));
        local_host_entry.key_fields.dst_vid_ifindex = local_net_entry->inet_cidr_route_entry.inet_cidr_route_if_index;
        local_host_entry.key_fields.dst_inet_addr = local_net_entry->inet_cidr_route_entry.inet_cidr_route_next_hop;
        if(AMTRL3_OM_GetHostRouteEntry(action_flags, fib_id, &local_host_entry))
        {
            local_host_entry.ref_count--;
            local_host_entry.ecmp_ref_count--;

            event_type = HOST_ROUTE_PARTIAL_EVENT;

            /* Appropriate host route status should be determined in MGR thru the
               Finite State Machine before passing to OM for data storage.
             */
            if (!AMTRL3_MGR_HostRouteEventHandler(action_flags, fib_id, event_type, &local_host_entry))
            {
                //Error set host route entry
            }
            nhop_entry = &local_host_entry;
        } /* end of if */

        if(local_net_entry->net_route_status == AMTRL3_OM_NET_ROUTE_READY)
        {
            /* delete the route with all paths from chip, if it's the first time */
            if(!delete_flag)
            {
                if(AMTRL3_MGR_DeleteECMPRouteFromChip(action_flags, fib_id, local_net_entry, active_count))
                    delete_flag = TRUE;
            }

            /* Update corresponding host route ref_count_in_chip if net route in ready state is deleted from chip
             */
            AMTRL3_MGR_UpdateHostRouteRefCountInChip(action_flags, fib_id, &local_host_entry, -1);

            if (local_host_entry.key_fields.dst_inet_addr.type == L_INET_ADDR_TYPE_IPV6Z &&
                local_host_entry.ref_count_in_chip == 0)
            {
                AMTRL3_MGR_HostRouteEventHandler(action_flags, fib_id, HOST_ROUTE_UNREFERENCE_EVENT, &local_host_entry);
            }
        }
        else if (local_net_entry->net_route_status == AMTRL3_OM_NET_ROUTE_RESOLVED)
        {
            if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
                resolved_net_entry.inverse_prefix_length = (AMTRL3_MGR_IPV4_MAXIMUM_PREFIX_LENGTH - local_net_entry->inet_cidr_route_entry.inet_cidr_route_pfxlen);
            else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
                resolved_net_entry.inverse_prefix_length = (AMTRL3_MGR_IPV6_MAXIMUM_PREFIX_LENGTH - local_net_entry->inet_cidr_route_entry.inet_cidr_route_pfxlen);
            resolved_net_entry.inet_cidr_route_dest = local_net_entry->inet_cidr_route_entry.inet_cidr_route_dest;
            resolved_net_entry.inet_cidr_route_next_hop = local_net_entry->inet_cidr_route_entry.inet_cidr_route_next_hop;

            if(AMTRL3_OM_DeleteResolvedNetEntry(action_flags, fib_id, &resolved_net_entry))
            {
                if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
                    AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_IPV4_RESOLVED_NET_ROUTE_COUNT, 1);
                else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
                    AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_IPV6_RESOLVED_NET_ROUTE_COUNT, 1);
            }
            else
            {
                if(amtrl3_mgr_debug_flag & DEBUG_LEVEL_ERROR)
                    BACKDOOR_MGR_Printf("AMTRL3_OM_DeleteResolvedNetEntry Fails\n");
                return (FALSE);
            }
        }

        AMTRL3_OM_DeleteNetRouteEntry(action_flags, fib_id, local_net_entry);

        /* For some chip which support ecmp table, all path (include unresolved) have the same hw info,
         * thus, there may be no path in chip, but hw info is not invalid.
         * So, finally, we should clear the hw info from chip */
        if(!delete_flag && (i == num_of_entry))
            AMTRL3_MGR_ClearNetRouteHWInfo(action_flags | AMTRL3_TYPE_FLAGS_ECMP, fib_id, local_net_entry);
    }

    if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
    {
        AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV4_ECMP_NH_IN_OM, num_of_entry);
        AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV4_ECMP_ROUTE_IN_OM, 1);
        if(is_default_route)
            AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_IPV4_DEFAULT_ROUTE_MULTIPATH_COUNTER, num_of_entry);
    }
    else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
    {
        AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV6_ECMP_NH_IN_OM, num_of_entry);
        AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV6_ECMP_ROUTE_IN_OM, 1);
        if(is_default_route)
            AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_IPV4_DEFAULT_ROUTE_MULTIPATH_COUNTER, num_of_entry);
    }

    return TRUE;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME : AMTRL3_MGR_AddECMPRouteOnePath
 * -------------------------------------------------------------------------
 * PURPOSE: Set ECMP route with one path
 *
 * INPUT:
 *      action_flags    -- AMTRL3_TYPE_FLAGS_IPV4 \
 *                         AMTRL3_TYPE_FLAGS_IPV6
 *      fib_id       -- FIB id
 *      net_route_entry -- records of forwarding table, only key fields are useful
 *                          in this function
 *                         key :
 *                           inet_cidr_route_dest          -- destination inet address
 *                           inet_cidr_route_pfxlen        -- prefix length
 *                           inet_cidr_route_policy        -- serve as an additional index which
 *                                                            may delineate between multiple entries to
 *                                                            the same destination.
 *                                                            Default is {0,0}
 *                           inet_cidr_route_next_hop      -- next hop inet address.
 *
 * OUTPUT: none.
 *
 * RETURN: TRUE : AMTRL3_MGR_OK       -- Successfully insert to database
 *         FALSE: AMTRL3_MGR_TABLE_FULL      -- over reserved space
 * NOTES: 1. If route exists, it must set the flag AMTRL3_PMGR_FLAGS_ECMP.
 * -------------------------------------------------------------------------*/
BOOL_T  AMTRL3_MGR_AddECMPRouteOnePath(UI32_T action_flags,
                                       UI32_T fib_id,
                                       AMTRL3_TYPE_InetCidrRouteEntry_T  *net_route_entry)
{
    /* Local Variable Declaration
     */
    AMTRL3_OM_NetRouteEntry_T       local_net_entry;
    AMTRL3_OM_HostRouteEntry_T      local_host_entry;
    UI32_T      begin_timetick = 0, end_timetick = 0;
    UI32_T      host_begin = 0, host_end = 0;
    BOOL_T      ret = FALSE, is_default_route = FALSE;
    BOOL_T      is_first_in_chip = FALSE, is_first_in_om = TRUE;
    UI8_T       ip_str[L_INET_MAX_IPADDR_STR_LEN] = {0};

    /* BODY
     */
    if(AMTRL3_MGR_GetFIBByID(fib_id) == NULL)
        return FALSE;

    if(amtrl3_mgr_netroute_performance_flag)
        begin_timetick = SYSFUN_GetSysTick();

    memset(&local_net_entry, 0, sizeof(AMTRL3_OM_NetRouteEntry_T));
    memset(&local_host_entry, 0, sizeof(AMTRL3_OM_HostRouteEntry_T));

    AMTRL3_MGR_CHECK_OPERATION_MODE(FALSE);

    if(net_route_entry->partial_entry.inet_cidr_route_metric1 < 0)
        return FALSE;

    net_route_entry->partial_entry.inet_cidr_route_dest.preflen = net_route_entry->partial_entry.inet_cidr_route_pfxlen;
    L_INET_ApplyMask(&net_route_entry->partial_entry.inet_cidr_route_dest);


    local_net_entry.inet_cidr_route_entry.inet_cidr_route_dest = net_route_entry->partial_entry.inet_cidr_route_dest;
    local_net_entry.inet_cidr_route_entry.inet_cidr_route_pfxlen = net_route_entry->partial_entry.inet_cidr_route_pfxlen;

    /* Get path of this ECMP route, if exists, get the hw_info to program chip */
    if(AMTRL3_OM_GetOneNetRouteEntryByDstIp(action_flags, fib_id, &local_net_entry))
    {
        if(!CHECK_FLAG(local_net_entry.flags, AMTRL3_TYPE_FLAGS_ECMP))
            return FALSE;

        /* If exists the same route, return */
        if(AMTRL3_OM_IsAddressEqual(&local_net_entry.inet_cidr_route_entry.inet_cidr_route_next_hop,
                                    &net_route_entry->partial_entry.inet_cidr_route_next_hop))
            return TRUE;

        if(!CHECK_FLAG(amtrl3_chip_capability_flags, AMTRL3_CHIP_SUPPORT_SAME_HW_INFO_FOR_ECMP_PATH))
            local_net_entry.hw_info = L_CVRT_UINT_TO_PTR(AMTRL3_OM_HW_INFO_INVALID);

        is_first_in_om = FALSE;
    }
    else
        local_net_entry.hw_info = L_CVRT_UINT_TO_PTR(AMTRL3_OM_HW_INFO_INVALID);

    local_net_entry.inet_cidr_route_entry = net_route_entry->partial_entry;

    if(AMTRL3_MGR_IsFirstPathOfECMPRouteInChip(action_flags, fib_id, &local_net_entry))
        is_first_in_chip = TRUE;

    if(amtrl3_mgr_debug_flag & DEBUG_LEVEL_TRACE_NET)
    {
        AMTRL3_MGR_Ntoa(&(net_route_entry->partial_entry.inet_cidr_route_dest), ip_str);
        BACKDOOR_MGR_Printf("AL3_AddECMPRouteOnePath IP: %s, ", ip_str);

        BACKDOOR_MGR_Printf("Prefix-length: %d, ", (int)net_route_entry->partial_entry.inet_cidr_route_pfxlen);

        AMTRL3_MGR_Ntoa(&(net_route_entry->partial_entry.inet_cidr_route_next_hop), ip_str);
        BACKDOOR_MGR_Printf("nhop: %s, if_index %d\n", ip_str, (int)net_route_entry->partial_entry.inet_cidr_route_if_index);
    }

    SET_FLAG(local_net_entry.flags, AMTRL3_TYPE_FLAGS_ECMP);

    /* If designated gateway entry alreaedy exist in OM, increment its reference count,
       Otherwise, create a gateway entry whose Ip address is next hop of this net
       route entry */
    if(AMTRL3_OM_IsAddressEqualZero(&(net_route_entry->partial_entry.inet_cidr_route_next_hop)) == FALSE)
    {
        if(amtrl3_mgr_netroute_performance_flag)
            host_begin = SYSFUN_GetSysTick();

        local_host_entry.key_fields.dst_inet_addr = net_route_entry->partial_entry.inet_cidr_route_next_hop;
        local_host_entry.key_fields.dst_vid_ifindex = net_route_entry->partial_entry.inet_cidr_route_if_index;

        /* If the nexthop host entry already exist in OM, get it out.
         * If not exist, threat it as dynamic and set it to OM. */
        if(AMTRL3_OM_GetHostRouteEntry(action_flags, fib_id, &local_host_entry) == FALSE)
        {
            local_host_entry.entry_type = VAL_ipNetToPhysicalExtType_dynamic;
            local_host_entry.hw_info = L_CVRT_UINT_TO_PTR(AMTRL3_OM_HW_INFO_INVALID);
        }
        local_host_entry.ref_count++;
        local_host_entry.ecmp_ref_count++;

        if(!AMTRL3_MGR_HostRouteEventHandler(action_flags, fib_id, HOST_ROUTE_PARTIAL_EVENT, &local_host_entry))
            return ret;

        if(amtrl3_mgr_netroute_performance_flag)
        {
            host_end = SYSFUN_GetSysTick();
            BACKDOOR_MGR_Printf("Host route takes %d ticks\n", (int)(host_end - host_begin));
        }
    }
    else
        return FALSE;

    if((AMTRL3_OM_IsAddressEqualZero(&(net_route_entry->partial_entry.inet_cidr_route_dest)) == TRUE) &&
       (net_route_entry->partial_entry.inet_cidr_route_pfxlen == 0))
        is_default_route = TRUE;

    /* removed by steven.gao */
#if 0
    /* Set default route specially */
    if(is_default_route && local_host_entry.in_chip_status)
    {
        if(!AMTRL3_MGR_SetDefaultRouteEntry(action_flags | AMTRL3_TYPE_FLAGS_ECMP, fib_id, &local_net_entry, &local_host_entry))
        {
            local_net_entry.net_route_status = AMTRL3_OM_NET_ROUTE_UNRESOLVED;
            if(amtrl3_mgr_debug_flag & DEBUG_LEVEL_ERROR)
                BACKDOOR_MGR_Printf("Set default Route Entry Fails\n");
        }
    }
    else
    {
#endif

    local_net_entry.inet_cidr_route_entry = net_route_entry->partial_entry;
    if(local_host_entry.in_chip_status)
    {
        if(!AMTRL3_MGR_AddECMPRouteOnePathToChip(action_flags, fib_id, &local_net_entry, &local_host_entry, is_first_in_chip))
            local_net_entry.net_route_status = AMTRL3_OM_NET_ROUTE_UNRESOLVED;
    }
    else
    {
        local_net_entry.net_route_status = AMTRL3_OM_NET_ROUTE_UNRESOLVED;
    }

    /* sync hw_info to all paths if create the first path in chip */
    if(is_first_in_chip && (local_net_entry.net_route_status == AMTRL3_OM_NET_ROUTE_READY))
    {
        if(CHECK_FLAG(amtrl3_chip_capability_flags, AMTRL3_CHIP_SUPPORT_SAME_HW_INFO_FOR_ECMP_PATH))
            AMTRL3_MGR_SyncHWInfoToAllECMPPath(action_flags, fib_id, &local_net_entry);
    }

    /* set to OM database */
    ret = AMTRL3_OM_SetNetRouteEntry(action_flags, fib_id, &local_net_entry);
    if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
    {
        AMTRL3_OM_IncreaseDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV4_ECMP_NH_IN_OM, 1);
        if(is_first_in_om)
            AMTRL3_OM_IncreaseDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV4_ECMP_ROUTE_IN_OM, 1);
        if(is_default_route)
        {
            AMTRL3_OM_IncreaseDatabaseValue(fib_id, AMTRL3_OM_IPV4_DEFAULT_ROUTE_MULTIPATH_COUNTER, 1);
            AMTRL3_OM_SetDatabaseValue(fib_id, AMTRL3_OM_IPV4_DEFAULT_ROUTE_ACTION_TYPE, SYS_CPNT_DEFAULT_ROUTE_ACTION_ROUTE);
        }
    }
    else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
    {
        AMTRL3_OM_IncreaseDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV6_ECMP_NH_IN_OM, 1);
        if(is_first_in_om)
            AMTRL3_OM_IncreaseDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV6_ECMP_ROUTE_IN_OM, 1);
        if(is_default_route)
        {
            AMTRL3_OM_IncreaseDatabaseValue(fib_id, AMTRL3_OM_IPV6_DEFAULT_ROUTE_MULTIPATH_COUNTER, 1);
            AMTRL3_OM_SetDatabaseValue(fib_id, AMTRL3_OM_IPV6_DEFAULT_ROUTE_ACTION_TYPE, SYS_CPNT_DEFAULT_ROUTE_ACTION_ROUTE);
        }
    }

    if(amtrl3_mgr_netroute_performance_flag)
    {
        end_timetick = SYSFUN_GetSysTick();
        BACKDOOR_MGR_Printf("AMTRL3_MGR_AddECMPRouteMultiPath takes %d ticks\n", (int)(end_timetick - begin_timetick));
    }

    return ret;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME : AMTRL3_MGR_DeleteECMPRouteOnePath
 * -------------------------------------------------------------------------
 * PURPOSE: Delete the ECMP route with the given path
 *
 * INPUT:
 *      action_flags    -- AMTRL3_TYPE_FLAGS_IPV4 \
 *                         AMTRL3_TYPE_FLAGS_IPV6
 *      fib_id       -- FIB id
 *      net_route_entry -- records of forwarding table, only key fields are useful
 *                          in this function
 *                         key :
 *                           inet_cidr_route_dest          -- destination inet address
 *                           inet_cidr_route_pfxlen        -- prefix length
 *                           inet_cidr_route_policy        -- serve as an additional index which
 *                                                            may delineate between multiple entries to
 *                                                            the same destination.
 *                                                            Default is {0,0}
 *                           inet_cidr_route_next_hop      -- next hop inet address.
 *
 * OUTPUT: none.
 *
 * RETURN: TRUE : AMTRL3_MGR_OK       -- Successfully remove from database
 *         FALSE:
 *                   -AMTRL3_MGR_CAN_NOT_FIND -- This entry does not exist in chip/database.
 *                    AMTRL3_MGR_CAN_NOT_DELETE        -- Cannot Delete
 * NOTES: 1. If route exists, it must set the flag AMTRL3_PMGR_FLAGS_ECMP.
 *        2. action_flags: AMTRL3_TYPE_FLAGS_IPV4/IPV6 must consistent with address type in dest.
 * -------------------------------------------------------------------------*/
BOOL_T  AMTRL3_MGR_DeleteECMPRouteOnePath(UI32_T action_flags,
                                          UI32_T fib_id,
                                          AMTRL3_TYPE_InetCidrRouteEntry_T  *net_route_entry)
{
    /* Local Variable Declaration
     */
    AMTRL3_OM_NetRouteEntry_T       local_net_entry;
    AMTRL3_OM_HostRouteEntry_T      local_host_entry, *nhop_entry = NULL;
    AMTRL3_OM_ResolvedNetRouteEntry_T   resolved_net_entry;
    BOOL_T      ret = FALSE, is_last_in_om = FALSE, is_default_route = FALSE;
    UI8_T       ip_str[L_INET_MAX_IPADDR_STR_LEN] = {0};
    UI32_T      event_type;

    /* BODY
     */
    if(AMTRL3_MGR_GetFIBByID(fib_id) == NULL)
        return FALSE;

    AMTRL3_MGR_CHECK_OPERATION_MODE(FALSE);

    net_route_entry->partial_entry.inet_cidr_route_dest.preflen = net_route_entry->partial_entry.inet_cidr_route_pfxlen;
    L_INET_ApplyMask(&net_route_entry->partial_entry.inet_cidr_route_dest);


    memset(&local_net_entry, 0, sizeof(AMTRL3_OM_NetRouteEntry_T));
    memset(&local_host_entry, 0, sizeof(AMTRL3_OM_HostRouteEntry_T));
    memset(&resolved_net_entry, 0, sizeof(AMTRL3_OM_ResolvedNetRouteEntry_T));

    local_net_entry.inet_cidr_route_entry = net_route_entry->partial_entry;

    /* find the exact route out */
    if(AMTRL3_OM_GetNetRouteEntry(action_flags, fib_id, &local_net_entry))
    {
        if(!CHECK_FLAG(local_net_entry.flags, AMTRL3_TYPE_FLAGS_ECMP))
            return FALSE;
    }
    else /* route not exist */
        return FALSE;

    if(amtrl3_mgr_debug_flag & DEBUG_LEVEL_TRACE_NET)
    {
        AMTRL3_MGR_Ntoa(&(local_net_entry.inet_cidr_route_entry.inet_cidr_route_dest), ip_str);
        BACKDOOR_MGR_Printf("AL3_DeleteECMPRouteEntry IP: %s, ", ip_str);
        BACKDOOR_MGR_Printf("Prefix-length: %d, ", (int)local_net_entry.inet_cidr_route_entry.inet_cidr_route_pfxlen);
        AMTRL3_MGR_Ntoa(&(local_net_entry.inet_cidr_route_entry.inet_cidr_route_next_hop), ip_str);
        BACKDOOR_MGR_Printf("nhop: %s\n", ip_str);
    }

    /* Update reference count of the designated gateway entry  */
    local_host_entry.key_fields.dst_vid_ifindex = local_net_entry.inet_cidr_route_entry.inet_cidr_route_if_index;
    local_host_entry.key_fields.dst_inet_addr = local_net_entry.inet_cidr_route_entry.inet_cidr_route_next_hop;
    if(AMTRL3_OM_GetHostRouteEntry(action_flags, fib_id, &local_host_entry))
    {
        local_host_entry.ref_count--;
        local_host_entry.ecmp_ref_count--;

        event_type = HOST_ROUTE_PARTIAL_EVENT;

        /* Appropriate host route status should be determined in MGR thru the
           Finite State Machine before passing to OM for data storage.
         */
        if (!AMTRL3_MGR_HostRouteEventHandler(action_flags, fib_id, event_type, &local_host_entry))
        {
            //Error set host route entry
        }
        nhop_entry = &local_host_entry;
    }
    else
        BACKDOOR_MGR_Printf("Host not found when delete one ECMP path!\n");

    if(AMTRL3_MGR_IsLastPathOfECMPRouteInOm(action_flags, fib_id, &local_net_entry))
        is_last_in_om = TRUE;

    if((AMTRL3_OM_IsAddressEqualZero(&(local_net_entry.inet_cidr_route_entry.inet_cidr_route_dest)) == TRUE) &&
        (local_net_entry.inet_cidr_route_entry.inet_cidr_route_pfxlen == 0))
        is_default_route = TRUE;

    if(is_default_route)
    {
        return AMTRL3_MGR_DeleteDefaultRoute(action_flags, fib_id, &local_net_entry, nhop_entry);
    }

    if(local_net_entry.net_route_status == AMTRL3_OM_NET_ROUTE_READY)
    {
        if(!AMTRL3_MGR_DeleteECMPRouteOnePathFromChip(action_flags, fib_id, &local_net_entry, nhop_entry, is_last_in_om))
        {
            return FALSE;
        }

        if (nhop_entry->key_fields.dst_inet_addr.type == L_INET_ADDR_TYPE_IPV6Z &&
            nhop_entry->ref_count_in_chip == 0)
        {
            AMTRL3_MGR_HostRouteEventHandler(action_flags, fib_id, HOST_ROUTE_UNREFERENCE_EVENT, nhop_entry);
        }
    }
    else if(local_net_entry.net_route_status == AMTRL3_OM_NET_ROUTE_RESOLVED)
    {
        if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
            resolved_net_entry.inverse_prefix_length = (AMTRL3_MGR_IPV4_MAXIMUM_PREFIX_LENGTH - local_net_entry.inet_cidr_route_entry.inet_cidr_route_pfxlen);
        else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
            resolved_net_entry.inverse_prefix_length = (AMTRL3_MGR_IPV6_MAXIMUM_PREFIX_LENGTH - local_net_entry.inet_cidr_route_entry.inet_cidr_route_pfxlen);
        resolved_net_entry.inet_cidr_route_dest = local_net_entry.inet_cidr_route_entry.inet_cidr_route_dest;
        resolved_net_entry.inet_cidr_route_next_hop = local_net_entry.inet_cidr_route_entry.inet_cidr_route_next_hop;

        if(AMTRL3_OM_DeleteResolvedNetEntry(action_flags, fib_id, &resolved_net_entry))
        {
            if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
                AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_IPV4_RESOLVED_NET_ROUTE_COUNT, 1);
            else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
                AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_IPV6_RESOLVED_NET_ROUTE_COUNT, 1);
        }
        else
        {
            if(amtrl3_mgr_debug_flag & DEBUG_LEVEL_ERROR)
                BACKDOOR_MGR_Printf("AMTRL3_OM_DeleteResolvedNetEntry Fails\n");
            return (FALSE);
        }
    }

    ret = AMTRL3_OM_DeleteNetRouteEntry(action_flags, fib_id, &local_net_entry);
    if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
    {
        AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV4_ECMP_NH_IN_OM, 1);
        if(is_last_in_om)
            AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV4_ECMP_ROUTE_IN_OM, 1);
    }
    else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
    {
        AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV6_ECMP_NH_IN_OM, 1);
        if(is_last_in_om)
            AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV6_ECMP_ROUTE_IN_OM, 1);
    }

    if(is_last_in_om)
        AMTRL3_MGR_ClearNetRouteHWInfo(action_flags | AMTRL3_TYPE_FLAGS_ECMP, fib_id, &local_net_entry);

    return ret;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_GetNumOfInetHostRouteEntry
 * -------------------------------------------------------------------------
 * PURPOSE: Get the total number of IPv4 or v6 Host Route Entries
 * INPUT:
 *      action_flags    -- AMTRL3_TYPE_FLAGS_IPV4 \
 *                         AMTRL3_TYPE_FLAGS_IPV6
 *      fib_id       -- FIB id
 * OUTPUT:  number_of_ip_host_route_entry
 * RETURN:  TRUE / FALSE
 * NOTES:
 * -------------------------------------------------------------------------
 */
BOOL_T AMTRL3_MGR_GetNumOfInetHostRouteEntry(
                                        UI32_T action_flags,
                                        UI32_T fib_id,
                                        UI32_T *number_of_ip_host_route_entry)
{
    AMTRL3_MGR_CHECK_OPERATION_MODE(FALSE);

    if(number_of_ip_host_route_entry == NULL)
       return FALSE;
    if(AMTRL3_MGR_GetFIBByID(fib_id) == NULL)
        return FALSE;

    if(action_flags == AMTRL3_TYPE_FLAGS_IPV4)
        *number_of_ip_host_route_entry = AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_IPV4_TOTAL_IPNET2PHYSICAL_COUNTS);
    else if(action_flags == AMTRL3_TYPE_FLAGS_IPV6)
        *number_of_ip_host_route_entry = AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_IPV6_TOTAL_IPNET2PHYSICAL_COUNTS);
    else if(action_flags == (AMTRL3_TYPE_FLAGS_IPV4 | AMTRL3_TYPE_FLAGS_IPV6))
        *number_of_ip_host_route_entry = AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_IPV4_TOTAL_IPNET2PHYSICAL_COUNTS) +
                                         AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_IPV6_TOTAL_IPNET2PHYSICAL_COUNTS);

    return TRUE;
}

/* -------------------------------------------------------------------------
 *  FUNCTION NAME: AMTRL3_MGR_LocalResolveBgpUnnumberedIpv4Lla
 * -------------------------------------------------------------------------
 * PURPOSE:  To resolve the reserved ipv4 lla (169.254.0.1) used for
 *           bgp unnumbered.
 * INPUT:    host_entry_p - pointer to host_entry to resolve
 * OUTPUT:   host_entry_p - pointer to host_entry of result
 * RETURN:   TRUE/FALSE
 * NOTES:    None
 * -------------------------------------------------------------------------*/
static BOOL_T AMTRL3_MGR_LocalResolveBgpUnnumberedIpv4Lla(
    AMTRL3_OM_HostRouteEntry_T  *host_entry_p)
{
    L_INET_AddrIp_T src_ip, nh_ip;
    UI32_T          nh_idx;
    BOOL_T          ret = FALSE;

    /* # route -n
     * Destination     Gateway         Genmask         Flags Metric Ref    Use Iface
     * 100.100.100.101 169.254.0.1     255.255.255.255 UGH   20     0        0 VLAN10
     * # arp -a
     * ? (169.254.0.1) at 08:00:27:ca:e8:e6 [ether] PERM on VLAN10
     *
     * ex: get the mac and ifidx info from 169.254.0.1 for 100.100.100.101.
     */
    if (IPAL_RESULT_OK == IPAL_ROUTE_RouteLookup(
        &host_entry_p->key_fields.dst_inet_addr, &src_ip, &nh_ip, &nh_idx))
    {
        if (IPAL_ROUTE_IS_BGP_UNNUMBERED_IPV4_LLA(nh_ip.addr))
        {
            IPAL_NeighborEntry_T    ipal_nbr;

            if (IPAL_RESULT_OK == IPAL_NEIGH_GetNeighbor(nh_idx, &nh_ip, &ipal_nbr))
            {
                AMTR_TYPE_AddrEntry_T   amtr_addr_entry;
                UI32_T                  tmp_vid;

                memset(&amtr_addr_entry, 0, sizeof(AMTR_TYPE_AddrEntry_T));
                memcpy(amtr_addr_entry.mac, ipal_nbr.phy_address, SYS_ADPT_MAC_ADDR_LEN);
                VLAN_OM_ConvertFromIfindex(nh_idx, &tmp_vid);
                amtr_addr_entry.vid = (UI16_T) tmp_vid;

                if (TRUE == AMTRL3_MGR_GetAMTRExactMAC(&amtr_addr_entry))
                {
                    host_entry_p->key_fields.lport = amtr_addr_entry.ifindex;
                    host_entry_p->key_fields.dst_vid_ifindex = nh_idx;
                    memcpy(host_entry_p->key_fields.dst_mac, ipal_nbr.phy_address, SYS_ADPT_MAC_ADDR_LEN);
                    if (amtrl3_mgr_debug_task & TASK_DEBUG_ARP_UNRESOLVED)
                    {
                        BACKDOOR_MGR_Printf("Get port MAC from AMTR: port=%d, MAC=%x-%x-%x-%x-%x-%x",
                            host_entry_p->key_fields.lport, L_INET_EXPAND_MAC(host_entry_p->key_fields.dst_mac));
                    }
                    ret = TRUE;
                }
            }
        }
    }

    return ret;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_RequestUnresolvedHostEntry
 * -------------------------------------------------------------------------
 * PURPOSE:  Send ARP request(ipv4) or Neighbor solicitation(ipv6)
 *           to resolve host entry.
 * INPUT:    action_flags  --  AMTRL3_TYPE_FLAGS_IPV4 \
 *                             AMTRL3_TYPE_FLAGS_IPV6
 *           fib_id     --  FIB id
 * OUTPUT:   none.
 * RETURN:   none.
 * NOTES:    If both flags are set, resolve ipv4 then ipv6 entries.
 * -------------------------------------------------------------------------*/
void AMTRL3_MGR_RequestUnresolvedHostEntry(UI32_T action_flags, UI32_T fib_id)
{
    UI32_T                      process_count, delta_time, event_type, flags = 0;
    UI32_T                      address_type, current_time = 0;
    AMTRL3_OM_HostRouteEntry_T  local_host_entry;
    UI8_T                       ip_str[L_INET_MAX_IPADDR_STR_LEN] = {0};
    BOOL_T                      continue_process = TRUE;

    AMTRL3_MGR_CHECK_OPERATION_MODE_WITHOUT_RETURN_VALUE();
    if(AMTRL3_MGR_GetFIBByID(fib_id) == NULL)
        return;
    if((amtrl3_mgr_arp_action == AMTRL3_MGR_DISABLE_ARP_UNRESOLVED) ||
        (amtrl3_mgr_arp_action == AMTRL3_MGR_DISALBE_ARP_ALL))
        return;

    memset(&local_host_entry, 0, sizeof(AMTRL3_OM_HostRouteEntry_T));

    process_count = delta_time = event_type = 0;

    current_time = SYSFUN_GetSysTick();
    while(continue_process)
    {
        continue_process = AMTRL3_OM_GetNextUnresolvedHostEntry(action_flags, fib_id, &local_host_entry);

        if((local_host_entry.entry_type == VAL_ipNetToPhysicalExtType_local) ||
           (local_host_entry.entry_type == VAL_ipNetToPhysicalExtType_vrrp))
            /*continue_process = FALSE;*/
            continue;

        if(continue_process)
        {
            /* need to use entry's fib_id, bcz om has only one fib.
             */
            fib_id = local_host_entry.key_fields.fib_id;

#if 0
            if(amtrl3_mgr_debug_tunnel)
                BACKDOOR_MGR_Printf("Unresolved get  [%lu](%u)%lx:%lx:%lx:%lx, type=%lu, status=%d->%d,ref=%lu, mac=%x-%x-%x-%x-%x-%x\r\n",
                                (unsigned long)local_host_entry.key_fields.dst_vid_ifindex,
                                local_host_entry.key_fields.dst_inet_addr.type,L_INET_EXPAND_IPV6(local_host_entry.key_fields.dst_inet_addr.addr),
                                (unsigned long)local_host_entry.entry_type,
                                local_host_entry.old_status,
                                local_host_entry.status,
                                (unsigned long)local_host_entry.ref_count,
                                L_INET_EXPAND_MAC(local_host_entry.key_fields.dst_mac)
                                );
#endif

#if (SYS_CPNT_IP_TUNNEL == TRUE)
            if(IS_TUNNEL_IFINDEX(local_host_entry.key_fields.dst_vid_ifindex))
            {
                switch(local_host_entry.key_fields.tunnel_entry_type)
                {
                    case AMTRL3_TYPE_INETTOPHYSICALEXTTYPE_TUNNEL_MANUAL:
                        event_type = HOST_ROUTE_READY_EVENT;
                        break;
                    case AMTRL3_TYPE_INETTOPHYSICALEXTTYPE_TUNNEL_6TO4:
                    case AMTRL3_TYPE_INETTOPHYSICALEXTTYPE_TUNNEL_ISATAP:
                    default:
                        event_type = HOST_ROUTE_UNREFERENCE_EVENT;
                        break;
                }
            }
            else
#endif /*SYS_CPNT_IP_TUNNEL*/
            {
                if(!local_host_entry.ref_count)
                {
                    if(local_host_entry.entry_type == VAL_ipNetToPhysicalExtType_static)
                    {
                        if (current_time >= local_host_entry.last_arp_timestamp)
                            delta_time = (current_time - local_host_entry.last_arp_timestamp) / SYS_BLD_TICKS_PER_SECOND;
                        else
                            delta_time = (0xFFFFFFFF - local_host_entry.last_arp_timestamp + current_time + 1) / SYS_BLD_TICKS_PER_SECOND;

                        if ( delta_time < static_host_retransmit_time[local_host_entry.arp_interval_index])
                        {
                            continue;
                        }

                        if ((local_host_entry.arp_interval_index+1) <AMTRL3_MGR_STATIC_ENTRY_DELAY_INTERVAL )
                        {
                            local_host_entry.arp_interval_index++;
                        }

                        local_host_entry.last_arp_timestamp = current_time;
                        event_type = HOST_ROUTE_PARTIAL_EVENT;
                    }
                    else
                        event_type = HOST_ROUTE_UNREFERENCE_EVENT;
                }
                else
                {
                    if (current_time >= local_host_entry.last_arp_timestamp)
                        delta_time = (current_time - local_host_entry.last_arp_timestamp) / SYS_BLD_TICKS_PER_SECOND;
                    else
                        delta_time = (0xFFFFFFFF - local_host_entry.last_arp_timestamp + current_time + 1) / SYS_BLD_TICKS_PER_SECOND;

                    if ( delta_time <  gateway_retransmit_time[local_host_entry.arp_interval_index])
                    {
                        continue;
                    }

                    if (local_host_entry.arp_interval_index < (AMTRL3_MGR_GATEWAY_ENTRY_DELAY_INTERVAL - 1))
                    {
                        local_host_entry.arp_interval_index++;
                    }

                    local_host_entry.last_arp_timestamp = current_time;

                    if (TRUE == AMTRL3_MGR_LocalResolveBgpUnnumberedIpv4Lla(&local_host_entry))
                        event_type = HOST_ROUTE_READY_EVENT;
                    else
                        event_type = HOST_ROUTE_PARTIAL_EVENT;
                }
            }

            address_type = local_host_entry.key_fields.dst_inet_addr.type;
            if((address_type == L_INET_ADDR_TYPE_IPV4) ||
               (address_type == L_INET_ADDR_TYPE_IPV4Z))
            {
                flags = AMTRL3_TYPE_FLAGS_IPV4;
            }
            else if((address_type == L_INET_ADDR_TYPE_IPV6) ||
                    (address_type == L_INET_ADDR_TYPE_IPV6Z))
            {
                flags = AMTRL3_TYPE_FLAGS_IPV6;
            }

            AMTRL3_MGR_HostRouteEventHandler(flags, fib_id, event_type, &local_host_entry);

            if(amtrl3_mgr_debug_task  & TASK_DEBUG_ARP_UNRESOLVED)
            {
                AMTRL3_MGR_Ntoa(&(local_host_entry.key_fields.dst_inet_addr), ip_str);
                BACKDOOR_MGR_Printf("AMTRL3 Sends ArpRqst on Unresolved IP: %s(%%%d) at %ld\n", ip_str,
                        local_host_entry.key_fields.dst_inet_addr.zoneid,
                        (long)local_host_entry.key_fields.dst_vid_ifindex);
            }

            process_count++;

#if (SYS_CPNT_IP_TUNNEL == TRUE)
            if(IS_TUNNEL_IFINDEX(local_host_entry.key_fields.dst_vid_ifindex))
            {
                AMTRL3_OM_HostRouteEntry_T nexthop;
                if(local_host_entry.key_fields.u.ip_tunnel.nexthop_vidifindex ==0)
                {
                    if(amtrl3_mgr_debug_tunnel)
                        BACKDOOR_MGR_Printf("fail, wrong tunnel ?\r\n");
                    break;
                }
                memset(&nexthop,0,sizeof(nexthop));
                nexthop.key_fields.dst_vid_ifindex = local_host_entry.key_fields.u.ip_tunnel.nexthop_vidifindex;
                nexthop.key_fields.dst_inet_addr =  local_host_entry.key_fields.u.ip_tunnel.nexthop_inet_addr;
                /* tunnel host route's nexthop is always ipv4 address , we should reset action flags to AMTRL3_TYPE_FLAGS_IPV4;*/
                flags = AMTRL3_TYPE_FLAGS_IPV4;
                AMTRL3_MGR_HostRouteEventHandler(flags, fib_id, HOST_ROUTE_PARTIAL_EVENT, &nexthop);
            }
            else
#endif /*SYS_CPNT_IP_TUNNEL*/
            {
                if(local_host_entry.entry_type == VAL_ipNetToPhysicalExtType_static)
                {
                    AMTR_TYPE_AddrEntry_T amtr_addr_entry;
                    UI32_T tmp_vid;

                    memset(&amtr_addr_entry, 0, sizeof(AMTR_TYPE_AddrEntry_T));
                    memcpy(amtr_addr_entry.mac,local_host_entry.key_fields.dst_mac,SYS_ADPT_MAC_ADDR_LEN);
                    VLAN_OM_ConvertFromIfindex(local_host_entry.key_fields.dst_vid_ifindex, &tmp_vid);
                    amtr_addr_entry.vid = (UI16_T)tmp_vid;
                    if(TRUE == AMTRL3_MGR_GetAMTRExactMAC(&amtr_addr_entry))
                    {
                        local_host_entry.key_fields.lport = amtr_addr_entry.ifindex;
                        if(amtrl3_mgr_debug_task  & TASK_DEBUG_ARP_UNRESOLVED)
                        {
                            BACKDOOR_MGR_Printf("Get port MAC from AMTR: port=%d, MAC=%x-%x-%x-%x-%x-%x",local_host_entry.key_fields.lport,L_INET_EXPAND_MAC(local_host_entry.key_fields.dst_mac));
                        }

                        AMTRL3_MGR_HostRouteEventHandler(flags, fib_id, HOST_ROUTE_READY_EVENT, &local_host_entry);
                    }
                    else
                        IPAL_NEIGH_SendNeighborRequest(local_host_entry.key_fields.dst_vid_ifindex,
                                &(local_host_entry.key_fields.dst_inet_addr));
                }
                else
                    IPAL_NEIGH_SendNeighborRequest(local_host_entry.key_fields.dst_vid_ifindex,
                            &(local_host_entry.key_fields.dst_inet_addr));
            }
        }

        if((process_count >= AMTRL3_MGR_NUMBER_OF_UNRESOLVED_ENTRY_TO_PROCESS) || (!continue_process))
            break;
    } /* end of while */

    return;

} /* end of AMTRL3_MGR_ArpRequestUnresolvedHostEntry() */

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_RequestGatewayEntry
 * -------------------------------------------------------------------------
 * PURPOSE:  Send ARP request(ipv4) or Neighbor solicitation(ipv6)
 *           for gateway entry to prevent it from aging out.
 * INPUT:    action_flags  --  AMTRL3_TYPE_FLAGS_IPV4 \
 *                             AMTRL3_TYPE_FLAGS_IPV6
 *           fib_id     --  FIB id
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTES:    If both flags are set, resolve ipv4 then ipv6 entries.
 * -------------------------------------------------------------------------*/
void AMTRL3_MGR_RequestGatewayEntry(UI32_T action_flags, UI32_T fib_id)
{
    UI32_T                      process_count = 0;
    AMTRL3_OM_HostRouteEntry_T  local_host_entry;
    UI8_T                       ip_str[L_INET_MAX_IPADDR_STR_LEN] = {0};
    BOOL_T                      continue_process = TRUE;

    AMTRL3_MGR_CHECK_OPERATION_MODE_WITHOUT_RETURN_VALUE();
    if(AMTRL3_MGR_GetFIBByID(fib_id) == NULL)
        return;
    memset(&local_host_entry, 0, sizeof(AMTRL3_OM_HostRouteEntry_T));

    if((amtrl3_mgr_arp_action == AMTRL3_MGR_DISABLE_ARP_GATEWAY) ||
       (amtrl3_mgr_arp_action == AMTRL3_MGR_DISALBE_ARP_ALL))
    {
        return;
    }

    if(amtrl3_mgr_debug_task & TASK_DEBUG_ARP_GATEWAY)
        BACKDOOR_MGR_Printf("AMTRL3_MGR_ArpRequestGatewayEntry\n");

    while(continue_process)
    {
        continue_process = AMTRL3_OM_GetNextGatewayEntry(action_flags, fib_id, &local_host_entry);

        if ((local_host_entry.entry_type == VAL_ipNetToPhysicalExtType_local) ||
            (local_host_entry.entry_type == VAL_ipNetToPhysicalExtType_vrrp))
            continue;

        if(continue_process)
        {
            /* need to use entry's fib_id, bcz om has only one fib.
             */
            fib_id = local_host_entry.key_fields.fib_id;

            if(amtrl3_mgr_debug_task & TASK_DEBUG_ARP_GATEWAY)
            {
                AMTRL3_MGR_Ntoa(&(local_host_entry.key_fields.dst_inet_addr), ip_str);
                BACKDOOR_MGR_Printf("AMTRL3 Sends ArpRqst on Gateway IP: %s:(%%%d)\n", ip_str,
                    local_host_entry.key_fields.dst_inet_addr.zoneid);
            } /* end of if */
#if (SYS_CPNT_IP_TUNNEL == TRUE)
            if(IS_TUNNEL_IFINDEX(local_host_entry.key_fields.dst_vid_ifindex))
            {
                IPAL_NEIGH_SendNeighborRequest(local_host_entry.key_fields.u.ip_tunnel.nexthop_vidifindex,
                           &(local_host_entry.key_fields.u.ip_tunnel.nexthop_inet_addr));
            }
            else
#endif
            IPAL_NEIGH_SendNeighborRequest(local_host_entry.key_fields.dst_vid_ifindex,
                            &(local_host_entry.key_fields.dst_inet_addr));
            process_count++; /*djd: ToDo: control process count in one time*/
        } /* end of if */
    } /* end of while */

    return;
}

/* -------------------------------------------------------------------------
 *  FUNCTION NAME: AMTRL3_MGR_CreateL3Interface
 * -------------------------------------------------------------------------
 * PURPOSE:  Add one MAC address that belonging to one VLAN.
 * INPUT:    fib_id:     FIB
 *           router_mac:     MAC address of vlan interface
 *           vid_ifindex:    VLAN ifindex of this MAC address
 * OUTPUT:   none.
 * RETURN:   TRUE/FALSE
 * NOTES:
 *      Layer3 interface should be create by IP_MGR, IP_MGR will call SWDRVL3
 *  directly.
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_MGR_CreateL3Interface(UI32_T fib_id, UI8_T *router_mac, UI32_T vid_ifIndex)
{
    /* Local Variable Declaration
     */
    UI32_T  vid = 0;
    BOOL_T  ret = TRUE;
    AMTRL3_OM_Interface_T vlan_intf;

    /* BODY
     */
    memset(&vlan_intf, 0, sizeof(AMTRL3_OM_Interface_T));

    if (NULL == router_mac)
        return FALSE;

    if(AMTRL3_MGR_GetFIBByID(fib_id) == NULL)
        return FALSE;

    VLAN_OM_ConvertFromIfindex(vid_ifIndex, &vid);


    if (ret == TRUE)
    {
        vlan_intf.vid = vid;
        memcpy(vlan_intf.route_mac, router_mac, SYS_ADPT_MAC_ADDR_LEN);

        ret = AMTRL3_OM_CreateInterface(fib_id, &vlan_intf);
    }

    return (ret);
}

/* -------------------------------------------------------------------------
 *  FUNCTION NAME: AMTRL3_MGR_DeleteL3Interface
 * -------------------------------------------------------------------------
 * PURPOSE: Remove one MAC address that belonging to one vlan interface.
 * INPUT:   fib_id:     FIB
 *          router_mac:     MAC address of vlan interface
 *          vid_ifIndex:    VLAN ifIndex of this MAC address
 * OUTPUT:  none
 * RETURN:  TRUE/FALSE
 * NOTES:
 *      Layer3 interface should be create by IP_MGR, IP_MGR will call SWDRVL3
 *  directly.
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_MGR_DeleteL3Interface(UI32_T fib_id, UI8_T *router_mac, UI32_T vid_ifIndex)
{
    /* Local Variable Declaration
     */
    UI32_T  vid = 0;
    BOOL_T  ret = FALSE;
    AMTRL3_OM_HostRouteEntry_T      host_route_entry;
    AMTRL3_OM_Interface_T vlan_intf;
    UI32_T      action_flags = 0;
    UI32_T      address_type;

    /* BODY
     */
    if(AMTRL3_MGR_GetFIBByID(fib_id) == NULL)
        return FALSE;

    memset(&vlan_intf, 0, sizeof(AMTRL3_OM_Interface_T));

    memset(&host_route_entry, 0, sizeof(AMTRL3_OM_HostRouteEntry_T));
    host_route_entry.key_fields.dst_vid_ifindex = vid_ifIndex;

    while(AMTRL3_OM_GetNextHostRouteEntryByVlanMac(AMTRL3_TYPE_FLAGS_IPV4 | AMTRL3_TYPE_FLAGS_IPV6, fib_id, &host_route_entry) == TRUE)
    {
        if(host_route_entry.key_fields.dst_vid_ifindex != vid_ifIndex)
            break;

        address_type = host_route_entry.key_fields.dst_inet_addr.type;
        if((address_type == L_INET_ADDR_TYPE_IPV4) ||
           (address_type == L_INET_ADDR_TYPE_IPV4Z))
        {
            action_flags = AMTRL3_TYPE_FLAGS_IPV4;
        }
        else if((address_type == L_INET_ADDR_TYPE_IPV6) ||
                (address_type == L_INET_ADDR_TYPE_IPV6Z))
        {
            action_flags = AMTRL3_TYPE_FLAGS_IPV6;
        }
        ret = AMTRL3_MGR_HostRouteEventHandler(action_flags, fib_id, HOST_ROUTE_REMOVE_EVENT, &host_route_entry);
        if (ret != TRUE)
        {
            return FALSE;
        }
    } /* end of while */

    VLAN_OM_ConvertFromIfindex(vid_ifIndex, &vid);

    vlan_intf.vid = vid;

    ret = AMTRL3_OM_DeleteInterface(fib_id, &vlan_intf);
    if (ret)
        ret = AMTR_PMGR_DeleteCpuMac(vid, router_mac);

    return (ret);
}

/* -------------------------------------------------------------------------
 *  FUNCTION NAME: AMTRL3_MGR_AddL3Mac
 * -------------------------------------------------------------------------
 * PURPOSE:  Add one MAC address with L3 bit On
 * INPUT:    fib_id:     FIB
 *           vlan_mac:       MAC address of vlan interface
 *           vid_ifindex:    VLAN ifindex of this MAC address
 * OUTPUT:   none.
 * RETURN:   TRUE/FALSE
 * NOTES:
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_MGR_AddL3Mac(UI8_T *l3_mac, UI32_T vid_ifIndex)
{
    /* Local Variable Declaration
     */
    UI32_T  vid = 0;

    /* BODY
     */
    if (NULL == l3_mac)
        return FALSE;

    VLAN_IFINDEX_CONVERTTO_VID(vid_ifIndex, vid);

    /* Set CPU Intervention */
    if (AMTR_TYPE_RET_SUCCESS != AMTR_PMGR_SetCpuMac(vid, l3_mac, TRUE))
        return FALSE;

    if (SWDRVL3_L3_NO_ERROR != SWDRVL3_AddL3Mac(l3_mac, vid))
        return FALSE;

    return TRUE;
}

/* -------------------------------------------------------------------------
 *  FUNCTION NAME: AMTRL3_MGR_DeleteL3Mac
 * -------------------------------------------------------------------------
 * PURPOSE: Remove one L3 MAC address that belonging to one vlan interface.
 * INPUT:
 *          vlan_mac:       MAC address of vlan interface
 *          vid_ifIndex:    VLAN ifIndex of this MAC address
 * OUTPUT:  none
 * RETURN:  TRUE/FALSE
 * NOTES:
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_MGR_DeleteL3Mac(UI8_T *l3_mac, UI32_T vid_ifIndex)
{
    /* Local Variable Declaration
     */
    UI32_T  vid = 0;
    BOOL_T  ret = FALSE;

    /* BODY
     */
    if (NULL == l3_mac)
        return FALSE;

    VLAN_IFINDEX_CONVERTTO_VID(vid_ifIndex, vid);
    ret = SWDRVL3_DeleteL3Mac(l3_mac, vid);
    /*notify MAC Learning to delete l3 mac */
    if (ret)
        ret = AMTR_PMGR_DeleteCpuMac(vid, l3_mac);

    return (ret);
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTRL3_MGR_MACTableDeleteByMstidOnPort
 *------------------------------------------------------------------------
 * FUNCTION: The function is used to delete port mac on this msit associated vlan
 * INPUT   : lport_ifindex - specific port removed from vlan member list
 *           mstid - specific spaning-tree msti index
 * OUTPUT  : None
 * RETURN:   TRUE / FALSE
 * NOTE    : none
 *------------------------------------------------------------------------*/
BOOL_T AMTRL3_MGR_MACTableDeleteByMstidOnPort(UI32_T mstid, UI32_T lport_ifindex)
{
    AMTRL3_OM_HostRouteEntry_T      host_route_entry;
    UI32_T      vid_ifindex = 0, action_flags = 0;
    UI32_T      address_type = 0;
    UI32_T      fib_id;
    XSTP_MGR_MstpInstanceEntry_T mstp_instance_entry;
    BOOL_T  is_exist=FALSE;
    AMTRL3_OM_Interface_T vlan_intf;

    AMTRL3_MGR_CHECK_OPERATION_MODE(FALSE);

    if(FALSE == XSTP_OM_GetMstpInstanceVlanMapped(mstid,&mstp_instance_entry))
        return FALSE;

    memset(&vlan_intf, 0, sizeof(AMTRL3_OM_Interface_T));
    for (fib_id = 0; fib_id < SYS_ADPT_MAX_NUMBER_OF_FIB; fib_id++)
    {
        /* Only iterate L3 interface */
        while(AMTRL3_OM_GetNextIpInterface(fib_id, &vlan_intf) ==
                    AMTRL3_TYPE_SUCCESS)
        {
            if ((0 < vlan_intf.vid) && (vlan_intf.vid < 1024))
            {
                if (mstp_instance_entry.mstp_instance_vlans_mapped[vlan_intf.vid>>3] &
                        (0x01<<(vlan_intf.vid%8)))
                {
                    is_exist = TRUE;
                }
            }
            else if ((1023 < vlan_intf.vid) && (vlan_intf.vid < 2048))
            {
                if (mstp_instance_entry.mstp_instance_vlans_mapped2k[(vlan_intf.vid>>3)-128] &
                        (0x01<<(vlan_intf.vid%8)))
                {
                    is_exist = TRUE;
                }
            }
            else if ((2047 < vlan_intf.vid) && (vlan_intf.vid < 3072))
            {
                if (mstp_instance_entry.mstp_instance_vlans_mapped3k[(vlan_intf.vid>>3)-256] &
                        (0x01<<(vlan_intf.vid%8)))
                {
                    is_exist = TRUE;
                }
            }
            else if ((3071 < vlan_intf.vid) && (vlan_intf.vid < 4096))
            {
                if (mstp_instance_entry.mstp_instance_vlans_mapped4k[(vlan_intf.vid>>3)-384] &
                        (0x01<<(vlan_intf.vid%8)))
                {
                    is_exist = TRUE;
                }
            }

            if(TRUE == is_exist)
            {
                is_exist=FALSE;
                if (amtrl3_mgr_callback_debug_flag & CALLBACK_DEBUG_PORT_N_VID)
                    BACKDOOR_MGR_Printf("AMTRL3_MGR_MACTableDeleteByVIDnPort_CallBack - port: %d, vid: %d\n",
                           (int)lport_ifindex, (int)vlan_intf.vid);

                VLAN_OM_ConvertToIfindex(vlan_intf.vid, &vid_ifindex);

                memset(&host_route_entry, 0, sizeof(AMTRL3_OM_HostRouteEntry_T));
                host_route_entry.key_fields.lport= lport_ifindex;

                while(AMTRL3_OM_GetNextHostRouteEntryByLport(
                        AMTRL3_TYPE_FLAGS_IPV4 | AMTRL3_TYPE_FLAGS_IPV6,
                        fib_id, &host_route_entry) == TRUE)
                {
                    if(host_route_entry.key_fields.lport != lport_ifindex)
                        break;
                    if(host_route_entry.key_fields.dst_vid_ifindex != vid_ifindex)
                        continue;

                    address_type = host_route_entry.key_fields.dst_inet_addr.type;
                    if((address_type == L_INET_ADDR_TYPE_IPV4) ||
                        (address_type == L_INET_ADDR_TYPE_IPV4Z))
                    {
                        action_flags = AMTRL3_TYPE_FLAGS_IPV4;
                    }
                    else if((address_type == L_INET_ADDR_TYPE_IPV6) ||
                        (address_type == L_INET_ADDR_TYPE_IPV6Z))
                    {
                        action_flags = AMTRL3_TYPE_FLAGS_IPV6;
                    }

                    AMTRL3_MGR_HostRouteEventHandler(action_flags, fib_id,
                                HOST_ROUTE_UNREFERENCE_EVENT, &host_route_entry);
                }
            }
        }
    }

    return TRUE;
} /* end of AMTRL3_MGR_MACTableDeleteByMstidOnPort() */

/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTRL3_MGR_SignalL3IfRifDestroy
 *------------------------------------------------------------------------
 * FUNCTION: The function is used to inform the destroy of rif on interface
 * INPUT   : ifindex   - the interface where rif is set
 *           ip_addr_p - the ip address of the rif
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------
 */
void AMTRL3_MGR_SignalL3IfRifDestroy(UI32_T ifindex, L_INET_AddrIp_T *ip_addr_p)
{
    AMTRL3_OM_HostRouteEntry_T  host_route_entry;
    UI32_T      action_flags = 0;
    UI32_T      address_type;
    UI32_T      fib_id = 0;

    if (!ip_addr_p)
        return;

    if (ip_addr_p->type == L_INET_ADDR_TYPE_IPV4 ||
        ip_addr_p->type == L_INET_ADDR_TYPE_IPV4Z)
        action_flags = AMTRL3_TYPE_FLAGS_IPV4;
    else if (ip_addr_p->type == L_INET_ADDR_TYPE_IPV6 ||
             ip_addr_p->type == L_INET_ADDR_TYPE_IPV6Z)
        action_flags = AMTRL3_TYPE_FLAGS_IPV6;
    else
        return;

    memset(&host_route_entry, 0, sizeof(AMTRL3_OM_HostRouteEntry_T));
    host_route_entry.key_fields.dst_vid_ifindex = ifindex;

    while(AMTRL3_OM_GetNextHostRouteEntryByVlanMac(action_flags, fib_id, &host_route_entry) == TRUE)
    {
        if(host_route_entry.key_fields.dst_vid_ifindex != ifindex)
            break;

        /* Remove all dynamic ARPs located inside the destroyed rif
         */
        if (IP_LIB_IsIpBelongToSubnet(ip_addr_p->addr, ip_addr_p->preflen, host_route_entry.key_fields.dst_inet_addr.addr))
        {
            if (!AMTRL3_MGR_HostRouteEventHandler(action_flags, fib_id, HOST_ROUTE_UNREFERENCE_EVENT, &host_route_entry))
            {
                return;
            }
        }
    }
}

#if 0
/*-------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_HotInsertProcessL3Interface
 * PURPOSE: Hot swap insertion for L3 interface
 * INPUT:   fib_id
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:
 *-------------------------------------------------------------------------*/
static void AMTRL3_MGR_HotInsertProcessL3Interface(UI32_T fib_id)
{
    AMTRL3_OM_Interface_T vlan_intf;

    memset(&vlan_intf, 0, sizeof(AMTRL3_OM_Interface_T));
    while(AMTRL3_OM_GetNextIpInterface(fib_id, &vlan_intf) ==
            AMTRL3_TYPE_SUCCESS)
    {
        SWDRVL3_HotInsertCreateL3Interface(fib_id, vlan_intf.vid, vlan_intf.route_mac);
    }

    return;
}
#endif

/*-------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_HotInsertProcessHostRoute
 * PURPOSE: Hot swap insertion for Host Route
 * INPUT:   fib_id
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:
 *-------------------------------------------------------------------------*/
static void AMTRL3_MGR_HotInsertProcessHostRoute(UI32_T fib_id)
{
    /* Local Variable Declaration
     */
    UI32_T      num_of_entry_retrieved = 0, process_count;
    AMTRL3_OM_HostRouteEntry_T  *host_entry = NULL;
    AMTRL3_MGR_FIB_T *mgr_fib = NULL;
    UI32_T action_flags = (AMTRL3_TYPE_FLAGS_IPV4 | AMTRL3_TYPE_FLAGS_IPV6);

    /* BODY
     */
    if((mgr_fib = AMTRL3_MGR_GetFIBByID(fib_id)) == NULL)
        return ;

    memset(mgr_fib->amtrl3_mgr_host_route_entry_block, 0, sizeof(AMTRL3_OM_HostRouteEntry_T)*AMTRL3_MGR_NUMBER_OF_HOST_ENTRY_TO_SCAN);

    // Get N HostEntry once from AL3_OM then to Option Module
    while(AMTRL3_OM_GetNextNHostRouteEntry(action_flags,
                                         fib_id,
                                         AMTRL3_MGR_NUMBER_OF_HOST_ENTRY_TO_SCAN,
                                         &num_of_entry_retrieved,
                                         mgr_fib->amtrl3_mgr_host_route_entry_block))
    {
        for (process_count = 0; process_count < num_of_entry_retrieved; process_count++)
        {
            host_entry = (AMTRL3_OM_HostRouteEntry_T *)(&mgr_fib->amtrl3_mgr_host_route_entry_block[process_count]);
            // check HostRouteEntry in MainBoard chip status
            if (NULL == host_entry)
                continue;

            if( host_entry->in_chip_status == TRUE )
            {

#if (SYS_CPNT_IP_TUNNEL == TRUE)
                if(IS_TUNNEL_IFINDEX(host_entry->key_fields.dst_vid_ifindex))
                {
                    if(AMTRL3_MGR_HotInsertAddTunnelHostRouteToModule(AMTRL3_TYPE_FLAGS_IPV6, fib_id, host_entry) != AMTRL3_MGR_ADD_HOST_OK)
                        if (amtrl3_mgr_debug_flag & DEBUG_LEVEL_ERROR)
                            BACKDOOR_MGR_Printf("AL3: Add tunnel Host Route To Chip Fail\n");
                }
                else
#endif
                if (host_entry->key_fields.dst_inet_addr.type == L_INET_ADDR_TYPE_IPV6 ||
                    host_entry->key_fields.dst_inet_addr.type == L_INET_ADDR_TYPE_IPV6Z)
                {
                    if( AMTRL3_MGR_HotInsertAddHostRouteToModule(AMTRL3_TYPE_FLAGS_IPV6, fib_id, host_entry) != AMTRL3_MGR_ADD_HOST_OK )
                        if (amtrl3_mgr_debug_flag & DEBUG_LEVEL_ERROR)
                            BACKDOOR_MGR_Printf("AL3: Add Host Route To Chip Fail\n");
                }
                else if (host_entry->key_fields.dst_inet_addr.type == L_INET_ADDR_TYPE_IPV4 ||
                    host_entry->key_fields.dst_inet_addr.type == L_INET_ADDR_TYPE_IPV4Z)
                {
                    if( AMTRL3_MGR_HotInsertAddHostRouteToModule(AMTRL3_TYPE_FLAGS_IPV4, fib_id, host_entry) != AMTRL3_MGR_ADD_HOST_OK )
                        if (amtrl3_mgr_debug_flag & DEBUG_LEVEL_ERROR)
                            BACKDOOR_MGR_Printf("AL3: Add Host Route To Chip Fail\n");
                }
            }
        }
        // set Next N Entry search key
        if (num_of_entry_retrieved == AMTRL3_MGR_NUMBER_OF_HOST_ENTRY_TO_SCAN)
        {
            if (num_of_entry_retrieved != 1)
            {
                memcpy(&(mgr_fib->amtrl3_mgr_host_route_entry_block[0].key_fields.dst_inet_addr),
                       &(mgr_fib->amtrl3_mgr_host_route_entry_block[num_of_entry_retrieved-1].key_fields.dst_inet_addr),
                       sizeof(L_INET_Addr_T));
            }
        }
        else // num_of_entry_retrieved < AMTRL3_MGR_NUMBER_OF_HOST_ENTRY_TO_SCAN, searching completed.
            break;
    }

    return;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_HotInsertProcessNetRoute
 * PURPOSE: Hot swap insertion for Net Route
 * INPUT:   fib_id
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:
 *-------------------------------------------------------------------------*/
static void AMTRL3_MGR_HotInsertProcessNetRoute(UI32_T fib_id)
{
    UI32_T action_flags = (AMTRL3_TYPE_FLAGS_IPV4 | AMTRL3_TYPE_FLAGS_IPV6);
    UI32_T local_action_flags = 0;
    AMTRL3_OM_NetRouteEntry_T   net_entry;
    AMTRL3_OM_NetRouteEntry_T   net_entry_block[AMTRL3_MGR_MAXIMUM_ECMP_PATH];
    AMTRL3_OM_NetRouteEntry_T   active_net_entry_block[AMTRL3_MGR_MAXIMUM_ECMP_PATH];
    AMTRL3_OM_HostRouteEntry_T  nh_entry;
    AMTRL3_OM_HostRouteEntry_T  local_host_entry;
    AMTRL3_OM_HostRouteEntry_T  active_nh_entry_block[AMTRL3_MGR_MAXIMUM_ECMP_PATH];
    UI8_T   ip_str[L_INET_MAX_IPADDR_STR_LEN] = {0};
    BOOL_T  ret;
    UI32_T  num_of_entry, active_count, i;

    memset(&net_entry, 0, sizeof(AMTRL3_OM_NetRouteEntry_T));

    /* In AMTRL3_OM_GetNextNetRouteEntry(), when action_flags = IPv4 + IPv6 and
     * net_entry's dest ip address is ipv4, it will search in IPv4, then IPv6.
     */
    net_entry.inet_cidr_route_entry.inet_cidr_route_dest.type = L_INET_ADDR_TYPE_IPV4;
    while ( AMTRL3_OM_GetNextNetRouteEntry(action_flags, fib_id, &net_entry))
    {
        if (amtrl3_mgr_debug_flag & DEBUG_LEVEL_HOTINSRT)
        {
            AMTRL3_MGR_Ntoa(&(net_entry.inet_cidr_route_entry.inet_cidr_route_dest), ip_str);
            BACKDOOR_MGR_Printf("%s: dest:%s net_route_status:%d\n", __FUNCTION__, ip_str, net_entry.net_route_status);
        }

        if( net_entry.net_route_status == AMTRL3_OM_NET_ROUTE_READY)
        {
            // Then Add NetRouteEntry to Module
            memset(&nh_entry, 0, sizeof (AMTRL3_OM_HostRouteEntry_T));
            nh_entry.key_fields.dst_vid_ifindex = net_entry.inet_cidr_route_entry.inet_cidr_route_if_index;
            nh_entry.key_fields.dst_inet_addr = net_entry.inet_cidr_route_entry.inet_cidr_route_next_hop;
            if (net_entry.inet_cidr_route_entry.inet_cidr_route_dest.type == L_INET_ADDR_TYPE_IPV6 ||
                net_entry.inet_cidr_route_entry.inet_cidr_route_dest.type == L_INET_ADDR_TYPE_IPV6Z)
                local_action_flags = AMTRL3_TYPE_FLAGS_IPV6;
            else if (net_entry.inet_cidr_route_entry.inet_cidr_route_dest.type == L_INET_ADDR_TYPE_IPV4 ||
                net_entry.inet_cidr_route_entry.inet_cidr_route_dest.type == L_INET_ADDR_TYPE_IPV4Z)
                local_action_flags = AMTRL3_TYPE_FLAGS_IPV4;
            else
                continue;

            /* Connected Route */
            if (net_entry.inet_cidr_route_entry.inet_cidr_route_proto == VAL_ipCidrRouteProto_local)
            {
                if(AMTRL3_MGR_HotInsertAddNetRouteToModule(local_action_flags, fib_id, &net_entry, NULL) == FALSE)
                {
                    if (amtrl3_mgr_debug_flag & DEBUG_LEVEL_ERROR)
                    {
                        AMTRL3_MGR_Ntoa(&(net_entry.inet_cidr_route_entry.inet_cidr_route_dest), ip_str);
                        BACKDOOR_MGR_Printf("%s: Add Connected Route (%s) To Chip of Option Module Fail\n",
                                __FUNCTION__, ip_str);
                    }
                }
            }
            else if (net_entry.flags & AMTRL3_TYPE_FLAGS_ECMP) /* ECMP route */
            {
                memset(net_entry_block, 0, sizeof(AMTRL3_OM_NetRouteEntry_T) * AMTRL3_MGR_MAXIMUM_ECMP_PATH);
                net_entry_block[0].inet_cidr_route_entry.inet_cidr_route_dest = net_entry.inet_cidr_route_entry.inet_cidr_route_dest;
                net_entry_block[0].inet_cidr_route_entry.inet_cidr_route_pfxlen = net_entry.inet_cidr_route_entry.inet_cidr_route_pfxlen;
                net_entry_block[0].inet_cidr_route_entry.inet_cidr_route_dest.preflen = net_entry_block[0].inet_cidr_route_entry.inet_cidr_route_pfxlen;
                L_INET_ApplyMask(&net_entry_block[0].inet_cidr_route_entry.inet_cidr_route_dest);
                ret = AMTRL3_OM_GetNextNNetRouteEntryByDstIp(local_action_flags,
                                                             fib_id,
                                                             AMTRL3_MGR_MAXIMUM_ECMP_PATH,
                                                             &num_of_entry,
                                                             net_entry_block);

                if (ret == FALSE || num_of_entry == 0)
                {
                    continue;
                }

                /* Add ECMP only once at the first ECMP's entry occurence
                 */
                if (memcmp(&(net_entry.inet_cidr_route_entry.inet_cidr_route_next_hop),
                           &(net_entry_block[0].inet_cidr_route_entry.inet_cidr_route_next_hop),
                           sizeof(L_INET_AddrIp_T)) != 0)
                {
                    continue;
                }

                active_count = 0;
                for(i = 0; i < num_of_entry; i++)
                {
                    memset(&local_host_entry, 0, sizeof(AMTRL3_OM_HostRouteEntry_T));
                    local_host_entry.key_fields.dst_inet_addr = net_entry_block[i].inet_cidr_route_entry.inet_cidr_route_next_hop;
                    local_host_entry.key_fields.dst_vid_ifindex = net_entry_block[i].inet_cidr_route_entry.inet_cidr_route_if_index;

                    if(AMTRL3_OM_GetHostRouteEntry(local_action_flags, fib_id, &local_host_entry) == FALSE)
                        continue;

                    if(local_host_entry.in_chip_status)
                    {
                        memcpy(&active_net_entry_block[active_count], &net_entry_block[i], sizeof(AMTRL3_OM_NetRouteEntry_T));
                        memcpy(&active_nh_entry_block[active_count], &local_host_entry, sizeof(AMTRL3_OM_HostRouteEntry_T));
                        active_count++;
                    }
                }

                if (AMTRL3_MGR_HotInsertAddECMPRouteMultiPathToModule(local_action_flags, fib_id,
                                                                    active_net_entry_block,
                                                                    active_nh_entry_block,
                                                                    active_count) == FALSE)
                {
                    if (amtrl3_mgr_debug_flag & DEBUG_LEVEL_ERROR)
                    {
                        AMTRL3_MGR_Ntoa(&(net_entry.inet_cidr_route_entry.inet_cidr_route_dest), ip_str);
                            BACKDOOR_MGR_Printf("%s: Add ICMP Net Route (%s) To Chip of Option Module Fail\n",
                                __FUNCTION__, ip_str);
                    }
                }
            }
            /* Normal Route */
            else if(AMTRL3_OM_GetHostRouteEntry(local_action_flags, fib_id, &nh_entry))
            {
                if(AMTRL3_MGR_HotInsertAddNetRouteToModule(local_action_flags, fib_id, &net_entry, &nh_entry) == FALSE)
                {
                    if (amtrl3_mgr_debug_flag & DEBUG_LEVEL_ERROR)
                    {
                        AMTRL3_MGR_Ntoa(&(net_entry.inet_cidr_route_entry.inet_cidr_route_dest), ip_str);
                            BACKDOOR_MGR_Printf("%s: Add Net Route (%s) To Chip of Option Module Fail\n",
                                __FUNCTION__, ip_str);
                    }
                }
            }
        }
    }
}


/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTRL3_MGR_HotRemovalProcessHostRoute
 *------------------------------------------------------------------------
 * FUNCTION: The function will delete the host route
 *           entry associated with a designated port.
 * INPUT   : lport_ifindex - specific port removed from vlan member list
 *           fib_id
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : none
 *------------------------------------------------------------------------*/
static void AMTRL3_MGR_HotRemovalProcessHostRoute(UI32_T fib_id, UI32_T lport)
{
    /* Local Variable Declaration
     */
    AMTRL3_OM_HostRouteEntry_T      host_route_entry;
    UI32_T      action_flags = 0;
    UI32_T      address_type;
    UI8_T       ip_str[L_INET_MAX_IPADDR_STR_LEN] = {0};

    /* BODY
     */
    if (amtrl3_mgr_debug_flag & DEBUG_LEVEL_HOTINSRT)
    {
        BACKDOOR_MGR_Printf("%s: fib_id:%ld lport:%ld\n", __FUNCTION__, (long)fib_id, (long)lport);
    }

    memset(&host_route_entry, 0, sizeof(AMTRL3_OM_HostRouteEntry_T));
    host_route_entry.key_fields.lport = lport;

    while(AMTRL3_OM_GetNextHostRouteEntryByLport((AMTRL3_TYPE_FLAGS_IPV4 | AMTRL3_TYPE_FLAGS_IPV6),
                            fib_id, &host_route_entry) == TRUE)
    {

#if (SYS_CPNT_IP_TUNNEL == TRUE)
        /* we don't remove static tunnel's host route, but remove dynamic host route */
        if(IS_TUNNEL_IFINDEX(host_route_entry.key_fields.dst_vid_ifindex)&&
           (host_route_entry.key_fields.tunnel_entry_type == AMTRL3_TYPE_INETTOPHYSICALEXTTYPE_TUNNEL_MANUAL))
            continue;

#endif

        if(host_route_entry.key_fields.lport != lport)
            break;

        address_type = host_route_entry.key_fields.dst_inet_addr.type;
        if((address_type == L_INET_ADDR_TYPE_IPV4) ||
           (address_type == L_INET_ADDR_TYPE_IPV4Z))
        {
            action_flags = AMTRL3_TYPE_FLAGS_IPV4;
        }
        else if((address_type == L_INET_ADDR_TYPE_IPV6) ||
                (address_type == L_INET_ADDR_TYPE_IPV6Z))
        {
            action_flags = AMTRL3_TYPE_FLAGS_IPV6;
        }

        if (amtrl3_mgr_debug_flag & DEBUG_LEVEL_TRACE_HOST)
        {
            AMTRL3_MGR_Ntoa(&(host_route_entry.key_fields.dst_inet_addr), ip_str);
            BACKDOOR_MGR_Printf("%s: Processing %s (ref_count:%ld)\n", __FUNCTION__, ip_str, (long)host_route_entry.ref_count);
        }

        /* For gateway host entry */
        if (host_route_entry.ref_count > 0)
        {
            /* First, delete NetRoute associated with the host route entry */
            AMTRL3_MGR_HotRemoveProcessNetRoute(fib_id, action_flags, &host_route_entry);

            AMTRL3_MGR_HostRouteEventHandler(action_flags, fib_id, HOST_ROUTE_UNREFERENCE_EVENT, &host_route_entry);
        }

        /* For normal host entry */
        if (host_route_entry.ref_count == 0)
        {
            if(host_route_entry.entry_type == VAL_ipNetToPhysicalExtType_static)
                AMTRL3_MGR_HostRouteEventHandler(action_flags, fib_id, HOST_ROUTE_UNREFERENCE_EVENT, &host_route_entry);
            else
                AMTRL3_MGR_HostRouteEventHandler(action_flags, fib_id, HOST_ROUTE_REMOVE_EVENT, &host_route_entry);
        }
    }

    return;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTRL3_MGR_HotRemoveProcessNetRoute
 *------------------------------------------------------------------------
 * FUNCTION: The function will delete the net route
 *           entry associated with a designated host route.
 * INPUT   : action_flags
 *           fib_id
 *           nh_entry -- designated host entry
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : none
 *------------------------------------------------------------------------*/
static void AMTRL3_MGR_HotRemoveProcessNetRoute(UI32_T fib_id, UI32_T action_flags,
                            AMTRL3_OM_HostRouteEntry_T *nh_entry)
{
    AMTRL3_OM_NetRouteEntry_T   *net_entry;
    AMTRL3_MGR_FIB_T *mgr_fib = NULL;
    UI32_T      num_of_entry = 0;
    UI32_T      process_count = 0;
    UI8_T       net_str[L_INET_MAX_IPADDR_STR_LEN] = {0};
    UI8_T       ip_str[L_INET_MAX_IPADDR_STR_LEN] = {0};
    UI8_T       gateway_str[L_INET_MAX_IPADDR_STR_LEN] = {0};

    /* BODY
     */
    if((mgr_fib = AMTRL3_MGR_GetFIBByID(fib_id)) == NULL)
        return ;

    memset(mgr_fib->amtrl3_mgr_net_route_entry_block, 0, sizeof(AMTRL3_OM_NetRouteEntry_T)*AMTRL3_MGR_NUMBER_OF_NET_ENTRY_TO_SCAN);
    net_entry = (AMTRL3_OM_NetRouteEntry_T*)(mgr_fib->amtrl3_mgr_net_route_entry_block);
    net_entry->inet_cidr_route_entry.inet_cidr_route_next_hop = nh_entry->key_fields.dst_inet_addr;

    while (AMTRL3_OM_GetNextNNetRouteEntryByNextHop(action_flags, fib_id,
                                            AMTRL3_MGR_NUMBER_OF_NET_ENTRY_TO_SCAN,
                                            &num_of_entry,
                                            mgr_fib->amtrl3_mgr_net_route_entry_block))
    {
        /* Process each entry individually
         */
        for(process_count = 0; process_count < num_of_entry; process_count++)
        {
            net_entry = (AMTRL3_OM_NetRouteEntry_T *)(&(mgr_fib->amtrl3_mgr_net_route_entry_block[process_count]));

            if (amtrl3_mgr_debug_flag & DEBUG_LEVEL_TRACE_NET)
            {
                AMTRL3_MGR_Ntoa(&(net_entry->inet_cidr_route_entry.inet_cidr_route_dest), net_str);
                AMTRL3_MGR_Ntoa(&(net_entry->inet_cidr_route_entry.inet_cidr_route_next_hop), gateway_str);
                AMTRL3_MGR_Ntoa(&(nh_entry->key_fields.dst_inet_addr), ip_str);
                BACKDOOR_MGR_Printf("%s: Processing %s/%ld via %s [%s]\n", __FUNCTION__, net_str,
                                (long)net_entry->inet_cidr_route_entry.inet_cidr_route_pfxlen,
                                gateway_str, ip_str);
            }

            /* If net entry is associated with different gateway, do not process it
             */
            if(AMTRL3_OM_IsAddressEqual(&(net_entry->inet_cidr_route_entry.inet_cidr_route_next_hop),
                                         &(nh_entry->key_fields.dst_inet_addr)) == FALSE)
            {
                break;
            }

            if(CHECK_FLAG(net_entry->flags, AMTRL3_TYPE_FLAGS_ECMP))
                SET_FLAG(action_flags, AMTRL3_TYPE_FLAGS_ECMP);

            /* Only remove net entry from chip if it exist in it
             */
            if(net_entry->net_route_status == AMTRL3_OM_NET_ROUTE_READY)
            {
                if (AMTRL3_MGR_DeleteNetRouteFromChip(action_flags, fib_id, net_entry, nh_entry) == TRUE)
                {
                    net_entry->net_route_status = AMTRL3_OM_NET_ROUTE_UNRESOLVED;
                    AMTRL3_OM_SetNetRouteEntry(action_flags, fib_id, net_entry);
                }
            }
        }

        // set Next N Entry search key
        if (num_of_entry == AMTRL3_MGR_NUMBER_OF_NET_ENTRY_TO_SCAN)
        {
            if (num_of_entry != 1)
            {
                memcpy(&mgr_fib->amtrl3_mgr_net_route_entry_block[0],
                       &mgr_fib->amtrl3_mgr_net_route_entry_block[num_of_entry-1],
                       sizeof(AMTRL3_OM_NetRouteEntry_T));
            }
        }
        else // num_of_entry_retrieved < AMTRL3_MGR_NUMBER_OF_HOST_ENTRY_TO_SCAN, searching completed.
            break;
    }

    return;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTRL3_MGR_UpdateHostRouteRefCountInChip
 *------------------------------------------------------------------------
 * FUNCTION: This function will offset to host_entry_p->ref_count_in_chip
 * INPUT   : action_flags
 *           fib_id
 *           host_entry_p -- designated host entry
 *           offset       -- update reference count offset
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : The ref_count_in_chip indicated the number of references by
 *           net route entry in chipset. When it reachs zero, means no
 *           net route refer to it. For IPv6 link-local host route, we
 *           write only the l3 egress object to chip but no l3 host route
 *           refer to it, and some chipset (Broadcom's firebolt etc) will
 *           delete this kind of l3 egress object (ref count reachs 0)
 *           automatically. When this occur, we send a HOST_ROUTE_UNREFERENCE_EVENT
 *           to target host route, trigger it to resolve and re-write the
 *           l3 egress object again.
 *------------------------------------------------------------------------
 */
static void AMTRL3_MGR_UpdateHostRouteRefCountInChip(UI32_T action_flags, UI32_T fib_id,
                            AMTRL3_OM_HostRouteEntry_T *host_entry_p, I32_T offset)
{
    AMTRL3_OM_HostRouteEntry_T local_entry;
    BOOL_T hw_info_chg=FALSE;
    BOOL_T ref_cnt_chg=FALSE;

    memset(&local_entry, 0, sizeof(local_entry));
    memcpy(&local_entry, host_entry_p, sizeof(local_entry));

    /* Make sure that we modify only the ref_count_in_chip field
     */
    if (AMTRL3_OM_GetHostRouteEntry(action_flags, fib_id, &local_entry))
    {
        local_entry.ref_count_in_chip = host_entry_p->ref_count_in_chip;
    }

    /* use local entry to update reference count */
    if ((((I32_T)local_entry.ref_count_in_chip)+offset) < 0)
    {
        local_entry.ref_count_in_chip = 0;
        ref_cnt_chg = TRUE;
    }
    else
    {
        local_entry.ref_count_in_chip += offset;
        ref_cnt_chg = TRUE;
    }
    /* For IPv6 link-local host route, we write L3 egress object only
     * We must clear the OM's hw_info since the chipset will delete the
     * L3 egress object automatically when ref count reachs zero
     */
    /*
    if (local_entry.key_fields.dst_inet_addr.type == L_INET_ADDR_TYPE_IPV6Z &&
        local_entry.ref_count_in_chip == 0)
    {
        local_entry.hw_info = L_CVRT_UINT_TO_PTR(AMTRL3_OM_HW_INFO_INVALID);
        hw_info_chg = TRUE;
    }
    */

    if(TRUE == AMTRL3_OM_SetHostRouteEntry(action_flags, fib_id, &local_entry))
    {
        /* if field is changed, we need to update to input argument */
        if(hw_info_chg)
            host_entry_p->hw_info = local_entry.hw_info;

        if(ref_cnt_chg)
            host_entry_p->ref_count_in_chip = local_entry.ref_count_in_chip;
    }
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_HandleHotInsertion
 * PURPOSE: Hot swap init function for insertion
 * INPUT:   starting_port_ifindex
 *          number_of_port
 *          use_default
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:
 *-------------------------------------------------------------------------*/
void AMTRL3_MGR_HandleHotInsertion(UI32_T starting_port_ifindex,
                                   UI32_T number_of_port,
                                   BOOL_T use_default)
{
    UI32_T fib_id = AMTRL3_OM_GET_FIRST_FIB_ID;

    AMTRL3_MGR_CHECK_OPERATION_MODE_WITHOUT_RETURN_VALUE();

    if (amtrl3_mgr_debug_flag & DEBUG_LEVEL_HOTINSRT)
    {
        BACKDOOR_MGR_Printf("AL3: Handle Hot Insertion....\n");
        BACKDOOR_MGR_Printf("    Insert starting port ifindex: %lu\n", (unsigned long)starting_port_ifindex);
        BACKDOOR_MGR_Printf("    Insert port number: %lu\n", (unsigned long)number_of_port);
    }

    while (AMTRL3_MGR_GetNextFIBID(&fib_id) == TRUE)
    {
        /* AMTRL3_MGR_HotInsertProcessL3Interface(fib_id); */
        AMTRL3_MGR_HotInsertProcessHostRoute(fib_id);
        AMTRL3_MGR_HotInsertProcessNetRoute(fib_id);
    }

    return;
} /* end of AMTRL3_MGR_HandleHotInsertion() */

/*-------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_HandleHotRemoval
 * PURPOSE: Hot swap init function for removal
 * INPUT:   starting_port_ifindex
 *          number_of_port
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:
 *------------------------------------------------------------------------*/
void AMTRL3_MGR_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port)
{
    UI32_T fib_id = AMTRL3_OM_GET_FIRST_FIB_ID;
    UI32_T lport_ifindex = starting_port_ifindex;
    int i;

    AMTRL3_MGR_CHECK_OPERATION_MODE_WITHOUT_RETURN_VALUE();

    if (amtrl3_mgr_debug_flag & DEBUG_LEVEL_HOTINSRT)
    {
        BACKDOOR_MGR_Printf("AL3: Handle Hot Remove....\n");
        BACKDOOR_MGR_Printf("    Remove starting port ifindex: %lu\n", (unsigned long)starting_port_ifindex);
        BACKDOOR_MGR_Printf("    Remove port number: %lu\n", (unsigned long)number_of_port);
    }

    /* Vai: In fact, it's the same operation as "port link down" or
     * "mac of the port age-out".
     * For Trunk port, L2 module will give the related callback.
     */
    while (AMTRL3_MGR_GetNextFIBID(&fib_id) == TRUE)
    {
        for (i = 0; i < number_of_port; i++)
        {
            AMTRL3_MGR_HotRemovalProcessHostRoute(fib_id, lport_ifindex);
            lport_ifindex++;
        }
    }

    return;
} /* end of AMTRL3_MGR_HandleHotRemoval() */

/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTRL3_MGR_ForwardingUPortAddToTrunk_CallBack
 *------------------------------------------------------------------------
 * FUNCTION: This is the callback function for add trunk members.
 *
 * INPUT   : fib_id      - FIB id
 *           trunk_ifindex  - specify which trunk becomes oper up.
 *           member_ifindex - specify which member of trunk oper up.
 *
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    :
 *------------------------------------------------------------------------*/
void AMTRL3_MGR_ForwardingUPortAddToTrunk_CallBack(UI32_T trunk_ifindex,UI32_T member_ifindex)
{
    AMTRL3_OM_HostRouteEntry_T  host_entry;
    UI32_T                      fib_id = 0;

    AMTRL3_MGR_CHECK_OPERATION_MODE_WITHOUT_RETURN_VALUE();

    if (amtrl3_mgr_callback_debug_flag & CALLBACK_DEBUG_TRUNK)
        BACKDOOR_MGR_Printf("AL3_ForwardingUPortAddToTrunk_Callback: trunk_ifindex %d, member_ifindex %d\n",
               (int)trunk_ifindex, (int)member_ifindex);

    for(fib_id = 0; fib_id < SYS_ADPT_MAX_NUMBER_OF_FIB; fib_id++)
    {
        if(AMTRL3_MGR_GetFIBByID(fib_id) == NULL)
            continue;

        memset(&host_entry, 0, sizeof(AMTRL3_OM_HostRouteEntry_T));
        host_entry.key_fields.lport = member_ifindex;

        while(AMTRL3_OM_GetNextHostRouteEntryByLport(AMTRL3_TYPE_FLAGS_IPV4 | AMTRL3_TYPE_FLAGS_IPV6, fib_id, &host_entry))
        {
            if (host_entry.key_fields.lport != member_ifindex)
                break;

            if ((host_entry.status != HOST_ROUTE_READY_NOT_SYNC) &&
                (host_entry.status != HOST_ROUTE_HOST_READY)     &&
                (host_entry.status != HOST_ROUTE_GATEWAY_READY))
                continue;

            /* Event_type of host route entry learned on member_ifindex will be different
             * due to chip limitation.  It is more comprehensible to seperate different
             *  behaviors into two different functions.
             */
            if(CHECK_FLAG(amtrl3_chip_capability_flags, AMTRL3_CHIP_SUPPORT_L3_TRUNK_LOAD_BALANCE))
                AMTRL3_MGR_TrunkMemberAddForChipWithLoadBalance(fib_id, trunk_ifindex, member_ifindex, &host_entry);
            else
                AMTRL3_MGR_TrunkMemberAddForChipWithoutLoadBalance(fib_id, trunk_ifindex, member_ifindex, &host_entry);
        }
    }
    return;
} /* end of AMTRL3_MGR_ForwardingUPortAddToTrunk_CallBack() */

/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTRL3_MGR_ForwardingTrunkMemberDelete_CallBack
 *------------------------------------------------------------------------
 * FUNCTION: This is the callback function for delete trunk member.
 *
 * INPUT   : fib_id      - FIB id
 *           trunk_ifindex  - specify which trunk becomes oper up.
 *           member_ifindex - specify which member of trunk oper up.
 *
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : use fib_id as one of parameter, or check all FIB
 *           to process all related entries in all FIBs.
 *           current just use fib_id.
 *------------------------------------------------------------------------*/
void AMTRL3_MGR_ForwardingTrunkMemberDelete_CallBack(UI32_T trunk_ifindex, UI32_T member_ifindex)
{
    UI32_T fib_id;;

    AMTRL3_MGR_CHECK_OPERATION_MODE_WITHOUT_RETURN_VALUE();

    if (amtrl3_mgr_callback_debug_flag & CALLBACK_DEBUG_TRUNK)
        BACKDOOR_MGR_Printf("AL3_ForwardingTrunkMemberDelete_CallBack: trunk_ifindex %d, member_ifindex %d\n",
               (int)trunk_ifindex, (int)member_ifindex);

    for(fib_id = 0; fib_id < SYS_ADPT_MAX_NUMBER_OF_FIB; fib_id++)
    {
        AMTRL3_MGR_LoadBalanceHostRouteForTrunkMember(fib_id, trunk_ifindex, member_ifindex);
    }

    return;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTRL3_MGR_ForwardingTrunkMemberToNonForwarding_CallBack
 *------------------------------------------------------------------------
 * FUNCTION: This function is used when a trunk port not oper up.
 *
 * INPUT   :
 *
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : When a port is deleted by some trunk , we have to re-install
 *           netroute table and host route into chip. It makes lower
 *           layer to handle trunk issue.
 *------------------------------------------------------------------------*/
void AMTRL3_MGR_ForwardingTrunkMemberToNonForwarding_CallBack(UI32_T trunk_ifindex, UI32_T member_ifindex)
{
    UI32_T     fib_id;

    AMTRL3_MGR_CHECK_OPERATION_MODE_WITHOUT_RETURN_VALUE();

    if (amtrl3_mgr_callback_debug_flag & CALLBACK_DEBUG_TRUNK)
        BACKDOOR_MGR_Printf("AL3_ForwardingTrunkMemberToNonForwarding_CallBack: trunk_ifindex %d, member_ifindex %d\n",
               (int)trunk_ifindex, (int)member_ifindex);

    for(fib_id = 0; fib_id < SYS_ADPT_MAX_NUMBER_OF_FIB; fib_id++)
    {
        AMTRL3_MGR_LoadBalanceHostRouteForTrunkMember(fib_id, trunk_ifindex, member_ifindex);
    }

    return;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTRL3_MGR_PortMove_CallBack
 *------------------------------------------------------------------------
 * FUNCTION: This function is called when a MAC port move
 * INPUT   : ifindex - the port where mac address move to
 *           vid     - vlan id
 *           mac     - mac address
 *           original_ifindex  - the port where mac address original learned
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void AMTRL3_MGR_PortMove_CallBack(UI32_T num_of_entry, AMTR_TYPE_PortMoveEntry_T *entry_p)
{
    UI32_T      action_flags = 0;
    UI8_T       ret = FALSE;
    AMTRL3_OM_HostRouteEntry_T  host_route_entry;
    UI8_T       ip_str[L_INET_MAX_IPADDR_STR_LEN] = {0};
    UI32_T      address_type;
    UI32_T      fib_id;
    UI32_T      vid_ifindex = 0;
    UI32_T      i;

    AMTRL3_MGR_CHECK_OPERATION_MODE_WITHOUT_RETURN_VALUE();

    for(fib_id = 0; fib_id < SYS_ADPT_MAX_NUMBER_OF_FIB; fib_id++)
    {
        if(AMTRL3_MGR_GetFIBByID(fib_id) == NULL)
            continue;

        for(i=0; i<num_of_entry; i++)
        {
            if(amtrl3_mgr_callback_debug_flag & CALLBACK_DEBUG_PORT_MOVE)
            {
                BACKDOOR_MGR_Printf("AMTRL3_MGR_PortMove_CallBack: lport %lu, vid %lu, mac %02x-%02x-%02x-%02x-%02x-%02x, ori port %lu\n",
                    (unsigned long)entry_p[i].event_entry.ifindex, (unsigned long)entry_p[i].event_entry.vid, entry_p[i].event_entry.mac[0], entry_p[i].event_entry.mac[1],
                    entry_p[i].event_entry.mac[2], entry_p[i].event_entry.mac[3], entry_p[i].event_entry.mac[4], entry_p[i].event_entry.mac[5],
                    (unsigned long)entry_p[i].original_port);
            }

            memset(&host_route_entry, 0, sizeof(AMTRL3_OM_HostRouteEntry_T));

            if(entry_p[i].event_entry.vid > SYS_ADPT_MAX_VLAN_ID)
                continue;

            VLAN_OM_ConvertToIfindex(entry_p[i].event_entry.vid, &vid_ifindex);
            host_route_entry.key_fields.dst_vid_ifindex = vid_ifindex;
            memcpy(host_route_entry.key_fields.dst_mac, entry_p[i].event_entry.mac, SYS_ADPT_MAC_ADDR_LEN);

            while(AMTRL3_OM_GetNextHostRouteEntryByVlanMac(AMTRL3_TYPE_FLAGS_IPV4 | AMTRL3_TYPE_FLAGS_IPV6, fib_id, &host_route_entry))
            {
                if(memcmp(host_route_entry.key_fields.dst_mac, entry_p[i].event_entry.mac, SYS_ADPT_MAC_ADDR_LEN) != 0)
                    break;

                host_route_entry.key_fields.lport = entry_p[i].event_entry.ifindex;
                host_route_entry.uport = entry_p[i].event_entry.ifindex;

                address_type = host_route_entry.key_fields.dst_inet_addr.type;
                if((address_type == L_INET_ADDR_TYPE_IPV4) ||
                   (address_type == L_INET_ADDR_TYPE_IPV4Z))
                {
                    action_flags = AMTRL3_TYPE_FLAGS_IPV4;
                }
                else if((address_type == L_INET_ADDR_TYPE_IPV6) ||
                        (address_type == L_INET_ADDR_TYPE_IPV6Z))
                {
                    action_flags = AMTRL3_TYPE_FLAGS_IPV6;
                }

                if ((amtrl3_mgr_callback_debug_flag & CALLBACK_DEBUG_PORT_MOVE) &&
                    (amtrl3_mgr_callback_debug_flag & CALLBACK_DEBUG_DETAIL))
                {
                    AMTRL3_MGR_Ntoa(&(host_route_entry.key_fields.dst_inet_addr), ip_str);
                    BACKDOOR_MGR_Printf("Process host entry IP: %s\n", ip_str);
                }

                AMTRL3_MGR_HostRouteEventHandler(action_flags, fib_id, HOST_ROUTE_READY_EVENT, &host_route_entry);
            }
        }
    }

    return;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTRL3_MGR_MACTableDeleteByPort_CallBack
 *------------------------------------------------------------------------
 * FUNCTION: This function is used when a port not oper up.
 * INPUT   : ifindex - specific port
 *           reason  - reason for this not oper up event as defined in
 *                     amtr_mgr.h
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : Currently, we process host table only.
 *------------------------------------------------------------------------*/
void AMTRL3_MGR_MACTableDeleteByPort_CallBack(UI32_T ifindex, UI32_T reason)
{
    char    reason_str[18];
    UI32_T  fib_id;

    AMTRL3_MGR_CHECK_OPERATION_MODE_WITHOUT_RETURN_VALUE();

    if(amtrl3_mgr_callback_debug_flag & CALLBACK_DEBUG_LPORT_DOWN)
    {
        switch (reason)
        {
            case AMTR_MGR_UNKNOWN:
                strcpy(reason_str, "unknown");
                break;
            case AMTR_MGR_UPORT_DOWN:
                strcpy(reason_str, "uport_down");
                break;
            case AMTR_MGR_TRUNK_MEMBER_ADD:
                strcpy(reason_str, "trunk_member_add");
                break;
            case AMTR_MGR_TRUNK_DESTORY:
                strcpy(reason_str, "trunk_destroy");
                break;
            case AMTR_MGR_ADMIN_DISABLE:
                strcpy(reason_str, "admin_disable");
                break;
            case AMTR_MGR_LEARNING_DISABLE:
                strcpy(reason_str, "learning_disable");
                break;
            default:
                strcpy(reason_str, "none");
                break;
        }

        BACKDOOR_MGR_Printf("AMTRL3_MGR_MACTableDeleteByPort_CallBack: lport %d, reason %s\n", (int)ifindex, reason_str);
    }

    if (reason == AMTR_MGR_TRUNK_MEMBER_ADD ||
        reason == AMTR_MGR_LEARNING_DISABLE)
        return;

    for(fib_id = 0; fib_id < SYS_ADPT_MAX_NUMBER_OF_FIB; fib_id++)
        AMTRL3_MGR_DeleteHostRouteEntryByLport(fib_id, ifindex);

    return;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTRL3_MGR_MacAgingOut_CallBack
 *------------------------------------------------------------------------
 * FUNCTION: The function is a call back function is used to change address
 *           when mac age out
 * INPUT   :
 *           num        - num of mac
 *           mac        - mac address catch
 *
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    :
 *------------------------------------------------------------------------*/
void AMTRL3_MGR_MacAgingOut_CallBack(UI32_T num, AMTRL3_TYPE_AddrEntry_T addr_buff[])
{
    UI32_T      action_flags = 0;
    UI8_T       ret = FALSE;
    AMTRL3_OM_HostRouteEntry_T      host_route_entry;
    UI8_T       ip_str[L_INET_MAX_IPADDR_STR_LEN] = {0};
    UI32_T      address_type;
    UI32_T      fib_id;
    int         i = 0;
    UI32_T      vid_ifindex = 0;

    AMTRL3_MGR_CHECK_OPERATION_MODE_WITHOUT_RETURN_VALUE();

    if(amtrl3_mgr_callback_debug_flag & CALLBACK_DEBUG_MAC)
    {
        BACKDOOR_MGR_Printf("AMTRL3_MGR_MacAgingOut_CallBack: vid %d\n", (int)addr_buff[i].vid);
        if(amtrl3_mgr_callback_debug_flag & CALLBACK_DEBUG_DETAIL)
        {
            for(i = 0; i < num; i++)
            {
                BACKDOOR_MGR_Printf("MAC[%d] = %02x-%02x-%02x-%02x-%02x-%02x\n", i,
                       addr_buff[i].mac[0], addr_buff[i].mac[1], addr_buff[i].mac[2],
                       addr_buff[i].mac[3], addr_buff[i].mac[4], addr_buff[i].mac[5]);
            }
        }
    }

    for(fib_id = 0; fib_id < SYS_ADPT_MAX_NUMBER_OF_FIB; fib_id++)
    {
        if(AMTRL3_MGR_GetFIBByID(fib_id) == NULL)
            continue;

        for(i = 0; i < num; i++)
        {
            memset(&host_route_entry, 0, sizeof(AMTRL3_OM_HostRouteEntry_T));

            if(addr_buff[i].vid > SYS_ADPT_MAX_VLAN_ID)
                continue;

            VLAN_OM_ConvertToIfindex(addr_buff[i].vid, &vid_ifindex);
            host_route_entry.key_fields.dst_vid_ifindex = vid_ifindex;
            memcpy(host_route_entry.key_fields.dst_mac, addr_buff[i].mac, (sizeof(UI8_T)*SYS_ADPT_MAC_ADDR_LEN));

            while(AMTRL3_OM_GetNextHostRouteEntryByVlanMac(AMTRL3_TYPE_FLAGS_IPV4 | AMTRL3_TYPE_FLAGS_IPV6, fib_id, &host_route_entry))
            {
                if(memcmp(host_route_entry.key_fields.dst_mac, addr_buff[i].mac, (sizeof(UI8_T)*SYS_ADPT_MAC_ADDR_LEN)) != 0)
                    break;

                address_type = host_route_entry.key_fields.dst_inet_addr.type;
                if((address_type == L_INET_ADDR_TYPE_IPV4) ||
                   (address_type == L_INET_ADDR_TYPE_IPV4Z))
                {
                    action_flags = AMTRL3_TYPE_FLAGS_IPV4;
                }
                else if((address_type == L_INET_ADDR_TYPE_IPV6) ||
                        (address_type == L_INET_ADDR_TYPE_IPV6Z))
                {
                    action_flags = AMTRL3_TYPE_FLAGS_IPV6;
                }

                if ((amtrl3_mgr_callback_debug_flag & CALLBACK_DEBUG_MAC) &&
                    (amtrl3_mgr_callback_debug_flag & CALLBACK_DEBUG_DETAIL))
                {
                    AMTRL3_MGR_Ntoa(&(host_route_entry.key_fields.dst_inet_addr), ip_str);
                    BACKDOOR_MGR_Printf("Process host entry IP: %s\n", ip_str);
                }

                ret = AMTRL3_MGR_HostRouteEventHandler(action_flags, fib_id, HOST_ROUTE_UNREFERENCE_EVENT, &host_route_entry);
            }
        }
    }

    return;
}

/* FUNCTION NAME - AMTRL3_MGR_MacAddrUpdateCallbackHandler
 * PURPOSE : Handle callback when mac added/removed
 * INPUT   : ifindex -- port on which MAC address is updated
 *           vid     -- VLAN ID
 *           mac_p   -- MAC address
 *           is_add  -- MAC address is added or removed
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */
void AMTRL3_MGR_MacAddrUpdateCallbackHandler(UI32_T ifindex, UI32_T vid, UI8_T *mac_p, BOOL_T is_add)
{
    UI32_T action_flags = 0;
    UI32_T address_type;
    UI32_T fib_id;
    UI32_T vid_ifindex = 0;
    AMTRL3_OM_HostRouteEntry_T host_route_entry;

    AMTRL3_MGR_CHECK_OPERATION_MODE_WITHOUT_RETURN_VALUE();

    if (amtrl3_mgr_callback_debug_flag & CALLBACK_DEBUG_MAC)
    {
        BACKDOOR_MGR_Printf("AMTRL3_MGR_MacAddrUpdateCallbackHandler: vid = %lu, port=%lu, is_add=%u\n", (unsigned long)vid, (unsigned long)ifindex, is_add);
        if(amtrl3_mgr_callback_debug_flag & CALLBACK_DEBUG_DETAIL)
        {
            BACKDOOR_MGR_Printf("MAC = %02x-%02x-%02x-%02x-%02x-%02x\n",
                       mac_p[0], mac_p[1], mac_p[2],
                       mac_p[3], mac_p[4], mac_p[5]);
        }
    }

    for (fib_id = 0; fib_id < SYS_ADPT_MAX_NUMBER_OF_FIB; fib_id++)
    {
        if (AMTRL3_MGR_GetFIBByID(fib_id) == NULL)
            continue;

        memset(&host_route_entry, 0, sizeof(AMTRL3_OM_HostRouteEntry_T));

        VLAN_OM_ConvertToIfindex(vid, &vid_ifindex);
        host_route_entry.key_fields.dst_vid_ifindex = vid_ifindex;
        memcpy(host_route_entry.key_fields.dst_mac, mac_p, SYS_ADPT_MAC_ADDR_LEN);

        while (AMTRL3_OM_GetNextHostRouteEntryByVlanMac(AMTRL3_TYPE_FLAGS_IPV4 | AMTRL3_TYPE_FLAGS_IPV6, fib_id, &host_route_entry))
        {
            if (host_route_entry.key_fields.dst_vid_ifindex != vid_ifindex ||
                memcmp(host_route_entry.key_fields.dst_mac, mac_p, SYS_ADPT_MAC_ADDR_LEN) != 0)
                break;

            address_type = host_route_entry.key_fields.dst_inet_addr.type;
            if((address_type == L_INET_ADDR_TYPE_IPV4) ||
               (address_type == L_INET_ADDR_TYPE_IPV4Z))
            {
                action_flags = AMTRL3_TYPE_FLAGS_IPV4;
            }
            else if((address_type == L_INET_ADDR_TYPE_IPV6) ||
                    (address_type == L_INET_ADDR_TYPE_IPV6Z))
            {
                action_flags = AMTRL3_TYPE_FLAGS_IPV6;
            }

            if ((amtrl3_mgr_callback_debug_flag & CALLBACK_DEBUG_MAC) &&
                (amtrl3_mgr_callback_debug_flag & CALLBACK_DEBUG_DETAIL))
            {
                UI8_T ip_str[L_INET_MAX_IPADDR_STR_LEN] = {0};
                AMTRL3_MGR_Ntoa(&(host_route_entry.key_fields.dst_inet_addr), ip_str);
                BACKDOOR_MGR_Printf("Process host entry IP: %s\n", ip_str);
            }

            if (is_add)
            {
                host_route_entry.key_fields.lport = ifindex;
                host_route_entry.uport = ifindex;
                AMTRL3_MGR_HostRouteEventHandler(action_flags, fib_id, HOST_ROUTE_READY_EVENT, &host_route_entry);
            }
            else
            {
                AMTRL3_MGR_HostRouteEventHandler(action_flags, fib_id, HOST_ROUTE_UNREFERENCE_EVENT, &host_route_entry);
            }
        }
    }

/* When vxlan network port is created, it will also check if the network port id of
       vxlan static MAC entry should be corrected in AMTR. But if the entry is not exist 
       in the hisam table of ATMR, for example, it is still in sync queue, the entry will
       be ignored. Here is a workround method. When receiving notification that the entry 
       has been added to hisam table of AMTR, check the entry. 
    */
    if (is_add && AMTR_TYPE_IS_REAL_VFI(vid))
    {
        UI16_T l_vfi;
        AMTR_TYPE_AddrEntry_T addr_entry;
        
        if(AMTRDRV_OM_GetAndAlocRVfiMapToLVfi(vid, &l_vfi)==FALSE)
        {
            return;
        }

        addr_entry.vid = l_vfi;
        memcpy(addr_entry.mac, mac_p, SYS_ADPT_MAC_ADDR_LEN);
        if (AMTR_PMGR_GetExactAddrEntry(&addr_entry) == TRUE)
        {
            if (addr_entry.r_vtep_ip[0])
            {
                AMTRL3_OM_VxlanTunnelEntry_T vxlan_tunnel;

                memset(&vxlan_tunnel, 0, sizeof(vxlan_tunnel));
                vxlan_tunnel.vfi_id = vid;

                if (AMTRL3_OM_GetNextVxlanTunnelEntry(SYS_ADPT_DEFAULT_FIB, &vxlan_tunnel))
                {
                    memcpy(vxlan_tunnel.remote_vtep.addr, addr_entry.r_vtep_ip, SYS_ADPT_IPV4_ADDR_LEN);

                    if (AMTRL3_OM_GetVxlanTunnelEntry(SYS_ADPT_DEFAULT_FIB, &vxlan_tunnel))
                    {
                        if (vxlan_tunnel.uc_vxlan_port)
                        {
                            UI16_T l_vxlan_port;

                            VXLAN_TYPE_R_PORT_CONVERTTO_L_PORT(vxlan_tunnel.uc_vxlan_port, l_vxlan_port);
                            if (ifindex != l_vxlan_port)
                            {
                                addr_entry.ifindex = l_vxlan_port;
                                if (!AMTR_PMGR_SetAddrEntry(&addr_entry))
                                {
                                    if (amtrl3_mgr_debug_flag & DEBUG_LEVEL_ERROR)
                                    {
                                        BACKDOOR_MGR_Printf("%s: Failed to set vxlan static MAC address table.\n", __FUNCTION__);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTRL3_MGR_MACTableDeleteByVIDnPort_CallBack
 *------------------------------------------------------------------------
 * FUNCTION: The function is a call back function used to delete host route
 *           entry associated with a designated port if it is remove from
 *           vlan.
 * INPUT   : lport_ifindex - specific port removed from vlan member list
 *           vid - specific vlan index
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : none
 *------------------------------------------------------------------------*/
void AMTRL3_MGR_MACTableDeleteByVIDnPort_CallBack(UI32_T vid, UI32_T lport_ifindex)
{
    AMTRL3_OM_HostRouteEntry_T      host_route_entry;
    UI32_T      vid_ifindex = 0, action_flags = 0;
    UI32_T      ret = FALSE;
    UI32_T      address_type;
    UI32_T      fib_id;

    if (amtrl3_mgr_callback_debug_flag & CALLBACK_DEBUG_PORT_N_VID)
        BACKDOOR_MGR_Printf("AMTRL3_MGR_MACTableDeleteByVIDnPort_CallBack - port: %d, vid: %d\n",
               (int)lport_ifindex, (int)vid);

    AMTRL3_MGR_CHECK_OPERATION_MODE_WITHOUT_RETURN_VALUE();

    VLAN_OM_ConvertToIfindex(vid, &vid_ifindex);

    for(fib_id = 0; fib_id < SYS_ADPT_MAX_NUMBER_OF_FIB; fib_id++)
    {
        memset(&host_route_entry, 0, sizeof(AMTRL3_OM_HostRouteEntry_T));
        host_route_entry.key_fields.lport= lport_ifindex;

        while(AMTRL3_OM_GetNextHostRouteEntryByLport(AMTRL3_TYPE_FLAGS_IPV4 | AMTRL3_TYPE_FLAGS_IPV6, fib_id, &host_route_entry) == TRUE)
        {
            if(host_route_entry.key_fields.lport != lport_ifindex)
                break;

            if(host_route_entry.key_fields.dst_vid_ifindex != vid_ifindex)
                continue;

            address_type = host_route_entry.key_fields.dst_inet_addr.type;
            if((address_type == L_INET_ADDR_TYPE_IPV4) ||
               (address_type == L_INET_ADDR_TYPE_IPV4Z))
            {
                action_flags = AMTRL3_TYPE_FLAGS_IPV4;
            }
            else if((address_type == L_INET_ADDR_TYPE_IPV6) ||
                    (address_type == L_INET_ADDR_TYPE_IPV6Z))
            {
                action_flags = AMTRL3_TYPE_FLAGS_IPV6;
            }
            ret = AMTRL3_MGR_HostRouteEventHandler(action_flags, fib_id, HOST_ROUTE_UNREFERENCE_EVENT, &host_route_entry);
        } /* end of while */
    }

    return;
} /* end of AMTRL3_MGR_MACTableDeleteByVIDnPort_CallBack() */

/* FUNCTION NAME:  AMTRL3_MGR_VlanDestroy_CallBack
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
void AMTRL3_MGR_VlanDestroy_CallBack(UI32_T vid_ifindex, UI32_T vlan_status)
{
    AMTRL3_OM_HostRouteEntry_T      host_route_entry;
    UI32_T      action_flags = 0;
    UI32_T      address_type;
    UI32_T      fib_id;

    if (amtrl3_mgr_callback_debug_flag & CALLBACK_DEBUG_VLAN_DESTROY)
        BACKDOOR_MGR_Printf("%s - vid_ifindex: %d\n", __FUNCTION__, (int)vid_ifindex);

    AMTRL3_MGR_CHECK_OPERATION_MODE_WITHOUT_RETURN_VALUE();

    for(fib_id = 0; fib_id < SYS_ADPT_MAX_NUMBER_OF_FIB; fib_id++)
    {
        memset(&host_route_entry, 0, sizeof(AMTRL3_OM_HostRouteEntry_T));
        host_route_entry.key_fields.dst_vid_ifindex = vid_ifindex;

        while(AMTRL3_OM_GetNextHostRouteEntryByVlanMac(AMTRL3_TYPE_FLAGS_IPV4 | AMTRL3_TYPE_FLAGS_IPV6, fib_id, &host_route_entry) == TRUE)
        {
            if(host_route_entry.key_fields.dst_vid_ifindex != vid_ifindex)
                break;

            address_type = host_route_entry.key_fields.dst_inet_addr.type;
            if((address_type == L_INET_ADDR_TYPE_IPV4) ||
               (address_type == L_INET_ADDR_TYPE_IPV4Z))
            {
                action_flags = AMTRL3_TYPE_FLAGS_IPV4;
            }
            else if((address_type == L_INET_ADDR_TYPE_IPV6) ||
                    (address_type == L_INET_ADDR_TYPE_IPV6Z))
            {
                action_flags = AMTRL3_TYPE_FLAGS_IPV6;
            }

            AMTRL3_MGR_HostRouteEventHandler(action_flags, fib_id, HOST_ROUTE_REMOVE_EVENT, &host_route_entry);
        } /* end of while */
    }

    return;
}

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
void AMTRL3_MGR_IfOperStatusChanged_CallBack(UI32_T vid_ifindex,
                                             UI32_T oper_status)
{
    AMTRL3_OM_HostRouteEntry_T      host_route_entry;
    UI32_T      action_flags = 0;
    UI32_T      address_type;
    UI32_T      fib_id;

    if (amtrl3_mgr_callback_debug_flag & CALLBACK_DEBUG_VLAN_DESTROY)
        BACKDOOR_MGR_Printf("%s - vid_ifindex: %d  oper_status: %d\n", __FUNCTION__,
            (int)vid_ifindex, (int)oper_status);

    AMTRL3_MGR_CHECK_OPERATION_MODE_WITHOUT_RETURN_VALUE();

    /* Only handle link-down callback */
    if (oper_status != VAL_ifOperStatus_down)
        return;

    for(fib_id = 0; fib_id < SYS_ADPT_MAX_NUMBER_OF_FIB; fib_id++)
    {
        memset(&host_route_entry, 0, sizeof(AMTRL3_OM_HostRouteEntry_T));
        host_route_entry.key_fields.dst_vid_ifindex = vid_ifindex;

        while(AMTRL3_OM_GetNextHostRouteEntryByVlanMac(AMTRL3_TYPE_FLAGS_IPV4 | AMTRL3_TYPE_FLAGS_IPV6, fib_id, &host_route_entry) == TRUE)
        {
            if(host_route_entry.key_fields.dst_vid_ifindex != vid_ifindex)
                break;

            address_type = host_route_entry.key_fields.dst_inet_addr.type;
            if((address_type == L_INET_ADDR_TYPE_IPV4) ||
               (address_type == L_INET_ADDR_TYPE_IPV4Z))
            {
                action_flags = AMTRL3_TYPE_FLAGS_IPV4;
            }
            else if((address_type == L_INET_ADDR_TYPE_IPV6) ||
                    (address_type == L_INET_ADDR_TYPE_IPV6Z))
            {
                action_flags = AMTRL3_TYPE_FLAGS_IPV6;
            }
            AMTRL3_MGR_HostRouteEventHandler(action_flags, fib_id, HOST_ROUTE_UNREFERENCE_EVENT, &host_route_entry);
        } /* end of while */
    }

    return;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTRL3_MGR_LoadBalanceHostRouteForTrunkMember
 *------------------------------------------------------------------------
 * FUNCTION: This function distribute all host / gateway entires on not oper
 *           up trunk member port to other oper up trunk member port
 *
 * INPUT   : fib_id - point to the database of a FIB
 *           trunk_ifindex  - specify which trunk becomes oper up.
 *           member_ifindex - specify which member of trunk oper up.
 *
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : This function only redistribute host / gateway entries learned
 *           on trunk member port.  Net entry associated with gateway entry
 *           will rely on AMTRL3 task to sychronize the information.
 *------------------------------------------------------------------------*/
static void AMTRL3_MGR_LoadBalanceHostRouteForTrunkMember(UI32_T fib_id,
                                                          UI32_T trunk_ifindex,
                                                          UI32_T member_ifindex)
{
    /* Local Variable Declaration
     */
    AMTRL3_OM_HostRouteEntry_T     host_route_entry;
    UI32_T      new_member_port, key, event_type, active_port_count;
    UI32_T      active_trunk_member[SYS_ADPT_MAX_NBR_OF_PORT_PER_TRUNK];
    UI32_T      action_flags = 0;
    UI32_T      address_type;

    /* BODY
     */
    memset (&host_route_entry, 0, sizeof(AMTRL3_OM_HostRouteEntry_T));
    memset (active_trunk_member, 0, sizeof(active_trunk_member));

    new_member_port = key = event_type = active_port_count = 0;
    host_route_entry.key_fields.lport = trunk_ifindex;

    if (!SWCTRL_GetActiveTrunkMember(trunk_ifindex, active_trunk_member, &active_port_count))
    {
        return;
    }

    /* Remove all host entry associated on this member port and find another
     * trunk member port which satisfy load balance requirement and set this
     * host entry to the new member port.
     */

    while(AMTRL3_OM_GetNextHostRouteEntryByLport(AMTRL3_TYPE_FLAGS_IPV4 | AMTRL3_TYPE_FLAGS_IPV6, fib_id, &host_route_entry))
    {
        if (host_route_entry.key_fields.lport != trunk_ifindex)
            break;

        if ((host_route_entry.uport != trunk_ifindex) && (host_route_entry.uport != member_ifindex))
            continue;

        if ((host_route_entry.status != HOST_ROUTE_READY_NOT_SYNC) &&
            (host_route_entry.status != HOST_ROUTE_HOST_READY)     &&
            (host_route_entry.status != HOST_ROUTE_GATEWAY_READY))
            continue;

        address_type = host_route_entry.key_fields.dst_inet_addr.type;
        if((address_type == L_INET_ADDR_TYPE_IPV4) ||
           (address_type == L_INET_ADDR_TYPE_IPV4Z))
        {
            action_flags = AMTRL3_TYPE_FLAGS_IPV4;
        }
        else if((address_type == L_INET_ADDR_TYPE_IPV6) ||
                (address_type == L_INET_ADDR_TYPE_IPV6Z))
        {
            action_flags = AMTRL3_TYPE_FLAGS_IPV6;
        }

        /* There is no other active member on this trunk, therefor, simply
           remove all host route on this port.
         */
        if (active_port_count == 0)
        {
            if (amtrl3_mgr_callback_debug_flag & CALLBACK_TRUNK_DETAIL)
                BACKDOOR_MGR_Printf("AL3_LoadBalanceHostRouteForTrunkMember no more member port \n");
            AMTRL3_MGR_HostRouteEventHandler(action_flags, fib_id, HOST_ROUTE_UNREFERENCE_EVENT, &host_route_entry);
        }
        else
        {
            if (!XSTP_POM_IsPortForwardingStateByVlan(host_route_entry.key_fields.dst_vid_ifindex, trunk_ifindex))
                continue;

            if(action_flags == AMTRL3_TYPE_FLAGS_IPV4)
            {
                memcpy(&key, &(host_route_entry.key_fields.dst_inet_addr.addr), sizeof(UI32_T));
            }
            else if(action_flags == AMTRL3_TYPE_FLAGS_IPV6)
            {
                memcpy(&key, ((void *)&(host_route_entry.key_fields.dst_inet_addr.addr)) + SYS_TYPE_PACKET_LENGTH_OF_IP6_ADDRESS - sizeof(UI32_T), sizeof(UI32_T));
            }
            key %= active_port_count;
            new_member_port = active_trunk_member[key];

            if (!new_member_port)
            {
                if (amtrl3_mgr_debug_flag & DEBUG_LEVEL_ERROR)
                    BACKDOOR_MGR_Printf("Invalid new member port \n");
            }
            if (amtrl3_mgr_callback_debug_flag & CALLBACK_TRUNK_DETAIL)
                BACKDOOR_MGR_Printf("AL3_LoadBalanceHostRouteForTrunkMember on old port: %d, new port: %d\n",
                       (int)member_ifindex, (int)new_member_port);

            host_route_entry.uport = new_member_port;

            if(!CHECK_FLAG(amtrl3_chip_capability_flags, AMTRL3_CHIP_SUPPORT_L3_TRUNK_LOAD_BALANCE))
                AMTRL3_MGR_HostRouteEventHandler(action_flags, fib_id, HOST_ROUTE_READY_EVENT, &host_route_entry);
            else
                AMTRL3_OM_SetHostRouteEntry(action_flags, fib_id, &host_route_entry);
        } /* end of else */
    } /* end of while */

    return;
} /* end of AMTRL3_MGR_LoadBalanceHostRouteForTrunkMember()*/

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_TrunkMemberAddForChipWithLoadBalance
 * -------------------------------------------------------------------------
 * PURPOSE:  This function updates chip and OM for trunk member add
 *           event in case of chip that supports L3 load balance. In such
 *           case, host route learned on member shall be removed and place
 *           on trunk port.
 * INPUT:    fib_id - point to the database of a FIB
 *           trunk_ifindex  - specific trunk member port joins.
 *           member_ifindex - specific member that joins trunk.
 *           host_entry     - host entry learned on member port.
 * OUTPUT:   none.
 * RETURN:   None
 * NOTES:    1. ES4612 supports L3 load balance.
 * -------------------------------------------------------------------------*/
static void AMTRL3_MGR_TrunkMemberAddForChipWithLoadBalance(UI32_T fib_id,
                                                            UI32_T trunk_ifindex,
                                                            UI32_T member_ifindex,
                                                            AMTRL3_OM_HostRouteEntry_T  *host_entry)
{
    /* Local Variable Declaration
     */
    UI32_T      vid, event_type;
    UI32_T      address_type;
    UI32_T      action_flags = 0;
    UI32_T      unit = 0, port = 0;

    /* BODY
     */
    VLAN_OM_ConvertFromIfindex(host_entry->key_fields.dst_vid_ifindex, &vid);

    /* 1. Host route learned on member port shall be updated to trunk_ifindex
     * 2. Remove host route from member_port.
     * 3. Add host route to trunk port.
     * 4. Update OM.
     */

    /* This is 802.1s operation, which forwarding state for trunk_ifindex
     * varies in each vlan
     */
    if (XSTP_POM_IsPortForwardingStateByVlan(vid, trunk_ifindex))
    {
        host_entry->key_fields.lport = trunk_ifindex;
        event_type = HOST_ROUTE_READY_EVENT;
    }
    else
    {
        event_type = HOST_ROUTE_UNREFERENCE_EVENT;
    } /* end of else */


    address_type = host_entry->key_fields.dst_inet_addr.type;
    if((address_type == L_INET_ADDR_TYPE_IPV4) || (address_type == L_INET_ADDR_TYPE_IPV4Z))
        action_flags |= AMTRL3_TYPE_FLAGS_IPV4;
    else if((address_type == L_INET_ADDR_TYPE_IPV6) || (address_type == L_INET_ADDR_TYPE_IPV6Z))
        action_flags |= AMTRL3_TYPE_FLAGS_IPV6;

    if(CHECK_FLAG(amtrl3_chip_capability_flags, AMTRL3_CHIP_KEEP_TRUNK_ID_IN_HOST_ROUTE))
        SWCTRL_POM_LogicalPortToUserPort(host_entry->key_fields.lport, &unit, &port, &(host_entry->key_fields.trunk_id));

    if (!AMTRL3_MGR_HostRouteEventHandler(action_flags, fib_id, event_type, host_entry))
    {
        if (amtrl3_mgr_debug_flag & DEBUG_LEVEL_ERROR)
            BACKDOOR_MGR_Printf("AMTRL3_MGR_HostRouteEventHandler Fails \n");
    } /* end of if */

    return;
} /* end of AMTRL3_MGR_TrunkMemberAddForChipWithLoadBalance() */

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_TrunkMemberAddForChipWithoutLoadBalance
 * -------------------------------------------------------------------------
 * PURPOSE:  This function updates chip and OM for trunk member add
 *           event in case of chip that does not supports L3 load balance.
 *           In such case, host route learned on member port shall remain
 *           with member port when it joins trunk.
 * INPUT:    fib_id - point to the database of a FIB
 *           trunk_ifindex  - specific trunk member port joins
 *           member_ifindex - specific member that joins trunk
 *           host_entry     - host entry learned on member port.
 * OUTPUT:   none.
 * RETURN:   None
 * NOTES:    1. ES3626G does not support L3 load balance.
 * -------------------------------------------------------------------------*/
static void AMTRL3_MGR_TrunkMemberAddForChipWithoutLoadBalance(UI32_T fib_id,
                                                               UI32_T trunk_ifindex,
                                                               UI32_T member_ifindex,
                                                               AMTRL3_OM_HostRouteEntry_T  *host_entry)
{
    /* Local Variable Declaration
     */
    //AMTRL3_OM_HostRouteEntry_T      current_entry;
    UI32_T      vid, event_type;
    UI32_T       address_type;
    UI32_T      action_flags = 0;
    UI32_T      unit = 0, port = 0;

    /* BODY
     */
    //memset(&current_entry, 0, sizeof(AMTRL3_OM_HostRouteEntry_T));
    //memcpy(&current_entry, host_entry, sizeof(AMTRL3_OM_HostRouteEntry_T));
    VLAN_OM_ConvertFromIfindex(host_entry->key_fields.dst_vid_ifindex, &vid);

    /* Host route learned on member port only needs to update its lport data
     * to trunk_ifindex.  Only OM information need to be update in this event.
     */
    /* This is 802.1s operation, which forwarding state for trunk_ifindex
     * varies in each vlan
     */
    if(XSTP_POM_IsPortForwardingStateByVlan(vid, trunk_ifindex))
    {
        host_entry->key_fields.lport = trunk_ifindex;
        event_type = HOST_ROUTE_PARTIAL_EVENT; /*djd*/
    }
    else
    {
        event_type = HOST_ROUTE_UNREFERENCE_EVENT;
    } /* end of else */

    address_type = host_entry->key_fields.dst_inet_addr.type;
    if((address_type == L_INET_ADDR_TYPE_IPV4) || (address_type == L_INET_ADDR_TYPE_IPV4Z))
        action_flags |= AMTRL3_TYPE_FLAGS_IPV4;
    else if((address_type == L_INET_ADDR_TYPE_IPV6) || (address_type == L_INET_ADDR_TYPE_IPV6Z))
        action_flags |= AMTRL3_TYPE_FLAGS_IPV6;

    if(CHECK_FLAG(amtrl3_chip_capability_flags, AMTRL3_CHIP_KEEP_TRUNK_ID_IN_HOST_ROUTE))
        SWCTRL_POM_LogicalPortToUserPort(host_entry->key_fields.lport, &unit, &port, &(host_entry->key_fields.trunk_id));

    if (!AMTRL3_MGR_HostRouteEventHandler(action_flags, fib_id, event_type, host_entry))
    {
        if (amtrl3_mgr_debug_flag & DEBUG_LEVEL_ERROR)
            BACKDOOR_MGR_Printf("AMTRL3_MGR_HostRouteEventHandler Fails \n");
    } /* end of if */

    return;
} /* end of AMTRL3_MGR_TrunkMemberAddForChipWithoutLoadBalance() */

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_AddMyIpHostRouteToChip
 * -------------------------------------------------------------------------
 * PURPOSE:  This function add the my IP host entry into chip which
 *           support local host route.
 * INPUT:    action_flags: AMTRL3_TYPE_FLAGS_IPV4 \ AMTRL3_TYPE_FLAGS_IPV6
 *           fib_id - FIB id
 *           host_entry - contains host route information to set to chip
 * OUTPUT:   None.
 * RETURN:   AMTRL3_MGR_ADD_HOST_ERROR         \
 *           AMTRL3_MGR_INSUFFICIENT_HOST_INFO \
 *           AMTRL3_MGR_ADD_HOST_OK
 * NOTES:    None
 * -------------------------------------------------------------------------*/
static UI32_T AMTRL3_MGR_SetMyIpHostRouteToChip(UI32_T action_flags, UI32_T fib_id, AMTRL3_OM_HostRouteEntry_T  *host_entry)
{
    /* Local Variable Declaration
     */
    UI32_T      begin_timetick = 0, end_timetick = 0;
    UI32_T      ret = AMTRL3_MGR_ADD_HOST_ERROR, result = 0;
    UI8_T       ip_str[L_INET_MAX_IPADDR_STR_LEN] = {0};
    UI32_T      vid = 0;
    SWDRVL3_Host_T  swdrvl3_host_entry;

    /* BODY
     */
    if(host_entry == NULL)
        return AMTRL3_MGR_ADD_HOST_ERROR;

    if(AMTRL3_MGR_GetFIBByID(fib_id) == NULL)
        return AMTRL3_MGR_ADD_HOST_ERROR;

    if (amtrl3_mgr_debug_flag & DEBUG_LEVEL_TRACE_HOST)
    {
        AMTRL3_MGR_Ntoa(&(host_entry->key_fields.dst_inet_addr), ip_str);

        BACKDOOR_MGR_Printf("AMTRL3_MGR_SetMyIpHostRouteToChip IP: %s, vlan: %d, port %d, type: %d\n",
                ip_str, (int)host_entry->key_fields.dst_vid_ifindex, (int)host_entry->key_fields.lport, (int)host_entry->entry_type);
        BACKDOOR_MGR_Printf("Dst MAC = %02x %02x %02x %02x %02x %02x\n", host_entry->key_fields.dst_mac[0], host_entry->key_fields.dst_mac[1],
                                                        host_entry->key_fields.dst_mac[2], host_entry->key_fields.dst_mac[3],
                                                        host_entry->key_fields.dst_mac[4], host_entry->key_fields.dst_mac[5]);
    }

    /* for link local address except my ip it shall not be added into chip */
    if(host_entry->key_fields.dst_inet_addr.type== L_INET_ADDR_TYPE_IPV6Z)
    {
        host_entry->in_chip_status = TRUE;
        return AMTRL3_MGR_ADD_HOST_OK;
    }

    if (amtrl3_mgr_hostroute_performance_flag)
        begin_timetick = SYSFUN_GetSysTick();

    memset(&swdrvl3_host_entry, 0, sizeof (SWDRVL3_Host_T));

    if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
    {
        SET_FLAG(swdrvl3_host_entry.flags, SWDRVL3_FLAG_IPV4);
        memcpy(&(swdrvl3_host_entry.ip_addr), &(host_entry->key_fields.dst_inet_addr.addr), SYS_TYPE_PACKET_LENGTH_OF_IP_ADDRESS);
    }
    else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
    {
        SET_FLAG(swdrvl3_host_entry.flags, SWDRVL3_FLAG_IPV6);
        memcpy(&(swdrvl3_host_entry.ip_addr), &(host_entry->key_fields.dst_inet_addr.addr), SYS_TYPE_PACKET_LENGTH_OF_IP6_ADDRESS);
    }

    swdrvl3_host_entry.fib_id = fib_id;
    VLAN_OM_ConvertFromIfindex(host_entry->key_fields.dst_vid_ifindex, &vid);
    swdrvl3_host_entry.vid = vid;
    memcpy(swdrvl3_host_entry.mac, host_entry->key_fields.dst_mac, sizeof(UI8_T) * SYS_ADPT_MAC_ADDR_LEN);
    result = SWDRVL3_AddInetMyIpHostRoute(&swdrvl3_host_entry);

    /* In current implementation, result will never be SWDRVL3_L3_EXISTS*/
    if(result == SWDRVL3_L3_EXISTS || result == SWDRVL3_L3_NO_ERROR)
        host_entry->in_chip_status = TRUE;
    else
        host_entry->in_chip_status = FALSE;

    if (amtrl3_mgr_hostroute_performance_flag)
    {
        end_timetick = SYSFUN_GetSysTick();
        BACKDOOR_MGR_Printf("SWDRVL3_SetMyIpHostRoute takes %d ticks\n", (int)(end_timetick - begin_timetick));
    }

    if (host_entry->in_chip_status)
    {
        if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
            AMTRL3_OM_IncreaseDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV4_HOST_ROUTE_IN_CHIP, 1);
        else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
            AMTRL3_OM_IncreaseDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV6_HOST_ROUTE_IN_CHIP, 1);
        ret = AMTRL3_MGR_ADD_HOST_OK;
    }
    else
    {
        ret = AMTRL3_MGR_ADD_HOST_ERROR;
        if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
            AMTRL3_OM_IncreaseDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV4_ADD_HOST_ROUTE_FAIL, 1);
        else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
            AMTRL3_OM_IncreaseDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV6_ADD_HOST_ROUTE_FAIL, 1);
        if (amtrl3_mgr_debug_flag & DEBUG_LEVEL_ERROR)
        {
            AMTRL3_MGR_Ntoa(&(host_entry->key_fields.dst_inet_addr), ip_str);
            BACKDOOR_MGR_Printf("SWDRVL3_SetMyIpHostRoute Fails on IP: %s\n", ip_str);
        }
    }
    host_entry->hw_info = swdrvl3_host_entry.hw_info;

    return ret;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_HotInsertAddMyIpHostRouteToChip
 * -------------------------------------------------------------------------
 * PURPOSE:  This function add the my IP host entry into chip which
 *           support local host route.
 * INPUT:    action_flags: AMTRL3_TYPE_FLAGS_IPV4 \ AMTRL3_TYPE_FLAGS_IPV6
 *           fib_id - FIB id
 *           host_entry - contains host route information to set to chip
 * OUTPUT:   None.
 * RETURN:   AMTRL3_MGR_ADD_HOST_ERROR         \
 *           AMTRL3_MGR_INSUFFICIENT_HOST_INFO \
 *           AMTRL3_MGR_ADD_HOST_OK
 * NOTES:    None
 * -------------------------------------------------------------------------*/
static UI32_T AMTRL3_MGR_HotInsertAddMyIpHostRouteToChip(UI32_T action_flags, UI32_T fib_id, AMTRL3_OM_HostRouteEntry_T  *host_entry)
{
    /* Local Variable Declaration
     */
    UI32_T      begin_timetick = 0, end_timetick = 0;
    UI32_T      ret = AMTRL3_MGR_ADD_HOST_ERROR, result = 0;
    UI8_T       ip_str[L_INET_MAX_IPADDR_STR_LEN] = {0};
    UI32_T      vid = 0;
    SWDRVL3_Host_T  swdrvl3_host_entry;

    /* BODY
     */
    if(host_entry == NULL)
        return AMTRL3_MGR_ADD_HOST_ERROR;

    if(AMTRL3_MGR_GetFIBByID(fib_id) == NULL)
        return AMTRL3_MGR_ADD_HOST_ERROR;

    /* for link local address except my ip it shall not be added into chip */
    if(host_entry->key_fields.dst_inet_addr.type == L_INET_ADDR_TYPE_IPV6Z)
    {
        host_entry->in_chip_status = TRUE;
        return AMTRL3_MGR_ADD_HOST_OK;
    }

    if (amtrl3_mgr_hostroute_performance_flag)
        begin_timetick = SYSFUN_GetSysTick();

    memset(&swdrvl3_host_entry, 0, sizeof (SWDRVL3_Host_T));

    if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
    {
        SET_FLAG(swdrvl3_host_entry.flags, SWDRVL3_FLAG_IPV4);
        memcpy(&(swdrvl3_host_entry.ip_addr), &(host_entry->key_fields.dst_inet_addr.addr), SYS_TYPE_PACKET_LENGTH_OF_IP_ADDRESS);
    }
    else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
    {
        SET_FLAG(swdrvl3_host_entry.flags, SWDRVL3_FLAG_IPV6);
        memcpy(&(swdrvl3_host_entry.ip_addr), &(host_entry->key_fields.dst_inet_addr.addr), SYS_TYPE_PACKET_LENGTH_OF_IP6_ADDRESS);
    }

    swdrvl3_host_entry.fib_id = fib_id;
    VLAN_OM_ConvertFromIfindex(host_entry->key_fields.dst_vid_ifindex, &vid);
    swdrvl3_host_entry.vid = vid;
    memcpy(swdrvl3_host_entry.mac, host_entry->key_fields.dst_mac, sizeof(UI8_T) * SYS_ADPT_MAC_ADDR_LEN);
    result = SWDRVL3_HotInsertAddInetMyIpHostRoute(&swdrvl3_host_entry);

    /* In current implementation, result will never be SWDRVL3_L3_EXISTS*/
    if(result == SWDRVL3_L3_EXISTS || result == SWDRVL3_L3_NO_ERROR)
        host_entry->in_chip_status = TRUE;
    else
        host_entry->in_chip_status = FALSE;

    if (amtrl3_mgr_hostroute_performance_flag)
    {
        end_timetick = SYSFUN_GetSysTick();
        BACKDOOR_MGR_Printf("AMTRL3_MGR_HotInsertAddMyIpHostRouteToChip takes %d ticks\n", (int)(end_timetick - begin_timetick));
    }

    if (host_entry->in_chip_status)
    {
        ret = AMTRL3_MGR_ADD_HOST_OK;
    }
    else
    {
        ret = AMTRL3_MGR_ADD_HOST_ERROR;
        if (amtrl3_mgr_debug_flag & DEBUG_LEVEL_ERROR)
        {
            AMTRL3_MGR_Ntoa(&(host_entry->key_fields.dst_inet_addr), ip_str);
            BACKDOOR_MGR_Printf("AMTRL3_MGR_HotInsertAddMyIpHostRouteToChip Fails on IP: %s\n", ip_str);
        }
    }

    return ret;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_DeleteMyIpHostRouteFromChip
 * -------------------------------------------------------------------------
 * PURPOSE:  This function removes my IP host route entry to swdrvl3 and determine
 *           host_route_entry in chip status base on operation result of
 *           driver layer.
 * INPUT:    action_flags: AMTRL3_TYPE_FLAGS_IPV4 \ AMTRL3_TYPE_FLAGS_IPV6
 *           fib_id - FIB id
 *           host_entry - contains host route information to set to chip
 * OUTPUT:   None.
 * RETURN:   TRUE
 *           FALSE
 * NOTES:    None
 * -------------------------------------------------------------------------*/
static BOOL_T AMTRL3_MGR_DeleteMyIpHostRouteFromChip(UI32_T action_flags, UI32_T fib_id, AMTRL3_OM_HostRouteEntry_T  *host_route_entry)
{
    /* Local Variable Declaration
     */
    UI8_T       ip_str[L_INET_MAX_IPADDR_STR_LEN] = {0};
    UI32_T      vid = 0;
    SWDRVL3_Host_T  swdrvl3_host_entry;

    /* BODY
     */
    if(host_route_entry == NULL)
        return FALSE;
    if(AMTRL3_MGR_GetFIBByID(fib_id) == NULL)
        return FALSE;

    if(amtrl3_mgr_debug_flag & DEBUG_LEVEL_TRACE_HOST)
    {
        AMTRL3_MGR_Ntoa(&(host_route_entry->key_fields.dst_inet_addr), ip_str);

        BACKDOOR_MGR_Printf("AMTRL3_MGR_DeleteMyIpHostRouteFromChip IP: %s, vlan: %d, port %d\n",
                ip_str, (int)host_route_entry->key_fields.dst_vid_ifindex, (int)host_route_entry->key_fields.lport);
    } /* end of if */

    if(host_route_entry->key_fields.dst_inet_addr.type== L_INET_ADDR_TYPE_IPV6Z)
        return TRUE;

    if(host_route_entry->in_chip_status == FALSE)
    {
        if (amtrl3_mgr_debug_flag & DEBUG_LEVEL_ERROR)
        {
            BACKDOOR_MGR_Printf("AL3 Internal Error - Delete chip when it does not exist\n");
        }
        return TRUE;
    }

    memset(&swdrvl3_host_entry, 0 ,sizeof(SWDRVL3_Host_T));

    if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
    {
        SET_FLAG(swdrvl3_host_entry.flags, SWDRVL3_FLAG_IPV4);
        memcpy(&(swdrvl3_host_entry.ip_addr), &(host_route_entry->key_fields.dst_inet_addr.addr), SYS_TYPE_PACKET_LENGTH_OF_IP_ADDRESS);
    }
    else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
    {
        SET_FLAG(swdrvl3_host_entry.flags, SWDRVL3_FLAG_IPV6);
        memcpy(&(swdrvl3_host_entry.ip_addr), &(host_route_entry->key_fields.dst_inet_addr.addr), SYS_TYPE_PACKET_LENGTH_OF_IP6_ADDRESS);
    }
    swdrvl3_host_entry.fib_id = fib_id;
    VLAN_OM_ConvertFromIfindex(host_route_entry->key_fields.dst_vid_ifindex, &vid);
    swdrvl3_host_entry.vid = vid;
    swdrvl3_host_entry.hw_info = host_route_entry->hw_info;

    if (!SWDRVL3_DeleteInetMyIpHostRoute(&swdrvl3_host_entry))
    {
        if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
            AMTRL3_OM_IncreaseDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV4_DEL_HOST_ROUTE_FAIL, 1);
        else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
            AMTRL3_OM_IncreaseDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV6_DEL_HOST_ROUTE_FAIL, 1);

        if (amtrl3_mgr_debug_flag & DEBUG_LEVEL_ERROR)
        {
            VLAN_OM_ConvertFromIfindex(host_route_entry->key_fields.dst_vid_ifindex, &vid);
            AMTRL3_MGR_Ntoa(&(host_route_entry->key_fields.dst_inet_addr), ip_str);
            BACKDOOR_MGR_Printf("SWDRVL3_DeleteMyIpHostRoute Fails: IP: %s, vid %d\n",
                   ip_str, (int)vid);
        }
        host_route_entry->hw_info = swdrvl3_host_entry.hw_info;
        return FALSE;
    }
    else
    {
        host_route_entry->in_chip_status = FALSE;
        if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
            AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV4_HOST_ROUTE_IN_CHIP, 1);
        else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
            AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV6_HOST_ROUTE_IN_CHIP, 1);
        host_route_entry->hw_info = swdrvl3_host_entry.hw_info;
        return TRUE;
    } /* end of else */

    host_route_entry->hw_info = swdrvl3_host_entry.hw_info;
    return FALSE;
}

static UI32_T AMTRL3_MGR_AddHostRouteToChip(UI32_T action_flags, UI32_T fib_id, AMTRL3_OM_HostRouteEntry_T  *host_entry)
{
#if (SYS_CPNT_IP_TUNNEL == TRUE)
    if(IS_TUNNEL_IFINDEX(host_entry->key_fields.dst_vid_ifindex))
    {
        return AMTRL3_MGR_AddTunnelHostRouteToChip(action_flags,fib_id, host_entry);
    }
#endif

    return AMTRL3_MGR_AddNonTunnelHostRouteToChip(action_flags,fib_id, host_entry);
}
/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_AddHostRouteToChip
 * -------------------------------------------------------------------------
 * PURPOSE:  This function sets host route entry to swdrvl3 and determine
 *           host_route_entry in chip status base on operation result of
 *           driver layer. In case of local host entries, host route entry
 *           in chip status shall always be FALSE.
 * INPUT:    action_flags: AMTRL3_TYPE_FLAGS_IPV4 \ AMTRL3_TYPE_FLAGS_IPV6
 *           fib_id - FIB ID
 *           host_entry - contains host route information to set to chip
 * OUTPUT:   None.
 * RETURN:   AMTRL3_MGR_ADD_HOST_ERROR         \
 *           AMTRL3_MGR_INSUFFICIENT_HOST_INFO \
 *           AMTRL3_MGR_ADD_HOST_OK
 * NOTES:    None
 * -------------------------------------------------------------------------*/
static UI32_T AMTRL3_MGR_AddNonTunnelHostRouteToChip(UI32_T action_flags, UI32_T fib_id, AMTRL3_OM_HostRouteEntry_T  *host_entry)
{
    /* Local Variable Declaration
     */
    BOOL_T      is_trunk,is_tagged;
    UI32_T      unit, port, trunk_id,vid;
    UI32_T      begin_timetick = 0, end_timetick = 0;
    UI32_T      ret = AMTRL3_MGR_ADD_HOST_ERROR, result = 0;
    UI8_T       ip_str[L_INET_MAX_IPADDR_STR_LEN] = {0};
    SWDRVL3_Host_T  swdrvl3_host_entry;

    /* BODY
     */
    if(host_entry == NULL)
        return AMTRL3_MGR_ADD_HOST_ERROR;
    if(AMTRL3_MGR_GetFIBByID(fib_id) == NULL)
        return AMTRL3_MGR_ADD_HOST_ERROR;
    //if(host_entry->hw_info != AMTRL3_OM_HW_INFO_INVALID)
        //return AMTRL3_MGR_ADD_HOST_ERROR;

    if (amtrl3_mgr_debug_flag & DEBUG_LEVEL_TRACE_HOST)
    {
        AMTRL3_MGR_Ntoa(&(host_entry->key_fields.dst_inet_addr), ip_str);

        BACKDOOR_MGR_Printf("AMTRL3_MGR_SetHostRouteToChip IP: %s, vlan: %d, port %d, type: %d\n",
                ip_str, (int)host_entry->key_fields.dst_vid_ifindex, (int)host_entry->key_fields.lport, (int)host_entry->entry_type);
        BACKDOOR_MGR_Printf("Dst MAC = %02x %02x %02x %02x %02x %02x\n", host_entry->key_fields.dst_mac[0], host_entry->key_fields.dst_mac[1],
                                                        host_entry->key_fields.dst_mac[2], host_entry->key_fields.dst_mac[3],
                                                        host_entry->key_fields.dst_mac[4], host_entry->key_fields.dst_mac[5]);
    } /* end of if */

    unit = port = trunk_id = vid = 0;
    is_trunk = is_tagged =  FALSE;
    memset(&swdrvl3_host_entry, 0 ,sizeof(SWDRVL3_Host_T));

    if ((host_entry->entry_type == VAL_ipNetToPhysicalExtType_local) ||
        (host_entry->entry_type == VAL_ipNetToPhysicalExtType_vrrp))
    {
        if(CHECK_FLAG(amtrl3_chip_capability_flags, AMTRL3_CHIP_USE_LOCAL_HOST_TO_TRAP_MY_IP_PKTS))
        {
            return AMTRL3_MGR_SetMyIpHostRouteToChip(action_flags, fib_id, host_entry);
        }
        else
        {
            host_entry->in_chip_status = FALSE;
            return AMTRL3_MGR_ADD_HOST_OK;
        }
    }

    /* For Dynamic host entries
     */
    if (AMTRL3_MGR_DeriveDetailPortInfo (host_entry->key_fields.dst_vid_ifindex,
                                         host_entry->key_fields.lport,
                                         &swdrvl3_host_entry.vid,
                                         &swdrvl3_host_entry.unit,
                                         &swdrvl3_host_entry.port,
                                         &swdrvl3_host_entry.trunk_id,
                                         &is_tagged, &is_trunk) == FALSE)
    {
        /* Cannot find sufficient port information   */
        return AMTRL3_MGR_INSUFFICIENT_HOST_INFO;
    }

    /* For efficiency consideration, below action may be confirmed by upper layer. */
#if 0
    if (!XSTP_POM_IsPortForwardingStateByVlan(vid, host_entry->key_fields.lport))
       return AMTRL3_MGR_INSUFFICIENT_HOST_INFO;
#endif

    if(CHECK_FLAG(amtrl3_chip_capability_flags, AMTRL3_CHIP_NEED_SRC_MAC_TO_PROGRM_HOST_ROUTE))
        VLAN_PMGR_GetVlanMac(host_entry->key_fields.dst_vid_ifindex, swdrvl3_host_entry.src_mac);

    if (amtrl3_mgr_hostroute_performance_flag)
        begin_timetick = SYSFUN_GetSysTick();


    if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
    {
        swdrvl3_host_entry.flags |= SWDRVL3_FLAG_IPV4;
        memcpy(&(swdrvl3_host_entry.ip_addr), &(host_entry->key_fields.dst_inet_addr.addr), SYS_TYPE_PACKET_LENGTH_OF_IP_ADDRESS);
    }
    else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
    {
        swdrvl3_host_entry.flags |= SWDRVL3_FLAG_IPV6;
        memcpy(&(swdrvl3_host_entry.ip_addr), &(host_entry->key_fields.dst_inet_addr.addr), SYS_TYPE_PACKET_LENGTH_OF_IP6_ADDRESS);
    }
    if(is_tagged)
        swdrvl3_host_entry.flags |= SWDRVL3_FLAG_TAGGED_EGRESS_VLAN;
    if(is_trunk)
        swdrvl3_host_entry.flags |= SWDRVL3_FLAG_TRUNK_EGRESS_PORT;

    swdrvl3_host_entry.fib_id = fib_id;
    swdrvl3_host_entry.hw_info = L_CVRT_UINT_TO_PTR(SWDRVL3_HW_INFO_INVALID);
    memcpy(swdrvl3_host_entry.mac, host_entry->key_fields.dst_mac, sizeof(UI8_T) * SYS_ADPT_MAC_ADDR_LEN);

    /* for link-local address, do not add l3_entry, but add l3_egress object only (to avoid routing) */
    if(host_entry->key_fields.dst_inet_addr.type== L_INET_ADDR_TYPE_IPV6Z)
        swdrvl3_host_entry.flags |= SWDRVL3_FLAG_PROCESS_L3_EGRESS_ONLY;

    result = SWDRVL3_SetInetHostRoute(&swdrvl3_host_entry);

    /* In current implementation, result will never be SWDRVL3_L3_EXISTS*/
    if(result == SWDRVL3_L3_EXISTS || result == SWDRVL3_L3_NO_ERROR)
    {
        host_entry->in_chip_status = TRUE;
        host_entry->hw_info = swdrvl3_host_entry.hw_info;
    }
    else
        host_entry->in_chip_status = FALSE;

    if (amtrl3_mgr_hostroute_performance_flag)
    {
        end_timetick = SYSFUN_GetSysTick();
        BACKDOOR_MGR_Printf("SWDRVL3_AddHostRoute takes %d ticks\n", (int)(end_timetick - begin_timetick));
    }

    if (host_entry->in_chip_status)
    {
        if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
            AMTRL3_OM_IncreaseDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV4_HOST_ROUTE_IN_CHIP, 1);
        else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
            AMTRL3_OM_IncreaseDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV6_HOST_ROUTE_IN_CHIP, 1);
        ret = AMTRL3_MGR_ADD_HOST_OK;
    }
    else
    {
        ret = AMTRL3_MGR_ADD_HOST_ERROR;
        if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
            AMTRL3_OM_IncreaseDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV4_ADD_HOST_ROUTE_FAIL, 1);
        else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
            AMTRL3_OM_IncreaseDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV6_ADD_HOST_ROUTE_FAIL, 1);
        if (amtrl3_mgr_debug_flag & DEBUG_LEVEL_ERROR)
        {
            AMTRL3_MGR_Ntoa(&(host_entry->key_fields.dst_inet_addr), ip_str);
            BACKDOOR_MGR_Printf("SWDRVL3_AddHostRoute Fails on IP: %s\n", ip_str);
        }
    } /* end of else */

    return ret;
} /* end of AMTRL3_MGR_SetHostRouteToChip() */

#if (SYS_CPNT_IP_TUNNEL == TRUE)
static UI32_T AMTRL3_MGR_AddTunnelHostRouteToChip(UI32_T action_flags, UI32_T fib_id, AMTRL3_OM_HostRouteEntry_T  *host_entry)
{
    /* Local Variable Declaration
     */
    BOOL_T      is_trunk,is_tagged;
    UI32_T      unit, port, trunk_id,vid;
    UI32_T      begin_timetick = 0, end_timetick = 0;
    UI32_T      ret = AMTRL3_MGR_ADD_HOST_ERROR, result = 0;
    UI8_T       ip_str[L_INET_MAX_IPADDR_STR_LEN] = {0};
    UI8_T       mymac[SYS_ADPT_MAC_ADDR_LEN];//VLAN_PMGR_GetVlanMac

    SWDRVL3_HostTunnel_T swdrvl3_tunnel_entry;

    NETCFG_TYPE_L3_Interface_T intf;
    /* BODY
     */
    if(host_entry == NULL)
        return AMTRL3_MGR_ADD_HOST_ERROR;
    if(AMTRL3_MGR_GetFIBByID(fib_id) == NULL)
        return AMTRL3_MGR_ADD_HOST_ERROR;


    if (amtrl3_mgr_debug_flag & DEBUG_LEVEL_TRACE_HOST)
    {
        AMTRL3_MGR_Ntoa(&(host_entry->key_fields.dst_inet_addr), ip_str);

        BACKDOOR_MGR_Printf("AMTRL3_MGR_SetTunnelHostRouteToChip IP: %s, vlan: %d, port %d, type: %d\n",
                ip_str, (int)host_entry->key_fields.dst_vid_ifindex, (int)host_entry->key_fields.lport, (int)host_entry->entry_type);
        BACKDOOR_MGR_Printf("Dst MAC = %02x %02x %02x %02x %02x %02x\n", host_entry->key_fields.dst_mac[0], host_entry->key_fields.dst_mac[1],
                                                        host_entry->key_fields.dst_mac[2], host_entry->key_fields.dst_mac[3],
                                                        host_entry->key_fields.dst_mac[4], host_entry->key_fields.dst_mac[5]);
    } /* end of if */

    memset(&intf,0,sizeof(intf));
    intf.ifindex = host_entry->key_fields.dst_vid_ifindex;
    if(NETCFG_TYPE_OK != NETCFG_POM_IP_GetL3Interface(&intf))
    {
        if(amtrl3_mgr_debug_tunnel)
            BACKDOOR_MGR_Printf("fail to get tunnel interface\r\n");
        return AMTRL3_MGR_ADD_HOST_ERROR;
    }

    unit = port = trunk_id = vid = 0;
    is_trunk = is_tagged =  FALSE;
    memset(&swdrvl3_tunnel_entry, 0 ,sizeof(swdrvl3_tunnel_entry));


    if(amtrl3_mgr_debug_tunnel)
    {
        BACKDOOR_MGR_Printf("host type=%lu, hw=%X\r\n", (unsigned long)host_entry->entry_type, host_entry->hw_info);
        BACKDOOR_MGR_Printf("dest v=%ld, ip=%lx:%lx:%lx:%lx, lport=%ld, mac=%x-%x-%x-%x-%x-%-x, trunk=%ld,tunnel src [%ld]%lx:%lx:%lx:%lx ;next[%ld]%lx:%lx:%lx:%lx, dst=%lx:%lx:%lx:%lx\r\n",
                        (long)host_entry->key_fields.dst_vid_ifindex,
                        L_INET_EXPAND_IPV6(host_entry->key_fields.dst_inet_addr.addr),
                        (long)host_entry->key_fields.lport,
                        L_INET_EXPAND_MAC(host_entry->key_fields.dst_mac),
                        (long)host_entry->key_fields.trunk_id,
                        (long)host_entry->key_fields.u.ip_tunnel.src_vidifindex,
                        L_INET_EXPAND_IPV6(host_entry->key_fields.u.ip_tunnel.src_inet_addr.addr),
                        (long)host_entry->key_fields.u.ip_tunnel.nexthop_vidifindex,
                        L_INET_EXPAND_IPV6(host_entry->key_fields.u.ip_tunnel.nexthop_inet_addr.addr),
                        L_INET_EXPAND_IPV6(host_entry->key_fields.u.ip_tunnel.dest_inet_addr.addr) );
        BACKDOOR_MGR_Printf("nexthop=%02x-%02x-%02x-%02x-%02x-%02x\r\n",L_INET_EXPAND_MAC(host_entry->key_fields.dst_mac));
    }
    if(host_entry->key_fields.u.ip_tunnel.nexthop_vidifindex ==0)
    {
        return AMTRL3_MGR_SetMyIpHostRouteToChip(action_flags, fib_id, host_entry);
    }

    memset(&amtrl3_mgr_zero_mac,0, sizeof(amtrl3_mgr_zero_mac));
    /* tunnel's ipv4 nexthop should be requested,
     * we only need to wait the nexthop to be resolved
     */
    if(!memcmp(amtrl3_mgr_zero_mac, host_entry->key_fields.dst_mac, SYS_ADPT_MAC_ADDR_LEN)||
       (host_entry->key_fields.lport == 0))
    {
        if(amtrl3_mgr_debug_tunnel)
        {
            BACKDOOR_MGR_Printf("%s[%d],not sufficient nexthop info\r\n",__FUNCTION__,__LINE__);
            return AMTRL3_MGR_INSUFFICIENT_HOST_INFO;
        }
    }


    /* For Dynamic host entries
     */
    if (AMTRL3_MGR_DeriveDetailPortInfo (host_entry->key_fields.dst_vid_ifindex,
                                         host_entry->key_fields.lport,
                                         &swdrvl3_tunnel_entry.vid,
                                         &swdrvl3_tunnel_entry.unit,
                                         &swdrvl3_tunnel_entry.port,
                                         &swdrvl3_tunnel_entry.trunk_id,
                                         &is_tagged, &is_trunk) == FALSE)
    {
        /* Cannot find sufficient port information   */
        return AMTRL3_MGR_INSUFFICIENT_HOST_INFO;
    }



    if(CHECK_FLAG(amtrl3_chip_capability_flags, AMTRL3_CHIP_NEED_SRC_MAC_TO_PROGRM_HOST_ROUTE))
        VLAN_PMGR_GetVlanMac(host_entry->key_fields.dst_vid_ifindex, swdrvl3_tunnel_entry.src_mac);

    if (amtrl3_mgr_hostroute_performance_flag)
        begin_timetick = SYSFUN_GetSysTick();


    if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
    {
        swdrvl3_tunnel_entry.flags |= SWDRVL3_FLAG_IPV4;
        memcpy(&(swdrvl3_tunnel_entry.ip_addr), &(host_entry->key_fields.dst_inet_addr.addr), SYS_TYPE_PACKET_LENGTH_OF_IP_ADDRESS);
    }
    else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
    {
        swdrvl3_tunnel_entry.flags |= SWDRVL3_FLAG_IPV6;
        memcpy(&(swdrvl3_tunnel_entry.ip_addr), &(host_entry->key_fields.dst_inet_addr.addr), SYS_TYPE_PACKET_LENGTH_OF_IP6_ADDRESS);
    }
    if(is_tagged)
        swdrvl3_tunnel_entry.flags |= SWDRVL3_FLAG_TAGGED_EGRESS_VLAN;
    if(is_trunk)
        swdrvl3_tunnel_entry.flags |= SWDRVL3_FLAG_TRUNK_EGRESS_PORT;

    swdrvl3_tunnel_entry.fib_id = fib_id;

    swdrvl3_tunnel_entry.hw_info = SWDRVL3_HW_INFO_INVALID;
    memcpy(swdrvl3_tunnel_entry.mac, host_entry->key_fields.dst_mac, sizeof(UI8_T) * SYS_ADPT_MAC_ADDR_LEN);

    //config initiator and terminator
    swdrvl3_tunnel_entry.tnl_init.l3_intf_id =0;//output
    VLAN_PMGR_GetVlanMac(host_entry->key_fields.u.ip_tunnel.nexthop_vidifindex ,mymac);
    memcpy(swdrvl3_tunnel_entry.src_mac , mymac ,SYS_ADPT_MAC_ADDR_LEN);
    VLAN_OM_ConvertFromIfindex(host_entry->key_fields.u.ip_tunnel.nexthop_vidifindex, &swdrvl3_tunnel_entry.vid);
    switch(host_entry->key_fields.tunnel_entry_type)
    {
        case AMTRL3_TYPE_INETTOPHYSICALEXTTYPE_TUNNEL_6TO4:
            swdrvl3_tunnel_entry.tnl_init.tunnel_type= SWDRVL3_TUNNELTYPE_6TO4;
            swdrvl3_tunnel_entry.tnl_term.tunnel_type= SWDRVL3_TUNNELTYPE_6TO4;
            swdrvl3_tunnel_entry.tnl_init.sip  = host_entry->key_fields.u.ip_tunnel.src_inet_addr;
            swdrvl3_tunnel_entry.tnl_init.dip = host_entry->key_fields.u.ip_tunnel.dest_inet_addr;
            swdrvl3_tunnel_entry.tnl_term.sip = host_entry->key_fields.u.ip_tunnel.dest_inet_addr;
            swdrvl3_tunnel_entry.tnl_term.sip.preflen = 0;  /* why use 0 ?*/
            swdrvl3_tunnel_entry.tnl_term.dip = host_entry->key_fields.u.ip_tunnel.src_inet_addr ;//my IP
            swdrvl3_tunnel_entry.tnl_term.dip.preflen = 32;
            break;
        case AMTRL3_TYPE_INETTOPHYSICALEXTTYPE_TUNNEL_ISATAP:
            swdrvl3_tunnel_entry.tnl_init.tunnel_type= SWDRVL3_TUNNELTYPE_ISATAP;
            IP_LIB_GetTunnelAddress(&host_entry->key_fields.u.ip_tunnel.src_inet_addr, &swdrvl3_tunnel_entry.tnl_init.sip );
            IP_LIB_GetTunnelAddress(&host_entry->key_fields.dst_inet_addr, &swdrvl3_tunnel_entry.tnl_init.dip);
            break;
        case AMTRL3_TYPE_INETTOPHYSICALEXTTYPE_TUNNEL_MANUAL:
            swdrvl3_tunnel_entry.tnl_init.tunnel_type= SWDRVL3_TUNNELTYPE_MANUAL;
            swdrvl3_tunnel_entry.tnl_term.tunnel_type= SWDRVL3_TUNNELTYPE_MANUAL;
            swdrvl3_tunnel_entry.tnl_init.sip = host_entry->key_fields.u.ip_tunnel.src_inet_addr;
            swdrvl3_tunnel_entry.tnl_init.dip = host_entry->key_fields.u.ip_tunnel.dest_inet_addr;
            swdrvl3_tunnel_entry.tnl_term.sip = host_entry->key_fields.u.ip_tunnel.dest_inet_addr;
            swdrvl3_tunnel_entry.tnl_term.sip.preflen = 32;
            swdrvl3_tunnel_entry.tnl_term.dip = host_entry->key_fields.u.ip_tunnel.src_inet_addr ;
            swdrvl3_tunnel_entry.tnl_term.dip.preflen = 32;
            break;
         default:
            if(amtrl3_mgr_debug_tunnel)
                BACKDOOR_MGR_Printf("unexpected error?\r\n");
            return AMTRL3_MGR_ADD_HOST_ERROR;
    }

    memcpy(swdrvl3_tunnel_entry.tnl_init.src_mac , mymac ,SYS_ADPT_MAC_ADDR_LEN);
    memcpy(swdrvl3_tunnel_entry.tnl_init.nexthop_mac, host_entry->key_fields.dst_mac ,SYS_ADPT_MAC_ADDR_LEN);
    if(!VLAN_OM_ConvertFromIfindex(host_entry->key_fields.u.ip_tunnel.src_vidifindex, &swdrvl3_tunnel_entry.tnl_init.vid))
    {
        if(amtrl3_mgr_debug_tunnel)
            BACKDOOR_MGR_Printf("Unknown source vlan %ld !\r\n", (long)host_entry->key_fields.u.ip_tunnel.src_vidifindex);
        return AMTRL3_MGR_ADD_HOST_ERROR;
    }
    swdrvl3_tunnel_entry.tnl_init.ttl = intf.u.tunnel_intf.ttl;
    {   /* get port list */
        VLAN_OM_Dot1qVlanCurrentEntry_T dot1qvlan;
        memset(&dot1qvlan,0,sizeof(dot1qvlan));
        dot1qvlan.dot1q_vlan_index = host_entry->key_fields.u.ip_tunnel.nexthop_vidifindex;
        if(!VLAN_POM_GetVlanEntry(&dot1qvlan))
        {
            if(amtrl3_mgr_debug_tunnel)
                BACKDOOR_MGR_Printf("Warning! Fail to get port bitmap\r\n");
        }
        memcpy(swdrvl3_tunnel_entry.tnl_term.lport, dot1qvlan.dot1q_vlan_static_egress_ports,SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);
    }

    if(amtrl3_mgr_debug_tunnel)
    {
        BACKDOOR_MGR_Printf("%s[%d]:To driver:\r\n",__FUNCTION__,__LINE__);
        AMTRL3_MGR_PrintSwdrvl3HostTunnelInfo(&swdrvl3_tunnel_entry);
    }
    result = SWDRVL3_AddInetHostTunnelRoute(&swdrvl3_tunnel_entry);
    if(amtrl3_mgr_debug_tunnel)
    {
        BACKDOOR_MGR_Printf("After to driver, output value:\r\n");
        BACKDOOR_MGR_Printf("Initiator l3_intf_id: %ld\r\n", (long)swdrvl3_tunnel_entry.tnl_init.l3_intf_id);
        BACKDOOR_MGR_Printf("HW_INFO: %lX\r\n", (unsigned long)swdrvl3_tunnel_entry.hw_info);
    }

    /* In current implementation, result will never be SWDRVL3_L3_EXISTS*/
    if(result == SWDRVL3_L3_EXISTS || result == SWDRVL3_L3_NO_ERROR)
    {
        host_entry->in_chip_status = TRUE;
        host_entry->hw_info = swdrvl3_tunnel_entry.hw_info;
        host_entry->hw_tunnel_index = swdrvl3_tunnel_entry.tnl_init.l3_intf_id;
    }
    else
    {
        if(amtrl3_mgr_debug_tunnel)
            BACKDOOR_MGR_Printf("To driver Fail :, result %X\r\n", result);
        host_entry->in_chip_status = FALSE;
    }

    if (amtrl3_mgr_hostroute_performance_flag)
    {
        end_timetick = SYSFUN_GetSysTick();
        BACKDOOR_MGR_Printf("SWDRVL3_AddHostRoute takes %d ticks\n", (int)(end_timetick - begin_timetick));
    }

    if (host_entry->in_chip_status)
    {
        if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
            AMTRL3_OM_IncreaseDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV4_HOST_ROUTE_IN_CHIP, 1);
        else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
            AMTRL3_OM_IncreaseDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV6_HOST_ROUTE_IN_CHIP, 1);

        if(amtrl3_mgr_debug_tunnel)
        {
            BACKDOOR_MGR_Printf("%s[%d],add to chip!, entry type:%lu, tunnel entry type:%u\r\n",__FUNCTION__,__LINE__,
                (unsigned long)host_entry->entry_type, host_entry->key_fields.tunnel_entry_type);
        }

        /* if this host route is 6to4 dynamic tunnel host route , increase database count */
        if((host_entry->entry_type == VAL_ipNetToPhysicalExtType_other)&&
           (host_entry->key_fields.tunnel_entry_type == AMTRL3_TYPE_INETTOPHYSICALEXTTYPE_TUNNEL_6TO4))
        {
            AMTRL3_OM_IncreaseDatabaseValue(fib_id, AMTRL3_OM_IPV6_6to4_TUNNEL_HOST_ROUTE_COUNT, 1);
        }

        ret = AMTRL3_MGR_ADD_HOST_OK;
    }
    else
    {
        ret = AMTRL3_MGR_ADD_HOST_ERROR;
        if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
            AMTRL3_OM_IncreaseDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV4_ADD_HOST_ROUTE_FAIL, 1);
        else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
            AMTRL3_OM_IncreaseDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV6_ADD_HOST_ROUTE_FAIL, 1);
        if (amtrl3_mgr_debug_flag & DEBUG_LEVEL_ERROR)
        {
            AMTRL3_MGR_Ntoa(&(host_entry->key_fields.dst_inet_addr), ip_str);
            BACKDOOR_MGR_Printf("SWDRVL3_AddHostRoute Fails on IP: %s\n", ip_str);
        }
    } /* end of else */

    return ret;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_HotInsertAddTunnelHostRouteToModule
 * -------------------------------------------------------------------------
 * PURPOSE:  This function sets host route entry to swdrvl3 and determine
 *           host_route_entry in chip status base on operation result of
 *           driver layer. In case of local host entries, host route entry
 *           in chip status shall always be FALSE.
 * INPUT:    action_flags: AMTRL3_TYPE_FLAGS_IPV4 \ AMTRL3_TYPE_FLAGS_IPV6
 *           fib_id - FIB ID
 *           host_entry - contains tunnel host route information to set to chip
 * OUTPUT:   None.
 * RETURN:   AMTRL3_MGR_ADD_HOST_ERROR         \
 *           AMTRL3_MGR_INSUFFICIENT_HOST_INFO \
 *           AMTRL3_MGR_ADD_HOST_OK
 * NOTES:    None
 * -------------------------------------------------------------------------*/
static UI32_T AMTRL3_MGR_HotInsertAddTunnelHostRouteToModule(UI32_T action_flags, UI32_T fib_id, AMTRL3_OM_HostRouteEntry_T  *host_entry)
{
    /* Local Variable Declaration
     */
    BOOL_T      is_trunk,is_tagged;
    UI32_T      unit, port, trunk_id,vid;
    UI32_T      begin_timetick = 0, end_timetick = 0;
    UI32_T      ret = AMTRL3_MGR_ADD_HOST_ERROR, result = 0;
    UI8_T       ip_str[L_INET_MAX_IPADDR_STR_LEN] = {0};
    UI8_T       mymac[SYS_ADPT_MAC_ADDR_LEN];
    SWDRVL3_HostTunnel_T swdrvl3_tunnel_entry;
    NETCFG_TYPE_L3_Interface_T intf;
    /* BODY
     */
    if(host_entry == NULL)
        return AMTRL3_MGR_ADD_HOST_ERROR;
    if(AMTRL3_MGR_GetFIBByID(fib_id) == NULL)
        return AMTRL3_MGR_ADD_HOST_ERROR;

    if (amtrl3_mgr_debug_flag & DEBUG_LEVEL_TRACE_HOST)
    {
        AMTRL3_MGR_Ntoa(&(host_entry->key_fields.dst_inet_addr), ip_str);

        BACKDOOR_MGR_Printf("AMTRL3_MGR_HotInsertAddHostRouteToModule IP: %s, vlan: %d, port %d, type: %d\n",
                ip_str, (int)host_entry->key_fields.dst_vid_ifindex, (int)host_entry->key_fields.lport, (int)host_entry->entry_type);
        BACKDOOR_MGR_Printf("Dst MAC = %02x %02x %02x %02x %02x %02x\n", host_entry->key_fields.dst_mac[0], host_entry->key_fields.dst_mac[1],
                                                        host_entry->key_fields.dst_mac[2], host_entry->key_fields.dst_mac[3],
                                                        host_entry->key_fields.dst_mac[4], host_entry->key_fields.dst_mac[5]);
    } /* end of if */

    memset(&intf,0,sizeof(intf));
    intf.ifindex = host_entry->key_fields.dst_vid_ifindex;
    if(NETCFG_TYPE_OK != NETCFG_POM_IP_GetL3Interface(&intf))
    {
        if(amtrl3_mgr_debug_tunnel)
            BACKDOOR_MGR_Printf("fail to get tunnel interface\r\n");
        return AMTRL3_MGR_ADD_HOST_ERROR;
    }

    unit = port = trunk_id = vid = 0;
    is_trunk = is_tagged =  FALSE;
    memset(&swdrvl3_tunnel_entry, 0 ,sizeof(swdrvl3_tunnel_entry));
    if(amtrl3_mgr_debug_tunnel)
    {
        BACKDOOR_MGR_Printf("host type=%lu, hw=%X\r\n", (unsigned long)host_entry->entry_type, host_entry->hw_info);
        BACKDOOR_MGR_Printf("dest v=%ld, ip=%lx:%lx:%lx:%lx, lport=%ld, mac=%x-%x-%x-%x-%x-%-x, trunk=%ld,tunnel src [%ld]%lx:%lx:%lx:%lx ;next[%ld]%lx:%lx:%lx:%lx, dst=%lx:%lx:%lx:%lx\r\n",
                        (long)host_entry->key_fields.dst_vid_ifindex,
                        L_INET_EXPAND_IPV6(host_entry->key_fields.dst_inet_addr.addr),
                        (long)host_entry->key_fields.lport,
                        L_INET_EXPAND_MAC(host_entry->key_fields.dst_mac),
                        (long)host_entry->key_fields.trunk_id,
                        (long)host_entry->key_fields.u.ip_tunnel.src_vidifindex,
                        L_INET_EXPAND_IPV6(host_entry->key_fields.u.ip_tunnel.src_inet_addr.addr),
                        (long)host_entry->key_fields.u.ip_tunnel.nexthop_vidifindex,
                        L_INET_EXPAND_IPV6(host_entry->key_fields.u.ip_tunnel.nexthop_inet_addr.addr),
                        L_INET_EXPAND_IPV6(host_entry->key_fields.u.ip_tunnel.dest_inet_addr.addr) );
    }

    /* For Dynamic host entries
     */
    if (AMTRL3_MGR_DeriveDetailPortInfo (host_entry->key_fields.dst_vid_ifindex,
                                         host_entry->key_fields.lport,
                                         &swdrvl3_tunnel_entry.vid,
                                         &swdrvl3_tunnel_entry.unit,
                                         &swdrvl3_tunnel_entry.port,
                                         &swdrvl3_tunnel_entry.trunk_id,
                                         &is_tagged, &is_trunk) == FALSE)
    {
        /* Cannot find sufficient port information   */
        return AMTRL3_MGR_INSUFFICIENT_HOST_INFO;
    }

    if(CHECK_FLAG(amtrl3_chip_capability_flags, AMTRL3_CHIP_NEED_SRC_MAC_TO_PROGRM_HOST_ROUTE))
        VLAN_PMGR_GetVlanMac(host_entry->key_fields.dst_vid_ifindex, swdrvl3_tunnel_entry.src_mac);


    if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
    {
        swdrvl3_tunnel_entry.flags |= SWDRVL3_FLAG_IPV4;
        memcpy(&(swdrvl3_tunnel_entry.ip_addr), &(host_entry->key_fields.dst_inet_addr.addr), SYS_TYPE_PACKET_LENGTH_OF_IP_ADDRESS);
    }
    else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
    {
        swdrvl3_tunnel_entry.flags |= SWDRVL3_FLAG_IPV6;
        memcpy(&(swdrvl3_tunnel_entry.ip_addr), &(host_entry->key_fields.dst_inet_addr.addr), SYS_TYPE_PACKET_LENGTH_OF_IP6_ADDRESS);
    }
    if(is_tagged)
        swdrvl3_tunnel_entry.flags |= SWDRVL3_FLAG_TAGGED_EGRESS_VLAN;
    if(is_trunk)
        swdrvl3_tunnel_entry.flags |= SWDRVL3_FLAG_TRUNK_EGRESS_PORT;

    swdrvl3_tunnel_entry.fib_id = fib_id;

    swdrvl3_tunnel_entry.hw_info = host_entry->hw_info; /* output */
    memcpy(swdrvl3_tunnel_entry.mac, host_entry->key_fields.dst_mac, sizeof(UI8_T) * SYS_ADPT_MAC_ADDR_LEN);

    /*config initiator and terminator*/
    swdrvl3_tunnel_entry.tnl_init.l3_intf_id = host_entry->hw_tunnel_index;/* 0 *//*output, should we set it ?*/
    VLAN_PMGR_GetVlanMac(host_entry->key_fields.u.ip_tunnel.nexthop_vidifindex ,mymac);
    memcpy(swdrvl3_tunnel_entry.src_mac , mymac ,SYS_ADPT_MAC_ADDR_LEN);
    VLAN_OM_ConvertFromIfindex(host_entry->key_fields.u.ip_tunnel.nexthop_vidifindex, &swdrvl3_tunnel_entry.vid);
    switch(host_entry->key_fields.tunnel_entry_type)
    {
        case AMTRL3_TYPE_INETTOPHYSICALEXTTYPE_TUNNEL_6TO4:
            swdrvl3_tunnel_entry.tnl_init.tunnel_type= SWDRVL3_TUNNELTYPE_6TO4;
            swdrvl3_tunnel_entry.tnl_term.tunnel_type= SWDRVL3_TUNNELTYPE_6TO4;
            swdrvl3_tunnel_entry.tnl_init.sip  = host_entry->key_fields.u.ip_tunnel.src_inet_addr;
            swdrvl3_tunnel_entry.tnl_init.dip = host_entry->key_fields.u.ip_tunnel.dest_inet_addr;
            swdrvl3_tunnel_entry.tnl_term.sip = host_entry->key_fields.u.ip_tunnel.dest_inet_addr;
            swdrvl3_tunnel_entry.tnl_term.sip.preflen = 0;  /* why use 0 ?*/
            swdrvl3_tunnel_entry.tnl_term.dip = host_entry->key_fields.u.ip_tunnel.src_inet_addr ;/*my IP*/
            swdrvl3_tunnel_entry.tnl_term.dip.preflen = 32;
            break;
        case AMTRL3_TYPE_INETTOPHYSICALEXTTYPE_TUNNEL_ISATAP:
            swdrvl3_tunnel_entry.tnl_init.tunnel_type= SWDRVL3_TUNNELTYPE_ISATAP;
            IP_LIB_GetTunnelAddress(&host_entry->key_fields.u.ip_tunnel.src_inet_addr, &swdrvl3_tunnel_entry.tnl_init.sip );
            IP_LIB_GetTunnelAddress(&host_entry->key_fields.dst_inet_addr, &swdrvl3_tunnel_entry.tnl_init.dip);
            break;
        case AMTRL3_TYPE_INETTOPHYSICALEXTTYPE_TUNNEL_MANUAL:
            swdrvl3_tunnel_entry.tnl_init.tunnel_type= SWDRVL3_TUNNELTYPE_MANUAL;
            swdrvl3_tunnel_entry.tnl_term.tunnel_type= SWDRVL3_TUNNELTYPE_MANUAL;
            swdrvl3_tunnel_entry.tnl_init.sip = host_entry->key_fields.u.ip_tunnel.src_inet_addr;
            swdrvl3_tunnel_entry.tnl_init.dip = host_entry->key_fields.u.ip_tunnel.dest_inet_addr;
            swdrvl3_tunnel_entry.tnl_term.sip = host_entry->key_fields.u.ip_tunnel.dest_inet_addr;
            swdrvl3_tunnel_entry.tnl_term.sip.preflen = 32;
            swdrvl3_tunnel_entry.tnl_term.dip = host_entry->key_fields.u.ip_tunnel.src_inet_addr ;
            swdrvl3_tunnel_entry.tnl_term.dip.preflen = 32;
            break;
         default:
            if(amtrl3_mgr_debug_tunnel)
                BACKDOOR_MGR_Printf("unexpected error?\r\n");
            return AMTRL3_MGR_ADD_HOST_ERROR;
    }

    memcpy(swdrvl3_tunnel_entry.tnl_init.src_mac , mymac ,SYS_ADPT_MAC_ADDR_LEN);
    memcpy(swdrvl3_tunnel_entry.tnl_init.nexthop_mac, host_entry->key_fields.dst_mac ,SYS_ADPT_MAC_ADDR_LEN);
    if(!VLAN_OM_ConvertFromIfindex(host_entry->key_fields.u.ip_tunnel.src_vidifindex, &swdrvl3_tunnel_entry.tnl_init.vid))
    {
        if(amtrl3_mgr_debug_tunnel)
            BACKDOOR_MGR_Printf("Unknown source vlan %ld !\r\n", (long)host_entry->key_fields.u.ip_tunnel.src_vidifindex);
        return   AMTRL3_MGR_ADD_HOST_ERROR;
    }
    swdrvl3_tunnel_entry.tnl_init.ttl = intf.u.tunnel_intf.ttl;
    {//get port list
        VLAN_OM_Dot1qVlanCurrentEntry_T dot1qvlan;
        memset(&dot1qvlan,0,sizeof(dot1qvlan));
        dot1qvlan.dot1q_vlan_index = host_entry->key_fields.u.ip_tunnel.nexthop_vidifindex;
        if(!VLAN_POM_GetVlanEntry(&dot1qvlan))
        {
            if(amtrl3_mgr_debug_tunnel)
                BACKDOOR_MGR_Printf("Warning! Fail to get port bitmap\r\n");
        }
        memcpy(swdrvl3_tunnel_entry.tnl_term.lport, dot1qvlan.dot1q_vlan_static_egress_ports,SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);
    }

    if(amtrl3_mgr_debug_tunnel)
    {
        BACKDOOR_MGR_Printf("%s[%d]:To driver:\r\n",__FUNCTION__,__LINE__);
        AMTRL3_MGR_PrintSwdrvl3HostTunnelInfo(&swdrvl3_tunnel_entry);
    }
    result = SWDRVL3_HotInsertAddInetHostTunnelRoute(&swdrvl3_tunnel_entry);


    /* In current implementation, result will never be SWDRVL3_L3_EXISTS*/
    if(result == SWDRVL3_L3_EXISTS || result == SWDRVL3_L3_NO_ERROR)
    {
        ret = AMTRL3_MGR_ADD_HOST_OK;
    }
    else
    {
        ret = AMTRL3_MGR_ADD_HOST_ERROR;
        if (amtrl3_mgr_debug_flag & DEBUG_LEVEL_ERROR)
        {
            AMTRL3_MGR_Ntoa(&(host_entry->key_fields.dst_inet_addr), ip_str);
            BACKDOOR_MGR_Printf("SWDRVL3_AddHostRoute Fails on IP: %s\n", ip_str);
        }
    }

    return ret;
}

/* FUNCTION NAME:  AMTRL3_MGR_TunnelNetRouteHitBitChange_CallBack
 * PURPOSE : Handle the callback event happening when slave unit's tunnel net route hit bit change
 *
 * INPUT   : fib_id      -- fib index
 *           dst_addr    -- net route destination address
 *           preflen     -- net route prefix length
 *           unit_id     -- tunnel hit bit change unit index
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *
 */
void AMTRL3_MGR_TunnelNetRouteHitBitChange_CallBack(UI32_T fib_id,UI8_T* dst_addr, UI32_T preflen, UI32_T unit_id)
{
    /* Local Variable Declaration
     */
    AMTRL3_OM_NetRouteEntry_T      net_route_entry;
    UI32_T      hit_bit = 0;

    /* BODY
     */
    AMTRL3_MGR_CHECK_OPERATION_MODE_WITHOUT_RETURN_VALUE();

    hit_bit = 1 << (unit_id-1);

    memset(&net_route_entry, 0, sizeof(net_route_entry));

    net_route_entry.inet_cidr_route_entry.inet_cidr_route_dest.type = L_INET_ADDR_TYPE_IPV6;


    if(amtrl3_mgr_debug_tunnel)
        BACKDOOR_MGR_Printf("destination:%lx:%lx:%lx:%lx/%lu\r\n",L_INET_EXPAND_IPV6(dst_addr),(unsigned long)preflen);

    while(AMTRL3_OM_GetNextNetRouteEntry(AMTRL3_TYPE_FLAGS_IPV6,fib_id, &net_route_entry))
    {

        if(net_route_entry.tunnel_entry_type == AMTRL3_TYPE_INETTOPHYSICALEXTTYPE_TUNNEL_6TO4)
        {

            if((0 == memcmp(net_route_entry.inet_cidr_route_entry.inet_cidr_route_dest.addr, dst_addr, SYS_ADPT_IPV6_ADDR_LEN))&&
               (net_route_entry.inet_cidr_route_entry.inet_cidr_route_pfxlen = preflen))
            {
                /* update corresponding unit's tunnel hit bit */
                net_route_entry.tunnel_hit ^= hit_bit;
                net_route_entry.tunnel_hit_timestamp = (SYSFUN_GetSysTick() / SYS_BLD_TICKS_PER_SECOND);
                if(FALSE == AMTRL3_OM_SetNetRouteEntry(AMTRL3_TYPE_FLAGS_IPV6, fib_id, &net_route_entry))
                {
                    if(amtrl3_mgr_debug_tunnel)
                        BACKDOOR_MGR_Printf("%s[%d]failed to update hit bit \r\n",__FUNCTION__,__LINE__);
                }
                break;
            }
        }
    }

    return;
}

/* FUNCTION NAME:  AMTRL3_MGR_TunnelUpdateTtl
 * PURPOSE : Update chip's tunnel ttl value
 *
 * INPUT   : al3_tunnel_host->dst_vid_ifindex         -- tunnel's vlan ifindex
 *                            tunnel_dest_inet_addr   -- ipv4 destination endpoint address
 *                            tunnel_src_vidifindex   -- ipv4 source endpoint vlan ifindex
 *                            tunnel_entry_type       -- tunnel mode
 *           tunnel_ttl                               -- ttl in ipv4 header
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE/FALSE.
 *
 * NOTES   : None.
 *
 */
static BOOL_T AMTRL3_MGR_TunnelUpdateTtl(AMTRL3_TYPE_InetHostRouteEntry_T * al3_tunnel_host, UI32_T tunnel_ttl)
{
    AMTRL3_OM_HostRouteEntry_T host_om;
    SWDRVL3_HostTunnel_T swdrvl3_tunnel_entry;
    BOOL_T found = FALSE;
    /* find corresponding tunnel host entry */
    memset(&host_om, 0, sizeof(host_om));
    host_om.key_fields.dst_inet_addr.type = L_INET_ADDR_TYPE_IPV6;
    while(AMTRL3_OM_GetNextHostRouteEntry(AMTRL3_TYPE_FLAGS_IPV6, SYS_ADPT_DEFAULT_FIB, &host_om))
    {
        if(!IS_TUNNEL_IFINDEX(host_om.key_fields.dst_vid_ifindex))
            continue;

        /* check if filled-in fields are the same */
        if((al3_tunnel_host->dst_vid_ifindex == host_om.key_fields.dst_vid_ifindex)&&
           (al3_tunnel_host->tunnel_entry_type == host_om.key_fields.tunnel_entry_type)&&
           (al3_tunnel_host->u.ip_tunnel.src_vidifindex == host_om.key_fields.u.ip_tunnel.src_vidifindex)&&
           (!memcmp(al3_tunnel_host->u.ip_tunnel.dest_inet_addr.addr, host_om.key_fields.u.ip_tunnel.dest_inet_addr.addr, SYS_ADPT_IPV4_ADDR_LEN)))
        {
            found = TRUE;
            break;
        }
    }

    if(found)
    {
        memset(&swdrvl3_tunnel_entry, 0, sizeof(swdrvl3_tunnel_entry));
        /* get initiator information */
        swdrvl3_tunnel_entry.tnl_init.l3_intf_id = host_om.hw_tunnel_index;
        swdrvl3_tunnel_entry.tnl_init.ttl = (UI8_T)tunnel_ttl;
        if(amtrl3_mgr_debug_tunnel)
        {
            BACKDOOR_MGR_Printf("%s[%d]\r\n",__FUNCTION__,__LINE__);
            BACKDOOR_MGR_Printf("tunnel l3 intf id:%lu\r\n", (unsigned long)swdrvl3_tunnel_entry.tnl_init.l3_intf_id);
            BACKDOOR_MGR_Printf("tunnel ttl: %lu\r\n", (unsigned long)swdrvl3_tunnel_entry.tnl_init.ttl);
        }
        /* swdrvl3_tunnel_entry.tnl_init.src_mac*/
        /* swdrvl3_tunnel_entry.tnl_init.vid */
        /* swdrvl3_tunnel_entry.tnl_init.tunnel_type */
        /* swdrvl3_tunnel_entry.tnl_init.sip */
        /* swdrvl3_tunnel_entry.tnl_init.dip */
        /* swdrvl3_tunnel_entry.tnl_init.nexthop_mac */

        if(SWDRVL3_L3_NO_ERROR != SWDRVL3_TunnelUpdateTtl(&swdrvl3_tunnel_entry))
            return FALSE;
    }
    else
        return TRUE;

    return TRUE;
}

#endif /*SYS_CPNT_IP_TUNNEL*/

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_MacScan
 * -------------------------------------------------------------------------
 * PURPOSE:  Perform MAC scan for all host routes, i.e. check whether
 *           corresponding MAC address still exist in AMTR
 * INPUT:    fib_id - FIB id.
 * OUTPUT:   None
 * RETURN:   TRUE  - All host entry has been processed.
 *           FALSE - Part of host table has not been scanned
 * NOTES:    None
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_MGR_MacScan(UI32_T fib_id)
{
    UI32_T process_cnt = 0, max_process_cnt;
    UI32_T action_flags = 0;
    UI32_T address_type;
    UI32_T tmp_vid, total_host;
    BOOL_T mac_entry_exist;
    AMTR_TYPE_AddrEntry_T amtr_addr_entry;
    static AMTRL3_OM_HostRouteEntry_T host_route_entry;

    AMTRL3_MGR_CHECK_OPERATION_MODE(TRUE);

    if (AMTRL3_MGR_GetFIBByID(fib_id) == NULL)
        return TRUE;

    if(amtrl3_mgr_debug_task  & TASK_DEBUG_MAC_SCAN)
        BACKDOOR_MGR_Printf("AMTRL3_MGR_ArpScan\n");

    total_host = AMTRL3_OM_GetTotalHostRouteNumber(AMTRL3_TYPE_FLAGS_IPV4 | AMTRL3_TYPE_FLAGS_IPV6, fib_id);
    max_process_cnt = AMTRL3_MGR_NUMBER_OF_HOST_ENTRY_TO_SCAN;
    if ((total_host/AMTRL3_MGR_MAX_MAC_SCAN_TIME) > max_process_cnt)
        max_process_cnt = total_host/AMTRL3_MGR_MAX_MAC_SCAN_TIME;

    while(AMTRL3_OM_GetNextHostRouteEntryByLport(AMTRL3_TYPE_FLAGS_IPV4 | AMTRL3_TYPE_FLAGS_IPV6,
                                                 fib_id, &host_route_entry))
    {
        if (host_route_entry.entry_type != VAL_ipNetToPhysicalExtType_dynamic &&
            host_route_entry.entry_type != VAL_ipNetToPhysicalExtType_static)
            continue;

        memset(&amtr_addr_entry, 0, sizeof(AMTR_TYPE_AddrEntry_T));
        memcpy(amtr_addr_entry.mac,host_route_entry.key_fields.dst_mac,SYS_ADPT_MAC_ADDR_LEN);
        VLAN_OM_ConvertFromIfindex(host_route_entry.key_fields.dst_vid_ifindex, &tmp_vid);
        amtr_addr_entry.vid = (UI16_T)tmp_vid;
        if(AMTRL3_MGR_GetAMTRExactMAC(&amtr_addr_entry))
            mac_entry_exist = TRUE;
        else
            mac_entry_exist = FALSE;

        address_type = host_route_entry.key_fields.dst_inet_addr.type;
        if ((address_type == L_INET_ADDR_TYPE_IPV4) || (address_type == L_INET_ADDR_TYPE_IPV4Z))
            action_flags = AMTRL3_TYPE_FLAGS_IPV4;
        else if((address_type == L_INET_ADDR_TYPE_IPV6) || (address_type == L_INET_ADDR_TYPE_IPV6Z))
            action_flags = AMTRL3_TYPE_FLAGS_IPV6;

        if (mac_entry_exist == FALSE)
        {
            if (!AMTRL3_MGR_HostRouteEventHandler(action_flags, fib_id, HOST_ROUTE_UNREFERENCE_EVENT, &host_route_entry))
            {
                if (amtrl3_mgr_debug_flag & DEBUG_LEVEL_ERROR)
                {
                    UI8_T ip_str[L_INET_MAX_IPADDR_STR_LEN] = {0};

                    AMTRL3_MGR_Ntoa(&(host_route_entry.key_fields.dst_inet_addr), ip_str);
                    BACKDOOR_MGR_Printf("AMTRL3_MGR_HostRouteEventHandler Fails %s\n", ip_str);
                }
            }
        }
        else if (amtr_addr_entry.ifindex != host_route_entry.key_fields.lport)
        {
            host_route_entry.key_fields.lport = amtr_addr_entry.ifindex;
            host_route_entry.uport = amtr_addr_entry.ifindex;
            if (!AMTRL3_MGR_HostRouteEventHandler(action_flags, fib_id, HOST_ROUTE_READY_EVENT, &host_route_entry))
            {
                if (amtrl3_mgr_debug_flag & DEBUG_LEVEL_ERROR)
                {
                    UI8_T ip_str[L_INET_MAX_IPADDR_STR_LEN] = {0};

                    AMTRL3_MGR_Ntoa(&(host_route_entry.key_fields.dst_inet_addr), ip_str);
                    BACKDOOR_MGR_Printf("AMTRL3_MGR_HostRouteEventHandler Fails %s\n", ip_str);
                }
            }
        }

        if (++process_cnt >= max_process_cnt)
            return FALSE;
    }

    memset(&host_route_entry, 0, sizeof(AMTRL3_OM_HostRouteEntry_T));

    return TRUE;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_HotInsertAddHostRouteToModule
 * -------------------------------------------------------------------------
 * PURPOSE:  This function sets host route entry to swdrvl3 and determine
 *           host_route_entry in chip status base on operation result of
 *           driver layer. In case of local host entries, host route entry
 *           in chip status shall always be FALSE.
 * INPUT:    action_flags: AMTRL3_TYPE_FLAGS_IPV4 \ AMTRL3_TYPE_FLAGS_IPV6
 *           fib_id - FIB ID
 *           host_entry - contains host route information to set to chip
 * OUTPUT:   None.
 * RETURN:   AMTRL3_MGR_ADD_HOST_ERROR         \
 *           AMTRL3_MGR_INSUFFICIENT_HOST_INFO \
 *           AMTRL3_MGR_ADD_HOST_OK
 * NOTES:    None
 * -------------------------------------------------------------------------*/
static UI32_T AMTRL3_MGR_HotInsertAddHostRouteToModule(UI32_T action_flags, UI32_T fib_id, AMTRL3_OM_HostRouteEntry_T  *host_entry)
{
    /* Local Variable Declaration
     */
    BOOL_T      is_trunk,is_tagged;
    UI32_T      unit, port, trunk_id,vid;
    UI32_T      begin_timetick = 0, end_timetick = 0;
    UI32_T      ret = AMTRL3_MGR_ADD_HOST_ERROR, result = 0;
    UI8_T       ip_str[L_INET_MAX_IPADDR_STR_LEN] = {0};
    SWDRVL3_Host_T  swdrvl3_host_entry;

    /* BODY
     */
    if(host_entry == NULL)
        return AMTRL3_MGR_ADD_HOST_ERROR;
    if(AMTRL3_MGR_GetFIBByID(fib_id) == NULL)
        return AMTRL3_MGR_ADD_HOST_ERROR;

    if (amtrl3_mgr_debug_flag & DEBUG_LEVEL_TRACE_HOST)
    {
        AMTRL3_MGR_Ntoa(&(host_entry->key_fields.dst_inet_addr), ip_str);

        BACKDOOR_MGR_Printf("AMTRL3_MGR_HotInsertAddHostRouteToModule IP: %s, vlan: %d, port %d, type: %d\n",
                ip_str, (int)host_entry->key_fields.dst_vid_ifindex, (int)host_entry->key_fields.lport, (int)host_entry->entry_type);
        BACKDOOR_MGR_Printf("Dst MAC = %02x %02x %02x %02x %02x %02x\n", host_entry->key_fields.dst_mac[0], host_entry->key_fields.dst_mac[1],
                                                        host_entry->key_fields.dst_mac[2], host_entry->key_fields.dst_mac[3],
                                                        host_entry->key_fields.dst_mac[4], host_entry->key_fields.dst_mac[5]);
    } /* end of if */

    unit = port = trunk_id = vid = 0;
    is_trunk = is_tagged =  FALSE;
    memset(&swdrvl3_host_entry, 0 ,sizeof(SWDRVL3_Host_T));

    if ((host_entry->entry_type == VAL_ipNetToPhysicalExtType_local) ||
        (host_entry->entry_type == VAL_ipNetToPhysicalExtType_vrrp))
    {
        if(CHECK_FLAG(amtrl3_chip_capability_flags, AMTRL3_CHIP_USE_LOCAL_HOST_TO_TRAP_MY_IP_PKTS))
        {
            return AMTRL3_MGR_HotInsertAddMyIpHostRouteToChip(action_flags, fib_id, host_entry);
        }
        else
        {
            host_entry->in_chip_status = FALSE;
            return AMTRL3_MGR_ADD_HOST_OK;
        }
    }
    /* for link local address except my ip it shall not be added into chip */
    if(host_entry->key_fields.dst_inet_addr.type == L_INET_ADDR_TYPE_IPV6Z)
    {
        host_entry->in_chip_status = TRUE;
        return AMTRL3_MGR_ADD_HOST_OK;
    }

    /* For Dynamic host entries
     */
    if (AMTRL3_MGR_DeriveDetailPortInfo (host_entry->key_fields.dst_vid_ifindex,
                                         host_entry->key_fields.lport,
                                         &swdrvl3_host_entry.vid,
                                         &swdrvl3_host_entry.unit,
                                         &swdrvl3_host_entry.port,
                                         &swdrvl3_host_entry.trunk_id,
                                         &is_tagged, &is_trunk) == FALSE)
    {
        /* Cannot find sufficient port information   */
        return AMTRL3_MGR_INSUFFICIENT_HOST_INFO;
    }

    /* For efficiency consideration, below action may be confirmed by upper layer. */
#if 0
    if (!XSTP_POM_IsPortForwardingStateByVlan(vid, host_entry->key_fields.lport))
       return AMTRL3_MGR_INSUFFICIENT_HOST_INFO;
#endif

    if(CHECK_FLAG(amtrl3_chip_capability_flags, AMTRL3_CHIP_NEED_SRC_MAC_TO_PROGRM_HOST_ROUTE))
        VLAN_PMGR_GetVlanMac(host_entry->key_fields.dst_vid_ifindex, swdrvl3_host_entry.src_mac);

    if (amtrl3_mgr_hostroute_performance_flag)
        begin_timetick = SYSFUN_GetSysTick();

    if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
    {
        swdrvl3_host_entry.flags |= SWDRVL3_FLAG_IPV4;
        memcpy(&(swdrvl3_host_entry.ip_addr), &(host_entry->key_fields.dst_inet_addr.addr), SYS_TYPE_PACKET_LENGTH_OF_IP_ADDRESS);
    }
    else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
    {
        swdrvl3_host_entry.flags |= SWDRVL3_FLAG_IPV6;
        memcpy(&(swdrvl3_host_entry.ip_addr), &(host_entry->key_fields.dst_inet_addr.addr), SYS_TYPE_PACKET_LENGTH_OF_IP6_ADDRESS);
    }
    if(is_tagged)
        swdrvl3_host_entry.flags |= SWDRVL3_FLAG_TAGGED_EGRESS_VLAN;
    if(is_trunk)
        swdrvl3_host_entry.flags |= SWDRVL3_FLAG_TRUNK_EGRESS_PORT;

    swdrvl3_host_entry.fib_id = fib_id;
    swdrvl3_host_entry.hw_info = host_entry->hw_info;
    memcpy(swdrvl3_host_entry.mac, host_entry->key_fields.dst_mac, sizeof(UI8_T) * SYS_ADPT_MAC_ADDR_LEN);
    result = SWDRVL3_HotInsertAddInetHostRoute(&swdrvl3_host_entry);

    /* In current implementation, result will never be SWDRVL3_L3_EXISTS*/
    if(result == SWDRVL3_L3_EXISTS || result == SWDRVL3_L3_NO_ERROR)
    {
        host_entry->in_chip_status = TRUE;
    }
    else
        host_entry->in_chip_status = FALSE;

    if (amtrl3_mgr_hostroute_performance_flag)
    {
        end_timetick = SYSFUN_GetSysTick();
        BACKDOOR_MGR_Printf("SWDRVL3_AddHostRoute takes %d ticks\n", (int)(end_timetick - begin_timetick));
    }

    if (host_entry->in_chip_status)
    {
        ret = AMTRL3_MGR_ADD_HOST_OK;
    }
    else
    {
        ret = AMTRL3_MGR_ADD_HOST_ERROR;
        if (amtrl3_mgr_debug_flag & DEBUG_LEVEL_ERROR)
        {
            AMTRL3_MGR_Ntoa(&(host_entry->key_fields.dst_inet_addr), ip_str);
            BACKDOOR_MGR_Printf("SWDRVL3_AddHostRoute Fails on IP: %s\n", ip_str);
        }
    } /* end of else */

    return ret;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_UpdateHostRouteToChip
 * -------------------------------------------------------------------------
 * PURPOSE:  This function update the host route in chip. Use the hw_info
 *           in the host route.
 * INPUT:    action_flags: AMTRL3_TYPE_FLAGS_IPV4 \ AMTRL3_TYPE_FLAGS_IPV6
 *           fib_id       -- FIB id
 *           host_entry - contains host route information to set to chip
 * OUTPUT:   None.
 * RETURN:   AMTRL3_MGR_ADD_HOST_ERROR         \
 *           AMTRL3_MGR_INSUFFICIENT_HOST_INFO \
 *           AMTRL3_MGR_ADD_HOST_OK
 * NOTES:    For some chips which support egress object, the host entry directly
 *           link to the egress object, and when L2 info change, there is no need
 *           to delete and add the host, we can use updating.
 * -------------------------------------------------------------------------*/
static UI32_T AMTRL3_MGR_UpdateHostRouteToChip(UI32_T action_flags, UI32_T fib_id, AMTRL3_OM_HostRouteEntry_T  *host_entry)
{
#if (SYS_CPNT_IP_TUNNEL == TRUE)
    if(IS_TUNNEL_IFINDEX(host_entry->key_fields.dst_vid_ifindex))
    {
         return AMTRL3_MGR_UpdateTunnelHostRouteToChip(action_flags, fib_id, host_entry);
    }
#endif

    return AMTRL3_MGR_UpdateNonTunnelHostRouteToChip(action_flags, fib_id, host_entry);
}

static UI32_T AMTRL3_MGR_UpdateNonTunnelHostRouteToChip(UI32_T action_flags, UI32_T fib_id, AMTRL3_OM_HostRouteEntry_T  *host_entry)
{
    /* Local Variable Declaration
     */
    BOOL_T      is_trunk,is_tagged;
    UI32_T      unit, port, trunk_id,vid;
    UI32_T      begin_timetick, end_timetick;
    UI32_T      ret = AMTRL3_MGR_ADD_HOST_ERROR, result = 0;
    UI8_T       ip_str[L_INET_MAX_IPADDR_STR_LEN] = {0};
    SWDRVL3_Host_T  swdrvl3_host_entry;

    /* BODY
     */

    begin_timetick = end_timetick = 0;

    if(host_entry == NULL)
        return AMTRL3_MGR_ADD_HOST_ERROR;
    if(AMTRL3_MGR_GetFIBByID(fib_id) == NULL)
        return AMTRL3_MGR_ADD_HOST_ERROR;
    if(host_entry->hw_info == L_CVRT_UINT_TO_PTR(AMTRL3_OM_HW_INFO_INVALID))
        return AMTRL3_MGR_ADD_HOST_ERROR;

    if (amtrl3_mgr_debug_flag & DEBUG_LEVEL_TRACE_HOST)
    {
        AMTRL3_MGR_Ntoa(&(host_entry->key_fields.dst_inet_addr), ip_str);

        BACKDOOR_MGR_Printf("AMTRL3_MGR_UpdateHostRouteToChip IP: %s, vlan: %d, port %d, type: %d\n",
                ip_str, (int)host_entry->key_fields.dst_vid_ifindex, (int)host_entry->key_fields.lport, (int)host_entry->entry_type);
        BACKDOOR_MGR_Printf("Dst MAC = %02x %02x %02x %02x %02x %02x\n", host_entry->key_fields.dst_mac[0], host_entry->key_fields.dst_mac[1],
                                                        host_entry->key_fields.dst_mac[2], host_entry->key_fields.dst_mac[3],
                                                        host_entry->key_fields.dst_mac[4], host_entry->key_fields.dst_mac[5]);
    }

    unit = port = trunk_id = vid = 0;
    is_trunk = is_tagged =  FALSE;
    memset(&swdrvl3_host_entry, 0 ,sizeof(SWDRVL3_Host_T));

    /* for link local address except my ip it shall not be added into chip */
    if(host_entry->key_fields.dst_inet_addr.type== L_INET_ADDR_TYPE_IPV6Z)
    {
        host_entry->in_chip_status = TRUE;
        return AMTRL3_MGR_ADD_HOST_OK;
    }

    /* For Dynamic host entries
     */
    if (AMTRL3_MGR_DeriveDetailPortInfo (host_entry->key_fields.dst_vid_ifindex,
                                         host_entry->key_fields.lport,
                                         &swdrvl3_host_entry.vid,
                                         &swdrvl3_host_entry.unit,
                                         &swdrvl3_host_entry.port,
                                         &swdrvl3_host_entry.trunk_id,
                                         &is_tagged, &is_trunk) == FALSE)
    {
        /* Cannot find sufficient port information   */
        return AMTRL3_MGR_INSUFFICIENT_HOST_INFO;
    }

    /* For efficiency consideration, below action may be confirmed by upper layer. */
#if 0
    if (!XSTP_POM_IsPortForwardingStateByVlan(vid, host_entry->key_fields.lport))
       return AMTRL3_MGR_INSUFFICIENT_HOST_INFO;
#endif

    if(CHECK_FLAG(amtrl3_chip_capability_flags, AMTRL3_CHIP_NEED_SRC_MAC_TO_PROGRM_HOST_ROUTE))
        VLAN_PMGR_GetVlanMac(host_entry->key_fields.dst_vid_ifindex, swdrvl3_host_entry.src_mac);

    if (amtrl3_mgr_hostroute_performance_flag)
        begin_timetick = SYSFUN_GetSysTick();


    if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
    {
        swdrvl3_host_entry.flags |= SWDRVL3_FLAG_IPV4;
        memcpy(&(swdrvl3_host_entry.ip_addr), &(host_entry->key_fields.dst_inet_addr.addr), SYS_TYPE_PACKET_LENGTH_OF_IP_ADDRESS);
    }
    else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
    {
        swdrvl3_host_entry.flags |= SWDRVL3_FLAG_IPV6;
        memcpy(&(swdrvl3_host_entry.ip_addr), &(host_entry->key_fields.dst_inet_addr.addr), SYS_TYPE_PACKET_LENGTH_OF_IP6_ADDRESS);
    }
    if(is_tagged)
        swdrvl3_host_entry.flags |= SWDRVL3_FLAG_TAGGED_EGRESS_VLAN;
    if(is_trunk)
        swdrvl3_host_entry.flags |= SWDRVL3_FLAG_TRUNK_EGRESS_PORT;

    swdrvl3_host_entry.fib_id = fib_id;
    memcpy(swdrvl3_host_entry.mac, host_entry->key_fields.dst_mac, sizeof(UI8_T) * SYS_ADPT_MAC_ADDR_LEN);
    swdrvl3_host_entry.hw_info = host_entry->hw_info;
    result = SWDRVL3_SetInetHostRoute(&swdrvl3_host_entry);

    /* added by steven.gao */
    if(result == SWDRVL3_L3_EXISTS || result == SWDRVL3_L3_NO_ERROR)
    {
        host_entry->in_chip_status = TRUE;
    }
    else
    {
        host_entry->in_chip_status = FALSE;
        host_entry->hw_info = L_CVRT_UINT_TO_PTR(SWDRVL3_HW_INFO_INVALID);
    }

    if (amtrl3_mgr_hostroute_performance_flag)
    {
        end_timetick = SYSFUN_GetSysTick();
        BACKDOOR_MGR_Printf("SWDRVL3_SetInetHostRoute takes %d ticks\n", (int)(end_timetick - begin_timetick));
    }

    if (host_entry->in_chip_status)
    {
        if ((amtrl3_mgr_debug_flag & DEBUG_LEVEL_TRACE_HOST) &&
            (amtrl3_mgr_debug_flag & DEBUG_LEVEL_DETAIL))
        {
            AMTRL3_MGR_Ntoa(&(host_entry->key_fields.dst_inet_addr), ip_str);
            BACKDOOR_MGR_Printf("AMTRL3_MGR_UpdateHostRouteToChip IP: %s hwinfo:%p\n", ip_str, host_entry->hw_info);
        }
        ret = AMTRL3_MGR_ADD_HOST_OK;
    }
    else
    {
        ret = AMTRL3_MGR_ADD_HOST_ERROR;
        if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
        {
            AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV4_HOST_ROUTE_IN_CHIP, 1);
            AMTRL3_OM_IncreaseDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV4_ADD_HOST_ROUTE_FAIL, 1);
        }
        else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
        {
            AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV6_HOST_ROUTE_IN_CHIP, 1);
            AMTRL3_OM_IncreaseDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV6_ADD_HOST_ROUTE_FAIL, 1);
        }
        if(amtrl3_mgr_debug_flag & DEBUG_LEVEL_ERROR)
        {
            AMTRL3_MGR_Ntoa(&(host_entry->key_fields.dst_inet_addr), ip_str);
            BACKDOOR_MGR_Printf("SWDRVL3_SetInetHostRoute Fails on IP: %s\n", ip_str);
        }
    }

    return ret;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_DeleteHostRouteFromChip
 * -------------------------------------------------------------------------
 * PURPOSE:  This function removes host route entry to swdrvl3 and determine
 *           host_route_entry in chip status base on operation result of
 *           driver layer.
 * INPUT:    action_flags: AMTRL3_TYPE_FLAGS_IPV4 \ AMTRL3_TYPE_FLAGS_IPV6
 *           fib_id       -- FIB id
 *           host_entry - contains host route information to set to chip
 * OUTPUT:   None.
 * RETURN:   TRUE
 *           FALSE
 * NOTES:    None
 * -------------------------------------------------------------------------*/
static BOOL_T AMTRL3_MGR_DeleteHostRouteFromChip(UI32_T action_flags, UI32_T fib_id, AMTRL3_OM_HostRouteEntry_T  *host_route_entry)
{
#if (SYS_CPNT_IP_TUNNEL == TRUE)
    if(IS_TUNNEL_IFINDEX(host_route_entry->key_fields.dst_vid_ifindex))
    {
         return AMTRL3_MGR_DeleteTunnelHostRouteFromChip(action_flags,fib_id, host_route_entry);
    }
#endif

    return AMTRL3_MGR_DeleteNonTunnelHostRouteFromChip(action_flags,fib_id, host_route_entry);
}

static BOOL_T AMTRL3_MGR_DeleteNonTunnelHostRouteFromChip(UI32_T action_flags, UI32_T fib_id, AMTRL3_OM_HostRouteEntry_T  *host_route_entry)
{
    /* Local Variable Declaration
     */
    UI8_T       ip_str[L_INET_MAX_IPADDR_STR_LEN] = {0};
    UI32_T      vid;
    SWDRVL3_Host_T  swdrvl3_host_entry;
    BOOL_T      ret = FALSE;
    /* BODY
     */
    if(host_route_entry == NULL)
        return FALSE;
    if(AMTRL3_MGR_GetFIBByID(fib_id) == NULL)
        return FALSE;

    if (amtrl3_mgr_debug_flag & DEBUG_LEVEL_TRACE_HOST)
    {
        AMTRL3_MGR_Ntoa(&(host_route_entry->key_fields.dst_inet_addr), ip_str);

        BACKDOOR_MGR_Printf("AMTRL3_MGR_DeleteHostRouteFromChip IP: %s, vlan: %d, port %d\n",
                ip_str, (int)host_route_entry->key_fields.dst_vid_ifindex, (int)host_route_entry->key_fields.lport);
    } /* end of if */

    /* Local host route */
    if((host_route_entry->entry_type == VAL_ipNetToPhysicalExtType_local) ||
       (host_route_entry->entry_type == VAL_ipNetToPhysicalExtType_vrrp))
    {
        if(CHECK_FLAG(amtrl3_chip_capability_flags, AMTRL3_CHIP_USE_LOCAL_HOST_TO_TRAP_MY_IP_PKTS))
            return AMTRL3_MGR_DeleteMyIpHostRouteFromChip(action_flags, fib_id, host_route_entry);
        else
            return TRUE;
    }

    if (host_route_entry->in_chip_status == FALSE)
    {
        if (amtrl3_mgr_debug_flag & DEBUG_LEVEL_ERROR)
        {
            BACKDOOR_MGR_Printf("AL3 Internal Error - Delete chip when it does not exist\n");
        }
        return TRUE;
    }

    memset(&swdrvl3_host_entry, 0 ,sizeof(SWDRVL3_Host_T));

    if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
    {
        swdrvl3_host_entry.flags |= SWDRVL3_FLAG_IPV4;
        memcpy(&(swdrvl3_host_entry.ip_addr), &(host_route_entry->key_fields.dst_inet_addr.addr), SYS_TYPE_PACKET_LENGTH_OF_IP_ADDRESS);
    }
    else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
    {
        swdrvl3_host_entry.flags |= SWDRVL3_FLAG_IPV6;
        memcpy(&(swdrvl3_host_entry.ip_addr), &(host_route_entry->key_fields.dst_inet_addr.addr), SYS_TYPE_PACKET_LENGTH_OF_IP6_ADDRESS);
    }
    swdrvl3_host_entry.fib_id = fib_id;
    swdrvl3_host_entry.hw_info = host_route_entry->hw_info;

    /* For Dynamic host entries
     */
    if (VLAN_OM_ConvertFromIfindex(host_route_entry->key_fields.dst_vid_ifindex,
                                &swdrvl3_host_entry.vid) == FALSE)
    {
        /* Cannot find sufficient port information   */
        return FALSE;
    }

    /* for link-local address, delete l3_egress object only, but not l3_entry*/
    if(host_route_entry->key_fields.dst_inet_addr.type== L_INET_ADDR_TYPE_IPV6Z)
        swdrvl3_host_entry.flags |= SWDRVL3_FLAG_PROCESS_L3_EGRESS_ONLY;

    ret = SWDRVL3_DeleteInetHostRoute(&swdrvl3_host_entry);

    if (!ret)
    {
        if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
            AMTRL3_OM_IncreaseDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV4_DEL_HOST_ROUTE_FAIL, 1);
        else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
            AMTRL3_OM_IncreaseDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV6_DEL_HOST_ROUTE_FAIL, 1);

        if (amtrl3_mgr_debug_flag & DEBUG_LEVEL_ERROR)
        {
            VLAN_OM_ConvertFromIfindex(host_route_entry->key_fields.dst_vid_ifindex, &vid);
            AMTRL3_MGR_Ntoa(&(host_route_entry->key_fields.dst_inet_addr), ip_str);
            BACKDOOR_MGR_Printf("SWDRVL3_DeleteHostRoute Fails: IP: %s, vid %d\n",
                   ip_str, (int)vid);
        }
        return FALSE;
    }
    else
    {
        host_route_entry->in_chip_status = FALSE;
        host_route_entry->hw_info = swdrvl3_host_entry.hw_info;
        if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
            AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV4_HOST_ROUTE_IN_CHIP, 1);
        else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
            AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV6_HOST_ROUTE_IN_CHIP, 1);
        return TRUE;
    } /* end of else */

    return FALSE;
} /* end of AMTRL3_MGR_DeleteHostRouteFromChip() */

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_ClearHostRouteHWInfo
 * -------------------------------------------------------------------------
 * PURPOSE:  Clear the host route table and egress table by hw_info.
 * INPUT:
 * OUTPUT:   none.
 * RETURN:   None
 * NOTES:
 * -------------------------------------------------------------------------*/
static void AMTRL3_MGR_ClearHostRouteHWInfo(UI32_T action_flags, UI32_T fib_id, AMTRL3_OM_HostRouteEntry_T *host_entry)
{
    /* LOCAL VARIABLE DECLARATION  */
    SWDRVL3_Host_T  swdrvl3_host_entry;

    /* BODY  */
    /* invalid means not in chip */
    if(host_entry->hw_info == NULL || host_entry->hw_info == L_CVRT_UINT_TO_PTR(AMTRL3_OM_HW_INFO_INVALID))
        return;

    memset(&swdrvl3_host_entry, 0, sizeof(SWDRVL3_Host_T));

    swdrvl3_host_entry.fib_id = fib_id;
    if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
        SET_FLAG(swdrvl3_host_entry.flags, SWDRVL3_FLAG_IPV4);
    else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
        SET_FLAG(swdrvl3_host_entry.flags, SWDRVL3_FLAG_IPV6);

    swdrvl3_host_entry.hw_info = host_entry->hw_info;
    SWDRVL3_ClearHostRouteHWInfo(&swdrvl3_host_entry);
    host_entry->hw_info = swdrvl3_host_entry.hw_info; /* should be AMTRL3_OM_HW_INFO_INVALID */

    return;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_AddNetRouteToChip
 * -------------------------------------------------------------------------
 * PURPOSE:  This function covers detail operation to set specific net route
 *           entry to driver and chip
 * INPUT:    action_flags: AMTRL3_TYPE_FLAGS_IPV4 \ AMTRL3_TYPE_FLAGS_IPV6 \
 *                         AMTRL3_TYPE_FLAGS_ECMP \ AMTRL3_TYPE_FLAGS_WCMP
 *           fib_id       -- FIB id
 *           net_entry - contains net route information to set to chip
 *           host_entry - contians gateway information for net route entry
 * OUTPUT:   none.
 * RETURN:   TRUE
 *           FALSE
 * NOTES:    action flag shall only be ipv4/ipv6 and ecmp/wcmp
 * -------------------------------------------------------------------------*/
static BOOL_T AMTRL3_MGR_AddNetRouteToChip(UI32_T action_flags,
                                           UI32_T fib_id,
                                           AMTRL3_OM_NetRouteEntry_T  *net_entry,
                                           AMTRL3_OM_HostRouteEntry_T *host_entry)
{
    /* Local Variable Declaration
     */
    UI32_T  ret = 0;
    BOOL_T  is_trunk, is_tagged, is_local, is_first, is_dynamic_tunnel, is_static_tunnel;
    UI8_T   ip_str[L_INET_MAX_IPADDR_STR_LEN] = {0};
    UI32_T  begin_timetick = 0, end_timetick = 0;
    SWDRVL3_Route_T  swdrvl3_net_entry;
    BOOL_T add_route = TRUE;

    /* BODY */
    is_trunk = is_tagged = is_local = is_first =is_dynamic_tunnel= is_static_tunnel =FALSE;
    memset(&swdrvl3_net_entry, 0, sizeof(SWDRVL3_Route_T));

    if(amtrl3_mgr_debug_flag)
        AMTRL3_MGR_Ntoa(&(net_entry->inet_cidr_route_entry.inet_cidr_route_dest), ip_str);

    if (amtrl3_mgr_debug_flag & DEBUG_LEVEL_TRACE_NET)
        BACKDOOR_MGR_Printf("AMTRL3_MGR_SetNetRouteToChip %s\n", ip_str);

    if (amtrl3_mgr_netroute_performance_flag)
        begin_timetick = SYSFUN_GetSysTick();

    if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
    {
        SET_FLAG(swdrvl3_net_entry.flags, SWDRVL3_FLAG_IPV4);
        memcpy(&(swdrvl3_net_entry.dst_ip), &(net_entry->inet_cidr_route_entry.inet_cidr_route_dest.addr), SYS_TYPE_PACKET_LENGTH_OF_IP_ADDRESS);
        if(CHECK_FLAG(amtrl3_chip_capability_flags, AMTRL3_CHIP_NEED_NH_IP_TO_PROGRAM_NET_ROUTE))
            memcpy(&(swdrvl3_net_entry.next_hop_ip), &(net_entry->inet_cidr_route_entry.inet_cidr_route_next_hop.addr), SYS_TYPE_PACKET_LENGTH_OF_IP_ADDRESS);
    }
    else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
    {
        SET_FLAG(swdrvl3_net_entry.flags, SWDRVL3_FLAG_IPV6);
        memcpy(&(swdrvl3_net_entry.dst_ip), &(net_entry->inet_cidr_route_entry.inet_cidr_route_dest.addr), SYS_TYPE_PACKET_LENGTH_OF_IP6_ADDRESS);
        if(CHECK_FLAG(amtrl3_chip_capability_flags, AMTRL3_CHIP_NEED_NH_IP_TO_PROGRAM_NET_ROUTE))
            memcpy(&(swdrvl3_net_entry.next_hop_ip), &(net_entry->inet_cidr_route_entry.inet_cidr_route_next_hop.addr), SYS_TYPE_PACKET_LENGTH_OF_IP6_ADDRESS);
    }

    swdrvl3_net_entry.fib_id = fib_id;
    swdrvl3_net_entry.prefix_length = net_entry->inet_cidr_route_entry.inet_cidr_route_pfxlen;
    swdrvl3_net_entry.hw_info = net_entry->hw_info;

    is_local = (net_entry->inet_cidr_route_entry.inet_cidr_route_proto == VAL_ipCidrRouteProto_local) ? TRUE : FALSE;

    if(!CHECK_FLAG(amtrl3_chip_capability_flags, AMTRL3_CHIP_SUPPORT_EGRESS_OBJECT))
    {
        if(CHECK_FLAG(amtrl3_chip_capability_flags, AMTRL3_CHIP_NEED_SRC_MAC_TO_PROGRM_NET_ROUTE))
            VLAN_PMGR_GetVlanMac(net_entry->inet_cidr_route_entry.inet_cidr_route_if_index, swdrvl3_net_entry.src_mac);
#if   (SYS_CPNT_IP_TUNNEL == TRUE)
        if(IS_TUNNEL_IFINDEX(net_entry->inet_cidr_route_entry.inet_cidr_route_if_index))
        {//use source vlan go get src_mac
            if(host_entry)
            {
                VLAN_PMGR_GetVlanMac(host_entry->key_fields.u.ip_tunnel.src_vidifindex, swdrvl3_net_entry.src_mac);
                if(amtrl3_mgr_debug_tunnel)
                    BACKDOOR_MGR_Printf("vlan=%ld, srcvlan=%ld\r\n", (long)net_entry->inet_cidr_route_entry.inet_cidr_route_if_index, (long)host_entry->key_fields.u.ip_tunnel.src_vidifindex);
            }
        }
#endif /*SYS_CPNT_IP_TUNNEL*/
        /* if mac_address is 0 or net entry is a local entry, some information can be irrelvant */
        if((host_entry == NULL) ||
           (memcmp(amtrl3_mgr_zero_mac, host_entry->key_fields.dst_mac, SYS_ADPT_MAC_ADDR_LEN) == 0) ||
            (net_entry->inet_cidr_route_entry.inet_cidr_route_proto == VAL_ipCidrRouteProto_local))
        {
            /* If mac is 0, we don't care about its unit and port.      */
            swdrvl3_net_entry.unit        = SYS_ADPT_DESTINATION_UNIT_UNKNOWN;        /* unit, don't care      */
            swdrvl3_net_entry.port        = SYS_ADPT_DESTINATION_PORT_UNKNOWN;        /* port, don't care      */
            swdrvl3_net_entry.trunk_id    = SYS_ADPT_DESTINATION_TRUNK_ID_UNKNOWN;    /* trunk_id, don't care  */
            is_trunk    = FALSE;                                    /* is_trunk, don't care  */
            is_tagged   = FALSE;                                    /* is_tagged, don't care */
            VLAN_OM_ConvertFromIfindex(net_entry->inet_cidr_route_entry.inet_cidr_route_if_index, &swdrvl3_net_entry.dst_vid);
            memcpy(swdrvl3_net_entry.nexthop_mac, swdrvl3_net_entry.src_mac, sizeof(UI8_T)*SYS_ADPT_MAC_ADDR_LEN);
        }
        else
        {
            if(AMTRL3_MGR_DeriveDetailPortInfo(net_entry->inet_cidr_route_entry.inet_cidr_route_if_index,
                                               host_entry->key_fields.lport,
                                               &swdrvl3_net_entry.dst_vid,
                                               &swdrvl3_net_entry.unit,
                                               &swdrvl3_net_entry.port,
                                               &swdrvl3_net_entry.trunk_id,
                                               &is_tagged, &is_trunk) == FALSE)
            {
                /* Cannot find sufficient port information to set to chip   */
                return FALSE;
            }
            memcpy(swdrvl3_net_entry.nexthop_mac, host_entry->key_fields.dst_mac, sizeof(UI8_T)*SYS_ADPT_MAC_ADDR_LEN);
        } /* end of else */

        if(is_tagged)
            SET_FLAG(swdrvl3_net_entry.flags, SWDRVL3_FLAG_TAGGED_EGRESS_VLAN);
        if(is_trunk)
            SET_FLAG(swdrvl3_net_entry.flags, SWDRVL3_FLAG_TRUNK_EGRESS_PORT);
    }
#if  (SYS_CPNT_IP_TUNNEL == TRUE)
        if(IS_TUNNEL_IFINDEX(net_entry->inet_cidr_route_entry.inet_cidr_route_if_index))
        {

            /* if this is dynamic tunnel need to trap packet to cpu,
             * should we add one tunnel entry type to net route entry ?*/
            if(host_entry==NULL)
                is_local = TRUE;
            else  if(host_entry->key_fields.tunnel_entry_type==AMTRL3_TYPE_INETTOPHYSICALEXTTYPE_TUNNEL_6TO4
                || host_entry->key_fields.tunnel_entry_type ==AMTRL3_TYPE_INETTOPHYSICALEXTTYPE_TUNNEL_ISATAP)
            {
                if(amtrl3_mgr_debug_tunnel)
                    BACKDOOR_MGR_Printf("This is dynamic tunnel\r\n");
                is_dynamic_tunnel =TRUE;
            }
            else if(host_entry->key_fields.tunnel_entry_type == AMTRL3_TYPE_INETTOPHYSICALEXTTYPE_TUNNEL_MANUAL)
            {
                if(amtrl3_mgr_debug_tunnel)
                    BACKDOOR_MGR_Printf("Manual tunnel\r\n");
                is_static_tunnel = TRUE;
            }
            else
            {
                 if(amtrl3_mgr_debug_tunnel)
                    BACKDOOR_MGR_Printf("invalid tunnel mode\r\n");
            }
        }
#endif

    /* distinguish which kind of route is */
    if(is_local)
    {
        if(is_static_tunnel)
        {
            /* static tunnel's local route should set next hop to tunnel host route */
            SET_FLAG(swdrvl3_net_entry.flags, SWDRVL3_FLAG_STATIC_TUNNEL);
            ret = SWDRVL3_AddInetNetRoute(&swdrvl3_net_entry, host_entry->hw_info);
        }
        else
        {
            /* Connected Route */
            SET_FLAG(swdrvl3_net_entry.flags, SWDRVL3_FLAG_LOCAL_CONNECTED_ROUTE);
            ret = SWDRVL3_AddInetNetRoute(&swdrvl3_net_entry, L_CVRT_UINT_TO_PTR(SWDRVL3_HW_INFO_INVALID));
        }
    }
    else if(is_dynamic_tunnel)
    {
        /* Dynamic tunnel route */

        /* if host entry's hwinfo is invalid, we set this net route as connected route */
        if(host_entry->hw_info == L_CVRT_UINT_TO_PTR(AMTRL3_OM_HW_INFO_INVALID))
        {
            /* should we set swdrvl3's flag to connected route ? */
            SET_FLAG(swdrvl3_net_entry.flags, SWDRVL3_FLAG_LOCAL_CONNECTED_ROUTE);
            ret = SWDRVL3_AddInetNetRoute(&swdrvl3_net_entry, L_CVRT_UINT_TO_PTR(SWDRVL3_HW_INFO_INVALID));
        }
        else
        {
            if (add_route == TRUE)
            {
                if(amtrl3_mgr_debug_tunnel)
                    BACKDOOR_MGR_Printf("To driver:%lx:%lx:%lx:%lx/%ld->%lx:%lx:%lx:%lx/vlan%ld,(%ld,%ld),%lx\r\n"
                                        ,L_INET_EXPAND_IPV6(swdrvl3_net_entry.dst_ip.ipv6_addr)
                                        ,(long)swdrvl3_net_entry.prefix_length
                                        ,L_INET_EXPAND_IPV6(swdrvl3_net_entry.next_hop_ip.ipv6_addr)
                                        ,(long)swdrvl3_net_entry.dst_vid
                                        ,(long)swdrvl3_net_entry.unit
                                        ,(long)swdrvl3_net_entry.port
                                        ,(uintptr_t)host_entry->hw_info);
                /* we set tunnel flag to dynamic tunnel net route */
                SET_FLAG(swdrvl3_net_entry.flags, SWDRVL3_FLAG_DYNAMIC_TUNNEL);
                ret = SWDRVL3_AddInetNetRoute(&swdrvl3_net_entry, host_entry->hw_info);
            }
        }
    }
    else
    {
        /* Dynamic Route
         * Apply chip capacity limits here.
         */
        if (net_entry->inet_cidr_route_entry.inet_cidr_route_proto !=
                                    VAL_ipCidrRouteProto_netmgmt)
        {
            if (AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_IPV4_DYNAMIC_ROUTE_IN_CHIP_COUNTS) >=
                        SYS_ADPT_MAX_NBR_OF_DYNAMIC_ROUTE_ENTRY)
            {
                add_route = FALSE;
                ret = SWDRVL3_L3_BUCKET_FULL;
            }
        }

        /* Static Route and the dynamic route which is not reach the limit */
        if (add_route == TRUE)
        {
            /* added by steven.gao */
            if ((amtrl3_mgr_debug_flag & DEBUG_LEVEL_TRACE_NET) ||
                (amtrl3_mgr_debug_flag & DEBUG_LEVEL_DETAIL))
            {
                BACKDOOR_MGR_Printf("SWDRVL3_AddInetNetRoute %s nexthop's hwinfo:%p\n", ip_str,
                    host_entry->hw_info);
            }
            if(amtrl3_mgr_debug_tunnel)
                BACKDOOR_MGR_Printf("To driver:%lx:%lx:%lx:%lx/%ld->%lx:%lx:%lx:%lx/vlan%ld,(%ld,%ld),%p\r\n"
                                    ,L_INET_EXPAND_IPV6(swdrvl3_net_entry.dst_ip.ipv6_addr)
                                    ,(long)swdrvl3_net_entry.prefix_length
                                    ,L_INET_EXPAND_IPV6(swdrvl3_net_entry.next_hop_ip.ipv6_addr)
                                    ,(long)swdrvl3_net_entry.dst_vid
                                    ,(long)swdrvl3_net_entry.unit
                                    ,(long)swdrvl3_net_entry.port
                                    ,host_entry->hw_info
                                );
            ret = SWDRVL3_AddInetNetRoute(&swdrvl3_net_entry, host_entry->hw_info);
        }
    }

    if (amtrl3_mgr_netroute_performance_flag)
    {
        end_timetick = SYSFUN_GetSysTick();
        BACKDOOR_MGR_Printf("SWDRVL3_AddNetRoute takes %d ticks\n", (int)(end_timetick - begin_timetick));
    }

    if ((amtrl3_mgr_debug_flag & DEBUG_LEVEL_SUPERNET) && (ret != SWDRVL3_L3_NO_ERROR))
    {
        /* modified by steven.gao for testing */
        BACKDOOR_MGR_Printf("AL3: SWDRVL3_AddNetRouteWithNextHopIp returns %s (%lu)\n",
               (ret == SWDRVL3_L3_BUCKET_FULL) ? "BUCKET_FULL" : "OTHERS", (unsigned long)ret);
    }

    /* modified by steven.gao */
    if ((ret == SWDRVL3_L3_NO_ERROR) || (ret == SWDRVL3_L3_EXISTS))
    {
        net_entry->hw_info = swdrvl3_net_entry.hw_info;
        net_entry->net_route_status = AMTRL3_OM_NET_ROUTE_READY;
        if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
        {
            if (net_entry->inet_cidr_route_entry.inet_cidr_route_proto ==
                            VAL_ipCidrRouteProto_other)
            {
                AMTRL3_OM_IncreaseDatabaseValue(fib_id, AMTRL3_OM_IPV4_DYNAMIC_ROUTE_IN_CHIP_COUNTS, 1);
            }

            AMTRL3_OM_IncreaseDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV4_NET_ROUTE_IN_CHIP, 1);
        }
        else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
            AMTRL3_OM_IncreaseDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV6_NET_ROUTE_IN_CHIP, 1);

        if (host_entry != NULL && ret == SWDRVL3_L3_NO_ERROR)
        {
            if (net_entry->hw_info != L_CVRT_UINT_TO_PTR(SWDRVL3_HW_INFO_INVALID) &&
                net_entry->hw_info != NULL)
            {
                AMTRL3_MGR_UpdateHostRouteRefCountInChip(action_flags, fib_id, host_entry, 1);
            }
        }
    }
    else if (ret == SWDRVL3_L3_BUCKET_FULL)
    {
        net_entry->hw_info = swdrvl3_net_entry.hw_info;
        /* If net_route_status is already in RESOLVED_STATE, it means that
         * this entry already exist in OM and compensateSuperNetRoutes operation
         * has already been processed. Therefor, it is unnecessary to go thru
         * the rest of operation again.
         */
        if (net_entry->net_route_status == AMTRL3_OM_NET_ROUTE_RESOLVED)
            return TRUE;

        net_entry->net_route_status = AMTRL3_OM_NET_ROUTE_RESOLVED;
        if (AMTRL3_MGR_CompensateSuperNetRoutes(action_flags, fib_id, net_entry) == FALSE)
        {
            if (amtrl3_mgr_debug_flag & DEBUG_LEVEL_ERROR)
                BACKDOOR_MGR_Printf("AMTRL3_MGR_CompensateSuperNetRoutes Fails\n");
        }

        if (amtrl3_mgr_debug_flag & DEBUG_LEVEL_ERROR)
        {
            BACKDOOR_MGR_Printf("SWDRVL3_AddNetRoute fails IP: %s, prefix: %d", ip_str, (int)net_entry->inet_cidr_route_entry.inet_cidr_route_pfxlen);
            BACKDOOR_MGR_Printf("GW MAC = %02x %02x %02x %02x %02x %02x\n", host_entry->key_fields.dst_mac[0], host_entry->key_fields.dst_mac[1],
                                                               host_entry->key_fields.dst_mac[2], host_entry->key_fields.dst_mac[3],
                                                               host_entry->key_fields.dst_mac[4], host_entry->key_fields.dst_mac[5]);
        }
    }
    else
        return FALSE;

    return TRUE;

} /* end of AMTRL3_MGR_SetNetRouteToChip() */

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_HotInsertAddNetRouteToModule
 * -------------------------------------------------------------------------
 * PURPOSE:  This function covers detail operation to set specific net route
 *           entry to driver and chip
 * INPUT:    action_flags: AMTRL3_TYPE_FLAGS_IPV4 \ AMTRL3_TYPE_FLAGS_IPV6 \
 *                         AMTRL3_TYPE_FLAGS_ECMP \ AMTRL3_TYPE_FLAGS_WCMP
 *           fib_id       -- FIB id
 *           net_entry - contains net route information to set to chip
 *           host_entry - contians gateway information for net route entry
 * OUTPUT:   none.
 * RETURN:   TRUE
 *           FALSE
 * NOTES:    action flag shall only be ipv4/ipv6 and ecmp/wcmp
 * -------------------------------------------------------------------------*/
static BOOL_T AMTRL3_MGR_HotInsertAddNetRouteToModule(UI32_T action_flags,
                                                      UI32_T fib_id,
                                                      AMTRL3_OM_NetRouteEntry_T  *net_entry,
                                                      AMTRL3_OM_HostRouteEntry_T *host_entry)
{
    /* Local Variable Declaration
     */
    UI32_T  ret;
    BOOL_T  is_trunk, is_tagged, is_local, is_first;
    UI8_T   ip_str[L_INET_MAX_IPADDR_STR_LEN] = {0};
    UI32_T  begin_timetick = 0, end_timetick = 0;
    SWDRVL3_Route_T  swdrvl3_net_entry;

    /* BODY */
    is_trunk = is_tagged = is_local = is_first = FALSE;
    memset(&swdrvl3_net_entry, 0, sizeof(SWDRVL3_Route_T));

    if(amtrl3_mgr_debug_flag)
        AMTRL3_MGR_Ntoa(&(net_entry->inet_cidr_route_entry.inet_cidr_route_dest), ip_str);

    if (amtrl3_mgr_debug_flag & DEBUG_LEVEL_TRACE_NET)
        BACKDOOR_MGR_Printf("AMTRL3_MGR_HotInsertAddNetRouteToModule %s\n", ip_str);

    if (amtrl3_mgr_netroute_performance_flag)
        begin_timetick = SYSFUN_GetSysTick();

    if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
    {
        SET_FLAG(swdrvl3_net_entry.flags, SWDRVL3_FLAG_IPV4);
        memcpy(&(swdrvl3_net_entry.dst_ip), &(net_entry->inet_cidr_route_entry.inet_cidr_route_dest.addr), SYS_TYPE_PACKET_LENGTH_OF_IP_ADDRESS);
        if(CHECK_FLAG(amtrl3_chip_capability_flags, AMTRL3_CHIP_NEED_NH_IP_TO_PROGRAM_NET_ROUTE))
            memcpy(&(swdrvl3_net_entry.next_hop_ip), &(net_entry->inet_cidr_route_entry.inet_cidr_route_next_hop.addr), SYS_TYPE_PACKET_LENGTH_OF_IP_ADDRESS);
    }
    else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
    {
        SET_FLAG(swdrvl3_net_entry.flags, SWDRVL3_FLAG_IPV6);
        memcpy(&(swdrvl3_net_entry.dst_ip), &(net_entry->inet_cidr_route_entry.inet_cidr_route_dest.addr), SYS_TYPE_PACKET_LENGTH_OF_IP6_ADDRESS);
        if(CHECK_FLAG(amtrl3_chip_capability_flags, AMTRL3_CHIP_NEED_NH_IP_TO_PROGRAM_NET_ROUTE))
            memcpy(&(swdrvl3_net_entry.next_hop_ip), &(net_entry->inet_cidr_route_entry.inet_cidr_route_next_hop.addr), SYS_TYPE_PACKET_LENGTH_OF_IP6_ADDRESS);
    }

    swdrvl3_net_entry.fib_id = fib_id;
    swdrvl3_net_entry.prefix_length = net_entry->inet_cidr_route_entry.inet_cidr_route_pfxlen;
    swdrvl3_net_entry.hw_info = net_entry->hw_info;

    is_local = (net_entry->inet_cidr_route_entry.inet_cidr_route_proto == VAL_ipCidrRouteProto_local) ? TRUE : FALSE;

    if(!CHECK_FLAG(amtrl3_chip_capability_flags, AMTRL3_CHIP_SUPPORT_EGRESS_OBJECT))
    {
        if(CHECK_FLAG(amtrl3_chip_capability_flags, AMTRL3_CHIP_NEED_SRC_MAC_TO_PROGRM_NET_ROUTE))
            VLAN_PMGR_GetVlanMac(net_entry->inet_cidr_route_entry.inet_cidr_route_if_index, swdrvl3_net_entry.src_mac);

        /* if mac_address is 0 or net entry is a local entry, some information can be irrelvant */
        if((memcmp(amtrl3_mgr_zero_mac, host_entry->key_fields.dst_mac, SYS_ADPT_MAC_ADDR_LEN) == 0) ||
            (net_entry->inet_cidr_route_entry.inet_cidr_route_proto == VAL_ipCidrRouteProto_local))
        {
            /* If mac is 0, we don't care about its unit and port.      */
            swdrvl3_net_entry.unit        = SYS_ADPT_DESTINATION_UNIT_UNKNOWN;        /* unit, don't care      */
            swdrvl3_net_entry.port        = SYS_ADPT_DESTINATION_PORT_UNKNOWN;        /* port, don't care      */
            swdrvl3_net_entry.trunk_id    = SYS_ADPT_DESTINATION_TRUNK_ID_UNKNOWN;    /* trunk_id, don't care  */
            is_trunk    = FALSE;                                    /* is_trunk, don't care  */
            is_tagged   = FALSE;                                    /* is_tagged, don't care */
            VLAN_OM_ConvertFromIfindex(net_entry->inet_cidr_route_entry.inet_cidr_route_if_index, &swdrvl3_net_entry.dst_vid);
            memcpy(swdrvl3_net_entry.nexthop_mac, swdrvl3_net_entry.src_mac, sizeof(UI8_T)*SYS_ADPT_MAC_ADDR_LEN);
        }
        else
        {
            if(AMTRL3_MGR_DeriveDetailPortInfo(net_entry->inet_cidr_route_entry.inet_cidr_route_if_index,
                                               host_entry->key_fields.lport,
                                               &swdrvl3_net_entry.dst_vid,
                                               &swdrvl3_net_entry.unit,
                                               &swdrvl3_net_entry.port,
                                               &swdrvl3_net_entry.trunk_id,
                                               &is_tagged, &is_trunk) == FALSE)
            {
                /* Cannot find sufficient port information to set to chip   */
                return FALSE;
            }
            memcpy(swdrvl3_net_entry.nexthop_mac, host_entry->key_fields.dst_mac, sizeof(UI8_T)*SYS_ADPT_MAC_ADDR_LEN);
        } /* end of else */

        if(is_tagged)
            SET_FLAG(swdrvl3_net_entry.flags, SWDRVL3_FLAG_TAGGED_EGRESS_VLAN);
        if(is_trunk)
            SET_FLAG(swdrvl3_net_entry.flags, SWDRVL3_FLAG_TRUNK_EGRESS_PORT);
    }

    if(is_local)
    {
        SET_FLAG(swdrvl3_net_entry.flags, SWDRVL3_FLAG_LOCAL_CONNECTED_ROUTE);
        ret = SWDRVL3_HotInsertAddInetNetRoute(&swdrvl3_net_entry, L_CVRT_UINT_TO_PTR(SWDRVL3_HW_INFO_INVALID));
    }
    else
    {
        ret = SWDRVL3_HotInsertAddInetNetRoute(&swdrvl3_net_entry, host_entry->hw_info);
    }

    if (amtrl3_mgr_netroute_performance_flag)
    {
        end_timetick = SYSFUN_GetSysTick();
        BACKDOOR_MGR_Printf("SWDRVL3_HotInsertAddInetNetRoute takes %d ticks\n", (int)(end_timetick - begin_timetick));
    }

    if ((ret != SWDRVL3_L3_NO_ERROR) && (amtrl3_mgr_debug_flag & DEBUG_LEVEL_TRACE_NET))
    {
        BACKDOOR_MGR_Printf("AL3: SWDRVL3_HotInsertAddInetNetRoute returns %s \n",
               (ret == SWDRVL3_L3_BUCKET_FULL) ? "BUCKET_FULL" : "OTHERS");
    }

    if (ret != SWDRVL3_L3_NO_ERROR)
        return FALSE;

    return TRUE;

} /* end of AMTRL3_MGR_SetNetRouteToChip() */

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_DeleteNetRouteFromChip
 * -------------------------------------------------------------------------
 * PURPOSE:  Delete Net route entry from chip
 * INPUT:    action_flags: AMTRL3_TYPE_FLAGS_IPV4 \ AMTRL3_TYPE_FLAGS_IPV6
 *           fib_id - point to the database of a FIB
 *           net_entry - Net route entry to remove from chip
 * OUTPUT:   none.
 * RETURN:   none
 * NOTES:    None
 * -------------------------------------------------------------------------*/
static BOOL_T AMTRL3_MGR_DeleteNetRouteFromChip(UI32_T action_flags,
                                                UI32_T fib_id,
                                                AMTRL3_OM_NetRouteEntry_T  *net_entry,
                                                AMTRL3_OM_HostRouteEntry_T *nhop_entry)
{
    /* LOCAL VARIABLE DECLARATION
     */
    SWDRVL3_Route_T  swdrvl3_net_entry;
    AMTRL3_OM_HostRouteEntry_T local_nhop_entry;
    BOOL_T      ret = FALSE;
    BOOL_T      is_local = FALSE;
    BOOL_T      is_last = FALSE;

    /* BODY
     */
    if(CHECK_FLAG(net_entry->flags, AMTRL3_TYPE_FLAGS_ECMP))
    {
        if(AMTRL3_MGR_IsLastPathOfECMPRouteInOm(action_flags, fib_id, net_entry))
            is_last = TRUE;
        return AMTRL3_MGR_DeleteECMPRouteOnePathFromChip(action_flags, fib_id, net_entry, nhop_entry, is_last);
    }

    memset(&swdrvl3_net_entry, 0, sizeof(SWDRVL3_Route_T));
    memset(&local_nhop_entry, 0, sizeof(AMTRL3_OM_HostRouteEntry_T));
    is_local = (net_entry->inet_cidr_route_entry.inet_cidr_route_proto == VAL_ipCidrRouteProto_local) ? TRUE : FALSE;

    if(CHECK_FLAG(amtrl3_chip_capability_flags, AMTRL3_CHIP_NEED_SRC_MAC_TO_PROGRM_NET_ROUTE))
        VLAN_PMGR_GetVlanMac(net_entry->inet_cidr_route_entry.inet_cidr_route_if_index, swdrvl3_net_entry.src_mac);
    swdrvl3_net_entry.fib_id = fib_id;
    swdrvl3_net_entry.prefix_length = net_entry->inet_cidr_route_entry.inet_cidr_route_pfxlen;
    if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
    {
        swdrvl3_net_entry.flags |= SWDRVL3_FLAG_IPV4;
        memcpy(&(swdrvl3_net_entry.dst_ip), &(net_entry->inet_cidr_route_entry.inet_cidr_route_dest.addr), SYS_TYPE_PACKET_LENGTH_OF_IP_ADDRESS);
        if(CHECK_FLAG(amtrl3_chip_capability_flags, AMTRL3_CHIP_NEED_NH_IP_TO_PROGRAM_NET_ROUTE))
            memcpy(&(swdrvl3_net_entry.next_hop_ip), &(net_entry->inet_cidr_route_entry.inet_cidr_route_next_hop.addr), SYS_TYPE_PACKET_LENGTH_OF_IP_ADDRESS);
    }
    else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
    {
        swdrvl3_net_entry.flags |= SWDRVL3_FLAG_IPV6;
        memcpy(&(swdrvl3_net_entry.dst_ip), &(net_entry->inet_cidr_route_entry.inet_cidr_route_dest.addr), SYS_TYPE_PACKET_LENGTH_OF_IP6_ADDRESS);
        if(CHECK_FLAG(amtrl3_chip_capability_flags, AMTRL3_CHIP_NEED_NH_IP_TO_PROGRAM_NET_ROUTE))
            memcpy(&(swdrvl3_net_entry.next_hop_ip), &(net_entry->inet_cidr_route_entry.inet_cidr_route_next_hop.addr), SYS_TYPE_PACKET_LENGTH_OF_IP6_ADDRESS);
    }

    if(is_local == TRUE)
    {
        swdrvl3_net_entry.flags |= SWDRVL3_FLAG_LOCAL_CONNECTED_ROUTE;
        if(TRUE!=(ret = SWDRVL3_DeleteInetNetRoute(&swdrvl3_net_entry, L_CVRT_UINT_TO_PTR(SWDRVL3_HW_INFO_INVALID))))
        {
            if(amtrl3_mgr_debug_tunnel)
                BACKDOOR_MGR_Printf("Fail to Delete Local route!");
        }
    }
    else
    {
        if(nhop_entry == NULL)
        {
            if (amtrl3_mgr_debug_flag & DEBUG_LEVEL_ERROR)
                BACKDOOR_MGR_Printf("Invalid input nexthop entry\n");
            return FALSE;
        }

        swdrvl3_net_entry.hw_info = net_entry->hw_info;
        if(net_entry->tunnel_entry_type == AMTRL3_TYPE_INETTOPHYSICALEXTTYPE_TUNNEL_6TO4)
            SET_FLAG(swdrvl3_net_entry.flags,SWDRVL3_FLAG_DYNAMIC_TUNNEL);

        if(TRUE!=(ret = SWDRVL3_DeleteInetNetRoute(&swdrvl3_net_entry, nhop_entry->hw_info)))
        {
            if(amtrl3_mgr_debug_tunnel)
                BACKDOOR_MGR_Printf("Fail to Delete net route!");
        }

        if (ret == TRUE)
        {
            if (net_entry->hw_info != L_CVRT_UINT_TO_PTR(SWDRVL3_HW_INFO_INVALID) &&
                net_entry->hw_info != NULL)
            {
                AMTRL3_MGR_UpdateHostRouteRefCountInChip(action_flags, fib_id, nhop_entry, -1);
            }
        }
    }

    if(ret)
    {
        if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
        {
            if (net_entry->inet_cidr_route_entry.inet_cidr_route_proto == VAL_ipCidrRouteProto_other)
            {
                AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_IPV4_DYNAMIC_ROUTE_IN_CHIP_COUNTS, 1);
            }

            AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV4_NET_ROUTE_IN_CHIP, 1);
        }
        else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
            AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV6_NET_ROUTE_IN_CHIP, 1);
    }
    else
    {
        if (amtrl3_mgr_debug_flag & DEBUG_LEVEL_ERROR)
            BACKDOOR_MGR_Printf("SWDRVL3_DeleteInetNetRoute Fails\n");
        return FALSE;
    }
    net_entry->hw_info = swdrvl3_net_entry.hw_info;

    return TRUE;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_AddECMPRouteOnePathToChip
 * -------------------------------------------------------------------------
 * PURPOSE:  Set one ecmp path to chip
 * INPUT:    action_flags: AMTRL3_TYPE_FLAGS_IPV4 \ AMTRL3_TYPE_FLAGS_IPV6 \
 *                         AMTRL3_TYPE_FLAGS_ECMP \ AMTRL3_TYPE_FLAGS_WCMP
 *           fib_id - FIB id
 *           net_entry - contains net route information to set to chip
 *           host_entry - contians gateway information for net route entry
 * OUTPUT:   none.
 * RETURN:   TRUE
 *           FALSE
 * NOTES:
 * -------------------------------------------------------------------------*/
static BOOL_T AMTRL3_MGR_AddECMPRouteOnePathToChip(UI32_T action_flags,
                                                   UI32_T fib_id,
                                                   AMTRL3_OM_NetRouteEntry_T  *net_entry,
                                                   AMTRL3_OM_HostRouteEntry_T *host_entry,
                                                   BOOL_T is_first)
{
    /* Local Variable Declaration
     */
    UI32_T  ret;
    BOOL_T  is_trunk, is_tagged;
    UI8_T   ip_str[L_INET_MAX_IPADDR_STR_LEN] = {0};
    UI32_T  begin_timetick = 0, end_timetick = 0;
    SWDRVL3_Route_T  swdrvl3_net_entry;
    BOOL_T add_ecmp = TRUE;

    /* BODY
     */
    is_trunk = is_tagged = FALSE;
    memset(&swdrvl3_net_entry, 0, sizeof(SWDRVL3_Route_T));

    if(amtrl3_mgr_debug_flag)
        AMTRL3_MGR_Ntoa(&(net_entry->inet_cidr_route_entry.inet_cidr_route_dest), ip_str);

    if (amtrl3_mgr_debug_flag & DEBUG_LEVEL_TRACE_NET)
    {
        UI8_T       nh_ip_str[L_INET_MAX_IPADDR_STR_LEN] = {0};

        AMTRL3_MGR_Ntoa(&(host_entry->key_fields.dst_inet_addr), nh_ip_str);
        BACKDOOR_MGR_Printf ("AMTRL3_MGR_AddECMPRouteOnePathToChip, network: %s nexthop:%s is_first:%s\n",
                ip_str, nh_ip_str, (is_first ? "TRUE":"FALSE"));
    }

    if (amtrl3_mgr_netroute_performance_flag)
        begin_timetick = SYSFUN_GetSysTick();

    if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
    {
        SET_FLAG(swdrvl3_net_entry.flags, SWDRVL3_FLAG_IPV4);
        memcpy(&(swdrvl3_net_entry.dst_ip), &(net_entry->inet_cidr_route_entry.inet_cidr_route_dest.addr), SYS_TYPE_PACKET_LENGTH_OF_IP_ADDRESS);
        memcpy(&(swdrvl3_net_entry.next_hop_ip), &(net_entry->inet_cidr_route_entry.inet_cidr_route_next_hop.addr), SYS_TYPE_PACKET_LENGTH_OF_IP_ADDRESS);
    }
    else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
    {
        SET_FLAG(swdrvl3_net_entry.flags, SWDRVL3_FLAG_IPV6);
        memcpy(&(swdrvl3_net_entry.dst_ip), &(net_entry->inet_cidr_route_entry.inet_cidr_route_dest.addr), SYS_TYPE_PACKET_LENGTH_OF_IP6_ADDRESS);
        memcpy(&(swdrvl3_net_entry.next_hop_ip), &(net_entry->inet_cidr_route_entry.inet_cidr_route_next_hop.addr), SYS_TYPE_PACKET_LENGTH_OF_IP6_ADDRESS);
    }
    SET_FLAG(swdrvl3_net_entry.flags, SWDRVL3_FLAG_ECMP);
    swdrvl3_net_entry.fib_id = fib_id;
    swdrvl3_net_entry.prefix_length = net_entry->inet_cidr_route_entry.inet_cidr_route_pfxlen;

    swdrvl3_net_entry.hw_info = net_entry->hw_info;

    if(!CHECK_FLAG(amtrl3_chip_capability_flags, AMTRL3_CHIP_SUPPORT_EGRESS_OBJECT))
    {
        VLAN_PMGR_GetVlanMac(net_entry->inet_cidr_route_entry.inet_cidr_route_if_index, swdrvl3_net_entry.src_mac);

        if(AMTRL3_MGR_DeriveDetailPortInfo(net_entry->inet_cidr_route_entry.inet_cidr_route_if_index,
                                           host_entry->key_fields.lport,
                                           &swdrvl3_net_entry.dst_vid,
                                           &swdrvl3_net_entry.unit,
                                           &swdrvl3_net_entry.port,
                                           &swdrvl3_net_entry.trunk_id,
                                           &is_tagged, &is_trunk) == FALSE)
        {
            /* Cannot find sufficient port information to set to chip   */
            return FALSE;
        }
        memcpy(swdrvl3_net_entry.nexthop_mac, host_entry->key_fields.dst_mac, sizeof(UI8_T)*SYS_ADPT_MAC_ADDR_LEN);

        if(is_tagged)
            SET_FLAG(swdrvl3_net_entry.flags, SWDRVL3_FLAG_TAGGED_EGRESS_VLAN);
        if(is_trunk)
            SET_FLAG(swdrvl3_net_entry.flags, SWDRVL3_FLAG_TRUNK_EGRESS_PORT);
    }

    if (is_first == TRUE)
    {
        if (net_entry->inet_cidr_route_entry.inet_cidr_route_proto !=
                    VAL_ipCidrRouteProto_netmgmt)
        {
            if (AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_IPV4_DYNAMIC_ROUTE_IN_CHIP_COUNTS) >=
                        SYS_ADPT_MAX_NBR_OF_DYNAMIC_ROUTE_ENTRY)
            {
                ret = SWDRVL3_L3_BUCKET_FULL;
                add_ecmp = FALSE;
            }
        }
    }

    if (add_ecmp == TRUE)
        ret = SWDRVL3_AddInetECMPRouteOnePath(&swdrvl3_net_entry, host_entry->hw_info, is_first);

    if (amtrl3_mgr_netroute_performance_flag)
    {
        end_timetick = SYSFUN_GetSysTick();
        BACKDOOR_MGR_Printf("SWDRVL3_AddNetRoute takes %d ticks\n", (int)(end_timetick - begin_timetick));
    }

    if ((amtrl3_mgr_debug_flag & DEBUG_LEVEL_SUPERNET) && (ret != SWDRVL3_L3_NO_ERROR))
    {
        BACKDOOR_MGR_Printf("AL3: SWDRVL3_AddInetECMPRouteOnePath returns %s \n",
               (ret == SWDRVL3_L3_BUCKET_FULL) ? "BUCKET_FULL" : "OTHERS");
    }

    if (ret == SWDRVL3_L3_NO_ERROR)
    {
        net_entry->hw_info = swdrvl3_net_entry.hw_info;

        net_entry->net_route_status = AMTRL3_OM_NET_ROUTE_READY;
        if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
            AMTRL3_OM_IncreaseDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV4_ECMP_NH_IN_CHIP, 1);
        else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
            AMTRL3_OM_IncreaseDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV6_ECMP_NH_IN_CHIP, 1);

        if(is_first)
        {
            if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
            {
                if (net_entry->inet_cidr_route_entry.inet_cidr_route_proto ==
                            VAL_ipCidrRouteProto_other)
                {
                    AMTRL3_OM_IncreaseDatabaseValue(fib_id, AMTRL3_OM_IPV4_DYNAMIC_ROUTE_IN_CHIP_COUNTS, 1);
                }

                AMTRL3_OM_IncreaseDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV4_ECMP_ROUTE_IN_CHIP, 1);
            }
            else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
                AMTRL3_OM_IncreaseDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV6_ECMP_ROUTE_IN_CHIP, 1);
        }

        AMTRL3_MGR_UpdateHostRouteRefCountInChip(action_flags, fib_id, host_entry, 1);
    }
    /* DEFIP table full and ecmp group table full all return SWDRVL3_L3_BUCKET_FULL */
    else if (ret == SWDRVL3_L3_BUCKET_FULL)
    {
        /* If net_route_status is already in RESOLVED_STATE, it means that
         * this entry already exist in OM and compensateSuperNetRoutes operation
         * has already been processed. Therefor, it is unnecessary to go thru
         * the rest of operation again.
         */
        if (net_entry->net_route_status == AMTRL3_OM_NET_ROUTE_RESOLVED)
            return TRUE;

        net_entry->net_route_status = AMTRL3_OM_NET_ROUTE_RESOLVED;
        if(AMTRL3_MGR_CompensateSuperNetRoutes(action_flags, fib_id, net_entry) == FALSE)
        {
            if (amtrl3_mgr_debug_flag & DEBUG_LEVEL_ERROR)
                BACKDOOR_MGR_Printf("AMTRL3_MGR_CompensateSuperNetRoutes Fails\n");
        }

        if(amtrl3_mgr_debug_flag & DEBUG_LEVEL_ERROR)
        {
            BACKDOOR_MGR_Printf("SWDRVL3_AddInetECMPRouteOnePath fails IP: %s, prefix: %d", ip_str, (int)net_entry->inet_cidr_route_entry.inet_cidr_route_pfxlen);
            BACKDOOR_MGR_Printf("GW MAC = %02x %02x %02x %02x %02x %02x\n", host_entry->key_fields.dst_mac[0], host_entry->key_fields.dst_mac[1],
                                                               host_entry->key_fields.dst_mac[2], host_entry->key_fields.dst_mac[3],
                                                               host_entry->key_fields.dst_mac[4], host_entry->key_fields.dst_mac[5]);
        }
    }
    else
        return FALSE;

    return TRUE;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_DeleteECMPRouteOnePathFromChip
 * -------------------------------------------------------------------------
 * PURPOSE:  Delete Net route entry from chip
 * INPUT:    action_flags: AMTRL3_TYPE_FLAGS_IPV4 \ AMTRL3_TYPE_FLAGS_IPV6
 *           fib_id - FIB id
 *           net_entry - Net route entry to remove from chip
 * OUTPUT:   none.
 * RETURN:   none
 * NOTES:    None
 * -------------------------------------------------------------------------*/
static BOOL_T AMTRL3_MGR_DeleteECMPRouteOnePathFromChip(UI32_T action_flags,
                                                        UI32_T fib_id,
                                                        AMTRL3_OM_NetRouteEntry_T  *net_entry,
                                                        AMTRL3_OM_HostRouteEntry_T *nhop_entry,
                                                        BOOL_T is_last)
{
    /* LOCAL VARIABLE DECLARATION
     */
    SWDRVL3_Route_T  swdrvl3_net_entry;
    AMTRL3_OM_HostRouteEntry_T local_nhop_entry;
    UI32_T      ret = 0;
    BOOL_T      host_ret = FALSE;
    BOOL_T      is_tagged = FALSE, is_trunk = FALSE;

    /* BODY
     */
    if (amtrl3_mgr_debug_flag & DEBUG_LEVEL_TRACE_NET)
    {
        UI8_T       ip_str[L_INET_MAX_IPADDR_STR_LEN] = {0};
        UI8_T       nh_ip_str[L_INET_MAX_IPADDR_STR_LEN] = {0};

        AMTRL3_MGR_Ntoa(&(net_entry->inet_cidr_route_entry.inet_cidr_route_dest), ip_str);
        AMTRL3_MGR_Ntoa(&(nhop_entry->key_fields.dst_inet_addr), nh_ip_str);
        BACKDOOR_MGR_Printf ("AMTRL3_MGR_DeleteECMPRouteOnePathFromChip, network: %s nexthop:%s is_last:%s\n",
                ip_str, nh_ip_str, (is_last ? "TRUE":"FALSE"));
    }

    memset(&swdrvl3_net_entry, 0, sizeof(SWDRVL3_Route_T));
    memset(&local_nhop_entry, 0, sizeof(AMTRL3_OM_HostRouteEntry_T));

    VLAN_PMGR_GetVlanMac(net_entry->inet_cidr_route_entry.inet_cidr_route_if_index, swdrvl3_net_entry.src_mac);
    swdrvl3_net_entry.fib_id = fib_id;
    swdrvl3_net_entry.prefix_length = net_entry->inet_cidr_route_entry.inet_cidr_route_pfxlen;
    if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
    {
        SET_FLAG(swdrvl3_net_entry.flags, SWDRVL3_FLAG_IPV4);
        memcpy(&(swdrvl3_net_entry.dst_ip), &(net_entry->inet_cidr_route_entry.inet_cidr_route_dest.addr), SYS_TYPE_PACKET_LENGTH_OF_IP_ADDRESS);
        memcpy(&(swdrvl3_net_entry.next_hop_ip), &(net_entry->inet_cidr_route_entry.inet_cidr_route_next_hop.addr), SYS_TYPE_PACKET_LENGTH_OF_IP_ADDRESS);
    }
    else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
    {
        SET_FLAG(swdrvl3_net_entry.flags, SWDRVL3_FLAG_IPV6);
        memcpy(&(swdrvl3_net_entry.dst_ip), &(net_entry->inet_cidr_route_entry.inet_cidr_route_dest.addr), SYS_TYPE_PACKET_LENGTH_OF_IP6_ADDRESS);
        memcpy(&(swdrvl3_net_entry.next_hop_ip), &(net_entry->inet_cidr_route_entry.inet_cidr_route_next_hop.addr), SYS_TYPE_PACKET_LENGTH_OF_IP6_ADDRESS);
    }

    /*Get next hop information for ECMP route*/
    SET_FLAG(swdrvl3_net_entry.flags, SWDRVL3_FLAG_ECMP);
    if(nhop_entry == NULL)
    {
        local_nhop_entry.key_fields.dst_vid_ifindex = net_entry->inet_cidr_route_entry.inet_cidr_route_if_index;
        local_nhop_entry.key_fields.dst_inet_addr = net_entry->inet_cidr_route_entry.inet_cidr_route_next_hop;
        host_ret = AMTRL3_OM_GetHostRouteEntry(action_flags, fib_id, &local_nhop_entry);
        if((host_ret == FALSE) || (local_nhop_entry.status == HOST_ROUTE_UNRESOLVED) || (local_nhop_entry.status == HOST_ROUTE_UNREFERENCE))
        {
            if(amtrl3_mgr_debug_flag & DEBUG_LEVEL_ERROR)
                BACKDOOR_MGR_Printf("AMTRL3_MGR_DeleteECMPRouteOnePathFromChip: ECMP route nexthop is not exist or unresolved\n");
            return FALSE;
        }
    }
    else
        local_nhop_entry = *nhop_entry;

    if(!CHECK_FLAG(amtrl3_chip_capability_flags, AMTRL3_CHIP_SUPPORT_EGRESS_OBJECT))
    {
        if (AMTRL3_MGR_DeriveDetailPortInfo(net_entry->inet_cidr_route_entry.inet_cidr_route_if_index,
                                         local_nhop_entry.key_fields.lport,
                                         &swdrvl3_net_entry.dst_vid,
                                         &swdrvl3_net_entry.unit,
                                         &swdrvl3_net_entry.port,
                                         &swdrvl3_net_entry.trunk_id,
                                         &is_tagged,&is_trunk) == FALSE)
        {
            /* Cannot find sufficient port information to set to chip */
            return FALSE;
        }
        memcpy(swdrvl3_net_entry.nexthop_mac, local_nhop_entry.key_fields.dst_mac,(sizeof(UI8_T)*SYS_ADPT_MAC_ADDR_LEN));
        if(is_tagged == TRUE)
            SET_FLAG(swdrvl3_net_entry.flags, SWDRVL3_FLAG_TAGGED_EGRESS_VLAN);

        if(CHECK_FLAG(amtrl3_chip_capability_flags, AMTRL3_CHIP_KEEP_TRUNK_ID_IN_HOST_ROUTE))
        {
            if(swdrvl3_net_entry.trunk_id != local_nhop_entry.key_fields.trunk_id)
            {
                /* If the type of old port and new port are not the same,
                 * we use the old one to delete. */
                is_trunk = !is_trunk;
                swdrvl3_net_entry.trunk_id = local_nhop_entry.key_fields.trunk_id;
            }
        }
        if(is_trunk == TRUE)
            SET_FLAG(swdrvl3_net_entry.flags, SWDRVL3_FLAG_TRUNK_EGRESS_PORT);
    }

    swdrvl3_net_entry.hw_info = net_entry->hw_info;
    ret = SWDRVL3_DeleteInetECMPRouteOnePath(&swdrvl3_net_entry, nhop_entry->hw_info, is_last);

    if (ret)
    {
        if (net_entry->hw_info != L_CVRT_UINT_TO_PTR(SWDRVL3_HW_INFO_INVALID) &&
            net_entry->hw_info != NULL)
        {
            AMTRL3_MGR_UpdateHostRouteRefCountInChip(action_flags, fib_id, nhop_entry, -1);
        }

        net_entry->hw_info = swdrvl3_net_entry.hw_info;
        if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
            AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV4_ECMP_NH_IN_CHIP, 1);
        else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
            AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV6_ECMP_NH_IN_CHIP, 1);

        if(is_last)/* last in om */
        {
            if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
            {
                if (net_entry->inet_cidr_route_entry.inet_cidr_route_proto == VAL_ipCidrRouteProto_other)
                {
                    AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_IPV4_DYNAMIC_ROUTE_IN_CHIP_COUNTS, 1);
                }

                AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV4_ECMP_ROUTE_IN_CHIP, 1);
            }
            else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
                AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV6_ECMP_ROUTE_IN_CHIP, 1);
        }
    }
    else
    {
        if (amtrl3_mgr_debug_flag & DEBUG_LEVEL_ERROR)
            BACKDOOR_MGR_Printf("SWDRVL3_DeleteInetECMPRouteOnePath Fails\n");
    }

    return TRUE;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_AddECMPRouteMultiPathToChip
 * -------------------------------------------------------------------------
 * PURPOSE:  Set multi ecmp path to chip
 * INPUT:    action_flags: AMTRL3_TYPE_FLAGS_IPV4 \ AMTRL3_TYPE_FLAGS_IPV6 \
 *                         AMTRL3_TYPE_FLAGS_ECMP \ AMTRL3_TYPE_FLAGS_WCMP
 *           fib_id - FIB id
 *           net_entry - contains net route information to set to chip
 *           host_entry - contians gateway information for net route entry
 * OUTPUT:   none.
 * RETURN:   TRUE
 *           FALSE
 * NOTES:    For FIB2 chip
 * -------------------------------------------------------------------------*/
static BOOL_T AMTRL3_MGR_AddECMPRouteMultiPathToChip(UI32_T action_flags,
                                                     UI32_T fib_id,
                                                     AMTRL3_OM_NetRouteEntry_T  *net_entry,
                                                     AMTRL3_OM_HostRouteEntry_T *host_entry,
                                                     UI32_T count)
{
    /* Local Variable Declaration
     */
    UI32_T  ret, i;
    UI8_T   ip_str[L_INET_MAX_IPADDR_STR_LEN] = {0};
    UI32_T  begin_timetick = 0,end_timetick = 0;
    SWDRVL3_Route_T  swdrvl3_net_entry;
    void*   nh_hw_info[AMTRL3_MGR_MAXIMUM_ECMP_PATH];
    BOOL_T add_ecmp = TRUE;

    /* BODY
     */
    memset(&swdrvl3_net_entry, 0, sizeof(SWDRVL3_Route_T));

    if(amtrl3_mgr_debug_flag)
        AMTRL3_MGR_Ntoa(&(net_entry[0].inet_cidr_route_entry.inet_cidr_route_dest), ip_str);

    if(amtrl3_mgr_debug_flag & DEBUG_LEVEL_TRACE_NET)
        BACKDOOR_MGR_Printf("%s, %s\n", __FUNCTION__, ip_str);

    if(amtrl3_mgr_netroute_performance_flag)
        begin_timetick = SYSFUN_GetSysTick();

    if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
    {
        SET_FLAG(swdrvl3_net_entry.flags, SWDRVL3_FLAG_IPV4);
        memcpy(&(swdrvl3_net_entry.dst_ip), &(net_entry[0].inet_cidr_route_entry.inet_cidr_route_dest.addr), SYS_TYPE_PACKET_LENGTH_OF_IP_ADDRESS);
    }
    else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
    {
        SET_FLAG(swdrvl3_net_entry.flags, SWDRVL3_FLAG_IPV6);
        memcpy(&(swdrvl3_net_entry.dst_ip), &(net_entry[0].inet_cidr_route_entry.inet_cidr_route_dest.addr), SYS_TYPE_PACKET_LENGTH_OF_IP6_ADDRESS);
    }
    SET_FLAG(swdrvl3_net_entry.flags, SWDRVL3_FLAG_ECMP);
    swdrvl3_net_entry.fib_id = fib_id;
    swdrvl3_net_entry.prefix_length = net_entry[0].inet_cidr_route_entry.inet_cidr_route_pfxlen;

    /* ecmp egress is INVALID, copy nh hw_info */
    for(i = 0; i < count; i++)
        nh_hw_info[i] = host_entry[i].hw_info;

    if (net_entry->inet_cidr_route_entry.inet_cidr_route_proto !=
                VAL_ipCidrRouteProto_netmgmt)
    {
        if (AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_IPV4_DYNAMIC_ROUTE_IN_CHIP_COUNTS) >=
                    SYS_ADPT_MAX_NBR_OF_DYNAMIC_ROUTE_ENTRY)
        {
            ret = SWDRVL3_L3_BUCKET_FULL;
            add_ecmp = FALSE;
        }
    }

    if (add_ecmp == TRUE)
        ret = SWDRVL3_AddInetECMPRouteMultiPath(&swdrvl3_net_entry, nh_hw_info, count);

    if(amtrl3_mgr_debug_flag & DEBUG_LEVEL_TRACE_NET)
        BACKDOOR_MGR_Printf("%s, %d, ret: %d, hw_info: %p\r\n", __FUNCTION__, __LINE__, ret, swdrvl3_net_entry.hw_info);

    if (amtrl3_mgr_netroute_performance_flag)
    {
        end_timetick = SYSFUN_GetSysTick();
        BACKDOOR_MGR_Printf("SWDRVL3_AddNetRoute takes %d ticks\n", (int)(end_timetick - begin_timetick));
    }

    if ((amtrl3_mgr_debug_flag & DEBUG_LEVEL_SUPERNET) && (ret != SWDRVL3_L3_NO_ERROR))
    {
        BACKDOOR_MGR_Printf("AL3: SWDRVL3_AddInetECMPRouteMultiPath returns %s \n",
               (ret == SWDRVL3_L3_BUCKET_FULL) ? "BUCKET_FULL" : "OTHERS");
    }

    if (ret == SWDRVL3_L3_NO_ERROR)
    {
        for(i = 0; i < count; i++)
        {
            net_entry[i].hw_info = swdrvl3_net_entry.hw_info;
            net_entry[i].net_route_status = AMTRL3_OM_NET_ROUTE_READY;

            AMTRL3_MGR_UpdateHostRouteRefCountInChip(action_flags, fib_id, &host_entry[i], 1);
        }

        if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
            AMTRL3_OM_IncreaseDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV4_ECMP_NH_IN_CHIP, count);
        else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
            AMTRL3_OM_IncreaseDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV6_ECMP_NH_IN_CHIP, count);

        if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
        {
            if (net_entry->inet_cidr_route_entry.inet_cidr_route_proto ==
                        VAL_ipCidrRouteProto_other)
            {
                AMTRL3_OM_IncreaseDatabaseValue(fib_id, AMTRL3_OM_IPV4_DYNAMIC_ROUTE_IN_CHIP_COUNTS, 1);
            }

            AMTRL3_OM_IncreaseDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV4_ECMP_ROUTE_IN_CHIP, 1);
        }
        else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
            AMTRL3_OM_IncreaseDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV6_ECMP_ROUTE_IN_CHIP, 1);
    }
    else if (ret == SWDRVL3_L3_BUCKET_FULL)
    {
        for(i = 0; i < count; i++)
            net_entry[i].net_route_status = AMTRL3_OM_NET_ROUTE_RESOLVED;

        if(AMTRL3_MGR_CompensateSuperNetRoutes(action_flags, fib_id, &net_entry[0]) == FALSE)
        {
            if(amtrl3_mgr_debug_flag & DEBUG_LEVEL_ERROR)
                BACKDOOR_MGR_Printf("AMTRL3_MGR_CompensateSuperNetRoutes Fails\n");
        }

        if(amtrl3_mgr_debug_flag & DEBUG_LEVEL_ERROR)
        {
            BACKDOOR_MGR_Printf("SWDRVL3_AddInetECMPRouteMultiPath fails IP: %s, prefix: %d", ip_str, (int)net_entry[0].inet_cidr_route_entry.inet_cidr_route_pfxlen);
        }
    }
    else
        return FALSE;

    return TRUE;

}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_HotInsertAddECMPRouteMultiPathToModule
 * -------------------------------------------------------------------------
 * PURPOSE:  Set multi ecmp path to chip in slaves
 * INPUT:    action_flags: AMTRL3_TYPE_FLAGS_IPV4 \ AMTRL3_TYPE_FLAGS_IPV6 \
 *                         AMTRL3_TYPE_FLAGS_ECMP \ AMTRL3_TYPE_FLAGS_WCMP
 *           fib_id - FIB id
 *           net_entry - contains net route information to set to chip
 *           host_entry - contians gateway information for net route entry
 * OUTPUT:   none.
 * RETURN:   TRUE
 *           FALSE
 * NOTES:    For FIB2 chip
 * -------------------------------------------------------------------------*/
static BOOL_T AMTRL3_MGR_HotInsertAddECMPRouteMultiPathToModule(UI32_T action_flags,
                                                          UI32_T fib_id,
                                                          AMTRL3_OM_NetRouteEntry_T  *net_entry,
                                                          AMTRL3_OM_HostRouteEntry_T *host_entry,
                                                          UI32_T count)
{
    UI32_T  ret, i;
    UI8_T   ip_str[L_INET_MAX_IPADDR_STR_LEN] = {0};
    SWDRVL3_Route_T  swdrvl3_net_entry;
    void*   nh_hw_info[AMTRL3_MGR_MAXIMUM_ECMP_PATH];

    memset(&swdrvl3_net_entry, 0, sizeof(SWDRVL3_Route_T));

    if(amtrl3_mgr_debug_flag)
        AMTRL3_MGR_Ntoa(&(net_entry[0].inet_cidr_route_entry.inet_cidr_route_dest), ip_str);

    if(amtrl3_mgr_debug_flag & DEBUG_LEVEL_TRACE_NET)
        BACKDOOR_MGR_Printf("AMTRL3_MGR_HotInsertAddECMPRouteMultiPathToModule %s\n", ip_str);

    if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
    {
        SET_FLAG(swdrvl3_net_entry.flags, SWDRVL3_FLAG_IPV4);
        memcpy(&(swdrvl3_net_entry.dst_ip), &(net_entry[0].inet_cidr_route_entry.inet_cidr_route_dest.addr), SYS_TYPE_PACKET_LENGTH_OF_IP_ADDRESS);
    }
    else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
    {
        SET_FLAG(swdrvl3_net_entry.flags, SWDRVL3_FLAG_IPV6);
        memcpy(&(swdrvl3_net_entry.dst_ip), &(net_entry[0].inet_cidr_route_entry.inet_cidr_route_dest.addr), SYS_TYPE_PACKET_LENGTH_OF_IP6_ADDRESS);
    }

    SET_FLAG(swdrvl3_net_entry.flags, SWDRVL3_FLAG_ECMP);
    swdrvl3_net_entry.fib_id = fib_id;
    swdrvl3_net_entry.prefix_length = net_entry[0].inet_cidr_route_entry.inet_cidr_route_pfxlen;

    for(i = 0; i < count; i++)
    {
        nh_hw_info[i] = host_entry[i].hw_info;
    }

    ret = SWDRVL3_HotInsertAddInetECMPRouteMultiPath(&swdrvl3_net_entry, (void*)nh_hw_info, count);

    if ((amtrl3_mgr_debug_flag & DEBUG_LEVEL_SUPERNET) && (ret != SWDRVL3_L3_NO_ERROR))
    {
        BACKDOOR_MGR_Printf("AL3: SWDRVL3_AddInetECMPRouteMultiPath returns %s \n",
               (ret == SWDRVL3_L3_BUCKET_FULL) ? "BUCKET_FULL" : "OTHERS");
    }

    if (ret == SWDRVL3_L3_NO_ERROR)
        return TRUE;
    else
        return FALSE;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_DeleteECMPRouteFromChip
 * -------------------------------------------------------------------------
 * PURPOSE:  Delete ECMP route from chip. (with all paths)
 * INPUT:    action_flags: AMTRL3_TYPE_FLAGS_IPV4 \ AMTRL3_TYPE_FLAGS_IPV6 \
 *                         AMTRL3_TYPE_FLAGS_ECMP \ AMTRL3_TYPE_FLAGS_WCMP
 *           fib_id - FIB id
 *           net_entry - contains net route information
 * OUTPUT:   none.
 * RETURN:   TRUE
 *           FALSE
 * NOTES:    For FIB2 chip
 * -------------------------------------------------------------------------*/
static BOOL_T AMTRL3_MGR_DeleteECMPRouteFromChip(UI32_T action_flags,
                                                 UI32_T fib_id,
                                                 AMTRL3_OM_NetRouteEntry_T *net_entry,
                                                 UI32_T active_count)
{
    /* LOCAL VARIABLE DECLARATION
     */
    SWDRVL3_Route_T  swdrvl3_net_entry;
    UI32_T      ret = 0;

    /* BODY
     */
    memset(&swdrvl3_net_entry, 0, sizeof(SWDRVL3_Route_T));

    swdrvl3_net_entry.fib_id = fib_id;
    swdrvl3_net_entry.prefix_length = net_entry->inet_cidr_route_entry.inet_cidr_route_pfxlen;
    if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
    {
        SET_FLAG(swdrvl3_net_entry.flags, SWDRVL3_FLAG_IPV4);
        memcpy(&(swdrvl3_net_entry.dst_ip), &(net_entry->inet_cidr_route_entry.inet_cidr_route_dest.addr), SYS_TYPE_PACKET_LENGTH_OF_IP_ADDRESS);
    }
    else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
    {
        SET_FLAG(swdrvl3_net_entry.flags, SWDRVL3_FLAG_IPV6);
        memcpy(&(swdrvl3_net_entry.dst_ip), &(net_entry->inet_cidr_route_entry.inet_cidr_route_dest.addr), SYS_TYPE_PACKET_LENGTH_OF_IP6_ADDRESS);
    }

    SET_FLAG(swdrvl3_net_entry.flags, SWDRVL3_FLAG_ECMP);
    swdrvl3_net_entry.hw_info = net_entry->hw_info;

    ret = SWDRVL3_DeleteInetECMPRoute(&swdrvl3_net_entry);

    if(ret)
    {
        net_entry->hw_info = swdrvl3_net_entry.hw_info;
        if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
            AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV4_ECMP_NH_IN_CHIP, active_count);
        else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
            AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV6_ECMP_NH_IN_CHIP, active_count);

        if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
        {
            if (net_entry->inet_cidr_route_entry.inet_cidr_route_proto == VAL_ipCidrRouteProto_other)
            {
                AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_IPV4_DYNAMIC_ROUTE_IN_CHIP_COUNTS, 1);
            }

            AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV4_ECMP_ROUTE_IN_CHIP, 1);
        }
        else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
            AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV6_ECMP_ROUTE_IN_CHIP, 1);
    }
    else
    {
        if(amtrl3_mgr_debug_flag & DEBUG_LEVEL_ERROR)
            BACKDOOR_MGR_Printf("AMTRL3_MGR_DeleteECMPRouteFromChip Fails\n");
    }

    return TRUE;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_ClearNetRouteHWInfo
 * -------------------------------------------------------------------------
 * PURPOSE:  Clear the net route DEFIP table and ecmp table(for ecmp only).
 * INPUT:
 * OUTPUT:   none.
 * RETURN:   None
 * NOTES:    Currentry, when delete ecmp route, the path may not in chip, but
 *           it's hw_info is not invalid. So, we need use its hw_info to clear
 *           DEFIP table and ecmp table.
 * -------------------------------------------------------------------------*/
static void AMTRL3_MGR_ClearNetRouteHWInfo(UI32_T action_flags, UI32_T fib_id, AMTRL3_OM_NetRouteEntry_T *net_entry)
{
    /* LOCAL VARIABLE DECLARATION  */
    SWDRVL3_Route_T  swdrvl3_net_entry;

    /* BODY  */
    /* invalid means has already cleared */
    if(net_entry->hw_info == L_CVRT_UINT_TO_PTR(AMTRL3_OM_HW_INFO_INVALID))
        return;

    memset(&swdrvl3_net_entry, 0, sizeof(SWDRVL3_Route_T));

    /* currently, only ecmp route need clear */
    if(!CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_ECMP))
        return;
    else
        SET_FLAG(swdrvl3_net_entry.flags, SWDRVL3_FLAG_ECMP);

     /* here should always be ECMP, and hw_info not invalid */
    if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_ECMP) &&
       (net_entry->hw_info != L_CVRT_UINT_TO_PTR(AMTRL3_OM_HW_INFO_INVALID)))
    {
        if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
            AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV4_ECMP_ROUTE_IN_CHIP, 1);
        else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
            AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV6_ECMP_ROUTE_IN_CHIP, 1);
    }

    swdrvl3_net_entry.fib_id = fib_id;
    swdrvl3_net_entry.prefix_length = net_entry->inet_cidr_route_entry.inet_cidr_route_pfxlen;
    if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
    {
        SET_FLAG(swdrvl3_net_entry.flags, SWDRVL3_FLAG_IPV4);
        memcpy(&(swdrvl3_net_entry.dst_ip), &(net_entry->inet_cidr_route_entry.inet_cidr_route_dest.addr), SYS_TYPE_PACKET_LENGTH_OF_IP_ADDRESS);
    }
    else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
    {
        SET_FLAG(swdrvl3_net_entry.flags, SWDRVL3_FLAG_IPV6);
        memcpy(&(swdrvl3_net_entry.dst_ip), &(net_entry->inet_cidr_route_entry.inet_cidr_route_dest.addr), SYS_TYPE_PACKET_LENGTH_OF_IP6_ADDRESS);
    }

    swdrvl3_net_entry.hw_info = net_entry->hw_info;
    /* djd: here dst ip & pfxlen need? Need confirm with driver layer. */
    SWDRVL3_ClearNetRouteHWInfo(&swdrvl3_net_entry);
    net_entry->hw_info = swdrvl3_net_entry.hw_info;
    AMTRL3_MGR_SyncHWInfoToAllECMPPath(action_flags, fib_id, net_entry);

    return;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_SyncHWInfoToAllECMPPath
 * -------------------------------------------------------------------------
 * PURPOSE:  Sync the ecmp egress to all paths (include all unresolved)
 * INPUT:
 * OUTPUT:   none.
 * RETURN:   None
 * NOTES:
 * -------------------------------------------------------------------------*/
static void AMTRL3_MGR_SyncHWInfoToAllECMPPath(UI32_T action_flags,
                                                    UI32_T fib_id,
                                                    AMTRL3_OM_NetRouteEntry_T *net_entry)
{
    UI32_T   i = 0, num_of_entry = 0;
    AMTRL3_OM_NetRouteEntry_T  net_entry_block[AMTRL3_MGR_MAXIMUM_ECMP_PATH];
    AMTRL3_OM_NetRouteEntry_T  *local_net_entry;

    memset(net_entry_block, 0, sizeof(AMTRL3_OM_NetRouteEntry_T)*AMTRL3_MGR_MAXIMUM_ECMP_PATH);
    local_net_entry = (AMTRL3_OM_NetRouteEntry_T*)net_entry_block;
    local_net_entry->inet_cidr_route_entry.inet_cidr_route_dest = net_entry->inet_cidr_route_entry.inet_cidr_route_dest;
    local_net_entry->inet_cidr_route_entry.inet_cidr_route_pfxlen = net_entry->inet_cidr_route_entry.inet_cidr_route_pfxlen;

    AMTRL3_OM_GetNextNNetRouteEntryByDstIp(action_flags,
                                           fib_id,
                                           AMTRL3_MGR_MAXIMUM_ECMP_PATH,
                                           &num_of_entry,
                                           net_entry_block);
    for(i = 0; i < num_of_entry; i++)
    {
        net_entry_block[i].hw_info = net_entry->hw_info;
        AMTRL3_OM_SetNetRouteEntry(action_flags, fib_id, &net_entry_block[i]);
    }
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_TimerEventHandler
 * -------------------------------------------------------------------------
 * PURPOSE:  Timer Event Handler : amtrl3 task timer event processing
 * INPUT:    fib_id - FIB id.
 * OUTPUT:   none.
 * RETURN:   TRUE  - All host entry has been processed.
 *           FALSE - Part of host table has not been scanned
 * NOTES:    Currently, we only provides functionality to update host route
 *           hit bit.
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_MGR_TimerEventHandler(UI32_T fib_id)
{
    /* Local Variable Declarations */
    UI32_T      num_of_entry_retrieved, process_count;
    UI32_T      hit = 0;
    BOOL_T      ret = FALSE;
    UI8_T       ip_str[L_INET_MAX_IPADDR_STR_LEN] = {0};
    AMTRL3_OM_HostRouteEntry_T  *local_host_entry;
    SWDRVL3_Host_T  swdrvl3_host_entry;
    UI32_T      action_flags, time_stamp;
    AMTRL3_MGR_FIB_T *mgr_fib = NULL;

    /* BODY */
    if((mgr_fib = AMTRL3_MGR_GetFIBByID(fib_id)) == NULL)
        return TRUE;

    if (amtrl3_mgr_hostroute_scanning_mode)
        return TRUE;

    if (amtrl3_mgr_debug_task  & TASK_DEBUG_HITBIT)
            BACKDOOR_MGR_Printf("AMTRL3_MGR_TimerEventHandler\n");

    AMTRL3_MGR_CHECK_OPERATION_MODE(FALSE);
    memset(mgr_fib->amtrl3_mgr_host_route_entry_block, 0, sizeof(AMTRL3_OM_HostRouteEntry_T) * AMTRL3_MGR_NUMBER_OF_HOST_ENTRY_TO_SCAN);

    /* Use GetNextN mechanism to get multiple number of record at once to improve
     * performance */
    local_host_entry = (AMTRL3_OM_HostRouteEntry_T*)(mgr_fib->amtrl3_mgr_host_route_entry_block);
    local_host_entry->key_fields.dst_inet_addr = mgr_fib->amtrl3_mgr_dstip_for_hitbit_scan;

    /* We currently only update host route timer hit bit in this event. */
    if (AMTRL3_OM_GetNextNHostRouteEntry(AMTRL3_TYPE_FLAGS_IPV4 | AMTRL3_TYPE_FLAGS_IPV6,
                                         fib_id,
                                         AMTRL3_MGR_NUMBER_OF_HOST_ENTRY_TO_SCAN,
                                         &num_of_entry_retrieved,
                                         mgr_fib->amtrl3_mgr_host_route_entry_block))
    {
        time_stamp = SYSFUN_GetSysTick() / SYS_BLD_TICKS_PER_SECOND;
        for (process_count = 0; process_count < num_of_entry_retrieved; process_count++)
        {
            local_host_entry = (AMTRL3_OM_HostRouteEntry_T *)(&(mgr_fib->amtrl3_mgr_host_route_entry_block[process_count]));

            if (local_host_entry->entry_type == VAL_ipNetToPhysicalExtType_local)
                continue;

            action_flags = 0;
            memset(&swdrvl3_host_entry, 0, sizeof(SWDRVL3_Host_T));
            if((local_host_entry->key_fields.dst_inet_addr.type== L_INET_ADDR_TYPE_IPV4) ||
               (local_host_entry->key_fields.dst_inet_addr.type== L_INET_ADDR_TYPE_IPV4Z))
            {
                action_flags |= AMTRL3_TYPE_FLAGS_IPV4;
                swdrvl3_host_entry.flags |= SWDRVL3_FLAG_IPV4;
                memcpy(&(swdrvl3_host_entry.ip_addr), &(local_host_entry->key_fields.dst_inet_addr.addr), SYS_TYPE_PACKET_LENGTH_OF_IP_ADDRESS);
            }
            else if((local_host_entry->key_fields.dst_inet_addr.type== L_INET_ADDR_TYPE_IPV6) ||
                    (local_host_entry->key_fields.dst_inet_addr.type== L_INET_ADDR_TYPE_IPV6Z))
            {
                action_flags |= AMTRL3_TYPE_FLAGS_IPV6;
                swdrvl3_host_entry.flags |= SWDRVL3_FLAG_IPV6;
                memcpy(&(swdrvl3_host_entry.ip_addr), &(local_host_entry->key_fields.dst_inet_addr.addr), SYS_TYPE_PACKET_LENGTH_OF_IP6_ADDRESS);
            }
            swdrvl3_host_entry.fib_id = fib_id;
            swdrvl3_host_entry.hw_info = local_host_entry->hw_info;

            if((local_host_entry->in_chip_status == TRUE) && (local_host_entry->key_fields.dst_inet_addr.type!= L_INET_ADDR_TYPE_IPV6Z))
            {
                if(SWDRVL3_ReadAndClearHostRouteEntryHitBit(&swdrvl3_host_entry, &hit))
                {
                    if (amtrl3_mgr_debug_task  & TASK_DEBUG_HITBIT)
                    {
                        AMTRL3_MGR_Ntoa(&(local_host_entry->key_fields.dst_inet_addr), ip_str);
                        BACKDOOR_MGR_Printf("AL3: Host entry  %s hit bit is %lu\n", ip_str, (unsigned long)hit);
                    }

                    if(hit)
                    {
                        UI32_T rc;
                        local_host_entry->hit_timestamp = time_stamp;
                        AMTRL3_OM_SetHostRouteEntry(action_flags, fib_id, local_host_entry);

                        /* we treat amtrl3 hit as "reachability confirmed by upper layer protocol", and
                        it is applied when ip_addr and h/w address of neigh entry are the same and in DELAY state */
                        rc =  SYSFUN_Syscall(SYSFUN_SYSCALL_AMTRL3_MGR,
                                K_AMTRL3_MGR_SYSCALL_CMD_HIT,
                                local_host_entry->key_fields.dst_inet_addr.addr,
                                L_CVRT_UINT_TO_PTR(local_host_entry->key_fields.dst_inet_addr.addrlen),
                                local_host_entry->key_fields.dst_mac,
                                L_CVRT_UINT_TO_PTR(local_host_entry->key_fields.dst_vid_ifindex));

                        continue;
                    }
                }
            }

            AMTRL3_MGR_CheckAndRemoveExpiredHostEntry(action_flags, fib_id, local_host_entry, time_stamp);
        }

        if (num_of_entry_retrieved == AMTRL3_MGR_NUMBER_OF_HOST_ENTRY_TO_SCAN)
        {
            mgr_fib->amtrl3_mgr_dstip_for_hitbit_scan = local_host_entry->key_fields.dst_inet_addr;
        }
        else
        {
            mgr_fib->amtrl3_mgr_dstip_for_hitbit_scan.type= L_INET_ADDR_TYPE_IPV4;
            memset(&(mgr_fib->amtrl3_mgr_dstip_for_hitbit_scan.addr), 0, sizeof(mgr_fib->amtrl3_mgr_dstip_for_hitbit_scan.addr));
        }
    }

    /* For dynamic tunnel, it will write net route to tunnel interface,
     * we can decide to delete it or not by periodically check its hit bit
     */
#if (SYS_CPNT_IP_TUNNEL == TRUE)
    {
        UI32_T num_of_net_entry_retrieved;
        AMTRL3_OM_NetRouteEntry_T  *local_net_entry;
        SWDRVL3_Route_T            swdrvl3_net_entry;
        UI32_T                     net_hit = 0;
        memset(mgr_fib->amtrl3_mgr_net_route_entry_block, 0, sizeof(AMTRL3_OM_NetRouteEntry_T) * AMTRL3_MGR_NUMBER_OF_NET_ENTRY_TO_SCAN);

        /* Use GetNextN mechanism to get multiple number of record at once to improve
         * performance */
        local_net_entry = (AMTRL3_OM_NetRouteEntry_T*)(mgr_fib->amtrl3_mgr_net_route_entry_block);
        local_net_entry->inet_cidr_route_entry.inet_cidr_route_if_index = SYS_ADPT_TUNNEL_1_IF_INDEX_NUMBER;   /* search from tunnel interface */
        local_net_entry->inet_cidr_route_entry.inet_cidr_route_dest = mgr_fib->amtrl3_mgr_tunnel_dstip_for_hitbit_scan; /* record the last hitbit scan net entry */

        if(AMTRL3_OM_GetNextNNetRouteEntryByIfIndex(AMTRL3_TYPE_FLAGS_IPV6,
                                                    fib_id,
                                                    AMTRL3_MGR_NUMBER_OF_NET_ENTRY_TO_SCAN,
                                                    &num_of_net_entry_retrieved,
                                                    local_net_entry))
        {
            time_stamp = SYSFUN_GetSysTick() / SYS_BLD_TICKS_PER_SECOND;

            for (process_count = 0; process_count < num_of_net_entry_retrieved; process_count++)
            {
                local_net_entry = (AMTRL3_OM_NetRouteEntry_T *)(&(mgr_fib->amtrl3_mgr_net_route_entry_block[process_count]));

                /* ISATAP not implement yet */
                if (local_net_entry->tunnel_entry_type != AMTRL3_TYPE_INETTOPHYSICALEXTTYPE_TUNNEL_6TO4)
                    continue;
                /* if its destination ifindex is not tunnel interface, do not process */
                if (!IS_TUNNEL_IFINDEX(local_net_entry->inet_cidr_route_entry.inet_cidr_route_if_index))
                    continue;
                /* if its destination address is not ipv6, do not process */
                if (local_net_entry->inet_cidr_route_entry.inet_cidr_route_dest.type != L_INET_ADDR_TYPE_IPV6)
                    continue;

                memset(&swdrvl3_net_entry, 0, sizeof(SWDRVL3_Route_T));

                swdrvl3_net_entry.flags |= SWDRVL3_FLAG_IPV6;
                memcpy(swdrvl3_net_entry.dst_ip.ipv6_addr, local_net_entry->inet_cidr_route_entry.inet_cidr_route_dest.addr, SYS_ADPT_IPV6_ADDR_LEN);

                swdrvl3_net_entry.fib_id = fib_id;
                swdrvl3_net_entry.prefix_length = local_net_entry->inet_cidr_route_entry.inet_cidr_route_pfxlen;

                if(SWDRVL3_ReadAndClearNetRouteHitBit(&swdrvl3_net_entry, &net_hit))
                {
                    UI32_T hit_bit=0;
                    hit_bit = 1 <<(amtrl3_my_unit_id-1);

                    if(net_hit)
                        local_net_entry->tunnel_hit |= hit_bit;
                    else
                        local_net_entry->tunnel_hit &= ~hit_bit;


                    /* if any of unit's hit bit is on, we reset the time stamp */
                    if(local_net_entry->tunnel_hit)
                    {
                        local_net_entry->tunnel_hit_timestamp = time_stamp;
                    }
                    AMTRL3_OM_SetNetRouteEntry(AMTRL3_TYPE_FLAGS_IPV6, fib_id, local_net_entry);
#if 0
                    if(net_hit)
                    {
                        UI32_T rc;
                        local_net_entry->tunnel_hit_timestamp = time_stamp;
                        AMTRL3_OM_SetNetRouteEntry(AMTRL3_TYPE_FLAGS_IPV6, fib_id, local_net_entry);

                        continue;
                    }
                    else
                    {
                        if (amtrl3_mgr_debug_task  & TASK_DEBUG_HITBIT)
                        {
                            AMTRL3_MGR_Ntoa(&(local_net_entry->inet_cidr_route_entry.inet_cidr_route_dest), ip_str);
                            BACKDOOR_MGR_Printf("AL3: Host entry  %s hit bit is ZERO\n", ip_str);
                        }
                    }
#endif
                }
                AMTRL3_MGR_CheckAndRemoveExpiredTunnelEntry(fib_id, local_net_entry, time_stamp);
            }

            if (num_of_net_entry_retrieved == AMTRL3_MGR_NUMBER_OF_NET_ENTRY_TO_SCAN)
            {
                mgr_fib->amtrl3_mgr_tunnel_dstip_for_hitbit_scan = local_net_entry->inet_cidr_route_entry.inet_cidr_route_dest;
            }
            else
            {
                mgr_fib->amtrl3_mgr_tunnel_dstip_for_hitbit_scan.type= L_INET_ADDR_TYPE_IPV4;
                memset(&(mgr_fib->amtrl3_mgr_tunnel_dstip_for_hitbit_scan.addr), 0, sizeof(mgr_fib->amtrl3_mgr_tunnel_dstip_for_hitbit_scan.addr));
            }
        }
    }
#endif

    ret =((mgr_fib->amtrl3_mgr_dstip_for_hitbit_scan.type== L_INET_ADDR_TYPE_IPV4) &&
          (AMTRL3_OM_IsAddressEqualZero(&(mgr_fib->amtrl3_mgr_dstip_for_hitbit_scan)))) ? TRUE : FALSE;

#if (SYS_CPNT_IP_TUNNEL == TRUE)
    ret &= ((mgr_fib->amtrl3_mgr_tunnel_dstip_for_hitbit_scan.type== L_INET_ADDR_TYPE_IPV4) &&
           (AMTRL3_OM_IsAddressEqualZero(&(mgr_fib->amtrl3_mgr_tunnel_dstip_for_hitbit_scan)))) ? TRUE : FALSE;
#endif

    return ret;
} /* end of AMTRL3_MGR_TimerEventHandler() */

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_CheckAndRemoveExpiredHostEntry
 * -------------------------------------------------------------------------
 * PURPOSE:  This function remove host entry from OM and chip if entry already
 *           passes ageout time.
 * INPUT:    action_flags: AMTRL3_TYPE_FLAGS_IPV4 \ AMTRL3_TYPE_FLAGS_IPV6
 *           fib_id - FIB id
 *           local_host_entry - given host route entry
 * OUTPUT:   None.
 * RETURN:   None
 * NOTES:
 * -------------------------------------------------------------------------*/
static void AMTRL3_MGR_CheckAndRemoveExpiredHostEntry(
                                        UI32_T action_flags,
                                        UI32_T fib_id,
                                        AMTRL3_OM_HostRouteEntry_T *local_host_entry,
                                        UI32_T current_time)
{
    UI8_T  ip_str[L_INET_MAX_IPADDR_STR_LEN] = {0};
    UI32_T amtrl3_mgr_host_entry_ageout_time = 0;

    /* BODY
     */
    if(local_host_entry == NULL)
        return;

    if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
        amtrl3_mgr_host_entry_ageout_time = AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_IPV4_HOST_ENTRY_AGEOUT_TIME);
    else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
        amtrl3_mgr_host_entry_ageout_time = AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_IPV6_HOST_ENTRY_AGEOUT_TIME);

    if(current_time - local_host_entry->hit_timestamp < amtrl3_mgr_host_entry_ageout_time)
        return;

    if(local_host_entry->ref_count)
        return;

    if(amtrl3_mgr_debug_task  & TASK_DEBUG_HITBIT)
    {
        AMTRL3_MGR_Ntoa(&(local_host_entry->key_fields.dst_inet_addr), ip_str);
        BACKDOOR_MGR_Printf("AMTRL3_MGR_CheckAndRemoveExpiredHostEntry %s\n", ip_str);
    }

    if(local_host_entry->entry_type == VAL_ipNetToPhysicalExtType_static)
        AMTRL3_MGR_HostRouteEventHandler(action_flags, fib_id, HOST_ROUTE_UNREFERENCE_EVENT, local_host_entry);
/* manual tunnel host route shouldn't be removed */

#if (SYS_CPNT_IP_TUNNEL == TRUE)
    else if(local_host_entry->key_fields.tunnel_entry_type == AMTRL3_TYPE_INETTOPHYSICALEXTTYPE_TUNNEL_MANUAL)
    {
        UI32_T nexthop_action_flags=0;
        AMTRL3_OM_HostRouteEntry_T nexthop;
        memset(&nexthop,0, sizeof(nexthop));
        switch(local_host_entry->key_fields.u.ip_tunnel.nexthop_inet_addr.type)
        {
            case L_INET_ADDR_TYPE_IPV4:
            case L_INET_ADDR_TYPE_IPV4Z:
                nexthop_action_flags = AMTRL3_TYPE_FLAGS_IPV4;
                break;
            case  L_INET_ADDR_TYPE_IPV6:
            case  L_INET_ADDR_TYPE_IPV6Z:
            default:
                nexthop_action_flags = AMTRL3_TYPE_FLAGS_IPV6;
                break;
        }

        nexthop.key_fields.dst_vid_ifindex = local_host_entry->key_fields.u.ip_tunnel.nexthop_vidifindex;
        nexthop.key_fields.dst_inet_addr = local_host_entry->key_fields.u.ip_tunnel.nexthop_inet_addr;
        if (AMTRL3_OM_GetHostRouteEntry(nexthop_action_flags,fib_id, &nexthop))
        {
            if(amtrl3_mgr_debug_tunnel)
                BACKDOOR_MGR_Printf("do not expire manual tunnel=>arp req send");
            IPAL_NEIGH_SendNeighborRequest(local_host_entry->key_fields.u.ip_tunnel.nexthop_vidifindex,&(local_host_entry->key_fields.u.ip_tunnel.nexthop_inet_addr));
        }
    }
    else if(local_host_entry->key_fields.tunnel_entry_type == AMTRL3_TYPE_INETTOPHYSICALEXTTYPE_TUNNEL_ISATAP ||
            local_host_entry->key_fields.tunnel_entry_type == AMTRL3_TYPE_INETTOPHYSICALEXTTYPE_TUNNEL_6TO4)
    {
        /* we shouldn't expire special tunnel host route ,
         * it will handle by AMTRL3_MGR_CheckAndRemoveExpiredTunnelEntry()
         */
    }
#endif
    else
        AMTRL3_MGR_HostRouteEventHandler(action_flags, fib_id, HOST_ROUTE_REMOVE_EVENT, local_host_entry);

    return;
} /* end of AMTRL3_MGR_CheckAndRemoveExpiredHostEntry() */

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_HostRouteEventHandler
 * -------------------------------------------------------------------------
 * PURPOSE:  Set host route status by using FSM
 * INPUT:    action_flags: AMTRL3_TYPE_FLAGS_IPV4 \ AMTRL3_TYPE_FLAGS_IPV6
 *           fib_id - FIB id
 *           host_route_event - action event occured for this host route entry
 *           rec              - host route entry record
 * OUTPUT:   none.
 * RETURN:   TRUE
 *           FALSE
 * NOTES:    action flag, ipv4 and ipv6 cannot be set at same time
 * -------------------------------------------------------------------------*/
static BOOL_T AMTRL3_MGR_HostRouteEventHandler(UI32_T action_flags, UI32_T fib_id,  UI32_T host_route_event,
                                               AMTRL3_OM_HostRouteEntry_T *host_route_entry)
{
    /* Local Variable Declaration
     */
    UI32_T      old_status, new_status = 0;
    BOOL_T      ret = FALSE;
    UI8_T       ip_str[L_INET_MAX_IPADDR_STR_LEN] = {0};
    AMTRL3_OM_HostRouteEntry_T old_host_entry;

    /* BODY
     */
    if((host_route_event < HOST_ROUTE_PARTIAL_EVENT) || (host_route_event > HOST_ROUTE_SYNC_COMPLETE_EVENT))
        return FALSE;

    if((host_route_entry->status < HOST_ROUTE_NOT_EXIST) || (host_route_entry->status > HOST_ROUTE_LAST_STATE))
        return FALSE;

    if(AMTRL3_MGR_GetFIBByID(fib_id) == NULL)
        return FALSE;

    action_flags = (action_flags & AMTRL3_TYPE_FLAGS_IPV4) | (action_flags & AMTRL3_TYPE_FLAGS_IPV6);

    host_route_entry->old_status = host_route_entry->status;
    old_status = host_route_entry->old_status;
    new_status = host_route_FSM[host_route_entry->status][host_route_event];

    if(amtrl3_mgr_debug_tunnel)
        BACKDOOR_MGR_Printf("event %ld for [%ld](%d)%lx:%lx:%lx:%lx, status=%d->%d, mac=%x-%x-%x-%x-%x-%x\r\n",
                        (long)host_route_event,
                        (long)host_route_entry->key_fields.dst_vid_ifindex,
                        host_route_entry->key_fields.dst_inet_addr.type,L_INET_EXPAND_IPV6(host_route_entry->key_fields.dst_inet_addr.addr),
                        host_route_entry->old_status,
                        new_status,
                        L_INET_EXPAND_MAC(host_route_entry->key_fields.dst_mac)
                        );

    if (amtrl3_mgr_debug_flag & DEBUG_LEVEL_DETAIL)
        AMTRL3_MGR_DebugFSM(action_flags, &host_route_event, host_route_entry->key_fields.dst_inet_addr, host_route_entry->status, new_status);

    switch(new_status)
    {
        case HOST_ROUTE_NOT_EXIST:
        case HOST_ROUTE_UNRESOLVED:
        case HOST_ROUTE_UNREFERENCE:
        case HOST_ROUTE_HOST_READY:
        case HOST_ROUTE_GATEWAY_READY:
            host_route_entry->status = new_status;
            /* Status of host route entry is not effected by current operation
             */
            break;

        case HOST_ROUTE_READY_NOT_SYNC:
            /* Notify TCP/IP if add a static ARP to replace a dynamic ARP */
            if(host_route_entry->entry_type == VAL_ipNetToPhysicalExtType_static)
                IPAL_NEIGH_AddNeighbor(
                             host_route_entry->key_fields.dst_vid_ifindex,
                             &(host_route_entry->key_fields.dst_inet_addr),
                             SYS_ADPT_MAC_ADDR_LEN,
                             host_route_entry->key_fields.dst_mac,
                             host_route_entry->entry_type,
                             TRUE);
            host_route_entry->status = new_status;
            /* Status of host route entry is not effected by current operation
             */
            break;

        case HOST_ROUTE_UNRESOLVED_1:
            if (host_route_entry->status == HOST_ROUTE_NOT_EXIST)
            {
                switch (host_route_entry->entry_type)
                {
                    case VAL_ipNetToPhysicalExtType_static:
                        if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
                        {
                            AMTRL3_OM_IncreaseDatabaseValue(fib_id, AMTRL3_OM_IPV4_STATIC_IPNET2PHYSICAL_COUNTS, 1);
                            AMTRL3_OM_IncreaseDatabaseValue(fib_id, AMTRL3_OM_IPV4_TOTAL_IPNET2PHYSICAL_COUNTS, 1);
                        }
                        else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
                        {
                            AMTRL3_OM_IncreaseDatabaseValue(fib_id, AMTRL3_OM_IPV6_STATIC_IPNET2PHYSICAL_COUNTS, 1);
                            AMTRL3_OM_IncreaseDatabaseValue(fib_id, AMTRL3_OM_IPV6_TOTAL_IPNET2PHYSICAL_COUNTS, 1);
                        }
                        break;

                    case VAL_ipNetToPhysicalExtType_dynamic:
                        if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
                        {
                            AMTRL3_OM_IncreaseDatabaseValue(fib_id, AMTRL3_OM_IPV4_DYNAMIC_IPNET2PHYSICAL_COUNTS, 1);
                            AMTRL3_OM_IncreaseDatabaseValue(fib_id, AMTRL3_OM_IPV4_TOTAL_IPNET2PHYSICAL_COUNTS, 1);
                        }
                        else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
                        {
                            AMTRL3_OM_IncreaseDatabaseValue(fib_id, AMTRL3_OM_IPV6_DYNAMIC_IPNET2PHYSICAL_COUNTS, 1);
                            AMTRL3_OM_IncreaseDatabaseValue(fib_id, AMTRL3_OM_IPV6_TOTAL_IPNET2PHYSICAL_COUNTS, 1);
                        }
                        break;

                    default:
                        break;
                }
            }

            host_route_entry->status = HOST_ROUTE_UNRESOLVED;
            break;

        case HOST_ROUTE_UNREFERENCE_1:
            AMTRL3_MGR_DeriveProperHostRouteStatus(host_route_event, host_route_entry);
#if (SYS_CPNT_IP_TUNNEL == TRUE)
                if(IS_TUNNEL_IFINDEX(host_route_entry->key_fields.dst_vid_ifindex))
                {
                    AMTRL3_MGR_RemoveTunnelNetRouteFromChipByGateway(  action_flags,   fib_id, host_route_entry);
                }

            AMTRL3_MGR_SetTunnelHostRouteEventByNexthopEntry(fib_id,host_route_entry,HOST_ROUTE_UNREFERENCE_EVENT);
#endif /*SYS_CPNT_IP_TUNNEL*/
#if (SYS_CPNT_VXLAN == TRUE)
            AMTRL3_MGR_SetVxlanTunnelByNexthopEvent(fib_id, host_route_entry, HOST_ROUTE_UNREFERENCE_EVENT);
#endif
            break;

        case HOST_ROUTE_UNREFERENCE_2:
            /* In Linux platform, first delete from TCP/IP */
            /* Note: static arp must always remain in kernel */
            if (host_route_entry->entry_type != VAL_ipNetToPhysicalExtType_static)
            {
                IPAL_NEIGH_DeleteNeighbor(host_route_entry->key_fields.dst_vid_ifindex,
                                &(host_route_entry->key_fields.dst_inet_addr));
            }
#if (SYS_CPNT_IP_TUNNEL == TRUE)
             AMTRL3_MGR_SetTunnelHostRouteEventByNexthopEntry(fib_id,host_route_entry,HOST_ROUTE_UNREFERENCE_EVENT);

            if(IS_TUNNEL_IFINDEX(host_route_entry->key_fields.dst_vid_ifindex))
            {
                if(amtrl3_mgr_debug_tunnel)
                    BACKDOOR_MGR_Printf("delete net route before host delete");
                AMTRL3_MGR_RemoveTunnelNetRouteFromChipByGateway(  action_flags,   fib_id, host_route_entry);
            }

#endif /*SYS_CPNT_IP_TUNNEL*/

#if (SYS_CPNT_VXLAN == TRUE)
            AMTRL3_MGR_SetVxlanTunnelByNexthopEvent(fib_id, host_route_entry, HOST_ROUTE_UNREFERENCE_EVENT);
#endif

            if (AMTRL3_MGR_DeleteHostRouteFromChip(action_flags, fib_id, host_route_entry))
            {
                AMTRL3_MGR_DeriveProperHostRouteStatus(host_route_event, host_route_entry);
            }
            else
            {
                AMTRL3_MGR_DeriveProperHostRouteStatus(HOST_ROUTE_UNREFERENCE_EVENT, host_route_entry);
            }

            break;
        case HOST_ROUTE_READY_1:
            if (host_route_entry->status == HOST_ROUTE_NOT_EXIST)
            {
                switch (host_route_entry->entry_type)
                {
                    case VAL_ipNetToPhysicalExtType_static:
                        if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
                        {
                            AMTRL3_OM_IncreaseDatabaseValue(fib_id, AMTRL3_OM_IPV4_STATIC_IPNET2PHYSICAL_COUNTS, 1);
                            AMTRL3_OM_IncreaseDatabaseValue(fib_id, AMTRL3_OM_IPV4_TOTAL_IPNET2PHYSICAL_COUNTS, 1);
                        }
                        else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
                        {
                            AMTRL3_OM_IncreaseDatabaseValue(fib_id, AMTRL3_OM_IPV6_STATIC_IPNET2PHYSICAL_COUNTS, 1);
                            AMTRL3_OM_IncreaseDatabaseValue(fib_id, AMTRL3_OM_IPV6_TOTAL_IPNET2PHYSICAL_COUNTS, 1);
                        }
                        break;
                    case VAL_ipNetToPhysicalExtType_dynamic:
                        if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
                        {
                            AMTRL3_OM_IncreaseDatabaseValue(fib_id, AMTRL3_OM_IPV4_DYNAMIC_IPNET2PHYSICAL_COUNTS, 1);
                            AMTRL3_OM_IncreaseDatabaseValue(fib_id, AMTRL3_OM_IPV4_TOTAL_IPNET2PHYSICAL_COUNTS, 1);
                        }
                        else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
                        {
                            AMTRL3_OM_IncreaseDatabaseValue(fib_id, AMTRL3_OM_IPV6_DYNAMIC_IPNET2PHYSICAL_COUNTS, 1);
                            AMTRL3_OM_IncreaseDatabaseValue(fib_id, AMTRL3_OM_IPV6_TOTAL_IPNET2PHYSICAL_COUNTS, 1);
                        }
                        break;

                    default:
                        break;
                }

#if (SYS_CPNT_IP_TUNNEL == TRUE)
                if(amtrl3_mgr_debug_tunnel)
                {
                    switch(host_route_entry->key_fields.tunnel_entry_type)
                    {
                        case AMTRL3_TYPE_INETTOPHYSICALEXTTYPE_TUNNEL_MANUAL:
                            BACKDOOR_MGR_Printf("Static tunnel ready!\r\n");
                            break;
                        case AMTRL3_TYPE_INETTOPHYSICALEXTTYPE_TUNNEL_6TO4:
                            BACKDOOR_MGR_Printf("6to4 tunnel ready!\r\n");
                            break;
                        case AMTRL3_TYPE_INETTOPHYSICALEXTTYPE_TUNNEL_ISATAP:
                            BACKDOOR_MGR_Printf("ISATAP tunnel ready!\r\n");
                            break;
                    }
                }
#endif
            }

            /* modified by steven.gao */
            if ((host_route_entry->hw_info == L_CVRT_UINT_TO_PTR(AMTRL3_OM_HW_INFO_INVALID)) ||
                (host_route_entry->hw_info == NULL))
                ret = AMTRL3_MGR_AddHostRouteToChip(action_flags, fib_id, host_route_entry);
            else
                ret = AMTRL3_MGR_UpdateHostRouteToChip(action_flags, fib_id, host_route_entry);

            if (ret == AMTRL3_MGR_ADD_HOST_OK)
            {

                /* derive proper host route status must before add tunnel net route and host route,
                 * or the host route status will be incorrect to cause infinite loop
                 */
                AMTRL3_MGR_DeriveProperHostRouteStatus(host_route_event, host_route_entry);
#if (SYS_CPNT_IP_TUNNEL == TRUE)
                AMTRL3_MGR_AddTunnelNetRouteToChipByGateway(  action_flags,   fib_id,   host_route_entry);
                AMTRL3_MGR_SetTunnelHostRouteEventByNexthopEntry(fib_id,host_route_entry,HOST_ROUTE_READY_EVENT);
#endif /*(SYS_CPNT_IP_TUNNEL == TRUE)*/
#if (SYS_CPNT_VXLAN == TRUE)
                AMTRL3_MGR_SetVxlanTunnelByNexthopEvent(fib_id, host_route_entry, HOST_ROUTE_READY_EVENT);
#endif
                /* Notify TCP/IP if add a static ARP */
                /*The ARP in TCP/IP stack is always set to static regardless of static ARP or dynamic ARP entry -- xiongyu 20081103*/
                if((host_route_entry->entry_type == VAL_ipNetToPhysicalExtType_static)||
                   (host_route_entry->entry_type == VAL_ipNetToPhysicalExtType_dynamic))
                {
                    IPAL_NEIGH_AddNeighbor(
                                 host_route_entry->key_fields.dst_vid_ifindex,
                                 &(host_route_entry->key_fields.dst_inet_addr),
                                 SYS_ADPT_MAC_ADDR_LEN,
                                 host_route_entry->key_fields.dst_mac,
                                 host_route_entry->entry_type,
                                 TRUE);
                }
            }
            else if (ret == AMTRL3_MGR_INSUFFICIENT_HOST_INFO)
            {
                AMTRL3_MGR_DeriveProperHostRouteStatus(HOST_ROUTE_UNREFERENCE_EVENT, host_route_entry);
#if (SYS_CPNT_IP_TUNNEL == TRUE)
                AMTRL3_MGR_SetTunnelHostRouteEventByNexthopEntry(fib_id,host_route_entry,HOST_ROUTE_UNREFERENCE_EVENT);
#endif
#if (SYS_CPNT_VXLAN == TRUE)
                AMTRL3_MGR_SetVxlanTunnelByNexthopEvent(fib_id, host_route_entry, HOST_ROUTE_UNREFERENCE_EVENT);
#endif
            }
            else
            {
                AMTRL3_MGR_DeriveProperHostRouteStatus(HOST_ROUTE_UNREFERENCE_EVENT, host_route_entry);
#if (SYS_CPNT_IP_TUNNEL == TRUE)
                AMTRL3_MGR_SetTunnelHostRouteEventByNexthopEntry(fib_id,host_route_entry,HOST_ROUTE_UNREFERENCE_EVENT);
#endif
#if (SYS_CPNT_VXLAN == TRUE)
                AMTRL3_MGR_SetVxlanTunnelByNexthopEvent(fib_id, host_route_entry, HOST_ROUTE_UNREFERENCE_EVENT);
#endif
            }

            break;
        case HOST_ROUTE_READY_2:
            memset(&old_host_entry, 0, sizeof(AMTRL3_OM_HostRouteEntry_T));
            old_host_entry.key_fields.dst_vid_ifindex = host_route_entry->key_fields.dst_vid_ifindex;
            old_host_entry.key_fields.dst_inet_addr = host_route_entry->key_fields.dst_inet_addr;
            AMTRL3_OM_GetHostRouteEntry(action_flags, fib_id, &old_host_entry);

            if((old_host_entry.key_fields.lport != host_route_entry->key_fields.lport) ||
               (memcmp(old_host_entry.key_fields.dst_mac, host_route_entry->key_fields.dst_mac, SYS_ADPT_MAC_ADDR_LEN) != 0))
            {
                if(CHECK_FLAG(amtrl3_chip_capability_flags, AMTRL3_CHIP_SUPPORT_EGRESS_OBJECT))
                {
                    /* When L2 info update, net route in chip has no need to re-program,
                                        so do not change the host route status to ready_not_sync */
                    if(amtrl3_mgr_debug_tunnel)
                    {
                        BACKDOOR_MGR_Printf("update2: vid %ld=> %ld\r\n", (long)old_host_entry.key_fields.dst_vid_ifindex, (long)host_route_entry->key_fields.dst_vid_ifindex);
                        BACKDOOR_MGR_Printf("update2: MAX %x.%x.%x.%x.%x.%x=> %x.%x.%x.%x.%x.%x\r\n",  L_INET_EXPAND_MAC(old_host_entry.key_fields.dst_mac), L_INET_EXPAND_MAC(host_route_entry->key_fields.dst_mac));
                    }
#if (SYS_CPNT_IP_TUNNEL == TRUE)
                    AMTRL3_MGR_SetTunnelHostRouteEventByNexthopEntry(fib_id,host_route_entry,HOST_ROUTE_PARTIAL_EVENT);
#endif
#if (SYS_CPNT_VXLAN == TRUE)
                    AMTRL3_MGR_SetVxlanTunnelByNexthopEvent(fib_id, host_route_entry, HOST_ROUTE_PARTIAL_EVENT);
#endif
                    AMTRL3_MGR_UpdateHostRouteToChip(action_flags, fib_id, host_route_entry);
                }
                else
                {
                    if((AMTRL3_MGR_DeleteHostRouteFromChip(action_flags, fib_id, &old_host_entry)) &&
                       (AMTRL3_MGR_AddHostRouteToChip(action_flags, fib_id, host_route_entry) != AMTRL3_MGR_ADD_HOST_ERROR))
                    {
                        AMTRL3_MGR_DeriveProperHostRouteStatus(host_route_event, host_route_entry);
#if (SYS_CPNT_IP_TUNNEL == TRUE)
                        AMTRL3_MGR_SetTunnelHostRouteEventByNexthopEntry(fib_id,host_route_entry,HOST_ROUTE_READY_EVENT);
#endif
#if (SYS_CPNT_VXLAN == TRUE)
                        AMTRL3_MGR_SetVxlanTunnelByNexthopEvent(fib_id, host_route_entry, HOST_ROUTE_READY_EVENT);
#endif
                    }
                    else
                    {
                        AMTRL3_MGR_DeriveProperHostRouteStatus(HOST_ROUTE_UNREFERENCE_EVENT, host_route_entry);
#if (SYS_CPNT_IP_TUNNEL == TRUE)
                        AMTRL3_MGR_SetTunnelHostRouteEventByNexthopEntry(fib_id,host_route_entry,HOST_ROUTE_UNREFERENCE_EVENT);
#endif
#if (SYS_CPNT_VXLAN == TRUE)
                        AMTRL3_MGR_SetVxlanTunnelByNexthopEvent(fib_id, host_route_entry, HOST_ROUTE_UNREFERENCE_EVENT);
#endif
                    }
                }
            }
#if (SYS_CPNT_VXLAN == TRUE)
            else
            {
                AMTRL3_MGR_SetVxlanTunnelByNexthopEvent(fib_id, host_route_entry, HOST_ROUTE_READY_EVENT);
            }
#endif

            /*When L2 info update,TCP/IP stack also need to update --xiongyu 20081104*/
            IPAL_NEIGH_AddNeighbor(host_route_entry->key_fields.dst_vid_ifindex,
                                 &(host_route_entry->key_fields.dst_inet_addr),
                                 SYS_ADPT_MAC_ADDR_LEN,
                                 host_route_entry->key_fields.dst_mac,
                                 host_route_entry->entry_type,
                                 TRUE);
#if (SYS_CPNT_IP_TUNNEL == TRUE)
            AMTRL3_MGR_AddTunnelNetRouteToChipByGateway(  action_flags,   fib_id,   host_route_entry);
#endif /*(SYS_CPNT_IP_TUNNEL == TRUE)*/
            break;
        case HOST_ROUTE_READY_3:
            AMTRL3_MGR_DeriveProperHostRouteStatus(host_route_event, host_route_entry);

            memset(&old_host_entry, 0, sizeof(AMTRL3_OM_HostRouteEntry_T));
            old_host_entry.key_fields.dst_vid_ifindex = host_route_entry->key_fields.dst_vid_ifindex;
            old_host_entry.key_fields.dst_inet_addr = host_route_entry->key_fields.dst_inet_addr;
            AMTRL3_OM_GetHostRouteEntry(action_flags, fib_id, &old_host_entry);
#if (SYS_CPNT_IP_TUNNEL == TRUE)
            AMTRL3_MGR_SetTunnelHostRouteEventByNexthopEntry(fib_id,host_route_entry,HOST_ROUTE_READY_EVENT);
#endif
#if (SYS_CPNT_VXLAN == TRUE)
            AMTRL3_MGR_SetVxlanTunnelByNexthopEvent(fib_id, host_route_entry, HOST_ROUTE_READY_EVENT);
#endif

            /* Notify TCP/IP if add a static ARP to replace a dynamic ARP */
            if((host_route_entry->entry_type == VAL_ipNetToPhysicalExtType_static) &&
               (old_host_entry.entry_type == VAL_ipNetToPhysicalExtType_dynamic))
                IPAL_NEIGH_AddNeighbor(
                             host_route_entry->key_fields.dst_vid_ifindex,
                             &(host_route_entry->key_fields.dst_inet_addr),
                             SYS_ADPT_MAC_ADDR_LEN,
                             host_route_entry->key_fields.dst_mac,
                             host_route_entry->entry_type,
                             TRUE);
            break;
        case HOST_ROUTE_REMOVE_1:
            /* Gateway cannot be removed
             */
            if (!host_route_entry->ref_count)
            {
                switch (host_route_entry->entry_type)
                {
                    case VAL_ipNetToPhysicalExtType_static:
                        if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
                        {
                            AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_IPV4_STATIC_IPNET2PHYSICAL_COUNTS, 1);
                            AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_IPV4_TOTAL_IPNET2PHYSICAL_COUNTS, 1);
                        }
                        else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
                        {
                            AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_IPV6_STATIC_IPNET2PHYSICAL_COUNTS, 1);
                            AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_IPV6_TOTAL_IPNET2PHYSICAL_COUNTS, 1);
                        }
                        break;

                    case VAL_ipNetToPhysicalExtType_dynamic:
                        if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
                        {
                            AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_IPV4_DYNAMIC_IPNET2PHYSICAL_COUNTS, 1);
                            AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_IPV4_TOTAL_IPNET2PHYSICAL_COUNTS, 1);
                        }
                        else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
                        {
                            AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_IPV6_DYNAMIC_IPNET2PHYSICAL_COUNTS, 1);
                            AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_IPV6_TOTAL_IPNET2PHYSICAL_COUNTS, 1);
                        }
                        break;

                    default:
                        break;
                }
                ret = AMTRL3_OM_DeleteHostRouteEntry(action_flags, fib_id, host_route_entry);
#if (SYS_CPNT_IP_TUNNEL == TRUE)
                if(amtrl3_mgr_debug_tunnel)
                    BACKDOOR_MGR_Printf("AMTRL3_OM_DeleteHostRouteEntry, event=%d", host_route_event);
                AMTRL3_MGR_SetTunnelHostRouteEventByNexthopEntry(fib_id,host_route_entry,HOST_ROUTE_UNREFERENCE_EVENT);
#endif
#if (SYS_CPNT_VXLAN == TRUE)
                AMTRL3_MGR_SetVxlanTunnelByNexthopEvent(fib_id, host_route_entry, HOST_ROUTE_UNREFERENCE_EVENT);
#endif

                /* After delete host from OM, if hw_info is not invalid, clear the hw_info in chip */
                AMTRL3_MGR_ClearHostRouteHWInfo(action_flags, fib_id, host_route_entry);
            }
            return ret;
            break;
        case HOST_ROUTE_REMOVE_2:
            if (!host_route_entry->ref_count)
            {
                /* In Linux platform, first delete from TCP/IP, then delete in chip */
                /* Skip the interface address (MY IP) and static arp
                 */
                if (host_route_entry->entry_type != VAL_ipNetToPhysicalExtType_local &&
                    host_route_entry->entry_type != VAL_ipNetToPhysicalExtType_static)
                {
                    IPAL_NEIGH_DeleteNeighbor(host_route_entry->key_fields.dst_vid_ifindex,
                                &(host_route_entry->key_fields.dst_inet_addr));
                }
#if (SYS_CPNT_IP_TUNNEL == TRUE)
                if(IS_TUNNEL_IFINDEX(host_route_entry->key_fields.dst_vid_ifindex))
                {
                    AMTRL3_MGR_RemoveTunnelNetRouteFromChipByGateway(  action_flags,   fib_id, host_route_entry);
                }

#endif /*SYS_CPNT_IP_TUNNEL*/

                if (AMTRL3_MGR_DeleteHostRouteFromChip(action_flags, fib_id, host_route_entry))
                {
#if (SYS_CPNT_IP_TUNNEL == TRUE)
                    AMTRL3_MGR_SetTunnelHostRouteEventByNexthopEntry(fib_id,host_route_entry,HOST_ROUTE_UNREFERENCE_EVENT);
#endif
#if (SYS_CPNT_VXLAN == TRUE)
                    AMTRL3_MGR_SetVxlanTunnelByNexthopEvent(fib_id, host_route_entry, HOST_ROUTE_UNREFERENCE_EVENT);
#endif

                    if (AMTRL3_OM_DeleteHostRouteEntry(action_flags, fib_id, host_route_entry))
                    {
                        switch (host_route_entry->entry_type)
                        {
                            case VAL_ipNetToPhysicalExtType_static:
                                if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
                                {
                                    AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_IPV4_STATIC_IPNET2PHYSICAL_COUNTS, 1);
                                    AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_IPV4_TOTAL_IPNET2PHYSICAL_COUNTS, 1);
                                }
                                else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
                                {
                                    AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_IPV6_STATIC_IPNET2PHYSICAL_COUNTS, 1);
                                    AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_IPV6_TOTAL_IPNET2PHYSICAL_COUNTS, 1);
                                }
                                break;

                            case VAL_ipNetToPhysicalExtType_dynamic:
                                if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
                                {
                                    AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_IPV4_DYNAMIC_IPNET2PHYSICAL_COUNTS, 1);
                                    AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_IPV4_TOTAL_IPNET2PHYSICAL_COUNTS, 1);
                                }
                                else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
                                {
                                    AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_IPV6_DYNAMIC_IPNET2PHYSICAL_COUNTS, 1);
                                    AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_IPV6_TOTAL_IPNET2PHYSICAL_COUNTS, 1);
                                }
                                break;

#if (SYS_CPNT_IP_TUNNEL == TRUE)
                            case VAL_ipNetToPhysicalExtType_other:
                                if(host_route_entry->key_fields.tunnel_entry_type == AMTRL3_TYPE_INETTOPHYSICALEXTTYPE_TUNNEL_6TO4)
                                {
                                    /*BACKDOOR_MGR_Printf("decrease 6to4 database value by 1\r\n");*/
                                    AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_IPV6_6to4_TUNNEL_HOST_ROUTE_COUNT, 1);
                                }
                                break;
#endif
                            default:
                                break;
                        }
                        /* After delete host from OM, if hw_info is not invalid, clear the hw_info in chip */
                        AMTRL3_MGR_ClearHostRouteHWInfo(action_flags, fib_id, host_route_entry);
                        return TRUE;
                    }
                    return FALSE;
                }
                else
                {
                    AMTRL3_MGR_DeriveProperHostRouteStatus(HOST_ROUTE_UNREFERENCE_EVENT, host_route_entry);
                }
            } /* end of if */

            break;
        default:
            break;
    } /* end of switch */

    if (amtrl3_mgr_debug_flag & DEBUG_LEVEL_ERROR)
    {
        if (host_route_entry->key_fields.lport != SYS_ADPT_DESTINATION_PORT_UNKNOWN &&
            host_route_entry->key_fields.lport > SYS_ADPT_TOTAL_NBR_OF_LPORT)
        {
            AMTRL3_MGR_Ntoa(&(host_route_entry->key_fields.dst_inet_addr), ip_str);
            BACKDOOR_MGR_Printf("Host Route entry contains wrong port info on IP: %s, lport: %d\n",
                   ip_str, (int)host_route_entry->key_fields.lport);
        }
    }

    if((host_route_entry->ref_count > 0) && (host_route_entry->old_status != HOST_ROUTE_NOT_EXIST))
        AMTRL3_MGR_HostRouteEventForNetRoute(action_flags, fib_id, host_route_entry);

    if((host_route_entry->ecmp_ref_count > 0) && (host_route_entry->old_status != HOST_ROUTE_NOT_EXIST))
        AMTRL3_MGR_HostRouteEventForECMPRoute(action_flags, fib_id, host_route_entry);

    /* If the host route is not in chip but it's hw_info still exist,
     * then delete the hw_info from chip
     */
    if(host_route_entry->in_chip_status == FALSE &&
       host_route_entry->hw_info != NULL &&
       host_route_entry->hw_info != L_CVRT_UINT_TO_PTR(AMTRL3_OM_HW_INFO_INVALID))
    {
        AMTRL3_MGR_ClearHostRouteHWInfo(action_flags, fib_id, host_route_entry);

        host_route_entry->ref_count_in_chip = 0;
    }

    /* For dynamic host route, if call AMTRL3_MGR_AddHostRouteToChip() return fail,
     * we need to remove it from OM, otherwise it will occupy an entry,
     * which as a result reduces available host route in OM
     */
    if ((new_status == HOST_ROUTE_READY_1 || new_status == HOST_ROUTE_READY_2) &&
         host_route_entry->status == HOST_ROUTE_UNREFERENCE &&
         host_route_entry->in_chip_status == FALSE &&
         host_route_entry->entry_type == VAL_ipNetToPhysicalExtType_dynamic)
    {
        if (CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
        {
            AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_IPV4_DYNAMIC_IPNET2PHYSICAL_COUNTS, 1);
            AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_IPV4_TOTAL_IPNET2PHYSICAL_COUNTS, 1);
        }
        else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
        {
            AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_IPV6_DYNAMIC_IPNET2PHYSICAL_COUNTS, 1);
            AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_IPV6_TOTAL_IPNET2PHYSICAL_COUNTS, 1);
        }

        AMTRL3_OM_DeleteHostRouteEntry(action_flags, fib_id, host_route_entry);
        ret = FALSE;
    }
    else
    {
        ret = AMTRL3_OM_SetHostRouteEntry(action_flags, fib_id, host_route_entry);
    }

#if (SYS_CPNT_PBR == TRUE)
    if (host_route_entry->pbr_ref_count > 0 &&
        old_status != host_route_entry->status)
    {
        BOOL_T pre_state = FALSE, post_state = FALSE;
        if (old_status == HOST_ROUTE_READY_NOT_SYNC ||
            old_status == HOST_ROUTE_HOST_READY ||
            old_status == HOST_ROUTE_GATEWAY_READY)
            pre_state = TRUE;

        if (host_route_entry->status == HOST_ROUTE_READY_NOT_SYNC ||
            host_route_entry->status == HOST_ROUTE_HOST_READY ||
            host_route_entry->status == HOST_ROUTE_GATEWAY_READY)
            post_state = TRUE;

        if (pre_state == FALSE && post_state == TRUE)
        {
            SYS_CALLBACK_MGR_HostRouteChanged(SYS_MODULE_AMTRL3, &host_route_entry->key_fields.dst_inet_addr, FALSE);
        }
        else if (pre_state == TRUE && post_state == FALSE)
        {
            SYS_CALLBACK_MGR_HostRouteChanged(SYS_MODULE_AMTRL3, &host_route_entry->key_fields.dst_inet_addr, TRUE);
        }
    }
#endif

    return ret;
} /* end of AMTRL3_MGR_HostRouteEventHandler() */

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_CompensateNetRouteEntry
 * -------------------------------------------------------------------------
 * PURPOSE:  Processing and Optimizing NetRoute Route into chip
 * INPUT:    fib_id - FIB id .
 * OUTPUT:   none.
 * RETURN:
 * NOTES:    In order to optimize the net route entry into chip.
 * -------------------------------------------------------------------------*/
void AMTRL3_MGR_CompensateNetRouteEntry(UI32_T fib_id)
{
    /* Local Variable Declaration
     */
    AMTRL3_OM_HostRouteEntry_T      local_host_entry;
    UI32_T      process_count, total_count;
    UI32_T      ret = 0;
    UI32_T      action_flags = 0;

    /* BODY
     */
    if(AMTRL3_MGR_GetFIBByID(fib_id) == NULL)
        return;

    memset(&local_host_entry, 0, sizeof(AMTRL3_OM_HostRouteEntry_T));
    process_count = total_count = 0;

    /* For net route entry whose nhop is associate with resolved_not_sync host entry
     * 1. Remove old entry from chip
     * 2. Update gateway mac and vlan info
     * 3. Insert to chip and OM
     */

    /* Check every net entry associated with Gateway located in resolved_not_sync list
     */
    while(AMTRL3_OM_GetNextResolvedNotSyncHostEntry(AMTRL3_TYPE_FLAGS_IPV4 | AMTRL3_TYPE_FLAGS_IPV6, fib_id, &local_host_entry))
    {
        action_flags = 0;
        if((local_host_entry.key_fields.dst_inet_addr.type== L_INET_ADDR_TYPE_IPV4) ||
           (local_host_entry.key_fields.dst_inet_addr.type== L_INET_ADDR_TYPE_IPV4Z))
           action_flags |= AMTRL3_TYPE_FLAGS_IPV4;
        else if((local_host_entry.key_fields.dst_inet_addr.type== L_INET_ADDR_TYPE_IPV6) ||
                (local_host_entry.key_fields.dst_inet_addr.type== L_INET_ADDR_TYPE_IPV6Z))
           action_flags |= AMTRL3_TYPE_FLAGS_IPV6;

        ret = AMTRL3_MGR_CompensateNetRouteByGateway(action_flags, fib_id, &local_host_entry, &process_count);

        total_count += process_count;

        if((ret == AMTRL3_MGR_SYNC_COMPLETE) || (ret == AMTRL3_MGR_NO_MORE_ENTRY))
        {
            AMTRL3_MGR_HostRouteEventHandler(action_flags, fib_id, HOST_ROUTE_SYNC_COMPLETE_EVENT, &local_host_entry);
        }

        if(total_count >= AMTRL3_MGR_NUMBER_OF_NET_ENTRY_TO_PROCESS)
            break;
    } /* end of while */
    return;

} /* end of AMTRL3_MGR_CompensateNetRouteEntry() */

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_CompensateNetRouteByGateway
 * -------------------------------------------------------------------------
 * PURPOSE:  Compensate net route entries whose next_hop is associated
 *           with the dst_ip of the given host route entry
 * INPUT:    action_flags: AMTRL3_TYPE_FLAGS_IPV4 \ AMTRL3_TYPE_FLAGS_IPV6
 *           fib_id       -- FIB id
 *           host_route_entry - given gateway information to check with net
 *           route entry
 * OUTPUT:   num_of_entry_processed - number of net entry successfully processed
 * RETURN:   AMTRL3_MGR_SYNC_COMPLETE   \
 *           AMTRL3_MGR_SYNC_INCOMPLETE \
 *           AMTRL3_MGR_NO_MORE_ENTRY
 * NOTES:    None
 * -------------------------------------------------------------------------*/
static UI32_T AMTRL3_MGR_CompensateNetRouteByGateway(UI32_T action_flags,
                                                     UI32_T fib_id,
                                                     AMTRL3_OM_HostRouteEntry_T *gateway_entry_p,
                                                     UI32_T   *num_of_entry_processed)
{
    /* Local Variable Declaration
     */
    AMTRL3_OM_NetRouteEntry_T  *local_net_entry;
    UI32_T      process_count, total_process_count;
    UI32_T      num_of_entry = 0;
    BOOL_T      sync_complete_flag, ret_local, ret;
    UI8_T       ip_str[L_INET_MAX_IPADDR_STR_LEN] = {0};
    AMTRL3_MGR_FIB_T *mgr_fib = NULL;
    BOOL_T is_first = FALSE;

    /* BODY
     */
    sync_complete_flag =  ret_local = ret = FALSE;
    total_process_count = 0;

    if((mgr_fib = AMTRL3_MGR_GetFIBByID(fib_id)) == NULL)
        return AMTRL3_MGR_NO_MORE_ENTRY;

    if (amtrl3_mgr_debug_task & TASK_DEBUG_COMPENSATE)
    {
        AMTRL3_MGR_Ntoa(&(gateway_entry_p->key_fields.dst_inet_addr), ip_str);
        BACKDOOR_MGR_Printf("AMTRL3_MGR_CompensateNetRouteByGateway %s\n", ip_str);
    } /* end of if */

    memset(mgr_fib->amtrl3_mgr_net_route_entry_block, 0, sizeof(AMTRL3_OM_NetRouteEntry_T)*AMTRL3_MGR_NUMBER_OF_NET_ENTRY_TO_SCAN);
    /* Use GetNextN mechanism to get multiple number of record at once to improve
     * performance
     */
    local_net_entry = (AMTRL3_OM_NetRouteEntry_T*)(mgr_fib->amtrl3_mgr_net_route_entry_block);
    local_net_entry->inet_cidr_route_entry.inet_cidr_route_next_hop = gateway_entry_p->key_fields.dst_inet_addr;

    while(!sync_complete_flag)
    {
        ret = AMTRL3_OM_GetNextNNetRouteEntryByNextHop(action_flags,
                                                       fib_id,
                                                       AMTRL3_MGR_NUMBER_OF_NET_ENTRY_TO_SCAN,
                                                       &num_of_entry,
                                                       mgr_fib->amtrl3_mgr_net_route_entry_block);
        if(ret != TRUE)
        {
            return AMTRL3_MGR_NO_MORE_ENTRY;
        }

        /* Process each entry individually
         */
        for(process_count = 0; process_count < num_of_entry; process_count++)
        {
            local_net_entry = (AMTRL3_OM_NetRouteEntry_T *)(&(mgr_fib->amtrl3_mgr_net_route_entry_block[process_count]));

            /* If net entry is associated with different gateway, do not process it
             */
            if(AMTRL3_OM_IsAddressEqual(&(local_net_entry->inet_cidr_route_entry.inet_cidr_route_next_hop),
                                         &(gateway_entry_p->key_fields.dst_inet_addr)) == FALSE)
            {
                sync_complete_flag = TRUE;
                break;
            }

            if(CHECK_FLAG(local_net_entry->flags, AMTRL3_TYPE_FLAGS_ECMP))
                SET_FLAG(action_flags, AMTRL3_TYPE_FLAGS_ECMP);

            /* Only remove net entry from chip if it exist in it
             */
            if(local_net_entry->net_route_status == AMTRL3_OM_NET_ROUTE_READY)
            {
                if(CHECK_FLAG(amtrl3_chip_capability_flags, AMTRL3_CHIP_SUPPORT_EGRESS_OBJECT))
                {
                    /* modified by steven.gao */
                    if (amtrl3_mgr_debug_task & TASK_DEBUG_COMPENSATE)
                    {
                        AMTRL3_MGR_Ntoa(&(local_net_entry->inet_cidr_route_entry.inet_cidr_route_dest), ip_str);
                        BACKDOOR_MGR_Printf("%s[%d] compensate route %s/%d hwinfo:%p\n", __FUNCTION__, __LINE__,
                                ip_str, (int)local_net_entry->inet_cidr_route_entry.inet_cidr_route_pfxlen,
                                local_net_entry->hw_info);
                    } /* end of if */

                    continue;
                }
                ret_local = AMTRL3_MGR_DeleteNetRouteFromChip(action_flags, fib_id, local_net_entry, gateway_entry_p);
            }

            local_net_entry->inet_cidr_route_entry.inet_cidr_route_if_index = gateway_entry_p->key_fields.dst_vid_ifindex;

            /* modified by steven.gao */
            if(gateway_entry_p->in_chip_status == TRUE)
            {
                if(CHECK_FLAG(local_net_entry->flags, AMTRL3_TYPE_FLAGS_ECMP))
                {
                    if(AMTRL3_MGR_IsFirstPathOfECMPRouteInChip(action_flags, fib_id, local_net_entry))
                        is_first = TRUE;
                    ret = AMTRL3_MGR_AddECMPRouteOnePathToChip(action_flags, fib_id, local_net_entry, gateway_entry_p, is_first);

                    if(is_first && (local_net_entry->net_route_status == AMTRL3_OM_NET_ROUTE_READY))
                        AMTRL3_MGR_SyncHWInfoToAllECMPPath(action_flags, fib_id, local_net_entry);
                }
                else
                {
                    ret = AMTRL3_MGR_AddNetRouteToChip(action_flags, fib_id, local_net_entry, gateway_entry_p);
                }

                if(!ret)
                    local_net_entry->net_route_status = AMTRL3_OM_NET_ROUTE_UNRESOLVED;
            }
            else
                local_net_entry->net_route_status = AMTRL3_OM_NET_ROUTE_UNRESOLVED;

            if(AMTRL3_OM_SetNetRouteEntry(action_flags, fib_id, local_net_entry) != TRUE)
            {
                if(amtrl3_mgr_debug_flag & DEBUG_LEVEL_ERROR)
                    BACKDOOR_MGR_Printf("AMTRL3_OM_SetNetRouteEntry\n");
            }
        } /* end of for */

        total_process_count += process_count;

        if(num_of_entry == AMTRL3_MGR_NUMBER_OF_NET_ENTRY_TO_SCAN)
        {
            if (num_of_entry != 1)
            {
                memcpy(&(mgr_fib->amtrl3_mgr_net_route_entry_block[0]),
                       &(mgr_fib->amtrl3_mgr_net_route_entry_block[num_of_entry-1]),
                       sizeof(AMTRL3_OM_NetRouteEntry_T));
            }
        }
        else
        {
            /* All net route entry has been processed
             */
            sync_complete_flag = TRUE;
        }
    } /* end of while */

    return AMTRL3_MGR_SYNC_COMPLETE;
} /* end of AMTRL3_MGR_CompensateNetRouteByGateway() */

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_CompensateResolvedNetRouteTable
 * -------------------------------------------------------------------------
 * PURPOSE:  Periodic compensate Resolved net entry in Resolved_table.
 * INPUT:    fib_id.
 * OUTPUT:   none.
 * RETURN:   none.
 * NOTES:    In order to optimize the net route entry into chip.
 * -------------------------------------------------------------------------
 */
void AMTRL3_MGR_CompensateResolvedNetRouteTable(UI32_T fib_id)
{
    /* Local Variable Declaration
     */
    UI32_T      process_count, current_prefix_length;
    UI8_T       ip_str[L_INET_MAX_IPADDR_STR_LEN] = {0};
    AMTRL3_OM_NetRouteStatus_T          previous_net_status, status_result = 0;
    AMTRL3_OM_ResolvedNetRouteEntry_T   resolved_net_entry;
    UI32_T      action_flags = 0;
    UI32_T      original_ipv4_resolved_net_route_count = 0, original_ipv6_resolved_net_route_count = 0;

    /* BODY
     */
    if(AMTRL3_MGR_GetFIBByID(fib_id) == NULL)
        return;

    /* There is no entry in resolved table waiting for process. */
    if(((original_ipv4_resolved_net_route_count = AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_IPV4_RESOLVED_NET_ROUTE_COUNT)) == 0) &&
       ((original_ipv6_resolved_net_route_count = AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_IPV6_RESOLVED_NET_ROUTE_COUNT)) == 0))
        return;

    if (amtrl3_mgr_debug_task & TASK_DEBUG_RESOLVED)
    {
        BACKDOOR_MGR_Printf ("\nAMTRL3_MGR_CompensateResolvedNetRouteTable");
    }

    previous_net_status = AMTRL3_OM_NET_ROUTE_READY;
    current_prefix_length = 0;
    process_count = 1;
    memset(&resolved_net_entry, 0, sizeof(AMTRL3_OM_ResolvedNetRouteEntry_T));

    /* Compensate Resolve Net entry action involes:
     * 1. Write Entry to chip
     * 2. Update net entry status from "resolved" to "ready"
     * 3. Remove Resolved entry from Resolved_Net_route_Table.
     */
    action_flags = AMTRL3_TYPE_FLAGS_IPV4;
    for (; process_count <= AMTRL3_MGR_NBR_OF_RESOLVED_NET_ENTRY_TO_PROCESS; process_count++)
    {
        if (!AMTRL3_OM_GetNextResolvedNetRouteEntry(action_flags, fib_id, &resolved_net_entry))
        {
            if(action_flags == AMTRL3_TYPE_FLAGS_IPV4)
            {
                previous_net_status = AMTRL3_OM_NET_ROUTE_READY;
                process_count = current_prefix_length = 0;
                memset(&resolved_net_entry, 0, sizeof(AMTRL3_OM_ResolvedNetRouteEntry_T));
                action_flags = AMTRL3_TYPE_FLAGS_IPV6;
                continue;
            }
            else
                break;
        }

        /* The purpose for this variable is to make sure that every net entry with
         * the same prefix length have equal opportunity to try to add to chip
         * in case of previous net entry fails.  Base on chip vendor's net table
         * hashing algorithm, 166.1.1.0/24 might not be able to add to net table
         * yet 192.168.1.0/24 could still success depends on availble bucket
         * entry.  Therefore, it is important to attempt to add every net entry
         * with the same prefix length to chip at least once.
         */
        if((process_count == 1) ||
           ((previous_net_status == AMTRL3_OM_NET_ROUTE_READY) &&
            (resolved_net_entry.inverse_prefix_length != current_prefix_length)))
        {
            current_prefix_length = resolved_net_entry.inverse_prefix_length;
        }
        else if((previous_net_status != AMTRL3_OM_NET_ROUTE_READY) &&
                (resolved_net_entry.inverse_prefix_length != current_prefix_length))
        {
            if(action_flags == AMTRL3_TYPE_FLAGS_IPV4)
            {
                previous_net_status = AMTRL3_OM_NET_ROUTE_READY;
                process_count = current_prefix_length = 0;
                memset(&resolved_net_entry, 0, sizeof(AMTRL3_OM_ResolvedNetRouteEntry_T));
                action_flags = AMTRL3_TYPE_FLAGS_IPV6;
                continue;
            }
            else
                break;
        }

        if(amtrl3_mgr_debug_task & TASK_DEBUG_RESOLVED)
        {
            AMTRL3_MGR_Ntoa(&(resolved_net_entry.inet_cidr_route_dest), ip_str);
            BACKDOOR_MGR_Printf("\nProcess IP:%s,  inverse_prefix_length:%d ",ip_str, (int)resolved_net_entry.inverse_prefix_length);
            AMTRL3_MGR_Ntoa(&(resolved_net_entry.inet_cidr_route_next_hop), ip_str);
            BACKDOOR_MGR_Printf("  NextHop: %s\n", ip_str);
        }

        if(!AMTRL3_MGR_CompensateResolvedNetRouteEntry(action_flags, fib_id, &resolved_net_entry, &status_result))
            break;

        if(status_result != AMTRL3_OM_NET_ROUTE_READY)
        {
            /* If chip is TCAM type, add one route fail, other routes with same prefix-len
               will always fail */
            if(CHECK_FLAG(amtrl3_chip_capability_flags, AMTRL3_CHIP_USE_TCAM_FOR_ROUTE_TABLE))
                break;
            else
                previous_net_status = status_result;
        }
    } /* end of for */

#if 0 /* According to current design, the default route has no special usage. No need to deal with it seperately. */
    /* == Below handle for default routes == */
    if((original_ipv4_resolved_net_route_count != 0) && (AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_IPV4_RESOLVED_NET_ROUTE_COUNT) == 0))
    {
        default_route_action_flags |= AMTRL3_TYPE_FLAGS_IPV4;
    }
    if((original_ipv6_resolved_net_route_count != 0) && (AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_IPV6_RESOLVED_NET_ROUTE_COUNT) == 0))
    {
        default_route_action_flags |= AMTRL3_TYPE_FLAGS_IPV6;
    }

    /* not compensate resolved routes or not compeleting compensate yet */
    if(default_route_action_flags == 0)
        return;

    /* default_route_action_flags != 0, must do something for default routes.
     * This means that there may be a user-configed default route entry
     * before chip table overflow and modify default route action.
     * Therefore, default route action shall be returned to user-configured action
     */
    if (((default_route_action_flags & AMTRL3_TYPE_FLAGS_IPV4) &&
         (AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_IPV4_SOFTWARE_FORWARDING_STATUS) == AMTRL3_TYPE_SOFTWARE_FORWARDING_ENABLE)) ||
        ((default_route_action_flags & AMTRL3_TYPE_FLAGS_IPV6) &&
         (AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_IPV6_SOFTWARE_FORWARDING_STATUS) == AMTRL3_TYPE_SOFTWARE_FORWARDING_ENABLE)))
    {
        AMTRL3_OM_NetRouteEntry_T net_route_default;
        AMTRL3_OM_HostRouteEntry_T nhop_entry;
        BOOL_T  ret;
        SWDRVL3_Route_T  swdrvl3_net_entry;

        if(default_route_action_flags & AMTRL3_TYPE_FLAGS_IPV4)
            AMTRL3_OM_SetDatabaseValue(fib_id, AMTRL3_OM_IPV4_DEFAULT_ROUTE_ACTION_TYPE, SYS_CPNT_DEFAULT_ROUTE_ACTION_ROUTE);
        if(default_route_action_flags & AMTRL3_TYPE_FLAGS_IPV6)
            AMTRL3_OM_SetDatabaseValue(fib_id, AMTRL3_OM_IPV6_DEFAULT_ROUTE_ACTION_TYPE, SYS_CPNT_DEFAULT_ROUTE_ACTION_ROUTE);

        /* first delete special default route if use default route trap to cpu */
        if(CHECK_FLAG(amtrl3_chip_capability_flags, AMTRL3_CHIP_USE_DEFAULT_ROUTE_TRAP_TO_CPU))
        {
            memset(&swdrvl3_net_entry, 0, sizeof(SWDRVL3_Route_T));
            swdrvl3_net_entry.fib_id = fib_id;
            swdrvl3_net_entry.unit = SYS_ADPT_DESTINATION_UNIT_UNKNOWN;
            swdrvl3_net_entry.port = SYS_ADPT_DESTINATION_PORT_UNKNOWN;
            swdrvl3_net_entry.trunk_id = SYS_ADPT_DESTINATION_TRUNK_ID_UNKNOWN;

            if(default_route_action_flags & AMTRL3_TYPE_FLAGS_IPV4)
            {
                swdrvl3_net_entry.flags |= SWDRVL3_FLAG_IPV4;
                ret = SWDRVL3_DeleteSpecialDefaultRoute(&swdrvl3_net_entry, SYS_CPNT_DEFAULT_ROUTE_ACTION_TRAP2CPU);
                if(ret)
                    AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV4_NET_ROUTE_IN_CHIP, 1);
            }
            if(default_route_action_flags & AMTRL3_TYPE_FLAGS_IPV6)
            {
                swdrvl3_net_entry.flags = 0;
                swdrvl3_net_entry.flags |= SWDRVL3_FLAG_IPV6;
                ret = SWDRVL3_DeleteSpecialDefaultRoute(&swdrvl3_net_entry, SYS_CPNT_DEFAULT_ROUTE_ACTION_TRAP2CPU);
                if(ret)
                    AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV6_NET_ROUTE_IN_CHIP, 1);
            }
        }

        memset(&net_route_default, 0, sizeof(AMTRL3_OM_NetRouteEntry_T));
        if(default_route_action_flags & AMTRL3_TYPE_FLAGS_IPV4)
        {
            action_flags = AMTRL3_TYPE_FLAGS_IPV4;
            net_route_default.inet_cidr_route_entry.inet_cidr_route_dest.type= L_INET_ADDR_TYPE_IPV4;
        }
        else
        {
            action_flags = AMTRL3_TYPE_FLAGS_IPV6;
            net_route_default.inet_cidr_route_entry.inet_cidr_route_dest.type= L_INET_ADDR_TYPE_IPV6;
        }

        /* try to program default route (status = resolved) to chip */
        while(AMTRL3_OM_GetNextNetRouteEntry(action_flags, fib_id, &net_route_default) == TRUE)
        {
            if((AMTRL3_OM_IsAddressEqualZero(&(net_route_default.inet_cidr_route_entry.inet_cidr_route_dest)) == FALSE) ||
                (net_route_default.inet_cidr_route_entry.inet_cidr_route_pfxlen != 0))
            {
                if((action_flags == AMTRL3_TYPE_FLAGS_IPV4) &&
                   (default_route_action_flags & AMTRL3_TYPE_FLAGS_IPV6))
                {
                    memset(&net_route_default, 0, sizeof(AMTRL3_OM_NetRouteEntry_T));
                    action_flags = AMTRL3_TYPE_FLAGS_IPV6;
                    net_route_default.inet_cidr_route_entry.inet_cidr_route_dest.type= L_INET_ADDR_TYPE_IPV6;
                    continue;
                }
                else
                    break;
            }
            if(net_route_default.net_route_status != AMTRL3_OM_NET_ROUTE_RESOLVED)
                continue;

            memset(&nhop_entry, 0, sizeof(AMTRL3_OM_HostRouteEntry_T));
            nhop_entry.key_fields.dst_inet_addr = net_route_default.inet_cidr_route_entry.inet_cidr_route_next_hop;
            ret = AMTRL3_OM_GetHostRouteEntry(action_flags, fib_id, &nhop_entry);
            if(ret == FALSE)
            {
                net_route_default.net_route_status = AMTRL3_OM_NET_ROUTE_UNRESOLVED;
                AMTRL3_OM_SetNetRouteEntry(action_flags, fib_id, &net_route_default);
                continue;
            }

            if (!AMTRL3_MGR_SetDefaultRouteEntry(action_flags, fib_id, &net_route_default, &nhop_entry))
            {
                net_route_default.net_route_status = AMTRL3_OM_NET_ROUTE_UNRESOLVED;
                if(amtrl3_mgr_debug_flag & DEBUG_LEVEL_ERROR)
                    BACKDOOR_MGR_Printf("Set default route entry to chip fails\n");
            }
            ret = AMTRL3_OM_SetNetRouteEntry(action_flags, fib_id, &net_route_default);
            /* Here only add resolved default route to chip, so no update statistics in om */
        }
    } /* end of if */
#endif /*  comment-out by vai */
    return;
} /* end of AMTRL3_MGR_CompensateResolvedNetRouteTable() */

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_CompensateResolvedNetRouteEntry
 * -------------------------------------------------------------------------
 * PURPOSE:  Process specific resolved net route entry.
 * INPUT:    action_flags: AMTRL3_TYPE_FLAGS_IPV4 \ AMTRL3_TYPE_FLAGS_IPV6
 *           fib_id - FIB id
 *           resolved net entry - Net route entry to insert to chip.
 * OUTPUT:   net_route_result - Result of net route entry after processing
 * RETURN:   TRUE \ FALSE
 * NOTES:    None
 * -------------------------------------------------------------------------*/
static BOOL_T AMTRL3_MGR_CompensateResolvedNetRouteEntry(UI32_T action_flags,
                                                         UI32_T fib_id,
                                                         AMTRL3_OM_ResolvedNetRouteEntry_T   *resolved_net_entry,
                                                         AMTRL3_OM_NetRouteStatus_T   *net_route_result)
{
    /* Local Variable Declaration
     */
    UI32_T i;
    AMTRL3_OM_HostRouteEntry_T          host_entry;
    AMTRL3_OM_NetRouteEntry_T           net_entry;
    BOOL_T                              is_first = FALSE;

    /* BODY
     */
    memset(&host_entry, 0, sizeof(AMTRL3_OM_HostRouteEntry_T));
    memset(&net_entry,  0, sizeof(AMTRL3_OM_NetRouteEntry_T));

    net_entry.inet_cidr_route_entry.inet_cidr_route_dest = resolved_net_entry->inet_cidr_route_dest;
    for(i = 0; i < SYS_ADPT_NUMBER_OF_INET_CIDR_ROUTE_POLICY_SUBIDENTIFIER; i++)
        net_entry.inet_cidr_route_entry.inet_cidr_route_policy[i] = resolved_net_entry->inet_cidr_route_policy[i];
    net_entry.inet_cidr_route_entry.inet_cidr_route_next_hop = resolved_net_entry->inet_cidr_route_next_hop;

    if((net_entry.inet_cidr_route_entry.inet_cidr_route_dest.type== L_INET_ADDR_TYPE_IPV4) ||
       (net_entry.inet_cidr_route_entry.inet_cidr_route_dest.type== L_INET_ADDR_TYPE_IPV4Z))
       net_entry.inet_cidr_route_entry.inet_cidr_route_pfxlen = (AMTRL3_MGR_IPV4_MAXIMUM_PREFIX_LENGTH - resolved_net_entry->inverse_prefix_length);
    else
       net_entry.inet_cidr_route_entry.inet_cidr_route_pfxlen = (AMTRL3_MGR_IPV6_MAXIMUM_PREFIX_LENGTH - resolved_net_entry->inverse_prefix_length);

    if (!AMTRL3_OM_GetNetRouteEntry(action_flags, fib_id, &net_entry))
        return FALSE;

    host_entry.key_fields.dst_vid_ifindex = net_entry.inet_cidr_route_entry.inet_cidr_route_if_index;
    host_entry.key_fields.dst_inet_addr = net_entry.inet_cidr_route_entry.inet_cidr_route_next_hop;
    if (!AMTRL3_OM_GetHostRouteEntry(action_flags, fib_id, &host_entry))
        return FALSE;

    if(CHECK_FLAG(net_entry.flags, AMTRL3_TYPE_FLAGS_ECMP))
    {
        if(AMTRL3_MGR_IsFirstPathOfECMPRouteInChip(action_flags, fib_id, &net_entry))
            is_first = TRUE;
        AMTRL3_MGR_AddECMPRouteOnePathToChip(action_flags, fib_id, &net_entry, &host_entry, is_first);

        if(is_first && (net_entry.net_route_status == AMTRL3_OM_NET_ROUTE_READY))
            AMTRL3_MGR_SyncHWInfoToAllECMPPath(action_flags, fib_id, &net_entry);
    }
    else
    {
        AMTRL3_MGR_AddNetRouteToChip(action_flags, fib_id, &net_entry, &host_entry);
    }

    *net_route_result = net_entry.net_route_status;

    if (net_entry.net_route_status == AMTRL3_OM_NET_ROUTE_READY)
    {
        if(AMTRL3_OM_DeleteResolvedNetEntry(action_flags, fib_id, resolved_net_entry))
        {
            if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
                AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_IPV4_RESOLVED_NET_ROUTE_COUNT, 1);
            else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
                AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_IPV6_RESOLVED_NET_ROUTE_COUNT, 1);
        }
        else
        {
            if(amtrl3_mgr_debug_flag & DEBUG_LEVEL_ERROR)
                BACKDOOR_MGR_Printf("AMTRL3_OM_DeleteResolvedNetEntry Fails\n");
            return FALSE;
        }

        if(!AMTRL3_OM_SetNetRouteEntry(action_flags, fib_id, &net_entry))
            return FALSE;
    }

    return TRUE;
} /* end of AMTRL3_MGR_CompensateResolvedNetRouteEntry() */

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_CompensateSuperNetRoutes
 * -------------------------------------------------------------------------
 * PURPOSE:  Compensate  Routes when Route table is full
 * >>These routes which cannot add to chip because of lack of bucket are stored in ResolvedNetRouteEntry<<
 * INPUT:    action_flags: AMTRL3_TYPE_FLAGS_IPV4 \ AMTRL3_TYPE_FLAGS_IPV6
 *           fib_id       -- FIB id
 *           net_entry - Net route entry to insert to chip.
 * OUTPUT:   none.
 * RETURN:   none
 * NOTES:    action flag can only be V4 or V6
 * -------------------------------------------------------------------------*/
static BOOL_T AMTRL3_MGR_CompensateSuperNetRoutes(UI32_T action_flags, UI32_T fib_id, AMTRL3_OM_NetRouteEntry_T  *net_entry)
{
    /* LOCAL VARIABLE DECLARATION
     */
    AMTRL3_OM_ResolvedNetRouteEntry_T       resolved_net_entry;
    AMTRL3_OM_HostRouteEntry_T              nh_entry;
    BOOL_T          ret;
    UI8_T   ip_str[L_INET_MAX_IPADDR_STR_LEN] = {0};
    UI32_T  action_type;

    /* BODY
     */
    if(AMTRL3_MGR_GetFIBByID(fib_id) == NULL)
        return FALSE;

    if (amtrl3_mgr_debug_flag & DEBUG_LEVEL_SUPERNET)
    {
        AMTRL3_MGR_Ntoa(&(net_entry->inet_cidr_route_entry.inet_cidr_route_dest), ip_str);
        BACKDOOR_MGR_Printf("AMTRL3_MGR_CompensateSuperNetRoutes on IP %s\n", ip_str);
    }

    memset(&resolved_net_entry, 0, sizeof(AMTRL3_OM_ResolvedNetRouteEntry_T));
    memset(&nh_entry, 0, sizeof(AMTRL3_OM_HostRouteEntry_T));
    /* For every net route entry that cannot write to chip because of table size capacity:
     * 1. Insert to Resolved net route table.
     * 2. Remove all parent routes from chip and insert to Resolved net table.
     * 3. Set default gateway action depdends on software forwarding status.
     */
    if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
        resolved_net_entry.inverse_prefix_length  = AMTRL3_MGR_IPV4_MAXIMUM_PREFIX_LENGTH - net_entry->inet_cidr_route_entry.inet_cidr_route_pfxlen;
    else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
        resolved_net_entry.inverse_prefix_length  = AMTRL3_MGR_IPV6_MAXIMUM_PREFIX_LENGTH - net_entry->inet_cidr_route_entry.inet_cidr_route_pfxlen;

    resolved_net_entry.inet_cidr_route_dest = net_entry->inet_cidr_route_entry.inet_cidr_route_dest;
    resolved_net_entry.inet_cidr_route_next_hop = net_entry->inet_cidr_route_entry.inet_cidr_route_next_hop;

    if (AMTRL3_OM_SetResolvedNetEntry(action_flags, fib_id, &resolved_net_entry))
    {
        if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
            AMTRL3_OM_IncreaseDatabaseValue(fib_id, AMTRL3_OM_IPV4_RESOLVED_NET_ROUTE_COUNT, 1);
        else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
            AMTRL3_OM_IncreaseDatabaseValue(fib_id, AMTRL3_OM_IPV6_RESOLVED_NET_ROUTE_COUNT, 1);
    }
    else
        return FALSE;

    AMTRL3_MGR_RefreshNetTable(action_flags, fib_id, &resolved_net_entry);

    /* Software Forwarding Design Concept:
     * 1. Save the original default route configuration;
     * 2. Change default route to CPU when net route entry fail to write to chip due to chip table full;
     * 3. Change default routes back to original settings if all routes are successfully write to chip.
     */
    if ((CHECK_FLAG(action_flags,  AMTRL3_TYPE_FLAGS_IPV4) &&
         (AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_IPV4_SOFTWARE_FORWARDING_STATUS) == AMTRL3_TYPE_SOFTWARE_FORWARDING_ENABLE)) ||
        (CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6) &&
         (AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_IPV6_SOFTWARE_FORWARDING_STATUS) == AMTRL3_TYPE_SOFTWARE_FORWARDING_ENABLE)))
    {

        AMTRL3_OM_NetRouteEntry_T net_route_default;
        SWDRVL3_Route_T             swdrvl3_net_entry;
        memset(&net_route_default, 0, sizeof(AMTRL3_OM_NetRouteEntry_T));
        if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
            net_route_default.inet_cidr_route_entry.inet_cidr_route_dest.type= L_INET_ADDR_TYPE_IPV4;
        else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
            net_route_default.inet_cidr_route_entry.inet_cidr_route_dest.type= L_INET_ADDR_TYPE_IPV6;

        /* Delete all default route of ipv4 or ipv6 at first */
        while(AMTRL3_OM_GetNextNetRouteEntry(action_flags, fib_id, &net_route_default))
        {
            if((AMTRL3_OM_IsAddressEqualZero(&(net_route_default.inet_cidr_route_entry.inet_cidr_route_dest)) == FALSE) ||
                (net_route_default.inet_cidr_route_entry.inet_cidr_route_pfxlen != 0))
            {
                break;
            }
            if(net_route_default.net_route_status != AMTRL3_OM_NET_ROUTE_READY)
                continue;

            nh_entry.key_fields.dst_vid_ifindex = net_route_default.inet_cidr_route_entry.inet_cidr_route_if_index;
            nh_entry.key_fields.dst_inet_addr = net_route_default.inet_cidr_route_entry.inet_cidr_route_next_hop;
            if(AMTRL3_OM_GetHostRouteEntry(action_flags, fib_id, &nh_entry))
                AMTRL3_MGR_DeleteNetRouteFromChip(action_flags, fib_id, &net_route_default, &nh_entry);

            /* set default route status to resolved after delete it from chip */
            net_route_default.net_route_status = AMTRL3_OM_NET_ROUTE_RESOLVED;
            AMTRL3_OM_SetNetRouteEntry(action_flags, fib_id, &net_route_default);
        }

        /* Add special default route to chip */
        if(CHECK_FLAG(amtrl3_chip_capability_flags, AMTRL3_CHIP_USE_DEFAULT_ROUTE_TRAP_TO_CPU))
        {
            if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
                AMTRL3_OM_SetDatabaseValue(fib_id, AMTRL3_OM_IPV4_DEFAULT_ROUTE_ACTION_TYPE, SYS_CPNT_DEFAULT_ROUTE_ACTION_TRAP2CPU);
            else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
                AMTRL3_OM_SetDatabaseValue(fib_id, AMTRL3_OM_IPV6_DEFAULT_ROUTE_ACTION_TYPE, SYS_CPNT_DEFAULT_ROUTE_ACTION_TRAP2CPU);

            memset(&swdrvl3_net_entry, 0, sizeof(SWDRVL3_Route_T));
            if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
                swdrvl3_net_entry.flags |= SWDRVL3_FLAG_IPV4;
            else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
                swdrvl3_net_entry.flags |= SWDRVL3_FLAG_IPV6;

            swdrvl3_net_entry.fib_id = fib_id;
            swdrvl3_net_entry.unit = SYS_ADPT_DESTINATION_UNIT_UNKNOWN;
            swdrvl3_net_entry.port = SYS_ADPT_DESTINATION_PORT_UNKNOWN;
            swdrvl3_net_entry.trunk_id = SYS_ADPT_DESTINATION_TRUNK_ID_UNKNOWN;
            action_type = SYS_CPNT_DEFAULT_ROUTE_ACTION_TRAP2CPU;

            ret = SWDRVL3_SetSpecialDefaultRoute(&swdrvl3_net_entry, action_type);
            if(ret)
            {
                if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
                    AMTRL3_OM_IncreaseDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV4_NET_ROUTE_IN_CHIP, 1);
                else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
                    AMTRL3_OM_IncreaseDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV6_NET_ROUTE_IN_CHIP, 1);
            }
            else
                return FALSE;
        }
    } /* end of if */
    return TRUE;
} /* end of AMTRL3_MGR_CompensateSuperNetRoutes() */

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_RefreshNetTable
 * -------------------------------------------------------------------------
 * PURPOSE:  Compensate SupperNetting Routes when Route table is full
 * INPUT:    action_flags: AMTRL3_TYPE_FLAGS_IPV4 \ AMTRL3_TYPE_FLAGS_IPV6
 *           fib_id - FIB id
 *           net_entry - Net route entry to insert to chip.
 * OUTPUT:   none.
 * RETURN:   none
 * NOTES:    None
 * -------------------------------------------------------------------------*/
static void AMTRL3_MGR_RefreshNetTable(UI32_T action_flags, UI32_T fib_id, AMTRL3_OM_ResolvedNetRouteEntry_T *child_entry)
{
    /* LOCAL VARIABLE DECLARATION
     */
    AMTRL3_OM_NetRouteEntry_T   supernet_entry;
    UI32_T      cider_len;
    UI8_T       ip_str[L_INET_MAX_IPADDR_STR_LEN] = {0};
    UI32_T      max_pfxlen, min_pfxlen;


    /* BODY
     */
    if ((CHECK_FLAG(action_flags,  AMTRL3_TYPE_FLAGS_IPV4) &&
         (AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_IPV4_SUPER_NETTING_STATUS) == AMTRL3_TYPE_SUPER_NET_DISABLE)) ||
        (CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6) &&
         (AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_IPV6_SUPER_NETTING_STATUS) == AMTRL3_TYPE_SUPER_NET_DISABLE)))
        return;

    if (amtrl3_mgr_debug_flag & DEBUG_LEVEL_SUPERNET)
    {
        AMTRL3_MGR_Ntoa(&(child_entry->inet_cidr_route_dest), ip_str);
        BACKDOOR_MGR_Printf("AMTRL3_MGR_RefreshNetTable on IP %s, inverse_prefix_length: %d\n", ip_str, (int)child_entry->inverse_prefix_length);
    }

    memset(&supernet_entry, 0, sizeof(AMTRL3_OM_NetRouteEntry_T));

    /*  For every net route that cannot write to chip succesfully because of
     *  limitation on net table size, all subnet that are parent of this subnet
     *  shall be remove from chip and insert to "resolved_net_table".
     */
    if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
    {
        max_pfxlen = AMTRL3_MGR_IPV4_MAXIMUM_PREFIX_LENGTH;
        min_pfxlen = AMTRL3_MGR_IPV4_MINIMUM_PREFIX_LENGTH;
    }
    else
    {
        max_pfxlen = AMTRL3_MGR_IPV6_MAXIMUM_PREFIX_LENGTH;
        min_pfxlen = AMTRL3_MGR_IPV6_MINIMUM_PREFIX_LENGTH;
    }

    if(  max_pfxlen <= child_entry->inverse_prefix_length)
    {
        return;
    }

    for ((cider_len = max_pfxlen- child_entry->inverse_prefix_length -1);
          cider_len >= min_pfxlen; cider_len--)
    {
        if (AMTRL3_OM_GetSuperNetEntry(action_flags,
                                       fib_id,
                                       child_entry->inet_cidr_route_dest,
                                       cider_len,
                                       &supernet_entry))
        {
            /* Only supernet entry that currently exists in chip needs to be remove
             * from chip and insert to Resolved Table.
             */
            if (supernet_entry.net_route_status != AMTRL3_OM_NET_ROUTE_READY)
                continue;

            if (!AMTRL3_MGR_ModifySuperNetRouteEntry(action_flags, fib_id, &supernet_entry))
                break;
        } /* end of if */
    } /* end of for */

    return;
} /* end of AMTRL3_MGR_RefreshNetTable() */

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_ModifySuperNetRouteEntry
 * -------------------------------------------------------------------------
 * PURPOSE:  Compensate SupperNetting Routes when Route table is full
 * INPUT:    action_flags: AMTRL3_TYPE_FLAGS_IPV4 \ AMTRL3_TYPE_FLAGS_IPV6
 *           fib_id - FIB id
 *           net_entry - Net route entry to insert to chip.
 * OUTPUT:   none.
 * RETURN:   none
 * NOTES:    None
 * -------------------------------------------------------------------------*/
static BOOL_T AMTRL3_MGR_ModifySuperNetRouteEntry(UI32_T action_flags, UI32_T fib_id, AMTRL3_OM_NetRouteEntry_T *net_entry)
{
    /* LOCAL VARIABLE DECLARATION
     */
    AMTRL3_OM_ResolvedNetRouteEntry_T   parent_entry;
    AMTRL3_OM_HostRouteEntry_T              nh_entry;
    UI8_T   ip_str[L_INET_MAX_IPADDR_STR_LEN] = {0};
    UI32_T  i;

    /* BODY
     */
    memset(&parent_entry, 0, sizeof(AMTRL3_OM_ResolvedNetRouteEntry_T));
    memset(&nh_entry, 0, sizeof(AMTRL3_OM_HostRouteEntry_T));

    if (net_entry == NULL)
        return FALSE;

    if (amtrl3_mgr_debug_flag & DEBUG_LEVEL_SUPERNET)
    {
        AMTRL3_MGR_Ntoa(&(net_entry->inet_cidr_route_entry.inet_cidr_route_dest), ip_str);
        BACKDOOR_MGR_Printf("AMTRL3_MGR_ModifySuperNetRouteEntry on IP %s\n", ip_str);
    }

    nh_entry.key_fields.dst_vid_ifindex = net_entry->inet_cidr_route_entry.inet_cidr_route_if_index;
    nh_entry.key_fields.dst_inet_addr = net_entry->inet_cidr_route_entry.inet_cidr_route_next_hop;
    if(AMTRL3_OM_GetHostRouteEntry(action_flags, fib_id, &nh_entry))
    {
        if(!AMTRL3_MGR_DeleteNetRouteFromChip(action_flags, fib_id, net_entry, &nh_entry))
            return FALSE;
    }
    else
        return FALSE;

    net_entry->net_route_status = AMTRL3_OM_NET_ROUTE_RESOLVED;

    if (!AMTRL3_OM_SetNetRouteEntry(action_flags, fib_id, net_entry))
        return FALSE;

    if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
        parent_entry.inverse_prefix_length  = (AMTRL3_MGR_IPV4_MAXIMUM_PREFIX_LENGTH - net_entry->inet_cidr_route_entry.inet_cidr_route_pfxlen);
    else
        parent_entry.inverse_prefix_length  = (AMTRL3_MGR_IPV6_MAXIMUM_PREFIX_LENGTH - net_entry->inet_cidr_route_entry.inet_cidr_route_pfxlen);

    parent_entry.inet_cidr_route_dest = net_entry->inet_cidr_route_entry.inet_cidr_route_dest;
    parent_entry.inet_cidr_route_next_hop = net_entry->inet_cidr_route_entry.inet_cidr_route_next_hop;
    for(i = 0; i < SYS_ADPT_NUMBER_OF_INET_CIDR_ROUTE_POLICY_SUBIDENTIFIER; i++)
        parent_entry.inet_cidr_route_policy[i] = net_entry->inet_cidr_route_entry.inet_cidr_route_policy[i];

    if (AMTRL3_OM_SetResolvedNetEntry(action_flags, fib_id, &parent_entry))
    {
        if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
            AMTRL3_OM_IncreaseDatabaseValue(fib_id, AMTRL3_OM_IPV4_RESOLVED_NET_ROUTE_COUNT, 1);
        else
            AMTRL3_OM_IncreaseDatabaseValue(fib_id, AMTRL3_OM_IPV6_RESOLVED_NET_ROUTE_COUNT, 1);
    }
    else
        return FALSE;

    return TRUE;
} /* end of AMTRL3_MGR_ModifySuperNetRouteEntry() */

/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTRL3_MGR_DeriveDetailPortInfo
 *------------------------------------------------------------------------
 * FUNCTION: This function uses input vid_ifindex and lport to derive
 *           all necessary information for this component to pass on
 *           to lower layer.
 *
 * INPUT   : vid_ifindex  - specific vid_ifindex associates with the entry
 *           lport        - specific port associates with the entry.
 *
 * OUTPUT  : *vid       - specific vlan which lport belongs to
 *           *unit      - specific unit which lport belongs to
 *           *port      - specific port of the lport
 *           *trunk_id  - specific trun which lport belongs to
 *           *is_tagged - indicates whether lport is tagged member of vid.
 *           *is_trunk  - indicates whether lport is a trunk port or trunk member
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
static BOOL_T AMTRL3_MGR_DeriveDetailPortInfo(UI32_T vid_ifindex, UI32_T lport, UI32_T *vid,
                                              UI32_T *unit, UI32_T *port, UI32_T *trunk_id,
                                              BOOL_T *is_tagged, BOOL_T *is_trunk)
{
    /* Local Variable Declaration
     */
    SWCTRL_Lport_Type_T     port_type = SWCTRL_LPORT_UNKNOWN_PORT;
    SYS_TYPE_Uport_T        unit_port;
    BOOL_T                  ret = FALSE;

    /* BODY
     */
    if ((vid_ifindex == 0) || (lport == 0))
        return FALSE;

    if (lport == SYS_ADPT_DESTINATION_PORT_UNKNOWN)
        return FALSE;

    memset(&unit_port, 0, sizeof(SYS_TYPE_Uport_T));
    VLAN_OM_ConvertFromIfindex(vid_ifindex, vid);

    port_type = SWCTRL_POM_LogicalPortToUserPort(lport, unit, port, trunk_id);

    switch (port_type)
    {
        case SWCTRL_LPORT_UNKNOWN_PORT:
            break;

        case SWCTRL_LPORT_NORMAL_PORT:
            *is_trunk = FALSE;
            ret = TRUE;
            break;

        case SWCTRL_LPORT_TRUNK_PORT:
            if (SWCTRL_POM_LportToActiveUport(SYS_TYPE_IGNORE_VID_CHECK, lport, &unit_port))
            {
                *is_trunk = TRUE;
                *unit     = unit_port.unit;
                *port     = unit_port.port;
                ret       = TRUE;
            }
            break;

        case SWCTRL_LPORT_TRUNK_PORT_MEMBER:
            *is_trunk = TRUE;
            ret = TRUE;
            break;

        default:
             break;

    } /* end of switch */

    *is_tagged = VLAN_POM_IsVlanUntagPortListMember(vid_ifindex, lport);
    *is_tagged = (*is_tagged == FALSE) ? TRUE : FALSE;

    return ret;
} /* end of AMTRL3_MGR_DeriveDetailPortInfo() */

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_DeriveProperHostRouteStatus
 * -------------------------------------------------------------------------
 * PURPOSE:  Print Host Route Finite State Machine information
 * INPUT:    event_type -  HOST_ROUTE_READY_EVENT \
 *                         HOST_ROUTE_UNREFERENCE_EVENT
 *           host_route_entry
 * OUTPUT:   none.
 * RETURN:   none
 * NOTES:    None
 * -------------------------------------------------------------------------*/
static void AMTRL3_MGR_DeriveProperHostRouteStatus(UI32_T event_type, AMTRL3_OM_HostRouteEntry_T *host_route_entry)
{
    UI32_T  flag;

    /* BODY
     */
    if (host_route_entry == NULL)
        return;

    if (event_type == HOST_ROUTE_READY_EVENT)
    {
        host_route_entry->status = (host_route_entry->ref_count) ? HOST_ROUTE_READY_NOT_SYNC : HOST_ROUTE_HOST_READY;
    }
    else if ((event_type == HOST_ROUTE_PARTIAL_EVENT) || (event_type == HOST_ROUTE_SYNC_COMPLETE_EVENT))
    {
        host_route_entry->status = (host_route_entry->ref_count) ? HOST_ROUTE_GATEWAY_READY : HOST_ROUTE_HOST_READY;
    }
    else if ((event_type == HOST_ROUTE_UNREFERENCE_EVENT) && (!host_route_entry->in_chip_status))
    {
        /* Static entries treated as unresolved, such that AMTRL3 will send out ARP request
         */
        if(host_route_entry->ref_count >0 ||
           host_route_entry->entry_type == VAL_ipNetToPhysicalExtType_static)
            host_route_entry->status = HOST_ROUTE_UNRESOLVED;
#if (SYS_CPNT_IP_TUNNEL == TRUE)
        else if((host_route_entry->entry_type == VAL_ipNetToPhysicalExtType_other)&&
                (IS_TUNNEL_IFINDEX(host_route_entry->key_fields.dst_vid_ifindex)))
        {
            /* if it fails to add tunnel host route to chip, it should treated as unresolved */
            host_route_entry->status = HOST_ROUTE_UNRESOLVED;
        }
#endif
        else
            host_route_entry->status = HOST_ROUTE_UNREFERENCE;
    }
    else
    {
        host_route_entry->status = HOST_ROUTE_UNRESOLVED;

        if (amtrl3_mgr_debug_flag & DEBUG_LEVEL_ERROR)
        {
            if((host_route_entry->key_fields.dst_inet_addr.type== L_INET_ADDR_TYPE_IPV4) ||
               (host_route_entry->key_fields.dst_inet_addr.type== L_INET_ADDR_TYPE_IPV4Z))
                flag = AMTRL3_TYPE_FLAGS_IPV4;
            else
                flag = AMTRL3_TYPE_FLAGS_IPV6;

            BACKDOOR_MGR_Printf("Invalid operation in AMTRL3_MGR_DeriveProperHostRouteStatus\n");
            AMTRL3_MGR_DebugFSM(flag, &event_type, host_route_entry->key_fields.dst_inet_addr, host_route_entry->status, HOST_ROUTE_UNRESOLVED);
        } /* end of if */
    } /* end of else */

    return;
} /* end of AMTRL3_MGR_DeriveProperHostRouteStatus() */

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_DebugFSM
 * -------------------------------------------------------------------------
 * PURPOSE:  Print Host Route Finite State Machine information
 * INPUT:    action_flags: AMTRL3_TYPE_FLAGS_IPV4 \ AMTRL3_TYPE_FLAGS_IPV6
 *           host_route_event - Type of event occur
 *           dst_ip
 *           old_status
 *           new_status
 * OUTPUT:   none.
 * RETURN:   none
 * NOTES:    None
 * -------------------------------------------------------------------------*/
static void AMTRL3_MGR_DebugFSM(UI32_T action_flags, UI32_T *host_route_event, L_INET_AddrIp_T dst_ip, UI32_T old_status, UI32_T new_status)
{
    char        display_str[18];
    UI8_T       ip_str[L_INET_MAX_IPADDR_STR_LEN] = {0};
    UI32_T      temp_status, index = 0;

    /* BODY
     */
    AMTRL3_MGR_Ntoa(&dst_ip, ip_str);

    BACKDOOR_MGR_Printf("%s: IP: %s, ", __FUNCTION__, ip_str);

    switch (*host_route_event)
    {
        case HOST_ROUTE_PARTIAL_EVENT:
            strcpy(display_str, "partial");
            break;
        case HOST_ROUTE_READY_EVENT:
            strcpy(display_str, "host ready");
            break;
        case HOST_ROUTE_UNREFERENCE_EVENT:
            strcpy(display_str, "unreference");
            break;
        case HOST_ROUTE_REMOVE_EVENT:
            strcpy(display_str, "remove");
            break;
        case HOST_ROUTE_SYNC_COMPLETE_EVENT:
            strcpy(display_str, "sync_complete");
            break;
        case HOST_ROUTE_DROP_EVENT:
            strcpy(display_str, "Drop");
            break;
        default:
            strcpy(display_str, "none");
            break;
    } /* end of for */
    BACKDOOR_MGR_Printf(" event: %s,", display_str);

    for (index = 0; index <= 1; index++)
    {
        memset(display_str, 0, sizeof(display_str));

        if (index == 0)
        {
            BACKDOOR_MGR_Printf(" old_status: ");
            temp_status = old_status;
        }
        else
        {
            BACKDOOR_MGR_Printf(" new_status: ");
            temp_status = new_status;
        }
        switch (temp_status)
        {
            case HOST_ROUTE_NOT_EXIST:
                strcpy(display_str, "not_exist");
                break;
            case HOST_ROUTE_UNRESOLVED:
                strcpy(display_str, "unresolved");
                break;
            case HOST_ROUTE_UNREFERENCE:
                strcpy(display_str, "unreference");
                break;
            case HOST_ROUTE_HOST_READY:
                strcpy(display_str, "host ready");
                break;
            case HOST_ROUTE_GATEWAY_READY:
                strcpy(display_str, "gateway ready");
                break;
            case HOST_ROUTE_READY_NOT_SYNC:
                strcpy(display_str, "ready_not_sync");
                break;
            case HOST_ROUTE_UNKNOWN:
                strcpy(display_str, "unknown");
                break;
            case HOST_ROUTE_UNRESOLVED_1:
                strcpy(display_str, "unresolved_1");
                break;
            case HOST_ROUTE_UNREFERENCE_1:
                strcpy(display_str, "unreference_1");
                break;
            case HOST_ROUTE_UNREFERENCE_2:
                strcpy(display_str, "unreference_2");
                break;
            case HOST_ROUTE_READY_1:
                strcpy(display_str, "ready_1");
                break;
            case HOST_ROUTE_READY_2:
                strcpy(display_str, "ready_2");
                break;
            case HOST_ROUTE_READY_3:
                strcpy(display_str, "ready_3");
                break;
            case HOST_ROUTE_REMOVE_1:
                strcpy(display_str, "remove_1");
                break;
            case HOST_ROUTE_REMOVE_2:
                strcpy(display_str, "remove_2");
                break;
            default:
                strcpy(display_str, "none");
                break;
        } /* end of is switch */
        BACKDOOR_MGR_Printf("%s,", display_str);
    } /* end of for */
    BACKDOOR_MGR_Printf("\n");

} /* end of AMTRL3_MGR_DebugFSM() */

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_SetDebugFlag
 * -------------------------------------------------------------------------
 * PURPOSE: To Disable / Enable AMTRL3 Debug Message
 * INPUT:   amtrl3_mgr_debug_flag
 * OUTPUT:  None.
 * RETURN:  None.
 * NOTES:   None.
 * -------------------------------------------------------------------------*/
void AMTRL3_MGR_SetDebugFlag(I32_T debug_flag)
{
    /* BODY
     */
    if (debug_flag == 0)
        return;
    else if (debug_flag == 99)
    {
        amtrl3_mgr_debug_flag = 0;
        amtrl3_mgr_callback_debug_flag = 0;
        amtrl3_mgr_debug_task = 0;
        amtrl3_mgr_debug_tunnel = FALSE;
    }
    else if ((debug_flag > 0) && (debug_flag < 7))
    {
        amtrl3_mgr_debug_flag ^= (0x01 << (debug_flag-1));
    }
    else if ((debug_flag >= 7) && (debug_flag < 16))
    {
        amtrl3_mgr_callback_debug_flag ^= (0x01 << (debug_flag-7));
    }
    else if ((debug_flag >= 16) && (debug_flag <= 21)) /* modified by steven.gao */
    {
        amtrl3_mgr_debug_task ^= (0x01 << (debug_flag-16));
    }
    else if(debug_flag==22)
    {
        amtrl3_mgr_debug_tunnel =~amtrl3_mgr_debug_tunnel;
    }
    else
        BACKDOOR_MGR_Printf("Invalid input! Please input again\n");

    return;
} /* end of AMTRL3_MGR_SetDebugFlag() */

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_PrintDebugMode
 * -------------------------------------------------------------------------
 * PURPOSE: To print AMTRL3 Debug Message
 * INPUT:   None.
 * OUTPUT:  None.
 * RETURN:  None.
 * NOTES:   Only for backdoor debug.
 * -------------------------------------------------------------------------*/
void AMTRL3_MGR_PrintDebugMode(void)
{
    BACKDOOR_MGR_Printf ("\n=======================================\n");
    BACKDOOR_MGR_Printf (" 0: exit\n");
    BACKDOOR_MGR_Printf (" 1: DEBUG_LEVEL_TRACE_HOST    \t%s\n", (amtrl3_mgr_debug_flag & DEBUG_LEVEL_TRACE_HOST) ? "ON":"OFF");
    BACKDOOR_MGR_Printf (" 2: DEBUG_LEVEL_TRACE_NET     \t%s\n", (amtrl3_mgr_debug_flag & DEBUG_LEVEL_TRACE_NET) ? "ON":"OFF");
    BACKDOOR_MGR_Printf (" 3: DEBUG_LEVEL_ERROR    \t%s\n", (amtrl3_mgr_debug_flag & DEBUG_LEVEL_ERROR ) ? "ON":"OFF");
    BACKDOOR_MGR_Printf (" 4: DEBUG_LEVEL_DETAIL   \t%s\n", (amtrl3_mgr_debug_flag & DEBUG_LEVEL_DETAIL) ? "ON":"OFF");
    BACKDOOR_MGR_Printf (" 5: DEBUG_LEVEL_SUPERNET \t%s\n", (amtrl3_mgr_debug_flag & DEBUG_LEVEL_SUPERNET) ? "ON":"OFF");
    BACKDOOR_MGR_Printf (" 6: DEBUG_LEVEL_HOT_INST \t%s\n", (amtrl3_mgr_debug_flag & DEBUG_LEVEL_HOTINSRT) ? "ON":"OFF");
    BACKDOOR_MGR_Printf ("\n");
    BACKDOOR_MGR_Printf (" 7: CALLBACK_DEBUG_LPORT_DOWN   \t%s\n", (amtrl3_mgr_callback_debug_flag  & CALLBACK_DEBUG_LPORT_DOWN) ? "ON":"OFF");
    BACKDOOR_MGR_Printf (" 8: CALLBACK_DEBUG_TRUNK        \t%s\n", (amtrl3_mgr_callback_debug_flag  & CALLBACK_DEBUG_TRUNK ) ? "ON":"OFF");
    BACKDOOR_MGR_Printf (" 9: CALLBACK_DEBUG_MAC          \t%s\n", (amtrl3_mgr_callback_debug_flag  & CALLBACK_DEBUG_MAC) ? "ON":"OFF");
    BACKDOOR_MGR_Printf ("10: CALLBACK_DEBUG_PORT_N_VID   \t%s\n", (amtrl3_mgr_callback_debug_flag  & CALLBACK_DEBUG_PORT_N_VID) ? "ON":"OFF");
    BACKDOOR_MGR_Printf ("11: CALLBACK_DEBUG_PORT_MOVE    \t%s\n", (amtrl3_mgr_callback_debug_flag  & CALLBACK_DEBUG_PORT_MOVE) ? "ON":"OFF");
    BACKDOOR_MGR_Printf ("12: CALLBACK_DEBUG_VLAN_DESTROY \t%s\n", (amtrl3_mgr_callback_debug_flag  & CALLBACK_DEBUG_VLAN_DESTROY) ? "ON":"OFF");
    BACKDOOR_MGR_Printf ("13: CALLBACK_DEBUG_VLAN_DOWN    \t%s\n", (amtrl3_mgr_callback_debug_flag  & CALLBACK_DEBUG_VLAN_DOWN) ? "ON":"OFF");
    BACKDOOR_MGR_Printf ("14: CALLBACK_DEBUG_DETAIL       \t%s\n", (amtrl3_mgr_callback_debug_flag  & CALLBACK_DEBUG_DETAIL) ? "ON":"OFF");
    BACKDOOR_MGR_Printf ("15: CALLBACK_TRUNK_DETAIL       \t%s\n", (amtrl3_mgr_callback_debug_flag  & CALLBACK_TRUNK_DETAIL) ? "ON":"OFF");
    BACKDOOR_MGR_Printf ("\n");
    BACKDOOR_MGR_Printf ("16: TASK_DEBUG_HITBIT         \t%s\n", (amtrl3_mgr_debug_task  & TASK_DEBUG_HITBIT) ? "ON":"OFF");
    BACKDOOR_MGR_Printf ("17: TASK_DEBUG_COMPENSATE     \t%s\n", (amtrl3_mgr_debug_task  & TASK_DEBUG_COMPENSATE) ? "ON":"OFF");
    BACKDOOR_MGR_Printf ("18: TASK_DEBUG_ARP_UNRESOLVED \t%s\n", (amtrl3_mgr_debug_task  & TASK_DEBUG_ARP_UNRESOLVED) ? "ON":"OFF");
    BACKDOOR_MGR_Printf ("19: TASK_DEBUG_ARP_GATEWAY    \t%s\n", (amtrl3_mgr_debug_task  & TASK_DEBUG_ARP_GATEWAY) ? "ON":"OFF");
    BACKDOOR_MGR_Printf ("20: TASK_DEBUG_RESOLVED       \t%s\n", (amtrl3_mgr_debug_task  & TASK_DEBUG_RESOLVED) ? "ON":"OFF");
    BACKDOOR_MGR_Printf ("21: TASK_DEBUG_MAC_SCAN       \t%s\n", (amtrl3_mgr_debug_task  & TASK_DEBUG_MAC_SCAN) ? "ON":"OFF");
    BACKDOOR_MGR_Printf ("\n");
    BACKDOOR_MGR_Printf ("22: DEBUG TUNNEL \t%s\n", (amtrl3_mgr_debug_tunnel)?"ON":"OFF");
    BACKDOOR_MGR_Printf ("99: DISABLE ALL FLAG \n");
    BACKDOOR_MGR_Printf ("=======================================\n");
    BACKDOOR_MGR_Printf ("Select = ");
    return;
} /* end of AMTRL3_MGR_PrintDebugMode() */

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_DisplayDebugCounters
 * -------------------------------------------------------------------------
 * PURPOSE:  Display all amtrl3 counters , debug and statistics
 * INPUT:    fib_id
 * OUTPUT:   None
 * RETURN:   None
 * NOTES:    Only for backdoor debug.
 * -------------------------------------------------------------------------*/
void AMTRL3_MGR_DisplayDebugCounters(UI32_T fib_id)
{
    /* BODY
     */
    if(AMTRL3_MGR_GetFIBByID(fib_id) == NULL)
        return;

    BACKDOOR_MGR_Printf("\n\r Add_host_fail_counter \n IPV4:%d \t IPV6:%d \t Total:%d",
        (int)AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV4_ADD_HOST_ROUTE_FAIL),
        (int)AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV6_ADD_HOST_ROUTE_FAIL),
        (int)AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV4_ADD_HOST_ROUTE_FAIL) +
        (int)AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV6_ADD_HOST_ROUTE_FAIL));

    BACKDOOR_MGR_Printf("\n\r Delete_host_fail_counter \n IPV4:%d \t IPV6:%d \t Total:%d",
        (int)AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV4_DEL_HOST_ROUTE_FAIL),
        (int)AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV6_DEL_HOST_ROUTE_FAIL),
        (int)AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV4_DEL_HOST_ROUTE_FAIL) +
        (int)AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV6_DEL_HOST_ROUTE_FAIL));

    BACKDOOR_MGR_Printf("\n\r Host_route_in_chip_counter \n IPV4:%d \t IPV6:%d \t Total:%d",
        (int)AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV4_HOST_ROUTE_IN_CHIP),
        (int)AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV6_HOST_ROUTE_IN_CHIP),
        (int)AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV4_HOST_ROUTE_IN_CHIP) +
        (int)AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV6_HOST_ROUTE_IN_CHIP));

    BACKDOOR_MGR_Printf("\n\r Net_route_in_chip_counter(non-ECMP) \n IPV4:%d \t IPV6:%d \t Total:%d",
        (int)AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV4_NET_ROUTE_IN_CHIP),
        (int)AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV6_NET_ROUTE_IN_CHIP),
        (int)AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV4_NET_ROUTE_IN_CHIP) +
        (int)AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV6_NET_ROUTE_IN_CHIP));

    BACKDOOR_MGR_Printf("\n\r Net_route_in_chip_counter(ECMP) \n IPV4: Route:%d  NextHop:%d\t IPV6: Route:%d  NextHop:%d \r\n Total: Route:%d  NextHop:%d",
        (int)AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV4_ECMP_ROUTE_IN_CHIP),
        (int)AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV4_ECMP_NH_IN_CHIP),
        (int)AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV6_ECMP_ROUTE_IN_CHIP),
        (int)AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV6_ECMP_NH_IN_CHIP),
        (int)AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV4_ECMP_ROUTE_IN_CHIP) +
        (int)AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV6_ECMP_ROUTE_IN_CHIP),
        (int)AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV4_ECMP_NH_IN_CHIP) +
        (int)AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV6_ECMP_NH_IN_CHIP));

    BACKDOOR_MGR_Printf("\n\r Dynamic Route in Chip counter \n IPV4: %d",
        (int)AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_IPV4_DYNAMIC_ROUTE_IN_CHIP_COUNTS));

    BACKDOOR_MGR_Printf("\n\r Net_route_in_om_counter(non-ECMP) \n IPV4:%d \t IPV6:%d \t Total:%d",
        (int)AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV4_NET_ROUTE_IN_OM),
        (int)AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV6_NET_ROUTE_IN_OM),
        (int)AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV4_NET_ROUTE_IN_OM) +
        (int)AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV6_NET_ROUTE_IN_OM));

    BACKDOOR_MGR_Printf("\n\r Net_route_in_om_counter(ECMP) \n IPV4: Route:%d  NextHop:%d\t IPV6: Route:%d  NextHop:%d \r\n Total: Route:%d  NextHop:%d",
        (int)AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV4_ECMP_ROUTE_IN_OM),
        (int)AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV4_ECMP_NH_IN_OM),
        (int)AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV6_ECMP_ROUTE_IN_OM),
        (int)AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV6_ECMP_NH_IN_OM),
        (int)AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV4_ECMP_ROUTE_IN_OM) +
        (int)AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV6_ECMP_ROUTE_IN_OM),
        (int)AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV4_ECMP_NH_IN_OM) +
        (int)AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV6_ECMP_NH_IN_OM));

    BACKDOOR_MGR_Printf("\n\r Add_net_route_fail_counter \n IPV4:%d \t IPV6:%d \t Total:%d",
        (int)AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV4_ADD_NET_ROUTE_FAIL),
        (int)AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV6_ADD_NET_ROUTE_FAIL),
        (int)AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV4_ADD_NET_ROUTE_FAIL) +
        (int)AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV6_ADD_NET_ROUTE_FAIL));

    BACKDOOR_MGR_Printf("\n\r Resolved Net Route counter \n IPV4:%d \t IPV6:%d \t Total:%d",
        (int)AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_IPV4_RESOLVED_NET_ROUTE_COUNT),
        (int)AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_IPV6_RESOLVED_NET_ROUTE_COUNT),
        (int)AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_IPV4_RESOLVED_NET_ROUTE_COUNT) +
        (int)AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_IPV6_RESOLVED_NET_ROUTE_COUNT));

    BACKDOOR_MGR_Printf("\n\r Total_neighbor_counts \n IPV4:%d \t IPV6:%d \t Total:%d",
        (int)AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_IPV4_TOTAL_IPNET2PHYSICAL_COUNTS),
        (int)AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_IPV6_TOTAL_IPNET2PHYSICAL_COUNTS),
        (int)AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_IPV4_TOTAL_IPNET2PHYSICAL_COUNTS) +
        (int)AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_IPV6_TOTAL_IPNET2PHYSICAL_COUNTS));

    BACKDOOR_MGR_Printf("\n\r Static_neighbor_counts \n IPV4:%d \t IPV6:%d \t Total:%d",
        (int)AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_IPV4_STATIC_IPNET2PHYSICAL_COUNTS),
        (int)AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_IPV6_STATIC_IPNET2PHYSICAL_COUNTS),
        (int)AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_IPV4_STATIC_IPNET2PHYSICAL_COUNTS) +
        (int)AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_IPV6_STATIC_IPNET2PHYSICAL_COUNTS));

    BACKDOOR_MGR_Printf("\n\r Dynamic_neighbor_counts \n IPV4:%d \t IPV6:%d \t Total:%d",
        (int)AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_IPV4_DYNAMIC_IPNET2PHYSICAL_COUNTS),
        (int)AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_IPV6_DYNAMIC_IPNET2PHYSICAL_COUNTS),
        (int)AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_IPV4_DYNAMIC_IPNET2PHYSICAL_COUNTS) +
        (int)AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_IPV6_DYNAMIC_IPNET2PHYSICAL_COUNTS));
#if (SYS_CPNT_IP_TUNNEL == TRUE)
    BACKDOOR_MGR_Printf("\n\r IPv6 dynamic 6to4 Tunnel counts \n Host:%d \t Net:%d",
        (int)AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_IPV6_6to4_TUNNEL_HOST_ROUTE_COUNT),
        (int)AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_IPV6_6to4_TUNNEL_NET_ROUTE_COUNT));
#endif
    BACKDOOR_MGR_Printf("\n\r");
    return;
} /* end of AMTRL3_MGR_DisplayDebugCounters() */

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_ClearDebugCounter
 * -------------------------------------------------------------------------
 * PURPOSE: Clear Amtrl3 debug counter
 * INPUT:   fib_id.
 * OUTPUT:  None.
 * RETURN:  None.
 * NOTES:   None.
 * -------------------------------------------------------------------------*/
void AMTRL3_MGR_ClearDebugCounter(UI32_T fib_id)
{
    if(AMTRL3_MGR_GetFIBByID(fib_id) == NULL)
        return;

    AMTRL3_OM_SetDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV4_ADD_HOST_ROUTE_FAIL, 0);
    AMTRL3_OM_SetDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV4_DEL_HOST_ROUTE_FAIL, 0);
    AMTRL3_OM_SetDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV4_ADD_NET_ROUTE_FAIL, 0);

    AMTRL3_OM_SetDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV6_ADD_HOST_ROUTE_FAIL, 0);
    AMTRL3_OM_SetDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV6_DEL_HOST_ROUTE_FAIL, 0);
    AMTRL3_OM_SetDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV6_ADD_NET_ROUTE_FAIL, 0);
    return;
} /* end of AMTRL3_MGR_ClearDebugCounter() */

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_MeasureHostRoutePerformance
 * -------------------------------------------------------------------------
 * PURPOSE: Measure the time it takes to write single host route entry.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETURN:  None.
 * NOTES:   None.
 * -------------------------------------------------------------------------*/
void AMTRL3_MGR_MeasureHostRoutePerformance(void)
{
    amtrl3_mgr_hostroute_performance_flag = (!amtrl3_mgr_hostroute_performance_flag);
    BACKDOOR_MGR_Printf ("Measure Host Route Performance flag\t%s\n",
            (amtrl3_mgr_hostroute_performance_flag  & TRUE) ? "ON":"OFF");
    return;

} /* end of AMTRL3_MGR_MeasureHostRoutePerformance() */

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_MeasureNetRoutePerformance
 * -------------------------------------------------------------------------
 * PURPOSE: Measure the time it takes to write single net route entry.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETURN:  None.
 * NOTES:   None.
 * -------------------------------------------------------------------------*/
void AMTRL3_MGR_MeasureNetRoutePerformance(void)
{
    amtrl3_mgr_netroute_performance_flag = (!amtrl3_mgr_netroute_performance_flag);
    BACKDOOR_MGR_Printf ("Measure Net Route Performance flag\t%s\n",
            (amtrl3_mgr_netroute_performance_flag  & TRUE) ? "ON":"OFF");

    return;
} /* end of AMTRL3_MGR_MeasureNetRoutePerformance() */

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_SetArpRequestFeature
 * -------------------------------------------------------------------------
 * PURPOSE: To Disable / Enable AMTRL3 ARP Request Action
 * INPUT:   debug_flag
 * OUTPUT:  None.
 * RETURN:  None.
 * NOTES:   For backdoor only.
 * -------------------------------------------------------------------------*/
void AMTRL3_MGR_SetArpRequestFeature(I32_T debug_flag)
{
    char   display_str[30];

    amtrl3_mgr_arp_action = debug_flag;

    memset(display_str, 0, sizeof(display_str));
    if (amtrl3_mgr_arp_action == AMTRL3_MGR_NORMAL_OPERATION)
        strcpy(display_str, "Normal Opeation");
    else if (amtrl3_mgr_arp_action == AMTRL3_MGR_DISABLE_ARP_UNRESOLVED)
        strcpy(display_str, "DISALBE_ARP_UNRESOLVED");
    else if (amtrl3_mgr_arp_action == AMTRL3_MGR_DISABLE_ARP_GATEWAY)
        strcpy(display_str, "AMTRL3_MGR_DISABLE_ARP_GATEWAY");
    else if (amtrl3_mgr_arp_action == AMTRL3_MGR_DISALBE_ARP_ALL)
        strcpy(display_str, "AMTRL3_MGR_DISALBE_ARP_ALL");
    else
        strcpy(display_str, "None");

    BACKDOOR_MGR_Printf ("ARP Request Tx action: %s\n", display_str);
    return;

} /* end of AMTRL3_MGR_SetArpRequestFeature() */

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_SetHostRouteScanningOperation
 * -------------------------------------------------------------------------
 * PURPOSE: To Disable / Enable scan host route hit bit operation
 * INPUT:   debug_flag
 * OUTPUT:  None.
 * RETURN:  None.
 * NOTES:   For backdoor only.
 * -------------------------------------------------------------------------*/
void AMTRL3_MGR_SetHostRouteScanningOperation(void)
{
    BACKDOOR_MGR_Printf("\nON:  Enable periodic timer event to scan and clear host route hit bit\n");
    BACKDOOR_MGR_Printf("OFF: Disable periodic timer event to scan and clear host route hit bit\n");
    amtrl3_mgr_hostroute_scanning_mode = (!amtrl3_mgr_hostroute_scanning_mode);
    BACKDOOR_MGR_Printf("\nHost Route Scanning Mode is %s\n", (amtrl3_mgr_hostroute_scanning_mode == 0) ? "ON" : "OFF");
    return;
} /* end of AMTRL3_MGR_SetHostRouteScanningOperation() */

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_SetDeleteHostRouteOperation
 * -------------------------------------------------------------------------
 * PURPOSE: To Disable / Enable delete host route operation
 * INPUT:   debug_flag
 * OUTPUT:  None.
 * RETURN:  None.
 * NOTES:   For backdoor only.
 * -------------------------------------------------------------------------*/
void AMTRL3_MGR_SetDeleteHostRouteOperation(void)
{
    BACKDOOR_MGR_Printf("\nON:  Host Route is removable\n");
    BACKDOOR_MGR_Printf("OFF: Host Route cannot be removed\n");
    amtrl3_mgr_hostroute_delete_mode = (!amtrl3_mgr_hostroute_delete_mode);
    BACKDOOR_MGR_Printf("\nHost Route Delete Mode is %s\n", (amtrl3_mgr_hostroute_delete_mode == 0) ? "ON" : "OFF");

    return;
} /* end of AMTRL3_MGR_SetDeleteHostRouteOperation() */

/* --------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_Ntoa
 * -------------------------------------------------------------------------
 * PURPOSE: Converts the network address into a character string.
 * INPUT:   flag: AMTRL3_TYPE_FLAGS_IPV4 \ AMTRL3_TYPE_FLAGS_IPV6
 *          address: ipv4 or ipv6 address
 * OUTPUT:  ip_str: the string to store the ip string
 * RETURN:  the ip string
 * NOTES:   None.
 *--------------------------------------------------------------------------*/
const UI8_T * AMTRL3_MGR_Ntoa(const L_INET_AddrIp_T *address, UI8_T *ip_str)
{
    //int family;
    UI32_T flag = 0;
    UI8_T * cp;

    if((address->type== L_INET_ADDR_TYPE_IPV4) ||
       (address->type== L_INET_ADDR_TYPE_IPV4Z))
        //family = IP_AF_INET;
        flag = AMTRL3_TYPE_FLAGS_IPV4;
    else if((address->type== L_INET_ADDR_TYPE_IPV6) ||
            (address->type== L_INET_ADDR_TYPE_IPV6Z))
        //family = IP_AF_INET6;
        flag = AMTRL3_TYPE_FLAGS_IPV6;

    if(flag == AMTRL3_TYPE_FLAGS_IPV4)
        return L_INET_Ntoa(*(UI32_T*)address->addr, ip_str);
    else
    {
        //cp = (UI8_T *)&ip_str;
        cp = ip_str;
        //sprintf((char *)cp, "%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x",cp[0],cp[1],cp[2],cp[3],cp[4],cp[5],cp[6],cp[7],cp[8],cp[9],cp[10],cp[11],cp[12],cp[13],cp[14],cp[15]);
        sprintf((char *)cp, "%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x",address->addr[0],address->addr[1],address->addr[2],address->addr[3],address->addr[4],address->addr[5],address->addr[6],address->addr[7],address->addr[8],address->addr[9],address->addr[10],address->addr[11],address->addr[12],address->addr[13],address->addr[14],address->addr[15]);

        if(address->zoneid)
            sprintf((char *)cp + strlen((char *)cp), "%%%d", (int)address->zoneid);
        return cp;
    }

    //return pal_inet_ntop(family, &(address->addr), ip_str, 40);
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_HostRouteEventForNetRoute
 * -------------------------------------------------------------------------
 * PURPOSE:  The action for Net route when host route states change
 * INPUT:    host_entry  - host information before update
 *           fib_id
 * OUTPUT:   none.
 * RETURN:   none
 * NOTES:
 * -------------------------------------------------------------------------*/
static void AMTRL3_MGR_HostRouteEventForNetRoute(UI32_T action_flags,
                                                 UI32_T fib_id,
                                                 AMTRL3_OM_HostRouteEntry_T *host_entry)
{
    AMTRL3_OM_HostRouteEntry_T old_host_entry;
    char ip_str[L_INET_MAX_IPADDR_STR_LEN] = {0};

    memset(&old_host_entry, 0, sizeof(AMTRL3_OM_HostRouteEntry_T));
    old_host_entry.key_fields.dst_vid_ifindex = host_entry->key_fields.dst_vid_ifindex;
    old_host_entry.key_fields.dst_inet_addr = host_entry->key_fields.dst_inet_addr;
    if(AMTRL3_OM_GetHostRouteEntry(action_flags, fib_id, &old_host_entry))
    {
        if((host_entry->status == HOST_ROUTE_UNRESOLVED) &&
           ((host_entry->old_status == HOST_ROUTE_GATEWAY_READY) || (host_entry->old_status == HOST_ROUTE_READY_NOT_SYNC)))
        {
            if(amtrl3_mgr_debug_flag & DEBUG_LEVEL_TRACE_HOST)
            {
                BACKDOOR_MGR_Printf("Net route nexthop %s change state to unresolved. Delete all related net route!\n", ip_str);
            }
            AMTRL3_MGR_DeleteAllRelatedNetRouteFromChip(action_flags, fib_id, &old_host_entry);
            host_entry->ref_count_in_chip = 0;
            host_entry->hw_info = old_host_entry.hw_info;
        }
        else if(((host_entry->old_status == HOST_ROUTE_GATEWAY_READY)
                 || (host_entry->old_status == HOST_ROUTE_READY_NOT_SYNC)) &&
                ((host_entry->status == HOST_ROUTE_GATEWAY_READY)
                 || (host_entry->status == HOST_ROUTE_READY_NOT_SYNC)))
        {
            if(!CHECK_FLAG(amtrl3_chip_capability_flags, AMTRL3_CHIP_SUPPORT_EGRESS_OBJECT))
            {
                if((memcmp(host_entry->key_fields.dst_mac, old_host_entry.key_fields.dst_mac, SYS_ADPT_MAC_ADDR_LEN) != 0) ||
                    (host_entry->key_fields.dst_vid_ifindex != old_host_entry.key_fields.dst_vid_ifindex) ||
                    (host_entry->key_fields.lport != old_host_entry.key_fields.lport) ||
                    (host_entry->uport != old_host_entry.uport))
                {
                    if(amtrl3_mgr_debug_flag & DEBUG_LEVEL_TRACE_HOST)
                    {
                        BACKDOOR_MGR_Printf("Net Nexthop %s old info: lport = %d, uport = %d, vid = %d, MAC = %02x-%02x-%02x-%02x-%02x-%02x, status = %d\n",
                                ip_str, (int)old_host_entry.key_fields.lport, (int)old_host_entry.uport, (int)old_host_entry.key_fields.dst_vid_ifindex,
                                old_host_entry.key_fields.dst_mac[0], old_host_entry.key_fields.dst_mac[1], old_host_entry.key_fields.dst_mac[2],
                                old_host_entry.key_fields.dst_mac[3], old_host_entry.key_fields.dst_mac[4], old_host_entry.key_fields.dst_mac[5],
                                host_entry->old_status);
                        BACKDOOR_MGR_Printf("Net Nexthop %s new info:lport = %d, uport = %d, vid = %d, MAC = %02x-%02x-%02x-%02x-%02x-%02x, status = %d\n",
                                ip_str, (int)host_entry->key_fields.lport, (int)host_entry->uport, (int)host_entry->key_fields.dst_vid_ifindex,
                                host_entry->key_fields.dst_mac[0], host_entry->key_fields.dst_mac[1], host_entry->key_fields.dst_mac[2],
                                host_entry->key_fields.dst_mac[3], host_entry->key_fields.dst_mac[4], host_entry->key_fields.dst_mac[5],
                                host_entry->status);
                        BACKDOOR_MGR_Printf("Delete all net route use this host as nexthop!\n");
                    }
                    AMTRL3_MGR_DeleteAllRelatedNetRouteFromChip(action_flags, fib_id, &old_host_entry);
                    host_entry->ref_count_in_chip = 0;
                    host_entry->hw_info = old_host_entry.hw_info;
                }
            }
        }
    }
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_HostRouteEventForECMPRoute
 * -------------------------------------------------------------------------
 * PURPOSE:  The action for ecmp route when host route states change
 * INPUT:    host_entry  - host information before update
 *           fib_id
 * OUTPUT:   none.
 * RETURN:   none
 * NOTES:
 * -------------------------------------------------------------------------*/
static void AMTRL3_MGR_HostRouteEventForECMPRoute(UI32_T action_flags,
                                                  UI32_T fib_id,
                                                  AMTRL3_OM_HostRouteEntry_T *host_entry)
{
    AMTRL3_OM_HostRouteEntry_T old_host_entry;
    char ip_str[L_INET_MAX_IPADDR_STR_LEN] = {0};

    memset(&old_host_entry, 0, sizeof(AMTRL3_OM_HostRouteEntry_T));
    old_host_entry.key_fields.dst_vid_ifindex = host_entry->key_fields.dst_vid_ifindex;
    old_host_entry.key_fields.dst_inet_addr = host_entry->key_fields.dst_inet_addr;
    if(AMTRL3_OM_GetHostRouteEntry(action_flags, fib_id, &old_host_entry))
    {
        if((host_entry->status == HOST_ROUTE_UNRESOLVED) &&
           ((host_entry->old_status == HOST_ROUTE_GATEWAY_READY) || (host_entry->old_status == HOST_ROUTE_READY_NOT_SYNC)))
        {
            if(amtrl3_mgr_debug_flag & DEBUG_LEVEL_TRACE_HOST)
            {
                BACKDOOR_MGR_Printf("ECMP nexthop %s change state to unresolved. Delete all related ECMP route!\n", ip_str);
            }
            AMTRL3_MGR_DeleteAllRelatedEcmpRouteFromChip(action_flags, fib_id, &old_host_entry);
            host_entry->ref_count_in_chip = 0;
            host_entry->hw_info = old_host_entry.hw_info;
        }
        else if(((host_entry->old_status == HOST_ROUTE_GATEWAY_READY)
                 || (host_entry->old_status == HOST_ROUTE_READY_NOT_SYNC)) &&
                ((host_entry->status == HOST_ROUTE_GATEWAY_READY)
                 || (host_entry->status == HOST_ROUTE_READY_NOT_SYNC)))
        {
            if(!CHECK_FLAG(amtrl3_chip_capability_flags, AMTRL3_CHIP_SUPPORT_EGRESS_OBJECT))
            {
                if((memcmp(host_entry->key_fields.dst_mac, old_host_entry.key_fields.dst_mac, SYS_ADPT_MAC_ADDR_LEN) != 0) ||
                    (host_entry->key_fields.dst_vid_ifindex != old_host_entry.key_fields.dst_vid_ifindex) ||
                    (host_entry->key_fields.lport != old_host_entry.key_fields.lport) ||
                    (host_entry->uport != old_host_entry.uport))
                {
                    if(amtrl3_mgr_debug_flag & DEBUG_LEVEL_TRACE_HOST)
                    {
                        BACKDOOR_MGR_Printf("ECMP Nexthop %s old info: lport = %d, uport = %d, vid = %d, MAC = %02x-%02x-%02x-%02x-%02x-%02x, status = %d\n",
                                ip_str, (int)old_host_entry.key_fields.lport, (int)old_host_entry.uport, (int)old_host_entry.key_fields.dst_vid_ifindex,
                                old_host_entry.key_fields.dst_mac[0], old_host_entry.key_fields.dst_mac[1], old_host_entry.key_fields.dst_mac[2],
                                old_host_entry.key_fields.dst_mac[3], old_host_entry.key_fields.dst_mac[4], old_host_entry.key_fields.dst_mac[5],
                                host_entry->old_status);
                        BACKDOOR_MGR_Printf("ECMP Nexthop %s new info:lport = %d, uport = %d, vid = %d, MAC = %02x-%02x-%02x-%02x-%02x-%02x, status = %d\n",
                                ip_str, (int)host_entry->key_fields.lport, (int)host_entry->uport, (int)host_entry->key_fields.dst_vid_ifindex,
                                host_entry->key_fields.dst_mac[0], host_entry->key_fields.dst_mac[1], host_entry->key_fields.dst_mac[2],
                                host_entry->key_fields.dst_mac[3], host_entry->key_fields.dst_mac[4], host_entry->key_fields.dst_mac[5],
                                host_entry->status);
                        BACKDOOR_MGR_Printf("Delete all ECMP route use this host as nexthop!\n");
                    }
                    AMTRL3_MGR_DeleteAllRelatedEcmpRouteFromChip(action_flags, fib_id, &old_host_entry);
                    host_entry->ref_count_in_chip = 0;
                    host_entry->hw_info = old_host_entry.hw_info;
                }
            }
        }
    }
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_DeleteAllRelatedNetRouteFromChip
 * -------------------------------------------------------------------------
 * PURPOSE:  Delete all Net route before update nexthop information
 * INPUT:    host_entry  - host information before update
 *           fib_id
 * OUTPUT:   none.
 * RETURN:   none
 * NOTES:    1. TODO: if too many route need delete from chip, shall only
 *              process part of them on time.
 * -------------------------------------------------------------------------*/
static void AMTRL3_MGR_DeleteAllRelatedNetRouteFromChip(UI32_T action_flags,
                                                        UI32_T fib_id,
                                                        AMTRL3_OM_HostRouteEntry_T *host_entry)
{
    /* Local Variable Declaration
     */
    UI32_T      process_count;
    UI32_T      num_of_entry = 0;
    BOOL_T      sync_complete_flag, ret = FALSE;
    UI8_T       ip_str[L_INET_MAX_IPADDR_STR_LEN] = {0};
    AMTRL3_OM_NetRouteEntry_T         *local_net_entry;
    AMTRL3_OM_NetRouteEntry_T         net_route_entry_block[AMTRL3_MGR_NBR_OF_RESOLVED_NET_ENTRY_TO_PROCESS];
    AMTRL3_OM_ResolvedNetRouteEntry_T resolved_net_entry;

    /* BODY
     */
    sync_complete_flag = ret = FALSE;
    memset(net_route_entry_block, 0, sizeof(AMTRL3_OM_NetRouteEntry_T) * AMTRL3_MGR_NBR_OF_RESOLVED_NET_ENTRY_TO_PROCESS);

    if (amtrl3_mgr_debug_flag & DEBUG_LEVEL_TRACE_HOST)
    {
        AMTRL3_MGR_Ntoa(&(host_entry->key_fields.dst_inet_addr), ip_str);
        BACKDOOR_MGR_Printf ("AMTRL3_MGR_DeleteAllRelatedNetRouteFromChip, nexthop:%s\n", ip_str);
    }

    /* Use GetNextN mechanism to get multiple number of record at once to improve
     * performance
     */
    local_net_entry = (AMTRL3_OM_NetRouteEntry_T*)net_route_entry_block;
    local_net_entry->inet_cidr_route_entry.inet_cidr_route_next_hop = host_entry->key_fields.dst_inet_addr;

    while(!sync_complete_flag)
    {
        ret = AMTRL3_OM_GetNextNNetRouteEntryByNextHop(action_flags,
                                                       fib_id,
                                                       AMTRL3_MGR_NBR_OF_RESOLVED_NET_ENTRY_TO_PROCESS,
                                                       &num_of_entry,
                                                       net_route_entry_block);
        if (ret != TRUE)
        {
            break;
        }

        /* Process each entry individually
         */
        for (process_count = 0; process_count < num_of_entry; process_count++)
        {
            local_net_entry = (AMTRL3_OM_NetRouteEntry_T *)(&net_route_entry_block[process_count]);

            /* If net entry is associated with different gateway, do not process it
             */
            if (AMTRL3_OM_IsAddressEqual(&(local_net_entry->inet_cidr_route_entry.inet_cidr_route_next_hop),
                                          &(host_entry->key_fields.dst_inet_addr)) == FALSE)
            {
                sync_complete_flag = TRUE;
                break;
            } /* end of if */

            if(!CHECK_FLAG(local_net_entry->flags, AMTRL3_TYPE_FLAGS_ECMP))
            {
                if(local_net_entry->net_route_status == AMTRL3_OM_NET_ROUTE_READY)
                {
                    AMTRL3_MGR_DeleteNetRouteFromChip(action_flags, fib_id, local_net_entry, host_entry);
                }
                else if(local_net_entry->net_route_status == AMTRL3_OM_NET_ROUTE_RESOLVED)
                {
                    memset(&resolved_net_entry, 0, sizeof(resolved_net_entry));
                    if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
                        resolved_net_entry.inverse_prefix_length = (AMTRL3_MGR_IPV4_MAXIMUM_PREFIX_LENGTH - local_net_entry->inet_cidr_route_entry.inet_cidr_route_pfxlen);
                    else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
                        resolved_net_entry.inverse_prefix_length = (AMTRL3_MGR_IPV6_MAXIMUM_PREFIX_LENGTH - local_net_entry->inet_cidr_route_entry.inet_cidr_route_pfxlen);
                    resolved_net_entry.inet_cidr_route_dest = local_net_entry->inet_cidr_route_entry.inet_cidr_route_dest;
                    resolved_net_entry.inet_cidr_route_next_hop = local_net_entry->inet_cidr_route_entry.inet_cidr_route_next_hop;

                    if (AMTRL3_OM_DeleteResolvedNetEntry(action_flags, fib_id, &resolved_net_entry))
                    {
                        if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
                            AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_IPV4_RESOLVED_NET_ROUTE_COUNT, 1);
                        else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
                            AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_IPV6_RESOLVED_NET_ROUTE_COUNT, 1);
                    }
                    else
                    {
                        if (amtrl3_mgr_debug_flag & DEBUG_LEVEL_ERROR)
                            BACKDOOR_MGR_Printf("AMTRL3_MGR_DeleteAllRelatedNetRouteFromChip: AMTRL3_OM_DeleteResolvedNetEntry Fails\n");
                    }
                }
                else
                {
                    continue;
                }

                local_net_entry->net_route_status = AMTRL3_OM_NET_ROUTE_UNRESOLVED;

                if (AMTRL3_OM_SetNetRouteEntry(action_flags, fib_id, local_net_entry) != TRUE)
                {
                    if (amtrl3_mgr_debug_flag & DEBUG_LEVEL_ERROR)
                        BACKDOOR_MGR_Printf("AMTRL3_MGR_DeleteAllRelatedNetRouteFromChip: AMTRL3_OM_SetNetRouteEntry\n");
                }
            }
        } /* end of for */

        if (num_of_entry == AMTRL3_MGR_NBR_OF_RESOLVED_NET_ENTRY_TO_PROCESS)
        {
            if (num_of_entry != 1)
            {
                memcpy(&net_route_entry_block[0], &net_route_entry_block[num_of_entry-1], sizeof(AMTRL3_OM_NetRouteEntry_T));
            }
        }
        else
        {
            /* All net route entry has been processed
             */
            sync_complete_flag = TRUE;
        }
    } /* end of while */

    AMTRL3_MGR_UpdateHostRouteRefCountInChip(action_flags, fib_id, host_entry, -host_entry->ref_count_in_chip);

    return;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_DeleteAllRelatedEcmpRouteFromChip
 * -------------------------------------------------------------------------
 * PURPOSE:  Delete all ECMP route before update nexthop information
 * INPUT:    host_entry  - host information before update
 *           fib_id
 * OUTPUT:   none.
 * RETURN:   none
 * NOTES:    1. TODO: if too many route need delete from chip, shall only
 *              process part of them on time.
 * -------------------------------------------------------------------------*/
static void AMTRL3_MGR_DeleteAllRelatedEcmpRouteFromChip(
                                                    UI32_T action_flags,
                                                    UI32_T fib_id,
                                                    AMTRL3_OM_HostRouteEntry_T *host_entry)
{
    /* Local Variable Declaration
     */
    UI32_T      process_count;
    UI32_T      num_of_entry = 0;
    BOOL_T      sync_complete_flag, ret = FALSE;
    UI8_T       ip_str[L_INET_MAX_IPADDR_STR_LEN] = {0};
    AMTRL3_OM_NetRouteEntry_T         *local_net_entry;
    AMTRL3_OM_NetRouteEntry_T         net_route_entry_block[AMTRL3_MGR_NBR_OF_RESOLVED_NET_ENTRY_TO_PROCESS];
    AMTRL3_OM_ResolvedNetRouteEntry_T resolved_net_entry;

    /* BODY
     */
    sync_complete_flag = ret = FALSE;
    memset(net_route_entry_block, 0, sizeof(AMTRL3_OM_NetRouteEntry_T) * AMTRL3_MGR_NBR_OF_RESOLVED_NET_ENTRY_TO_PROCESS);

    if (amtrl3_mgr_debug_flag & DEBUG_LEVEL_TRACE_HOST)
    {
        AMTRL3_MGR_Ntoa(&(host_entry->key_fields.dst_inet_addr), ip_str);
        BACKDOOR_MGR_Printf ("AMTRL3_MGR_DeleteAllRelatedEcmpRouteFromChip, nexthop:%s\n", ip_str);
    }

    /* Use GetNextN mechanism to get multiple number of record at once to improve
     * performance
     */
    local_net_entry = (AMTRL3_OM_NetRouteEntry_T*)net_route_entry_block;
    local_net_entry->inet_cidr_route_entry.inet_cidr_route_next_hop = host_entry->key_fields.dst_inet_addr;

    while(!sync_complete_flag)
    {
        ret = AMTRL3_OM_GetNextNNetRouteEntryByNextHop(action_flags,
                                                       fib_id,
                                                       AMTRL3_MGR_NBR_OF_RESOLVED_NET_ENTRY_TO_PROCESS,
                                                       &num_of_entry,
                                                       net_route_entry_block);
        if (ret != TRUE)
        {
            return;
        }

        /* Process each entry individually
         */
        for (process_count = 0; process_count < num_of_entry; process_count++)
        {
            local_net_entry = (AMTRL3_OM_NetRouteEntry_T *)(&net_route_entry_block[process_count]);

            /* If net entry is associated with different gateway, do not process it
             */
            if (AMTRL3_OM_IsAddressEqual(&(local_net_entry->inet_cidr_route_entry.inet_cidr_route_next_hop),
                                          &(host_entry->key_fields.dst_inet_addr)) == FALSE)
            {
                sync_complete_flag = TRUE;
                break;
            } /* end of if */

            if(CHECK_FLAG(local_net_entry->flags, AMTRL3_TYPE_FLAGS_ECMP))
            {
                if(local_net_entry->net_route_status == AMTRL3_OM_NET_ROUTE_READY)
                {
                    BOOL_T is_last = FALSE;

                    is_last = AMTRL3_MGR_IsLastPathOfECMPRouteInOm(action_flags, fib_id, local_net_entry);
                    //AMTRL3_MGR_DeleteNetRouteFromChip(action_flags, fib_id, local_net_entry, host_entry);
                    AMTRL3_MGR_DeleteECMPRouteOnePathFromChip(action_flags, fib_id, local_net_entry, host_entry, is_last);
                    if (is_last == TRUE)
                        AMTRL3_MGR_ClearNetRouteHWInfo(action_flags | AMTRL3_TYPE_FLAGS_ECMP, fib_id, local_net_entry);
                }
                else if(local_net_entry->net_route_status == AMTRL3_OM_NET_ROUTE_RESOLVED)
                {
                    memset(&resolved_net_entry, 0, sizeof(resolved_net_entry));
                    if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
                        resolved_net_entry.inverse_prefix_length = (AMTRL3_MGR_IPV4_MAXIMUM_PREFIX_LENGTH - local_net_entry->inet_cidr_route_entry.inet_cidr_route_pfxlen);
                    else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
                        resolved_net_entry.inverse_prefix_length = (AMTRL3_MGR_IPV6_MAXIMUM_PREFIX_LENGTH - local_net_entry->inet_cidr_route_entry.inet_cidr_route_pfxlen);
                    resolved_net_entry.inet_cidr_route_dest = local_net_entry->inet_cidr_route_entry.inet_cidr_route_dest;
                    resolved_net_entry.inet_cidr_route_next_hop = local_net_entry->inet_cidr_route_entry.inet_cidr_route_next_hop;

                    if (AMTRL3_OM_DeleteResolvedNetEntry(action_flags, fib_id, &resolved_net_entry))
                    {
                        if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
                            AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_IPV4_RESOLVED_NET_ROUTE_COUNT, 1);
                        else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
                            AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_IPV6_RESOLVED_NET_ROUTE_COUNT, 1);
                    }
                    else
                    {
                        if (amtrl3_mgr_debug_flag & DEBUG_LEVEL_ERROR)
                            BACKDOOR_MGR_Printf("AMTRL3_MGR_DeleteAllRelatedEcmpRouteFromChip: AMTRL3_OM_DeleteResolvedNetEntry Fails\n");
                    }
                }
                else
                {
                    continue;
                }

                local_net_entry->net_route_status = AMTRL3_OM_NET_ROUTE_UNRESOLVED;

                if (AMTRL3_OM_SetNetRouteEntry(action_flags | AMTRL3_TYPE_FLAGS_ECMP, fib_id, local_net_entry) != TRUE)
                {
                    if (amtrl3_mgr_debug_flag & DEBUG_LEVEL_ERROR)
                        BACKDOOR_MGR_Printf("AMTRL3_MGR_DeleteAllRelatedEcmpRouteFromChip: AMTRL3_OM_SetNetRouteEntry\n");
                }
            }
        } /* end of for */

        if (num_of_entry == AMTRL3_MGR_NBR_OF_RESOLVED_NET_ENTRY_TO_PROCESS)
        {
            if (num_of_entry != 1)
            {
                memcpy(&net_route_entry_block[0], &net_route_entry_block[num_of_entry-1], sizeof(AMTRL3_OM_NetRouteEntry_T));
            }
        }
        else
        {
            /* All net route entry has been processed
             */
            sync_complete_flag = TRUE;
        }
    } /* end of while */

    return;
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - AMTRL3_MGR_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void AMTRL3_MGR_Create_InterCSC_Relation(void)
{
    /* register callbacks */
#if (AMTRL3_SUPPORT_ACCTON_BACKDOOR == TRUE)
    BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack("AMTRL3",
        SYS_BLD_AMTRL3_GROUP_IPCMSGQ_KEY, AMTRL3_Backdoor_CallBack);
#endif

    return;
}

/* FUNCTION NAME : AMTRL3_MGR_HandleIPCReqMsg
 * PURPOSE:
 *    Handle the ipc request message for AMTRL3 mgr.
 * INPUT:
 *    ipcmsg_p  --  input request ipc message buffer
 *
 * OUTPUT:
 *    ipcmsg_p  --  output response ipc message buffer
 *
 * RETURN:
 *    TRUE  - There is a response need to send.
 *    FALSE - There is no response to send.
 *
 * NOTES:
 *    None.
 *
 */
BOOL_T AMTRL3_MGR_HandleIPCReqMsg(SYSFUN_Msg_T* ipcmsg_p)
{
    AMTRL3_MGR_IPCMsg_T *amtrl3_mgr_msg_p;
    BOOL_T need_respond = TRUE;

    if(ipcmsg_p == NULL)
        return FALSE;

    amtrl3_mgr_msg_p = (AMTRL3_MGR_IPCMsg_T*)ipcmsg_p->msg_buf;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_TRANSITION_MODE)
    {
        if(amtrl3_mgr_msg_p->type.cmd < AMTRL3_MGR_IPCCMD_FOLLOWISASYNCHRONISMIPC)
        {
            amtrl3_mgr_msg_p->type.result_ui32 = AMTRL3_TYPE_FAIL;
            ipcmsg_p->msg_size = AMTRL3_MGR_MSGBUF_TYPE_SIZE;
            need_respond = TRUE;
        }
        else
        {
            need_respond = FALSE;
        }

        return need_respond;
    }

    switch(amtrl3_mgr_msg_p->type.cmd)
    {
        /* System Wise Configuration */
        case AMTRL3_MGR_IPCCMD_CREATEFIB:
            amtrl3_mgr_msg_p->type.result_ui32 = AMTRL3_MGR_CreateFIB(amtrl3_mgr_msg_p->data.ui32_v);
            ipcmsg_p->msg_size = AMTRL3_MGR_MSGBUF_TYPE_SIZE;
            need_respond = TRUE;
            break;
        case AMTRL3_MGR_IPCCMD_DELETEFIB:
            amtrl3_mgr_msg_p->type.result_ui32 = AMTRL3_MGR_DeleteFIB(amtrl3_mgr_msg_p->data.ui32_v);
            ipcmsg_p->msg_size = AMTRL3_MGR_MSGBUF_TYPE_SIZE;
            need_respond = TRUE;
            break;
        case AMTRL3_MGR_IPCCMD_ENABLEIPFORWARDING:
            amtrl3_mgr_msg_p->type.result_bool = AMTRL3_MGR_EnableIpForwarding(
                          amtrl3_mgr_msg_p->data.ip_forwarding_status_index.action_flags,
                          amtrl3_mgr_msg_p->data.ip_forwarding_status_index.vr_id);
            ipcmsg_p->msg_size = AMTRL3_MGR_MSGBUF_TYPE_SIZE;
            need_respond = TRUE;
            break;
         case AMTRL3_MGR_IPCCMD_DISABLEIPFORWARDING:
            amtrl3_mgr_msg_p->type.result_bool = AMTRL3_MGR_DisableIpForwarding(
                          amtrl3_mgr_msg_p->data.ip_forwarding_status_index.action_flags,
                          amtrl3_mgr_msg_p->data.ip_forwarding_status_index.vr_id);
            ipcmsg_p->msg_size = AMTRL3_MGR_MSGBUF_TYPE_SIZE;
            need_respond = TRUE;
            break;
        case AMTRL3_MGR_IPCCMD_SETHOSTROUTE:
            amtrl3_mgr_msg_p->type.result_bool = AMTRL3_MGR_SetHostRoute(
                          amtrl3_mgr_msg_p->data.host_route.action_flags,
                          amtrl3_mgr_msg_p->data.host_route.fib_id,
                          &amtrl3_mgr_msg_p->data.host_route.host_entry,
                          NULL,
                          amtrl3_mgr_msg_p->data.host_route.type);
            ipcmsg_p->msg_size = AMTRL3_MGR_MSGBUF_TYPE_SIZE;
            need_respond = TRUE;
            break;
        case AMTRL3_MGR_IPCCMD_DELETEHOSTROUTE:
            amtrl3_mgr_msg_p->type.result_bool = AMTRL3_MGR_DeleteHostRoute(
                          amtrl3_mgr_msg_p->data.host_route.action_flags,
                          amtrl3_mgr_msg_p->data.host_route.fib_id,
                          &amtrl3_mgr_msg_p->data.host_route.host_entry);
            ipcmsg_p->msg_size = AMTRL3_MGR_MSGBUF_TYPE_SIZE;
            need_respond = TRUE;
            break;
#if (SYS_CPNT_PBR == TRUE)
        case AMTRL3_MGR_IPCCMD_SETHOSTROUTEFORPBR:
            amtrl3_mgr_msg_p->type.result_bool = AMTRL3_MGR_SetHostRouteForPbr(
                          amtrl3_mgr_msg_p->data.host_route.action_flags,
                          amtrl3_mgr_msg_p->data.host_route.fib_id,
                          &amtrl3_mgr_msg_p->data.host_route.host_entry);
            ipcmsg_p->msg_size = AMTRL3_MGR_MSGBUF_TYPE_SIZE;
            need_respond = TRUE;
            break;
        case AMTRL3_MGR_IPCCMD_DELETEHOSTROUTEFORPBR:
            amtrl3_mgr_msg_p->type.result_bool = AMTRL3_MGR_DeleteHostRouteForPbr(
                          amtrl3_mgr_msg_p->data.host_route.action_flags,
                          amtrl3_mgr_msg_p->data.host_route.fib_id,
                          &amtrl3_mgr_msg_p->data.host_route.host_entry);
            ipcmsg_p->msg_size = AMTRL3_MGR_MSGBUF_TYPE_SIZE;
            need_respond = TRUE;
            break;
#endif
        case AMTRL3_MGR_IPCCMD_REPLACEHOSTROUTE:
            amtrl3_mgr_msg_p->type.result_bool = AMTRL3_MGR_ReplaceExistHostRoute(
                          amtrl3_mgr_msg_p->data.host_route.action_flags,
                          amtrl3_mgr_msg_p->data.host_route.fib_id,
                          &amtrl3_mgr_msg_p->data.host_route.host_entry,
                          amtrl3_mgr_msg_p->data.host_route.type);
            ipcmsg_p->msg_size = AMTRL3_MGR_MSGBUF_TYPE_SIZE;
            need_respond = TRUE;
            break;
        case AMTRL3_MGR_IPCCMD_SETHOSTROUTETIMEOUT:
            amtrl3_mgr_msg_p->type.result_bool = AMTRL3_MGR_SetIpNetToPhysicalEntryTimeout(
                          amtrl3_mgr_msg_p->data.host_timeout.action_flags,
                          amtrl3_mgr_msg_p->data.host_timeout.fib_id,
                          amtrl3_mgr_msg_p->data.host_timeout.v4_timeout,
                          amtrl3_mgr_msg_p->data.host_timeout.v6_timeout);
            ipcmsg_p->msg_size = AMTRL3_MGR_MSGBUF_TYPE_SIZE;
            need_respond = TRUE;
            break;
        case AMTRL3_MGR_IPCCMD_SETINETCIDRROUTE:
            amtrl3_mgr_msg_p->type.result_bool = AMTRL3_MGR_SetInetCidrRouteEntry(
                          amtrl3_mgr_msg_p->data.net_route.action_flags,
                          amtrl3_mgr_msg_p->data.net_route.fib_id,
                          &amtrl3_mgr_msg_p->data.net_route.net_route_entry);
            ipcmsg_p->msg_size = AMTRL3_MGR_MSGBUF_TYPE_SIZE;
            need_respond = TRUE;
            break;
        case AMTRL3_MGR_IPCCMD_DELETEINETCIDRROUTE:
            amtrl3_mgr_msg_p->type.result_bool = AMTRL3_MGR_DeleteInetCidrRouteEntry(
                          amtrl3_mgr_msg_p->data.net_route.action_flags,
                          amtrl3_mgr_msg_p->data.net_route.fib_id,
                          &amtrl3_mgr_msg_p->data.net_route.net_route_entry);
            need_respond = TRUE;
            break;
        case AMTRL3_MGR_IPCCMD_ADDECMPROUTEMULTIPATH:
            amtrl3_mgr_msg_p->type.result_bool = AMTRL3_MGR_AddECMPRouteMultiPath(
                          amtrl3_mgr_msg_p->data.ecmp_route.action_flags,
                          amtrl3_mgr_msg_p->data.ecmp_route.fib_id,
                          amtrl3_mgr_msg_p->data.ecmp_route.net_route_entry,
                          amtrl3_mgr_msg_p->data.ecmp_route.num);
            ipcmsg_p->msg_size = AMTRL3_MGR_MSGBUF_TYPE_SIZE;
            need_respond = TRUE;
            break;
        case AMTRL3_MGR_IPCCMD_DELETEECMPROUTE:
            amtrl3_mgr_msg_p->type.result_bool = AMTRL3_MGR_DeleteECMPRoute(
                          amtrl3_mgr_msg_p->data.net_route.action_flags,
                          amtrl3_mgr_msg_p->data.net_route.fib_id,
                          &amtrl3_mgr_msg_p->data.net_route.net_route_entry);
            ipcmsg_p->msg_size = AMTRL3_MGR_MSGBUF_TYPE_SIZE;
            need_respond = TRUE;
            break;
        case AMTRL3_MGR_IPCCMD_ADDECMPROUTEONEPATH:
            amtrl3_mgr_msg_p->type.result_bool = AMTRL3_MGR_AddECMPRouteOnePath(
                          amtrl3_mgr_msg_p->data.net_route.action_flags,
                          amtrl3_mgr_msg_p->data.net_route.fib_id,
                          &amtrl3_mgr_msg_p->data.net_route.net_route_entry);
            ipcmsg_p->msg_size = AMTRL3_MGR_MSGBUF_TYPE_SIZE;
            need_respond = TRUE;
            break;
        case AMTRL3_MGR_IPCCMD_DELETEECMPROUTEONEPATH:
            amtrl3_mgr_msg_p->type.result_bool = AMTRL3_MGR_DeleteECMPRouteOnePath(
                          amtrl3_mgr_msg_p->data.net_route.action_flags,
                          amtrl3_mgr_msg_p->data.net_route.fib_id,
                          &amtrl3_mgr_msg_p->data.net_route.net_route_entry);
            ipcmsg_p->msg_size = AMTRL3_MGR_MSGBUF_TYPE_SIZE;
            need_respond = TRUE;
            break;
        case AMTRL3_MGR_IPCCMD_CREATEL3INTERFACE:
            amtrl3_mgr_msg_p->type.result_bool = AMTRL3_MGR_CreateL3Interface(
                          amtrl3_mgr_msg_p->data.l3_if.fib_id,
                          amtrl3_mgr_msg_p->data.l3_if.route_mac,
                          amtrl3_mgr_msg_p->data.l3_if.ifindex);
            ipcmsg_p->msg_size = AMTRL3_MGR_MSGBUF_TYPE_SIZE;
            need_respond = TRUE;
            break;
        case AMTRL3_MGR_IPCCMD_DELETEL3INTERFACE:
            amtrl3_mgr_msg_p->type.result_bool = AMTRL3_MGR_DeleteL3Interface(
                          amtrl3_mgr_msg_p->data.l3_if.fib_id,
                          amtrl3_mgr_msg_p->data.l3_if.route_mac,
                          amtrl3_mgr_msg_p->data.l3_if.ifindex);
            ipcmsg_p->msg_size = AMTRL3_MGR_MSGBUF_TYPE_SIZE;
            need_respond = TRUE;
            break;
        case AMTRL3_MGR_IPCCMD_ADDL3MAC:
            amtrl3_mgr_msg_p->type.result_bool = AMTRL3_MGR_AddL3Mac(
                          amtrl3_mgr_msg_p->data.l3_if.route_mac,
                          amtrl3_mgr_msg_p->data.l3_if.ifindex);
            ipcmsg_p->msg_size = AMTRL3_MGR_MSGBUF_TYPE_SIZE;
            need_respond = TRUE;
            break;
        case AMTRL3_MGR_IPCCMD_DELETEL3MAC:
            amtrl3_mgr_msg_p->type.result_bool = AMTRL3_MGR_DeleteL3Mac(
                          amtrl3_mgr_msg_p->data.l3_if.route_mac,
                          amtrl3_mgr_msg_p->data.l3_if.ifindex);
            ipcmsg_p->msg_size = AMTRL3_MGR_MSGBUF_TYPE_SIZE;
            need_respond = TRUE;
            break;

        case AMTRL3_MGR_IPCCMD_CLEARALLDYNAMICARP:
            amtrl3_mgr_msg_p->type.result_bool = AMTRL3_MGR_ClearAllDynamicARP(
                          amtrl3_mgr_msg_p->data.clear_arp.action_flags,
                          amtrl3_mgr_msg_p->data.clear_arp.fib_id);
            ipcmsg_p->msg_size = AMTRL3_MGR_MSGBUF_TYPE_SIZE;
            need_respond = TRUE;
            break;
        case AMTRL3_MGR_IPCCMD_MACTABLEDELETEBYMSTIDONPORT:
            AMTRL3_MGR_MACTableDeleteByMstidOnPort(
                          amtrl3_mgr_msg_p->data.arg_ui32_ui32.ui32_1,
                          amtrl3_mgr_msg_p->data.arg_ui32_ui32.ui32_2);
            need_respond = FALSE;
            break;
        case AMTRL3_MGR_IPCCMD_SIGNAL_L3IF_RIF_DESTROY:
            AMTRL3_MGR_SignalL3IfRifDestroy(
                            amtrl3_mgr_msg_p->data.arg_ui32_addr.ui32_1,
                            &amtrl3_mgr_msg_p->data.arg_ui32_addr.addr_2);
            need_respond = FALSE;
            break;
#if (SYS_CPNT_IP_TUNNEL == TRUE)
        case AMTRL3_MGR_IPCCMD_DELETETUNNELENTRIES:
            amtrl3_mgr_msg_p->type.result_ui32 = AMTRL3_MGR_DeleteTunnelHostEntries(amtrl3_mgr_msg_p->data.arg_ui32_ui32.ui32_1, amtrl3_mgr_msg_p->data.arg_ui32_ui32.ui32_2);
            ipcmsg_p->msg_size = AMTRL3_MGR_MSGBUF_TYPE_SIZE;
            need_respond = TRUE;
            break;

        case AMTRL3_MGR_IPCCMD_UPDATETUNNELTTL:
            amtrl3_mgr_msg_p->type.result_bool = AMTRL3_MGR_TunnelUpdateTtl(
                                &(amtrl3_mgr_msg_p->data.tunnel_update_ttl.host_entry),
                                  amtrl3_mgr_msg_p->data.tunnel_update_ttl.ttl);
            ipcmsg_p->msg_size = AMTRL3_MGR_MSGBUF_TYPE_SIZE;
            need_respond = TRUE;
            break;
#endif /*SYS_CPNT_IP_TUNNEL*/

#if (SYS_CPNT_VXLAN == TRUE)
        case AMTRL3_MGR_IPCCMD_ADD_VXLAN_TUNNEL:
            amtrl3_mgr_msg_p->type.result_bool = AMTRL3_MGR_AddVxlanTunnel(
                                  amtrl3_mgr_msg_p->data.vxlan_tunnel.fib_id,
                                  &amtrl3_mgr_msg_p->data.vxlan_tunnel.tunnel);
            ipcmsg_p->msg_size = AMTRL3_MGR_MSGBUF_TYPE_SIZE;
            need_respond = TRUE;
            break;
        case AMTRL3_MGR_IPCCMD_DEL_VXLAN_TUNNEL:
            amtrl3_mgr_msg_p->type.result_bool = AMTRL3_MGR_DeleteVxlanTunnel(
                                  amtrl3_mgr_msg_p->data.vxlan_tunnel.fib_id,
                                  &amtrl3_mgr_msg_p->data.vxlan_tunnel.tunnel);
            ipcmsg_p->msg_size = AMTRL3_MGR_MSGBUF_TYPE_SIZE;
            need_respond = TRUE;
            break;
        case AMTRL3_MGR_IPCCMD_ADD_VXLAN_TUNNEL_NEXTHOP:
            amtrl3_mgr_msg_p->type.result_bool = AMTRL3_MGR_AddVxlanTunnelNexthop(
                                  amtrl3_mgr_msg_p->data.vxlan_tunnel_nexthop.fib_id,
                                  &amtrl3_mgr_msg_p->data.vxlan_tunnel_nexthop.nexthop);
            ipcmsg_p->msg_size = AMTRL3_MGR_MSGBUF_TYPE_SIZE;
            need_respond = TRUE;
            break;
        case AMTRL3_MGR_IPCCMD_DEL_VXLAN_TUNNEL_NEXTHOP:
            amtrl3_mgr_msg_p->type.result_bool = AMTRL3_MGR_DeleteVxlanTunnelNexthop(
                                  amtrl3_mgr_msg_p->data.vxlan_tunnel_nexthop.fib_id,
                                  &amtrl3_mgr_msg_p->data.vxlan_tunnel_nexthop.nexthop);
            ipcmsg_p->msg_size = AMTRL3_MGR_MSGBUF_TYPE_SIZE;
            need_respond = TRUE;
            break;
#endif
        default:
            SYSFUN_Debug_Printf("%s(): Invalid cmd(%d).\n", __FUNCTION__, (int)(ipcmsg_p->cmd));
            amtrl3_mgr_msg_p->type.result_ui32 = AMTRL3_TYPE_FAIL;
            ipcmsg_p->msg_size = AMTRL3_MGR_MSGBUF_TYPE_SIZE;
    }

    return need_respond;

}

/* for backdoor debug only */
void AMTRL3_MGR_SetEgressObjectFlag(void)
{
    if(CHECK_FLAG(amtrl3_chip_capability_flags, AMTRL3_CHIP_SUPPORT_EGRESS_OBJECT))
        UNSET_FLAG(amtrl3_chip_capability_flags, AMTRL3_CHIP_SUPPORT_EGRESS_OBJECT);
    else
        SET_FLAG(amtrl3_chip_capability_flags, AMTRL3_CHIP_SUPPORT_EGRESS_OBJECT);

    BACKDOOR_MGR_Printf("\n Current chip use egress object flag is: %s",
                          CHECK_FLAG(amtrl3_chip_capability_flags, AMTRL3_CHIP_SUPPORT_EGRESS_OBJECT) ? "ON" : "OFF");
}

void AMTRL3_MGR_SetECMPTableSameEgressFlag(void)
{
    if(CHECK_FLAG(amtrl3_chip_capability_flags, AMTRL3_CHIP_SUPPORT_SAME_HW_INFO_FOR_ECMP_PATH))
        UNSET_FLAG(amtrl3_chip_capability_flags, AMTRL3_CHIP_SUPPORT_SAME_HW_INFO_FOR_ECMP_PATH);
    else
        SET_FLAG(amtrl3_chip_capability_flags, AMTRL3_CHIP_SUPPORT_SAME_HW_INFO_FOR_ECMP_PATH);

    BACKDOOR_MGR_Printf("\n Current chip use same egress for ECMP table flag is: %s",
                          CHECK_FLAG(amtrl3_chip_capability_flags, AMTRL3_CHIP_SUPPORT_SAME_HW_INFO_FOR_ECMP_PATH) ? "ON" : "OFF");
}

void AMTRL3_MGR_SetChipFullFlag(void)
{
    if(chip_full_flag)
        chip_full_flag = FALSE;
    else
        chip_full_flag = TRUE;

    BACKDOOR_MGR_Printf("\n Current chip full flag is: %s", chip_full_flag == TRUE ? "ON" : "OFF");
}

/* for backdoor debug only */
void AMTRL3_MGR_SetDefaultRouteTrapToCpuFlag(void)
{
    if(CHECK_FLAG(amtrl3_chip_capability_flags, AMTRL3_CHIP_USE_DEFAULT_ROUTE_TRAP_TO_CPU))
        UNSET_FLAG(amtrl3_chip_capability_flags, AMTRL3_CHIP_USE_DEFAULT_ROUTE_TRAP_TO_CPU);
    else
        SET_FLAG(amtrl3_chip_capability_flags, AMTRL3_CHIP_USE_DEFAULT_ROUTE_TRAP_TO_CPU);

    BACKDOOR_MGR_Printf("\n Current use default route trap to cpu flag is: %s",
                          CHECK_FLAG(amtrl3_chip_capability_flags, AMTRL3_CHIP_USE_DEFAULT_ROUTE_TRAP_TO_CPU) ? "ON" : "OFF");
}

void AMTRL3_MGR_SetLoaclHostTrapMyIpPktFlag(void)
{
    if(CHECK_FLAG(amtrl3_chip_capability_flags, AMTRL3_CHIP_USE_LOCAL_HOST_TO_TRAP_MY_IP_PKTS))
        UNSET_FLAG(amtrl3_chip_capability_flags, AMTRL3_CHIP_USE_LOCAL_HOST_TO_TRAP_MY_IP_PKTS);
    else
        SET_FLAG(amtrl3_chip_capability_flags, AMTRL3_CHIP_USE_LOCAL_HOST_TO_TRAP_MY_IP_PKTS);

    BACKDOOR_MGR_Printf("\n Current use local host to trap my ip pkt flag is: %s",
                          CHECK_FLAG(amtrl3_chip_capability_flags, AMTRL3_CHIP_USE_LOCAL_HOST_TO_TRAP_MY_IP_PKTS) ? "ON" : "OFF");
}

#if (SYS_CPNT_IP_TUNNEL == TRUE)

/* no longer use */
#if 0
/*
input:    tidIfindex : tunnel ifindex
            tunnel_type: tunnel type, ex AMTRL3_TYPE_INETTOPHYSICALEXTTYPE_TUNNEL_6TO4
            src_addr: tunnel source IP, should be IPv4 form 6->4 tunnel (6to4 and ISATAP)
            dest_addr: tunnel destination, should be IPv4 form 6->4 tunnel (6to4 and ISATAP)
*/
BOOL_T  AMTRL3_MGR_SetDynamicTunnel(UI32_T  tidIfindex, UI32_T  tunnel_type, L_INET_AddrIp_T* src_addr,  L_INET_AddrIp_T * dest_addr)
{
    SWDRVL3_TunnelTerminator_T tln_term;
    NETCFG_TYPE_L3_Interface_T intf;
    VLAN_OM_Dot1qVlanCurrentEntry_T  vlan_info;
    UI32_T result;
    tln_term.fib_id =SYS_ADPT_DEFAULT_FIB;
    //find tunnel sourc vlan
    memset(&intf,0,sizeof(intf));
    intf.ifindex = tidIfindex;
    if(!NETCFG_TYPE_OK== NETCFG_POM_IP_GetL3Interface(&intf))
    {
        if(amtrl3_mgr_debug_tunnel)
            BACKDOOR_MGR_Printf("Fail to get rif");
        return FALSE;
    }
    memset(&vlan_info,0,sizeof(vlan_info));
    vlan_info.dot1q_vlan_index = intf.u.tunnel_intf.src_vid_ifindex;
    if(!VLAN_POM_GetVlanEntry(&vlan_info))
    {
        if(amtrl3_mgr_debug_tunnel)
            BACKDOOR_MGR_Printf("Warning! Fail to get port bitmap");
        return FALSE;
    }
    memcpy(tln_term.lport, vlan_info.dot1q_vlan_current_egress_ports, sizeof(tln_term.lport));

    switch(tunnel_type)
    {
        case AMTRL3_TYPE_INETTOPHYSICALEXTTYPE_TUNNEL_6TO4:
            tln_term.tunnel_type = SWDRVL3_TUNNELTYPE_6TO4;
            memset(&tln_term.sip,0,sizeof(tln_term.sip));
            tln_term.sip.type =L_INET_ADDR_TYPE_IPV4;//from any V4
            tln_term.dip=*src_addr;//my source rif ipv4
            tln_term.dip.preflen = 32;
            break;
        default:
            break;
    }
    if(amtrl3_mgr_debug_tunnel)
    {
        BACKDOOR_MGR_Printf("Set TUnnel Terminator:");
        BACKDOOR_MGR_Printf("fibid=%d\r\n", tln_term.fib_id);
        BACKDOOR_MGR_Printf("tunnel_type=%d\r\n", tln_term.tunnel_type);
        BACKDOOR_MGR_Printf("sip=%d.%d.%d.%d / %d\r\n", L_INET_EXPAND_IPV4(tln_term.sip.addr), tln_term.sip.preflen);
        BACKDOOR_MGR_Printf("dip=%d.%d.%d.%d / %d\r\n", L_INET_EXPAND_IPV4(tln_term.dip.addr), tln_term.dip.preflen);
        DUMP_MEMORY(tln_term.lport, sizeof(tln_term.lport));
    }
    result = SWDRVL3_AddTunnelTerminator(&tln_term);
    if(result != DEV_SWDRVL3_L3_NO_ERROR && result != DEV_SWDRVL3_L3_EXISTS)
    {
        if(amtrl3_mgr_debug_tunnel)
            BACKDOOR_MGR_Printf("Fai to set driver ! %lX", result);
        return FALSE;
    }
    return  TRUE;
}
#endif

/*--------------------------------------------------------------------------
 * FUNCTION NAME - AMTRL3_MGR_DeleteTunnelHostEntries
 *--------------------------------------------------------------------------
 * PURPOSE  : This function deletes tunnel's host route
 * INPUT    : fibid        -- fib index
 *            tidIfindex   -- tunnle l3 ifindex
 * OUTPUT   : none
 * RETURN   : TRUE/FALSE
 * NOTES    : none
 *--------------------------------------------------------------------------
 */
BOOL_T AMTRL3_MGR_DeleteTunnelHostEntries( UI32_T fibid, UI32_T  tidIfindex)
{
    AMTRL3_OM_HostRouteEntry_T host_route_entry;
    memset(&host_route_entry,0,sizeof(host_route_entry));
    host_route_entry.key_fields.dst_inet_addr.type = AMTRL3_TYPE_FLAGS_IPV6;
    while( AMTRL3_OM_GetNextHostRouteEntry(AMTRL3_TYPE_FLAGS_IPV6,fibid, &host_route_entry))
    {
        if(!IS_TUNNEL_IFINDEX(host_route_entry.key_fields.dst_vid_ifindex))
            continue;
        if(host_route_entry.key_fields.dst_vid_ifindex != tidIfindex)
            continue;
        if(host_route_entry.key_fields.tunnel_entry_type!=AMTRL3_TYPE_INETTOPHYSICALEXTTYPE_TUNNEL_ISATAP &&
            host_route_entry.key_fields.tunnel_entry_type!=AMTRL3_TYPE_INETTOPHYSICALEXTTYPE_TUNNEL_6TO4 &&
            host_route_entry.key_fields.tunnel_entry_type!=AMTRL3_TYPE_INETTOPHYSICALEXTTYPE_TUNNEL_MANUAL )
            continue;
        if (!AMTRL3_MGR_HostRouteEventHandler(AMTRL3_TYPE_FLAGS_IPV6, fibid, HOST_ROUTE_REMOVE_EVENT, &host_route_entry))
        {
            if(amtrl3_mgr_debug_tunnel)
                BACKDOOR_MGR_Printf("Fail to remove %lx:%lx:%lx:%lx", L_INET_EXPAND_IPV6(host_route_entry.key_fields.dst_inet_addr.addr));
        }

    }
    return TRUE;
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - AMTRL3_MGR_DeleteTunnelHostRouteFromChip
 *--------------------------------------------------------------------------
 * PURPOSE  : This function deletes tunnel's host route from chip
 * INPUT    : action_flags -- indicate which action(AMTRL3_TYPE_FLAGS_IPV4/AMTRL3_TYPE_FLAGS_IPV6)
 *            fibid        -- fib index
 *            tidIfindex   -- tunnle l3 ifindex
 * OUTPUT   : none
 * RETURN   : TRUE/FALSE
 * NOTES    : tunnel's host route in chip is used to create l3 interface with one initiator,
              and also create terminator
 *--------------------------------------------------------------------------
 */
static BOOL_T AMTRL3_MGR_DeleteTunnelHostRouteFromChip(UI32_T action_flags, UI32_T fib_id, AMTRL3_OM_HostRouteEntry_T  *host_entry)
{
    /* Local Variable Declaration
     */
    BOOL_T      is_trunk,is_tagged;
    UI32_T      unit, port, trunk_id,vid;
    UI32_T      begin_timetick = 0, end_timetick = 0;
    UI32_T      ret = AMTRL3_MGR_ADD_HOST_ERROR, result = 0;
    UI8_T       ip_str[L_INET_MAX_IPADDR_STR_LEN] = {0};
    UI8_T      mymac[SYS_ADPT_MAC_ADDR_LEN];//VLAN_PMGR_GetVlanMac
    //SWDRVL3_Host_T  swdrvl3_host_entry;
    SWDRVL3_HostTunnel_T swdrvl3_tunnel_entry;

    NETCFG_TYPE_L3_Interface_T intf;
    /* BODY
     */
    if(host_entry == NULL)
        return FALSE;
    if(AMTRL3_MGR_GetFIBByID(fib_id) == NULL)
        return FALSE;

    if (amtrl3_mgr_debug_flag & DEBUG_LEVEL_TRACE_HOST)
    {
        AMTRL3_MGR_Ntoa(&(host_entry->key_fields.dst_inet_addr), ip_str);

        BACKDOOR_MGR_Printf("AMTRL3_MGR_UnsetTunnelHostRouteToChip IP: %s, vlan: %d, port %d, type: %d\n",
                ip_str, (int)host_entry->key_fields.dst_vid_ifindex, (int)host_entry->key_fields.lport, (int)host_entry->entry_type);
        BACKDOOR_MGR_Printf("Dst MAC = %02x %02x %02x %02x %02x %02x\n", host_entry->key_fields.dst_mac[0], host_entry->key_fields.dst_mac[1],
                                                        host_entry->key_fields.dst_mac[2], host_entry->key_fields.dst_mac[3],
                                                        host_entry->key_fields.dst_mac[4], host_entry->key_fields.dst_mac[5]);
    } /* end of if */

    memset(&intf,0,sizeof(intf));
    intf.ifindex = host_entry->key_fields.dst_vid_ifindex;
    if(NETCFG_TYPE_OK != NETCFG_POM_IP_GetL3Interface(&intf))
    {
        if(amtrl3_mgr_debug_tunnel)
            BACKDOOR_MGR_Printf("fail to get tunnel interface %ld", (long)intf.ifindex);
        return FALSE;
    }

    unit = port = trunk_id = vid = 0;
    is_trunk = is_tagged =  FALSE;
    memset(&swdrvl3_tunnel_entry, 0 ,sizeof(swdrvl3_tunnel_entry));


    //may return AMTRL3_MGR_INSUFFICIENT_HOST_INFO??
    //resolve nexthop
    if(amtrl3_mgr_debug_tunnel)
    {
        BACKDOOR_MGR_Printf("host type=%ld, hw=%X", (long)host_entry->entry_type, (UI32_T)host_entry->hw_info);
        BACKDOOR_MGR_Printf("from  is   %d.%d.%d.%d",  L_INET_EXPAND_IPV4(host_entry->key_fields.u.ip_tunnel.src_inet_addr.addr));
        BACKDOOR_MGR_Printf("nexthop isv%ld= %d.%d.%d.%d", (long)host_entry->key_fields.u.ip_tunnel.nexthop_vidifindex, L_INET_EXPAND_IPV4(host_entry->key_fields.u.ip_tunnel.nexthop_inet_addr.addr));
    }
    if(host_entry->key_fields.u.ip_tunnel.nexthop_vidifindex ==0)
    {
        if(amtrl3_mgr_debug_tunnel)
            BACKDOOR_MGR_Printf("delete my IP");
        return AMTRL3_MGR_DeleteMyIpHostRouteFromChip(action_flags, fib_id, host_entry);
    }

    //resolve nexthop
    /*
    {
        AMTRL3_OM_HostRouteEntry_T nexthop_entry;
        memset(&nexthop_entry, 0, sizeof(nexthop_entry));
        nexthop_entry.key_fields.dst_inet_addr = host_entry->key_fields.u.ip_tunnel.nexthop_inet_addr;
        if( AMTRL3_OM_GetHostRouteEntry(AMTRL3_TYPE_FLAGS_IPV4,fib_id, &nexthop_entry))
        {
            NOTEprintf("Bingo. entry do exist");
            memcpy(host_entry->key_fields.dst_mac, nexthop_entry.key_fields.dst_mac, sizeof(UI8_T) * SYS_ADPT_MAC_ADDR_LEN);

        }
        else
        {
            DBGprintf("Try to resolve next hop entry??");
            return AMTRL3_MGR_INSUFFICIENT_HOST_INFO;
        }
    }
    */
    /* For Dynamic host entries
     */
    if (AMTRL3_MGR_DeriveDetailPortInfo (host_entry->key_fields.dst_vid_ifindex,
                                         host_entry->key_fields.lport,
                                         &swdrvl3_tunnel_entry.vid,
                                         &swdrvl3_tunnel_entry.unit,
                                         &swdrvl3_tunnel_entry.port,
                                         &swdrvl3_tunnel_entry.trunk_id,
                                         &is_tagged, &is_trunk) == FALSE)
    {
        /* Cannot find sufficient port information   */
        return FALSE;
    }

    if(CHECK_FLAG(amtrl3_chip_capability_flags, AMTRL3_CHIP_NEED_SRC_MAC_TO_PROGRM_HOST_ROUTE))
        VLAN_PMGR_GetVlanMac(host_entry->key_fields.dst_vid_ifindex, swdrvl3_tunnel_entry.src_mac);

    if (amtrl3_mgr_hostroute_performance_flag)
        begin_timetick = SYSFUN_GetSysTick();


    if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
    {
        swdrvl3_tunnel_entry.flags |= SWDRVL3_FLAG_IPV4;
        memcpy(&(swdrvl3_tunnel_entry.ip_addr), &(host_entry->key_fields.dst_inet_addr.addr), SYS_TYPE_PACKET_LENGTH_OF_IP_ADDRESS);
    }
    else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
    {
        swdrvl3_tunnel_entry.flags |= SWDRVL3_FLAG_IPV6;
        memcpy(&(swdrvl3_tunnel_entry.ip_addr), &(host_entry->key_fields.dst_inet_addr.addr), SYS_TYPE_PACKET_LENGTH_OF_IP6_ADDRESS);
    }
    if(is_tagged)
        swdrvl3_tunnel_entry.flags |= SWDRVL3_FLAG_TAGGED_EGRESS_VLAN;
    if(is_trunk)
        swdrvl3_tunnel_entry.flags |= SWDRVL3_FLAG_TRUNK_EGRESS_PORT;

    swdrvl3_tunnel_entry.fib_id = fib_id;

    swdrvl3_tunnel_entry.hw_info = host_entry->hw_info;
    memcpy(swdrvl3_tunnel_entry.mac, host_entry->key_fields.dst_mac, sizeof(UI8_T) * SYS_ADPT_MAC_ADDR_LEN);

    //config initiator and terminator
    swdrvl3_tunnel_entry.tnl_init.l3_intf_id =host_entry->hw_tunnel_index;
    VLAN_PMGR_GetVlanMac(host_entry->key_fields.u.ip_tunnel.nexthop_vidifindex ,mymac);
    memcpy(swdrvl3_tunnel_entry.src_mac , mymac ,SYS_ADPT_MAC_ADDR_LEN);
    VLAN_OM_ConvertFromIfindex(host_entry->key_fields.u.ip_tunnel.nexthop_vidifindex, &swdrvl3_tunnel_entry.vid);
    switch(host_entry->key_fields.tunnel_entry_type)
    {
        case AMTRL3_TYPE_INETTOPHYSICALEXTTYPE_TUNNEL_6TO4:
            swdrvl3_tunnel_entry.tnl_init.tunnel_type= SWDRVL3_TUNNELTYPE_6TO4;
            swdrvl3_tunnel_entry.tnl_term.tunnel_type= SWDRVL3_TUNNELTYPE_6TO4;
            swdrvl3_tunnel_entry.tnl_init.sip  = host_entry->key_fields.u.ip_tunnel.src_inet_addr;
            swdrvl3_tunnel_entry.tnl_init.dip = host_entry->key_fields.u.ip_tunnel.dest_inet_addr;
            swdrvl3_tunnel_entry.tnl_term.sip = host_entry->key_fields.u.ip_tunnel.dest_inet_addr;
            swdrvl3_tunnel_entry.tnl_term.sip.preflen = 0;
            swdrvl3_tunnel_entry.tnl_term.dip = host_entry->key_fields.u.ip_tunnel.src_inet_addr ;//my IP
            swdrvl3_tunnel_entry.tnl_term.dip.preflen = 32;
            break;
        case AMTRL3_TYPE_INETTOPHYSICALEXTTYPE_TUNNEL_ISATAP:
            swdrvl3_tunnel_entry.tnl_init.tunnel_type= SWDRVL3_TUNNELTYPE_ISATAP;
            IP_LIB_GetTunnelAddress(&host_entry->key_fields.u.ip_tunnel.src_inet_addr, &swdrvl3_tunnel_entry.tnl_init.sip );
            IP_LIB_GetTunnelAddress(&host_entry->key_fields.dst_inet_addr, &swdrvl3_tunnel_entry.tnl_init.dip);
            break;
        case AMTRL3_TYPE_INETTOPHYSICALEXTTYPE_TUNNEL_MANUAL:
            swdrvl3_tunnel_entry.tnl_init.tunnel_type= SWDRVL3_TUNNELTYPE_MANUAL;
            swdrvl3_tunnel_entry.tnl_term.tunnel_type= SWDRVL3_TUNNELTYPE_MANUAL;
            swdrvl3_tunnel_entry.tnl_init.sip = host_entry->key_fields.u.ip_tunnel.src_inet_addr;
            swdrvl3_tunnel_entry.tnl_init.dip = host_entry->key_fields.u.ip_tunnel.dest_inet_addr;
            swdrvl3_tunnel_entry.tnl_term.sip = host_entry->key_fields.u.ip_tunnel.dest_inet_addr;
            swdrvl3_tunnel_entry.tnl_term.sip.preflen = 32;
            swdrvl3_tunnel_entry.tnl_term.dip = host_entry->key_fields.u.ip_tunnel.src_inet_addr ;
            swdrvl3_tunnel_entry.tnl_term.dip.preflen = 32;
            break;
         default:
            if(amtrl3_mgr_debug_tunnel)
                BACKDOOR_MGR_Printf("error?");
            return FALSE;

    }

    memcpy(swdrvl3_tunnel_entry.tnl_init.src_mac , mymac ,SYS_ADPT_MAC_ADDR_LEN);
    memcpy(swdrvl3_tunnel_entry.tnl_init.nexthop_mac, host_entry->key_fields.dst_mac ,SYS_ADPT_MAC_ADDR_LEN);
    VLAN_OM_ConvertFromIfindex(host_entry->key_fields.u.ip_tunnel.src_vidifindex, &swdrvl3_tunnel_entry.tnl_init.vid);
    swdrvl3_tunnel_entry.tnl_init.ttl = intf.u.tunnel_intf.ttl;
    {//get port list
        VLAN_OM_Dot1qVlanCurrentEntry_T dot1qvlan;
        memset(&dot1qvlan,0,sizeof(dot1qvlan));
        dot1qvlan.dot1q_vlan_index = host_entry->key_fields.u.ip_tunnel.nexthop_vidifindex;
        if(!VLAN_POM_GetVlanEntry(&dot1qvlan))
        {
            if(amtrl3_mgr_debug_tunnel)
                BACKDOOR_MGR_Printf("Warning! Fail to get port bitmap");
        }
        memcpy(swdrvl3_tunnel_entry.tnl_term.lport, dot1qvlan.dot1q_vlan_static_egress_ports,SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);
    }

    if(amtrl3_mgr_debug_tunnel)
    {
        BACKDOOR_MGR_Printf("To driver:\r\n");
        BACKDOOR_MGR_Printf("flag=%lx\r\n", (unsigned long)swdrvl3_tunnel_entry.flags);
        BACKDOOR_MGR_Printf("fib=%lu\r\n", (unsigned long)swdrvl3_tunnel_entry.fib_id);
        BACKDOOR_MGR_Printf("ip=%lx.%lx.%lx.%lx\r\n",L_INET_EXPAND_IPV6(&swdrvl3_tunnel_entry.ip_addr));
        BACKDOOR_MGR_Printf("MAC=%x-%x-%x-%x-%x-%x\r\n",L_INET_EXPAND_MAC(swdrvl3_tunnel_entry.mac));
        BACKDOOR_MGR_Printf("SMAC=%x-%x-%x-%x-%x-%x\r\n",L_INET_EXPAND_MAC(swdrvl3_tunnel_entry.src_mac));
        BACKDOOR_MGR_Printf("vid=%lu\r\n", (unsigned long)swdrvl3_tunnel_entry.vid);
        BACKDOOR_MGR_Printf("unit=%lu\r\n", (unsigned long)swdrvl3_tunnel_entry.unit);
        BACKDOOR_MGR_Printf("port=%lu\r\n", (unsigned long)swdrvl3_tunnel_entry.port);
        BACKDOOR_MGR_Printf("hwinfo=%lX\r\n",((unsigned long))swdrvl3_tunnel_entry.hw_info);
        BACKDOOR_MGR_Printf("init\r\n");
        BACKDOOR_MGR_Printf("tnl_l3_intfid=%lu\r\n", (unsigned long)swdrvl3_tunnel_entry.tnl_init.l3_intf_id);
        BACKDOOR_MGR_Printf("tnl_SMAC=%lx-%lx-%lx-%lx-%lx-%lx\r\n",L_INET_EXPAND_MAC(swdrvl3_tunnel_entry.tnl_init.src_mac));
        BACKDOOR_MGR_Printf("tnl_vid=%lu\r\n", (unsigned long)swdrvl3_tunnel_entry.tnl_init.vid);
        BACKDOOR_MGR_Printf("tnl_tunnel_type=%u\r\n",swdrvl3_tunnel_entry.tnl_init.tunnel_type);
        BACKDOOR_MGR_Printf("tnl_sip =%lx.%lx.%lx.%lx\r\n",L_INET_EXPAND_IPV6(swdrvl3_tunnel_entry.tnl_init.sip.addr));
        BACKDOOR_MGR_Printf("tnl_dip =%lx.%lx.%lx.%lx\r\n",L_INET_EXPAND_IPV6(swdrvl3_tunnel_entry.tnl_init.dip.addr));
        BACKDOOR_MGR_Printf("tnl_ttl=%d\r\n",swdrvl3_tunnel_entry.tnl_init.ttl);
        BACKDOOR_MGR_Printf("tnl_nextMAC=%lx-%lx-%lx-%lx-%lx-%lx\r\n",L_INET_EXPAND_MAC(swdrvl3_tunnel_entry.tnl_init.nexthop_mac));
        BACKDOOR_MGR_Printf("terminator\r\n");
        BACKDOOR_MGR_Printf("tnl_fib_id=%d\r\n",swdrvl3_tunnel_entry.tnl_term.fib_id);
        BACKDOOR_MGR_Printf("tnl_tunnel_type=%u\r\n",swdrvl3_tunnel_entry.tnl_term.tunnel_type);
        BACKDOOR_MGR_Printf("tnl_sip=%lx:%lx:%lx:%lx-%d\r\n",L_INET_EXPAND_IPV6(swdrvl3_tunnel_entry.tnl_term.sip.addr), swdrvl3_tunnel_entry.tnl_term.sip.preflen);
        BACKDOOR_MGR_Printf("tnl_dip=%lx:%lx:%lx:%lx-%d\r\n",L_INET_EXPAND_IPV6(swdrvl3_tunnel_entry.tnl_term.dip.addr),swdrvl3_tunnel_entry.tnl_term.dip.preflen);
        DUMP_MEMORY(swdrvl3_tunnel_entry.tnl_term.lport, sizeof(swdrvl3_tunnel_entry.tnl_term.lport));
    }
    result = SWDRVL3_DeleteInetHostTunnelRoute(&swdrvl3_tunnel_entry);
    if(amtrl3_mgr_debug_tunnel)
    {
        BACKDOOR_MGR_Printf("output1 is now: %ld", (long)swdrvl3_tunnel_entry.tnl_init.l3_intf_id);
        BACKDOOR_MGR_Printf("output2 is now: %lX", (unsigned long)swdrvl3_tunnel_entry.hw_info);
    }

    if (  result != SWDRVL3_L3_NO_ERROR)
    {
        if(amtrl3_mgr_debug_tunnel)
            BACKDOOR_MGR_Printf("To driver Fail :, result %X", result);
        if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
            AMTRL3_OM_IncreaseDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV4_DEL_HOST_ROUTE_FAIL, 1);
        else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
            AMTRL3_OM_IncreaseDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV6_DEL_HOST_ROUTE_FAIL, 1);

        if (amtrl3_mgr_debug_flag & DEBUG_LEVEL_ERROR)
        {
            VLAN_OM_ConvertFromIfindex(host_entry->key_fields.dst_vid_ifindex, &vid);
            AMTRL3_MGR_Ntoa(&(host_entry->key_fields.dst_inet_addr), ip_str);
            BACKDOOR_MGR_Printf("SWDRVL3_DeleteHostRoute Fails: IP: %s, vid %d\n",
                   ip_str, (int)vid);
        }
        return FALSE;
    }
    else
    {
        host_entry->in_chip_status = FALSE;
        host_entry->hw_info = swdrvl3_tunnel_entry.hw_info;
        host_entry->hw_tunnel_index = swdrvl3_tunnel_entry.tnl_init.l3_intf_id;
        if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
            AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV4_HOST_ROUTE_IN_CHIP, 1);
        else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
            AMTRL3_OM_DecreaseDatabaseValue(fib_id, AMTRL3_OM_DEBUG_COUNTER_IPV6_HOST_ROUTE_IN_CHIP, 1);
        return TRUE;
    } /* end of else */

    return TRUE;
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - AMTRL3_MGR_UpdateTunnelHostRouteToChip
 *--------------------------------------------------------------------------
 * PURPOSE  : This function updates tunnel's host route to chip
 * INPUT    : action_flags -- indicate which action(AMTRL3_TYPE_FLAGS_IPV4/AMTRL3_TYPE_FLAGS_IPV6)
 *            fibid        -- fib index
 *            host_entry   -- amtrl3 om's host entry
 * OUTPUT   : none
 * RETURN   : AMTRL3_MGR_ADD_HOST_OK
 * NOTES    :
 *--------------------------------------------------------------------------
 */
static UI32_T AMTRL3_MGR_UpdateTunnelHostRouteToChip(UI32_T action_flags, UI32_T fib_id, AMTRL3_OM_HostRouteEntry_T  *host_entry)
{
    //currently we do not have update API, just delete+add
    if(amtrl3_mgr_debug_tunnel)
        BACKDOOR_MGR_Printf("Tunnel to delete: %lx:%lx:%lx:%lx, vlan=%ld, Tunnel, HW=%lx, HWTunnel=%ld",
                        L_INET_EXPAND_IPV6(host_entry->key_fields.dst_inet_addr.addr),
                        (long)host_entry->key_fields.dst_vid_ifindex,
                        (unsigned long)host_entry->hw_info,
                        (long)host_entry->hw_tunnel_index
                        );
    AMTRL3_MGR_RemoveTunnelNetRouteFromChipByGateway(action_flags, fib_id, host_entry);
    if(TRUE!= AMTRL3_MGR_DeleteTunnelHostRouteFromChip(  action_flags,   fib_id,  host_entry))
    {
        if(amtrl3_mgr_debug_tunnel)
            BACKDOOR_MGR_Printf("Fail to delete from chip=>skip error!");
        //return AMTRL3_MGR_ADD_HOST_ERROR;
    }
    AMTRL3_MGR_AddTunnelHostRouteToChip(  action_flags,   fib_id,  host_entry);
    AMTRL3_MGR_AddTunnelNetRouteToChipByGateway(action_flags, fib_id, host_entry);
    return AMTRL3_MGR_ADD_HOST_OK;
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - AMTRL3_MGR_AddTunnelNetRouteToChipByGateway
 *--------------------------------------------------------------------------
 * PURPOSE  : This function adds tunnel's net route to chip by gateway host route
 * INPUT    : action_flags -- indicate which action(AMTRL3_TYPE_FLAGS_IPV4/AMTRL3_TYPE_FLAGS_IPV6)
 *            fibid        -- fib index
 *            gateway      -- gateway host route
 * OUTPUT   : none
 * RETURN   : TRUE/FALSE
 * NOTES    : for dynamic tunnel(6to4,ISATAP),if there's one dynamic tunnel host route was created,
              we should create a identical net route with the same nexthop.
              This is because we need a dynamic net route to resolved ipv4 nexthop, or the 6to4 packet
              will not know how to forward packet.
 *--------------------------------------------------------------------------
 */
static BOOL_T AMTRL3_MGR_AddTunnelNetRouteToChipByGateway(UI32_T action_flags, UI32_T fib_id, AMTRL3_OM_HostRouteEntry_T *gateway)
{
    AMTRL3_OM_NetRouteEntry_T net_route_entry;
    AMTRL3_OM_HostRouteEntry_T nexthop_host_entry;

    if(!IS_TUNNEL_IFINDEX(gateway->key_fields.dst_vid_ifindex))
        return FALSE;
    if(gateway->key_fields.dst_inet_addr.type!=L_INET_ADDR_TYPE_IPV6
            &&gateway->key_fields.dst_inet_addr.type!=L_INET_ADDR_TYPE_IPV6Z )
    {
        return FALSE;
    }
    if(!gateway->in_chip_status)
        return FALSE;

    if(amtrl3_mgr_debug_tunnel)
    {
        BACKDOOR_MGR_Printf("%s[%d]\r\n",__FUNCTION__,__LINE__);
        BACKDOOR_MGR_Printf("Gateway Information:\r\n");
        BACKDOOR_MGR_Printf("action flags:%lu\r\n", (unsigned long)action_flags);
        BACKDOOR_MGR_Printf("destination vid ifindex:%lu\r\n", (unsigned long)gateway->key_fields.dst_vid_ifindex);
        BACKDOOR_MGR_Printf("type:%u\r\n",gateway->key_fields.dst_inet_addr.type);
        BACKDOOR_MGR_Printf("address:%lx:%lx:%lx:%lx/%u\r\n",
            L_INET_EXPAND_IPV6(gateway->key_fields.dst_inet_addr.addr),
            gateway->key_fields.dst_inet_addr.preflen);
        BACKDOOR_MGR_Printf("tunnel next hop:%lx:%lx:%lx:%lx\r\n",
            L_INET_EXPAND_IPV6(gateway->key_fields.u.ip_tunnel.nexthop_inet_addr.addr));
        BACKDOOR_MGR_Printf("in chip status:%s\r\n",gateway->in_chip_status?"TRUE":"FALSE");
    }


    /* if static tunnel add host route, we should check if any net route to tunnel interface,
     * this net route's nexthop hwinfo should be the same as this host route
     */
    if(gateway->key_fields.tunnel_entry_type == AMTRL3_TYPE_INETTOPHYSICALEXTTYPE_TUNNEL_MANUAL)
    {
        memset(&net_route_entry,0,sizeof(net_route_entry));
        net_route_entry.inet_cidr_route_entry.inet_cidr_route_if_index  = gateway->key_fields.dst_vid_ifindex;
        net_route_entry.inet_cidr_route_entry.inet_cidr_route_dest.type= L_INET_ADDR_TYPE_IPV6;

        /* check this while loop, it should filter some net route */
        while(AMTRL3_MGR_GetNextNetRouteByTunnelID(  action_flags,   fib_id, &net_route_entry))
        {
            if(amtrl3_mgr_debug_tunnel)
            {
                BACKDOOR_MGR_Printf("%s[%d]:Tunnel net route\r\n",__FUNCTION__,__LINE__);
                BACKDOOR_MGR_Printf("net route status=%ld, HW=%lx, tunnel entry type=%u\r\n",
                     (long)net_route_entry.net_route_status, (unsigned long)net_route_entry.hw_info,net_route_entry.tunnel_entry_type);
                BACKDOOR_MGR_Printf("destination=%lx:%lx:%lx:%lx/%lu\r\n",
                    L_INET_EXPAND_IPV6(net_route_entry.inet_cidr_route_entry.inet_cidr_route_dest.addr),
                    (unsigned long)net_route_entry.inet_cidr_route_entry.inet_cidr_route_dest.preflen);
                BACKDOOR_MGR_Printf("nexthop:%lx:%lx:%lx:%lx(%lu)\r\n",
                    L_INET_EXPAND_IPV6(net_route_entry.tunnel_nexthop_inet_addr.addr),
                    (unsigned long)net_route_entry.tunnel_nexthop_inet_addr.addrlen);
            }

#if 0
            /* we should check if this net route's and gateway's tunnel nexthop is the same */
            if(0 != memcmp(gateway->key_fields.u.ip_tunnel.nexthop_inet_addr.addr,net_route_entry.tunnel_tunnel.nexthop_inet_addr.addr, gateway->key_fields.u.ip_tunnel.nexthop_inet_addr.addrlen))
            {
                /*printf("get next tunnel net route\r\n");*/
                continue;
            }
#endif
            /* If route to tunnel interface trap to cpu, we should change its hwinfo to gateway */
            if(net_route_entry.hw_info==AMTRL3_OM_HW_INFO_INVALID)
            {
                if(AMTRL3_MGR_AddNetRouteToChip( action_flags,   fib_id, &net_route_entry, gateway))
                {
                    if(amtrl3_mgr_debug_tunnel)
                        BACKDOOR_MGR_Printf("%s[%d]:%lx:%lx:%lx:%lx/%d status is now %ld\r\n",__FUNCTION__,__LINE__
                                ,L_INET_EXPAND_IPV6(net_route_entry.inet_cidr_route_entry.inet_cidr_route_dest.addr)
                                ,net_route_entry.inet_cidr_route_entry.inet_cidr_route_dest.preflen
                                ,(long)net_route_entry.net_route_status);
                    /* initial tunnle net route hit time stamp */
                    net_route_entry.tunnel_hit_timestamp = (SYSFUN_GetSysTick() / SYS_BLD_TICKS_PER_SECOND);
                    net_route_entry.tunnel_entry_type = AMTRL3_TYPE_INETTOPHYSICALEXTTYPE_TUNNEL_MANUAL;
                    memcpy(&(net_route_entry.tunnel_nexthop_inet_addr), &(gateway->key_fields.u.ip_tunnel.nexthop_inet_addr), sizeof(net_route_entry.tunnel_nexthop_inet_addr));

                    if(!AMTRL3_OM_SetNetRouteEntry(action_flags, fib_id, &net_route_entry))
                    {
                        if(amtrl3_mgr_debug_tunnel)
                            BACKDOOR_MGR_Printf("%s[%d]:Fail to set to OM\r\n",__FUNCTION__,__LINE__);
                        return FALSE;
                    }
                    /*return TRUE;*/
                }
                else
                {
                    if(amtrl3_mgr_debug_tunnel)
                        BACKDOOR_MGR_Printf("%s[%d]:Fail to set net route to chip\r\n",__FUNCTION__,__LINE__);
                    return FALSE;
                }
            }
        }
    }

    /* If this is dynamic tunnel , we automatic add one net route to the same nexthop hwinfo as gateway */
    if(gateway->key_fields.tunnel_entry_type == AMTRL3_TYPE_INETTOPHYSICALEXTTYPE_TUNNEL_6TO4)  /* ISATAP not implemented */
    {
        memset(&net_route_entry, 0, sizeof(AMTRL3_OM_NetRouteEntry_T));
        net_route_entry.inet_cidr_route_entry.inet_cidr_route_if_index  = gateway->key_fields.dst_vid_ifindex;
        net_route_entry.inet_cidr_route_entry.inet_cidr_route_dest = gateway->key_fields.dst_inet_addr;
        net_route_entry.tunnel_entry_type = gateway->key_fields.tunnel_entry_type;
        if(FALSE == AMTRL3_OM_GetNetRouteEntryByTunnelPrefix(action_flags,fib_id,&net_route_entry))
        {
            memset(&net_route_entry, 0, sizeof(AMTRL3_OM_NetRouteEntry_T));
            net_route_entry.inet_cidr_route_entry.inet_cidr_route_if_index  = gateway->key_fields.dst_vid_ifindex;
            net_route_entry.inet_cidr_route_entry.inet_cidr_route_proto = VAL_ipCidrRouteProto_netmgmt;  /* temp */
            net_route_entry.inet_cidr_route_entry.inet_cidr_route_type = VAL_ipCidrRouteType_other;      /* temp */
            net_route_entry.tunnel_nexthop_inet_addr = gateway->key_fields.u.ip_tunnel.nexthop_inet_addr; /* destination is the same as the tunnel host entry */
            net_route_entry.inet_cidr_route_entry.inet_cidr_route_dest = gateway->key_fields.dst_inet_addr;
            net_route_entry.inet_cidr_route_entry.inet_cidr_route_pfxlen = SYS_ADPT_TUNNEL_6to4_PREFIX_LEN;
            net_route_entry.hw_info = gateway->hw_info;
            net_route_entry.tunnel_entry_type = gateway->key_fields.tunnel_entry_type;

            if(amtrl3_mgr_debug_tunnel)
            {
                BACKDOOR_MGR_Printf("%s[%d]:set tunnel net route to tunnel interface\r\n",__FUNCTION__,__LINE__);
                BACKDOOR_MGR_Printf("tunnel entry type:%u, hw_info:%lx\r\n", (unsigned long)net_route_entry.tunnel_entry_type,net_route_entry.hw_info);
                BACKDOOR_MGR_Printf("destination ifindex:%lu\r\n", (unsigned long)net_route_entry.inet_cidr_route_entry.inet_cidr_route_if_index);
                BACKDOOR_MGR_Printf("destination:%lx:%lx:%lx:%lx/%lu\r\n",
                    L_INET_EXPAND_IPV6(net_route_entry.inet_cidr_route_entry.inet_cidr_route_dest.addr),
                    (unsigned long)net_route_entry.inet_cidr_route_entry.inet_cidr_route_dest.preflen);
                BACKDOOR_MGR_Printf("tunnel nexthop:%lx:%lx:%lx:%lx(%lu)\r\n",
                    L_INET_EXPAND_IPV6(net_route_entry.tunnel_nexthop_inet_addr.addr),
                    (unsigned long)net_route_entry.tunnel_nexthop_inet_addr.addrlen);
            }
            if(AMTRL3_MGR_AddNetRouteToChip( action_flags,   fib_id, &net_route_entry, gateway))
            {
                if(amtrl3_mgr_debug_tunnel)
                    BACKDOOR_MGR_Printf("%s[%d]:%lx:%lx:%lx:%lx/%d status is now %ld\r\n",__FUNCTION__,__LINE__
                                ,L_INET_EXPAND_IPV6(net_route_entry.inet_cidr_route_entry.inet_cidr_route_dest.addr)
                                ,net_route_entry.inet_cidr_route_entry.inet_cidr_route_dest.preflen
                                ,(long)net_route_entry.net_route_status);
                net_route_entry.tunnel_hit_timestamp = (SYSFUN_GetSysTick() / SYS_BLD_TICKS_PER_SECOND);

                if(!AMTRL3_OM_SetNetRouteEntry(action_flags, fib_id, &net_route_entry))
                {
                    if(amtrl3_mgr_debug_tunnel)
                        BACKDOOR_MGR_Printf("%s[%d]:Fail to set to OM",__FUNCTION__,__LINE__);
                }

                AMTRL3_OM_IncreaseDatabaseValue(fib_id, AMTRL3_OM_IPV6_6to4_TUNNEL_NET_ROUTE_COUNT, 1);


                /* find out nexthop host entry */
                memset(&nexthop_host_entry, 0, sizeof(AMTRL3_OM_HostRouteEntry_T));
                nexthop_host_entry.key_fields.dst_vid_ifindex = gateway->key_fields.u.ip_tunnel.nexthop_vidifindex;
                memcpy(&nexthop_host_entry.key_fields.dst_inet_addr,&(gateway->key_fields.u.ip_tunnel.nexthop_inet_addr),sizeof(L_INET_AddrIp_T));
                if(AMTRL3_OM_GetHostRouteEntry(AMTRL3_TYPE_FLAGS_IPV4,fib_id,&nexthop_host_entry))
                {
                    /* add nexthop ipv4 host route's reference count by 1 */
                    nexthop_host_entry.ref_count++;
                    /* because tunnel's nexthop is always ipv4, we use AMTRL3_TYPE_FLAGS_IPV4 as action_flags*/
                    if(!AMTRL3_MGR_HostRouteEventHandler(AMTRL3_TYPE_FLAGS_IPV4, fib_id, HOST_ROUTE_PARTIAL_EVENT, &nexthop_host_entry))
                    {
                        if(amtrl3_mgr_debug_tunnel)
                            BACKDOOR_MGR_Printf("%s[%d]:Fail to add getway reference count",__FUNCTION__,__LINE__);
                        return FALSE;
                    }
                }
            }
        }
    }
    return TRUE;

}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - AMTRL3_MGR_RemoveTunnelNetRouteFromChipByGateway
 *--------------------------------------------------------------------------
 * PURPOSE  : This function removes tunnel's net route to chip by gateway host route
 * INPUT    : action_flags -- indicate which action(AMTRL3_TYPE_FLAGS_IPV4/AMTRL3_TYPE_FLAGS_IPV6)
 *            fibid        -- fib index
 *            gateway      -- gateway host route
 * OUTPUT   : none
 * RETURN   : TRUE/FALSE
 * NOTES    : for dynamic tunnel(6to4,ISATAP),if there's one dynamic tunnel host route was deleted,
              we should delete a identical net route with the same nexthop.
              This is because we the dynamic net route and host route were created at the same time,
              we should remove them together.
 *--------------------------------------------------------------------------
 */
static BOOL_T AMTRL3_MGR_RemoveTunnelNetRouteFromChipByGateway(UI32_T action_flags, UI32_T fib_id, AMTRL3_OM_HostRouteEntry_T *gateway)
{
    AMTRL3_OM_NetRouteEntry_T net_route_entry;
    if(!IS_TUNNEL_IFINDEX(gateway->key_fields.dst_vid_ifindex))
        return FALSE;
    if(gateway->key_fields.dst_inet_addr.type!=L_INET_ADDR_TYPE_IPV6
            &&gateway->key_fields.dst_inet_addr.type!=L_INET_ADDR_TYPE_IPV6Z )
    {
        return FALSE;
    }

    if(amtrl3_mgr_debug_tunnel)
        BACKDOOR_MGR_Printf("remove route to tunnel%ld\r\n", (long)gateway->key_fields.dst_vid_ifindex);
    memset(&net_route_entry,0,sizeof(net_route_entry));
    net_route_entry.inet_cidr_route_entry.inet_cidr_route_if_index  = gateway->key_fields.dst_vid_ifindex;
    net_route_entry.inet_cidr_route_entry.inet_cidr_route_dest.type= gateway->key_fields.dst_inet_addr.type;/*L_INET_ADDR_TYPE_IPV6;*/

    while(AMTRL3_MGR_GetNextNetRouteByTunnelID(  action_flags,   fib_id, &net_route_entry))
    {
        if(amtrl3_mgr_debug_tunnel)
        {
            BACKDOOR_MGR_Printf("%s[%d]:Tunnel net route\r\n",__FUNCTION__,__LINE__);
            BACKDOOR_MGR_Printf("net route status=%ld, HW=%lx, tunnel entry type=%u\r\n",
                 (long)net_route_entry.net_route_status, (unsigned long)net_route_entry.hw_info,net_route_entry.tunnel_entry_type);
            BACKDOOR_MGR_Printf("destination=%lx:%lx:%lx:%lx/%lu\r\n",
                L_INET_EXPAND_IPV6(net_route_entry.inet_cidr_route_entry.inet_cidr_route_dest.addr),
                (unsigned long)net_route_entry.inet_cidr_route_entry.inet_cidr_route_dest.preflen);
            BACKDOOR_MGR_Printf("nexthop:%lx:%lx:%lx:%lx(%lu)\r\n",
                L_INET_EXPAND_IPV6(net_route_entry.tunnel_nexthop_inet_addr.addr),
                (unsigned long)net_route_entry.tunnel_nexthop_inet_addr.addrlen);
        }


        /* we should check if this net route's and gateway's tunnel nexthop is the same */
        if(0 != memcmp(gateway->key_fields.u.ip_tunnel.nexthop_inet_addr.addr,net_route_entry.tunnel_nexthop_inet_addr.addr, gateway->key_fields.u.ip_tunnel.nexthop_inet_addr.addrlen))
        {
            /*continue;*/
        }

        if(net_route_entry.hw_info!=AMTRL3_OM_HW_INFO_INVALID)
        {
            if(AMTRL3_MGR_DeleteNetRouteFromChip( action_flags,   fib_id, &net_route_entry, gateway))
            {
                if(gateway->key_fields.tunnel_entry_type == AMTRL3_TYPE_INETTOPHYSICALEXTTYPE_TUNNEL_MANUAL)
                {
                    net_route_entry.net_route_status ==AMTRL3_OM_NET_ROUTE_UNRESOLVED;
                    AMTRL3_OM_SetNetRouteEntry(action_flags, fib_id, &net_route_entry);
                }
                else if(gateway->key_fields.tunnel_entry_type == AMTRL3_TYPE_INETTOPHYSICALEXTTYPE_TUNNEL_6TO4)
                {

                    if(FALSE == AMTRL3_OM_DeleteNetRouteEntry(AMTRL3_TYPE_FLAGS_IPV6, fib_id, &net_route_entry))
                    {
                        return FALSE;
                    }
                    else
                    {
                        return TRUE;
                    }
                }
            }
            else
            {
                if(amtrl3_mgr_debug_tunnel)
                    BACKDOOR_MGR_Printf("Fail to AMTRL3_MGR_DeleteNetRouteFromChip, %lx:%lx:%lx:%lx/%d"
                        ,L_INET_EXPAND_IPV6(net_route_entry.inet_cidr_route_entry.inet_cidr_route_dest.addr)
                        ,net_route_entry.inet_cidr_route_entry.inet_cidr_route_dest.preflen
                    );
                //continue to next tunnel
                return FALSE;
            }
        }
    }
    return TRUE;
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - AMTRL3_MGR_GetNextNetRouteByTunnelID
 *--------------------------------------------------------------------------
 * PURPOSE  : This function get next net route by specified tunnel ifindex
 * INPUT    : action_flags -- indicate which action(AMTRL3_TYPE_FLAGS_IPV4/AMTRL3_TYPE_FLAGS_IPV6)
 *            fibid        -- fib index
 *            net_entry    -- inet_cidr_route_entry
 *                            KEY:  inet_cidr_route_if_index (route interface ifindex)
 * OUTPUT   : none
 * RETURN   : TRUE/FALSE
 * NOTES    : none
 *--------------------------------------------------------------------------
 */
static BOOL_T AMTRL3_MGR_GetNextNetRouteByTunnelID(UI32_T action_flags, UI32_T fib_id, AMTRL3_OM_NetRouteEntry_T  *net_entry)
{
    UI32_T ifindex = net_entry->inet_cidr_route_entry.inet_cidr_route_if_index;
    while(AMTRL3_OM_GetNextNetRouteEntry(action_flags, fib_id, net_entry))
    {
        if(net_entry->inet_cidr_route_entry.inet_cidr_route_if_index!=ifindex)
            continue;
        return TRUE;
    }
    return FALSE;
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - AMTRL3_MGR_HandleTunnelNetRouteToChip
 *--------------------------------------------------------------------------
 * PURPOSE  : This function handles different tunnel net route to chip
 * INPUT    : action_flags -- indicate which action(AMTRL3_TYPE_FLAGS_IPV4/AMTRL3_TYPE_FLAGS_IPV6)
 *            fibid        -- fib index
 *            tunnel_local_net_route_entry
 *            tunnel_net_route_entry
 * OUTPUT   : none
 * RETURN   : TRUE/FALSE
 * NOTES    : If it's dynamic tunnel net route, we will increate the reference count of the next hop,
              this will prevent this gateway ageout.
 *--------------------------------------------------------------------------
 */
static BOOL_T AMTRL3_MGR_HandleTunnelNetRouteToChip(UI32_T action_flags, UI32_T fib_id,
                                                    AMTRL3_OM_NetRouteEntry_T        *tunnel_local_net_route_entry,
                                                    AMTRL3_TYPE_InetCidrRouteEntry_T *tunnel_net_route_entry)
{
    NETCFG_TYPE_L3_Interface_T  tunnel_if;
    AMTRL3_OM_HostRouteEntry_T  local_host_entry, nexthop_host_entry;

    /* we should get tunnel interface to check its tunnel mode first */
    memset(&tunnel_if, 0, sizeof(NETCFG_TYPE_L3_Interface_T));
    tunnel_if.ifindex = tunnel_net_route_entry->partial_entry.inet_cidr_route_if_index;
    if(NETCFG_TYPE_OK != NETCFG_POM_IP_GetL3Interface(&tunnel_if))
    {
        return FALSE;
    }

    memset(&local_host_entry, 0, sizeof(AMTRL3_OM_HostRouteEntry_T));
    switch(tunnel_if.u.tunnel_intf.tunnel_mode)
    {
        case NETCFG_TYPE_TUNNEL_MODE_CONFIGURED:
        {

            local_host_entry.key_fields.dst_vid_ifindex = tunnel_net_route_entry->partial_entry.inet_cidr_route_if_index;
            local_host_entry.key_fields.tunnel_entry_type = AMTRL3_TYPE_INETTOPHYSICALEXTTYPE_TUNNEL_MANUAL;
            if(AMTRL3_OM_GetHostRouteEntryByTunnelIfIndex(AMTRL3_TYPE_FLAGS_IPV6, fib_id,&local_host_entry))
            {
                if(amtrl3_mgr_debug_tunnel)
                {
                    BACKDOOR_MGR_Printf("%s[%d],in chip status: %s, hw info = %02x\r\n",__FUNCTION__,__LINE__,
                           local_host_entry.in_chip_status?"TRUE":"FALSE",
                           (UI8_T *)(local_host_entry.hw_info));
                }
                /* we must check if tunnle l3 interface, initiator and terminator is in chip by this special tunnl host route */
                if(local_host_entry.in_chip_status)
                {


                    if (AMTRL3_MGR_AddNetRouteToChip(action_flags, fib_id, tunnel_local_net_route_entry, &local_host_entry) == TRUE)
                    {

                        /* find out nexthop host entry */
                        memset(&nexthop_host_entry, 0, sizeof(AMTRL3_OM_HostRouteEntry_T));
                        nexthop_host_entry.key_fields.dst_vid_ifindex = local_host_entry.key_fields.u.ip_tunnel.nexthop_vidifindex;
                        memcpy(&nexthop_host_entry.key_fields.dst_inet_addr,&local_host_entry.key_fields.u.ip_tunnel.nexthop_inet_addr,sizeof(L_INET_AddrIp_T));
                        if(AMTRL3_OM_GetHostRouteEntry(AMTRL3_TYPE_FLAGS_IPV4,fib_id,&nexthop_host_entry))
                        {
                            /* add nexthop ipv4 host route's reference count by 1 */
                            nexthop_host_entry.ref_count++;
                            /* because tunnel's nexthop is always ipv4, we use AMTRL3_TYPE_FLAGS_IPV4 as action_flags*/
                            if(!AMTRL3_MGR_HostRouteEventHandler(AMTRL3_TYPE_FLAGS_IPV4, fib_id, HOST_ROUTE_PARTIAL_EVENT, &nexthop_host_entry))
                            {
                                return FALSE;
                            }
                        }
                        return TRUE;
                    }
                    else
                    {
                        if(amtrl3_mgr_debug_tunnel)
                            BACKDOOR_MGR_Printf("Fail to add netroute to Chip!");
                        return FALSE;
                    }
                }
            }
        }
        break;
        case NETCFG_TYPE_TUNNEL_MODE_6TO4:
        {
            /* If we can't find corresponding host route in chip,
             * we should add this net route's next hop ifindex(hw_info) to cpu
             */

            local_host_entry.key_fields.dst_vid_ifindex = tunnel_net_route_entry->partial_entry.inet_cidr_route_if_index;
            local_host_entry.key_fields.tunnel_entry_type = AMTRL3_TYPE_INETTOPHYSICALEXTTYPE_TUNNEL_6TO4;
            local_host_entry.key_fields.dst_inet_addr = tunnel_net_route_entry->partial_entry.inet_cidr_route_dest;

            if(FALSE == AMTRL3_OM_GetHostRouteEntryByTunnelPrefix(AMTRL3_TYPE_FLAGS_IPV6,fib_id,&local_host_entry))
            {
                local_host_entry.hw_info = AMTRL3_OM_HW_INFO_INVALID;
                /* only set tunnel type to host entry, we need to trap this net route to cpu */

            }


            if (AMTRL3_MGR_AddNetRouteToChip(action_flags, fib_id, tunnel_local_net_route_entry, &local_host_entry) == TRUE)
            {

                /* find out nexthop host entry */
                memset(&nexthop_host_entry, 0, sizeof(AMTRL3_OM_HostRouteEntry_T));
                nexthop_host_entry.key_fields.dst_vid_ifindex = local_host_entry.key_fields.u.ip_tunnel.nexthop_vidifindex;
                memcpy(&nexthop_host_entry.key_fields.dst_inet_addr,&local_host_entry.key_fields.u.ip_tunnel.nexthop_inet_addr,sizeof(L_INET_AddrIp_T));
                if(AMTRL3_OM_GetHostRouteEntry(AMTRL3_TYPE_FLAGS_IPV4,fib_id,&nexthop_host_entry))
                {
                    /* add nexthop ipv4 host route's reference count by 1 */
                    nexthop_host_entry.ref_count++;
                    /* because tunnel's nexthop is always ipv4, we use AMTRL3_TYPE_FLAGS_IPV4 as action_flags*/
                    if(!AMTRL3_MGR_HostRouteEventHandler(AMTRL3_TYPE_FLAGS_IPV4, fib_id, HOST_ROUTE_PARTIAL_EVENT, &nexthop_host_entry))
                    {
                        return FALSE;
                    }
                }
                return TRUE;
            }
            else
            {
                if(amtrl3_mgr_debug_tunnel)
                    BACKDOOR_MGR_Printf("Fail to add netroute to Chip!");
                return FALSE;
            }
        }
        break;
        case NETCFG_TYPE_TUNNEL_MODE_ISATAP:
        {

        }
        default:
        break;
    }

    return TRUE;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_CheckAndRemoveExpiredTunnelEntry
 * -------------------------------------------------------------------------
 * PURPOSE:  This function remove net route entry and corresponding tunnel host route entry
 *           from OM and chip if entry already
 *           passes ageout time.
 * INPUT:    fib_id           - FIB id
 *           tunnel_net_entry - given tunnel net route entry
 *           current time     - system current time
 * OUTPUT:   None.
 * RETURN:   None
 * NOTES:    If tunnel's net route is not hit for a period, we'll remove this expired dynamic tunnel net route,
             and also its dynamic host route.
 * -------------------------------------------------------------------------*/
static void AMTRL3_MGR_CheckAndRemoveExpiredTunnelEntry(
                                        UI32_T fib_id,
                                        AMTRL3_OM_NetRouteEntry_T *tunnel_net_entry,
                                        UI32_T current_time)
{
    UI8_T  ip_str[L_INET_MAX_IPADDR_STR_LEN] = {0};
    UI32_T amtrl3_mgr_tunnel_entry_ageout_time = SYS_DFLT_IPV6_TUNNEL_DYNAMIC_ROUTE_AGEOUT_TIME * 60;
    AMTRL3_TYPE_InetCidrRouteEntry_T  local_net_entry;
    AMTRL3_OM_HostRouteEntry_T        local_host_entry;

    /* BODY
     */
    if(tunnel_net_entry == NULL)
        return;

    /* not timeout */
    if(current_time - tunnel_net_entry->tunnel_hit_timestamp < amtrl3_mgr_tunnel_entry_ageout_time)
        return;

    if(amtrl3_mgr_debug_task  & TASK_DEBUG_HITBIT)
    {
        AMTRL3_MGR_Ntoa(&(tunnel_net_entry->inet_cidr_route_entry.inet_cidr_route_dest), ip_str);
        BACKDOOR_MGR_Printf("AMTRL3_MGR_CheckAndRemoveExpiredHostEntry %s\n", ip_str);
    }

    /*  6to4 dynamic tunnel should handle the following things:
     * (1) Remove this tunnel net route
     * (2) Remove corresponding tunnel host route
     * (3) Decrease their nexthop host route reference count by 1
     */
    if((tunnel_net_entry->tunnel_entry_type == AMTRL3_TYPE_INETTOPHYSICALEXTTYPE_TUNNEL_6TO4))
    {
        /* (1) Remove this tunnel net route */
        memset(&local_net_entry, 0, sizeof(AMTRL3_TYPE_InetCidrRouteEntry_T));
        memcpy(&local_net_entry.partial_entry,&(tunnel_net_entry->inet_cidr_route_entry),sizeof(AMTRL3_TYPE_InetCidrRoutePartialEntry_T));

        if(FALSE == AMTRL3_MGR_DeleteInetCidrRouteEntry(AMTRL3_TYPE_FLAGS_IPV6, fib_id, &local_net_entry))
        {
            return;
        }

        /* (2) Remove corresponding tunnel host route */
        memset(&local_host_entry, 0, sizeof(AMTRL3_OM_HostRouteEntry_T));
        memcpy(&(local_host_entry.key_fields.dst_inet_addr), &(tunnel_net_entry->inet_cidr_route_entry.inet_cidr_route_dest),sizeof(L_INET_AddrIp_T));
        local_host_entry.key_fields.dst_vid_ifindex = tunnel_net_entry->inet_cidr_route_entry.inet_cidr_route_if_index;
        local_host_entry.key_fields.tunnel_entry_type = tunnel_net_entry->tunnel_entry_type;
        if(FALSE == AMTRL3_OM_GetHostRouteEntryByTunnelPrefix(AMTRL3_TYPE_FLAGS_IPV6, fib_id, &local_host_entry))
        {
            return;
        }

        if(FALSE == AMTRL3_MGR_HostRouteEventHandler(AMTRL3_TYPE_FLAGS_IPV6, fib_id, HOST_ROUTE_REMOVE_EVENT, &local_host_entry))
        {
            return;
        }

        /* (3) Decrease nexthop host route reference count by 1 will be handle by (1) */
    }

    return;
} /* end of AMTRL3_MGR_CheckAndRemoveExpiredHostEntry() */

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_PrintSwdrvl3TunnelEntryInfo
 * -------------------------------------------------------------------------
 * PURPOSE:  Print out host tunnel entry information
 * INPUT:    entry     -- swdrvl3 host tunnel entry
 * OUTPUT:   None.
 * RETURN:   None
 * NOTES:    For debug use
 * -------------------------------------------------------------------------*/
static void AMTRL3_MGR_PrintSwdrvl3HostTunnelInfo(SWDRVL3_HostTunnel_T *entry)
{
    BACKDOOR_MGR_Printf("===SWDRVL3 Host Tunnel Entry===\r\n");
    BACKDOOR_MGR_Printf("flag      =%ld\r\n",entry->flags);
    BACKDOOR_MGR_Printf("fib       =%ld\r\n",entry->fib_id);
    BACKDOOR_MGR_Printf("ip        =%lx.%lx.%lx.%lx\r\n",L_INET_EXPAND_IPV6(&entry->ip_addr));
    BACKDOOR_MGR_Printf("host MAC  =%02X-%02X-%02X-%02X-%02X-%02X\r\n",L_INET_EXPAND_MAC(entry->mac));
    BACKDOOR_MGR_Printf("Router MAC=%02X-%02X-%02X-%02X-%02X-%02X\r\n",L_INET_EXPAND_MAC(entry->src_mac));
    BACKDOOR_MGR_Printf("vid       =%ld\r\n",entry->vid);
    BACKDOOR_MGR_Printf("unit      =%ld\r\n",entry->unit);
    BACKDOOR_MGR_Printf("port      =%ld\r\n",entry->port);
    BACKDOOR_MGR_Printf("hwinfo    =%lX\r\n",entry->hw_info);

    BACKDOOR_MGR_Printf("***Initiator information***\r\n");
    BACKDOOR_MGR_Printf("l3_intfid  =%ld\r\n",entry->tnl_init.l3_intf_id);
    BACKDOOR_MGR_Printf("SMAC       =%02X-%02X-%02X-%02X-%02X-%02X\r\n",L_INET_EXPAND_MAC(entry->tnl_init.src_mac));
    BACKDOOR_MGR_Printf("vid        =%d\r\n",entry->tnl_init.vid);
    BACKDOOR_MGR_Printf("tunnel_type=%d\r\n",entry->tnl_init.tunnel_type);
    BACKDOOR_MGR_Printf("sip        =%lx.%lx.%lx.%lx\r\n",L_INET_EXPAND_IPV6(entry->tnl_init.sip.addr));
    BACKDOOR_MGR_Printf("dip        =%lx.%lx.%lx.%lx\r\n",L_INET_EXPAND_IPV6(entry->tnl_init.dip.addr));
    BACKDOOR_MGR_Printf("ttl        =%d\r\n",entry->tnl_init.ttl);
    BACKDOOR_MGR_Printf("nexthop MAC=%02X-%02X-%02X-%02X-%02X-%02X\r\n",L_INET_EXPAND_MAC(entry->tnl_init.nexthop_mac));

    BACKDOOR_MGR_Printf("***Terminator information***\r\n");
    BACKDOOR_MGR_Printf("fib_id     =%d\r\n",entry->tnl_term.fib_id);
    BACKDOOR_MGR_Printf("tunnel_type=%d\r\n",entry->tnl_term.tunnel_type);
    BACKDOOR_MGR_Printf("sip        =%lx.%lx.%lx.%lx/%d\r\n",L_INET_EXPAND_IPV6(entry->tnl_term.sip.addr), entry->tnl_term.sip.preflen);
    BACKDOOR_MGR_Printf("dip        =%lx.%lx.%lx.%lx/%d\r\n",L_INET_EXPAND_IPV6(entry->tnl_term.dip.addr),entry->tnl_term.dip.preflen);
    BACKDOOR_MGR_Printf("================================\r\n");
    BACKDOOR_MGR_Printf("***Memory Dump***\r\n");
    DUMP_MEMORY(entry->tnl_term.lport, sizeof(entry->tnl_term.lport));
}

static void AMTRL3_MGR_SetTunnelHostRouteEventByNexthopEntry( UI32_T fib_id, AMTRL3_OM_HostRouteEntry_T* host_route_entry , UI32_T event)
{
    if (VAL_ipNetToPhysicalExtType_dynamic == host_route_entry->entry_type ||VAL_ipNetToPhysicalExtType_static== host_route_entry->entry_type)
    {
        //regular host ready, update tunnel nextop
        AMTRL3_OM_HostRouteEntry_T tunnel_entry;
        memset(&tunnel_entry, 0, sizeof(tunnel_entry));
        tunnel_entry.key_fields.dst_inet_addr.type = L_INET_ADDR_TYPE_IPV6;
        if(amtrl3_mgr_debug_tunnel)
           BACKDOOR_MGR_Printf("%s[%d]:for tunnel with nexthop =%lx:%lx:%lx:%lx, setevent %ld\r\n",
                    __FUNCTION__,__LINE__,
                    L_INET_EXPAND_IPV6(host_route_entry->key_fields.dst_inet_addr.addr),
                    event);
        while(AMTRL3_OM_GetNextHostRouteEntryByTunnelNexthop(AMTRL3_TYPE_FLAGS_IPV6, fib_id, &host_route_entry->key_fields.dst_inet_addr,&tunnel_entry))
        {
            switch(tunnel_entry.key_fields.tunnel_entry_type )
            {
                case AMTRL3_TYPE_INETTOPHYSICALEXTTYPE_TUNNEL_ISATAP:
                case AMTRL3_TYPE_INETTOPHYSICALEXTTYPE_TUNNEL_6TO4:
                case AMTRL3_TYPE_INETTOPHYSICALEXTTYPE_TUNNEL_MANUAL:
                    break;
                default:
                    continue;
            }

            /* update tunnel host route's lport, uport and MAC */
            tunnel_entry.key_fields.lport = host_route_entry->key_fields.lport;
            tunnel_entry.uport = host_route_entry->uport;
            tunnel_entry.key_fields.trunk_id= host_route_entry->key_fields.trunk_id;
            memcpy(tunnel_entry.key_fields.dst_mac,host_route_entry->key_fields.dst_mac,SYS_ADPT_MAC_ADDR_LEN);
            AMTRL3_MGR_HostRouteEventHandler(AMTRL3_TYPE_FLAGS_IPV6, fib_id, event, &tunnel_entry);

       //patch to avoid nexthop ageout
            if(event ==HOST_ROUTE_UNREFERENCE_EVENT
                    && tunnel_entry.key_fields.tunnel_entry_type ==AMTRL3_TYPE_INETTOPHYSICALEXTTYPE_TUNNEL_MANUAL)
            {
                if(amtrl3_mgr_debug_tunnel)
                    BACKDOOR_MGR_Printf("Send arp to avoid timeout");
                IPAL_NEIGH_SendNeighborRequest(host_route_entry->key_fields.dst_vid_ifindex,
                                &(host_route_entry->key_fields.dst_inet_addr));
            }
        }
    }
}
#endif /*(SYS_CPNT_IP_TUNNEL == TRUE)*/

#if (SYS_CPNT_VXLAN == TRUE)
BOOL_T AMTRL3_MGR_AddVxlanTunnel(UI32_T fib_id, AMTRL3_TYPE_VxlanTunnelEntry_T* vxlan_tunnel_p)
{
    AMTRL3_OM_VxlanTunnelEntry_T vxlan_tunnel;

    if (amtrl3_mgr_debug_tunnel)
    {
        UI8_T ip_str[L_INET_MAX_IPADDR_STR_LEN] = {0};
        BACKDOOR_MGR_Printf("AMTRL3_MGR_AddVxlanTunnel\r\n");
        BACKDOOR_MGR_Printf("fib_id = %lu\r\n", fib_id);
        BACKDOOR_MGR_Printf("vfi_id = %lu\r\n", vxlan_tunnel_p->vfi_id);
        AMTRL3_MGR_Ntoa(&vxlan_tunnel_p->local_vtep, ip_str);
        BACKDOOR_MGR_Printf("local_vtep = %s\r\n", ip_str);
        AMTRL3_MGR_Ntoa(&vxlan_tunnel_p->remote_vtep, ip_str);
        BACKDOOR_MGR_Printf("remote_vtep = %s\r\n", ip_str);
        BACKDOOR_MGR_Printf("is mc = %u\r\n", vxlan_tunnel_p->is_mc);
        BACKDOOR_MGR_Printf("udp port = %u\r\n", vxlan_tunnel_p->udp_port);
        BACKDOOR_MGR_Printf("bcast group = %u\r\n", vxlan_tunnel_p->bcast_group);
    }

    memset(&vxlan_tunnel, 0, sizeof(vxlan_tunnel));
    vxlan_tunnel.vfi_id = vxlan_tunnel_p->vfi_id;
    vxlan_tunnel.local_vtep = vxlan_tunnel_p->local_vtep;
    vxlan_tunnel.remote_vtep = vxlan_tunnel_p->remote_vtep;
    if (AMTRL3_OM_GetVxlanTunnelEntry(fib_id, &vxlan_tunnel))
        return FALSE;

    vxlan_tunnel.is_mc = vxlan_tunnel_p->is_mc;
    vxlan_tunnel.udp_port = vxlan_tunnel_p->udp_port;
    vxlan_tunnel.bcast_group = vxlan_tunnel_p->bcast_group;
    vxlan_tunnel.in_chip_status = FALSE;
    vxlan_tunnel.uc_vxlan_port = 0;
    vxlan_tunnel.mc_vxlan_port = 0;
    vxlan_tunnel.uc_binding_cnt = 0;
    vxlan_tunnel.uc_hw_info = L_CVRT_UINT_TO_PTR(AMTRL3_OM_HW_INFO_INVALID);
    vxlan_tunnel.mc_hw_info = L_CVRT_UINT_TO_PTR(AMTRL3_OM_HW_INFO_INVALID);
    vxlan_tunnel.is_ecmp = TRUE;

    if (AMTRL3_OM_SetVxlanTunnelEntry(fib_id, &vxlan_tunnel) == FALSE)
    {
        BACKDOOR_MGR_Printf("AMTRL3_OM_SetVxlanTunnelEntry fail\r\n");
    }

    return TRUE;
}

static void AMTRL3_MGR_DelOneVxlanTunnelNexthop(
    UI32_T                              fib_id,
    AMTRL3_OM_VxlanTunnelEntry_T        *vxlan_tunnel_p,
    AMTRL3_OM_HostRouteEntry_T          *host_route_entry_p,
    AMTRL3_OM_VxlanTunnelNexthopEntry_T *tunnel_nexthop_p,
    BOOL_T                              is_recreate_mc_vxlan_port,
    BOOL_T                              is_save_chg)
{
    BOOL_T  is_tunnel_changed, is_tunnel_nexthop_changed;

    if (NULL == vxlan_tunnel_p)
        return;

    if (!tunnel_nexthop_p->is_uc_binding && !tunnel_nexthop_p->is_mc_binding)
        return;

    is_tunnel_nexthop_changed = FALSE;
    if (vxlan_tunnel_p->uc_vxlan_port != 0 &&
        tunnel_nexthop_p->is_uc_binding &&
        AMTRL3_MGR_IS_VALID_HW_INFO(host_route_entry_p->vxlan_uc_hw_info))
    {
        if (vxlan_tunnel_p->uc_binding_cnt > 1)
            SWDRV_RemoveVxlanEcmpNexthop(vxlan_tunnel_p->vfi_id,
                                vxlan_tunnel_p->uc_vxlan_port,
                                host_route_entry_p->vxlan_uc_hw_info);

        if (vxlan_tunnel_p->uc_binding_cnt > 0)
            vxlan_tunnel_p->uc_binding_cnt--;

        if (vxlan_tunnel_p->uc_binding_cnt == 0)
        {
            UI32_T ifindex;
            VXLAN_TYPE_R_PORT_CONVERTTO_L_PORT(vxlan_tunnel_p->uc_vxlan_port, ifindex);
            AMTR_PMGR_DeleteAddrByLifeTimeAndLPort(ifindex, AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT);

            SWDRV_DestroyVxlanPort(vxlan_tunnel_p->vfi_id, vxlan_tunnel_p->uc_vxlan_port, vxlan_tunnel_p->is_ecmp);
            vxlan_tunnel_p->uc_vxlan_port = 0;
        }

        host_route_entry_p->vxlan_uc_ref_count --;
        tunnel_nexthop_p->is_uc_binding = FALSE;
        is_tunnel_changed = TRUE;
        is_tunnel_nexthop_changed = TRUE;
    }

    if (vxlan_tunnel_p->mc_vxlan_port != 0 &&
        tunnel_nexthop_p->is_mc_binding &&
        AMTRL3_MGR_IS_VALID_HW_INFO(host_route_entry_p->vxlan_mc_hw_info))
    {
        AMTRL3_OM_VxlanTunnelNexthopEntry_T tmp_tunnel_nexthop;
        AMTRL3_OM_HostRouteEntry_T host_route;

        SWDRV_AddVtepIntoMcastGroup(vxlan_tunnel_p->bcast_group,
                            vxlan_tunnel_p->mc_vxlan_port, FALSE);

        SWDRV_DestroyVxlanPort(vxlan_tunnel_p->vfi_id, vxlan_tunnel_p->mc_vxlan_port, FALSE);
        host_route_entry_p->vxlan_mc_ref_count --;

        vxlan_tunnel_p->mc_vxlan_port = 0;
        tunnel_nexthop_p->is_mc_binding = FALSE;
        is_tunnel_nexthop_changed = TRUE;

        if (TRUE == is_recreate_mc_vxlan_port)
        {
            memset(&tmp_tunnel_nexthop, 0, sizeof(tmp_tunnel_nexthop));
            tmp_tunnel_nexthop.vfi_id = vxlan_tunnel_p->vfi_id;
            tmp_tunnel_nexthop.local_vtep = vxlan_tunnel_p->local_vtep;
            tmp_tunnel_nexthop.remote_vtep = vxlan_tunnel_p->remote_vtep;
            while (AMTRL3_OM_GetNextVxlanTunnelNexthopEntry(fib_id, &tmp_tunnel_nexthop))
            {
                if (0 == memcmp(tmp_tunnel_nexthop.nexthop_addr.addr, host_route_entry_p->key_fields.dst_inet_addr.addr, SYS_ADPT_IPV4_ADDR_LEN))
                    continue;

                host_route.key_fields.dst_vid_ifindex = tmp_tunnel_nexthop.nexthop_ifindex;
                host_route.key_fields.dst_inet_addr = tmp_tunnel_nexthop.nexthop_addr;

                if (AMTRL3_OM_GetHostRouteEntry(AMTRL3_TYPE_FLAGS_IPV4, fib_id, &host_route))
                {
                    if (AMTRL3_MGR_IS_VALID_HW_INFO(host_route.vxlan_mc_hw_info))
                    {
                        void  *hw_info;
                        NETCFG_TYPE_L3_Interface_T vlan_intf = {0};
                        UI32_T vid, unit, port, trunk_id;
                        BOOL_T is_tagged, is_trunk;

                        vlan_intf.ifindex = host_route.key_fields.dst_vid_ifindex;
                        if (NETCFG_TYPE_OK != NETCFG_POM_IP_GetL3Interface(&vlan_intf))
                            continue;

                        if (AMTRL3_MGR_DeriveDetailPortInfo (host_route.key_fields.dst_vid_ifindex,
                                                             host_route.key_fields.lport,
                                                             &vid, &unit, &port, &trunk_id,
                                                             &is_tagged, &is_trunk) == FALSE)
                            continue;

                        SWDRV_CreateVxlanNexthop(vlan_intf.drv_l3_intf_index, unit, port,
                              host_route.key_fields.dst_mac, TRUE,
                              &hw_info);

                        if (SWDRV_CreateVxlanNetworkPort(vxlan_tunnel_p->vfi_id,
                                        vxlan_tunnel_p->udp_port,
                                        TRUE,
                                        &vxlan_tunnel_p->local_vtep,
                                        &vxlan_tunnel_p->remote_vtep,
                                        FALSE,
                                        hw_info,
                                        &vxlan_tunnel_p->mc_vxlan_port))
                        {
                            SWDRV_AddVtepIntoMcastGroup(vxlan_tunnel_p->bcast_group,
                                            vxlan_tunnel_p->mc_vxlan_port, TRUE);
                            tmp_tunnel_nexthop.is_mc_binding = TRUE;
                            AMTRL3_OM_SetVxlanTunnelNexthopEntry(fib_id, &tmp_tunnel_nexthop);

                            //TODO: need to update and save the host_route.vxlan_mc_ref_count
                            host_route.vxlan_mc_ref_count++;
                            AMTRL3_OM_SetHostRouteEntry(AMTRL3_TYPE_FLAGS_IPV4, fib_id, &host_route);
                            break;
                        }
                    }
                }
            }
        }
    }

    if (TRUE == is_save_chg)
    {
        if (is_tunnel_changed)
            AMTRL3_OM_SetVxlanTunnelEntry(fib_id, vxlan_tunnel_p);

        if (is_tunnel_nexthop_changed)
            AMTRL3_OM_SetVxlanTunnelNexthopEntry(fib_id, tunnel_nexthop_p);
    }
}

BOOL_T AMTRL3_MGR_DeleteVxlanTunnel(UI32_T fib_id, AMTRL3_TYPE_VxlanTunnelEntry_T* vxlan_tunnel_p)
{
    AMTRL3_OM_VxlanTunnelEntry_T vxlan_tunnel;
    AMTRL3_OM_VxlanTunnelNexthopEntry_T tunnel_nexthop;
    AMTRL3_OM_HostRouteEntry_T host_route;

    if (amtrl3_mgr_debug_tunnel)
    {
        UI8_T ip_str[L_INET_MAX_IPADDR_STR_LEN] = {0};
        BACKDOOR_MGR_Printf("AMTRL3_MGR_DeleteVxlanTunnel\r\n");
        BACKDOOR_MGR_Printf("fib_id = %lu\r\n", fib_id);
        BACKDOOR_MGR_Printf("vfi_id = %lu\r\n", vxlan_tunnel_p->vfi_id);
        AMTRL3_MGR_Ntoa(&vxlan_tunnel_p->local_vtep, ip_str);
        BACKDOOR_MGR_Printf("local_vtep = %s\r\n", ip_str);
        AMTRL3_MGR_Ntoa(&vxlan_tunnel_p->remote_vtep, ip_str);
        BACKDOOR_MGR_Printf("remote_vtep = %s\r\n", ip_str);
        BACKDOOR_MGR_Printf("is mc = %u\r\n", vxlan_tunnel_p->is_mc);
        BACKDOOR_MGR_Printf("udp port = %u\r\n", vxlan_tunnel_p->udp_port);
        BACKDOOR_MGR_Printf("bcast group = %u\r\n", vxlan_tunnel_p->bcast_group);
    }

    memset(&vxlan_tunnel, 0, sizeof(vxlan_tunnel));
    vxlan_tunnel.vfi_id = vxlan_tunnel_p->vfi_id;
    vxlan_tunnel.local_vtep = vxlan_tunnel_p->local_vtep;
    vxlan_tunnel.remote_vtep = vxlan_tunnel_p->remote_vtep;
    if (!AMTRL3_OM_GetVxlanTunnelEntry(fib_id, &vxlan_tunnel))
        return TRUE;

    memset(&tunnel_nexthop, 0, sizeof(tunnel_nexthop));
    tunnel_nexthop.vfi_id = vxlan_tunnel.vfi_id;
    tunnel_nexthop.local_vtep = vxlan_tunnel.local_vtep;
    tunnel_nexthop.remote_vtep = vxlan_tunnel.remote_vtep;
    while (AMTRL3_OM_GetNextVxlanTunnelNexthopEntry(fib_id, &tunnel_nexthop))
    {
        if (tunnel_nexthop.vfi_id != vxlan_tunnel.vfi_id)
            break;
        if (0 != memcmp(tunnel_nexthop.local_vtep.addr, vxlan_tunnel.local_vtep.addr, SYS_ADPT_IPV4_ADDR_LEN))
            break;
        if (0 != memcmp(tunnel_nexthop.remote_vtep.addr, vxlan_tunnel.remote_vtep.addr, SYS_ADPT_IPV4_ADDR_LEN))
            break;

        memset(&host_route, 0, sizeof(AMTRL3_OM_HostRouteEntry_T));
        host_route.key_fields.dst_vid_ifindex = tunnel_nexthop.nexthop_ifindex;
        host_route.key_fields.dst_inet_addr = tunnel_nexthop.nexthop_addr;

        if (AMTRL3_OM_GetHostRouteEntry(AMTRL3_TYPE_FLAGS_IPV4, fib_id, &host_route))
        {
            AMTRL3_MGR_DelOneVxlanTunnelNexthop(
                fib_id, &vxlan_tunnel, &host_route, &tunnel_nexthop, FALSE, FALSE);

            if (host_route.ref_count > 0)
                host_route.ref_count--;

            if (host_route.vxlan_uc_ref_count > 0)
            {
                host_route.vxlan_uc_ref_count--;
                if (  (host_route.vxlan_uc_ref_count == 0)
                    &&(AMTRL3_MGR_IS_VALID_HW_INFO(host_route.vxlan_uc_hw_info))
                   )
                {
                    SWDRV_DestroyVxlanNexthop(host_route.vxlan_uc_hw_info);
                    host_route.vxlan_uc_hw_info = L_CVRT_UINT_TO_PTR(AMTRL3_OM_HW_INFO_INVALID);
                }
            }

            if (vxlan_tunnel.is_mc && host_route.vxlan_mc_ref_count > 0)
            {
                host_route.vxlan_mc_ref_count--;

                if (  (host_route.vxlan_mc_ref_count == 0)
                    &&(AMTRL3_MGR_IS_VALID_HW_INFO(host_route.vxlan_mc_hw_info))
                   )
                {
                    SWDRV_DestroyVxlanNexthop(host_route.vxlan_mc_hw_info);
                    host_route.vxlan_mc_hw_info = L_CVRT_UINT_TO_PTR(AMTRL3_OM_HW_INFO_INVALID);
                }
            }

            AMTRL3_OM_SetHostRouteEntry(AMTRL3_TYPE_FLAGS_IPV4, fib_id, &host_route);
        }

        AMTRL3_OM_DeleteVxlanTunnelNexthopEntry(fib_id, &tunnel_nexthop);
    }

    return AMTRL3_OM_DeleteVxlanTunnelEntry(fib_id, &vxlan_tunnel);
}

BOOL_T AMTRL3_MGR_AddVxlanTunnelNexthop(UI32_T fib_id, AMTRL3_TYPE_VxlanTunnelNexthopEntry_T* tunnel_nexthop_p)
{
    AMTRL3_OM_VxlanTunnelEntry_T vxlan_tunnel;
    AMTRL3_OM_VxlanTunnelNexthopEntry_T vxlan_tunnel_nexthop;
    AMTRL3_OM_HostRouteEntry_T nexthop;
    UI32_T                     event_type;

    if (amtrl3_mgr_debug_tunnel)
    {
        UI8_T ip_str[L_INET_MAX_IPADDR_STR_LEN] = {0};
        BACKDOOR_MGR_Printf("AMTRL3_MGR_AddVxlanTunnelNexthop\r\n");
        BACKDOOR_MGR_Printf("vfi_id = %lu\r\n", tunnel_nexthop_p->vfi_id);
        AMTRL3_MGR_Ntoa(&tunnel_nexthop_p->local_vtep, ip_str);
        BACKDOOR_MGR_Printf("local_vtep = %s\r\n", ip_str);
        AMTRL3_MGR_Ntoa(&tunnel_nexthop_p->remote_vtep, ip_str);
        BACKDOOR_MGR_Printf("remote_vtep = %s\r\n", ip_str);
        AMTRL3_MGR_Ntoa(&tunnel_nexthop_p->nexthop_addr, ip_str);
        BACKDOOR_MGR_Printf("nexthop addr = %s\r\n", ip_str);
        BACKDOOR_MGR_Printf("nexthop ifindex = %lu\r\n", tunnel_nexthop_p->nexthop_ifindex);
    }

    memset(&vxlan_tunnel, 0, sizeof(vxlan_tunnel));
    vxlan_tunnel.vfi_id = tunnel_nexthop_p->vfi_id;
    vxlan_tunnel.local_vtep = tunnel_nexthop_p->local_vtep;
    vxlan_tunnel.remote_vtep = tunnel_nexthop_p->remote_vtep;
    if (!AMTRL3_OM_GetVxlanTunnelEntry(fib_id, &vxlan_tunnel))
        return FALSE;

    vxlan_tunnel_nexthop.vfi_id = tunnel_nexthop_p->vfi_id;
    vxlan_tunnel_nexthop.local_vtep = tunnel_nexthop_p->local_vtep;
    vxlan_tunnel_nexthop.remote_vtep = tunnel_nexthop_p->remote_vtep;
    vxlan_tunnel_nexthop.nexthop_addr = tunnel_nexthop_p->nexthop_addr;
    vxlan_tunnel_nexthop.nexthop_ifindex = tunnel_nexthop_p->nexthop_ifindex;
    vxlan_tunnel_nexthop.is_uc_binding = FALSE;
    vxlan_tunnel_nexthop.is_mc_binding = FALSE;

    /* existance check */
    if (AMTRL3_OM_GetVxlanTunnelNexthopEntry(fib_id, &vxlan_tunnel_nexthop))
        return FALSE;

    if (!AMTRL3_OM_SetVxlanTunnelNexthopEntry(fib_id, &vxlan_tunnel_nexthop))
        return FALSE;

    memset(&nexthop, 0, sizeof(AMTRL3_OM_HostRouteEntry_T));
    nexthop.key_fields.dst_vid_ifindex = tunnel_nexthop_p->nexthop_ifindex;
    nexthop.key_fields.dst_inet_addr = tunnel_nexthop_p->nexthop_addr;

    if (AMTRL3_OM_GetHostRouteEntry(AMTRL3_TYPE_FLAGS_IPV4, fib_id, &nexthop))
    {
        if (nexthop.status == HOST_ROUTE_HOST_READY ||
            nexthop.status == HOST_ROUTE_GATEWAY_READY ||
            nexthop.status == HOST_ROUTE_READY_NOT_SYNC)
        {
            event_type = HOST_ROUTE_READY_EVENT;
        }
        else
        {
            event_type = HOST_ROUTE_PARTIAL_EVENT;
        }
    }
    else
    {
        nexthop.hw_info = L_CVRT_UINT_TO_PTR(AMTRL3_OM_HW_INFO_INVALID);
        nexthop.vxlan_uc_hw_info = L_CVRT_UINT_TO_PTR(AMTRL3_OM_HW_INFO_INVALID);
        nexthop.vxlan_mc_hw_info = L_CVRT_UINT_TO_PTR(AMTRL3_OM_HW_INFO_INVALID);
        nexthop.entry_type = VAL_ipNetToPhysicalExtType_dynamic;
        event_type = HOST_ROUTE_PARTIAL_EVENT;
    }

    if (vxlan_tunnel.is_mc)
        nexthop.vxlan_mc_ref_count++;
    nexthop.vxlan_uc_ref_count++;
    nexthop.ref_count++;
    AMTRL3_MGR_HostRouteEventHandler(AMTRL3_TYPE_FLAGS_IPV4, fib_id, event_type, &nexthop);

    return TRUE;
}

/* only delete nexthop in one tunnel
 */
BOOL_T AMTRL3_MGR_DeleteVxlanTunnelNexthop(UI32_T fib_id, AMTRL3_TYPE_VxlanTunnelNexthopEntry_T* tunnel_nexthop_p)
{
    AMTRL3_OM_VxlanTunnelEntry_T vxlan_tunnel;
    AMTRL3_OM_VxlanTunnelNexthopEntry_T vxlan_tunnel_nexthop;
    AMTRL3_OM_HostRouteEntry_T host_route;

    if (amtrl3_mgr_debug_tunnel)
    {
        UI8_T ip_str[L_INET_MAX_IPADDR_STR_LEN] = {0};
        BACKDOOR_MGR_Printf("AMTRL3_MGR_DeleteVxlanTunnelNexthop\r\n");
        BACKDOOR_MGR_Printf("vfi_id = %lu\r\n", tunnel_nexthop_p->vfi_id);
        AMTRL3_MGR_Ntoa(&tunnel_nexthop_p->local_vtep, ip_str);
        BACKDOOR_MGR_Printf("local_vtep = %s\r\n", ip_str);
        AMTRL3_MGR_Ntoa(&tunnel_nexthop_p->remote_vtep, ip_str);
        BACKDOOR_MGR_Printf("remote_vtep = %s\r\n", ip_str);
        AMTRL3_MGR_Ntoa(&tunnel_nexthop_p->nexthop_addr, ip_str);
        BACKDOOR_MGR_Printf("nexthop addr = %s\r\n", ip_str);
        BACKDOOR_MGR_Printf("nexthop ifindex = %lu\r\n", tunnel_nexthop_p->nexthop_ifindex);
    }

    memset(&vxlan_tunnel, 0, sizeof(vxlan_tunnel));
    vxlan_tunnel.vfi_id = tunnel_nexthop_p->vfi_id;
    vxlan_tunnel.local_vtep = tunnel_nexthop_p->local_vtep;
    vxlan_tunnel.remote_vtep = tunnel_nexthop_p->remote_vtep;
    if (!AMTRL3_OM_GetVxlanTunnelEntry(fib_id, &vxlan_tunnel))
        return FALSE;

    vxlan_tunnel_nexthop.vfi_id = tunnel_nexthop_p->vfi_id;
    vxlan_tunnel_nexthop.local_vtep = tunnel_nexthop_p->local_vtep;
    vxlan_tunnel_nexthop.remote_vtep = tunnel_nexthop_p->remote_vtep;
    vxlan_tunnel_nexthop.nexthop_addr = tunnel_nexthop_p->nexthop_addr;
    vxlan_tunnel_nexthop.nexthop_ifindex = tunnel_nexthop_p->nexthop_ifindex;
    if (!AMTRL3_OM_GetVxlanTunnelNexthopEntry(fib_id, &vxlan_tunnel_nexthop))
        return FALSE;

    memset(&host_route, 0, sizeof(AMTRL3_OM_HostRouteEntry_T));
    host_route.key_fields.dst_vid_ifindex = tunnel_nexthop_p->nexthop_ifindex;
    host_route.key_fields.dst_inet_addr = tunnel_nexthop_p->nexthop_addr;

    if (AMTRL3_OM_GetHostRouteEntry(AMTRL3_TYPE_FLAGS_IPV4, fib_id, &host_route))
    {
        AMTRL3_MGR_DelOneVxlanTunnelNexthop(
            fib_id, &vxlan_tunnel, &host_route, &vxlan_tunnel_nexthop, TRUE, TRUE);

        if (host_route.ref_count > 0)
            host_route.ref_count--;

        if (host_route.vxlan_uc_ref_count > 0)
        {
            host_route.vxlan_uc_ref_count--;
            if (  (host_route.vxlan_uc_ref_count == 0)
                &&(AMTRL3_MGR_IS_VALID_HW_INFO(host_route.vxlan_uc_hw_info))
               )
            {
                SWDRV_DestroyVxlanNexthop(host_route.vxlan_uc_hw_info);
                host_route.vxlan_uc_hw_info = L_CVRT_UINT_TO_PTR(AMTRL3_OM_HW_INFO_INVALID);
            }
        }

        if (vxlan_tunnel.is_mc && host_route.vxlan_mc_ref_count > 0)
        {
            host_route.vxlan_mc_ref_count--;

            if (  (host_route.vxlan_mc_ref_count == 0)
                &&(AMTRL3_MGR_IS_VALID_HW_INFO(host_route.vxlan_mc_hw_info))
               )
            {
                SWDRV_DestroyVxlanNexthop(host_route.vxlan_mc_hw_info);
                host_route.vxlan_mc_hw_info = L_CVRT_UINT_TO_PTR(AMTRL3_OM_HW_INFO_INVALID);
            }
        }

        AMTRL3_OM_SetHostRouteEntry(AMTRL3_TYPE_FLAGS_IPV4, fib_id, &host_route);
    }

    return AMTRL3_OM_DeleteVxlanTunnelNexthopEntry(fib_id, &vxlan_tunnel_nexthop);
}

static void AMTRL3_MGR_SetVxlanTunnelByNexthopEvent(UI32_T fib_id, AMTRL3_OM_HostRouteEntry_T *host_route_entry ,UI32_T event)
{
    AMTRL3_OM_VxlanTunnelEntry_T vxlan_tunnel;
    AMTRL3_OM_VxlanTunnelNexthopEntry_T tunnel_nexthop;
    NETCFG_TYPE_L3_Interface_T vlan_intf;
    UI32_T vid, unit, port, trunk_id;
    BOOL_T is_tagged, is_trunk;
    BOOL_T is_tunnel_changed, is_tunnel_nexthop_changed;

    if (host_route_entry->entry_type != VAL_ipNetToPhysicalExtType_dynamic &&
        host_route_entry->entry_type != VAL_ipNetToPhysicalExtType_static)
        return;

    if (amtrl3_mgr_debug_tunnel)
    {
        UI8_T ip_str[L_INET_MAX_IPADDR_STR_LEN] = {0};
        BACKDOOR_MGR_Printf("AMTRL3_MGR_SetVxlanTunnelByNexthopEvent\r\n");
        AMTRL3_MGR_Ntoa(&host_route_entry->key_fields.dst_inet_addr, ip_str);
        BACKDOOR_MGR_Printf("ip addr = %s\r\n", ip_str);
        BACKDOOR_MGR_Printf("event = %lu\r\n", event);
    }

    if (event == HOST_ROUTE_READY_EVENT)
    {
        memset(&vlan_intf, 0, sizeof (vlan_intf));
        vlan_intf.ifindex = host_route_entry->key_fields.dst_vid_ifindex;
        if (NETCFG_TYPE_OK != NETCFG_POM_IP_GetL3Interface(&vlan_intf))
            return;

        if (AMTRL3_MGR_DeriveDetailPortInfo (host_route_entry->key_fields.dst_vid_ifindex,
                                             host_route_entry->key_fields.lport,
                                             &vid, &unit, &port, &trunk_id,
                                             &is_tagged, &is_trunk) == FALSE)
            return;

        if (host_route_entry->vxlan_uc_ref_count > 0 && !AMTRL3_MGR_IS_VALID_HW_INFO(host_route_entry->vxlan_uc_hw_info))
        {
            SWDRV_CreateVxlanNexthop(vlan_intf.drv_l3_intf_index, unit, port,
                  host_route_entry->key_fields.dst_mac, FALSE,
                  &host_route_entry->vxlan_uc_hw_info);
        }

        if (host_route_entry->vxlan_mc_ref_count > 0 && !AMTRL3_MGR_IS_VALID_HW_INFO(host_route_entry->vxlan_mc_hw_info))
        {
            SWDRV_CreateVxlanNexthop(vlan_intf.drv_l3_intf_index, unit, port,
                  host_route_entry->key_fields.dst_mac, TRUE,
                  &host_route_entry->vxlan_mc_hw_info);
        }

        memset(&tunnel_nexthop, 0, sizeof(tunnel_nexthop));
        tunnel_nexthop.nexthop_addr = host_route_entry->key_fields.dst_inet_addr;
        tunnel_nexthop.nexthop_ifindex = host_route_entry->key_fields.dst_vid_ifindex;
        while (AMTRL3_OM_GetNextVxlanTunnelByNexthop(fib_id, &tunnel_nexthop))
        {
            if (tunnel_nexthop.is_uc_binding && tunnel_nexthop.is_mc_binding)
                continue;

            vxlan_tunnel.vfi_id = tunnel_nexthop.vfi_id;
            vxlan_tunnel.local_vtep = tunnel_nexthop.local_vtep;
            vxlan_tunnel.remote_vtep = tunnel_nexthop.remote_vtep;
            if (AMTRL3_OM_GetVxlanTunnelEntry(fib_id, &vxlan_tunnel) == FALSE)
                continue;

            is_tunnel_changed = FALSE;
            is_tunnel_nexthop_changed = FALSE;
            if (AMTRL3_MGR_IS_VALID_HW_INFO(host_route_entry->vxlan_uc_hw_info))
            {
                if (vxlan_tunnel.uc_vxlan_port == 0)
                {
                    if (SWDRV_CreateVxlanNetworkPort(vxlan_tunnel.vfi_id,
                                    vxlan_tunnel.udp_port,
                                    FALSE,
                                    &vxlan_tunnel.local_vtep,
                                    &vxlan_tunnel.remote_vtep,
                                    vxlan_tunnel.is_ecmp,
                                    host_route_entry->vxlan_uc_hw_info,
                                    &vxlan_tunnel.uc_vxlan_port))
                    {
                        AMTR_TYPE_AddrEntry_T addr_entry;
                        UI16_T l_vxlan_port;

                        tunnel_nexthop.is_uc_binding = TRUE;
                        vxlan_tunnel.uc_binding_cnt = 1;
                        is_tunnel_changed = TRUE;
                        is_tunnel_nexthop_changed = TRUE;

                        /* update network port to vxlan static MAC entry*/
                        memset(&addr_entry, 0, sizeof(AMTR_TYPE_AddrEntry_T));
                        addr_entry.vid = vxlan_tunnel.vfi_id;
                        VXLAN_TYPE_R_PORT_CONVERTTO_L_PORT(vxlan_tunnel.uc_vxlan_port, l_vxlan_port);

                        while (AMTR_PMGR_GetNextVMAddrEntry(&addr_entry, AMTR_MGR_GET_STATIC_ADDRESS))
                        {
                            if (0==memcmp(addr_entry.r_vtep_ip, &vxlan_tunnel.remote_vtep.addr, SYS_ADPT_IPV4_ADDR_LEN) && 
                                (l_vxlan_port != addr_entry.ifindex))
                            {
                                addr_entry.ifindex = l_vxlan_port;
                                if (!AMTR_PMGR_SetAddrEntry(&addr_entry))
                                {
                                    if (amtrl3_mgr_debug_flag & DEBUG_LEVEL_ERROR)
                                    {
                                        BACKDOOR_MGR_Printf("%s: Failed to set vxlan static MAC address table.\n", __FUNCTION__);
                                    }
                                }
                            }
                        }
                    }
                }
                else
                {
                    if (tunnel_nexthop.is_uc_binding == FALSE)
                    {
                        if (SWDRV_AddVxlanEcmpNexthop(vxlan_tunnel.vfi_id,
                                            vxlan_tunnel.uc_vxlan_port,
                                            host_route_entry->vxlan_uc_hw_info) == TRUE)
                        {
                            vxlan_tunnel.uc_binding_cnt++;
                            tunnel_nexthop.is_uc_binding = TRUE;
                            is_tunnel_changed = TRUE;
                            is_tunnel_nexthop_changed = TRUE;
                        }
                    }
                }
            }

            if (AMTRL3_MGR_IS_VALID_HW_INFO(host_route_entry->vxlan_mc_hw_info))
            {
                if (vxlan_tunnel.mc_vxlan_port == 0)
                {
                    void  *hw_info;

                    SWDRV_CreateVxlanNexthop(vlan_intf.drv_l3_intf_index, unit, port,
                          host_route_entry->key_fields.dst_mac, TRUE,
                          &hw_info);

                    if (SWDRV_CreateVxlanNetworkPort(vxlan_tunnel.vfi_id,
                                    vxlan_tunnel.udp_port,
                                    TRUE,
                                    &vxlan_tunnel.local_vtep,
                                    &vxlan_tunnel.remote_vtep,
                                    FALSE,//vxlan_tunnel.is_ecmp,
                                    hw_info,//host_route_entry->vxlan_mc_hw_info,
                                    &vxlan_tunnel.mc_vxlan_port))
                    {
                        SWDRV_AddVtepIntoMcastGroup(vxlan_tunnel.bcast_group,
                                        vxlan_tunnel.mc_vxlan_port, TRUE);
                        tunnel_nexthop.is_mc_binding = TRUE;
                        is_tunnel_changed = TRUE;
                        is_tunnel_nexthop_changed = TRUE;
                    }
                }
                /*else  // mc_vxlan_port not support ECMP now?
                {
                    if (tunnel_nexthop.is_mc_binding == FALSE)
                    {
                        if  (SWDRV_AddVxlanEcmpNexthop(vxlan_tunnel.vfi_id,
                                        vxlan_tunnel.mc_vxlan_port,
                                        host_route_entry->vxlan_mc_hw_info) == TRUE)
                        {
                            tunnel_nexthop.is_mc_binding = TRUE;
                            is_tunnel_nexthop_changed = TRUE;
                        }
                    }
                }*/
            }

            if (is_tunnel_changed)
                AMTRL3_OM_SetVxlanTunnelEntry(fib_id, &vxlan_tunnel);

            if (is_tunnel_nexthop_changed)
                AMTRL3_OM_SetVxlanTunnelNexthopEntry(fib_id, &tunnel_nexthop);
        }
    }
    else
    {
        memset(&tunnel_nexthop, 0, sizeof(tunnel_nexthop));
        tunnel_nexthop.nexthop_addr = host_route_entry->key_fields.dst_inet_addr;
        tunnel_nexthop.nexthop_ifindex = host_route_entry->key_fields.dst_vid_ifindex;
        while (AMTRL3_OM_GetNextVxlanTunnelByNexthop(fib_id, &tunnel_nexthop))
        {
            vxlan_tunnel.vfi_id = tunnel_nexthop.vfi_id;
            vxlan_tunnel.local_vtep = tunnel_nexthop.local_vtep;
            vxlan_tunnel.remote_vtep = tunnel_nexthop.remote_vtep;
            if (AMTRL3_OM_GetVxlanTunnelEntry(fib_id, &vxlan_tunnel) == FALSE)
                continue;

            AMTRL3_MGR_DelOneVxlanTunnelNexthop(
                fib_id, &vxlan_tunnel, host_route_entry, &tunnel_nexthop, TRUE, TRUE);
        }

        if (AMTRL3_MGR_IS_VALID_HW_INFO(host_route_entry->vxlan_uc_hw_info))
        {
            SWDRV_DestroyVxlanNexthop(host_route_entry->vxlan_uc_hw_info);
            host_route_entry->vxlan_uc_hw_info = L_CVRT_UINT_TO_PTR(AMTRL3_OM_HW_INFO_INVALID);
        }

        if (AMTRL3_MGR_IS_VALID_HW_INFO(host_route_entry->vxlan_mc_hw_info))
        {
            SWDRV_DestroyVxlanNexthop(host_route_entry->vxlan_mc_hw_info);
            host_route_entry->vxlan_mc_hw_info = L_CVRT_UINT_TO_PTR(AMTRL3_OM_HW_INFO_INVALID);
        }
    }
}

static UI32_T AMTRL3_MGR_HotInsertAddVxlanHostRouteToModule(UI32_T fib_id, AMTRL3_OM_HostRouteEntry_T *host_entry)
{
    /* Currently not support stacking
     */
    return AMTRL3_MGR_ADD_HOST_ERROR;
}

#endif

static BOOL_T AMTRL3_MGR_GetAMTRExactMAC(AMTR_TYPE_AddrEntry_T *amtr_addr_entry_p)
{
    extern BOOL_T AMTRDRV_OM_GetExactRecord(UI8_T *addr_entry);

    AMTRDRV_TYPE_Record_T get_entry;

    memcpy(&get_entry.address, amtr_addr_entry_p, sizeof(AMTR_TYPE_AddrEntry_T));

    if (FALSE == AMTRDRV_OM_GetExactRecord((UI8_T *)&get_entry))
    {
        return FALSE;
    }

    memcpy(amtr_addr_entry_p, &get_entry.address, sizeof(AMTR_TYPE_AddrEntry_T));

    return TRUE;
}

