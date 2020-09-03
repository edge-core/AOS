/*-----------------------------------------------------------------------------
 * FILE NAME: AMTRL3_POM.H
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *    This file declares the APIs for AMTRL3 OM IPC.
 *
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    2008/01/30     --- djd, Create
 *
 * Copyright(C)      Accton Corporation, 2008
 *-----------------------------------------------------------------------------
 */

#ifndef _AMTRL3_POM_H
#define _AMTRL3_POM_H

#include "sys_type.h"
#include "amtrl3_type.h"
#include "amtrl3_type.h"
#if (SYS_CPNT_VXLAN == TRUE)
#include "amtrl3_om.h"
#endif

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : AMTRL3_POM_InitiateProcessResource
 *-----------------------------------------------------------------------------
 * PURPOSE : Initiate resource for AMTRL3_POM in the calling process.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  --  Sucess
 *           FALSE --  Error
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T AMTRL3_POM_InitiateProcessResource(void);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_POM_GetNextIpNetToPhysicalEntry
 * -------------------------------------------------------------------------
 * PURPOSE:  To get the next record of ipNetToPhysical entry after the specified key
 *
 * INPUT:    
 *      action_flags    -- AMTRL3_TYPE_FLAGS_IPV4 \
 *                         AMTRL3_TYPE_FLAGS_IPV6 
 *      fib_id       -- FIB id
 *      ip_net_to_physical_entry -- get the entry match the key fields in this structure
 *                                  KEY: 
 *                                  ip_net_to_physical_if_index
 *                                  ip_net_to_physical_net_address_type
 *                                  ip_net_to_physical_net_address
 *
 * OUTPUT:   ip_net_to_physical_entry -- record of ipNetToPhysical table.
 *
 * RETURN:   TRUE - Successfully
 *           FALSE- No more record (EOF) or can't get other record.
 *
 * NOTES:    
 *      1. If only AMTRL3_TYPE_FLAGS_IPV4 is set (address type must be IPV4), 
 *         only traverse all ipv4 entries.
 *      2. If only AMTRL3_TYPE_FLAGS_IPV6 is set (address type must be IPV6)
 *         only traverse all ipv6 entries.
 *      3. If both IPV4 and IPV6 flag are set (address type may be IPV4 or IPV6),  
 *         if address type is IPV4, traverse IPV4 entries at first then continue on IPV6 entries.
 *         if address type is IPV6, only traverse IPV6 entries.
 *      4. if all keys are zero, means get first one.
 * -------------------------------------------------------------------------*/
BOOL_T  AMTRL3_POM_GetNextIpNetToPhysicalEntry (UI32_T action_flags,
                                                UI32_T fib_id,
                                                AMTRL3_TYPE_ipNetToPhysicalEntry_T  *ip_net_to_physical_entry);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_POM_GetInetHostRouteEntry
 * -------------------------------------------------------------------------
 * PURPOSE: Get ipv4 or ipv6 Host Route Entry
 * INPUT:   
 *      action_flags    -- AMTRL3_TYPE_FLAGS_IPV4 \
 *                         AMTRL3_TYPE_FLAGS_IPV6 
 *      fib_id       -- FIB id
 *      inet_host_route_entry->dst_vid_ifindex --  ifindex (KEY)
 *      inet_host_route_entry->inet_address --  Destination IPv4/v6 Address (KEY)
 * OUTPUT:  inet_host_route_entry
 * RETURN:  TRUE / FALSE    
 * NOTES:
 *      1. If AMTRL3_TYPE_FLAGS_IPV4 is set, only search all ipv4 entries.
 *      2. If AMTRL3_TYPE_FLAGS_IPV6 is set, only search all ipv6 entries.
 *      3. Can not set these 2 flags at same time.
 *         flag must be consistent with inet_host_route_entry->inet_address_type.
 * -------------------------------------------------------------------------
 */
BOOL_T AMTRL3_POM_GetInetHostRouteEntry(
                                        UI32_T action_flags,
                                        UI32_T fib_id,
                                        AMTRL3_TYPE_InetHostRouteEntry_T *host_entry);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_POM_GetNextInetHostRouteEntry
 * -------------------------------------------------------------------------
 * PURPOSE: Get next ipv4 or ipv6 Host Route Entry
 * INPUT:   
 *      action_flags    -- AMTRL3_TYPE_FLAGS_IPV4 \
 *                         AMTRL3_TYPE_FLAGS_IPV6 
 *      fib_id       -- FIB id
 *      inet_host_route_entry->dst_vid_ifindex --  ifindex (KEY)
 *      inet_host_route_entry->inet_address --  Destination IPv4/v6 Address (KEY)
 * OUTPUT:  inet_host_route_entry
 * RETURN:  TRUE / FALSE    
 * NOTES:
 *      1. If AMTRL3_TYPE_FLAGS_IPV4 is set, only search all ipv4 entries.
 *      2. If AMTRL3_TYPE_FLAGS_IPV6 is set, only search all ipv6 entries.
 *      3. If the inet_host_route_entry->inet_address = 0, get the first entry
 * -------------------------------------------------------------------------
 */
BOOL_T AMTRL3_POM_GetNextInetHostRouteEntry(
                                        UI32_T action_flags,
                                        UI32_T fib_id,
                                        AMTRL3_TYPE_InetHostRouteEntry_T *host_entry);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_POM_GetIpNetToPhysicalEntry
 * -------------------------------------------------------------------------
 * PURPOSE:  To get record of IpNetToPhysical entry matching specified key
 *
 * INPUT: 
 *      action_flags    -- AMTRL3_TYPE_FLAGS_IPV4 \
 *                         AMTRL3_TYPE_FLAGS_IPV6 
 *      fib_id       -- FIB id
 *      ip_net_to_physical_entry -- get the entry match the key fields in this structure
 *                                  KEY: 
 *                                  ip_net_to_physical_if_index
 *                                  ip_net_to_physical_net_address_type
 *                                  ip_net_to_physical_net_address
 *
 *
 * OUTPUT:   ip_net_to_physical_entry -- record of IpNetToPhysical table.
 *
 * RETURN:   TRUE - Successfully
 *           FALSE- can't get the specified record.
 *
 * NOTES:
 *      1. action flag AMTRL3_TYPE_FLAGS_IPV4/IPV6 must consistent with
 *         address type in ip_net_to_physical_entry structure.
 * -------------------------------------------------------------------------
 */
BOOL_T AMTRL3_POM_GetIpNetToPhysicalEntry(
                                       UI32_T action_flags,
				       UI32_T fib_id,
				       AMTRL3_TYPE_ipNetToPhysicalEntry_T *ip_net_to_physical_entry);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_POM_GetTotalHostRouteNumber
 * -------------------------------------------------------------------------
 * PURPOSE:  To get the total number of host route entries include ipv4 and ipv6 
 * INPUT:  
 *        action_flags:   AMTRL3_TYPE_FLAGS_IPV4 -- include ipv4 entries
 *                        AMTRL3_TYPE_FLAGS_IPV6 -- include ipv6 entries 
 *        fib_id      :   FIB id
 * OUTPUT:
 * RETURN: total number of host route number which satisfying action_flags
 * NOTES:  none
 * -------------------------------------------------------------------------*/
UI32_T AMTRL3_POM_GetTotalHostRouteNumber(UI32_T action_flags, UI32_T fib_id);

#if (SYS_CPNT_VXLAN == TRUE)
BOOL_T AMTRL3_POM_GetVxlanTunnelEntryByVxlanPort(UI32_T fib_id, AMTRL3_TYPE_VxlanTunnelEntry_T *tunnel_entry_p);
BOOL_T AMTRL3_POM_GetVxlanTunnelEntry(UI32_T fib_id, AMTRL3_OM_VxlanTunnelEntry_T *tunnel_entry_p);
BOOL_T AMTRL3_POM_GetNextVxlanTunnelEntry(UI32_T fib_id, AMTRL3_OM_VxlanTunnelEntry_T *tunnel_entry_p);
#endif

#endif
