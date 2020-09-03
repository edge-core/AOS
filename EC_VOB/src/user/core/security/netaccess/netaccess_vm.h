/* Project Name: New Feature
 * File_Name : netaccess_vm.h
 * Purpose     : NETACCESS state machine
 *
 * 2006/01/27    : Ricky Lin  Create this file
 *
 * Copyright(C)      Accton Corporation, 2001, 2002
 *
 * Note    : Designed for new platform (ACP_V3_Enhancement)
 */
#ifndef NETACCESS_VM_H
#define NETACCESS_VM_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_cpnt.h"
#if (SYS_CPNT_NETACCESS == TRUE)
#include "netaccess_type.h"
#include "1x_om.h"
#if (SYS_CPNT_NETACCESS_DYNAMIC_QOS == TRUE)
#include "rule_type.h"
#endif
/* NAMING CONSTANT DECLARATIONS
 */
#define NETACCESS_MAX_SESSION_TIME              0x7fffffff
#define NETACCESS_MAX_SESSION_EXPIRE_TIME       0xffffffff

/* MACRO FUNCTION DECLARATIONS */

/* DATA TYPE DECLARATIONS
 */
typedef enum NETACCESS_VM_VlanCalcResult_E
{
    NETACCESS_VLAN_SUBSET_IS_EMPTY,     /* subset = {NULL} */
    NETACCESS_VLAN_WITH_DEFAULT,        /* all returned vlan strings are null string */
    NETACCESS_VLAN_SUBSET_EXISTING,     /* subset != {NULL} */
} NETACCESS_VM_VlanCalcResult_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS */

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_Initialize
 * ---------------------------------------------------------------------
 * PURPOSE: initialize VM
 * INPUT:  none
 * OUTPUT: None.
 * RETURN: TRUE -- succeeded / FALSE -- failed
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_VM_Initialize();

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_ProcessEventPortModeChange
 * ---------------------------------------------------------------------
 * PURPOSE: process event (port mode change)
 * INPUT:  lport, new_port_mode
 * OUTPUT: None.
 * RETURN: TRUE -- succeeded / FALSE -- failed
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_VM_ProcessEventPortModeChange(UI32_T lport, NETACCESS_PortMode_T new_port_mode);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_ProcessEventNewMac
 * ---------------------------------------------------------------------
 * PURPOSE: process event (new mac)
 * INPUT:  msg
 * OUTPUT: None.
 * RETURN: TRUE -- succeeded / FALSE -- failed
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_VM_ProcessEventNewMac(NETACCESS_NEWMAC_MSGQ_T *new_mac_msg_ptr);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_ProcessEventPortMove
 * ---------------------------------------------------------------------
 * PURPOSE: process event (port move)
 * INPUT:  lport, new_mac, vid
 * OUTPUT: None.
 * RETURN: TRUE -- should learn the new mac / FALSE -- should not learn this new mac
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_VM_ProcessEventPortMove(UI32_T lport, UI8_T *new_mac, UI32_T vid);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_ProcessEventPortLinkUp
 * ---------------------------------------------------------------------
 * PURPOSE: process event (port link-up)
 * INPUT:  lport
 * OUTPUT: None.
 * RETURN: TRUE -- succeeded / FALSE -- failed
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_VM_ProcessEventPortLinkUp(UI32_T lport);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_ProcessEventPortLinkDown
 * ---------------------------------------------------------------------
 * PURPOSE: process event (port link-down)
 * INPUT:  lport
 * OUTPUT: None.
 * RETURN: TRUE -- succeeded / FALSE -- failed
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_VM_ProcessEventPortLinkDown(UI32_T lport);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_ProcessEventPortAdminUp
 * ---------------------------------------------------------------------
 * PURPOSE: process event (port admin-up)
 * INPUT:  lport
 * OUTPUT: None.
 * RETURN: TRUE -- succeeded / FALSE -- failed
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_VM_ProcessEventPortAdminUp(UI32_T lport);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_ProcessEventPortAdminDown
 * ---------------------------------------------------------------------
 * PURPOSE: process event (port admin-down)
 * INPUT:  lport
 * OUTPUT: None.
 * RETURN: TRUE -- succeeded / FALSE -- failed
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_VM_ProcessEventPortAdminDown(UI32_T lport);

#if NeedModify
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_ProcessEventPortJoinVlan
 * ---------------------------------------------------------------------
 * PURPOSE: process event (port join vlan)
 * INPUT:  lport
 * OUTPUT: None.
 * RETURN: TRUE -- succeeded / FALSE -- failed
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_VM_ProcessEventPortJoinVlan(UI32_T vid, UI32_T lport, UI32_T status);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_ProcessEventPortDepartVlan
 * ---------------------------------------------------------------------
 * PURPOSE: process event (port depart vlan)
 * INPUT:  lport
 * OUTPUT: None.
 * RETURN: TRUE -- succeeded / FALSE -- failed
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_VM_ProcessEventPortDepartVlan(UI32_T vid, UI32_T lport, UI32_T status);
#endif

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_ProcessEventPortVlanChange
 * ---------------------------------------------------------------------
 * PURPOSE: process event (port join/depart vlan)
 * INPUT:  vid,lport,status,change_event
 * OUTPUT: None.
 * RETURN: TRUE -- succeeded / FALSE -- failed
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_VM_ProcessEventPortVlanChange(UI32_T vid, UI32_T lport, UI32_T status, NETACCESS_VlanModified_T change_event);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_ProcessEventReauthentication
 * ---------------------------------------------------------------------
 * PURPOSE: process event (reauthenticate mac)
 * INPUT:  reauth_mac
 * OUTPUT: None.
 * RETURN: TRUE -- succeeded / FALSE -- failed
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_VM_ProcessEventReauthentication(UI8_T *reauth_mac);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_ProcessEventTimerUp
 * ---------------------------------------------------------------------
 * PURPOSE: process event (timer period)
 * INPUT:  None.
 * OUTPUT: None.
 * RETURN: TRUE -- succeeded / FALSE -- failed
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_VM_ProcessEventTimerUp();

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_ProcessRadiusMsg
 * ---------------------------------------------------------------------
 * PURPOSE: process radius msg
 * INPUT:  radius_msg.
 * OUTPUT: None.
 * RETURN: TRUE -- succeeded / FALSE -- failed
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_VM_ProcessRadiusMsg(NETACCESS_RADIUS_MSGQ_T *radius_msg);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_ProcessDot1xMsg
 * ---------------------------------------------------------------------
 * PURPOSE: process dot1x msg
 * INPUT:  dot1x_msg.
 * OUTPUT: None.
 * RETURN: TRUE -- succeeded / FALSE -- failed
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_VM_ProcessDot1xMsg(NETACCESS_DOT1X_MSGQ_T *dot1x_msg);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_ProcessMacAgeOutMsg
 * ---------------------------------------------------------------------
 * PURPOSE: process mac age out msg
 * INPUT:  macageout_msg.
 * OUTPUT: None.
 * RETURN: TRUE -- succeeded / FALSE -- failed
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_VM_ProcessMacAgeOutMsg(NETACCESS_MACAGEOUT_MSGQ_T *macageout_msg);

#if (SYS_CPNT_NETACCESS_DYNAMIC_QOS == TRUE)

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_VM_ProcessEventDelPolicyMap
 *-------------------------------------------------------------------------
 * PURPOSE  : Process for detecting a policy map be deleted.
 * INPUT    : policy_map_name   -- which policy map be deleted.
 *            dynamic_port_list -- the port list that bind the deleted policy
 *                                 map with dynamic type.
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void NETACCESS_VM_ProcessEventDelPolicyMap(const char *policy_map_name, UI8_T *dynamic_port_list);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_VM_ProcessEventDelAcl
 *-------------------------------------------------------------------------
 * PURPOSE  : Process for detecting a ACL be deleted.
 * INPUT    : acl_name          -- which ACL be deleted.
 *            acl_type          -- which type of ACL
 *            dynamic_port_list -- the port list that bind the deleted ACL
 *                                 with dynamic type.
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void NETACCESS_VM_ProcessEventDelAcl(const char *acl_name, UI32_T acl_type, UI8_T *dynamic_port_list);

#endif /* #if (SYS_CPNT_NETACCESS_DYNAMIC_QOS == TRUE) */

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_DetectPortMove
 * ---------------------------------------------------------------------
 * PURPOSE: detect event (port move)
 * INPUT:  lport, new_mac
 * OUTPUT: None.
 * RETURN: TRUE -- should learn the new mac / FALSE -- should not learn this new mac
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_VM_DetectPortMove(UI32_T lport, UI8_T *new_mac, UI32_T vid);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_VM_AddMacToAmtrByPort
 *-------------------------------------------------------------------------
 * PURPOSE  : add the specified mac address to each VLAN by lport
 * INPUT    : lport, mac
 * OUTPUT   : counter
 * RETURN   : TRUE - succeeded; FALSE - failed
 * NOTE     : counter means how many MAC+VLAN will be added
 *            caller should update the counter to om by itself
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_VM_AddMacToAmtrByPort(UI32_T lport, UI8_T *mac, UI32_T *counter);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_VM_DeleteMacFromAmtrByPort
 *-------------------------------------------------------------------------
 * PURPOSE  : delete the specified mac address from AMTR by lport
 * INPUT    : lport, mac
 * OUTPUT   : None
 * RETURN   : TRUE - succeeded; FALSE - failed
 * NOTE     : none
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_VM_DeleteMacFromAmtrByPort(UI32_T lport, UI8_T *mac);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_VM_DeleteAllDynamicMacFromAmtrByPort
 *-------------------------------------------------------------------------
 * PURPOSE : Delete all dynamic MAC address from MAC address table by port.
 * INPUT   : lport -- logic port number.
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE.
 * NOTE    : All actions should be completed after this function returned.
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_VM_DeleteAllDynamicMacFromAmtrByPort(UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_VM_DeleteAuthorizedUserByIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : destroy a mac address by mac_index (amtr & om & hisam)
 * INPUT    : mac_index
 * OUTPUT   : None
 * RETURN   : TRUE - succeeded; FALSE - failed
 * NOTE     : if mac is not existed, return FALSE
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_VM_DeleteAuthorizedUserByIndex(UI32_T mac_index);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_VM_DeleteAllAuthorizedUserByPort
 *-------------------------------------------------------------------------
 * PURPOSE  : delete all mac address from AMTR & om by lport
 * INPUT    : lport
 * OUTPUT   : None
 * RETURN   : TRUE - succeeded; FALSE - failed
 * NOTE     : none
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_VM_DeleteAllAuthorizedUserByPort(UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_VM_ResetVlanQos2Default
 *-------------------------------------------------------------------------
 * PURPOSE  : reset a port VLAN and QoS to administrative configuration
 * INPUT    : lport
 * OUTPUT   : None
 * RETURN   : TRUE - succeeded; FALSE - failed
 * NOTE     : none
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_VM_ResetVlanQos2Default(UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_VM_SetAllMacExpiredByLport
 *-------------------------------------------------------------------------
 * PURPOSE  : let all mac on lport expire so that they will be reauthenticated
 * INPUT    : lport
 * OUTPUT   : none
 * RETURN   : TRUE -- succeeded / FALSE -- failed
 * NOTE     : None
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_VM_SetAllMacExpiredByLport(UI32_T lport);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_IsValidVlanList
 * ---------------------------------------------------------------------
 * PURPOSE: This function validates the VLAN list.
 * INPUT  : lport       -- logic port number
 *          vlan_list_p -- VLAN string list
 * OUTPUT : None.
 * RETURN : TRUE  -- valid
 *          FALSE -- invalid
 * NOTES :  This function checks the port mode and verifies the VLAN list.
 *          After the check. If the VLAN list can be applied to port,
 *          the return_vlan_change flag should be set.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_VM_IsValidVlanList(UI32_T lport, const char *vlan_list_p);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_IsValidQosProfiles
 * ---------------------------------------------------------------------
 * PURPOSE: This function validates the QoS profiles.
 * INPUT  : lport           -- logic port number
 *          str_profiles    -- QoS profiles string
 * OUTPUT : None.
 * RETURN : TRUE  -- valid
 *          FALSE -- invalid
 * NOTES :  This function checks the port mode and verifies the QoS profiles.
 *          After the check. If the 802.1p profile can be applied to port,
 *          the return_default_port_priority_changed flag should be set.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_VM_IsValidQosProfiles(UI32_T lport, const char *str_profiles);

#if (SYS_CPNT_NETACCESS_AGING_MODE == SYS_CPNT_NETACCESS_AGING_MODE_CONFIGURABLE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_VM_SetMacAddressAgingMode
 *-------------------------------------------------------------------------
 * PURPOSE  : Set the MAC address aging mode.
 * INPUT    : mode -- MAC address aging mode
 * OUTPUT   : None.
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : Aging mode
 *            VAL_networkAccessAging_enabled for aging enabled
 *            VAL_networkAccessAging_disabled for aging disabled
 *-------------------------------------------------------------------------
*/
BOOL_T NETACCESS_VM_SetMacAddressAgingMode(UI32_T mode);
#endif /* #if (SYS_CPNT_NETACCESS_AGING_MODE == SYS_CPNT_NETACCESS_AGING_MODE_CONFIGURABLE) */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_VM_GetSecureNumberAddressesStoredAuthorByRada
 *-------------------------------------------------------------------------
 * PURPOSE  : get the stored secure number of address that be authorized by RADIUS
 * INPUT    : lport
 * OUTPUT   : none
 * RETURN   : the stored secure number of address
 * NOTE     : None
 *-------------------------------------------------------------------------*/
UI32_T NETACCESS_VM_GetSecureNumberAddressesStoredAuthorByRada(UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETACCESS_VM_GetSecureNumberAddressesStoredWithoutMacFilter
 *-------------------------------------------------------------------------
 * PURPOSE  : Get the stored secure number of address without MAC filter
 * INPUT    : lport
 * OUTPUT   : none
 * RETURN   : the stored secure number of address
 * NOTE     : None
 *-------------------------------------------------------------------------*/
UI32_T NETACCESS_VM_GetSecureNumberAddressesStoredWithoutMacFilter(UI32_T lport);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_IsMacFilterMatched
 * ---------------------------------------------------------------------
 * PURPOSE: Match the MAC address with each entry of MAC filter table
 * INPUT  : mac, lport
 * OUTPUT : None
 * RETURN : TRUE - Matched; FALSE - No match
 * NOTES  : None
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_VM_IsMacFilterMatched(UI32_T lport, UI8_T *mac);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_IsInRestrictedVlan
 * ---------------------------------------------------------------------
 * PURPOSE  : Check the port is in restricted VLAN
 * INPUT    : lport -- lport index
 * OUTPUT   : None.
 * RETURN   : TRUE -- in restricted VLAN, FALSE -- no
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_VM_IsInRestrictedVlan(UI32_T lport);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_AddToGuestVlan
 * ---------------------------------------------------------------------
 * PURPOSE  : Add the port to restricted VLAN
 * INPUT    : lport -- lport index
 * OUTPUT   : None.
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_VM_SetToGuestVlan(UI32_T lport);

BOOL_T NETACCESS_VM_LeaveGuestVlan(UI32_T lport);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_IsInAutoVlan
 * ---------------------------------------------------------------------
 * PURPOSE  : Check the port is in auto VLAN
 * INPUT    : lport -- lport index
 * OUTPUT   : None.
 * RETURN   : TRUE -- in auto VLAN, FALSE -- no
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_VM_IsInAutoVlan(UI32_T lport);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_AddToGuestVlan
 * ---------------------------------------------------------------------
 * PURPOSE  : Add the port to restricted VLAN
 * INPUT    : lport -- lport index
 * OUTPUT   : None.
 * RETURN   : TRUE -- success, FALSE -- fail
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_VM_AddToGuestVlan(UI32_T lport);

#if (SYS_CPNT_DOT1X_EAPOL_PASS_THROUGH == TRUE)
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_VM_SetDriverForRecievingEapPacket
 * ---------------------------------------------------------------------
 * PURPOSE: The action to driver when receive an EAP packet.
 * INPUT  : dot1x_sys_ctrl  -- global status of 802.1X
 *          eapol_pass_thru -- status of EAPOL frames pass-through
 * OUTPUT : None.
 * RETURN : TRUE  -- succeeded
 *          FALSE -- failed
 * NOTES :
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_VM_SetDriverForRecievingEapPacket(UI32_T dot1x_sys_ctrl, DOT1X_OM_EapolPassThru_T eapol_pass_thru);
#endif /* #if (SYS_CPNT_DOT1X_EAPOL_PASS_THROUGH == TRUE) */

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  NETACCESS_VM_IsMacAuthEnabled
 *-------------------------------------------------------------------------
 * PURPOSE : Check mac-authentication is enabled on port.
 * INPUT   : lport -- checked port number
 * OUTPUT  : None.
 * RETURN  : TRUE  -- enabled
 *           FALSE -- disabled
 * NOTE    :
 *-------------------------------------------------------------------------*/
BOOL_T NETACCESS_VM_IsMacAuthEnabled(UI32_T lport);

#endif /* #if (SYS_CPNT_NETACCESS == TRUE) */
#endif /* NETACCESS_VM_H */

