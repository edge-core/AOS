/*-----------------------------------------------------------------------------
 * Module   : netaccess_pmgr.h
 *-----------------------------------------------------------------------------
 * PURPOSE  : Provide APIs to access NETACCESS.
 *-----------------------------------------------------------------------------
 * NOTES    :
 *
 *-----------------------------------------------------------------------------
 * HISTORY  : 07/23/2007 - Wakka Tu, Create
 *
 *-----------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2007
 *-----------------------------------------------------------------------------
 */

#ifndef NETACCESS_PMGR_H
#define NETACCESS_PMGR_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_cpnt.h"
#if (SYS_CPNT_NETACCESS == TRUE)
#include "netaccess_mgr.h"

/* NAME CONSTANT DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_PMGR_InitiateProcessResources
 *-------------------------------------------------------------------------
 * PURPOSE: Initiate resource used in the calling process.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : TRUE  - success
 *          FALSE - failure
 * NOTE   : None
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_InitiateProcessResources(void);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_PMGR_GetSecurePortMode
 * ---------------------------------------------------------------------
 * PURPOSE: Get security modes of the port
 * INPUT  : lport
 * OUTPUT : secure_port_mode.
 * RETURN : TRUE/FALSE.
 * NOTES  : please reference NETACCESS_PortMode_T for secure_port_mode
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetSecurePortMode(UI32_T lport, NETACCESS_PortMode_T *secure_port_mode);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_PMGR_SetDot1xConfigSettingToDefault
 *-------------------------------------------------------------------------
 * PURPOSE: This function will set default value of 1X configuration
 * INPUT  : none
 * OUTPUT : none
 * RETURN : TRUE -- success, FALSE -- fail
 * NOTES  : [to replace DOT1X_MGR_SetConfigSettingToDefault]
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_SetDot1xConfigSettingToDefault(void);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_PMGR_ClearSecureAddressEntryByFilter
 * ---------------------------------------------------------------------
 * PURPOSE: clear secure mac address table entry by filter
 * INPUT  : in_filter.
 * OUTPUT : None.
 * RETURN : TRUE/FALSE.
 * NOTES  : None
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_ClearSecureAddressEntryByFilter(
    NETACCESS_MGR_SecureAddressFilter_T *in_filter);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_PMGR_GetMacAuthPortEntry
 *-------------------------------------------------------------------------
 * PURPOSE: Get the port entry of mac-authentication.
 * INPUT  : field_id     : field id.
 *          *key         : input key
 *          buffer_size  : buffer size
 * OUTPUT : *buffer      : The secure port entry.
 *          *used_buffer : used buffer size for output
 * RETURN : FALSE : error, TRUE : success
 * NOTES  : 1.used_buffer need to include '\0' length,ex:str[10] = "123",
 *            so used_buffer = 4
 *          2.buffer_size for available buffer_size, used_buffer for used buffer_size
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetMacAuthPortEntry(
    UI32_T field_id, void *key, void *buffer, UI32_T buffer_size, UI32_T *used_buffer);

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_PMGR_GetNextMacAuthPortEntry
 *-----------------------------------------------------------------------------------
 * PURPOSE: Get the next port entry of mac-authentication.
 * INPUT  : field_id     : field id.
 *          *key         : input key
 *          buffer_size  : buffer size
 * OUTPUT : *buffer      : The secure port entry.
 *          *used_buffer : used buffer size for output
 * RETURN : FALSE : error, TRUE : success
 * NOTES  : 1.used_buffer need to include '\0' length,ex:str[10] = "123",
 *            so used_buffer = 4
 *          2.buffer_size for available buffer_size, used_buffer for used buffer_size
 *-----------------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetNextMacAuthPortEntry(
    UI32_T field_id, void *key, void *buffer, UI32_T buffer_size, UI32_T *used_buffer);

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_PMGR_GetNextSecureAddressEntry
 *-----------------------------------------------------------------------------------
 * PURPOSE: This function will get next secure address entry by the lport and the mac_address.
 * INPUT  : field_id     : field id.
 *          *key         : input key
 *          buffer_size  : buffer size
 * OUTPUT : *buffer      : The secure port entry.
 *          *used_buffer : used buffer size for output
 * RETURN : negative integers : error, 0 : success
 * NOTES  : 1.used_buffer need to include '\0' length,ex:str[10] = "123",
 *            so used_buffer = 4
 *          2.buffer_size for available buffer_size, used_buffer for used buffer_size
 *          3.key must be NETACCESS_MGR_SecureAddressEntryKey_T
 *-----------------------------------------------------------------------------------
 */
I32_T NETACCESS_PMGR_GetNextSecureAddressEntry(
        UI32_T field_id,
        NETACCESS_MGR_SecureAddressEntryKey_T *key,
        void *buffer,
        UI32_T buffer_size,
        UI32_T *used_buffer);

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_PMGR_GetNextSecureAddressEntryByFilter
 *-----------------------------------------------------------------------------------
 * PURPOSE: This function will get next secure address entry by filter.
 * INPUT  : field_id     : field id.
 *          *key         : input key
 *          buffer_size  : buffer size
 * OUTPUT : *buffer      : The secure port entry.
 *          *used_buffer : used buffer size for output
 * RETURN : negative integers : error, 0 : success
 * NOTES  : 1.used_buffer need to include '\0' length,ex:str[10] = "123",
 *            so used_buffer = 4
 *          2.buffer_size for available buffer_size, used_buffer for used buffer_size
 *          3.key must be NETACCESS_MGR_SecureAddressEntryKeyFilter_T
 *-----------------------------------------------------------------------------------
 */
I32_T NETACCESS_PMGR_GetNextSecureAddressEntryByFilter(
        UI32_T field_id,
        NETACCESS_MGR_SecureAddressEntryKeyFilter_T *key,
        void *buffer,
        UI32_T buffer_size,
        UI32_T *used_buffer);

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_PMGR_GetNextSecurePortEntry
 *-----------------------------------------------------------------------------------
 * PURPOSE: This function will get secure port entry by the unit and the port.
 * INPUT  : field_id     : field id.
 *          *key         : input key
 *          buffer_size  : buffer size
 * OUTPUT : *buffer      : The secure port entry.
 *          *used_buffer : used buffer size for output
 * RETURN : negative integers : error, 0 : success
 * NOTES  : 1.used_buffer need to include '\0' length,ex:str[10] = "123",
 *            so used_buffer = 4
 *          2.buffer_size for available buffer_size, used_buffer for used buffer_size
 *-----------------------------------------------------------------------------------
 */
I32_T NETACCESS_PMGR_GetNextSecurePortEntry(
    UI32_T field_id, void *key, void *buffer, UI32_T buffer_size, UI32_T *used_buffer);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_PMGR_GetRunningSecureReauthTime
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default reauth_time is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT  : None.
 * OUTPUT : reauth_time
 * RETURN : SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES  : 1. This function shall only be invoked by CLI to save the
 *             "running configuration" to local or remote files.
 *          2. Since only non-default configuration will be saved, this
 *             function shall return non-default system name.
 *          3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T NETACCESS_PMGR_GetRunningSecureReauthTime(UI32_T *reauth_time);

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_PMGR_GetSecureAddressEntry
 *-----------------------------------------------------------------------------------
 * PURPOSE: This function will get secure address entry entry by the unit,the port and the mac_address.
 * INPUT  : field_id     : field id.
 *          *key         : input key
 *          buffer_size  : buffer size
 * OUTPUT : *buffer      : The secure port entry.
 *          *used_buffer : used buffer size for output
 * RETURN : negative integers : error, 0 : success
 * NOTES  : 1.used_buffer need to include '\0' length,ex:str[10] = "123",
 *            so used_buffer = 4
 *          2.buffer_size for available buffer_size, used_buffer for used buffer_size
 *-----------------------------------------------------------------------------------
 */
I32_T NETACCESS_PMGR_GetSecureAddressEntry(
        UI32_T field_id,
        NETACCESS_MGR_SecureAddressEntryKey_T *key,
        void *buffer,
        UI32_T buffer_size,
        UI32_T *used_buffer);

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_PMGR_GetSecurePortEntry
 *-----------------------------------------------------------------------------------
 * PURPOSE: This function will get secure port entry by the unit and the port.
 * INPUT  : field_id     : field id.
 *          *key         : input key
 *          buffer_size  : buffer size
 * OUTPUT : *buffer      : The secure port entry.
 *          *used_buffer : used buffer size for output
 * RETURN : negative integers : error, 0 : success
 * NOTES  : 1.used_buffer need to include '\0' length,ex:str[10] = "123",
 *            so used_buffer = 4
 *          2.buffer_size for available buffer_size, used_buffer for used buffer_size
 *-----------------------------------------------------------------------------------
 */
I32_T NETACCESS_PMGR_GetSecurePortEntry(
    UI32_T field_id, void *key, void *buffer, UI32_T buffer_size, UI32_T *used_buffer);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_PMGR_GetSecureReauthTime
 * ---------------------------------------------------------------------
 * PURPOSE: Get the reauth time.
 * INPUT  : None.
 * OUTPUT : reauth_time.
 * RETURN : TRUE/FALSE.
 * NOTES  : Specifies the default session lifetime in seconds before
 *          a forwarding MAC address is re-authenticated.
 *          The default time is 1800 seconds.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetSecureReauthTime(UI32_T *reauth_time);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_PMGR_SetSecureReauthTime
 * ---------------------------------------------------------------------
 * PURPOSE: Set the reauth time.
 * INPUT  : reauth_time.
 * OUTPUT : None.
 * RETURN : TRUE/FALSE.
 * NOTES  : Specifies the reauth time in seconds before
 *          a forwarding MAC address is re-authenticated.
 *          The default time is 1800 seconds.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_SetSecureReauthTime(UI32_T reauth_time);

#if (SYS_CPNT_NETACCESS_AGING_MODE == SYS_CPNT_NETACCESS_AGING_MODE_CONFIGURABLE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_PMGR_GetMacAddressAgingMode
 *-------------------------------------------------------------------------
 * PURPOSE: Get the MAC address aging mode.
 * INPUT  : None.
 * OUTPUT : mode_p -- MAC address aging mode
 * RETURN : TRUE -- success, FALSE -- fail
 * NOTES  : Aging mode
 *          VAL_networkAccessAging_enabled for aging enabled
 *          VAL_networkAccessAging_disabled for aging disabled
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetMacAddressAgingMode(UI32_T *mode_p);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_PMGR_GetRunningMacAddressAgingMode
 * ---------------------------------------------------------------------
 * PURPOSE: Get the MAC address aging mode.
 * INPUT  : None.
 * OUTPUT : mode_p -- VAL_networkAccessAging_enabled,
 *                    VAL_networkAccessAging_disabled
 * RETURN : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- success
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- no change
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL      -- fail
 * NOTES  : None.
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T NETACCESS_PMGR_GetRunningMacAddressAgingMode(UI32_T *mode_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_PMGR_SetMacAddressAgingMode
 *-------------------------------------------------------------------------
 * PURPOSE: Set the MAC address aging mode.
 * INPUT  : mode -- MAC address aging mode
 * OUTPUT : None.
 * RETURN : TRUE -- success, FALSE -- fail
 * NOTES  : Aging mode
 *          VAL_networkAccessAging_enabled for aging enabled
 *          VAL_networkAccessAging_disabled for aging disabled
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_SetMacAddressAgingMode(UI32_T mode);

#endif /* #if (SYS_CPNT_NETACCESS_AGING_MODE == SYS_CPNT_NETACCESS_AGING_MODE_CONFIGURABLE) */

#if(SYS_CPNT_NETACCESS_DYNAMIC_VLAN == TRUE)
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_PMGR_GetRunningSecureDynamicVlanStatus
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default dynamic_vlan_status is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT  : lport.
 * OUTPUT : dynamic_vlan_status
 * RETURN : SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES  : 1. This function shall only be invoked by CLI to save the
 *             "running configuration" to local or remote files.
 *          2. Since only non-default configuration will be saved, this
 *             function shall return non-default system name.
 *          3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T NETACCESS_PMGR_GetRunningSecureDynamicVlanStatus(
    UI32_T lport, UI32_T *dynamic_vlan_status);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_PMGR_GetSecureDynamicVlanStatus
 * ---------------------------------------------------------------------
 * PURPOSE: Get dynamic VLAN configuration control
 * INPUT  : lport.
 * OUTPUT : dynamic_vlan_status
 * RETURN : TRUE/FALSE.
 * NOTES  : The user-based port configuration control. Setting this attribute
 *          TRUE causes the port to be configured with any configuration
 *          parameters supplied by the authentication server. Setting this
 *          attribute to FALSE causes any configuration parameters supplied
 *          by the authentication server to be ignored.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetSecureDynamicVlanStatus(UI32_T lport, UI32_T *dynamic_vlan_status);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_PMGR_SetSecureDynamicVlanStatus
 * ---------------------------------------------------------------------
 * PURPOSE: Set dynamic VLAN configuration control
 * INPUT  : lport, dynamic_vlan_status
 * OUTPUT : None.
 * RETURN : TRUE/FALSE.
 * NOTES  : The user-based port configuration control. Setting this attribute
 *          TRUE causes the port to be configured with any configuration
 *          parameters supplied by the authentication server. Setting this
 *          attribute to FALSE causes any configuration parameters supplied
 *          by the authentication server to be ignored.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_SetSecureDynamicVlanStatus(UI32_T lport, UI32_T dynamic_vlan_status);

#endif /* #if(SYS_CPNT_NETACCESS_DYNAMIC_VLAN == TRUE) */

#if(SYS_CPNT_NETACCESS_DYNAMIC_QOS == TRUE)
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_PMGR_GetRunningDynamicQosStatus
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default dynamic_vlan_status is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT  : lport.
 * OUTPUT : dynamic_vlan_status
 * RETURN : SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES  : 1. This function shall only be invoked by CLI to save the
 *             "running configuration" to local or remote files.
 *          2. Since only non-default configuration will be saved, this
 *             function shall return non-default system name.
 *          3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T NETACCESS_PMGR_GetRunningDynamicQosStatus(
    UI32_T lport, UI32_T *dynamic_qos_status);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_PMGR_GetDynamicQosStatus
 * ---------------------------------------------------------------------
 * PURPOSE: Get dynamic QoS configuration control
 * INPUT  : lport.
 * OUTPUT : dynamic_vlan_status
 * RETURN : TRUE/FALSE.
 * NOTES  : The user-based port configuration control. Setting this attribute
 *          TRUE causes the port to be configured with any configuration
 *          parameters supplied by the authentication server. Setting this
 *          attribute to FALSE causes any configuration parameters supplied
 *          by the authentication server to be ignored.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetDynamicQosStatus(UI32_T lport, UI32_T *dynamic_qos_status);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_PMGR_SetDynamicQosStatus
 * ---------------------------------------------------------------------
 * PURPOSE: Set dynamic VLAN configuration control
 * INPUT  : lport, dynamic_vlan_status
 * OUTPUT : None.
 * RETURN : TRUE/FALSE.
 * NOTES  : The user-based port configuration control. Setting this attribute
 *          TRUE causes the port to be configured with any configuration
 *          parameters supplied by the authentication server. Setting this
 *          attribute to FALSE causes any configuration parameters supplied
 *          by the authentication server to be ignored.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_SetDynamicQosStatus(UI32_T lport, UI32_T dynamic_qos_status);

#endif /* #if(SYS_CPNT_NETACCESS_DYNAMIC_QOS == TRUE) */


#if(SYS_CPNT_NETACCESS_GUEST_VLAN == TRUE)
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_PMGR_GetRunningDot1xPortIntrusionAction
 * ---------------------------------------------------------------------
 * PURPOSE: Get the intrusion action to determine the action if an unauthorized device
 *          transmits on this port.
 * INPUT  : lport
 * OUTPUT : action_status -- VAL_dot1xAuthConfigExtPortIntrusionAction_block_traffic,
 *                           VAL_dot1xAuthConfigExtPortIntrusionAction_guest_vlan
 * RETURN : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- success
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- no change
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL      -- fail
 * NOTES  : none.
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T NETACCESS_PMGR_GetRunningDot1xPortIntrusionAction(
    UI32_T lport, UI32_T *action_status);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_PMGR_GetRunningSecureGuestVlanId
 * ---------------------------------------------------------------------
 * PURPOSE: Get per-port guest VLAN ID.
 * INPUT  : lport.
 * OUTPUT : vid.
 * RETURN : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- success
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- no change
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL      -- fail
 * NOTES  : None.
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T NETACCESS_PMGR_GetRunningSecureGuestVlanId(
    UI32_T lport, UI32_T *vid);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_PMGR_GetDot1xPortIntrusionAction
 * ---------------------------------------------------------------------
 * PURPOSE: Get the intrusion action to determine the action if an unauthorized device
 *          transmits on this port.
 * INPUT  : lport
 * OUTPUT : action_status -- VAL_dot1xAuthConfigExtPortIntrusionAction_block_traffic,
 *                           VAL_dot1xAuthConfigExtPortIntrusionAction_guest_vlan
 * RETURN : TRUE/FALSE.
 * NOTES  : none.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetDot1xPortIntrusionAction(UI32_T lport, UI32_T *action_status);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_PMGR_GetNextDot1xPortIntrusionAction
 * ---------------------------------------------------------------------
 * PURPOSE: Get next port intrusion action that determine the action if an unauthorized device
 *          transmits on this port.
 * INPUT  : lport
 * OUTPUT : action_status -- VAL_dot1xAuthConfigExtPortIntrusionAction_block_traffic,
 *                           VAL_dot1xAuthConfigExtPortIntrusionAction_guest_vlan
 * RETURN : TRUE/FALSE.
 * NOTES  : none.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetNextDot1xPortIntrusionAction(UI32_T *lport, UI32_T *action_status);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_PMGR_GetSecureGuestVlanId
 * ---------------------------------------------------------------------
 * PURPOSE: Get per-port guest VLAN ID.
 * INPUT  : lport.
 * OUTPUT : vid.
 * RETURN : TRUE/FALSE.
 * NOTES  : None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetSecureGuestVlanId(UI32_T lport, UI32_T *vid);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_PMGR_SetDot1xPortIntrusionAction
 * ---------------------------------------------------------------------
 * PURPOSE: Set the intrusion action to determine the action if an unauthorized device
 *          transmits on this port.
 * INPUT  : lport,
 *          action_status -- VAL_dot1xAuthConfigExtPortIntrusionAction_block_traffic,
 *                           VAL_dot1xAuthConfigExtPortIntrusionAction_guest_vlan
 * OUTPUT : none.
 * RETURN : TRUE/FALSE.
 * NOTES  : none.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_SetDot1xPortIntrusionAction(UI32_T lport, UI32_T action_status);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_PMGR_SetSecureGuestVlanId
 * ---------------------------------------------------------------------
 * PURPOSE: Set per-port guest VLAN ID.
 * INPUT  : lport,vid.
 * OUTPUT : None.
 * RETURN : TRUE/FALSE.
 * NOTES  : None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_SetSecureGuestVlanId(UI32_T lport, UI32_T vid);

#endif /* #if(SYS_CPNT_NETACCESS_GUEST_VLAN == TRUE) */


#if (SYS_CPNT_NETACCESS_MACAUTH == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_PMGR_GetMacAuthPortIntrusionAction
 *-------------------------------------------------------------------------
 * PURPOSE: Get the intrusion action of mac-authentication for the specified port.
 * INPUT  : lport -- logic port number
 * OUTPUT : action -- intrusion action
 * RETURN : TRUE -- success, FALSE -- fail
 * NOTES  : intrusion action
 *          VAL_macAuthPortIntrusionAction_block_traffic for block action
 *          VAL_macAuthPortIntrusionAction_pass_traffic for pass action
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetMacAuthPortIntrusionAction(UI32_T lport, UI32_T *action);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_PMGR_GetMacAuthPortMaxMacCount
 *-------------------------------------------------------------------------
 * PURPOSE: Get the max allowed MAC number of mac-authentication for the
 *          specified port.
 * INPUT  : lport -- logic port number
 * OUTPUT : count -- max mac count
 * RETURN : TRUE -- success, FALSE -- fail
 * NOTES  : none.
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetMacAuthPortMaxMacCount(UI32_T lport, UI32_T *count);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_MPGR_GetMacAuthPortStatus
 *-------------------------------------------------------------------------
 * PURPOSE: Get the mac-authentication status for the specified port.
 * INPUT  : lport -- logic port number.
 * OUTPUT : mac_auth_status -- mac-authentication status.
 * RETURN : TRUE -- success, FALSE -- fail
 * NOTES  : mac-authentication status
 *          NETACCESS_TYPE_MACAUTH_ENABLED for Enable mac-authentication
 *          NETACCESS_TYPE_MACAUTH_DISABLED for Disable mac-authentication
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetMacAuthPortStatus(UI32_T lport, UI32_T *mac_auth_status);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_PMGR_GetRunningMacAuthPortStatus
 *-------------------------------------------------------------------------
 * PURPOSE: Get the mac-authentication status for the specified port.
 * INPUT  : lport.
 * OUTPUT : mac_auth_status -- mac-authentication status.
 * RETURN : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- success
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- no change
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL      -- fail
 * NOTES  : mac-authentication status
 *          NETACCESS_TYPE_MACAUTH_ENABLED for Enable mac-authentication
 *          NETACCESS_TYPE_MACAUTH_DISABLED for Disable mac-authentication
 *-------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T NETACCESS_PMGR_GetRunningMacAuthPortStatus(
    UI32_T lport, UI32_T *mac_auth_status);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_PMGR_GetRunningMacAuthPortIntrusionAction
 *-------------------------------------------------------------------------
 * PURPOSE: Get the intrusion action of mac-authentication for the specified port
 * INPUT  : lport -- logic port number
 * OUTPUT : action -- intrusion action
 * RETURN : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- success
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- no change
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL      -- fail
 * NOTES  : intrusion action
 *          VAL_macAuthPortIntrusionAction_block_traffic for block action
 *          VAL_macAuthPortIntrusionAction_pass_traffic for pass action
 *-------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T NETACCESS_PMGR_GetRunningMacAuthPortIntrusionAction(
    UI32_T lport, UI32_T *action);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_PMGR_GetRunningMacAuthPortMaxMacCount
 *-------------------------------------------------------------------------
 * PURPOSE: Get the max allowed MAC number of mac-authentication for the
 *          specified port.
 * INPUT  : lport -- logic port number
 * OUTPUT : count -- max mac count
 * RETURN : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- success
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- no change
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL      -- fail
 * NOTES  : none.
 *-------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T NETACCESS_PMGR_GetRunningMacAuthPortMaxMacCount(UI32_T lport, UI32_T *count);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_PMGR_GetRunningSecureNumberAddresses
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default number is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT  : lport.
 * OUTPUT : number:secureNumberAddresses
 * RETURN : SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES  : 1. This function shall only be invoked by CLI to save the
 *             "running configuration" to local or remote files.
 *          2. Since only non-default configuration will be saved, this
 *             function shall return non-default system name.
 *          3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T NETACCESS_PMGR_GetRunningSecureNumberAddresses(
    UI32_T lport, UI32_T *number);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_PMGR_GetSecureNumberAddresses
 * ---------------------------------------------------------------------
 * PURPOSE: This function will get secureNumberAddresses by lport.
 * INPUT  : lport : logical port.
 * OUTPUT : number:secureNumberAddresses.
 * RETURN : TRUE/FALSE
 * NOTES  :
 * The maximum number of addresses that the port can learn or
 * store. Reducing this number may cause some addresses to be deleted.
 * This value is set by the user and cannot be automatically changed by the
 * agent.
 *
 * The following relationship must be preserved.
 *
 *    secureNumberAddressesStored <= secureNumberAddresses <=
 *    secureMaximumAddresses.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetSecureNumberAddresses(UI32_T lport, UI32_T *number);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_PMGR_SetMacAuthPortIntrusionAction
 *-------------------------------------------------------------------------
 * PURPOSE: Set the intrustion action of mac-authentication for the specified port.
 * INPUT  : lport  -- logic port number
 *          action -- intrusion action
 * OUTPUT : none.
 * RETURN : TRUE -- success, FALSE -- fail
 * NOTES  : intrusion action
 *          VAL_macAuthPortIntrusionAction_block_traffic for block action
 *          VAL_macAuthPortIntrusionAction_pass_traffic for pass action
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_SetMacAuthPortIntrusionAction(UI32_T lport, UI32_T action);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_PMGR_SetMacAuthPortMaxMacCount
 *-------------------------------------------------------------------------
 * PURPOSE: Set the max allowed MAC number of mac-authentication for the
 *          specified port.
 * INPUT  : lport -- logic port number
 *          count -- max mac count
 * OUTPUT : None.
 * RETURN : TRUE -- success, FALSE -- fail
 * NOTES  : None.
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_SetMacAuthPortMaxMacCount(UI32_T lport, UI32_T count);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_PMGR_SetMacAuthPortStatus
 *-------------------------------------------------------------------------
 * PURPOSE: Set the mac-authentication status for the specified port.
 * INPUT  : lport -- logic port number.
 *          mac_auth_status -- mac-authentication status.
 * OUTPUT : None.
 * RETURN : TRUE -- success, FALSE -- fail
 * NOTES  : mac-authentication status
 *          NETACCESS_TYPE_MACAUTH_ENABLED for Enable mac-authentication
 *          NETACCESS_TYPE_MACAUTH_DISABLED for Disable mac-authentication
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_SetMacAuthPortStatus(UI32_T lport, UI32_T mac_auth_status);

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_PMGR_SetSecureNumberAddresses
 *-----------------------------------------------------------------------------------
 * PURPOSE: This function will set secureNumberAddresses by lport.
 * INPUT  : lport : logical port.
 *          number:secureNumberAddresses
 * OUTPUT : None.
 * RETURN : TRUE/FALSE
 * NOTES  :
 * The maximum number of addresses that the port can learn or
 * store. Reducing this number may cause some addresses to be deleted.
 * This value is set by the user and cannot be automatically changed by the
 * agent.
 *
 * The following relationship must be preserved.
 *
 *    secureNumberAddressesStored <= secureNumberAddresses <=
 *    secureMaximumAddresses.
 *-----------------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_SetSecureNumberAddresses(UI32_T lport, UI32_T number);

#endif /* #if (SYS_CPNT_NETACCESS_MACAUTH == TRUE) */

#if (SYS_CPNT_NETACCESS_MACAUTH_AUTH_AGE == TRUE)
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_PMGR_GetSecureAuthAge
 * ---------------------------------------------------------------------
 * PURPOSE: Get the auth age.
 * INPUT  : None.
 * OUTPUT : auth_age.
 * RETURN : TRUE/FALSE.
 * NOTES  : none
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetSecureAuthAge(UI32_T *auth_age);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_PMGR_SetSecureAuthAge
 * ---------------------------------------------------------------------
 * PURPOSE: Set the auth age.
 * INPUT  : None.
 * OUTPUT : auth_age.
 * RETURN : TRUE/FALSE.
 * NOTES  : none
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_SetSecureAuthAge(UI32_T auth_age);

#endif /* #if (SYS_CPNT_NETACCESS_MACAUTH_AUTH_AGE == TRUE) */

#if (SYS_CPNT_NETACCESS_MAC_FILTER_TABLE == TRUE)
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_PMGR_GetNextFilterMac
 * ---------------------------------------------------------------------
 * PURPOSE: Get next secure filter table entry
 * INPUT  : filter_id,mac_address
 * OUTPUT : filter_id,mac_address
 * RETURN : TRUE/FALSE.
 * NOTES  : None
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetNextFilterMac(UI32_T *filter_id, UI8_T *mac_address, UI8_T *mask);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_PMGR_GetNextFilterMacByFilterId
 * ---------------------------------------------------------------------
 * PURPOSE: Get next secure filter table entry by filter id
 * INPUT  : filter_id, mac_addr, mac_mask
 * OUTPUT : mac_addr, mac_mask
 * RETURN : TRUE/FALSE.
 * NOTES  : None
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetNextFilterMacByFilterId(
    UI32_T  filter_id,  UI8_T   *mac_addr, UI8_T   *mac_mask);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_PMGR_GetRunningFilterMac
 * ---------------------------------------------------------------------
 * PURPOSE: Get next secure filter table entry
 * INPUT  : filter_id,mac_address
 * OUTPUT : filter_id,mac_address
 * RETURN : TRUE/FALSE.
 * NOTES  : None
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetRunningFilterMac(
    UI32_T  *filter_id, UI8_T   *mac_addr,  UI8_T   *mac_mask);

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_PMGR_GetFilterIdOnPort
 *-----------------------------------------------------------------------------------
 * PURPOSE: Get secure filter id which bind to port
 * INPUT  : lport
 * OUTPUT : filter_id.
 * RETURN : TRUE/FALSE
 * NOTES  : None
 *-----------------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetFilterIdOnPort(UI32_T lport, UI32_T *filter_id);

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_PMGR_GetRunningFilterIdOnPort
 *-----------------------------------------------------------------------------------
 * PURPOSE: Get secure filter id which bind to port
 * INPUT  : lport
 * OUTPUT : filter_id.
 * RETURN : SYS_TYPE_Get_Running_Cfg_T
 * NOTES  : None
 *-----------------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T NETACCESS_PMGR_GetRunningFilterIdOnPort(UI32_T lport, UI32_T *filter_id);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_PMGR_GetFilterMac
 * ---------------------------------------------------------------------
 * PURPOSE: Get secure filter table entry
 * INPUT  : filter_id, mac_addr, mac_mask
 * OUTPUT : none
 * RETURN : TRUE/FALSE.
 * NOTES  : TURE means entry exist,FALSE to not exist
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetFilterMac(
    UI32_T  filter_id,  UI8_T   *mac_addr, UI8_T   *mac_mask);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_PMGR_SetFilterMac
 * ---------------------------------------------------------------------
 * PURPOSE: Set secure filter table
 * INPUT  : filter_id, mac_addr, mac_mask, is_add
 * OUTPUT : None.
 * RETURN : TRUE/FALSE.
 * NOTES  : None
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_SetFilterMac(
    UI32_T  filter_id,  UI8_T   *mac_addr,  UI8_T   *mac_mask,  BOOL_T  is_add);

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_PMGR_SetFilterIdToPort
 *-----------------------------------------------------------------------------------
 * PURPOSE: bind secure filter id to port
 * INPUT  : lport, filter_id
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTES  : None
 *-----------------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_SetFilterIdToPort(UI32_T lport, UI32_T filter_id);

#endif /* (SYS_CPNT_NETACCESS_MAC_FILTER_TABLE == TRUE) */

/* begin for ieee_8021x.c/cli_api_1x.c
 */
/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_PMGR_DoDot1xReAuthenticate
 *-------------------------------------------------------------------------
 * PURPOSE: use the command to manually initiate a re-authentication of
 *          the specified 1X enabled port
 * INPUT  : lport
 * OUTPUT : none
 * RETURN : TRUE -- success, FALSE -- fail
 * NOTES  : none
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_DoDot1xReAuthenticate(UI32_T lport);

/*-------------------------------------------------------------------------
 * ROUTINE NAME:   NETACCESS_PMGR_GetDot1xAuthConfigEntry
 *-------------------------------------------------------------------------
 * PURPOSE: A table that contains the configuration objects for the Authenticator
 *          PAE associated with each port.
 *          An entry appears in this table for each port that may authenticate
 *          access to itself.
 * INPUT  : lport.
 * OUTPUT : AuthConfigEntry.
 * RETURN : TRUE/FALSE.
 * NOTES  : None.
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetDot1xAuthConfigEntry(
    UI32_T lport, DOT1X_AuthConfigEntry_T *AuthConfigPortEntry);

/*-------------------------------------------------------------------------
 * ROUTINE NAME:   NETACCESS_PMGR_GetDot1xAuthDiagEntry
 *-------------------------------------------------------------------------
 * PURPOSE: A table that contains the diagnostics objects for the Authenticator
 *          PAE associated with each port.
 *          An entry appears in this table for each port that may authenticate
 *          access to itself.
 * INPUT  : lport.
 * OUTPUT : AuthDiagPortEntry.
 * RETURN : TRUE/FALSE.
 * NOTES  : None.
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetDot1xAuthDiagEntry(
    UI32_T lport, DOT1X_AuthDiagEntry_T *AuthDiagPortEntry);

/*-------------------------------------------------------------------------
 * ROUTINE NAME:   NETACCESS_PMGR_GetDot1xAuthStatsEntry
 *-------------------------------------------------------------------------
 * PURPOSE: A table that contains the statistics objects for the Authenticator
 *          PAE associated with each port.
 *          An entry appears in this table for each port that may authenticate
 *          access to itself.
 * INPUT  : lort.
 * OUTPUT : AuthStateEntry.
 * RETURN : TRUE  -- The port support 802.1x
 *          FALSE -- The port don't support 802.1x.
 * NOTES  : None.
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetDot1xAuthStatsEntry(
    UI32_T lport, DOT1X_AuthStatsEntry_T *AuthStatePortEntry);

/*-------------------------------------------------------------------------
 * ROUTINE NAME:   NETACCESS_PMGR_GetDot1xPaePortEntry
 *-------------------------------------------------------------------------
 * PURPOSE: A table of system level information for each port supported
 *          by the Port Access Entity. An entry appears in this table for
 *          each port of this system.
 * INPUT  : lport.
 * OUTPUT : PaePortEntry.
 * RETURN : TRUE/FALSE.
 * NOTES  : None.
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetDot1xPaePortEntry(
    UI32_T lport, DOT1X_PaePortEntry_T *PaePortEntry);

/*-------------------------------------------------------------------------
 * ROUTINE NAME:   NETACCESS_PMGR_GetDot1xSessionStatsEntry
 *-------------------------------------------------------------------------
 * PURPOSE: The session statistics information for an Authenticatior PAE.
 *          This shows the current values being collected for each session
 *          that is still in progress, or the final values for the last valid
 *          session on each port where there is no session currently active.
 * INPUT  : lport.
 * OUTPUT : AuthSessionStatsPortEntry.
 * RETURN : TRUE/FALSE.
 * NOTES  : None.
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetDot1xSessionStatsEntry(
    UI32_T lport, DOT1X_AuthSessionStatsEntry_T *AuthSessionStatsPortEntry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_PMGR_GetDot1xSystemAuthControl
 *-------------------------------------------------------------------------
 * PURPOSE: Get the administrative enable/disable state for Port Access
 *          Control in a system.
 * INPUT  : None.
 * OUTPUT : None.
 * RETURN : SystemAuthControl
 * NOTES  : VAL_dot1xPaeSystemAuthControl_enabled for Enable SystemAuthControl
 *          VAL_dot1xPaeSystemAuthControl_disabled for Disable SystemAuthControl
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetDot1xSystemAuthControl(UI32_T *control_status);

/*-------------------------------------------------------------------------
 * ROUTINE NAME:   NETACCESS_PMGR_GetNextDot1xAuthConfigEntry
 *-------------------------------------------------------------------------
 * PURPOSE: Get next table that contains the configuration objects for the Authenticator
 *          PAE associated with each port.
 *          An entry appears in this table for each port that may authenticate
 *          access to itself.
 * INPUT  : lport.
 * OUTPUT : AuthConfigEntry.
 * RETURN : TRUE/FALSE.
 * NOTES  : None.
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetNextDot1xAuthConfigEntry(
    UI32_T *lport, DOT1X_AuthConfigEntry_T *AuthConfigPortEntry);

/*-------------------------------------------------------------------------
 * ROUTINE NAME:   NETACCESS_PMGR_GetNextDot1xAuthDiagEntry
 *-------------------------------------------------------------------------
 * PURPOSE: Get next table that contains the diagnostics objects for the Authenticator
 *          PAE associated with each port.
 *          An entry appears in this table for each port that may authenticate
 *          access to itself.
 * INPUT  : lport.
 * OUTPUT : AuthDiagPortEntry.
 * RETURN : TRUE/FALSE.
 * NOTES  : None.
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetNextDot1xAuthDiagEntry(
    UI32_T *lport, DOT1X_AuthDiagEntry_T *AuthDiagPortEntry);

/*-------------------------------------------------------------------------
 * ROUTINE NAME:   NETACCESS_PMGR_GetNextDot1xAuthStatsEntry
 *-------------------------------------------------------------------------
 * PURPOSE: Get next table that contains the statistics objects for the Authenticator
 *          PAE associated with each port.
 *          An entry appears in this table for each port that may authenticate
 *          access to itself.
 * INPUT  : lport.
 * OUTPUT : AuthStateEntry.
 * RETURN : TRUE  -- The port support 802.1x
 *          FALSE -- The port don't support 802.1x.
 * NOTES  : None.
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetNextDot1xAuthStatsEntry(
    UI32_T *lport, DOT1X_AuthStatsEntry_T *AuthStatePortEntry);

/*-------------------------------------------------------------------------
 * ROUTINE NAME:   NETACCESS_PMGR_GetNextDot1xPaePortEntry
 *-------------------------------------------------------------------------
 * PURPOSE: Get next table of system level information for each port supported
 *          by the Port Access Entity. An entry appears in this table for
 *          each port of this system.
 * INPUT  : lport.
 * OUTPUT : PaePortEntry.
 * RETURN : TRUE/FALSE.
 * NOTES  : None.
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetNextDot1xPaePortEntry(
    UI32_T *lport, DOT1X_PaePortEntry_T *PaePortEntry);

/*-------------------------------------------------------------------------
 * ROUTINE NAME:   NETACCESS_PMGR_GetDot1xPortAuthServerTimeout
 *-------------------------------------------------------------------------
 * PURPOSE: Get the value ,in seconds ,of the serverTimeout constant currently
 *          in use by the Backend Authentication state machine.
 * INPUT  : lport.
 * OUTPUT : None.
 * RETURN : seconds.
 * NOTES  :
 *-------------------------------------------------------------------------
 */
UI32_T NETACCESS_PMGR_GetDot1xPortAuthServerTimeout(UI32_T lport, UI32_T *timeout);

/*-------------------------------------------------------------------------
 * ROUTINE NAME:   NETACCESS_PMGR_GetDot1xPortAuthSuppTimeout
 *-------------------------------------------------------------------------
 * PURPOSE: Get the value ,in seconds ,of the suppTimeout constant currently
 *          in use by the Backend Authentication state machine.
 * INPUT  : lport.
 * OUTPUT : None.
 * RETURN : seconds.
 * NOTES  :
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetDot1xPortAuthSuppTimeout(UI32_T lport, UI32_T *timeout);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_PMGR_GetDot1xPortAuthorized
 *-------------------------------------------------------------------------
 * PURPOSE:  Get dot1x port authentication status.
 * INPUT  :  lport.
 * OUTPUT :  None.
 * RETURN :  VAL_dot1xAuthAuthControlledPortStatus_unauthorized  -- n/a (Unauthorized)
 *           VAL_dot1xAuthAuthControlledPortStatus_authorized  -- yes (Authorized)
 * NOTE   :  None.
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetDot1xPortAuthorized(UI32_T lport, NETACCESS_MGR_Dot1XAuthControlledPortStatus_T *port_status);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_PMGR_GetDot1xPortControlMode
 *-------------------------------------------------------------------------
 * PURPOSE: get the port control mode of 1X configuration
 * INPUT  : lport -- logic port number
 * OUTPUT : control_mode -- control mode
 * RETURN : TRUE -- success, FALSE -- fail
 * NOTES  : control_mode (define in leaf_Ieee8021x.h):
 *                VAL_dot1xAuthAuthControlledPortControl_forceUnauthorized
 *                VAL_dot1xAuthAuthControlledPortControl_forceAuthorized
 *                VAL_dot1xAuthAuthControlledPortControl_auto
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetDot1xPortControlMode(UI32_T lport, UI32_T *control_mode);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_PMGR_GetDot1xPortMaxReq
 *-------------------------------------------------------------------------
 * PURPOSE: Get port MaxReq of 1X configuration
 * INPUT  : lport
 * OUTPUT : times
 * RETURN : TRUE -- success, FALSE -- fail
 * NOTES  : None.
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetDot1xPortMaxReq(UI32_T lport, UI32_T *times);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_PMGR_GetDot1xPortOperationMode
 *-------------------------------------------------------------------------
 * PURPOSE: get the port operation mode of 1X configuration
 * INPUT  : lport -- logic port number
 * OUTPUT : mode -- operation mode
 * RETURN : TRUE -- success, FALSE -- fail
 * NOTES  : DOT1X_PORT_OPERATION_MODE_ONEPASS   for OnePass   (Single-Host)
 *          DOT1X_PORT_OPERATION_MODE_MULTIPASS for MultiPass (Multi-Host)
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetDot1xPortOperationMode(UI32_T lport, UI32_T *mode);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_PMGR_GetNextDot1xPortOperationMode
 *-------------------------------------------------------------------------
 * PURPOSE: get the next port operation mode of 1X configuration
 * INPUT  : lport -- logic port number
 * OUTPUT : lport -- logic port number
 *          mode -- operation mode
 * RETURN : TRUE -- success, FALSE -- fail
 * NOTES  : DOT1X_PORT_OPERATION_MODE_ONEPASS   for OnePass   (Single-Host)
 *          DOT1X_PORT_OPERATION_MODE_MULTIPASS for MultiPass (Multi-Host)
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetNextDot1xPortOperationMode(UI32_T *lport, UI32_T *mode);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_PMGR_GetDot1xPortQuietPeriod
 *-------------------------------------------------------------------------
 * PURPOSE: Get port QuietPeriod of 1X configuration
 * INPUT  : lport
 * OUTPUT : seconds
 * RETURN : TRUE -- success, FALSE -- fail
 * NOTES  : none
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetDot1xPortQuietPeriod(UI32_T lport, UI32_T *seconds);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_PMGR_GetDot1xPortReAuthEnabled
 *-------------------------------------------------------------------------
 * PURPOSE: Get port re-authentication status
 * INPUT  : lport
 * OUTPUT : control
 * RETURN : TRUE -- success, FALSE -- fail
 * NOTES  : control (define in leaf_Ieee8021x.h):
 *     VAL_dot1xPaePortReauthenticate_true  for Enable re-authentication
 *     VAL_dot1xPaePortReauthenticate_false for Disable re-authentication
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetDot1xPortReAuthEnabled(UI32_T lport ,UI32_T *control);

/*-------------------------------------------------------------------------
 * ROUTINE NAME:   NETACCESS_PMGR_GetDot1xPortReAuthMax
 *-------------------------------------------------------------------------
 * PURPOSE: Get the port's re-auth max
 * INPUT  : lport.
 * OUTPUT : None.
 * RETURN : PortReAuthMax.
 * NOTES  : None.
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetDot1xPortReAuthMax(UI32_T lport, UI32_T *reauth_max);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_PMGR_GetDot1xPortReAuthPeriod
 *-------------------------------------------------------------------------
 * PURPOSE: Get port ReAuthPeriod of 1X configuration
 * INPUT  : lport
 * OUTPUT : seconds
 * RETURN : TRUE -- success, FALSE -- fail
 * NOTES  : none
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetDot1xPortReAuthPeriod(UI32_T lport,UI32_T *seconds);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_PMGR_GetDot1xPortTxPeriod
 *-------------------------------------------------------------------------
 * PURPOSE: Get port TxPeriod of 1X configuration
 * INPUT  : lport
 * OUTPUT : seconds
 * RETURN : TRUE -- success, FALSE -- fail
 * NOTES  : none
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetDot1xPortTxPeriod(UI32_T lport,UI32_T *seconds);

/*-------------------------------------------------------------------------
 * ROUTINE NAME:   NETACCESS_PMGR_GetNextDot1xSessionStatsEntry
 *-------------------------------------------------------------------------
 * PURPOSE: Get next session statistics information for an Authenticatior PAE.
 *          This shows the current values being collected for each session
 *          that is still in progress, or the final values for the last valid
 *          session on each port where there is no session currently active.
 * INPUT  : lport.
 * OUTPUT : AuthSessionStatsPortEntry.
 * RETURN : TRUE/FALSE.
 * NOTES  : None.
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetNextDot1xSessionStatsEntry(
    UI32_T *lport, DOT1X_AuthSessionStatsEntry_T *AuthSessionStatsPortEntry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_PMGR_SetDot1xPortAdminCtrlDirections
 *-------------------------------------------------------------------------
 * PURPOSE: Set the value of the administrative controlled directions
 *          parameter for the port.
 * INPUT  : lport,direction.
 * OUTPUT : None.
 * RETURN : TRUE/FALSE.
 * NOTES  : direction =   VAL_dot1xAuthAdminControlledDirections_both for Both
 *                        VAL_dot1xAuthAdminControlledDirections_in for In
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_SetDot1xPortAdminCtrlDirections(UI32_T lport, UI32_T dir);

/*-------------------------------------------------------------------------
 * ROUTINE NAME:   NETACCESS_PMGR_SetDot1xPortAuthServerTimeout
 *-------------------------------------------------------------------------
 * PURPOSE: Set the value ,in seconds ,of the serverTimeout constant currently
 *          in use by the Backend Authentication state machine.
 * INPUT  : lport,seconds.
 * OUTPUT : None.
 * RETURN : TRUE/FALSE.
 * NOTES  :
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_SetDot1xPortAuthServerTimeout(UI32_T lport, UI32_T timeout);

/*-------------------------------------------------------------------------
 * ROUTINE NAME:   NETACCESS_PMGR_SetDot1xPortAuthSuppTimeout
 *-------------------------------------------------------------------------
 * PURPOSE: Set the value ,in seconds ,of the suppTimeout constant currently
 *          in use by the Backend Authentication state machine.
 * INPUT  : lport,timeout.
 * OUTPUT : None.
 * RETURN : TRUE/FALSE.
 * NOTES  :
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_SetDot1xPortAuthSuppTimeout(UI32_T lport, UI32_T timeout);

/* ------------------------------------------------------------------------
 * ROUTINE NAME: NETACCESS_PMGR_SetDot1xPortAuthTxEnabled
 * ------------------------------------------------------------------------
 * PURPOSE: Set the value of the keyTransmissionEnabled constant currently
 *          in use by the Authenticator PAE state machine.
 * INPUT  : lport - port number
 *          value - VAL_dot1xAuthKeyTxEnabled_true or
 *                  VAL_dot1xAuthKeyTxEnabled_false
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTES  : None
 * ------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_SetDot1xPortAuthTxEnabled(UI32_T lport, UI32_T value);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_PMGR_SetDot1xPortControlMode
 *-------------------------------------------------------------------------
 * PURPOSE: set the port control mode of 1X configuration
 * INPUT  : lport -- logic port number
 *          control_mode -- control mode
 * OUTPUT : none
 * RETURN : TRUE -- success, FALSE -- fail
 * NOTES  : control_mode (define in leaf_Ieee8021x.h):
 *                VAL_dot1xAuthAuthControlledPortControl_forceUnauthorized
 *                VAL_dot1xAuthAuthControlledPortControl_forceAuthorized
 *                VAL_dot1xAuthAuthControlledPortControl_auto
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_SetDot1xPortControlMode(UI32_T lport, UI32_T control_mode);

/* ------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_PMGR_GetDot1xPortDetails
 * ------------------------------------------------------------------------
 * PURPOSE: Get dot1x detail information.
 * INPUT  : lport
 * OUTPUT : port_details_p
 * RETURN : TRUE  -- enabled
 *          FALSE -- disabled
 * NOTE   : None
 * ------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetDot1xPortDetails(
    UI32_T lport, DOT1X_PortDetails_T *port_details_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_PMGR_SetDot1xPortMaxReAuthReq
 *-------------------------------------------------------------------------
 * PURPOSE: Set port Max-ReAuth Req of 1X configuration
 * INPUT  : lport,times
 * OUTPUT : none
 * RETURN : TRUE -- success, FALSE -- fail
 * NOTES  : None.
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_SetDot1xPortMaxReAuthReq(UI32_T lport, UI32_T times);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_PMGR_SetDot1xPortMaxReq
 *-------------------------------------------------------------------------
 * PURPOSE: Set port MaxReq of 1X configuration
 * INPUT  : lport,times
 * OUTPUT : none
 * RETURN : TRUE -- success, FALSE -- fail
 * NOTES  : None.
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_SetDot1xPortMaxReq(UI32_T lport, UI32_T times);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_PMGR_SetDot1xPortMultiHostMacCount
 *-------------------------------------------------------------------------
 * PURPOSE: set the max allowed MAC number in multi-host mode
 * INPUT  : lport -- logic port number
 *          count -- max mac count for multi-host mode
 * OUTPUT : none.
 * RETURN : TRUE -- success, FALSE -- fail
 * NOTES  : none.
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_SetDot1xPortMultiHostMacCount(UI32_T lport, UI32_T count);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_PMGR_GetDot1xPortMultiHostMacCount
 *-------------------------------------------------------------------------
 * PURPOSE: Get the max allowed MAC number in multi-host mode
 * INPUT  : lport -- logic port number
 *          count -- max mac count for multi-host mode
 * OUTPUT : none.
 * RETURN : TRUE -- success, FALSE -- fail
 * NOTES  : none.
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetDot1xPortMultiHostMacCount(UI32_T lport, UI32_T *count);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_PMGR_GetNextDot1xPortMultiHostMacCount
 *-------------------------------------------------------------------------
 * PURPOSE: Get the next max allowed MAC number in multi-host mode
 * INPUT  : lport -- logic port number
 * OUTPUT : lport -- logic port number
 *          count -- max mac count for multi-host mode
 * RETURN : TRUE -- success, FALSE -- fail
 * NOTES  : none.
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetNextDot1xPortMultiHostMacCount(UI32_T *lport, UI32_T *count);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_PMGR_SetDot1xPortOperationMode
 *-------------------------------------------------------------------------
 * PURPOSE: set the port operation mode of 1X configuration
 * INPUT  : lport -- logic port number
 *          mode -- operation mode
 * OUTPUT : none
 * RETURN : TRUE -- success, FALSE -- fail
 * NOTES  : DOT1X_PORT_OPERATION_MODE_ONEPASS   for OnePass   (Single-Host)
 *          DOT1X_PORT_OPERATION_MODE_MULTIPASS for MultiPass (Multi-Host)
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_SetDot1xPortOperationMode(UI32_T lport,UI32_T mode);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_PMGR_SetDot1xPortPaePortInitialize
 *-------------------------------------------------------------------------
 * PURPOSE: Set this attribute TRUE causes the Port to be initialized.
 *          the attribute value reverts to FALSE once initialization has completed.
 * INPUT  : lport,control.
 * OUTPUT : None.
 * RETURN : TRUE/FALSE.
 * NOTES  : control =   VAL_dot1xPaePortInitialize_true for Enable Initialize
 *                      VAL_dot1xPaePortInitialize_false Disable Initialize
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_SetDot1xPortPaePortInitialize(UI32_T lport, UI32_T control);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_PMGR_SetDot1xPortQuietPeriod
 * ---------------------------------------------------------------------
 * PURPOSE: set port QuietPeriod of 1X configuration
 * INPUT  : lport,seconds
 * OUTPUT : none
 * RETURN : TRUE -- success, FALSE -- fail
 * NOTES  : none
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_SetDot1xPortQuietPeriod(UI32_T lport, UI32_T seconds);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_PMGR_SetDot1xPortReAuthEnabled
 *-------------------------------------------------------------------------
 * PURPOSE: enable/disable port period re-authentication of the 1X client,
 *          which is disabled by default
 * INPUT  : lport,control
 * OUTPUT : none
 * RETURN : TRUE -- success, FALSE -- fail
 * NOTES  : control (define in leaf_Ieee8021x.h):
 *     VAL_dot1xPaePortReauthenticate_true  for Enable re-authentication
 *     VAL_dot1xPaePortReauthenticate_false for Disable re-authentication
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_SetDot1xPortReAuthEnabled(UI32_T lport, UI32_T control);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_PMGR_SetDot1xPortReAuthPeriod
 *-------------------------------------------------------------------------
 * PURPOSE: set port ReAuthPeriod of 1X configuration
 * INPUT  : lport,seconds
 * OUTPUT : none
 * RETURN : TRUE -- success, FALSE -- fail
 * NOTES  : none
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_SetDot1xPortReAuthPeriod(UI32_T lport, UI32_T seconds);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_PMGR_SetDot1xPortTxPeriod
 *-------------------------------------------------------------------------
 * PURPOSE: Set port TxPeriod of 1X configuration
 * INPUT  : lport,seconds
 * OUTPUT : none
 * RETURN : TRUE -- success, FALSE -- fail
 * NOTES  : none
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_SetDot1xPortTxPeriod(UI32_T lport, UI32_T seconds);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_PMGR_SetDot1xSystemAuthControl
 *-------------------------------------------------------------------------
 * PURPOSE: set system auth control of 1X configuration
 * INPUT  : control_status
 * OUTPUT : none
 * RETURN : TRUE -- success, FALSE -- fail
 * NOTES  : control_status (define in leaf_Ieee8021x.h):
 *         VAL_dot1xPaeSystemAuthControl_enabled for Enable SystemAuthControl
 *         VAL_dot1xPaeSystemAuthControl_disabled for Disable SystemAuthControl
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_SetDot1xSystemAuthControl(UI32_T control_status);

#if (SYS_CPNT_DOT1X_EAPOL_PASS_THROUGH == TRUE)
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_PMGR_SetDot1xEapolPassThrough
 * ---------------------------------------------------------------------
 * PURPOSE  : Set status of EAPOL frames pass-through when the global
 *            status dot1x is disabled
 * INPUT    : status    - VAL_dot1xEapolPassThrough_enabled(1)
 *                        VAL_dot1xEapolPassThrough_disabled(2)
 * OUTPUT   : None.
 * RETURN   : TRUE/FALSE.
 * NOTES    :
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_SetDot1xEapolPassThrough(UI32_T status);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_PMGR_GetDot1xEapolPassThrough
 * ---------------------------------------------------------------------
 * PURPOSE  : Get the intrusion action to determine the action if an unauthorized device
 *            transmits on this port.
 * INPUT    : lport
 * OUTPUT   : action_status -- VAL_dot1xAuthConfigExtPortIntrusionAction_block_traffic,
 *                             VAL_dot1xAuthConfigExtPortIntrusionAction_guest_vlan
 * RETURN   : TRUE/FALSE.
 * NOTES    : none.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetDot1xEapolPassThrough(UI32_T *status);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_MGR_GetRunningDot1xEapolPassThrough
 * ---------------------------------------------------------------------
 * PURPOSE  : Get the intrusion action to determine the action if an unauthorized device
 *            transmits on this port.
 * INPUT    : lport
 * OUTPUT   : action_status -- VAL_dot1xAuthConfigExtPortIntrusionAction_block_traffic,
 *                             VAL_dot1xAuthConfigExtPortIntrusionAction_guest_vlan
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- success
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- no change
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL      -- fail
 * NOTES    : none.
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T NETACCESS_PMGR_GetRunningDot1xEapolPassThrough(UI32_T *status);
#endif /* #if (SYS_CPNT_DOT1X_EAPOL_PASS_THROUGH == TRUE) */

/* begin for cli_api_ethernet.c [port security]
 */
/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_PMGR_SetPortSecurityActionStatus
 *-------------------------------------------------------------------------
 * PURPOSE: This function will set port security action status
 * INPUT  : lport          -- which port to
 *          action_status  -- action status
 * OUTPUT : None
 * RETURN : TRUE: Successfully, FALSE: If not available
 * NOTE   : if the port is not in portSecurity port mode,will return FALSE
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_SetPortSecurityActionStatus (UI32_T lport, UI32_T action_status);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_PMGR_SetPortSecurityMaxMacCount
 *-------------------------------------------------------------------------
 * PURPOSE: This function will set port auto learning mac counts
 * INPUT  : lport          -- which port to
 *          max_mac_count  -- maximum mac learning count
 * OUTPUT : None
 * RETURN : TRUE: Successfully, FALSE: If not available
 * NOTE   : Default : max_mac_count = 1
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_SetPortSecurityMaxMacCount(UI32_T lport, UI32_T max_mac_count);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_PMGR_SetPortSecurityStatus
 *-------------------------------------------------------------------------
 * PURPOSE: This function will Set the port security status
 * INPUT  : lport                  - interface index
 *          portsec_status         - VAL_portSecPortStatus_enabled
 *                                    VAL_portSecPortStatus_disabled
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTE   : Port security doesn't support
 *          1) unknown port, 2) trunk member, and 3) trunk port
 *-------------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_SetPortSecurityStatus(UI32_T lport, UI32_T portsec_status);

/*------------------------------------------------------------------------
 * ROUTINE NAME - NETACCESS_PMGR_ConvertPortSecuritySecuredAddressIntoManual
 *------------------------------------------------------------------------
 * FUNCTION: To convert port security learnt secured MAC address into manual
 *           configured on the specified interface. If interface is
 *           PSEC_MGR_INTERFACE_INDEX_FOR_ALL, it will apply to all interface.
 * INPUT   : ifindex  -- interface
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 *------------------------------------------------------------------------
 */
BOOL_T
NETACCESS_PMGR_ConvertPortSecuritySecuredAddressIntoManual(
    UI32_T ifindex
);

#if (SYS_CPNT_NETACCESS_LINK_DETECTION == TRUE)
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_PMGR_SetLinkDetectionStatus
 * ---------------------------------------------------------------------
 * PURPOSE: Set the link detection status on this port.
 * INPUT:  lport, status
 * OUTPUT: None
 * RETURN: TRUE/FALSE
 * NOTES: VAL_networkAccessPortLinkDetectionStatus_enabled
 *        VAL_networkAccessPortLinkDetectionStatus_disabled
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_SetLinkDetectionStatus(UI32_T lport, UI32_T status);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_PMGR_GetLinkDetectionStatus
 * ---------------------------------------------------------------------
 * PURPOSE: Get the link detection status on this port.
 * INPUT:  lport
 * OUTPUT: status_p
 * RETURN: TRUE/FALSE
 * NOTES: VAL_networkAccessPortLinkDetectionStatus_enabled
 *        VAL_networkAccessPortLinkDetectionStatus_disabled
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetLinkDetectionStatus(UI32_T lport, UI32_T *status_p);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_PMGR_GetRunningLinkDetectionStatus
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default link detection status is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT:   lport
 * OUTPUT:  status_p
 * RETURN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default intrusion action.
 *        3. Caller has to prepare buffer for storing link detection status
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T NETACCESS_PMGR_GetRunningLinkDetectionStatus(UI32_T lport, UI32_T *status_p);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_PMGR_SetLinkDetectionMode
 * ---------------------------------------------------------------------
 * PURPOSE: Set the link detection mode on this port.
 * INPUT:  lport, mode
 * OUTPUT: None
 * RETURN: TRUE/FALSE.
 * NOTES: VAL_networkAccessPortLinkDetectionMode_linkUp
 *        VAL_networkAccessPortLinkDetectionMode_linkDown
 *        VAL_networkAccessPortLinkDetectionMode_linkUpDown
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_SetLinkDetectionMode(UI32_T lport, UI32_T mode);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_PMGR_GetLinkDetectionMode
 * ---------------------------------------------------------------------
 * PURPOSE: Get the link detection mode on this port.
 * INPUT:  lport
 * OUTPUT: mode_p
 * RETURN: TRUE/FALSE.
 * NOTES: VAL_networkAccessPortLinkDetectionMode_linkUp
 *        VAL_networkAccessPortLinkDetectionMode_linkDown
 *        VAL_networkAccessPortLinkDetectionMode_linkUpDown
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetLinkDetectionMode(UI32_T lport, UI32_T *mode_p);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_PMGR_GetRunningLinkDetectionMode
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default link detection mode is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT:   lport
 * OUTPUT:  mode_p
 * RETURN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default intrusion action.
 *        3. Caller has to prepare buffer for storing link detection mode
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T NETACCESS_PMGR_GetRunningLinkDetectionMode(UI32_T lport, UI32_T *mode_p);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_PMGR_SetLinkDetectionAction
 * ---------------------------------------------------------------------
 * PURPOSE: Set the link detection action on this port.
 * INPUT:  lport, action
 * OUTPUT: None
 * RETURN: TRUE/FALSE
 * NOTES: action:
 *        VAL_networkAccessPortLinkDetectionAciton_trap
 *        VAL_networkAccessPortLinkDetectionAciton_shutdown
 *        VAL_networkAccessPortLinkDetectionAciton_trapAndShutdown
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_SetLinkDetectionAction(UI32_T lport, UI32_T action);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_PMGR_GetLinkDetectionAction
 * ---------------------------------------------------------------------
 * PURPOSE: Get the link detection action on this port.
 * INPUT:  lport
 * OUTPUT: action_p
 * RETURN: TRUE/FALSE.
 * NOTES: action:
 *        VAL_networkAccessPortLinkDetectionAciton_trap
 *        VAL_networkAccessPortLinkDetectionAciton_shutdown
 *        VAL_networkAccessPortLinkDetectionAciton_trapAndShutdown
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_GetLinkDetectionAction(UI32_T lport, UI32_T *action_p);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_PMGR_GetRunningLinkDetectionAction
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default link detection action is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT:   lport
 * OUTPUT:  mode_p
 * RETURN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default intrusion action.
 *        3. Caller has to prepare buffer for storing link detection action
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T NETACCESS_PMGR_GetRunningLinkDetectionAction(UI32_T lport, UI32_T *action_p);
#endif /* #if (SYS_CPNT_NETACCESS_LINK_DETECTION == TRUE) */

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_PMGR_AuthenticatePacket
 * ---------------------------------------------------------------------
 * PURPOSE:Asynchronous call to authenticate packet
 * INPUT:  cookie
 * OUTPUT: None.
 * RETURN: TRUE/FALSE
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_PMGR_AuthenticatePacket(void  *cookie);

#endif /* #if (SYS_CPNT_NETACCESS == TRUE) */
#endif /* NETACCESS_PMGR_H */

