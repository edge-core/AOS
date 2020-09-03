/* MODULE NAME:  netcfg_pom_pbr.h
 * PURPOSE:
 *    This file provides APIs for other process or CSC group to 
 *    access NETCFG_OM_PBR service.
 *
 * NOTES:
 *
 * REASON:
 * Description:
 * HISTORY:
 *    2015/07/16     KH Shi, Create
 *
 * Copyright(C)      Accton Corporation, 2015
 */


/* INCLUDE FILE DECLARATIONS
 */
 
#ifndef NETCFG_POM_PBR_H
#define NETCFG_POM_PBR_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "netcfg_om_pbr.h"

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
/* FUNCTION NAME : NETCFG_POM_PBR_InitiateProcessResource
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
 *    Before other CSC use NETCFG_POM_PBR, it should initiate the resource (get the message queue handler internally)
 *
 */
BOOL_T NETCFG_POM_PBR_InitiateProcessResources(void);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: NETCFG_POM_PBR_GetNextBindingEntry
 * -------------------------------------------------------------------------
 * PURPOSE:  Get next binding entry from OM
 * INPUT:    binding_entry_p -- NETCFG_OM_PBR_BindingEntry_T
 * OUTPUT:   none
 * RETURN:   TRUE/FALSE
 * NOTES:    
 * -------------------------------------------------------------------------
 */
BOOL_T NETCFG_POM_PBR_GetNextBindingEntry(NETCFG_OM_PBR_BindingEntry_T *binding_entry_p);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: NETCFG_OM_PBR_GetRunningBindingRouteMap
 * -------------------------------------------------------------------------
 * PURPOSE:  Get the binding route-map for a vlan
 * INPUT:    vid       -- vlan id
 * OUTPUT:   rmap_name -- bing route-map name
 * RETURN:   
 *           SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE 
 * NOTES:    
 * -------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T NETCFG_POM_PBR_GetRunningBindingRouteMap(UI32_T vid, char rmap_name[SYS_ADPT_MAX_ROUTE_MAP_NAME_LENGTH+1]);
#endif /* NETCFG_POM_PBR_H */

