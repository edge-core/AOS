/* MODULE NAME:  amtrl3_pmgr.hs
 * PURPOSE:
 *    This file provides APIs for other process or CSC group to access AMTRL3_MGR and AMTRL3_OM service.
 *    In Linux platform, the communication between CSC group are done via IPC.
 *    Other CSC can call AMTRL3_PMGR_XXX for APIs AMTRL3_MGR_XXX provided by AMTRL3, and same as AMTRL3_POM for
 *    AMTRL3_OM APIs
 *
 * NOTES:
 *
 * REASON:
 * Description:
 * HISTORY
 *    01/16/2008 - djd, Created
 *
 * Copyright(C)      Accton Corporation, 2008
 */
#ifndef _AMTRL3_PMGR_H_
#define _AMTRL3_PMGR_H_

#include "amtrl3_type.h"


/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME : AMTRL3_PMGR_InitiateProcessResource
 * PURPOSE:
 *    Initiate resource used in the calling process.
 *
 * INPUT:
 *    None
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE  --  Sucess
 *    FALSE --  Error
 *
 * NOTES:
 *    Before other CSC use AMTRL3_PMGR, it should initiate the resource (get the message queue handler internally)
 *
 */
BOOL_T AMTRL3_PMGR_InitiateProcessResource(void);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_PMGR_CreateFIB
 * -------------------------------------------------------------------------
 * PURPOSE:  create a FIB instance
 * INPUT:    fib_id       --  FIB id (1 ~ 255).
 * OUTPUT:   none.
 * RETURN:   AMTRL3_PMGR_FIB_SUCCESS, 
 *           AMTRL3_PMGR_FIB_ALREADY_EXIST,
 *           AMTRL3_PMGR_FIB_OM_ERROR,
 *           AMTRL3_PMGR_FIB_FAIL.
 * NOTES:
 * -------------------------------------------------------------------------*/
UI32_T AMTRL3_PMGR_CreateFIB(UI32_T fib_id);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_PMGR_DeleteFIB()
 * -------------------------------------------------------------------------
 * PURPOSE:  Delete a FIB
 * INPUT:    fib_id  --  FIB id (1 ~ 255).
 * OUTPUT:   none.
 * RETURN:   AMTRL3_PMGR_FIB_SUCCESS, 
 *           AMTRL3_PMGR_FIB_NOT_EXIST,
 *           AMTRL3_PMGR_FIB_OM_ERROR,
 *           AMTRL3_PMGR_FIB_FAIL.
 * NOTES:   
 * -------------------------------------------------------------------------*/
UI32_T AMTRL3_PMGR_DeleteFIB(UI32_T fib_id);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_PMGR_EnableIpForwarding
 * -------------------------------------------------------------------------
 * PURPOSE:  Enable ipv4 or ipv6 forwarding function in chip.
 * INPUT:    action_flags    --  AMTRL3_TYPE_FLAGS_IPV4 \
 *                               AMTRL3_TYPE_FLAGS_IPV6
 *           vr_id          --  Virtual Router id            
 * OUTPUT:   none.               
 * RETURN:   TRUE / FALSE
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_PMGR_EnableIpForwarding(UI32_T action_flags, UI32_T vr_id);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_PMGR_DisableIpForwarding
 * -------------------------------------------------------------------------
 * PURPOSE:  Disable ipv4 or ipv6 forwarding function in chip.
 * INPUT:    action_flags    --  AMTRL3_TYPE_FLAGS_IPV4 \
 *                               AMTRL3_TYPE_FLAGS_IPV6
 *           vr_id          --  Virtual Router id            
 * OUTPUT:   none.               
 * RETURN:   TRUE / FALSE
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_PMGR_DisableIpForwarding(UI32_T action_flags, UI32_T vr_id);


/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_PMGR_SetHostRoute
 * -------------------------------------------------------------------------
 * PURPOSE:  Add one ipv4 or ipv6 host route to Host Route database 
 *           and host route table in chip.
 * INPUT:    action_flags    --  AMTRL3_TYPE_FLAGS_IPV4 \
 *                               AMTRL3_TYPE_FLAGS_IPV6
 *           fib_id          --  FIB id            
 *           host_entry      --  Inet Host Route Entry
 *                               all structure fileds must be set
 *           type            --  VAL_ipNetToPhysicalExtType_other     \
 *                               VAL_ipNetToPhysicalExtType_invalid   \
 *                               VAL_ipNetToPhysicalExtType_dynamic   \
 *                               VAL_ipNetToPhysicalExtType_static    \
 *                               VAL_ipNetToPhysicalExtType_local     \
 *                               VAL_ipNetToPhysicalExtType_broadcast \
 *                               VAL_ipNetToPhysicalExtType_vrrp      \  
 * OUTPUT:   none.               
 * RETURN:   TRUE / FALSE
 * NOTES:    1. IML should get the return value to decide whether the ARP pkt
 *              should be sent to TCP/IP stack, if it is TRUE, need send.
 *           2. For link local ipv6 address, address type and zone index must 
 *              be correctly set.
 *           3. only one flag can be set in one time
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_PMGR_SetHostRoute(UI32_T action_flags,
                                            UI32_T fib_id,
                                            AMTRL3_TYPE_InetHostRouteEntry_T *host_entry,
                                            UI32_T type);

/* -------------------------------------------------------------------------
 * FUNCTION NAME : AMTRL3_PMGR_DeleteHostRoute
 * -------------------------------------------------------------------------
 * PURPOSE:
 *      Remove IPv4 or IPv6 neighbor entry base on given IpNetToPhsical entry information
 *
 * INPUT:
 *      action_flags    -- AMTRL3_TYPE_FLAGS_IPV4 \
 *                         AMTRL3_TYPE_FLAGS_IPV6 
 *      fib_id          -- FIB id
 *      inet_host_entry -- delete the entry match the key fields in this structure
 *                         KEY: dst_inet_addr
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE : AMTRL3_MGR_OK   - Successfully delete the entry.
 *      FALSE: AMTRL3_MGR_CAN_NOT_FIND   -- the host entry do not exist.
 *             AMTRL3_MGR_CAN_NOT_DELETE -- Error in removing specific entry
 *
 * NOTES:
 *      1. action flag: AMTRL3_TYPE_FLAGS_IPV4/IPV6 must consistent with
 *         address type in ip_net_to_physical_entry structure.
 -------------------------------------------------------------------------*/
BOOL_T  AMTRL3_PMGR_DeleteHostRoute( UI32_T action_flags,
                                                UI32_T fib_id,
                                                AMTRL3_TYPE_InetHostRouteEntry_T *host_entry);

#if (SYS_CPNT_PBR == TRUE)
/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_PMGR_DeleteHostRouteForPbr
 * -------------------------------------------------------------------------
 * PURPOSE:  Add a host route entry in OM if it doesn't exist yet
 * INPUT:    action_flags   -- AMTRL3_TYPE_FLAGS_IPV4
 *           fib_id         -- FIB id            
 *           host_entry     -- host route entry
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE
 * NOTES:    1. dst_mac is ignored.
 *           2. do address resolution on dst_vid_ifindex.
 *           3. increase ref_count & pbr_ref_count, and do address resolution(ARP)
 * -------------------------------------------------------------------------
 */
BOOL_T AMTRL3_PMGR_SetHostRouteForPbr(UI32_T action_flags,
                                      UI32_T fib_id,
                                      AMTRL3_TYPE_InetHostRouteEntry_T *host_entry);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_PMGR_DeleteHostRouteForPbr
 * -------------------------------------------------------------------------
 * PURPOSE:  Unreference host route entry.
 * INPUT:    action_flags   -- AMTRL3_TYPE_FLAGS_IPV4
 *           fib_id         -- FIB id            
 *           host_entry     -- host route entry
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE
 * NOTES:    1. decrease the ref_count & pbr_ref_count
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_PMGR_DeleteHostRouteForPbr(UI32_T action_flags,
                                         UI32_T fib_id,
                                         AMTRL3_TYPE_InetHostRouteEntry_T *host_entry);
#endif

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_PMGR_ReplaceExistHostRoute
 * -------------------------------------------------------------------------
 * PURPOSE:  Modify an existing host route, this function will search the host
 *           table, if the IP existed, then update the entry based on the new
 *           information, if the IP is not exist in the table then do nothing.
 * INPUT:    action_flags    --  AMTRL3_TYPE_FLAGS_IPV4 \
 *                               AMTRL3_TYPE_FLAGS_IPV6
 *           fib_id       --  FIB id
 *           host_entry      --  host route entry
 *           type            --  VAL_ipNetToPhysicalExtType_other     \
 *                               VAL_ipNetToPhysicalExtType_invalid   \
 *                               VAL_ipNetToPhysicalExtType_dynamic   \
 *                               VAL_ipNetToPhysicalExtType_static    \
 *                               VAL_ipNetToPhysicalExtType_local     \
 *                               VAL_ipNetToPhysicalExtType_broadcast \
 *                               VAL_ipNetToPhysicalExtType_vrrp      \  
 * OUTPUT:   none.               
 * RETURN:   TRUE / FALSE
 * NOTE:
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_PMGR_ReplaceExistHostRoute(UI32_T action_flags,
                                        UI32_T fib_id,
                                        AMTRL3_TYPE_InetHostRouteEntry_T *host_entry,
                                        UI32_T type);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_PMGR_SetIpNetToPhysicalEntryTimeout
 * -------------------------------------------------------------------------
 * PURPOSE:  Set host route table ageout timer
 * INPUT:    action_flags    --  AMTRL3_TYPE_FLAGS_IPV4 \
 *                               AMTRL3_TYPE_FLAGS_IPV6
 *           fib_id       --  FIB id
 *           v4_timeout      --  ageout time in seconds for ipv4 entries
 *           v6_timeout      --  ageout time in seconds for ipv6 entries
 * OUTPUT:   none.
 * RETURN:   TRUE / FALSE
 * NOTES:    Only set the age timer with the corresponding flag set.
 *           If only AMTRL3_TYPE_FLAGS_IPV4 was set, only ipv4 entries
 *           ageout timer will be set.
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_PMGR_SetIpNetToPhysicalEntryTimeout(UI32_T action_flags,
                                                    UI32_T fib_id,
                                                    UI32_T v4_timeout,
                                                    UI32_T v6_timeout);

/* -------------------------------------------------------------------------
 * FUNCTION NAME : AMTRL3_PMGR_SetInetCidrRouteEntry
 * -------------------------------------------------------------------------
 * PURPOSE: Set one record to ipv4 or ipv6 net route table 
 *          and set configuration to driver
 *
 * INPUT:
 *      action_flags    -- AMTRL3_TYPE_FLAGS_IPV4 \
 *                         AMTRL3_TYPE_FLAGS_IPV6 
 *      fib_id       -- FIB id
 *      net_route_entry -- record of forwarding table.
 *                         key :
 *                           inet_cidr_route_dest          -- destination inet address
 *                           inet_cidr_route_pfxlen        -- prefix length
 *                           inet_cidr_route_policy        -- serve as an additional index which 
 *                                                            may delineate between multiple entries to 
 *                                                            the same destination.
 *                                                            Default is {0,0}
 *                           inet_cidr_route_next_hop      -- next hop inet address.
 *
 * OUTPUT: none.
 *
 * RETURN: 
 *      TRUE : AMTRL3_MGR_OK              -- Successfully insert to database
 *      FALSE: AMTRL3_MGR_ALREADY_EXIST   -- this entry existed in chip/database.
 *             AMTRL3_MGR_TABLE_FULL      -- over reserved space
 *                                       
 * NOTES: 1. If the entry with same key value already exist, this function will
 *        update this entry except those key fields value.
 *        2. action flag: AMTRL3_TYPE_FLAGS_IPV4/IPV6 must consistent with
 *        address type in net_route_entry structure.
 *        3. for dest and nexthop address, unused bytes must be zero. 
 * -------------------------------------------------------------------------*/
BOOL_T  AMTRL3_PMGR_SetInetCidrRouteEntry(UI32_T action_flags,
                                          UI32_T fib_id,
                                          AMTRL3_TYPE_InetCidrRouteEntry_T  *net_route_entry);

/* -------------------------------------------------------------------------
 * FUNCTION NAME : AMTRL3_PMGR_DeleteInetCidrRouteEntry
 * -------------------------------------------------------------------------
 * PURPOSE: Delete specified ipv4 or ipv6 record with the key from database and chip.
 *
 * INPUT:
 *      action_flags    -- AMTRL3_TYPE_FLAGS_IPV4 \
 *                         AMTRL3_TYPE_FLAGS_IPV6 
 *      fib_id       -- FIB id
 *      net_route_entry -- record of forwarding table, only key fields are useful
 *                         in this function.
 *                         KEY :
 *                           inet_cidr_route_dest          -- destination inet address
 *                           inet_cidr_route_pfxlen        -- prefix length
 *                           inet_cidr_route_policy        -- serve as an additional index which 
 *                                                            may delineate between multiple entries to 
 *                                                            the same destination.
 *                                                            Default is {0,0}
 *                           inet_cidr_route_next_hop      -- next hop inet address.

 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE    - AMTRL3_MGR_OK                    -- Successfully remove from database
 *      FALSE   - AMTRL3_MGR_CAN_NOT_FIND          -- This entry does not exist in chip/database.
 *                AMTRL3_MGR_CAN_NOT_DELETE        -- Cannot Delete
 *
 * NOTES:
 *      1. action flag: AMTRL3_TYPE_FLAGS_IPV4/IPV6 must consistent with
 *      address type in net_route_entry structure.
 *      2. Only key fields in net_route_entry are used in this function. 
 *      Other fields can have random value.
 * -------------------------------------------------------------------------*/
BOOL_T  AMTRL3_PMGR_DeleteInetCidrRouteEntry( UI32_T action_flags,
                                            UI32_T fib_id,
                                            AMTRL3_TYPE_InetCidrRouteEntry_T  *net_route_entry);

/* -------------------------------------------------------------------------
 * FUNCTION NAME : AMTRL3_PMGR_AddECMPRouteMultiPath
 * -------------------------------------------------------------------------
 * PURPOSE: Set ECMP route with multiple paths
 *
 * INPUT:
 *      action_flags    -- AMTRL3_TYPE_FLAGS_IPV4 \
 *                         AMTRL3_TYPE_FLAGS_IPV6 
 *      fib_id       -- FIB id
 *      net_route_entry -- array records of forwarding table, only key fields are useful
 *                          in this function
 *                         key :
 *                           inet_cidr_route_dest          -- destination inet address
 *                           inet_cidr_route_pfxlen        -- prefix length
 *                           inet_cidr_route_policy        -- serve as an additional index which 
 *                                                            may delineate between multiple entries to 
 *                                                            the same destination.
 *                                                            Default is {0,0}
 *                           inet_cidr_route_next_hop      -- next hop inet address.
 *
 * OUTPUT: none.
 *
 * RETURN: TRUE : AMTRL3_MGR_OK       -- Successfully insert to database
 *         FALSE: AMTRL3_MGR_TABLE_FULL      -- over reserved space
 * NOTES: 1. The key fields (dest_type, dest, pfxlen, policy) must be 
 *           the same in all records of the array. Upper layer (NSM) must confirm.
 *        2. If route exists, it must set the flag AMTRL3_TYPE_FLAGS_ECMP.
 *        3. If path number overflows the spec, only add those in front.
 * -------------------------------------------------------------------------*/
BOOL_T  AMTRL3_PMGR_AddECMPRouteMultiPath(UI32_T action_flags,
                                          UI32_T fib_id,
                                          AMTRL3_TYPE_InetCidrRouteEntry_T  *net_route_entry,
                                          UI32_T num);

/* -------------------------------------------------------------------------
 * FUNCTION NAME : AMTRL3_PMGR_DeleteECMPRoute
 * -------------------------------------------------------------------------
 * PURPOSE: Delete the ECMP route with all paths.
 *
 * INPUT:
 *      action_flags    -- AMTRL3_TYPE_FLAGS_IPV4 \
 *                         AMTRL3_TYPE_FLAGS_IPV6 
 *      fib_id       -- FIB id
 *      net_route_entry -- only key fields (dest and pfxlen) are useful for deleting
 *                         key :
 *                           inet_cidr_route_dest          -- destination inet address
 *                           inet_cidr_route_pfxlen        -- prefix length
 *
 * OUTPUT: none.
 *
 * RETURN: TRUE : AMTRL3_MGR_OK      -- Successfully remove from database
 *         FALSE: 
 *                -AMTRL3_MGR_CAN_NOT_FIND -- This entry does not exist in chip/database.
 *                -AMTRL3_MGR_CAN_NOT_DELETE   -- Cannot Delete 
 * NOTES: 1. If route exists, it must set the flag AMTRL3_TYPE_FLAGS_ECMP. 
 *        2. action_flags: AMTRL3_TYPE_FLAGS_IPV4/IPV6 must consistent with address type in dest
 * -------------------------------------------------------------------------*/
BOOL_T  AMTRL3_PMGR_DeleteECMPRoute(UI32_T action_flags,
                                          UI32_T fib_id,
                                          AMTRL3_TYPE_InetCidrRouteEntry_T  *net_route_entry);


/* -------------------------------------------------------------------------
 * FUNCTION NAME : AMTRL3_PMGR_AddECMPRouteOnePath
 * -------------------------------------------------------------------------
 * PURPOSE: Set ECMP route with one path
 *
 * INPUT:
 *      action_flags    -- AMTRL3_TYPE_FLAGS_IPV4 \
 *                         AMTRL3_TYPE_FLAGS_IPV6 
 *      fib_id       -- FIB id
 *      net_route_entry -- records of forwarding table, only key fields are useful 
 *                          in this function
 *                         key :
 *                           inet_cidr_route_dest          -- destination inet address
 *                           inet_cidr_route_pfxlen        -- prefix length
 *                           inet_cidr_route_policy        -- serve as an additional index which 
 *                                                            may delineate between multiple entries to 
 *                                                            the same destination.
 *                                                            Default is {0,0}
 *                           inet_cidr_route_next_hop      -- next hop inet address.
 *
 * OUTPUT: none.
 *
 * RETURN: TRUE : AMTRL3_MGR_OK       -- Successfully insert to database
 *         FALSE: AMTRL3_MGR_TABLE_FULL      -- over reserved space
 * NOTES: 1. If route exists, it must set the flag AMTRL3_TYPE_FLAGS_ECMP. 
 * -------------------------------------------------------------------------*/
BOOL_T  AMTRL3_PMGR_AddECMPRouteOnePath(UI32_T action_flags,
                                          UI32_T fib_id,
                                          AMTRL3_TYPE_InetCidrRouteEntry_T  *net_route_entry);


/* -------------------------------------------------------------------------
 * FUNCTION NAME : AMTRL3_PMGR_DeleteECMPRouteOnePath
 * -------------------------------------------------------------------------
 * PURPOSE: Delete the ECMP route with the given path
 *
 * INPUT:
 *      action_flags    -- AMTRL3_TYPE_FLAGS_IPV4 \
 *                         AMTRL3_TYPE_FLAGS_IPV6 
 *      fib_id       -- FIB id
 *      net_route_entry -- records of forwarding table, only key fields are useful 
 *                          in this function
 *                         key :
 *                           inet_cidr_route_dest          -- destination inet address
 *                           inet_cidr_route_pfxlen        -- prefix length
 *                           inet_cidr_route_policy        -- serve as an additional index which 
 *                                                            may delineate between multiple entries to 
 *                                                            the same destination.
 *                                                            Default is {0,0}
 *                           inet_cidr_route_next_hop      -- next hop inet address.
 *
 * OUTPUT: none.
 *
 * RETURN: TRUE : AMTRL3_MGR_OK       -- Successfully remove from database
 *         FALSE: 
 *                   -AMTRL3_MGR_CAN_NOT_FIND -- This entry does not exist in chip/database.
 *                    AMTRL3_MGR_CAN_NOT_DELETE        -- Cannot Delete
 * NOTES: 1. If route exists, it must set the flag AMTRL3_TYPE_FLAGS_ECMP. 
 *        2. action_flags: AMTRL3_TYPE_FLAGS_IPV4/IPV6 must consistent with address type in dest.
 * -------------------------------------------------------------------------*/
BOOL_T  AMTRL3_PMGR_DeleteECMPRouteOnePath(UI32_T action_flags,
                                          UI32_T fib_id,
                                          AMTRL3_TYPE_InetCidrRouteEntry_T  *net_route_entry);

/* -------------------------------------------------------------------------
 *  FUNCTION NAME: AMTRL3_PMGR_AddL3Mac
 * -------------------------------------------------------------------------
 * PURPOSE:  Add one MAC address with L3 bit On
 * INPUT:    
 *           l3_mac:       MAC address of vlan interface
 *           vid_ifindex:    VLAN ifindex of this MAC address
 * OUTPUT:   none.
 * RETURN:   TRUE/FALSE
 * NOTES:    
 * -------------------------------------------------------------------------*/ 
BOOL_T AMTRL3_PMGR_AddL3Mac(UI8_T *l3_mac, UI32_T vid_ifIndex);

/* -------------------------------------------------------------------------
 *  FUNCTION NAME: AMTRL3_PMGR_DeleteL3Mac
 * -------------------------------------------------------------------------
 * PURPOSE: Remove one L3 MAC address that belonging to one vlan interface.
 * INPUT:   
 *          l3_mac:       MAC address of vlan interface
 *          vid_ifIndex:    VLAN ifIndex of this MAC address
 * OUTPUT:  none
 * RETURN:  TRUE/FALSE
 * NOTES:
 * -------------------------------------------------------------------------*/ 
BOOL_T AMTRL3_PMGR_DeleteL3Mac(UI8_T *l3_mac, UI32_T vid_ifIndex);

/* -------------------------------------------------------------------------
 * FUNCTION NAME : AMTRL3_PMGR_ClearAllDynamicARP
 * -------------------------------------------------------------------------
 * PURPOSE: Clear all dynamic ARP from chip and TCP/IP stack
 *
 * INPUT:
 *      action_flags    -- AMTRL3_TYPE_FLAGS_IPV4 \
 *                         AMTRL3_TYPE_FLAGS_IPV6 
 *      fib_id          -- FIB id
 *
 * OUTPUT: none.
 *
 * RETURN: 
 *          TRUE        -- Delete successfully
 *          FALSE:      -- Delete fail
 * NOTES:  If set AMTRL3_TYPE_FLAGS_IPV4 flag, only delete all ipv4 dynamic ARP.
 *         If set AMTRL3_TYPE_FLAGS_IPV6 flag, only delete all ipv6 dynamic ARP.
 *         If all flag set, delete all ipv4 & ipv6 dynamic ARP.
 * -------------------------------------------------------------------------*/
BOOL_T  AMTRL3_PMGR_ClearAllDynamicARP(UI32_T action_flags, UI32_T fib_id);
/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTRL3_PMGR_MACTableDeleteByMstidnPort
 *------------------------------------------------------------------------
 * FUNCTION: The function is used to delete port mac on this msit associated vlan
 * INPUT   : lport_ifindex - specific port removed from vlan member list
 *           mstid - specific spaning-tree msti index
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : none                                                         
 *------------------------------------------------------------------------*/
BOOL_T AMTRL3_PMGR_MACTableDeleteByMstidOnPort(UI32_T mstid, UI32_T lport_ifindex);

/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTRL3_PMGR_SignalL3IfRifDestroy
 *------------------------------------------------------------------------
 * FUNCTION: The function is used to inform the destroy of rif on interface
 * INPUT   : ifindex   - the interface where rif is set
 *           ip_addr_p - the ip address of the rif
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : none                                                         
 *------------------------------------------------------------------------
 */
BOOL_T AMTRL3_PMGR_SignalL3IfRifDestroy(UI32_T ifindex, L_INET_AddrIp_T *ip_addr_p);

#if (SYS_CPNT_IP_TUNNEL == TRUE)
/*BOOL_T  AMTRL3_PMGR_SetDynamicTunnel(UI32_T  tidIfindex, UI32_T  tunnel_type,   L_INET_AddrIp_T * src_addr, L_INET_AddrIp_T * dest_addr);*/
/*--------------------------------------------------------------------------
 * FUNCTION NAME - AMTRL3_PMGR_DeleteTunnelEntries
 *--------------------------------------------------------------------------
 * PURPOSE  : This function deletes tunnel's host route
 * INPUT    : fibid        -- fib index
 *            tidIfindex   -- tunnle l3 ifindex 
 * OUTPUT   : none
 * RETURN   : TRUE/FALSE
 * NOTES    : none
 *--------------------------------------------------------------------------
 */
BOOL_T AMTRL3_PMGR_DeleteTunnelEntries(UI32_T fib_id, UI32_T  tidIfindex);
/* FUNCTION NAME:  AMTRL3_PMGR_TunnelUpdateTtl
 * PURPOSE : Update chip's tunnel ttl value 
 *
 * INPUT   : host_entry->dst_vid_ifindex         -- tunnel's vlan ifindex
 *                       tunnel_dest_inet_addr   -- ipv4 destination endpoint address
 *                       tunnel_src_vidifindex   -- ipv4 source endpoint vlan ifindex
 *                       tunnel_entry_type       -- tunnel mode
 *           ttl                                 -- ttl in ipv4 header   
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE/FALSE.
 *
 * NOTES   : None.
 *
 */
BOOL_T AMTRL3_PMGR_TunnelUpdateTtl(AMTRL3_TYPE_InetHostRouteEntry_T *host_entry, UI32_T  ttl);
#endif /*SYS_CPNT_IP_TUNNEL*/

#if (SYS_CPNT_VXLAN == TRUE)
BOOL_T AMTRL3_PMGR_AddVxlanTunnel(UI32_T fib_id, AMTRL3_TYPE_VxlanTunnelEntry_T* vxlan_tunnel_p);
BOOL_T AMTRL3_PMGR_DeleteVxlanTunnel(UI32_T fib_id, AMTRL3_TYPE_VxlanTunnelEntry_T* vxlan_tunnel_p);
BOOL_T AMTRL3_PMGR_AddVxlanTunnelNexthop(UI32_T fib_id, AMTRL3_TYPE_VxlanTunnelNexthopEntry_T* tunnel_nexthop_p);
BOOL_T AMTRL3_PMGR_DeleteVxlanTunnelNexthop(UI32_T fib_id, AMTRL3_TYPE_VxlanTunnelNexthopEntry_T* tunnel_nexthop_p);
#endif

#endif
