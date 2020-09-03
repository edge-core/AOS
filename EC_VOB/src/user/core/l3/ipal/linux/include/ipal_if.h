/*
 *   File Name: ipal_if.h
 *   Purpose:   TCP/IP shim layer(ipal) L3 Interface/IP Address management implementation API
 *   Note:
 *   Create:    Basen LV     2008.04.06
 *
 *   Histrory:
 *              Modify         Date      Reason
 *              kh_shi      2008/10/22   ipal_interface_mgr.h --> ipal_if.h
 *                                       remove old functions, add new functions
 *
 *   Copyright(C)  Accton Corporation 2007~2009
 */

#ifndef __IPAL_IF_H
#define __IPAL_IF_H

#include "sys_type.h"
#include "ipal_types.h"

/*
 * INCLUDE FILE DECLARATIONS
 */


/*
 * NAMING CONST DECLARATIONS
 */


/*
 * MACRO FUNCTION DECLARATIONS
 */


/*
 * DATA TYPE DECLARATIONS
 */
struct ip_mib {
    unsigned long   ipForwarding;
    unsigned long   ipDefaultTTL;
    unsigned long   ipInReceives;
    unsigned long   ipInHdrErrors;
    unsigned long   ipInAddrErrors;
    unsigned long   ipForwDatagrams;
    unsigned long   ipInUnknownProtos;
    unsigned long   ipInDiscards;
    unsigned long   ipInDelivers;
    unsigned long   ipOutRequests;
    unsigned long   ipOutDiscards;
    unsigned long   ipOutNoRoutes;
    unsigned long   ipReasmTimeout;
    unsigned long   ipReasmReqds;
    unsigned long   ipReasmOKs;
    unsigned long   ipReasmFails;
    unsigned long   ipFragOKs;
    unsigned long   ipFragFails;
    unsigned long   ipFragCreates;
    unsigned long   ipRoutingDiscards;
};

struct icmp_mib {
    unsigned long   icmpInMsgs;
    unsigned long   icmpInErrors;
    unsigned long   icmpInDestUnreachs;
    unsigned long   icmpInTimeExcds;
    unsigned long   icmpInParmProbs;
    unsigned long   icmpInSrcQuenchs;
    unsigned long   icmpInRedirects;
    unsigned long   icmpInEchos;
    unsigned long   icmpInEchoReps;
    unsigned long   icmpInTimestamps;
    unsigned long   icmpInTimestampReps;
    unsigned long   icmpInAddrMasks;
    unsigned long   icmpInAddrMaskReps;
    unsigned long   icmpOutMsgs;
    unsigned long   icmpOutErrors;
    unsigned long   icmpOutDestUnreachs;
    unsigned long   icmpOutTimeExcds;
    unsigned long   icmpOutParmProbs;
    unsigned long   icmpOutSrcQuenchs;
    unsigned long   icmpOutRedirects;
    unsigned long   icmpOutEchos;
    unsigned long   icmpOutEchoReps;
    unsigned long   icmpOutTimestamps;
    unsigned long   icmpOutTimestampReps;
    unsigned long   icmpOutAddrMasks;
    unsigned long   icmpOutAddrMaskReps;
};

struct udp_mib {
    unsigned long   udpInDatagrams;
    unsigned long   udpNoPorts;
    unsigned long   udpInErrors;
    unsigned long   udpOutDatagrams;
};

struct tcp_mib {
    unsigned long   tcpRtoAlgorithm;
    unsigned long   tcpRtoMin;
    unsigned long   tcpRtoMax;
    unsigned long   tcpMaxConn;
    unsigned long   tcpActiveOpens;
    unsigned long   tcpPassiveOpens;
    unsigned long   tcpAttemptFails;
    unsigned long   tcpEstabResets;
    unsigned long   tcpCurrEstab;
    unsigned long   tcpInSegs;
    unsigned long   tcpOutSegs;
    unsigned long   tcpRetransSegs;
    unsigned long   tcpInErrs;
    unsigned long   tcpOutRsts;
    short           tcpInErrsValid;
    short           tcpOutRstsValid;
};

/* replaced with IPAL_Ipv6Statistics_T, IPAL_Icmpv6Statistics_T,
 *   IPAL_Udpv6Statistics_T in ipal_types.h
 */
#if 0
struct ip6_mib {
    unsigned long Ip6InReceives;
    unsigned long Ip6InHdrErrors;
    unsigned long Ip6InTooBigErrors;
    unsigned long Ip6InNoRoutes;
    unsigned long Ip6InAddrErrors;
    unsigned long Ip6InUnknownProtos;
    unsigned long Ip6InTruncatedPkts;
    unsigned long Ip6InDiscards;
    unsigned long Ip6InDelivers;
    unsigned long Ip6OutForwDatagrams;
    unsigned long Ip6OutRequests;
    unsigned long Ip6OutDiscards;
    unsigned long Ip6OutNoRoutes;
    unsigned long Ip6ReasmTimeout;
    unsigned long Ip6ReasmReqds;
    unsigned long Ip6ReasmOKs;
    unsigned long Ip6ReasmFails;
    unsigned long Ip6FragOKs;
    unsigned long Ip6FragFails;
    unsigned long Ip6FragCreates;
    unsigned long Ip6InMcastPkts;
    unsigned long Ip6OutMcastPkts;
};

struct icmp6_mib {
    unsigned long Icmp6InMsgs;
    unsigned long Icmp6InErrors;
    unsigned long Icmp6InDestUnreachs;
    unsigned long Icmp6InPktTooBigs;
    unsigned long Icmp6InTimeExcds;
    unsigned long Icmp6InParmProblems;
    unsigned long Icmp6InEchos;
    unsigned long Icmp6InEchoReplies;
    unsigned long Icmp6InGroupMembQueries;
    unsigned long Icmp6InGroupMembResponses;
    unsigned long Icmp6InGroupMembReductions;
    unsigned long Icmp6InRouterSolicits;
    unsigned long Icmp6InRouterAdvertisements;
    unsigned long Icmp6InNeighborSolicits;
    unsigned long Icmp6InNeighborAdvertisements;
    unsigned long Icmp6InRedirects;
    unsigned long Icmp6OutMsgs;
    unsigned long Icmp6OutDestUnreachs;
    unsigned long Icmp6OutPktTooBigs;
    unsigned long Icmp6OutTimeExcds;
    unsigned long Icmp6OutParmProblems;
    unsigned long Icmp6OutEchoReplies;
    unsigned long Icmp6OutRouterSolicits;
    unsigned long Icmp6OutNeighborSolicits;
    unsigned long Icmp6OutNeighborAdvertisements;
    unsigned long Icmp6OutRedirects;
    unsigned long Icmp6OutGroupMembResponses;
    unsigned long Icmp6OutGroupMembReductions;
};

struct udp6_mib {
    unsigned long Udp6InDatagrams;
    unsigned long Udp6NoPorts;
    unsigned long Udp6InErrors;
    unsigned long Udp6OutDatagrams;
};
#endif /* replaced in ipal_types.h */

typedef struct IPAL_IF_Tcpv4ConnEntry_S
{
    UI32_T    tcp_conn_state;             /*  tcpConnState;           Valid : 1..12   */
    UI32_T    tcp_conn_local_address;     /*  tcpConnLocalAddress;                    */
    UI32_T    tcp_conn_local_port;        /*  tcpConnLocalPort;       Valid : 0..65535*/
    UI32_T    tcp_conn_rem_address;       /*  tcpConnRemAddress;                      */
    UI32_T    tcp_conn_rem_port;          /*  tcpConnRemPort;         Valid : 0..65535*/
}IPAL_IF_Tcpv4ConnEntry_T;

/*  udpEntry : RFC 2013 (SNMPv2 MIB for UDP) Nov-1996   */
typedef struct IPAL_IF_Udpv4Entry_S
{
    UI32_T udp_local_address;
    UI32_T udp_local_port;
} IPAL_IF_Udpv4Entry_T;


typedef enum IPAL_Ipv4StatisticType_E
{
    /* IP */
    IPAL_Ipv4StatisticType_IpForwarding = 0,
    IPAL_Ipv4StatisticType_IpDefaultTTL,
    IPAL_Ipv4StatisticType_IpInReceives,
    IPAL_Ipv4StatisticType_IpInHdrErrors,
    IPAL_Ipv4StatisticType_IpInAddrErrors,
    IPAL_Ipv4StatisticType_IpForwDatagrams,
    IPAL_Ipv4StatisticType_IpInUnknownProtos,
    IPAL_Ipv4StatisticType_IpInDiscards,
    IPAL_Ipv4StatisticType_IpInDelivers,
    IPAL_Ipv4StatisticType_IpOutRequests,
    IPAL_Ipv4StatisticType_IpOutDiscards,
    IPAL_Ipv4StatisticType_IpOutNoRoutes,
    IPAL_Ipv4StatisticType_IpReasmTimeout,
    IPAL_Ipv4StatisticType_IpReasmReqds,
    IPAL_Ipv4StatisticType_IpReasmOKs,
    IPAL_Ipv4StatisticType_IpReasmFails,
    IPAL_Ipv4StatisticType_IpFragOKs,
    IPAL_Ipv4StatisticType_IpFragFails,
    IPAL_Ipv4StatisticType_IpFragCreates,
    /* ipRoutingDiscards not suported on linux. */

    /* ICMP */
    IPAL_Ipv4StatisticType_IcmpInMsgs,
    IPAL_Ipv4StatisticType_IcmpInErrors,
    IPAL_Ipv4StatisticType_IcmpInDestUnreachs,
    IPAL_Ipv4StatisticType_IcmpInTimeExcds,
    IPAL_Ipv4StatisticType_IcmpInParmProbs,
    IPAL_Ipv4StatisticType_IcmpInSrcQuenchs,
    IPAL_Ipv4StatisticType_IcmpInRedirects,
    IPAL_Ipv4StatisticType_IcmpInEchos,
    IPAL_Ipv4StatisticType_IcmpInEchoReps,
    IPAL_Ipv4StatisticType_IcmpInTimestamps,
    IPAL_Ipv4StatisticType_IcmpInTimestampReps,
    IPAL_Ipv4StatisticType_IcmpInAddrMasks,
    IPAL_Ipv4StatisticType_IcmpInAddrMaskReps,
    IPAL_Ipv4StatisticType_IcmpOutMsgs,
    IPAL_Ipv4StatisticType_IcmpOutErrors,
    IPAL_Ipv4StatisticType_IcmpOutDestUnreachs,
    IPAL_Ipv4StatisticType_IcmpOutTimeExcds,
    IPAL_Ipv4StatisticType_IcmpOutParmProbs,
    IPAL_Ipv4StatisticType_IcmpOutSrcQuenchs,
    IPAL_Ipv4StatisticType_IcmpOutRedirects,
    IPAL_Ipv4StatisticType_IcmpOutEchos,
    IPAL_Ipv4StatisticType_IcmpOutEchoReps,
    IPAL_Ipv4StatisticType_IcmpOutTimestamps,
    IPAL_Ipv4StatisticType_IcmpOutTimestampReps,
    IPAL_Ipv4StatisticType_IcmpOutAddrMasks,
    IPAL_Ipv4StatisticType_IcmpOutAddrMaskReps,

    /* TCP */
    IPAL_Ipv4StatisticType_TcpRtoAlgorithm,
    IPAL_Ipv4StatisticType_TcpRtoMin,
    IPAL_Ipv4StatisticType_TcpRtoMax,
    IPAL_Ipv4StatisticType_TcpMaxConn,
    IPAL_Ipv4StatisticType_TcpActiveOpens,
    IPAL_Ipv4StatisticType_TcpPassiveOpens,
    IPAL_Ipv4StatisticType_TcpAttemptFails,
    IPAL_Ipv4StatisticType_TcpEstabResets,
    IPAL_Ipv4StatisticType_TcpCurrEstab,
    IPAL_Ipv4StatisticType_TcpInSegs,
    IPAL_Ipv4StatisticType_TcpOutSegs,
    IPAL_Ipv4StatisticType_TcpRetransSegs,
    IPAL_Ipv4StatisticType_TcpInErrs,
    IPAL_Ipv4StatisticType_TcpOutRsts,

    /* UDP */
    IPAL_Ipv4StatisticType_UdpInDatagrams,
    IPAL_Ipv4StatisticType_UdpNoPorts,
    IPAL_Ipv4StatisticType_UdpInErrors,
    IPAL_Ipv4StatisticType_UdpOutDatagrams,
    IPAL_Ipv4StatisticType_UdpRcvbufErrors,
    IPAL_Ipv4StatisticType_UdpSndbufErrors,

    /* Maximun Type */
    IPAL_Ipv4StatisticType_MAX
} IPAL_Ipv4StatisticType_T;

typedef enum IPAL_Ipv6StatisticType_E
{
    /* IPv6 */
    IPAL_Ipv6StatisticType_Ip6InReceives,
    IPAL_Ipv6StatisticType_Ip6InHdrErrors,
    IPAL_Ipv6StatisticType_Ip6InTooBigErrors,
    IPAL_Ipv6StatisticType_Ip6InNoRoutes,
    IPAL_Ipv6StatisticType_Ip6InAddrErrors,
    IPAL_Ipv6StatisticType_Ip6InUnknownProtos,
    IPAL_Ipv6StatisticType_Ip6InTruncatedPkts,
    IPAL_Ipv6StatisticType_Ip6InDiscards,
    IPAL_Ipv6StatisticType_Ip6InDelivers,
    IPAL_Ipv6StatisticType_Ip6OutForwDatagrams,
    IPAL_Ipv6StatisticType_Ip6OutRequests,
    IPAL_Ipv6StatisticType_Ip6OutDiscards,
    IPAL_Ipv6StatisticType_Ip6OutNoRoutes,
    IPAL_Ipv6StatisticType_Ip6ReasmTimeout,
    IPAL_Ipv6StatisticType_Ip6ReasmReqds,
    IPAL_Ipv6StatisticType_Ip6ReasmOKs,
    IPAL_Ipv6StatisticType_Ip6ReasmFails,
    IPAL_Ipv6StatisticType_Ip6FragOKs,
    IPAL_Ipv6StatisticType_Ip6FragFails,
    IPAL_Ipv6StatisticType_Ip6FragCreates,
    IPAL_Ipv6StatisticType_Ip6InMcastPkts,
    IPAL_Ipv6StatisticType_Ip6OutMcastPkts,

    /* ICMP6 */
    IPAL_Ipv6StatisticType_Icmp6InMsgs,
    IPAL_Ipv6StatisticType_Icmp6InErrors,
    IPAL_Ipv6StatisticType_Icmp6OutMsgs,
    IPAL_Ipv6StatisticType_Icmp6InDestUnreachs,
    IPAL_Ipv6StatisticType_Icmp6InPktTooBigs,
    IPAL_Ipv6StatisticType_Icmp6InTimeExcds,
    IPAL_Ipv6StatisticType_Icmp6InParmProblems,
    IPAL_Ipv6StatisticType_Icmp6InEchos,
    IPAL_Ipv6StatisticType_Icmp6InEchoReplies,
    IPAL_Ipv6StatisticType_Icmp6InGroupMembQueries,
    IPAL_Ipv6StatisticType_Icmp6InGroupMembResponses,
    IPAL_Ipv6StatisticType_Icmp6InGroupMembReductions,
    IPAL_Ipv6StatisticType_Icmp6InRouterSolicits,
    IPAL_Ipv6StatisticType_Icmp6InRouterAdvertisements,
    IPAL_Ipv6StatisticType_Icmp6InNeighborSolicits,
    IPAL_Ipv6StatisticType_Icmp6InNeighborAdvertisements,
    IPAL_Ipv6StatisticType_Icmp6InRedirects,
    IPAL_Ipv6StatisticType_Icmp6InMLDv2Reports,
    IPAL_Ipv6StatisticType_Icmp6OutDestUnreachs,
    IPAL_Ipv6StatisticType_Icmp6OutPktTooBigs,
    IPAL_Ipv6StatisticType_Icmp6OutTimeExcds,
    IPAL_Ipv6StatisticType_Icmp6OutParmProblems,
    IPAL_Ipv6StatisticType_Icmp6OutEchos,
    IPAL_Ipv6StatisticType_Icmp6OutEchoReplies,
    IPAL_Ipv6StatisticType_Icmp6OutGroupMembQueries,
    IPAL_Ipv6StatisticType_Icmp6OutGroupMembResponses,
    IPAL_Ipv6StatisticType_Icmp6OutGroupMembReductions,
    IPAL_Ipv6StatisticType_Icmp6OutRouterSolicits,
    IPAL_Ipv6StatisticType_Icmp6OutRouterAdvertisements,
    IPAL_Ipv6StatisticType_Icmp6OutNeighborSolicits,
    IPAL_Ipv6StatisticType_Icmp6OutNeighborAdvertisements,
    IPAL_Ipv6StatisticType_Icmp6OutRedirects,
    IPAL_Ipv6StatisticType_Icmp6OutMLDv2Reports,

    /* UDP */
    IPAL_Ipv6StatisticType_Udp6InDatagrams,
    IPAL_Ipv6StatisticType_Udp6NoPorts,
    IPAL_Ipv6StatisticType_Udp6InErrors,
    IPAL_Ipv6StatisticType_Udp6OutDatagrams,

    /* Maximun Type */
    IPAL_Ipv6StatisticType_MAX
} IPAL_Ipv6StatisticType_T;

/*
 * STATIC VARIABLE DECLARATIONS
 */


/*
 * LOCAL SUBPROGRAM DECLARATIONS
 */

/* FUNCTION NAME : IPAL_IF_GetSnmpStatisticByTpye
 * PURPOSE:
 *      Get IPv4 SNMP statistic counter
 *
 * INPUT:
 *      stat_type  -- One of snmp type in IPAL_SnmpStatisticType_T
 *
 * OUTPUT:
 *      value_p    -- retrieved value
 *
 * RETURN:
 *      IPAL_RESULT_OK   --  success
 *      IPAL_RESULT_FAIL --  fail
 *
 * NOTES:
 *      1. Linux: Stored in /proc/net/snmp
 */
UI32_T IPAL_IF_GetIpv4StatisticByType(IPAL_Ipv4StatisticType_T stat_type, UI32_T *stat_value_p);


#if (SYS_CPNT_IPV6 == TRUE)
/* FUNCTION NAME : IPAL_IF_GetIpv6StatisticByTpye
 * PURPOSE:
 *      Get IPv6 SNMP statistic counter
 *
 * INPUT:
 *      stat_type  -- One of snmp type in IPAL_Ipv6StatisticType_T
 *
 * OUTPUT:
 *      value_p    -- retrieved value
 *
 * RETURN:
 *      IPAL_RESULT_OK   --  success
 *      IPAL_RESULT_FAIL --  fail
 *
 * NOTES:
 *      1. Linux: stored in /proc/net/Ipv6
 */
UI32_T IPAL_IF_GetIpv6StatisticByType(IPAL_Ipv6StatisticType_T stat_type, UI32_T *stat_value_p);
#endif /* #if (SYS_CPNT_IPV6 == TRUE) */

/* FUNCTION NAME : IPAL_IF_ClearIpv4statistic
 * PURPOSE:
 *      Clear all IPv4 SNMP statistic counters
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      IPAL_RESULT_OK   --  success
 *      IPAL_RESULT_FAIL --  fail
 *
 * NOTES:
 *      Not supported yet
 */
UI32_T IPAL_IF_ClearIpv4statistic();


#if (SYS_CPNT_IPV6 == TRUE)
/* FUNCTION NAME : IPAL_IF_ClearIpv6statistic
 * PURPOSE:
 *      Clear all IPv6 SNMP statistic counters
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      IPAL_RESULT_OK   --  success
 *      IPAL_RESULT_FAIL --  fail
 *
 * NOTES:
 *      Not supported yet
 */
UI32_T IPAL_IF_ClearIpv6statistic();
#endif /* #if (SYS_CPNT_IPV6 == TRUE) */


/*
 *  Name    IPAL_IF_GetIpv4DefaultTtl
 *  Purpose get  IPv4 ipDefaultTTL in TCP/IP stack
 *  Input   int    *ipDefaultTTL
 *  Output  IPv4 ipDefaultTTL value
 *  Return  IPAL_RESULT_OK if success, IPAL_RESULT_FAIL otherwise
 *  Note    None
 */
UI32_T IPAL_IF_GetIpv4DefaultTtl(UI32_T *default_ttl_p);

/*
 *  Name    IPAL_IF_SetIpv4DefaultTtl
 *  Purpose set  IPv4 ipDefaultTTL to TCP/IP stack
 *  Input   int    ipDefaultTTL
 *  Output  none.
 *  Return  IPAL_RESULT_OK if success, IPAL_RESULT_FAIL otherwise
 *  Note    None
 */
UI32_T IPAL_IF_SetIpv4DefaultTtl(UI32_T default_ttl);

/*
 *  Name    IPAL_IF_GetAllIpv4Statistic
 *  Purpose get   ip statistic parameter from  TCP/IP stack
 *  Input   struct ip_mib *ipstat
 *  Output  ipstat.
 *  Return  IPAL_RESULT_OK if success, IPAL_RESULT_FAIL otherwise
 *  Note    None
 */
UI32_T IPAL_IF_GetAllIpv4Statistic(struct ip_mib *ipstat_p);


/*
 *  Name    IPAL_IF_GetAllIcmpStatistic
 *  Purpose get   icmp  statistic parameter from  TCP/IP stack
 *  Input   struct icmp_mib *icmpstat
 *  Output  icmpstat.
 *  Return  IPAL_RESULT_OK if success, IPAL_RESULT_FAIL otherwise
 *  Note    None
 */
UI32_T IPAL_IF_GetAllIcmpStatistic(struct icmp_mib *icmpstat_p);

/*
 *  Name    IPAL_IF_GetAllTcpStatistic
 *  Purpose get  tcp  statistic parameter from  TCP/IP stack
 *  Input   struct tcp_mib *tcpstat
 *  Output  tcpstat.
 *  Return  IPAL_RESULT_OK if success, IPAL_RESULT_FAIL otherwise
 *  Note    None
 */
UI32_T IPAL_IF_GetAllTcpStatistic(struct tcp_mib *tcpstat_p);


/*
 *  Name    IPAL_IF_GetAllUdpStatistic
 *  Purpose get udp statistic parameter from  TCP/IP stack
 *  Input   struct udp_mib *udpstat
 *  Output  udpstat.
 *  Return  IPAL_RESULT_OK if success, IPAL_RESULT_FAIL otherwise
 *  Note    None
 */
UI32_T IPAL_IF_GetAllUdpStatistic(struct udp_mib *udpstat_p);


#if (SYS_CPNT_IPV6 == TRUE)
/*
 *  Name    IPAL_IF_GetAllIpv6Statistic
 *  Purpose get ip6 statistic parameter from  TCP/IP stack
 *  Input   IPAL_Ipv6Statistics_T *ipstat
 *  Output  ipstat.
 *  Return  IPAL_RESULT_OK if success, IPAL_RESULT_FAIL otherwise
 *  Note    None
 */
UI32_T IPAL_IF_GetAllIpv6Statistic(IPAL_Ipv6Statistics_T *ip6stat_p);

/*
 *  Name    IPAL_IF_GetAllIcmp6Statistic
 *  Purpose get icmp6 statistic parameter from  TCP/IP stack
 *  Input   IPAL_Icmpv6Statistics_T *icmp6stat_p
 *  Output  icmp6stat_p.
 *  Return  IPAL_RESULT_OK if success, IPAL_RESULT_FAIL otherwise
 *  Note    None
 */
UI32_T IPAL_IF_GetAllIcmp6Statistic(IPAL_Icmpv6Statistics_T *icmp6stat_p);

/*
 *  Name    IPAL_IF_GetAllUdp6Statistic
 *  Purpose get udp6 statistic parameter from  TCP/IP stack
 *  Input   IPAL_Udpv6Statistics_T *udp6stat_p
 *  Output  udp6stat_p.
 *  Return  IPAL_RESULT_OK if success, IPAL_RESULT_FAIL otherwise
 *  Note    None
 */
UI32_T IPAL_IF_GetAllUdp6Statistic(IPAL_Udpv6Statistics_T *udp6stat_p);
#endif /* #if (SYS_CPNT_IPV6 == TRUE) */

/*
 *  Name    IPAL_GetTcpConnEntry
 *  Purpose get  tcp conn table entry  from  TCP/IP stack
 *  Input   IPAL_TCPV4CONN_ENTRY_T *tcp_conn_entry_p
 *  Output  tcp_conn_entry_p  .
 *  Return  IPAL_RESULT_OK if success, IPAL_RESULT_FAIL otherwise
 *  Note    None
 */
UI32_T IPAL_IF_GetTcpConnEntry(IPAL_IF_Tcpv4ConnEntry_T *tcp_conn_entry_p);

/*
 *  Name    IPAL_GetNextTcpConnEntry
 *  Purpose get next  tcp conn table entry  from  TCP/IP stack
 *  Input   IPAL_TCPV4CONN_ENTRY_T *tcp_conn_entry_p
 *  Output  tcp_conn_entry_p  .
 *  Return  IPAL_RESULT_OK if success, IPAL_RESULT_FAIL otherwise
 *  Note    None
 */
UI32_T IPAL_IF_GetNextTcpConnEntry(IPAL_IF_Tcpv4ConnEntry_T *tcp_conn_entry_p);

/*
 *  Name    IPAL_GetUdpEntry
 *  Purpose get udp table entry  from  TCP/IP stack
 *  Input   IPAL_UDPV4_ENTRY_T *udp_entry_p
 *  Output  udp_entry_p  .
 *  Return  IPAL_RESULT_OK if success, IPAL_RESULT_FAIL otherwise
 *  Note    None
 */
UI32_T IPAL_IF_GetUdpEntry(IPAL_IF_Udpv4Entry_T *udp_entry_p);

/*
 *  Name    IPAL_GetNextUdpEntry
 *  Purpose get next udp table entry  from  TCP/IP stack
 *  Input   IPAL_UDPV4_ENTRY_T *udp_entry_p
 *  Output  udp_entry_p  .
 *  Return  IPAL_RESULT_OK if success, IPAL_RESULT_FAIL otherwise
 *  Note    None
 */
UI32_T IPAL_IF_GetNextUdpEntry(IPAL_IF_Udpv4Entry_T *udp_entry_p);


#if (SYS_CPNT_IPV6 == TRUE)
/* FUNCTION NAME : IPAL_IF_SetIpv6DefaultAutoconfig
 * PURPOSE:
 *      Set default autoconfig value of IPv6. If the value is TRUE,
 *      IPv6 auto config interface address from received Router
 *      Advertisement packet
 *
 * INPUT:
 *      auto_config  --  default autoconfig value
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      IPAL_RESULT_OK   --  success
 *      IPAL_RESULT_FAIL --  fail
 *
 * NOTES:
 *      None
 */
UI32_T IPAL_IF_SetIpv6DefaultAutoconfig(BOOL_T auto_config);


/* FUNCTION NAME : IPAL_IF_EnableIpv6Autoconfig
 * PURPOSE:
 *      Enable IPv6 auto config interface address from received
 *      Router Advertisement
 *
 * INPUT:
 *      ifindex   --  L3 interface index
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      IPAL_RESULT_OK   --  success
 *      IPAL_RESULT_FAIL --  fail
 *
 * NOTES:
 *      None
 */
UI32_T IPAL_IF_EnableIpv6Autoconfig(UI32_T ifindex);


/* FUNCTION NAME : IPAL_IF_DisableIpv6Autoconfig
 * PURPOSE:
 *      Disable IPv6 auto config interface address from received
 *      Router Advertisement
 *
 * INPUT:
 *      ifindex   --  L3 interface index
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      IPAL_RESULT_OK   --  success
 *      IPAL_RESULT_FAIL --  fail
 *
 * NOTES:
 *      None
 */
UI32_T IPAL_IF_DisableIpv6Autoconfig(UI32_T ifindex);


/* FUNCTION NAME : IPAL_IF_EnableAutoLinkLocalAdr
 * PURPOSE:
 *      Enable IPv6 auto config interface link local address
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      IPAL_RESULT_OK   --  success
 *      IPAL_RESULT_FAIL --  fail
 *
 * NOTES:
 *      Not supported yet (currently always enabled)
 */
UI32_T IPAL_IF_EnableAutoLinkLocalAdr();


/* FUNCTION NAME : IPAL_IF_DisableAutoLinkLocalAdr
 * PURPOSE:
 *      Enable IPv6 auto config interface link local address
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      IPAL_RESULT_OK   --  success
 *      IPAL_RESULT_FAIL --  fail
 *
 * NOTES:
 *      Not supported yet (currently always enabled)
 */
UI32_T IPAL_IF_DisableAutoLinkLocalAdr();

/* FUNCTION NAME : IPAL_IF_SetDefaultHopLimit
 * PURPOSE:
 *      Set the default hop limit of interface
 *      (/proc/sys/net/ipv6/conf/default/hop_limit)
 * INPUT:
 *      hop_limit  -- default hop limit
 * OUTPUT:
 *      None
 * RETURN:
 *      IPAL_RESULT_OK   --  success
 *      IPAL_RESULT_FAIL --  fail to set the dad transmit time
 * NOTES:
 */
UI32_T IPAL_IF_SetDefaultHopLimit(UI32_T hop_limit);

/* FUNCTION NAME : IPAL_IF_SetIfHopLimit
 * PURPOSE:
 *      Set the hop limit of interface
 *      (/proc/sys/net/ipv6/conf/eth1/hop_limit)
 * INPUT:
 *      ifindex    -- L3 interface index
 *      hop_limit  -- interface hop limit
 * OUTPUT:
 *      None
 * RETURN:
 *      IPAL_RESULT_OK   --  success
 *      IPAL_RESULT_FAIL --  fail to set the dad transmit time
 * NOTES:
 */
UI32_T IPAL_IF_SetIfHopLimit(UI32_T ifindex, UI32_T hop_limit);

/* FUNCTION NAME : IPAL_IF_EnableIpv6AcceptRaPrefixInfo
 * PURPOSE:
 *      Enable IPv6 interface stateless address from received
 *      Router Advertisement
 *
 * INPUT:
 *      ifindex   --  L3 interface index
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      IPAL_RESULT_OK   --  success
 *      IPAL_RESULT_FAIL --  fail
 *
 * NOTES:
 *      None
 */
UI32_T IPAL_IF_EnableIpv6AcceptRaPrefixInfo(UI32_T ifindex);


/* FUNCTION NAME : IPAL_IF_DisableIpv6AcceptRaPrefixInfo
 * PURPOSE:
 *      Disable IPv6 interface stateless address from received
 *      Router Advertisement
 *
 * INPUT:
 *      ifindex   --  L3 interface index
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      IPAL_RESULT_OK   --  success
 *      IPAL_RESULT_FAIL --  fail
 *
 * NOTES:
 *      None
 */
UI32_T IPAL_IF_DisableIpv6AcceptRaPrefixInfo(UI32_T ifindex);
#endif /* #if (SYS_CPNT_IPV6 == TRUE) */

/*
 *  Name    IPAL_IF_CreateInterface
 *  Purpose Create an interface in TCP/IP stack
 *  Input   UI32_T ifindex
 *          UI8_T *mac_addr
 *  Output  None
 *  Return  IPAL_RESULT_OK   --  success
 *          IPAL_RESULT_FAIL --  fail
 *  Note    None
 */
UI32_T IPAL_IF_CreateInterface (UI32_T ifindex, const UI8_T *mac_addr);

/*
 *  Name    IPAL_IF_CreateLoopbackInterface
 *  Purpose Create an interface in TCP/IP stack
 *  Input   UI32_T ifindex
 *  Output  None
 *  Return  IPAL_RESULT_OK   --  success
 *          IPAL_RESULT_FAIL --  fail
 *  Note    None
 */
UI32_T IPAL_IF_CreateLoopbackInterface (UI32_T ifindex);

/*
 *  Name    IPAL_IF_DestroyInterface
 *  Purpose Delete an interface in TCP/IP stack
 *  Input   UI32_T ifindex
 *  Output  None
 *  Return  IPAL_RESULT_OK   --  success
 *          IPAL_RESULT_FAIL --  fail
 *  Note    None
 */
UI32_T IPAL_IF_DestroyInterface (UI32_T ifindex);

/*
 *  Name    IPAL_IF_DestroyLoopbackInterface
 *  Purpose Delete an interface in TCP/IP stack
 *  Input   UI32_T ifindex
 *  Output  None
 *  Return  0: success
 *          the error no.
 *  Note    None
 */
UI32_T IPAL_IF_DestroyLoopbackInterface (UI32_T ifindex);

/*
 *  Name    IPAL_IF_GetIfFlags
 *  Purpose Get the flags of an interface in TCP/IP stack
 *  Input   UI32_T ifindex
 *          UI16_T *flags_p
 *  Output  None
 *  Return  IPAL_RESULT_OK   --  success
 *          IPAL_RESULT_FAIL --  fail
 *  Note    None
 */
UI32_T IPAL_IF_GetIfFlags (UI32_T ifindex, UI16_T *flags_p);

/*
 *  Name    IPAL_IF_SetIfFlags
 *  Purpose Set the flags of an interface in TCP/IP stack
 *  Input   UI32_T ifindex
 *          UI8_T  *ifname
 *          UI16_T flags
 *  Output  None
 *  Return  IPAL_RESULT_OK   --  success
 *          IPAL_RESULT_FAIL --  fail
 *  Note    None
 */
UI32_T IPAL_IF_SetIfFlags (UI32_T ifindex, UI16_T flags);

/*
 *  Name    IPAL_IF_UnsetIfFlags
 *  Purpose Unset the flags of an interface in TCP/IP stack
 *  Input   UI32_T ifindex
 *          UI16_T flags
 *  Output  None
 *  Return  IPAL_RESULT_OK   --  success
 *          IPAL_RESULT_FAIL --  fail
 *  Note    None
 */
UI32_T IPAL_IF_UnsetIfFlags (UI32_T ifindex, UI16_T flags);

/*
 *  Name    IPAL_IF_GetIfMtu
 *  Purpose Get the MTU of an interface in TCP/IP stack
 *  Input   UI32_T ifindex
 *          UI32_T *mtu_p
 *  Output  None
 *  Return  IPAL_RESULT_OK   --  success
 *          IPAL_RESULT_FAIL --  fail
 *  Note    None
 */
UI32_T IPAL_IF_GetIfMtu (UI32_T ifindex, UI32_T *mtu_p);

/*
 *  Name    IPAL_IF_SetIfMtu
 *  Purpose Set the MTU of an interface in TCP/IP stack
 *  Input   UI32_T ifindex
 *          UI32_T mtu
 *  Output  None
 *  Return  IPAL_RESULT_OK   --  success
 *          IPAL_RESULT_FAIL --  fail
 *  Note    None
 */
UI32_T IPAL_IF_SetIfMtu (UI32_T ifindex, UI32_T mtu);

/*
 *  Name    IPAL_IF_SetIfBandwidth
 *  Purpose Set the bandwidth of an interface in TCP/IP stack
 *  Input   UI32_T ifindex
 *          UI32_T bandwidth
 *  Output  None
 *  Return  IPAL_RESULT_OK   --  success
 *          IPAL_RESULT_FAIL --  fail
 *  Note    None
 */
UI32_T IPAL_IF_SetIfBandwidth (UI32_T ifindex, UI32_T bandwidth);

/*
 *  Name    IPAL_IF_SetIfMac
 *  Purpose Set the MAC address of an interface in TCP/IP stack
 *  Input   UI32_T ifindex
 *          UI8_T *mac_addr
 *  Output  None
 *  Return  IPAL_RESULT_OK   --  success
 *          IPAL_RESULT_FAIL --  fail
 *  Note    None
 */
UI32_T IPAL_IF_SetIfMac (UI32_T ifindex, const UI8_T *mac_addr);

/*
 *  Name    IPAL_IF_AddIpAddress
 *  Purpose Add an IP address of an interface in TCP/IP stack
 *  Input   UI32_T ifindex
 *          L_INET_AddrIp_T *p
 *  Output  None
 *  Return  IPAL_RESULT_OK   --  success
 *          IPAL_RESULT_FAIL --  fail
 *  Note    None
 */
UI32_T IPAL_IF_AddIpAddress (UI32_T ifindex, const L_INET_AddrIp_T *p);

/*
 *  Name    IPAL_IF_AddIpAddressAlias
 *  Purpose Add an alias IP address of an interface in TCP/IP stack
 *  Input   UI32_T ifindex
 *          L_INET_AddrIp_T *p
 *			UI32_T alias_id
 *  Output  None
 *  Return  IPAL_RESULT_OK   --  success
 *          IPAL_RESULT_FAIL --  fail
 *  Note    None
 */
UI32_T IPAL_IF_AddIpAddressAlias (UI32_T ifindex, const L_INET_AddrIp_T *p,UI32_T alias_id);

/*
 *  Name    IPAL_IF_DeleteIpAddress
 *  Purpose Delete an IPv4 address of an interface from TCP/IP stack
 *  Input   UI32_T ifindex
 *          L_INET_AddrIp_T *p
 *  Output  None
 *  Return  IPAL_RESULT_OK   --  success
 *          IPAL_RESULT_FAIL --  fail
 *  Note    None
 */
UI32_T IPAL_IF_DeleteIpAddress (UI32_T ifindex, const L_INET_AddrIp_T *p);

/*
 *  Name    IPAL_IF_AddIpAddress
 *  Purpose Add an IP address of an interface in TCP/IP stack
 *  Input   UI32_T ifindex
 *          L_INET_AddrIp_T *p
 *			UI32_T alias_id
 *  Output  None
 *  Return  IPAL_RESULT_OK   --  success
 *          IPAL_RESULT_FAIL --  fail
 *  Note    None
 */
UI32_T IPAL_IF_DeleteIpAddressAlias (UI32_T ifindex, const L_INET_AddrIp_T *p,UI32_T alias_id);

UI32_T IPAL_IF_GetIpv4IfInfo(UI32_T ifindex, IPAL_IfInfo_T *if_info_p);

#if (SYS_CPNT_IPV6 == TRUE)
UI32_T IPAL_IF_GetIpv6IfInfo(UI32_T ifindex, IPAL_IfInfo_T *if_info_p);
#endif

UI32_T IPAL_IF_GetIfIpv4Addr(UI32_T ifindex, L_INET_AddrIp_T *ipaddr_p);

#if (SYS_CPNT_IPV6 == TRUE)
UI32_T IPAL_IF_GetIfIpv6Addr(UI32_T ifindex, L_INET_AddrIp_T *ipaddr_p);

UI32_T IPAL_IF_GetIfIpv6AddrInfo(UI32_T ifindex, IPAL_IpAddressInfoEntry_T *addr_info_p);

UI32_T IPAL_IF_GetNextIfIpv6AddrInfo(UI32_T ifindex, IPAL_IpAddressInfoEntry_T *addr_info_p);

UI32_T IPAL_IF_GetNextIfJoinIpv6McastAddr(UI32_T ifindex, L_INET_AddrIp_T *mcaddr_p);

/*
 *  Name    IPAL_IF_SetIpv6Mtu
 *  Purpose Set the interface Ipv6 MTU of an interface
 *  Input   UI32_T ifindex
 *          UI32_T mtu
 *  Output  None
 *  Return  IPAL_RESULT_OK   --  success
 *          IPAL_RESULT_FAIL --  fail
 *  Note    None
 */
UI32_T IPAL_IF_SetIpv6Mtu (UI32_T ifindex, UI32_T mtu);

/*
 *  Name    IPAL_IF_GetIpv6Mtu
 *  Purpose Get the interface Ipv6 MTU of an interface
 *  Input   UI32_T ifindex
 *          UI32_T *mtu_p
 *  Output  None
 *  Return  IPAL_RESULT_OK   --  success
 *          IPAL_RESULT_FAIL --  fail
 *  Note    None
 */
UI32_T IPAL_IF_GetIpv6Mtu (UI32_T ifindex, UI32_T *mtu_p);

#endif /* #if (SYS_CPNT_IPV6 == TRUE) */

UI32_T IPAL_IF_ShowIfInfo();

#endif /* end of __IPAL_IF_H */
