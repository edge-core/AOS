/*
 *   File Name: ipal_vrrp.h
 *   Purpose:   TCP/IP shim layer(ipal) VRRP management implementation
 *   Note:
 *   Create:    Vai Wang     2009.04.06
 *
 *   Histrory:
 *              Modify		   Date      Reason
 *
 */

#ifndef __IPAL_VRRP_H
#define __IPAL_VRRP_H

/*
 * EXPORTED SUBPROGRAM DECLARATIONS
 */
UI32_T IPAL_VRRP_AddVrrpVirturalIp(
	UI32_T ifindex, 
	UI8_T vrrp_id,
	L_PREFIX_IPv4_T  *vip_prefix
);


UI32_T IPAL_VRRP_DeleteVrrpVirturalIp(
	UI32_T ifindex, 
	UI8_T vrrp_id,
	L_PREFIX_IPv4_T  *vip_prefix
);

#endif /* __IPAL_VRRP_H */

