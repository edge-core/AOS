/* MODULE NAME:  nsm_mgr.h
 * PURPOSE:
 *     This module provides APIs for ACCTON CSC to use.
 *
 * NOTES:
 *
 * HISTORY
 *    25/6/2007 - Charlie Chen, Created
 *    15/01/2008 - Vai Wang, Modified
 *
 * Copyright(C)      Accton Corporation, 2007
 */
#ifndef NSM_MGR_H
#define NSM_MGR_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sysfun.h"
#include "leaf_es3626a.h"
#include "nsm_type.h"
#include "l_inet.h"
#include "leaf_4001.h"
#include "sys_adpt.h"
#include "sys_cpnt.h"


/* NAMING CONSTANT DECLARATIONS
 */

/***************************************************
 **    nsm_mgr ipc request command definitions    **
 ***************************************************
 */
enum
{
    NSM_MGR_IPCCMD_ADD_STATIC_ROUTE_ENTRY = 0,
    NSM_MGR_IPCCMD_DEL_STATIC_ROUTE_ENTRY,
    NSM_MGR_IPCCMD_DEL_ALL_STATIC_IPV4_ROUTE_ENTRIES,
    NSM_MGR_IPCCMD_DEL_ALL_STATIC_IPV6_ROUTE_ENTRIES,
    NSM_MGR_IPCCMD_GET_ROUTE_NUMBER,
    NSM_MGR_IPCCMD_GET_NEXT_IPV4_ROUTE,
    NSM_MGR_IPCCMD_GET_NEXT_IPV6_ROUTE,
    NSM_MGR_IPCCMD_GET_NEXT_INET_ROUTE,
    NSM_MGR_IPCCMD_ENABLE_IPFORWARDING,
    NSM_MGR_IPCCMD_DISABLE_IPFORWARDING,
    NSM_MGR_IPCCMD_SNMP_GET_IPV4_ROUTE,
    NSM_MGR_IPCCMD_SNMP_GET_IPV6_ROUTE,
    
    NSM_MGR_IPCCMD_SNMP_SET_ROUTE,
    NSM_MGR_IPCCMD_SNMP_GET_ROUTE,
    NSM_MGR_IPCCMD_SNMP_GET_NEXT_ROUTE,
    NSM_MGR_IPCCMD_ADD_RIF,
    NSM_MGR_IPCCMD_DEL_RIF,
    NSM_MGR_IPCCMD_GET_NEXT_RIF,
    NSM_MGR_IPCCMD_FIND_BEST_ROUTE,
    NSM_MGR_IPCCMD_CREATE_INTERFACE,
    NSM_MGR_IPCCMD_CREATE_LOOPBACK_INTERFACE,
    NSM_MGR_IPCCMD_DELETE_INTERFACE,
    NSM_MGR_IPCCMD_DELETE_LOOPBACK_INTERFACE,
    NSM_MGR_IPCCMD_SET_INTERFACE_FLAGS,
    NSM_MGR_IPCCMD_UNSET_INTERFACE_FLAGS,
    NSM_MGR_IPCCMD_GET_NEXT_INTERFACE,
    NSM_MGR_IPCCMD_SET_MULTIPATH_NUMBER,
    NSM_MGR_IPCCMD_GET_MULTIPATH_NUMBER,
    NSM_MGR_IPCCMD_PREFETCH_IPV4_ROUTE,
    NSM_MGR_IPCCMD_POSTFETCH_IPV4_ROUTE,
    NSM_MGR_IPCCMD_CLI_GETNEXT_N_IPV4_ROUTE_BUFFER,
    NSM_MGR_IPCCMD_SET_RA,
    NSM_MGR_IPCCMD_SET_RA_INTERVAL,
    NSM_MGR_IPCCMD_UNSET_RA_INTERVAL,
    NSM_MGR_IPCCMD_SET_RA_LIFETIME,
    NSM_MGR_IPCCMD_UNSET_RA_LIFETIME,
    NSM_MGR_IPCCMD_SET_RA_REACHABLE_TIME,
    NSM_MGR_IPCCMD_UNSET_RA_REACHABLE_TIME,
    NSM_MGR_IPCCMD_SET_RA_MANAGED_FLAG,
    NSM_MGR_IPCCMD_SET_RA_OTHER_FLAG,
    NSM_MGR_IPCCMD_SET_RA_PREFIX,
    NSM_MGR_IPCCMD_UNSET_RA_PREFIX,
    NSM_MGR_IPCCMD_SET_RA_ROUTER_PREFERENCE,
    NSM_MGR_IPCCMD_SET_DEFAULT_RA_HOPLIMIT,
    NSM_MGR_IPCCMD_SET_RA_HOPLIMIT,
    NSM_MGR_IPCCMD_UNSET_RA_HOPLIMIT,
    NSM_MGR_IPCCMD_SET_NS_INTERVAL,
    NSM_MGR_IPCCMD_UNSET_NS_INTERVAL,
    NSM_MGR_IPCCMD_SET_BACKDOOR_DEBUG_FLAG,
    NSM_MGR_IPCCMD_SET_MROUTESTATUS,
    NSM_MGR_IPCCMD_GET_MCAST_INFO,
    NSM_MGR_IPCCMD_GET_NEXT_MUTICAST_ROUTE,
    NSM_MGR_IPCCMD_GET_MUTICAST_ROUTE,
    NSM_MGR_IPCCMD_GET_IP_MROUTE_ROUTE_ENTRY,
    NSM_MGR_IPCCMD_GET_NEXT_IP_MROUTE_ROUTE_ENTRY,
    NSM_MGR_IPCCMD_GET_IP_MROUTE_NEXT_HOP_ENTRY,
    NSM_MGR_IPCCMD_GET_NEXT_IP_MROUTE_NEXT_HOP_ENTRY,
    NSM_MGR_IPCCMD_GET_IP_MROUTE_INTERFACE_ENTRY,
    NSM_MGR_IPCCMD_GET_NEXIT_IP_MROUTE_INTERFACE_ENTRY,
    NSM_MGR_IPCCMD_SET_RA_MTU6,
    NSM_MGR_IPCCMD_UNSET_RA_MTU6,

    /*add by simon shih for IP tunnel*/
    NSM_MGR_IPCCMD_CREATE_TUNNEL_INTERFACE,
    NSM_MGR_IPCCMD_IS_TUNNEL_INTERFACE_EXIST,
    NSM_MGR_IPCCMD_DELETE_TUNNEL_INTERFACE,
#if 0
    NSM_MGR_IPCCMD_SET_TUNNEL_LOCAL_IPV6,    
    NSM_MGR_IPCCMD_UNSET_TUNNEL_LOCAL_IPV6,
#endif    
    NSM_MGR_IPCCMD_SET_TUNNEL_SOURCE,
    NSM_MGR_IPCCMD_UNSET_TUNNEL_SOURCE,

    NSM_MGR_IPCCMD_SET_TUNNEL_MODE,
    NSM_MGR_IPCCMD_UNSET_TUNNEL_MODE,

    NSM_MGR_IPCCMD_SET_TUNNEL_TTL,
    NSM_MGR_IPCCMD_UNSET_TUNNEL_TTL,
    
    NSM_MGR_IPCCMD_SET_TUNNEL_DESTINATION,
    NSM_MGR_IPCCMD_UNSET_TUNNEL_DESTINATION,

    NSM_MGR_IPCCMD_SET_M6ROUTESTATUS,
    NSM_MGR_IPCCMD_GET_MCAST6_INFO,
    NSM_MGR_IPCCMD_GET_NEXT_MUTICAST6_ROUTE,
    NSM_MGR_IPCCMD_GET_MUTICAST6_ROUTE,
    #if 0 /*mcast6 snmp*/
    NSM_MGR_IPCCMD_GET_IP_M6ROUTE_ROUTE_ENTRY,
    NSM_MGR_IPCCMD_GET_NEXT_IP_M6ROUTE_ROUTE_ENTRY,
    NSM_MGR_IPCCMD_GET_IP_M6ROUTE_NEXT_HOP_ENTRY,
    NSM_MGR_IPCCMD_GET_NEXT_IP_M6ROUTE_NEXT_HOP_ENTRY,
    #endif
#if (SYS_CPNT_VRRP == TRUE)
    NSM_MGR_IPCCMD_ADD_VRRP_VIRTUAL_IP,
    NSM_MGR_IPCCMD_DEL_VRRP_VIRTUAL_IP,
#endif

    NSM_MGR_IPCCMD_READ_NETLINK,

};

/*********************************************
 **         route type definitions          **
 ** (for NSM_MGR_IPCCMD_GET_ROUTE_NUMBER)   **
 *********************************************
 */

/* Interface Active Mode definition */
typedef enum
{
    NSM_MGR_ROUTE_TYPE_IPV4_LOCAL=0,
    NSM_MGR_ROUTE_TYPE_IPV4_STATIC,
    NSM_MGR_ROUTE_TYPE_IPV4_RIP,
    NSM_MGR_ROUTE_TYPE_IPV4_OSPF,
    NSM_MGR_ROUTE_TYPE_IPV4_BGP,
    NSM_MGR_ROUTE_TYPE_IPV4_ISIS,
    NSM_MGR_ROUTE_TYPE_IPV4_DYNAMIC,
    NSM_MGR_ROUTE_TYPE_IPV4_ALL,
    NSM_MGR_ROUTE_TYPE_IPV6_LOCAL,
    NSM_MGR_ROUTE_TYPE_IPV6_STATIC,
    NSM_MGR_ROUTE_TYPE_IPV6_RIP,
    NSM_MGR_ROUTE_TYPE_IPV6_OSPF,
    NSM_MGR_ROUTE_TYPE_IPV6_BGP,
    NSM_MGR_ROUTE_TYPE_IPV6_ISIS,
    NSM_MGR_ROUTE_TYPE_IPV6_DYNAMIC,
    NSM_MGR_ROUTE_TYPE_IPV6_ALL,
    NSM_MGR_ROUTE_TYPE_ALL,
    NSM_MGR_ROUTE_TYPE_MAX
} NSM_MGR_RouteType_T;

typedef enum
{
    /* Route sub types.  (type & 0x8) is ISIS sub type.  */
    NSM_MGR_ROUTE_SUBTYPE_OSPF_IA=1,
    NSM_MGR_ROUTE_SUBTYPE_OSPF_NSSA_1,
    NSM_MGR_ROUTE_SUBTYPE_OSPF_NSSA_2,
    NSM_MGR_ROUTE_SUBTYPE_OSPF_EXTERNAL_1,
    NSM_MGR_ROUTE_SUBTYPE_OSPF_EXTERNAL_2,
    NSM_MGR_ROUTE_SUBTYPE_ISIS_L1,
    NSM_MGR_ROUTE_SUBTYPE_ISIS_L2,
    NSM_MGR_ROUTE_SUBTYPE_ISIS_IA,
    NSM_MGR_ROUTE_SUBTYPE_BGP_MPLS,
    NSM_MGR_ROUTE_SUBTYPE_MAX
}NSM_MGR_RouteSubType_T;

enum NSM_MGR_IPv4Rif_ACTIVE_MODE_E
{
	NSM_MGR_IPv4Rif_ACTIVE_MODE_PRIMARY=VAL_netConfigPrimaryInterface_primary,			/*	this rif is primary interface of vid_ifIndex	*/
	NSM_MGR_IPv4Rif_ACTIVE_MODE_SECONDARY=VAL_netConfigPrimaryInterface_secondary,			/*	this rif is secondary interface of vid_ifIndex	*/
    NSM_MGR_IPv4Rif_ACTIVE_MODE_PT_TO_PT,             /*  this rif is a point to point interface, not used now */
    NSM_MGR_IPv4Rif_ACTIVE_MODE_NONE
};

#if 0
/* the expression (&(((NSM_MGR_IPCMsg_T*)0)->data)) to get the size of the
 * message size will result to error "variably modified 'reserved' at file scope"
 * at gcc compiler v4.8.1
 */
#define NSM_MGR_MSG_HEADER_SIZE ((UI32_T)(&(((NSM_MGR_IPCMsg_T*)0)->data)))
#else
#define NSM_MGR_MSG_HEADER_SIZE (sizeof(NSM_MGR_IPCMsg_Header_T))
#endif

/* defines the number of next hop entries in NSM_MGR_IPC_GetNextRouteResp_T
 */
#define NSM_MGR_IPC_GETNEXTROUTERESP_NEXT_HOP_NUM_OF_ENTRIES  SYS_ADPT_MAX_NBR_OF_ECMP_ENTRY_PER_ROUTE

#define NSM_MGR_IPV4_ROUTE_DISPLAY_BUFFER_SIZE         256
#define NSM_MGR_CLI_GET_NEXT_N_IPV4_ROUTE_BUFFER_SIZE  2048


#ifndef MAXMIFS
#define MAXMIFS SYS_ADPT_MAX_NBR_OF_IP_MULTICAST_VIFS /* shall be same as defined in pal_mcast.h and pal_mcast6.h */
#else
#undef MAXMIFS
#define MAXMIFS SYS_ADPT_MAX_NBR_OF_IP_MULTICAST_VIFS
#endif

/* MACRO FUNCTION DECLARATIONS
 */

/* MACRO FUNCTION NAME : NSM_MGR_GET_MSGBUFSIZE
 * PURPOSE:
 *      Get the size of the message buffer which is used by NSM ipc message
 *      according to what type is used in NSM_MGR_IPCMsg_T.data.
 *
 * INPUT:
 *      msg_data_type  --  The type(NSM_MGR_IPCMsg_T.data) used in this message.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      The size of the message buffer which is used by NSM ipc message.
 *
 * NOTES:
 *      None.
 */
#define NSM_MGR_GET_MSGBUFSIZE(msg_data_type) \
    (NSM_MGR_MSG_HEADER_SIZE + sizeof(msg_data_type))

#define NSM_MGR_GET_MSG_SIZE(field_name)                       \
            (NSM_MGR_MSG_HEADER_SIZE +                        \
            sizeof(((NSM_MGR_IPCMsg_T *)0)->data.field_name))

/* MACRO FUNCTION NAME : NSM_MGR_GET_NEXTROUTERESP_MSGBUFSIZE
 * PURPOSE:
 *      Get the size of the message buffer which is used by NSM ipc message
 *      when the type of NSM_MGR_IPCMsg_T.data is NSM_MGR_IPC_GetNextRouteResp_T.
 *
 * INPUT:
 *      entry_p  --  The pointer to NSM_MGR_IPC_GetNextRouteResp_T
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      The size of the message buffer which is used by NSM_MGR_IPC_GetNextRouteResp_T.
 *
 * NOTES:
 *      None.
 */
#define NSM_MGR_GET_NEXTROUTERESP_MSGBUFSIZE(entry_p) \
    (NSM_MGR_MSG_HEADER_SIZE + (UI32_T)(((NSM_MGR_IPC_GetNextRouteResp_T*)0)->ip_next_hop) + (entry_p)->num_of_next_hop*4)

/* DATA TYPE DECLARATIONS
 */

/***************************************************
 **      structures used in nsm_mgr ipc msgs      **
 ***************************************************
 */

typedef struct NSM_MGR_IPC_IpForwardingStatusIndex_S {
    UI32_T  vr_id;
    UI8_T  addr_type; 
}   NSM_MGR_IPC_IpForwardingStatusIndex_T;

typedef struct NSM_MGR_IPC_StaticIpCidrRoute_S
{
    L_INET_AddrIp_T ip_addr_route_dest; 
    L_INET_AddrIp_T ip_addr_route_next_hop;
//    UI8_T  ip_addr_route_dest[SYS_ADPT_IPV4_ADDR_LEN];
//    UI8_T  ip_addr_route_mask[SYS_ADPT_IPV4_ADDR_LEN];
//    UI8_T  ip_addr_next_hop[SYS_ADPT_IPV4_ADDR_LEN];
    UI32_T next_hop_ifindex;
    UI32_T ip_route_distance;
} NSM_MGR_IPC_StaticIpCidrRoute_T;

typedef struct NSM_MGR_IPC_StaticInetCidrRoute_S
{
    UI32_T inet_route_dest_type;
    UI8_T   inet_route_dest[SYS_ADPT_IPV6_ADDR_LEN];
    UI32_T iinet_route_pfxlen;
    UI32_T inet_route_next_hop_type;
    UI8_T inet_route_next_hop[SYS_ADPT_IPV6_ADDR_LEN];
    UI32_T inet_route_metric;
}NSM_MGR_IPC_StaticInetCidrRoute_T;

typedef struct NSM_MGR_IPC_GetLocalRouteNumber_S
{
    UI8_T  multipath_num;
    UI32_T route_number;
    UI32_T fib_route_number;
    UI8_T  route_type;   /* NSM_MGR_ROUTE_TYPE_XXX */
} NSM_MGR_IPC_GetRouteNumber_T;

typedef struct NSM_MGR_IPC_GetNextRouteReq_S
{
    /* set as NULL to get the first route entry
     */
    void* current_route_node;
    /* set as NULL to get the first route entry
     */
    void* current_rib_node;
    /* set as NULL to get the first route entry
     */
    void* current_next_hop_node;

    UI32_T addr_type;/*L_INET_ADDR_TYPE_IPV4 or 6*/
    BOOL_T is_last;
} NSM_MGR_IPC_GetNextRouteReq_T;

typedef struct NSM_MGR_IPC_GetNextRouteResp_S
{
   /* set as next route node to get
    * set as NULL when reachs the end of the route table
    */
    void*   next_route_node;
    void*   next_rib_node;
    void*   next_hop_node;
    UI32_T  addr_type;
    BOOL_T  is_last;
    UI8_T   distance;
    UI8_T   ip_route_dest_prefix_len;
    UI8_T   num_of_next_hop;
    L_INET_AddrIp_T ip_route_dest;
    UI8_T   ip_route_type;
    UI8_T   ip_route_subtype;
    UI32_T  metric;
    UI8_T   flags;
    /* the number of valid number is determined by num_of_next_hop
     */
    L_INET_AddrIp_T ip_next_hop[NSM_MGR_IPC_GETNEXTROUTERESP_NEXT_HOP_NUM_OF_ENTRIES];
    UI8_T   next_hop_type[NSM_MGR_IPC_GETNEXTROUTERESP_NEXT_HOP_NUM_OF_ENTRIES];
    UI8_T   next_hop_flags[NSM_MGR_IPC_GETNEXTROUTERESP_NEXT_HOP_NUM_OF_ENTRIES];
    UI32_T   next_hop_ifindex[NSM_MGR_IPC_GETNEXTROUTERESP_NEXT_HOP_NUM_OF_ENTRIES];
    char   next_hop_ifname[NSM_MGR_IPC_GETNEXTROUTERESP_NEXT_HOP_NUM_OF_ENTRIES][NSM_TYPE_NEXTHOP_IFNAME_SIZE + 1];
} NSM_MGR_IPC_GetNextRouteResp_T;

typedef struct NSM_MGR_IPC_GetNextInetRouteResp_S
{
   /* set as next route node to get
    * set as NULL when reachs the end of the route table
    */
    void*   next_route_node;
    void*   next_rib_node;
    void*   next_hop_node;
    UI32_T  addr_type;/*L_INET_ADDR_TYPE_IPV4 or 6*/
    BOOL_T  is_last;
    UI8_T   distance;
    UI8_T   num_of_next_hop;
    L_INET_AddrIp_T ip_route_dest;

    UI8_T   ip_route_type;
    UI8_T   ip_route_subtype;
    UI32_T  metric;
    UI8_T   flags;
    /* the number of valid number is determined by num_of_next_hop
     */
    L_INET_AddrIp_T ip_next_hop[NSM_MGR_IPC_GETNEXTROUTERESP_NEXT_HOP_NUM_OF_ENTRIES];
    
    UI8_T   next_hop_type[NSM_MGR_IPC_GETNEXTROUTERESP_NEXT_HOP_NUM_OF_ENTRIES];
    UI8_T   next_hop_flags[NSM_MGR_IPC_GETNEXTROUTERESP_NEXT_HOP_NUM_OF_ENTRIES];
    UI32_T   next_hop_ifindex[NSM_MGR_IPC_GETNEXTROUTERESP_NEXT_HOP_NUM_OF_ENTRIES];
    char   next_hop_ifname[NSM_MGR_IPC_GETNEXTROUTERESP_NEXT_HOP_NUM_OF_ENTRIES][NSM_TYPE_NEXTHOP_IFNAME_SIZE + 1];
}NSM_MGR_IPC_GetNextInetRouteResp_T;

typedef struct NSM_MGR_IPC_Rif_S
{
    UI32_T ifindex;
    L_INET_AddrIp_T ip_addr;
    UI32_T primary_interface;
} NSM_MGR_IPC_Rif_T;

typedef struct
{
    UI32_T owner;  /* route type, ex: static, rip.*/
    L_INET_AddrIp_T  dest_addr;
    UI32_T nexthop_count;
    struct 
    {
        L_INET_AddrIp_T ip;
        UI32_T          ifindex;
    } nexthops[SYS_ADPT_MAX_NBR_OF_ECMP_ENTRY_PER_ROUTE];
} NSM_MGR_IPC_Route_T;

#if 0
#if (SYS_CPNT_IPV6 == TRUE)
typedef struct
{
    UI32_T ifindex;
    UI32_T owner;  /* route type, ex: static, rip.*/
    UI8_T  dest_addr[SYS_ADPT_IPV6_ADDR_LEN];
    UI8_T  nexthop[SYS_ADPT_IPV6_ADDR_LEN];
} NSM_MGR_IPC_IPv6Route_T;
#endif
#endif

typedef struct
{
    UI32_T ifindex;
    UI16_T if_flags;
    UI8_T logical_mac[SYS_ADPT_MAC_ADDR_LEN];
    UI32_T mtu;
} NSM_MGR_IPC_Interface_T;

typedef struct NSM_MGR_IPC_CLIGetNextNIpv4Route_S
{
    UI32_T start_from_line;
    UI32_T max_fetch_lines;
    UI32_T fetched_lines;
    UI32_T buffer_size;
    char   buffer[NSM_MGR_CLI_GET_NEXT_N_IPV4_ROUTE_BUFFER_SIZE];
}NSM_MGR_IPC_CLIGetNextNIpv4Route_T;

typedef struct NSM_MGR_IPC_PrefetchIpv4Route_S
{
    BOOL_T show_database;
    NSM_MGR_RouteType_T route_type;
    
}NSM_MGR_IPC_PrefetchIpv4Route_T;

#if (SYS_CPNT_VRRP == TRUE)
typedef struct NSM_MGR_IPC_VrrpVirtualIp_S
{
    L_INET_AddrIp_T vip;
} NSM_MGR_IPC_VrrpVirtualIp_T;
#endif

/* NSM_MGR_IPCMsg_Header_T is defined for ease of the macro function
 * NSM_MGR_MSG_HEADER_SIZE. The definition of this structure must be
 * exactly the same with the header fields in NSM_MGR_IPCMsg_T.
 * Besides, the size of NSM_MGR_IPCMsg_Header_T must be aligned to 4.
 */
typedef struct NSM_MGR_IPCMsg_Header_S
{
    union NSM_MGR_IPCMsg_Header_Type_U
    {
        UI32_T cmd;    /* for sending IPC request command */
        BOOL_T result_bool; /*respond bool return*/
        UI32_T result; /* for response */
    } type;

    UI32_T vr_id;  /* for nsm in zebos */
    UI32_T vrf_id; /* virtual router id for nsm in zebos */
} NSM_MGR_IPCMsg_Header_T;

/*****************************************
 **      nsm_mgr ipc msg structure      **
 *****************************************
 */
typedef struct NSM_MGR_IPCMsg_S
{
    union NSM_MGR_IPCMsg_Type_U
    {
        UI32_T cmd;    /* for sending IPC request command */
        BOOL_T result_bool; /*respond bool return*/
        UI32_T result; /* for response */
    } type;

    UI32_T vr_id;  /* for nsm in zebos */
    UI32_T vrf_id; /* virtual router id for nsm in zebos */

    union NSM_MGR_IPCMsg_Data_U
    {
        NSM_MGR_IPC_StaticIpCidrRoute_T route_entry; /* request, NSM_MGR_IPCCMD_ADD_STATIC_ROUTE_ENTRY */
        NSM_MGR_IPC_GetRouteNumber_T get_route_number; /* request,response, NSM_MGR_IPCCMD_GET_ROUTE_NUMBER */
        NSM_MGR_IPC_GetNextRouteReq_T get_next_route_req; /* request, NSM_MGR_IPCCMD_GET_NEXT_ROUTE */
        NSM_MGR_IPC_GetNextRouteResp_T get_next_route_resp; /* response, NSM_MGR_IPCCMD_GET_NEXT_ROUTE */
        NSM_MGR_IPC_GetNextInetRouteResp_T get_next_inet_route_resp;
        NSM_TYPE_IpCidrRouteEntry_T snmp_cidr_entry;
        NSM_MGR_IPC_Rif_T   rif;
        NSM_MGR_IPC_Route_T route;
        NSM_MGR_IPC_Interface_T interface;
        NSM_MGR_IPC_PrefetchIpv4Route_T prefetch_req;
        NSM_MGR_IPC_CLIGetNextNIpv4Route_T cli_route_buffer;
        UI8_T multipath_num;        
        NSM_MGR_IPC_IpForwardingStatusIndex_T ifsi;
#if (SYS_CPNT_VRRP == TRUE)
        NSM_MGR_IPC_VrrpVirtualIp_T vip_info;
#endif
        UI32_T  mroute_status;
        struct
        {
            UI32_T     vid_ifIndex;
            UI32_T     ui32_v;
        } arg_ifindex_and_ui32;
        struct
        {
            UI32_T     vid_ifIndex;
            UI32_T     ui32_1_v;
            UI32_T     ui32_2_v;
        } arg_ifindex_and_ui32x2;
        struct
        {
            UI32_T     vid_ifIndex;
            BOOL_T    bool_v;
        } arg_ifindex_and_bool;
        struct
        {
            UI32_T ifindex;
            L_INET_AddrIp_T addr;
        }arg_ifindex_and_addr;
        struct
        {
            UI32_T ifindex;
            UI8_T   string_v[64];
        }arg_ifindex_and_string;
        
        struct
        {
            UI32_T vid_ifindex;
            L_INET_AddrIp_T addr;
            UI32_T vlifetime;
            UI32_T plifetime;
            BOOL_T enable_on_link;
            BOOL_T enable_autoconf;
        }arg_ra_prefix;
        
        UI8_T   arg_nodata[0];
    } data;
} NSM_MGR_IPCMsg_T;


/*****************************************
 *****************************************
 */
typedef struct NSM_MGR_GetNextRouteEntry_S
{
    /* reserved: for sending ipc message
     *           caller shall ignore this field.
     */
    UI8_T reserved[SYSFUN_SIZE_OF_MSG(0) + NSM_MGR_MSG_HEADER_SIZE];
    NSM_MGR_IPC_GetNextRouteResp_T data;
} NSM_MGR_GetNextRouteEntry_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
void NSM_MGR_InitiateProcessResources();
/* FUNCTION NAME : NSM_MGR_HandleIPCReqMsg
 * PURPOSE:
 *      Handle the ipc request received from mgr queue.
 *
 * INPUT:
 *      sysfun_msg_p  --  the ipc request for NSM_MGR.
 *
 * OUTPUT:
 *      sysfun_msg_p  --  the ipc response to send when return value is TRUE
 *
 * RETURN:
 *      TRUE   --  A response is required to send
 *      FALSE  --  Need not to send response.
 *
 * NOTES:
 *      None.
 */
BOOL_T NSM_MGR_HandleIPCReqMsg(SYSFUN_Msg_T *sysfun_msg_p);

void NSM_BackDoorMain(void);


#endif    /* End of NSM_MGR_H */

