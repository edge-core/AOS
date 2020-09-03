/*-----------------------------------------------------------------------------
 * FILE NAME: LACP_POM.H
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *    This file declares the APIs for LACP OM IPC.
 *
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    2007/06/21     --- Timon, Create
 *
 * Copyright(C)      Accton Corporation, 2007
 *-----------------------------------------------------------------------------
 */

#ifndef LACP_POM_H
#define LACP_POM_H


/* INCLUDE FILE DECLARATIONS
 */

#include "sys_type.h"


/* NAMING CONSTANT DECLARATIONS
 */


/* MACRO FUNCTION DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - LACP_POM_InitiateProcessResource
 *-----------------------------------------------------------------------------
 * PURPOSE : Initiate resource for LACP_POM in the calling process.
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
BOOL_T LACP_POM_InitiateProcessResource(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LACP_POM_GetRunningDot3adLacpPortEnabled
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the LACP enabled value of a port.
 * INPUT    :   UI32_T lport            -- lport number
 * OUTPUT   :   UI32_T *lacp_state        -- pointer of the enable value
 *                                         VAL_LacpStuts_enable or VAL_LacpStuts_disable (defined in swctrl.h)
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T LACP_POM_GetRunningDot3adLacpPortEnabled(UI32_T ifindex, UI32_T *lacp_state);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_OM_GetRunningDot3adAggPortPartnerAdminSystemId
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the dot3ad_agg_port_partner_admin_system_id information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 * OUTPUT   :   UI8_T   *system_id  -- the dot3ad_agg_port_partner_admin_system_id value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_POM_GetRunningDot3adAggPortPartnerAdminSystemId(UI16_T port_index, UI8_T *system_id);


#endif /* #ifndef LACP_POM_H */
