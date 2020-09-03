/*-----------------------------------------------------------------------------
 * FILE NAME: NETCFG_PMGR_ARP.H
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *    This file declares the APIs for ARP MGR IPC.
 *
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    2008/01/18     --- Lin.Li, Create
 *
 * Copyright(C)      Accton Corporation, 2008
 *-----------------------------------------------------------------------------
 */

#ifndef NETCFG_PMGR_ARP_H
#define NETCFG_PMGR_ARP_H

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ARP_InitiateProcessResource
 *-----------------------------------------------------------------------------
 * PURPOSE : Initiate resource for NETCFG_PMGR_ARP in the calling process.
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
BOOL_T NETCFG_PMGR_ARP_InitiateProcessResource(void);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ARP_GetNextIpNetToMediaEntry
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
UI32_T NETCFG_PMGR_ARP_GetNextIpNetToMediaEntry(NETCFG_TYPE_IpNetToMediaEntry_T *entry);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ARP_DeleteAllDynamicIpNetToMediaEntry
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
UI32_T NETCFG_PMGR_ARP_DeleteAllDynamicIpNetToMediaEntry(void);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ARP_SetIpNetToMediaTimeout
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
UI32_T NETCFG_PMGR_ARP_SetIpNetToMediaTimeout(UI32_T age_time);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ARP_AddStaticIpNetToMediaEntry
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
UI32_T NETCFG_PMGR_ARP_AddStaticIpNetToMediaEntry(UI32_T vid_ifIndex,
                                                                         UI32_T ip_addr,
                                                                         UI32_T phy_addr_len,
                                                                         UI8_T *phy_addr);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ARP_DeleteStaticIpNetToMediaEntry
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
UI32_T NETCFG_PMGR_ARP_DeleteStaticIpNetToMediaEntry(UI32_T vid_ifIndex, UI32_T ip_addr);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ARP_GetStatistics
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
UI32_T NETCFG_PMGR_ARP_GetStatistics(NETCFG_TYPE_IpNetToMedia_Statistics_T *stat);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ARP_GetIpNetToMediaEntry
 *-----------------------------------------------------------------------------
 * PURPOSE : Get an ARP entry information.
 *
 * INPUT   : entry.
 *
 * OUTPUT  : entry.
 *
 * RETURN  :TRUE/
 *                 FALSE
 *
 * NOTES   :key are ifindex and ip address.
 *-----------------------------------------------------------------------------
 */
BOOL_T NETCFG_PMGR_ARP_GetIpNetToMediaEntry(NETCFG_TYPE_IpNetToMediaEntry_T *entry);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ARP_SetStaticIpNetToMediaEntry
 *-----------------------------------------------------------------------------
 * PURPOSE : Set a static ARP entry invalid or valid.
 *
 * INPUT   : entry, type.
 *
 * OUTPUT  : None.
 *
 * RETURN  :NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T NETCFG_PMGR_ARP_SetStaticIpNetToMediaEntry(NETCFG_TYPE_StaticIpNetToMediaEntry_T *entry, int type);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ARP_GetRunningIpNetToMediaTimeout
 *-----------------------------------------------------------------------------
 * PURPOSE :Get running ARP timeout.
 *
 * INPUT   : None.
 *
 * OUTPUT  : age_time.
 *
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS/
 *                  SYS_TYPE_GET_RUNNING_CFG_FAIL /
 *               SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE  = 3
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T
NETCFG_PMGR_ARP_GetRunningIpNetToMediaTimeout(UI32_T *age_time);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_ARP_GetNextStaticIpNetToMediaEntry
 *-----------------------------------------------------------------------------
 * PURPOSE :Lookup next static ARP entry.
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
UI32_T NETCFG_PMGR_ARP_GetNextStaticIpNetToMediaEntry(NETCFG_TYPE_StaticIpNetToMediaEntry_T *entry);

#endif /* #ifndef NETCFG_PMGR_ARP_H */

