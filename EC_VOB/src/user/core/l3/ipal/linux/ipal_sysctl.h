/*
 *   File Name: ipal_sysctl.H
 *   Purpose:   TCP/IP shim layer(ipal) sysctl utility
 *   Note:
 *   Create:    Basen LV     2008.04.06
 *
 *   Histrory:
 *              Modify		   Date      Reason
 *
 *
 *   Copyright(C)  Accton Corporation 2007~2009
 */

#ifndef __IPAL_SYSCTL_H
#define __IPAL_SYSCTL_H

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


/*
 *  Enable IPv4 Forwarding in TCP/IP Stack
 */
UI32_T IPAL_Sysctl_EnableIpv4Forwarding ( );

/*
 *  Disable IPv4 Forwarding in TCP/IP Stack
 */
UI32_T IPAL_Sysctl_DisableIpv4Forwarding ( );

/*
 *  Get IPv4 Forwarding in TCP/IP Stack
 */
UI32_T IPAL_Sysctl_GetIpv4Forwarding (int *ip_forward_p);

/*
 *  Enable IPv4 Forwarding on interface in TCP/IP Stack
 */
UI32_T IPAL_Sysctl_EnableIfIpv4Forwarding (UI32_T ifindex);

/*
 *  Disable IPv4 Forwarding on interface in TCP/IP Stack
 */
UI32_T IPAL_Sysctl_DisableIfIpv4Forwarding (UI32_T ifindex);

/*
 *  Get IPv4 Forwarding status on interface in TCP/IP Stack
 */
UI32_T IPAL_Sysctl_GetIfIpv4Forwarding (UI32_T ifindex, int *ip_forward_p);

/*
 *  Enable IPv6 Forwarding in TCP/IP Stack
 */
UI32_T IPAL_Sysctl_EnableIpv6Forwarding ( );

/*
 *  Disable IPv6 Forwarding in TCP/IP Stack
 */
UI32_T IPAL_Sysctl_DisableIpv6Forwarding ( );

/*
 *  Get IPv6 Forwarding in TCP/IP Stack
 */
UI32_T IPAL_Sysctl_GetIpv6Forwarding(int *ip_forward_p);

#if (SYS_CPNT_PROXY_ARP == TRUE)
/*
 *  Enable IPv4 Arp Proxy in TCP/IP Stack
 */
UI32_T IPAL_Sysctl_EnableProxyArp (UI32_T ifindex);

/*
 *  Disable IPv4 Arp Proxy in TCP/IP Stack
 */
UI32_T IPAL_Sysctl_DisableProxyArp (UI32_T ifindex);

#endif /* #if (SYS_CPNT_PROXY_ARP == TRUE) */

/*
 *  Get IPv4 Arp Proxy in TCP/IP Stack
 */
UI32_T IPAL_Sysctl_GetProxyArp (UI32_T ifindex, int *arp_proxy);

/*
 * Set IPv4 Dynamic Arp Aging Timeout
 */
UI32_T IPAL_Sysctl_SetIpv4ArpAgingTime (UI32_T timeout);

/*
 * Get IPv4 Dynamic Arp Aging Timeout
 */
UI32_T IPAL_Sysctl_GetIpv4ArpAgingTime (UI32_T *timeout);

/*get ip default ttl.*/
UI32_T IPAL_Sysctl_GetIpv4DefaultTtl(UI32_T *default_ttl_p);

/*set ip default ttl*/
UI32_T IPAL_Sysctl_SetIpv4DefaultTtl(UI32_T default_ttl);

/*
 * Set IPv6 ICMP Rate Limit
 */
UI32_T IPAL_Sysctl_SetIpv6IcmpRateLimit (UI32_T rate_limit);


/*
 * Set IPv6 default Neighbor retransmit time (Neighbor Solicitation)
 */
UI32_T IPAL_Sysctl_SetIpv6NeighDefaultRetransTime (UI32_T retrans_time);


/*
 * Set IPv6 Neighbor retransmit time (Neighbor Solicitation)
 */
UI32_T IPAL_Sysctl_SetIpv6NeighRetransTime (UI32_T ifindex, UI32_T retrans_time);


/*
 * Set IPv6 Neighbor Default Reachable Time (aging time)
 */
UI32_T IPAL_Sysctl_SetIpv6NeighDefaultReachableTime (UI32_T reachable_time);


/*
 * Set IPv6 Neighbor Interface Reachable Time (aging time)
 */
UI32_T IPAL_Sysctl_SetIpv6NeighReachableTime (UI32_T ifindex, UI32_T reachable_time);


/*
 * Set IPv6 default hop limit
 */
UI32_T IPAL_Sysctl_SetIpv6DefaultHopLimit (UI32_T hot_limit);


/*
 * Set IPv6 interface hop limit
 */
UI32_T IPAL_Sysctl_SetIpv6HopLimit (UI32_T ifindex, UI32_T hop_limit);



/*
 * Set IPv6 DAD transmits time
 */
UI32_T IPAL_Sysctl_SetIpv6DadTransmits (UI32_T ifindex, UI32_T dad_trans_time);

/*
 * Set IPv6 Proxy NDP
 */
UI32_T IPAL_Sysctl_SetIpv6ProxyNdp (UI32_T ifindex, UI32_T proxy_ndp);

/*
 * Get IPv6 Proxy NDP
 */
UI32_T IPAL_Sysctl_GetIpv6ProxyNdp (UI32_T ifindex, UI32_T *proxy_ndp_p);

/*
 * Set IPv6 default autoconfig
 */
UI32_T IPAL_Sysctl_SetIpv6DefaultAutoconf (UI32_T autoconf);

/*
 * Set IPv6 autoconfig
 */
UI32_T IPAL_Sysctl_SetIpv6Autoconf (UI32_T ifindex, UI32_T autoconf);

/*
 * Get IPv6 MTU expries time
 */
UI32_T IPAL_Sysctl_GetIpv6MtuExpires(UI32_T *expires_time_p);

/*
 * Set IPv6 interface MTU
 */
UI32_T IPAL_Sysctl_SetIpv6Mtu(UI32_T ifindex, UI32_T ipv6_mtu);

/*
 * Get IPv6 interface MTU
 */
UI32_T IPAL_Sysctl_GetIpv6Mtu(UI32_T ifindex, UI32_T *ipv6_mtu_p);

/*
 * Set IPv6 interface accept ra prefix info
 */
UI32_T IPAL_Sysctl_SetIpv6AcceptRaPrefixInfo (UI32_T ifindex, UI32_T accept_ra_pinfo);

#endif /* end of __IPAL_SYSCTL_H */

