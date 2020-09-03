/* MODULE NAME:  netcfg_pom_arp.h
 * PURPOSE:
 *    This file provides APIs for other process or CSC group to access NETCFG_OM_ARP service.
 *    In Linux platform, the communication between CSC group are done via IPC.
 *    Other CSC can call NETCFG_POM_XXX for APIs NETCFG_OM_XXX provided by NETCFG, and same as NETCFG_PMGR_ARP for
 *    NETCFG_MGR_ARP APIs
 *
 * NOTES:
 *
 * REASON:
 * Description:
 * HISTORY
 *    2008/01/18     --- Lin.Li, Create
 *
 * Copyright(C)      Accton Corporation, 2008
 */
 
#ifndef NETCFG_POM_ARP_H
#define NETCFG_POM_ARP_H

#include "sys_type.h"
#include "netcfg_type.h"

/* FUNCTION NAME : NETCFG_POM_ARP_InitiateProcessResource
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
 *    Before other CSC use NETCFG_POM_ARP, it should initiate the resource (get the message queue handler internally)
 
 *
 */
BOOL_T NETCFG_POM_ARP_InitiateProcessResource(void);

/* FUNCTION NAME : NETCFG_POM_ARP_GetIpNetToMediaTimeout
 * PURPOSE:
 *    Get ARP timeout
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    age_time.
 *
 * RETURN:
 *    TRUE  --  Sucess
 *    FALSE --  Error
 *
 * NOTES:
 *
 */
BOOL_T NETCFG_POM_ARP_GetIpNetToMediaTimeout(UI32_T *age_time);
#endif /*NETCFG_POM_ARP_H*/

