/* FILE NAME: NETCFG_PMGR_PBR.H
 * PURPOSE:
 *    Declare the APIs for IPCs of NETCFG_PMGR_PBR
 *
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    2015/07/14     KH Shi, Create
 *
 * Copyright(C)      Accton Corporation, 2015
 */

#ifndef _NETCFG_PMGR_PBR_H
#define _NETCFG_PMGR_PBR_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/* FUNCTION NAME: NETCFG_PMGR_PBR_InitiateProcessResources
 * PURPOSE:
 *          Initiate resources for PBR_PMGR in the calling process.
 * INPUT:
 *          None.
 * OUTPUT:
 *          None.
 * RETURN:
 *          TRUE    -- Success
 *          FALSE   -- Fail
 * NOTES:
 *          None.
 */
BOOL_T NETCFG_PMGR_PBR_InitiateProcessResources(void);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: NETCFG_PMGR_PBR_BindRouteMap
 * -------------------------------------------------------------------------
 * PURPOSE:  binding vlan with route-map
 * INPUT:    vid       --  VLAN ID
 *           rmap_name --  route-map name
 * OUTPUT:   none.
 * RETURN:   success - NETCFG_TYPE_OK
 *           fail    - Error code
 * NOTES:    
 * -------------------------------------------------------------------------
 */
UI32_T NETCFG_PMGR_PBR_BindRouteMap(UI32_T vid, char rmap_name[SYS_ADPT_MAX_ROUTE_MAP_NAME_LENGTH+1]);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: NETCFG_MGR_PBR_UnbindRouteMap
 * -------------------------------------------------------------------------
 * PURPOSE:  unbinding vlan with route-map
 * INPUT:    vid     --  VLAN ID
 * OUTPUT:   none.
 * RETURN:   success - NETCFG_TYPE_OK
 *           fail    - Error code
 * NOTES:    
 * -------------------------------------------------------------------------
 */
UI32_T NETCFG_PMGR_PBR_UnbindRouteMap(UI32_T vid);

#endif

 

