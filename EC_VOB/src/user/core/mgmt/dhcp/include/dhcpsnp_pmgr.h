/* MODULE NAME:  DHCPSNP_pmgr.h
 * PURPOSE:
 *    PMGR implement for DHCPSNP.
 *
 * NOTES:
 *
 * REASON:
 * Description:
 * HISTORY
 *    12/12/2007 - Rich Lee, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */

#ifndef DHCPSNP_PMGR_H
#define DHCPSNP_PMGR_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "dhcpsnp_mgr.h"
/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

 /* EXPORTED SUBPROGRAM BODIES
 */

/*------------------------------------------------------------------------------
 * ROUTINE NAME : DHCPSNP_PMGR_InitiateProcessResource
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Initiate resource used in the calling process, means the process that use
 *    this pmgr functions should call this init.
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE  --  Sucess
 *    FALSE --  Error
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T DHCPSNP_PMGR_InitiateProcessResource(void);

/* FUNCTION NAME : DHCPSNP_PMGR_ClearBindingEntriesInFlash
 * PURPOSE:
 *      To clear DHCP snooping binding entries written in flash memory. 
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
 *      1. In Global CLI: clear ip dhcp snooping database flash
 *      2. There is only 1 entry type will be written to the flash: DHCPSNP_TYPE_DYN_SNOOPING_BINDING_ENTRY.
 *      3. Read the flash when provision complete.
 */
UI32_T DHCPSNP_PMGR_ClearBindingEntriesInFlash(void);

/* FUNCTION NAME : DHCPSNP_PMGR_DisableDhcpSnoopingByVlan
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
UI32_T DHCPSNP_PMGR_DisableDhcpSnoopingByVlan(UI32_T vid);

/* FUNCTION NAME : DHCPSNP_PMGR_EnableDhcpSnoopingByVlan
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
UI32_T DHCPSNP_PMGR_EnableDhcpSnoopingByVlan(UI32_T vid);

/* FUNCTION NAME : DHCPSNP_PMGR_GetDhcpSnoopingBindingEntry
 * PURPOSE:
 *      Get the dhcp snooping binding entry that the type is DHCPSNP_TYPE_DYN_SNOOPING_BINDING_ENTRY;
 *
 * INPUT:
 *      binding_entry_p->ip_addr      --  the ip address
 *      binding_entry_p->mac_p        --  the MAC of this binding entry
 *
 * OUTPUT:
 *      binding_entry_p               --  the dynamic snooping binding entry
 *
 * RETURN:
 *      DHCPSNP_TYPE_OK
 *      DHCPSNP_TYPE_FAIL
 *      DHCPSNP_TYPE_BINDING_ENTRY_NOT_EXISTED
 *
 * NOTES:
 *      1.There're 3 entry types
 *          a.DHCPSNP_TYPE_DYN_SNOOPING_BINDING_ENTRY
 *          b.DHCPSNP_TYPE_STA_IP_SRC_GRD_BINDING_ENTRY.
 *        - ip soure guard binding entry includes type a,b.
 *        - dhcp snooping binding entry includes type a.
 */
UI32_T DHCPSNP_PMGR_GetDhcpSnoopingBindingEntry(DHCPSNP_TYPE_BindingEntry_T *binding_entry_p);

/* FUNCTION NAME : DHCPSNP_PMGR_GetDhcpSnoopingStatusByVlan
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
UI32_T DHCPSNP_PMGR_GetDhcpSnoopingStatusByVlan(UI32_T vid, UI8_T *status_p);

/* FUNCTION NAME : DHCPSNP_PMGR_GetGlobalDhcpSnoopingStatus
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
UI32_T DHCPSNP_PMGR_GetGlobalDhcpSnoopingStatus(UI8_T *status_p);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - DHCPSNP_PMGR_GetInformationOptionStatus                                
 *------------------------------------------------------------------------------
 * PURPOSE  : Get DHCPSNP option82 status  : enable/disable
 * INPUT    : none
 * OUTPUT   : current status. 1 :DHCPSNP_TYPE_OPTION82_ENABLED, 
 *            2 :DHCPSNP_TYPE_OPTION82_DISABLED
 * RETURN   : DHCPSNP_TYPE_OK : If success
 *            DHCPSNP_TYPE_FAIL:
 * NOTES    : none                                                              
 *------------------------------------------------------------------------------*/  
UI32_T  DHCPSNP_PMGR_GetInformationOptionStatus(UI8_T *status_p);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - DHCPSNP_PMGR_GetInformationPolicy                               
 *------------------------------------------------------------------------------
 * PURPOSE  : Get DHCPSNP option82 Policy  : 
 * INPUT    : none
 * OUTPUT   : current option82 policy.1 :DHCPSNP_TYPE_OPTION82_POLICY_DROP, 2 :DHCPSNP_TYPE_OPTION82_POLICY_REPLACE
 *                 3:DHCPSNP_TYPE_OPTION82_POLICY_KEEP
 * RETURN   : DHCPSNP_TYPE_OK : If success
 *            DHCPSNP_TYPE_FAIL:
 * NOTES    : none                                                              
 *------------------------------------------------------------------------------*/ 
UI32_T  DHCPSNP_PMGR_GetInformationPolicy(UI8_T *policy_p);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - DHCPSNP_PMGR_GetInformationRidMode                               
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
UI32_T DHCPSNP_PMGR_GetInformationRidMode(UI8_T *mode);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - DHCPSNP_PMGR_GetInformationRidValue                               
 *------------------------------------------------------------------------------
 * PURPOSE  : Set DHCPSNP option82 remote id value 
 * INPUT    : rid_value  -- configured string
 * OUTPUT   : none
 * RETURN   : DHCPSNP_TYPE_OK : If success
 *            DHCPSNP_TYPE_FAIL:
 * NOTES    : none                                                              
 *------------------------------------------------------------------------------*/ 
 UI32_T DHCPSNP_PMGR_GetInformationRidValue(UI8_T *rid_value);


/*------------------------------------------------------------------------------
 * FUNCTION NAME - DHCPSNP_PMGR_GetInformationPortCidMode                              
 *------------------------------------------------------------------------------
 * PURPOSE  : Get DHCPSNP option82 circuit id mode  : 
 * INPUT    : lport_ifindex  -- port ifindex
 * OUTPUT   : current option82 circuit id mode. DHCPSNP_TYPE_OPTION82_CID_VLAN_UNIT_PORT / DHCPSNP_TYPE_OPTION82_CID_CONFIGURED_STRING
 * RETURN   : DHCPSNP_TYPE_OK : If success
 *            DHCPSNP_TYPE_FAIL:
 * NOTES    : none                                                              
 *------------------------------------------------------------------------------*/ 
 UI32_T  DHCPSNP_PMGR_GetInformationPortCidMode(UI32_T lport_ifindex, UI8_T *mode_p);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - DHCPSNP_PMGR_GetInformationPortCidValue                             
 *------------------------------------------------------------------------------
 * PURPOSE  : Get DHCPSNP option82 circuit id value 
 * INPUT    : lport_ifindex  -- port ifindex
 * OUTPUT   : cid_value_p    -- configured string 
 * RETURN   : DHCPSNP_TYPE_OK : If success
 *            DHCPSNP_TYPE_FAIL:
 * NOTES    : none                                                              
 *------------------------------------------------------------------------------*/ 
 UI32_T  DHCPSNP_PMGR_GetInformationPortCidValue(UI32_T lport_ifindex, UI8_T *cid_value_p);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - DHCPSNP_PMGR_GetInformationFormat                       
 *------------------------------------------------------------------------------
 * PURPOSE  : Get DHCPSNP option82 subytpe format : TRUE / FALSE
 * INPUT    : subtype_format_p  
 * OUTPUT   : subtype_format_p  -- use subtype format or not  (TRUE/FALSE)
 * RETURN   : DHCPSNP_TYPE_OK : If success
 *            DHCPSNP_TYPE_FAIL:
 * NOTES    : none                                                              
 *------------------------------------------------------------------------------*/ 
 UI32_T DHCPSNP_PMGR_GetInformationFormat(BOOL_T *subtype_format_p);

/* FUNCTION NAME : DHCPSNP_PMGR_GetNextActiveDhcpSnoopingVlan
 * PURPOSE:
 *      Get the next active vlan id with DHCP Snooping Enabled.
 *
 * INPUT:
 *      vid_p --  current vlan id.
 *
 * OUTPUT:
 *      vid_p --  the next active vlan id with DHCP Snooping Enabled.
 *
 * RETURN:
 *      DHCPSNP_TYPE_OK
 *      DHCPSNP_TYPE_FAIL
 *      DHCPSNP_TYPE_NO_MORE_ENTRY
 *
 * NOTES:
 *      1.In show CLI: show ip dhcp snooping
 */
UI32_T DHCPSNP_PMGR_GetNextActiveDhcpSnoopingVlan(UI32_T *vid_p);

/* FUNCTION NAME : DHCPSNP_PMGR_GetNextDhcpSnoopingBindingEntry
 * PURPOSE:
 *      Get the next dhcp snooping binding entry that the type is DHCPSNP_TYPE_DYN_SNOOPING_BINDING_ENTRY.
 *
 * INPUT :  binding_entry_p  --  binding entry
 *
 * OUTPUT:
 *      binding_entry_p   --  the next dhcp snooping binding entry
 *
 * RETURN:
 *      DHCPSNP_TYPE_OK
 *      DHCPSNP_TYPE_FAIL
 *      DHCPSNP_TYPE_NO_MORE_ENTRY
 *
 * NOTES:
 *      1.In show CLI: show ip dhcp snooping binding
 *      2.There're 3 entry types
 *          a.DHCPSNP_TYPE_DYN_SNOOPING_BINDING_ENTRY
 *          b.DHCPSNP_TYPE_STA_IP_SRC_GRD_BINDING_ENTRY.
 *        - ip soure guard binding entry includes type a,b.
 *        - dhcp snooping binding entry includes type a.
 */
UI32_T DHCPSNP_PMGR_GetNextDhcpSnoopingBindingEntry(DHCPSNP_TYPE_BindingEntry_T *binding_entry_p);

/* FUNCTION NAME : DHCPSNP_PMGR_GetNextDhcpSnoopingStatusByVlan
 * PURPOSE:
 *      Get the next existed vlan id with DHCP Snooping status.
 *
 * INPUT:
 *      vid_p --  current vlan id.
 *
 * OUTPUT:
 *      vid_p     --  the next existed vlan id with DHCP Snooping status.
 *      status_p  --  DHCPSNP_TYPE_SNOOPING_VLAN_DISABLED or DHCPSNP_TYPE_SNOOPING_VLAN_ENABLED.
 *
 * RETURN:
 *      DHCPSNP_TYPE_OK
 *      DHCPSNP_TYPE_FAIL
 *      DHCPSNP_TYPE_NO_MORE_ENTRY
 *
 * NOTES:
 *      1.In show CLI: show ip dhcp snooping
 */
UI32_T DHCPSNP_PMGR_GetNextDhcpSnoopingStatusByVlan(UI32_T *vid_p, UI8_T *status_p);

/* FUNCTION NAME : DHCPSNP_PMGR_GetNextDynamicIpSrcGuardBindingEntry
 * PURPOSE:
 *      Get the next ip source guard binding entry that is learnt from DHCP Snooping.
 *
 * INPUT :  binding_entry_p  --  binding entry
 *
 * OUTPUT:
 *      binding_entry_p   --  the next dynamic ip source guard binding entry
 *
 * RETURN:
 *      DHCPSNP_TYPE_OK
 *      DHCPSNP_TYPE_FAIL
 *      DHCPSNP_TYPE_NO_MORE_ENTRY
 *
 * NOTES:
 *      1.In show CLI: sh ip source-guard dhcp-snooping
 */
UI32_T DHCPSNP_PMGR_GetNextDynamicIpSrcGuardBindingEntry(DHCPSNP_TYPE_BindingEntry_T *binding_entry_p);

/* FUNCTION NAME : DHCPSNP_PMGR_GetNextPortInfo
 * PURPOSE:
 *      Get the next port information.
 *
 * INPUT:
 *      lport_ifindex_p    --  the logic port to get the next port info.
 *
 * OUTPUT:
 *      lport_ifindex    --  the next logic port.
 *      port_info_p      --  the next port information.
 *
 * RETURN:
 *      DHCPSNP_TYPE_OK
 *      DHCPSNP_TYPE_FAIL
 *      DHCPSNP_TYPE_INVALID_ARG
 *      DHCPSNP_TYPE_NO_MORE_ENTRY
 *
 * NOTES:
 *      1.This API will get all port information.
 */
UI32_T DHCPSNP_PMGR_GetNextPortInfo(UI32_T *lport_ifindex_p, DHCPSNP_TYPE_PortInfo_T *port_info_p);

/* FUNCTION NAME : DHCPSNP_PMGR_GetNextRunningDhcpSnoopingStatusByVlan
 * PURPOSE:
 *      Get the next user configured vlan id. And check is it the default value.
 *
 * INPUT:
 *      vid_p       --  current vlan id.
 *
 * OUTPUT:
 *      vid_p       --  the next vlan id configured by the user.
 *      status_p    --  DHCPSNP_TYPE_SNOOPING_VLAN_DISABLED or DHCPSNP_TYPE_SNOOPING_VLAN_ENABLED
 *
 * RETURN:
 *      SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- not same as default
 *      SYS_TYPE_GET_RUNNING_CFG_FAIL      -- get failure
 *      SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 *
 * NOTES:
 *      1. The DHCP Snooping of the vlan may be enabled/disabled. \
 *         This API only return the vlan id configured by user.
 */
UI32_T DHCPSNP_PMGR_GetNextRunningDhcpSnoopingStatusByVlan(UI32_T *vid_p, UI8_T *status_p);

/* FUNCTION NAME : DHCPSNP_PMGR_GetNextRunningStaticIpSrcGuardBindingEntry
 * PURPOSE:
 *      Get the next ip source guard binding entry that the type is DHCPSNP_TYPE_STA_IP_SRC_GRD_BINDING_ENTRY.
 *
 * INPUT :  binding_entry_p  --  binding entry
 *
 * OUTPUT:
 *      binding_entry_p   --  the next ip source guard binding entry
 *
 * RETURN:
 *      SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- not same as default
 *      SYS_TYPE_GET_RUNNING_CFG_FAIL      -- get failure
 *      SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 *
 * NOTES:
 *      The dynamic dhcpsnp binding entries are read from CFGDB in DHCPSNP_MGR_EnterMasterMode
 *      by DHCPSNP component automatically.
 */
UI32_T DHCPSNP_PMGR_GetNextRunningStaticIpSrcGuardBindingEntry(DHCPSNP_TYPE_BindingEntry_T *binding_entry_p);

/* FUNCTION NAME : DHCPSNP_PMGR_GetPortInfo
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
UI32_T DHCPSNP_PMGR_GetPortInfo(UI32_T lport_ifindex, DHCPSNP_TYPE_PortInfo_T *port_info_p);

/* FUNCTION NAME : DHCPSNP_PMGR_GetRunningGlobalDhcpSnoopingStatus
 * PURPOSE:
 *      Get the global DHCP Snooping function status. And check is it the default value.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      status_p  --  DHCPSNP_TYPE_SNOOPING_GLOBAL_DISABLED or DHCPSNP_TYPE_SNOOPING_GLOBAL_ENABLED
 *
 * RETURN:
 *      SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- not same as default
 *      SYS_TYPE_GET_RUNNING_CFG_FAIL      -- get failure
 *      SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 *
 * NOTES:
 *      None.
 */
UI32_T DHCPSNP_PMGR_GetRunningGlobalDhcpSnoopingStatus(UI8_T *status_p);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - DHCPSNP_PMGR_GetRunningInformationOptionStatus
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the status of disable/enable mapping of system
 * INPUT:    None
 * OUTPUT:   status -- DHCPSNP_TYPE_OPTION82_ENABLED/DHCPSNP_TYPE_OPTION82_DISABLED
 * RETURN:   SYS_TYPE_GET_RUNNING_CFG_SUCCESS -- different from default value
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL -- error (system is not in MASTER mode)
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 *
 * NOTE:     default value is (2)  DHCPSNP_TYPE_OPTION82_DISABLED
 *---------------------------------------------------------------------------*/
UI32_T  DHCPSNP_PMGR_GetRunningInformationOptionStatus(UI8_T *status_p);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - DHCPSNP_PMGR_GetRunningInformationPolicy
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the status of drop/replace/keep mapping of system
 * INPUT:    None
 * OUTPUT:   status -- drop/replace/keep
 * RETURN:   SYS_TYPE_GET_RUNNING_CFG_SUCCESS -- different from default value
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL -- error (system is not in MASTER mode)
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 *           
 * NOTE:     default value is (2) DHCPSNP_TYPE_OPTION82_POLICY_REPLACE
 *---------------------------------------------------------------------------*/
UI32_T  DHCPSNP_PMGR_GetRunningInformationPolicy(UI8_T *policy_p);


/*--------------------------------------------------------------------------
 * ROUTINE NAME - DHCPSNP_PMGR_GetRunningInformationRidMode
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the mode of mac/ip address in remote id sub-option
 * INPUT:    None
 * OUTPUT:   mode -- DHCPSNP_TYPE_OPTION82_RID_MAC/DHCPSNP_TYPE_OPTION82_RID_IP
 * RETURN:   SYS_TYPE_GET_RUNNING_CFG_SUCCESS -- different from default value
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL -- error (system is not in MASTER mode)
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 *           
 * NOTE:     default value is DHCPSNP_TYPE_OPTION82_RID_MAC
 *---------------------------------------------------------------------------*/
 UI32_T  DHCPSNP_PMGR_GetRunningInformationRidMode(UI8_T *mode_p);


/* FUNCTION NAME : DHCPSNP_PMGR_GetRunningPortCidMode
 * PURPOSE:
 *      Get the cid mode of the port. And check is it the default value.
 *
 * INPUT:
 *      lport_ifindex   --  the port to be get the trust status.
 *
 * OUTPUT:
 *      cid_mode_p      --  DHCPSNP_TYPE_OPTION82_CID_VLAN_UNIT_PORT or DHCPSNP_TYPE_OPTION82_CID_CONFIGURED_STRING
 *
 * RETURN:
 *      SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- not same as default
 *      SYS_TYPE_GET_RUNNING_CFG_FAIL      -- get failure
 *      SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 *
 * NOTES:
 *      None.
 */
UI32_T DHCPSNP_PMGR_GetRunningPortCidMode(UI32_T lport_ifindex, UI8_T *cid_mode_p);;

/* FUNCTION NAME : DHCPSNP_PMGR_GetRunningIpSourceGuardMode
 * PURPOSE:
 *      Get the filtering mode of the ip source guard in a port. And check is it the default value.
 *
 * INPUT:
 *      lport_ifindex   --  the logic port to get the the filtering mode
 *
 * OUTPUT:
 *      filter_mode_p     --  the filtering mode of the ip source guard shall be DHCPSNP_TYPE_IP_SRC_GRD_SIP_FILTER,
 *                            DHCPSNP_TYPE_IP_SRC_GRD_SIP_AND_MAC_FILTER or DHCPSNP_TYPE_IP_SRC_GRD_DISABLED.
 *
 * RETURN:
 *      SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- not same as default
 *      SYS_TYPE_GET_RUNNING_CFG_FAIL      -- get failure
 *      SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 *
 * NOTES:
 *      None.
 */
UI32_T DHCPSNP_PMGR_GetRunningIpSourceGuardMode(UI32_T lport_ifindex, UI8_T *filter_mode_p);

/* FUNCTION NAME : DHCPSNP_PMGR_GetRunningPortTrustStatus
 * PURPOSE:
 *      Get the trsut status of the port. And check is it the default value.
 *
 * INPUT:
 *      lport_ifindex   --  the port to be get the trust status.
 *
 * OUTPUT:
 *      status_p        --  DHCPSNP_TYPE_PORT_UNTRUSTED or DHCPSNP_TYPE_PORT_TRUSTED
 *
 * RETURN:
 *      SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- not same as default
 *      SYS_TYPE_GET_RUNNING_CFG_FAIL      -- get failure
 *      SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 *
 * NOTES:
 *      None.
 */
UI32_T DHCPSNP_PMGR_GetRunningPortTrustStatus(UI32_T lport_ifindex, UI8_T *status_p);

/* FUNCTION NAME : DHCPSNP_PMGR_GetRunningVerifyMacAddressStatus
 * PURPOSE:
 *      Get the status of verify MAC address function. And check is it the default value.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      status_p  --  DHCPSNP_TYPE_VERIFY_MAC_ADDRESS_ENABLED or DHCPSNP_TYPE_VERIFY_MAC_ADDRESS_DISABLED
 *
 * RETURN:
 *      SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- not same as default
 *      SYS_TYPE_GET_RUNNING_CFG_FAIL      -- get failure
 *      SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 *
 * NOTES:
 *      None.
 */
UI32_T DHCPSNP_PMGR_GetRunningVerifyMacAddressStatus(UI8_T *status_p);

/* FUNCTION NAME : DHCPSNP_PMGR_GetTotalForwardedPktCounter
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
UI32_T DHCPSNP_PMGR_GetTotalForwardedPktCounter(UI32_T *total_fwd_counter_p);

/* FUNCTION NAME : DHCPSNP_PMGR_GetUntrustedDroppedPktCounter
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
UI32_T DHCPSNP_PMGR_GetUntrustedDroppedPktCounter(UI32_T *untrust_drp_counter_p);

/* FUNCTION NAME : DHCPSNP_PMGR_GetVerifyMacAddressStatus
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
UI32_T DHCPSNP_PMGR_GetVerifyMacAddressStatus(UI8_T *status_p);


/* FUNCTION NAME : DHCPSNP_PMGR_SetGlobalDhcpSnoopingStatus
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
UI32_T DHCPSNP_PMGR_SetGlobalDhcpSnoopingStatus(UI8_T status);

#if (SYS_CPNT_DHCPSNP_INFORMATION_OPTION == TRUE)	
/*------------------------------------------------------------------------------
 * FUNCTION NAME - DHCPSNP_PMGR_SetInformationOptionStatus                               
 *------------------------------------------------------------------------------
 * PURPOSE  : Set DHCPSNP option82 status : enable/disable
 * INPUT    : status you want to set. 1 :DHCPSNP_TYPE_OPTION82_ENABLED, 
 *            2 :DHCPSNP_TYPE_OPTION82_DISABLED
 * OUTPUT   : none
 * RETURN   : DHCPSNP_TYPE_OK : If success
 *            DHCPSNP_TYPE_FAIL:
 * NOTES    : none                                                              
 *------------------------------------------------------------------------------*/ 
UI32_T DHCPSNP_PMGR_SetInformationOptionStatus(UI8_T status);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - DHCPSNP_PMGR_SetInformationPolicy                         
 *------------------------------------------------------------------------------
 * PURPOSE  : Set DHCPSNP option82 policy : drop/replace/keep
 * INPUT    : policy you want to set. 1 :DHCPSNP_TYPE_OPTION82_POLICY_DROP, 2 :DHCPSNP_TYPE_OPTION82_POLICY_REPLACE
 *                 3:DHCPSNP_TYPE_OPTION82_POLICY_KEEP
 * OUTPUT   : none
 * RETURN   : DHCPSNP_TYPE_OK : If success
 *            DHCPSNP_TYPE_FAIL:
 * NOTES    : none                                                              
 *------------------------------------------------------------------------------*/ 
UI32_T DHCPSNP_PMGR_SetInformationPolicy(UI8_T policy);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - DHCPSNP_PMGR_SetInformationRidMode                               
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
UI32_T DHCPSNP_PMGR_SetInformationRidMode(UI8_T mode);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - DHCPSNP_PMGR_SetInformationRidValue                               
 *------------------------------------------------------------------------------
 * PURPOSE  : Set DHCPSNP option82 remote id value 
 * INPUT    : rid_value  -- configured string
 * OUTPUT   : none
 * RETURN   : DHCPSNP_TYPE_OK : If success
 *            DHCPSNP_TYPE_FAIL:
 * NOTES    : none                                                              
 *------------------------------------------------------------------------------*/ 
UI32_T DHCPSNP_PMGR_SetInformationRidValue(UI8_T *rid_value);


/*------------------------------------------------------------------------------
 * FUNCTION NAME - DHCPSNP_PMGR_SetInformationPortCidMode                       
 *------------------------------------------------------------------------------
 * PURPOSE  : Set DHCPSNP option82 circuit id mode : vlan-unit-port / configured string
 * INPUT    : lport_ifindex  -- port ifindex
 *            mode           -- DHCPSNP_TYPE_OPTION82_CID_VLAN_UNIT_PORT
 *                              DHCPSNP_TYPE_OPTION82_CID_CONFIGURED_STRING
 * OUTPUT   : none
 * RETURN   : DHCPSNP_TYPE_OK : If success
 *            DHCPSNP_TYPE_FAIL:
 * NOTES    : none                                                              
 *------------------------------------------------------------------------------*/ 
 UI32_T DHCPSNP_PMGR_SetInformationPortCidMode(UI32_T lport_ifindex,UI8_T mode);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - DHCPSNP_PMGR_SetInformationPortCidValue                       
 *------------------------------------------------------------------------------
 * PURPOSE  : Set DHCPSNP option82 circuit id value 
 * INPUT    : lport_ifindex  -- port ifindex
 *            cid_value_p    -- configured string 
 * OUTPUT   : none
 * RETURN   : DHCPSNP_TYPE_OK : If success
 *            DHCPSNP_TYPE_FAIL:
 * NOTES    : max number of characters of cid_value is 32                                                               
 *------------------------------------------------------------------------------*/ 
 UI32_T DHCPSNP_PMGR_SetInformationPortCidValue(UI32_T lport_ifindex,UI8_T *cid_value_p);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - DHCPSNP_PMGR_SetInformationFormat                       
 *------------------------------------------------------------------------------
 * PURPOSE  : Set DHCPSNP option82 subytpe format : TRUE / FALSE
 * INPUT    : subtype_format  -- use subtype format or not  (TRUE/FALSE)
 * OUTPUT   : none
 * RETURN   : DHCPSNP_TYPE_OK : If success
 *            DHCPSNP_TYPE_FAIL:
 * NOTES    : none                                                              
 *------------------------------------------------------------------------------*/ 
 UI32_T DHCPSNP_PMGR_SetInformationFormat(BOOL_T subtype_format);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - DHCPSNP_PMGR_GetInformationOptionStatus                                
 *------------------------------------------------------------------------------
 * PURPOSE  : Get DHCPSNP option82 status  : enable/disable
 * INPUT    : none
 * OUTPUT   : current status. 1 :DHCPSNP_TYPE_OPTION82_ENABLED, 
 *            2 :DHCPSNP_TYPE_OPTION82_DISABLED
 * RETURN   : DHCPSNP_TYPE_OK : If success
 *            DHCPSNP_TYPE_FAIL:
 * NOTES    : none                                                              
 *------------------------------------------------------------------------------*/  
UI32_T  DHCPSNP_PMGR_GetInformationOptionStatus(UI8_T *status_p);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - DHCPSNP_PMGR_GetInformationPolicy                               
 *------------------------------------------------------------------------------
 * PURPOSE  : Get DHCPSNP option82 Policy  : 
 * INPUT    : none
 * OUTPUT   : current option82 policy.1 :DHCPSNP_TYPE_OPTION82_POLICY_DROP, 2 :DHCPSNP_TYPE_OPTION82_POLICY_REPLACE
 *                 3:DHCPSNP_TYPE_OPTION82_POLICY_KEEP
 * RETURN   : DHCPSNP_TYPE_OK : If success
 *            DHCPSNP_TYPE_FAIL:
 * NOTES    : none                                                              
 *------------------------------------------------------------------------------*/ 
UI32_T  DHCPSNP_PMGR_GetInformationPolicy(UI8_T *policy_p);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - DHCPSNP_PMGR_GetInformationRidMode                               
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
UI32_T DHCPSNP_PMGR_GetInformationRidMode(UI8_T *mode);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - DHCPSNP_PMGR_GetInformationRidValue                               
 *------------------------------------------------------------------------------
 * PURPOSE  : Set DHCPSNP option82 remote id value 
 * INPUT    : rid_value  -- configured string
 * OUTPUT   : none
 * RETURN   : DHCPSNP_TYPE_OK : If success
 *            DHCPSNP_TYPE_FAIL:
 * NOTES    : none                                                              
 *------------------------------------------------------------------------------*/ 
 UI32_T DHCPSNP_PMGR_GetInformationRidValue(UI8_T *rid_value);


/*------------------------------------------------------------------------------
 * FUNCTION NAME - DHCPSNP_PMGR_GetInformationPortCidMode                              
 *------------------------------------------------------------------------------
 * PURPOSE  : Get DHCPSNP option82 circuit id mode  : 
 * INPUT    : lport_ifindex  -- port ifindex
 * OUTPUT   : current option82 circuit id mode. DHCPSNP_TYPE_OPTION82_CID_VLAN_UNIT_PORT / DHCPSNP_TYPE_OPTION82_CID_CONFIGURED_STRING
 * RETURN   : DHCPSNP_TYPE_OK : If success
 *            DHCPSNP_TYPE_FAIL:
 * NOTES    : none                                                              
 *------------------------------------------------------------------------------*/ 
 UI32_T  DHCPSNP_PMGR_GetInformationPortCidMode(UI32_T lport_ifindex, UI8_T *mode_p);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - DHCPSNP_PMGR_GetInformationPortCidValue                             
 *------------------------------------------------------------------------------
 * PURPOSE  : Get DHCPSNP option82 circuit id value 
 * INPUT    : lport_ifindex  -- port ifindex
 * OUTPUT   : cid_value_p    -- configured string 
 * RETURN   : DHCPSNP_TYPE_OK : If success
 *            DHCPSNP_TYPE_FAIL:
 * NOTES    : none                                                              
 *------------------------------------------------------------------------------*/ 
 UI32_T  DHCPSNP_PMGR_GetInformationPortCidValue(UI32_T lport_ifindex, UI8_T *cid_value_p);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - DHCPSNP_PMGR_GetInformationFormat                       
 *------------------------------------------------------------------------------
 * PURPOSE  : Get DHCPSNP option82 subytpe format : TRUE / FALSE
 * INPUT    : subtype_format_p  
 * OUTPUT   : subtype_format_p  -- use subtype format or not  (TRUE/FALSE)
 * RETURN   : DHCPSNP_TYPE_OK : If success
 *            DHCPSNP_TYPE_FAIL:
 * NOTES    : none                                                              
 *------------------------------------------------------------------------------*/ 
 UI32_T DHCPSNP_PMGR_GetInformationFormat(BOOL_T *subtype_format_p);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - DHCPSNP_PMGR_GetRunningInformationOptionStatus
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the status of disable/enable mapping of system
 * INPUT:    None
 * OUTPUT:   status -- DHCPSNP_TYPE_OPTION82_ENABLED/DHCPSNP_TYPE_OPTION82_DISABLED
 * RETURN:   SYS_TYPE_GET_RUNNING_CFG_SUCCESS -- different from default value
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL -- error (system is not in MASTER mode)
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 *
 * NOTE:     default value is (2)  DHCPSNP_TYPE_OPTION82_DISABLED
 *---------------------------------------------------------------------------*/
UI32_T  DHCPSNP_PMGR_GetRunningInformationOptionStatus(UI8_T *status_p);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - DHCPSNP_PMGR_GetRunningInformationPolicy
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the status of drop/replace/keep mapping of system
 * INPUT:    None
 * OUTPUT:   status -- drop/replace/keep
 * RETURN:   SYS_TYPE_GET_RUNNING_CFG_SUCCESS -- different from default value
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL -- error (system is not in MASTER mode)
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 *           
 * NOTE:     default value is (2) DHCPSNP_TYPE_OPTION82_POLICY_REPLACE
 *---------------------------------------------------------------------------*/
UI32_T  DHCPSNP_PMGR_GetRunningInformationPolicy(UI8_T *policy_p);


/*--------------------------------------------------------------------------
 * ROUTINE NAME - DHCPSNP_PMGR_GetRunningInformationRidMode
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the mode of mac/ip address in remote id sub-option
 * INPUT:    None
 * OUTPUT:   mode -- DHCPSNP_TYPE_OPTION82_RID_MAC/DHCPSNP_TYPE_OPTION82_RID_IP
 * RETURN:   SYS_TYPE_GET_RUNNING_CFG_SUCCESS -- different from default value
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL -- error (system is not in MASTER mode)
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 *           
 * NOTE:     default value is DHCPSNP_TYPE_OPTION82_RID_MAC
 *---------------------------------------------------------------------------*/
 UI32_T  DHCPSNP_PMGR_GetRunningInformationRidMode(UI8_T *mode_p);

/* FUNCTION NAME : DHCPSNP_PMGR_GetRunningInformationFormat
 * PURPOSE:
 *      Get the option82 subtype format. And check is it the default value.
 *
 * INPUT:
 *      subtype_format_p
 *
 * OUTPUT:
 *      subtype_format_p      --  TRUE or FALSE
 *
 * RETURN:
 *      SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- not same as default
 *      SYS_TYPE_GET_RUNNING_CFG_FAIL      -- get failure
 *      SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 *
 * NOTES:
 *      None.
 */
UI32_T DHCPSNP_PMGR_GetRunningInformationFormat(BOOL_T *subtype_format_p);

#endif /* SYS_CPNT_DHCPSNP_INFORMATION_OPTION */

/* FUNCTION NAME : DHCPSNP_PMGR_SetPortTrustStatus
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
UI32_T DHCPSNP_PMGR_SetPortTrustStatus(UI32_T lport_ifindex, UI8_T status);

/* FUNCTION NAME : DHCPSNP_PMGR_SetVerifyMacAddressStatus
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
UI32_T DHCPSNP_PMGR_SetVerifyMacAddressStatus(UI8_T status);

/* FUNCTION NAME : DHCPSNP_PMGR_WriteBindingEntriesToFlash
 * PURPOSE:
 *      Write only dynamic dhcp snooping binding into the flash.
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
 *      1. In Global CLI: ip dhcp snooping/database/flash
 *      2. There is only 1 entry type will be written to the flash: DHCPSNP_TYPE_DYN_SNOOPING_BINDING_ENTRY.
 *      3. Read the flash when provision complete.
 */
UI32_T DHCPSNP_PMGR_WriteBindingEntriesToFlash(void);

/* FUNCTION NAME : DHCPSNP_MGR_SetBindingsLeaseTime
 * PURPOSE:
 *      Set the lease time of the binding entry.
 *
 * INPUT:
 *      vid         --  the vlan id.    (one of the key in the binding table)
 *      mac_addr_p  --  the MAC address (another key in the binding table)
 *      lease_time  --  the lease of the binding entry.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      DHCPSNP_TYPE_OK
 *      DHCPSNP_TYPE_FAIL
 *
 * NOTES:
 *      1. When vlaues of fileds in an entry are all valid, we will set the
 *         rowstatus to 'active' automatically.
 *      2. When the entry type is static ip source guard binding, lease time can not be set.
 *         It's infinite when the type is static ip source guard binding.
 */
UI32_T DHCPSNP_PMGR_SetBindingsLeaseTime(UI32_T vid, UI8_T *mac_addr_p, UI32_T lease_time);

/* FUNCTION NAME : DHCPSNP_MGR_GetPortTrustStatus
 * PURPOSE:
 *      Get the trsut status of the port.
 *
 * INPUT:
 *      lport_ifindex   --  the port to be get the trust status.
 *
 * OUTPUT:
 *      status_p        --  DHCPSNP_TYPE_PORT_UNTRUSTED or DHCPSNP_TYPE_PORT_TRUSTED
 *
 * RETURN:
 *      DHCPSNP_TYPE_OK
 *      DHCPSNP_TYPE_FAIL
 *      DHCPSNP_TYPE_INVALID_ARG
 *
 * NOTES:
 *      None.
 */
UI32_T DHCPSNP_PMGR_GetPortTrustStatus(UI32_T lport_ifindex, UI8_T *status_p);

/*fuzhimin,20090505*/
/* FUNCTION NAME : DHCPSNP_PMGR_SendDhcpClientPkt
 * PURPOSE:
 *      Send the DHCP client packet from the DUT when it's also a DHCP Client.
 *
 * INPUT:
 *      mem_ref_p         --  L_MREF descriptor.
 *      packet_length   --  the length of the packet.
 *      txRifNum        --  the tx rif number (always zero)
 *      dst_mac_p         --  the dstination MAC address
 *      src_mac_p         --  the source MAC address
 *      vid             --  the vlan id
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      DHCPSNP_TYPE_OK
 *      DHCPSNP_TYPE_FAIL
 *
 * NOTES:
 *      1.This API is called by DHCP_TXRX when DHCPSNP is supported.
 */

UI32_T DHCPSNP_PMGR_SendDhcpClientPkt(L_MM_Mref_Handle_T    *mem_ref_p,
                                     UI32_T      packet_length,
                                     UI32_T      txRifNum,
                                     UI8_T       *dst_mac_p,
                                     UI8_T       *src_mac_p,
                                     UI32_T      vid);

/*------------------------------------------------------------------------------
 * ROUTINE NAME  - DHCPSNP_PMGR_SetPortFilterTableMode
 *------------------------------------------------------------------------------
 * PURPOSE: Configure port IPSG filter table mode
 * INPUT :  lport  -  logical port ifindex
 *          mode   -  filter table mode
 * OUTPUT:  None
 * RETURN:  DHCPSNP_TYPE_OK/
 *          DHCPSNP_TYPE_FAIL/
 *          DHCPSNP_TYPE_INVALID_ARG
 * NOTES :  None
 *------------------------------------------------------------------------------
 */
UI32_T DHCPSNP_PMGR_SetPortFilterTableMode(UI32_T lport, UI8_T mode);

#if (SYS_CPNT_DAI == TRUE)
/* FUNCTION NAME : DHCPSNP_PMGR_IsDaiIpMacPortMatchInBindingTable
 * PURPOSE:
 *      Check if the incoming IP, MAC and Port match the corresponding fields in binding table.
 *
 * INPUT:
 *      vid             --  the vlan id
 *      mac_p           --  the MAC address
 *      ip_addr         --  the ip address
 *      lport_ifindex   --  the port related with this binding entry.
 *		allow_zero_sip  --  allows zero source ip or not 
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE or FALSE
 *
 * NOTES:
 *
 */
BOOL_T DHCPSNP_PMGR_IsDaiIpMacPortMatchInBindingTable(
	UI32_T vid, 
	UI8_T *mac_p, 
	UI32_T lport_ifindex, 
	UI32_T ip_addr,
	BOOL_T allow_zero_sip);
#endif


#if ( SYS_ADPT_DHCPSNP_MAX_NBR_OF_CLIENT_PER_PORT_CONFIGURABLE == TRUE)
/*------------------------------------------------------------------------------
 * ROUTINE NAME  - DHCPSNP_PGR_SetPortBindingEntryLimit
 *------------------------------------------------------------------------------
 * PURPOSE: Configure port IPSG maximum entry limit
 * INPUT :  lport_ifindex  -  logical port ifindex
 *          mode           -  filter table mode
 *          limit          -  entry limit
 * OUTPUT:  None
 * RETURN:  DHCPSNP_TYPE_OK/
 *          DHCPSNP_TYPE_FAIL/
 *          DHCPSNP_TYPE_INVALID_ARG
 * NOTES :  None
 *------------------------------------------------------------------------------
 */
UI32_T DHCPSNP_PMGR_SetPortBindingEntryLimit(UI32_T lport_ifindex, UI8_T mode, UI32_T limit);

/*------------------------------------------------------------------------------
 * ROUTINE NAME  - DHCPSNP_PMGR_GetPortBindingEntryLimit
 *------------------------------------------------------------------------------
 * PURPOSE: Get port IPSG maximum entry limit
 * INPUT :  lport_ifindex  -  logical port ifindex
 *          mode           -  filter table mode
 * OUTPUT:  limit_p        -  port entry limit
 * RETURN:  DHCPSNP_TYPE_OK/
 *          DHCPSNP_TYPE_FAIL/
 *          DHCPSNP_TYPE_INVALID_ARG
 * NOTES :  None
 *------------------------------------------------------------------------------
 */
UI32_T DHCPSNP_PMGR_GetPortBindingEntryLimit(UI32_T lport_ifindex, UI8_T mode, UI32_T *limit_p);

/*------------------------------------------------------------------------------
 * ROUTINE NAME  - DHCPSNP_PMGR_GetRunningPortBindingEntryLimit
 *------------------------------------------------------------------------------
 * PURPOSE: Get the next per-port IPSG maximum entry limit
 * INPUT :  lport_ifindex  -  logical port ifindex
 *          mode           -  filter table mode
 * OUTPUT:  limit_p        -  entry limit
 * RETURN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- not same as default
 *      SYS_TYPE_GET_RUNNING_CFG_FAIL      -- get failure
 *      SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 * NOTES :  None
 *------------------------------------------------------------------------------
 */
UI32_T DHCPSNP_PMGR_GetRunningPortBindingEntryLimit(UI32_T lport_ifindex, UI8_T mode, UI32_T *limit_p);

/* FUNCTION NAME : DHCPSNP_PMGR_DeleteDynamicDhcpSnoopingBindingEntry
 * PURPOSE:
 *      Delete dynamic DHCP snooping binding entry from the the binding table.
 *
 * INPUT :  binding_entry_p  --  binding entry
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      DHCPSNP_TYPE_OK
 *      DHCPSNP_TYPE_FAIL
 *      DHCPSNP_TYPE_INVALID_ARG
 *      DHCPSNP_TYPE_BINDING_TABLE_FULL
 *
 * NOTES:
 *      It can only delete dynamic learnt DHCP binding entry 
 */
UI32_T DHCPSNP_PMGR_DeleteDynamicDhcpSnoopingBindingEntry(DHCPSNP_TYPE_BindingEntry_T *binding_entry_p);

/* FUNCTION NAME : DHCPSNP_PMGR_DeleteAllDynamicDhcpSnoopingBindingEntry
 * PURPOSE:
 *      Delete all dynamic DHCP snooping binding entry from the the binding table.
 *
 * INPUT:
 *      none
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      DHCPSNP_TYPE_OK
 *      DHCPSNP_TYPE_FAIL
 *      DHCPSNP_TYPE_INVALID_ARG
 *      DHCPSNP_TYPE_BINDING_TABLE_FULL
 *
 * NOTES:
 *      It can only clear dynamic learnt DHCP binding entry 
 */
UI32_T DHCPSNP_PMGR_DeleteAllDynamicDhcpSnoopingBindingEntry(void);
#endif


#if (SYS_CPNT_DHCPSNP_SYSTEM_RATELIMIT==TRUE)
/*------------------------------------------------------------------------------
 * ROUTINE NAME  - DHCPSNP_PMGR_SetGlobalRateLimit
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
UI32_T DHCPSNP_PMGR_SetGlobalRateLimit(UI32_T rate);

/*------------------------------------------------------------------------------
 * ROUTINE NAME  - DHCPSNP_PMGR_GetGlobalRateLimit
 *------------------------------------------------------------------------------
 * PURPOSE: Get system-wise dhcp packet rate limit
 * INPUT :  rate	--	the limited rate
 * OUTPUT:  rate	--	the limited rate
 * RETURN:  DHCPSNP_TYPE_OK/
 *          DHCPSNP_TYPE_FAIL/
 *          DHCPSNP_TYPE_INVALID_ARG
 * NOTES :  if rate = 0, that means no rate limit.
 *------------------------------------------------------------------------------
 */
UI32_T DHCPSNP_PMGR_GetGlobalRateLimit(UI32_T *rate);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - DHCPSNP_PMGR_GetRunningGlobalRateLimit
 *---------------------------------------------------------------------------
 * PURPOSE:  Get running config of system-wise rate limit
 * INPUT:    rate	--	rate limit	
 * OUTPUT:   rate 	--	rate limit
 * RETURN:   SYS_TYPE_GET_RUNNING_CFG_SUCCESS -- different from default value
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL -- error (system is not in MASTER mode)
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 *
 * NOTE:     if rate = 0, that means no rate limit.
 *---------------------------------------------------------------------------*/
UI32_T  DHCPSNP_PMGR_GetRunningGlobalRateLimit(UI32_T *rate);
#endif

#if (SYS_CPNT_DHCPSNP_INFORMATION_OPTION_TR101_FORMAT == TRUE)
/*------------------------------------------------------------------------------
 * ROUTINE NAME  - DHCPSNP_PMGR_SetBoardId
 *------------------------------------------------------------------------------
 * PURPOSE: Set TR101 format board id 
 * INPUT :  board_id    --  board identifier
 * OUTPUT:  None
 * RETURN:  DHCPSNP_TYPE_OK/
 *          DHCPSNP_TYPE_INVALID_ARG/
 *          DHCPSNP_TYPE_FAIL/
 * NOTES :  None
 *------------------------------------------------------------------------------
 */
UI32_T DHCPSNP_PMGR_SetBoardId(UI8_T board_id);

/*------------------------------------------------------------------------------
 * ROUTINE NAME  - DHCPSNP_PMGR_GetBoardId
 *------------------------------------------------------------------------------
 * PURPOSE: Set TR101 format board id 
 * INPUT :  board_id    --  board identifier
 * OUTPUT:  board_id    --  board identifier
 * RETURN:  DHCPSNP_TYPE_OK/
 *          DHCPSNP_TYPE_INVALID_ARG/
 *          DHCPSNP_TYPE_FAIL/
 * NOTES :  None
 *------------------------------------------------------------------------------
 */
UI32_T DHCPSNP_PMGR_GetBoardId(UI8_T *board_id);

/*------------------------------------------------------------------------------
 * ROUTINE NAME  - DHCPSNP_PMGR_GetRunningBoardId
 *------------------------------------------------------------------------------
 * PURPOSE: Get running config of TR101 format board id 
 * INPUT :  board_id    --  board identifier
 * OUTPUT:  board_id    --  board identifier
 * RETURN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- different from default value
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL      -- error (system is not in MASTER mode)
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 * NOTES :  None
 *------------------------------------------------------------------------------
 */
UI32_T DHCPSNP_PMGR_GetRunningBoardId(UI8_T *board_id);
#endif

#if (SYS_CPNT_DHCPSNP_INFORMATION_OPTION_RID_SUB_OPTION == TRUE)
/*------------------------------------------------------------------------------
 * FUNCTION NAME - DHCPSNP_PMGR_SetOptionDelimiter
 *------------------------------------------------------------------------------
 * PURPOSE  : Set option82 delimiter
 * INPUT    : delimiter  --  option delimiter
 * OUTPUT   : none
 * RETURN   : DHCPSNP_TYPE_OK/
 *            DHCPSNP_TYPE_FAIL
 * NOTES    : none
 *------------------------------------------------------------------------------*/
UI32_T DHCPSNP_PMGR_SetOptionDelimiter(UI8_T delimiter);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - DHCPSNP_PMGR_GetOptionDelimiter
 *------------------------------------------------------------------------------
 * PURPOSE  : Get option82 delimiter
 * INPUT    : delimiter  --  option delimiter
 * OUTPUT   : delimiter  --  option delimiter
 * RETURN   : DHCPSNP_TYPE_OK/
 *            DHCPSNP_TYPE_INVALID_ARG/
 *            DHCPSNP_TYPE_FAIL
 * NOTES    : none
 *------------------------------------------------------------------------------*/
UI32_T DHCPSNP_PMGR_GetOptionDelimiter(UI8_T *delimiter);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - DHCPSNP_PMGR_GetRunningOptionDelimiter
 *---------------------------------------------------------------------------
 * PURPOSE:  Get running config of remote-id option delimiter
 * INPUT:    delimiter	--	option delimiter
 * OUTPUT:   delimiter 	--	option delimiter
 * RETURN:   SYS_TYPE_GET_RUNNING_CFG_SUCCESS -- different from default value
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL -- error (system is not in MASTER mode)
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 *
 * NOTE:     if rate = 0, that means no rate limit.
 *---------------------------------------------------------------------------*/
UI32_T  DHCPSNP_PMGR_GetRunningOptionDelimiter(UI8_T *delimiter);
#endif


/*------------------------------------------------------------------------------
 * ROUTINE NAME  - DHCPSNP_PMGR_RxProcessPacket
 *------------------------------------------------------------------------------
 * PURPOSE: Process received packet
 * INPUT :  mref_p      --  L_MM_Mref descriptor
 *          dst_mac     --  destination mac address
 *          src_mac     --  source mac address
 *          ing_vid     --  ingress vlan id
 *          ing_lport   --  ingress logical port
 *          new_mref_pp	--	new allocated L_MM_Mref descriptor
 * OUTPUT:  None
 * RETURN:  TRUE/FALSE
 * NOTES :  None
 *------------------------------------------------------------------------------
 */
BOOL_T DHCPSNP_PMGR_RxProcessPacket(
    L_MM_Mref_Handle_T    *mref_p,
    UI32_T                pkt_len,
    UI8_T                 dst_mac[SYS_ADPT_MAC_ADDR_LEN],
    UI8_T                 src_mac[SYS_ADPT_MAC_ADDR_LEN],
    UI32_T                ing_vid,
    UI32_T                ing_lport,
    L_MM_Mref_Handle_T  **new_mref_pp
);
#if (SYS_CPNT_DHCP_RELAY == TRUE)
/*------------------------------------------------------------------------------
 * ROUTINE NAME  - DHCPSNP_PMGR_SetL3RelayForwarding
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
UI32_T DHCPSNP_PMGR_SetL3RelayForwarding(UI32_T vid, BOOL_T forwarding);
#endif  //SYS_CPNT_DHCP_RELAY

#if (SYS_CPNT_DHCP_RELAY_OPTION82 == TRUE)
/*------------------------------------------------------------------------------
 * ROUTINE NAME  - DHCPSNP_PMGR_SetL2RelayForwarding
 *------------------------------------------------------------------------------
 * PURPOSE: Set L2 forwarding flag for DHCPSNP to know if the packet needs to forward
 * INPUT :  forwarding  -   needs forwarding or not
 * OUTPUT:  None
 * RETURN:  DHCPSNP_TYPE_OK/
 *          DHCPSNP_TYPE_FAIL
 * NOTES :  None
 *------------------------------------------------------------------------------
 */
UI32_T DHCPSNP_PMGR_SetL2RelayForwarding(BOOL_T forwarding);
#endif  //SYS_CPNT_DHCP_RELAY_OPTION82
#endif /* #ifndef DHCPSNP_PMGR_H */
