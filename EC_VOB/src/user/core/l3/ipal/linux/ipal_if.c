/*
 *   File Name: ipal_if.c
 *   Purpose:   TCP/IP shim layer(ipal) L3 Interface/IP Address management implementation
 *   Note:
 *   Create:    kh_shi     2008.11.26
 *
 *   Histrory:
 *              Modify         Date      Reason
 *
 *
 *   Copyright(C)  Accton Corporation 2007~2009
 */

/*
 * INCLUDE FILE DECLARATIONS
 */
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "stdint.h"

#include "sys_type.h"
#include "sys_cpnt.h"
#include "sys_adpt.h"
#include "sysfun.h"
#include "l_prefix.h"
#include "l_inet.h"
#include "l_sort_lst.h"
#include "l_mm.h"

#include "vlan_type.h"
#include "vlan_lib.h"
#include "netcfg_netdevice.h"
#include "backdoor_mgr.h"

#include "ipal_debug.h"
#include "ipal_if.h"
#include "ipal_types.h"
#include "ipal_ioctl.h"
#include "ipal_sysctl.h"
#include "ipal_rt_netlink.h"

#include <linux/rtnetlink.h>

/*
 * NAMING CONST DECLARATIONS
 */
#define SNMP_STAT_FILE_PATH     "/proc/net/snmp"
#define SNMP6_STAT_FILE_PATH    "/proc/net/snmp6"

#define IP_STATS_LINE   "Ip: %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu"
#define ICMP_STATS_LINE "Icmp: %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu"
#define TCP_STATS_LINE  "Tcp: %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu"
#define UDP_STATS_LINE  "Udp: %lu %lu %lu %lu"

#define IP_STATS_PREFIX_LEN     4
#define ICMP_STATS_PREFIX_LEN   6
#define TCP_STATS_PREFIX_LEN    5
#define UDP_STATS_PREFIX_LEN    5

/*
 * MACRO FUNCTION DECLARATIONS
 */
#define IPAL_IF_VALIDATE_VLAN_IFINDEX(ifindex) \
    if(!IS_VLAN_IFINDEX_VAILD(ifindex))\
        return IPAL_RESULT_FAIL;

/*
 * DATA TYPE DECLARATIONS
 */
struct ip_mib   cached_ip_mib;
struct icmp_mib cached_icmp_mib;
struct tcp_mib  cached_tcp_mib;
struct udp_mib  cached_udp_mib;

/*
 * STATIC VARIABLE DECLARATIONS
 */
/* IPAL_IF_snmp_statistic_label must be consistant with
 * IPAL_Ipv4StatisticType_T enum defined in ipal_if.h
 */
static char *IPAL_IF_snmp_statistic_label[IPAL_Ipv4StatisticType_MAX][2]={
    /* IP */
    {"Ip","Forwarding"},
    {"Ip","DefaultTTL"},
    {"Ip","InReceives"},
    {"Ip","InHdrErrors"},
    {"Ip","InAddrErrors"},
    {"Ip","ForwDatagrams"},
    {"Ip","InUnknownProtos"},
    {"Ip","InDiscards"},
    {"Ip","InDelivers"},
    {"Ip","OutRequests"},
    {"Ip","OutDiscards"},
    {"Ip","OutNoRoutes"},
    {"Ip","ReasmTimeout"},
    {"Ip","ReasmReqds"},
    {"Ip","ReasmOKs"},
    {"Ip","ReasmFails"},
    {"Ip","FragOKs"},
    {"Ip","FragFails"},
    {"Ip","FragCreates"},
    /* ipRoutingDiscards not suported on linux. */

    /* ICMP */
    {"Icmp","InMsgs"},
    {"Icmp","InErrors"},
    {"Icmp","InDestUnreachs"},
    {"Icmp","InTimeExcds"},
    {"Icmp","InParmProbs"},
    {"Icmp","InSrcQuenchs"},
    {"Icmp","InRedirects"},
    {"Icmp","InEchos"},
    {"Icmp","InEchoReps"},
    {"Icmp","InTimestamps"},
    {"Icmp","InTimestampReps"},
    {"Icmp","InAddrMasks"},
    {"Icmp","InAddrMaskReps"},
    {"Icmp","OutMsgs"},
    {"Icmp","OutErrors"},
    {"Icmp","OutDestUnreachs"},
    {"Icmp","OutTimeExcds"},
    {"Icmp","OutParmProbs"},
    {"Icmp","OutSrcQuenchs"},
    {"Icmp","OutRedirects"},
    {"Icmp","OutEchos"},
    {"Icmp","OutEchoReps"},
    {"Icmp","OutTimestamps"},
    {"Icmp","OutTimestampReps"},
    {"Icmp","OutAddrMasks"},
    {"Icmp","OutAddrMaskReps"},

    /* TCP */
    {"Tcp","RtoAlgorithm"},
    {"Tcp","RtoMin"},
    {"Tcp","RtoMax"},
    {"Tcp","MaxConn"},
    {"Tcp","ActiveOpens"},
    {"Tcp","PassiveOpens"},
    {"Tcp","AttemptFails"},
    {"Tcp","EstabResets"},
    {"Tcp","CurrEstab"},
    {"Tcp","InSegs"},
    {"Tcp","OutSegs"},
    {"Tcp","RetransSegs"},
    {"Tcp","InErrs"},
    {"Tcp","OutRsts"},

    /* UDP */
    {"Udp","InDatagrams"},
    {"Udp","NoPorts"},
    {"Udp","InErrors"},
    {"Udp","OutDatagrams"},
    {"Udp","RcvbufErrors"},
    {"Udp","SndbufErrors"}
};

#if (SYS_CPNT_IPV6 == TRUE)
/* IPAL_IF_snmp6_statistic_label must be consistant with
 * IPAL_Ipv6StatisticType_T enum defined in ipal_if.h
 */
static char *IPAL_IF_snmp6_statistic_label[IPAL_Ipv6StatisticType_MAX]=
{
    /* IPv6 */
    "Ip6InReceives",
    "Ip6InHdrErrors",
    "Ip6InTooBigErrors",
    "Ip6InNoRoutes",
    "Ip6InAddrErrors",
    "Ip6InUnknownProtos",
    "Ip6InTruncatedPkts",
    "Ip6InDiscards",
    "Ip6InDelivers",
    "Ip6OutForwDatagrams",
    "Ip6OutRequests",
    "Ip6OutDiscards",
    "Ip6OutNoRoutes",
    "Ip6ReasmTimeout",
    "Ip6ReasmReqds",
    "Ip6ReasmOKs",
    "Ip6ReasmFails",
    "Ip6FragOKs",
    "Ip6FragFails",
    "Ip6FragCreates",
    "Ip6InMcastPkts",
    "Ip6OutMcastPkts",

    /* ICMP6 */
    "Icmp6InMsgs",
    "Icmp6InErrors",
    "Icmp6OutMsgs",
    "Icmp6InDestUnreachs",
    "Icmp6InPktTooBigs",
    "Icmp6InTimeExcds",
    "Icmp6InParmProblems",
    "Icmp6InEchos",
    "Icmp6InEchoReplies",
    "Icmp6InGroupMembQueries",
    "Icmp6InGroupMembResponses",
    "Icmp6InGroupMembReductions",
    "Icmp6InRouterSolicits",
    "Icmp6InRouterAdvertisements",
    "Icmp6InNeighborSolicits",
    "Icmp6InNeighborAdvertisements",
    "Icmp6InRedirects",
    "Icmp6InMLDv2Reports",
    "Icmp6OutDestUnreachs",
    "Icmp6OutPktTooBigs",
    "Icmp6OutTimeExcds",
    "Icmp6OutParmProblems",
    "Icmp6OutEchos",
    "Icmp6OutEchoReplies",
    "Icmp6OutGroupMembQueries",
    "Icmp6OutGroupMembResponses",
    "Icmp6OutGroupMembReductions",
    "Icmp6OutRouterSolicits",
    "Icmp6OutRouterAdvertisements",
    "Icmp6OutNeighborSolicits",
    "Icmp6OutNeighborAdvertisements",
    "Icmp6OutRedirects",
    "Icmp6OutMLDv2Reports",

    /* UDP6 */
    "Udp6InDatagrams",
    "Udp6NoPorts",
    "Udp6InErrors",
    "Udp6OutDatagrams"
};
#endif /* #if (SYS_CPNT_IPV6 == TRUE) */

/*
 * LOCAL SUBPROGRAM DECLARATIONS
 */
static UI32_T IPAL_IF_FetchAllIpv4IfInfo (IPAL_IfInfoList_T **if_list_pp);
static UI32_T IPAL_IF_FetchAllIpv4Addr (IPAL_Ipv4AddressList_T **addr_list_pp);
#if (SYS_CPNT_IPV6 == TRUE)
static UI32_T IPAL_IF_FetchAllIpv6IfInfo (IPAL_IfInfoList_T **if_list_pp);
static UI32_T IPAL_IF_FetchAllIpv6Addr (IPAL_Ipv6AddressList_T **addr_list_pp);
#endif

static int IPAL_IF_TcpConnEntryCompare(void *node_entry, void *input_entry);
static int IPAL_IF_UdpEntryCompare(void *node_entry, void *input_entry);
static UI32_T IPAL_IF_GetMibIIStat(void);
static UI32_T IPAL_IF_GetTcpConnTable(L_SORT_LST_List_T *tcp_entry_list);
static UI32_T IPAL_IF_GetUdpTable(L_SORT_LST_List_T *udp_entry_list);


/*
 * EXPORTED FUNCTION BODY
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
UI32_T IPAL_IF_GetIpv4StatisticByType(IPAL_Ipv4StatisticType_T stat_type, UI32_T *stat_value_p)
{
    char *group_name, *label_name;
    FILE *fp;  /* file descriptor for file operation */
    char buf1[512], buf2[512]; /*Buffer 1 for storing the lable, buffer 2 for storing value */
    char *label_p, *value_p, *p, *q; /* label_p for bufer 1 pointer, value_p for buffer 2, p & q is just for insert "\0" */
    BOOL_T is_found;
    int endflag;

    if ((stat_type < 0) || (stat_type >= IPAL_Ipv4StatisticType_MAX))
    {
        return IPAL_RESULT_FAIL;
    }

    if (stat_value_p == NULL)
    {
        return IPAL_RESULT_FAIL;
    }

    fp = fopen(SNMP_STAT_FILE_PATH, "r");
    if (NULL == fp)
    {
        return IPAL_RESULT_FAIL;
    }

    group_name = IPAL_IF_snmp_statistic_label[stat_type][0];
    label_name = IPAL_IF_snmp_statistic_label[stat_type][1];

    label_p = NULL;
    value_p = NULL;

    /* The file display as follows
     * Icmp: InMsgs InErrors InDestUnreachs InTimeExcds InParmProbs ......
     * Icmp: 79 3 60 16 0 0 0 0 0 0 0 0 0 43 0 43 0 0 0 0 0 0 0 0 0 0
     */
    is_found=FALSE;

    /* Get first line -- label line */
    while (fgets(buf1, sizeof buf1, fp))
    {
        /* Get second line -- value line */
        if (!fgets(buf2, sizeof buf2, fp))
        {
            break;
        }

        label_p = strchr(buf1, ':'); /* Move pointer to postion of ":" */
        value_p = strchr(buf2, ':');
        if (!label_p || !value_p)
        {
            /* If can not find ":", the format must be wrong, return FALSE */
            break;
        }
        *label_p = '\0'; /*insert a end of string into buf1 -- label line */

        if (strcmp(buf1,group_name)==0)
        {
            /* We found the ICMP statistic, now buf1 store label, and buf2 store value*/
            is_found = TRUE;
            break;
        }
    }
    fclose(fp);

    if (TRUE != is_found)
    {
        /* Can not find the ICMP statistic */
        return IPAL_RESULT_FAIL;
    }

    /* Now the line is ICMP statistic, start to parse the content */
    value_p++;
    label_p++;
    endflag = 0;
    while (!endflag)
    {
        label_p += strspn(label_p, " \t\n"); /* skip next blank, move to start of next label */
        value_p += strspn(value_p, " \t\n"); /* skip next blank, move to start of next value*/

        p = label_p+strcspn(label_p, " \t\n"); /* move p to blank after end of label  */
        q = value_p+strcspn(value_p, " \t\n"); /* move q to blank after end of value  */
        if (*p == '\0')
        {
            /* This is the last label we parsed */
            endflag = 1;
        }else{
            /* insert a end of string after label */
            *p = '\0';
            /* insert a end of string after value */
            *q = '\0';
        }

        if (0 == strcmp(label_p,label_name))
        {
            /* We found the specified type, then copy value */
            *stat_value_p = strtoul(value_p, &value_p, 10);
            return IPAL_RESULT_OK;
        }
        label_p = p + 1;
        value_p = q + 1;
    }

    return IPAL_RESULT_FAIL;
}


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
UI32_T IPAL_IF_GetIpv6StatisticByType(IPAL_Ipv6StatisticType_T stat_type, UI32_T *stat_value_p)
{
    char *label_name;
    FILE *fp;  /* file descriptor for file operation */
    char buf[80];
    char *label_p, *p, *value_p, *end_p;

    if ((stat_type < 0) || (stat_type >= IPAL_Ipv6StatisticType_MAX))
    {
        return IPAL_RESULT_FAIL;
    }

    if (stat_value_p == NULL)
    {
        return IPAL_RESULT_FAIL;
    }

    fp = fopen(SNMP6_STAT_FILE_PATH, "r");
    if (NULL == fp)
    {
        return IPAL_RESULT_FAIL;
    }

    label_name = IPAL_IF_snmp6_statistic_label[stat_type];

    while (fgets(buf, sizeof(buf), fp))
    {
        label_p = buf;
        p = label_p + strcspn(label_p, " \t\n");
        *p = '\0';
        if (strcmp(label_p, label_name) == 0)
        {
            value_p = (p+1) + strspn(p+1, " \t\n");
            *stat_value_p = strtoul(value_p, &end_p, 10);
            fclose(fp);
            return IPAL_RESULT_OK;
        }
    }

    fclose(fp);
    return IPAL_RESULT_FAIL;
}
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
UI32_T IPAL_IF_ClearIpv4statistic()
{
    return IPAL_RESULT_FAIL;
}


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
UI32_T IPAL_IF_ClearIpv6statistic()
{
    return IPAL_RESULT_FAIL;
}
#endif /* #if (SYS_CPNT_IPV6 == TRUE) */


/*
 *  Name    IPAL_IF_GetIpv4DefaultTtl
 *  Purpose get  IPv4 ipDefaultTTL in TCP/IP stack
 *  Input   int    *ipDefaultTTL
 *  Output  IPv4 ipDefaultTTL value
 *  Return  IPAL_RESULT_OK if success, IPAL_RESULT_FAIL otherwise
 *  Note    None
 */
UI32_T IPAL_IF_GetIpv4DefaultTtl(UI32_T *default_ttl_p)
{
    if(default_ttl_p == NULL)
        return IPAL_RESULT_FAIL;

    return IPAL_Sysctl_GetIpv4DefaultTtl(default_ttl_p);
}

/*
 *  Name    IPAL_IF_SetIpv4DefaultTtl
 *  Purpose set  IPv4 ipDefaultTTL to TCP/IP stack
 *  Input   int    ipDefaultTTL
 *  Output  none.
 *  Return  IPAL_RESULT_OK if success, IPAL_RESULT_FAIL otherwise
 *  Note    None
 */
UI32_T IPAL_IF_SetIpv4DefaultTtl(UI32_T default_ttl)
{
    FILE    *fp;
    char    ttl_str[16];

    if (( default_ttl <= 0 )|| (default_ttl >255))
        return IPAL_RESULT_FAIL;

    fp = fopen("/proc/sys/net/ipv4/ip_default_ttl", "w");

    /* In some versions of Linux kernel (2.6.22.18 ~ 2.6.28-15, may include other
     * versions too), call sysctl to set ipv4 default TTL will cause exception.
     * It is because in devinet_conf_sysctl() function, it use extra1 field of the
     * ctl_table (use as a pointer to struct ipv4_devconf), but ctl_table ipv4_table[]
     * in sysctl_net_ip4.c didn't define .extra1 field for NET_IPV4_DEFAULT_TTL
     * So we modified to access /proc/sys/net/ipv4/ip_default_ttl directly
     * Will roll back to sysctl function if the newer Linux kernel fix this bug.
     */
    /* return IPAL_Sysctl_SetIpv4DefaultTtl(default_ttl); */

    if (NULL == fp)
        return IPAL_RESULT_FAIL;

    sprintf(ttl_str, "%lu", (unsigned long)default_ttl);
    if (fputs(ttl_str, fp) == EOF)
    {
        fclose(fp);
        return IPAL_RESULT_FAIL;
    }

    fclose(fp);
    return IPAL_RESULT_OK;
}

/*
 *  Name    IPAL_IF_GetAllIpv4Statistic
 *  Purpose get   ip statistic parameter from  TCP/IP stack
 *  Input   struct ip_mib *ipstat
 *  Output  ipstat.
 *  Return  IPAL_RESULT_OK if success, IPAL_RESULT_FAIL otherwise
 *  Note    None
 */
UI32_T IPAL_IF_GetAllIpv4Statistic(struct ip_mib *ipstat_p)
{
    memset((char *) ipstat_p, (0), sizeof(*ipstat_p));
    if (IPAL_IF_GetMibIIStat() != IPAL_RESULT_OK)
        return IPAL_RESULT_FAIL;
    memcpy((char *) ipstat_p, (char *) &cached_ip_mib, sizeof(*ipstat_p));
    return IPAL_RESULT_OK;
}


/*
 *  Name    IPAL_IF_GetAllIcmpStatistic
 *  Purpose get icmp  statistic parameter from  TCP/IP stack
 *  Input   struct icmp_mib *icmpstat
 *  Output  icmpstat.
 *  Return  IPAL_RESULT_OK if success, IPAL_RESULT_FAIL otherwise
 *  Note    None
 */
UI32_T IPAL_IF_GetAllIcmpStatistic(struct icmp_mib *icmpstat_p)
{
    memset((char *) icmpstat_p, (0), sizeof(*icmpstat_p));
    if (IPAL_IF_GetMibIIStat() != IPAL_RESULT_OK)
        return IPAL_RESULT_FAIL;
    memcpy((char *) icmpstat_p, (char *) &cached_icmp_mib,
           sizeof(*icmpstat_p));
    return IPAL_RESULT_OK;
}


/*
 *  Name    IPAL_IF_GetAllTcpStatistic
 *  Purpose get  tcp  statistic parameter from  TCP/IP stack
 *  Input   struct tcp_mib *tcpstat
 *  Output  tcpstat.
 *  Return  IPAL_RESULT_OK if success, IPAL_RESULT_FAIL otherwise
 *  Note    None
 */
UI32_T IPAL_IF_GetAllTcpStatistic(struct tcp_mib *tcpstat_p)
{
    memset((char *) tcpstat_p, (0), sizeof(*tcpstat_p));
    if (IPAL_IF_GetMibIIStat() != IPAL_RESULT_OK)
        return IPAL_RESULT_FAIL;
    memcpy((char *) tcpstat_p, (char *) &cached_tcp_mib, sizeof(*tcpstat_p));
    return IPAL_RESULT_OK;
}

/*
 *  Name    IPAL_IF_GetAllUdpStatistic
 *  Purpose get  udp  statistic parameter from  TCP/IP stack
 *  Input   struct udp_mib *udpstat
 *  Output  udpstat.
 *  Return  IPAL_RESULT_OK if success, IPAL_RESULT_FAIL otherwise
 *  Note    None
 */
UI32_T IPAL_IF_GetAllUdpStatistic(struct udp_mib *udpstat_p)
{
    memset((char *) udpstat_p, (0), sizeof(*udpstat_p));
    if (IPAL_IF_GetMibIIStat() != IPAL_RESULT_OK)
        return IPAL_RESULT_FAIL;
    memcpy((char *) udpstat_p, (char *) &cached_udp_mib, sizeof(*udpstat_p));
    return IPAL_RESULT_OK;
}


#if (SYS_CPNT_IPV6 == TRUE)
/*
 *  Name    IPAL_IF_GetAllIpv6Statistic
 *  Purpose get   ip6 statistic parameter from  TCP/IP stack
 *  Input   IPAL_Ipv6Statistics_T *ipstat
 *  Output  ipstat.
 *  Return  IPAL_RESULT_OK if success, IPAL_RESULT_FAIL otherwise
 *  Note    None
 */
UI32_T IPAL_IF_GetAllIpv6Statistic(IPAL_Ipv6Statistics_T *ip6stat_p)
{
    FILE *fp;  /* file descriptor for file operation */
    char buf[80];
    char *label_p, *p, *value_p, *end_p;
    UI32_T i, *stat_value_p;

    if (ip6stat_p == NULL)
    {
        return IPAL_RESULT_FAIL;
    }

    memset((char *) ip6stat_p, (0), sizeof(*ip6stat_p));

    fp = fopen(SNMP6_STAT_FILE_PATH, "r");
    if (NULL == fp)
    {
        return IPAL_RESULT_FAIL;
    }

    while (fgets(buf, sizeof buf, fp))
    {
        label_p = buf;
        p = label_p + strcspn(label_p, " \t\n");
        *p = '\0';
        for (i=IPAL_Ipv6StatisticType_Ip6InReceives;
             i<=IPAL_Ipv6StatisticType_Ip6OutMcastPkts;
             i++)
        {
            if (strcmp(label_p, IPAL_IF_snmp6_statistic_label[i]) == 0)
            {
                value_p = (p+1) + strspn(p+1, " \t\n");
                stat_value_p = (((UI32_T*)ip6stat_p)+(i-IPAL_Ipv6StatisticType_Ip6InReceives));
                *stat_value_p = strtoul(value_p, &end_p, 10);
                break;
            }
        }
    }

    fclose(fp);
    return IPAL_RESULT_OK;
}

/*
 *  Name    IPAL_IF_GetAllIcmp6Statistic
 *  Purpose get icmp6 statistic parameter from  TCP/IP stack
 *  Input   IPAL_Icmpv6Statistics_T *icmp6stat_p
 *  Output  icmp6stat_p.
 *  Return  IPAL_RESULT_OK if success, IPAL_RESULT_FAIL otherwise
 *  Note    None
 */
UI32_T IPAL_IF_GetAllIcmp6Statistic(IPAL_Icmpv6Statistics_T *icmp6stat_p)
{
    FILE *fp;  /* file descriptor for file operation */
    char buf[80];
    char *label_p, *p, *value_p, *end_p;
    UI32_T i, *stat_value_p;

    if (icmp6stat_p == NULL)
    {
        return IPAL_RESULT_FAIL;
    }

    memset((char *) icmp6stat_p, (0), sizeof(*icmp6stat_p));

    fp = fopen(SNMP6_STAT_FILE_PATH, "r");
    if (NULL == fp)
    {
        return IPAL_RESULT_FAIL;
    }

    while (fgets(buf, sizeof buf, fp))
    {
        label_p = buf;
        p = label_p + strcspn(label_p, " \t\n");
        *p = '\0';
        for (i=IPAL_Ipv6StatisticType_Icmp6InMsgs;
             i<=IPAL_Ipv6StatisticType_Icmp6OutMLDv2Reports;
             i++)
        {
            if (strcmp(label_p, IPAL_IF_snmp6_statistic_label[i]) == 0)
            {
                value_p = (p+1) + strspn(p+1, " \t\n");
                stat_value_p = (((UI32_T*)icmp6stat_p)+(i-IPAL_Ipv6StatisticType_Icmp6InMsgs));
                *stat_value_p = strtoul(value_p, &end_p, 10);
                break;
            }
        }
    }

    fclose(fp);
    return IPAL_RESULT_OK;
}

/*
 *  Name    IPAL_IF_GetAllUdp6Statistic
 *  Purpose get udp6 statistic parameter from  TCP/IP stack
 *  Input   IPAL_Udpv6Statistics_T *udp6stat_p
 *  Output  udp6stat_p.
 *  Return  IPAL_RESULT_OK if success, IPAL_RESULT_FAIL otherwise
 *  Note    None
 */
UI32_T IPAL_IF_GetAllUdp6Statistic(IPAL_Udpv6Statistics_T *udp6stat_p)
{
    FILE *fp;  /* file descriptor for file operation */
    char buf[80];
    char *label_p, *p, *value_p, *end_p;
    UI32_T i, *stat_value_p;

    if (udp6stat_p == NULL)
    {
        return IPAL_RESULT_FAIL;
    }

    memset((char *) udp6stat_p, (0), sizeof(*udp6stat_p));

    fp = fopen(SNMP6_STAT_FILE_PATH, "r");
    if (NULL == fp)
    {
        return IPAL_RESULT_FAIL;
    }

    while (fgets(buf, sizeof buf, fp))
    {
        label_p = buf;
        p = label_p + strcspn(label_p, " \t\n");
        *p = '\0';
        for (i=IPAL_Ipv6StatisticType_Udp6InDatagrams;
             i<=IPAL_Ipv6StatisticType_Udp6OutDatagrams;
             i++)
        {
            if (strcmp(label_p, IPAL_IF_snmp6_statistic_label[i]) == 0)
            {
                value_p = (p+1) + strspn(p+1, " \t\n");
                stat_value_p = (((UI32_T*)udp6stat_p)+(i-IPAL_Ipv6StatisticType_Udp6InDatagrams));
                *stat_value_p = strtoul(value_p, &end_p, 10);
                break;
            }
        }
    }

    fclose(fp);
    return IPAL_RESULT_OK;
}
#endif /* #if (SYS_CPNT_IPV6 == TRUE) */

/*
 *  Name    IPAL_IF_GetTcpConnEntry
 *  Purpose get  tcp conn table entry  from  TCP/IP stack
 *  Input   IPAL_TCPV4CONN_ENTRY_T *tcp_conn_entry_p
 *  Output  tcp_conn_entry_p  .
 *  Return  IPAL_RESULT_OK if success, IPAL_RESULT_FAIL otherwise
 *  Note    None
 */
UI32_T IPAL_IF_GetTcpConnEntry(IPAL_IF_Tcpv4ConnEntry_T *tcp_conn_entry_p)
{
    IPAL_IF_Tcpv4ConnEntry_T    tcp_conn_entry;
    L_SORT_LST_List_T    tcp_entry_list;

    if(tcp_conn_entry_p == NULL)
        return IPAL_RESULT_FAIL;
    if(IPAL_IF_GetTcpConnTable(&tcp_entry_list) == -1)
        return IPAL_RESULT_FAIL;

    memcpy(&tcp_conn_entry, tcp_conn_entry_p, sizeof(IPAL_IF_Tcpv4ConnEntry_T));

    if(L_SORT_LST_Get(&tcp_entry_list, &tcp_conn_entry) == TRUE)
    {
        memcpy(tcp_conn_entry_p, &tcp_conn_entry, sizeof(IPAL_IF_Tcpv4ConnEntry_T));
        L_SORT_LST_Delete_All(&tcp_entry_list);
        return IPAL_RESULT_OK;
    }
    L_SORT_LST_Delete_All(&tcp_entry_list);
    return IPAL_RESULT_FAIL;
}

/*
 *  Name    IPAL_GetNextTcpConnEntry
 *  Purpose get next  tcp conn table entry  from  TCP/IP stack
 *  Input   IPAL_TCPV4CONN_ENTRY_T *tcp_conn_entry_p
 *  Output  tcp_conn_entry_p  .
 *  Return  IPAL_RESULT_OK if success, IPAL_RESULT_FAIL otherwise
 *  Note    None
 */
UI32_T IPAL_IF_GetNextTcpConnEntry(IPAL_IF_Tcpv4ConnEntry_T *tcp_conn_entry_p)
{
    IPAL_IF_Tcpv4ConnEntry_T    tcp_conn_entry;
    L_SORT_LST_List_T    tcp_entry_list;

    if(tcp_conn_entry_p == NULL)
        return IPAL_RESULT_FAIL;
    if(IPAL_IF_GetTcpConnTable(&tcp_entry_list) == -1)
        return IPAL_RESULT_FAIL;

    memcpy(&tcp_conn_entry, tcp_conn_entry_p, sizeof(IPAL_IF_Tcpv4ConnEntry_T));

    if(L_SORT_LST_Get_Next(&tcp_entry_list, &tcp_conn_entry) == TRUE)
    {
        memcpy(tcp_conn_entry_p, &tcp_conn_entry, sizeof(IPAL_IF_Tcpv4ConnEntry_T));
        L_SORT_LST_Delete_All(&tcp_entry_list);
        return IPAL_RESULT_OK;
    }
    L_SORT_LST_Delete_All(&tcp_entry_list);
    return IPAL_RESULT_FAIL;
}


/*
 *  Name    IPAL_IF_GetUdpEntry
 *  Purpose get udp table entry  from  TCP/IP stack
 *  Input   IPAL_UDPV4_ENTRY_T *udp_entry_p
 *  Output  udp_entry_p  .
 *  Return  IPAL_RESULT_OK if success, IPAL_RESULT_FAIL otherwise
 *  Note    None
 */
UI32_T IPAL_IF_GetUdpEntry(IPAL_IF_Udpv4Entry_T *udp_entry_p)
{
    IPAL_IF_Udpv4Entry_T    udp_entry;
    L_SORT_LST_List_T     udp_entry_list;
    if(udp_entry_p == NULL)
        return IPAL_RESULT_FAIL;
    if(IPAL_IF_GetUdpTable(&udp_entry_list) == -1)
        return IPAL_RESULT_FAIL;

    memcpy(&udp_entry, udp_entry_p, sizeof(IPAL_IF_Udpv4Entry_T));

    if(L_SORT_LST_Get(&udp_entry_list, &udp_entry) == TRUE)
    {
        memcpy(udp_entry_p, &udp_entry, sizeof(IPAL_IF_Udpv4Entry_T));
        L_SORT_LST_Delete_All(&udp_entry_list);
        return IPAL_RESULT_OK;
    }

    L_SORT_LST_Delete_All(&udp_entry_list);
    return IPAL_RESULT_FAIL;
}

/*
 *  Name    IPAL_IF_GetNextUdpEntry
 *  Purpose get next udp table entry  from  TCP/IP stack
 *  Input   IPAL_UDPV4_ENTRY_T *udp_entry_p
 *  Output  udp_entry_p  .
 *  Return  IPAL_RESULT_OK if success, IPAL_RESULT_FAIL otherwise
 *  Note    None
 */
UI32_T IPAL_IF_GetNextUdpEntry(IPAL_IF_Udpv4Entry_T *udp_entry_p)
{
    IPAL_IF_Udpv4Entry_T   udp_entry;
    L_SORT_LST_List_T     udp_entry_list;

    if(udp_entry_p == NULL)
        return IPAL_RESULT_FAIL;

    memset(&udp_entry, 0, sizeof(IPAL_IF_Udpv4Entry_T));

    if(IPAL_IF_GetUdpTable(&udp_entry_list) == -1)
        return IPAL_RESULT_FAIL;

    memcpy(&udp_entry, udp_entry_p, sizeof(IPAL_IF_Udpv4Entry_T));

    if(L_SORT_LST_Get_Next(&udp_entry_list, &udp_entry) == TRUE)
    {
        memcpy(udp_entry_p, &udp_entry, sizeof(IPAL_IF_Udpv4Entry_T));
        L_SORT_LST_Delete_All(&udp_entry_list);
        return IPAL_RESULT_OK;
    }

    L_SORT_LST_Delete_All(&udp_entry_list);
    return IPAL_RESULT_FAIL;

}


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
UI32_T IPAL_IF_SetIpv6DefaultAutoconfig(BOOL_T auto_config)
{
    if (auto_config)
        return IPAL_Sysctl_SetIpv6DefaultAutoconf(1);
    else
        return IPAL_Sysctl_SetIpv6DefaultAutoconf(0);
}

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
UI32_T IPAL_IF_EnableIpv6Autoconfig(UI32_T ifindex)
{
    return IPAL_Sysctl_SetIpv6Autoconf(ifindex, 1);
}


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
UI32_T IPAL_IF_DisableIpv6Autoconfig(UI32_T ifindex)
{
    return IPAL_Sysctl_SetIpv6Autoconf(ifindex, 0);
}


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
UI32_T IPAL_IF_EnableIpv6AcceptRaPrefixInfo(UI32_T ifindex)
{
    return IPAL_Sysctl_SetIpv6AcceptRaPrefixInfo(ifindex, 1);
}


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
UI32_T IPAL_IF_DisableIpv6AcceptRaPrefixInfo(UI32_T ifindex)
{
    return IPAL_Sysctl_SetIpv6AcceptRaPrefixInfo(ifindex, 0);
}


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
UI32_T IPAL_IF_EnableAutoLinkLocalAdr()
{
    return IPAL_RESULT_FAIL;
}


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
UI32_T IPAL_IF_DisableAutoLinkLocalAdr()
{
    return IPAL_RESULT_FAIL;
}

/* FUNCTION NAME : IPAL_IF_SetDefaultHopLimit
 * PURPOSE:
 *      Set the transmit times of DAD
 *      (/proc/sys/net/ipv6/conf/eth1/hop_limit)
 * INPUT:
 *      hop_limit  -- default hop limit
 * OUTPUT:
 *      None
 * RETURN:
 *      IPAL_RESULT_OK   --  success
 *      IPAL_RESULT_FAIL --  fail to set the dad transmit time
 * NOTES:
 */
UI32_T IPAL_IF_SetDefaultHopLimit(UI32_T hop_limit)
{
    return IPAL_Sysctl_SetIpv6DefaultHopLimit(hop_limit);
}

/* FUNCTION NAME : IPAL_IF_SetIfHopLimit
 * PURPOSE:
 *      Set the transmit times of DAD
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
UI32_T IPAL_IF_SetIfHopLimit(UI32_T ifindex, UI32_T hop_limit)
{
    return IPAL_Sysctl_SetIpv6HopLimit(ifindex, hop_limit);
}
#endif /* #if (SYS_CPNT_IPV6 == TRUE) */


/*
 *  Name    IPAL_IF_CreateInterface
 *  Purpose Create an interface in TCP/IP stack
 *  Input   UI32_T ifindex
 *          UI8_T *mac_addr
 *  Output  None
 *  Return  0: success
 *          the error no.
 *  Note    None
 */
UI32_T IPAL_IF_CreateInterface (UI32_T ifindex, const UI8_T *mac_addr)
{
    UI32_T vid;

    IPAL_DEBUG_PRINT ("Create Vlan%ld Interface, Mac Address:%s.\r\n", (long)ifindex, mac_addr);

    if (mac_addr == NULL)
        return IPAL_RESULT_FAIL;

    IPAL_IF_VALIDATE_VLAN_IFINDEX(ifindex);
    VLAN_IFINDEX_CONVERTTO_VID(ifindex,vid);

    return SYSFUN_Syscall (SYSFUN_SYSCALL_VLAN_MGR,
                          (void *)VLAN_TYPE_SYSCALL_CMD_CREATE_VLAN_DEV,
                          (void *)(uintptr_t)vid,
                          (void *)mac_addr,
                          0, 0);
}

/*
 *  Name    IPAL_IF_CreateLoopbackInterface
 *  Purpose Create an interface in TCP/IP stack
 *  Input   UI32_T ifindex
 *  Output  None
 *  Return  0: success
 *          the error no.
 *  Note    None
 */
UI32_T IPAL_IF_CreateLoopbackInterface (UI32_T ifindex)
{
    UI32_T lo_id = 0;

    IPAL_DEBUG_PRINT ("Create Loopback Interface, ifindex %lu.\r\n", (unsigned long)ifindex);

    if (!IP_LIB_ConvertLoopbackIfindexToId(ifindex, &lo_id))
        return IPAL_RESULT_FAIL;

    return SYSFUN_Syscall(SYSFUN_SYSCALL_IPAL_IF,
                          (void *)IPAL_TYPE_SYSCALL_CMD_CREATE_LOOPBACK_DEV,
                          (void *)(uintptr_t)lo_id,
                          0, 0, 0);
}

/*
 *  Name    IPAL_IF_DestroyInterface
 *  Purpose Delete an interface in TCP/IP stack
 *  Input   UI32_T ifindex
 *  Output  None
 *  Return  0: success
 *          the error no.
 *  Note    None
 */
UI32_T IPAL_IF_DestroyInterface (UI32_T ifindex)
{
    UI32_T vid;

    IPAL_DEBUG_PRINT ("Destroy Vlan%ld Interface.\r\n", (long)ifindex);

    IPAL_IF_VALIDATE_VLAN_IFINDEX(ifindex);
    VLAN_IFINDEX_CONVERTTO_VID(ifindex,vid);

    return SYSFUN_Syscall (SYSFUN_SYSCALL_VLAN_MGR,
                          (void *)VLAN_TYPE_SYSCALL_CMD_DESTROY_VLAN_DEV,
                          (void *)(uintptr_t)vid,
                          0, 0, 0);
}

/*
 *  Name    IPAL_IF_DestroyLoopbackInterface
 *  Purpose Delete an interface in TCP/IP stack
 *  Input   UI32_T ifindex
 *  Output  None
 *  Return  0: success
 *          the error no.
 *  Note    None
 */
UI32_T IPAL_IF_DestroyLoopbackInterface (UI32_T ifindex)
{
    UI32_T lo_id = 0;

    IPAL_DEBUG_PRINT ("Destroy Loopback Interface, ifindex %lu.\r\n", (unsigned long)ifindex);

    if (!IP_LIB_ConvertLoopbackIfindexToId(ifindex, &lo_id))
        return IPAL_RESULT_FAIL;

    return SYSFUN_Syscall (SYSFUN_SYSCALL_IPAL_IF,
                          (void *)IPAL_TYPE_SYSCALL_CMD_DESTROY_LOOPBACK_DEV,
                          (void *)(uintptr_t)lo_id,
                          0, 0, 0);
}

/*
 *  Name    IPAL_IF_GetIfFlags
 *  Purpose Get the flags of an interface in TCP/IP stack
 *  Input   UI32_T ifindex
 *          UI8_T  *ifname
 *          UI16_T flags
 *  Output  None
 *  Return  0: success
 *          the error no.
 *  Note    None
 */
UI32_T IPAL_IF_GetIfFlags (UI32_T ifindex, UI16_T *flags_p)
{
    char ifname[NETCFG_NETDEVICE_IFNAME_SIZE+1];

    /*IPAL_IF_VALIDATE_VLAN_IFINDEX(ifindex);*/
    if(FALSE == NETCFG_NETDEVICE_IfindexToIfname(ifindex, ifname))
        return IPAL_RESULT_FAIL;

    IPAL_DEBUG_PRINT ("Get interface %s flags\n", ifname);

    return IPAL_Ioctl_GetIfFlags (ifname, flags_p);
}

/*
 *  Name    IPAL_IF_SetIfFlags
 *  Purpose Set the flags of an interface in TCP/IP stack
 *  Input   UI32_T ifindex
 *          UI8_T  *ifname
 *          UI16_T flags
 *  Output  None
 *  Return  0: success
 *          the error no.
 *  Note    None
 */
UI32_T IPAL_IF_SetIfFlags (UI32_T ifindex, UI16_T flags)
{
    char ifname[NETCFG_NETDEVICE_IFNAME_SIZE+1];

    /*IPAL_IF_VALIDATE_VLAN_IFINDEX(ifindex);*/
    if(FALSE == NETCFG_NETDEVICE_IfindexToIfname(ifindex, ifname))
        return IPAL_RESULT_FAIL;

    IPAL_DEBUG_PRINT ("Set interface %s flags: 0x%x", ifname, flags);

    return IPAL_Ioctl_SetIfFlags (ifname, flags);
}

/*
 *  Name    IPAL_IF_UnsetIfFlags
 *  Purpose Unset the flags of an interface in TCP/IP stack
 *  Input   UI32_T ifindex
 *          UI16_T flags
 *  Output  None
 *  Return  0: success
 *          the error no.
 *  Note    None
 */
UI32_T IPAL_IF_UnsetIfFlags (UI32_T ifindex, UI16_T flags)
{
    char ifname[NETCFG_NETDEVICE_IFNAME_SIZE+1];

    /*IPAL_IF_VALIDATE_VLAN_IFINDEX(ifindex);*/
    if(FALSE == NETCFG_NETDEVICE_IfindexToIfname(ifindex, ifname))
        return IPAL_RESULT_FAIL;

    IPAL_DEBUG_PRINT ("UnSet interface %s flags: 0x%x", ifname, flags);

    return IPAL_Ioctl_UnSetIfFlags (ifname, flags);
}


/*
 *  Name    IPAL_IF_GetIfMtu
 *  Purpose Get the MTU of an interface in TCP/IP stack
 *  Input   UI32_T ifindex
 *          UI32_T *mtu_p
 *  Output  None
 *  Return  0: success
 *          the error no.
 *  Note    None
 */
UI32_T IPAL_IF_GetIfMtu (UI32_T ifindex, UI32_T *mtu_p)
{
    char ifname[NETCFG_NETDEVICE_IFNAME_SIZE+1];

    /*IPAL_IF_VALIDATE_VLAN_IFINDEX(ifindex);*/
    if(FALSE == NETCFG_NETDEVICE_IfindexToIfname(ifindex, ifname))
        return IPAL_RESULT_FAIL;

    IPAL_DEBUG_PRINT ("Get interface %s\n", ifname);

    return IPAL_Ioctl_GetIfMtu (ifname, mtu_p);
}


/*
 *  Name    IPAL_IF_SetIfMtu
 *  Purpose Set the MTU of an interface in TCP/IP stack
 *  Input   UI32_T ifindex
 *          UI32_T mtu
 *  Output  None
 *  Return  0: success
 *          the error no.
 *  Note    None
 */
UI32_T IPAL_IF_SetIfMtu (UI32_T ifindex, UI32_T mtu)
{
    char ifname[NETCFG_NETDEVICE_IFNAME_SIZE+1];

    /*IPAL_IF_VALIDATE_VLAN_IFINDEX(ifindex);*/
    if(FALSE == NETCFG_NETDEVICE_IfindexToIfname(ifindex, ifname))
        return IPAL_RESULT_FAIL;

    IPAL_DEBUG_PRINT ("Set interface %s MTU: %ld", ifname, (long)mtu);

    return IPAL_Ioctl_SetIfMtu (ifname, mtu);
}

/*
 *  Name    IPAL_IF_SetIfBandwidth
 *  Purpose Set the bandwidth of an interface in TCP/IP stack
 *  Input   UI32_T ifindex
 *          UI32_T bandwidth
 *  Output  None
 *  Return  0: success
 *          the error no.
 *  Note    None
 */
UI32_T IPAL_IF_SetIfBandwidth (UI32_T ifindex, UI32_T bandwidth)
{
    char ifname[NETCFG_NETDEVICE_IFNAME_SIZE+1];

    /*IPAL_IF_VALIDATE_VLAN_IFINDEX(ifindex);*/
    if(FALSE == NETCFG_NETDEVICE_IfindexToIfname(ifindex, ifname))
        return IPAL_RESULT_FAIL;

    IPAL_DEBUG_PRINT ("Set interface %s Bandwidth: %ld", ifname, (long)bandwidth);

    return IPAL_Ioctl_SetIfBandwidth (ifname, bandwidth);
}

/*
 *  Name    IPAL_IF_SetIfMac
 *  Purpose Set the MAC address of an interface in TCP/IP stack
 *  Input   UI32_T ifindex
 *          UI8_T *mac_addr
 *  Output  None
 *  Return  0: success
 *          the error no.
 *  Note    None
 */
UI32_T IPAL_IF_SetIfMac (UI32_T ifindex, const UI8_T *mac_addr)
{
    char ifname[NETCFG_NETDEVICE_IFNAME_SIZE+1];

    /*IPAL_IF_VALIDATE_VLAN_IFINDEX(ifindex);*/
    if(FALSE == NETCFG_NETDEVICE_IfindexToIfname(ifindex, ifname))
        return IPAL_RESULT_FAIL;

    IPAL_DEBUG_PRINT ("Set interface %s Mac: %x-%x-%x-%x-%x-%x", ifname,
                                        mac_addr[0], mac_addr[1],
                                        mac_addr[2], mac_addr[3],
                                        mac_addr[4], mac_addr[5]);
    if(mac_addr == NULL)
        return IPAL_RESULT_FAIL;

    return IPAL_Ioctl_SetIfMac (ifname, mac_addr);
}

/*
 *  Name    IPAL_IF_AddIpAddress
 *  Purpose Add an IP address of an interface in TCP/IP stack
 *  Input   UI32_T ifindex
 *          L_INET_AddrIp_T *p
 *  Output  None
 *  Return  0: success
 *          the error no.
 *  Note    None
 */
UI32_T IPAL_IF_AddIpAddress (UI32_T ifindex, const L_INET_AddrIp_T *p)
{
    char ifname[NETCFG_NETDEVICE_IFNAME_SIZE + 1];

    /*IPAL_IF_VALIDATE_VLAN_IFINDEX(ifindex);*/
    if(FALSE == NETCFG_NETDEVICE_IfindexToIfname(ifindex, ifname))
        return IPAL_RESULT_FAIL;

    IPAL_DEBUG_PRINT ("Set interface %s IPv4 Address: %d.%d.%d.%d", ifname, p->addr[0], p->addr[1], p->addr[2], p->addr[3]);

    if(p == NULL)
        return IPAL_RESULT_FAIL;

    return IPAL_Rt_Netlink_AddIfIpAddress (ifindex, p);
}

/*
 *  Name    IPAL_IF_AddIpAddressAlias
 *  Purpose Add an alias IP address of an interface in TCP/IP stack
 *  Input   UI32_T ifindex
 *          L_INET_AddrIp_T *p
 *			UI32_T alias_id
 *  Output  None
 *  Return  0: success
 *          the error no.
 *  Note    None
 */
UI32_T IPAL_IF_AddIpAddressAlias (UI32_T ifindex, const L_INET_AddrIp_T *p,UI32_T alias_id)
{
    if(p == NULL)
        return IPAL_RESULT_FAIL;

	IPAL_DEBUG_PRINT ("Set interface %lu alias IPv4 Address: %d.%d.%d.%d:%lu", (unsigned long)ifindex, p->addr[0], p->addr[1], p->addr[2], p->addr[3], (unsigned long)alias_id);

    return IPAL_Rt_Netlink_AddIfIpAddressAlias (ifindex, p,alias_id);
}

/*
 *  Name    IPAL_IF_DeleteIpAddress
 *  Purpose Delete an IPv4 address of an interface from TCP/IP stack
 *  Input   UI32_T ifindex
 *          L_INET_AddrIp_T *p
 *  Output  None
 *  Return  0: success
 *          the error no.
 *  Note    None
 */
UI32_T IPAL_IF_DeleteIpAddress (UI32_T ifindex, const L_INET_AddrIp_T *p)
{
    char ifname[NETCFG_NETDEVICE_IFNAME_SIZE+1];

    /*IPAL_IF_VALIDATE_VLAN_IFINDEX(ifindex);*/
    if(FALSE == NETCFG_NETDEVICE_IfindexToIfname(ifindex, ifname))
        return IPAL_RESULT_FAIL;

    IPAL_DEBUG_PRINT ("Delete interface %s IPv4 Address: %d.%d.%d.%d", ifname, p->addr[0], p->addr[1], p->addr[2], p->addr[3]);

    if(p == NULL)
        return IPAL_RESULT_FAIL;

    return IPAL_Rt_Netlink_DeleteIfIpAddress (ifindex, p);
}

/*
 *  Name    IPAL_IF_AddIpAddress
 *  Purpose Add an IP address of an interface in TCP/IP stack
 *  Input   UI32_T ifindex
 *          L_INET_AddrIp_T *p
 *			UI32_T alias_id
 *  Output  None
 *  Return  0: success
 *          the error no.
 *  Note    None
 */
UI32_T IPAL_IF_DeleteIpAddressAlias (UI32_T ifindex, const L_INET_AddrIp_T *p,UI32_T alias_id)
{
    if(p == NULL)
        return IPAL_RESULT_FAIL;

	IPAL_DEBUG_PRINT ("Delete interface %lu alias IPv4 Address: %d.%d.%d.%d:%lu", (unsigned long)ifindex, p->addr[0], p->addr[1], p->addr[2], p->addr[3], (unsigned long)alias_id);

    return IPAL_Rt_Netlink_DeleteIfIpAddressAlias (ifindex, p,alias_id);
}

static UI32_T IPAL_IF_FetchAllIpv4IfInfo (IPAL_IfInfoList_T **if_list_pp)
{
    return IPAL_Rt_Netlink_FetchAllIpv4Interface (if_list_pp);
}

static UI32_T IPAL_IF_FetchAllIpv4Addr (IPAL_Ipv4AddressList_T **addr_list_pp)
{
    return IPAL_Rt_Netlink_FetchAllIpv4Addr(addr_list_pp);
}

#if (SYS_CPNT_IPV6 == TRUE)
static UI32_T IPAL_IF_FetchAllIpv6IfInfo (IPAL_IfInfoList_T **if_list_pp)
{
    return IPAL_Rt_Netlink_FetchAllIpv6Interface (if_list_pp);
}

static UI32_T IPAL_IF_FetchAllIpv6Addr (IPAL_Ipv6AddressList_T **addr_list_pp)
{
    return IPAL_Rt_Netlink_FetchAllIpv6Addr(addr_list_pp);
}
#endif /* #if (SYS_CPNT_IPV6 == TRUE) */

UI32_T IPAL_IF_GetIpv4IfInfo(UI32_T ifindex, IPAL_IfInfo_T *if_info_p)
{
    IPAL_IfInfoList_T *if_list_p = NULL, *pre_if_list_p;
    UI32_T ret = IPAL_RESULT_FAIL;

    IPAL_IF_FetchAllIpv4IfInfo(&if_list_p);

    while (if_list_p)
    {
        if(ret != IPAL_RESULT_OK &&
           if_list_p->if_entry.ifindex == ifindex)
        {
            memcpy(if_info_p, &(if_list_p->if_entry), sizeof(IPAL_IfInfo_T));
            if_info_p->inet6_flags = 0x0; /* because this is get IPv4 interface info */
            ret = IPAL_RESULT_OK;
        }

        pre_if_list_p = if_list_p;
        if_list_p = if_list_p->next_p;
        L_MM_Free(pre_if_list_p);
    }

    return ret;
}


#if (SYS_CPNT_IPV6 == TRUE)
UI32_T IPAL_IF_GetIpv6IfInfo(UI32_T ifindex, IPAL_IfInfo_T *if_info_p)
{
    IPAL_IfInfoList_T *if_list_p = NULL, *pre_if_list_p;
    UI32_T ret = IPAL_RESULT_FAIL;

    IPAL_IF_FetchAllIpv6IfInfo(&if_list_p);

    while (if_list_p)
    {
        if(if_list_p->if_entry.ifindex == ifindex)
        {
            memcpy(if_info_p, &(if_list_p->if_entry), sizeof(IPAL_IfInfo_T));
            ret = IPAL_RESULT_OK;
        }

        pre_if_list_p = if_list_p;
        if_list_p = if_list_p->next_p;
        L_MM_Free(pre_if_list_p);
    }

    return ret;
}
#endif /* #if (SYS_CPNT_IPV6 == TRUE) */

/* Get first (any) IPv4 address from an interface
 */
UI32_T IPAL_IF_GetIfIpv4Addr(UI32_T ifindex, L_INET_AddrIp_T *ipaddr_p)
{
#if 0
    char ifname[NETCFG_NETDEVICE_IFNAME_SIZE+1];

    IPAL_IF_VALIDATE_VLAN_IFINDEX(ifindex);
    NETCFG_NETDEVICE_IfindexToIfname(ifindex, ifname);

    return IPAL_Ioctl_GetIfIPaddr (ifname, ipaddr_p);
#endif

    IPAL_Ipv4AddressList_T *addr_list_p = NULL, *pre_addr_list_p;
    UI32_T ret = IPAL_RESULT_FAIL;

    IPAL_IF_FetchAllIpv4Addr (&addr_list_p);

    while (addr_list_p)
    {
        if(ret != IPAL_RESULT_OK &&
           addr_list_p->addr_entry.ifindex == ifindex)
        {
            memset(ipaddr_p, 0x0, sizeof(L_INET_AddrIp_T));
            memcpy(ipaddr_p->addr, addr_list_p->addr_entry.addr, SYS_ADPT_IPV4_ADDR_LEN);
            ipaddr_p->addrlen = SYS_ADPT_IPV4_ADDR_LEN;
            ipaddr_p->preflen = addr_list_p->addr_entry.prefixlen;
            if (L_INET_ADDR_IS_IPV4_LINK_LOCAL(ipaddr_p->addr))
            {
                ipaddr_p->type = L_INET_ADDR_TYPE_IPV4Z;
                VLAN_OM_ConvertFromIfindex(ifindex, &(ipaddr_p->zoneid));
            }
            else
            {
                ipaddr_p->type = L_INET_ADDR_TYPE_IPV4;
            }
            ret = IPAL_RESULT_OK;
        }

        pre_addr_list_p = addr_list_p;
        addr_list_p = addr_list_p->next_p;
        L_MM_Free(pre_addr_list_p);
    }

    return ret;
}


#if (SYS_CPNT_IPV6 == TRUE)
/* Get first (any) IPv6 address from an interface
 */
UI32_T IPAL_IF_GetIfIpv6Addr(UI32_T ifindex, L_INET_AddrIp_T *ipaddr_p)
{
    IPAL_Ipv6AddressList_T *addr_list_p = NULL, *pre_addr_list_p;
    UI32_T ret = IPAL_RESULT_FAIL;

    IPAL_IF_FetchAllIpv6Addr (&addr_list_p);

    while (addr_list_p)
    {
        if(ret != IPAL_RESULT_OK &&
           addr_list_p->addr_entry.ifindex == ifindex)
        {
            memset(ipaddr_p, 0x0, sizeof(L_INET_AddrIp_T));
            memcpy(ipaddr_p->addr, addr_list_p->addr_entry.addr, SYS_ADPT_IPV6_ADDR_LEN);
            ipaddr_p->addrlen = SYS_ADPT_IPV6_ADDR_LEN;
            ipaddr_p->preflen = addr_list_p->addr_entry.prefixlen;
            if (L_INET_ADDR_IS_IPV6_LINK_LOCAL(ipaddr_p->addr))
            {
                ipaddr_p->type = L_INET_ADDR_TYPE_IPV6Z;
                VLAN_OM_ConvertFromIfindex(ifindex, &(ipaddr_p->zoneid));
            }
            else
            {
                ipaddr_p->type = L_INET_ADDR_TYPE_IPV6;
            }
            ret = IPAL_RESULT_OK;
        }

        pre_addr_list_p = addr_list_p;
        addr_list_p = addr_list_p->next_p;
        L_MM_Free(pre_addr_list_p);
    }

    return ret;
}

UI32_T IPAL_IF_GetIfIpv6AddrInfo(UI32_T ifindex, IPAL_IpAddressInfoEntry_T *addr_info_p)
{
    IPAL_Ipv6AddressList_T *addr_list_p = NULL, *pre_addr_list_p;
    UI32_T ret = IPAL_RESULT_FAIL;

    IPAL_IF_FetchAllIpv6Addr (&addr_list_p);

    while (addr_list_p)
    {
        if(ret != IPAL_RESULT_OK &&
           addr_list_p->addr_entry.ifindex == ifindex &&
           0 == memcmp(addr_info_p->ipaddr.addr,addr_list_p->addr_entry.addr,SYS_ADPT_IPV6_ADDR_LEN))
        {
            memset(addr_info_p, 0x0, sizeof(IPAL_IpAddressInfoEntry_T));
            memcpy(addr_info_p->ipaddr.addr, addr_list_p->addr_entry.addr, SYS_ADPT_IPV6_ADDR_LEN);
            addr_info_p->ipaddr.addrlen = SYS_ADPT_IPV6_ADDR_LEN;
            addr_info_p->ipaddr.preflen = addr_list_p->addr_entry.prefixlen;
            if (L_INET_ADDR_IS_IPV6_LINK_LOCAL(addr_list_p->addr_entry.addr))
            {
                addr_info_p->ipaddr.type = L_INET_ADDR_TYPE_IPV6Z;
                VLAN_OM_ConvertFromIfindex(ifindex, &(addr_info_p->ipaddr.zoneid));
            }
            else
            {
                addr_info_p->ipaddr.type = L_INET_ADDR_TYPE_IPV6;
            }
            addr_info_p->preferred_lft = addr_list_p->addr_entry.preferred_lft;
            addr_info_p->valid_lft     = addr_list_p->addr_entry.valid_lft;
            addr_info_p->scope         = addr_list_p->addr_entry.scope;
            addr_info_p->deprecated    = !!(addr_list_p->addr_entry.ifa_flags & IFA_F_DEPRECATED);
            addr_info_p->permanent     = !!(addr_list_p->addr_entry.ifa_flags & IFA_F_PERMANENT);
            addr_info_p->tentative     = !!(addr_list_p->addr_entry.ifa_flags & IFA_F_TENTATIVE);
            ret = IPAL_RESULT_OK;
        }

        pre_addr_list_p = addr_list_p;
        addr_list_p = addr_list_p->next_p;
        L_MM_Free(pre_addr_list_p);
    }

    return ret;
}

UI32_T IPAL_IF_GetNextIfIpv6AddrInfo(UI32_T ifindex, IPAL_IpAddressInfoEntry_T *addr_info_p)
{
    IPAL_Ipv6AddressList_T *addr_list_p = NULL, *pre_addr_list_p;
    UI32_T ret = IPAL_RESULT_FAIL;
    BOOL_T  found = FALSE;
    char    all_zero[SYS_ADPT_IPV6_ADDR_LEN] = {0};

    IPAL_IF_FetchAllIpv6Addr (&addr_list_p);

    /* Note: Here we didn't assume the addr_list_p is sorted according to ifindex and/or
     * IPv6 address, so the implementation of getnext here would be a slightly different with
     * normal getnext function. First, we skip all entries which infindex is not match. Then,
     * get first -> first entry found
     * get next  -> next entry from the entry which exact match the input IPv6 address
     * P.S. So we can't get next from the entry which is not found/match the exist entries list
     */
    if (0 == memcmp(addr_info_p->ipaddr.addr, all_zero, SYS_ADPT_IPV6_ADDR_LEN))
        found = TRUE;

    while (addr_list_p)
    {
        /* skip all address entries which are not on the target interface
         */
        if(addr_list_p->addr_entry.ifindex != ifindex)
        {
            pre_addr_list_p = addr_list_p;
            addr_list_p = addr_list_p->next_p;
            L_MM_Free(pre_addr_list_p);
            continue;
        }

        if (found == TRUE)
        {
            memset(addr_info_p, 0x0, sizeof(IPAL_IpAddressInfoEntry_T));
            memcpy(addr_info_p->ipaddr.addr, addr_list_p->addr_entry.addr, SYS_ADPT_IPV6_ADDR_LEN);
            addr_info_p->ipaddr.addrlen = SYS_ADPT_IPV6_ADDR_LEN;
            addr_info_p->ipaddr.preflen = addr_list_p->addr_entry.prefixlen;
            if (L_INET_ADDR_IS_IPV6_LINK_LOCAL(addr_list_p->addr_entry.addr))
            {
                addr_info_p->ipaddr.type = L_INET_ADDR_TYPE_IPV6Z;
                VLAN_OM_ConvertFromIfindex(ifindex, &(addr_info_p->ipaddr.zoneid));
            }
            else
            {
                addr_info_p->ipaddr.type = L_INET_ADDR_TYPE_IPV6;
            }
            addr_info_p->preferred_lft = addr_list_p->addr_entry.preferred_lft;
            addr_info_p->valid_lft     = addr_list_p->addr_entry.valid_lft;
            addr_info_p->scope         = addr_list_p->addr_entry.scope;
            addr_info_p->deprecated    = !!(addr_list_p->addr_entry.ifa_flags & IFA_F_DEPRECATED);
            addr_info_p->permanent     = !!(addr_list_p->addr_entry.ifa_flags & IFA_F_PERMANENT);
            addr_info_p->tentative     = !!(addr_list_p->addr_entry.ifa_flags & IFA_F_TENTATIVE);
            ret = IPAL_RESULT_OK;
            break;
        }
        else
        {
            if (0 == memcmp(addr_info_p->ipaddr.addr,addr_list_p->addr_entry.addr,SYS_ADPT_IPV6_ADDR_LEN))
                found = TRUE;
        }

        pre_addr_list_p = addr_list_p;
        addr_list_p = addr_list_p->next_p;
        L_MM_Free(pre_addr_list_p);
    }

    while (addr_list_p)
    {
        pre_addr_list_p = addr_list_p;
        addr_list_p = addr_list_p->next_p;
        L_MM_Free(pre_addr_list_p);
    }

    return ret;
}

static int parse_hex(char *str, unsigned char *hex, int hex_len)
{
    int len=0;

    while (*str) {
        int tmp;
        if (str[1] == 0)
            return -1;
        if (sscanf(str, "%02x", &tmp) != 1)
            return -1;
        hex[len] = tmp;
        if ((++len) >= hex_len)
            break;
        str += 2;
    }
    return len;
}

UI32_T IPAL_IF_GetNextIfJoinIpv6McastAddr(UI32_T ifindex, L_INET_AddrIp_T *mcaddr_p)
{
    char    buf[256];
    FILE    *fp = fopen("/proc/net/igmp6", "r");
    BOOL_T  found = FALSE;
    char    maddr[L_INET_MAX_IP6ADDR_STR_LEN] = {0};
    char    name[IFNAMSIZ];
    int     index = 0;
    L_INET_AddrIp_T tmpaddr;

    if (!fp)
        return IPAL_RESULT_FAIL;

    while (fgets(buf, sizeof(buf), fp))
    {
        if (3 != sscanf(buf, "%d%s%s", &index, name, maddr))
            continue;

        if (found == TRUE && ifindex != index)
        {
            fclose(fp);
            return IPAL_RESULT_FAIL;
        }

        if (ifindex != index)
            continue;

        if (mcaddr_p->addr[0] == 0)
            found = TRUE;

        memset(&tmpaddr, 0x0, sizeof(L_INET_AddrIp_T));
        if (parse_hex(maddr, tmpaddr.addr, sizeof(tmpaddr.addr))>=0)
        {
            tmpaddr.addrlen = SYS_ADPT_IPV6_ADDR_LEN;
            /* the addresses all are ff02::XX, thus use IPv6Z address type*/
            tmpaddr.type    = L_INET_ADDR_TYPE_IPV6Z;
        }
        else
        {
            fclose(fp);
            return IPAL_RESULT_FAIL;
        }

        if (found == TRUE)
        {
            memcpy(mcaddr_p, &tmpaddr, sizeof(L_INET_AddrIp_T));
            fclose(fp);
            return IPAL_RESULT_OK;
        }
        else
        {
            if (memcmp(mcaddr_p->addr, tmpaddr.addr, SYS_ADPT_IPV6_ADDR_LEN)==0)
                found = TRUE;
        }
    }

    fclose(fp);
    return IPAL_RESULT_FAIL;
}

/*
 *  Name    IPAL_IF_SetIpv6Mtu
 *  Purpose Set the interface Ipv6 MTU of an interface
 *  Input   UI32_T ifindex
 *          UI32_T mtu
 *  Output  None
 *  Return  0: success
 *          the error no.
 *  Note    None
 */
UI32_T IPAL_IF_SetIpv6Mtu (UI32_T ifindex, UI32_T mtu)
{
    return IPAL_Sysctl_SetIpv6Mtu(ifindex, mtu);
}

/*
 *  Name    IPAL_IF_GetIpv6Mtu
 *  Purpose Get the interface Ipv6 MTU of an interface
 *  Input   UI32_T ifindex
 *          UI32_T *mtu_p
 *  Output  None
 *  Return  0: success
 *          the error no.
 *  Note    None
 */
UI32_T IPAL_IF_GetIpv6Mtu (UI32_T ifindex, UI32_T *mtu_p)
{
    return IPAL_Sysctl_GetIpv6Mtu(ifindex, mtu_p);
}

#endif /* #if (SYS_CPNT_IPV6 == TRUE) */

UI32_T IPAL_IF_ShowIfInfo()
{
    IPAL_IfInfoList_T *if_list_p = NULL, *pre_if_list_p;
    UI8_T        mac[18]= {0};

    IPAL_IF_FetchAllIpv4IfInfo (&if_list_p);

    BACKDOOR_MGR_Printf("\r\nIfindex      Ifname          mtu      hw addr\r\n");
    BACKDOOR_MGR_Printf("---------    ---------       -------  -----------------\r\n");

    while (if_list_p)
    {
        sprintf((char *)mac, "%02X-%02X-%02X-%02X-%02X-%02X",
                 if_list_p->if_entry.hw_addr[0], if_list_p->if_entry.hw_addr[1],
                 if_list_p->if_entry.hw_addr[2], if_list_p->if_entry.hw_addr[3],
                 if_list_p->if_entry.hw_addr[4], if_list_p->if_entry.hw_addr[5]);

        BACKDOOR_MGR_Printf("%-10lu %-13s %-16lu %-8s\r\n", (unsigned long)if_list_p->if_entry.ifindex,
            if_list_p->if_entry.ifname, (unsigned long)if_list_p->if_entry.mtu, mac);

        pre_if_list_p = if_list_p;
        if_list_p = if_list_p->next_p;
        L_MM_Free(pre_if_list_p);
    }

    return IPAL_RESULT_OK;
}


/* local functions body
 */
static int IPAL_IF_TcpConnEntryCompare(void *node_entry, void *input_entry)
{
    IPAL_IF_Tcpv4ConnEntry_T *node, *input;

    node = (IPAL_IF_Tcpv4ConnEntry_T *) node_entry;
    input = (IPAL_IF_Tcpv4ConnEntry_T  *) input_entry;

    if (node->tcp_conn_local_address != input->tcp_conn_local_address )
    {
        if(node->tcp_conn_local_address > input->tcp_conn_local_address )
            return 1;
        else
            return -1;
    }
    else if (node->tcp_conn_local_port != input->tcp_conn_local_port)
        return (node->tcp_conn_local_port - input->tcp_conn_local_port);
    else if (node->tcp_conn_rem_address != input->tcp_conn_rem_address )
    {
        if(node->tcp_conn_rem_address > input->tcp_conn_rem_address )
            return 1;
        else
            return -1;
    }
    else
        return (node->tcp_conn_rem_port - input->tcp_conn_rem_port);
}

static int IPAL_IF_UdpEntryCompare(void *node_entry, void *input_entry)
{
    IPAL_IF_Udpv4Entry_T *node, *input;

    node = (IPAL_IF_Udpv4Entry_T *) node_entry;
    input = (IPAL_IF_Udpv4Entry_T  *) input_entry;
    if ( node->udp_local_address != input->udp_local_address)
    {
        if ( node->udp_local_address > input->udp_local_address)
            return 1;
        else
            return -1;
    }
    else
        return (node->udp_local_port - input->udp_local_port);
}


/*get ip tcp udp icmp statistic parameter in tcp/ip stack from proc fs
and save in cache*/
static UI32_T IPAL_IF_GetMibIIStat(void)
{
    FILE *in = fopen(SNMP_STAT_FILE_PATH, "r");
    char line[1024];

    if (!in)
        return IPAL_RESULT_FAIL;

       while (line == fgets(line, sizeof(line), in)) {
           if (!strncmp(line, IP_STATS_LINE, IP_STATS_PREFIX_LEN)) {
               sscanf(line, IP_STATS_LINE,
                      &cached_ip_mib.ipForwarding,
                      &cached_ip_mib.ipDefaultTTL,
                      &cached_ip_mib.ipInReceives,
                      &cached_ip_mib.ipInHdrErrors,
                      &cached_ip_mib.ipInAddrErrors,
                      &cached_ip_mib.ipForwDatagrams,
                      &cached_ip_mib.ipInUnknownProtos,
                      &cached_ip_mib.ipInDiscards,
                      &cached_ip_mib.ipInDelivers,
                      &cached_ip_mib.ipOutRequests,
                      &cached_ip_mib.ipOutDiscards,
                      &cached_ip_mib.ipOutNoRoutes,
                      &cached_ip_mib.ipReasmTimeout,
                      &cached_ip_mib.ipReasmReqds,
                      &cached_ip_mib.ipReasmOKs,
                      &cached_ip_mib.ipReasmFails,
                      &cached_ip_mib.ipFragOKs,
                      &cached_ip_mib.ipFragFails,
                      &cached_ip_mib.ipFragCreates);
               cached_ip_mib.ipRoutingDiscards = 0;        /* XXX */
           } else if (!strncmp(line, ICMP_STATS_LINE, ICMP_STATS_PREFIX_LEN)) {
               sscanf(line, ICMP_STATS_LINE,
                      &cached_icmp_mib.icmpInMsgs,
                      &cached_icmp_mib.icmpInErrors,
                      &cached_icmp_mib.icmpInDestUnreachs,
                      &cached_icmp_mib.icmpInTimeExcds,
                      &cached_icmp_mib.icmpInParmProbs,
                      &cached_icmp_mib.icmpInSrcQuenchs,
                      &cached_icmp_mib.icmpInRedirects,
                      &cached_icmp_mib.icmpInEchos,
                      &cached_icmp_mib.icmpInEchoReps,
                      &cached_icmp_mib.icmpInTimestamps,
                      &cached_icmp_mib.icmpInTimestampReps,
                      &cached_icmp_mib.icmpInAddrMasks,
                      &cached_icmp_mib.icmpInAddrMaskReps,
                      &cached_icmp_mib.icmpOutMsgs,
                      &cached_icmp_mib.icmpOutErrors,
                      &cached_icmp_mib.icmpOutDestUnreachs,
                      &cached_icmp_mib.icmpOutTimeExcds,
                      &cached_icmp_mib.icmpOutParmProbs,
                      &cached_icmp_mib.icmpOutSrcQuenchs,
                      &cached_icmp_mib.icmpOutRedirects,
                      &cached_icmp_mib.icmpOutEchos,
                      &cached_icmp_mib.icmpOutEchoReps,
                      &cached_icmp_mib.icmpOutTimestamps,
                      &cached_icmp_mib.icmpOutTimestampReps,
                      &cached_icmp_mib.icmpOutAddrMasks,
                      &cached_icmp_mib.icmpOutAddrMaskReps);
           } else if (!strncmp(line, TCP_STATS_LINE, TCP_STATS_PREFIX_LEN)) {
               int             ret = sscanf(line, TCP_STATS_LINE,
                                            &cached_tcp_mib.tcpRtoAlgorithm,
                                            &cached_tcp_mib.tcpRtoMin,
                                            &cached_tcp_mib.tcpRtoMax,
                                            &cached_tcp_mib.tcpMaxConn,
                                            &cached_tcp_mib.tcpActiveOpens,
                                            &cached_tcp_mib.tcpPassiveOpens,
                                            &cached_tcp_mib.tcpAttemptFails,
                                            &cached_tcp_mib.tcpEstabResets,
                                            &cached_tcp_mib.tcpCurrEstab,
                                            &cached_tcp_mib.tcpInSegs,
                                            &cached_tcp_mib.tcpOutSegs,
                                            &cached_tcp_mib.tcpRetransSegs,
                                            &cached_tcp_mib.tcpInErrs,
                                            &cached_tcp_mib.tcpOutRsts);
               cached_tcp_mib.tcpInErrsValid = (ret > 12) ? 1 : 0;
               cached_tcp_mib.tcpOutRstsValid = (ret > 13) ? 1 : 0;
           } else if (!strncmp(line, UDP_STATS_LINE, UDP_STATS_PREFIX_LEN)) {
               sscanf(line, UDP_STATS_LINE,
                      &cached_udp_mib.udpInDatagrams,
                      &cached_udp_mib.udpNoPorts,
                      &cached_udp_mib.udpInErrors,
                      &cached_udp_mib.udpOutDatagrams);
           }
       }
       fclose(in);

       /*
        * Tweak illegal values:
        *
        * valid values for ipForwarding are 1 == yup, 2 == nope
        * a 0 is forbidden, so patch:
        */
       if (!cached_ip_mib.ipForwarding)
           cached_ip_mib.ipForwarding = 2;

       /*
        * 0 is illegal for tcpRtoAlgorithm
        * so assume `other' algorithm:
        */
       if (!cached_tcp_mib.tcpRtoAlgorithm)
           cached_tcp_mib.tcpRtoAlgorithm = 1;

       return IPAL_RESULT_OK;
}

/*get tcp conn table from tcp ip stack (rfc 2012)
*/
static UI32_T IPAL_IF_GetTcpConnTable(L_SORT_LST_List_T *tcp_entry_list)
{
    FILE *in;
    char line[256];

    if (L_SORT_LST_Create(tcp_entry_list, 100,
                         sizeof(IPAL_IF_Tcpv4ConnEntry_T), IPAL_IF_TcpConnEntryCompare) == FALSE)
    {
        return IPAL_RESULT_FAIL;
    }

    if (!(in = fopen("/proc/net/tcp", "r")))
    {
        IPAL_DEBUG_PRINT(("mibII/tcpTable", "Failed to load TCP Table (linux1)\n"));
        return IPAL_RESULT_FAIL;
    }

    /*
     * scan proc-file and build up a linked list
     */
    while (line == fgets(line, sizeof(line), in))
    {
        IPAL_IF_Tcpv4ConnEntry_T   tcp_conn_entry;
        static int      linux_states[12] =
            { 1, 5, 3, 4, 6, 7, 11, 1, 8, 9, 2, 10 };
        int             state, lp, fp, uid;

        memset(&tcp_conn_entry, 0, sizeof(IPAL_IF_Tcpv4ConnEntry_T));
        if (6 != sscanf(line,
                        "%*d: %x:%x %x:%x %x %*X:%*X %*X:%*X %*X %d",
                        (int *)&tcp_conn_entry.tcp_conn_local_address, &lp,
                        (int *)&tcp_conn_entry.tcp_conn_rem_address, &fp, &state, &uid))
            continue;

        tcp_conn_entry.tcp_conn_local_port = lp;
        tcp_conn_entry.tcp_conn_rem_port = fp;
        tcp_conn_entry.tcp_conn_state = (state & 0xf) < 12 ? linux_states[state & 0xf] : 2;
        //if (tcp_conn_entry.tcp_conn_state == 5 /* established */ ||
           // tcp_conn_entry.tcp_conn_state == 8 /*  closeWait  */ )
            //tcp_estab++;

        L_SORT_LST_Set(tcp_entry_list, &tcp_conn_entry);
    }

    fclose(in);

    IPAL_DEBUG_PRINT(("mibII/tcpTable", "Loaded TCP Table\n"));
    return IPAL_RESULT_OK;
}

/* get udp table from tcp ip stack (RFC 2013)
 */
static UI32_T IPAL_IF_GetUdpTable(L_SORT_LST_List_T *udp_entry_list)
{
    FILE *in;
    char line[256];

    if (L_SORT_LST_Create(udp_entry_list, 100,
                         sizeof(IPAL_IF_Udpv4Entry_T), IPAL_IF_UdpEntryCompare) == FALSE)
    {
        return IPAL_RESULT_FAIL;
    }

    if (!(in = fopen("/proc/net/udp", "r")))
    {
        IPAL_DEBUG_PRINT("mibII/udpTable" "Failed to read UDP Table \n");
        return IPAL_RESULT_FAIL;
    }

    while (line == fgets(line, sizeof(line), in))
    {
        IPAL_IF_Udpv4Entry_T udpv4_entry;
        unsigned int    state, lport;

        memset(&udpv4_entry, 0, sizeof(IPAL_IF_Udpv4Entry_T));

        if (3 != sscanf(line, "%*d: %x:%x %*x:%*x %x",
                        (int *)&udpv4_entry.udp_local_address, &lport, &state))
            continue;

        if (state != 7)         /* fix me:  UDP_LISTEN ??? */
            continue;

        udpv4_entry.udp_local_port = lport;
        L_SORT_LST_Set(udp_entry_list, &udpv4_entry);
    }

    fclose(in);

    IPAL_DEBUG_PRINT(("mibII/udpTable", "Loaded UDP Table\n"));
    return IPAL_RESULT_OK;
}

