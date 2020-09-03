/*
 *   File Name: ipal_icmp.c
 *   Purpose:   TCP/IP shim layer(ipal) ICMP management implementation API
 *   Note:
 *   Create:    kh_shi     2008.11.27
 *
 *   Histrory:
 *              Modify		   Date      Reason
 *
 *
 *   Copyright(C)  Accton Corporation 2007~2009
 */

/*
 * INCLUDE FILE DECLARATIONS
 */
#include "string.h"
#include "sys_type.h"
#include "sysfun.h"
#include "sys_adpt.h"

#include "l_prefix.h"
#include "l_inet.h"

#include "ipal_types.h"
#include "ipal_debug.h"
#include "ipal_sysctl.h"
#include "ipal_if.h"

/*
 * NAMING CONST DECLARATIONS
 */
#define IF_RA_OTHERCONF	0x80
#define IF_RA_MANAGED	0x40

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

#if (SYS_CPNT_IPV6 == TRUE)
/* FUNCTION NAME : IPAL_ICMP_SetIpv6IcmpRateLimit
 * PURPOSE:
 *      Set the rate limit of ICMP error message sending
 *      (/proc/sys/net/ipv6/icmp/ratelimit)
 * INPUT:
 *      rate_limit   -- rate limit in miliseconds
 * OUTPUT:
 *      None
 * RETURN:
 *      IPAL_RESULT_OK   --  success
 *      IPAL_RESULT_FAIL --  fail to set rate limit of icmp message
 * NOTES:
 */
UI32_T IPAL_ICMP_SetIpv6IcmpRateLimit(UI32_T rate_limit)
{
    return IPAL_Sysctl_SetIpv6IcmpRateLimit(rate_limit);
}


/* FUNCTION NAME : IPAL_ICMP_SetIpv6NeighDefaultRetransTime
 * PURPOSE:
 *      Set the default retransmit time interval of Neighbor Solicitation packet
 *      (/proc/sys/net/ipv6/neigh/default/retrans_time_ms)
 * INPUT:
 *      retrans_time   -- time interval in miliseconds
 * OUTPUT:
 *      None
 * RETURN:
 *      IPAL_RESULT_OK   --  success
 *      IPAL_RESULT_FAIL --  fail to set the time interval
 * NOTES:
 */
UI32_T IPAL_ICMP_SetIpv6NeighDefaultRetransTime(UI32_T retrans_time)
{
    return IPAL_Sysctl_SetIpv6NeighDefaultRetransTime(retrans_time);
}

/* FUNCTION NAME : IPAL_ICMP_SetIpv6NeighRetransTime
 * PURPOSE:
 *      Set the retransmit time interval of Neighbor Solicitation packet
 *      (/proc/sys/net/ipv6/neigh/eth0/retrans_time_ms)
 * INPUT:
 *      ifindex        -- L3 interface index
 *      retrans_time   -- time interval in miliseconds
 * OUTPUT:
 *      None
 * RETURN:
 *      IPAL_RESULT_OK   --  success
 *      IPAL_RESULT_FAIL --  fail to set the time interval
 * NOTES:
 */
UI32_T IPAL_ICMP_SetIpv6NeighRetransTime(UI32_T ifindex, UI32_T retrans_time)
{
    return IPAL_Sysctl_SetIpv6NeighRetransTime(ifindex, retrans_time);
}

/* FUNCTION NAME : IPAL_ICMP_SetIpv6NeighDefaultReachableTime
 * PURPOSE:
 *      Set the retransmit time interval of Neighbor Solicitation packet
 *      (/proc/sys/net/ipv6/neigh/default/base_reachable_time_ms)
 * INPUT:
 *      reachable_time   -- time interval in miliseconds
 * OUTPUT:
 *      None
 * RETURN:
 *      IPAL_RESULT_OK   --  success
 *      IPAL_RESULT_FAIL --  fail to set the time interval
 * NOTES:
 */
UI32_T IPAL_ICMP_SetIpv6NeighDefaultReachableTime(UI32_T reachable_time)
{
    return IPAL_Sysctl_SetIpv6NeighDefaultReachableTime(reachable_time);
}


/* FUNCTION NAME : IPAL_ICMP_SetIpv6NeighReachableTime
 * PURPOSE:
 *      Set the retransmit time interval of Neighbor Solicitation packet
 *      (/proc/sys/net/ipv6/neigh/eth0/base_reachable_time_ms)
 * INPUT:
 *      ifindex          -- L3 interface index
 *      reachable_time   -- time interval in miliseconds
 * OUTPUT:
 *      None
 * RETURN:
 *      IPAL_RESULT_OK   --  success
 *      IPAL_RESULT_FAIL --  fail to set the time interval
 * NOTES:
 */
UI32_T IPAL_ICMP_SetIpv6NeighReachableTime(UI32_T ifindex, UI32_T reachable_time)
{
    return IPAL_Sysctl_SetIpv6NeighReachableTime(ifindex, reachable_time);
}


/* FUNCTION NAME : IPAL_ICMP_SetDadTransmits
 * PURPOSE:
 *      Set the transmit times of DAD
 *      (/proc/sys/net/ipv6/conf/eth1/dad_transmits)
 * INPUT:
 *      ifindex        -- L3 interface index
 *      dad_trans_time -- number of DAD transmit time
 * OUTPUT:
 *      None
 * RETURN:
 *      IPAL_RESULT_OK   --  success
 *      IPAL_RESULT_FAIL --  fail to set the dad transmit time
 * NOTES:
 */
UI32_T IPAL_ICMP_SetIpv6DadTransmits(UI32_T ifindex, UI32_T dad_trans_time)
{
    return IPAL_Sysctl_SetIpv6DadTransmits(ifindex, dad_trans_time);
}


/* FUNCTION NAME : IPAL_ICMP_GetRaMoBits
 * PURPOSE:
 *      Get the Managed and Otherconf Bit of the last received Router
 *      Advertisement packet
 * INPUT:
 *      ifindex   -- L3 interface index
 * OUTPUT:
 *      managed_bits_p     -- managed bit
 *      otherconf_bits_p   -- otherconf bit
 *
 * RETURN:
 *      IPAL_RESULT_OK   --  success
 *      IPAL_RESULT_FAIL --  fail to get the M/O bits
 * NOTES:
 */
UI32_T IPAL_ICMP_GetRaMoBits(UI32_T ifindex, BOOL_T *managed_bits_p, BOOL_T *otherconf_bits_p)
{
    IPAL_IfInfo_T  if_info;
    UI32_T         ret;

    memset(&if_info, 0, sizeof(if_info));
    if ((ret = IPAL_IF_GetIpv6IfInfo(ifindex, &if_info)) != IPAL_RESULT_OK)
    {
        return ret;
    }

    *managed_bits_p   = !!(if_info.inet6_flags & IF_RA_MANAGED);
    *otherconf_bits_p = !!(if_info.inet6_flags & IF_RA_OTHERCONF);

    return IPAL_RESULT_OK;
}
#endif /* #if (SYS_CPNT_IPV6 == TRUE) */

