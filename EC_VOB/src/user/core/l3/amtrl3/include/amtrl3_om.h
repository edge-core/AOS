/*
 * MODULE NAME: AMTRL3_OM.h
 *
 * PURPOSE: To store and manage AMTRL3 HostRoute and NetRoute Tables.
 *          It also provides HISAM utility for amtrl3 mgr and task.
 *	        This database replaces the AMTRL3_VM and AMTRL3_ARPOM in
 *          the original
 *
 * NOTES:	This file is redesign base on the original function designed
 *          in AMTRL3_VM.h
 *
 *  History :
 *      Date            Modifier    Reason
 *  ---------------------------------------------------------------------
 *      01-05-2004       amytu     Redesign AMTRL3
 *      06-16-2004                 SuperNetting Feature
 * ----------------------------------------------------------------------
 * Copyright(C)        Accton Techonology Corporation, 2004
 * ======================================================================
 */
#ifndef _AMTRL3_OM_H_
#define _AMTRL3_OM_H_

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_adpt.h"
#include "l_hisam.h"
#include "l_dlist.h"
#include "l_sort_lst.h"
#include "amtrl3_type.h"
#include "swdrvl3.h"



/* NAME CONSTANT DECLARATIONS
 */

/* invalid hardware info */
#define AMTRL3_OM_HW_INFO_INVALID           SWDRVL3_HW_INFO_INVALID

/* Flag manipulation macros. */
#define CHECK_FLAG(V,F)      ((V) & (F))
#define SET_FLAG(V,F)        (V) = (V) | (F)
#define UNSET_FLAG(V,F)      (V) = (V) & ~(F)
#define FLAG_ISSET(V,F)      (((V) & (F)) == (F))

#define AMTRL3_OM_MSGBUF_TYPE_SIZE sizeof(union AMTRL3_OM_IPCMsg_Type_U)
#define AMTRL3_OM_GET_MSG_SIZE(field_name)                       \
            (AMTRL3_OM_MSGBUF_TYPE_SIZE +                        \
            sizeof(((AMTRL3_OM_IPCMsg_T*)0)->data.field_name))

#define AMTRL3_OM_GET_FIRST_FIB_ID          0xffffffff


/* TYPE DEFINITIONS
 */
#if (SYS_CPNT_VXLAN == TRUE)
typedef struct AMTRL3_OM_VxlanTunnelEntry_S
{
    UI32_T          vfi_id;
    L_INET_AddrIp_T local_vtep;
    L_INET_AddrIp_T remote_vtep;
    BOOL_T          is_mc;
    BOOL_T          is_ecmp;
    UI32_T          udp_port;
    UI32_T          bcast_group;
    BOOL_T          in_chip_status;
    UI32_T          uc_vxlan_port;
    UI32_T          mc_vxlan_port;
    UI32_T          uc_binding_cnt;
    void*           uc_hw_info;      /* nhi object from hardware (ecmp or l3 egress) */
    void*           mc_hw_info;
} AMTRL3_OM_VxlanTunnelEntry_T;

typedef struct AMTRL3_OM_VxlanTunnelNexthopEntry_S
{
    UI32_T          vfi_id;
    L_INET_AddrIp_T local_vtep;
    L_INET_AddrIp_T remote_vtep;
    L_INET_AddrIp_T nexthop_addr;
    UI32_T          nexthop_ifindex;
    BOOL_T          is_uc_binding;
    BOOL_T          is_mc_binding;
}  AMTRL3_OM_VxlanTunnelNexthopEntry_T;
#endif

/* structure for the request/response ipc message in csc pmgr and mgr
 */
typedef struct AMTRL3_OM_IPCMsg_S
{
    union AMTRL3_OM_IPCMsg_Type_U
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
            UI32_T fib_id;
            AMTRL3_TYPE_ipNetToPhysicalEntry_T ip_net_to_physical_entry;
        } ip_net_to_physical;

        struct
        {
            UI32_T action_flags;
            UI32_T fib_id;
        } total_host_route_nbr;

#if (SYS_CPNT_VXLAN == TRUE)
        struct
        {
            UI32_T fib_id;
            AMTRL3_TYPE_VxlanTunnelEntry_T tunnel;
        } vxlan_tunnel;
        
        struct
        {
            UI32_T fib_id;
            AMTRL3_OM_VxlanTunnelEntry_T tunnel;
        } vxlan_tunnel_entry;
#endif
    } data; /* contains the supplemntal data for the corresponding cmd */
} AMTRL3_OM_IPCMsg_T;

typedef enum AMTRL3_OM_IPCCMD_E
{
    AMTRL3_OM_IPCCMD_GETIPNETTOPHYSICALENTRY,
    AMTRL3_OM_IPCCMD_GETNEXTIPNETTOPHYSICALENTRY,
    AMTRL3_OM_IPCCMD_GETINETHOSTROUTEENTRY,
    AMTRL3_OM_IPCCMD_GETNEXTINETHOSTROUTEENTRY,
    AMTRL3_OM_IPCCMD_GETTOTALHOSTROUTENUMBER,
#if (SYS_CPNT_VXLAN == TRUE)
    AMTRL3_OM_IPCCMD_GET_VXLAN_TUNNEL_ENTRY_BY_VXLAN_PORT,
    AMTRL3_OM_IPCCMD_GET_VXLAN_TUNNEL_ENTRY,
    AMTRL3_OM_IPCCMD_GET_NEXT_VXLAN_TUNNEL_ENTRY,
#endif
}AMTRL3_OM_IPCCMD_T;

/* Key: vid
 */
typedef struct AMTRL3_OM_Interface_S
{
    /* Lookup Key */
    UI32_T vid;

    /* Route MAC
     */
    UI8_T route_mac[SYS_ADPT_MAC_ADDR_LEN];
}AMTRL3_OM_Interface_T;

/*  Design concept for host route status
 *  HOST_ROUTE_NOT_EXIST       - Initial state of host route status.  This host route
 *                               does not exist in neither chip nor OM.
 *  HOST_ROUTE_UNRESOLVED      - Unresolved state indicates that this entry only exist
 *                               in OM but not in chip because one or more crucial
 *                               information is missing.  Host entry with ref_count > 0,
 *                               which indicates that this is a gateway entry, will
 *                               remain in unresolved_list until its status becomes
 *                               HOST_ROUTE_READY or HOST_ROUTE_READY_NOT_SYN. Host
 *                               entry with ref_count equal to zero will become
 *                               Unreferenced after ArpReq() is generated but not responded.
 *   HOST_ROUTE_UNREFERENCE    - Unreferenced state indicates that crucial information
 *                               for this entry has not been completed and will remain
 *                               unreference until all key elements are completed.
 *   HOST_ROUTE_READY          - Ready state indicates that all key information of this
 *                               host entry has been completed and all net route entry
 *                               associated with this host entry has also been updated.
 *   HOST_ROUTE_READY_NOT_SYNC - This state indicates that crucial information for this
 *                               entry has been completed, but net entry associated with
 *                               this host entry has not been updated accordingly.
 *                               Therefore, it is necessary for this host entry to remain
 *                               in this state until all net route whose next_hop is in the
 *                               same subnet with this host entry are also updated.
 *   HOST_ROUTE_GATEWAY_READY  - Gateway Ready state indicates that all net route entry associated
 *                               with this host entry has been synchronized both in chip and OM.
 *   HOST_ROUTE_ERROR          - Error state indicates that an unexpected error condition occured
 *                               when host entry add / remove from chip.
 *   HOST_ROUTE_UNKNOWN        - Unknown state represents a transition states between two status.
 *                               Host entry is set to unknown state if it is a Unresolved_entry but
 *                               does not exist in unresolved_entry list. The existance of this state
 *                               is to better manage double link list operation.
 *   HOST_ROUTE_LAST_STATE     - The purpose of this state is for AMTRL3_MGR.c to defined Internal
 *                               host route status base on the last index of Host route status.
 */
typedef enum AMTRL3_OM_HostRouteStatus_E
{
    HOST_ROUTE_NOT_EXIST = 0,
    HOST_ROUTE_UNRESOLVED,
    HOST_ROUTE_UNREFERENCE,
    HOST_ROUTE_READY_NOT_SYNC,
    HOST_ROUTE_HOST_READY,
    HOST_ROUTE_GATEWAY_READY,
    HOST_ROUTE_ERROR,
    HOST_ROUTE_UNKNOWN,
    HOST_ROUTE_LAST_STATE
}AMTRL3_OM_HostRouteStatus_T;

/* Design concept for net route status
 *
 * AMTRL3_OM_NET_ROUTE_NOT_EXIST  - Initial state of net route entry. This net entry
 *                                  does not exist in neither chip or OM.
 * AMTRL3_OM_NET_ROUTE_UNRESOLVED - This state indicates that net entry exist in OM,
 *                                  but not in chip because its nhop entry does not
 *                                  exist in chip. Another instance could be that
 *                                  net entry encountered unexpected error while add
 *                                  add net route to chip(exclude Table_FULL event).
 * AMTRL3_OM_NET_ROUTE_RESOLVED   - This state indicates that net entry exist in OM
 *                                  but does not exist in chip because of lack of
 *                                  bucket entry in chip net table.
 * AMTRL3_OM_NET_ROUTE_READY      - This state indicates that net entry exist in both
 *                                  OM and chip.
 */
typedef enum AMTRL3_OM_NetRouteStatus_E
{
    AMTRL3_OM_NET_ROUTE_NOT_EXIST = 0,
    AMTRL3_OM_NET_ROUTE_UNRESOLVED,
    AMTRL3_OM_NET_ROUTE_RESOLVED,
    AMTRL3_OM_NET_ROUTE_READY
}AMTRL3_OM_NetRouteStatus_T;


/* Operational Data Structure used in all AMTRL3 component <NetRoute>
 * Hisam will handle key mapping
 */
typedef struct AMTRL3_OM_NetRouteEntry_S
{
    AMTRL3_TYPE_InetCidrRoutePartialEntry_T      inet_cidr_route_entry;
    AMTRL3_OM_NetRouteStatus_T                 net_route_status;
    UI32_T                                     flags;     /* indicate whether ECMP... */
    L_INET_AddrIp_T  tunnel_nexthop_inet_addr;
    UI32_T           tunnel_hit_timestamp;
    UI32_T           tunnel_hit;               /* record stacking unit's hit in different bits*/
    UI8_T            tunnel_entry_type;
    void*                                      hw_info;   /* egress object from hardware for some chip */
#if (SYS_CPNT_VXLAN == TRUE)
    UI32_T           vxlan_port;
#endif
}AMTRL3_OM_NetRouteEntry_T;

typedef struct AMTRL3_OM_DefaultRouteInfo_S
{
    UI32_T  action_type;            /* Packet will be routed/drop/trap2cpu */
    UI32_T  multipath_counter;      /* ECMP or WCMP path counters */
}AMTRL3_OM_DefaultRouteInfo_T;

/* For debug purpose only
 */
typedef struct AMTRL3_OM_Debug_Counter_S
{
    /* Host route debug counter
     */
    UI32_T      amtrl3_om_ipv4_add_host_route_fail_counter;
    UI32_T      amtrl3_om_ipv4_delete_host_route_fail_counter;
    UI32_T      amtrl3_om_ipv4_host_route_in_chip_counter;

    UI32_T      amtrl3_om_ipv6_add_host_route_fail_counter;
    UI32_T      amtrl3_om_ipv6_delete_host_route_fail_counter;
    UI32_T      amtrl3_om_ipv6_host_route_in_chip_counter;

    /* Net route debug counter
     */
    UI32_T      amtrl3_om_ipv4_net_route_in_chip_counter; /* non-ecmp */
    UI32_T      amtrl3_om_ipv4_ecmp_route_in_chip_counter;
    UI32_T      amtrl3_om_ipv4_ecmp_nh_in_chip_counter;
    UI32_T      amtrl3_om_ipv4_net_route_in_om_counter;  /* non-ecmp */
    UI32_T      amtrl3_om_ipv4_ecmp_route_in_om_counter;
    UI32_T      amtrl3_om_ipv4_ecmp_nh_in_om_counter;
    UI32_T      amtrl3_om_ipv4_add_net_route_fail_counter;

    UI32_T      amtrl3_om_ipv6_net_route_in_chip_counter; /* non-ecmp */
    UI32_T      amtrl3_om_ipv6_ecmp_route_in_chip_counter;
    UI32_T      amtrl3_om_ipv6_ecmp_nh_in_chip_counter;
    UI32_T      amtrl3_om_ipv6_net_route_in_om_counter;   /* non-ecmp */
    UI32_T      amtrl3_om_ipv6_ecmp_route_in_om_counter;
    UI32_T      amtrl3_om_ipv6_ecmp_nh_in_om_counter;
    UI32_T      amtrl3_om_ipv6_add_net_route_fail_counter;
}AMTRL3_OM_Debug_Counter_T;

typedef struct AMTRL3_OM_FIB_S
{
    UI32_T   fib_id;

    L_SORT_LST_List_T *intf_list;

    UI32_T   amtrl3_om_ipv4_total_neighbor_counts;
    UI32_T   amtrl3_om_ipv4_static_neighbor_counts;
    UI32_T   amtrl3_om_ipv4_dynamic_neighbor_counts;
    UI32_T   amtrl3_om_ipv4_host_entry_ageout_time;
    UI32_T   amtrl3_om_ipv4_resolved_net_route_count;
    BOOL_T   amtrl3_om_ipv4_software_forwarding_status;
    UI8_T    amtrl3_om_ipv4_super_netting_status;
    /* The Connected and Static Route have already limited in
     * upper-layer, therefore, in AMTRL3 module only limits the
     * dynamic route. To ensure dynamic routes will never occupy
     * the Connected and Static Route's hardware resources.
     */
    UI32_T   amtrl3_om_ipv4_dynamic_route_in_chip_counts;
    UI32_T   amtrl3_om_ipv4_cidr_route_number;                         /* variable based on updated rfc2096 mib file*/
    UI32_T   amtrl3_om_ipv4_cidr_route_discards;                       /* variable based on updated rfc2096 mib file*/
    AMTRL3_OM_DefaultRouteInfo_T    amtrl3_om_ipv4_default_route_entry;

    UI32_T   amtrl3_om_ipv6_total_neighbor_counts;
    UI32_T   amtrl3_om_ipv6_static_neighbor_counts;
    UI32_T   amtrl3_om_ipv6_dynamic_neighbor_counts;
    UI32_T   amtrl3_om_ipv6_host_entry_ageout_time;
    UI32_T   amtrl3_om_ipv6_resolved_net_route_count;
#if (SYS_CPNT_IP_TUNNEL == TRUE)
    UI32_T   amtrl3_om_ipv6_6to4_tunnel_host_route_count;
    UI32_T   amtrl3_om_ipv6_6to4_tunnel_net_route_count;
#endif
    BOOL_T   amtrl3_om_ipv6_software_forwarding_status;
    UI8_T    amtrl3_om_ipv6_super_netting_status;
    UI32_T   amtrl3_om_ipv6_cidr_route_number;
    UI32_T   amtrl3_om_ipv6_cidr_route_discards;
    AMTRL3_OM_DefaultRouteInfo_T   amtrl3_om_ipv6_default_route_entry;
    AMTRL3_OM_Debug_Counter_T   debug_counter;


    L_HISAM_Desc_T               amtrl3_om_ipv4_net_route_hisam_desc;             /* net route hisam descriptor           */
    L_HISAM_Desc_T               amtrl3_om_ipv4_resolved_net_route_hisam_desc;    /* resolved net route hisam descriptor  */
    L_HISAM_Desc_T               amtrl3_om_ipv4_host_route_hisam_desc;            /* host learn route hisam descriptor    */
    L_DLST_Indexed_Dblist_T      amtrl3_om_ipv4_unresolve_host_entry_list;        /* unresolve host entry list            */
    L_DLST_Indexed_Dblist_T      amtrl3_om_ipv4_resolve_host_entry_list;          /* resolve host entry list              */
    L_DLST_Indexed_Dblist_T      amtrl3_om_ipv4_gateway_entry_list;               /* stores gateway && ready entry        */
    L_INET_AddrIp_T		         amtrl3_om_ipv4_first_unresolved_dip;
    UI32_T                       amtrl3_om_ipv4_first_unresolved_fib;
    L_INET_AddrIp_T		         amtrl3_om_ipv4_first_gateway_dip;
    UI32_T                       amtrl3_om_ipv4_first_gateway_fib;

    L_HISAM_Desc_T               amtrl3_om_ipv6_net_route_hisam_desc;             /* net route hisam descriptor           */
    L_HISAM_Desc_T               amtrl3_om_ipv6_resolved_net_route_hisam_desc;    /* resolved net route hisam descriptor  */
    L_HISAM_Desc_T               amtrl3_om_ipv6_host_route_hisam_desc;            /* host learn route hisam descriptor    */
    L_DLST_Indexed_Dblist_T      amtrl3_om_ipv6_unresolve_host_entry_list;        /* unresolve host entry list            */
    L_DLST_Indexed_Dblist_T      amtrl3_om_ipv6_resolve_host_entry_list;          /* resolve host entry list              */
    L_DLST_Indexed_Dblist_T      amtrl3_om_ipv6_gateway_entry_list;               /* stores gateway && ready entry        */
    L_INET_AddrIp_T		         amtrl3_om_ipv6_first_unresolved_dip;
    UI32_T                       amtrl3_om_ipv6_first_unresolved_fib;
    L_INET_AddrIp_T 		     amtrl3_om_ipv6_first_gateway_dip;
    UI32_T                       amtrl3_om_ipv6_first_gateway_fib;

#if (SYS_CPNT_VXLAN == TRUE)
    L_HISAM_Desc_T               amtrl3_om_vxlan_tunnel_hisam_desc;
    L_HISAM_Desc_T               amtrl3_om_vxlan_tunnel_nexthop_hisam_desc;
#endif

}AMTRL3_OM_FIB_T;

typedef enum AMTRL3_OM_VALUE_TYPE_E
{
    AMTRL3_OM_IPV4_TOTAL_IPNET2PHYSICAL_COUNTS,
    AMTRL3_OM_IPV4_STATIC_IPNET2PHYSICAL_COUNTS,
    AMTRL3_OM_IPV4_DYNAMIC_IPNET2PHYSICAL_COUNTS,
    AMTRL3_OM_IPV4_HOST_ENTRY_AGEOUT_TIME,
    AMTRL3_OM_IPV4_RESOLVED_NET_ROUTE_COUNT,
    AMTRL3_OM_IPV4_SOFTWARE_FORWARDING_STATUS,
    AMTRL3_OM_IPV4_SUPER_NETTING_STATUS,
    AMTRL3_OM_IPV4_DYNAMIC_ROUTE_IN_CHIP_COUNTS,
    AMTRL3_OM_IPV4_CIDR_ROUTE_NUMBER,
    AMTRL3_OM_IPV4_CIDR_ROUTE_DISCARDS,
    AMTRL3_OM_IPV4_DEFAULT_ROUTE_ACTION_TYPE,
    AMTRL3_OM_IPV4_DEFAULT_ROUTE_MULTIPATH_COUNTER,

    AMTRL3_OM_IPV6_TOTAL_IPNET2PHYSICAL_COUNTS,
    AMTRL3_OM_IPV6_STATIC_IPNET2PHYSICAL_COUNTS,
    AMTRL3_OM_IPV6_DYNAMIC_IPNET2PHYSICAL_COUNTS,
    AMTRL3_OM_IPV6_HOST_ENTRY_AGEOUT_TIME,
    AMTRL3_OM_IPV6_RESOLVED_NET_ROUTE_COUNT,
    AMTRL3_OM_IPV6_SOFTWARE_FORWARDING_STATUS,
    AMTRL3_OM_IPV6_SUPER_NETTING_STATUS,
    AMTRL3_OM_IPV6_CIDR_ROUTE_NUMBER,
    AMTRL3_OM_IPV6_CIDR_ROUTE_DISCARDS,
    AMTRL3_OM_IPV6_DEFAULT_ROUTE_ACTION_TYPE,
    AMTRL3_OM_IPV6_DEFAULT_ROUTE_MULTIPATH_COUNTER,


#if(SYS_CPNT_IP_TUNNEL == TRUE)
    AMTRL3_OM_IPV6_6to4_TUNNEL_HOST_ROUTE_COUNT,
    AMTRL3_OM_IPV6_6to4_TUNNEL_NET_ROUTE_COUNT,
#endif


    AMTRL3_OM_DEBUG_COUNTER_IPV4_ADD_HOST_ROUTE_FAIL,
    AMTRL3_OM_DEBUG_COUNTER_IPV4_DEL_HOST_ROUTE_FAIL,
    AMTRL3_OM_DEBUG_COUNTER_IPV4_HOST_ROUTE_IN_CHIP,

    AMTRL3_OM_DEBUG_COUNTER_IPV6_ADD_HOST_ROUTE_FAIL,
    AMTRL3_OM_DEBUG_COUNTER_IPV6_DEL_HOST_ROUTE_FAIL,
    AMTRL3_OM_DEBUG_COUNTER_IPV6_HOST_ROUTE_IN_CHIP,

    AMTRL3_OM_DEBUG_COUNTER_IPV4_NET_ROUTE_IN_CHIP,
    AMTRL3_OM_DEBUG_COUNTER_IPV4_ECMP_ROUTE_IN_CHIP,
    AMTRL3_OM_DEBUG_COUNTER_IPV4_ECMP_NH_IN_CHIP,
    AMTRL3_OM_DEBUG_COUNTER_IPV4_NET_ROUTE_IN_OM,
    AMTRL3_OM_DEBUG_COUNTER_IPV4_ECMP_ROUTE_IN_OM,
    AMTRL3_OM_DEBUG_COUNTER_IPV4_ECMP_NH_IN_OM,
    AMTRL3_OM_DEBUG_COUNTER_IPV4_ADD_NET_ROUTE_FAIL,

    AMTRL3_OM_DEBUG_COUNTER_IPV6_NET_ROUTE_IN_CHIP,
    AMTRL3_OM_DEBUG_COUNTER_IPV6_ECMP_ROUTE_IN_CHIP,
    AMTRL3_OM_DEBUG_COUNTER_IPV6_ECMP_NH_IN_CHIP,
    AMTRL3_OM_DEBUG_COUNTER_IPV6_NET_ROUTE_IN_OM,
    AMTRL3_OM_DEBUG_COUNTER_IPV6_ECMP_ROUTE_IN_OM,
    AMTRL3_OM_DEBUG_COUNTER_IPV6_ECMP_NH_IN_OM,
    AMTRL3_OM_DEBUG_COUNTER_IPV6_ADD_NET_ROUTE_FAIL
}AMTRL3_OM_VALUE_TYPE_T;

typedef struct AMTRL3_OM_HostRouteEntry_S
{
    AMTRL3_TYPE_HostRoutePartialEntry_T key_fields;
    UI32_T      uport;
    UI32_T      entry_type;
    UI32_T      ref_count;
#if (SYS_CPNT_PBR == TRUE)
    UI32_T      pbr_ref_count;
#endif
    UI32_T      ref_count_in_chip;
    UI32_T      ecmp_ref_count;
    UI32_T      arp_interval_index;
    UI32_T      hit_timestamp;
    UI32_T      last_arp_timestamp;
    UI32_T      flags;
#if (SYS_CPNT_IP_TUNNEL == TRUE)
    UI32_T      hw_tunnel_index;
#endif
#if (SYS_CPNT_VXLAN == TRUE)
    UI32_T      vxlan_uc_ref_count;
    UI32_T      vxlan_mc_ref_count;
    void*       vxlan_uc_hw_info;
    void*       vxlan_mc_hw_info;
#endif
    BOOL_T      in_chip_status;

    void*       hw_info;      /* egress object from hardware for some chip */
    AMTRL3_OM_HostRouteStatus_T     old_status;
    AMTRL3_OM_HostRouteStatus_T     status;

}AMTRL3_OM_HostRouteEntry_T;

typedef struct AMTRL3_OM_ResolvedNetRouteEntry_S
{
    UI32_T          fib_id;
    UI32_T          inverse_prefix_length;       /* key */
    L_INET_AddrIp_T inet_cidr_route_dest;        /* key */
    L_INET_AddrIp_T inet_cidr_route_next_hop;    /* key */
    UI32_T          inet_cidr_route_policy[SYS_ADPT_NUMBER_OF_INET_CIDR_ROUTE_POLICY_SUBIDENTIFIER];
}AMTRL3_OM_ResolvedNetRouteEntry_T;

/* MACRO FUNCTION DECLARATIONS
*/


/* DATA TYPE DECLARATIONS
*/


/* EXPORTED SUBPROGRAM SPECIFICATIONS
*/
/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_Initiate_System_Resources
 * -------------------------------------------------------------------------
 * PURPOSE:  Init the OM resouces.
 * INPUT:    none
 * OUTPUT:   none.
 * RETURN:
 * NOTES:
 * -------------------------------------------------------------------------*/
void AMTRL3_OM_Initiate_System_Resources(void);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_SetVid2Fib
 * -------------------------------------------------------------------------
 * PURPOSE:  Set vid2fib table by vid and fib id.
 * INPUT:    fib_id - FIB id (0 ~ 254).
 *           vid    - VLAN ID
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE.
 * NOTES:    None.
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_OM_SetVid2Fib(UI32_T vid, UI32_T fib_id);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_GetFibIdByVid
 * -------------------------------------------------------------------------
 * PURPOSE:  Get fib id by vid.
 * INPUT:    vid
 * OUTPUT:   None.
 * RETURN:   fib id
 * NOTES:    None.
 * -------------------------------------------------------------------------*/
UI32_T AMTRL3_OM_GetFibIdByVid(UI32_T vid);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_CreateFIB
 * -------------------------------------------------------------------------
 * PURPOSE:  This function initialize amtrl3_om for the FIB.
 *           Information contain in this OM that shall be initialized
 *           are host_table, net_table, unresolve_list, resolve_list,
 *           sort_by_port_list.
 * INPUT:    fib_id:   FIB id.
 * OUTPUT:   none.
 * RETURN:   AMTRL3_OM_FIB_ACTION_SUCCESS ,
 *           AMTRL3_OM_FIB_ACTION_FAIL,
 *           AMTRL3_OM_FIB_ACTION_EXCEPTION.
 * NOTES:
 * -------------------------------------------------------------------------*/
UI32_T AMTRL3_OM_CreateFIB(UI32_T fib_id);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_DeleteFIB()
 * -------------------------------------------------------------------------
 * PURPOSE:  Delete one FIB;
 * INPUT:    .
 * OUTPUT:   none.
 * RETURN:   AMTRL3_OM_FIB_ACTION_SUCCESS ,
 *           AMTRL3_OM_FIB_ACTION_FAIL,
 *           AMTRL3_OM_FIB_ACTION_EXCEPTION.
 * NOTES:
 * -------------------------------------------------------------------------*/
UI32_T AMTRL3_OM_DeleteFIB(UI32_T fib_id);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_ClearFIB()
 * -------------------------------------------------------------------------
 * PURPOSE:  Clear one FIB
 * INPUT:    fib_id
 * OUTPUT:   none
 * RETURN:   AMTRL3_OM_FIB_ACTION_SUCCESS ,
 *           AMTRL3_OM_FIB_ACTION_FAIL,
 *           AMTRL3_OM_FIB_ACTION_EXCEPTION.
 * NOTES:
 * -------------------------------------------------------------------------*/
UI32_T AMTRL3_OM_ClearFIB(UI32_T fib_id);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_GetNextFIBID
 * -------------------------------------------------------------------------
 * PURPOSE:  Get next FIB id(0 ~ 254).
 * INPUT:    fib_id - AMTRL3_OM_GET_FIRST_FIB_ID to get first
 * OUTPUT:   fib_id
 * RETURN:   TRUE/FALSE
 * NOTES:
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_OM_GetNextFIBID(UI32_T *fib_id);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_GetDatabaseValue
 * -------------------------------------------------------------------------
 * PURPOSE:  Get config info or statistics in this FIB .
 * INPUT:    none.
 * OUTPUT:   none.
 * RETURN:   none.
 * NOTES:    MUST confirm the fib exists before calling this function.
 * -------------------------------------------------------------------------*/
UI32_T AMTRL3_OM_GetDatabaseValue(UI32_T fib_id, UI32_T type);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_SetDatabaseValue
 * -------------------------------------------------------------------------
 * PURPOSE:  Set the value of config info or statistics in this FIB .
 * INPUT:    none.
 * OUTPUT:   none.
 * RETURN:   none.
 * NOTES:    MUST confirm the fib exists before calling this function.
 * -------------------------------------------------------------------------*/
void AMTRL3_OM_SetDatabaseValue(UI32_T fib_id, UI32_T type, UI32_T value);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_GetDatabaseValue
 * -------------------------------------------------------------------------
 * PURPOSE:  Increase the statistics in this FIB .
 * INPUT:    none.
 * OUTPUT:   none.
 * RETURN:   none.
 * NOTES:    MUST confirm the fib exists before calling this function.
 * -------------------------------------------------------------------------*/
void AMTRL3_OM_IncreaseDatabaseValue(UI32_T fib_id, UI32_T type, UI32_T step);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_DecreaseDatabaseValue
 * -------------------------------------------------------------------------
 * PURPOSE:  Decrease the statistics in this FIB .
 * INPUT:    none.
 * OUTPUT:   none.
 * RETURN:   none.
 * NOTES:    MUST confirm the fib exists before calling this function.
 * -------------------------------------------------------------------------*/
void AMTRL3_OM_DecreaseDatabaseValue(UI32_T fib_id, UI32_T type, UI32_T step);


/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_SetNetRouteEntry
 * -------------------------------------------------------------------------
 * PURPOSE: Set one route entry.
 * INPUT:
 *        action_flags:       AMTRL3_TYPE_FLAGS_IPV4 -- set ipv4 entry
 *                            AMTRL3_TYPE_FLAGS_IPV6 -- set ipv6 entry
 *        fib_id:             FIB ID.
 *        net_route_entry:    net route need be saved into OM database
 *                            KEY :
 *                            inet_cidr_route_dest          -- destination inet address
 *                            inet_cidr_route_pfxlen        -- prefix length
 *                            inet_cidr_route_policy        -- serve as an additional index which
 *                                                             may delineate between multiple entries to
 *                                                             the same destination.
 *                                                             Default is {0,0}
 *                            inet_cidr_route_next_hop      -- next hop inet address.
 * OUTPUT:  none.
 * RETURN:  TRUE  -- successfully insert to database.
 *          FALSE -- Cannot insert this entry to database
 * NOTES:  1. This function supports both create and set operation of net route
 *            entry.  A new entry will be created if net entry does not exist,
 *            otherwise, replace it.
 *         2. ipv4 and ipv6 cannot set at the same time
 *         3. action flag: AMTRL3_TYPE_FLAGS_IPV4/IPV6 must consistent with
 *            address type in net_route_entry structure
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_OM_SetNetRouteEntry(UI32_T action_flags,
                                  UI32_T fib_id,
                                  AMTRL3_OM_NetRouteEntry_T *net_route_entry);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_DeleteNetRouteEntry
 * -------------------------------------------------------------------------
 * PURPOSE:  Delete one route entry.
 * INPUT:
 *        action_flags:       AMTRL3_TYPE_FLAGS_IPV4 -- delete ipv4 entry
 *                            AMTRL3_TYPE_FLAGS_IPV6 -- delete ipv6 entry
 *        fib_id:             FIB ID.
 *        net_route_entry:    net route structure saved key values
 *                            KEY :
 *                            inet_cidr_route_dest          -- destination inet address
 *                            inet_cidr_route_pfxlen        -- prefix length
 *                            inet_cidr_route_policy        -- serve as an additional index which
 *                                                             may delineate between multiple entries to
 *                                                             the same destination.
 *                                                             Default is {0,0}
 *                            inet_cidr_route_next_hop      -- next hop inet address.
 * OUTPUT:   none.
 * RETURN:   TRUE                    -- Successfully remove from database
 *           FALSE                   -- This entry does not exist in chip/database.
 *                                   -- or cannot Delete this entry
 * NOTES:   1. ipv4 and ipv6 cannot set at the same time
 *          2. action flag: AMTRL3_TYPE_FLAGS_IPV4/IPV6 must consistent with
 *             address type in net_route_entry structure
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_OM_DeleteNetRouteEntry(UI32_T action_flags,
                                     UI32_T fib_id,
                                     AMTRL3_OM_NetRouteEntry_T *net_route_entry);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_GetNetRouteEntry
 * -------------------------------------------------------------------------
 * PURPOSE: get the specified cidr route entry
 * INPUT:
 *        action_flags:       AMTRL3_TYPE_FLAGS_IPV4 -- get ipv4 entry
 *                            AMTRL3_TYPE_FLAGS_IPV6 -- get ipv6 entry
 *        fib_id:             FIB ID.
 *        net_route_entry:   net route structure saved key value
 *                            KEY :
 *                            inet_cidr_route_dest          -- destination inet address
 *                            inet_cidr_route_pfxlen        -- prefix length
 *                            inet_cidr_route_policy        -- serve as an additional index which
 *                                                             may delineate between multiple entries to
 *                                                             the same destination.
 *                                                             Default is {0,0}
 *                            inet_cidr_route_next_hop      -- next hop inet address.
 *
 * OUTPUT:   net_route_entry -- record of forwarding table.
 * RETURN:   TRUE    - Successfully,
 *           FALSE   - If not available or EOF.
 * NOTES:   1. ipv4 and ipv6 cannot set at the same time
 *          2. action flag: AMTRL3_TYPE_FLAGS_IPV4/IPV6 must consistent with
 *             address type in net_route_entry structure
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_OM_GetNetRouteEntry(UI32_T action_flags,
                                  UI32_T fib_id,
                                  AMTRL3_OM_NetRouteEntry_T *net_route_entry);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_GetNextNetRouteEntry
 * -------------------------------------------------------------------------
 * PURPOSE: To get the next record of cidr_route entry after the specified key
 * INPUT:
 *        action_flags:       AMTRL3_TYPE_FLAGS_IPV4 -- get ipv4 entry
 *                            AMTRL3_TYPE_FLAGS_IPV6 -- get ipv6 entry
 *        fib_id:             FIB ID.
 *        net_route_entry:    net route need be saved into OM database
 *                            Primary KEY :
 *                            inet_cidr_route_dest          -- destination inet address
 *                            inet_cidr_route_pfxlen        -- prefix length
 *                            inet_cidr_route_policy        -- serve as an additional index which
 *                                                             may delineate between multiple entries to
 *                                                             the same destination.
 *                                                             Default is {0,0}
 *                            inet_cidr_route_next_hop      -- next hop inet address.
 *
 * OUTPUT:  net_route_entry -- record of next forwarding table.
 * RETURN:   TRUE    - Successfully,
 *           FALSE   - If not available or EOF.
 * NOTES:   1. if all keys are zero, means get first one.
 *          2. action_flags: if v4\v6 is set, only search in v4\v6 entry, but
 *                           it must consistent with address type in net_route_entry.
 *                           if both are set, and the address type is v4, search in
 *                           v4, then v6; if address type is v6, just search in v6.
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_OM_GetNextNetRouteEntry(UI32_T action_flags,
                                      UI32_T fib_id,
                                      AMTRL3_OM_NetRouteEntry_T *net_route_entry);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_GetNextNNetRouteEntryByNextHop
 * -------------------------------------------------------------------------
 * PURPOSE:  Get next N entries of net routes by next-hop.
 * INPUT:
 *        action_flags:       AMTRL3_TYPE_FLAGS_IPV4 -- get ipv4 entry
 *                            AMTRL3_TYPE_FLAGS_IPV6 -- get ipv6 entry
 *        fib_id:             FIB ID.
 *        num_of_entry_requested -- the number of entry to get at once
 *        net_route_entry_block  -- a specific gateway ip to be compared with
 *                                  KEY:
 *                                  inet_cidr_route_next_hop      -- next hop inet address.
 *                                  Other fields shall be zero.
 * OUTPUT:   num_of_entry_retrieved - the number of net route entry actually retrieved
 *           net_route_entry_block  - a memory block to store specific number of
 *                                    matching net route entries.
 * RETURN:   TRUE  -- Found available entries.
 *           FALSE -- can't find any net route
 * NOTES:    1. Buffer size for net_route_entry_block must be sufficient to store
 *           the number of net entries requested as indicated by the third input
 *           parameter.
 *           2. Only ipv4 or ipv6 flag shall be set.
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_OM_GetNextNNetRouteEntryByNextHop(
                                            UI32_T action_flags,
                                            UI32_T fib_id,
                                            UI32_T  num_of_entry_requested,
                                            UI32_T  *num_of_entry_retrieved,
                                            AMTRL3_OM_NetRouteEntry_T  *net_route_entry_block);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_GetNextNNetRouteEntryByIfIndex
 * -------------------------------------------------------------------------
 * PURPOSE:  Get next N entries of net routes by ifindex
 * INPUT:
 *        action_flags:       AMTRL3_TYPE_FLAGS_IPV4 -- get ipv4 entry
 *                            AMTRL3_TYPE_FLAGS_IPV6 -- get ipv6 entry
 *        fib_id:             FIB ID.
 *        num_of_entry_requested -- the number of entry to get at once
 *        net_route_entry_block  -- a specific gateway ip to be compared with
 *                                  KEY:
 *                                  inet_cidr_route_if_index      -- route ifindex.
 *                                  Other fields shall be zero.
 * OUTPUT:   num_of_entry_retrieved - the number of net route entry actually retrieved
 *           net_route_entry_block  - a memory block to store specific number of
 *                                    matching net route entries.
 * RETURN:   TRUE  -- Found available entries.
 *           FALSE -- can't find any net route
 * NOTES:    1. Buffer size for net_route_entry_block must be sufficient to store
 *           the number of net entries requested as indicated by the third input
 *           parameter.
 *           2. Only ipv4 or ipv6 flag shall be set.
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_OM_GetNextNNetRouteEntryByIfIndex(
                                            UI32_T action_flags,
                                            UI32_T fib_id,
                                            UI32_T  num_of_entry_requested,
                                            UI32_T  *num_of_entry_retrieved,
                                            AMTRL3_OM_NetRouteEntry_T  *net_route_entry_block);

#if 0
/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_GetNextNNetRouteEntryByStatus
 * -------------------------------------------------------------------------
 * PURPOSE:  Get next N entries of net routes in the specific net route status
 * INPUT:
 *        action_flags:       AMTRL3_TYPE_FLAGS_IPV4 -- get ipv4 entry
 *                            AMTRL3_TYPE_FLAGS_IPV6 -- get ipv6 entry
 *        fib_id:             FIB ID.
 *        num_of_entry_requested -- the number of entry to get at once
 *        net_status      --  AMTRL3_OM_NET_ROUTE_UNRESOLVED \ AMTRL3_OM_NET_ROUTE_RESOLVED\
 *                            AMTRL3_OM_NET_ROUTE_READY
 *        net_route_entry_block  -- a specific gateway ip to be compared with
 *                                  KEY:
 *                                  inet_cidr_route_dest          -- destination inet address
 *                                  inet_cidr_route_pfxlen        -- prefix length
 *                                  inet_cidr_route_policy        -- serve as an additional index which
 *                                  inet_cidr_route_next_hop      -- next hop inet address.
 * OUTPUT:   num_of_entry_retrieved - the number of net route entry actually retrieved
 *           net_route_entry_block - a memory block to store specific number of
 *                                   matching net route entries.
 * RETURN:   TRUE  -- Found available entries.
 *           FALSE -- can't find any net route
 * NOTES:    1. Buffer size for net_route_entry_block must be sufficient to store
 *           the number of net entries requested as indicated by the third input
 *           parameter.
 *           2. If all key fields are 0, search request entry from beginning.
 *           3. If key fields are not 0, search entries behind specified entry by this key.
 *           4. action_flags: if v4 and v6 are both set, get in v4 entry first, if num_of_entry_retrieved
 *                            is less than num_of_entry_requested, then get the rest in v6 entry .
 * -------------------------------------------------------------------------
 */
BOOL_T AMTRL3_OM_GetNextNNetRouteEntryByStatus(
                                               UI32_T action_flags,
                                               UI32_T fib_id,
                                               UI32_T  num_of_entry_requested,
                                               AMTRL3_OM_NetRouteStatus_T   net_status,
                                               UI32_T  *num_of_entry_retrieved,
                                               AMTRL3_OM_NetRouteEntry_T  *net_route_entry_block);
#endif

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_GetNextNNetRouteEntryByDstIp
 * -------------------------------------------------------------------------
 * PURPOSE:  Get next N entries of net routes by dest ip.
 * INPUT:
 *        action_flags:       AMTRL3_TYPE_FLAGS_IPV4 -- get ipv4 entry
 *                            AMTRL3_TYPE_FLAGS_IPV6 -- get ipv6 entry
 *        fib_id       -- FIB id
 *        num_of_entry_requested -- the number of entry to get at once
 *        net_route_entry_block  -- a specific gateway ip to be compared with
 *                                  KEY:
 *                                  inet_cidr_route_next_hop_type -- inet address type of nexthop
 *                                  inet_cidr_route_next_hop      -- next hop inet address.
 *                                  Other fields shall be zero.
 * OUTPUT:   num_of_entry_retrieved - the number of net route entry actually retrieved
 *           net_route_entry_block  - a memory block to store specific number of
 *                                    matching net route entries.
 * RETURN:   TRUE  -- Found available entries.
 *           FALSE -- can't find any net route
 * NOTES:    1. Buffer size for net_route_entry_block must be sufficient to store
 *           the number of net entries requested as indicated by the third input
 *           parameter.
 *           2. Only ipv4 or ipv6 flag shall be set.
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_OM_GetNextNNetRouteEntryByDstIp(
                                            UI32_T action_flags,
                                            UI32_T fib_id,
                                            UI32_T  num_of_entry_requested,
                                            UI32_T  *num_of_entry_retrieved,
                                            AMTRL3_OM_NetRouteEntry_T  *net_route_entry_block);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_SetHostRouteEntry
 * -------------------------------------------------------------------------
 * PURPOSE: This function sets given host route entry to OM and adjust
 *          double link list location accordingly.
 * INPUT:
 *        action_flags:       AMTRL3_TYPE_FLAGS_IPV4 -- set ipv4 entry
 *                            AMTRL3_TYPE_FLAGS_IPV6 -- set ipv6 entry
 *        fib_id:             FIB ID.
 *        host_route_entry  -- host route need be saved in OM database
 *                             Primary KEY:
 *                             dst_vid_ifindex:    interface's ifindex
 *                             dst_inet_addr:      inet address
 * OUTPUT:  none.
 * RETURN:  TRUE \ FALSE
 * NOTES:   1. ipv4 and ipv6 cannot set at the same time
 *          2. action flag: AMTRL3_TYPE_FLAGS_IPV4/IPV6 must consistent with
 *              address type in host_route_entry structure
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_OM_SetHostRouteEntry(UI32_T action_flags,
                                   UI32_T fib_id,
                                   AMTRL3_OM_HostRouteEntry_T *host_route_entry);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_DeleteHostRouteEntry
 * -------------------------------------------------------------------------
 * PURPOSE:  Delete one route entry by destination inet addresss.
 * INPUT:
 *        action_flags:       AMTRL3_TYPE_FLAGS_IPV4 -- delete ipv4 entry
 *                            AMTRL3_TYPE_FLAGS_IPV6 -- delete ipv6 entry
 *        fib_id:             FIB ID.
 *        host_route_entry  -- target host route to delete
 *                             Primary KEY:
 *                             dst_vid_ifindex:    interface's ifindex
 *                             dst_inet_addr:      inet address
 * OUTPUT:   none.
 * RETURN:   TRUE           -- Successfully remove from database
 *           FALSE          -- This entry does not exist in chip/database.
 *                          -- or cannot Delete this entry
 * NOTES:   1. action_flags: ipv4 and ipv6 cannot be set at the same time
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_OM_DeleteHostRouteEntry(UI32_T action_flags,
                                      UI32_T fib_id,
                                      AMTRL3_OM_HostRouteEntry_T *host_route_entry);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_GetHostRouteEntry
 * -------------------------------------------------------------------------
 * PURPOSE: get the specified host route entry
 * INPUT:
 *        action_flags:       AMTRL3_TYPE_FLAGS_IPV4 -- get ipv4 entry
 *                            AMTRL3_TYPE_FLAGS_IPV6 -- get ipv6 entry
 *        fib_id:             FIB ID.
 *        host_route_entry  -- host route structure saved key value
 *                             KEY:
 *                             dst_inet_addr:      inet address
 *
 * OUTPUT:   host_route_entry -- record of host route entry.
 * RETURN:   TRUE    - Successfully,
 *           FALSE   - If not available or EOF.
 * NOTES:   1. ipv4 and ipv6 cannot set at the same time
 *          2. action flag: AMTRL3_TYPE_FLAGS_IPV4/IPV6 must consistent with
 *             address type in net_route_entry structure
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_OM_GetHostRouteEntry(UI32_T action_flags,
                                   UI32_T fib_id,
                                   AMTRL3_OM_HostRouteEntry_T *host_route_entry);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_GetNextHostRouteEntry
 * -------------------------------------------------------------------------
 * PURPOSE: To get the next record of host entry after the specified key
 * INPUT:
 *        action_flags:       AMTRL3_TYPE_FLAGS_IPV4 -- get ipv4 entry
 *                            AMTRL3_TYPE_FLAGS_IPV6 -- get ipv6 entry
 *        fib_id:             FIB ID.
 *        host_route_entry  -- host route structure saved key value
 *                             KEY:
 *                             dst_inet_addr:      inet address
 * OUTPUT:  host_route_entry -- record of host route entry.
 * RETURN:   TRUE    - Successfully,
 *           FALSE   - If not available or EOF.
 * NOTES:   1. Key is dst_inet_addr in host_route_entry
 *          2. if key is (0,0), means get first one.
 *          3. action_flags: if v4\v6 is set, only search in v4\v6 entry, but
 *                           it must consistent with address type in host_route_entry.
 *                           if both are set, and the address type is v4, search in
 *                           v4, then v6; if address type is v6, just search in v6.
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_OM_GetNextHostRouteEntry(UI32_T action_flags,
                                       UI32_T fib_id,
                                       AMTRL3_OM_HostRouteEntry_T *host_route_entry);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_GetNextHostRouteEntryByVlanMac
 * -------------------------------------------------------------------------
 * PURPOSE: get the specified host route entry by vid, inet addr and MAC addr
 * INPUT:
 *        action_flags:       AMTRL3_TYPE_FLAGS_IPV4 -- get ipv4 entry
 *                            AMTRL3_TYPE_FLAGS_IPV6 -- get ipv6 entry
 *        fib_id:             FIB ID.
 *        host_route_entry  -- host route structure saved key value
 *                             KEY:
 *                             dst_vid_ifindex:    vlan ifindex
 *                             dst_mac:            destination mac addrs
 *                             dst_inet_addr:      inet address
 * OUTPUT:  host_route_entry -- record of host route entry.
 * RETURN:  TRUE    - Successfully,
 *          FALSE   - If not available or EOF.
 * NOTES:   1. If all keys equal to zero, means get the first record
 *          2. action_flags: if v4 and v6 are both set, search in v4 entry,
 *                           if not found, then search in v6 entry.
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_OM_GetNextHostRouteEntryByVlanMac(
                                                UI32_T action_flags,
                                                UI32_T fib_id,
                                                AMTRL3_OM_HostRouteEntry_T *host_route_entry);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_GetNextHostRouteEntryByLport
 * -------------------------------------------------------------------------
 * PURPOSE: To get the next record of host route entry after the specified key
 * INPUT:
 *        action_flags:       AMTRL3_TYPE_FLAGS_IPV4 -- get ipv4 entry
 *                            AMTRL3_TYPE_FLAGS_IPV6 -- get ipv6 entry
 *        fib_id:             FIB ID.
 *        host_route_entry --  host route with key value
 *                             KEY:
 *                             lport: logical port index
 *                             dst_inet_addr:      inet address
 * OUTPUT:   host_route_entry -- record of next host route entry.
 * RETURN:   TRUE    - Successfully,
 *           FALSE   - If not available or EOF.
 * NOTES:   1. Key is (lport, dst_inet_addr_type, dst_inet_addr)
 *             in AMTRL3_OM_HostRouteEntry_T.
 *          2. if key is (0,0,0), means get first one.
 *          3. action_flags: if v4 and v6 are both set, search in v4 entry,
 *                           if not found, then search in v6 entry.
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_OM_GetNextHostRouteEntryByLport(
                                              UI32_T action_flags,
                                              UI32_T fib_id,
                                              AMTRL3_OM_HostRouteEntry_T *host_route_entry);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_GetNextNHostRouteEntry
 * -------------------------------------------------------------------------
 * PURPOSE: To get the next N record of host entry after the specified key
 * INPUT:
 *        action_flags:       AMTRL3_TYPE_FLAGS_IPV4 -- get ipv4 entry
 *                            AMTRL3_TYPE_FLAGS_IPV6 -- get ipv6 entry
 *        fib_id:             FIB ID.
 *        num_of_entry_requested  -- the number of entry to get at once
 *        host_route_entry_block  -- save a specific inet addr to be compared with
 *                                   KEY:
 *                                   dst_inet_addr:      inet address
 * OUTPUT:
 *        num_of_entry_retrieved - the number of host route entry actually retrieved
 *        host_route_entry_block - a memory block to store specific number of
 *                                 matching net route entries.
 * RETURN:   TRUE  -- Found available entries.
 *           FALSE -- can't find any host route
 * NOTES:    1. Buffer size for host_route_entry_block must be sufficient to store
 *              the number of host entries requested as indicated by the third input
 *              parameter.
 *           2. action_flags: if v4 and v6 are both set, get in v4 entry first, if num_of_entry_retrieved
 *                            is less than num_of_entry_requested, then get the rest in v6 entry.
 * ---------------------------------------------------------------------------------------*/
BOOL_T AMTRL3_OM_GetNextNHostRouteEntry(UI32_T action_flags,
                                        UI32_T fib_id,
                                        UI32_T num_of_entry_requested,
                                        UI32_T *num_of_entry_retrieved,
                                        AMTRL3_OM_HostRouteEntry_T  *host_route_entry_block);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_ClearHostRouteDatabase
 * -------------------------------------------------------------------------
 * PURPOSE:  Delete all ipv4 and ipv6 host route entries.
 * INPUT:
 *        fib_id:             FIB ID.
 * OUTPUT:   none.
 * RETURN:   none.
 * NOTES:   1. If chip not support batch processing, should use one by one
 *              in this function using compile control to handle it.
 *          2. if any reason can't delete all, must work around in driver layer as possible.
 * -------------------------------------------------------------------------*/
void AMTRL3_OM_ClearHostRouteDatabase(UI32_T fib_id);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_GetTotalHostRouteNumber
 * -------------------------------------------------------------------------
 * PURPOSE:  To get the total number of host route entries include ipv4 and ipv6
 * INPUT:
 *        action_flags:   AMTRL3_TYPE_FLAGS_IPV4 -- include ipv4 entries
 *                        AMTRL3_TYPE_FLAGS_IPV6 -- include ipv6 entries
 *        fib_id      :   FIB id
 * OUTPUT:
 * RETURN: total number of host route number which satisfying action_flags
 * NOTES:    none
 * -------------------------------------------------------------------------*/
UI32_T AMTRL3_OM_GetTotalHostRouteNumber(UI32_T action_flags, UI32_T fib_id);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_SetResolvedNetEntry
 * -------------------------------------------------------------------------
 * PURPOSE: Set net entry in Resolved state to a Resolved HISAM table
 * INPUT:
 *        action_flags:       AMTRL3_TYPE_FLAGS_IPV4 -- set ipv4 entry
 *                            AMTRL3_TYPE_FLAGS_IPV6 -- set ipv6 entry
 *        fib_id:             FIB ID.
 *        net_route_entry -- record of AMTRL3_OM_ResolvedNetRouteEntry_T
 *          key :
 *             ip_cidr_route_dest       - subnet ip address.
 *             inverse_prefix_length    - 32 minus prefix length for subnet mask
 * OUTPUT:  none.
 * RETURN:  TRUE  -- successfully insert to database.
 *          FALSE -- Cannot insert this entry to database
 * NOTES:   1. This function supports both create and set operation of net route
 *             entry.  A new entry will be created if net entry does not exist,
 *             otherwise, replace it.
 *          2. action_flags: ipv4 and ipv6 cannot set at the same time
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_OM_SetResolvedNetEntry(UI32_T action_flags, UI32_T fib_id, AMTRL3_OM_ResolvedNetRouteEntry_T *net_route_entry);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_DeleteResolvedNetEntry
 * -------------------------------------------------------------------------
 * PURPOSE:  Delete net route entry from Resolved HISAM table
 * INPUT:
 *        action_flags:       AMTRL3_TYPE_FLAGS_IPV4 -- delete ipv4 entry
 *                            AMTRL3_TYPE_FLAGS_IPV6 -- delete ipv6 entry
 *        fib_id:             FIB ID.
 *        net_route_entry:    the resolved net entry to be deleted
 * OUTPUT:   none.
 * RETURN:   TRUE                   -- Successfully remove from database
 *           FALSE                  -- This entry does not exist in chip/database.
 *                                  -- or cannot Delete this entry
 * NOTES:  action_flags: ipv4 and ipv6 cannot set at the same time
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_OM_DeleteResolvedNetEntry(UI32_T action_flags, UI32_T fib_id, AMTRL3_OM_ResolvedNetRouteEntry_T  *net_route_entry);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_GetResolvedNetRouteEntry
 * -------------------------------------------------------------------------
 * PURPOSE: get the specified cidr route entry
 * INPUT:
 *        action_flags:       AMTRL3_TYPE_FLAGS_IPV4 -- get ipv4 entry
 *                            AMTRL3_TYPE_FLAGS_IPV6 -- get ipv6 entry
 *        fib_id:             FIB ID.
 *
 * OUTPUT:   net_route_entry -- record of Resolved Net entry
 * RETURN:   TRUE    - Successfully,
 *           FALSE   - If not available or EOF.
 * NOTES:   1. Key is (ip_cidr_route_dest, ip_cidr_route_mask) in
 *             AMTRL3_OM_ResolvedNetRouteEntry_T.
 *          2. action_flags: ipv4 and ipv6 cannot set at the same time
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_OM_GetResolvedNetRouteEntry(UI32_T action_flags, UI32_T fib_id, AMTRL3_OM_ResolvedNetRouteEntry_T *net_route_entry);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_GetNextResolvedNetRouteEntry
 * -------------------------------------------------------------------------
 * PURPOSE: To get the next record of Resolved net entry after the specified key
 * INPUT:
 *        action_flags:       AMTRL3_TYPE_FLAGS_IPV4 -- get ipv4 entry
 *                            AMTRL3_TYPE_FLAGS_IPV6 -- get ipv6 entry
 *        fib_id:             FIB ID.
 *        net_route_entry:    include search key: inverse prefix length, dst address type,
 *                             dst address, nexthop_type, next hop address
 * OUTPUT:  net_route_entry  -- record of next Resolved Net Entry
 * RETURN:   TRUE    - Successfully,
 *           FALSE   - If not available or EOF.
 * NOTES:   1. Key is (ip_cidr_route_dest, ip_cidr_route_mask) in
 *             AMTRL3_OM_ResolvedNetRouteEntry_T.
 *          2. if key is (0,0), means get first one.
 *          3. action_flags can only be v4 or v6
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_OM_GetNextResolvedNetRouteEntry(UI32_T action_flags,
                                              UI32_T fib_id,
                                              AMTRL3_OM_ResolvedNetRouteEntry_T *net_route_entry);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_ClearResolvedNetRouteDatabase
 * -------------------------------------------------------------------------
 * PURPOSE:  Delete all Resolved Net route entry from Resolved Net Table, include ipv4 and ipv6.
 * INPUT:
 *        fib_id:             FIB ID.
 * OUTPUT:   none.
 * RETURN:   none.
 * NOTES:    Resolved Net Table should be clear if CLI disable superNetting feature.
 * -------------------------------------------------------------------------*/
void AMTRL3_OM_ClearResolvedNetRouteDatabase(UI32_T fib_id);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_GetNextUnresolvedHostEntry
 * -------------------------------------------------------------------------
 * PURPOSE: Get the next available host route entry from the Unresolve_List
 * INPUT:
 *        action_flags:       AMTRL3_TYPE_FLAGS_IPV4 -- get ipv4 entry
 *                            AMTRL3_TYPE_FLAGS_IPV6 -- get ipv6 entry
 *        fib_id:             FIB ID.
 *
 * OUTPUT:  host_route_entry -- host route record
 * RETURN:  TRUE       --  Successfully,
 *          FALSE      --  Fail.
 * NOTES:   1. get the first entry from the unresloved host route list
 *          2. return FALSE when list is empty or one loop has finished
 *          3. action_flags: if v4\v6 is set, only search in v4\v6 entry,
 *                           if both are set, search in v4, then v6
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_OM_GetNextUnresolvedHostEntry(
                                            UI32_T action_flags,
                                            UI32_T fib_id,
                                            AMTRL3_OM_HostRouteEntry_T *host_route_entry);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_GetNextResolvedNotSyncHostEntry
 * -------------------------------------------------------------------------
 * PURPOSE: Get the host entry in Resolved_not_sync state. Net entry with
 *          nhop pointed to host_entry's ip need to be sync to OM and Chip
 * INPUT:
 *        action_flags:       AMTRL3_TYPE_FLAGS_IPV4 -- get ipv4 entry
 *                            AMTRL3_TYPE_FLAGS_IPV6 -- get ipv6 entry
 *        fib_id:             FIB ID.
 *
 * OUTPUT:  host_route_entry  -- host route record
 * RETURN:  TRUE       --  Successfully,
 *          FALSE      --  Fail.
 * NOTES:   1. get the first entry from the resloved_not_sync host route list
 *          2. return FALSE when list is empty
 *          3. action_flags: if v4\v6 is set, only search in v4\v6 entry,
 *                           if both are set, search in v4, then v6
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_OM_GetNextResolvedNotSyncHostEntry (
                                                 UI32_T action_flags,
                                                 UI32_T fib_id,
                                                 AMTRL3_OM_HostRouteEntry_T  *host_route_entry);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_GetNextGatewayEntry
 * -------------------------------------------------------------------------
 * PURPOSE: Get the host entry in Gateway entry list.
 * INPUT:
 *        action_flags:       AMTRL3_TYPE_FLAGS_IPV4 -- get ipv4 entry
 *                            AMTRL3_TYPE_FLAGS_IPV6 -- get ipv6 entry
 *        fib_id:             FIB ID.
 *
 * OUTPUT:  host_route_entry  -- gateway host route record.
 * RETURN:  TRUE       --  Successfully,
 *          FALSE      --  Fail.
 * NOTES:   1. get the first entry from the gateway entry list
 *          2. return FALSE when list is empty or one loop has finished
 *          3. action_flags: if v4\v6 is set, only search in v4\v6 entry,
 *                           if both are set, search in v4, then v6
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_OM_GetNextGatewayEntry (UI32_T action_flags,
                                      UI32_T fib_id,
                                      AMTRL3_OM_HostRouteEntry_T  *host_route_entry);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_GetSuperNetEntry
 * -------------------------------------------------------------------------
 * PURPOSE:  Find any matching net route entry that is a supernet of the
 *           specific subnet entry
 * INPUT:
 *           action_flags:       AMTRL3_TYPE_FLAGS_IPV4 -- get ipv4 entry
 *                               AMTRL3_TYPE_FLAGS_IPV6 -- get ipv6 entry
 *           fib_id:             FIB ID.
 *           child_subnet - specific subnet to compare with.
 *           compare_length - specific length of the subnet.
 * OUTPUT:   supernet_entry - A net entry that is the parent of the given specific
 *                            subnet entry, which covers larger network than the
 *                            child_subnet route.
 * RETURN:   none.
 * NOTES:    ipv4 and ipv6 cannot set at the same time
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_OM_GetSuperNetEntry(UI32_T action_flags,
                                  UI32_T fib_id,
                                  L_INET_AddrIp_T child_subnet,
                                  UI32_T compare_length,
                                  AMTRL3_OM_NetRouteEntry_T *supernet_entry);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_GetDoulbeLinkListCounter
 * -------------------------------------------------------------------------
 * PURPOSE: This function prints out the number of element currently exist in
 *          each double link list
 * INPUT:
 *        fib_id:             FIB ID.
 *
 * OUTPUT:  none.
 * RETURN:  None.
 * NOTES:   None.
 * -------------------------------------------------------------------------*/
void AMTRL3_OM_GetDoulbeLinkListCounter (UI32_T fib_id);

/* --------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_IsAddressEqual
 * -------------------------------------------------------------------------
 * PURPOSE: To check if the two address are equal
 * INPUT:   addr1 - the first address
 *          addr2 - the second address
 * OUTPUT:  None.
 * RETURN:  TRUE - equal
 *          FALSE - not eaual.
 * NOTES:   None.
 *--------------------------------------------------------------------------*/
BOOL_T AMTRL3_OM_IsAddressEqual(L_INET_AddrIp_T *addr1, L_INET_AddrIp_T *addr2);

/* --------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_IsAddressEqualZero
 * -------------------------------------------------------------------------
 * PURPOSE: To check if IP address is equal to zero.
 * INPUT:   addr - IP address
 * OUTPUT:  None.
 * RETURN:  TRUE - equal
 *          FALSE - not eaual.
 * NOTES:   we donot check it's address type.
 *--------------------------------------------------------------------------*/
BOOL_T AMTRL3_OM_IsAddressEqualZero(L_INET_AddrIp_T *addr);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_GetOneNetRouteEntryByDstIp
 * -------------------------------------------------------------------------
 * PURPOSE: get one cidr route entry which with the same dest IP, for ECMP purpose
 * INPUT:
 *        action_flags:       AMTRL3_TYPE_FLAGS_IPV4 -- get ipv4 entry
 *                            AMTRL3_TYPE_FLAGS_IPV6 -- get ipv6 entry
 *        fib_id:              pointer to om database of FIB .
 *        net_route_entry:   net route structure saved key value
 *                            KEY :
 *                            inet_cidr_route_dest          -- destination inet address
 *                            inet_cidr_route_pfxlen        -- prefix length
 *
 * OUTPUT:   net_route_entry -- record of forwarding table.
 * RETURN:   TRUE    - Successfully,
 *           FALSE   - If not available or EOF.
 * NOTES:   1. ipv4 and ipv6 cannot set at the same time
 *          2. action flag: AMTRL3_TYPE_FLAGS_IPV4/IPV6 must consistent with
 *             address type in net_route_entry structure
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_OM_GetOneNetRouteEntryByDstIp(UI32_T action_flags,
                                            UI32_T fib_id,
                                            AMTRL3_OM_NetRouteEntry_T *net_route_entry);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_GetOneNetRouteEntryByDstIp
 * -------------------------------------------------------------------------
 * PURPOSE: get one cidr route entry which with the same dest IP, for ECMP purpose
 * INPUT:
 *        action_flags:       AMTRL3_TYPE_FLAGS_IPV4 -- get ipv4 entry
 *                            AMTRL3_TYPE_FLAGS_IPV6 -- get ipv6 entry
 *        fib_id:              pointer to om database of FIB .
 *        net_route_entry:   net route structure saved key value
 *                            KEY :
 *                            inet_cidr_route_dest          -- destination inet address
 *                            inet_cidr_route_pfxlen        -- prefix length
 *
 * OUTPUT:   net_route_entry -- record of forwarding table.
 * RETURN:   TRUE    - Successfully,
 *           FALSE   - If not available or EOF.
 * NOTES:   1. ipv4 and ipv6 cannot set at the same time
 *          2. action flag: AMTRL3_TYPE_FLAGS_IPV4/IPV6 must consistent with
 *             address type in net_route_entry structure
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_OM_GetOneNetRouteEntryByDstIp(UI32_T action_flags,
                                            UI32_T fib_id,
                                            AMTRL3_OM_NetRouteEntry_T *net_route_entry);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - AMTRL3_OM_HandleIPCReqMsg
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the ipc request message for AMTRL3 OM.
 *
 * INPUT   : msgbuf_p -- input request ipc message buffer
 *
 * OUTPUT  : msgbuf_p -- output response ipc message buffer
 *
 * RETURN  : TRUE  - there is a response required to be sent
 *           FALSE - there is no response required to be sent
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T AMTRL3_OM_HandleIPCReqMsg(SYSFUN_Msg_T* msgbuf_p);


/* FUNCTION NAME : AMTRL3_OM_CreateInterface
 * PURPOSE:
 *          Store the new created ip interface entry.
 *
 * INPUT:
 *      fib_id
 *      intf  -- the interface entry.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE/FALSE.
 *
 * NOTES:
 *      None.
 */
BOOL_T AMTRL3_OM_CreateInterface(UI32_T fib_id, AMTRL3_OM_Interface_T *intf);


/* FUNCTION NAME : AMTRL3_OM_DeleteInterface
 * PURPOSE:
 *          Delete an ip interface entry.
 *
 * INPUT:
 *      fib_id
 *      intf  -- the interface entry.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE/FALSE.
 *
 * NOTES:
 *      None.
 */
BOOL_T AMTRL3_OM_DeleteInterface(UI32_T fib_id, AMTRL3_OM_Interface_T *intf);


/* FUNCTION NAME : AMTRL3_OM_GetNextInterface
 * PURPOSE:
 *          Get Next IP interface entry
 *
 * INPUT:
 *      fib_id
 *      intf->vid
 *
 * OUTPUT:
 *      intf
 *
 * RETURN:
 *      AMTRL3_TYPE_SUCCESS
 *      AMTRL3_TYPE_FAIL
 *
 * NOTES:
 */
UI32_T AMTRL3_OM_GetNextIpInterface(UI32_T fib_id, AMTRL3_OM_Interface_T *intf);

#if (SYS_CPNT_IP_TUNNEL == TRUE)
/* FUNCTION NAME : AMTRL3_OM_GetNextHostRouteEntryByTunnelNexthop
 * PURPOSE:
 *          Get Next host route intry by tunnel's next hop ip address
 *
 * INPUT:
 *      action_flags : AMTRL3_TYPE_FLAGS_IPV4 or AMTRL3_TYPE_FLAGS_IPV6
 *      fib_id
 *      nexthop      : tunnel's nexthop address
 *      host_route_entry:  key is host_route_entry.key_fields.dst_vid_ifindex
 *
 * OUTPUT:
 *      host_route_entry: tunnel host route entry
 *
 * RETURN:
 *      AMTRL3_TYPE_SUCCESS
 *      AMTRL3_TYPE_FAIL
 *
 * NOTES:
 *  This API do what AMTRL3_OM_GetNextHostRouteEntry() does, but filter out entries that does not match giving nexthop address
 */
BOOL_T AMTRL3_OM_GetNextHostRouteEntryByTunnelNexthop(
                                                UI32_T action_flags,
                                                UI32_T fib_id,
                                                L_INET_AddrIp_T *nexthop,
                                                AMTRL3_OM_HostRouteEntry_T *host_route_entry);


/* FUNCTION NAME : AMTRL3_OM_GetHostRouteEntryByTunnelIfIndex
 * PURPOSE:
 *          Get  host route intry by tunnel ifindex
 *
 * INPUT:
 *      action_flags : AMTRL3_TYPE_FLAGS_IPV4 or AMTRL3_TYPE_FLAGS_IPV6
 *      fib_id
 *      host_route_entry:  key is host_route_entry.key_fields.dst_vid_ifindex
 *
 * OUTPUT:
 *      host_route_entry: tunnel host route entry
 *
 * RETURN:
 *      AMTRL3_TYPE_SUCCESS
 *      AMTRL3_TYPE_FAIL
 *
 * NOTES:
 */
BOOL_T AMTRL3_OM_GetHostRouteEntryByTunnelIfIndex(UI32_T action_flags,
                                                UI32_T fib_id,
                                                AMTRL3_OM_HostRouteEntry_T *host_route_entry);


/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_GetHostRouteEntryByTunnelPrefix
 * -------------------------------------------------------------------------
 * PURPOSE: get the specified host route entry by tunnel ifindex and tunnel entry type
 * INPUT:
 *        action_flags:       AMTRL3_TYPE_FLAGS_IPV4 -- get ipv4 entry
 *                            AMTRL3_TYPE_FLAGS_IPV6 -- get ipv6 entry
 *        fib_id       -- FIB id
 *        host_route_entry  -- host route structure saved key value
 *                             KEY:
 *                             dst_vid_ifindex:    vlan ifindex
 *                             dst_inet_addr:      address and prefixlen
 *                             tunnel_entry_type:  tunnel mode
 * OUTPUT:  host_route_entry -- record of host route entry.
 * RETURN:  TRUE    - Successfully,
 *          FALSE   - If not available or EOF.
 * NOTES:   1. If all keys equal to zero, means get the first record
 *          2. action_flags: if v4 and v6 are both set, search in v4 entry,
 *                           if not found, then search in v6 entry.
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_OM_GetHostRouteEntryByTunnelPrefix(UI32_T action_flags,
                                                UI32_T fib_id,
                                                AMTRL3_OM_HostRouteEntry_T *host_route_entry);
/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_GetNetRouteEntryByTunnelPrefix
 * -------------------------------------------------------------------------
 * PURPOSE: get the specified net route entry by tunnel ifindex and tunnel entry type
 * INPUT:
 *        action_flags:       AMTRL3_TYPE_FLAGS_IPV4 -- get ipv4 entry
 *                            AMTRL3_TYPE_FLAGS_IPV6 -- get ipv6 entry
 *        fib_id       -- FIB id
 *        net_route_entry  -- host route structure saved key value
 *                             KEY:
 *                             inet_cidr_route_if_index:  vlan ifindex
 *                             inet_cidr_route_dest:      address and prefixlen
 *                             tunnel_entry_type:  tunnel mode
 * OUTPUT:  host_route_entry -- record of host route entry.
 * RETURN:  TRUE    - Successfully,
 *          FALSE   - If not available or EOF.
 * NOTES:   1. If all keys equal to zero, means get the first record
 *          2. action_flags: if v4 and v6 are both set, search in v4 entry,
 *                           if not found, then search in v6 entry.
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_OM_GetNetRouteEntryByTunnelPrefix(UI32_T action_flags,
                                                UI32_T fib_id,
                                                AMTRL3_OM_NetRouteEntry_T *net_route_entry);
#endif/*SYS_CPNT_IP_TUNNEL*/

#if (SYS_CPNT_VXLAN == TRUE)
BOOL_T AMTRL3_OM_SetVxlanTunnelEntry(UI32_T fib_id, AMTRL3_OM_VxlanTunnelEntry_T *tunnel_entry_p);
BOOL_T AMTRL3_OM_DeleteVxlanTunnelEntry(UI32_T fib_id, AMTRL3_OM_VxlanTunnelEntry_T *tunnel_entry_p);
BOOL_T AMTRL3_OM_GetVxlanTunnelEntry(UI32_T fib_id, AMTRL3_OM_VxlanTunnelEntry_T *tunnel_entry_p);
BOOL_T AMTRL3_OM_GetNextVxlanTunnelEntry(UI32_T fib_id, AMTRL3_OM_VxlanTunnelEntry_T *tunnel_entry_p);
BOOL_T AMTRL3_OM_GetVxlanTunnelEntryByVxlanPort(UI32_T fib_id, AMTRL3_TYPE_VxlanTunnelEntry_T *tunnel_entry_p);

BOOL_T AMTRL3_OM_SetVxlanTunnelNexthopEntry(UI32_T fib_id, AMTRL3_OM_VxlanTunnelNexthopEntry_T *tunnel_nexthop_entry_p);
BOOL_T AMTRL3_OM_DeleteVxlanTunnelNexthopEntry(UI32_T fib_id, AMTRL3_OM_VxlanTunnelNexthopEntry_T *tunnel_nexthop_entry_p);
BOOL_T AMTRL3_OM_GetVxlanTunnelNexthopEntry(UI32_T fib_id, AMTRL3_OM_VxlanTunnelNexthopEntry_T *tunnel_nexthop_entry_p);
BOOL_T AMTRL3_OM_GetNextVxlanTunnelNexthopEntry(UI32_T fib_id, AMTRL3_OM_VxlanTunnelNexthopEntry_T *tunnel_nexthop_entry_p);

BOOL_T AMTRL3_OM_GetNextVxlanTunnelByNexthop(UI32_T fib_id, AMTRL3_OM_VxlanTunnelNexthopEntry_T *tunnel_nexthop_p);

#endif

#endif
