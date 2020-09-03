#include "radiusclient.h"
#include "netcfg_type.h"
#include "netcfg_pom_ip.h"
#include "iml_pmgr.h"
#include "l_inet.h"
#include "ipal_types.h"
#include "ipal_route.h"
#include <string.h>
#include <stdio.h>
/*
 * Function: rc_get_ipaddr
 *
 * Purpose: return an IP address in host long notation from a host
 *          name or address in dot notation.
 *
 * Returns: 0 on failure
 */


UI32_T rc_get_ipaddr (char *host)
{
    UI32_T ipaddr = 0;

    if (rc_good_ipaddr (host) == 0)
    {
        L_INET_Aton((UI8_T *)host, &ipaddr);
        return ipaddr /*inet_addr(host)*/;
    }
   return 0;
}


/*
 * Function: rc_good_ipaddr
 *
 * Purpose: check for valid IP address in standard dot notation.
 *
 * Returns: 0 on success, -1 when failure
 *
 */

int rc_good_ipaddr (char *addr)
{
    int             dot_count;
    int             digit_count;

    if (addr == NULL)
        return (-1);

    dot_count = 0;
    digit_count = 0;
    while (*addr != '\0' && *addr != ' ')
    {
        if (*addr == '.')
        {
            dot_count++;
            digit_count = 0;
        }
        else if (!isdigit (*addr))
        {
            dot_count = 5;
        }
        else
        {
            digit_count++;
            if (digit_count > 3)
            {
                dot_count = 5;
            }
        }
        addr++;
    }
    if (dot_count != 3)
    {
        return (-1);
    }
    else
    {
        return (0);
    }
}

/*
 * Function: rc_own_ipaddress
 *
 * Purpose: get the IP address of this host in host order
 *
 * Returns: IP address on success, 0 on failure
 *
 */

UI32_T rc_own_ipaddress(void)
{
    UI32_T this_host_ipaddr = 0;
    char hostname[256];

    if (!this_host_ipaddr) {

        if ((this_host_ipaddr = rc_get_ipaddr (hostname)) == 0) {
                   /*     printf("rc_own_ipaddress: couldn't get own IP address");*/
            return 0;
        }
    }

    return this_host_ipaddr;
}


/*************************************************************/
void get_local_ip(L_INET_AddrIp_T *dest_ip_p, L_INET_AddrIp_T *local_ip_p)
{
    L_INET_AddrIp_T  src_ip, nexthop_ip;
    UI32_T           out_ifindex;
    NETCFG_TYPE_InetRifConfig_T rif_config;
#if (SYS_CPNT_CRAFT_PORT == TRUE)
    NETCFG_TYPE_CraftInetAddress_T craft_rif;
#endif

    /* Get rif from best routing interface
     */
    if (IPAL_RESULT_OK == IPAL_ROUTE_RouteLookup(dest_ip_p, &src_ip, &nexthop_ip, &out_ifindex))
    {
        *local_ip_p = src_ip;
        return;
    }

    /* Get first available rif
     */
    memset(&rif_config, 0, sizeof(rif_config));
    rif_config.addr.type = L_INET_ADDR_TYPE_IPV4;

    if (NETCFG_POM_IP_GetNextRifFromInterface(&rif_config) == NETCFG_TYPE_OK)
    {
        *local_ip_p = rif_config.addr;
        return;
    }

#if (SYS_CPNT_IPV6 == TRUE)
    memset(&rif_config, 0, sizeof(rif_config));
    rif_config.addr.type = L_INET_ADDR_TYPE_IPV6;

    if (NETCFG_POM_IP_GetNextIPv6RifFromInterface(&rif_config) == NETCFG_TYPE_OK)
    {
        *local_ip_p = rif_config.addr;
        return;
    }
#endif /* SYS_CPNT_IPV6 */

#if (SYS_CPNT_CRAFT_PORT == TRUE)
    memset(&craft_rif, 0, sizeof(craft_rif));
    craft_rif.ifindex = SYS_ADPT_CRAFT_INTERFACE_IFINDEX;
    craft_rif.addr.type = L_INET_ADDR_TYPE_IPV4;
    if (NETCFG_TYPE_OK == NETCFG_POM_IP_GetCraftInterfaceInetAddress(&craft_rif))
    {
        *local_ip_p = craft_rif.addr;
        return;
    }
#if (SYS_CPNT_IPV6 == TRUE)
    memset(&craft_rif, 0, sizeof(craft_rif));
    craft_rif.ifindex = SYS_ADPT_CRAFT_INTERFACE_IFINDEX;
    craft_rif.addr.type = L_INET_ADDR_TYPE_IPV6;
    if (NETCFG_TYPE_OK == NETCFG_POM_IP_GetCraftInterfaceInetAddress(&craft_rif))
    {
        *local_ip_p = craft_rif.addr;
        return;
    }
#endif
#endif

}
