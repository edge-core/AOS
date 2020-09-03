/*-----------------------------------------------------------------------------
 * MODULE NAME: VLAN_OM_PRIVATE.H
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *    The declarations of VLAN OM which are only used by VLAN.
 *
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    2007/05/26     --- Timon, Create
 *
 * Copyright(C)      Accton Corporation, 2007
 *-----------------------------------------------------------------------------
 */

#ifndef VLAN_OM_PRIVATE_H
#define VLAN_OM_PRIVATE_H


/* INCLUDE FILE DECLARATIONS
 */

#include "sys_type.h"
#include "vlan_om.h"


/* NAMING CONSTANT DECLARATIONS
 */

#define VLAN_MGR_DOT1Q_DEFAULT_PVID         SYS_DFLT_1Q_PORT_VLAN_PVID


/* MACRO FUNCTION DECLARATIONS
 */

/*-------------------------------------------------------------------------
 * MACRO NAME - VLAN_OM_IS_MEMBER
 *-------------------------------------------------------------------------
 * PURPOSE  : Verify whether the port is in port list
 * INPUT    : port_list -- bit map
 *            port      -- ifindex
 * OUTPUT   : NONE
 * RETURN   : TRUE/FALSE
 * NOTE     : NONE
 *-------------------------------------------------------------------------
 */
#define VLAN_OM_IS_MEMBER(port_list, port)  (((port_list[((port) - 1) >> 3]) & (1 << (7 - (((port) - 1) & 7)))) != 0)

#define VLAN_OM_ADD_MEMBER(port_list, port) port_list[((port)-1) >> 3] |= (1 << (7 - (((port)-1) & 7)))
#define VLAN_OM_DEL_MEMBER(port_list, port) port_list[((port)-1) >> 3] &= (~(1 << (7 - (((port)-1) & 7))))


/* DATA TYPE DECLARATIONS
 */


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_Init
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function initialize vlan_om.  This will allocate memory for
 *            vlan table and create linklist to manage vlan table.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *-----------------------------------------------------------------------------
 */
void VLAN_OM_Init(void);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_ClearDatabase
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function is clears all entry in vlan table and vlan port table.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : 1. This function shall be invoked when system enters transition mode.
 *            2. All the entries in database will be purged.
 *-----------------------------------------------------------------------------
 */
void VLAN_OM_ClearDatabase(void);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_DeleteVlanEntry
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function returns true if vlan is succussfully deleted.
 *            Otherwise, returns false.
 * INPUT    : vid -- specify which vlan to be deleted.
 * OUTPUT   : NONE
 * RETURN   : TRUE/FALSE
 * NOTES    : none
 *-----------------------------------------------------------------------------
 */
BOOL_T VLAN_OM_DeleteVlanEntry(UI32_T vid);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_SetVlanEntry
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function returns true if vlan entry has been modified
 *            successfully. otherwise, false is return.
 * INPUT    : vlan info
 * OUTPUT   : NONE
 * RETURN   : TRUE / FALSE.
 * NOTES    : 1. If the specified entry does not exist, a new entry will be created.
 *            2. VLAN_MGR shall use this function to create a new VLAN entry.
 *-----------------------------------------------------------------------------
 */
BOOL_T VLAN_OM_SetVlanEntry(VLAN_OM_Dot1qVlanCurrentEntry_T *vlan_info);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_DeleteVlanPortEntry
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the vlan port entry has been deleted.
 *            otherwise, returns false;
 * INPUT    : lport_ifindex -- specify which port information to be deleted
 * OUTPUT   : none
 * RETURN   : TRUE / fALSE
 * NOTES    : none
 *-----------------------------------------------------------------------------
 */
BOOL_T VLAN_OM_DeleteVlanPortEntry(UI32_T lport_ifindex);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_SetVlanPortEntry
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the new VLAN port info is successfully
 *            updated to the specified entry. Otherwise, false is returned.
 * INPUT    : *vlan_port_info -- The specific vlan port info to be modified
 * OUTPUT   : none
 * RETURN   : TRUE \ FALSE
 * NOTES    : 1. If the specified entry does not exist, a new entry will be created.
 *            2. VLAN_MGR shall use this function to create a new VLAN port entry.
 *-----------------------------------------------------------------------------
 */
BOOL_T VLAN_OM_SetVlanPortEntry(VLAN_OM_Vlan_Port_Info_T *vlan_port_info);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_SetGlobalGvrpStatus
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the gvrp status for the device
 *            can be set successfully.  Otherwise, return false.
 * INPUT    : gvrp_status - VAL_dot1qGvrpStatus_enabled \
 *                          VAL_dot1qGvrpStatus_disabled
 * OUTPUT   : None
 * RETURN   : TRUE \ FALSE
 * NOTES    : The administrative status requested by management for GVRP
 *--------------------------------------------------------------------------*/
BOOL_T VLAN_OM_SetGlobalGvrpStatus(UI32_T gvrp_status);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_SetDot1qConstraintTypeDefault
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the constrain type for this device
 *            can be set successfully.  Otherwise, return false.
 * INPUT    : VAL_dot1qConstraintTypeDefault_independent\
 *            VAL_dot1qConstraintTypeDefault_shared
 * OUTPUT   : none
 * RETURN   : TRUE \ FALSE
 * NOTES    : System_default is IVL.
 *--------------------------------------------------------------------------*/
BOOL_T VLAN_OM_SetDot1qConstraintTypeDefault(UI32_T constrain_type);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_SetGlobalDefaultVlan
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the default VLAN for this device
 *            can be set successfully.  Otherwise, return false.
 * INPUT    : vid
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void VLAN_OM_SetGlobalDefaultVlan(UI32_T vid);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_SetManagementVlanId
 *-----------------------------------------------------------------------------
 * PURPOSE : Set the global management VLAN
 * INPUT   : vid - management vlan id
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 *-----------------------------------------------------------------------------
 */
void VLAN_OM_SetManagementVlanId(UI32_T vid);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetManagementVlanId
 *-----------------------------------------------------------------------------
 * PURPOSE : Get the global management VLAN
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : management vlan id
 * NOTES   : None
 *-----------------------------------------------------------------------------
 */
UI32_T VLAN_OM_GetManagementVlanId(void);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetIfTableLastUpdateTimePtr
 *-----------------------------------------------------------------------------
 * PURPOSE  : Get the pointer to the last time when vlan_om is updated by management
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : pointer to Time tick
 * NOTES    : None
 *-----------------------------------------------------------------------------
 */
UI32_T* VLAN_OM_GetIfTableLastUpdateTimePtr();

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_SetIfStackLastUpdateTime
 *-----------------------------------------------------------------------------
 * PURPOSE  : Set the value of the last time when vlan egress port list is modified
 * INPUT    : time_mark - the time value
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 *-----------------------------------------------------------------------------
 */
void VLAN_OM_SetIfStackLastUpdateTime(UI32_T time_mark);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetIfStackLastUpdateTimePtr
 *-----------------------------------------------------------------------------
 * PURPOSE  : Get the pointer to the last time when vlan egress port list is modified
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : pointer to Time tick
 * NOTES    : None
 *-----------------------------------------------------------------------------
 */
UI32_T* VLAN_OM_GetIfStackLastUpdateTimePtr();

/*-------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_EnterCriticalSection
 * ------------------------------------------------------------------------
 * PURPOSE  :   Enter critical section before a task invokes the spanning
 *              tree objects.
 * INPUT    :   None
 * OUTPUT   :   None
 * RETURN   :   None
 * NOTE     :   Utilize SYSFUN_OM_ENTER_CRITICAL_SECTION().
 *-------------------------------------------------------------------------
 */
void VLAN_OM_EnterCriticalSection(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_LeaveCriticalSection
 * ------------------------------------------------------------------------
 * PURPOSE  :   Leave critical section after a task invokes the spanning
 *              tree objects.
 * INPUT    :   None
 * OUTPUT   :   None
 * RETURN   :   None
 * NOTE     :   Utilize SYSFUN_OM_LEAVE_CRITICAL_SECTION().
 *-------------------------------------------------------------------------
 */
void VLAN_OM_LeaveCriticalSection(void);


#endif /* #ifndef VLAN_OM_PRIVATE_H */
