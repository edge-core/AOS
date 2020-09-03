/* MODULE NAME:  bgp_type.h
 * PURPOSE:
 *     Define common types used in BGP.
 *
 * NOTES:
 *
 * HISTORY
 *    02/14/2011 - Peter Yu, Created
 *
 * Copyright(C)      Accton Corporation, 2011
 */
#ifndef BGP_TYPE_H
#define BGP_TYPE_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_adpt.h"
#include "sys_dflt.h"
#include "l_inet.h"
#include "leaf_4273.h"

/* NAME CONSTANT DECLARATIONS
 */
#define BGP_TYPE_MAX_NBR_OF_INSTANCE                    SYS_ADPT_BGP_MAX_NBR_OF_INSTANCE
#define BGP_TYPE_MAX_NETWORK_PER_INSTANCE               SYS_ADPT_BGP_MAX_NETWORK_PER_INSTANCE
#define BGP_TYPE_MAX_NEIGHBOR_PER_INSTANCE              SYS_ADPT_BGP_MAX_NEIGHBOR_PER_INSTANCE
#define BGP_TYPE_MAX_CONFEDERATION_PEER_PER_INSATNCE    SYS_ADPT_BGP_MAX_CONFEDERATION_PEER_PER_INSATNCE
#define BGP_TYPE_MAX_DISTANCE_PER_INSATNCE              SYS_ADPT_BGP_MAX_DISTANCE_PER_INSATNCE
#define BGP_TYPE_MAX_PEER_GROUP_PER_INSTANCE            SYS_ADPT_BGP_MAX_PEER_GROUP_PER_INSTANCE
#define BGP_TYPE_MAX_PEER_PER_PEER_GROUP                SYS_ADPT_BGP_MAX_PEER_PER_PEER_GROUP
#define BGP_TYPE_MAX_AGGREGATE_ADDR_PER_INSTANCE        SYS_ADPT_BGP_MAX_AGGREGATE_ADDR_PER_INSTANCE

#define BGP_TYPE_PEER_GROUP_NAME_LEN            SYS_ADPT_BGP_MAX_PEER_GROUP_NAME_LENGTH
#define BGP_TYPE_PEER_STR_LEN                   SYS_ADPT_BGP_MAX_PEER_STRING_LENGTH
#define BGP_TYPE_CLEAR_BGP_ARG_STR_LEN          SYS_ADPT_BGP_MAX_CLEAR_BGP_ARGUMENT_STRING_LENGTH
#define BGP_TYPE_NEIGHBOR_DESC_LEN              SYS_ADPT_BGP_MAX_NEIGHBOR_DESCRIPTION_LENGTH
#define BGP_TYPE_PEER_PASSWORD_LEN             SYS_ADPT_BGP_MD5_AUTHENCATION_KEY_LEN


#define BGP_TYPE_DEFAULT_LOCAL_PREF             SYS_DFLT_BGP_LOCAL_PREF
#define BGP_TYPE_DEFAULT_SCAN_INTERVAL          SYS_DFLT_BGP_SCAN_INTERVAL
#define BGP_TYPE_DEFAULT_KEEP_ALIVE_INTERVAL    SYS_DFLT_BGP_KEEP_ALIVE_INTERVAL
#define BGP_TYPE_DEFAULT_HOLD_TIME              SYS_DFLT_BGP_HOLD_TIME
#define BGP_TYPE_DEFAULT_IBGP_ROUTEADV          SYS_DFLT_BGP_IBGP_ROUTEADV
#define BGP_TYPE_DEFAULT_EBGP_ROUTEADV          SYS_DFLT_BGP_EBGP_ROUTEADV
#define BGP_TYPE_DEFAULT_ALLOW_AS_IN            SYS_DFLT_BGP_ALLOW_AS_IN
#define BGP_TYPE_DEFAULT_EBGP_MULTIHOP          SYS_DFLT_BGP_EBGP_MULTIHOP
#define BGP_TYPE_DEFAULT_HALF_LIFE              SYS_DFLT_BGP_DAMPENING_HALF_LIFE
#define BGP_TYPE_DEFAULT_REUSE                  SYS_DFLT_BGP_DAMPENING_REUSE_LIMIT
#define BGP_TYPE_DEFAULT_SUPPRESS               SYS_DFLT_BGP_DAMPENING_SUPPRESS_LIMIT
#define BGP_TYPE_DEFAULT_CONNECT_RETRY          SYS_DFLT_BGP_CONNECT_RETRY
#define BGP_TYPE_DEFAULT_IBGP_DISTANCE          SYS_DFLT_BGP_IBGP_DISTANCE
#define BGP_TYPE_DEFAULT_EBGP_DISTANCE          SYS_DFLT_BGP_EBGP_DISTANCE

#define BGP_TYPE_ACCESS_LIST_NAME_LEN           SYS_ADPT_MAX_ACCESS_LIST_NAME_LENGTH
#define BGP_TYPE_ROUTE_MAP_NAME_LEN             SYS_ADPT_MAX_ROUTE_MAP_NAME_LENGTH
#define BGP_TYPE_PREFIX_LIST_NAME_LEN           SYS_ADPT_MAX_PREFIX_LIST_NAME_LENGTH
#define BGP_TYPE_AS_LIST_NAME_LEN               SYS_ADPT_MAX_AS_PATH_ACCESS_LIST_NAME_LENGTH
#define BGP_TYPE_COMMUNITY_NAME_LEN             SYS_ADPT_MAX_COMMUNITY_NAME_LENGTH
#define BGP_TYPE_ROUTE_MAP_COMMAND_LEN          SYS_ADPT_MAX_ROUTE_MAP_COMMAND_LENGTH
#define BGP_TYPE_ROUTE_MAP_ARG_LEN              SYS_ADPT_MAX_ROUTE_MAP_ARGUMENT_LENGTH
#define BGP_TYPE_REGULAR_EXP_LEN                SYS_ADPT_MAX_REGULAR_EXPRESSION_LENGTH

/**********************************
 ** definitions for return value **
 **********************************
 */
enum
{
    BGP_TYPE_RESULT_OK = 0,
    BGP_TYPE_RESULT_FAIL,
    BGP_TYPE_RESULT_MULTIPLE_INSTANCE_NOT_SET,
    BGP_TYPE_RESULT_INSTANCE_NOT_EXIST,
    BGP_TYPE_RESULT_INTERFACE_NOT_EXIST,
    BGP_TYPE_RESULT_ROUTE_NOT_EXIST,
    BGP_TYPE_RESULT_INVALID_COMMAND,
    BGP_TYPE_RESULT_INVALID_ADDRESS,
    BGP_TYPE_RESULT_INVALID_AS,
    BGP_TYPE_RESULT_INVALID_ARG,
    BGP_TYPE_RESULT_INVALID_BGP,
    BGP_TYPE_RESULT_UNKNOWN_PEER,
    BGP_TYPE_RESULT_UNKNOWN_PEER_GROUP,
    BGP_TYPE_RESULT_INVALID_FOR_PEER_GROUP_MEMBER,
    BGP_TYPE_RESULT_SEND_MSG_FAIL,
    BGP_TYPE_RESULT_NO_MORE_ENTRY,
    BGP_TYPE_RESULT_ENTRY_EXIST,
    BGP_TYPE_RESULT_ENTRY_NOT_EXIST,
    BGP_TYPE_RESULT_ROUTE_MAP_GOTO_BACKWARD_ERROR,
    BGP_TYPE_RESULT_COMMUNITY_LIST_ERR_STANDARD_CONFLICT,
    BGP_TYPE_RESULT_COMMUNITY_LIST_ERR_EXPANDED_CONFLICT,
    BGP_TYPE_RESULT_COMMUNITY_LIST_ERR_NAME_IS_ALL_DIGIT,
};

/* Cluster id format: decimal or ip address
 */
#define BGP_TYPE_CLUSTER_ID_FORMAT_DECIMAL      1
#define BGP_TYPE_CLUSTER_ID_FORMAT_IP_ADDRESS   2
#define BGP_TYPE_CLUSTER_ID_FORMAT_MAX          3

/* Address family numbers from RFC1700. 
 * Must identical with AFI_XXXX in zebra.h
 */
#define BGP_TYPE_AFI_IP                    1
#define BGP_TYPE_AFI_IP6                   2
#define BGP_TYPE_AFI_MAX                   3

/* Subsequent Address Family Identifier. 
 * Must identical with SAFI_XXXX in zebra.h
 */
#define BGP_TYPE_SAFI_UNICAST              1
#define BGP_TYPE_SAFI_MULTICAST            2
#define BGP_TYPE_SAFI_UNICAST_MULTICAST    3
#define BGP_TYPE_SAFI_MPLS_VPN             4
#define BGP_TYPE_SAFI_MAX                  5

/* Filter direction constant
 * Must identical with FILTER_XXX in Filter direction of zebra.h
 */
#define BGP_TYPE_FILTER_DIRECTION_IN       0
#define BGP_TYPE_FILTER_DIRECTION_OUT      1
#define BGP_TYPE_FILTER_DIRECTION_MAX      2

/* Route map direction constant
 * Must identical with RMAP_XXX in bgpd.h
 */
#define BGP_TYPE_RMAP_IN            0
#define BGP_TYPE_RMAP_OUT           1
#define BGP_TYPE_RMAP_IMPORT        2
#define BGP_TYPE_RMAP_EXPORT        3
#define BGP_TYPE_RMAP_MAX           4

/* BGP finite state machine status.  
 * Must identical with section "BGP finite state machine status" in bgpd.h
 */
#define BGP_TYPE_STATUS_IDLE                                    1
#define BGP_TYPE_STATUS_CONNECT                                 2
#define BGP_TYPE_STATUS_ACTIVE                                  3
#define BGP_TYPE_STATUS_OPENSENT                                4
#define BGP_TYPE_STATUS_OPENCONFIRM                             5
#define BGP_TYPE_STATUS_ESTABLISHED                             6
#define BGP_TYPE_STATUS_CLEARING                                7
#define BGP_TYPE_STATUS_DELETED                                 8
#define BGP_TYPE_STATUS_MAX                                     9


/* Per BGP instance flag
 * Must identical with BGP_FLAG_XXXX in bgpd.h under "u_int16_t flags;"
 */
#define BGP_TYPE_BGP_FLAG_ALWAYS_COMPARE_MED                (1 << 0)
#define BGP_TYPE_BGP_FLAG_DETERMINISTIC_MED                 (1 << 1)
#define BGP_TYPE_BGP_FLAG_MED_MISSING_AS_WORST              (1 << 2)
#define BGP_TYPE_BGP_FLAG_MED_CONFED                        (1 << 3)
#define BGP_TYPE_BGP_FLAG_NO_DEFAULT_ACTIVATE_IPV4_UNICAST  (1 << 4)
#define BGP_TYPE_BGP_FLAG_NO_CLIENT_TO_CLIENT_REFLECTION    (1 << 5)
#define BGP_TYPE_BGP_FLAG_ENFORCE_FIRST_AS                  (1 << 6)
#define BGP_TYPE_BGP_FLAG_BESTPATH_COMPARE_ROUTERID         (1 << 7)
#define BGP_TYPE_BGP_FLAG_BESTPATH_AS_PATH_IGNORE           (1 << 8)
#define BGP_TYPE_BGP_FLAG_NETWORK_IMPORT_CHECK              (1 << 9)
#define BGP_TYPE_BGP_FLAG_NO_FAST_EXTERNAL_FAILOVER         (1 << 10)
#define BGP_TYPE_BGP_FLAG_LOG_NEIGHBOR_CHANGES              (1 << 11)
#define BGP_TYPE_BGP_FLAG_GRACEFUL_RESTART                  (1 << 12)
#define BGP_TYPE_BGP_FLAG_BESTPATH_AS_PATH_CONFED           (1 << 13)

/* Per neighbor flag
 * Must identical with PEER_FLAG_XXXX in bgpd.h under "u_int32_t flags;"
 */
#define BGP_TYPE_NEIGHBOR_FLAG_PASSIVE                   (1 << 0) /* passive mode */
#define BGP_TYPE_NEIGHBOR_FLAG_SHUTDOWN                  (1 << 1) /* shutdown */
#define BGP_TYPE_NEIGHBOR_FLAG_DONT_CAPABILITY           (1 << 2) /* dont-capability */
#define BGP_TYPE_NEIGHBOR_FLAG_OVERRIDE_CAPABILITY       (1 << 3) /* override-capability */
#define BGP_TYPE_NEIGHBOR_FLAG_STRICT_CAP_MATCH          (1 << 4) /* strict-match */
#define BGP_TYPE_NEIGHBOR_FLAG_DYNAMIC_CAPABILITY        (1 << 5) /* dynamic capability */
#define BGP_TYPE_NEIGHBOR_FLAG_DISABLE_CONNECTED_CHECK   (1 << 6) /* disable-connected-check */
#define BGP_TYPE_NEIGHBOR_FLAG_LOCAL_AS_NO_PREPEND       (1 << 7) /* local-as no-prepend */

/* Per neighbor AFI SAFI flag
 * Must identical with PEER_FLAG_XXXX in bgpd.h under "u_int32_t af_flags[AFI_MAX][SAFI_MAX];"
 */
#define BGP_TYPE_NEIGHBOR_AF_FLAG_SEND_COMMUNITY            (1 << 0) /* send-community */
#define BGP_TYPE_NEIGHBOR_AF_FLAG_SEND_EXT_COMMUNITY        (1 << 1) /* send-community ext. */
#define BGP_TYPE_NEIGHBOR_AF_FLAG_NEXTHOP_SELF              (1 << 2) /* next-hop-self */
#define BGP_TYPE_NEIGHBOR_AF_FLAG_REFLECTOR_CLIENT          (1 << 3) /* reflector-client */
#define BGP_TYPE_NEIGHBOR_AF_FLAG_RSERVER_CLIENT            (1 << 4) /* route-server-client */
#define BGP_TYPE_NEIGHBOR_AF_FLAG_SOFT_RECONFIG             (1 << 5) /* soft-reconfiguration */
#define BGP_TYPE_NEIGHBOR_AF_FLAG_AS_PATH_UNCHANGED         (1 << 6) /* transparent-as */
#define BGP_TYPE_NEIGHBOR_AF_FLAG_NEXTHOP_UNCHANGED         (1 << 7) /* transparent-next-hop */
#define BGP_TYPE_NEIGHBOR_AF_FLAG_MED_UNCHANGED             (1 << 8) /* transparent-next-hop */
#define BGP_TYPE_NEIGHBOR_AF_FLAG_DEFAULT_ORIGINATE         (1 << 9) /* default-originate */
#define BGP_TYPE_NEIGHBOR_AF_FLAG_REMOVE_PRIVATE_AS         (1 << 10) /* remove-private-as */
#define BGP_TYPE_NEIGHBOR_AF_FLAG_ALLOWAS_IN                (1 << 11) /* set allowas-in */
#define BGP_TYPE_NEIGHBOR_AF_FLAG_ORF_PREFIX_SM             (1 << 12) /* orf capability send-mode */
#define BGP_TYPE_NEIGHBOR_AF_FLAG_ORF_PREFIX_RM             (1 << 13) /* orf capability receive-mode */
#define BGP_TYPE_NEIGHBOR_AF_FLAG_MAX_PREFIX                (1 << 14) /* maximum prefix */
#define BGP_TYPE_NEIGHBOR_AF_FLAG_MAX_PREFIX_WARNING        (1 << 15) /* maximum prefix warning-only */
#define BGP_TYPE_NEIGHBOR_AF_FLAG_NEXTHOP_LOCAL_UNCHANGED   (1 << 16) /* leave link-local nexthop unchanged */

/* Route type 
 * Must identical with ZEBRA_ROUTE_XXXX in zebra.h
 */
#define BGP_TYPE_ROUTE_SYSTEM               0
#define BGP_TYPE_ROUTE_KERNEL               1
#define BGP_TYPE_ROUTE_CONNECT              2
#define BGP_TYPE_ROUTE_STATIC               3
#define BGP_TYPE_ROUTE_RIP                  4
#define BGP_TYPE_ROUTE_RIPNG                5
#define BGP_TYPE_ROUTE_OSPF                 6
#define BGP_TYPE_ROUTE_OSPF6                7
#define BGP_TYPE_ROUTE_ISIS                 8
#define BGP_TYPE_ROUTE_BGP                  9
#define BGP_TYPE_ROUTE_HSLS		            10
#define BGP_TYPE_ROUTE_MAX                  11

/* should mapping to bgp_route.c,  enum bgp_show_type */
enum BGP_TYPE_SHOW_TYPE
{
    BGP_TYPE_SHOW_TYPE_NORMAL,
    BGP_TYPE_SHOW_TYPE_REGEXP,
    BGP_TYPE_SHOW_TYPE_PREFIX_LIST,
    BGP_TYPE_SHOW_TYPE_FILTER_LIST,
    BGP_TYPE_SHOW_TYPE_ROUTE_MAP,
    BGP_TYPE_SHOW_TYPE_NEIGHBOR,
    BGP_TYPE_SHOW_TYPE_CIDR_ONLY,
    BGP_TYPE_SHOW_TYPE_PREFIX_LONGER,
    BGP_TYPE_SHOW_TYPE_COMMUNITY_ALL,
    BGP_TYPE_SHOW_TYPE_COMMUNITY,
    BGP_TYPE_SHOW_TYPE_COMMUNITY_EXACT,
    BGP_TYPE_SHOW_TYPE_COMMUNITY_LIST,
    BGP_TYPE_SHOW_TYPE_COMMUNITY_LIST_EXACT,
    BGP_TYPE_SHOW_TYPE_FLAP_STATISTICS,
    BGP_TYPE_SHOW_TYPE_FLAP_ADDRESS,
    BGP_TYPE_SHOW_TYPE_FLAP_PREFIX,
    BGP_TYPE_SHOW_TYPE_FLAP_CIDR_ONLY,
    BGP_TYPE_SHOW_TYPE_FLAP_REGEXP,
    BGP_TYPE_SHOW_TYPE_FLAP_FILTER_LIST,
    BGP_TYPE_SHOW_TYPE_FLAP_PREFIX_LIST,
    BGP_TYPE_SHOW_TYPE_FLAP_PREFIX_LONGER,
    BGP_TYPE_SHOW_TYPE_FLAP_ROUTE_MAP,
    BGP_TYPE_SHOW_TYPE_FLAP_NEIGHBOR,
    BGP_TYPE_SHOW_TYPE_DAMPEND_PATHS,
    BGP_TYPE_SHOW_TYPE_DAMP_NEIGHBOR
};

/* IBGP/EBGP identifier.  We also have a CONFED peer, which is to say,
   a peer who's AS is part of our Confederation.  */
/* Must identical with BGP_PEER_XXX in bgpd.h */   
enum
{
    BGP_TYPE_NEIGHBOR_IBGP,
    BGP_TYPE_NEIGHBOR_EBGP,
    BGP_TYPE_NEIGHBOR_INTERNAL,
    BGP_TYPE_NEIGHBOR_CONFED
};

/* ref. bgp_vty.c */
/* BGP clear sort. */
enum BGP_TYPE_CLEAR_SORT
{
    BGP_TYPE_CLEAR_ALL,
    BGP_TYPE_CLEAR_PEER,
    BGP_TYPE_CLEAR_GROUP,
    BGP_TYPE_CLEAR_EXTERNAL,
    BGP_TYPE_CLEAR_AS,
};
/* BGP clear type */
enum BGP_TYPE_CLEAR_TYPE
{
    BGP_TYPE_CLEAR_SOFT_NONE,
    BGP_TYPE_CLEAR_SOFT_OUT,
    BGP_TYPE_CLEAR_SOFT_IN,
    BGP_TYPE_CLEAR_SOFT_BOTH,
    BGP_TYPE_CLEAR_SOFT_IN_ORF_PREFIX,
    BGP_TYPE_CLEAR_SOFT_RSCLIENT
};


/* Route map's type. */
/* must identical with enum route_map_type in routemap.h */
enum BGP_TYPE_ROUTE_MAP_TYPE
{
    BGP_TYPE_RMAP_PERMIT,
    BGP_TYPE_RMAP_DENY,
    BGP_TYPE_RMAP_ANY
};

/* Must identical with BGP_SAFI_XXX */
/* SAFI which used in open capability negotiation.  */
#define BGP_TYPE_SAFI_VPNV4                         128
#define BGP_TYPE_SAFI_VPNV6 

/* Must identical with XXX in bgp_open.h */
/* Capability Code */
#define BGP_TYPE_CAPABILITY_CODE_MP              1 /* Multiprotocol Extensions */
#define BGP_TYPE_CAPABILITY_CODE_REFRESH         2 /* Route Refresh Capability */
#define BGP_TYPE_CAPABILITY_CODE_ORF             3 /* Cooperative Route Filtering Capability */
#define BGP_TYPE_CAPABILITY_CODE_RESTART        64 /* Graceful Restart Capability */
#define BGP_TYPE_CAPABILITY_CODE_AS4            65 /* 4-octet AS number Capability */
#define BGP_TYPE_CAPABILITY_CODE_DYNAMIC        66 /* Dynamic Capability */
#define BGP_TYPE_CAPABILITY_CODE_REFRESH_OLD   128 /* Route Refresh Capability(cisco) */
#define BGP_TYPE_CAPABILITY_CODE_ORF_OLD       130 /* Cooperative Route Filtering Capability(cisco) */

/* Capability Length */
#define BGP_TYPE_CAPABILITY_CODE_MP_LEN          4
#define BGP_TYPE_CAPABILITY_CODE_REFRESH_LEN     0
#define BGP_TYPE_CAPABILITY_CODE_DYNAMIC_LEN     0
#define BGP_TYPE_CAPABILITY_CODE_RESTART_LEN     2 /* Receiving only case */
#define BGP_TYPE_CAPABILITY_CODE_AS4_LEN         4

/* ORF Type */
#define BGP_TYPE_ORF_TYPE_PREFIX                64 
#define BGP_TYPE_ORF_TYPE_PREFIX_OLD           128

/* Community-list entry types.  */
/* Must identical with XXX in bgp_clist.h */
#define BGP_TYPE_COMMUNITY_LIST_STANDARD        0 /* Standard community-list.  */
#define BGP_TYPE_COMMUNITY_LIST_EXPANDED        1 /* Expanded community-list.  */
#define BGP_TYPE_EXTCOMMUNITY_LIST_STANDARD     2 /* Standard extcommunity-list.  */
#define BGP_TYPE_EXTCOMMUNITY_LIST_EXPANDED     3 /* Expanded extcommunity-list.  */


/* MACRO FUNCTION DECLARATIONS
 */
#define BGP_TYPE_IS_VALID_AFI_SAFI(afi, safi) (afi > 0 && afi < BGP_TYPE_AFI_MAX && safi > 0 && safi < BGP_TYPE_SAFI_MAX)

#define BGP_TYPE_IS_VALID_AFI_RTYPE(afi, rtype) (afi > 0 && afi < BGP_TYPE_AFI_MAX && rtype < BGP_TYPE_ROUTE_MAX)

/* DATA TYPE DECLARATIONS
 */

typedef struct
{
    UI32_T as_number;     /* Key */

  /* Self peer.  */
  L_INET_AddrIp_T peer_self;

//TODO:    L_SORT_LST_List_T neighbor_list;
    //TODO:   L_SORT_LST_List_T neighbor_group_list;

    UI32_T neighbor_count;
    UI32_T rsclient_count;
    UI32_T neighbor_groups_count;

  /* BGP configuration.  */
  UI16_T config;
#define BGP_TYPE_CONFIG_ROUTER_ID              (1 << 0)
#define BGP_TYPE_CONFIG_CLUSTER_ID             (1 << 1)
#define BGP_TYPE_CONFIG_CONFEDERATION          (1 << 2)

  /* BGP router identifier.  */
    UI32_T router_id;
    UI32_T router_id_static;

  /* BGP route reflector cluster ID.  */
    UI32_T cluster_id;

  /* BGP confederation information.  */    
    UI32_T confed_id;
    UI32_T confed_peers[BGP_TYPE_MAX_CONFEDERATION_PEER_PER_INSATNCE];
    UI32_T confed_peers_cnt;

  /* BGP flags. */
    UI32_T flags; /* Note: flag values are defined at upper */
  /* BGP Per AF flags */
    UI32_T af_flags[BGP_TYPE_AFI_MAX][BGP_TYPE_SAFI_MAX];
#define BGP_TYPE_CONFIG_DAMPENING              (1 << 0)
#if 0
  /* Static route configuration.  */
  struct bgp_table *route[AFI_MAX][SAFI_MAX];

  /* Aggregate address configuration.  */
  struct bgp_table *aggregate[AFI_MAX][SAFI_MAX];

#endif
  	/* BGP routing information base.  */
  	//struct bgp_table *rib[AFI_MAX][SAFI_MAX];
	UI32_T rib_count[BGP_TYPE_AFI_MAX][BGP_TYPE_SAFI_MAX]; /* RIB entries count */



    /* BGP redistribute configuration. */
    UI8_T redist[BGP_TYPE_AFI_MAX][BGP_TYPE_ROUTE_MAX];
    
    
    /* BGP redistribute metric configuration. */
    UI8_T redist_metric_flag[BGP_TYPE_AFI_MAX][BGP_TYPE_ROUTE_MAX];
    UI32_T redist_metric[BGP_TYPE_AFI_MAX][BGP_TYPE_ROUTE_MAX];
    
    /* BGP redistribute route-map.  */
    char rmap[BGP_TYPE_AFI_MAX][BGP_TYPE_ROUTE_MAX][BGP_TYPE_ROUTE_MAP_NAME_LEN];
    
    /* BGP distance configuration.  */
    UI32_T distance_ebgp;
    UI32_T distance_ibgp;
    UI32_T distance_local;

    /* BGP default local-preference.  */
    UI32_T default_local_pref;

  /* BGP default timer.  */
  UI32_T default_holdtime;
  UI32_T default_keepalive;

  /* BGP graceful restart */
  UI32_T restart_time;
  UI32_T stalepath_time;

    //TODO: BGP_OM_DampeningInfo_T dampening_info[BGP_TYPE_AFI_MAX][BGP_TYPE_SAFI_MAX];
    /* 
    L_SORT_LST_List_T aggregate_addr_list[BGP_TYPE_AFI_MAX][BGP_TYPE_SAFI_MAX];
    L_SORT_LST_List_T network_list[BGP_TYPE_AFI_MAX][BGP_TYPE_SAFI_MAX];
    */
} BGP_TYPE_Instance_T;


//ref. bgp_filter
/* BGP filter structure. */
typedef struct
{
  /* Distribute-list.  */
  struct 
  {
    char name[BGP_TYPE_ACCESS_LIST_NAME_LEN +1];
    BOOL_T is_valid;
  } dlist[BGP_TYPE_FILTER_DIRECTION_MAX];

  /* Prefix-list.  */
  struct
  {
    char name[BGP_TYPE_PREFIX_LIST_NAME_LEN +1];
    BOOL_T is_valid;
  } plist[BGP_TYPE_FILTER_DIRECTION_MAX];

  /* Filter-list.  */
  struct
  {
    char name[BGP_TYPE_AS_LIST_NAME_LEN +1];
    BOOL_T is_valid;
  } aslist[BGP_TYPE_FILTER_DIRECTION_MAX];

  /* Route-map.  */
  struct
  {
    char name[BGP_TYPE_ROUTE_MAP_NAME_LEN +1];
    BOOL_T is_valid;
  } map[BGP_TYPE_RMAP_MAX];

  /* Unsuppress-map.  */
  struct
  {
    char name[BGP_TYPE_ROUTE_MAP_NAME_LEN +1];
    BOOL_T is_valid;
  } usmap;
} BGP_TYPE_BgpFilter_T;

//ref. struct capability_header
/* Standard header for capability TLV */
typedef struct
{
  UI8_T code;
  UI8_T length;
} BGP_TYPE_CapabilityHeader_T;

//ref. struct capability_mp_data
/* Generic MP capability data */
typedef struct
{
  UI16_T afi;
  UI8_T reserved;
  UI8_T safi;
} BGP_TYPE_CapabilityMpData_T;

#define BGP_TYPE_MAX_NOTIFY_DATA    8
//ref. struct bgp_notify
/* BGP Notify message format. */
typedef struct
{
  UI8_T code;
  UI8_T subcode;
  struct {
    BGP_TYPE_CapabilityHeader_T hdr;
    BGP_TYPE_CapabilityMpData_T mpc;
  } data[BGP_TYPE_MAX_NOTIFY_DATA];
  UI32_T length;
} BGP_TYPE_BgpNotify_T;

typedef struct
{   
    /* Key */
    L_INET_AddrIp_T neighbor_addr;

    /* BGP structure. get from local_as  */

    /* BGP peer group.  */
    char group_name[BGP_TYPE_PEER_GROUP_NAME_LEN +1];
    UI8_T af_group[BGP_TYPE_AFI_MAX][BGP_TYPE_SAFI_MAX];
    
    /* Peer's remote AS number. */
    UI32_T as;

    /* Peer's local AS number. */
    UI32_T local_as ;

    /* Peer's Change local AS number. */
    UI32_T change_local_as;
    /* Remote router ID. */
    UI32_T remote_id;

    /* Local router ID. */
    UI32_T local_id;


  /* Peer specific RIB when configured as route-server-client. */
  //struct bgp_table *rib[AFI_MAX][SAFI_MAX];

  /* Packet receive and send buffer. */
  //struct stream *ibuf;
  //struct stream_fifo *obuf;
  //struct stream *work;
	UI32_T obuf_count;

    /* Status of the peer. */
    int status;
    int ostatus;

  /* Peer index, used for dumping TABLE_DUMP_V2 format */
  //uint16_t table_dump_index;

    /* Peer information */
	UI8_T peer_sort; // BGP_PEER_IBGP,...

  	//int fd;			/* File descriptor */
    int ttl;			/* TTL of TCP connection to the peer. */
    char desc[BGP_TYPE_NEIGHBOR_DESC_LEN +1];			/* Description of the peer. */
    unsigned short port;          /* Destination port for peer */
//    char *host;			/* Printable address of the peer. */
    //union sockunion su;		/* Sockunion address of the peer. */
    time_t uptime;		/* Last Up/Down time */
    time_t readtime;		/* Last read time */
    time_t resettime;		/* Last reset time */
/* BGP uptime string length. */
#define  BGP_TYPE_NEIGHBOR_UPTIME_STR_LEN     25
    char uptime_str[BGP_TYPE_NEIGHBOR_UPTIME_STR_LEN +1];
    char readtime_str[BGP_TYPE_NEIGHBOR_UPTIME_STR_LEN +1];
    char resettime_str[BGP_TYPE_NEIGHBOR_UPTIME_STR_LEN +1];
    
 	UI32_T ifindex;		/* ifindex of the BGP connection. */

#define  BGP_TYPE_NEIGHBOR_IFNAME_LEN     32
 
 	char ifname[BGP_TYPE_NEIGHBOR_IFNAME_LEN +1];			/* bind interface name. */
    char update_ifname[BGP_TYPE_NEIGHBOR_IFNAME_LEN +1];
    //union sockunion *update_source;
#define  BGP_TYPE_NEIGHBOR_SOCKUNION_STR_LEN     80

    char update_source_str[BGP_TYPE_NEIGHBOR_SOCKUNION_STR_LEN +1];

    //struct zlog *log;
    
    //union sockunion *su_local;	/* Sockunion of local address.  */
	UI32_T local_ip;
	UI32_T local_port;
    //union sockunion *su_remote;	/* Sockunion of remote address.  */
	UI32_T remote_ip;
	UI32_T remote_port;
	
    //int shared_network;		/* Is this peer shared same network. */
    L_INET_AddrIp_T nexthop; /* Nexthop */
    
    /* Peer address family configuration. */
    UI8_T afc[BGP_TYPE_AFI_MAX][BGP_TYPE_SAFI_MAX];
    UI8_T afc_nego[BGP_TYPE_AFI_MAX][BGP_TYPE_SAFI_MAX];
    UI8_T afc_adv[BGP_TYPE_AFI_MAX][BGP_TYPE_SAFI_MAX];
    UI8_T afc_recv[BGP_TYPE_AFI_MAX][BGP_TYPE_SAFI_MAX];
    
    /* Capability flags (reset in bgp_stop) */
    UI16_T cap;
/* Must identical with PEER_XXX in bgpd.h */    
#define BGP_TYPE_NEIGHBOR_CAP_REFRESH_ADV                (1 << 0) /* refresh advertised */
#define BGP_TYPE_NEIGHBOR_CAP_REFRESH_OLD_RCV            (1 << 1) /* refresh old received */
#define BGP_TYPE_NEIGHBOR_CAP_REFRESH_NEW_RCV            (1 << 2) /* refresh rfc received */
#define BGP_TYPE_NEIGHBOR_CAP_DYNAMIC_ADV                (1 << 3) /* dynamic advertised */
#define BGP_TYPE_NEIGHBOR_CAP_DYNAMIC_RCV                (1 << 4) /* dynamic received */
#define BGP_TYPE_NEIGHBOR_CAP_RESTART_ADV                (1 << 5) /* restart advertised */
#define BGP_TYPE_NEIGHBOR_CAP_RESTART_RCV                (1 << 6) /* restart received */
#define BGP_TYPE_NEIGHBOR_CAP_AS4_ADV                    (1 << 7) /* as4 advertised */
#define BGP_TYPE_NEIGHBOR_CAP_AS4_RCV                    (1 << 8) /* as4 received */

    /* Capability flags (reset in bgp_stop) */
    UI16_T af_cap[BGP_TYPE_AFI_MAX][BGP_TYPE_SAFI_MAX];
/* Must identical with PPER_XXX in bgpd.h */    
#define BGP_TYPE_NEIGHBOR_CAP_ORF_PREFIX_SM_ADV          (1 << 0) /* send-mode advertised */
#define BGP_TYPE_NEIGHBOR_CAP_ORF_PREFIX_RM_ADV          (1 << 1) /* receive-mode advertised */
#define BGP_TYPE_NEIGHBOR_CAP_ORF_PREFIX_SM_RCV          (1 << 2) /* send-mode received */
#define BGP_TYPE_NEIGHBOR_CAP_ORF_PREFIX_RM_RCV          (1 << 3) /* receive-mode received */
#define BGP_TYPE_NEIGHBOR_CAP_ORF_PREFIX_SM_OLD_RCV      (1 << 4) /* send-mode received */
#define BGP_TYPE_NEIGHBOR_CAP_ORF_PREFIX_RM_OLD_RCV      (1 << 5) /* receive-mode received */
#define BGP_TYPE_NEIGHBOR_CAP_RESTART_AF_RCV             (1 << 6) /* graceful restart afi/safi received */
#define BGP_TYPE_NEIGHBOR_CAP_RESTART_AF_PRESERVE_RCV    (1 << 7) /* graceful restart afi/safi F-bit received */

    /* Global configuration flags. */
    UI32_T flags;  /* Note: flag values are defined at upper */


    /* NSF mode (graceful restart) */
    u_char nsf[BGP_TYPE_AFI_MAX][BGP_TYPE_SAFI_MAX];

    /* Per AF configuration flags. */
    UI32_T af_flags[BGP_TYPE_AFI_MAX][BGP_TYPE_SAFI_MAX];

    /* MD5 password */
    BOOL_T is_set_password;

    /* default-originate route-map.  */
    //TODO: show ip neighbor
    struct
    {
      char name[BGP_TYPE_ROUTE_MAP_NAME_LEN];
      //struct route_map *map;
      BOOL_T is_valid;
    } default_rmap[BGP_TYPE_AFI_MAX][BGP_TYPE_SAFI_MAX];

    /* Peer status flags. */
    UI16_T sflags;
 // Must identical with PEER_STATUS_XXX in bgpd.h
#define BGP_TYPE_NEIGHBOR_STATUS_ACCEPT_PEER        (1 << 0) /* accept peer */
#define BGP_TYPE_NEIGHBOR_STATUS_PREFIX_OVERFLOW    (1 << 1) /* prefix-overflow */
#define BGP_TYPE_NEIGHBOR_STATUS_CAPABILITY_OPEN    (1 << 2) /* capability open send */
#define BGP_TYPE_NEIGHBOR_STATUS_HAVE_ACCEPT        (1 << 3) /* accept peer's parent */
#define BGP_TYPE_NEIGHBOR_STATUS_GROUP              (1 << 4) /* peer-group conf */
#define BGP_TYPE_NEIGHBOR_STATUS_NSF_MODE           (1 << 5) /* NSF aware peer */
#define BGP_TYPE_NEIGHBOR_STATUS_NSF_WAIT           (1 << 6) /* wait comeback peer */


    /* Peer status af flags (reset in bgp_stop) */
    UI16_T af_sflags[BGP_TYPE_AFI_MAX][BGP_TYPE_SAFI_MAX];
#define BGP_TYPE_NEIGHBOR_STATUS_ORF_PREFIX_SEND   (1 << 0) /* prefix-list send peer */
#define BGP_TYPE_NEIGHBOR_STATUS_ORF_WAIT_REFRESH  (1 << 1) /* wait refresh received peer */
#define BGP_TYPE_NEIGHBOR_STATUS_DEFAULT_ORIGINATE (1 << 2) /* default-originate peer */
#define BGP_TYPE_NEIGHBOR_STATUS_PREFIX_THRESHOLD  (1 << 3) /* exceed prefix-threshold */
#define BGP_TYPE_NEIGHBOR_STATUS_PREFIX_LIMIT      (1 << 4) /* exceed prefix-limit */
#define BGP_TYPE_NEIGHBOR_STATUS_EOR_SEND          (1 << 5) /* end-of-rib send to peer */
#define BGP_TYPE_NEIGHBOR_STATUS_EOR_RECEIVED      (1 << 6) /* end-of-rib received from peer */


    /* Default attribute value for the peer. */
    UI32_T config; 
/* Must identical with PEER_XXX bgpd.h */
#define BGP_TYPE_NEIGHBOR_CONFIG_WEIGHT            (1 << 0) /* Default weight. */
#define BGP_TYPE_NEIGHBOR_CONFIG_TIMER             (1 << 1) /* keepalive & holdtime */
#define BGP_TYPE_NEIGHBOR_CONFIG_CONNECT           (1 << 2) /* connect */
#define BGP_TYPE_NEIGHBOR_CONFIG_ROUTEADV          (1 << 3) /* route advertise */
    
    UI32_T weight;
    UI32_T holdtime;
    UI32_T keepalive;
    UI32_T connect;
    UI32_T routeadv;
    
    /* Timer values. */
    UI32_T v_start;
    UI32_T v_connect;
    UI32_T v_holdtime;
    UI32_T v_keepalive;
    UI32_T v_asorig;
    UI32_T v_routeadv;
    UI32_T v_pmax_restart;
    UI32_T v_gr_restart;


  /* Threads. */
  //struct thread *t_read;
  //struct thread *t_write;

  //struct thread *t_start;
  //struct thread *t_connect;
   	BOOL_T t_read;
   	BOOL_T t_write;
   	BOOL_T t_start;
	UI32_T t_start_remain_second;

   	BOOL_T t_connect;
	UI32_T t_connect_remain_second;

  	BOOL_T t_pmax_restart;
	UI32_T t_pmax_restart_remain_second;
	
    /* Statistics field */
    UI32_T open_in;		/* Open message input count */
    UI32_T open_out;		/* Open message output count */
    UI32_T update_in;		/* Update message input count */
    UI32_T update_out;		/* Update message ouput count */
    time_t update_time;		/* Update message received time. */
    char update_time_str[BGP_TYPE_NEIGHBOR_UPTIME_STR_LEN +1];

    UI32_T keepalive_in;	/* Keepalive input count */
    UI32_T keepalive_out;	/* Keepalive output count */
    UI32_T notify_in;		/* Notify input count */
    UI32_T notify_out;		/* Notify output count */
    UI32_T refresh_in;		/* Route Refresh input count */
    UI32_T refresh_out;	/* Route Refresh output count */
    UI32_T dynamic_cap_in;	/* Dynamic Capability input count.  */
    UI32_T dynamic_cap_out;	/* Dynamic Capability output count.  */
    
    /* BGP state count */
    UI32_T established;	/* Established */
    UI32_T dropped;		/* Dropped */
    
    /* Syncronization list and time.  */
    struct bgp_synchronize *sync[BGP_TYPE_AFI_MAX][BGP_TYPE_SAFI_MAX];
    time_t synctime;
    
    /* Send prefix count. */
    unsigned long scount[BGP_TYPE_AFI_MAX][BGP_TYPE_SAFI_MAX];
    
    /* Announcement attribute hash.  */
    struct hash *hash[BGP_TYPE_AFI_MAX][BGP_TYPE_SAFI_MAX];
    
    /* Notify data. */
    BGP_TYPE_BgpNotify_T notify;
    
    /* Whole packet size to be read. */
    unsigned long packet_size;
    
    /* Filter structure. */
    BGP_TYPE_BgpFilter_T filter[BGP_TYPE_AFI_MAX][BGP_TYPE_SAFI_MAX];
    
    /* ORF Prefix-list */
    //TODO: struct prefix_list *orf_plist[BGP_TYPE_AFI_MAX][BGP_TYPE_SAFI_MAX];
    
    /* Prefix count. */
    unsigned long pcount[BGP_TYPE_AFI_MAX][BGP_TYPE_SAFI_MAX];
    
    /* Max prefix count. */
    unsigned long pmax[BGP_TYPE_AFI_MAX][BGP_TYPE_SAFI_MAX];
    u_char pmax_threshold[BGP_TYPE_AFI_MAX][BGP_TYPE_SAFI_MAX];
    UI16_T pmax_restart[BGP_TYPE_AFI_MAX][BGP_TYPE_SAFI_MAX];
#define MAXIMUM_PREFIX_THRESHOLD_DEFAULT 75

    /* allowas-in. */
    char allowas_in[BGP_TYPE_AFI_MAX][BGP_TYPE_SAFI_MAX];
    
    /* peer reset cause */
    char last_reset;
#define BGP_TYPE_NEIGHBOR_LAST_RESET_CAUSE_LEN	80    
    char last_reset_str[BGP_TYPE_NEIGHBOR_LAST_RESET_CAUSE_LEN +1];
    /* The kind of route-map Flags.*/
    u_char rmap_type;

  
    UI32_T advertise_interval;
    UI32_T flag;
    UI32_T ebgp_multihop;
//    UI32_T ifindex;
    UI32_T allow_as_in[BGP_TYPE_AFI_MAX][BGP_TYPE_SAFI_MAX];
    UI32_T af_flag[BGP_TYPE_AFI_MAX][BGP_TYPE_SAFI_MAX];
    BOOL_T activate_status[BGP_TYPE_AFI_MAX][BGP_TYPE_SAFI_MAX];
    char desctiption[BGP_TYPE_NEIGHBOR_DESC_LEN];
} BGP_TYPE_Neighbor_T;


enum BGP_TYPE_RUNNING_FIELD
{
	BGP_TYPE_RUNNING_FIELD_BGP = 0,
	BGP_TYPE_RUNNING_FIELD_NEIGHBOR,
		
};

#define BGP_TYPE_AS_PATH_STR_LEN	64


// ref. struct attr in bgp_attr.h

#define ATTR_FLAG_BIT(X)  (1 << ((X) - 1))

// must identical with bgpd.h
/* BGP4 attribute type codes.  */
#define BGP_TYPE_ATTR_ORIGIN                          1
#define BGP_TYPE_ATTR_AS_PATH                         2
#define BGP_TYPE_ATTR_NEXT_HOP                        3
#define BGP_TYPE_ATTR_MULTI_EXIT_DISC                 4
#define BGP_TYPE_ATTR_LOCAL_PREF                      5
#define BGP_TYPE_ATTR_ATOMIC_AGGREGATE                6
#define BGP_TYPE_ATTR_AGGREGATOR                      7
#define BGP_TYPE_ATTR_COMMUNITIES                     8
#define BGP_TYPE_ATTR_ORIGINATOR_ID                   9
#define BGP_TYPE_ATTR_CLUSTER_LIST                   10
#define BGP_TYPE_ATTR_DPA                            11
#define BGP_TYPE_ATTR_ADVERTISER                     12
#define BGP_TYPE_ATTR_RCID_PATH                      13
#define BGP_TYPE_ATTR_MP_REACH_NLRI                  14
#define BGP_TYPE_ATTR_MP_UNREACH_NLRI                15
#define BGP_TYPE_ATTR_EXT_COMMUNITIES                16
#define BGP_TYPE_ATTR_AS4_PATH                       17
#define BGP_TYPE_ATTR_AS4_AGGREGATOR                 18
#define BGP_TYPE_ATTR_AS_PATHLIMIT                   21

/* BGP notify message codes.  */
#define BGP_TYPE_NOTIFY_HEADER_ERR                         1
#define BGP_TYPE_NOTIFY_OPEN_ERR                           2
#define BGP_TYPE_NOTIFY_UPDATE_ERR                         3
#define BGP_TYPE_NOTIFY_HOLD_ERR                           4
#define BGP_TYPE_NOTIFY_FSM_ERR                            5
#define BGP_TYPE_NOTIFY_CEASE                              6
#define BGP_TYPE_NOTIFY_CAPABILITY_ERR                     7
#define BGP_TYPE_NOTIFY_MAX	                               8


/* BGP_NOTIFY_HEADER_ERR sub codes.  */
#define BGP_TYPE_NOTIFY_HEADER_NOT_SYNC               1
#define BGP_TYPE_NOTIFY_HEADER_BAD_MESLEN             2
#define BGP_TYPE_NOTIFY_HEADER_BAD_MESTYPE            3
#define BGP_TYPE_NOTIFY_HEADER_MAX                    4

/* BGP_NOTIFY_OPEN_ERR sub codes.  */
#define BGP_TYPE_NOTIFY_OPEN_UNSUP_VERSION            1
#define BGP_TYPE_NOTIFY_OPEN_BAD_PEER_AS              2
#define BGP_TYPE_NOTIFY_OPEN_BAD_BGP_IDENT            3
#define BGP_TYPE_NOTIFY_OPEN_UNSUP_PARAM              4
#define BGP_TYPE_NOTIFY_OPEN_AUTH_FAILURE             5
#define BGP_TYPE_NOTIFY_OPEN_UNACEP_HOLDTIME          6
#define BGP_TYPE_NOTIFY_OPEN_UNSUP_CAPBL              7
#define BGP_TYPE_NOTIFY_OPEN_MAX                      8

/* BGP update origin.  */
#define BGP_TYPE_ORIGIN_IGP                           0
#define BGP_TYPE_ORIGIN_EGP                           1
#define BGP_TYPE_ORIGIN_INCOMPLETE                    2

#define BGP_TYPE_COMMUNITY_STR_LEN	BGP_TYPE_REGULAR_EXP_LEN
#define BGP_TYPE_MAX_COMMUNITY		16

/* Additional/uncommon BGP attributes.
 * lazily allocated as and when a struct attr
 * requires it.
 */
typedef struct
{
  /* Multi-Protocol Nexthop, AFI IPv6 */
#if 0 // #ifdef HAVE_IPV6
  L_INET_AddrIp_T mp_nexthop_global;
  L_INET_AddrIp_T mp_nexthop_local;
#endif /* HAVE_IPV6 */

  /* Extended Communities attribute. */
  //struct ecommunity *ecommunity;
  char ecommunity_str[BGP_TYPE_COMMUNITY_STR_LEN +1];
  
  /* Route-Reflector Cluster attribute */
  //struct cluster_list *cluster;
  
  /* Unknown transitive attribute. */
  //struct transit *transit;

  //struct in_addr mp_nexthop_global_in;
  //struct in_addr mp_nexthop_local_in;
  
  /* Aggregator Router ID attribute */
  UI32_T aggregator_addr;
  
  /* Route Reflector Originator attribute */
  UI32_T originator_id;
  
  /* Local weight, not actually an attribute */
  UI32_T weight;
  
  /* Aggregator ASN */
  UI32_T aggregator_as;
  
  /* MP Nexthop length */
  UI8_T mp_nexthop_len;
} BGP_TYPE_AttrExtra_T;


/* BGP core attribute structure. */
typedef struct
{
  /* AS Path structure */
	char aspath_str[BGP_TYPE_AS_PATH_STR_LEN + 1];
  /* Community structure */
  char community_str[BGP_TYPE_COMMUNITY_STR_LEN +1];
  /* Lazily allocated pointer to extra attributes */
  BGP_TYPE_AttrExtra_T extra;
  
  /* Reference count of this attribute. */
  unsigned long refcnt;

  /* Flag of attribute is set or not. */
  UI32_T flag;
  
  /* Apart from in6_addr, the remaining static attributes */
  L_INET_AddrIp_T nexthop;
  UI32_T med;
  UI32_T local_pref;
  
  /* AS-Pathlimit */
  struct {
    UI32_T as;
    UI8_T ttl;
  } pathlimit;
  
  /* Path origin attribute */
  UI8_T origin;
  UI32_T aspath_hops_count;
} BGP_TYPE_Attr_T; //ref. struct attr


/* Communities attribute.  */
/* ref. struct community */
typedef struct
{
  /* Reference count of communities value.  */
  UI32_T refcnt;

  /* Communities value size.  */
  int size;

  /* Communities value.  */
  UI32_T vals[BGP_TYPE_MAX_COMMUNITY];

  /* String of community attribute.  This sring is used by vty output
     and expanded community-list for regular expression match.  */
  char str[BGP_TYPE_COMMUNITY_STR_LEN +1];
} BGP_TYPE_Community_T;


// ref. struct aspath in bgp_aspath.h
/* AS path may be include some AsSegments.  */
typedef struct
{
  /* Reference count to this aspath.  */
  UI32_T refcnt;

  /* segment data */
  //struct assegment *segments;
  
  /* String expression of AS path.  This string is used by vty output
     and AS path regular expression match.  */
  char str[BGP_TYPE_AS_PATH_STR_LEN +1];
} BGP_TYPE_AsPath_T;



enum BGP_TYPE_Data_Type_E
{
	BGP_TYPE_Data_Type_Attribute_Info = 0,
	BGP_TYPE_Data_Type_Community_Info,
	BGP_TYPE_Data_Type_AsPath_Info,		
};

typedef struct
{
	UI32_T type;    /* BGP_TYPE_Data_Type_E */
	UI32_T index1;  /* for attr hash */
	UI32_T index2;  /* for attr hash */
    UI32_T backet_addr; /* for UI */
	union
	{
		BGP_TYPE_Attr_T attr;
		BGP_TYPE_Community_T community;
		BGP_TYPE_AsPath_T aspath;
	} u;
} BGP_TYPE_DataUnion_T;

typedef struct 
{
    BOOL_T is_valid;

      /* Doubly linked list.  This information must be linked to
         reuse_list or no_reuse_list.  */
      //struct bgp_damp_info *next;
      //struct bgp_damp_info *prev;

      /* Figure-of-merit.  */
      UI16_T penalty;

      /* Number of flapping.  */
      UI16_T flap;
    	
      /* First flap time  */
      time_t start_time;
      char start_time_str[BGP_TYPE_NEIGHBOR_UPTIME_STR_LEN +1];
     
      /* Last time penalty was updated.  */
      time_t t_updated;

      /* Time of route start to be suppressed.  */
      time_t suppress_time;

      /* Back reference to bgp_info. */
      //struct bgp_info *binfo;

      /* Back reference to bgp_node. */
      //struct bgp_node *rn;

      /* Current index in the reuse_list. */
      int index;

      /* Last time message type. */
      u_char lastrecord;
#define BGP_RECORD_UPDATE	1U
#define BGP_RECORD_WITHDRAW	2U

      UI32_T afi;
      UI32_T safi;

    /* Reuse time  */
    char reuse_time_str[BGP_TYPE_NEIGHBOR_UPTIME_STR_LEN +1];

} BGP_TYPE_BgpDampInfo_T; //BGP_TYPE_bgp_damp_info
 
  /* Extra information */
typedef struct 
	{
    BOOL_T is_valid;
 	/* Pointer to dampening structure.  */
    BGP_TYPE_BgpDampInfo_T damp_info;

    /* This route is suppressed with aggregation.  */
    int suppress;

    /* Nexthop reachability check.  */
    u_int32_t igpmetric;

    /* MPLS label.  */
    u_char tag[3];  

} BGP_TYPE_BgpInfoExtra_T;  //  struct BGP_TYPE_bgp_info_extra
  

// ref. struct bgp_info
typedef struct
{
  /* Peer structure.  */
  L_INET_AddrIp_T peer_addr;
  UI32_T peer_as;
  UI32_T peer_local_as;
  UI32_T peer_remote_id;
  UI32_T peer_af_flags[BGP_TYPE_AFI_MAX][BGP_TYPE_SAFI_MAX];
  
  /* Attribute structure.  */
  BGP_TYPE_Attr_T attr;
  BOOL_T attr_community_no_advertise;
  BOOL_T attr_community_no_export;
  BOOL_T attr_community_local_as;
  
  BGP_TYPE_BgpInfoExtra_T extra;
  
  /* Uptime.  */
  time_t uptime;

  /* BGP information status.  */
  u_int16_t flags;
#define BGP_INFO_IGP_CHANGED    (1 << 0)
#define BGP_INFO_DAMPED         (1 << 1)
#define BGP_INFO_HISTORY        (1 << 2)
#define BGP_INFO_SELECTED       (1 << 3)
#define BGP_INFO_VALID          (1 << 4)
#define BGP_INFO_ATTR_CHANGED   (1 << 5)
#define BGP_INFO_DMED_CHECK     (1 << 6)
#define BGP_INFO_DMED_SELECTED  (1 << 7)
#define BGP_INFO_STALE          (1 << 8)
#define BGP_INFO_REMOVED        (1 << 9)
#define BGP_INFO_COUNTED	(1 << 10)

  /* BGP route type.  This can be static, RIP, OSPF, BGP etc.  */
  u_char type;

  /* When above type is BGP.  This sub type specify BGP sub type
     information.  */
  u_char sub_type;
#define BGP_ROUTE_NORMAL       0
#define BGP_ROUTE_STATIC       1
#define BGP_ROUTE_AGGREGATE    2
#define BGP_ROUTE_REDISTRIBUTE 3 
} BGP_TYPE_BgpInfo_T;

enum BGP_TYPE_FIELD_ID
{
    BGP_TYPE_FIELD_ID_BGP_FLAG, 
    BGP_TYPE_FIELD_ID_AGGREGATE_ADDRESS,

/*    BGP_TYPE_FIELD_ID_BGP_FLAG_ALWAYS_COMPARE_MED,               
    BGP_TYPE_FIELD_ID_BGP_FLAG_DETERMINISTIC_MED,                
    BGP_TYPE_FIELD_ID_BGP_FLAG_MED_MISSING_AS_WORST,             
    BGP_TYPE_FIELD_ID_BGP_FLAG_MED_CONFED,
    BGP_TYPE_FIELD_ID_BGP_FLAG_NO_DEFAULT_ACTIVATE_IPV4_UNICAST, 
    BGP_TYPE_FIELD_ID_BGP_FLAG_NO_CLIENT_TO_CLIENT_REFLECTION,   
    BGP_TYPE_FIELD_ID_BGP_FLAG_ENFORCE_FIRST_AS,                 
    BGP_TYPE_FIELD_ID_BGP_FLAG_BESTPATH_COMPARE_ROUTERID,        
    BGP_TYPE_FIELD_ID_BGP_FLAG_BESTPATH_AS_PATH_IGNORE,          
    BGP_TYPE_FIELD_ID_BGP_FLAG_NETWORK_IMPORT_CHECK,             
    BGP_TYPE_FIELD_ID_BGP_FLAG_NO_FAST_EXTERNAL_FAILOVER,        
    BGP_TYPE_FIELD_ID_BGP_FLAG_LOG_NEIGHBOR_CHANGES,             
    BGP_TYPE_FIELD_ID_BGP_FLAG_GRACEFUL_RESTART,                 
    BGP_TYPE_FIELD_ID_BGP_FLAG_BESTPATH_AS_PATH_CONFED,          
*/
};


/* Must identical with BGP_OM_AggregateAddr_T */
typedef struct
{
    L_INET_AddrIp_T prefix; /* key */
    BOOL_T          is_summary_only;
    BOOL_T          is_as_set;
} BGP_TYPE_AggregateAddr_T;


typedef struct
{
    UI32_T              as_number; 
    UI32_T              afi; 
    UI32_T              safi;
    /* value is BGP_TYPE_FIELD_ID */ 
    UI32_T              field_id; 
    char                peerstr[BGP_TYPE_PEER_STR_LEN];            
    union {
        UI32_T          ui32_data;
        BOOL_T          bool_data;
        BGP_TYPE_AggregateAddr_T aggregate_addr;
    } u;
}  BGP_TYPE_Config_Data_T;



/* Community-list deny and permit.  */
#define BGP_TYPE_COMMUNITY_DENY                 0
#define BGP_TYPE_COMMUNITY_PERMIT               1

/* Number and string based community-list name.  */
#define BGP_TYPE_COMMUNITY_LIST_STRING          0
#define BGP_TYPE_COMMUNITY_LIST_NUMBER          1

/* Community-list entry types.  */
#define BGP_TYPE_COMMUNITY_LIST_STANDARD        0 /* Standard community-list.  */
#define BGP_TYPE_COMMUNITY_LIST_EXPANDED        1 /* Expanded community-list.  */
#define BGP_TYPE_EXTCOMMUNITY_LIST_STANDARD     2 /* Standard extcommunity-list.  */
#define BGP_TYPE_EXTCOMMUNITY_LIST_EXPANDED     3 /* Expanded extcommunity-list.  */

/* Community-list.  */
//struct community_list
typedef struct
{
  /* Name of the community-list.  */
  char name[BGP_TYPE_COMMUNITY_NAME_LEN +1];

  /* String or number.  */
  int sort;

  /* Link to upper list.  */
  //struct community_list_list *parent;

  /* Linked list for other community-list.  */
  //struct community_list *next;
  //struct community_list *prev;

  /* Community-list entry in this community-list.  */
  //struct community_entry *head;
  //struct community_entry *tail;
} BGP_TYPE_CommunityList_T;

/* Each entry in community-list.  */
//struct community_entry
typedef struct
{
  /* Name of the community-list.  */
  char list_name[BGP_TYPE_COMMUNITY_NAME_LEN +1];
  UI32_T index;

  //struct community_entry *next;
  //struct community_entry *prev;

  /* Permit or deny.  */
  UI8_T direct;

  /* Standard or expanded.  */
  UI8_T style;

  /* Any match.  */
  UI8_T any;

  /* Community structure.  */
  union
  {
    BGP_TYPE_Community_T com;
    BGP_TYPE_Community_T ecom; /* ext-com has same struct as com */
  } u;

  /* Configuration string.  */
  char config[BGP_TYPE_REGULAR_EXP_LEN +1];

  /* Expanded community-list regular expression.  */
  //regex_t *reg;
  //char reg[BGP_TYPE_REGULAR_EXP_LEN];
} BGP_TYPE_CommunityEntry_T;



// ref bgp_filter.c/h

enum BGP_TYPE_AsListType_E
{
  BGP_TYPE_ACCESS_TYPE_STRING,
  BGP_TYPE_ACCESS_TYPE_NUMBER
};
enum BGP_TYPE_AsFilterType_E
{
  BGP_TYPE_AS_FILTER_DENY,
  BGP_TYPE_AS_FILTER_PERMIT
};

/* AS path filter list. */
//struct as_list
typedef struct
{
  char name[BGP_TYPE_AS_LIST_NAME_LEN +1];

  enum BGP_TYPE_AsListType_E list_type;

  /* struct as_list *next;
  struct as_list *prev;

  struct as_filter *head;
  struct as_filter *tail;
 */

} BGP_TYPE_AsList_T;

// ref bgp_filter.c/h
/* Element of AS path filter. */
//struct as_filter
typedef struct
{
//  struct as_filter *next;
//  struct as_filter *prev;
  char as_list_name[BGP_TYPE_AS_LIST_NAME_LEN +1];
  UI32_T index;
  enum BGP_TYPE_AsFilterType_E filter_type; /* deny/permit */

//  regex_t *reg;
  char reg_str[BGP_TYPE_REGULAR_EXP_LEN +1];
} BGP_TYPE_AsFilter_T;

typedef struct
{
    UI32_T index;
    UI32_T id;
} BGP_TYPE_Cluster_T;

/* BGP nexthop cache value structure. */
// ref. struct bgp_nexthop_cache
typedef struct
{
    /* This nexthop exists in IGP. */
    BOOL_T valid;

    /* Nexthop is changed. */
    BOOL_T changed;

    /* Nexthop is changed. */
    BOOL_T metricchanged;

    /* IGP route's metric. */
    UI32_T metric;

    /* Nexthop number and nexthop linked list.*/
    UI8_T nexthop_num;
    //struct nexthop *nexthop;
} BGP_TYPE_BgpNexthopCache_T;

typedef struct
{
    UI32_T as_number;
    UI32_T afi;
    UI32_T safi;
    UI32_T half_life; /* min */
    UI32_T reuse_limit;
    UI32_T suppress_limit;
    UI32_T max_suppress_time; /* min */
} BGP_TYPE_DampeningParameters_T;

/* MIB 
 */
typedef struct
{
    char version[MAXSIZE_bgpVersion+1];
} BGP_TYPE_MIB_BgpVersion_T;

typedef struct
{
    UI32_T              bgpPeerIdentifier;  /* IpAddress */
    I32_T               bgpPeerState;       /* INTEGER */
    I32_T               bgpPeerAdminStatus; /* INTEGER */
    I32_T               bgpPeerNegotiatedVersion;   /* Integer32 */
    L_INET_AddrIp_T     bgpPeerLocalAddr;   /* IpAddress */
    I32_T               bgpPeerLocalPort;   /* Integer32 */
    L_INET_AddrIp_T     bgpPeerRemoteAddr;  /* IpAddress */
    I32_T               bgpPeerRemotePort;  /* Integer32 */
    I32_T               bgpPeerRemoteAs;    /* Integer32 */
    UI32_T              bgpPeerInUpdates;   /* Counter32 */
    UI32_T              bgpPeerOutUpdates;  /* Counter32 */
    UI32_T              bgpPeerInTotalMessages;     /* Counter32 */
    UI32_T              bgpPeerOutTotalMessages;    /* Counter32 */
    UI8_T               bgpPeerLastError[SIZE_bgpPeerLastError]; /* OCTET STRING */
    UI32_T              bgpPeerFsmEstablishedTransitions; /* Counter32 */
    UI32_T              bgpPeerFsmEstablishedTime;  /* Gauge32 */
    I32_T               bgpPeerConnectRetryInterval;/* Integer32 */
    I32_T               bgpPeerHoldTime;    /* Integer32 */
    I32_T               bgpPeerKeepAlive;   /* Integer32 */
    I32_T               bgpPeerHoldTimeConfigured;  /* Integer32 */
    I32_T               bgpPeerKeepAliveConfigured; /* Integer32 */
    I32_T               bgpPeerMinASOriginationInterval;    /* Integer32 */
    I32_T               bgpPeerMinRouteAdvertisementInterval;   /* Integer32 */
    UI32_T              bgpPeerInUpdateElapsedTime; /* Gauge32 */
} BGP_TYPE_MIB_BgpPeerEntry_T;

typedef struct
{
    L_INET_AddrIp_T     bgp4PathAttrPeer;   /* IpAddress */
    /* bgp4PathAttrIpAddrPrefixLen   Integer32, */ /* prefix len store in bgp4PathAttrIpAddrPrefix */
    L_INET_AddrIp_T     bgp4PathAttrIpAddrPrefix; /* IpAddress */
    I32_T               bgp4PathAttrOrigin; /* INTEGER */
    UI8_T               bgp4PathAttrASPathSegment[MAXSIZE_bgp4PathAttrASPathSegment]; /* OCTET STRING */
    UI32_T              bgp4PathAttrASPathSegment_len;
    L_INET_AddrIp_T     bgp4PathAttrNextHop;/* IpAddress */
    I32_T               bgp4PathAttrMultiExitDisc; /* Integer32 */
    I32_T               bgp4PathAttrLocalPref; /* Integer32 */
    I32_T               bgp4PathAttrAtomicAggregate; /* INTEGER */
    I32_T               bgp4PathAttrAggregatorAS; /* Integer32 */
    L_INET_AddrIp_T     bgp4PathAttrAggregatorAddr; /* IpAddress */
    I32_T               bgp4PathAttrCalcLocalPref; /* Integer32 */
    I32_T               bgp4PathAttrBest; /* INTEGER */
    UI8_T               bgp4PathAttrUnknown[MAXSIZE_bgp4PathAttrUnknown]; /* OCTET STRING */
    UI32_T              bgp4PathAttrUnknown_len;
} BGP_TYPE_MIB_Bgp4PathAttrEntry_T;

#endif    /* End of BGP_TYPE_H */

