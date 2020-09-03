/* Module Name: NETCFG_OM_IP_PRIVATE.H
 * Purpose:
 *      Provide private function only for ipcfg.c
 *
 * Notes:
 *      1. 
 *
 *
 * History:
 *      Date        -- 	Modifier,  	Reason
 *      12/10/2008  --  Peter Yu,   Created
 *
 * Copyright(C)      Accton Corporation, 2008.
 */

#ifndef     _NETCFG_OM_IP_PRIVATE_H
#define     _NETCFG_OM_IP_PRIVATE_H


/* INCLUDE FILE DECLARATIONS
 */

/* NAME CONSTANT DECLARATIONS
 */


/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/* FUNCTION NAME : NETCFG_OM_IP_SetIpAddressMode
 * PURPOSE:
 *      Set the addr_mode of specified L3 interface.
 *
 * INPUT:
 *      ifindex     -- the ifindex of L3 interface.
 *      mode
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_ENTRY_NOT_EXIST
 * NOTES:
 *      None.
 */
UI32_T NETCFG_OM_IP_SetIpAddressMode(UI32_T ifindex, UI32_T mode);

/* FUNCTION NAME : NETCFG_OM_IP_SetInetRifRowStatus
 * PURPOSE:
 *      Set row_status of the rif.
 *
 * INPUT:
 *      rif_config_p->ip_addr       -- ip address (key).
 *      rif_config_p->row_status    -- row_status.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_INVALID_ARG
 * NOTES:
 *      1. Only rowstatus value is updated, no any action.
 */
UI32_T NETCFG_OM_IP_SetInetRifRowStatus(NETCFG_OM_IP_InetRifConfig_T *rif_config_p);

/* FUNCTION NAME : NETCFG_OM_IP_DeleteInetRif
 * PURPOSE:
 *      Delete rif by specified ifindex and ip_addr.
 *
 * INPUT:
 *      rif_config_p->ifindex           -- specified ifindex.
 *      rif_config_p->primary_interface -- role.
 *      rif_config_p->ip_addr           -- the ip address.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_INVALID_ARG
 * NOTES:
 *      1. It will check if there is a rif entry with specified ip in the specified interface.
 */
UI32_T NETCFG_OM_IP_DeleteInetRif(NETCFG_OM_IP_InetRifConfig_T *rif_config_p);

#if (SYS_CPNT_PROXY_ARP == TRUE)
/* FUNCTION NAME : NETCFG_OM_IP_SetIpNetToMediaProxyStatus
 * PURPOSE:
 *      Set ARP proxy enable/disable.
 *
 * INPUT:
 *      ifindex -- the interface.
 *      status -- one of {TRUE(stand for enable) |FALSE(stand for disable)}
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK/
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 */
UI32_T NETCFG_OM_IP_SetIpNetToMediaProxyStatus(UI32_T ifindex, BOOL_T status);
#endif /* #if (SYS_CPNT_PROXY_ARP == TRUE) */

/* FUNCTION NAME : NETCFG_OM_IP_SetInterfaceFlags
 * PURPOSE:
 *          Set flags of an ip interface entry.
 *
 * INPUT:
 *      intf  -- the interface entry.
 *      flags
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      None.
 */
UI32_T NETCFG_OM_IP_SetInterfaceFlags(NETCFG_TYPE_L3_Interface_T *intf, UI16_T flags);

/* FUNCTION NAME : NETCFG_OM_IP_UnsetInterfaceFlags
 * PURPOSE:
 *          Unset flags of an ip interface entry.
 *
 * INPUT:
 *      intf  -- the interface entry.
 *      flags
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      None.
 */
UI32_T NETCFG_OM_IP_UnsetInterfaceFlags(NETCFG_TYPE_L3_Interface_T *intf, UI16_T flags);

#if 0 // peter_yu, move back to netcfg_om_ip.h for public use
/* FUNCTION NAME : NETCFG_OM_IP_GetNextInetRifByIfindex
 * PURPOSE:
 *          Get next routing interface entry of a given interface.
 *
 * INPUT:
 *      ifindex
 *      rif->addr
 *
 * OUTPUT:
 *      rif
 *
 * RETURN:
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_ENTRY_NOT_EXIST
 *      NETCFG_TYPE_OK
 *
 * NOTES:
 *      The input ifindex must not be 0 and this function will not go
 *      next interface.
 */
UI32_T NETCFG_OM_IP_GetNextInetRifByIfindex(NETCFG_OM_IP_InetRifConfig_T *rif, UI32_T ifindex);
#endif

#if (SYS_CPNT_IPV6 == TRUE)
/* FUNCTION NAME : NETCFG_OM_IP_SetIPv6EnableStatus
 * PURPOSE:
 *      CMD: "ipv6 enable", to enable/disable IPv6 processing on an interface 
 *      that has not been configured with an explicit IPv6 address.
 *
 * INPUT:
 *      ifindex -- the L3 interface.
 *      status  -- TRUE: enable
 *                 FALSE: disable
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      1. There is no ipv6 enable configuration in linux kernel, thus we only set it in OM.
 */
UI32_T NETCFG_OM_IP_SetIPv6EnableStatus(UI32_T ifindex, BOOL_T status);

/* FUNCTION NAME : NETCFG_OM_IP_SetIPv6AddrAutoconfigEnableStatus
 * PURPOSE:
 *      To enable/disable automatic configuration of IPv6 addresses using stateless 
 *      autoconfiguration on an interface and enable IPv6 processing on the interface. 
 *
 * INPUT:
 *      ifindex     -- interface index
 *      status      -- enable/disable
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      1. If router advertisements (RAs) received on this interface have the 
 *      "other configuration" flag set, then the interface will also attempt 
 *      to acquire other configuration (i.e., non-address) using DHCP for IPv6.
 */
UI32_T NETCFG_OM_IP_SetIPv6AddrAutoconfigEnableStatus(UI32_T ifindex, BOOL_T status);

/* FUNCTION NAME : NETCFG_OM_IP_SetIPv6UnicastRouting
 * PURPOSE:
 *      Set global IPv6 unicast routing.
 *
 * INPUT:
 *      status  -- Enable(TRUE)/Disable(FALSE)
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *
 * NOTES:
 *      None.
 */
UI32_T NETCFG_OM_IP_SetIPv6UnicastRouting(BOOL_T status);

/* FUNCTION NAME : NETCFG_OM_IP_SetIPv6InterfaceMTU
 * PURPOSE:
 *      Set IPv6 interface MTU.
 *
 * INPUT:
 *      ifindex -- the interface be assoicated.
 *      mtu     -- MTU
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      None.
 */
UI32_T NETCFG_OM_IP_SetIPv6InterfaceMTU(UI32_T ifindex, UI32_T mtu);

/* FUNCTION NAME : NETCFG_OM_IP_UnsetIPv6InterfaceMTU
 * PURPOSE:
 *      Unset IPv6 interface MTU.
 *
 * INPUT:
 *      ifindex -- the interface be assoicated.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      None.
 */
UI32_T NETCFG_OM_IP_UnsetIPv6InterfaceMTU(UI32_T ifindex);

UI32_T NETCFG_OM_IP_Debug_ShowInetRifOfInterface(UI32_T ifindex);

#endif /* SYS_CPNT_IPV6 */

#endif /* _NETCFG_OM_IP_PRIVATE_H */
