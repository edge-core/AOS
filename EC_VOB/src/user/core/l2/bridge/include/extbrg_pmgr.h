/*-----------------------------------------------------------------------------
 * FILE NAME: EXTBRG_PMGR.H
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *    This file declares the APIs for EXTBRG MGR IPC.
 *
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    2007/07/24     --- Timon, Create
 *
 * Copyright(C)      Accton Corporation, 2007
 *-----------------------------------------------------------------------------
 */

#ifndef EXTBRG_PMGR_H
#define EXTBRG_PMGR_H


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
 * ROUTINE NAME : EXTBRG_PMGR_InitiateProcessResource
 *-----------------------------------------------------------------------------
 * PURPOSE : Initiate resource for EXTBRG_PMGR in the calling process.
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
BOOL_T EXTBRG_PMGR_InitiateProcessResource(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - EXTBRG_PMGR_GetDot1dDeviceCapabilities
 *--------------------------------------------------------------------------
 * Purpose: This procedure returns the extended capability of this bridge device.
 * Input:   None.
 * Output:  bridge_device_capability - capabilities defined as a bit map
 * Return:  TRUE/FALSE.
 * Note:    1. Please refer to the naming constant declarations for detailed bit map
 *             definition of bridge_device_capability.
 *          2. When bit is set to '1', the corresponding capability is provided.
 *--------------------------------------------------------------------------
 */
BOOL_T  EXTBRG_PMGR_GetDot1dDeviceCapabilities(UI32_T *bridge_device_capability);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - EXTBRG_PMGR_GetDot1dPortCapabilities
 *--------------------------------------------------------------------------
 * Purpose: This function returns the extended capability of a given bridge port.
 * Input:   lport_ifindex.
 * Output:  bridge_port_capability - capabilities defined as a bit map
 * Return:  TRUE/FALSE.
 * Note:    1. Please refer to the naming constant declarations for detailed bit map
 *             definition of bridge_port_capability.
 *          2. When bit is set to '1', the corresponding capability is provided.
 *--------------------------------------------------------------------------
 */
BOOL_T  EXTBRG_PMGR_GetDot1dPortCapabilities(UI32_T lport_ifindex, UI32_T *bridge_port_capability);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - EXTBRG_PMGR_GetNextDot1dPortCapabilities
 *--------------------------------------------------------------------------
 * Purpose: This function returns the extended capability of next available bridge port.
 * Input:   lport_ifindex.
 * Output:  lport_ifindex.
 *          bridge_port_capability      -- capabilities defined as a bit map
 * Return:  TRUE/FALSE.
 * Note:    1. Please refer to the naming constant declarations for detailed bit map
 *             definition of bridge_port_capability.
 *          2. When bit is set to '1', the corresponding capability is provided.
 *--------------------------------------------------------------------------
 */
BOOL_T  EXTBRG_PMGR_GetNextDot1dPortCapabilities(UI32_T *linear_port_id, UI32_T *bridge_port_capability);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - EXTBRG_PMGR_GetDot1dGmrpStatus
 *--------------------------------------------------------------------------
 * Purpose: This procedure returns the GMRP operation status of bridge device.
 * Input:   None.
 * Output:  gmrp_status - VAL_dot1dGmrpStatus_enabled \
 *                        VAL_dot1dGmrpStatus_disabled
 * Return:  TRUE/FALSE.
 * Note:    Please refer to the naming constant declarations for definition of gmrp_status.
 *--------------------------------------------------------------------------
 */
BOOL_T  EXTBRG_PMGR_GetDot1dGmrpStatus(UI32_T *gmrp_status);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - EXTBRG_PMGR_SetDot1dGmrpStatus
 *--------------------------------------------------------------------------
 * Purpose: This procedure set the GMRP operation status to a given value.
 * Input:   gmrp_status - VAL_dot1dGmrpStatus_enabled \
 *                        VAL_dot1dGmrpStatus_disabled
 * Output:  None.
 * Return:  TRUE/FALSE.
 * Note:    Please refer to the naming constant declarations for definition of gmrp_status.
 *--------------------------------------------------------------------------
 */
BOOL_T  EXTBRG_PMGR_SetDot1dGmrpStatus(UI32_T gmrp_status);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - EXTBRG_PMGR_GetDot1dTrafficClassesEnabled
 *--------------------------------------------------------------------------
 * Purpose: This procedure returns the traffic classes operation status of bridge device.
 * Input: None.
 * Output: traffic_class_status  - VAL_dot1dTrafficClassesEnabled_true \
 *                                 VAL_dot1dTrafficClassesEnabled_false
 * Return: TRUE/FALSE.
 * Note: Please refer to the naming constant declarations for definition of traffic_class_status.
 *--------------------------------------------------------------------------
 */
BOOL_T  EXTBRG_PMGR_GetDot1dTrafficClassesEnabled(UI32_T *traffic_class_status);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - EXTBRG_PMGR_SetDot1dTrafficClassesEnabled
 *--------------------------------------------------------------------------
 * Purpose: This procedure enables the traffic class operation on the bridge.
 * Input:   traffic_class_status - VAL_dot1dTrafficClassesEnabled_true \
 *                                 VAL_dot1dTrafficClassesEnabled_false
 * Output:  None.
 * Return:  TRUE/FALSE.
 * Note:    Please refer to the naming constant declarations for definition of traffic_class_status.
 *--------------------------------------------------------------------------
 */
BOOL_T  EXTBRG_PMGR_SetDot1dTrafficClassesEnabled(UI32_T traffic_class_status);


#endif /* #ifndef EXTBRG_PMGR_H */
