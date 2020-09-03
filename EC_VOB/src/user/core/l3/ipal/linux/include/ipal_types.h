/*
 *   File Name: ipal_types.h
 *   Purpose:   TCP/IP shim layer(ipal) data type define
 *   Note:
 *   Create:    Basen LV     2008.04.06
 *
 *   Histrory:
 *              Modify		   Date      Reason
 *
 *
 *   Copyright(C)  Accton Corporation 2007~2009
 */

#ifndef __IPAL_TYPES_H
#define __IPAL_TYPES_H

/*
 * INCLUDE FILE DECLARATIONS
 */
#include "sys_adpt.h"
#include "l_inet.h"
#include "l_prefix.h"
#ifndef __KERNEL__
#include <netinet/in.h>
#else
#include <linux/in.h>
#endif
#include <linux/if_addr.h>

/*
 * NAMING CONST DECLARATIONS
 */

/*
 * MACRO FUNCTION DECLARATIONS
 */
#define IPAL_RESULT_OK              0x0
#define IPAL_RESULT_ENTRY_EXIST     0x1
#define IPAL_RESULT_FAIL            0xFFFFFFFF

#ifndef IFNAMSIZ
#define IFNAMSIZ  16
#endif

/*
 * DATA TYPE DECLARATIONS
 */
#define IPAL_SCOPE_GLOBAL   RT_SCOPE_UNIVERSE
#define IPAL_SCOPE_SITE     RT_SCOPE_SITE
#define IPAL_SCOPE_LINK     RT_SCOPE_LINK
#define IPAL_SCOPE_HOST     RT_SCOPE_HOST
#define IPAL_SCOPE_NOWHERE  RT_SCOPE_NOWHERE

/* RIB message flags (refer to pal_types.def) */
#define IPAL_RIB_FLAG_INTERNAL           0x01
#define IPAL_RIB_FLAG_SELFROUTE          0x02
#define IPAL_RIB_FLAG_BLACKHOLE          0x04
#define IPAL_RIB_FLAG_NON_FIB            0x08
#define IPAL_RIB_FLAG_SELECTED           0x10
#define IPAL_RIB_FLAG_CHANGED            0x20
#define IPAL_RIB_FLAG_STATIC             0x40
#define IPAL_RIB_FLAG_STALE              0x80

/* ifa_flags, refer to if_addr.h */
#define IPAL_IFA_F_SECONDARY        IFA_F_SECONDARY
#define IPAL_IFA_F_TEMPORARY        IFA_F_SECONDARY
#define	IPAL_IFA_F_NODAD            IFA_F_NODAD
#define	IPAL_IFA_F_HOMEADDRESS      IFA_F_HOMEADDRESS
#define IPAL_IFA_F_DEPRECATED       IFA_F_DEPRECATED
#define IPAL_IFA_F_TENTATIVE        IFA_F_TENTATIVE
#define IPAL_IFA_F_PERMANENT        IFA_F_PERMANENT

/* for trace_id of user_id when allocate buffer with l_mm
 */
enum
{
    IPAL_TYPE_TRACE_ID_IPAL_RT_NETLINK_PARSE_INFO = 0,
};

/* return value definition
 */
enum
{
    IPAL_TYPE_RETVAL_OK,
    IPAL_TYPE_RETVAL_INVALID_ARG,
    IPAL_TYPE_RETVAL_CREATE_LOOPBACK_DEV_FAIL,
    IPAL_TYPE_RETVAL_LOOPBACK_DEV_NOT_EXISTS,
    IPAL_TYPE_RETVAL_UNKNOWN_SYSCALL_CMD,
    IPAL_TYPE_RETVAL_UNKNOWN_ERROR
};

/* loopback interface system call command definition
 */
enum
{
    IPAL_TYPE_SYSCALL_CMD_CREATE_LOOPBACK_DEV,
    IPAL_TYPE_SYSCALL_CMD_DESTROY_LOOPBACK_DEV
};

typedef struct IPAL_NeighborEntry_S
{
    UI32_T          ifindex;
    L_INET_AddrIp_T ip_addr;
    UI32_T          phy_address_len;
    UI8_T           phy_address[SYS_ADPT_MAC_ADDR_LEN];
    UI32_T          state;   /* bitmap, IPAL_NEIGH_NUD_XXX, define in ipal_neigh.h */
    UI32_T          last_update;   /* time since last update */
}IPAL_NeighborEntry_T;

typedef struct IPAL_NeighborEntryIpv4_S
{
    UI32_T  ifindex;
    UI8_T   ip_addr[4];
    UI32_T  phy_address_len;
    UI8_T   phy_address[SYS_ADPT_MAC_ADDR_LEN];
    UI32_T  state;   /* bitmap, IPAL_NEIGH_NUD_XXX, define in ipal_neigh.h */
    UI32_T          last_update;   /* time since last update */
}IPAL_NeighborEntryIpv4_T;

typedef struct IPAL_NeighborListIpv4_S
{
    IPAL_NeighborEntryIpv4_T *neigh_entry;
    struct IPAL_NeighborListIpv4_S *neigh_list_next;
}IPAL_NeighborListIpv4_T;

typedef struct IPAL_NeighborEntryIpv6_S
{
    UI32_T  ifindex;
    UI8_T   ip_addr[16];
    UI32_T  phy_address_len;
    UI8_T   phy_address[SYS_ADPT_MAC_ADDR_LEN];
    UI32_T  state;   /* bitmap, IPAL_NEIGH_NUD_XXX, define in ipal_neigh.h */
	UI32_T  last_update;   /* time since last update */
}IPAL_NeighborEntryIpv6_T;

typedef struct IPAL_NeighborListIpv6_S
{
    IPAL_NeighborEntryIpv6_T *neigh_entry;
    struct IPAL_NeighborListIpv6_S *neigh_list_next;
}IPAL_NeighborListIpv6_T;

typedef struct IPAL_Ipv4AddressEntry_S
{
    UI32_T  ifindex;
    UI32_T  valid_lft;
    UI32_T  preferred_lft;
    UI32_T  ifa_flags;
    UI32_T  prefixlen;
    UI32_T  scope;
    UI8_T   addr[SYS_ADPT_IPV4_ADDR_LEN];
}IPAL_Ipv4AddressEntry_T;

typedef struct IPAL_Ipv4AddressList_S
{
    IPAL_Ipv4AddressEntry_T         addr_entry;
    struct IPAL_Ipv4AddressList_S   *next_p;
}IPAL_Ipv4AddressList_T;

typedef struct IPAL_Ipv6AddressEntry_S
{
    UI32_T  ifindex;
    UI32_T  valid_lft;
    UI32_T  preferred_lft;
    UI32_T  ifa_flags;
    UI32_T  prefixlen;
    UI32_T  scope;
    UI8_T   addr[SYS_ADPT_IPV6_ADDR_LEN];
}IPAL_Ipv6AddressEntry_T;

typedef struct IPAL_Ipv6AddressList_S
{
    IPAL_Ipv6AddressEntry_T         addr_entry;
    struct IPAL_Ipv6AddressList_S   *next_p;
}IPAL_Ipv6AddressList_T;

typedef struct IPAL_IpAddressInfoEntry_S
{
    UI32_T  ifindex;
    UI32_T  valid_lft;     /* in seconds, 0xFFFFFFFF = forever */
    UI32_T  preferred_lft; /* in seconds, 0xFFFFFFFF = forever */
    UI32_T  scope;
    BOOL_T  tentative;
    BOOL_T  deprecated;
    BOOL_T  permanent;
    L_INET_AddrIp_T ipaddr;
}IPAL_IpAddressInfoEntry_T;

#if 0
typedef struct IPAL_IPv4_ARP_S
{
    UI32_T ifindex;
    UI32_T ip_addr;

    UI32_T phy_address_len;
    UI8_T phy_address[SYS_ADPT_MAC_ADDR_LEN];

    UI32_T state;   /* define in neighbour.h */
}IPAL_IPv4_ARP_T;

typedef struct IPAL_IPv4_ARP_LIST_S
{
    IPAL_IPv4_ARP_T *arp_entry;
    struct IPAL_IPv4_ARP_LIST_S *arp_list_next;
}IPAL_IPv4_ARP_LIST_T;
#endif

typedef struct IPAL_Ipv4UcNextHop_S
{
    UI32_T flags;
    UI32_T egressIfindex;
    UI32_T nexthopCount;
    UI32_T mtu;
    I32_T expires;
    struct in_addr nexthopIP;
}IPAL_Ipv4UcNextHop_T;

#if (SYS_CPNT_IPV6 == TRUE)
typedef struct IPAL_Ipv6UcNextHop_S
{
    UI32_T flags;
    UI32_T egressIfindex;
    UI32_T nexthopCount;
    UI32_T mtu;
    I32_T expires;
    struct in6_addr nexthopIP;
}IPAL_Ipv6UcNextHop_T;
#endif

typedef union IPAL_IpUcNextHop_S
{
    union
    {
        IPAL_Ipv4UcNextHop_T ipv4_nh;
#if (SYS_CPNT_IPV6 == TRUE)
        IPAL_Ipv6UcNextHop_T ipv6_nh;
#endif
    } u;
}IPAL_IpUcNextHop_T;

typedef struct IPAL_Ipv4UcRouteEntry_S
{
    UI32_T table_id;
	L_PREFIX_T dst_p;
	IPAL_IpUcNextHop_T  nh;
}IPAL_Ipv4UcRouteEntry_T;

typedef struct IPAL_Ipv4UcRouteList_S
{
    IPAL_Ipv4UcRouteEntry_T       route_entry;
    struct IPAL_Ipv4UcRouteList_S *next_p;
}IPAL_Ipv4UcRouteList_T;

typedef struct IPAL_Ipv6UcRouteEntry_S
{
    UI32_T table_id;
	L_PREFIX_T dst_p;
	IPAL_IpUcNextHop_T  nh;
}IPAL_Ipv6UcRouteEntry_T;

typedef struct IPAL_Ipv6UcRouteList_S
{
    IPAL_Ipv6UcRouteEntry_T       route_entry;
    struct IPAL_Ipv6UcRouteList_S *next_p;
}IPAL_Ipv6UcRouteList_T;

typedef struct IPAL_RouteIfInfo_S
{
    L_INET_AddrIp_T nexthop;
    L_INET_AddrIp_T src;
    UI32_T          ifindex;
} IPAL_RouteIfInfo_T;

typedef struct IPAL_IfInfo_S
{
	UI32_T  ifindex;
	UI32_T  mtu;
//	struct  in_addr ipaddr;
	UI8_T   ifname[IFNAMSIZ];
	UI8_T   hw_addr[SYS_ADPT_MAC_ADDR_LEN];
    UI32_T  inet6_flags; /* IFLA_PROTINFO ->IFLA_INET6_FLAGS */
}IPAL_IfInfo_T;

typedef struct IPAL_IfInfoList_S
{
    IPAL_IfInfo_T            if_entry;
    struct IPAL_IfInfoList_S *next_p;
}IPAL_IfInfoList_T;

typedef struct IPAL_PmtuEntry_S
{
    UI32_T          pmtu;
    L_INET_AddrIp_T destip;
    UI32_T          since_time; /* in seconds */
} IPAL_PmtuEntry_T;

typedef struct IPAL_Ipv4ArpStatistics_S
{
    UI32_T      in_request;
    UI32_T      in_reply;
    UI32_T      out_request;
    UI32_T      out_reply;
} IPAL_Ipv4ArpStatistics_T;

typedef struct IPAL_Ipv6Statistics_S
{
    UI32_T  Ip6InReceives;
    UI32_T  Ip6InHdrErrors;
    UI32_T  Ip6InTooBigErrors;
    UI32_T  Ip6InNoRoutes;
    UI32_T  Ip6InAddrErrors;
    UI32_T  Ip6InUnknownProtos;
    UI32_T  Ip6InTruncatedPkts;
    UI32_T  Ip6InDiscards;
    UI32_T  Ip6InDelivers;
    UI32_T  Ip6OutForwDatagrams;
    UI32_T  Ip6OutRequests;
    UI32_T  Ip6OutDiscards;
    UI32_T  Ip6OutNoRoutes;
    UI32_T  Ip6ReasmTimeout;
    UI32_T  Ip6ReasmReqds;
    UI32_T  Ip6ReasmOKs;
    UI32_T  Ip6ReasmFails;
    UI32_T  Ip6FragOKs;
    UI32_T  Ip6FragFails;
    UI32_T  Ip6FragCreates;
    UI32_T  Ip6InMcastPkts;
    UI32_T  Ip6OutMcastPkts;
} IPAL_Ipv6Statistics_T;

typedef struct IPAL_Icmpv6Statistics_S
{
    UI32_T  Icmp6InMsgs;
    UI32_T  Icmp6InErrors;
    UI32_T  Icmp6OutMsgs;
    UI32_T  Icmp6InDestUnreachs;
    UI32_T  Icmp6InPktTooBigs;
    UI32_T  Icmp6InTimeExcds;
    UI32_T  Icmp6InParmProblems;
    UI32_T  Icmp6InEchos;
    UI32_T  Icmp6InEchoReplies;
    UI32_T  Icmp6InGroupMembQueries;
    UI32_T  Icmp6InGroupMembResponses;
    UI32_T  Icmp6InGroupMembReductions;
    UI32_T  Icmp6InRouterSolicits;
    UI32_T  Icmp6InRouterAdvertisements;
    UI32_T  Icmp6InNeighborSolicits;
    UI32_T  Icmp6InNeighborAdvertisements;
    UI32_T  Icmp6InRedirects;
    UI32_T  Icmp6InMLDv2Reports;
    UI32_T  Icmp6OutDestUnreachs;
    UI32_T  Icmp6OutPktTooBigs;
    UI32_T  Icmp6OutTimeExcds;
    UI32_T  Icmp6OutParmProblems;
    UI32_T  Icmp6OutEchos;
    UI32_T  Icmp6OutEchoReplies;
    UI32_T  Icmp6OutGroupMembQueries;
    UI32_T  Icmp6OutGroupMembResponses;
    UI32_T  Icmp6OutGroupMembReductions;
    UI32_T  Icmp6OutRouterSolicits;
    UI32_T  Icmp6OutRouterAdvertisements;
    UI32_T  Icmp6OutNeighborSolicits;
    UI32_T  Icmp6OutNeighborAdvertisements;
    UI32_T  Icmp6OutRedirects;
    UI32_T  Icmp6OutMLDv2Reports;
} IPAL_Icmpv6Statistics_T;

typedef struct IPAL_Udpv6Statistics_S {
    UI32_T  Udp6InDatagrams;
    UI32_T  Udp6NoPorts;
    UI32_T  Udp6InErrors;
    UI32_T  Udp6OutDatagrams;
} IPAL_Udpv6Statistics_T;

#endif	/*end of __IPAL_TYPES_H*/


