/*-----------------------------------------------------------------------------
 * Module Name: lldp_om.h
 *-----------------------------------------------------------------------------
 * PURPOSE: Definitions for the LLDP object manager
 *-----------------------------------------------------------------------------
 * NOTES:
 *
 *-----------------------------------------------------------------------------
 * HISTORY:
 *    11/17/2005 - Gordon Kao, Created
 *
 *
 *-----------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2005
 *-----------------------------------------------------------------------------
 */

#ifndef LLDP_OM_H
#define LLDP_OM_H


/* INCLUDE FILE DECLARATIONS
 */

#include "sys_type.h"
#include "sysfun.h"
#include "lldp_type.h"


/* NAMING CONSTANT DECLARATIONS
 */

#define LLDP_OM_IPCMSG_TYPE_SIZE sizeof(union LLDP_OM_IpcMsg_Type_U)

/* command used in IPC message
 */
enum
{
    LLDP_OM_IPC_GETSYSADMINSTATUS,
    LLDP_OM_IPC_GETRUNNINGSYSADMINSTATUS,
    LLDP_OM_IPC_GETSYSCONFIGENTRY,
    LLDP_OM_IPC_GETREMOTESYSTEMDATA,
    LLDP_OM_IPC_GETNEXTREMMANADDRBYINDEX,
    LLDP_OM_IPC_GETRUNNINGMSGTXHOLDMUL,
    LLDP_OM_IPC_GETRUNNINGMSGTXINTERVAL,
    LLDP_OM_IPC_GETRUNNINGNOTIFYINTERVAL,
    LLDP_OM_IPC_GETRUNNINGREINITDELAY,
    LLDP_OM_IPC_GETRUNNINGTXDELAY,
    LLDP_OM_IPC_GETRUNNINGPORTADMINSTATUS,
    LLDP_OM_IPC_GETRUNNINGPORTBASICTLVTRANSFER,
    LLDP_OM_IPC_GETRUNNINGPORTMANADDRTLVTRANSFER,
    LLDP_OM_IPC_GETRUNNINGPORTNOTIFICATIONENABLE,
    LLDP_OM_IPC_GETRUNNINGXDOT1CONFIGENTRY,
    LLDP_OM_IPC_GETXDOT1REMENTRY,
    LLDP_OM_IPC_GETXDOT1REMPROTOVLANENTRY,
    LLDP_OM_IPC_GETNEXTXDOT1REMPROTOVLANENTRYBYINDEX,
    LLDP_OM_IPC_GETXDOT1REMVLANNAMEENTRY,
    LLDP_OM_IPC_GETNEXTXDOT1REMVLANNAMEENTRYBYINDEX,
    LLDP_OM_IPC_GETXDOT1REMPROTOCOLENTRY,
    LLDP_OM_IPC_GETNEXTXDOT1REMPROTOCOLENTRYBYINDEX,
    LLDP_OM_IPC_GETXDOT1REMCNENTRY,
    LLDP_OM_IPC_GETRUNNINGXDOT3PORTCONFIGENTRY,
    LLDP_OM_IPC_GETXDOT3REMPORTENTRY,
    LLDP_OM_IPC_GETXDOT3REMPOWERENTRY,
    LLDP_OM_IPC_GETXDOT3REMLINKAGGENTRY,
    LLDP_OM_IPC_GETXDOT3REMMAXFRAMESIZEENTRY,
    LLDP_OM_IPC_GETXMEDCONFIGENTRY,
    LLDP_OM_IPC_GETRUNNINGXMEDFASTSTARTREPEATCOUNT,
    LLDP_OM_IPC_GETXMEDREMCAPENTRY,
    LLDP_OM_IPC_GETXMEDREMMEDIAPOLICYENTRY,
    LLDP_OM_IPC_GETXMEDREMINVENTORYENTRY,
    LLDP_OM_IPC_GETXMEDREMLOCATIONENTRY,
    LLDP_OM_IPC_GETXMEDREMPOEENTRY,
    LLDP_OM_IPC_GETXMEDREMPOEPSEENTRY,
    LLDP_OM_IPC_GETXMEDREMPOEPDENTRY,
    LLDP_OM_IPC_GETALLREMINDEXBYPORT,
};


/* MACRO FUNCTION DECLARATIONS
 */

/* Macro function for computation of IPC msg_buf size based on field name
 * used in VLAN_OM_IpcMsg_T.data
 */
#define LLDP_OM_GET_MSG_SIZE(field_name)                        \
            (LLDP_OM_IPCMSG_TYPE_SIZE +                         \
            sizeof(((LLDP_OM_IpcMsg_T*)0)->data.field_name))


/* DATA TYPE DECLARATIONS
 */

/* IPC message structure
 */
typedef struct
{
    union LLDP_OM_IpcMsg_Type_U
    {
        UI32_T cmd;
        BOOL_T ret_bool;
		UI32_T ret_ui32;
    } type; /* the intended action or return value */

    union
    {
        UI32_T                               arg_ui32;
        LLDP_MGR_SysConfigEntry_T            arg_sys_config_entry;
        LLDP_MGR_RemoteSystemData_T          arg_remote_system_data;
        LLDP_MGR_RemoteManagementAddrEntry_T arg_remote_mgmt_addr_entry;
#if (LLDP_TYPE_EXT_802DOT1 == TRUE)
        LLDP_MGR_Xdot1ConfigEntry_T          arg_xdot1_config_entry;
        LLDP_MGR_Xdot1RemEntry_T             arg_xdot1_rem_entry;
        LLDP_MGR_Xdot1RemProtoVlanEntry_T    arg_xdot1_rem_proto_vlan_entry;
        LLDP_MGR_Xdot1RemVlanNameEntry_T     arg_xdot1_rem_vlan_name_entry;
        LLDP_MGR_Xdot1RemProtocolEntry_T     arg_xdot1_rem_protocol_entry;
#if (SYS_CPNT_CN == TRUE)
        LLDP_MGR_Xdot1RemCnEntry_T           arg_xdot1_rem_cn_entry;
#endif
#endif
#if (LLDP_TYPE_EXT_802DOT3 == TRUE)
        LLDP_MGR_Xdot3PortConfigEntry_T      arg_xdot3_port_config_entry;
        LLDP_MGR_Xdot3RemPortEntry_T         arg_xdot3_rem_port_entry;
        LLDP_MGR_Xdot3RemPowerEntry_T        arg_xdot3_rem_power_entry;
        LLDP_MGR_Xdot3RemLinkAggEntry_T      arg_xdot3_rem_link_agg_entry;
        LLDP_MGR_Xdot3RemMaxFrameSizeEntry_T arg_xdot3_rem_max_frame_size_entry;
#endif
#if (LLDP_TYPE_MED == TRUE)
        LLDP_MGR_XMedConfig_T                arg_xmed_config_entry;
        LLDP_MGR_XMedRemCapEntry_T           arg_xmed_rem_cap_entry;
        LLDP_MGR_XMedRemMediaPolicyEntry_T   arg_xmed_rem_med_policy_entry;
        LLDP_MGR_XMedRemInventoryEntry_T     arg_xmed_rem_inventory_entry;
        LLDP_MGR_XMedRemLocationEntry_T      arg_xmed_rem_location_entry;
        LLDP_MGR_XMedRemPoeEntry_T           arg_xmed_rem_poe_entry;
        LLDP_MGR_XMedRemPoePseEntry_T        arg_xmed_rem_pse_entry;
        LLDP_MGR_XMedRemPoePdEntry_T         arg_xmed_rem_pd_entry;
#endif
        struct
        {
            UI32_T arg_ui32;
            UI8_T  arg_ui8;
        } arg_grp_ui32_ui8;
        struct
        {
            UI32_T arg_ui32;
            UI8_T  arg_ui8_1;
            UI8_T  arg_ui8_2;
        } arg_grp_ui32_ui8_ui8;
        struct
        {
            UI32_T arg_ui32;
            UI32_T arg_index[LLDP_TYPE_MAX_NBR_OF_REM_DATA_ENTRY_PER_PORT];
        } arg_grp_ui32_index;
    } data;
} LLDP_OM_IpcMsg_T;


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetRemDataPoolFreeNum
 *-------------------------------------------------------------------------
 * PURPOSE  : Get number of free remote-data entry in pool
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : UI32_T    -- free entry number
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_OM_GetRemDataPoolFreeNum(void) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetPortRemDataNum
 *-------------------------------------------------------------------------
 * PURPOSE  : Get number of free remote-data entry in port list
 * INPUT    : lport
 * OUTPUT   : None
 * RETUEN   : UI32_T    -- free entry number
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_OM_GetPortRemDataNum(UI32_T lport) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetTotalNumOfRemData
 *-------------------------------------------------------------------------
 * PURPOSE  : Get total number of remote data in the DB
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : UI32_T
 * NOTE     : This function is used by backdoor
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_OM_GetTotalNumOfRemData() ;


/*=============================================================================
 * Moved from lldp_mgr.h
 *=============================================================================
 */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetSysAdminStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP global admin status
 * INPUT    : None
 * OUTPUT   : admin_status
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_OM_GetSysAdminStatus(UI32_T *admin_status) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetSysAdminStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP global admin status
 * INPUT    : None
 * OUTPUT   : admin_status
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_OM_GetRunningSysAdminStatus(UI32_T *admin_status) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetConfigEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get configuration entry info. for system.
 * INPUT    : LLDP_MGR_SysConfigEntry_T  *config_entry
 * OUTPUT   : LLDP_MGR_SysConfigEntry_T  *config_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 12.2, IEEE Std 802.1AB-2005.
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_OM_GetSysConfigEntry(LLDP_MGR_SysConfigEntry_T *config_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetRemoteSystemData
 *-------------------------------------------------------------------------
 * PURPOSE  : Get related TLV info. in the remote system.
 * INPUT    : LLDP_MGR_RemoteSystemData_T *system_entry
 * OUTPUT   : LLDP_MGR_RemoteSystemData_T *system_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 9.5.2.2/9.5.2.3/9.5.3.3/9.5.5.2/
 *            9.5.6.2/9.5.7.2/9.5.8.1/9.5.8.2, IEEE Std 802.1AB-2005.
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_OM_GetRemoteSystemData(LLDP_MGR_RemoteSystemData_T *system_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetNextRemoteManagementAddressTlvEntryByPort
 *-------------------------------------------------------------------------
 * PURPOSE  : Get management address TLV info. in the remote system.
 * INPUT    : LLDP_MGR_RemoteManagementAddrEntry_T *man_addr_entry
 * OUTPUT   : LLDP_MGR_RemoteManagementAddrEntry_T *man_addr_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : This function is used by CLI/WEB
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_OM_GetNextRemManAddrByIndex(LLDP_MGR_RemoteManagementAddrEntry_T *man_addr_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetRunningMsgTxHoldMul
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
UI32_T LLDP_OM_GetRunningMsgTxHoldMul(UI32_T  *msg_tx_hold) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetRunningMsgTxInterval
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
UI32_T LLDP_OM_GetRunningMsgTxInterval(UI32_T  *msg_tx_interval) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetRunningNotifyInterval
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
UI32_T LLDP_OM_GetRunningNotifyInterval(UI32_T *notify_interval) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetRunningReinitDelay
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
UI32_T LLDP_OM_GetRunningReinitDelay(UI32_T *reinit_delay) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetRunningTxDelay
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
UI32_T LLDP_OM_GetRunningTxDelay(UI32_T  *tx_delay_time) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetRunningPortAdminStatus
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
UI32_T LLDP_OM_GetRunningPortAdminStatus(UI32_T lport, UI8_T *admin_status) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetRunningPortBasicTlvTransfer
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
UI32_T LLDP_OM_GetRunningPortBasicTlvTransfer(UI32_T lport, UI8_T *basic_tlvs_tx_flag, UI8_T *basic_tlvs_change_flag) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetRunningPortManAddrTlvTransfer
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
UI32_T LLDP_OM_GetRunningPortManAddrTlvTransfer(UI32_T lport, UI8_T *man_addr_tlv_enable) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetRunningPortNotify
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
UI32_T LLDP_OM_GetRunningPortNotificationEnable(UI32_T lport, BOOL_T *notify_enable) ;

#if (LLDP_TYPE_EXT_802DOT1 == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetRunningXdot1ConfigEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get current 802.1 extension configuraiton
 * INPUT    : xdot1_config_entry
 * OUTPUT   : xdot1_config_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_OM_GetRunningXdot1ConfigEntry(LLDP_MGR_Xdot1ConfigEntry_T *xdot1_config_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetXdot1RemEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.1 extension remote entry
 * INPUT    : xdot1_rem_entry
 * OUTPUT   : xdot1_rem_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_OM_GetXdot1RemEntry(LLDP_MGR_Xdot1RemEntry_T *xdot1_rem_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetXdot1RemProtoVlanEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.1 extension remote protocol vlan entry
 * INPUT    : xdot1_rem_proto_vlan_entry
 * OUTPUT   : xdot1_rem_proto_vlan_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_OM_GetXdot1RemProtoVlanEntry(LLDP_MGR_Xdot1RemProtoVlanEntry_T *xdot1_rem_proto_vlan_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetNextXdot1RemProtoVlanEntryByIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.1 extension remote protocol vlan entry
 * INPUT    : xdot1_rem_proto_vlan_entry
 * OUTPUT   : xdot1_rem_proto_vlan_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : for WEB/CLI
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_OM_GetNextXdot1RemProtoVlanEntryByIndex(LLDP_MGR_Xdot1RemProtoVlanEntry_T *xdot1_rem_proto_vlan_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetXdot1RemProtoVlanEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.1 extension remote vlan name entry
 * INPUT    : xdot1_rem_vlan_name_entry
 * OUTPUT   : xdot1_rem_vlan_name_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_OM_GetXdot1RemVlanNameEntry(LLDP_MGR_Xdot1RemVlanNameEntry_T *xdot1_rem_vlan_name_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetNextXdot1RemVlanNameEntryByIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.1 extension remote vlan name entry
 * INPUT    : xdot1_rem_vlan_name_entry
 * OUTPUT   : xdot1_rem_vlan_name_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : for WEB/CLI
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_OM_GetNextXdot1RemVlanNameEntryByIndex(LLDP_MGR_Xdot1RemVlanNameEntry_T *xdot1_rem_vlan_name_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetXdot1RemProtocolEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.1 extension remote protocol entry
 * INPUT    : xdot1_rem_protocol_entry
 * OUTPUT   : xdot1_rem_protocol_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_OM_GetXdot1RemProtocolEntry(LLDP_MGR_Xdot1RemProtocolEntry_T *xdot1_rem_protocol_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetNextXdot1RemProtcolEntryByIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.1 extension remote protocol entry
 * INPUT    : xdot1_rem_protocol_entry
 * OUTPUT   : xdot1_rem_protocol_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : for WEB/CLI
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_OM_GetNextXdot1RemProtocolEntryByIndex(LLDP_MGR_Xdot1RemProtocolEntry_T * xdot1_rem_protocol_entry) ;
#endif /* #if (LLDP_TYPE_EXT_802DOT1 == TRUE) */

#if (LLDP_TYPE_EXT_802DOT3 == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetRunningXdot3PortConfigEntry
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
UI32_T  LLDP_OM_GetRunningXdot3PortConfigEntry(LLDP_MGR_Xdot3PortConfigEntry_T * xdot3_port_config_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetXdot3RemPortEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.3 extension remote port entry
 * INPUT    : xdot3_rem_port_entry
 * OUTPUT   : xdot3_rem_port_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_OM_GetXdot3RemPortEntry(LLDP_MGR_Xdot3RemPortEntry_T *xdot3_rem_port_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetXdot3RemPowerEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.3 extension remote power entry
 * INPUT    : xdot3_rem_power_entry
 * OUTPUT   : xdot3_rem_power_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_OM_GetXdot3RemPowerEntry(LLDP_MGR_Xdot3RemPowerEntry_T *xdot3_rem_power_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetXdot3RemLinkAggEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.3 extension remote link aggregation entry
 * INPUT    : xdot3_rem_link_agg_entry
 * OUTPUT   : xdot3_rem_link_agg_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_OM_GetXdot3RemLinkAggEntry(LLDP_MGR_Xdot3RemLinkAggEntry_T *xdot3_rem_link_agg_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetXdot3RemMaxFrameSizeEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.3 extension remote maximum frame size entry
 * INPUT    : xdot3_rem_max_frame_size_entry
 * OUTPUT   : xdot3_rem_max_frame_size_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_OM_GetXdot3RemMaxFrameSizeEntry(LLDP_MGR_Xdot3RemMaxFrameSizeEntry_T *xdot3_rem_max_frame_size_entry) ;
#endif /* #if (LLDP_TYPE_EXT_802DOT3 == TRUE) */

#if (LLDP_TYPE_MED == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetXMedConfigEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED configuration entry
 * INPUT    : None
 * OUTPUT   : xmed_config_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T LLDP_OM_GetXMedConfigEntry(LLDP_MGR_XMedConfig_T *xmed_config_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetRunningXMedFastStartRepeatCount
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
UI32_T LLDP_OM_GetRunningXMedFastStartRepeatCount(UI32_T  *repeat_count) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetXMedRemCapEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED remote device capability entry
 * INPUT    : None
 * OUTPUT   : rem_cap_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_OM_GetXMedRemCapEntry(LLDP_MGR_XMedRemCapEntry_T *rem_cap_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetXMedRemMediaPolicyEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED remote device media policy entry
 * INPUT    : None
 * OUTPUT   : rem_med_policy_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_OM_GetXMedRemMediaPolicyEntry(LLDP_MGR_XMedRemMediaPolicyEntry_T *rem_med_policy_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetXMedRemInventoryEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED remote device inventory entry
 * INPUT    : None
 * OUTPUT   : rem_inventory_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_OM_GetXMedRemInventoryEntry(LLDP_MGR_XMedRemInventoryEntry_T *rem_inventory_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetXMedRemLocationEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED remote device location entry
 * INPUT    : None
 * OUTPUT   : rem_location_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_OM_GetXMedRemLocationEntry(LLDP_MGR_XMedRemLocationEntry_T *rem_location_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetXMedRemPoeEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED remote device poe entry
 * INPUT    : None
 * OUTPUT   : rem_poe_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_OM_GetXMedRemPoeEntry(LLDP_MGR_XMedRemPoeEntry_T *rem_poe_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetXMedRemPoePseEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED remote device poe pse entry
 * INPUT    : None
 * OUTPUT   : rem_pse_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_OM_GetXMedRemPoePseEntry(LLDP_MGR_XMedRemPoePseEntry_T *rem_pse_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetXMedRemPoePdEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED remote device poe pse entry
 * INPUT    : None
 * OUTPUT   : rem_pd_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_OM_GetXMedRemPoePdEntry(LLDP_MGR_XMedRemPoePdEntry_T *rem_pd_entry) ;
#endif /* #if (LLDP_TYPE_MED == TRUE) */

#if (SYS_CPNT_ADD == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetNextTelephoneDevice
 *-------------------------------------------------------------------------
 * PURPOSE  : To get next telephone device
 * INPUT    : lport, rem_dev_index
 * OUTPUT   : rem_dev_index, mac_addr, network_addr_subtype, network_addr,
 *             network_addr_len, network_addr_ifindex
 * RETURN   : TRUE/FALSE
 * NOTE     : To get first entry, rem_dev_index is set to zero.
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_OM_GetNextTelephoneDevice(UI32_T lport,
                                        UI32_T *rem_dev_index,
                                        UI8_T *mac_addr,
                                        UI8_T *network_addr_subtype,
                                        UI8_T *network_addr,
                                        UI8_T *network_addr_len,
                                        UI32_T *network_addr_ifindex);
#endif /* #if (SYS_CPNT_ADD == TRUE) */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_GetAllRemIndexByPort
 *-------------------------------------------------------------------------
 * PURPOSE  : Get all remote index for a port
 * INPUT    : lport
 * OUTPUT   : remote_index
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_OM_GetAllRemIndexByPort(UI32_T lport, UI32_T remote_index[LLDP_TYPE_MAX_NBR_OF_REM_DATA_ENTRY_PER_PORT]) ;

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_OM_HandleIPCReqMsg
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the ipc request message for LLDP OM.
 *
 * INPUT   : msgbuf_p -- input request ipc message buffer
 *
 * OUTPUT  : msgbuf_p -- output response ipc message buffer
 *
 * RETURN  : TRUE  - there is a response required to be sent
 *           FALSE - there is no response required to be sent
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T LLDP_OM_HandleIPCReqMsg(SYSFUN_Msg_T* msgbuf_p);


#endif /* #ifndef LLDP_OM_H */
