#ifndef IF_PMGR_H
#define IF_PMGR_H
#include "if_mgr.h"
/*------------------------------------------------------------------------------
 * ROUTINE NAME : TACACS_PMGR_Init
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Do initialization procedures for TACACS_PMGR.
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    None.
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
void IF_PMGR_Init(void);


/* FUNCTION NAME: IF_PMGR_GetIfNumber
 * PURPOSE: This funtion returns the number of network interfaces
 *          (regardless of their current state) present on this system.
 * INPUT:  None.
 * OUTPUT: if_number        - the total number of network interfaces presented on the system
 * RETURN: TRUE/FALSE
 * NOTES: For those interfaces which are not installed shall not be count into this number.
 */
BOOL_T IF_PMGR_GetIfNumber(UI32_T *if_number);

/* FUNCTION NAME: IF_PMGR_GetIfNumber
 * PURPOSE: This funtion returns the value of sysUpTime at the time of the
 *          last creation or deletion of an entry in the ifTable. If the number of
 *          entries has been unchanged since the last re-initialization
 *          of the local network management subsystem, then this object
 *          contains a zero value.
 * INPUT:  None.
 * OUTPUT: if_table_last_change_time
 * RETURN: TRUE/FALSE
 * NOTES: None.
 */
BOOL_T IF_PMGR_GetIfTableLastChange (UI32_T *if_table_last_change_time);

/* FUNCTION NAME: IF_PMGR_GetIfEntry
 * PURPOSE: This funtion returns true if the specified interface entry info
 *          can be successfully retrieved. Otherwise, false is returned.
 * INPUT:   if_entry->if_index      - key to specify a unique interface entry
 * OUTPUT:  if_entry                - interface entry info of specified if_index
 * RETURN:  TRUE/FALSE
 * NOTES:   None.
 */
BOOL_T IF_PMGR_GetIfEntry (IF_MGR_IfEntry_T  *if_entry);

/* FUNCTION NAME: IF_PMGR_GetNextIfEntry
 * PURPOSE: This funtion returns true if the next available interface entry info
 *          can be successfully retrieved. Otherwise, false is returned.
 * INPUT:   if_entry->if_index      - key to specify a unique interface entry
 * OUTPUT:  if_entry                - interface entry info of specified if_index
 * RETURN:  TRUE/FALSE
 * NOTES:   If next available interface entry is available, the if_entry->if_index
 *          will be updated and the entry info will be retrieved from the table.
 */
BOOL_T IF_PMGR_GetNextIfEntry (IF_MGR_IfEntry_T  *if_entry);

/* FUNCTION NAME: IF_PMGR_SetIfAdminStatus
 * PURPOSE: This funtion returns true if desired/new admin status is successfully set to
 *          the specified interface. Otherwise, false is returned.
 * INPUT:  if_index         - key to specify a unique interface
 *         if_admin_status  - VAL_ifAdminStatus_up / VAL_ifAdminStatus_down /
 *                            VAL_ifAdminStatus_testing
 * OUTPUT: None.
 * RETURN: TRUE/FALSE
 * NOTES: 1. The testing(3) state indicates that no operational packets can be passed.
 *        2. When a managed system initializes, all interfaces start with ifAdminStatus
 *           in the down(2) state.
 *        3. As a result of either explicit management action or per configuration
 *           information retained by the managed system, ifAdminStatus is then changed
 *           to either the up(1) or  testing(3) states (or remains in the down(2) state).
 */
BOOL_T IF_PMGR_SetIfAdminStatus (UI32_T if_index, UI32_T if_admin_status);

/* FUNCTION NAME: IF_PMGR_GetIfXEntry
 * PURPOSE: This funtion returns true if the specified extension interface entry info
 *          can be successfully retrieved. Otherwise, false is returned.
 * INPUT:   if_x_entry->if_index    - key to specify a unique extension interface entry
 * OUTPUT:  if_entry                - extension interface entry info of specified if_index
 * RETURN:  TRUE/FALSE
 * NOTES:   None.
 */
BOOL_T IF_PMGR_GetIfXEntry (IF_MGR_IfXEntry_T  *if_x_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME: IF_PMGR_GetNextIfXEntry
 * ------------------------------------------------------------------------
 * PURPOSE: This funtion returns true if the next available extension interface entry info
 *          can be successfully retrieved. Otherwise, false is returned.
 * INPUT:   if_x_entry->if_index    - key to specify a unique extension interface entry
 * OUTPUT:  if_x_entry              - extension interface entry info of specified if_index
 * RETURN:  TRUE/FALSE
 * NOTES:   If next available extension interface entry is available, the if_x_entry->if_index
 *          will be updated and the entry info will be retrieved from the table.
 * ------------------------------------------------------------------------
 */
BOOL_T IF_PMGR_GetNextIfXEntry (IF_MGR_IfXEntry_T  *if_x_entry);

/* ------------------------------------------------------------------------
 * FUNCTION NAME: IF_PMGR_SetIfLinkUpDownTrapEnableGlobal
 * ------------------------------------------------------------------------
 * PURPOSE: Set trap status to all interfaces.
 * INPUT:   trap_status -- trap status
 * OUTPUT:  None
 * RETURN:  TRUE/FALSE
 * NOTES:   None
 * ------------------------------------------------------------------------
 */
BOOL_T IF_PMGR_SetIfLinkUpDownTrapEnableGlobal(
    UI32_T trap_status
);

/* ------------------------------------------------------------------------
 * FUNCTION NAME: IF_PMGR_SetIfLinkUpDownTrapEnable
 * ------------------------------------------------------------------------
 * PURPOSE: This funtion returns true if the trap status of the specific interface
 *          can be set successfully.  Otherwise, return FALSE.
 * INPUT:   if_x_index  -- key to specify which index to configured.
 *          trap_status -- trap status
 *                        VAL_ifLinkUpDownTrapEnable_enabled  (1)
 *                        VAL_ifLinkUpDownTrapEnable_disabled (2)
 * OUTPUT:  none
 * RETURN:  TRUE/FALSE
 * NOTES:   none
 * ------------------------------------------------------------------------
 */
BOOL_T IF_PMGR_SetIfLinkUpDownTrapEnable(UI32_T if_x_index, UI32_T trap_status);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - IF_PMGR_GetRunningIfLinkUpDownTrapEnable
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the global LinkUpDownTrapEnable status.
 * INPUT    :   if_x_index              -- the specified IfIndex
 * OUTPUT   :   UI32_T *trap_status     -- pointer of the status value
 *                      VAL_ifLinkUpDownTrapEnable_enabled  (1)
 *                      VAL_ifLinkUpDownTrapEnable_disabled (2)
 *
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * REF      :   None
 * ------------------------------------------------------------------------
 */
UI32_T  IF_PMGR_GetRunningIfLinkUpDownTrapEnable(UI32_T if_x_index, UI32_T *trap_status);

/* FUNCTION NAME: IF_PMGR_SetIfAlias
 * PURPOSE: This funtion returns true if the if_alias of the specific interface
 *          can be set successfully.  Otherwise, return FALSE.
 * INPUT:   if_x_index - key to specify which index to configured.
 *			if_alias - the read/write name of the specific interface
 * OUTPUT:  none
 * RETURN:  TRUE/FALSE
 * NOTES:   none
 */
BOOL_T IF_PMGR_SetIfAlias(UI32_T if_x_index, UI8_T *if_alias);

/* FUNCTION NAME: IF_PMGR_GetIfStackEntry
 * PURPOSE: This funtion returns true if next available interface stack entry info
 *          can be successfully retrieved. Otherwise, false is returned.
 * INPUT:  if_stack_entry->if_stack_higher_layer_if_index   - primary key to specify
 *                                                          - a unique interface stack entry
 *         if_stack_entry->if_stack_lower_layer_if_index    - secondary key
 * OUTPUT: if_stack_entry                   - next available interface stack entry info
 * RETURN: TRUE/FALSE
 * NOTES: 1. The write-access to this object will not be supported by this device.
 *        2. Changing the value of this object from 'active' to 'notInService' or 'destroy'
 *           will likely have consequences up and down the interface stack.
 *           Thus, write access to this object is likely to be inappropriate for some types of
 *           interfaces, and many implementations will choose not to support write-access for
 *           any type of interface.
 */
BOOL_T IF_PMGR_GetIfStackEntry(IF_MGR_IfStackEntry_T *if_stack_entry);

/* FUNCTION NAME: IF_PMGR_GetNextIfStackEntry
 * PURPOSE: This funtion returns true if next available interface stack entry info
 *          can be successfully retrieved. Otherwise, false is returned.
 * INPUT:  if_stack_entry->if_stack_higher_layer_if_index   - primary key to specify
 *                                                          - a unique interface stack entry
 *         if_stack_entry->if_stack_lower_layer_if_index    - secondary key
 * OUTPUT: if_stack_entry                   - next available interface stack entry info
 * RETURN: TRUE/FALSE
 * NOTES: 1. The write-access to this object will not be supported by this device.
 *        2. Changing the value of this object from 'active' to 'notInService' or 'destroy'
 *           will likely have consequences up and down the interface stack.
 *           Thus, write access to this object is likely to be inappropriate for some types of
 *           interfaces, and many implementations will choose not to support write-access for
 *           any type of interface.
 */
BOOL_T IF_PMGR_GetNextIfStackEntry(IF_MGR_IfStackEntry_T *if_stack_entry);

/* FUNCTION NAME: IF_PMGR_GetIfStackLastChange
 * PURPOSE: This funtion returns true if the last update time of specified interface stack
 *          can be successfully retrieved. Otherwise, false is returned.
 * INPUT:   none
 * OUTPUT:  if_stack_last_change_time - The value of sysUpTime at the time of the last change of
 *                                      the (whole) interface stack.
 * RETURN: TRUE/FALSE
 * NOTES:  A change of the interface stack is defined to be any creation, deletion, or change in
 *         value of any instance of ifStackStatus.  If the interface stack has been unchanged
 *         since the last re-initialization of the local network management subsystem, then
 *         this object contains a zero value.
 */
BOOL_T IF_PMGR_GetIfStackLastChange (UI32_T *if_stack_last_change_time);

/* FUNCTION NAME: IF_PMGR_IfnameToIfindex
 * PURPOSE: This function returns true if the given ifname has a corresponding ifindex existed
 *          in the system.  Otherwise, returns false.
 * INPUT:   ifname - a read-only name for each interface defined during intialization
 * OUTPUT:  ifindex - corresponding interface index for the specific name.
 * RETURN:  TRUE/FALSE
 * NOTES:   None
 */
BOOL_T IF_PMGR_IfnameToIfindex (UI8_T *ifname, UI32_T *ifindex);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - IF_PMGR_GetIfType
 *------------------------------------------------------------------------------
 * PURPOSE  : This function determines the type of interface based on ifindex
 *            value.
 * INPUT    : ifindex -- interface index
 * OUTPUT   : iftype -- type of interface based on ifindex
 *                      (IF_MGR_NORMAL_IFINDEX,
 *                       IF_MGR_TRUNK_IFINDEX,
 *                       IF_MGR_RS232_IFINDEX,
 *                       IF_MGR_VLAN_IFINDEX,
 *                       IF_MGR_ERROR_IFINDEX)
 * RETURN   : TRUE/FALSE
 * NOTES    : None
 *------------------------------------------------------------------------------
 */
BOOL_T  IF_PMGR_GetIfType(UI32_T ifindex, UI32_T *iftype);

#endif /*end of IF_PMGR_H*/
