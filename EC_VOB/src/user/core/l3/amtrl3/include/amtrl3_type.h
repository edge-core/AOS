/* =====================================================================================*
 * FILE NAME: AMTRL3_TYPE.h                                                               *
 *                                                                                      *
 * ABSTRACT:  This file contains the defined datatypes for AMTRL3.
 *                                                                                      *
 * MODIFICATION HISOTRY:                                                                *
 *                                                                                      *
 * MODIFIER        DATE        DESCRIPTION                                              *
 * -------------------------------------------------------------------------------------*
 * djd          01-29-2008     First Create                                             *
 *                                                                                      *
 * -------------------------------------------------------------------------------------*
 * Copyright(C)        Accton Techonology Corporation 2008                              *
 * =====================================================================================*/

#ifndef _AMTRL3_TYPE_H
#define _AMTRL3_TYPE_H

#include "sys_type.h"
#include "sys_adpt.h"
#include "sysfun.h"
#include "l_inet.h"

/* NAME CONSTANT DECLARATIONS
 */


/* MACRO FUNCTION DECLARATIONS
 */

/* Operation on ipv4 entries */
#define    AMTRL3_TYPE_FLAGS_IPV4                1
/* Operation on ipv6 entries */
#define    AMTRL3_TYPE_FLAGS_IPV6                (1 << 1)
/* Operation on ECMP entries, mutually exclusive with AMTRL3_TYPE_FLAGS_WCMP */
#define    AMTRL3_TYPE_FLAGS_ECMP                (1 << 2)
/* Operation on WCMP entries */
#define    AMTRL3_TYPE_FLAGS_WCMP                (1 << 3)
/* Work Around flag that indicates HOT INSERTION processed entry */
#define    AMTRL3_TYPE_FLAGS_WA_HOTINSERT_ECMP   (1 << 4)
/* Indicates whether the override flag of Neighbor Advertisement is on */
#define    AMTRL3_TYPE_FLAGS_NOT_OVERRIDE        (1 << 5)

#define    AMTRL3_TYPE_SUCCESS                    0
#define    AMTRL3_TYPE_FAIL                       1
#define    AMTRL3_TYPE_FIB_SUCCESS                0
#define    AMTRL3_TYPE_FIB_FAIL                   1
#define    AMTRL3_TYPE_FIB_ALREADY_EXIST          2
#define    AMTRL3_TYPE_FIB_NOT_EXIST              3
#define    AMTRL3_TYPE_FIB_OM_ERROR               4


#define AMTRL3_TYPE_INETTOPHYSICALEXTTYPE_TUNNEL_ISATAP  104/*VAL_ipNetToPhysicalExtType_tunnel_isatap ?*/
#define AMTRL3_TYPE_INETTOPHYSICALEXTTYPE_TUNNEL_6TO4   105/*VAL_ipNetToPhysicalExtType_tunnel_6to4    ?*/
#define AMTRL3_TYPE_INETTOPHYSICALEXTTYPE_TUNNEL_MANUAL  106/*VAL_ipNetToPhysicalExtType_tunnel_ip6manual?*/
#define AMTRL3_TYPE_INETTOPHYSICALEXTTYPE_TUNNEL_VXLAN   107

/* djd: temp define */
//  typedef struct IpAddr_S{
//      UI32_T  address_type;
//      union{
//      UI8_T   ipv4 [4];
//      UI8_T   ipv6 [16];
//      }addr;
//      UI32_T  zone_index;
//  }IpAddr_T;


typedef struct AMTRL3_TYPE_PhysAddress_S
{
    UI16_T  phy_address_type;           /*  PhyAddress type               */
    UI16_T  phy_address_len;            /*  PhyAddress physical length    */
                                        /*  Buffer to keep PhyAddress     */
    UI8_T   phy_address_octet_string[SYS_ADPT_MAC_ADDR_LEN];
}   AMTRL3_TYPE_PhysAddress_T;


/*  ipNetToPhysicalTable : RFC 2011 update to support IPv6
 *
 *  ipNetToPhysicalEntry OBJECT-TYPE
 *      SYNTAX     IpNetToPhysicalEntry
 *      MAX-ACCESS not-accessible
 *      STATUS     current
 *      DESCRIPTION
 *                "Each entry contains one IP address to `physical' address
 *                 equivalence."
 *      INDEX     { ipNetToPhysicalIfIndex,
 *                  ipNetToPhysicalNetAddressType,
 *                  ipNetToPhysicalNetAddress }
 *      ::= { ipNetToPhysicalTable 1 }
 *
 *
 *  IpNetToPhysicalEntry ::= SEQUENCE {
 *       ipNetToPhysicalIfIndex         InterfaceIndex,
 *       ipNetToPhysicalNetAddressType  InetAddressType,
 *       ipNetToPhysicalNetAddress      InetAddress,
 *       ipNetToPhysicalPhysAddress     PhysAddress,
 *       ipNetToPhysicalLastUpdated     TimeStamp,   : not used in AMTRL3
 *       ipNetToPhysicalType            INTEGER,
 *       ipNetToPhysicalState           INTEGER,     : not used in AMTRL3
 *       ipNetToPhysicalRowStatus       RowStatus    : not used in AMTRL3
 *   }
 */

typedef struct AMTRL3_TYPE_ipNetToPhysicalEntry_S
{
    UI32_T                          ip_net_to_physical_if_index;          /* 1st KEY   */
    L_INET_AddrIp_T		            ip_net_to_physical_net_address;       /* 2nd KEY */
    AMTRL3_TYPE_PhysAddress_T       ip_net_to_physical_phys_address;
    UI32_T                          ip_net_to_physical_type;
}AMTRL3_TYPE_ipNetToPhysicalEntry_T;

/* Operational Data Structure used in all AMTRL3 component <HostRoute>
 * Hisam will handle key mapping
 * Design concept for Host route entry:
 *
 *    dst_inet_addr      - inet address of this host entry. Must be global unique.
 *                         for ipv6 link-local address must include scope id.
 *    lport              - lport index this entry is associated with.
 *    uport              - uport index this entry is associated with.
 *    dst_vid_ifindex    - vlan index of host entry.
 *    entry_type         - indicates whether this entry is of local, dynamic, hsrp, vrrp type
 *    ref_count          - Indicates the number of net route entry with nhop associated with
 *                         this host entry
 *    arp_interval_index - A counter to indicate the time this entry shall wait to retransmit
 *                         ARP request packet
 *    hit_timestamp      - Reference ARP ageout timer in seconds to determine age out mechanism
 *                         for this entry
 *    last_arp_timestamp - A timestamp to record the last time ARP Request is transmitted for
 *                         this entry.
 *    in_chip_status     - TRUE for host entry exist in chip. False for host entry not in chip
 *                         due to lack of information
 *    dst_mac            - next hop Mac Address
 *    flags              - flags to indicate whether ECMP...
 *    old_status         - previous status of this host route entry
 *    status             - Current status of this host route entry
 *
 */
typedef struct AMTRL3_TYPE_HostRoutePartialEntry_S
{
    UI32_T          fib_id;
    UI32_T          dst_vid_ifindex;
    L_INET_AddrIp_T dst_inet_addr;
    UI32_T          lport;
    UI8_T           dst_mac[SYS_ADPT_MAC_ADDR_LEN];
    /* For some reasons, we add trunk_id as the member of this structure.
     * When we add a normal port to a trunk or delete a port from a trunk,
     * it should delete all ECMP routes which related to this port.
     * But the old intformation(whether it is a trunk port) cannot be found,
     * and the routes cannot be deleted if this information is missing.
     * So we add trunk_id to keep old information in OM.
     */
    UI32_T          trunk_id;       /* if port is trunk port  */

#if (SYS_CPNT_PBR == TRUE)
    void*           hw_info;
#endif

    /* The following for ip tunnel */
#if (SYS_CPNT_IP_TUNNEL == TRUE)
    UI8_T tunnel_entry_type;
    union
    {
#if (SYS_CPNT_IP_TUNNEL == TRUE)
        struct
        {
            UI32_T           src_vidifindex;
            L_INET_AddrIp_T  src_inet_addr;
            UI32_T           nexthop_vidifindex;
            L_INET_AddrIp_T  nexthop_inet_addr;
            L_INET_AddrIp_T  dest_inet_addr;
        } ip_tunnel;
#endif
    } u;
#endif

}AMTRL3_TYPE_HostRoutePartialEntry_T;


typedef AMTRL3_TYPE_HostRoutePartialEntry_T AMTRL3_TYPE_InetHostRouteEntry_T;


/* inetCidrRouteEntry    : RFC 2096 (IP Forwarding Table MIB) Jan-1997
 * Hisam will handle key mapping
 */
typedef struct AMTRL3_TYPE_InetCidrRoutePartialEntry_S
{
    UI32_T          fib_id;
    L_INET_AddrIp_T inet_cidr_route_dest;
    UI32_T          inet_cidr_route_pfxlen;
    UI32_T          inet_cidr_route_policy[SYS_ADPT_NUMBER_OF_INET_CIDR_ROUTE_POLICY_SUBIDENTIFIER];
    L_INET_AddrIp_T inet_cidr_route_next_hop;
    UI32_T          inet_cidr_route_if_index;
    I32_T           inet_cidr_route_type;
    I32_T           inet_cidr_route_proto;
    UI32_T          inet_cidr_route_age;
    UI32_T          inet_cidr_route_next_hop_as;
    UI32_T          inet_cidr_route_metric1;
}AMTRL3_TYPE_InetCidrRoutePartialEntry_T;

/*  inetCidrRouteEntry OBJECT-TYPE
        SYNTAX     InetCidrRouteEntry
        MAX-ACCESS not-accessible
        STATUS     current
        DESCRIPTION
               "A particular route to a particular destination, under a
                particular policy (as reflected in the
                inetCidrRoutePolicy object).

                Dynamically created rows will survive an agent reboot.

                Implementers need to be aware that if the total number
                of elements (octets or sub-identifiers) in
                inetCidrRouteDest, inetCidrRoutePolicy, and
                inetCidrRouteNextHop exceeds 111 then OIDs of column
                instances in this table will have more than 128 sub-
                identifiers and cannot be accessed using SNMPv1,
                SNMPv2c, or SNMPv3."
        INDEX {
            inetCidrRouteDestType,
            inetCidrRouteDest,
            inetCidrRoutePfxLen,
            inetCidrRoutePolicy,
            inetCidrRouteNextHopType,
            inetCidrRouteNextHop
            }
        ::= { inetCidrRouteTable 1 }

    InetCidrRouteEntry ::= SEQUENCE {
            inetCidrRouteDestType     InetAddressType,
            inetCidrRouteDest         InetAddress,
            inetCidrRoutePfxLen       InetAddressPrefixLength,
            inetCidrRoutePolicy       OBJECT IDENTIFIER,
            inetCidrRouteNextHopType  InetAddressType,
            inetCidrRouteNextHop      InetAddress,
            inetCidrRouteIfIndex      InterfaceIndexOrZero,
            inetCidrRouteType         INTEGER,
            inetCidrRouteProto        IANAipRouteProtocol,
            inetCidrRouteAge          Gauge32,
            inetCidrRouteNextHopAS    InetAutonomousSystemNumber,
            inetCidrRouteMetric1      Integer32,
            inetCidrRouteMetric2      Integer32,
            inetCidrRouteMetric3      Integer32,
            inetCidrRouteMetric4      Integer32,
            inetCidrRouteMetric5      Integer32,
            inetCidrRouteStatus       RowStatus
        }
*/
typedef struct AMTRL3_TYPE_InetCidrRouteEntry_S
{
    AMTRL3_TYPE_InetCidrRoutePartialEntry_T   partial_entry;
    UI32_T      inet_cidr_route_metric2;
    UI32_T      inet_cidr_route_metric3;
    UI32_T      inet_cidr_route_metric4;
    UI32_T      inet_cidr_route_metric5;
    UI32_T      inet_cidr_route_status;
}AMTRL3_TYPE_InetCidrRouteEntry_T;

/* use this data structure to send ISC, need pack.
 */
typedef struct
{
    UI16_T vid;
    UI16_T ifindex;
    UI8_T  mac[SYS_ADPT_MAC_ADDR_LEN];
    UI8_T  action;
    UI8_T  source;
    UI8_T  life_time;
}__attribute__((packed, aligned(1)))AMTRL3_TYPE_AddrEntry_T;

#if (SYS_CPNT_VXLAN == TRUE)
typedef struct AMTRL3_TYPE_VxlanTunnelEntry_S
{
    UI32_T          vfi_id;
    L_INET_AddrIp_T local_vtep;
    L_INET_AddrIp_T remote_vtep;
    BOOL_T          is_mc;
    UI32_T          udp_port;
    UI32_T          bcast_group;
    UI32_T          vxlan_port;
} AMTRL3_TYPE_VxlanTunnelEntry_T;

typedef struct AMTRL3_TYPE_VxlanTunnelNexthopEntry_S
{
    UI32_T          vfi_id;
    L_INET_AddrIp_T local_vtep;
    L_INET_AddrIp_T remote_vtep;
    L_INET_AddrIp_T nexthop_addr;
    UI32_T          nexthop_ifindex;
} AMTRL3_TYPE_VxlanTunnelNexthopEntry_T;
#endif

enum AMTRL3_TYPE_SuperNettingStatus_E
{
    AMTRL3_TYPE_SUPER_NET_ENABLE = 1,
    AMTRL3_TYPE_SUPER_NET_DISABLE,
};

enum AMTRL3_TYPE_SoftwareForwardingStatus_E
{
    AMTRL3_TYPE_SOFTWARE_FORWARDING_ENABLE = 1,
    AMTRL3_TYPE_SOFTWARE_FORWARDING_DISABLE,
};

#endif
