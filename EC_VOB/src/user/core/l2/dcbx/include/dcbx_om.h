/*-----------------------------------------------------------------------------
 * Module Name: dcbx_om.h
 *-----------------------------------------------------------------------------
 * PURPOSE: Definitions for the DCBX object manager
 *-----------------------------------------------------------------------------
 * NOTES:
 *
 *-----------------------------------------------------------------------------
 * HISTORY:
 *    9/20/2012 - Ricky Lin, Created
 *
 *
 *-----------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2012
 *-----------------------------------------------------------------------------
 */

#ifndef DCBX_OM_H
#define DCBX_OM_H


/* INCLUDE FILE DECLARATIONS
 */

#include "sys_type.h"
#include "l_sort_lst.h"
#include "sysfun.h"
#include "dcbx_type.h"


/* NAMING CONSTANT DECLARATIONS
 */


/* command used in IPC message
 */


/* MACRO FUNCTION DECLARATIONS
 */

/* Macro function for computation of IPC msg_buf size based on field name
 * used in VLAN_OM_IpcMsg_T.data
 */


/* DATA TYPE DECLARATIONS
 */

/* IPC message structure
 */


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_OM_SetPortStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : Set port status
 * INPUT    : lport
 *            port_status
 * OUTPUT   : None
 * RETUEN   : DCBX_TYPE_RETURN_OK/DCBX_TYPE_RETURN_ERROR
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
UI32_T DCBX_OM_SetPortStatus(UI32_T lport, BOOL_T port_status);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_OM_GetPortStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : Get port status
 * INPUT    : lport
 * OUTPUT   : port_status_p
 * RETUEN   : DCBX_TYPE_RETURN_OK/DCBX_TYPE_RETURN_ERROR
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
UI32_T DCBX_OM_GetPortStatus(UI32_T lport, BOOL_T *port_status_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_OM_GetRunningPortStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : Get running configuration
 * INPUT    : lport
 * OUTPUT   : port_status
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T DCBX_OM_GetRunningPortStatus(UI32_T lport, BOOL_T *port_status);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_OM_SetPortMode
 *-------------------------------------------------------------------------
 * PURPOSE  : Set port mode
 * INPUT    : lport
 *            port_mode
 * OUTPUT   : None
 * RETUEN   : DCBX_TYPE_RETURN_OK/DCBX_TYPE_RETURN_ERROR
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
UI32_T DCBX_OM_SetPortMode(UI32_T lport, UI32_T port_mode);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_OM_GetPortStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : Get port mode
 * INPUT    : lport
 * OUTPUT   : port_mode_p
 * RETUEN   : DCBX_TYPE_RETURN_OK/DCBX_TYPE_RETURN_ERROR
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
UI32_T DCBX_OM_GetPortMode(UI32_T lport, UI32_T *port_mode_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DCBX_OM_GetRunningPortMode
 *-------------------------------------------------------------------------
 * PURPOSE  : Get running configuration
 * INPUT    : lport
 * OUTPUT   : port_mode
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T DCBX_OM_GetRunningPortMode(UI32_T lport, UI32_T *port_mode);

#endif /* #ifndef DCBX_OM_H */
