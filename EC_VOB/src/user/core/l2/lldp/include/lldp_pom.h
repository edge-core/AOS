/*-----------------------------------------------------------------------------
 * FILE NAME: LLDP_POM.H
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *    This file declares the APIs for LLDP OM IPC.
 *
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    2007/07/07     --- Timon, Create
 *
 * Copyright(C)      Accton Corporation, 2007
 *-----------------------------------------------------------------------------
 */

#ifndef LLDP_POM_H
#define LLDP_POM_H


/* INCLUDE FILE DECLARATIONS
 */

#include "sys_type.h"
#include "lldp_type.h"


/* NAMING CONSTANT DECLARATIONS
 */


/* MACRO FUNCTION DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : LLDP_POM_InitiateProcessResources
 *-----------------------------------------------------------------------------
 * PURPOSE : Initiate resources for LLDP_POM in the calling process.
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
BOOL_T LLDP_POM_InitiateProcessResources(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_POM_GetSysAdminStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP global admin status
 * INPUT    : None
 * OUTPUT   : admin_status
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_POM_GetSysAdminStatus(UI32_T *admin_status);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_POM_GetRunningSysAdminStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP global admin status
 * INPUT    : None
 * OUTPUT   : admin_status
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_POM_GetRunningSysAdminStatus(UI32_T *admin_status);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_POM_GetSysConfigEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get configuration entry info. for system.
 * INPUT    : LLDP_MGR_SysConfigEntry_T  *config_entry
 * OUTPUT   : LLDP_MGR_SysConfigEntry_T  *config_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 12.2, IEEE Std 802.1AB-2005.
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_POM_GetSysConfigEntry(LLDP_MGR_SysConfigEntry_T *config_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_POM_GetRemoteSystemData
 *-------------------------------------------------------------------------
 * PURPOSE  : Get related TLV info. in the remote system.
 * INPUT    : LLDP_MGR_RemoteSystemData_T *system_entry
 * OUTPUT   : LLDP_MGR_RemoteSystemData_T *system_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 9.5.2.2/9.5.2.3/9.5.3.3/9.5.5.2/
 *            9.5.6.2/9.5.7.2/9.5.8.1/9.5.8.2, IEEE Std 802.1AB-2005.
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_POM_GetRemoteSystemData(LLDP_MGR_RemoteSystemData_T *system_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_POM_GetNextRemManAddrByIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : Get management address TLV info. in the remote system.
 * INPUT    : LLDP_MGR_RemoteManagementAddrEntry_T *man_addr_entry
 * OUTPUT   : LLDP_MGR_RemoteManagementAddrEntry_T *man_addr_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : This function is used by CLI/WEB
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_POM_GetNextRemManAddrByIndex(LLDP_MGR_RemoteManagementAddrEntry_T *man_addr_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_POM_GetRunningMsgTxHoldMul
 *-------------------------------------------------------------------------
 * PURPOSE  : Get running configuration
 * INPUT    : None
 * OUTPUT   : UI32_T  *msg_tx_hold
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_POM_GetRunningMsgTxHoldMul(UI32_T  *msg_tx_hold);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_POM_GetRunningMsgTxInterval
 *-------------------------------------------------------------------------
 * PURPOSE  : Get running configuration
 * INPUT    : None
 * OUTPUT   : UI32_T  *msg_tx_interval
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_POM_GetRunningMsgTxInterval(UI32_T  *msg_tx_interval);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_POM_GetRunningNotifyInterval
 *-------------------------------------------------------------------------
 * PURPOSE  : Get running configuration
 * INPUT    : None
 * OUTPUT   : UI32_T *notify_interval
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_POM_GetRunningNotifyInterval(UI32_T *notify_interval);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_POM_GetRunningReinitDelay
 *-------------------------------------------------------------------------
 * PURPOSE  : Get running configuration
 * INPUT    : None
 * OUTPUT   : UI32_T *reinit_delay
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_POM_GetRunningReinitDelay(UI32_T *reinit_delay);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_POM_GetRunningTxDelay
 *-------------------------------------------------------------------------
 * PURPOSE  : Get running configuration
 * INPUT    : None
 * OUTPUT   : UI32_T  *tx_delay_time
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_POM_GetRunningTxDelay(UI32_T  *tx_delay_time);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_POM_GetRunningPortAdminStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : Get running configuration
 * INPUT    : lport
 * OUTPUT   : basic_tlv_enable
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_POM_GetRunningPortAdminStatus(UI32_T lport, UI8_T *admin_status);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_POM_GetRunningPortBasicTlvTransfer
 *-------------------------------------------------------------------------
 * PURPOSE  : Get running configuration
 * INPUT    : lport
 * OUTPUT   : basic_tlvs_tx_flag, basic_tlvs_change_flag
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_POM_GetRunningPortBasicTlvTransfer(UI32_T lport, UI8_T *basic_tlvs_tx_flag, UI8_T *basic_tlvs_change_flag) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_POM_GetRunningPortManAddrTlvTransfer
 *-------------------------------------------------------------------------
 * PURPOSE  : Get running configuration
 * INPUT    : lport
 * OUTPUT   : man_addr_tlv_enable
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_POM_GetRunningPortManAddrTlvTransfer(UI32_T lport, UI8_T *man_addr_tlv_enable);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_POM_GetRunningPortNotificationEnable
 *-------------------------------------------------------------------------
 * PURPOSE  : Get running configuration
 * INPUT    : lport
 * OUTPUT   : notify_enable
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_POM_GetRunningPortNotificationEnable(UI32_T lport, BOOL_T *notify_enable);

#if (LLDP_TYPE_EXT_802DOT1 == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_POM_GetRunningXdot1ConfigEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get current 802.1 extension configuraiton
 * INPUT    : xdot1_config_entry
 * OUTPUT   : xdot1_config_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_POM_GetRunningXdot1ConfigEntry(LLDP_MGR_Xdot1ConfigEntry_T *xdot1_config_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_POM_GetXdot1RemEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.1 extension remote entry
 * INPUT    : xdot1_rem_entry
 * OUTPUT   : xdot1_rem_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_POM_GetXdot1RemEntry(LLDP_MGR_Xdot1RemEntry_T *xdot1_rem_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_POM_GetXdot1RemProtoVlanEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.1 extension remote protocol vlan entry
 * INPUT    : xdot1_rem_proto_vlan_entry
 * OUTPUT   : xdot1_rem_proto_vlan_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_POM_GetXdot1RemProtoVlanEntry(LLDP_MGR_Xdot1RemProtoVlanEntry_T *xdot1_rem_proto_vlan_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_POM_GetNextXdot1RemProtoVlanEntryByIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.1 extension remote protocol vlan entry
 * INPUT    : xdot1_rem_proto_vlan_entry
 * OUTPUT   : xdot1_rem_proto_vlan_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : for WEB/CLI
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_POM_GetNextXdot1RemProtoVlanEntryByIndex(LLDP_MGR_Xdot1RemProtoVlanEntry_T *xdot1_rem_proto_vlan_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_POM_GetXdot1RemProtoVlanEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.1 extension remote vlan name entry
 * INPUT    : xdot1_rem_vlan_name_entry
 * OUTPUT   : xdot1_rem_vlan_name_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_POM_GetXdot1RemVlanNameEntry(LLDP_MGR_Xdot1RemVlanNameEntry_T *xdot1_rem_vlan_name_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_POM_GetNextXdot1RemVlanNameEntryByIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.1 extension remote vlan name entry
 * INPUT    : xdot1_rem_vlan_name_entry
 * OUTPUT   : xdot1_rem_vlan_name_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : for WEB/CLI
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_POM_GetNextXdot1RemVlanNameEntryByIndex(LLDP_MGR_Xdot1RemVlanNameEntry_T *xdot1_rem_vlan_name_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_POM_GetXdot1RemProtocolEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.1 extension remote protocol entry
 * INPUT    : xdot1_rem_protocol_entry
 * OUTPUT   : xdot1_rem_protocol_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_POM_GetXdot1RemProtocolEntry(LLDP_MGR_Xdot1RemProtocolEntry_T *xdot1_rem_protocol_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_POM_GetNextXdot1RemProtocolEntryByIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.1 extension remote protocol entry
 * INPUT    : xdot1_rem_protocol_entry
 * OUTPUT   : xdot1_rem_protocol_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : for WEB/CLI
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_POM_GetNextXdot1RemProtocolEntryByIndex(LLDP_MGR_Xdot1RemProtocolEntry_T * xdot1_rem_protocol_entry);

#if (SYS_CPNT_CN == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_POM_GetXdot1RemCnEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.1 extension remote congestion notification entry
 * INPUT    : xdot1_rem_cn_entry
 * OUTPUT   : xdot1_rem_cn_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_POM_GetXdot1RemCnEntry(LLDP_MGR_Xdot1RemCnEntry_T *xdot1_rem_cn_entry);
#endif /* #if (SYS_CPNT_CN == TRUE) */
#endif /* #if (LLDP_TYPE_EXT_802DOT1 == TRUE) */

#if (LLDP_TYPE_EXT_802DOT3 == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_POM_GetRunningXdot3PortConfigEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get current 802.3 extension port config entry
 * INPUT    : xdot3_port_config_entry
 * OUTPUT   : xdot3_port_config_entry
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_POM_GetRunningXdot3PortConfigEntry(LLDP_MGR_Xdot3PortConfigEntry_T * xdot3_port_config_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_POM_GetXdot3RemPortEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.3 extension remote port entry
 * INPUT    : xdot3_rem_port_entry
 * OUTPUT   : xdot3_rem_port_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_POM_GetXdot3RemPortEntry(LLDP_MGR_Xdot3RemPortEntry_T *xdot3_rem_port_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_POM_GetXdot3RemPowerEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.3 extension remote power entry
 * INPUT    : xdot3_rem_power_entry
 * OUTPUT   : xdot3_rem_power_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_POM_GetXdot3RemPowerEntry(LLDP_MGR_Xdot3RemPowerEntry_T *xdot3_rem_power_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_POM_GetXdot3RemLinkAggEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.3 extension remote link aggregation entry
 * INPUT    : xdot3_rem_link_agg_entry
 * OUTPUT   : xdot3_rem_link_agg_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_POM_GetXdot3RemLinkAggEntry(LLDP_MGR_Xdot3RemLinkAggEntry_T *xdot3_rem_link_agg_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_POM_GetXdot3RemMaxFrameSizeEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.3 extension remote maximum frame size entry
 * INPUT    : xdot3_rem_max_frame_size_entry
 * OUTPUT   : xdot3_rem_max_frame_size_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_POM_GetXdot3RemMaxFrameSizeEntry(LLDP_MGR_Xdot3RemMaxFrameSizeEntry_T *xdot3_rem_max_frame_size_entry);
#endif /* #if (LLDP_TYPE_EXT_802DOT3 == TRUE) */

#if (LLDP_TYPE_MED == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_POM_GetXMedConfigEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED configuration entry
 * INPUT    : None
 * OUTPUT   : xmed_config_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T LLDP_POM_GetXMedConfigEntry(LLDP_MGR_XMedConfig_T *xmed_config_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_POM_GetRunningXMedFastStartRepeatCount
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED running cfg fast start repeat count
 * INPUT    : repeat_count
 * OUTPUT   : repeat_count
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_POM_GetRunningXMedFastStartRepeatCount(UI32_T  *repeat_count) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_POM_GetXMedRemCapEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED remote device capability entry
 * INPUT    : None
 * OUTPUT   : rem_cap_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_POM_GetXMedRemCapEntry(LLDP_MGR_XMedRemCapEntry_T *rem_cap_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_POM_GetXMedRemMediaPolicyEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED remote device media policy entry
 * INPUT    : None
 * OUTPUT   : rem_med_policy_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_POM_GetXMedRemMediaPolicyEntry(LLDP_MGR_XMedRemMediaPolicyEntry_T *rem_med_policy_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_POM_GetXMedRemInventoryEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED remote device inventory entry
 * INPUT    : None
 * OUTPUT   : rem_inventory_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_POM_GetXMedRemInventoryEntry(LLDP_MGR_XMedRemInventoryEntry_T *rem_inventory_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_POM_GetXMedRemLocationEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED remote device location entry
 * INPUT    : None
 * OUTPUT   : rem_location_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_POM_GetXMedRemLocationEntry(LLDP_MGR_XMedRemLocationEntry_T *rem_location_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_POM_GetXMedRemPoeEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED remote device poe entry
 * INPUT    : None
 * OUTPUT   : rem_poe_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_POM_GetXMedRemPoeEntry(LLDP_MGR_XMedRemPoeEntry_T *rem_poe_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_POM_GetXMedRemPoePseEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED remote device poe pse entry
 * INPUT    : None
 * OUTPUT   : rem_pse_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_POM_GetXMedRemPoePseEntry(LLDP_MGR_XMedRemPoePseEntry_T *rem_pse_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_POM_GetXMedRemPoePdEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED remote device poe pse entry
 * INPUT    : None
 * OUTPUT   : rem_pd_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_POM_GetXMedRemPoePdEntry(LLDP_MGR_XMedRemPoePdEntry_T *rem_pd_entry) ;
#endif /* #if (LLDP_TYPE_MED == TRUE) */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_POM_GetAllRemIndexByPort
 *-------------------------------------------------------------------------
 * PURPOSE  : Get all remote index for a port
 * INPUT    : lport
 * OUTPUT   : remote_index
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_POM_GetAllRemIndexByPort(UI32_T lport, UI32_T remote_index[LLDP_TYPE_MAX_NBR_OF_REM_DATA_ENTRY_PER_PORT]) ;

#endif /* #ifndef LLDP_POM_H */
