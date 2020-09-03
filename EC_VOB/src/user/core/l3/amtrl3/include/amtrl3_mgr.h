/* =====================================================================================
 *  Module Name: AMTRL3_MGR.H
 *  Purpose :
 *      Interface between IP(v4 & v6) stack and SWDRVL3, manage routing table, neighbor table
 *      and pass configuration to SWDRVL3 setting chip which has Layer-3 capability.
 *
 *  Notes:
 *      1. AMTRL3 handle all configuration commands, maintains Net Route Table and
 *         Neighbor host table which provide MIB information.
 *      2. AMTRL3 also handle all lower layer event, eg. port link up/down, trunking
 *         maintain host route table.
 *      3. AMTRL3 does not support the following criteria:
 *         No Replacement  - New host entry will be discard if max num of host entry
 *                           is reached.
 *         No ARP age out  - AMTRL3 does not support automatic arp ageout mechanism.
 *                           Host entry will only be remove base on ARP Ageout request
 *                           from IP stack.
 *         No Supernetting - AMTRL3 does not support supernetting for Net routes
 *
 *
 *  History :
 *      Date            Modifier    Reason
 *  -----------------------------------------------------------------------
 *      04-28-2003        hyliao    First Created
 *      01-12-2004        amytu     Redesign
 *      06-16-2005        Lloyd     Add IPv6 support
 * -------------------------------------------------------------------------------------
 * Copyright(C)        Accton Techonology Corporation, 2002,2003,2004,2005
 * =====================================================================================
 */

#ifndef _AMTRL3_MGR_H_
#define _AMTRL3_MGR_H_

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_adpt.h"
#include "sysfun.h"
#include "amtr_type.h"
#include "amtrl3_om.h"
#include "amtrl3_type.h"


/* NAME CONSTANT DECLARATIONS
 */


/* MACRO FUNCTION DECLARATIONS
 */

#define     AMTRL3_MGR_NUMBER_OF_HOST_ENTRY_TO_SCAN             (SYS_ADPT_MAX_NBR_OF_HOST_ROUTE / SYS_ADPT_MAX_SCAN_TIME * 2)
#define     AMTRL3_MGR_MAX_MAC_SCAN_TIME                        20  /* how many timer events */
#define     AMTRL3_MGR_NUMBER_OF_NET_ENTRY_TO_PROCESS           200
#define     AMTRL3_MGR_NUMBER_OF_NET_ENTRY_TO_SCAN              50
#define     AMTRL3_MGR_NUMBER_OF_UNRESOLVED_ENTRY_TO_PROCESS    50
#define     AMTRL3_MGR_NUMBER_OF_GATEWAY_ENTRY_TO_PROCESS       50
#define     AMTRL3_MGR_GATEWAY_ENTRY_DELAY_INTERVAL             4
#define     AMTRL3_MGR_STATIC_ENTRY_DELAY_INTERVAL              4
#define     AMTRL3_MGR_IPV4_MAXIMUM_PREFIX_LENGTH               32
#define     AMTRL3_MGR_IPV4_MINIMUM_PREFIX_LENGTH               8
#define     AMTRL3_MGR_IPV6_MAXIMUM_PREFIX_LENGTH               64
#define     AMTRL3_MGR_IPV6_MINIMUM_PREFIX_LENGTH               3
#define     AMTRL3_MGR_NBR_OF_RESOLVED_NET_ENTRY_TO_PROCESS     8
#define     AMTRL3_MGR_MAXIMUM_ECMP_PATH                        SYS_ADPT_MAX_NBR_OF_ECMP_ENTRY_PER_ROUTE

#define DEBUG_LEVEL_TRACE_HOST      0x01
#define DEBUG_LEVEL_TRACE_NET       0x02
#define DEBUG_LEVEL_ERROR           0x04
#define DEBUG_LEVEL_DETAIL          0x08
#define DEBUG_LEVEL_SUPERNET        0x10
#define DEBUG_LEVEL_HOTINSRT        0x20

#define CALLBACK_DEBUG_LPORT_DOWN       0x0001
#define CALLBACK_DEBUG_TRUNK            0x0002
#define CALLBACK_DEBUG_MAC              0x0004
#define CALLBACK_DEBUG_PORT_N_VID       0x0008
#define CALLBACK_DEBUG_PORT_MOVE        0x0010
#define CALLBACK_DEBUG_VLAN_DESTROY     0x0020
#define CALLBACK_DEBUG_VLAN_DOWN        0x0040
#define CALLBACK_DEBUG_DETAIL           0x0080
#define CALLBACK_TRUNK_DETAIL           0x0100

#define TASK_DEBUG_HITBIT           0x01
#define TASK_DEBUG_COMPENSATE       0x02
#define TASK_DEBUG_ARP_UNRESOLVED   0x04
#define TASK_DEBUG_ARP_GATEWAY      0x08
#define TASK_DEBUG_RESOLVED         0x10
#define TASK_DEBUG_MAC_SCAN         0x20

/* chip capacity flags */
/* */
#define AMTRL3_CHIP_SUPPORT_EGRESS_OBJECT                           1
#define AMTRL3_CHIP_SUPPORT_SAME_HW_INFO_FOR_ECMP_PATH              (1 << 1)
#define AMTRL3_CHIP_SUPPORT_L3_TRUNK_LOAD_BALANCE                   (1 << 2)
#define AMTRL3_CHIP_USE_LOCAL_HOST_TO_TRAP_MY_IP_PKTS               (1 << 3)
#define AMTRL3_CHIP_USE_DEFAULT_ROUTE_TRAP_TO_CPU                   (1 << 4)
#define AMTRL3_CHIP_USE_TCAM_FOR_ROUTE_TABLE                        (1 << 5)
#define AMTRL3_CHIP_KEEP_TRUNK_ID_IN_HOST_ROUTE                     (1 << 6)
#define AMTRL3_CHIP_NEED_SRC_MAC_TO_PROGRM_HOST_ROUTE               (1 << 7)
#define AMTRL3_CHIP_NEED_SRC_MAC_TO_PROGRM_NET_ROUTE                (1 << 8)
#define AMTRL3_CHIP_NEED_NH_IP_TO_PROGRAM_NET_ROUTE                 (1 << 9)

#define AMTRL3_MGR_MSGBUF_TYPE_SIZE sizeof(union AMTRL3_MGR_IPCMsg_Type_U)
#define AMTRL3_MGR_GET_MSG_SIZE(field_name)                       \
            (AMTRL3_MGR_MSGBUF_TYPE_SIZE +                        \
            sizeof(((AMTRL3_MGR_IPCMsg_T*)0)->data.field_name))


/* DATA TYPE DECLARATIONS
 */
typedef struct AMTRL3_MGR_FIB_S
{
    UI32_T   fib_id;
    L_INET_AddrIp_T amtrl3_mgr_dstip_for_hitbit_scan;              /* inet address of the last hitbit scaned entryp */
#if(SYS_CPNT_IP_TUNNEL == TRUE)
    L_INET_AddrIp_T amtrl3_mgr_tunnel_dstip_for_hitbit_scan;
#endif
    AMTRL3_OM_NetRouteEntry_T    amtrl3_mgr_net_route_entry_block[AMTRL3_MGR_NUMBER_OF_NET_ENTRY_TO_SCAN];
    AMTRL3_OM_HostRouteEntry_T   amtrl3_mgr_host_route_entry_block[AMTRL3_MGR_NUMBER_OF_HOST_ENTRY_TO_SCAN];
}AMTRL3_MGR_FIB_T;

/* structure for the request/response ipc message in csc pmgr and mgr
 */
typedef struct AMTRL3_MGR_IPCMsg_S
{
    union AMTRL3_MGR_IPCMsg_Type_U
    {
        UI32_T cmd;    /* for sending IPC request. CSCA_MGR_IPC_CMD1 ... */
        BOOL_T result_bool;  /*respond bool return*/
        UI32_T result_ui32;  /*respond ui32 return*/
        UI32_T result_i32;  /*respond i32 return*/
        SYS_TYPE_Get_Running_Cfg_T result_running_cfg; /* For running config API */
    } type;

    union
    {
        BOOL_T  bool_v;
        UI8_T   ui8_v;
        I8_T    i8_v;
        UI32_T  ui32_v;
        UI16_T  ui16_v;
        I32_T   i32_v;
        I16_T   i16_v;
        UI8_T   ip4_v[SYS_ADPT_IPV4_ADDR_LEN];
        int     int_v;

        struct
        {
            UI32_T action_flags;
            UI32_T fib_id;
            AMTRL3_TYPE_InetHostRouteEntry_T host_entry;
            UI32_T type;
        } host_route;

        struct
        {
            UI32_T action_flags;
            UI32_T vr_id;
        } ip_forwarding_status_index;

        struct
        {
            UI32_T action_flags;
            UI32_T fib_id;
            AMTRL3_TYPE_ipNetToPhysicalEntry_T ip_net_to_physical_entry;
        } ip_net_to_physical;

        struct
        {
            UI32_T action_flags;
            UI32_T fib_id;
            UI32_T v4_timeout;
            UI32_T v6_timeout;
        } host_timeout;

        struct
        {
            UI32_T action_flags;
            UI32_T fib_id;
            AMTRL3_TYPE_InetCidrRouteEntry_T net_route_entry;
        } net_route;

        struct
        {
            UI32_T action_flags;
            UI32_T fib_id;
            AMTRL3_TYPE_InetCidrRouteEntry_T net_route_entry[SYS_ADPT_MAX_NBR_OF_ECMP_ENTRY_PER_ROUTE];
            UI32_T num;
        } ecmp_route;

        struct
        {
            UI32_T fib_id;
            UI8_T route_mac[SYS_ADPT_MAC_ADDR_LEN];
            UI32_T ifindex;
        } l3_if;

        struct
        {
            UI32_T action_flags;
            UI32_T fib_id;
        } clear_arp;
        struct
        {
            UI32_T ui32_1;
            UI32_T ui32_2;
        }arg_ui32_ui32;
        struct
        {
            UI32_T ui32_1;
            L_INET_AddrIp_T  addr_2;
        }arg_ui32_addr;

        struct
        {
            AMTRL3_TYPE_InetHostRouteEntry_T host_entry;
            UI32_T                           ttl;
        } tunnel_update_ttl;
#if (SYS_CPNT_VXLAN == TRUE)
        struct
        {
            UI32_T fib_id;
            AMTRL3_TYPE_VxlanTunnelEntry_T tunnel;
        } vxlan_tunnel;

        struct
        {
            UI32_T fib_id;
            AMTRL3_TYPE_VxlanTunnelNexthopEntry_T nexthop;
        } vxlan_tunnel_nexthop;
#endif
    } data; /* contains the supplemntal data for the corresponding cmd */
} AMTRL3_MGR_IPCMsg_T;

typedef enum AMTRL3_MGR_IPCCMD_E
{
    AMTRL3_MGR_IPCCMD_CREATEFIB,
    AMTRL3_MGR_IPCCMD_DELETEFIB,
    AMTRL3_MGR_IPCCMD_ENABLEIPFORWARDING,
    AMTRL3_MGR_IPCCMD_DISABLEIPFORWARDING,
    AMTRL3_MGR_IPCCMD_SETHOSTROUTE,
    AMTRL3_MGR_IPCCMD_DELETEHOSTROUTE,
#if (SYS_CPNT_PBR == TRUE)
    AMTRL3_MGR_IPCCMD_SETHOSTROUTEFORPBR,
    AMTRL3_MGR_IPCCMD_DELETEHOSTROUTEFORPBR,
#endif
    AMTRL3_MGR_IPCCMD_REPLACEHOSTROUTE,
    AMTRL3_MGR_IPCCMD_SETHOSTROUTETIMEOUT,
    AMTRL3_MGR_IPCCMD_SETINETCIDRROUTE,
    AMTRL3_MGR_IPCCMD_DELETEINETCIDRROUTE,
    AMTRL3_MGR_IPCCMD_ADDECMPROUTEMULTIPATH,
    AMTRL3_MGR_IPCCMD_DELETEECMPROUTE,
    AMTRL3_MGR_IPCCMD_ADDECMPROUTEONEPATH,
    AMTRL3_MGR_IPCCMD_DELETEECMPROUTEONEPATH,
    AMTRL3_MGR_IPCCMD_CREATEL3INTERFACE,
    AMTRL3_MGR_IPCCMD_DELETEL3INTERFACE,
    AMTRL3_MGR_IPCCMD_ADDL3MAC,
    AMTRL3_MGR_IPCCMD_DELETEL3MAC,
    AMTRL3_MGR_IPCCMD_CLEARALLDYNAMICARP,
#if (SYS_CPNT_IP_TUNNEL == TRUE)
    /*AMTRL3_MGR_IPCCMD_CREATEDYNAMICTUNNEL,*/
    AMTRL3_MGR_IPCCMD_DELETETUNNELENTRIES,
    AMTRL3_MGR_IPCCMD_UPDATETUNNELTTL,
#endif/*SYS_CPNT_IP_TUNNEL*/

    AMTRL3_MGR_IPCCMD_FOLLOWISASYNCHRONISMIPC,

    AMTRL3_MGR_IPCCMD_MACTABLEDELETEBYMSTIDONPORT,
    AMTRL3_MGR_IPCCMD_SIGNAL_L3IF_RIF_DESTROY,

#if (SYS_CPNT_VXLAN == TRUE)
    AMTRL3_MGR_IPCCMD_ADD_VXLAN_TUNNEL,
    AMTRL3_MGR_IPCCMD_DEL_VXLAN_TUNNEL,
    AMTRL3_MGR_IPCCMD_ADD_VXLAN_TUNNEL_NEXTHOP,
    AMTRL3_MGR_IPCCMD_DEL_VXLAN_TUNNEL_NEXTHOP,
#endif

}AMTRL3_MGR_IPCCMD_T;


/* EXPORTED FUNCTION PROTOTYPE
 */
/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_Initiate_System_Resources
 * -------------------------------------------------------------------------
 * PURPOSE:  Initialize amtrl3_arpom amtrl3_netroute amtrl3_hostroute to manage.
 * INPUT:    none.
 * OUTPUT:   none.
 * RETURN:   none.
 * NOTES:
 * -------------------------------------------------------------------------*/
void AMTRL3_MGR_Initiate_System_Resources(void);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_EnterMasterMode
 * -------------------------------------------------------------------------
 * PURPOSE:  This function will set amtrl3_mgr into master mode.
 * INPUT:    none.
 * OUTPUT:   none.
 * RETURN:   none.
 * NOTES:
 * -------------------------------------------------------------------------*/
void AMTRL3_MGR_EnterMasterMode(void);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_EnterSlaveMode
 * -------------------------------------------------------------------------
 * PURPOSE:  This function will set amtrl3_mgr into slave mode.
 * INPUT:    none.
 * OUTPUT:   none.
 * RETURN:   none.
 * NOTES:
 * -------------------------------------------------------------------------*/
void AMTRL3_MGR_EnterSlaveMode(void);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_EnterTransitionMode
 * -------------------------------------------------------------------------
 * PURPOSE:  This function will set amtrl3_mgr into transition mode.
 * INPUT:    none.
 * OUTPUT:   none.
 * RETURN:   none.
 * NOTES:
 * -------------------------------------------------------------------------*/
void AMTRL3_MGR_EnterTransitionMode(void);

/*------------------------------------------------------------------------------
 * Function : AMTRL3_MGR_SetTransitionMode()
 *------------------------------------------------------------------------------
 * Purpose  : This function will set the operation mode to transition mode
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-----------------------------------------------------------------------------*/
void AMTRL3_MGR_SetTransitionMode(void);

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
SYS_TYPE_Stacking_Mode_T AMTRL3_MGR_GetOperationMode(void);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_CreateFIB
 * -------------------------------------------------------------------------
 * PURPOSE:  create a FIB
 * INPUT:    fib_id - FIB id (1 ~ 255).
 * OUTPUT:   none.
 * RETURN:   AMTRL3_MGR_FIB_ACTION_SUCCESS,
 *           AMTRL3_MGR_FIB_ACTION_EXCEPTION,
 *           AMTRL3_MGR_FIB_ACTION_FAIL.
 * NOTES:
 * -------------------------------------------------------------------------*/
UI32_T AMTRL3_MGR_CreateFIB(UI32_T fib_id);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_DeleteFIB
 * -------------------------------------------------------------------------
 * PURPOSE:  Delete a FIB
 * INPUT:    fib_id - FIB id (1 ~ 255).
 * OUTPUT:   none.
 * RETURN:   AMTRL3_MGR_FIB_ACTION_SUCCESS,
 *           AMTRL3_MGR_FIB_ACTION_EXCEPTION,
 *           AMTRL3_MGR_FIB_ACTION_FAIL.
 * NOTES:
 * -------------------------------------------------------------------------*/
UI32_T AMTRL3_MGR_DeleteFIB(UI32_T fib_id);

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
UI32_T AMTRL3_MGR_ClearFIB(UI32_T fib_id);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_GetNextFIBID
 * -------------------------------------------------------------------------
 * PURPOSE:  Get next FIB id(0 ~ 254).
 * INPUT:    fib_id - AMTRL3_OM_GET_FIRST_FIB_ID to get first
 * OUTPUT:   fib_id
 * RETURN:   TRUE/FALSE
 * NOTES:
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_MGR_GetNextFIBID(UI32_T *fib_id);

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
                                           UI32_T type);


/* -------------------------------------------------------------------------
 * FUNCTION NAME : AMTRL3_MGR_DeleteHostRoute
 * -------------------------------------------------------------------------
 * PURPOSE:
 *      Remove IPv4 or IPv6 neighbor entry base on given IpNetToPhsical entry information
 *
 * INPUT:
 *      action_flags    -- AMTRL3_TYPE_FLAGS_IPV4 \
 *                         AMTRL3_TYPE_FLAGS_IPV6
 *      fib_id          --  FIB id
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
BOOL_T  AMTRL3_MGR_DeleteHostRoute(
                                    UI32_T action_flags,
                                    UI32_T fib_id,
                                    AMTRL3_TYPE_InetHostRouteEntry_T *inet_host_entry);

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
                                     AMTRL3_TYPE_InetHostRouteEntry_T *host_entry);

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
                                        AMTRL3_TYPE_InetHostRouteEntry_T *host_entry);
#endif

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_ReplaceExistHostRoute
 * -------------------------------------------------------------------------
 * PURPOSE:  Modify an existing host route, this function will search the host
 *           table, if the IP existed, then update the entry based on the new
 *           information, if the IP is not exist in the table then do nothing.
 * INPUT:    action_flags    --  AMTRL3_TYPE_FLAGS_IPV4 \
 *                               AMTRL3_TYPE_FLAGS_IPV6
 *           fib_id          --  FIB id
 *           host_entry      --  Inet Host Route Entry
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
                                        AMTRL3_TYPE_InetHostRouteEntry_T *host_entry,/*Fill all fields*/
                                        UI32_T type);

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
BOOL_T AMTRL3_MGR_EnableIpForwarding(UI32_T action_flags, UI32_T vr_id);


/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_DisableIpForwarding
 * -------------------------------------------------------------------------
 * PURPOSE:  Disable IPv4/v6 forwarding
 * INPUT:    action_flags    --  AMTRL3_TYPE_FLAGS_IPV4 \
 *                               AMTRL3_TYPE_FLAGS_IPV6
 *           vr_id          --  Virtual Router id
 * OUTPUT:   none.
 * RETURN:   TRUE / FALSE
 * NOTES:    1.Only set the forwarding status with the corresponding flag set.
 *             If only AMTRL3_TYPE_FLAGS_IPV4 was set, only ipv4_forwarding
 *             is checked. If both flags are set, enalbe or disable them at
 *             the same time.
 *           2.Now only support set ipv4 and ipv6 at same time
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_MGR_DisableIpForwarding(UI32_T action_flags, UI32_T fib_id);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_SetSoftwareFwdStatus
 * -------------------------------------------------------------------------
 * PURPOSE: To Disable / Enable Software forwarding status.
 * INPUT:    action_flags    --  AMTRL3_TYPE_FLAGS_IPV4 \
 *                               AMTRL3_TYPE_FLAGS_IPV6
 *           fib_id          --  FIB id
 *           v4_fwd_status   --  AMTRL3_TYPE_SOFTWARE_FORWARDING_ENABLE
 *                               AMTRL3_TYPE_SOFTWARE_FORWARDING_DISABLE
 *           v6_fwd_status   --  AMTRL3_TYPE_SOFTWARE_FORWARDING_ENABLE
 *                               AMTRL3_TYPE_SOFTWARE_FORWARDING_DISABLE
 * OUTPUT:  None.
 * RETURN:  TRUE \ FALSE.
 * NOTES:   Set IPv4 and IPv6 software forwarding separately or at same time
 * -------------------------------------------------------------------------
 */
BOOL_T AMTRL3_MGR_SetSoftwareFwdStatus(
                                        UI32_T action_flags,
                                        UI32_T fib_id,
                                        UI8_T v4_fwd_status,
                                        UI8_T v6_fwd_status);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_GetSoftwareForwardingStatus
 * -------------------------------------------------------------------------
 * PURPOSE: Get Sotware forwarding status.
 * INPUT:    action_flags    --  AMTRL3_TYPE_FLAGS_IPV4 \
 *                               AMTRL3_TYPE_FLAGS_IPV6
 *           fib_id          --  FIB id
 *
 * OUTPUT:   v4_fwd_status   --  AMTRL3_TYPE_SOFTWARE_FORWARDING_ENABLE
 *                               AMTRL3_TYPE_SOFTWARE_FORWARDING_DISABLE
 *           v6_fwd_status   --  AMTRL3_TYPE_SOFTWARE_FORWARDING_ENABLE
 *                               AMTRL3_TYPE_SOFTWARE_FORWARDING_DISABLE
 * RETURN:  TRUE \ FALSE.
 * NOTES:   Get IPv4 and IPv6 software forwarding separately or at same time
 * -------------------------------------------------------------------------
 */
BOOL_T AMTRL3_MGR_GetSoftwareFwdStatus(
                                        UI32_T action_flags,
                                        UI32_T fib_id,
                                        UI8_T *v4_fwd_status,
                                        UI8_T *v6_fwd_status);

/* -------------------------------------------------------------------------
 *  FUNCTION NAME: AMTRL3_MGR_AddVlanMac
 * -------------------------------------------------------------------------
 * PURPOSE:  Add one MAC address that belonging to one VLAN.
 * INPUT:
 *           vlan_mac:       MAC address of vlan interface
 *           vid_ifindex:    VLAN ifindex of this MAC address
 * OUTPUT:   none.
 * RETURN:   TRUE/FALSE
 * NOTES:
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_MGR_AddVlanMac(UI8_T *router_mac, UI32_T vid_ifIndex);


/* -------------------------------------------------------------------------
 *  FUNCTION NAME: AMTRL3_MGR_SetSuperNettingStatus
 * -------------------------------------------------------------------------
 * PURPOSE:  Disable / Enable ipv4 and ipv6 SuperNetting Status
 * INPUT:    action_flags - AMTRL3_TYPE_FLAGS_IPV4 \
 *                          AMTRL3_TYPE_FLAGS_IPV6
 *           fib_id          --  FIB id
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
                                        UI8_T v6_super_netting_status);

/* -------------------------------------------------------------------------
 *  FUNCTION NAME: AMTRL3_MGR_GetSuperNettingStatus
 * -------------------------------------------------------------------------
 * PURPOSE:  Get ipv4 and ipv6 SuperNetting Status
 * INPUT:    action_flags - AMTRL3_TYPE_FLAGS_IPV4 \
 *                          AMTRL3_TYPE_FLAGS_IPV6
 *           fib_id          --  FIB id
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
                                                        UI8_T *v6_super_netting_status);


/* -------------------------------------------------------------------------
 * FUNCTION NAME : AMTRL3_MGR_SetInetCidrRouteEntry
 * -------------------------------------------------------------------------
 * PURPOSE: Set one record to ipv4 or ipv6 net route table
 *          and set configuration to driver
 *
 * INPUT:
 *      action_flags    -- AMTRL3_TYPE_FLAGS_IPV4 \
 *                         AMTRL3_TYPE_FLAGS_IPV6 \
 *                         AMTRL3_TYPE_FLAGS_ECMP \
 *                         AMTRL3_TYPE_FLAGS_WCMP
 *      fib_id          --  FIB id
 *      net_route_entry -- record of forwarding table.
 *                         key :
 *                           inet_cidr_route_dest_type     -- inet address type
 *                           inet_cidr_route_dest          -- destination inet address
 *                           inet_cidr_route_pfxlen        -- prefix length
 *                           inet_cidr_route_policy        -- serve as an additional index which
 *                                                            may delineate between multiple entries to
 *                                                            the same destination.
 *                                                            Default is {0,0}
 *                           inet_cidr_route_next_hop_type -- inet address type of nexthop
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
 *        2. Must set AMTRL3_TYPE_FLAGS_ECMP / WCMP flag when set multipath
 *        route entries.
 *        3. action flag: AMTRL3_TYPE_FLAGS_IPV4/IPV6 must consistent with
 *        address type in net_route_entry structure.
 * -------------------------------------------------------------------------*/
BOOL_T  AMTRL3_MGR_SetInetCidrRouteEntry(
                                      UI32_T action_flags,
                                      UI32_T fib_id,
                                      AMTRL3_TYPE_InetCidrRouteEntry_T  *net_route_entry);

/* -------------------------------------------------------------------------
 * FUNCTION NAME : AMTRL3_MGR_DeleteInetCidrRouteEntry
 * -------------------------------------------------------------------------
 * PURPOSE: Delete specified ipv4 or ipv6 record with the key from database and chip.
 *
 * INPUT:
 *      action_flags    -- AMTRL3_TYPE_FLAGS_IPV4 \
 *                         AMTRL3_TYPE_FLAGS_IPV6 \
 *                         AMTRL3_TYPE_FLAGS_ECMP \
 *                         AMTRL3_TYPE_FLAGS_WCMP
 *      fib_id          --  FIB id
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
 *      1.Must set AMTRL3_TYPE_FLAGS_ECMP / WCMP flag when delete multipath
 *      route entries.
 *      2. action flag: AMTRL3_TYPE_FLAGS_IPV4/IPV6 must consistent with
 *      address type in net_route_entry structure.
 *      3. Only key fields in net_route_entry are used in this function.
 *      Other fields can have random value.
 * -------------------------------------------------------------------------*/
BOOL_T  AMTRL3_MGR_DeleteInetCidrRouteEntry(
                                        UI32_T action_flags,
                                        UI32_T fib_id,
                                        AMTRL3_TYPE_InetCidrRouteEntry_T  *net_route_entry);

/* -------------------------------------------------------------------------
 * FUNCTION NAME : AMTRL3_MGR_GetInetCidrRouteEntry
 * -------------------------------------------------------------------------
 * PURPOSE: Get record with specified key.
 *
 * INPUT:
 *      action_flags    -- AMTRL3_TYPE_FLAGS_IPV4 \
 *                         AMTRL3_TYPE_FLAGS_IPV6  --ECMP/WCMP ??
 *      fib_id          --  FIB id
 *      inet_cidr_route_entry -- record of forwarding table, use key fields to find
 *                               out the matched entry.
 *                           KEY :
 *                           inet_cidr_route_dest          -- destination inet address
 *                           inet_cidr_route_pfxlen        -- prefix length
 *                           inet_cidr_route_policy        -- serve as an additional index which
 *                                                            may delineate between multiple entries to
 *                                                            the same destination.
 *                                                            Default is {0,0}
 *                           inet_cidr_route_next_hop      -- next hop inet address.
 *
 *
 * OUTPUT:
 *      inet_cidr_route_entry -- record of forwarding table.
 *
 * RETURN:
 *      TRUE    - Successfully,
 *      FALSE   - If not available or EOF.
 *
 * NOTES:
 *      1. action flag: AMTRL3_TYPE_FLAGS_IPV4/IPV6 must consistent with
 *         address type in inet_cidr_route_entry structure.
 * -------------------------------------------------------------------------
 */
BOOL_T  AMTRL3_MGR_GetInetCidrRouteEntry(
                                        UI32_T action_flags,
                                        UI32_T fib_id,
                                        AMTRL3_TYPE_InetCidrRouteEntry_T  *inet_cidr_route_entry);

/* -------------------------------------------------------------------------
 * FUNCTION NAME : AMTRL3_MGR_GetNextInetCidrRouteEntry
 * -------------------------------------------------------------------------
 * PURPOSE: Get next record after the specified key.
 *
 * INPUT:
 *      action_flags    -- AMTRL3_TYPE_FLAGS_IPV4 \
 *                         AMTRL3_TYPE_FLAGS_IPV6
 *      fib_id          --  FIB id
 *      inet_cidr_route_entry -- record of forwarding table, use key fields to find
 *                               out the next entry.
 *                           KEY :
 *                           inet_cidr_route_dest          -- destination inet address
 *                           inet_cidr_route_pfxlen        -- prefix length
 *                           inet_cidr_route_policy        -- serve as an additional index which
 *                                                            may delineate between multiple entries to
 *                                                            the same destination.
 *                                                            Default is {0,0}
 *                           inet_cidr_route_next_hop      -- next hop inet address.
 *
 *
 * OUTPUT:
 *      ip_cidr_route_entry -- record of forwarding table.
 *
 * RETURN:
 *      TRUE    - Successfully,
 *      FALSE   - If not available or EOF.
 *
 * NOTES:
 *      1. If only AMTRL3_TYPE_FLAGS_IPV4 is set (address type must be IPV4 now),
 *         only traverse all ipv4 entries.
 *      2. If only AMTRL3_TYPE_FLAGS_IPV6 is set (address type must be IPV6 now)
 *         only traverse all ipv6 entries.
 *      3. If both IPV4 and IPV6 flag are set (address type may be IPV4 or IPV6),
 *         if address type is IPV4, traverse IPV4 entries at first then continue on IPV6 entries.
 *         if address type is IPV6, only traverse IPV6 entries.
 *      4. if keys are all zero, means get first one.
 * -------------------------------------------------------------------------*/
BOOL_T  AMTRL3_MGR_GetNextInetCidrRouteEntry(
                                    UI32_T action_flags,
                                    UI32_T fib_id,
                                    AMTRL3_TYPE_InetCidrRouteEntry_T  *inet_cidr_route_entry);

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
BOOL_T  AMTRL3_MGR_ClearAllNetRoute(UI32_T action_flags, UI32_T fib_id);

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
                                                 UI32_T *multipath_num);

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
BOOL_T AMTRL3_MGR_SetIpNetToPhysicalEntryTimeout(
                                                                UI32_T action_flags,
                                                                UI32_T fib_id,
                                                                UI32_T v4_timeout,
                                                                UI32_T v6_timeout);

/* -------------------------------------------------------------------------
 * FUNCTION NAME : AMTRL3_MGR_DeleteIpNetToPhysicalEntry
 * -------------------------------------------------------------------------
 * PURPOSE:
 *      Remove IPv4 or IPv6 neighbor entry base on given IpNetToPhsical entry information
 *
 * INPUT:
 *      action_flags    -- AMTRL3_TYPE_FLAGS_IPV4 \
 *                         AMTRL3_TYPE_FLAGS_IPV6
 *      fib_id       -- FIB id
 *      ip_net_to_physical_entry -- delete the entry match the key fields in this structure
 *                                  KEY:
 *                                  ip_net_to_physical_net_address_type
 *                                  ip_net_to_physical_net_address
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
BOOL_T  AMTRL3_MGR_DeleteIpNetToPhysicalEntry(
                                    UI32_T action_flags,
                                    UI32_T fib_id,
                                    AMTRL3_TYPE_ipNetToPhysicalEntry_T *ip_net_to_physical_entry);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_GetNextIpNetToPhysicalEntry
 * -------------------------------------------------------------------------
 * PURPOSE:  To get the next record of ipNetToPhysical entry after the specified key
 *
 * INPUT:
 *      action_flags    -- AMTRL3_TYPE_FLAGS_IPV4 \
 *                         AMTRL3_TYPE_FLAGS_IPV6
 *      fib_id       -- FIB id
 *      ip_net_to_physical_entry -- get the entry match the key fields in this structure
 *                                  KEY:
 *                                  ip_net_to_physical_net_address_type
 *                                  ip_net_to_physical_net_address
 *
 * OUTPUT:   ip_net_to_media_entry -- record of ipNetToPhysical table.
 *
 * RETURN:   TRUE - Successfully
 *           FALSE- No more record (EOF) or can't get other record.
 *
 * NOTES:
 *      1. If only AMTRL3_TYPE_FLAGS_IPV4 is set (address type must be IPV4),
 *         only traverse all ipv4 entries.
 *      2. If only AMTRL3_TYPE_FLAGS_IPV6 is set (address type must be IPV6)
 *         only traverse all ipv6 entries.
 *      3. If both IPV4 and IPV6 flag are set (address type may be IPV4 or IPV6),
 *         if address type is IPV4, traverse IPV4 entries at first then continue on IPV6 entries.
 *         if address type is IPV6, only traverse IPV6 entries.
 *      4. if all keys are zero, means get first one.
 * -------------------------------------------------------------------------*/
BOOL_T  AMTRL3_MGR_GetNextIpNetToPhysicalEntry (
                                                    UI32_T action_flags,
                                                    UI32_T fib_id,
                                                    AMTRL3_TYPE_ipNetToPhysicalEntry_T  *ip_net_to_physical_entry);

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
BOOL_T  AMTRL3_MGR_DeleteDynamicNetRoute(L_INET_AddrIp_T ip_address , UI32_T prefix_length);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - AMTRL3_MGR_IsIPStillUsed
 * -------------------------------------------------------------------------
 * FUNCTION: The function is used to check the host entry is still used.
 * INPUT   :
 *       fib_id           -- FIB id
 *       inet_address_type   -- inet address type
 *       inet_address        -- inet address
 *
 * OUTPUT  :
 *       free_time  -- seconds from the last time this entry was used
 *
 * RETURN  : TRUE   -- the IP is in using.
 *           FALSE  -- the IP is expired, can be replaced.
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_MGR_IsIPStillUsed(UI32_T fib_id,
                                         L_INET_AddrIp_T inet_address,
                                         UI32_T *free_time);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_GetInetHostRouteEntry
 * -------------------------------------------------------------------------
 * PURPOSE: Get ipv4 or ipv6 Host Route Entry
 * INPUT:
 *      action_flags    -- AMTRL3_TYPE_FLAGS_IPV4 \
 *                         AMTRL3_TYPE_FLAGS_IPV6
 *      fib_id       -- FIB id
 *      inet_host_route_entry->inet_address --  Destination IPv4/v6 Address (KEY)
 * OUTPUT:  inet_host_route_entry
 * RETURN:  TRUE / FALSE
 * NOTES:
 *      1. If AMTRL3_TYPE_FLAGS_IPV4 is set, only search all ipv4 entries.
 *      2. If AMTRL3_TYPE_FLAGS_IPV6 is set, only search all ipv6 entries.
 *      3. Can not set these 2 flags at same time.
 *         flag must be consistent with inet_host_route_entry->inet_address_type.
 * -------------------------------------------------------------------------
 */
BOOL_T AMTRL3_MGR_GetInetHostRouteEntry(
                                        UI32_T action_flags,
                                        UI32_T fib_id,
                                        AMTRL3_TYPE_InetHostRouteEntry_T *inet_host_route_entry);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_GetNextInetHostRouteEntry
 * -------------------------------------------------------------------------
 * PURPOSE: Get next ipv4 or ipv6 Host Route Entry
 * INPUT:
 *      action_flags    -- AMTRL3_TYPE_FLAGS_IPV4 \
 *                         AMTRL3_TYPE_FLAGS_IPV6
 *      fib_id       -- FIB id
 *      inet_host_route_entry->inet_address --  Destination IPv4/v6 Address (KEY)
 * OUTPUT:  inet_host_route_entry
 * RETURN:  TRUE / FALSE
 * NOTES:
 *      1. If AMTRL3_TYPE_FLAGS_IPV4 is set, only search all ipv4 entries.
 *      2. If AMTRL3_TYPE_FLAGS_IPV6 is set, only search all ipv6 entries.
 *      3. If the inet_host_route_entry->inet_address = 0, get the first entry
 * -------------------------------------------------------------------------
 */
BOOL_T AMTRL3_MGR_GetNextInetHostRouteEntry(
                                        UI32_T action_flags,
                                        UI32_T fib_id,
                                        AMTRL3_TYPE_InetHostRouteEntry_T *inet_host_route_entry);

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
 * NOTES:   Do not support set those 2 flags at same time.
 * -------------------------------------------------------------------------
 */
BOOL_T AMTRL3_MGR_GetNumOfInetHostRouteEntry(
                                        UI32_T action_flags,
                                        UI32_T fib_id,
                                        UI32_T *number_of_ip_host_route_entry);

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
void AMTRL3_MGR_RequestUnresolvedHostEntry(
                                                     UI32_T action_flags,
                                                     UI32_T fib_id);

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
void AMTRL3_MGR_RequestGatewayEntry(
                                                     UI32_T action_flags,
                                                     UI32_T fib_id);

/* -------------------------------------------------------------------------
 *  FUNCTION NAME: AMTRL3_MGR_CreateL3Interface
 * -------------------------------------------------------------------------
 * PURPOSE:  Add one MAC address that belonging to one VLAN.
 * INPUT:    fib_id:     FIB id
 *           router_mac:     MAC address of vlan interface
 *           vid_ifindex:    VLAN ifindex of this MAC address
 * OUTPUT:   none.
 * RETURN:   TRUE/FALSE
 * NOTES:
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_MGR_CreateL3Interface(UI32_T fib_id, UI8_T *router_mac, UI32_T vid_ifIndex);

/* -------------------------------------------------------------------------
 *  FUNCTION NAME: AMTRL3_MGR_DeleteL3Interface
 * -------------------------------------------------------------------------
 * PURPOSE: Remove one MAC address that belonging to one vlan interface.
 * INPUT:   fib_id:     FIB id
 *          router_mac:     MAC address of vlan interface
 *          vid_ifIndex:    VLAN ifIndex of this MAC address
 * OUTPUT:  none
 * RETURN:  TRUE/FALSE
 * NOTES:
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_MGR_DeleteL3Interface(UI32_T fib_id, UI8_T *router_mac, UI32_T vid_ifIndex);

/* -------------------------------------------------------------------------
 *  FUNCTION NAME: AMTRL3_MGR_AddL3Mac
 * -------------------------------------------------------------------------
 * PURPOSE:  Add one MAC address with L3 bit On
 * INPUT:    fib_id:      FIB id
 *           l3_mac:       MAC address of vlan interface
 *           vid_ifindex:    VLAN ifindex of this MAC address
 * OUTPUT:   none.
 * RETURN:   TRUE/FALSE
 * NOTES:
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_MGR_AddL3Mac(UI8_T *l3_mac, UI32_T vid_ifIndex);

/* -------------------------------------------------------------------------
 *  FUNCTION NAME: AMTRL3_MGR_DeleteL3Mac
 * -------------------------------------------------------------------------
 * PURPOSE: Remove one L3 MAC address that belonging to one vlan interface.
 * INPUT:   fib_id:      FIB id
 *          l3_mac:       MAC address of vlan interface
 *          vid_ifIndex:    VLAN ifIndex of this MAC address
 * OUTPUT:  none
 * RETURN:  TRUE/FALSE
 * NOTES:
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_MGR_DeleteL3Mac(UI8_T *l3_mac, UI32_T vid_ifIndex);

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
                                    BOOL_T use_default);

/* -------------------------------------------------------------------------
 *  FUNCTION NAME: AMTRL3_MGR_AddManagementIP
 * -------------------------------------------------------------------------
 * PURPOSE: Add my ip host entry into chip
 * INPUT:   action_flags:        AMTRL3_TYPE_FLAGS_IPV4 / AMTRL3_TYPE_FLAGS_IPV6
 *          fib_id:           FIB id
 *          myip_host_entry:     useful fields in this structure:
 *                                     dst_vid_ifindex
 *                                     dst_inet_addr_type
 *                                     dst_inet_addr
 *                                     dst_mac
 *
 * OUTPUT:  none
 * RETURN:  TRUE/FALSE
 * NOTES:   1. This host entry in chip will used for trapping packets to CPU
 *          2. When add first ip address on an vlan interface, call below
 *             functions in order:
 *             a. AMTRL3_MGR_CreateL3Interface
 *             b. AMTRL3_MGR_AddManagementIP
 *             c. AMTRL3_MGR_AddInetNetRoute for local connected route entry.
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_MGR_AddManagementIP(UI32_T action_flags,
                                                       UI32_T fib_id,
                                                       AMTRL3_TYPE_InetHostRouteEntry_T *myip_host_entry);

/* -------------------------------------------------------------------------
 *  FUNCTION NAME: AMTRL3_MGR_DeleteManagementIP
 * -------------------------------------------------------------------------
 * PURPOSE: Delete my ip host entry from chip
 * INPUT:   action_flags:        AMTRL3_TYPE_FLAGS_IPV4 / AMTRL3_TYPE_FLAGS_IPV6
 *          fib_id:           FIB id
 *          myip_host_entry:     useful fields in this structure:
 *                                     dst_vid_ifindex
 *                                     dst_inet_addr_type
 *                                     dst_inet_addr
 *                                     dst_mac
 *
 * OUTPUT:  none
 * RETURN:  TRUE/FALSE
 * NOTES:   1. This host entry in chip will used for trapping packets to CPU
 *          2. After delete last ip address on an vlan interface, call below
 *             functions in order:
 *             a. AMTRL3_MGR_DeleteInetCidrRouteEntry for local connected route.
 *             b. AMTRL3_MGR_DeleteManagementIP
 *             c. AMTRL3_MGR_DeleteL3Interface
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_MGR_DeleteManagementIP(UI32_T action_flags,
                                                   UI32_T fib_id,
                                                   AMTRL3_TYPE_InetHostRouteEntry_T *myip_host_entry);


/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_SetIpNetToMediaEntryTimeout
 * -------------------------------------------------------------------------
 * PURPOSE:  Set Arp table ageout timer
 * INPUT:    timeout - ageout time in seconds
 * OUTPUT:   none.
 * RETURN:   TRUE / FALSE
 * NOTES:    none.
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_MGR_SetIpNetToMediaEntryTimeout(UI32_T fib_id, UI32_T  timeout);


/*-------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_HandleHotRemoval
 * PURPOSE: Hot swap init function for removal
 * INPUT:   starting_port_ifindex
 *          number_of_port
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:
 *------------------------------------------------------------------------*/
void AMTRL3_MGR_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port);


/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_CompensateNetRouteEntry
 * -------------------------------------------------------------------------
 * PURPOSE:  Processing and Optimizing NetRoute Route into chip
 * INPUT:    fib_id - FIB id .
 * OUTPUT:   none.
 * RETURN:
 * NOTES:    In order to optimize the net route entry into chip.
 * -------------------------------------------------------------------------*/
void AMTRL3_MGR_CompensateNetRouteEntry(UI32_T fib_id);

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
void AMTRL3_MGR_CompensateResolvedNetRouteTable(UI32_T fib_id);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_SetDebugFlag
 * -------------------------------------------------------------------------
 * PURPOSE: To Disable / Enable AMTRL3 Debug Message
 * INPUT:   amtrl3_mgr_debug_flag
 * OUTPUT:  None.
 * RETURN:  None.
 * NOTES:   None.
 * -------------------------------------------------------------------------*/
void AMTRL3_MGR_SetDebugFlag(I32_T debug_flag);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_PrintDebugMode
 * -------------------------------------------------------------------------
 * PURPOSE: To print AMTRL3 Debug Message
 * INPUT:   None.
 * OUTPUT:  None.
 * RETURN:  None.
 * NOTES:   None.
 * -------------------------------------------------------------------------*/
void AMTRL3_MGR_PrintDebugMode(void);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_DisplayDebugCounters
 * -------------------------------------------------------------------------
 * PURPOSE:  Display all amtrl3 counters , debug and statistics
 * INPUT:    None
 * OUTPUT:   None
 * RETURN:   None
 * NOTES:    None
 * -------------------------------------------------------------------------*/
void AMTRL3_MGR_DisplayDebugCounters(UI32_T fib_id);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_ClearDebugCounter
 * -------------------------------------------------------------------------
 * PURPOSE: Clear Amtrl3 debug counter
 * INPUT:   fib_id.
 * OUTPUT:  None.
 * RETURN:  None.
 * NOTES:   None.
 * -------------------------------------------------------------------------*/
void AMTRL3_MGR_ClearDebugCounter(UI32_T fib_id);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_MeasureHostRoutePerformance
 * -------------------------------------------------------------------------
 * PURPOSE: Measure the time it takes to write single host route entry.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETURN:  None.
 * NOTES:   None.
 * -------------------------------------------------------------------------*/
void AMTRL3_MGR_MeasureHostRoutePerformance(void);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_MeasureNetRoutePerformance
 * -------------------------------------------------------------------------
 * PURPOSE: Measure the time it takes to write single net route entry.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETURN:  None.
 * NOTES:   None.
 * -------------------------------------------------------------------------*/
void AMTRL3_MGR_MeasureNetRoutePerformance(void);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_SetArpRequestFeature
 * -------------------------------------------------------------------------
 * PURPOSE: To Disable / Enable AMTRL3 ARP Request Action
 * INPUT:   debug_flag
 * OUTPUT:  None.
 * RETURN:  None.
 * NOTES:   None.
 * -------------------------------------------------------------------------*/
void AMTRL3_MGR_SetArpRequestFeature(I32_T debug_flag);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_SetHostRouteScanningOperation
 * -------------------------------------------------------------------------
 * PURPOSE: To Disable / Enable scan host route hit bit operation
 * INPUT:   debug_flag
 * OUTPUT:  None.
 * RETURN:  None.
 * NOTES:   None.
 * -------------------------------------------------------------------------*/
void AMTRL3_MGR_SetHostRouteScanningOperation(void);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_SetDeleteHostRouteOperation
 * -------------------------------------------------------------------------
 * PURPOSE: To Disable / Enable delete host route operation
 * INPUT:   debug_flag
 * OUTPUT:  None.
 * RETURN:  None.
 * NOTES:   None.
 * -------------------------------------------------------------------------*/
void AMTRL3_MGR_SetDeleteHostRouteOperation(void);

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
const UI8_T * AMTRL3_MGR_Ntoa(const L_INET_AddrIp_T *address, UI8_T *ip_str);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_SetMyIpHostRoute
 * -------------------------------------------------------------------------
 * PURPOSE:  Add one ipv4 or ipv6 my ip my mac host route to
 *           Host Route database and host route table in chip.
 * INPUT:    action_flags    --  AMTRL3_TYPE_FLAGS_IPV4 \
 *                               AMTRL3_TYPE_FLAGS_IPV6
 *           fib_id          --  FIB id
 *           host_entry      --  Inet Host Route Entry
 *                               all structure fileds must be set
 * OUTPUT:   none.
 * RETURN:   TRUE / FALSE
 * NOTES:
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_MGR_SetMyIpHostRoute(UI32_T action_flags,
                                               UI32_T fib_id,
                                               AMTRL3_TYPE_InetHostRouteEntry_T *host_entry);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_MGR_DeleteMyIpHostRoute
 * -------------------------------------------------------------------------
 * PURPOSE:  Delete one ipv4 or ipv6 my ip my mac host route from
 *           Host Route database and host route table in chip.
 * INPUT:    action_flags    --  AMTRL3_TYPE_FLAGS_IPV4 \
 *                               AMTRL3_TYPE_FLAGS_IPV6
 *           fib_id          --  FIB id
 *           host_entry      --  Inet Host Route Entry
 *
 * OUTPUT:   none.
 * RETURN:   TRUE / FALSE
 * NOTES:
 * -------------------------------------------------------------------------*/
BOOL_T  AMTRL3_MGR_DeleteMyIpHostRoute(UI32_T action_flags,
                                                UI32_T fib_id,
                                                AMTRL3_TYPE_InetHostRouteEntry_T *inet_host_entry);



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
 *        2. If route exists, it must set the flag AMTRL3_PMGR_FLAGS_ECMP.
 *        3. If path number overflows the spec, only add those in front.
 * -------------------------------------------------------------------------*/
BOOL_T  AMTRL3_MGR_AddECMPRouteMultiPath(UI32_T action_flags,
                                                  UI32_T fib_id,
                                                  AMTRL3_TYPE_InetCidrRouteEntry_T  *net_route_entry,
                                                  UI32_T num);


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
                                                AMTRL3_TYPE_InetCidrRouteEntry_T  *net_route_entry);


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
                                                      AMTRL3_TYPE_InetCidrRouteEntry_T  *net_route_entry);

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
                                          AMTRL3_TYPE_InetCidrRouteEntry_T  *net_route_entry);
/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTRL3_MGR_MACTableDeleteByMstidOnPort
 *------------------------------------------------------------------------
 * FUNCTION: The function is used to delete port mac on this msit associated vlan
 * INPUT   : lport_ifindex - specific port removed from vlan member list
 *           mstid - specific spaning-tree msti index
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : none
 *------------------------------------------------------------------------*/
BOOL_T AMTRL3_MGR_MACTableDeleteByMstidOnPort(UI32_T mstid,
                                            UI32_T lport_ifindex);

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
void AMTRL3_MGR_SignalL3IfRifDestroy(UI32_T ifindex, L_INET_AddrIp_T *ip_addr_p);

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
BOOL_T AMTRL3_MGR_TimerEventHandler(UI32_T fib_id);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - AMTRL3_MGR_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void AMTRL3_MGR_Create_InterCSC_Relation(void);

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
BOOL_T AMTRL3_MGR_HandleIPCReqMsg(SYSFUN_Msg_T* ipcmsg_p);

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
void AMTRL3_MGR_VlanDestroy_CallBack(UI32_T vid_ifindex, UI32_T vlan_status);


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
void AMTRL3_MGR_IfOperStatusChanged_CallBack(UI32_T vid_ifindex, UI32_T oper_status);


void AMTRL3_MGR_ForwardingUPortAddToTrunk_CallBack(UI32_T  trunk_ifindex,UI32_T member_ifindex);
void AMTRL3_MGR_ForwardingTrunkMemberDelete_CallBack(UI32_T trunk_ifindex, UI32_T member_ifindex);
void AMTRL3_MGR_ForwardingTrunkMemberToNonForwarding_CallBack(UI32_T trunk_ifindex, UI32_T member_ifindex);
void AMTRL3_MGR_PortMove_CallBack(UI32_T num_of_entry, AMTR_TYPE_PortMoveEntry_T *entry_p);
void AMTRL3_MGR_MACTableDeleteByPort_CallBack(UI32_T ifindex, UI32_T reason);
//void AMTRL3_MGR_NewMacWorkAround_CallBack(UI32_T vid, UI8_T *mac, UI32_T l_port, BOOL_T is_age_out);
void AMTRL3_MGR_MacAgingOut_CallBack(UI32_T num, AMTRL3_TYPE_AddrEntry_T addr_buff[]);
void AMTRL3_MGR_MacAddrUpdateCallbackHandler(UI32_T ifindex, UI32_T vid, UI8_T *mac_p, BOOL_T is_add);
void AMTRL3_MGR_MACTableDeleteByVIDnPort_CallBack(UI32_T vid, UI32_T lport_ifindex);

void AMTRL3_MGR_SetEgressObjectFlag(void);
void AMTRL3_MGR_SetECMPTableSameEgressFlag(void);
void AMTRL3_MGR_SetChipFullFlag(void);
void AMTRL3_MGR_SetDefaultRouteTrapToCpuFlag(void);
void AMTRL3_MGR_SetLoaclHostTrapMyIpPktFlag(void);

#if (SYS_CPNT_IP_TUNNEL == TRUE)
/*BOOL_T  AMTRL3_MGR_SetDynamicTunnel(UI32_T  tidIfindex, UI32_T  tunnel_type, L_INET_AddrIp_T* src_addr,  L_INET_AddrIp_T * dest_addr);*/
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
BOOL_T AMTRL3_MGR_DeleteTunnelHostEntries( UI32_T fibid, UI32_T  tidIfindex);
void AMTRL3_MGR_TunnelNetRouteHitBitChange_CallBack(UI32_T fib_id,UI8_T* dst_addr, UI32_T preflen, UI32_T unit_id);
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
BOOL_T AMTRL3_MGR_MacScan(UI32_T fib_id);

#if (SYS_CPNT_VXLAN == TRUE)
BOOL_T AMTRL3_MGR_AddVxlanTunnel(UI32_T fib_id, AMTRL3_TYPE_VxlanTunnelEntry_T* vxlan_tunnel_p);
BOOL_T AMTRL3_MGR_DeleteVxlanTunnel(UI32_T fib_id, AMTRL3_TYPE_VxlanTunnelEntry_T* vxlan_tunnel_p);
BOOL_T AMTRL3_MGR_AddVxlanTunnelNexthop(UI32_T fib_id, AMTRL3_TYPE_VxlanTunnelNexthopEntry_T* tunnel_nexthop_p);
BOOL_T AMTRL3_MGR_DeleteVxlanTunnelNexthop(UI32_T fib_id, AMTRL3_TYPE_VxlanTunnelNexthopEntry_T* tunnel_nexthop_p);
#endif

#endif /*_AMTRL3_MGR_H_*/
