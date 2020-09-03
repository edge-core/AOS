
#ifndef		_DHCPSNP_OM_H
#define		_DHCPSNP_OM_H

#include "string.h"
#include "sysfun.h"
#include "dhcpsnp_type.h"

#define DHCPSNP_OM_EnterCriticalSection()   orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dhcpsnp_om_semaphore_id)
#define DHCPSNP_OM_LeaveCriticalSection()   SYSFUN_OM_LEAVE_CRITICAL_SECTION(dhcpsnp_om_semaphore_id, orig_priority)

#define DHCPSNP_OM_ACL_HASH_DEPTH               4
#define DHCPSNP_OM_ACL_HISAM_N1                 3                          /* table balance threshold */
#define DHCPSNP_OM_ACL_HISAM_N2                 15                         /* table balance threshold */
#define DHCPSNP_OM_ACL_HASH_NBR                 (DHCPSNP_TYPE_MAX_NBR_OF_BINDING_ENTRY / 10)
#define DHCPSNP_OM_ACL_HASH_NBR_BASE            17                         /* table's hash number must larger than this value,
                                                                           or L_HISAM will cause some error */
#define DHCPSNP_OM_ACL_INDEX_NBR               100                         /* table is divided to 100 blocks  */
#define DHCPSNP_OM_ACL_NUMBER_OF_KEYS           2

#define DHCPSNP_OM_ACL_KEY_BY_IP_MAC_FIELD_NUM          2
#define DHCPSNP_OM_ACL_KEY_BY_LPORT_TYPE_IP_MAC_FIELD_NUM    4
/* FUNCTION NAME : DHCPSNP_OM_Init
 * PURPOSE:
 *        Initialize DHCPSNP_OM used system resource.
 *
 * INPUT:
 *        None.
 *
 * OUTPUT:
 *        None.
 *
 * RETURN:
 *        None.
 *
 * NOTES:
 *        None.
 */
void DHCPSNP_OM_Init(void);

/*------------------------------------------------------------------------------
 * ROUTINE NAME  - DHCPSNP_OM_Clear
 *------------------------------------------------------------------------------
 * PURPOSE: Clear all stored data in OM
 * INPUT :  None
 * OUTPUT:  None
 * RETURN:  DHCPSNP_TYPE_OK/
 *          DHCPSNP_TYPE_FAIL/
 *          DHCPSNP_TYPE_INVALID_ARG
 * NOTES :  Database should be cleared when enter transition mode, 
 *          all table should be cleared but not destroyed.
 *------------------------------------------------------------------------------
 */
void DHCPSNP_OM_Clear(void);


/* FUNCTION NAME : DHCPSNP_OM_SetGlobalDhcpSnoopingStatus
 * PURPOSE:
 *      Enable/Diasble the global DHCP Snooping function.
 *
 * INPUT:
 *      status  --  DHCPSNP_TYPE_SNOOPING_GLOBAL_DISABLED or DHCPSNP_TYPE_SNOOPING_GLOBAL_ENABLED
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      DHCPSNP_TYPE_OK
 *      DHCPSNP_TYPE_FAIL
 *      DHCPSNP_TYPE_INVALID_ARG
 *
 * NOTES:
 *      1. In Global CLI: "ip dhcp snooping" or "no ip dhcp snooping"
 *      2. default is disabled.
 *      3. If set the global status to disabled, we have to clear all dynamic dhcp snooping binding entry.
 */
UI32_T DHCPSNP_OM_SetGlobalDhcpSnoopingStatus(UI8_T status);

/* FUNCTION NAME : DHCPSNP_OM_GetGlobalDhcpSnoopingStatus
 * PURPOSE:
 *      Get the global DHCP Snooping function status.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      status_p  --  DHCPSNP_TYPE_SNOOPING_GLOBAL_DISABLED or DHCPSNP_TYPE_SNOOPING_GLOBAL_ENABLED
 *
 * RETURN:
 *      DHCPSNP_TYPE_OK
 *      DHCPSNP_TYPE_FAIL
 *
 * NOTES:
 *      1. In Global CLI: show ip dhcp snooping
 */
UI32_T DHCPSNP_OM_GetGlobalDhcpSnoopingStatus(UI8_T *status_p);

/* FUNCTION NAME : DHCPSNP_OM_EnableDhcpSnoopingByVlan
 * PURPOSE:
 *      Enable dhcp snooping on specified vlan.
 *
 * INPUT:
 *      vid --  the vlan that will be added.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      DHCPSNP_TYPE_OK
 *      DHCPSNP_TYPE_FAIL
 *
 * NOTES:
 *      1.In Global CLI: ip dhcp snooping/vlan [a range]
 *      2.CLI call it by a "for loop" to handle the range. ex: ip dhcp snooping/vlan 100-200 
 */
UI32_T DHCPSNP_OM_EnableDhcpSnoopingByVlan(UI32_T vid);

/* FUNCTION NAME : DHCPSNP_OM_DisableDhcpSnoopingByVlan
 * PURPOSE:
 *      Disable dhcp snooping on specified vlan.
 *
 * INPUT:
 *      vid --  the vlan that will be deleted.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      DHCPSNP_TYPE_OK
 *      DHCPSNP_TYPE_FAIL
 *
 * NOTES:
 *      1.In Global CLI: no ip dhcp snooping/vlan [a range]
 *      2.CLI call it by a "for loop" to handle the range. ex: no ip dhcp snooping/vlan 100-200 
 */
UI32_T DHCPSNP_OM_DisableDhcpSnoopingByVlan(UI32_T vid);

/* FUNCTION NAME : DHCPSNP_OM_GetDhcpSnoopingStatusByVlan
 * PURPOSE:
 *      Get the DHCP Snooping status by the vlan id.
 *
 * INPUT:
 *      vid     --  the vlan id.
 *
 * OUTPUT:
 *      status_p  --  DHCPSNP_TYPE_SNOOPING_VLAN_DISABLED or DHCPSNP_TYPE_SNOOPING_VLAN_ENABLED.
 *
 * RETURN:
 *      DHCPSNP_TYPE_OK
 *      DHCPSNP_TYPE_FAIL
 *
 * NOTES:
 *      None.
 */
UI32_T DHCPSNP_OM_GetDhcpSnoopingStatusByVlan(UI32_T vid, UI8_T *status_p);

/* FUNCTION NAME : DHCPSNP_OM_GetNextRunningConfigDhcpSnoopingVlan
 * PURPOSE:
 *      Get the next user configured vlan id. 
 *
 * INPUT:
 *      vid_p --  current vlan id.
 *
 * OUTPUT:
 *      vid_p --  the next vlan id configured by the user.
 *
 * RETURN:
 *      DHCPSNP_TYPE_OK
 *      DHCPSNP_TYPE_FAIL
 *      DHCPSNP_TYPE_NO_MORE_ENTRY
 *
 * NOTES:
 *      1. The DHCP Snooping of the vlan may be enabled/disabled. \
 *         This API only return the vlan id configured by user. 
 *      2. The vlan bitmap in byte row is as follow:
 *         -----------------------------------
 *         | v0  v1  v2  v3  v4  v5  v6  v7  |  --> 0 th   row
 *         | v8  v9  v10 v11 v12 v13 v14 v15 |  --> 1 th   row
 *         |.................................|
 *         |........................... v4095|  --> 511 th row
 *
 */
UI32_T DHCPSNP_OM_GetNextRunningConfigDhcpSnoopingVlan(UI32_T *vid_p);

/* FUNCTION NAME : DHCPSNP_OM_SetAllVlanDefaultDhcpSnoopingStatus
 * PURPOSE:
 *      Set all vlan dhcp snooping status to default status.
 *
 * INPUT:
 *      status  --  the default status.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      DHCPSNP_TYPE_OK
 *      DHCPSNP_TYPE_FAIL
 *
 * NOTES:
 *      Only called by DHCPSNP_MGR_EnterMasterMode
 */
UI32_T DHCPSNP_OM_SetAllVlanDefaultDhcpSnoopingStatus(UI8_T status);

/* FUNCTION NAME : DHCPSNP_OM_SetAllPortDefaultCid
 * PURPOSE:
 *      Set all port cid mode to default value.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      DHCPSNP_TYPE_OK
 *      DHCPSNP_TYPE_FAIL
 *
 * NOTES:
 *      Only called by DHCPSNP_MGR_EnterMasterMode
 */
UI32_T DHCPSNP_OM_SetAllPortDefaultCid(void);

/* FUNCTION NAME : DHCPSNP_OM_SetIpSourceGuardMode
 * PURPOSE:
 *      Set the filtering mode of the ip source guard to DHCPSNP_TYPE_IP_SRC_GRD_SIP_FILTER,
 *      DHCPSNP_TYPE_IP_SRC_GRD_SIP_AND_MAC_FILTER or DHCPSNP_TYPE_IP_SRC_GRD_DISABLED.
 *
 * INPUT:
 *      lport_ifindex   --  the logic port to set the the filtering mode
 *      filter_mode     --  the filtering mode of the ip source guard
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      DHCPSNP_TYPE_OK
 *      DHCPSNP_TYPE_FAIL
 *      DHCPSNP_TYPE_INVALID_ARG
 *
 * NOTES:
 *      1.the per port CLI: ip/verify/source, ip/verify/source-and-mac or no ip/verify.
 */
UI32_T DHCPSNP_OM_SetIpSourceGuardMode(UI32_T lport_ifindex, UI8_T filter_mode);

/* FUNCTION NAME : DHCPSNP_OM_GetIpSourceGuardMode
 * PURPOSE:
 *      Get the filtering mode of the ip source guard in a port.
 *
 * INPUT:
 *      lport_ifindex   --  the logic port to get the the filtering mode
 *
 * OUTPUT:
 *      filter_mode_p     --  the filtering mode of the ip source guard shall be DHCPSNP_TYPE_IP_SRC_GRD_SIP_FILTER,
 *                          DHCPSNP_TYPE_IP_SRC_GRD_SIP_AND_MAC_FILTER or DHCPSNP_TYPE_IP_SRC_GRD_DISABLED.
 *
 * RETURN:
 *      DHCPSNP_TYPE_OK
 *      DHCPSNP_TYPE_FAIL
 *      DHCPSNP_TYPE_INVALID_ARG
 *
 * NOTES:
 *      None.
 */
UI32_T DHCPSNP_OM_GetIpSourceGuardMode(UI32_T lport_ifindex, UI8_T *filter_mode_p);

/* FUNCTION NAME : DHCPSNP_OM_SetVerifyMacAddressStatus
 * PURPOSE:
 *      Enable/Disable verify MAC address funtion.
 *
 * INPUT:
 *      status  --  DHCPSNP_TYPE_VERIFY_MAC_ADDRESS_ENABLED or DHCPSNP_TYPE_VERIFY_MAC_ADDRESS_DISABLED
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      DHCPSNP_TYPE_OK
 *      DHCPSNP_TYPE_FAIL
 *      DHCPSNP_TYPE_INVALID_ARG
 *
 * NOTES:
 *      1.Default value is enabled.
 *      2.In Global CLI: "ip dhcp snooping/verify/mac-address" or "no ip dhcp snooping/verify/mac-address"
 */
UI32_T DHCPSNP_OM_SetVerifyMacAddressStatus(UI8_T status);

/* FUNCTION NAME : DHCPSNP_OM_GetVerifyMacAddressStatus
 * PURPOSE:
 *      Get the status of verify MAC address function.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      status_p  --  DHCPSNP_TYPE_VERIFY_MAC_ADDRESS_ENABLED or DHCPSNP_TYPE_VERIFY_MAC_ADDRESS_DISABLED
 *
 * RETURN:
 *      DHCPSNP_TYPE_OK
 *      DHCPSNP_TYPE_FAIL
 *
 * NOTES:
 *      1.Default value is enabled.
 */
UI32_T DHCPSNP_OM_GetVerifyMacAddressStatus(UI8_T *status_p);

/* FUNCTION NAME : DHCPSNP_OM_SetPortTrustStatus
 * PURPOSE:
 *      Set the trsut status of the port.
 *
 * INPUT:
 *      lport_ifindex   --  the port to be set the trust status.
 *      status          --  DHCPSNP_TYPE_PORT_UNTRUSTED or DHCPSNP_TYPE_PORT_TRUSTED
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      DHCPSNP_TYPE_OK
 *      DHCPSNP_TYPE_FAIL
 *      DHCPSNP_TYPE_INVALID_ARG
 *
 * NOTES:
 *      1.In per port CLI: "ip dhcp snooping/trust" or "no ip dhcp snooping/trust";
 */
UI32_T DHCPSNP_OM_SetPortTrustStatus(UI32_T lport_ifindex, UI8_T status);

/* FUNCTION NAME : DHCPSNP_OM_GetPortTrustStatus
 * PURPOSE:
 *      Get the trsut status of the port.
 *
 * INPUT:
 *      lport_ifindex   --  the port to be get the trust status.
 *
 * OUTPUT:
 *      status_p          --  DHCPSNP_TYPE_PORT_UNTRUSTED or DHCPSNP_TYPE_PORT_TRUSTED
 *
 * RETURN:
 *      DHCPSNP_TYPE_OK
 *      DHCPSNP_TYPE_FAIL
 *      DHCPSNP_TYPE_INVALID_ARG
 *
 * NOTES:
 *      None.
 */
UI32_T DHCPSNP_OM_GetPortTrustStatus(UI32_T lport_ifindex, UI8_T *status_p);

/* FUNCTION NAME : DHCPSNP_OM_GetPortInfo
 * PURPOSE:
 *      Get the port information.
 *
 * INPUT:
 *      lport_ifindex    --  the logic port to get the port info.
 *
 * OUTPUT:
 *      port_info_p      --  the port information.
 *
 * RETURN:
 *      DHCPSNP_TYPE_OK
 *      DHCPSNP_TYPE_FAIL
 *      DHCPSNP_TYPE_INVALID_ARG
 *
 * NOTES:
 *      None.
 */
UI32_T DHCPSNP_OM_GetPortInfo(UI32_T lport_ifindex, DHCPSNP_TYPE_PortInfo_T *port_info_p);

/* FUNCTION NAME : DHCPSNP_OM_AddBindingEntry
 * PURPOSE:
 *      Add a binding entry to the the binding table.
 *
 * INPUT:
 *      binding_entry   --  2 kinds of static bindging entry is added into the binding table.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      DHCPSNP_TYPE_OK
 *      DHCPSNP_TYPE_FAIL
 *
 * NOTES:
 *      None.
 */
UI32_T DHCPSNP_OM_AddBindingEntry(DHCPSNP_TYPE_BindingEntry_T binding_entry);

/* FUNCTION NAME : DHCPSNP_OM_DeleteBindingEntry
 * PURPOSE:
 *      Delete a binding entry from the binding table.
 *
 * INPUT:
 *      binding_entry   --  the bindging entry is deleted from the binding table.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      DHCPSNP_TYPE_OK
 *      DHCPSNP_TYPE_FAIL
 *
 * NOTES:
 *      None.
 */
UI32_T DHCPSNP_OM_DeleteBindingEntry(DHCPSNP_TYPE_BindingEntry_T binding_entry);

/* FUNCTION NAME : DHCPSNP_OM_GetBindingEntry
 * PURPOSE:
 *      Get the binding entry.
 *
 * INPUT:
 *      key_index       --  the key type to access HISAM table.
 *
 * OUTPUT:
 *      binding_entry_p   --  the binding entry
 *
 * RETURN:
 *      DHCPSNP_TYPE_OK
 *      DHCPSNP_TYPE_FAIL
 *
 * NOTES:
 *      None.
 */
UI32_T DHCPSNP_OM_GetBindingEntry(UI32_T key_index, DHCPSNP_TYPE_BindingEntry_T *binding_entry_p);

/* FUNCTION NAME : DHCPSNP_OM_GetNextBindingEntry
 * PURPOSE:
 *      Get the next binding entry.
 *
 * INPUT:
 *      key_index       --  the key type to access HISAM table.
 *
 * OUTPUT:
 *      binding_entry_p   --  the next binding entry
 *
 * RETURN:
 *      DHCPSNP_TYPE_OK
 *      DHCPSNP_TYPE_FAIL
 *
 * NOTES:
 *      None.
 */
UI32_T DHCPSNP_OM_GetNextBindingEntry(UI32_T key_index, DHCPSNP_TYPE_BindingEntry_T *binding_entry_p);

/* FUNCTION NAME : DHCPSNP_OM_IncreaseTotalForwardedPktCounter
 * PURPOSE:
 *      Increase the total forwarded packet counter.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      DHCPSNP_TYPE_OK
 *      DHCPSNP_TYPE_FAIL
 *
 * NOTES:
 *      None.
 */
UI32_T DHCPSNP_OM_IncreaseTotalForwardedPktCounter();

/* FUNCTION NAME : DHCPSNP_OM_GetTotalForwardedPktCounter
 * PURPOSE:
 *      Get the total forwarded packet counter.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      total_fwd_counter_p   --  the total forwarded packet counter
 *
 * RETURN:
 *      DHCPSNP_TYPE_OK
 *      DHCPSNP_TYPE_FAIL
 *
 * NOTES:
 *      1.When the DHCP packets pass the filtering criteria, then we will increase the counter and forward it.
 */
/* 2007-09, Joseph, for ES3528M-SFP-FLF-38-00112 */
UI32_T DHCPSNP_OM_GetTotalForwardedPktCounter(UI32_T *total_fwd_counter_p); 


/*------------------------------------------------------------------------------
 * ROUTINE NAME  - DHCPSNP_OM_SetPortFilterTableMode
 *------------------------------------------------------------------------------
 * PURPOSE: Set port filter table mode 
 * INPUT :  ifindex   -- logical port ifindex
 *          mode      -- filter table mode 
 * OUTPUT:  None
 * RETURN:  DHCPSNP_TYPE_OK/
 *          DHCPSNP_TYPE_FAIL/
 *          DHCPSNP_TYPE_INVALID_ARG
 * NOTES :  mode{DHCPSNP_TYPE_FILTER_TABLE_ACL_MODE/
 *               DHCPSNP_TYPE_FILTER_TABLE_MAC_MODE}
 *------------------------------------------------------------------------------
 */
UI32_T DHCPSNP_OM_SetPortFilterTableMode(UI32_T ifindex, UI8_T mode);

/*------------------------------------------------------------------------------
 * ROUTINE NAME  - DHCPSNP_OM_GetPortFilterTableMode
 *------------------------------------------------------------------------------
 * PURPOSE: Get port filter table mode 
 * INPUT :  ifindex   -- logical port ifindex
 * OUTPUT:  mode_p    -- filter table mode
 * RETURN:  DHCPSNP_TYPE_OK/
 *          DHCPSNP_TYPE_FAIL/
 *          DHCPSNP_TYPE_INVALID_ARG
 * NOTES :  mode{DHCPSNP_TYPE_FILTER_TABLE_ACL_MODE/
 *               DHCPSNP_TYPE_FILTER_TABLE_MAC_MODE}
 *------------------------------------------------------------------------------
 */
UI32_T DHCPSNP_OM_GetPortFilterTableMode(UI32_T ifindex, UI8_T *mode_p);


/* begin 2006-06, Joseph */
#if (SYS_CPNT_DHCPSNP_INFORMATION_OPTION == TRUE)	
/*------------------------------------------------------------------------------
 * FUNCTION NAME - DHCPSNP_OM_SetOption82Status                               
 *------------------------------------------------------------------------------
 * PURPOSE  : Set DHCPSNP option82 status : enable/disable
 * INPUT    : status you want to set. 1 :DHCPSNP_TYPE_OPTION82_ENABLED, 2 :DHCPSNP_TYPE_OPTION82_DISABLED
 * OUTPUT   : none
 * RETURN   : DHCPSNP_TYPE_OK : If success
 *            DHCPSNP_TYPE_FAIL:
 * NOTES    : none                                                              
 *------------------------------------------------------------------------------*/ 
 UI32_T DHCPSNP_OM_SetOption82Status(UI8_T status);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - DHCPSNP_OM_GetOption82Status                                
 *------------------------------------------------------------------------------
 * PURPOSE  : Get DHCPSNP option82 status  : enable/disable
 * INPUT    : none
 * OUTPUT   : current status. 1 :DHCPSNP_TYPE_OPTION82_ENABLED, 2 :DHCPSNP_TYPE_OPTION82_DISABLED
 * RETURN   : DHCPSNP_TYPE_OK : If success
 *            DHCPSNP_TYPE_FAIL:
 * NOTES    : none                                                              
 *------------------------------------------------------------------------------*/ 
 UI32_T  DHCPSNP_OM_GetOption82Status(UI8_T *status_p);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - DHCPSNP_OM_SetOption82Policy                         
 *------------------------------------------------------------------------------
 * PURPOSE  : Set DHCPSNP option82 policy : drop/replace/keep
 * INPUT    : policy you want to set. 1 :DHCPSNP_TYPE_OPTION82_POLICY_DROP, 2 :DHCPSNP_TYPE_OPTION82_POLICY_REPLACE
 *                 3:DHCPSNP_TYPE_OPTION82_POLICY_KEEP
 * OUTPUT   : none
 * RETURN   : DHCPSNP_TYPE_OK : If success
 *            DHCPSNP_TYPE_FAIL:
 * NOTES    : none                                                              
 *------------------------------------------------------------------------------*/ 
 UI32_T DHCPSNP_OM_SetOption82Policy(UI8_T policy);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - DHCPSNP_OM_GetOption82Policy                               
 *------------------------------------------------------------------------------
 * PURPOSE  : Get DHCPSNP option82 Policy  : 
 * INPUT    : none
 * OUTPUT   : current option82 policy.1 :DHCPSNP_TYPE_OPTION82_POLICY_DROP, 2 :DHCPSNP_TYPE_OPTION82_POLICY_REPLACE
 *                 3:DHCPSNP_TYPE_OPTION82_POLICY_KEEP
 * RETURN   : DHCPSNP_TYPE_OK : If success
 *            DHCPSNP_TYPE_FAIL:
 * NOTES    : none                                                              
 *------------------------------------------------------------------------------*/ 
 UI32_T  DHCPSNP_OM_GetOption82Policy(UI8_T *policy_p);
 
/*------------------------------------------------------------------------------
 * FUNCTION NAME - DHCPSNP_OM_SetOption82RidMode                        
 *------------------------------------------------------------------------------
 * PURPOSE  : Set DHCPSNP option82 remote id mode 
 * INPUT    : mode   -- DHCP_OPTION82_RID_MAC_HEX 
 *                      DHCP_OPTION82_RID_MAC_ASCII
 *                      DHCP_OPTION82_RID_IP_HEX
 *                      DHCP_OPTION82_RID_IP_ASCII
 *                      DHCP_OPTION82_RID_CONFIGURED_STRING
 * OUTPUT   : none
 * RETURN   : DHCPSNP_TYPE_OK : If success
 *            DHCPSNP_TYPE_FAIL:
 * NOTES    : none                                                              
 *------------------------------------------------------------------------------*/ 
 UI32_T DHCPSNP_OM_SetOption82RidMode(UI8_T mode);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - DHCPSNP_OM_GetOption82RidMode                        
 *------------------------------------------------------------------------------
 * PURPOSE  : Get DHCPSNP option82 remote id mode 
 * INPUT    : none
 * OUTPUT   : mode   -- DHCP_OPTION82_RID_MAC_HEX 
 *                      DHCP_OPTION82_RID_MAC_ASCII
 *                      DHCP_OPTION82_RID_IP_HEX
 *                      DHCP_OPTION82_RID_IP_ASCII
 *                      DHCP_OPTION82_RID_CONFIGURED_STRING
 * RETURN   : DHCPSNP_TYPE_OK : If success
 *            DHCPSNP_TYPE_FAIL:
 * NOTES    : none                                                              
 *------------------------------------------------------------------------------*/ 
 UI32_T DHCPSNP_OM_GetOption82RidMode(UI8_T *mode);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - DHCPSNP_OM_SetOption82RidValue                       
 *------------------------------------------------------------------------------
 * PURPOSE  : Set DHCPSNP option82 remote id value
 * INPUT    : rid_value_p
 * OUTPUT   : none
 * RETURN   : DHCPSNP_TYPE_OK : If success
 *            DHCPSNP_TYPE_FAIL:
 * NOTES    : none                                                              
 *------------------------------------------------------------------------------*/ 
 UI32_T DHCPSNP_OM_SetOption82RidValue(UI8_T *rid_value_p);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - DHCPSNP_OM_GetOption82RidValue                       
 *------------------------------------------------------------------------------
 * PURPOSE  : Get DHCPSNP option82 remote id value
 * INPUT    : rid_value_p
 * OUTPUT   : none
 * RETURN   : DHCPSNP_TYPE_OK : If success
 *            DHCPSNP_TYPE_FAIL:
 * NOTES    : none                                                              
 *------------------------------------------------------------------------------*/ 
 UI32_T DHCPSNP_OM_GetOption82RidValue(UI8_T *rid_value_p);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - DHCPSNP_OM_SetOption82PortCidMode                        
 *------------------------------------------------------------------------------
 * PURPOSE  : Set DHCPSNP option82 circuit id mode : mac/ip address
 * INPUT    : lport_ifindex  -- port ifindex
 *            mode           -- DHCPSNP_TYPE_OPTION82_CID_VLAN_UNIT_PORT
 *                              DHCPSNP_TYPE_OPTION82_CID_CONFIGURED_STRING
 * OUTPUT   : none
 * RETURN   : DHCPSNP_TYPE_OK : If success
 *            DHCPSNP_TYPE_FAIL:
 * NOTES    : none                                                              
 *------------------------------------------------------------------------------*/ 
 UI32_T DHCPSNP_OM_SetOption82PortCidMode(UI32_T lport_ifindex, UI8_T mode);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - DHCPSNP_OM_GetOption82PortCidMode                              
 *------------------------------------------------------------------------------
 * PURPOSE  : Get DHCPSNP option82 circuit id mode  : 
 * INPUT    : lport_ifindex  -- port ifindex
 * OUTPUT   : current option82 circuit id mode.DHCPSNP_TYPE_OPTION82_CID_VLAN_UNIT_PORT/DHCPSNP_TYPE_OPTION82_CID_CONFIGURED_STRING
 * RETURN   : DHCPSNP_TYPE_OK : If success
 *            DHCPSNP_TYPE_FAIL:
 * NOTES    : none                                                              
 *------------------------------------------------------------------------------*/ 
 UI32_T  DHCPSNP_OM_GetOption82PortCidMode(UI32_T lport_ifindex, UI8_T *mode_p);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - DHCPSNP_OM_SetOption82PortCidValue                        
 *------------------------------------------------------------------------------
 * PURPOSE  : Set DHCPSNP option82 circuit id value
 * INPUT    : lport_ifindex  -- port ifindex
 *            cid_value_p    -- configured string
 * OUTPUT   : none
 * RETURN   : DHCPSNP_TYPE_OK : If success
 *            DHCPSNP_TYPE_FAIL:
 * NOTES    : none                                                              
 *------------------------------------------------------------------------------*/ 
 UI32_T DHCPSNP_OM_SetOption82PortCidValue(UI32_T lport_ifindex, UI8_T *cid_value_p);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - DHCPSNP_OM_GetOption82PortCidValue                        
 *------------------------------------------------------------------------------
 * PURPOSE  : Set DHCPSNP option82 circuit id value
 * INPUT    : lport_ifindex  -- port ifindex
 *            
 * OUTPUT   : cid_value_p    -- configured string
 * RETURN   : DHCPSNP_TYPE_OK : If success
 *            DHCPSNP_TYPE_FAIL:
 * NOTES    : none                                                              
 *------------------------------------------------------------------------------*/ 
 UI32_T DHCPSNP_OM_GetOption82PortCidValue(UI32_T lport_ifindex, UI8_T *cid_value_p);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - DHCPSNP_OM_SetOption82Format                              
 *------------------------------------------------------------------------------
 * PURPOSE  : Set DHCPSNP option82 subtype format  : 
 * INPUT    : subtype_format     --  subtype format(TRUE/FALSE)
 * OUTPUT   : current option82 subtype format.(TRUE/FALSE)
 * RETURN   : DHCPSNP_TYPE_OK : If success
 *            DHCPSNP_TYPE_FAIL:
 * NOTES    : none                                                              
 *------------------------------------------------------------------------------*/ 
 UI32_T  DHCPSNP_OM_SetOption82Format(BOOL_T subtype_format);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - DHCPSNP_OM_GetOption82Format                              
 *------------------------------------------------------------------------------
 * PURPOSE  : Get DHCPSNP option82 subtype format  : 
 * INPUT    : none
 * OUTPUT   : current option82 subtype format.(TRUE/FALSE)
 * RETURN   : DHCPSNP_TYPE_OK : If success
 *            DHCPSNP_TYPE_FAIL:
 * NOTES    : none                                                              
 *------------------------------------------------------------------------------*/ 
 UI32_T  DHCPSNP_OM_GetOption82Format(BOOL_T *subtype_format);

#endif
/* end 2006-06 */

#if 0
/* FUNCTION NAME : DHCPSNP_OM_IncreaseTotalDroppedPktCounter
 * PURPOSE:
 *      Increase the total dropped packet counter.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      DHCPSNP_TYPE_OK
 *      DHCPSNP_TYPE_FAIL
 *
 * NOTES:
 *      None.
 */
UI32_T DHCPSNP_OM_IncreaseTotalDroppedPktCounter();

/* FUNCTION NAME : DHCPSNP_OM_GetTotalDroppedPktCounter
 * PURPOSE:
 *      Get the total dropped packet counter.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      total_drp_counter_p   --  the total dropped packet counter
 *
 * RETURN:
 *      DHCPSNP_TYPE_OK
 *      DHCPSNP_TYPE_FAIL
 *
 * NOTES:
 *      1.When the DHCP packets don't pass the filtering criteria, then we will increase the counter and drop it.
 *      2.When the IP packets don't pass the ip source guard, then we will increase the counter and drop it.
 */
/* 2007-09, Joseph, for ES3528M-SFP-FLF-38-00112 */
UI32_T DHCPSNP_OM_GetTotalDroppedPktCounter(UI32_T *total_drp_counter_p); 

#endif
/* FUNCTION NAME : DHCPSNP_OM_IncreaseUntrustedDroppedPktCounter
 * PURPOSE:
 *      Increase the untrusted dropped packet counter.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      DHCPSNP_TYPE_OK
 *      DHCPSNP_TYPE_FAIL
 *
 * NOTES:
 *      None.
 */
UI32_T DHCPSNP_OM_IncreaseUntrustedDroppedPktCounter();

/* FUNCTION NAME : DHCPSNP_OM_GetUntrustedDroppedPktCounter
 * PURPOSE:
 *      Get the untrusted dropped packet counter.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      untrust_drp_counter_p   --  the dropped packet counter for the untrusted port
 *
 * RETURN:
 *      DHCPSNP_TYPE_OK
 *      DHCPSNP_TYPE_FAIL
 *
 * NOTES:
 *      1.When the DHCP packets don't pass the filtering criteria, then we will increase the counter and drop it.
 *      2.The counter also equals to the number of the DHCP packets dropped by DHCP Snooping filtering criteria.
 */
/* 2007-09, Joseph, for ES3528M-SFP-FLF-38-00112 */
UI32_T DHCPSNP_OM_GetUntrustedDroppedPktCounter(UI32_T *untrust_drp_counter_p);


/*------------------------------------------------------------------------------
 * ROUTINE NAME  - DHCPSNP_OM_IncreaseAddDynamicBindingFailDropCounter
 *------------------------------------------------------------------------------
 * PURPOSE: Increase drop counter due to fail to add dynamic binding entry
 * INPUT :  None
 * OUTPUT:  None
 * RETURN:  DHCPSNP_TYPE_OK
 * NOTES :  None
 *------------------------------------------------------------------------------
 */
UI32_T DHCPSNP_OM_IncreaseAddDynamicBindingFailDropCounter(void);

/*------------------------------------------------------------------------------
 * ROUTINE NAME  - DHCPSNP_OM_IncreaseNoFwdPortDropCounter
 *------------------------------------------------------------------------------
 * PURPOSE: Increase drop counter due to no forward port
 * INPUT :  None
 * OUTPUT:  None
 * RETURN:  DHCPSNP_TYPE_OK
 * NOTES :  None
 *------------------------------------------------------------------------------
 */
UI32_T DHCPSNP_OM_IncreaseNoFwdPortDropCounter(void);

/*------------------------------------------------------------------------------
 * ROUTINE NAME  - DHCPSNP_OM_GetStatisticCounter
 *------------------------------------------------------------------------------
 * PURPOSE: Get statistic counter structure
 * INPUT :  counter		--	statistic counter pointer
 * OUTPUT:  counter 	-- 	sattistic counter
 * RETURN:  DHCPSNP_TYPE_INVALID_ARG/DHCPSNP_TYPE_OK
 * NOTES :  None
 *------------------------------------------------------------------------------
 */
UI32_T DHCPSNP_OM_GetStatisticCounter(DHCPSNP_TYPE_StatisticCounter_T *counter);

/*------------------------------------------------------------------------------
 * ROUTINE NAME  - DHCPSNP_OM_ClearStatisticCounter
 *------------------------------------------------------------------------------
 * PURPOSE: Clear statistic counter
 * INPUT :  None
 * OUTPUT:  None
 * RETURN:  DHCPSNP_TYPE_OK
 * NOTES :  None
 *------------------------------------------------------------------------------
 */
UI32_T DHCPSNP_OM_ClearStatisticCounter(void);

/*------------------------------------------------------------------------------
 * ROUTINE NAME  - DHCPSNP_OM_GetPortEntryCount
 *------------------------------------------------------------------------------
 * PURPOSE: Get port interface entry Count 
 * INPUT :  ifindex   -- logical port ifindex
 * OUTPUT:  count_p   -- entry number  
 * RETURN:  DHCPSNP_TYPE_OK/
 *          DHCPSNP_TYPE_FAIL/
 *          DHCPSNP_TYPE_INVALID_ARG
 * NOTES :  This api only get port in ACL mode entry count 
 *------------------------------------------------------------------------------
 */
UI32_T DHCPSNP_OM_GetPortEntryCount(UI32_T ifindex, UI32_T *count_p);

/*------------------------------------------------------------------------------
 * ROUTINE NAME  - DHCPSNP_OM_GetPortEntryLimit
 *------------------------------------------------------------------------------
 * PURPOSE: Get port interface binding entry limit 
 * INPUT :  ifindex    -- logical port ifindex
 *          mode       -- filter table mode 
 * OUTPUT:  limit_p    -- entry limit 
 * RETURN:  DHCPSNP_TYPE_OK/
 *          DHCPSNP_TYPE_FAIL/
 *          DHCPSNP_TYPE_INVALID_ARG
 * NOTES:
 *------------------------------------------------------------------------------
 */
UI32_T DHCPSNP_OM_GetPortEntryLimit(UI32_T ifindex, UI8_T mode, UI32_T *limit_p);

/*------------------------------------------------------------------------------
 * ROUTINE NAME  - DHCPSNP_OM_SetPortEntryLimit
 *------------------------------------------------------------------------------
 * PURPOSE: Set port interface binding entry limit 
 * INPUT :  ifindex    -- logical port ifindex
 *          mode       -- filter table mode 
 *          limit      -- entry limit
 * OUTPUT:  None
 * RETURN:  DHCPSNP_TYPE_OK/
 *          DHCPSNP_TYPE_FAIL/
 *          DHCPSNP_TYPE_INVALID_ARG
 * NOTES:
 *------------------------------------------------------------------------------
 */
UI32_T DHCPSNP_OM_SetPortEntryLimit(UI32_T ifindex, UI8_T mode, UI32_T limit);

#if (SYS_CPNT_DHCPSNP_SYSTEM_RATELIMIT == TRUE)
/*------------------------------------------------------------------------------
 * ROUTINE NAME  - DHCPSNP_OM_SetGlobalRateLimit
 *------------------------------------------------------------------------------
 * PURPOSE: configure system-wise dhcp packet rate limit
 * INPUT :  rate	--	the limited rate
 * OUTPUT:  none
 * RETURN:  DHCPSNP_TYPE_OK/
 *          DHCPSNP_TYPE_FAIL/
 *          DHCPSNP_TYPE_INVALID_ARG
 * NOTES :  if rate = 0, that means no rate limit.
 *------------------------------------------------------------------------------
 */
UI32_T DHCPSNP_OM_SetGlobalRateLimit(UI32_T rate);

/*------------------------------------------------------------------------------
 * ROUTINE NAME  - DHCPSNP_OM_GetGlobalRateLimit
 *------------------------------------------------------------------------------
 * PURPOSE: configure system-wise dhcp packet rate limit
 * INPUT :  rate	--	the limited rate
 * OUTPUT:  rate	--	the limited rate
 * RETURN:  DHCPSNP_TYPE_OK/
 *          DHCPSNP_TYPE_FAIL/
 *          DHCPSNP_TYPE_INVALID_ARG
 * NOTES :  if rate = 0, that means no rate limit.
 *------------------------------------------------------------------------------
 */
UI32_T DHCPSNP_OM_GetGlobalRateLimit(UI32_T *rate);

/*------------------------------------------------------------------------------
 * ROUTINE NAME  - DHCPSNP_OM_IncreaseGlobalPktCount
 *------------------------------------------------------------------------------
 * PURPOSE: Increase global rate calculation packet count
 * INPUT :  none
 * OUTPUT:  none
 * RETURN:  DHCPSNP_TYPE_OK
 * NOTES :  none
 *------------------------------------------------------------------------------
 */
UI32_T DHCPSNP_OM_IncreaseGlobalPktCount(void);

/*------------------------------------------------------------------------------
 * ROUTINE NAME  - DHCPSNP_OM_ResetGlobalPktCount
 *------------------------------------------------------------------------------
 * PURPOSE: Reset global rate calculation packet count to zero
 * INPUT :  none
 * OUTPUT:  none
 * RETURN:  DHCPSNP_TYPE_OK
 * NOTES :  none
 *------------------------------------------------------------------------------
 */
UI32_T DHCPSNP_OM_ResetGlobalPktCount(void);

/*------------------------------------------------------------------------------
 * ROUTINE NAME  - DHCPSNP_OM_GetGlobalPktCount
 *------------------------------------------------------------------------------
 * PURPOSE: Get global rate calculation packet count
 * INPUT :  count	--	packet count
 * OUTPUT:  count	--	packet count
 * RETURN:  DHCPSNP_TYPE_OK/
 *			DHCPSNP_TYPE_INVALID_ARG
 * NOTES :  none
 *------------------------------------------------------------------------------
 */
UI32_T DHCPSNP_OM_GetGlobalPktCount(UI32_T *count);


/*------------------------------------------------------------------------------
 * ROUTINE NAME  - DHCPSNP_OM_SetGlobalLastRateResult
 *------------------------------------------------------------------------------
 * PURPOSE: Set calculated rate in last time interval
 * INPUT :  rate	--	the calculated rate
 * OUTPUT:  none
 * RETURN:  DHCPSNP_TYPE_OK
 * NOTES :  none
 *------------------------------------------------------------------------------
 */
UI32_T DHCPSNP_OM_SetGlobalLastRateResult(UI32_T rate);

/*------------------------------------------------------------------------------
 * ROUTINE NAME  - DHCPSNP_OM_GetGlobalLastRateResult
 *------------------------------------------------------------------------------
 * PURPOSE: Get calculated rate in last time interval
 * INPUT :  rate	--	the calculated rate
 * OUTPUT:  rate	--	the calculated rate
 * RETURN:  DHCPSNP_TYPE_OK/
 *			DHCPSNP_TYPE_INVALID_ARG
 * NOTES :  none
 *------------------------------------------------------------------------------
 */

UI32_T DHCPSNP_OM_GetGlobalLastRateResult(UI32_T *rate);
#endif

#if (SYS_CPNT_DHCPSNP_INFORMATION_OPTION_TR101_FORMAT == TRUE)
/*------------------------------------------------------------------------------
 * ROUTINE NAME  - DHCPSNP_OM_SetBoardId
 *------------------------------------------------------------------------------
 * PURPOSE: Set TR101 format board id
 * INPUT :  board_id    --  board identifier
 * OUTPUT:  none
 * RETURN:  DHCPSNP_TYPE_OK			
 * NOTES :  none
 *------------------------------------------------------------------------------
 */
UI32_T DHCPSNP_OM_SetBoardId(UI8_T board_id);

/*------------------------------------------------------------------------------
 * ROUTINE NAME  - DHCPSNP_OM_GetBoardId
 *------------------------------------------------------------------------------
 * PURPOSE: Get TR101 format board id
 * INPUT :  board_id    --  board identifier
 * OUTPUT:  board_id    --  board identifier
 * RETURN:  DHCPSNP_TYPE_OK/
 *          DHCPSNP_TYPE_INVALID_ARG
 * NOTES :  none
 *------------------------------------------------------------------------------
 */
UI32_T DHCPSNP_OM_GetBoardId(UI8_T *board_id);
#endif

#if (SYS_CPNT_DHCPSNP_INFORMATION_OPTION_RID_SUB_OPTION == TRUE)
/*------------------------------------------------------------------------------
 * ROUTINE NAME  - DHCPSNP_OM_SetOptionDelimiter
 *------------------------------------------------------------------------------
 * PURPOSE: Set option delimiter
 * INPUT :  delimiter   --  option delimiter
 * OUTPUT:  none
 * RETURN:  DHCPSNP_TYPE_OK			
 * NOTES :  none
 *------------------------------------------------------------------------------
 */
UI32_T DHCPSNP_OM_SetOptionDelimiter(UI8_T delimiter);

/*------------------------------------------------------------------------------
 * ROUTINE NAME  - DHCPSNP_OM_GetOptionDelimiter
 *------------------------------------------------------------------------------
 * PURPOSE: Get option delimiter
 * INPUT :  delimiter   --  option delimiter
 * OUTPUT:  delimiter   --  option delimiter
 * RETURN:  DHCPSNP_TYPE_OK/
 *          DHCPSNP_TYPE_INVALID_ARG
 * NOTES :  none
 *------------------------------------------------------------------------------
 */
UI32_T DHCPSNP_OM_GetOptionDelimiter(UI8_T *delimiter);
#endif

#if (SYS_CPNT_DHCP_RELAY_OPTION82 == TRUE)
/*------------------------------------------------------------------------------
 * ROUTINE NAME  - DHCPSNP_OM_SetL2RelayForwarding
 *------------------------------------------------------------------------------
 * PURPOSE: Set L2 forwarding flag for DHCPSNP to know if the packet needs to forward
 * INPUT :  forwarding  -   needs forwarding or not
 * OUTPUT:  None
 * RETURN:  DHCPSNP_TYPE_OK/
 *          DHCPSNP_TYPE_FAIL
 * NOTES :  None
 *------------------------------------------------------------------------------
 */
UI32_T DHCPSNP_OM_SetL2RelayForwarding(BOOL_T forwarding);

/*------------------------------------------------------------------------------
 * ROUTINE NAME  - DHCPSNP_OM_GetL2RelayForwarding
 *------------------------------------------------------------------------------
 * PURPOSE: Get L2 forwarding flag for DHCPSNP to know if the packet needs to forward
 * INPUT :  None
 * OUTPUT:  forwarindg  -   needs forward or not
 * RETURN:  DHCPSNP_TYPE_OK/
 *          DHCPSNP_TYPE_FAIL
 * NOTES :  None
 *------------------------------------------------------------------------------
 */
UI32_T DHCPSNP_OM_GetL2RelayForwarding(BOOL_T *forwarding);
#endif  //SYS_CPNT_DHCP_RELAY_OPTION82

#if (SYS_CPNT_DHCP_RELAY == TRUE)
/*------------------------------------------------------------------------------
 * ROUTINE NAME  - DHCPSNP_OM_SetL3RelayForwarding
 *------------------------------------------------------------------------------
 * PURPOSE: Set L3 forwarding flag for DHCPSNP to know if the packet needs to forward
 * INPUT :  vid         -   vlan id
 *          forwarding  -   needs forward or not
 * OUTPUT:  None
 * RETURN:  DHCPSNP_TYPE_OK/
 *          DHCPSNP_TYPE_FAIL
 * NOTES :  None
 *------------------------------------------------------------------------------
 */
UI32_T DHCPSNP_OM_SetL3RelayForwarding(UI32_T vid, BOOL_T forwarding);

/*------------------------------------------------------------------------------
 * ROUTINE NAME  - DHCPSNP_OM_SetAllVlansL3RelayForwarding
 *------------------------------------------------------------------------------
 * PURPOSE: Set all L3 forwarding flag for DHCPSNP to know if the packet needs to forward
 * INPUT :  forwarding  -   needs forward or not
 * OUTPUT:  None
 * RETURN:  DHCPSNP_TYPE_OK/
 *          DHCPSNP_TYPE_FAIL
 * NOTES :  None
 *------------------------------------------------------------------------------
 */
UI32_T DHCPSNP_OM_SetAllVlansL3RelayForwarding(BOOL_T forwarding);

/*------------------------------------------------------------------------------
 * ROUTINE NAME  - DHCPSNP_OM_GetL3RelayForwarding
 *------------------------------------------------------------------------------
 * PURPOSE: Get L3 forwarding flag for DHCPSNP to know if the packet needs to forward
 * INPUT :  vid         -   vlan id
 * OUTPUT:  forwarding  -   needs forward or not
 * RETURN:  DHCPSNP_TYPE_OK/
 *          DHCPSNP_TYPE_FAIL
 * NOTES :  None
 *------------------------------------------------------------------------------
 */
UI32_T DHCPSNP_OM_GetL3RelayForwarding(UI32_T vid, BOOL_T *forwarding);
#endif  //SYS_CPNT_DHCP_RELAY
#endif

