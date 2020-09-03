/* Module Name: ip_mgr.h
 * Purpose:
 *      IP provides Layer-3 interface management function, including create routing interface,
 *      destroy routing interface, change ip address of the interface.
 *      IP_MGR is the porting interface which porting all access points of ZebOS, a Routing
 *      protocol package.
 *      The interface management will be done via NSM(the core of ZebOS) and it will then
 *      pass the configuration to Linux Kernel (TCP/IP stack).
 *      In whole system, the management hierarch is NETCFG --> IPCFG --> IP_MGR --> ZebOS -> Linux Kernel.
 *
 * Notes:
 *      1. Recordset definition :
 *         Circuit : { rif_no, phyAddress, if_MTU, if_speed }
 *                   Key : rif_no.
 *         Subnet  : { router_ip, netmask, rif_no }
 *                   Key : rif_no.
 *         Routing : { dst_ip, dst_mask, next_hop, rif_no, metric }
 *                   Key : (dst_ip, dst_mask)
 *         ARP     : { ip_addr, rif_no, phyAddress }
 *                   Key : (ip_addr, rif_no)
 *
 *
 * History:
 *       Date       --  Modifier,   Reason
 *  0.1 2002.01.27  --  William,    Created
 *  0.2 2002.02.28  --  William,    Modify spec. to IP_MGR only, ARP and ROUTE function
 *                                  should move to ARP, ROUTE.
 *  0.3 2007.07.16  --  Max Chen,   Porting to Linux Platform
 *  0.4 2008.01.23  --  Vai Wang,   Porting to Linux Platform
 *
 * Copyright(C)      Accton Corporation, 2007.
 */

#ifndef     _IP_MGR_H
#define     _IP_MGR_H


/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_bld.h"
#include "netcfg_type.h"
#include "l_threadgrp.h"

/* NAME CONSTANT DECLARATIONS
 */

/*  Function returned value */

/* MACRO FUNCTION DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */
typedef struct IP_MGR_Interface_S
{
    /* Lookup Key */
    UI32_T ifindex;

    struct
    {
        struct
        {
            /* The interface flags, defined in TCP/IP stack,
             * IFF_UP, IFF_RUNNING
             */
            UI16_T if_flags;

            UI8_T logical_mac[SYS_ADPT_MAC_ADDR_LEN];

            /* Vlan MTU
             */
            UI32_T mtu;

            /* Vlan Bandwidth
             */
            UI32_T bandwidth;
        } physical_intf;



        struct
        {
            UI8_T tunnel_mode;
            UI32_T src_vid_ifindex;
            UI8_T ttl;
            UI8_T tos;
            UI8_T dip_addr[SYS_ADPT_IPV4_ADDR_LEN];
        } tunnel_intf;
    } u;
}IP_MGR_Interface_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/* FUNCTION NAME : IP_MGR_InitiateProcessResources
 * PURPOSE:
 *      Initialize IP_MGR used system resource.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      None.
 */
void IP_MGR_InitiateProcessResources(void);

/* FUNCTION NAME : IP_MGR_Create_InterCSC_Relation
 * PURPOSE:
 *      This function initializes all function pointer registration operations.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *
 * NOTES:
 *
 */
void IP_MGR_Create_InterCSC_Relation(void);

/* FUNCTION NAME : IP_MGR_Enter_Master_Mode
 * PURPOSE:
 *      Enter Master Mode; could handle TCP/IP protocol stack management operation.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *
 */
void IP_MGR_Enter_Master_Mode(void);

/* FUNCTION NAME : IP_MGR_Enter_Slave_Mode
 * PURPOSE:
 *      Enter Slave Mode; ignore any request.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *
 * NOTES:
 *
 */
void IP_MGR_Enter_Slave_Mode (void);


/* FUNCTION NAME : IP_MGR_Enter_Transition_Mode
 * PURPOSE:
 *      Enter Transition Mode; release all resource of IP.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *
 * NOTES:
 *      1. Reset all modules, release all resources.
 */
void IP_MGR_Enter_Transition_Mode (void);

/*------------------------------------------------
 *  IP_MGR Configure API.
 *------------------------------------------------
 */
/* FUNCTION NAME : IP_MGR_CreateInterface
 * PURPOSE:
 *      Create an interface in PMs and NSM->TCP/IP stack.
 *
 * INPUT:
 *      intf -- the interface entry.
 *      drv_l3_intf_index_p -- pointer to the L3If index (hwinfo) in chip
 *                             if L3If index != SWDRVL3_HW_INFO_INVALID -> use it to create L3If
 *                             otherwise -> create a new one and return it's L3If index
 *
 * OUTPUT:
 *      drv_l3_intf_index_p -- pointer to the drv_l3_intf_index
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_MAC_COLLISION
 *      
 * NOTES:
 *      None.
 *
 */
UI32_T IP_MGR_CreateInterface(IP_MGR_Interface_T *intf, UI32_T *drv_l3_intf_index_p);

/* FUNCTION NAME : IP_MGR_CreateLoopbackInterface
 * PURPOSE:
 *      Create a loopback interface in PMs and NSM->TCP/IP stack.
 *
 * INPUT:
 *      intf -- the interface entry.
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
 *
 */
UI32_T IP_MGR_CreateLoopbackInterface(IP_MGR_Interface_T *intf);


/* FUNCTION NAME : IP_MGR_DeleteInterface
 * PURPOSE:
 *      Delete an interface in PMs and NSM->TCP/IP stack.
 *
 * INPUT:
 *      ifindex -- the index of interface.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE/FALSE
 *
 * NOTES:
 *      None.
 *
 */
BOOL_T IP_MGR_DeleteInterface(UI32_T ifindex);

/* FUNCTION NAME : IP_MGR_DeleteLoopbackInterface
 * PURPOSE:
 *      Delete a loopback interface in PMs and NSM->TCP/IP stack.
 *
 * INPUT:
 *      ifindex -- the index of interface.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE/FALSE
 *
 * NOTES:
 *      None.
 *
 */
BOOL_T IP_MGR_DeleteLoopbackInterface(UI32_T ifindex);


/* FUNCTION NAME : IP_MGR_SetIfFlags
 * PURPOSE:
 *      Set flags of an interface in PMs and NSM->TCP/IP stack.
 *
 * INPUT:
 *      ifindex -- the index of interface.
 *      flags
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE/FALSE
 *
 * NOTES:
 *      None.
 *
 */
BOOL_T IP_MGR_SetIfFlags(UI32_T ifindex, UI16_T flags);

/* FUNCTION NAME : IP_MGR_UnsetIfFlags
 * PURPOSE:
 *      Unset flags of an interface in PMs and NSM->TCP/IP stack.
 *
 * INPUT:
 *      ifindex -- the index of interface.
 *      flags
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE/FALSE
 *
 * NOTES:
 *      None.
 *
 */
BOOL_T IP_MGR_UnsetIfFlags(UI32_T ifindex, UI16_T flags);

/* FUNCTION NAME : IP_MGR_AddInetRif
 * PURPOSE:
 *      Add Inet Rif.
 *
 * INPUT:
 *      rif->ifindex            --  the ifindex of interface (currently, equal to vid_ifindex)
 *      rif->addr.addr          --  subnet ip address, the ip associated with Router.
 *      rif->addr.mask          --  subnet mask.
 *      rif->ipv4_role          --  set as the primary or secondary interface
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_INVALID_ARG -- input argument is invalid
 *      NETCFG_TYPE_CAN_NOT_ADD -- Fail to Add the specified IPV4 Rif
 *      NETCFG_TYPE_OK          -- The Rif is added successfully
 *
 * NOTES:
 *      1. In Linux and ZebOS, each interface can bound with multiple IP address
 *         The interface is identified via name(retrieved from ifindex)
 *      2. For both IPv4 or IPv6.
 *
 */
UI32_T IP_MGR_AddInetRif(NETCFG_TYPE_InetRifConfig_T *rif);


/* FUNCTION NAME : IP_MGR_DeleteIPv4Rif
 * PURPOSE:
 *      Delete Inet Rif.
 *
 * INPUT:
 *      rif->ifindex            --  the ifindex of interface (currently, equal to vid_ifindex)
 *      rif->addr.addr          --  subnet ip address, the ip associated with Router.
 *      rif->addr.mask          --  subnet mask.
 *      rif->ipv4_role          --  figure out if the interface is primary or secondary
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK           -- Successfully delete the ip address .
 *      NETCFG_TYPE_ENTRY_NOT_EXIST
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_CAN_NOT_DELETE -- Can not delete
 *
 * NOTES:
 *      1. In Linux and ZebOS, each interface can bound with multiple IP address
 *         The interface is identified via name(retrieved from ifindex)
 *      2. For both IPv4 or IPv6.
 */
UI32_T IP_MGR_DeleteInetRif(NETCFG_TYPE_InetRifConfig_T *rif);


/* FUNCTION NAME : IP_MGR_IsEmbeddedUdpPort
 * PURPOSE:
 *      Check the udp-port is used in protocol engine or not.
 *
 * INPUT:
 *      udp_port -- the udp-port to be checked.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE    -- this port is used in protocl engine, eg. 520.
 *      FALSE   -- this port is available for sokcet used,
 *                 but possibly already used in socket engine
 *
 * NOTES:
 *      1. This function is used for udp-helper; a passive function.
 *      2. Because get only, do not enter critical section and
 *         do not check master mode.
 */
BOOL_T IP_MGR_IsEmbeddedUdpPort (UI32_T udp_port);


/* FUNCTION NAME : IP_MGR_IsEmbeddedTcpPort
 * PURPOSE:
 *      Check the tcp-port is used in protocol engine or not.
 *
 * INPUT:
 *      tcp_port -- the tcp-port to be checked.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE    -- this port is used in protocl engine, eg. 520.
 *      FALSE   -- this port is available for sokcet used,
 *                 but possibly already used in socket engine
 *
 * NOTES:
 *      1. This function is used for UI; a passive function.
 *      2. Ref. linux net-tools-1.60 netstat.c
 */
BOOL_T IP_MGR_IsEmbeddedTcpPort(UI32_T tcp_port);

#if (SYS_CPNT_PROXY_ARP == TRUE)
/* FUNCTION NAME : IP_MGR_SetIpNetToMediaProxyStatus
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
UI32_T  IP_MGR_SetIpNetToMediaProxyStatus(UI32_T ifindex, BOOL_T status);
#endif

#if (SYS_CPNT_NSM == TRUE)
void IP_MGR_DumpZebOSRif(L_THREADGRP_Handle_T tg_handle, UI32_T member_id);
void IP_MGR_DumpZebOSInterface(L_THREADGRP_Handle_T tg_handle, UI32_T member_id);
#endif

#if (SYS_CPNT_IPV6 == TRUE)
/* FUNCTION NAME : IP_MGR_IPv6AddrAutoconfigEnable
 * PURPOSE:
 *      To enable automatic configuration of IPv6 addresses using stateless
 *      autoconfiguration on an interface.
 *
 * INPUT:
 *      ifindex     -- interface index
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      1. The autoconfiguration process specified here applies only to hosts, not routers.
 *      2. The configuration is passed into TCP/IP protocol stack, and after the process,
 *         the stack will call NETCFG_MGR_IP_SetInetRif to add into OM.
 */
UI32_T IP_MGR_IPv6AddrAutoconfigEnable(UI32_T ifindex);

/* FUNCTION NAME : IP_MGR_IPv6AddrAutoconfigDisable
 * PURPOSE:
 *      To disable automatic configuration of IPv6 addresses using stateless
 *      autoconfiguration on an interface.
 *
 * INPUT:
 *      ifindex     -- interface index
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      1. The autoconfiguration process specified here applies only to hosts, not routers.
 *      2. The configuration is passed into TCP/IP protocol stack, and after the process,
 *         the stack will call NETCFG_MGR_IP_SetInetRif to add into OM.
 */
UI32_T IP_MGR_IPv6AddrAutoconfigDisable(UI32_T ifindex);

/* FUNCTION NAME : IP_MGR_GetNextIfJoinIpv6McastAddr
 * PURPOSE:
 *      Get next joined multicast group for the interface.
 *
 * INPUT:
 *      ifindex     -- interface
 *
 * OUTPUT:
 *      mcaddr_p    -- pointer to multicast address
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      None.
 */
UI32_T IP_MGR_GetNextIfJoinIpv6McastAddr(UI32_T ifindex, L_INET_AddrIp_T *mcaddr_p);

/* FUNCTION NAME : IP_MGR_GetNextPMTUEntry
 * PURPOSE:
 *      Get next path mtu entry.
 *
 * INPUT:
 *      entry_p     -- pointer to entry
 *
 * OUTPUT:
 *      entry_p     -- pointer to entry
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      None.
 */
UI32_T IP_MGR_GetNextPMTUEntry(NETCFG_TYPE_PMTU_Entry_T *entry_p);

/* FUNCTION NAME : IP_MGR_GetIfIpv6AddrInfo
 * PURPOSE:
 *      Get ipv6 address info
 *
 * INPUT:
 *      ifindex     -- interface
 *
 * OUTPUT:
 *      addr_info_p -- pointer to address info
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      None.
 */
UI32_T IP_MGR_GetIfIpv6AddrInfo(NETCFG_TYPE_IpAddressInfoEntry_T *addr_info_p);

#endif /* SYS_CPNT_IPV6 */
#if (SYS_CPNT_IP_TUNNEL == TRUE)
/* FUNCTION NAME : IP_MGR_CreateTunnelInterface
 * PURPOSE:
 *      create tunnel interface
 *
 * INPUT:
 *      tid_ifindex     -- tunnel ifindex
 *
 * OUTPUT:
 *      none
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      None.
 */
UI32_T IP_MGR_CreateTunnelInterface(UI32_T tid_ifindex);

/* FUNCTION NAME : IP_MGR_DeleteTunnelInterface
 * PURPOSE:
 *      destroy tunnel interface
 *
 * INPUT:
 *      tid_ifindex     -- tunnel ifindex
 *
 * OUTPUT:
 *      none
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      None.
 */
UI32_T IP_MGR_DeleteTunnelInterface(UI32_T tid_ifindex);

/* FUNCTION NAME : IP_MGR_SetTunnelSourceAddress
 * PURPOSE:
 *      set tunnel source IPv4 addess
 *
 * INPUT:
 *      tid_ifindex     -- tunnel ifindex
        src_addr
 *
 * OUTPUT:
 *      none
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      None.
 */
UI32_T IP_MGR_SetTunnelSourceAddress(UI32_T tid_ifindex,L_INET_AddrIp_T *src_addr);

/* FUNCTION NAME : IP_MGR_UnsetTunnelSourceAddress
 * PURPOSE:
 *      unset  tunnel source IPv4 addess
 *
 * INPUT:
 *      tid_ifindex     -- tunnel ifindex
 *
 * OUTPUT:
 *      none
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      None.
 */
UI32_T IP_MGR_UnsetTunnelSourceAddress(UI32_T tid_ifindex);

/* FUNCTION NAME : IP_MGR_SetTunnelDestinationAddress
 * PURPOSE:
 *      set  tunnel destination IPv4 addess
 *
 * INPUT:
 *      tid_ifindex     -- tunnel ifindex
        dst_addr
 *
 * OUTPUT:
 *      none
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      None.
 */
UI32_T IP_MGR_SetTunnelDestinationAddress(UI32_T tid_ifindex,L_INET_AddrIp_T *dst_addr);

/* FUNCTION NAME : IP_MGR_UnsetTunnelDestinationAddress
 * PURPOSE:
 *      unset  tunnel destination IPv4 addess
 *
 * INPUT:
 *      tid_ifindex     -- tunnel ifindex
 *
 * OUTPUT:
 *      none
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      None.
 */
UI32_T IP_MGR_UnsetTunnelDestinationAddress(UI32_T tid_ifindex);

/* FUNCTION NAME : IP_MGR_SetTunnelMode
 * PURPOSE:
 *      set  tunnel mode
 *
 * INPUT:
 *      tid_ifindex     -- tunnel ifindex
         mode            -- tunnel mode, type of NETCFG_TYPE_TUNNEL_MODE_T
 *
 * OUTPUT:
 *      none
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      None.
 */
UI32_T IP_MGR_SetTunnelMode(UI32_T tid_ifindex, UI32_T mode);

/* FUNCTION NAME : IP_MGR_SetTunnelTtl
 * PURPOSE:
 *      set  tunnel TTL
 *
 * INPUT:
 *      tid_ifindex     -- tunnel ifindex
         ttl
 *
 * OUTPUT:
 *      none
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      None.
 */
UI32_T IP_MGR_SetTunnelTtl(UI32_T tid_ifindex, UI32_T ttl);

/* FUNCTION NAME : IP_MGR_SetTunnelLocalIPv6Rif
 * PURPOSE:
 *      set  ipv6 address of tunnel interface
 *
 * INPUT:
 *      tid_ifindex : tunnel ifindex
        addr  : ipv6 address
 *
 * OUTPUT:
 *      none
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      None.
 */
UI32_T IP_MGR_SetTunnelLocalIPv6Rif(UI32_T tid_ifindex ,L_INET_AddrIp_T* addr);

/* FUNCTION NAME : IP_MGR_UnsetTunnelLocalIPv6Rif
 * PURPOSE:
 *      remove  ipv6 address of tunnel interface
 *
 * INPUT:
 *      tid_ifindex : tunnel ifindex
        addr  : ipv6 address
 *
 * OUTPUT:
 *      none
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      None.
 */
UI32_T IP_MGR_UnsetTunnelLocalIPv6Rif( UI32_T tid_ifindex, L_INET_AddrIp_T* addr);
#endif /*SYS_CPNT_IP_TUNNEL*/

#endif   /* _IP_MGR_H */
