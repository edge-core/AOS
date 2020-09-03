/* MODULE NAME:  bgp_om.h
 * PURPOSE:
 *
 * NOTES:
 *     None.
 *
 * HISTORY
 *    Date          -- Modifier,    Reason
 *    03/01/2011    -- Peter Yu,    Created
 *    03/25/2011    -- KH Shi,      Add and Modify APIs
 *
 * Copyright(C)      Edge-Core Corporation, 2011
 */
#ifndef _BGP_OM_H
#define _BGP_OM_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sysfun.h"
#include "l_inet.h"
#include "l_sort_lst.h"
#include "bgp_type.h"

/* NAME CONSTANT DECLARATIONS
 */
#define BGP_OM_MAX_NBR_OF_AGGREGATE_ADDRESS     128
#define BGP_OM_MAX_NBR_OF_CONFEDERATION_PEER    128
#define BGP_OM_MAX_NBR_OF_NETWORK               128
#define BGP_OM_MAX_NBR_OF_DISTANCE              128

/* MACRO FUNCTION DECLARATIONS
 */
#define BGP_OM_IPCMSG_TYPE_SIZE sizeof(union BGP_OM_IpcMsg_Type_U)

#define BGP_OM_MSG_HEADER_SIZE ((UI32_T)(&(((BGP_OM_IpcMsg_T*)0)->data)))


#define BGP_OM_GET_MSG_SIZE(field_name)                       \
            (BGP_OM_IPCMSG_TYPE_SIZE +                        \
            sizeof(((BGP_OM_IpcMsg_T *)0)->data.field_name))

/* DATA TYPE DECLARATIONS
 */
typedef enum 
{
    BGP_OM_BGP_INSTANCE_LOCAL_PREFERENCE = 0,
    
    BGP_OM_BGP_INSTANCE_FLAG,
    BGP_OM_FIELD_BGP_MAIN_SCAN_TIME,
    BGP_OM_FIELD_BGP_MAIN_INSTANCE_LIST,

    BGP_OM_FIELD_BGP_INSTANCE_FLAG,
    BGP_OM_FIELD_BGP_INSTANCE_CLUSTER_ID,
    BGP_OM_FIELD_BGP_INSTANCE_CLUSTER_ID_FORMAT,
    BGP_OM_FIELD_BGP_INSTANCE_CONFEDERATION_ID,
    BGP_OM_FIELD_BGP_INSTANCE_DAMPENING_INFO,
    BGP_OM_FIELD_BGP_INSTANCE_LOCAL_PREFERENCE,
    BGP_OM_FIELD_BGP_INSTANCE_ROUTER_ID,
    BGP_OM_FIELD_BGP_INSTANCE_DISTANCE_IBGP,
    BGP_OM_FIELD_BGP_INSTANCE_DISTANCE_EBGP,
    BGP_OM_FIELD_BGP_INSTANCE_DISTANCE_LOCAL,
    BGP_OM_FIELD_BGP_INSTANCE_KEEP_ALIVE_INTERVAL,
    BGP_OM_FIELD_BGP_INSTANCE_HOLD_TIME,
    BGP_OM_FIELD_BGP_INSTANCE_SCAN_TIME,
    BGP_OM_FIELD_BGP_INSTANCE_CONFEDERATION_PEER_LIST,
    BGP_OM_FIELD_BGP_INSTANCE_DISTANCE_LIST,
    BGP_OM_FIELD_BGP_INSTANCE_NEIGHBOR_LIST,
    BGP_OM_FIELD_BGP_INSTANCE_PEER_GROUP_LIST,
    BGP_OM_FIELD_BGP_INSTANCE_PEER_GROUP_MEMBER_LIST,
    BGP_OM_FIELD_BGP_INSTANCE_AFI_SAFI_AGGREGATE_ADDR_LIST,
    BGP_OM_FIELD_BGP_INSTANCE_AFI_SAFI_DAMPENING_INFO,
    BGP_OM_FIELD_BGP_INSTANCE_AFI_SAFI_NETWORK_LIST,
    BGP_OM_FIELD_BGP_INSTANCE_AFI_RTYPE_REDISTRIBUTE_STATUS,
    BGP_OM_FIELD_BGP_INSTANCE_AFI_RTYPE_REDISTRIBUTE_ROUTEMAP,
    BGP_OM_FIELD_BGP_INSTANCE_AFI_RTYPE_REDISTRIBUTE_METRIC,
    BGP_OM_FIELD_BGP_INSTANCE_CONFIG,
    BGP_OM_FIELD_BGP_INSTANCE_AFI_SAFI_CONFIG,
    BGP_OM_FIELD_BGP_INSTANCE_AFI_RTYPE_CONFIG,

    BGP_OM_FIELD_NEIGHBOR_REMOTE_AS,
    BGP_OM_FIELD_NEIGHBOR_FLAG,
    BGP_OM_FIELD_NEIGHBOR_ADVERTISEMENT_INTERVAL,
    BGP_OM_FIELD_NEIGHBOR_DESCRIPTION,
    BGP_OM_FIELD_NEIGHBOR_EBGP_MULTIHOP,
    BGP_OM_FIELD_NEIGHBOR_INTERFACE,
    BGP_OM_FIELD_NEIGHBOR_EBGP_MULTI_HOP,
    BGP_OM_FIELD_NEIGHBOR_PORT,
    BGP_OM_FIELD_NEIGHBOR_KEEP_ALIVE_INTERVAL,
    BGP_OM_FIELD_NEIGHBOR_HOLD_TIME,
    BGP_OM_FIELD_NEIGHBOR_CONNECT_RETRY_INTERVAL,
    BGP_OM_FIELD_NEIGHBOR_UPDATE_SOURCE,
    BGP_OM_FIELD_NEIGHBOR_WEIGHT,
    BGP_OM_FIELD_NEIGHBOR_CONFIG,
    BGP_OM_FIELD_NEIGHBOR_PASSWORD,

    BGP_OM_FIELD_NEIGHBOR_AFI_SAFI_FLAG,
    BGP_OM_FIELD_NEIGHBOR_AFI_SAFI_ACTIVE_STATUS,
    BGP_OM_FIELD_NEIGHBOR_AFI_SAFI_ALLOWAS_IN,
    //BGP_OM_FIELD_NEIGHBOR_AFI_SAFI_ATTR_UNCHANGED_FLAG,
    BGP_OM_FIELD_NEIGHBOR_AFI_SAFI_DEFAULT_ORIGINATE,
    BGP_OM_FIELD_NEIGHBOR_AFI_SAFI_DISTRIBUTE_LIST,
    BGP_OM_FIELD_NEIGHBOR_AFI_SAFI_FILTER_LIST,
    BGP_OM_FIELD_NEIGHBOR_AFI_SAFI_PREFIX_LIST,
    BGP_OM_FIELD_NEIGHBOR_AFI_SAFI_ROUTE_MAP,
    BGP_OM_FIELD_NEIGHBOR_AFI_SAFI_UNSUPPRESS_MAP,
    BGP_OM_FIELD_NEIGHBOR_AFI_SAFI_MAX_PREFIX_INFO,
    BGP_OM_FIELD_NEIGHBOR_AFI_SAFI_AF_GROUP,
    BGP_OM_FIELD_NEIGHBOR_AFI_SAFI_CONFIG,

} BGP_OM_FieldName_E;

typedef enum
{
    BGP_OM_OPERATION_GET = 0,
    BGP_OM_OPERATION_SET,
    BGP_OM_OPERATION_ADD,
    BGP_OM_OPERATION_DEL,
    BGP_OM_OPERATION_GET_FIRST,
    BGP_OM_OPERATION_GET_NEXT,
    BGP_OM_OPERATION_SET_FLAG,
    BGP_OM_OPERATION_UNSET_FLAG,
} BGP_OM_OPERATION_E;

typedef enum 
{
    BGP_OM_CONFIG_ROUTER_ID = 0,
    BGP_OM_CONFIG_CLUSTER_ID,
    BGP_OM_CONFIG_CONFEDERATION_ID,
    BGP_OM_CONFIG_LOCAL_PREFERENCE,
    BGP_OM_CONFIG_DISTANCE_BGP,
    BGP_OM_CONFIG_TIMERS_BGP,
    BGP_OM_CONFIG_SCAN_TIME,
} BGP_OM_Config_E;

typedef enum 
{
    BGP_OM_CONFIG_AFI_SAFI_DAMPENING_HALF_LIFE = 0, 
    BGP_OM_CONFIG_AFI_SAFI_DAMPENING_REUSE_LIMIT,
    BGP_OM_CONFIG_AFI_SAFI_DAMPENING_SUPPRESS_LIMIT,
    BGP_OM_CONFIG_AFI_SAFI_DAMPENING_MAX_SUPPRESS_TIME,
} BGP_OM_ConfigAfiSafi_E;

typedef enum
{
    BGP_OM_CONFIG_AFI_RTYPE_REDISTRIBUTE_STATUS = 0,
    BGP_OM_CONFIG_AFI_RTYPE_REDISTRIBUTE_RMAP, 
    BGP_OM_CONFIG_AFI_RTYPE_REDISTRIBUTE_METRIC,
} BGP_OM_ConfigAfiRtype_E;

typedef enum 
{
    BGP_OM_CONFIG_NEIGHBOR_GROUP_MEMBER = 0,
    BGP_OM_CONFIG_NEIGHBOR_AS_NUMBER,
    BGP_OM_CONFIG_NEIGHBOR_ADVERTISEMENT_INTERVAL,
    BGP_OM_CONFIG_NEIGHBOR_EBGP_MULTIHOP,
    BGP_OM_CONFIG_NEIGHBOR_INTERFACE,
    BGP_OM_CONFIG_NEIGHBOR_PORT,
    BGP_OM_CONFIG_NEIGHBOR_TIMERS,
    BGP_OM_CONFIG_NEIGHBOR_CONNECT_RETRY_INTERVAL,
    BGP_OM_CONFIG_NEIGHBOR_UPDATE_SOURCE,
    BGP_OM_CONFIG_NEIGHBOR_WEIGHT,
    BGP_OM_CONFIG_NEIGHBOR_DESCRIPTION,
} BGP_OM_ConfigNeighbor_E;

typedef enum 
{
    BGP_OM_CONFIG_NEIGHBOR_AFI_SAFI_ALLOW_AS_IN = 0,
    BGP_OM_CONFIG_NEIGHBOR_AFI_SAFI_ACTIVATE_STATUS,
    //BGP_OM_CONFIG_NEIGHBOR_AFI_SAFI_DEFAULT_ORIGINATE,
    //BGP_OM_CONFIG_NEIGHBOR_AFI_SAFI_DISTRIBUTE_LIST,
    //BGP_OM_CONFIG_NEIGHBOR_AFI_SAFI_UNSUPPRESS_MAP,
    BGP_OM_CONFIG_NEIGHBOR_AFI_SAFI_MAXIMUM_PREFIX,
} BGP_OM_ConfigNeighborAfiSafi_E;

typedef struct
{
    L_INET_AddrIp_T     prefix; /* key */
    UI32_T              distance;
    char                access_list[BGP_TYPE_ACCESS_LIST_NAME_LEN+1];
} BGP_OM_Distance_T;

typedef struct
{
    //L_INET_AddrIp_T     listen_addr;
    //UI32_T              listen_port;
    BOOL_T              is_support_multi_instance;
    L_SORT_LST_List_T   instance_list;
} BGP_OM_Main_T;

typedef struct
{
    L_INET_AddrIp_T prefix; /* key */
    BOOL_T          is_summary_only;
    BOOL_T          is_as_set;
} BGP_OM_AggregateAddr_T;

typedef struct
{
    UI32_T half_life;
    UI32_T reuse_limit;
    UI32_T suppress_limit;
    UI32_T max_suppress_time;
    BOOL_T is_enable;
} BGP_OM_DampeningInfo_T;

typedef struct
{   
    UI32_T as_number;     /* Key */
    UI32_T router_id;
    UI32_T cluster_id;
    UI32_T cluster_id_format;
    UI32_T confederation_id;
    UI32_T flag;
    UI32_T local_preference;
    UI32_T distance_ibgp;
    UI32_T distance_ebgp;
    UI32_T distance_local;
    UI32_T keep_alive_interval;
    UI32_T hold_time;
    UI32_T scan_time;
    BGP_OM_DampeningInfo_T dampening_info[BGP_TYPE_AFI_MAX][BGP_TYPE_SAFI_MAX];
    L_SORT_LST_List_T confederation_peer_list; /* UI32_T */
    L_SORT_LST_List_T distance_list;           /* BGP_OM_Distance_T */
    L_SORT_LST_List_T neighbor_list;           /* BGP_OM_Neighbor_T */
    L_SORT_LST_List_T peer_group_list;         /* BGP_OM_PeerGroup_T Caution: contains L_SORT_LST_List_T */
    L_SORT_LST_List_T aggregate_addr_list[BGP_TYPE_AFI_MAX][BGP_TYPE_SAFI_MAX]; /* BGP_OM_AggregateAddr_T */
    L_SORT_LST_List_T network_list[BGP_TYPE_AFI_MAX][BGP_TYPE_SAFI_MAX]; /* BGP_OM_Network_T */
    BOOL_T redistribute_status[BGP_TYPE_AFI_MAX][BGP_TYPE_ROUTE_MAX];
    char   redistribute_rmap[BGP_TYPE_AFI_MAX][BGP_TYPE_ROUTE_MAX][BGP_TYPE_ROUTE_MAP_NAME_LEN+1];
    UI32_T redistribute_metric [BGP_TYPE_AFI_MAX][BGP_TYPE_ROUTE_MAX];
    UI32_T config;
    UI32_T config_afi_safi[BGP_TYPE_AFI_MAX][BGP_TYPE_SAFI_MAX];
    UI32_T config_afi_rtype[BGP_TYPE_AFI_MAX][BGP_TYPE_ROUTE_MAX];
} BGP_OM_Instance_T;

typedef struct
{
    L_INET_AddrIp_T prefix;  /* key */
    char            route_map[BGP_TYPE_ROUTE_MAP_NAME_LEN+1];
    UI32_T          backdoor;
    UI8_T           ttl;
} BGP_OM_Network_T;

typedef struct
{
    UI32_T direction;
    char access_list[BGP_TYPE_ACCESS_LIST_NAME_LEN+1];
} BGP_OM_AccessListInfo_T;

typedef struct
{
    UI32_T direction;
    char prefix_list[BGP_TYPE_PREFIX_LIST_NAME_LEN+1];
} BGP_OM_PrefixListInfo_T;

typedef struct
{
    UI32_T direction;
    char route_map[BGP_TYPE_ROUTE_MAP_NAME_LEN+1];
} BGP_OM_RouteMapInfo_T;

typedef struct
{
    UI32_T max_prefix;
    UI32_T warn_only;
    UI16_T restart_interval;
    UI8_T  threshold;
} BGP_OM_MaxPrefixInfo_T;

#if 0
typedef struct
{   
    /* Key */
    L_INET_AddrIp_T neighbor_addr;

    UI32_T as_number;
    UI32_T advertise_interval;
    UI32_T flag;
    UI32_T ebgp_multihop;
    UI32_T ifindex;
    UI32_T port;
    UI32_T keep_alive_interval; /* if not set, it will use keep_alive_interval in bgp instance */
    UI32_T hold_time;           /* if not set, it will use hold_time in bgp instance */
    UI32_T connect_retry_interval;
    UI32_T update_source;
    UI32_T weight;
    UI32_T allow_as_in[BGP_TYPE_AFI_MAX][BGP_TYPE_SAFI_MAX];
    UI32_T af_flag[BGP_TYPE_AFI_MAX][BGP_TYPE_SAFI_MAX];
    //UI32_T attr_unchanged_flag[BGP_TYPE_AFI_MAX][BGP_TYPE_SAFI_MAX];
    BOOL_T activate_status[BGP_TYPE_AFI_MAX][BGP_TYPE_SAFI_MAX];
    BOOL_T af_group[BGP_TYPE_AFI_MAX][BGP_TYPE_SAFI_MAX];
    char af_group_name[BGP_TYPE_AFI_MAX][BGP_TYPE_SAFI_MAX][BGP_TYPE_PEER_GROUP_NAME_LEN+1];
    char default_originate[BGP_TYPE_AFI_MAX][BGP_TYPE_SAFI_MAX][BGP_TYPE_ROUTE_MAP_NAME_LEN+1];
    char distribute_list[BGP_TYPE_AFI_MAX][BGP_TYPE_SAFI_MAX][BGP_TYPE_FILTER_DIRECTION_MAX][BGP_TYPE_ACCESS_LIST_NAME_LEN+1];
    char filter_list[BGP_TYPE_AFI_MAX][BGP_TYPE_SAFI_MAX][BGP_TYPE_FILTER_DIRECTION_MAX][BGP_TYPE_ACCESS_LIST_NAME_LEN+1];
    char prefix_list[BGP_TYPE_AFI_MAX][BGP_TYPE_SAFI_MAX][BGP_TYPE_FILTER_DIRECTION_MAX][BGP_TYPE_PREFIX_LIST_NAME_LEN+1];
    char route_map[BGP_TYPE_AFI_MAX][BGP_TYPE_SAFI_MAX][BGP_TYPE_FILTER_DIRECTION_MAX][BGP_TYPE_ROUTE_MAP_NAME_LEN+1];
    char unsuppress_map[BGP_TYPE_AFI_MAX][BGP_TYPE_SAFI_MAX][BGP_TYPE_ROUTE_MAP_NAME_LEN+1];
    char description[BGP_TYPE_NEIGHBOR_DESC_LEN+1];
    BGP_OM_MaxPrefixInfo_T max_prefix_info[BGP_TYPE_AFI_MAX][BGP_TYPE_SAFI_MAX];

    UI32_T config;
    UI32_T config_afi_safi[BGP_TYPE_AFI_MAX][BGP_TYPE_SAFI_MAX];
} BGP_OM_Neighbor_T;
#endif

typedef struct
{   
    UI32_T as_number;
    UI32_T advertise_interval;
    UI32_T flag;
    UI32_T ebgp_multihop;
    UI32_T ifindex;
    UI32_T port;
    UI32_T keep_alive_interval; /* if not set, it will use keep_alive_interval in bgp instance */
    UI32_T hold_time;           /* if not set, it will use hold_time in bgp instance */
    UI32_T connect_retry_interval;
    UI32_T update_source;
    UI32_T weight;
    char description[BGP_TYPE_NEIGHBOR_DESC_LEN+1];
    char group_name[BGP_TYPE_PEER_GROUP_NAME_LEN+1];
    UI32_T config;
    char password [BGP_TYPE_PEER_PASSWORD_LEN+1];
} BGP_OM_NeighborCommonCfg_T;

typedef struct
{
    UI32_T allow_as_in;
    UI32_T af_flag;
    //UI32_T attr_unchanged_flag;
    BOOL_T activate_status;
    BOOL_T af_group;
    char default_originate[BGP_TYPE_ROUTE_MAP_NAME_LEN+1];
    char distribute_list[BGP_TYPE_FILTER_DIRECTION_MAX][BGP_TYPE_ACCESS_LIST_NAME_LEN+1];
    char filter_list[BGP_TYPE_FILTER_DIRECTION_MAX][BGP_TYPE_ACCESS_LIST_NAME_LEN+1];
    char prefix_list[BGP_TYPE_FILTER_DIRECTION_MAX][BGP_TYPE_PREFIX_LIST_NAME_LEN+1];
    char route_map[BGP_TYPE_RMAP_MAX][BGP_TYPE_ROUTE_MAP_NAME_LEN+1];
    char unsuppress_map[BGP_TYPE_ROUTE_MAP_NAME_LEN+1];
    BGP_OM_MaxPrefixInfo_T max_prefix_info;
    UI32_T config_afi_safi;
} BGP_OM_NeighborAfiSafiCfg_T;

/* The following BGP_OM_Neighbor_T is very large (>16KB), only used for database access,
 * don't declare dynamic variable of this data type (will use stack frame memory) !!!
 */
typedef struct
{
    /* Key */
    L_INET_AddrIp_T             neighbor_addr;

    BGP_OM_NeighborCommonCfg_T  cmn_cfg;
    BGP_OM_NeighborAfiSafiCfg_T afi_safi_cfg[BGP_TYPE_AFI_MAX][BGP_TYPE_SAFI_MAX];
} BGP_OM_Neighbor_T;

typedef struct
{
    /* Key */
    L_INET_AddrIp_T             neighbor_addr;
    UI32_T                      afi;
    UI32_T                      safi;

    BGP_OM_NeighborCommonCfg_T  cmn_cfg;
    BGP_OM_NeighborAfiSafiCfg_T afi_safi_cfg;
} BGP_OM_AfiSafiNeighbor_T;

/* The following BGP_OM_PeerGroup_T is very large (>16KB), only used for database access,
 * don't declare dynamic variable of this data type (will use stack frame memory) !!!
 */
typedef struct
{
    char group_name[BGP_TYPE_PEER_GROUP_NAME_LEN+1]; /* Key */
    L_SORT_LST_List_T peer_list; /* L_INET_IpAddr_T */
    BGP_OM_Neighbor_T config;
} BGP_OM_PeerGroup_T;

typedef struct
{   
    char group_name[BGP_TYPE_PEER_GROUP_NAME_LEN+1];
    L_INET_AddrIp_T peer_member;
} BGP_OM_PeerGroupMemberInfo_T;

typedef struct
{   
    UI32_T as_number;     /* Key */
    UI32_T router_id;
    UI32_T cluster_id;
    UI32_T cluster_id_format;
    UI32_T confederation_id;
    UI32_T flag;
    UI32_T local_preference;
    UI32_T distance_ibgp;
    UI32_T distance_ebgp;
    UI32_T distance_local;
    UI32_T keep_alive_interval;
    UI32_T hold_time;
    UI32_T scan_time;
    BGP_OM_DampeningInfo_T dampening_info[BGP_TYPE_AFI_MAX][BGP_TYPE_SAFI_MAX];
    BOOL_T redistribute_status[BGP_TYPE_AFI_MAX][BGP_TYPE_ROUTE_MAX];
    char   redistribute_rmap[BGP_TYPE_AFI_MAX][BGP_TYPE_ROUTE_MAX][BGP_TYPE_ROUTE_MAP_NAME_LEN+1];
    UI32_T redistribute_metric [BGP_TYPE_AFI_MAX][BGP_TYPE_ROUTE_MAX];
    UI32_T config;
    UI32_T config_afi_safi[BGP_TYPE_AFI_MAX][BGP_TYPE_SAFI_MAX];
    UI32_T config_afi_rtype[BGP_TYPE_AFI_MAX][BGP_TYPE_ROUTE_MAX];
} BGP_OM_RunCfgInstance_T;

typedef struct
{
    UI32_T                  as_number;
    UI32_T                  afi;
    UI32_T                  safi;
    BGP_OM_AggregateAddr_T  aggr_addr;
} BGP_OM_RunCfgAggregateAddr_T;

typedef struct
{
    UI32_T              as_number;
    BGP_OM_Distance_T   distance;
} BGP_OM_RunCfgDistance_T;

typedef struct
{
    UI32_T              as_number;
    BGP_OM_AfiSafiNeighbor_T  neighbor;
} BGP_OM_RunCfgNeighbor_T;

typedef struct
{
    UI32_T                  as_number;
    UI32_T                  afi;
    UI32_T                  safi;
    BGP_OM_Network_T        network;
} BGP_OM_RunCfgNetwork_T;

typedef struct
{
    UI32_T              as_number;
    char                group_name[BGP_TYPE_PEER_GROUP_NAME_LEN+1];
    BGP_OM_AfiSafiNeighbor_T    config;
} BGP_OM_RunCfgPeerGroup_T;

typedef struct
{
    UI32_T          as_number;
    char            group_name[BGP_TYPE_PEER_GROUP_NAME_LEN+1];
    L_INET_AddrIp_T ipaddr;
} BGP_OM_RunCfgPeerGroupMember_T;

enum 
{
    BGP_OM_IPCCMD_GET_CONFIG_DATA_BY_FIELD = 0,
    BGP_OM_IPCCMD_GET_NEXT_RUNCFG_INSTANCE,
    BGP_OM_IPCCMD_GET_NEXT_RUNCFG_CONFEDERATION_PEER,
    BGP_OM_IPCCMD_GET_NEXT_RUNCFG_AGGREGATE_ADDR,
    BGP_OM_IPCCMD_GET_NEXT_RUNCFG_DISTANCE,
    BGP_OM_IPCCMD_GET_NEXT_RUNCFG_NEIGHBOR,
    BGP_OM_IPCCMD_GET_NEXT_RUNCFG_NETWORK,
    BGP_OM_IPCCMD_GET_RUNCFG_PEER_GROUP,
    BGP_OM_IPCCMD_GET_NEXT_RUNCFG_PEER_GROUP,
    BGP_OM_IPCCMD_GET_NEXT_RUNCFG_PEER_GROUP_MEMBER,
};


/* structure for the request/response ipc message in bgp pom and om
 */
typedef struct
{
    union BGP_OM_IpcMsg_Type_U
    {
        UI32_T cmd;          /* for sending IPC request. BGP_OM_IPCCMD_xxx ... */
        BOOL_T ret_bool;     /* respond bool return */
        UI32_T ret_ui32;     /* respond ui32 return */
        SYS_TYPE_Get_Running_Cfg_T ret_running_cfg; /* For running config API */
    } type;

    union
    {
        BOOL_T  arg_bool;
        UI32_T  arg_ui32;
        BGP_OM_RunCfgInstance_T         runcfg_instance;
        BGP_OM_RunCfgAggregateAddr_T    runcfg_aggr_addr; 
        BGP_OM_RunCfgDistance_T         runcfg_distance; 
        BGP_OM_RunCfgNeighbor_T         runcfg_neighbor;
        BGP_OM_RunCfgNetwork_T          runcfg_network;
        BGP_OM_RunCfgPeerGroup_T        runcfg_peer_group;
        BGP_OM_RunCfgPeerGroupMember_T  runcfg_group_member;
        BGP_TYPE_Config_Data_T config_data;
        struct
        {    
            UI32_T          ui32_1;
            UI32_T          ui32_2;
        } arg_grp_ui32x2;
        struct
        {    
            UI32_T          arg_ui32;
            BOOL_T          arg_bool;
        } arg_grp_ui32_bool;         
    } data; /* contains the supplemntal data for the corresponding cmd */
} BGP_OM_IpcMsg_T;



/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
BOOL_T BGP_OM_Init(void);
BOOL_T BGP_OM_EnableDebug(void);
BOOL_T BGP_OM_DisableDebug(void);
BOOL_T BGP_OM_GetDebugStatus(BOOL_T *debug);
BOOL_T BGP_OM_ClearAll(void);

BOOL_T BGP_OM_MainFieldOperation(UI32_T field_name, UI32_T oper, void *buf_p, UI32_T buf_len);
BOOL_T BGP_OM_BgpInstanceFieldOperation(UI32_T as_number, UI32_T field_name, UI32_T oper, void *buf_p, UI32_T buf_len);
BOOL_T BGP_OM_BgpInstanceAfiSafiFieldOperation(UI32_T as_number, UI32_T afi, UI32_T safi, UI32_T field_name, UI32_T oper, void *buf_p, UI32_T buf_len);
BOOL_T BGP_OM_BgpInstanceAfiTypeFieldOperation(UI32_T as_number, UI32_T afi, UI32_T type, UI32_T field_name, UI32_T oper, void *buf_p, UI32_T buf_len);
BOOL_T BGP_OM_NeighborsFieldOperation(UI32_T as_number, char peer_str[BGP_TYPE_PEER_STR_LEN+1], UI32_T field_name, UI32_T oper, void *buf_p, UI32_T buf_len, BOOL_T apply_to_member);
BOOL_T BGP_OM_NeighborsAfiSafiFieldOperation(UI32_T as_number, UI32_T afi, UI32_T safi, char peer_str[BGP_TYPE_PEER_STR_LEN+1], UI32_T field_name, UI32_T oper, void *buf_p, UI32_T buf_len, BOOL_T apply_to_member);

BOOL_T BGP_OM_NeighborPeerGroupBind(UI32_T as_number, UI32_T afi, UI32_T safi, char group_name[BGP_TYPE_PEER_GROUP_NAME_LEN+1], L_INET_AddrIp_T *neighbor_ip_p);
BOOL_T BGP_OM_NeighborPeerGroupUnbind(UI32_T as_number, UI32_T afi, UI32_T safi, char group_name[BGP_TYPE_PEER_GROUP_NAME_LEN+1], L_INET_AddrIp_T *neighbor_ip_p);
BOOL_T BGP_OM_NeighborPeerGroupDeleteAllMembers(UI32_T as_number, char group_name[BGP_TYPE_PEER_GROUP_NAME_LEN+1]);
BOOL_T BGP_OM_NeighborsRemoteAsDelete(UI32_T as_number, char peer_str[BGP_TYPE_PEER_STR_LEN+1]);
BOOL_T BGP_OM_NeighborsDeactivate(UI32_T as_number, UI32_T afi, UI32_T safi, char peer_str[BGP_TYPE_PEER_STR_LEN+1]);

BOOL_T BGP_OM_HandleIpcReqMsg(SYSFUN_Msg_T *sysfun_msg_p);

#endif /* _BGP_OM_H */
