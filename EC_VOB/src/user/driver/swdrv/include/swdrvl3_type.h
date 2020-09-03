#ifndef _SWDRVL3_TYPE_H_
#define _SWDRVL3_TYPE_H_
#include "sys_type.h"
#include "l_hash.h"
#include "sysfun.h"
#include "l_sort_lst.h"

typedef struct SWDRVL3_StackInfo_S
{
    UI32_T  my_unit_id;
    UI32_T  num_of_units;
    UI32_T  stack_unit_id_tbl[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK];
    UI8_T   master_unit_id;
} SWDRVL3_StackInfo_T;

/* service ID list
 */
typedef enum
{
    SWDRVL3_ENABLE_ROUTING = 0,                 /* 0  */
    SWDRVL3_DISABLE_ROUTING,                    /* 1  */
    SWDRVL3_ENABLE_IP_FORWARDING,               /* 2  */
    SWDRVL3_DISABLE_IP_FORWARDING,              /* 3  */
    SWDRVL3_ADD_L3_MAC,                         /* 4  */
    SWDRVL3_DELETE_L3_MAC,                      /* 5  */
    SWDRVL3_SET_L3_BIT,                         /* 6  */
    SWDRVL3_UNSET_L3_BIT,                       /* 7  */
    SWDRVL3_CREATE_L3_INTERFACE,                /* 8  */
    SWDRVL3_DELETE_L3_INTERFACE,                /* 9  */
    SWDRVL3_SET_INET_HOST_ROUTE,                /* 10 */
    SWDRVL3_DELETE_INET_HOST_ROUTE,             /* 11 */
    SWDRVL3_ADD_INET_NET_ROUTE,                 /* 12 */
    SWDRVL3_DELETE_INET_NET_ROUTE,              /* 13 */
    SWDRVL3_ADD_INET_MY_IP_HOST_ROUTE,          /* 14 */
    SWDRVL3_DELETE_INET_MY_IP_HOST_ROUTE,       /* 15 */
    SWDRVL3_ADD_INET_ECMP_ROUTE_MULTIPATH,      /* 16 */
    SWDRVL3_DELETE_INET_ECMP_ROUTE_MULTIPATH,   /* 17 */
    SWDRVL3_ADD_INET_ECMP_ROUTE_ONE_PATH,       /* 18 */
    SWDRVL3_DELETE_INET_ECMP_ROUTE_ONE_PATH,    /* 19 */
    SWDRVL3_SET_SPECIAL_DEFAULT_ROUTE,          /* 20 */
    SWDRVL3_DELETE_SPECIAL_DEFAULT_ROUTE,       /* 21 */
    SWDRVL3_READ_AND_CLEAR_HOST_ROUTE_HIT_BIT,  /* 22 */
    SWDRVL3_CLEAR_HOST_ROUTE_HW_INFO,           /* 23 */
    SWDRVL3_CLEAR_NET_ROUTE_HW_INFO,            /* 24 */
    SWDRVL3_ADD_TUNNEL_INITIATOR,               /* 25 */
    SWDRVL3_DELETE_TUNNEL_INITIATOR,            /* 26 */
    SWDRVL3_ADD_TUNNEL_TERMINATOR,              /* 27 */
    SWDRVL3_DELETE_TUNNEL_TERMINATOR,           /* 28 */
    SWDRVL3_ADD_INET_HOST_TUNNEL_ROUTE,         /* 29 */
    SWDRVL3_DELETE_INET_HOST_TUNNEL_ROUTE,      /* 30 */
    SWDRVL3_READ_AND_CLEAR_NET_ROUTE_HIT_BIT,   /* 31 */
    SWDRVL3_TUNNEL_NET_ROUTE_HIT_BIT_CHANGE,    /* 32 */
    SWDRVL3_TUNNEL_UPDATE_TTL,                  /* 33 */
    SWDRVL3_ADD_TUNNEL_INTF_L3,                 /* 34 */
    SWDRVL3_SET_ECMP_BALANCE_MODE,              /* 35 */
    /* SWDRVL3_SET_IP_MCAST_ROUTE, not used */             /* 12 */
    /* SWDRVL3_CREATE_IP_MCAST_ROUTE, not used */          /* 13 */
    /* SWDRVL3_DELETE_IP_MCAST_ROUTE, not used */          /* 14 */
    /* SWDRVL3_ADD_IP_MCAST_PORT_MEMBER, not used */        /* 15 */
    /* SWDRVL3_ADD_IP_MCAST_L2_PORT_MEMBER, not used*/     /* 16 */
    /* SWDRVL3_DELETE_IP_MCAST_PORT_MEMBER, not used*/    /* 17 */
    /* SWDRVL3_DELETE_IP_MCAST_L2_PORT_MEMBER, not used*/  /* 18 */
#if 0 /* VaiWang, Wednesday, May 21, 2008 1:31:04 */
    SWDRVL3_SET_AGE_TIMER,                   /* 19 */
    SWDRVL3_DELETE_HOST_ROUTE_BY_SUBNET,     /* 20 */
    SWDRVL3_DELETE_HOST_ROUTE_BY_PORT,       /* 21 */
    SWDRVL3_CLEAR_ALL_HOST_ROUTE_TABLE,      /* 22 */
    SWDRVL3_AGE_OUT_HOST_TABLE,              /* 23 */
    SWDRVL3_READ_AND_CLEAR_HOST_ROUTE_HIT_BIT, /* 24 */
#endif /* comment out by Vai */
    SWDRVL3_MAX_SERVICE_ID
    /*  end of 2004.09.23, ruliang */

} SWDRVL3_ServicesID_T;

typedef struct L3INTERFACE_S
{
    UI32_T fib_id;
    UI32_T vid;
    UI8_T  mac[SYS_ADPT_MAC_ADDR_LEN];
    UI32_T hw_info;
}__attribute__((packed, aligned(1)))L3Interface_T;

typedef struct IP_FORWARDING_OP_S
{
    UI32_T flags;
    UI32_T vr_id;
}IP_FORWARDING_OP_T;

#if 0 /* VaiWang, Friday, May 23, 2008 1:55:37 */
typedef struct DEFAULTROUTE_S
{
     UI32_T next_hop_gateway_ip;
     UI8_T  src_mac[SYS_ADPT_MAC_ADDR_LEN];
     UI8_T  next_hop_mac[SYS_ADPT_MAC_ADDR_LEN];
     UI32_T dst_vid;
     UI32_T egress_unit;
     UI32_T egress_port;
     UI32_T action;
     UI32_T trunk_id;
     BOOL_T is_trunk;
     BOOL_T tagged_frame;
}__attribute__((packed, aligned(1)))DefaultRoute_T;
#endif /* Vai comment out */

typedef struct NETROUTE_S
{
     UI32_T action;
     UI32_T flags;
     UI32_T fib_id;
     union{
         UI32_T ipv4_addr;
         UI8_T  ipv6_addr[SYS_ADPT_IPV6_ADDR_LEN];
     }dst_ip;
     UI32_T prefix_length;
    union{
        UI32_T ipv4_addr;
        UI8_T  ipv6_addr[SYS_ADPT_IPV6_ADDR_LEN];
    }next_hop_ip;
     UI8_T  next_hop_mac[SYS_ADPT_MAC_ADDR_LEN];
     UI8_T  src_mac[SYS_ADPT_MAC_ADDR_LEN];
     UI32_T dest_vid;
     UI32_T unit;
     UI32_T port;
     UI32_T trunk_id;
     void *hw_info;
     void *nh_hw_info[SYS_ADPT_MAX_NBR_OF_ECMP_ENTRY_PER_ROUTE];
     UI32_T nh_count;
     UI32_T hit;
}__attribute__((packed, aligned(1)))NetRoute_T;

typedef struct HOSTROUTE_S
{
     UI32_T flags;
     UI32_T fib_id;
     union{
         UI32_T ipv4_addr;
         UI8_T  ipv6_addr[SYS_ADPT_IPV6_ADDR_LEN];
     }ip_addr;
     UI8_T  dst_mac[SYS_ADPT_MAC_ADDR_LEN];
     UI8_T  src_mac[SYS_ADPT_MAC_ADDR_LEN];
     UI32_T dst_vid;
     UI32_T unit;
     UI32_T port;
     UI32_T trunk_id;
     UI32_T hit;
     void *hw_info;
}__attribute__((packed, aligned(1)))HostRoute_T;

typedef struct IPMCROUTE_S
{
     UI32_T mcast_ip;
     UI32_T src_ip;
     UI8_T  src_mac[SYS_ADPT_MAC_ADDR_LEN];
     UI32_T src_vid;
     UI32_T dst_vid;
     UI32_T unit;
     UI32_T port;
     UI32_T trunk_id;
     BOOL_T is_trunk;
     BOOL_T tagged_frame;
     BOOL_T check_src_port;
}__attribute__((packed, aligned(1)))IpmcRoute_T;

typedef struct TUNNELINITIATOR_S
{
    UI32_T l3_intf_id;	    /* L3 Interface Id associated with this tunnel */
    UI32_T vid;		    // Vlan ID of the L3 interface with the tunnel attached
    UI8_T src_mac[SYS_ADPT_MAC_ADDR_LEN]; // MAC address of the L3 interface with the tunnel attached
    UI8_T tunnel_type;	    /* Tunnel Type (Manual, 6to4, ISATAP) */
    UI8_T ttl;		    // Tunnel Header TTL
    UI8_T nexthop_mac[SYS_ADPT_MAC_ADDR_LEN]; // Next Hop's MAC address
    L_INET_AddrIp_T sip;    // Tunnel source IPv4 address
    L_INET_AddrIp_T dip;    // Tunnel destination IPv4 address

}__attribute__((packed, aligned(1)))TunnelInitiator_T;

typedef struct TUNNELTERMINATOR_S
{
    I8_T fib_id;	    // FIB ID
    UI8_T tunnel_type;	    // Tunnel Type (Manual, 6to4, ISATAP)
    UI8_T lport[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST]; //portmap for this tunel
    L_INET_AddrIp_T sip;    // Tunnel source IPv4 address with masklen
    L_INET_AddrIp_T dip;    // Tunnel destination IPv4 address with masklen
}__attribute__((packed, aligned(1)))TunnelTerminator_T;

typedef struct HOSTTUNNELROUTE_S
{
     UI32_T flags;
     UI32_T fib_id;
     union{
         UI32_T ipv4_addr;
         UI8_T  ipv6_addr[SYS_ADPT_IPV6_ADDR_LEN];
     }ip_addr;
     UI8_T  dst_mac[SYS_ADPT_MAC_ADDR_LEN];
     UI8_T  src_mac[SYS_ADPT_MAC_ADDR_LEN];
     UI32_T dst_vid;
     UI32_T unit;
     UI32_T port;
     UI32_T trunk_id;
     UI32_T hit;
     void *hw_info;
     TunnelInitiator_T tnl_init;
     TunnelTerminator_T tnl_term;
}__attribute__((packed, aligned(1)))HostTunnelRoute_T;

typedef struct SWDRVL3_TunnelIntfL3_S
{
    UI32_T  l3_intf_id;	    /* L3 Interface Id associated with this tunnel */
    UI16_T  vid;		        // Vlan ID of the L3 interface with the tunnel attached
    UI8_T   src_mac[SYS_ADPT_MAC_ADDR_LEN]; // MAC address of the L3 interface with the tunnel attached
    BOOL_T  is_add;
}__attribute__((packed, aligned(1)))SWDRVL3_TunnelIntfL3_T;

#if (SYS_CPNT_ECMP_BALANCE_MODE == TRUE)
typedef enum
{
   SWDRVL3_ECMP_DST_IP = DEV_SWDRVL3_ECMP_DST_IP,
   SWDRVL3_ECMP_L4_PORT = DEV_SWDRVL3_ECMP_L4_PORT,
} SWDRVL3_Ecmp_Mode_T;
#endif

typedef struct SWDRVL3_RxIscBuf_S
{
    UI8_T    ServiceID;      /* Service ID  */
    UI32_T   unit;           /* stack id (unit number) */
    UI32_T   port;           /* port number */
    union
    {
        UI32_T                  interface_num;
        /*SWDRVL3_RountingInterface_T rif_s;*/
        L3Interface_T           l3_interface;
	    IP_FORWARDING_OP_T	    ip_forwarding_op;
        NetRoute_T              net_route;
        HostRoute_T             host_route;
        IpmcRoute_T             ipmc_route;
	    HostTunnelRoute_T	    host_tunnel_route;
	    TunnelInitiator_T	    tunnel_initiator;
	    TunnelTerminator_T	    tunnel_terminator;
        SWDRVL3_TunnelIntfL3_T  tl3;
#if (SYS_CPNT_ECMP_BALANCE_MODE == TRUE)
        UI32_T                  mode;
#endif
    }__attribute__((packed, aligned(1)))info;

}__attribute__((packed, aligned(1)))SWDRVL3_RxIscBuf_T;

enum VLAN_DEBUG_FLAG_E
{
    SDL3C_DEBUG_FLAG_NONE       = 0x00000000L,
    SDL3C_DEBUG_FLAG_CALLIN     = 0x00000001L,
    SDL3C_DEBUG_FLAG_CALLOUT    = 0x00000002L,
    SDL3C_DEBUG_FLAG_ERRMSG     = 0x00000004L,
    SDL3C_DEBUG_FLAG_ALL        = 0xFFFFFFFFL
};

typedef struct
{
    char   *cmd_str;
    char   *cmd_title;
    char   *cmd_description;
    void    (*cmd_function)(UI8_T *cmd_buf);
} SDL3C_BACKDOOR_CommandStruct_T;

typedef struct
{
    UI32_T  flag;
    char    *flag_string;
} SDL3C_BACKDOOR_DebugStruct_T;

typedef struct
{
    SYSFUN_DECLARE_CSC_ON_SHMEM

    UI32_T                       swdrvl3_task_tid;
    void*                              swdrvl3_tunnel_task_tid;
    L_HASH_ShMem_Desc_T                SWDRVL3_net_route_hash_desc;             /* net route hash descriptor        */
    UI32_T                             SWDRVL3_net_route_hash_sz;
    L_HASH_ShMem_Desc_T                SWDRVL3_host_route_hash_desc;            /* host learn route hash descriptor */
    UI32_T                             SWDRVL3_host_route_hash_sz;
    L_HASH_ShMem_Desc_T                SWDRVL3_host_tunnel_route_hash_desc;     /* host learn tunnel route hash descriptor */
    UI32_T                             SWDRVL3_host_tunnel_route_hash_sz;
    L_SORT_LST_ShMem_List_T            SWDRVL3_net_tunnel_route_sort_lst_desc;  /* a sort list to store tunnel net route */
    UI32_T                             SWDRVL3_net_tunnel_route_sort_lst_sz;
    L_HASH_ShMem_Desc_T                SWDRVL3_tunnel_initiator_hash_desc;      /* tunnel initiator learn hash descriptor */  /* not inited now. Not used ? */
    UI32_T                             SWDRVL3_tunnel_initiator_route_hash_sz;
    L_HASH_ShMem_Desc_T                SWDRVL3_tunnel_terminator_hash_desc;     /* tunnel terminator learn hash descriptor */ /* not inited now. Not used ? */




    SWDRVL3_StackInfo_T          swdrvl3_stack_info;
#if (SWDRVL3_ENABLE_DROP_UNKNOWN_IP == TRUE)
    BOOL_T                       default_route_inited;
#endif
    UI32_T               max_num_process;
    UI32_T               host_added_counter;   /* debug purpose */
    UI32_T               host_deleted_counter; /* debug purpose */
    UI32_T               net_added_counter;    /* debug purpose */
    UI32_T               net_deleted_counter;  /* debug purpose */
#if (SWDRVL3_ENABLE_SYSTEM_PERFORMANCE_COUNTER == TRUE)
    UI32_T               swdrvl3_task_backdoor_tid;
    UI32_T               swdrvl3_backdoor_task_tmid;
    UI32_T               system_performance_counters_index;
    BOOL_T               system_counter_enable;
    UI32_T               system_performance_counters[200][15];
    UI32_T               host_added_in_chip_counter;   /* debug purpose */

    UI32_T               process_counter1;   /* debug purpose */
    UI32_T               process_counter2;   /* debug purpose */
    UI32_T               system_ticker1_1;
    UI32_T               system_ticker1_2;
    UI32_T               system_ticker2_1;
    UI32_T               system_ticker2_2;
    UI32_T               system_tick_usage_index;
    UI32_T               system_tick_usage[200][8];
    BOOL_T               system_ticker_enable;
#endif
#if (SWDRVL3_ENABLE_AUTO_ADD_HOST_ROUTES == TRUE)
    UI32_T               swdrvl3_task_backdoora_tid;
    UI32_T               swdrvl3_backdoora_task_tmid;
    BOOL_T               auto_host_route_enable;
#endif
	UI32_T  			 SDL3C_DebugFlag;
}SWDRVL3_ShmemData_T;
#endif
