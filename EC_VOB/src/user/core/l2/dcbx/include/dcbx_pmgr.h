/*-----------------------------------------------------------------------------
 * FILE NAME: DCBX_PMGR.H
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *    This file declares the APIs for DCBX MGR IPC.
 *
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    9/20/2012 - Ricky Lin, Created
 *
 * Copyright(C)      Accton Corporation, 2012
 *-----------------------------------------------------------------------------
 */

#ifndef DCBX_PMGR_H
#define DCBX_PMGR_H


/* INCLUDE FILE DECLARATIONS
 */

#include "sys_type.h"
#include "dcbx_mgr.h"


/* NAMING CONSTANT DECLARATIONS
 */


/* MACRO FUNCTION DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : DCBX_PMGR_InitiateProcessResources
 *-----------------------------------------------------------------------------
 * PURPOSE : Initiate resources for DCBX_PMGR in the calling process.
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
BOOL_T DCBX_PMGR_InitiateProcessResources(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_PMGR_SetPortStatus
 *-------------------------------------------------------------------------
 * PURPOSE  :  Set port_status to control DCBX is enabled
 *             or is disabled.
 * INPUT    : UI32_T lport            -- lport number
 *            BOOL_T port_status      -- Enable(TRUE)
 *                                       Disable(FALSE)
 * OUTPUT   : None
 * RETURN   : DCBX_TYPE_RETURN_OK/DCBX_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 38.4, IEEE Std P802.1Qaz/D2.5.
 *            Default value: Enable(TRUE).
 *-------------------------------------------------------------------------
 */
UI32_T DCBX_PMGR_SetPortStatus(UI32_T lport, BOOL_T port_status);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_PMGR_GetPortStatus
 *-------------------------------------------------------------------------
 * PURPOSE  :  Get port_statu
 * INPUT    : UI32_T lport            -- lport number
 * OUTPUT   : port_status_p
 * RETURN   : DCBX_TYPE_RETURN_OK/DCBX_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 38.4, IEEE Std P802.1Qaz/D2.5.
 *            Default value: Enable(TRUE).
 *-------------------------------------------------------------------------
 */
UI32_T DCBX_PMGR_GetPortStatus(UI32_T lport, BOOL_T *port_status_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_PMGR_GetRunningPortStatus
 *-------------------------------------------------------------------------
 * PURPOSE  :  Get port_status
 * INPUT    : UI32_T lport            -- lport number
 * OUTPUT   : port_status_p
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     : Ref to the description in 38.4, IEEE Std P802.1Qaz/D2.5.
 *            Default value: Enable(TRUE).
 *-------------------------------------------------------------------------
 */
UI32_T DCBX_PMGR_GetRunningPortStatus(UI32_T lport, BOOL_T *port_status_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_PMGR_SetPortMode
 *-------------------------------------------------------------------------
 * PURPOSE  : Set port mode
 * INPUT    : UI32_T lport            -- lport number
 *            UI32_T port_mode        -- manual(1),
 *                                       configuration_source(2),
 *                                       auto-upstream(3),
 *                                       auto-downstream(4)
 * OUTPUT   : None
 * RETURN   : DCBX_TYPE_RETURN_OK/DCBX_TYPE_RETURN_ERROR
 * NOTE     : Default value: manual(1).
 *-------------------------------------------------------------------------
 */
UI32_T DCBX_PMGR_SetPortMode(UI32_T lport, UI32_T port_mode);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_PMGR_GetPortMode
 *-------------------------------------------------------------------------
 * PURPOSE  : Get port mode
 * INPUT    : UI32_T lport            -- lport number
 * OUTPUT   : port_mode_p -- manual(1),
 *                                                  configuration_source(2),
 *                                                  auto-upstream(3),
 *                                                  auto-downstream(4)
 * RETURN   : DCBX_TYPE_RETURN_OK/DCBX_TYPE_RETURN_ERROR
 * NOTE     : Default value: manual(1).
 *-------------------------------------------------------------------------
 */
UI32_T DCBX_PMGR_GetPortMode(UI32_T lport, UI32_T *port_mode_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_PMGR_GetRunningPortMode
 *-------------------------------------------------------------------------
 * PURPOSE  : Get port mode
 * INPUT    : UI32_T lport            -- lport number
 * OUTPUT   : port_mode_p -- manual(1),
 *                                                  configuration_source(2),
 *                                                  auto-upstream(3),
 *                                                  auto-downstream(4)
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     : Default value: manual(1).
 *-------------------------------------------------------------------------
 */
UI32_T DCBX_PMGR_GetRunningPortMode(UI32_T lport, UI32_T *port_mode_p);

#if 0
/*-------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_PMGR_GetEtsWillingBit
 *-------------------------------------------------------------------------
 * PURPOSE  : Get operating ETS willing bit for specified port.
 * INPUT    : UI32_T lport            -- lport number
 * OUTPUT   : BOOL_T *ets_willing_bit_p
 * RETURN   : DCBX_TYPE_RETURN_OK/DCBX_TYPE_RETURN_ERROR
 * NOTE     : 
 *-------------------------------------------------------------------------
 */
UI32_T  DCBX_PMGR_GetEtsWillingBit(UI32_T lport, BOOL_T *ets_willing_bit_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_PMGR_GetPfcWillingBit
 *-------------------------------------------------------------------------
 * PURPOSE  : Get operating PFC willing bit for specified port.
 * INPUT    : UI32_T lport            -- lport number
 * OUTPUT   : BOOL_T *pfc_willing_bit_p
 * RETURN   : DCBX_TYPE_RETURN_OK/DCBX_TYPE_RETURN_ERROR
 * NOTE     : 
 *-------------------------------------------------------------------------
 */
UI32_T  DCBX_PMGR_GetPfcWillingBit(UI32_T lport, BOOL_T *pfc_willing_bit_p);
#endif

#endif /* #ifndef DCBX_PMGR_H */
