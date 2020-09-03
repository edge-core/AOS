/*
 *   File Name: ipal_sysctl.c
 *   Purpose:   TCP/IP shim layer(ipal) sysctl interface implementation
 *   Note:
 *   Create:    Basen LV     2008.04.06
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
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <errno.h>
#include <linux/sysctl.h>

#include "sys_type.h"
#include "sysfun.h"

#include "l_prefix.h"
#include "ipal_types.h"
#include "ipal_debug.h"

/*
 * NAMING CONST DECLARATIONS
 */



/*
 * MACRO FUNCTION DECLARATIONS
 */


/*
 * DATA TYPE DECLARATIONS
 */


/*
 * STATIC VARIABLE DECLARATIONS
 */



/*
 * LOCAL SUBPROGRAM DECLARATIONS
 */

/*
 *  Enable IPv4 Forwarding in TCP/IP Stack
 */
UI32_T IPAL_Sysctl_EnableIpv4Forwarding ( )
{
    int ip_forward = 1;
    struct __sysctl_args args;
    int name[] = {CTL_NET, NET_IPV4, NET_IPV4_FORWARD };

    memset(&args, 0, sizeof(struct __sysctl_args));

    args.name = name;
    args.nlen = sizeof(name)/sizeof(name[0]);

    args.newval = &ip_forward;
    args.newlen = sizeof(ip_forward);

    if (syscall(SYS__sysctl, &args) == -1) {
        return IPAL_RESULT_FAIL;
    }

    return IPAL_RESULT_OK;
}


/*
 *  Disable IPv4 Forwarding in TCP/IP Stack
 */
UI32_T IPAL_Sysctl_DisableIpv4Forwarding ( )
{
    int ip_forward = 0;
    struct __sysctl_args args;
    int name[] = {CTL_NET, NET_IPV4, NET_IPV4_FORWARD };

    memset(&args, 0, sizeof(struct __sysctl_args));

    args.name = name;
    args.nlen = sizeof(name)/sizeof(name[0]);

    args.newval = &ip_forward;
    args.newlen = sizeof(ip_forward);

    if (syscall(SYS__sysctl, &args) == -1) {
        return IPAL_RESULT_FAIL;
    }

    return IPAL_RESULT_OK;
}

/*
 *  Get IPv4 Forwarding in TCP/IP Stack
 */
UI32_T IPAL_Sysctl_GetIpv4Forwarding (int *ip_forward_p)
{
    size_t var_len;
    int forwarding;
    struct __sysctl_args args;
    int name[] = {CTL_NET, NET_IPV4, NET_IPV4_FORWARD };

    memset(&args, 0, sizeof(struct __sysctl_args));

    args.name = name;
    args.nlen = sizeof(name)/sizeof(name[0]);

    var_len = sizeof(int);
    args.oldval = &forwarding;
    args.oldlenp = &var_len;

    if (syscall(SYS__sysctl, &args) == -1) {
        return IPAL_RESULT_FAIL;
    }

    *ip_forward_p = forwarding;
    return IPAL_RESULT_OK;
}

/*
 *  Enable IPv4 Forwarding on interface in TCP/IP Stack
 */
UI32_T IPAL_Sysctl_EnableIfIpv4Forwarding (UI32_T ifindex)
{

    struct __sysctl_args args;
    int forwarding = 1;
    int name[] = {CTL_NET, NET_IPV4, NET_IPV4_CONF, ifindex, NET_IPV4_CONF_FORWARDING};

    memset(&args, 0, sizeof(struct __sysctl_args));
    args.name = name;
    args.nlen = sizeof(name)/sizeof(name[0]);

    args.newval = &forwarding;
    args.newlen = sizeof(int);

    if (syscall(SYS__sysctl, &args) == -1) {
        return IPAL_RESULT_FAIL;
    }

    return IPAL_RESULT_OK;
}

/*
 *  Disable IPv4 Forwarding on interface in TCP/IP Stack
 */
UI32_T IPAL_Sysctl_DisableIfIpv4Forwarding (UI32_T ifindex)
{
    struct __sysctl_args args;
    int forwarding = 0;
    int name[] = {CTL_NET, NET_IPV4, NET_IPV4_CONF, ifindex, NET_IPV4_CONF_FORWARDING};

    memset(&args, 0, sizeof(struct __sysctl_args));
    args.name = name;
    args.nlen = sizeof(name)/sizeof(name[0]);

    args.newval = &forwarding;
    args.newlen = sizeof(int);

    if (syscall(SYS__sysctl, &args) == -1) {
        return IPAL_RESULT_FAIL;
    }

    return IPAL_RESULT_OK;
}

/*
 *  Get IPv4 Forwarding status on interface in TCP/IP Stack
 */
UI32_T IPAL_Sysctl_GetIfIpv4Forwarding (UI32_T ifindex, int *ip_forward_p)
{
    size_t var_len;
    int forwarding;
    struct __sysctl_args args;
    int name[] = {CTL_NET, NET_IPV4, NET_IPV4_CONF, ifindex, NET_IPV4_CONF_FORWARDING};

    memset(&args, 0, sizeof(struct __sysctl_args));

    args.name = name;
    args.nlen = sizeof(name)/sizeof(name[0]);

    var_len = sizeof(int);
    args.oldval = &forwarding;
    args.oldlenp = &var_len;

    if (syscall(SYS__sysctl, &args) == -1) {
        return IPAL_RESULT_FAIL;
    }

    *ip_forward_p = forwarding;
    return IPAL_RESULT_OK;
}

/*
 *  Enable IPv6 Forwarding in TCP/IP Stack
 */
UI32_T IPAL_Sysctl_EnableIpv6Forwarding ( )
{
    struct __sysctl_args args;
    int ip_forward = 1;

    int name[] = {CTL_NET, NET_IPV6, NET_IPV6_CONF, NET_PROTO_CONF_DEFAULT, NET_IPV6_FORWARDING};
    int name2[] = {CTL_NET, NET_IPV6, NET_IPV6_CONF, NET_PROTO_CONF_ALL, NET_IPV6_FORWARDING};

    /* Change default forwarding
     */
    memset(&args, 0, sizeof(struct __sysctl_args));
    args.name = name;
    args.nlen = sizeof(name)/sizeof(name[0]);

    args.newval = &ip_forward;
    args.newlen = sizeof(UI32_T);

    if (syscall(SYS__sysctl, &args) == -1) {
        return IPAL_RESULT_FAIL;
    }

    /* Change all interface forwarding
     */
    memset(&args, 0, sizeof(struct __sysctl_args));
    args.name = name2;
    args.nlen = sizeof(name2)/sizeof(name2[0]);

    args.newval = &ip_forward;
    args.newlen = sizeof(UI32_T);

    if (syscall(SYS__sysctl, &args) == -1) {
        return IPAL_RESULT_FAIL;
    }

    return IPAL_RESULT_OK;
}


/*
 *  Disable IPv6 Forwarding in TCP/IP Stack
 */
UI32_T IPAL_Sysctl_DisableIpv6Forwarding ( )
{
    struct __sysctl_args args;
    int ip_forward = 0;

    int name[] = {CTL_NET, NET_IPV6, NET_IPV6_CONF, NET_PROTO_CONF_DEFAULT, NET_IPV6_FORWARDING};
    int name2[] = {CTL_NET, NET_IPV6, NET_IPV6_CONF, NET_PROTO_CONF_ALL, NET_IPV6_FORWARDING};

    /* Change default forwarding
     */
    memset(&args, 0, sizeof(struct __sysctl_args));
    args.name = name;
    args.nlen = sizeof(name)/sizeof(name[0]);

    args.newval = &ip_forward;
    args.newlen = sizeof(UI32_T);

    if (syscall(SYS__sysctl, &args) == -1) {
        return IPAL_RESULT_FAIL;
    }

    /* Change all interface forwarding
     */
    memset(&args, 0, sizeof(struct __sysctl_args));
    args.name = name2;
    args.nlen = sizeof(name2)/sizeof(name2[0]);

    args.newval = &ip_forward;
    args.newlen = sizeof(UI32_T);

    if (syscall(SYS__sysctl, &args) == -1) {
        return IPAL_RESULT_FAIL;
    }

    return IPAL_RESULT_OK;
}

/*
 *  Get IPv6 Forwarding in TCP/IP Stack
 */
UI32_T IPAL_Sysctl_GetIpv6Forwarding (int *ip_forward_p)
{
    size_t osnamelth;
    int ip_forward;
    struct __sysctl_args args;
    int name[] = {CTL_NET, NET_IPV6, NET_IPV6_CONF, NET_PROTO_CONF_ALL, NET_IPV6_FORWARDING};

    memset(&args, 0, sizeof(struct __sysctl_args));
    args.name = name;
    args.nlen = sizeof(name)/sizeof(name[0]);

    args.oldval = &ip_forward;
    args.oldlenp = &osnamelth;

    osnamelth = sizeof(int);

    if (syscall(SYS__sysctl, &args) == -1) {
        return IPAL_RESULT_FAIL;
    }

    *ip_forward_p = ip_forward;
    return IPAL_RESULT_OK;
}

#if (SYS_CPNT_PROXY_ARP == TRUE)
/*
 *  Enable IPv4 Arp Proxy in TCP/IP Stack
 */
UI32_T IPAL_Sysctl_EnableProxyArp (UI32_T ifindex)
{
    struct __sysctl_args args;
    int arp_proxy = 1;
    int name[] = {CTL_NET, NET_IPV4, NET_IPV4_CONF, ifindex, NET_IPV4_CONF_PROXY_ARP};

    memset(&args, 0, sizeof(struct __sysctl_args));
    args.name = name;
    args.nlen = sizeof(name)/sizeof(name[0]);

    args.newval = &arp_proxy;
    args.newlen = sizeof(int);

    if (syscall(SYS__sysctl, &args) == -1) {
        return IPAL_RESULT_FAIL;
    }

    return IPAL_RESULT_OK;
}

/*
 *  Disable IPv4 Arp Proxy in TCP/IP Stack
 */
UI32_T IPAL_Sysctl_DisableProxyArp (UI32_T ifindex)
{
    struct __sysctl_args args;
    int arp_proxy = 0;
    int name[] = {CTL_NET, NET_IPV4, NET_IPV4_CONF, ifindex, NET_IPV4_CONF_PROXY_ARP};
    memset(&args, 0, sizeof(struct __sysctl_args));
    args.name = name;
    args.nlen = sizeof(name)/sizeof(name[0]);

    args.newval = &arp_proxy;
    args.newlen = sizeof(int);

    if (syscall(SYS__sysctl, &args) == -1) {
        return IPAL_RESULT_FAIL;
    }

    return IPAL_RESULT_OK;
}
#endif /* #if (SYS_CPNT_PROXY_ARP == TRUE) */

/*
 *  Get IPv4 Arp Proxy in TCP/IP Stack
 */
UI32_T IPAL_Sysctl_GetProxyArp (UI32_T ifindex, int *arp_proxy)
{
    size_t osnamelth;
    int arp_status;
    struct __sysctl_args args;
    int name[] = {CTL_NET, NET_IPV4, NET_IPV4_CONF, ifindex, NET_IPV4_CONF_PROXY_ARP};

    memset(&args, 0, sizeof(struct __sysctl_args));
    args.name = name;
    args.nlen = sizeof(name)/sizeof(name[0]);

    args.oldval = &arp_status;
    args.oldlenp = &osnamelth;

    osnamelth = sizeof(int);

    if (syscall(SYS__sysctl, &args) == -1) {
        return IPAL_RESULT_FAIL;
    }

    *arp_proxy = arp_status;
    return IPAL_RESULT_OK;
}

/*
 * Set IPv4 Dynamic Arp Aging Timeout
 */
UI32_T IPAL_Sysctl_SetIpv4ArpAgingTime (UI32_T timeout)
{
    struct __sysctl_args args;

    int name[] = {CTL_NET, NET_IPV4, NET_IPV4_NEIGH, NET_PROTO_CONF_DEFAULT, NET_NEIGH_REACHABLE_TIME};

    memset(&args, 0, sizeof(struct __sysctl_args));
    args.name = name;
    args.nlen = sizeof(name)/sizeof(name[0]);

    args.newval = &timeout;
    args.newlen = sizeof(UI32_T);

    if (syscall(SYS__sysctl, &args) == -1) {
        return IPAL_RESULT_FAIL;
    }

    return IPAL_RESULT_OK;
}

/*
 * Get IPv4 Dynamic Arp Aging Timeout
 */
UI32_T IPAL_Sysctl_GetIpv4ArpAgingTime (UI32_T *timeout)
{
    size_t osnamelth;
    struct __sysctl_args args;

    int name[] = {CTL_NET, NET_IPV4, NET_IPV4_NEIGH, NET_PROTO_CONF_DEFAULT, NET_NEIGH_REACHABLE_TIME};

    memset(&args, 0, sizeof(struct __sysctl_args));
    args.name = name;
    args.nlen = sizeof(name)/sizeof(name[0]);

    args.oldval = timeout;
    args.oldlenp = &osnamelth;

    osnamelth = sizeof(int);

    if (syscall(SYS__sysctl, &args) == -1) {
        return IPAL_RESULT_FAIL;
    }

    return IPAL_RESULT_OK;
}

/*get ip default ttl.*/
UI32_T IPAL_Sysctl_GetIpv4DefaultTtl(UI32_T *default_ttl_p)
{
    size_t var_len;
    UI32_T ttl;
    struct __sysctl_args args;

    int name[] = {CTL_NET, NET_IPV4, NET_IPV4_DEFAULT_TTL};

    memset(&args, 0, sizeof(struct __sysctl_args));

    args.name = name;
    args.nlen = sizeof(name)/sizeof(name[0]);

    var_len = sizeof(UI32_T);
    args.oldval = &ttl;
    args.oldlenp = &var_len;

    if (syscall(SYS__sysctl, &args) == -1) {
        return IPAL_RESULT_FAIL;
    }

    *default_ttl_p = ttl;
    return IPAL_RESULT_OK;
}

/*set ip default ttl*/
UI32_T IPAL_Sysctl_SetIpv4DefaultTtl(UI32_T default_ttl)
{
    struct __sysctl_args args;

    int name[] = {CTL_NET, NET_IPV4, NET_IPV4_DEFAULT_TTL};

    memset(&args, 0, sizeof(struct __sysctl_args));

    args.name = name;
    args.nlen = sizeof(name)/sizeof(name[0]);

    args.newval = &default_ttl;
    args.newlen = sizeof(default_ttl);

    if (syscall(SYS__sysctl, &args) == -1) {
        return IPAL_RESULT_FAIL;
    }

    return IPAL_RESULT_OK;
}

/*
 * Set IPv6 ICMP Rate Limit
 */
UI32_T IPAL_Sysctl_SetIpv6IcmpRateLimit (UI32_T rate_limit)
{
    struct __sysctl_args args;

    int name[] = {CTL_NET, NET_IPV6, NET_IPV6_ICMP, NET_IPV6_ICMP_RATELIMIT};

    memset(&args, 0, sizeof(struct __sysctl_args));
    args.name = name;
    args.nlen = sizeof(name)/sizeof(name[0]);

    args.newval = &rate_limit;
    args.newlen = sizeof(UI32_T);

    if (syscall(SYS__sysctl, &args) == -1) {
        return IPAL_RESULT_FAIL;
    }

    return IPAL_RESULT_OK;
}


/*
 * Set IPv6 default Neighbor retransmit time (Neighbor Solicitation)
 */
UI32_T IPAL_Sysctl_SetIpv6NeighDefaultRetransTime (UI32_T retrans_time)
{
    struct __sysctl_args args;

    int name[] = {CTL_NET, NET_IPV6, NET_IPV6_NEIGH, NET_PROTO_CONF_DEFAULT, NET_NEIGH_RETRANS_TIME_MS};

    memset(&args, 0, sizeof(struct __sysctl_args));
    args.name = name;
    args.nlen = sizeof(name)/sizeof(name[0]);

    args.newval = &retrans_time;
    args.newlen = sizeof(UI32_T);

    if (syscall(SYS__sysctl, &args) == -1) {
        return IPAL_RESULT_FAIL;
    }

    return IPAL_RESULT_OK;
}


/*
 * Set IPv6 Neighbor retransmit time (Neighbor Solicitation)
 */
UI32_T IPAL_Sysctl_SetIpv6NeighRetransTime (UI32_T ifindex, UI32_T retrans_time)
{
    struct __sysctl_args args;

    int name[] = {CTL_NET, NET_IPV6, NET_IPV6_NEIGH, ifindex, NET_NEIGH_RETRANS_TIME_MS};

    memset(&args, 0, sizeof(struct __sysctl_args));
    args.name = name;
    args.nlen = sizeof(name)/sizeof(name[0]);

    args.newval = &retrans_time;
    args.newlen = sizeof(UI32_T);

    if (syscall(SYS__sysctl, &args) == -1) {
        return IPAL_RESULT_FAIL;
    }

    return IPAL_RESULT_OK;
}

/*
 * Set IPv6 Neighbor Default Reachable Time (aging time)
 */
UI32_T IPAL_Sysctl_SetIpv6NeighDefaultReachableTime (UI32_T reachable_time)
{
    struct __sysctl_args args;

    int name[] = {CTL_NET, NET_IPV6, NET_IPV6_NEIGH, NET_PROTO_CONF_DEFAULT, NET_NEIGH_REACHABLE_TIME_MS};

    memset(&args, 0, sizeof(struct __sysctl_args));
    args.name = name;
    args.nlen = sizeof(name)/sizeof(name[0]);

    args.newval = &reachable_time;
    args.newlen = sizeof(UI32_T);

    if (syscall(SYS__sysctl, &args) == -1) {
        return IPAL_RESULT_FAIL;
    }

    return IPAL_RESULT_OK;
}


/*
 * Set IPv6 Neighbor Interface Reachable Time (aging time)
 */
UI32_T IPAL_Sysctl_SetIpv6NeighReachableTime (UI32_T ifindex, UI32_T reachable_time)
{
    struct __sysctl_args args;

    int name[] = {CTL_NET, NET_IPV6, NET_IPV6_NEIGH, ifindex, NET_NEIGH_REACHABLE_TIME_MS};

    memset(&args, 0, sizeof(struct __sysctl_args));
    args.name = name;
    args.nlen = sizeof(name)/sizeof(name[0]);

    args.newval = &reachable_time;
    args.newlen = sizeof(UI32_T);

    if (syscall(SYS__sysctl, &args) == -1) {
        return IPAL_RESULT_FAIL;
    }

    return IPAL_RESULT_OK;
}

/*
 * Set IPv6 default hop limit
 */
UI32_T IPAL_Sysctl_SetIpv6DefaultHopLimit (UI32_T hop_limit)
{
    struct __sysctl_args args;

    int name[] = {CTL_NET, NET_IPV6, NET_IPV6_CONF, NET_PROTO_CONF_DEFAULT, NET_IPV6_HOP_LIMIT};

    memset(&args, 0, sizeof(struct __sysctl_args));
    args.name = name;
    args.nlen = sizeof(name)/sizeof(name[0]);

    args.newval = &hop_limit;
    args.newlen = sizeof(UI32_T);

    if (syscall(SYS__sysctl, &args) == -1) {
        return IPAL_RESULT_FAIL;
    }

    return IPAL_RESULT_OK;
}


/*
 * Set IPv6 interface hop limit
 */
UI32_T IPAL_Sysctl_SetIpv6HopLimit (UI32_T ifindex, UI32_T hop_limit)
{
    struct __sysctl_args args;

    int name[] = {CTL_NET, NET_IPV6, NET_IPV6_CONF, ifindex, NET_IPV6_HOP_LIMIT};

    memset(&args, 0, sizeof(struct __sysctl_args));
    args.name = name;
    args.nlen = sizeof(name)/sizeof(name[0]);

    args.newval = &hop_limit;
    args.newlen = sizeof(UI32_T);

    if (syscall(SYS__sysctl, &args) == -1) {
        return IPAL_RESULT_FAIL;
    }

    return IPAL_RESULT_OK;
}

/*
 * Set IPv6 DAD transmits time
 */
UI32_T IPAL_Sysctl_SetIpv6DadTransmits (UI32_T ifindex, UI32_T dad_trans_time)
{
    struct __sysctl_args args;

    int name[] = {CTL_NET, NET_IPV6, NET_IPV6_CONF, ifindex, NET_IPV6_DAD_TRANSMITS};

    memset(&args, 0, sizeof(struct __sysctl_args));
    args.name = name;
    args.nlen = sizeof(name)/sizeof(name[0]);

    args.newval = &dad_trans_time;
    args.newlen = sizeof(UI32_T);

    if (syscall(SYS__sysctl, &args) == -1) {
        return IPAL_RESULT_FAIL;
    }

    return IPAL_RESULT_OK;
}


/*
 * Set IPv6 Proxy NDP
 */
UI32_T IPAL_Sysctl_SetIpv6ProxyNdp (UI32_T ifindex, UI32_T proxy_ndp)
{
    struct __sysctl_args args;

    int name[] = {CTL_NET, NET_IPV6, NET_IPV6_CONF, ifindex, NET_IPV6_PROXY_NDP};

    memset(&args, 0, sizeof(struct __sysctl_args));
    args.name = name;
    args.nlen = sizeof(name)/sizeof(name[0]);

    args.newval = &proxy_ndp;
    args.newlen = sizeof(UI32_T);

    if (syscall(SYS__sysctl, &args) == -1) {
        return IPAL_RESULT_FAIL;
    }

    return IPAL_RESULT_OK;
}

/*
 * Get IPv6 Proxy NDP
 */
UI32_T IPAL_Sysctl_GetIpv6ProxyNdp (UI32_T ifindex, UI32_T *proxy_ndp_p)
{
    size_t osnamelth;
    int proxy_ndp;
    struct __sysctl_args args;
    int name[] = {CTL_NET, NET_IPV6, NET_IPV6_CONF, ifindex, NET_IPV6_PROXY_NDP};

    memset(&args, 0, sizeof(struct __sysctl_args));
    args.name = name;
    args.nlen = sizeof(name)/sizeof(name[0]);

    args.oldval = &proxy_ndp;
    args.oldlenp = &osnamelth;

    osnamelth = sizeof(int);

    if (syscall(SYS__sysctl, &args) == -1) {
        return IPAL_RESULT_FAIL;
    }

    *proxy_ndp_p = proxy_ndp;
    return IPAL_RESULT_OK;
}

/*
 * Set IPv6 default autoconfig
 */
UI32_T IPAL_Sysctl_SetIpv6DefaultAutoconf (UI32_T autoconf)
{
    struct __sysctl_args args;

    int name[] = {CTL_NET, NET_IPV6, NET_IPV6_CONF, NET_PROTO_CONF_DEFAULT, NET_IPV6_AUTOCONF};

    memset(&args, 0, sizeof(struct __sysctl_args));
    args.name = name;
    args.nlen = sizeof(name)/sizeof(name[0]);

    args.newval = &autoconf;
    args.newlen = sizeof(UI32_T);

    if (syscall(SYS__sysctl, &args) == -1) {
        return IPAL_RESULT_FAIL;
    }

    return IPAL_RESULT_OK;
}

/*
 * Set IPv6 interface autoconfig
 */
UI32_T IPAL_Sysctl_SetIpv6Autoconf (UI32_T ifindex, UI32_T autoconf)
{
    struct __sysctl_args args;

    int name[] = {CTL_NET, NET_IPV6, NET_IPV6_CONF, ifindex, NET_IPV6_AUTOCONF};

    memset(&args, 0, sizeof(struct __sysctl_args));
    args.name = name;
    args.nlen = sizeof(name)/sizeof(name[0]);

    args.newval = &autoconf;
    args.newlen = sizeof(UI32_T);

    if (syscall(SYS__sysctl, &args) == -1) {
        return IPAL_RESULT_FAIL;
    }

    return IPAL_RESULT_OK;
}


/*
 * Get IPv6 MTU expries time
 */
UI32_T IPAL_Sysctl_GetIpv6MtuExpires(UI32_T *expires_time_p)
{
    size_t osnamelth;
    int expires_time;
    struct __sysctl_args args;
    int name[] = {CTL_NET, NET_IPV6, NET_IPV6_ROUTE, NET_IPV6_ROUTE_MTU_EXPIRES};

    memset(&args, 0, sizeof(struct __sysctl_args));
    args.name = name;
    args.nlen = sizeof(name)/sizeof(name[0]);

    args.oldval = &expires_time;
    args.oldlenp = &osnamelth;

    osnamelth = sizeof(int);

    if (syscall(SYS__sysctl, &args) == -1) {
        return IPAL_RESULT_FAIL;
    }

    *expires_time_p = expires_time;
    return IPAL_RESULT_OK;
}

/*
 * Set IPv6 interface MTU
 */
UI32_T IPAL_Sysctl_SetIpv6Mtu(UI32_T ifindex, UI32_T ipv6_mtu)
{
    struct __sysctl_args args;

    int name[] = {CTL_NET, NET_IPV6, NET_IPV6_CONF, ifindex, NET_IPV6_MTU};

    memset(&args, 0, sizeof(struct __sysctl_args));
    args.name = name;
    args.nlen = sizeof(name)/sizeof(name[0]);

    args.newval = &ipv6_mtu;
    args.newlen = sizeof(UI32_T);

    if (syscall(SYS__sysctl, &args) == -1) {
        return IPAL_RESULT_FAIL;
    }

    return IPAL_RESULT_OK;
}

/*
 * Get IPv6 interface MTU
 */
UI32_T IPAL_Sysctl_GetIpv6Mtu(UI32_T ifindex, UI32_T *ipv6_mtu_p)
{
    size_t osnamelth;
    int ipv6_mtu;
    struct __sysctl_args args;
    int name[] = {CTL_NET, NET_IPV6, NET_IPV6_CONF, ifindex, NET_IPV6_MTU};

    memset(&args, 0, sizeof(struct __sysctl_args));
    args.name = name;
    args.nlen = sizeof(name)/sizeof(name[0]);

    args.oldval = &ipv6_mtu;
    args.oldlenp = &osnamelth;

    osnamelth = sizeof(int);

    if (syscall(SYS__sysctl, &args) == -1) {
        return IPAL_RESULT_FAIL;
    }

    *ipv6_mtu_p = ipv6_mtu;
    return IPAL_RESULT_OK;
}

/*
 * Set IPv6 interface accept ra prefix info
 */
UI32_T IPAL_Sysctl_SetIpv6AcceptRaPrefixInfo (UI32_T ifindex, UI32_T accept_ra_pinfo)
{
    struct __sysctl_args args;

    int name[] = {CTL_NET, NET_IPV6, NET_IPV6_CONF, ifindex, NET_IPV6_ACCEPT_RA_PINFO};

    memset(&args, 0, sizeof(struct __sysctl_args));
    args.name = name;
    args.nlen = sizeof(name)/sizeof(name[0]);

    args.newval = &accept_ra_pinfo;
    args.newlen = sizeof(UI32_T);

    if (syscall(SYS__sysctl, &args) == -1) {
        return IPAL_RESULT_FAIL;
    }

    return IPAL_RESULT_OK;
}

