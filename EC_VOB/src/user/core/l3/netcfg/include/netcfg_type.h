/* Module Name: netcfg_type.h
 * Purpose:
 *      NETCFG_TYPE defines all data structure used in NETCFG, which is refered from RFC 2011.
 *
 * Notes:
 *      1.
 *
 *
 * History:
 *       Date       --  Modifier,   Reason
 *  0.1 2002.02.21  --  William,    Created
 *  0.2 2007.07.18  --  Max Chen,   Port to Linux Platform
 *
 * Copyright(C)      Accton Corporation, 2007.
 */

#ifndef     _NETCFG_TYPE_H
#define     _NETCFG_TYPE_H


/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_adpt.h"
#include "leaf_1724.h"
#include "leaf_es3626a.h"
#include "l_rstatus.h"
#include "l_inet.h"


/* NAME CONSTANT DECLARATIONS
 */

/******** return values *****************************/
/* success values */
#define NETCFG_TYPE_OK                      0x00000000
#define NETCFG_TYPE_REPLACE_DYNAMIC_ENTRY   0x00000001
#define NETCFG_TYPE_NO_CHANGE               0x00000002
#define NETCFG_TYPE_CHANGED                 0x00000003
#define NETCFG_TYPE_DUPLICATE_GATEWAY_IP    0x00000004

/* fail value */
#define NETCFG_TYPE_ENTRY_EXIST             0x80000001
#define NETCFG_TYPE_ENTRY_NOT_EXIST         0x80000002
#define NETCFG_TYPE_INVALID_ARG             0x80000003
#define NETCFG_TYPE_INVLAID_NEXT_HOP        0x80000004
#define NETCFG_TYPE_CAN_NOT_ADD             0x80000005
#define NETCFG_TYPE_CAN_NOT_DELETE          0x80000006
#define NETCFG_TYPE_CAN_NOT_GET             0x80000007
#define NETCFG_TYPE_CAN_NOT_SET             0x80000008
#define NETCFG_TYPE_DESTINATION_IS_LOCAL    0x80000009
#define NETCFG_TYPE_CAN_NOT_ADD_LOCAL_IP    0x8000000A
#define NETCFG_TYPE_CAN_NOT_DELETE_LOCAL_IP 0x8000000B
#define NETCFG_TYPE_TABLE_FULL              0x8000000C
#define NETCFG_TYPE_IP_ALREADY_EXIST        0x8000000D
#define NETCFG_TYPE_FAIL                    0x8000000E
#define NETCFG_TYPE_REJECT_SETTING_ENTRY    0x8000000F
#define NETCFG_TYPE_INTERFACE_NOT_EXISTED   0x80000010
#define NETCFG_TYPE_NO_SUCH_SUBNET          0x80000011
#define NETCFG_TYPE_NOT_IMPLEMENT           0x80000012
#define NETCFG_TYPE_NOT_FOUND               0x80000013
#define NETCFG_TYPE_NOT_MASTER_MODE         0x80000014
#define NETCFG_TYPE_NO_MORE_ENTRY           0x80000015
#define NETCFG_TYPE_CAN_NOT_ACTIVE          0x80000016
#define NETCFG_TYPE_NOT_EXIST               0x80000017
#define NETCFG_TYPE_CAN_NOT_DELETE_DYNAMIC_ENTRY  0x80000018
#define NETCFG_TYPE_CAN_NOT_DELETE_LOCAL_ENTRY    0x80000019
#define NETCFG_TYPE_NO_MORE_INTERFACE       0x8000001A
#define NETCFG_TYPE_NOT_IN_SERVICE          0x8000001B
#define NETCFG_TYPE_INVALID_IP              0x8000001C
#define NETCFG_TYPE_ALREADY_FORWARDING      0x8000001D
#define NETCFG_TYPE_NO_SUCH_INTERFACE       0x8000001E
#define NETCFG_TYPE_UNKNOWN_SYSCALL_CMD     0x8000001F
#define NETCFG_TYPE_INVALID_ROW_STATUS      0x80000020
#define NETCFG_TYPE_NOT_ALL_CONFIG          0x80000021
#define NETCFG_TYPE_MORE_THAN_TWO_OVERLAPPED    0x80000022
#define NETCFG_TYPE_MUST_DELETE_SECONDARY_FIRST 0x80000023
#define NETCFG_TYPE_IP_IS_VRRP_ADDRESS      0x80000024 /*Lin.Li, for ARP porting*/
#define NETCFG_TYPE_RA_LIFETIME_LESS_THAN_RA_INTERVAL           0x80000025 /*simon shih*/
#define NETCFG_TYPE_PRIMARY_IP_NOT_EXIST     0x80000026
#define NETCFG_TYPE_CAN_NOT_PERFORM_CHANGE   0x80000027
#define NETCFG_TYPE_IPV6_LINK_LOCAL_ADDR_INVALID_ZONE_ID     0x80000028
#define NETCFG_TYPE_IPV6_LINK_LOCAL_ADDR_INVALID_FORMAT      0x80000029
#define NETCFG_TYPE_MAC_COLLISION                            0x8000002A

/*Lin.Li, for RIP porting, modify start*/
#define NETCFG_TYPE_INSTANCE_NOT_EXIST              0x80000038
#define NETCFG_TYPE_INTERFACE_NOT_UP                0x80000039
#define NETCFG_TYPE_DISTANCE_TABLE_NOT_EXIST        0x80000040
/*Lin.Li, for RIP porting, modify end*/
/*netcfg ospf return type*/
#define NETCFG_TYPE_OSPF_MASTER_NOT_EXIST                       0x80000050
#define NETCFG_TYPE_OSPF_VRF_NOT_EXIST                          0x80000051
#define NETCFG_TYPE_OSPF_IPADDRESS_NOT_EXIST                    0x80000052
#define NETCFG_TYPE_OSPF_IPINTERFACE_NOT_EXIST                  0x80000053
#define NETCFG_TYPE_OSPF_ENTRY_INSERT_ERROR                     0x80000054
#define NETCFG_TYPE_OSPF_ENTRY_INSERT_SUCCESS                   0x80000055
#define NETCFG_TYPE_OSPF_MD5_KEY_EXIST                          0x80000056
#define NETCFG_TYPE_OSPF_NETWORK_ALREADY_EXIST                  0x80000057
#define NETCFG_TYPE_OSPF_AREA_ID_NOT_MATCH                      0x80000058
#define NETCFG_TYPE_OSPF_AREA_IS_DEFAULT                        0x80000059
#define NETCFG_TYPE_OSPF_AREA_HAS_VLINK                         0x8000005A
#define NETCFG_TYPE_OSPF_AREA_IS_NSSA                           0x8000005B
#define NETCFG_TYPE_SET_FAIL_TO_OSPF_ORIGINATE_DEFAULT_ROUTE    0x8000005C
#define NETCFG_TYPE_SET_FAIL_TO_OSPF_ADVERTISE_DEFAULT_ROUTE    0x8000005D
#define NETCFG_TYPE_SET_FAIL_TO_OSPF_EXTERNAL_METRIC_TYPE       0x8000005E
#define NETCFG_TYPE_SET_FAIL_TO_OSPF_DEFAULT_EXTERNAL_METRIC    0x8000005F


/* netcfg igmp return type */
#define NETCFG_TYPE_IGMP_PIM_ENABLED              0x80000070
#define NETCFG_TYPE_MLD_PIM_ENABLED               0x80000071

#if (SYS_CPNT_IP_TUNNEL == TRUE)
#define NETCFG_TYPE_TUNNEL_SOURCE_VLAN_NOT_EXIST               0x80000080
#define NETCFG_TYPE_TUNNEL_SOURCE_VLAN_HAS_NO_IPV4_ADDRESS             0x80000081
#define NETCFG_TYPE_TUNNEL_NOT_CONFIGURED_TUNNEL              0x80000082
#define NETCFG_TYPE_TUNNEL_TUNNEL_DEST_MUST_UNCONFIGURED            0x80000083
#endif

/* fail value for static route */
#define NETCFG_TYPE_INVALID_NEXTHOP_IPV6_UNSPECIFIED        0x80000100
#define NETCFG_TYPE_INVALID_NEXTHOP_IPV6_LOOPBACK           0x80000101
#define NETCFG_TYPE_INVALID_NEXTHOP_IPV6_MULTICAST          0x80000102
#define NETCFG_TYPE_INVALID_NEXTHOP_LOCAL_IP                0x80000103
#define NETCFG_TYPE_INVALID_ROUTE_IPV6_UNSPECIFIED          0x80000104
#define NETCFG_TYPE_INVALID_ROUTE_IPV6_LOOPBACK             0x80000105
#define NETCFG_TYPE_INVALID_ROUTE_IPV6_MULTICAST            0x80000106
#define NETCFG_TYPE_INVALID_ROUTE_IPV6_LINKLOCAL            0x80000107
#define NETCFG_TYPE_ROUTE_PREFIX_LEN_MORE_THAN_64           0x80000108
#define NETCFG_TYPE_ERR_ROUTE_NEXTHOP_CONFLICT_ON_NORMAL_INT_AND_CRAFT_INT  0x80000109
#define NETCFG_TYPE_ERR_ROUTE_TWO_NEXTHOP_ON_CRAFT_INT_IS_NOT_ALLOWED       0x80000110
#if (SYS_CPNT_STATIC_ROUTE_AND_METER_WORKAROUND == TRUE)
#define NETCFG_TYPE_SW_ROUTE_NOT_NEED					    0x80000111
#endif

/* fail value for IPv6 */
#define NETCFG_TYPE_INVALID_IPV6_UNSPECIFIED                0x800000A0
#define NETCFG_TYPE_INVALID_IPV6_LOOPBACK                   0x800000A1
#define NETCFG_TYPE_INVALID_IPV6_MULTICAST                  0x800000A2
#define NETCFG_TYPE_HAS_EXPLICIT_IPV6_ADDR                  0x800000A3
#define NETCFG_TYPE_IPV6_MTU_EXCEED_IF_MTU                  0x800000A4


#define NETCFG_TYPE_MAX_REDISTRIBUTE_ENTRY          2
#define NETCFG_TYPE_REDISTRIBUTE_PROTOCOL_OSPF      1
#define NETCFG_TYPE_REDISTRIBUTE_PROTOCOL_STATIC    2

#define NETCFG_TYPE_RipRedistributeStatus_active    1L
#define NETCFG_TYPE_RipRedistributeStatus_notInService  2L
#define NETCFG_TYPE_RipRedistributeStatus_notReady  3L
#define NETCFG_TYPE_RipRedistributeStatus_createAndGo   4L
#define NETCFG_TYPE_RipRedistributeStatus_createAndWait 5L
#define NETCFG_TYPE_RipRedistributeStatus_destroy   6L

#define NETCFG_TYPE_StaticIpCidrEntryRowStatus_active           1L
#define NETCFG_TYPE_StaticIpCidrEntryRowStatus_notInService     2L
#define NETCFG_TYPE_StaticIpCidrEntryRowStatus_notReady         3L
#define NETCFG_TYPE_StaticIpCidrEntryRowStatus_createAndGo      4L
#define NETCFG_TYPE_StaticIpCidrEntryRowStatus_createAndWait    5L
#define NETCFG_TYPE_StaticIpCidrEntryRowStatus_destroy          6L

/****************** end of return values *************/

#define NETCFG_TYPE_PHY_ADDRESEE_LENGTH         SYS_ADPT_MAC_ADDR_LEN          /*  Physical address length */
#define NETCFG_TYPE_RIP_AUTH_STRING_LENGTH      16 /*Lin.Li, for RIP*/
#define NETCFG_TYPE_RIP_MIN_METRIC              1
#define NETCFG_TYPE_RIP_INFINITY_METRIC         16
#define NETCFG_TYPE_RIP_MAX_NBR_OF_IPV4_ROUTE   SYS_ADPT_MAX_NBR_OF_RIP_ROUTE_ENTRY
#define NETCFG_TYPE_RIP_RECVBUF_DEFAULT         8192
#define NETCFG_TYPE_RIP_RECVBUF_MAXSIZE         2147483647
/* OSPF Authentication Type. */
#define NETCFG_TYPE_OSPF_AUTH_NULL           0
#define NETCFG_TYPE_OSPF_AUTH_SIMPLE         1
#define NETCFG_TYPE_OSPF_AUTH_CRYPTOGRAPHIC	 2

#define NETCFG_TYPE_OSPF_TIMER_STR_MAXLEN  9
#define NETCFG_TYPE_OSPF_AUTH_SIMPLE_SIZE  8
#define NETCFG_TYPE_OSPF_AUTH_MD5_SIZE     16

#define NETCFG_TYPE_RIF_FLAG_IPV6_INVALID       0x01
/* #define NETCFG_TYPE_RIF_FLAG_IPV6_TENTATIVE 0x02 */
#define NETCFG_TYPE_RIF_FLAG_IFA_F_PERMANENT    0x04   /* same as IFA_F_PERMANENT in kernel */

/* MACRO FUNCTION DECLARATIONS
 */
#define DUMP_RIF_ENTRY(rif_p)  do{\
    printf("%s, %d\n", __FUNCTION__, __LINE__);\
    printf(" *rif_p: %p\n", (rif_p));\
    printf("ifindex: %ld\n", (long)(rif_p)->ifindex);\
    printf("ipv4_role: %ld\n", (long)(rif_p)->ipv4_role);\
    printf("ipv6_addr_config_type: %ld\n", (long)(rif_p)->ipv6_addr_config_type);\
    printf("ipv6_addr_type: %ld\n", (long)(rif_p)->ipv6_addr_type);\
    printf("row_status: %ld\n", (long)(rif_p)->row_status);\
    DUMP_INET_ADDR((rif_p)->addr);\
} while(0)

#define DUMP_INET_ADDR(a)  do{\
    int i;\
    printf("a.type: %d\n", a.type);\
    printf("a.addrlen: %d\n", a.addrlen);\
    printf("a.addr: ");\
    for(i=0;(i<a.addrlen) || (i<4) ;i++) /* print at least first 4 addr, in case addrlen is 0 */\
    {\
        printf("%x ",a.addr[i]);\
    }\
    printf("\n");\
    printf("a.preflen: %d\n", a.preflen);\
    printf("a.zoneid: %ld\n", (long)a.zoneid);\
} while(0)

#ifndef RIP_IF_PARAM_CHECK
	#define RIP_IF_PARAM_CHECK(P, T) \
	    (CHECK_FLAG ((P)->config, RIP_IF_PARAM_ ## T))
#endif

#define RIP_IF_PARAM_SET(P, T)		((P)->config |= RIP_IF_PARAM_ ## T)
#define RIP_IF_PARAM_UNSET(P, T)	((P)->config &= ~RIP_IF_PARAM_ ## T)
#define RIP_IF_PARAMS_EMPTY(P)		((P)->config == 0)

/* DATA TYPE DECLARATIONS
 */
#if (SYS_CPNT_ECMP_BALANCE_MODE == TRUE)
typedef enum
{
   NETCFG_TYPE_ECMP_DIP_L4_PORT,
   NETCFG_TYPE_ECMP_HASH_SELECTION,
   NETCFG_TYPE_ECMP_MAX,
} NETCFG_TYPE_ECMP_MODE_T;
#endif

typedef enum NETCFG_TYPE_TUNNEL_MODE_E
{
    NETCFG_TYPE_TUNNEL_MODE_UNSPEC=0,
    NETCFG_TYPE_TUNNEL_MODE_CONFIGURED = VAL_ipv6TunnelMode_configured,
    NETCFG_TYPE_TUNNEL_MODE_6TO4 = VAL_ipv6TunnelMode_6to4,
    NETCFG_TYPE_TUNNEL_MODE_ISATAP = VAL_ipv6TunnelMode_Isatap,

    NETCFG_TYPE_TUNNEL_MODE_MAXVAL
} NETCFG_TYPE_TUNNEL_MODE_T;

#define NETCFG_TYPE_DEFAULT_TUNNEL_TOS 0

enum    NETCFG_TYPE_INTERFACE_MODE_E
{
    NETCFG_TYPE_MODE_PRIMARY=VAL_netConfigPrimaryInterface_primary,         /*  this rif is primary interface of vid_ifIndex    */
    NETCFG_TYPE_MODE_SECONDARY=VAL_netConfigPrimaryInterface_secondary,         /*  this rif is secondary interface of vid_ifIndex  */
    NETCFG_TYPE_MODE_PT_TO_PT,               /*  this rif is a point to point interface, not used now */
#if(SYS_CPNT_VIRTUAL_IP == TRUE)
    NETCFG_TYPE_MODE_VIRTUAL,                 /*  this rif is a virtual interface */
#endif    
};

typedef enum NETCFG_TYPE_IP_ADDRESS_MODE_E
{
    NETCFG_TYPE_IP_ADDRESS_MODE_USER_DEFINE = VAL_vlanAddressMethod_user,
    NETCFG_TYPE_IP_ADDRESS_MODE_DHCP        = VAL_vlanAddressMethod_dhcp,
#if (SYS_CPNT_BOOTP == TRUE)
    NETCFG_TYPE_IP_ADDRESS_MODE_BOOTP       = VAL_vlanAddressMethod_bootp,
#endif
} NETCFG_TYPE_IP_ADDRESS_MODE_T;

enum NETCFG_TYPE_IP_CONFIGURE_TYPE_E
{
    NETCFG_TYPE_IP_CONFIGURATION_TYPE_CLI_WEB = 1,
    NETCFG_TYPE_IP_CONFIGURATION_TYPE_SNMP,
    NETCFG_TYPE_IP_CONFIGURATION_TYPE_DYNAMIC
};

enum NETCFG_TYPE_IPV6_ADDRESS_TYPE_E
{
    NETCFG_TYPE_IPV6_ADDRESS_TYPE_LINK_LOCAL = 1,
    NETCFG_TYPE_IPV6_ADDRESS_TYPE_GLOBAL,
    NETCFG_TYPE_IPV6_ADDRESS_TYPE_EUI64
};

enum NETCFG_TYPE_IPV6_ADDRESS_CONFIG_TYPE_E
{
    NETCFG_TYPE_IPV6_ADDRESS_CONFIG_TYPE_MANUAL = 1,
    NETCFG_TYPE_IPV6_ADDRESS_CONFIG_TYPE_AUTO_STATELESS,
    NETCFG_TYPE_IPV6_ADDRESS_CONFIG_TYPE_AUTO_STATEFULL,
    NETCFG_TYPE_IPV6_ADDRESS_CONFIG_TYPE_AUTO_OTHER
};

enum NETCFG_TYPE_ND_ROUTER_PREFERENCE_E
{
   NETCFG_TYPE_ND_ROUTER_PERFERENCE_HIGH = 1,
   NETCFG_TYPE_ND_ROUTER_PERFERENCE_MEDIUM=0,
   NETCFG_TYPE_ND_ROUTER_PERFERENCE_LOW=3
} ;

/* This should be replace by MIB definition if supported */
enum NETCFG_TYPE_ND_NEIGHBOR_STATE_E
{
   NETCFG_TYPE_ND_NEIGHBOR_STATE_REACHABLE = 1,
   NETCFG_TYPE_ND_NEIGHBOR_STATE_STALE=2,
   NETCFG_TYPE_ND_NEIGHBOR_STATE_DELAY=3,
   NETCFG_TYPE_ND_NEIGHBOR_STATE_PROBE=4,
   NETCFG_TYPE_ND_NEIGHBOR_STATE_INVALID=5,
   NETCFG_TYPE_ND_NEIGHBOR_STATE_UNKNOWN=6,
   NETCFG_TYPE_ND_NEIGHBOR_STATE_INCOMPLETE=7
} ;

/* For IPv6 Statistics */
enum NETCFG_TYPE_IPV6_STAT_TYPE_E
{
    NETCFG_TYPE_IPV6_STAT_TYPE_IP6 = 1,
    NETCFG_TYPE_IPV6_STAT_TYPE_ICMP6,
    NETCFG_TYPE_IPV6_STAT_TYPE_UDP6,
    NETCFG_TYPE_IPV6_STAT_TYPE_ALL
};


/*  tcpConnEntry : RFC 2012 (SNMPv2 MIB for TCP) Nov-1996   */
typedef struct NETCFG_TYPE_tcpConnEntry_S
{
    int     tcp_conn_state;             /*  tcpConnState;           Valid : 1..12   */
    UI8_T   tcp_conn_local_address[SYS_ADPT_IPV4_ADDR_LEN];     /*  tcpConnLocalAddress;                    */
    int     tcp_conn_local_port;        /*  tcpConnLocalPort;       Valid : 0..65535*/
    UI8_T   tcp_conn_rem_address[SYS_ADPT_IPV4_ADDR_LEN];       /*  tcpConnRemAddress;                      */
    int     tcp_conn_rem_port;          /*  tcpConnRemPort;         Valid : 0..65535*/
}   NETCFG_TYPE_tcpConnEntry_T;

/*  udpEntry : RFC 2013 (SNMPv2 MIB for UDP) Nov-1996   */
typedef struct NETCFG_TYPE_udpEntry_S
{
    UI8_T   udp_local_address[SYS_ADPT_IPV4_ADDR_LEN];          /*  udpLocalAddress;                        */
    int     udp_local_port;             /*  udpLocalPort;   Valid : 0..65535        */
} NETCFG_TYPE_udpEntry_T;

/*  ipAddrEntry : RFC 2011 (SNMPv2 MIB for IP) Nov-1996 */
typedef struct NETCFG_TYPE_ipAddrEntry_S
{
    UI8_T   ip_ad_ent_addr[SYS_ADPT_IPV4_ADDR_LEN];             /*  ipAdEntAddr;                            */
    int     ip_ad_ent_if_index;         /*  ipAdEntIfIndex; valid : 1-2147483647    */
    UI8_T   ip_ad_ent_net_mask[SYS_ADPT_IPV4_ADDR_LEN];         /*  ipAdEntNetMask;                         */
    int     ip_ad_ent_bcast_addr;       /*  ipAdEntBcastAddr;   valid : 0-1         */
    int     ip_ad_ent_reasm_maxSize;    /*  ipAdEntReasmMaxSize;valid : 0-65535     */
} NETCFG_TYPE_ipAddrEntry_T;

/*Lin.Li, for ARP porting, modify start*/
/*  PhyAddress : RFC 1213 (MIB II) Mar-1991 */
typedef struct NETCFG_TYPE_PhysAddress_S
{
    UI32_T  phy_address_type;           /*  PhyAddress type                         */
    UI32_T  phy_address_len;            /*  PhyAddress physical length              */
                                        /*  Buffer to keep PhyAddress               */
    UI8_T   phy_address_cctet_string[NETCFG_TYPE_PHY_ADDRESEE_LENGTH];
} NETCFG_TYPE_PhysAddress_T;

/*  ipNetToMediaEntry : RFC 2011 (SNMPv2 MIB for IP) Nov-1996
 *      Key : (ip_net_to_media_if_index, ip_net_to_media_net_address)
 */
typedef struct NETCFG_TYPE_IpNetToMediaEntry_S
{
    UI32_T                          ip_net_to_media_if_index;          /*  ipNetToMediaIfIndex      */
    NETCFG_TYPE_PhysAddress_T       ip_net_to_media_phys_address;     /*  ipNetToMediaPhyAddress  */
    UI32_T                          ip_net_to_media_net_address;     /*  ipNetToMediaNetAddress  */
    /*  ipNetToMediaType valid :     1..4
    *      VAL_ipNetToMediaType_other  (1),       -- none of the following
    *      VAL_ipNetToMediaType_invalid(2),       -- an invalidated mapping
    *      VAL_ipNetToMediaType_dynamic(3),
    *      VAL_ipNetToMediaType_static (4)
    */
    int                             ip_net_to_media_type;             /*  ipNetToMediaType; */

} NETCFG_TYPE_IpNetToMediaEntry_T;

typedef struct NETCFG_TYPE_Ipv6NetToMediaEntry_S
{
    UI32_T                          ip_net_to_media_if_index;          /*  ipv6IfIndex      */
    NETCFG_TYPE_PhysAddress_T       ip_net_to_media_phys_address;     /*  ipv6NetToMediaPhysAddress  */
    UI8_T                 ip_net_to_media_net_address[SYS_ADPT_IPV6_ADDR_LEN];     /*  ipv6NetToMediaNetAddress  */
    UI32_T                          ip_net_to_media_last_update;            /*  ipv6IfNetToMediaLastUpdated; */
    /*  ipNetToMediaType valid :     1..5
    *      VAL_ipNetToPhysicalType_other  (1),       -- none of the following
    *      VAL_ipNetToPhysicalType_invalid(2),       -- an invalidated mapping
    *      VAL_ipNetToPhysicalType_dynamic(3),
    *      VAL_ipNetToPhysicalType_static (4)
	*      VAL_ipNetToPhysicalType_local  (5)
    */
    int                             ip_net_to_media_type;             /*  ipv6NetToMediaType; */
	/*  ipNetToMediaType valid :     1..7
    *      VAL_ipNetToPhysicalState_reachable  (1),       -- none of the following
    *      VAL_ipNetToPhysicalState_stale      (2),       -- an invalidated mapping
    *      VAL_ipNetToPhysicalState_delay      (3),
    *      VAL_ipNetToPhysicalState_probe      (4),
	*      VAL_ipNetToPhysicalState_invalid    (5),
	*      VAL_ipNetToPhysicalState_unknown    (6),
	*      VAL_ipNetToPhysicalState_incomplete (7)
    */
	int                             ip_net_to_media_state;            /*  ipv6IfNetToMediaState; */

} NETCFG_TYPE_Ipv6NetToMediaEntry_T;

typedef struct NETCFG_TYPE_IpNetToPhysicalEntry_S
{
    UI32_T                          ip_net_to_physical_if_index;          /*  ipNetToMediaIfIndex      */
    NETCFG_TYPE_PhysAddress_T       ip_net_to_physical_phys_address;     /*  ipNetToMediaPhyAddress  */
    L_INET_AddrIp_T                          ip_net_to_physical_net_address;     /*  ipNetToMediaNetAddress  */
	UI32_T                          ip_net_to_physical_last_update;          /*  ipv6IfNetToMediaLastUpdated; */

    /*  ipNetToMediaType valid :     1..4
    *      VAL_ipNetToMediaType_other  (1),       -- none of the following
    *      VAL_ipNetToMediaType_invalid(2),       -- an invalidated mapping
    *      VAL_ipNetToMediaType_dynamic(3),
    *      VAL_ipNetToMediaType_static (4)
    */
    int                             ip_net_to_physical_type;             /*  ipNetToMediaType; */
		/*  ipNetToMediaType valid :     1..7
    *      VAL_ipNetToPhysicalState_reachable  (1),       -- none of the following
    *      VAL_ipNetToPhysicalState_stale      (2),       -- an invalidated mapping
    *      VAL_ipNetToPhysicalState_delay      (3),
    *      VAL_ipNetToPhysicalState_probe      (4),
	*      VAL_ipNetToPhysicalState_invalid    (5),
	*      VAL_ipNetToPhysicalState_unknown    (6),
	*      VAL_ipNetToPhysicalState_incomplete (7)
    */
	int                             ip_net_to_physical_state;            /*  ipv6IfNetToMediaState; */

} NETCFG_TYPE_IpNetToPhysicalEntry_T;

typedef struct NETCFG_TYPE_StaticIpNetToMediaEntry_S
{
    NETCFG_TYPE_IpNetToMediaEntry_T     ip_net_to_media_entry;
    BOOL_T                              status;
} NETCFG_TYPE_StaticIpNetToMediaEntry_T;

typedef struct NETCFG_TYPE_StaticIpv6NetToMediaEntry_S
{
    NETCFG_TYPE_Ipv6NetToMediaEntry_T     ip_net_to_media_entry;
    BOOL_T                              status;
} NETCFG_TYPE_StaticIpv6NetToMediaEntry_T;

typedef struct
{
    NETCFG_TYPE_IpNetToPhysicalEntry_T     ip_net_to_physical_entry;
    BOOL_T                              status;
} NETCFG_TYPE_StaticIpNetToPhysicalEntry_T;


typedef struct NETCFG_TYPE_IpNetToMedia_Statistics_S
{
    UI32_T      in_request;
    UI32_T      in_reply;
    UI32_T      out_request;
    UI32_T      out_reply;
} NETCFG_TYPE_IpNetToMedia_Statistics_T;
/*Lin.Li, for ARP porting, modify end*/


/*  ipNetToMediaExtEntry : Private MIB.
    This Entry is the extension of ipNetToMediaEntry in order to support
    more media types defined and limited by standard mib 2011
 */
typedef struct NETCFG_TYPE_IpNetToMediaExtEntry_S
{
    int                         ip_net_to_media_if_index;       /* KEY 1 */
    UI8_T                       ip_net_to_media_net_address[SYS_ADPT_IPV4_ADDR_LEN];    /* KEY 2 */
    int                         ip_net_to_media_ext_type;
} NETCFG_TYPE_IpNetToMediaExtEntry_T;

/*  ipCidrRouteEntry    : RFC 2096 (IP Forwarding Table MIB) Jan-1997   */
typedef struct NETCFG_TYPE_IpCidrRouteEntry_S
{
    L_INET_AddrIp_T route_dest;         /*  ipCidrRouteDest : IpAddress */
    UI8_T  ip_cidr_route_dest[SYS_ADPT_IPV4_ADDR_LEN];         /*  ipCidrRouteDest : IpAddress */
    UI8_T  ip_cidr_route_mask[SYS_ADPT_IPV4_ADDR_LEN];         /*  ipCidrRouteMask : IpAddress */
    I32_T   ip_cidr_route_tos;          /*  ipCidrRouteTos              */
    UI8_T  ip_cidr_route_next_hop[SYS_ADPT_IPV4_ADDR_LEN];     /*  ipCidrRouteNextHop;         */
    L_INET_AddrIp_T route_next_hop; 			/*  ipCidrRouteNextHop;         */
    I32_T   ip_cidr_route_if_index;     /*  ipCidrRouteIfIndex;         */
    /*  ipCidrRouteType Valid : 1..4
     *      VAL_ipCidrRouteType_other    (1), -- not specified by this MIB
     *      VAL_ipCidrRouteType_reject   (2), -- route which discards traffic
     *      VAL_ipCidrRouteType_local    (3), -- local interface
     *      VAL_ipCidrRouteType_remote   (4)  -- remote destination
     */
    int     ip_cidr_route_type;         /*  ipCidrRouteType;    */
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
    int     ip_cidr_route_proto;            /*  ipCidrRouteProto;   */
    I32_T   ip_cidr_route_age;              /*  ipCidrRouteAge      */
    /*  ipCidrRouteInfo     OBJECT IDENTIFIER,      */
    I32_T   ip_cidr_route_nextHopAS;            /*  ipCidrRouteNextHopAS;       */
    I32_T   ip_cidr_route_metric1;             /*   ipCidrRouteMetric1;         */
    I32_T   ip_cidr_route_metric2;             /*   ipCidrRouteMetric2;         */
    I32_T   ip_cidr_route_metric3;             /*   ipCidrRouteMetric3;         */
    I32_T   ip_cidr_route_metric4;             /*   ipCidrRouteMetric4;         */
    I32_T   ip_cidr_route_metric5;             /*   ipCidrRouteMetric5;         */
    I32_T   ip_cidr_route_ext_subtype;         /*   Private MIB, for recording Protocl sub_type */

    UI32_T  ip_cidr_route_distance;            /* Administrative Distance */

    enum L_RSTATUS_TRANSITION_STATE_E           ip_cidr_route_status;
}   NETCFG_TYPE_IpCidrRouteEntry_T;

/*  typedef NETIF_MGR_RifConfig_T   NETCFG_TYPE_RifConfig_T;    */
typedef struct NETCFG_TYPE_RifConfigExt_S
{
    UI32_T  ifindex;            /*  ifindex which bounnd to this routing interface.
                                 *  Currently, vid_ifindex is used
                                 *  start from SYS_ADPT_VLAN_1_IF_INDEX_NUMBER.
                                 */
    L_INET_AddrIp_T         addr;
    /*  Routing Interface indicator keeps : primary, secondary, or point-to-point
     *      NETCFG_TYPE_MODE_PRIMARY,
     *      NETCFG_TYPE_MODE_SECONDARY,
     *      NETCFG_TYPE_MODE_PT_TO_PT.
     */
    UI32_T  ipv4_role;  /* Indicate what mode this RIF active: PRIMARY/SECONDARY    */
    UI32_T  row_status;         /* enableStatus; ref. MIB (es3626a) define  */

}NETCFG_TYPE_RifConfigExt_T;

typedef struct
{
    UI32_T                  ifindex;
    UI32_T                  row_status;
    UI32_T                  ipv4_role; /* Values of ipv4_role is enum NETCFG_TYPE_INTERFACE_MODE_E {NETCFG_TYPE_MODE_PRIMARY, NETCFG_TYPE_MODE_SECONDARY, NETCFG_TYPE_MODE_PT_TO_PT, NETCFG_TYPE_MODE_VIRTUAL } */
    L_INET_AddrIp_T         addr;
    UI32_T                  ipv6_addr_type;
    UI32_T                  ipv6_addr_config_type;
    UI32_T                  flags;

} NETCFG_TYPE_InetRifConfig_T;

typedef struct
{
    UI32_T  ifindex;
    UI32_T  valid_lft;     /* in seconds, 0xFFFFFFFF = forever */
    UI32_T  preferred_lft; /* in seconds, 0xFFFFFFFF = forever */
    UI32_T  scope;
    BOOL_T  tentative;
    BOOL_T  deprecated;
    BOOL_T  permanent;
    L_INET_AddrIp_T ipaddr;
} NETCFG_TYPE_IpAddressInfoEntry_T;


typedef struct
{
    UI32_T  pmtu;
    L_INET_AddrIp_T destip;
    UI32_T  since_time; /* in seconds */
} NETCFG_TYPE_PMTU_Entry_T;

typedef struct
{
    UI8_T tunnel_mode;
    UI16_T row_status;

    UI32_T src_vid_ifindex;
    UI8_T ttl;
    UI8_T tos;
    L_INET_AddrIp_T dip;
    L_INET_AddrIp_T dip_nexthop;
} NETCFG_TYPE_Tunnel_Interface_T;

/* Key: ifindex
 */
typedef struct
{
    /* Lookup Key */
    UI32_T ifindex;

    /* interface type: physical /tunnel */
    UI8_T  iftype;
    UI32_T  drv_l3_intf_index;  /* to save driver layer l3_intf_index here for distinguish physical/tunner int in driver layer */

    BOOL_T  ipv6_enable;
    BOOL_T  ipv6_autoconf_enable;

#if (SYS_CPNT_DHCP_INFORM == TRUE)
    BOOL_T  dhcp_inform;
#endif
    /* union */
    struct
    {
        struct
        {
    /* The interface flags, defined in TCP/IP stack,
     * IFF_UP, IFF_RUNNING
     */
    UI16_T if_flags;

    /* Vlan MAC
     */
    UI8_T hw_addr[SYS_ADPT_MAC_ADDR_LEN];
    UI8_T logical_mac[SYS_ADPT_MAC_ADDR_LEN];

    /* For SNMP use, TRUE specifies a logical mac has set
     */
    BOOL_T logical_mac_set;

    /* Vlan MTU
     */
    UI32_T mtu;

#if (SYS_CPNT_IPV6 ==  TRUE)
    /* IPv6 interface mtu
     */
    UI32_T mtu6;
    BOOL_T accept_ra_pinfo; /* ipv6 accept ra prefix info */
#endif

    /* Vlan Bandwidth
     */
    UI32_T bandwidth;

    /* Proxy ARP status
     */
    BOOL_T proxy_arp_enable;

    UI32_T ipv4_address_mode;


        } physical_intf;
#if (SYS_CPNT_IP_TUNNEL == TRUE)
         NETCFG_TYPE_Tunnel_Interface_T  tunnel_intf;
#endif
    } u;

}NETCFG_TYPE_L3_Interface_T;

/*Lin.Li, for RIP porting, modify start*/
typedef enum NETCFG_TYPE_RIP_Packet_Debug_Type_E
{
    NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE = 1,
    NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE_SEND,
    NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE_SENDANDDETAIL,
    NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE_RECEIVE,
    NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE_RECEIVEANDDETAIL,
    NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE_DETAIL
}NETCFG_TYPE_RIP_Packet_Debug_Type_T;

typedef struct NETCFG_TYPE_RIP_Debug_Status_S
{
    BOOL_T  nsm_all_status;
    BOOL_T  event_all_status;
    BOOL_T  packet_all_status;
    BOOL_T  packet_recv_status;
    BOOL_T  packet_send_status;
    BOOL_T  packet_detail_status;
}NETCFG_TYPE_RIP_Debug_Status_T;

enum NETCFG_TYPE_RIP_Distribute_Type_E
{
  NETCFG_TYPE_RIP_DISTRIBUTE_IN = 0,
  NETCFG_TYPE_RIP_DISTRIBUTE_OUT,
  NETCFG_TYPE_RIP_DISTRIBUTE_MAX
};

enum NETCFG_TYPE_RIP_Distribute_List_Type_E
{
    NETCFG_TYPE_RIP_DISTRIBUTE_ACCESS_LIST = 1,
    NETCFG_TYPE_RIP_DISTRIBUTE_PREFIX_LIST = 2
};

typedef struct NETCFG_TYPE_RIP_Timer_S
{
    UI32_T   update;
    UI32_T   timeout;
    UI32_T   garbage;
}NETCFG_TYPE_RIP_Timer_T;

enum NETCFG_TYPE_RIP_Version_E
{
    NETCFG_TYPE_RIP_VERSION_UNSPEC = 0,
    NETCFG_TYPE_RIP_VERSION_VERSION_1,
    NETCFG_TYPE_RIP_VERSION_VERSION_2,
    NETCFG_TYPE_RIP_VERSION_VERSION_1_AND_2,
    NETCFG_TYPE_RIP_VERSION_VERSION_1_COMPATIBLE,
    NETCFG_TYPE_RIP_VERSION_VERSION_MAX
};

enum NETCFG_TYPE_RIP_Auth_Mode_E
{
    NETCFG_TYPE_RIP_NO_AUTH = 0,
    NETCFG_TYPE_RIP_AUTH_DATA,
    NETCFG_TYPE_RIP_AUTH_SIMPLE_PASSWORD,
    NETCFG_TYPE_RIP_AUTH_MD5
};

enum NETCFG_TYPE_RIP_Split_Horizon_E
{
    NETCFG_TYPE_RIP_SPLIT_HORIZON_POISONED = 0,
    NETCFG_TYPE_RIP_SPLIT_HORIZON_NONE,
    NETCFG_TYPE_RIP_SPLIT_HORIZON
};

enum NETCFG_TYPE_RIP_Global_Version_E
{
    NETCFG_TYPE_RIP_GLOBAL_VERSION_1 = 1,
    NETCFG_TYPE_RIP_GLOBAL_VERSION_2,
    NETCFG_TYPE_RIP_GLOBAL_VERSION_BY_INTERFACE
};

enum NETCFG_TYPE_RIP_Redistribute_Type_E
{
    NETCFG_TYPE_RIP_Redistribute_Connected = 0,
    NETCFG_TYPE_RIP_Redistribute_Static,
    NETCFG_TYPE_RIP_Redistribute_Ospf,
    NETCFG_TYPE_RIP_Redistribute_Bgp,
    NETCFG_TYPE_RIP_Redistribute_Max
};

typedef struct NETCFG_TYPE_RIP_Redistribute_S
{
    char    rmap_name[SYS_ADPT_ACL_MAX_NAME_LEN + 1];
    UI32_T  metric;
}NETCFG_TYPE_RIP_Redistribute_T;

typedef struct NETCFG_TYPE_RIP_Redistribute_Table_S
{
    enum NETCFG_TYPE_RIP_Redistribute_Type_E protocol;
    NETCFG_TYPE_RIP_Redistribute_T    table;
}NETCFG_TYPE_RIP_Redistribute_Table_T;


typedef struct NETCFG_TYPE_RIP_Network_S
{
    UI32_T pfxlen;
    UI32_T ip_addr;
}NETCFG_TYPE_RIP_Network_T;

typedef struct NETCFG_TYPE_RIP_Distance_S
{
    UI32_T pfxlen;
    UI32_T ip_addr;
    UI32_T  distance;
    char    alist_name[SYS_ADPT_ACL_MAX_NAME_LEN + 1];
}NETCFG_TYPE_RIP_Distance_T;

typedef struct NETCFG_TYPE_RIP_Distribute_S
{
    UI32_T  ifindex;
    char    acl_list[NETCFG_TYPE_RIP_DISTRIBUTE_MAX][SYS_ADPT_ACL_MAX_NAME_LEN + 1];
    char    pre_list[NETCFG_TYPE_RIP_DISTRIBUTE_MAX][SYS_ADPT_ACL_MAX_NAME_LEN + 1];

}NETCFG_TYPE_RIP_Distribute_T;

typedef struct NETCFG_TYPE_RIP_Route_S
{
    UI32_T dest_pfxlen;
    UI32_T dest_addr;
    UI32_T nexthop_addr;
    UI32_T from_addr;
    UI32_T metric;
    UI32_T ifindex;
    UI32_T distance;
    int    type;
    int    sub_type;
    char   type_str[3];
    char   timebuf [100];
}NETCFG_TYPE_RIP_Route_T;

typedef struct NETCFG_TYPE_RIP_Peer_Entry_S
{
    UI32_T peer_addr;
    int    domain;
    char   timebuf[25];
    long   uptime;
    int    ifindex;
    int    version;
    int    recv_badpackets;
    int    recv_badroutes;
} NETCFG_TYPE_RIP_Peer_Entry_T;

typedef struct NETCFG_TYPE_RIP_Instance_S
{
    UI32_T   instance;
    BOOL_T   default_information;
    UI32_T   default_metric;
    UI32_T   distance;
    UI32_T   pmax;
    UI32_T   recv_buffer_size;
    NETCFG_TYPE_RIP_Timer_T     timer;
    NETCFG_TYPE_RIP_Redistribute_T          *redistribute[NETCFG_TYPE_RIP_Redistribute_Max];
    enum NETCFG_TYPE_RIP_Global_Version_E   version;

}NETCFG_TYPE_RIP_Instance_T;

typedef struct NETCFG_TYPE_RIP_If_S
{
    UI32_T   ifindex;
    BOOL_T   network_if;/*the network by ifindex status*/
  /* Configured flags. */
  UI16_T config;
#define RIP_IF_PARAM_ROUTER		(1 << 0)
#define RIP_IF_PARAM_DISABLE_RECV	(1 << 1)
#define RIP_IF_PARAM_DISABLE_SEND	(1 << 2)
#define RIP_IF_PARAM_RECV_VERSION	(1 << 3)
#define RIP_IF_PARAM_SEND_VERSION	(1 << 4)
#define RIP_IF_PARAM_AUTH_MODE		(1 << 5)
#define RIP_IF_PARAM_AUTH_STRING	(1 << 6)
#define RIP_IF_PARAM_KEY_CHAIN		(1 << 7)
#define RIP_IF_PARAM_SPLIT_HORIZON	(1 << 8)
    BOOL_T   pass_if;
    BOOL_T   recv_packet;
    BOOL_T   send_packet;
    char     auth_str[NETCFG_TYPE_RIP_AUTH_STRING_LENGTH + 1];
    char     key_chain[NETCFG_TYPE_RIP_AUTH_STRING_LENGTH + 1];
    NETCFG_TYPE_RIP_Distribute_T            distribute_table;
    enum NETCFG_TYPE_RIP_Version_E          recv_version_type;
    enum NETCFG_TYPE_RIP_Version_E          send_version_type;
    enum NETCFG_TYPE_RIP_Auth_Mode_E        auth_mode;
    enum NETCFG_TYPE_RIP_Split_Horizon_E    split_horizon;
}NETCFG_TYPE_RIP_If_T;

typedef struct NETCFG_TYPE_RIP_Global_Statistics_S
{
    int global_route_changes;
    int global_queries;
} NETCFG_TYPE_RIP_Global_Statistics_T;

/*Lin.Li, for RIP porting, modify end*/

typedef struct NETCFG_TYPE_PIM_If_S
{
    UI32_T   ifindex;
    BOOL_T   if_status;/*if the rif up*/
    UI32_T   ip_addr;
    UI32_T   ip_mask;

    int mode;
#define NETCFG_TYPE_PIM_PIM_MODE_NONE           0
#define NETCFG_TYPE_PIM_PIM_MODE_DENSE          1
#define NETCFG_TYPE_PIM_PIM_MODE_SPARSE         2
#define NETCFG_TYPE_PIM_PIM_MODE_SPARSEDENSE    3
#define NETCFG_TYPE_PIM_PIM_MODE_SPARSE_PASSIVE 4
#define NETCFG_TYPE_PIM_PIM_MODE_DENSE_PASSIVE  5

    UI32_T  config;
#define NETCFG_TYPE_PIM_VIF_CONFIG_PRIORITY       (1 << 0) /* DR priority. */
#define NETCFG_TYPE_PIM_VIF_EXCLUDE_GENID         (1 << 1) /* Gen-id exclude. */
#define NETCFG_TYPE_PIM_VIF_CONFIG_HELLO_INTERVAL (1 << 2) /* Hello interval. */
#define NETCFG_TYPE_PIM_VIF_CONFIG_HELLO_HOLDTIME (1 << 3) /* Hello holdtime. */

    UI32_T       hello_period;
    UI16_T       holdtime;
    UI32_T       dr_priority;
    BOOL_T       exclude_genid_yes;
    char        *nbr_flt;

}NETCFG_TYPE_PIM_If_T;

/*ospf redistribute type -- xiongyu*/
enum NETCFG_TYPE_OSPF_Redistribute_Type_E
{
    NETCFG_TYPE_OSPF_Redistribute_Connected = 0,
    NETCFG_TYPE_OSPF_Redistribute_Static,
    NETCFG_TYPE_OSPF_Redistribute_Rip,
    NETCFG_TYPE_OSPF_Redistribute_Max
};

/* OSPF Area ID format */
#define NETCFG_TYPE_OSPF_AREA_ID_FORMAT_DEFAULT		 0
#define NETCFG_TYPE_OSPF_AREA_ID_FORMAT_ADDRESS		 1
#define NETCFG_TYPE_OSPF_AREA_ID_FORMAT_DECIMAL		 2
/* OSPF area type.  */
#define NETCFG_TYPE_OSPF_AREA_DEFAULT			0
#define NETCFG_TYPE_OSPF_AREA_STUB				1
#define NETCFG_TYPE_OSPF_AREA_NSSA				2
#define NETCFG_TYPE_OSPF_AREA_TYPE_MAX			3

typedef struct NETCFG_TYPE_OSPF_Timer_S
{
    UI32_T   delay;
    UI32_T   hold;
}NETCFG_TYPE_OSPF_Timer_T;

/* OSPF config network structure. */
typedef struct NETCFG_TYPE_OSPF_Network_S
{
    UI32_T area_id;
    UI32_T network;
    UI32_T masklen;

    /* Network prefix. */
    struct L_ls_prefix *lp;

    /* Area ID format. */
    UI32_T format;
}NETCFG_TYPE_OSPF_Network_T;

/* OSPF area parameter. */
typedef struct NETCFG_TYPE_OSPF_Area_Config_S
{
    /* Configuration flags.  */
    UI16_T config;
#define NETCFG_TYPE_OSPF_AREA_CONF_EXTERNAL_ROUTING   (1 << 0) /* external routing. */
#define NETCFG_TYPE_OSPF_AREA_CONF_NO_SUMMARY         (1 << 1) /* stub no import summary.*/
#define NETCFG_TYPE_OSPF_AREA_CONF_DEFAULT_COST       (1 << 2) /* stub default cost. */
#define NETCFG_TYPE_OSPF_AREA_CONF_AUTH_TYPE          (1 << 3) /* Area auth type. */
#define NETCFG_TYPE_OSPF_AREA_CONF_NSSA_TRANSLATOR    (1 << 4) /* NSSA translate role. */
#define NETCFG_TYPE_OSPF_AREA_CONF_STABILITY_INTERVAL (1 << 5) /* NSSA StabilityInterval.*/
#define NETCFG_TYPE_OSPF_AREA_CONF_NO_REDISTRIBUTION  (1 << 6) /* NSSA redistribution. */
#define NETCFG_TYPE_OSPF_AREA_CONF_DEFAULT_ORIGINATE  (1 << 7) /* NSSA default originate.*/
#define NETCFG_TYPE_OSPF_AREA_CONF_METRIC		      (1 << 8) /* NSSA default metric. */
#define NETCFG_TYPE_OSPF_AREA_CONF_METRIC_TYPE	      (1 << 9) /* NSSA default mtype. */
#define NETCFG_TYPE_OSPF_AREA_CONF_SHORTCUT_ABR       (1 << 10)/* Shortcut ABR. */
#define NETCFG_TYPE_OSPF_AREA_CONF_ACCESS_LIST_IN     (1 << 11)/* Access-List in. */
#define NETCFG_TYPE_OSPF_AREA_CONF_ACCESS_LIST_OUT    (1 << 12)/* Access-List out. */
#define NETCFG_TYPE_OSPF_AREA_CONF_PREFIX_LIST_IN	  (1 << 13)/* Prefix-List in. */
#define NETCFG_TYPE_OSPF_AREA_CONF_PREFIX_LIST_OUT	  (1 << 14)/* Prefix-List out. */
#define NETCFG_TYPE_OSPF_AREA_CONF_SNMP_CREATE        (1 << 15)/* ospfAreaStatus.  */

    /* Area ID format */
    UI32_T format;

    /* External routing capability. */
    UI32_T external_routing;

    /* Authentication type. */
    UI32_T auth_type;
#define NETCFG_TYPE_OSPF_AREA_AUTH_NULL             OSPF_AUTH_NULL
#define NETCFG_TYPE_OSPF_AREA_AUTH_SIMPLE           OSPF_AUTH_SIMPLE
#define NETCFG_TYPE_OSPF_AREA_AUTH_CRYPTOGRAPHIC    OSPF_AUTH_CRYPTOGRAPHIC

    /* StubDefaultCost. */
    UI32_T default_cost;

    /* Area configured as shortcut. */
    UI32_T shortcut;
#define NETCFG_TYPE_OSPF_SHORTCUT_DEFAULT		0
#define NETCFG_TYPE_OSPF_SHORTCUT_ENABLE		1
#define NETCFG_TYPE_OSPF_SHORTCUT_DISABLE		2

#ifdef HAVE_NSSA
    /* NSSA Role during configuration. */
    UI32_T nssa_translator_role;
#define NETCFG_TYPE_OSPF_NSSA_TRANSLATE_NEVER	0
#define NETCFG_TYPE_OSPF_NSSA_TRANSLATE_ALWAYS	1
#define NETCFG_TYPE_OSPF_NSSA_TRANSLATE_CANDIDATE	2

    /* NSSA default-information originate metric-type. */
    UI32_T metric_type;
#define NETCFG_TYPE_OSPF_NSSA_METRIC_TYPE_DEFAULT	EXTERNAL_METRIC_TYPE_2

    /* NSSA default-information originate metric. */
    UI32_T metric;
#define NETCFG_TYPE_OSPF_NSSA_METRIC_DEFAULT	1

    /* NSSA TranslatorStabilityInterval */
    UI16_T nssa_stability_interval;
#define NETCFG_TYPE_OSPF_TIMER_NSSA_STABILITY_INTERVAL	40
#endif /* HAVE_NSSA */
}NETCFG_TYPE_OSPF_Area_Config_T;



/* OSPF area structure. */
typedef struct NETCFG_TYPE_OSPF_Area_S
{
    /* Area ID. */
    UI32_T area_id;

    /* OSPF process ID. */
    UI32_T ospf_id;

    /* VR ID. */
    UI32_T vr_id;

    /* Lock count. */
    int lock;

    /* Area parameter. */
    NETCFG_TYPE_OSPF_Area_Config_T conf;

    /* Row Status. */
    UI32_T status;

    UI32_T vlink_count;        /* VLINK interfaces. */


    /* Tables. */
    struct L_ls_table *ranges;      /* Area ranges. */

}NETCFG_TYPE_OSPF_Area_T;

/* OSPF area range structure. */
typedef struct NETCFG_TYPE_OSPF_Area_Range_S
{
  /* OSPF area. */
  NETCFG_TYPE_OSPF_Area_T *area;

  /* Area range prefix. */
  struct L_ls_prefix *lp;

  /* Substitute prefix. */
  struct L_ls_prefix *subst;

  /* Flag for advertise or not. */
  unsigned char flags;
#define OSPF_AREA_RANGE_ADVERTISE   (1 << 0)
#define OSPF_AREA_RANGE_SUBSTITUTE  (1 << 1)

  /* Row Status. */
  UI32_T status;

  /* Number of mathced prefix. */
  UI32_T matched;

  /* Cost for summary route. */
  UI32_T cost;
}NETCFG_TYPE_OSPF_Area_Range_T;

typedef struct NETCFG_TYPE_OSPF_IfConfig_S
{
    /* Interface index. */
    UI32_T ifindex;

    BOOL_T default_flag;

    UI32_T ip_address;
    UI8_T mask_len;

    /* Configured flags. */
    UI32_T config;
#define NETCFG_TYPE_OSPF_IF_PARAM_NETWORK_TYPE		(1 << 0)
#define NETCFG_TYPE_OSPF_IF_PARAM_AUTH_TYPE			(1 << 1)
#define NETCFG_TYPE_OSPF_IF_PARAM_PRIORITY			(1 << 2)
#define NETCFG_TYPE_OSPF_IF_PARAM_OUTPUT_COST		(1 << 3)
#define NETCFG_TYPE_OSPF_IF_PARAM_TRANSMIT_DELAY		(1 << 4)
#define NETCFG_TYPE_OSPF_IF_PARAM_RETRANSMIT_INTERVAL	(1 << 5)
#define NETCFG_TYPE_OSPF_IF_PARAM_HELLO_INTERVAL		(1 << 6)
#define NETCFG_TYPE_OSPF_IF_PARAM_DEAD_INTERVAL		(1 << 7)
#define NETCFG_TYPE_OSPF_IF_PARAM_AUTH_SIMPLE		(1 << 8)
#define NETCFG_TYPE_OSPF_IF_PARAM_AUTH_CRYPT		(1 << 9)
#define NETCFG_TYPE_OSPF_IF_PARAM_DATABASE_FILTER		(1 << 10)
#define NETCFG_TYPE_OSPF_IF_PARAM_DISABLE_ALL		(1 << 12)
#define NETCFG_TYPE_OSPF_IF_PARAM_MTU			(1 << 13)
#define NETCFG_TYPE_OSPF_IF_PARAM_MTU_IGNORE		(1 << 14)
#define NETCFG_TYPE_OSPF_IF_PARAM_RESYNC_TIMEOUT		(1 << 15)

    /* OSPF Network Type. */
    UI8_T network_type;

    /* Authentication type. */
    UI8_T auth_type;

    /* Router priority. */
    UI8_T priority;

    /* MTU size. */
    UI16_T mtu;

    /* Interface output cost. */
    UI32_T output_cost;

    /* Interface transmit delay. */
    UI32_T transmit_delay;

    /* Retransmit interval. */
    UI32_T retransmit_interval;

    /* Hello interval. */
    UI32_T hello_interval;

    /* Router dead interval. */
    UI32_T dead_interval;

    /* Resync timeout. */
    UI32_T resync_timeout;

    /* Password for simple. */
    char auth_simple[NETCFG_TYPE_OSPF_AUTH_SIMPLE_SIZE + 1];

    /* Password list for crypt. */
    char auth_crypt[256][NETCFG_TYPE_OSPF_AUTH_MD5_SIZE + 1];
}NETCFG_TYPE_OSPF_IfConfig_T;

/* Row Status. */
#define NETCFG_TYPE_ROW_STATUS_MIN                  1
#define NETCFG_TYPE_ROW_STATUS_ACTIVE		1
#define NETCFG_TYPE_ROW_STATUS_NOTINSERVICE		2
#define NETCFG_TYPE_ROW_STATUS_NOTREADY		3
#define NETCFG_TYPE_ROW_STATUS_CREATEANDGO		4
#define NETCFG_TYPE_ROW_STATUS_CREATEANDWAIT	5
#define NETCFG_TYPE_ROW_STATUS_DESTROY		6
#define NETCFG_TYPE_ROW_STATUS_MAX                  7


#define NETCFG_TYPE_OSPF_REDIST_ENABLE		    (1 << 0)
#define NETCFG_TYPE_OSPF_REDIST_METRIC_TYPE		(1 << 1)
#define NETCFG_TYPE_OSPF_REDIST_METRIC		    (1 << 2)
#define NETCFG_TYPE_OSPF_REDIST_ROUTE_MAP		(1 << 3)
#define NETCFG_TYPE_OSPF_REDIST_TAG			    (1 << 4)
#define NETCFG_TYPE_OSPF_DEFAULT_ORIGINATE_ALWAYS	2

enum NETCFG_TYPE_OSPF_AreaNssa_TranslatorRole_Type_E
{
    NETCFG_TYPE_OSPF_TRANSLATOR_ALAWAYS = 0,
    NETCFG_TYPE_OSPF_TRANSLATOR_CANDIDATE,
    NETCFG_TYPE_OSPF_TRANSLATOR_NEVER
};

enum NETCFG_TYPE_OSPF_AreaNssa_Metric_Type_E
{
    NETCFG_TYPE_OSPF_METRIC_TYPE_1 = 1,
    NETCFG_TYPE_OSPF_METRIC_TYPE_2
};

/* OSPF area nssa parameters. */
typedef struct NETCFG_TYPE_OSPF_Area_Nssa_Para_S
{
	/* Configuration flags.  */
	UI32_T config;
#define NETCFG_TYPE_OSPF_AREA_NSSA_CONF_TRANSLATOR_ROLE   				(1 << 0)
#define NETCFG_TYPE_OSPF_AREA_NSSA_CONF_NO_REDISTRIBUTION   			(1 << 1)
#define NETCFG_TYPE_OSPF_AREA_NSSA_CONF_NO_SUMMARY       				(1 << 2)
#define NETCFG_TYPE_OSPF_AREA_NSSA_CONF_DFLT_INFORMATION    			(1 << 3)
#define NETCFG_TYPE_OSPF_AREA_NSSA_CONF_DFLT_INFORMATION_METRIC    		(1 << 4)
#define NETCFG_TYPE_OSPF_AREA_NSSA_CONF_DFLT_INFORMATION_METRIC_TYPE    (1 << 5)

	enum NETCFG_TYPE_OSPF_AreaNssa_TranslatorRole_Type_E translator_role;

	UI32_T metric;

	enum NETCFG_TYPE_OSPF_AreaNssa_Metric_Type_E metric_type;
}NETCFG_TYPE_OSPF_Area_Nssa_Para_T;

/* OSPF area virtual-link parameters. */
typedef struct NETCFG_TYPE_OSPF_Area_Virtual_Link_Para_S
{
    UI32_T vlink_addr;
        /* Configuration flags.  */
        UI32_T config;
#define NETCFG_TYPE_OSPF_AREA_VLINK_DEAD_INTERVAL   				(1 << 0)
#define NETCFG_TYPE_OSPF_AREA_VLINK_HELLO_INTERVAL  			    (1 << 1)
#define NETCFG_TYPE_OSPF_AREA_VLINK_RETRANSMIT_INTERVAL      		(1 << 2)
#define NETCFG_TYPE_OSPF_AREA_VLINK_TRANSMIT_DELAY    			    (1 << 3)
#define NETCFG_TYPE_OSPF_AREA_VLINK_AUTHENTICATION   		        (1 << 4)
#define NETCFG_TYPE_OSPF_AREA_VLINK_AUTHENTICATIONKEY               (1 << 5)
#define NETCFG_TYPE_OSPF_AREA_VLINK_MESSAGEDIGESTKEY               (1 << 6)
    UI32_T dead_interval;
    UI32_T hello_interval;
    UI32_T retransmit_interval;
    UI32_T transmit_delay;
    char   auth_key[255];
    char   md5_key[255];
    UI32_T key_id;
    UI32_T auth_type;
#define NETCFG_TYPE_OSPF_AUTH_NULL				 0
#define NETCFG_TYPE_OSPF_AUTH_SIMPLE			 1
#define NETCFG_TYPE_OSPF_AUTH_CRYPTOGRAPHIC			 2

}NETCFG_TYPE_OSPF_Area_Virtual_Link_Para_T;

/* OSPF instance statistics. */
typedef struct NETCFG_TYPE_OSPF_Passive_If_S
{
  UI32_T ifindex;
  UI32_T addr;

}NETCFG_TYPE_OSPF_Passive_If_T;

/* OSPF instance statistics. */
typedef struct NETCFG_TYPE_OSPF_Instance_Para_S
{
  /* OSPF process ID. */
  UI32_T ospf_id;

  /* VR ID. */
  UI32_T vr_id;

  /* OSPF Router ID. */
  UI32_T router_id_config;

  /* Configuration variables. */
  UI16_T config;
#define NETCFG_TYPE_OSPF_CONFIG_ROUTER_ID		(1 << 0)/*Lin.Li, used in netcfg*/
#define NETCFG_TYPE_OSPF_CONFIG_DEFAULT_METRIC	(1 << 1)/*Lin.Li, used in netcfg*/
#define NETCFG_TYPE_OSPF_CONFIG_MAX_CONCURRENT_DD	(1 << 2)
#define NETCFG_TYPE_OSPF_CONFIG_RFC1583_COMPATIBLE	(1 << 3)/*Lin.Li, used in netcfg*/
#define NETCFG_TYPE_OSPF_CONFIG_OPAQUE_LSA		(1 << 4)
#define NETCFG_TYPE_OSPF_CONFIG_TRAFFIC_ENGINEERING	(1 << 5)
#define NETCFG_TYPE_OSPF_CONFIG_RESTART_METHOD	(1 << 6)
#define NETCFG_TYPE_OSPF_CONFIG_OVERFLOW_LSDB_LIMIT	(1 << 7)
#define NETCFG_TYPE_OSPF_CONFIG_ROUTER_ID_USE	(1 << 8)

  /* SPF timer config. */
  NETCFG_TYPE_OSPF_Timer_T timer;

  /* Redistribute default metric. */
  UI32_T default_metric;
}NETCFG_TYPE_OSPF_Instance_Para_T;


/* Define for SYSCALL command in netcfg
 */
enum
{
    NETCFG_TYPE_SYSCALL_CMD_ROUTE_MGR_RT6_MTU_CHANGE,
    //NETCFG_TYPE_SYSCALL_CMD_ROUTE_MGR_GET_BEST_ROUTING_INTERFACE,
};

#if (SYS_CPNT_CRAFT_PORT == TRUE)
typedef struct
{
    UI32_T                  ifindex;
    UI32_T                  row_status;
//    UI32_T                  ipv4_role;
    L_INET_AddrIp_T         addr;
    UI32_T                  ipv6_addr_type;
/* example:
enum NETCFG_TYPE_IPV6_ADDRESS_TYPE_E
{
    NETCFG_TYPE_IPV6_ADDRESS_TYPE_LINK_LOCAL = 1,
    NETCFG_TYPE_IPV6_ADDRESS_TYPE_GLOBAL,
    NETCFG_TYPE_IPV6_ADDRESS_TYPE_EUI64
};
*/
    UI32_T                  ipv6_addr_config_type;
/* example:
enum NETCFG_TYPE_IPV6_ADDRESS_CONFIG_TYPE_E
{
    NETCFG_TYPE_IPV6_ADDRESS_CONFIG_TYPE_MANUAL = 1,
    NETCFG_TYPE_IPV6_ADDRESS_CONFIG_TYPE_AUTO_STATELESS,
    NETCFG_TYPE_IPV6_ADDRESS_CONFIG_TYPE_AUTO_STATEFULL,
    NETCFG_TYPE_IPV6_ADDRESS_CONFIG_TYPE_AUTO_OTHER (cmd: ipv6 enable)
};
*/
    UI32_T                  flags;
    UI32_T                  by_user_mode_helper_func; /* 1: yes */
} NETCFG_TYPE_CraftInetAddress_T;
#endif /* SYS_CPNT_CRAFT_PORT */


enum
{
    NETCFG_TYPE_RG_PKT_RA,
    NETCFG_TYPE_RG_PKT_RR,
    NETCFG_TYPE_RG_PKT_MAX
};

/* Trace ID for L_MM
 */
enum
{
    NETCFG_TYPE_TRACE_ID_ND_RAGUARD_LOCALBUILDMREF,
};



#endif   /* _NETCFG_TYPE_H */

