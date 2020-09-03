/*
 *   File Name: ipal_vrrp.c
 *   Purpose:   TCP/IP shim layer(ipal) VRRP management implementation
 *   Note:
 *   Create:    Vai Wang     2009.04.06
 *
 *   Histrory:
 *              Modify		   Date      Reason
 *
 */

/*
 * INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <linux/neighbour.h>
#include <linux/if_ether.h>
#include <linux/if_arp.h>
#include <linux/if_packet.h>
#include <errno.h>
#include <unistd.h>

#include "sys_type.h"
#include "sysfun.h"
#include "sys_adpt.h"

#include "l_prefix.h"
#include "l_inet.h"

#include "ipal_types.h"
#include "ipal_debug.h"
#include "ipal_neigh.h"
#include "ipal_vrrp.h"
#include "ipal_if.h"
/*
 * NAMING CONST DECLARATIONS
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
 * EXPORTED SUBPROGRAM
 */
UI32_T IPAL_VRRP_AddVrrpVirturalIp(
    UI32_T ifindex, 
    UI8_T vrrp_id,
    L_PREFIX_IPv4_T  *vip_prefix)
{       
    L_INET_AddrIp_T vip;

    if (NULL == vip_prefix)
        return IPAL_RESULT_FAIL;

    memset(&vip, 0, sizeof(vip));
    vip.type = L_INET_ADDR_TYPE_IPV4;
    vip.addrlen = SYS_ADPT_IPV4_ADDR_LEN;
    vip.preflen = vip_prefix->prefixlen;
    memcpy(vip.addr, (UI8_T *)&(vip_prefix->prefix.s_addr),
                SYS_ADPT_IPV4_ADDR_LEN);

    /* add alias ip address 
     */
    if(IPAL_RESULT_OK!=IPAL_IF_AddIpAddressAlias(ifindex, &vip, vrrp_id))
        return IPAL_RESULT_FAIL;

    return IPAL_RESULT_OK;
}


UI32_T IPAL_VRRP_DeleteVrrpVirturalIp(
    UI32_T ifindex, 
    UI8_T vrrp_id,
    L_PREFIX_IPv4_T  *vip_prefix)
{
    L_INET_AddrIp_T vip;

    if (NULL == vip_prefix)
        return IPAL_RESULT_FAIL;

    memset(&vip, 0, sizeof(vip));
    vip.type = L_INET_ADDR_TYPE_IPV4;
    vip.addrlen = SYS_ADPT_IPV4_ADDR_LEN;
    vip.preflen = vip_prefix->prefixlen;
    memcpy(vip.addr, (UI8_T *)&(vip_prefix->prefix.s_addr),
                SYS_ADPT_IPV4_ADDR_LEN);

    /* when delete, just remove all alias ip address
    */
    if(IPAL_RESULT_OK != IPAL_IF_DeleteIpAddressAlias(ifindex, &vip, vrrp_id))
        return IPAL_RESULT_FAIL;

    return IPAL_RESULT_OK;
}

