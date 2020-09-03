/*-----------------------------------------------------------------------------
 * FILE NAME: NETCFG_OM_ARP_PRIVATE.H
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    2008/03/10    --- Lin.Li, Create
 *
 * Copyright(C)      Accton Corporation, 2008
 *-----------------------------------------------------------------------------
 */
#ifndef NETCFG_OM_ARP_PRIVATE_H
#define NETCFG_OM_ARP_PRIVATE_H

#include "sys_type.h"
#include "sys_adpt.h"
#include "sys_cpnt.h"
#include "sysfun.h"
#include "netcfg_type.h"

/* FUNCTION NAME : NETCFG_OM_ARP_AddStaticEntry
 * PURPOSE:
 *      Add an static ARP entry.
 *
 * INPUT:
 *      entry
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      Key is (entry.ip_net_to_media_if_index, entry.ip_net_to_media_net_address).
 */
BOOL_T NETCFG_OM_ARP_AddStaticEntry(NETCFG_TYPE_StaticIpNetToMediaEntry_T *entry);

/* FUNCTION NAME : NETCFG_OM_ARP_DeleteStaticEntry
 * PURPOSE:
 *      Remove a static arp entry.
 *
 * INPUT:
 *      entry
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      Key is (entry.ip_net_to_media_if_index, entry.ip_net_to_media_net_address).
 */
BOOL_T	NETCFG_OM_ARP_DeleteStaticEntry(NETCFG_TYPE_StaticIpNetToMediaEntry_T *entry);

/* FUNCTION NAME : NETCFG_OM_ARP_GetStaticEntry
 * PURPOSE:
 *      Get a static arp entry.
 *
 * INPUT:
 *      entry
 *
 * OUTPUT:
 *      entry.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      Key is (entry.ip_net_to_media_if_index, entry.ip_net_to_media_net_address).
 */
BOOL_T NETCFG_OM_ARP_GetStaticEntry(NETCFG_TYPE_StaticIpNetToMediaEntry_T *entry);

/* FUNCTION NAME : NETCFG_OM_ARP_GetNextStaticEntry
 * PURPOSE:
 *      Get next available static arp entry.
 *
 * INPUT:
 *      entry
 *
 * OUTPUT:
 *      entry.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      Key is (entry.ip_net_to_media_if_index, entry.ip_net_to_media_net_address).
 *      If key is (0, 0), get first one.
 */
BOOL_T NETCFG_OM_ARP_GetNextStaticEntry(NETCFG_TYPE_StaticIpNetToMediaEntry_T *entry);

/* FUNCTION NAME : NETCFG_OM_ARP_DeleteAllStaticEntry
 * PURPOSE:
 *          Remove all static arp entries.
 *
 * INPUT:  None.
 *
 * OUTPUT: None.
 *
 * RETURN: None.
 *
 * NOTES:  None.
 */
void NETCFG_OM_ARP_DeleteAllStaticEntry(void);

/* FUNCTION NAME : NETCFG_OM_ARP_GetTimeout
 * PURPOSE:
 *      Get arp age timeout.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      age_time.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 */
BOOL_T NETCFG_OM_ARP_GetTimeout(UI32_T *age_time);

/* FUNCTION NAME : NETCFG_OM_ARP_SetTimeout
 * PURPOSE:
 *      Set arp age timeout.
 *
 * INPUT:
 *      age_time
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 */
BOOL_T NETCFG_OM_ARP_SetTimeout(UI32_T age_time);

/* FUNCTION NAME : NETCFG_OM_ARP_UpdateStaticEntry
 * PURPOSE:
 *      Update a static arp entry.
 *
 * INPUT:
 *      entry
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      Key is (entry.ip_net_to_media_if_index, entry.ip_net_to_media_net_address).
 */
BOOL_T    NETCFG_OM_ARP_UpdateStaticEntry(NETCFG_TYPE_StaticIpNetToMediaEntry_T *entry);
#endif /*NETCFG_OM_ARP_PRIVATE_H*/

