/* MODULE NAME:  netcfg_pom_ip.h
 * PURPOSE:
 *    This file provides APIs for other process or CSC group to 
 *    NETCFG_OM_IP service.
 *
 * NOTES:
 *
 * REASON:
 * Description:
 * HISTORY
 *    02/19/2008 - Vai Wang, Created
 *
 * Copyright(C)      Accton Corporation, 2008
 */

/* INCLUDE FILE DECLARATIONS
 */
 
#ifndef NETCFG_POM_IP_H
#define NETCFG_POM_IP_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "netcfg_type.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME : NETCFG_POM_IP_InitiateProcessResource
 * PURPOSE:
 *    Initiate resource for CSCA_POM in the calling process.
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE  --  Sucess
 *    FALSE --  Error
 *
 * NOTES:
 *    Before other CSC use NETCFG_POM_IP, it should initiate the resource (get the message queue handler internally)
 
 *
 */
BOOL_T NETCFG_POM_IP_InitiateProcessResource(void);


/* FUNCTION NAME : NETCFG_POM_IP_GetRifConfig
 * PURPOSE:
 *      Get the rif by the ip address.
 *
 * INPUT:
 *      rif_config_p        -- pointer to the rif_config
 *      
 * OUTPUT:
 *      rif_config_p        -- pointer to the rif_config
 *
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_INVALID_ARG
 * NOTES:
 *      The KEY is only ip address.
 */
UI32_T NETCFG_POM_IP_GetRifConfig(NETCFG_TYPE_InetRifConfig_T *rif_config_p);


/* FUNCTION NAME : NETCFG_POM_IP_GetNextRifConfig
 * PURPOSE:
 *      Get the rif by the ip address.
 *
 * INPUT:
 *      rif_config_p        -- pointer to the rif_config
 *      
 * OUTPUT:
 *      rif_config_p        -- pointer to the rif_config
 *
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_INVALID_ARG
 * NOTES:
 *      The KEY is only ip address.
 *      If you need ifindex, Inet prefix as the KEY such as 
 *      SNMP GetNext, CLI running-config, this function is not
 *      the right choice.
 */
UI32_T NETCFG_POM_IP_GetNextRifConfig(NETCFG_TYPE_InetRifConfig_T *rif_config_p);

/* FUNCTION NAME: NETCFG_POM_IP_GetRifFromInterface
 * PURPOSE:
 *      Get the IPv4 rif from the L3 interface by ifindex, and ip_addr.
 * INPUT:
 *      rif_config_p->ifindex           -- ifindex of interface (must).
 *      rif_config_p->addr.addr         -- ip address (optional).
 *
 * OUTPUT:
 *      rif_config_p                    -- pointer to the rif_config.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_ENTRY_NOT_EXIST
 *      NETCFG_TYPE_INVALID_ARG
 * NOTES:
 *      1. The search key priority is ifindex > ip_addr.
 *      2. If interface entry doesn't exist, return fail.
 *      3. If the ip address is specified, try to find it.
 *      4. For IPv4 only.
 */
UI32_T NETCFG_POM_IP_GetRifFromInterface(NETCFG_TYPE_InetRifConfig_T *rif_config_p);


/* FUNCTION NAME: NETCFG_POM_IP_GetPrimaryRifFromInterface
 * PURPOSE:
 *      Get the primary rif from the L3 interface by ifindex.
 * INPUT:
 *      rif_config_p->ifindex           -- ifindex of interface (must).
 *
 * OUTPUT:
 *      rif_config_p                    -- pointer to the rif_config.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_ENTRY_NOT_EXIST
 *      NETCFG_TYPE_INVALID_ARG
 * NOTES:
 *      None
 */
UI32_T NETCFG_POM_IP_GetPrimaryRifFromInterface(NETCFG_TYPE_InetRifConfig_T *rif_config_p);


/* FUNCTION NAME: NETCFG_POM_IP_GetSecondaryRifFromInterface
 * PURPOSE:
 *      Get the primary rif from the L3 interface by ifindex.
 * INPUT:
 *      rif_config_p->ifindex           -- ifindex of interface (must).
 *
 * OUTPUT:
 *      rif_config_p                    -- pointer to the rif_config.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_ENTRY_NOT_EXIST
 *      NETCFG_TYPE_INVALID_ARG
 * NOTES:
 *      None
 */
UI32_T NETCFG_POM_IP_GetNextSecondaryRifFromInterface(NETCFG_TYPE_InetRifConfig_T *rif_config_p);

/* FUNCTION NAME: NETCFG_POM_IP_GetNextRifFromInterface
 * PURPOSE:
 *      Get the next rif from the L3 interface by ifindex.
 * INPUT:
 *      rif_config_p->ifindex   -- the beginning interface trying to search (key).
 *      rif_config_p->ip_addr   -- ip address (key).
 *      rif_config_p->mask      -- mask (key).
 *
 * OUTPUT:
 *      rif_config_p    -- pointer to the rif_config.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_INVALID_ARG
 * NOTES:
 *      1. This function WILL iterate to the next L3 interface.
 *      2. CLI "show ip interface" and SNMP GetNext should call this function.
 */
UI32_T NETCFG_POM_IP_GetNextRifFromInterface(NETCFG_TYPE_InetRifConfig_T *rif_config_p);


/* FUNCTION NAME: NETCFG_POM_IP_GetNextInetRifOfInterface
 * PURPOSE:
 *      Get the next rif from the L3 interface by ifindex.
 * INPUT:
 *      rif_config_p    -- pointer to the rif_config.
 *
 * OUTPUT:
 *      rif_config_p    -- pointer to the rif_config.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_INVALID_ARG
 * NOTES:
 *      1. This function WILL NOT iterate to the next L3 interface.
 *      2. For IPv4/IPv6.
 */
UI32_T NETCFG_POM_IP_GetNextInetRifOfInterface(NETCFG_TYPE_InetRifConfig_T *rif_config_p);


/* FUNCTION NAME : NETCFG_POM_IP_GetRifFromIp
 * PURPOSE:
 *      Get the rif whose subnet covers the target IP address.
 *
 * INPUT:
 *      rif_config_p->ip_addr   -- the target IP address for checking.
 *      
 * OUTPUT:
 *      rif_config_p            -- pointer to rif
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_INVALID_ARG
 * NOTES:
 *      None.
 */
UI32_T NETCFG_POM_IP_GetRifFromIp(NETCFG_TYPE_InetRifConfig_T *rif_config_p);
/* FUNCTION NAME : NETCFG_POM_IP_GetRifFromExactIp
 * PURPOSE:
 *      Find the interface which this ip on.
 *
 * INPUT:
 *      rif_config_p->ip_addr   -- the target IP address for checking.
 *
 * OUTPUT:
 *      rif_config_p            -- pointer to rif
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_ENTRY_NOT_EXIST
 * NOTES:
 *      1. For both IPv4/IPv6.
 */
UI32_T NETCFG_POM_IP_GetRifFromExactIp(NETCFG_TYPE_InetRifConfig_T *rif_config_p);

/* FUNCTION NAME : NETCFG_POM_IP_GetIpAddressMode
 * PURPOSE:
 *      Get the addr_mode of specified L3 interface.
 *
 * INPUT:
 *      ifindex     -- the ifindex of L3 interface.
 *      
 * OUTPUT:
 *      mode_p      -- pointer to the mode.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_INVALID_ARG
 * NOTES:
 *      None.
 */
UI32_T NETCFG_POM_IP_GetIpAddressMode(UI32_T ifindex, UI32_T *mode_p);


/* FUNCTION NAME : NETCFG_POM_IP_GetNextIpAddressMode
 * PURPOSE:
 *      Get the addr_mode for the L3 interface next to the specified one.
 *
 * INPUT:
 *      ifindex     -- the ifindex of L3 interface.
 *      
 * OUTPUT:
 *      ifindex     -- the ifindex of next L3 interface.
 *      mode_p      -- pointer to the mode.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_INVALID_ARG
 * NOTES:
 *      None.
 */
UI32_T NETCFG_POM_IP_GetNextIpAddressMode(UI32_T *ifindex_p, UI32_T *mode_p);


/* FUNCTION NAME : NETCFG_POM_IP_GetIpInterface
 * PURPOSE:
 *      Get the L3 interface.
 *
 * INPUT:
 *      intf_p->ifindex     -- the ifindex of L3 interface.
 *      
 * OUTPUT:
 *      intf_p
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_ENTRY_NOT_EXIST
 *      NETCFG_TYPE_INVALID_ARG
 * NOTES:
 *      None.
 */
UI32_T NETCFG_POM_IP_GetL3Interface(NETCFG_TYPE_L3_Interface_T *intf_p);


/* FUNCTION NAME : NETCFG_POM_IP_GetNextIpInterface
 * PURPOSE:
 *      Get Next L3 interface.
 *
 * INPUT:
 *      intf_p->ifindex     -- the ifindex of L3 interface.
 *      
 * OUTPUT:
 *      intf_p
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_INVALID_ARG
 * NOTES:
 *      None.
 */
UI32_T NETCFG_POM_IP_GetNextL3Interface(NETCFG_TYPE_L3_Interface_T *intf_p);


/* FUNCTION NAME: NETCFG_POM_IP_GetIpAddrEntry
 * PURPOSE:
 *      Get IpAddrEntry.
 * INPUT:
 *      ip_addr_entry_p     -- pointer to the ip_addr_entry.
 *
 * OUTPUT:
 *      ip_addr_entry_p     -- pointer to the ip_addr_entry.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_ENTRY_NOT_EXIST
 * NOTES:
 *      None.
 */
UI32_T NETCFG_POM_IP_GetIpAddrEntry(NETCFG_TYPE_ipAddrEntry_T  *ip_addr_entry_p);



/* FUNCTION NAME: NETCFG_POM_IP_GetNextIpAddrEntry
 * PURPOSE:
 *      Get next IpAddrEntry.
 * INPUT:
 *      ip_addr_entry_p     -- pointer to the ip_addr_entry.
 *
 * OUTPUT:
 *      ip_addr_entry_p     -- pointer to the ip_addr_entry.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_NO_MORE_ENTRY
 * NOTES:
 *      None.
 */
UI32_T NETCFG_POM_IP_GetNextIpAddrEntry(NETCFG_TYPE_ipAddrEntry_T  *ip_addr_entry_p);

#if (SYS_CPNT_PROXY_ARP == TRUE)
/* FUNCTION NAME : NETCFG_POM_IP_GetIpNetToMediaProxyStatus
 * PURPOSE:
 *      Get proxy ARP status.
 *
 * INPUT:
 *      ifindex -- the interface.
 *
 * OUTPUT:
 *      status -- one of {TRUE(stand for enable) |FALSE(stand for disable)}
 *
 * RETURN:
 *     NETCFG_TYPE_OK/
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 */
UI32_T NETCFG_POM_IP_GetIpNetToMediaProxyStatus(UI32_T ifindex, BOOL_T *status);

/* FUNCTION NAME : NETCFG_POM_IP_GetNextIpNetToMediaProxyStatus
 * PURPOSE:
 *      Get next interface's proxy ARP status.
 *
 * INPUT:
 *      ifindex -- the interface.
 *
 * OUTPUT:
 *      status -- one of {TRUE(stand for enable) |FALSE(stand for disable)}
 *
 * RETURN:
 *     NETCFG_TYPE_OK/
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 */
UI32_T NETCFG_POM_IP_GetNextIpNetToMediaProxyStatus(UI32_T *ifindex, BOOL_T *status);
#endif /* #if (SYS_CPNT_PROXY_ARP == TRUE) */

#if (SYS_CPNT_IPV6 == TRUE)
    
/* FUNCTION NAME : NETCFG_POM_IP_GetIPv6EnableStatus
 * PURPOSE:
 *      Get "ipv6 enable" configuration.
 *
 * INPUT:
 *      ifindex     -- the interface.
 *
 * OUTPUT:
 *      status_p    -- status
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      1. There is no ipv6 enable configuration in linux kernel, thus we only get it in OM.
 */
UI32_T NETCFG_POM_IP_GetIPv6EnableStatus(UI32_T ifindex, BOOL_T *status_p);

/* FUNCTION NAME : NETCFG_POM_IP_GetIPv6AddrAutoconfigEnableStatus
 * PURPOSE:
 *      Get IPv6 address autoconfig status of the L3 interface.
 *
 * INPUT:
 *      ifindex     -- interface index
 *
 * OUTPUT:
 *      status_p    -- pointer to status
 *                     TRUE: autoconfig enabled.
 *                     FALSE: autoconfig disabled.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      None.
 */
UI32_T NETCFG_POM_IP_GetIPv6AddrAutoconfigEnableStatus(UI32_T ifindex, BOOL_T *status_p);

/* FUNCTION NAME : NETCFG_POM_IP_GetNextIPv6AddrAutoconfigEnableStatus
 * PURPOSE:
 *      Get next L3 interface's IPv6 address autoconfig status.
 *
 * INPUT:
 *      ifindex_p   -- interface index
 *
 * OUTPUT:
 *      ifindex_p   -- interface index
 *      status_p    -- pointer to status
 *                     TRUE: autoconfig enabled.
 *                     FALSE: autoconfig disabled.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      None.
 */
UI32_T NETCFG_POM_IP_GetNextIPv6AddrAutoconfigEnableStatus(UI32_T *ifindex_p, BOOL_T *status_p);

/* FUNCTION NAME: NETCFG_POM_IP_GetIPv6RifFromInterface
 * PURPOSE:
 *      Get the IPv6 rif from the L3 interface by ifindex, and ip_addr.
 * INPUT:
 *      rif_config_p->ifindex           -- ifindex of interface (must).
 *      rif_config_p->addr.addr           -- ip address (optional).
 *
 * OUTPUT:
 *      rif_config_p                    -- pointer to the rif_config.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_ENTRY_NOT_EXIST
 *      NETCFG_TYPE_INVALID_ARG
 * NOTES:
 *      1. The search key priority is ifindex > ip_addr.
 *      2. If interface entry doesn't exist, return fail.
 *      3. If the ip address is specified, try to find it.
 *      4. For IPv6 only.
 */
UI32_T NETCFG_POM_IP_GetIPv6RifFromInterface(NETCFG_TYPE_InetRifConfig_T *rif_config_p);

/* FUNCTION NAME : NETCFG_POM_IP_GetIPv6RifConfig
 * PURPOSE:
 *      Get the rif by the ip address.
 *
 * INPUT:
 *      rif_config_p        -- pointer to the rif_config
 *      
 * OUTPUT:
 *      rif_config_p        -- pointer to the rif_config
 *
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_INVALID_ARG
 * NOTES:
 *      The KEY is only ip address.
 */
UI32_T NETCFG_POM_IP_GetIPv6RifConfig(NETCFG_TYPE_InetRifConfig_T *rif_config_p);

/* FUNCTION NAME : NETCFG_POM_IP_GetNextIPv6RifConfig
 * PURPOSE:
 *      Get the next rif by the ip address.
 *
 * INPUT:
 *      rif_config_p        -- pointer to the rif_config
 *      
 * OUTPUT:
 *      rif_config_p        -- pointer to the rif_config
 *
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_INVALID_ARG
 * NOTES:
 *      The KEY is only ip address.
 *      If you need ifindex, Inet prefix as the KEY such as 
 *      SNMP GetNext, CLI running-config, this function is not
 *      the right choice.
 */
UI32_T NETCFG_POM_IP_GetNextIPv6RifConfig(NETCFG_TYPE_InetRifConfig_T *rif_config_p);

/* FUNCTION NAME : NETCFG_POM_IP_GetLinkLocalRifFromInterface
 * PURPOSE:
 *      Get RIF with link local address for IPv4 or IPv6 in 
 *      the specified L3 interface.
 *
 * INPUT:
 *      rif_p       -- pointer to rif.
 *
 * OUTPUT:
 *      rif_p       -- pointer to rif.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_NOT_IMPLEMENT
 *      NETCFG_TYPE_ENTRY_NOT_EXIST
 *
 * NOTES:
 *      1. rif_p->addr.type should be L_INET_ADDR_TYPE_IPV4Z or L_INET_ADDR_TYPE_IPV6Z
 */
UI32_T NETCFG_POM_IP_GetLinkLocalRifFromInterface(NETCFG_TYPE_InetRifConfig_T *rif_p);

/* FUNCTION NAME: NETCFG_POM_IP_GetNextIPv6RifFromInterface
 * PURPOSE:
 *      Get the IPv6 rif from the L3 interface by ifindex, and ip_addr.
 * INPUT:
 *      rif_config_p->ifindex           -- ifindex of interface (must).
 *      rif_config_p->ip_addr           -- ip address (optional).
 *
 * OUTPUT:
 *      rif_config_p                    -- pointer to the rif_config.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_ENTRY_NOT_EXIST
 *      NETCFG_TYPE_INVALID_ARG
 * NOTES:
 *      1. The search key priority is ifindex > ip_addr.
 *      2. If interface entry doesn't exist, return fail.
 *      3. If the ip address is specified, try to find it.
 *      4. For IPv6 only.
 */
UI32_T NETCFG_POM_IP_GetNextIPv6RifFromInterface(NETCFG_TYPE_InetRifConfig_T *rif_config_p);

/* FUNCTION NAME : NETCFG_POM_IP_GetIPv6InterfaceMTU
 * PURPOSE:
 *      Get IPv6 interface MTU from OM.
 *
 * INPUT:
 *      ifindex -- the interface be assoicated.
 *
 * OUTPUT:
 *      mtu_p   -- MTU
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      None.
 */
UI32_T NETCFG_POM_IP_GetIPv6InterfaceMTU(UI32_T ifindex, UI32_T *mtu_p);

/* FUNCTION NAME : NETCFG_POM_IP_GetIPv6EnableStatus
 * PURPOSE:
 *      Get "ipv6 enable" configuration.
 *
 * INPUT:
 *      ifindex     -- the interface.
 *
 * OUTPUT:
 *      status_p    -- status
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      1. There is no ipv6 enable configuration in linux kernel, thus we only get it in OM.
 */
UI32_T NETCFG_POM_IP_GetIPv6EnableStatus(UI32_T ifindex, BOOL_T *status_p);

/* FUNCTION NAME : NETCFG_POM_IP_GetRunningIPv6EnableStatus
 * PURPOSE:
 *      Get running config of ipv6 enable status
 *
 * INPUT:
 *      ifindex     -- the interface.
 *
 * OUTPUT:
 *      status_p    -- status
 *
 * RETURN:
 *      SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *      SYS_TYPE_GET_RUNNING_CFG_FAIL
 *      SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *
 * NOTES:
 *      None
 */
SYS_TYPE_Get_Running_Cfg_T NETCFG_POM_IP_GetRunningIPv6EnableStatus(UI32_T ifindex, BOOL_T *status_p);

/* FUNCTION NAME : NETCFG_POM_IP_GetRunningIPv6InterfaceMTU
 * PURPOSE:
 *      Get running config of ipv6 interface MTU
 *
 * INPUT:
 *      ifindex  -- the interface ifindex
 *
 * OUTPUT:
 *      mtu_p    -- interface MTU
 *
 * RETURN:
 *      SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *      SYS_TYPE_GET_RUNNING_CFG_FAIL
 *      SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *
 * NOTES:
 *      None
 */
SYS_TYPE_Get_Running_Cfg_T NETCFG_POM_IP_GetRunningIPv6InterfaceMTU(UI32_T ifindex, UI32_T *mtu_p);

/* FUNCTION NAME : NETCFG_POM_IP_GetRunningIPv6AddrAutoconfigEnableStatus
 * PURPOSE:
 *      Get running config of ipv6 interface autoconfig enable status
 *
 * INPUT:
 *      ifindex  -- the interface ifindex
 *
 * OUTPUT:
 *      status_p -- interface autoconfig enable status
 *
 * RETURN:
 *      SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *      SYS_TYPE_GET_RUNNING_CFG_FAIL
 *      SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *
 * NOTES:
 *      None
 */
SYS_TYPE_Get_Running_Cfg_T NETCFG_POM_IP_GetRunningIPv6AddrAutoconfigEnableStatus(UI32_T ifindex, BOOL_T *status_p);


#endif /* SYS_CPNT_IPV6 */

#if (SYS_CPNT_CRAFT_PORT == TRUE)
/* FUNCTION NAME : NETCFG_POM_IP_GetCraftInterfaceInetAddress
 * PURPOSE:
 *      To get ipv4/v6 address on craft interface
 * INPUT:
 *      craft_addr_p        -- pointer to address
 *
 * OUTPUT:
 *      None.
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 * NOTES:
 *      None
 */
UI32_T NETCFG_POM_IP_GetCraftInterfaceInetAddress(NETCFG_TYPE_CraftInetAddress_T *craft_addr_p);
UI32_T NETCFG_POM_IP_GetIPv6EnableStatus_Craft(UI32_T ifindex, BOOL_T *status_p);

#endif /* SYS_CPNT_CRAFT_PORT */

#if (SYS_CPNT_DHCP_INFORM == TRUE)
/* FUNCTION NAME : NETCFG_POM_IP_GetDhcpInform
 * PURPOSE:
 *      Get the IPv4 addr_mode of specified L3 interface.
 *
 * INPUT:
 *      ifindex     -- the ifindex of L3 interface.
 *
 * OUTPUT:
 *      do_enable_p -- status of dhcp inform
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_ENTRY_NOT_EXIST
 * NOTES:
 *      1. This is only for IPv4.
 */
UI32_T NETCFG_POM_IP_GetDhcpInform(UI32_T ifindex, BOOL_T *do_enable_p);

/* FUNCTION NAME : NETCFG_POM_IP_GetRunningDhcpInform
 * PURPOSE:
 *      Get running config of DHCP inform of L3 interface.
 *
 * INPUT:
 *      ifindex     -- the ifindex of L3 interface.
 *
 * OUTPUT:
 *      do_enable_p -- status of dhcp inform
 *
 * RETURN:
 *   SYS_TYPE_GET_RUNNING_CFG_SUCCESS  
 *   SYS_TYPE_GET_RUNNING_CFG_FAIL     
 *   SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE  
 * NOTES:
 *      1. This is only for IPv4.
 */
SYS_TYPE_Get_Running_Cfg_T NETCFG_POM_IP_GetRunningDhcpInform(UI32_T ifindex, BOOL_T *do_enable_p);
#endif /* SYS_CPNT_DHCP_INFORM */

#if (SYS_CPNT_VIRTUAL_IP == TRUE)
UI32_T NETCFG_POM_IP_GetNextVirtualRifByIfindex(NETCFG_TYPE_InetRifConfig_T *rif_config_p);
#endif
#endif /* NETCFG_POM_IP_H */

