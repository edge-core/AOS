/*
 *   File Name: ipal_route.c
 *   Purpose:   TCP/IP shim layer(ipal) IP management implementation
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
#include "ipal_types.h"

/*
 * NAMING CONST DECLARATIONS
 */


/*
 * MACRO FUNCTION DECLARATIONS
 */
/* reserved IPv4 LLA used in BGP unnumbered (i.e. 169.254.0.1)
 */
#define IPAL_ROUTE_IS_BGP_UNNUMBERED_IPV4_LLA(ip_ar) \
            (  (ip_ar[0] == 169) \
             &&(ip_ar[1] == 254) \
             &&(ip_ar[2] == 0)   \
             &&(ip_ar[3] == 1) )

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
 *  Name    IPAL_Enable_Ipv4Forwarding
 *  Purpose Enable IPv4 Forwarding in TCP/IP stack
 *  Input   None
 *  Output  None
 *  Return  0: success
 *          the error no.
 *  Note    None
 */
UI32_T IPAL_ROUTE_EnableIpv4Forwarding(void);

/*
 *  Name    IPAL_Disable_Ipv4Forwarding
 *  Purpose Disable IPv4 Forwarding in TCP/IP stack
 *  Input   None
 *  Output  None
 *  Return  0: success
 *          the error no.
 *  Note    None
 */
UI32_T IPAL_ROUTE_DisableIpv4Forwarding(void);

/*
 *  Name    IPAL_Get_Ipv4Forwarding
 *  Purpose Disable IPv4 Forwarding in TCP/IP stack
 *  Input   int *ip_forward
 *  Output  IPv4 forwarding or not
 *  Return  0: success
 *          the error no.
 *  Note    None
 */
UI32_T IPAL_ROUTE_GetIpv4Forwarding (I32_T *ip_forward);

/*
 *  Name    IPAL_ROUTE_EnableIfIpv4Forwarding
 *  Purpose Enable IPv4 Forwarding on an interface in TCP/IP stack
 *  Input   ifindex  -- interface ifindex
 *  Output  None
 *  Return  0: success
 *          the error no.
 *  Note    None
 */
UI32_T IPAL_ROUTE_EnableIfIpv4Forwarding(UI32_T ifindex);

/*
 *  Name    IPAL_ROUTE_DisableIfIpv4Forwarding
 *  Purpose Disable IPv4 Forwarding on an interface in TCP/IP stack
 *  Input   ifindex  -- interface ifindex
 *  Output  None
 *  Return  0: success
 *          the error no.
 *  Note    None
 */
UI32_T IPAL_ROUTE_DisableIfIpv4Forwarding(UI32_T ifindex);

/*
 *  Name    IPAL_ROUTE_GetIfIpv4Forwarding
 *  Purpose Get IPv4 Forwarding status on an interface in TCP/IP stack
 *  Input   ifindex      -- interface ifindex
 *  Output  *ip_forward  -- IPv4 forwarding or not
 *  Return  0: success
 *          the error no.
 *  Note    None
 */
UI32_T IPAL_ROUTE_GetIfIpv4Forwarding (UI32_T ifindex, I32_T *ip_forward);

UI32_T IPAL_ROUTE_ShowAllIpv4Route();

/*
 *  Name    IPAL_AddIpv4Route
 *  Purpose Add IPv4 route entry in TCP/IP stack
 *  Input   p           -- destination address
 *          nhs         -- nexthop
 *          ifindex     -- ifindex
 *  Output  None
 *  Return  0: success
 *          the error no.
 *  Note    None
 */
UI32_T IPAL_ROUTE_AddIpv4Route(const L_INET_AddrIp_T *p, const L_INET_AddrIp_T *nexthop, UI32_T ifindex);

/*
 *  Name    IPAL_DeleteIpv4Route
 *  Purpose Delete IPv4 route entry in TCP/IP stack
 *  Input   p           -- destination address
 *          nhs         -- nexthop
 *          ifindex     -- ifindex
 *  Output  None
 *  Return  0: success
 *          the error no.
 *  Note    None
 */
UI32_T IPAL_ROUTE_DeleteIpv4Route (const L_INET_AddrIp_T *p, const L_INET_AddrIp_T *nexthop, UI32_T ifindex);

/*
 *  Name    IPAL_ROUTE_DeleteAllIpv4Route
 *  Purpose Delete All IPv4 route entries in TCP/IP stack
 *  Input   None
 *  Output  None
 *  Return  0: success
 *          the error no.
 *  Note    None
 */
UI32_T IPAL_ROUTE_DeleteAllIpv4Route();

/*
 *  Name    IPAL_ROUTE_RouteLookup
 *  Purpose Find route for a destination address
 *  Input   dest_ip_p -- destination address
 *  Output  src_ip_p     -- source address
 *          nexthop_ip_p -- nexthop address
 *          nexthop_if_p -- nexthop egress interface
 *  Return  0: success
 *          the error no.
 *  Note    None
 */
UI32_T IPAL_ROUTE_RouteLookup(L_INET_AddrIp_T *dest_ip_p, L_INET_AddrIp_T *src_ip_p,
                                L_INET_AddrIp_T *nexthop_ip_p, UI32_T *nexthop_if_p);

#if (SYS_CPNT_IPV6 == TRUE)
/*
 *  Name    IPAL_Enable_Ipv6Forwarding
 *  Purpose Enable IPv6 Forwarding in TCP/IP stack
 *  Input   None
 *  Output  None
 *  Return  0: success
 *          the error no.
 *  Note    None
 */
UI32_T IPAL_ROUTE_EnableIpv6Forwarding(void);

/*
 *  Name    IPAL_Disable_Ipv6Forwarding
 *  Purpose Disable IPv6 Forwarding in TCP/IP stack
 *  Input   None
 *  Output  None
 *  Return  0: success
 *          the error no.
 *  Note    None
 */
UI32_T IPAL_ROUTE_DisableIpv6Forwarding(void);

/*
 *  Name    IPAL_Get_Ipv6Forwarding
 *  Purpose Disable IPv6 Forwarding in TCP/IP stack
 *  Input   int *ip_forward
 *  Output  IPv4 forwarding or not
 *  Return  0: success
 *          the error no.
 *  Note    None
 */
UI32_T IPAL_ROUTE_GetIpv6Forwarding (I32_T *ip_forward);

UI32_T IPAL_ROUTE_GetNextNPmtuEntry(IPAL_PmtuEntry_T *pmtu_entry_p, UI32_T *entry_num_p);

UI32_T IPAL_ROUTE_ShowAllIpv6Route();

/*
 *  Name    IPAL_ROUTE_AddIpv6Route
 *  Purpose Add IPv6 route entry in TCP/IP stack
 *  Input   p           -- destination address
 *          nhs         -- nexthop
 *          ifindex     -- ifindex
 *  Output  None
 *  Return  0: success
 *          the error no.
 *  Note    None
 */
UI32_T IPAL_ROUTE_AddIpv6Route(const L_INET_AddrIp_T *p, const L_INET_AddrIp_T *nexthop, UI32_T ifindex);

/*
 *  Name    IPAL_ROUTE_DeleteIpv6Route
 *  Purpose Delete IPv6 route entry in TCP/IP stack
 *  Input   p           -- destination address
 *          nhs         -- nexthop
 *          ifindex     -- ifindex
 *  Output  None
 *  Return  0: success
 *          the error no.
 *  Note    None
 */
UI32_T IPAL_ROUTE_DeleteIpv6Route (const L_INET_AddrIp_T *p, const L_INET_AddrIp_T *nexthop, UI32_T ifindex);

/*
 *  Name    IPAL_ROUTE_DeleteAllIpv6Route
 *  Purpose Delete All IPv6 route entries in TCP/IP stack
 *  Input   None
 *  Output  None
 *  Return  0: success
 *          the error no.
 *  Note    None
 */
UI32_T IPAL_ROUTE_DeleteAllIpv6Route();

#endif /* #if (SYS_CPNT_IPV6 == TRUE) */


