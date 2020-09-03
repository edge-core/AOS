/* MODULE NAME:  nsm_type.h
 * PURPOSE:
 *     Define common types used in nsm.
 *
 * NOTES:
 *
 * HISTORY
 *    25/6/2007 - Charlie Chen, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */
#ifndef NSM_TYPE_H
#define NSM_TYPE_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_adpt.h"
#include "leaf_2096.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define NSM_TYPE_ROUTE_FLAG_INTERNAL         0x01
#define NSM_TYPE_ROUTE_SELFROUTE             0x02
#define NSM_TYPE_ROUTE_BLACKHOLE             0x04
#define NSM_TYPE_ROUTE_NON_FIB               0x08
#define NSM_TYPE_ROUTE_SELECTED              0x10
#define NSM_TYPE_ROUTE_CHANGED               0x20
#define NSM_TYPE_ROUTE_STATIC                0x40
#define NSM_TYPE_ROUTE_STALE                 0x80


#define NSM_TYPE_NEXTHOP_FLAG_ACTIVE     (1 << 0) /* This nexthop is alive. */
#define NSM_TYPE_NEXTHOP_FLAG_FIB        (1 << 1) /* FIB nexthop. */
#define NSM_TYPE_NEXTHOP_FLAG_RECURSIVE  (1 << 2) /* Recursive nexthop. */
#define NSM_TYPE_NEXTHOP_FLAG_MROUTE     (1 << 3) /* Multicast route nexthop. */

#define NSM_TYPE_NEXTHOP_TYPE_IFINDEX        1 /* Directly connected. */
#define NSM_TYPE_NEXTHOP_TYPE_IFNAME         2 /* Interface route. */
#define NSM_TYPE_NEXTHOP_TYPE_IPV4           3 /* IPv4 nexthop. */
#define NSM_TYPE_NEXTHOP_TYPE_IPV4_IFINDEX   4 /* IPv4 nexthop with ifindex. */
#define NSM_TYPE_NEXTHOP_TYPE_IPV4_IFNAME    5 /* IPv4 nexthop with ifname. */
#define NSM_TYPE_NEXTHOP_TYPE_IPV6           6 /* IPv6 nexthop. */
#define NSM_TYPE_NEXTHOP_TYPE_IPV6_IFINDEX   7 /* IPv6 nexthop with ifindex. */
#define NSM_TYPE_NEXTHOP_TYPE_IPV6_IFNAME    8 /* IPv6 nexthop with ifname. */
#define NSM_TYPE_NEXTHOP_TYPE_MPLS           9 /* pass to MPLS forwarder */

#define NSM_TYPE_NEXTHOP_IFNAME_SIZE         31


/* defines for route types */
#define NSM_TYPE_SNMP_ROUTE_MIN     (VAL_ipCidrRouteType_other - 1)
#define NSM_TYPE_SNMP_ROUTE_OTHER   VAL_ipCidrRouteType_other
#define NSM_TYPE_SNMP_ROUTE_REJECT  VAL_ipCidrRouteType_reject
#define NSM_TYPE_SNMP_ROUTE_LOCAL   VAL_ipCidrRouteType_local
#define NSM_TYPE_SNMP_ROUTE_REMOTE  VAL_ipCidrRouteType_remote
#define NSM_TYPE_SNMP_ROUTE_MAX     (VAL_ipCidrRouteType_remote + 1)
#define NSM_TYPE_SNM_ROUTE_UNKNOWN  NSM_TYPE_SNMP_ROUTE_MIN

/* defines for route protocol */
#define NSM_TYPE_SNMP_PROTO_OTHER                   VAL_ipCidrRouteProto_other
#define NSM_TYPE_SNMP_PROTO_LOCAL                   VAL_ipCidrRouteProto_local
#define NSM_TYPE_SNMP_PROTO_NETMGMT                 VAL_ipCidrRouteProto_netmgmt
#define NSM_TYPE_SNMP_PROTO_ICMP                    VAL_ipCidrRouteProto_icmp
#define NSM_TYPE_SNMP_PROTO_EGP                     VAL_ipCidrRouteProto_egp
#define NSM_TYPE_SNMP_PROTO_GGP                     VAL_ipCidrRouteProto_ggp
#define NSM_TYPE_SNMP_PROTO_HELLO                   VAL_ipCidrRouteProto_hello
#define NSM_TYPE_SNMP_PROTO_RIP                     VAL_ipCidrRouteProto_rip
#define NSM_TYPE_SNMP_PROTO_ISIS                    VAL_ipCidrRouteProto_isIs
#define NSM_TYPE_SNMP_PROTO_ESIS                    VAL_ipCidrRouteProto_esIs
#define NSM_TYPE_SNMP_PROTO_CISCOIGRP               VAL_ipCidrRouteProto_ciscoIgrp
#define NSM_TYPE_SNMP_PROTO_BBNSPFIGP               VAL_ipCidrRouteProto_bbnSpfIgp
#define NSM_TYPE_SNMP_PROTO_OSPF                    VAL_ipCidrRouteProto_ospf
#define NSM_TYPE_SNMP_PROTO_BGP                     VAL_ipCidrRouteProto_bgp
#define NSM_TYPE_SNMP_PROTO_IDPR                    VAL_ipCidrRouteProto_idpr
#define NSM_TYPE_SNMP_PROTO_CISCOEIGRP              VAL_ipCidrRouteProto_ciscoEigrp
#define NSM_TYPE_SNMP_PROTO_UNKNOWN                 (NSM_TYPE_SNMP_PROTO_OTHER - 1)


/* SNMP Set Field Mask */
#define NSM_TYPE_SNMP_SET_IPCIDRTABLE_METRIC        (1 << 0)
#define NSM_TYPE_SNMP_SET_IPCIDRTABLE_ROUTETYPE     (1 << 1)
#define NSM_TYPE_SNMP_SET_IPCIDRTABLE_IFINDEX       (1 << 2)
#define NSM_TYPE_SNMP_SET_IPCIDRTABLE_ROWSTATUS     (1 << 3)

/**********************************
 ** definitions for return value **
 **********************************
 */
enum
{
    NSM_TYPE_RESULT_OK=0,
    NSM_TYPE_RESULT_FAIL,
    NSM_TYPE_RESULT_INVALID_COMMAND,
    NSM_TYPE_RESULT_INVALID_ARG,
    NSM_TYPE_RESULT_VR_LOOKUP_FAIL,
    NSM_TYPE_RESULT_VRF_LOOKUP_FAIL,
    NSM_TYPE_RESULT_VRF_NOT_EXIST,
    NSM_TYPE_RESULT_ENTRY_NOT_EXIST,
    NSM_TYPE_RESULT_SEND_MSG_FAIL,
    NSM_TYPE_RESULT_EOF,
    NSM_TYPE_RESILT_ADDRESS_OVERLAPPED,
    NSM_TYPE_RESULT_CANT_SET_ADDRESS_WITH_ZERO_IFINDEX,
    NSM_TYPE_RESULT_IF_NOT_EXIST,
    NSM_TYPE_RESULT_MASTER_NOT_EXIST,
    NSM_TYPE_RESULT_CANT_CHANGE_PRIMARY,
    NSM_TYPE_RESULT_CANT_CHANGE_SECONDARY,
    NSM_TYPE_RESULT_SAME_ADDRESS_EXIST,
    NSM_TYPE_RESULT_ADDRESS_NOT_EXIST,
    NSM_TYPE_RESULT_MUST_DELETE_SECONDARY_FIRST,
    NSM_TYPE_RESULT_ADDRESS_OVERLAPPED,
    NSM_TYPE_RESULT_CANT_SET_SECONDARY_FIRST,
    NSM_TYPE_RESULT_MALFORMED_GATEWAY,
    NSM_TYPE_RESULT_FLAG_UP_CANT_SET,
    NSM_TYPE_RESULT_FLAG_UP_CANT_UNSET,
    NSM_TYPE_RESULT_TUNNEL_DEST_MUST_UNCONFIGURED,
    NSM_TYPE_RESULT_INSSUFICIENT_BUFFER_SIZE,
    NSM_TYPE_RESULT_UNKNOWN_ERR
};

 typedef enum
{
   NSM_TYPE_ND_ROUTER_PERFERENCE_HIGH = 1,
   NSM_TYPE_ND_ROUTER_PERFERENCE_MEDIUM=0,
   NSM_TYPE_ND_ROUTER_PERFERENCE_LOW=3
} NSM_PMGR_RA_ROUTER_PREFERENCE_T;
#if (SYS_CPNT_IP_TUNNEL == TRUE)
/*from nsm tunnel.h
#define TUNNEL_MODE_NONE            0
#define TUNNEL_MODE_IPIP            1
#define TUNNEL_MODE_GREIP            2
#define TUNNEL_MODE_IPV6IP            3
#define TUNNEL_MODE_IPV6IP_6TO4            4
#define TUNNEL_MODE_IPV6IP_ISATAP        5
#define TUNNEL_MODE_MAX                6
*/
#define NSM_TYPE_TUNNEL_MODE_NONE 0
#define NSM_TYPE_TUNNEL_MODE_IPIP            1
#define NSM_TYPE_TUNNEL_MODE_GREIP            2
#define NSM_TYPE_TUNNEL_MODE_IPV6IP            3
#define NSM_TYPE_TUNNEL_MODE_IPV6IP_6TO4            4
#define NSM_TYPE_TUNNEL_MODE_IPV6IP_ISATAP        5
#define NSM_TYPE_TUNNEL_MODE_MAX                6
#endif

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
/*  ipCidrRouteEntry    : RFC 2096 */
typedef struct NSM_TYPE_IpCidrRouteEntry_S
{
    UI8_T  ip_cidr_route_dest[SYS_ADPT_IPV4_ADDR_LEN];      /*  ipCidrRouteDest : IpAddress */
    UI8_T  ip_cidr_route_mask[SYS_ADPT_IPV4_ADDR_LEN];      /*  ipCidrRouteMask : IpAddress */
    I32_T   ip_cidr_route_tos;                              /*  ipCidrRouteTos              */
    UI8_T  ip_cidr_route_next_hop[SYS_ADPT_IPV4_ADDR_LEN];  /*  ipCidrRouteNextHop;         */
    I32_T   ip_cidr_route_if_index;                         /*  ipCidrRouteIfIndex;         */
    /*  ipCidrRouteType Valid : 1..4
     *      VAL_ipCidrRouteType_other    (1), -- not specified by this MIB
     *      VAL_ipCidrRouteType_reject   (2), -- route which discards traffic
     *      VAL_ipCidrRouteType_local    (3), -- local interface
     *      VAL_ipCidrRouteType_remote   (4)  -- remote destination
     */
    int     ip_cidr_route_type;                             /*  ipCidrRouteType;    */
    /*  ipCidrRouteProto    Valid : 1..16
     *      VAL_ipCidrRouteProto_other     (1),  -- not specified
     *      VAL_ipCidrRouteProto_local     (2),  -- local interface
     *      VAL_ipCidrRouteProto_netmgmt   (3),  -- static route
     *      VAL_ipCidrRouteProto_icmp      (4),  -- result of ICMP Redirect

                -- the following are all dynamic
                -- routing protocols

     *      VAL_ipCidrRouteProto_egp        (5),  -- Exterior Gateway Protocol
     *      VAL_ipCidrRouteProto_ggp        (6),  -- Gateway-Gateway Protocol
     *      VAL_ipCidrRouteProto_hello      (7),  -- FuzzBall HelloSpeak
     *      VAL_ipCidrRouteProto_rip        (8),  -- Berkeley RIP or RIP-II
     *      VAL_ipCidrRouteProto_isIs       (9),  -- Dual IS-IS
     *      VAL_ipCidrRouteProto_esIs       (10), -- ISO 9542
     *      VAL_ipCidrRouteProto_ciscoIgrp  (11), -- Cisco IGRP
     *      VAL_ipCidrRouteProto_bbnSpfIgp  (12), -- BBN SPF IGP
     *      VAL_ipCidrRouteProto_ospf       (13), -- Open Shortest Path First
     *      VAL_ipCidrRouteProto_bgp        (14), -- Border Gateway Protocol
     *      VAL_ipCidrRouteProto_idpr       (15), -- InterDomain Policy Routing
     *      VAL_ipCidrRouteProto_ciscoEigrp (16)  -- Cisco EIGRP
     */
    int     ip_cidr_route_proto;                            /*  ipCidrRouteProto;   */
    I32_T   ip_cidr_route_age;                              /*  ipCidrRouteAge      */
    /*  ipCidrRouteInfo     OBJECT IDENTIFIER,      */
    I32_T   ip_cidr_route_nextHopAS;                        /*  ipCidrRouteNextHopAS;       */
    I32_T   ip_cidr_route_metric1;                          /*   ipCidrRouteMetric1;         */
    I32_T   ip_cidr_route_metric2;                          /*   ipCidrRouteMetric2;         */
    I32_T   ip_cidr_route_metric3;                          /*   ipCidrRouteMetric3;         */
    I32_T   ip_cidr_route_metric4;                          /*   ipCidrRouteMetric4;         */
    I32_T   ip_cidr_route_metric5;                          /*   ipCidrRouteMetric5;         */
    I32_T   ip_cidr_route_ext_subtype;                      /*   Private MIB, for recording Protocl sub_type */
    UI32_T  ip_cidr_route_status;                           /*   ipCidrRouteStatus;         */

    UI32_T  ip_cidr_route_set_mask;                         /*   The mask to indicate SNMP set field */
}NSM_TYPE_IpCidrRouteEntry_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

#endif    /* End of NSM_TYPE_H */

