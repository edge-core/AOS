#ifndef _ARP_MGR_H
#define _ARP_MGR_H

#include <sys_type.h>
#include "sysfun.h"
#include "sys_type.h"
#include "netcfg_type.h"

/* FUNCTION NAME : ARP_MGR_InitiateProcessResources
 * PURPOSE: Initialize process resources for ARP_MGR
 *
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
void ARP_MGR_InitiateProcessResources(void);

/* FUNCTION NAME : ARP_MGR_GetIpNetToMediaEntry
 * PURPOSE:
 *      Get an arp entry.
 *
 * INPUT:
 *      entry
 *
 * OUTPUT:
 *      entry.
 *
 * RETURN  :NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES:
 */
UI32_T     ARP_MGR_GetIpNetToMediaEntry(NETCFG_TYPE_IpNetToMediaEntry_T *entry);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : ARP_MGR_AddStaticIpNetToMediaEntry
 *-----------------------------------------------------------------------------
 * PURPOSE : Add a static ARP entry.
 *
 * INPUT   : vid_ifindex, ip_addr, phy_addr_len and phy_addr.
 *
 * OUTPUT  : None.
 *
 * RETURN  :NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T ARP_MGR_AddStaticIpNetToMediaEntry(UI32_T vid_ifIndex,
                                                            UI32_T ip_addr,
                                                            UI32_T phy_addr_len,
                                                            UI8_T *phy_addr);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : ARP_MGR_DeleteStaticIpNetToMediaEntry
 *-----------------------------------------------------------------------------
 * PURPOSE : Delete a static ARP entry.
 *
 * INPUT   : vid_ifindex, ip_addr.
 *
 * OUTPUT  : None.
 *
 * RETURN  :NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T ARP_MGR_DeleteStaticIpNetToMediaEntry(UI32_T vid_ifIndex, UI32_T ip_addr);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : ARP_MGR_SetIpNetToMediaEntryTimeout
 *-----------------------------------------------------------------------------
 * PURPOSE : Set ARP timeout.
 *
 * INPUT   : age_time.
 *
 * OUTPUT  : None.
 *
 * RETURN  :NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T ARP_MGR_SetIpNetToMediaEntryTimeout(UI32_T age_time, UI32_T pre_timeout);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : ARP_MGR_DeleteAllDynamicIpNetToMediaEntry
 *-----------------------------------------------------------------------------
 * PURPOSE : Delete all dynamic ARP entries.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  :NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T ARP_MGR_DeleteAllDynamicIpNetToMediaEntry(void);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : ARP_MGR_GetNextIpNetToMediaEntry
 *-----------------------------------------------------------------------------
 * PURPOSE : Look up next ARP entry.
 *
 * INPUT   : entry.
 *
 * OUTPUT  : entry.
 *
 * RETURN  :NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T ARP_MGR_GetNextIpNetToMediaEntry(NETCFG_TYPE_IpNetToMediaEntry_T *entry);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : ARP_MGR_GetStatistic
 *-----------------------------------------------------------------------------
 * PURPOSE : Get ARP packet statistics.
 *
 * INPUT   : None.
 *
 * OUTPUT  : stat.
 *
 * RETURN  :NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T ARP_MGR_GetStatistic(NETCFG_TYPE_IpNetToMedia_Statistics_T *stat);


#endif
