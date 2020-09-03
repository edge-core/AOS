/*-----------------------------------------------------------------------------
 * FILE NAME: LLDP_PMGR.H
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *    This file declares the APIs for LLDP MGR IPC.
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

#ifndef LLDP_PMGR_H
#define LLDP_PMGR_H


/* INCLUDE FILE DECLARATIONS
 */

#include "sys_type.h"
#include "lldp_mgr.h"


/* NAMING CONSTANT DECLARATIONS
 */


/* MACRO FUNCTION DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : LLDP_PMGR_InitiateProcessResources
 *-----------------------------------------------------------------------------
 * PURPOSE : Initiate resources for LLDP_PMGR in the calling process.
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
BOOL_T LLDP_PMGR_InitiateProcessResources(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_SetSysAdminStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : Set LLDP global admin status
 * INPUT    : admin_status
 * OUTPUT   : None
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_PMGR_SetSysAdminStatus(UI32_T admin_status) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_SetMsgTxInterval
 *-------------------------------------------------------------------------
 * PURPOSE  : Set msg_tx_interval to determine interval at which LLDP frames are transmitted
 * INPUT    : UI32_T msg_tx_interval    --  time value
 * OUTPUT   : None
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 10.5.3.3, IEEE Std 802.1AB-2005.
 *            Range: 5~32768
 *            Default value: 30 seconds.
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_PMGR_SetMsgTxInterval(UI32_T msg_tx_interval) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_SetMsgTxHoldMul
 *-------------------------------------------------------------------------
 * PURPOSE  : Set msg_hold time multiplier to determine the actual TTL value used in an LLDPDU.
 * INPUT    : UI32_T msg_tx_hold_multiplier      --  a multiplier on the msgTxInterval
 * OUTPUT   : None
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 10.5.3.3, IEEE Std 802.1AB-2005.
 *            Range: 2~10
 *            Default value: 4.
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_PMGR_SetMsgTxHoldMul(UI32_T msg_tx_hold_mul) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_SetReinitDelay
 *-------------------------------------------------------------------------
 * PURPOSE  : Set the amount of delay from when adminStatus becomes ¡§disabled¡¦
 *            until re-initialization will be attempted.
 * INPUT    : UI32_T reinit_delay    --  time value
 * OUTPUT   : None
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 10.5.3.3, IEEE Std 802.1AB-2005.
 *            Range: 1~10
 *            Default value: 2 seconds.
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_PMGR_SetReinitDelay(UI32_T reinit_delay) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_SetTxDelay
 *-------------------------------------------------------------------------
 * PURPOSE  : Set the minimum delay between successive LLDP frame transmissions.
 * INPUT    : UI32_T tx_delay    --  time value
 * OUTPUT   : None
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 10.5.3.3, IEEE Std 802.1AB-2005.
 *            Range: 1~8192
 *            Default value: 2 seconds.
 *            The recommended value is set by the following formula:
 *              1 <= lldpTxDelay <= (0.25 * lldpMessageTxInterval)
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_PMGR_SetTxDelay(UI32_T tx_delay) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_SetNotificationInterval
 *-------------------------------------------------------------------------
 * PURPOSE  : Set this value to control the transmission of LLDP notifications.
 * INPUT    : UI32_T interval_time    --  time value
 * OUTPUT   : None
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 12.2, IEEE Std 802.1AB-2005.
 *            Range: 5~3600
 *            Default value: 5 seconds.
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_PMGR_SetNotificationInterval(UI32_T notify_interval_time) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_SetPortConfigAdminStatus
 *-------------------------------------------------------------------------
 * PURPOSE  :  Set admin_status to control whether or not a local LLDP agent is
 *             enabled(transmit and receive, transmit only, or receive only)
 *             or is disabled.
 * INPUT    : UI32_T lport            -- lport number
 *            UI32_T admin_status     -- status vaule:
 *                                       txOnly(1),
 *                                       rxOnly(2),
 *                                       txAndRx(3),
 *                                       disabled(4)
 * OUTPUT   : None
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 10.5.1, IEEE Std 802.1AB-2005.
 *            Default value: txAndRx(3).
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_PMGR_SetPortConfigAdminStatus(UI32_T lport, UI32_T admin_status) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_SetPortAdminDisable
 *-------------------------------------------------------------------------
 * PURPOSE  :  port is admin shutdown
 * INPUT    : UI32_T lport            -- lport number
 *            UI32_T admin_status     -- status vaule:
 *                                       txOnly(1),
 *                                       rxOnly(2),
 *                                       txAndRx(3),
 *                                       disabled(4)
 * OUTPUT   : None
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 10.5.1, IEEE Std 802.1AB-2005.
 *            Default value: txAndRx(3).
 *-------------------------------------------------------------------------
 */
void LLDP_PMGR_SetPortAdminDisable(UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_SetPortConfigNotificationEnable
 *-------------------------------------------------------------------------
 * PURPOSE  : The value true(1) means that notifications are enabled;
 *            The value false(2) means that they are not.
 * INPUT    : UI32_T lport            -- lport number
 *            UI32_T status           -- status vaule:
 *                                       true(1),
 *                                       false(2),
 * OUTPUT   : None
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 12.2, IEEE Std 802.1AB-2005.
 *            Default value: false(2).
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_PMGR_SetPortConfigNotificationEnable(UI32_T lport, BOOL_T status) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_SetPortOptionalTlvStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : Set optional TLVs status that may be included in LLDPDU.
 * INPUT    : UI32_T lport            -- lport number
 *            UI8_T tlv_status        -- bitmap:
 *                                       BIT_0: Port Description TLV,
 *                                       BIT_1: System Name TLV,
 *                                       BIT_2: System Description TLV,
 *                                       BIT_3: System Capabilities TLV,
 * OUTPUT   : None
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 10.2.1.1, IEEE Std 802.1AB-2005.
 *            Default value: no bit on (empty set).
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_PMGR_SetPortOptionalTlvStatus(UI32_T port, UI8_T tlv_status) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_SetConfigManAddrTlv
 *-------------------------------------------------------------------------
 * PURPOSE  : Set optional TLVs status that may be included in LLDPDU.
 * INPUT    : lport, transfer_enable
 * OUTPUT   : None
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_PMGR_SetConfigManAddrTlv(UI32_T lport, BOOL_T transfer_enable) ;
/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_SetConfigAllPortManAddrTlv
 *-------------------------------------------------------------------------
 * PURPOSE  : Set optional TLVs status that may be included in LLDPDU.
 * INPUT    : lport, transfer_enable
 * OUTPUT   : None
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_PMGR_SetConfigAllPortManAddrTlv(UI8_T *lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetPortConfigEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get configuration entry info. for specified port.
 * INPUT    : LLDP_MGR_PortConfigEntry_T  *port_config_entry
 * OUTPUT   : LLDP_MGR_PortConfigEntry_T  *port_config_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 12.2, IEEE Std 802.1AB-2005.
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetPortConfigEntry(LLDP_MGR_PortConfigEntry_T *port_config_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetNextPortConfigEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This funtion returns true if the next configuration entry info.
 *            can be successfully retrieved. Otherwise, false.
 * INPUT    : LLDP_MGR_PortConfigEntry_T  *port_config_entry
 * OUTPUT   : LLDP_MGR_PortConfigEntry_T  *port_config_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 12.2, IEEE Std 802.1AB-2005.
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetNextPortConfigEntry(LLDP_MGR_PortConfigEntry_T *port_config_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetConfigManAddrTlv
 *-------------------------------------------------------------------------
 * PURPOSE  : This funtion returns true if the config man addr entry info.
 *            can be successfully retrieved. Otherwise, false.
 * INPUT    : None
 * OUTPUT   : enable
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 12.2, IEEE Std 802.1AB-2005.
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetConfigManAddrTlv(UI32_T lport, UI8_T *enabled) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetSysStatisticsEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get statistics info.
 * INPUT    : LLDP_MGR_Statistics_T *statistics_entry
 * OUTPUT   : LLDP_MGR_Statistics_T *statistics_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 12.2, IEEE Std 802.1AB-2005.
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetSysStatisticsEntry(LLDP_MGR_Statistics_T *statistics_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetPortTxStatisticsEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get tx statistics info. for specified port.
 * INPUT    : LLDP_MGR_PortTxStatistics_T *port_statistics_entry
 * OUTPUT   : LLDP_MGR_PortTxStatistics_T *port_statistics_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 10.5.2.1, IEEE Std 802.1AB-2005.
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetPortTxStatisticsEntry(LLDP_MGR_PortTxStatistics_T *port_statistics_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetNextPortTxStatisticsEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This funtion returns true if the next statistics info.
 *            can be successfully retrieved. Otherwise, false.
 * INPUT    : LLDP_MGR_PortStatistics_T *port_statistics_entry
 * OUTPUT   : LLDP_MGR_PortStatistics_T *port_statistics_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 10.5.2.1, IEEE Std 802.1AB-2005.
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetNextPortTxStatisticsEntry(LLDP_MGR_PortTxStatistics_T *port_statistics_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetPortRxStatisticsEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get rx statistics info. for specified port.
 * INPUT    : LLDP_MGR_PortRxStatistics_T *port_statistics_entry
 * OUTPUT   : LLDP_MGR_PortRxStatistics_T *port_statistics_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 10.5.2.2, IEEE Std 802.1AB-2005.
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetPortRxStatisticsEntry(LLDP_MGR_PortRxStatistics_T *port_statistics_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetNextPortRxStatisticsEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This funtion returns true if the next statistics info.
 *            can be successfully retrieved. Otherwise, false.
 * INPUT    : LLDP_MGR_PortRxStatistics_T *port_statistics_entry
 * OUTPUT   : LLDP_MGR_PortRxStatistics_T *port_statistics_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 10.5.2.2, IEEE Std 802.1AB-2005.
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetNextPortRxStatisticsEntry(LLDP_MGR_PortRxStatistics_T *port_statistics_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetLocalSystemData
 *-------------------------------------------------------------------------
 * PURPOSE  : Get related TLV info. in the local system.
 * INPUT    : LLDP_MGR_LocalSystemData_T *system_entry
 * OUTPUT   : LLDP_MGR_LocalSystemData_T *system_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 9.5.2/9.5.6/9.5.7/9.5.8, IEEE Std 802.1AB-2005.
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetLocalSystemData(LLDP_MGR_LocalSystemData_T *system_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetLocalPortData
 *-------------------------------------------------------------------------
 * PURPOSE  : Get statistics info. in the local system for specified port.
 * INPUT    : LLDP_MGR_LocalPortData_T *port_entry
 * OUTPUT   : LLDP_MGR_LocalPortData_T *port_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 9.5.3/9.5.5, IEEE Std 802.1AB-2005.
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetLocalPortData(LLDP_MGR_LocalPortData_T *port_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetNextLocalPortData
 *-------------------------------------------------------------------------
 * PURPOSE  : This funtion returns true if the next statistics info.
 *            can be successfully retrieved. Otherwise, false.
 * INPUT    : LLDP_MGR_LocalPortData_T *port_entry
 * OUTPUT   : LLDP_MGR_LocalPortData_T *port_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 9.5.3/9.5.5, IEEE Std 802.1AB-2005.
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetNextLocalPortData(LLDP_MGR_LocalPortData_T *port_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetLocalManagementAddressTlvEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get management address TLV info. in the local system.
 * INPUT    : None
 * OUTPUT   : LLDP_MGR_LocalManagementAddrEntry_T *man_addr_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 9.5.9, IEEE Std 802.1AB-2005.
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetLocalManagementAddressTlvEntry(LLDP_MGR_LocalManagementAddrEntry_T *man_addr_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetLocalManagementAddress
 *-------------------------------------------------------------------------
 * PURPOSE  : Get management address TLV info. in the local system.
 * INPUT    : None
 * OUTPUT   : LLDP_MGR_LocalManagementAddrEntry_T *man_addr_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 9.5.9, IEEE Std 802.1AB-2005.
 *            This function is for WE
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetLocalManagementAddress(LLDP_MGR_LocalManagementAddrEntry_T *man_addr_entry) ;
/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetNextLocalManagementAddress
 *-------------------------------------------------------------------------
 * PURPOSE  : Get management address TLV info. in the local system.
 * INPUT    : None
 * OUTPUT   : LLDP_MGR_LocalManagementAddrEntry_T *man_addr_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 9.5.9, IEEE Std 802.1AB-2005.
 *            This function is for WE
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetNextLocalManagementAddress(LLDP_MGR_LocalManagementAddrEntry_T *man_addr_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetNextLocalManagementAddressTlvEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get management address TLV info. in the local system.
 * INPUT    : LLDP_MGR_LocalManagementAddrEntry_T *man_addr_entry
 * OUTPUT   : LLDP_MGR_LocalManagementAddrEntry_T *man_addr_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 9.5.9, IEEE Std 802.1AB-2005.
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetNextLocalManagementAddressTlvEntry(LLDP_MGR_LocalManagementAddrEntry_T *man_addr_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetNextRemoteSystemData
 *-------------------------------------------------------------------------
 * PURPOSE  : Get related TLV info. in the remote system.
 * INPUT    : LLDP_MGR_RemoteSystemData_T *system_entry
 * OUTPUT   : LLDP_MGR_RemoteSystemData_T *system_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 9.5.2.2/9.5.2.3/9.5.3.3/9.5.5.2/
 *            9.5.6.2/9.5.7.2/9.5.8.1/9.5.8.2, IEEE Std 802.1AB-2005.
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetNextRemoteSystemData(LLDP_MGR_RemoteSystemData_T *system_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetNextRemoteSystemDataByPort
 *-------------------------------------------------------------------------
 * PURPOSE  : Get related TLV info. in the remote system.
 * INPUT    : LLDP_MGR_RemoteSystemData_T *system_entry
 * OUTPUT   : LLDP_MGR_RemoteSystemData_T *system_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : If system_entry->index = 0, the return system entry will be
 *            the 1st data of the port
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetNextRemoteSystemDataByPort(LLDP_MGR_RemoteSystemData_T *system_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetNextRemoteSystemDataByIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : Get related TLV info. in the remote system.
 * INPUT    : LLDP_MGR_RemoteSystemData_T *system_entry
 * OUTPUT   : LLDP_MGR_RemoteSystemData_T *system_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetNextRemoteSystemDataByIndex(LLDP_MGR_RemoteSystemData_T *system_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetRemoteManagementAddressTlvEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get management address TLV info. in the remote system.
 * INPUT    : LLDP_MGR_RemoteManagementAddrEntry_T *man_addr_entry
 * OUTPUT   : LLDP_MGR_RemoteManagementAddrEntry_T *man_addr_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 9.5.9, IEEE Std 802.1AB-2005.
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetRemoteManagementAddressTlvEntry(LLDP_MGR_RemoteManagementAddrEntry_T *man_addr_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetNextRemoteManagementAddressTlvEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get management address TLV info. in the remote system.
 * INPUT    : LLDP_MGR_RemoteManagementAddrEntry_T *man_addr_entry
 * OUTPUT   : LLDP_MGR_RemoteManagementAddrEntry_T *man_addr_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 9.5.9, IEEE Std 802.1AB-2005.
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetNextRemoteManagementAddressTlvEntry(LLDP_MGR_RemoteManagementAddrEntry_T *man_addr_entry) ;

#if (LLDP_TYPE_EXT_802DOT1 == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetXdot1ConfigEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.1 extension configuraiton
 * INPUT    : xdot1_config_entry
 * OUTPUT   : xdot1_config_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetXdot1ConfigEntry(LLDP_MGR_Xdot1ConfigEntry_T *xdot1_config_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetNextXdot1ConfigEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.1 extension configuraiton
 * INPUT    : xdot1_config_entry
 * OUTPUT   : xdot1_config_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetNextXdot1ConfigEntry(LLDP_MGR_Xdot1ConfigEntry_T *xdot1_config_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_SetXdot1ConfigEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Set 802.1 extension configuraiton
 * INPUT    : xdot1_config_entry
 * OUTPUT   : None
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_SetXdot1ConfigEntry(LLDP_MGR_Xdot1ConfigEntry_T *xdot1_config_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_SetXdot1ConfigPortVlanTxEnable
 *-------------------------------------------------------------------------
 * PURPOSE  : Set 802.1 extension configuraiton
 * INPUT    : xdot1_config_entry
 * OUTPUT   : None
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_PMGR_SetXdot1ConfigPortVlanTxEnable(UI32_T lport, UI32_T port_vlan_tx_enable) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_SetXdot1ConfigProtoVlanTxEnable
 *-------------------------------------------------------------------------
 * PURPOSE  : Set 802.1 extension configuraiton
 * INPUT    : xdot1_config_entry
 * OUTPUT   : None
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_PMGR_SetXdot1ConfigProtoVlanTxEnable(UI32_T lport, UI32_T proto_vlan_tx_enable) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_SetXdot1ConfigVlanNameTxEnable
 *-------------------------------------------------------------------------
 * PURPOSE  : Set 802.1 extension configuraiton
 * INPUT    : xdot1_config_entry
 * OUTPUT   : None
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_PMGR_SetXdot1ConfigVlanNameTxEnable(UI32_T lport, UI32_T vlan_name_tx_enable) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_SetXdot1ConfigProtocolTxEnable
 *-------------------------------------------------------------------------
 * PURPOSE  : Set 802.1 extension configuraiton
 * INPUT    : xdot1_config_entry
 * OUTPUT   : None
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_PMGR_SetXdot1ConfigProtocolTxEnable(UI32_T lport, UI32_T protocol_tx_enable) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetXdot1LocEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.1 extension local entry
 * INPUT    : xdot1_loc_entry
 * OUTPUT   : xdot1_loc_entry
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetXdot1LocEntry(LLDP_MGR_Xdot1LocEntry_T *xdot1_loc_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetNextXdot1LocEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.1 extension local entry
 * INPUT    : xdot1_loc_entry
 * OUTPUT   : xdot1_loc_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetNextXdot1LocEntry(LLDP_MGR_Xdot1LocEntry_T *xdot1_loc_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetXdot1LocProtoVlanEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.1 extension local protocol vlan entry
 * INPUT    : xdot1_loc_proto_vlan_entry
 * OUTPUT   : xdot1_loc_proto_vlan_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetXdot1LocProtoVlanEntry(LLDP_MGR_Xdot1LocProtoVlanEntry_T *xdot1_loc_proto_vlan_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetNextXdot1LocProtoVlanEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.1 extension local protocol vlan entry
 * INPUT    : xdot1_loc_proto_vlan_entry
 * OUTPUT   : xdot1_loc_proto_vlan_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetNextXdot1LocProtoVlanEntry(LLDP_MGR_Xdot1LocProtoVlanEntry_T *xdot1_loc_proto_vlan_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetXdot1LocVlanNameEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.1 extension local vlan name entry
 * INPUT    : xdot1_loc_vlan_name_entry
 * OUTPUT   : xdot1_loc_vlan_name_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetXdot1LocVlanNameEntry(LLDP_MGR_Xdot1LocVlanNameEntry_T *xdot1_loc_vlan_name_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetNextXdot1LocVlanNameEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.1 extension local vlan name entry
 * INPUT    : xdot1_loc_vlan_name_entry
 * OUTPUT   : xdot1_loc_vlan_name_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetNextXdot1LocVlanNameEntry(LLDP_MGR_Xdot1LocVlanNameEntry_T *xdot1_loc_vlan_name_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetXdot1LocProtocolEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.1 extension protocol entry
 * INPUT    : xdot1_loc_protocol_entry
 * OUTPUT   : xdot1_loc_protocol_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetXdot1LocProtocolEntry(LLDP_MGR_Xdot1LocProtocolEntry_T *xdot1_loc_protocol_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetNextXdot1LocProtocolEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.1 extension protocol entry
 * INPUT    : xdot1_loc_protocol_entry
 * OUTPUT   : xdot1_loc_protocol_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetNextXdot1LocProtocolEntry(LLDP_MGR_Xdot1LocProtocolEntry_T *xdot1_loc_protocol_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetNextXdot1RemEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.1 extension remote entry
 * INPUT    : xdot1_rem_entry
 * OUTPUT   : xdot1_rem_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetNextXdot1RemEntry(LLDP_MGR_Xdot1RemEntry_T *xdot1_rem_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextXdot1RemEntryByIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.1 extension remote entry
 * INPUT    : xdot1_rem_entry
 * OUTPUT   : xdot1_rem_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : for WEB/CLI
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetNextXdot1RemEntryByIndex(LLDP_MGR_Xdot1RemEntry_T *xdot1_rem_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetNextXdot1RemProtoVlanEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.1 extension remote protocol vlan entry
 * INPUT    : xdot1_rem_proto_vlan_entry
 * OUTPUT   : xdot1_rem_proto_vlan_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetNextXdot1RemProtoVlanEntry(LLDP_MGR_Xdot1RemProtoVlanEntry_T *xdot1_rem_proto_vlan_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetXdot1RemProtoVlanEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.1 extension remote vlan name entry
 * INPUT    : xdot1_rem_vlan_name_entry
 * OUTPUT   : xdot1_rem_vlan_name_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetNextXdot1RemVlanNameEntry(LLDP_MGR_Xdot1RemVlanNameEntry_T *xdot1_rem_vlan_name_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetNextXdot1RemProtocolEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.1 extension remote protocol entry
 * INPUT    : xdot1_rem_protocol_entry
 * OUTPUT   : xdot1_rem_protocol_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetNextXdot1RemProtocolEntry(LLDP_MGR_Xdot1RemProtocolEntry_T *xdot1_rem_protocol_entry) ;
#endif /* #if (LLDP_TYPE_EXT_802DOT1 == TRUE) */

#if (LLDP_TYPE_EXT_802DOT3 == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetXdot3PortConfigEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.3 extension port config entry
 * INPUT    : xdot3_port_config_entry
 * OUTPUT   : xdot3_port_config_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetXdot3PortConfigEntry(LLDP_MGR_Xdot3PortConfigEntry_T *xdot3_port_config_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetNextXdot3PortConfigEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.3 extension port config entry
 * INPUT    : xdot3_port_config_entry
 * OUTPUT   : xdot3_port_config_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetNextXdot3PortConfigEntry(LLDP_MGR_Xdot3PortConfigEntry_T *xdot3_port_config_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_SetXdot3PortConfigEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Set 802.3 extension port config entry
 * INPUT    : xdot3_port_config_entry
 * OUTPUT   : None
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : for snmp
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_SetXdot3PortConfigEntry(UI32_T lport, UI8_T status) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_SetXdot3PortConfig
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.3 extension port config entry
 * INPUT    : xdot3_port_config_entry
 * OUTPUT   : xdot3_port_config_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : for WEB/CLI
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_SetXdot3PortConfig(LLDP_MGR_Xdot3PortConfigEntry_T *xdot3_port_config_entry_p) ;

UI32_T  LLDP_PMGR_GetXdot3LocPortEntry(LLDP_MGR_Xdot3LocPortEntry_T *xdot3_port_local_entry);

UI32_T  LLDP_PMGR_GetNextXdot3LocPortEntry(LLDP_MGR_Xdot3LocPortEntry_T *xdot3_port_local_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetXdot3LocPowerEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.3 extension local power entry
 * INPUT    : xdot3_loc_power_entry
 * OUTPUT   : xdot3_loc_power_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetXdot3LocPowerEntry(LLDP_MGR_Xdot3LocPowerEntry_T *xdot3_loc_power_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetNextXdot3LocPowerEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.3 extension local power entry
 * INPUT    : xdot3_loc_power_entry
 * OUTPUT   : xdot3_loc_power_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetNextXdot3LocPowerEntry(LLDP_MGR_Xdot3LocPowerEntry_T *xdot3_loc_power_entry);

UI32_T  LLDP_PMGR_GetXdot3LocLinkAggEntry(LLDP_MGR_Xdot3LocLinkAggEntry_T *xdot3_loc_link_agg_entry);

UI32_T  LLDP_PMGR_GetNextXdot3LocLinkAggEntry(LLDP_MGR_Xdot3LocLinkAggEntry_T *xdot3_loc_link_agg_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetXdot3LocMaxFrameSizeEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.3 extension local maximum frame size entry
 * INPUT    : xdot3_loc_max_frame_size_entry
 * OUTPUT   : xdot3_loc_max_frame_size_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetXdot3LocMaxFrameSizeEntry(LLDP_MGR_Xdot3LocMaxFrameSizeEntry_T *xdot3_loc_max_frame_size_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetNextXdot3LocMaxFrameSizeEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.3 extension local maximum frame size entry
 * INPUT    : xdot3_loc_max_frame_size_entry
 * OUTPUT   : xdot3_loc_max_frame_size_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetNextXdot3LocMaxFrameSizeEntry(LLDP_MGR_Xdot3LocMaxFrameSizeEntry_T *xdot3_loc_max_frame_size_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetXdot3RemPortEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.3 extension remote port entry
 * INPUT    : xdot3_rem_port_entry
 * OUTPUT   : xdot3_rem_port_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : for snmp
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetXdot3RemPortEntry(LLDP_MGR_Xdot3RemPortEntry_T *xdot3_rem_port_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetNextXdot3RemPortEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.3 extension remote port entry
 * INPUT    : xdot3_rem_port_entry
 * OUTPUT   : xdot3_rem_port_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : for snmp
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetNextXdot3RemPortEntry(LLDP_MGR_Xdot3RemPortEntry_T *xdot3_rem_port_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetNextXdot3RemPowerEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.3 extension remote power entry
 * INPUT    : xdot3_rem_power_entry
 * OUTPUT   : xdot3_rem_power_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetNextXdot3RemPowerEntry(LLDP_MGR_Xdot3RemPowerEntry_T *xdot3_rem_power_entry);
#endif /* #if (LLDP_TYPE_EXT_802DOT3 == TRUE) */

#if (SYS_CPNT_ADD == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_IsTelephoneMac
 *-------------------------------------------------------------------------
 * PURPOSE  : To determine if the mac address is belonged to a telephone.
 * INPUT    : lport, device_mac, device_mac_len
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_PMGR_IsTelephoneMac(UI32_T lport, UI8_T  *device_mac, UI32_T device_mac_len);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_IsTelephoneNetworkAddr
 *-------------------------------------------------------------------------
 * PURPOSE  : To determine if the netowrk address is belonged to a telephone.
 * INPUT    : lport, device_network_addr_subtype, device_network_addr, device_network_addr_len
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_PMGR_IsTelephoneNetworkAddr(UI32_T lport, UI8_T device_network_addr_subtype, UI8_T  *device_network_addr, UI32_T device_network_addr_len) ;
#endif /* #if (SYS_CPNT_ADD == TRUE) */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_NotifySysNameChanged
 *-------------------------------------------------------------------------
 * PURPOSE  : When there is something changed, this function will be called.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void LLDP_PMGR_NotifySysNameChanged(void) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_NotifyRifChanged
 *-------------------------------------------------------------------------
 * PURPOSE  : When there is something changed, this function will be called.
 * INPUT    : vid_ifindex -- the specified vlan;
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void LLDP_PMGR_NotifyRifChanged(UI32_T vid_ifindex) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_NotifyRoutingChanged
 *-------------------------------------------------------------------------
 * PURPOSE  : When there is something changed, this function will be called.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void LLDP_PMGR_NotifyRoutingChanged(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_NotifyPseTableChanged
 *-------------------------------------------------------------------------
 * PURPOSE  : When there is something changed, this function will be called.
 * INPUT    : lport -- the specified port
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : lport can be 0, which means changes for all lports
 *-------------------------------------------------------------------------
 */
void LLDP_PMGR_NotifyPseTableChanged(UI32_T lport) ;
/*added by Jinhua Wei ,to remove warning ,becaued the relative function's head file not included */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetXdot3RemLinkAggEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.3 extension remote link aggregation entry
 * INPUT    : xdot3_rem_link_agg_entry
 * OUTPUT   : xdot3_rem_link_agg_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetXdot3RemLinkAggEntry(LLDP_MGR_Xdot3RemLinkAggEntry_T *xdot3_rem_link_agg_entry);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetNextXdot3RemLinkAggEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.3 extension remote link aggregation entry
 * INPUT    : xdot3_rem_link_agg_entry
 * OUTPUT   : xdot3_rem_link_agg_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetNextXdot3RemLinkAggEntry(LLDP_MGR_Xdot3RemLinkAggEntry_T *xdot3_rem_link_agg_entry);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetXdot3RemMaxFrameSizeEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.3 extension remote maximum frame size entry
 * INPUT    : xdot3_rem_max_frame_size_entry
 * OUTPUT   : xdot3_rem_max_frame_size_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetXdot3RemMaxFrameSizeEntry(LLDP_MGR_Xdot3RemMaxFrameSizeEntry_T *xdot3_rem_max_frame_size_entry);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetNextXdot3RemMaxFrameSizeEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.3 extension remote maximum frame size entry
 * INPUT    : xdot3_rem_max_frame_size_entry
 * OUTPUT   : xdot3_rem_max_frame_size_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetNextXdot3RemMaxFrameSizeEntry(LLDP_MGR_Xdot3RemMaxFrameSizeEntry_T *xdot3_rem_max_frame_size_entry);

#if (LLDP_TYPE_MED == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_SetXMedFastStartRepeatCount
 *-------------------------------------------------------------------------
 * PURPOSE  : Set LLDP-MED fast start repeat count
 * INPUT    : repeat_count
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T LLDP_PMGR_SetXMedFastStartRepeatCount(UI32_T  repeat_count) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetXMedPortConfigEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED port configuration entry
 * INPUT    : None
 * OUTPUT   : port_config_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_PMGR_GetXMedPortConfigEntry(LLDP_MGR_XMedPortConfigEntry_T *port_config_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetNextXMedPortConfigEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED port configuration entry
 * INPUT    : None
 * OUTPUT   : port_config_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_PMGR_GetNextXMedPortConfigEntry(LLDP_MGR_XMedPortConfigEntry_T *port_config_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_SetXMedPortConfigTlvsTx
 *-------------------------------------------------------------------------
 * PURPOSE  : Set LLDP-MED port transfer tlvs
 * INPUT    : lport, tlvs_tx_enabled
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_PMGR_SetXMedPortConfigTlvsTx(UI32_T lport, UI16_T tlvs_tx_enabled) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetRunningXMedPortConfigTlvsTx
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED port configuration entry
 * INPUT    : lport
 * OUTPUT   : tlvs_tx_enabled
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     : CLI
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetRunningXMedPortConfigTlvsTx(UI32_T lport, UI16_T *tlvs_tx_enabled);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_SetXMedPortConfigNotifEnabled
 *-------------------------------------------------------------------------
 * PURPOSE  : Set LLDP-MED port notification
 * INPUT    : notif_enabled
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_PMGR_SetXMedPortConfigNotifEnabled(UI32_T lport, UI8_T notif_enabled) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetRunningXMedPortNotification
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED port configuration entry
 * INPUT    : lport
 * OUTPUT   : enabled
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     : CLI
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetRunningXMedPortNotification(UI32_T lport, BOOL_T *enabled);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetXMedLocMediaPolicyEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED media policy entry
 * INPUT    : None
 * OUTPUT   : loc_med_policy_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_PMGR_GetXMedLocMediaPolicyEntry(LLDP_MGR_XMedLocMediaPolicyEntry_T *loc_med_policy_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetNextXMedLocMediaPolicyEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED media policy entry
 * INPUT    : None
 * OUTPUT   : loc_med_policy_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_PMGR_GetNextXMedLocMediaPolicyEntry(LLDP_MGR_XMedLocMediaPolicyEntry_T *loc_med_policy_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetXMedLocLocationEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED local location entry
 * INPUT    : None
 * OUTPUT   : loc_location_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : This API is only used by SNMP
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_PMGR_GetXMedLocLocationEntry(LLDP_MGR_XMedLocLocationEntry_T *loc_location_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetNextXMedLocLocationEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED local location entry
 * INPUT    : None
 * OUTPUT   : loc_location_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : This API is only used by SNMP
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_PMGR_GetNextXMedLocLocationEntry(LLDP_MGR_XMedLocLocationEntry_T *loc_location_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_SetXMedLocLocationEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Set LLDP-MED local location entry
 * INPUT    : loc_location_entry
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : This API is only used by SNMP.
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_PMGR_SetXMedLocLocationEntry(LLDP_MGR_XMedLocLocationEntry_T *loc_location_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetXMedLocLocationStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED local location status
 * INPUT    : lport
 *            location_type
 * OUTPUT   : status_p
 * RETURN   : TRUE/FALSE
 * NOTE     : This API is used for CLI/WEB.
 *-------------------------------------------------------------------------
 */
BOOL_T LLDP_PMGR_GetXMedLocLocationStatus(UI32_T lport, UI32_T location_type, BOOL_T *status_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_SetXMedLocLocationStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : Set LLDP-MED local location status
 * INPUT    : lport
 *            location_type
 *            status
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : This API is used for CLI/WEB.
 *-------------------------------------------------------------------------
 */
BOOL_T LLDP_PMGR_SetXMedLocLocationStatus(UI32_T lport, UI32_T location_type, BOOL_T status);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetRunningXMedLocLocationStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED local location status
 * INPUT    : lport
 *            location_type
 * OUTPUT   : status_p
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_PMGR_GetRunningXMedLocLocationStatus(UI32_T lport, UI32_T location_type, BOOL_T *status_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetXMedLocLocationCivicAddrCoutryCode
 *-------------------------------------------------------------------------
 * PURPOSE  : Get civic addr country code.
 * INPUT    : lport
 * OUTPUT   : country_code.
 * RETURN   : TRUE/FALSE
 * NOTE     : This API is used for CLI/WEB.
 *-------------------------------------------------------------------------
 */
BOOL_T LLDP_PMGR_GetXMedLocLocationCivicAddrCoutryCode(UI32_T lport, UI8_T *country_code) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_SetXMedLocLocationCivicAddrCoutryCode
 *-------------------------------------------------------------------------
 * PURPOSE  : Set civic addr country code.
 * INPUT    : lport, country_code
 * OUTPUT   : None.
 * RETURN   : TRUE/FALSE
 * NOTE     : This API is used for CLI/WEB.
 *-------------------------------------------------------------------------
 */
BOOL_T LLDP_PMGR_SetXMedLocLocationCivicAddrCoutryCode(UI32_T lport, UI8_T *country_code) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetRunningXMedLocLocationCivicAddrCoutryCode
 *-------------------------------------------------------------------------
 * PURPOSE  : Get civic addr country code.
 * INPUT    : lport
 * OUTPUT   : country_code.
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     : This API is used for CLI/WEB.
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_PMGR_GetRunningXMedLocLocationCivicAddrCoutryCode(UI32_T lport, UI8_T *country_code);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetXMedLocLocationCivicAddrWhat
 *-------------------------------------------------------------------------
 * PURPOSE  : Get civic addr what value.
 * INPUT    : lport, what
 * OUTPUT   : None.
 * RETURN   : TRUE/FALSE
 * NOTE     : This API is used for CLI/WEB.
 *-------------------------------------------------------------------------
 */
BOOL_T LLDP_PMGR_GetXMedLocLocationCivicAddrWhat(UI32_T lport, UI8_T *what) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_SetXMedLocLocationCivicAddrWhat
 *-------------------------------------------------------------------------
 * PURPOSE  : Set civic addr what value.
 * INPUT    : lport, what
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : This API is used for CLI/WEB.
 *-------------------------------------------------------------------------
 */
BOOL_T LLDP_PMGR_SetXMedLocLocationCivicAddrWhat(UI32_T lport, UI8_T what) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetRunningXMedLocLocationCivicAddrWhat
 *-------------------------------------------------------------------------
 * PURPOSE  : Get civic addr what value.
 * INPUT    : lport
 * OUTPUT   : what
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     : This API is used for CLI/WEB.
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_PMGR_GetRunningXMedLocLocationCivicAddrWhat(UI32_T lport, UI8_T *what);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_Get1stXMedLocLocationCivicAddrCaEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get civic addr country code.
 * INPUT    : lport
 * OUTPUT   : ca_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : This API is used for CLI/WEB.
 *-------------------------------------------------------------------------
 */
BOOL_T LLDP_PMGR_Get1stXMedLocLocationCivicAddrCaEntry(UI32_T lport, LLDP_MGR_XMedLocationCivicAddrCaEntry_T *ca_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetNextXMedLocLocationCivicAddrCaEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get civic addr country code.
 * INPUT    : lport
 * OUTPUT   : ca_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : This API is used for CLI/WEB.
 *-------------------------------------------------------------------------
 */
BOOL_T LLDP_PMGR_GetNextXMedLocLocationCivicAddrCaEntry(UI32_T lport, LLDP_MGR_XMedLocationCivicAddrCaEntry_T *ca_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_SetXMedLocLocationCivicAddrCaEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Set civic addr tlv.
 * INPUT    : lport, ca_tlv, set_or_unset
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : This API is used for CLI/WEB.
 *-------------------------------------------------------------------------
 */
BOOL_T LLDP_PMGR_SetXMedLocLocationCivicAddrCaEntry(UI32_T lport, LLDP_MGR_XMedLocationCivicAddrCaEntry_T *ca_entry, BOOL_T set_or_unset) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetXMedLocXPoePsePortEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED local port poe/pse entry
 * INPUT    : None
 * OUTPUT   : loc_poe_pse_port_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_PMGR_GetXMedLocXPoePsePortEntry(LLDP_MGR_XMedLocXPoePsePortEntry_T *loc_poe_pse_port_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetNextXMedLocXPoePsePortEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED local port poe/pse entry
 * INPUT    : None
 * OUTPUT   : loc_poe_pse_port_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_PMGR_GetNextXMedLocXPoePsePortEntry(LLDP_MGR_XMedLocXPoePsePortEntry_T *loc_poe_pse_port_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetXMedLocXPoeEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED local poe entry
 * INPUT    : None
 * OUTPUT   : loc_poe_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_PMGR_GetXMedLocXPoeEntry(LLDP_MGR_XMedLocXPoeEntry_T *loc_poe_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetXMedLocInventoryEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED local inventory entry
 * INPUT    : None
 * OUTPUT   : loc_inventory_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_PMGR_GetXMedLocInventoryEntry(LLDP_MGR_XMedLocInventory_T *loc_inventory_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetNextXMedRemCapEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED remote device capability entry
 * INPUT    : None
 * OUTPUT   : rem_cap_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_PMGR_GetNextXMedRemCapEntry(LLDP_MGR_XMedRemCapEntry_T *rem_cap_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetNextXMedRemMediaPolicyEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED remote device media policy entry
 * INPUT    : None
 * OUTPUT   : rem_med_policy_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_PMGR_GetNextXMedRemMediaPolicyEntry(LLDP_MGR_XMedRemMediaPolicyEntry_T *rem_med_policy_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetXMedRemMediaPolicyEntryByIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED remote device media policy entry by lport, index and app_type
 *            (i.e. without timemark)
 * INPUT    : None
 * OUTPUT   : rem_med_policy_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : This API is used only for WEB/CLI.
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_PMGR_GetXMedRemMediaPolicyEntryByIndex(LLDP_MGR_XMedRemMediaPolicyEntry_T *rem_med_policy_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetNextXMedRemInventoryEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED remote device inventory entry
 * INPUT    : None
 * OUTPUT   : rem_inventory_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_PMGR_GetNextXMedRemInventoryEntry(LLDP_MGR_XMedRemInventoryEntry_T *rem_inventory_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetNextXMedRemLocationEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED remote device location entry
 * INPUT    : None
 * OUTPUT   : rem_location_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_PMGR_GetNextXMedRemLocationEntry(LLDP_MGR_XMedRemLocationEntry_T *rem_location_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetNextXMedRemLocationEntryByIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED remote device location entry
 * INPUT    : None
 * OUTPUT   : rem_location_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : This API is used onlu for WEB/CLI.
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_PMGR_GetNextXMedRemLocationEntryByIndex(LLDP_MGR_XMedRemLocationEntry_T *rem_location_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetXMedRemLocationCivicAddrCountryCode
 *-------------------------------------------------------------------------
 * PURPOSE  : Set civic addr country code.
 * INPUT    : rem_loc_port_num
 *            rem_index
 * OUTPUT   : country_code
 * RETURN   : TRUE/FALSE
 * NOTE     : This API is used for CLI/WEB.
 *-------------------------------------------------------------------------
 */
BOOL_T LLDP_PMGR_GetXMedRemLocationCivicAddrCountryCode(UI32_T rem_loc_port_num,
                                                        UI32_T rem_index,
                                                        UI8_T *country_code);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetXMedRemLocationCivicAddrWhat
 *-------------------------------------------------------------------------
 * PURPOSE  : Get civic addr what value.
 * INPUT    : rem_loc_port_num
 *            rem_index
 * OUTPUT   : what
 * RETURN   : TRUE/FALSE
 * NOTE     : This API is used for CLI/WEB.
 *-------------------------------------------------------------------------
 */
BOOL_T LLDP_PMGR_GetXMedRemLocationCivicAddrWhat(UI32_T rem_loc_port_num,
                                                 UI32_T rem_index,
                                                 UI8_T *what);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_Get1stXMedRemLocationCivicAddrCaEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get first civic addr entry.
 * INPUT    : rem_loc_port_num
 *            rem_index
 * OUTPUT   : ca_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : This API is used for CLI/WEB.
 *-------------------------------------------------------------------------
 */
BOOL_T LLDP_PMGR_Get1stXMedRemLocationCivicAddrCaEntry(UI32_T rem_loc_port_num,
                                                       UI32_T rem_index,
                                                       LLDP_MGR_XMedLocationCivicAddrCaEntry_T *ca_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetNextXMedRemLocationCivicAddrCaEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get next civic addr entry.
 * INPUT    : rem_loc_port_num
 *            rem_index
 * OUTPUT   : ca_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : This API is used for CLI/WEB.
 *-------------------------------------------------------------------------
 */
BOOL_T LLDP_PMGR_GetNextXMedRemLocationCivicAddrCaEntry(UI32_T rem_loc_port_num,
                                                        UI32_T rem_index,
                                                        LLDP_MGR_XMedLocationCivicAddrCaEntry_T *ca_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetXMedRemLocationElin
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED remote device location elin entry
 * INPUT    : None
 * OUTPUT   : rem_loc_elin_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : This API are used for CLI/WEB.
 *-------------------------------------------------------------------------
 */
BOOL_T LLDP_PMGR_GetXMedRemLocationElin(UI32_T rem_loc_port_num,
                                        UI32_T rem_index,
                                        LLDP_MGR_XMedLocationElin_T *rem_loc_elin_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetNextXMedRemPoeEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED remote device poe entry
 * INPUT    : None
 * OUTPUT   : rem_poe_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_PMGR_GetNextXMedRemPoeEntry(LLDP_MGR_XMedRemPoeEntry_T *rem_poe_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetNextXMedRemPoePseEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED remote device poe pse entry
 * INPUT    : None
 * OUTPUT   : rem_pse_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_PMGR_GetNextXMedRemPoePseEntry(LLDP_MGR_XMedRemPoePseEntry_T *rem_pse_entry) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetNextXMedRemPoePdEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED remote device poe pd entry
 * INPUT    : None
 * OUTPUT   : rem_pd_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_PMGR_GetNextXMedRemPoePdEntry(LLDP_MGR_XMedRemPoePdEntry_T *rem_pd_entry) ;
#endif

#if (LLDP_TYPE_DCBX == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_SetXdcbxPortConfig
 *-------------------------------------------------------------------------
 * PURPOSE  : Set DCBX port config
 * INPUT    : lport, xdcbx_port_config_entry_p
 * OUTPUT   : None
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_SetXdcbxPortConfig(LLDP_TYPE_XdcbxPortConfigEntry_T *xdcbx_port_config_entry_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetXdcbxPortConfig
 *-------------------------------------------------------------------------
 * PURPOSE  : Get DCBX port config
 * INPUT    : lport
 * OUTPUT   : xdcbx_port_config_entry_p
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetXdcbxPortConfig(LLDP_TYPE_XdcbxPortConfigEntry_T *xdcbx_port_config_entry_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetRunningXdcbxPortConfig
 *-------------------------------------------------------------------------
 * PURPOSE  : Get DCBX port config
 * INPUT    : lport
 * OUTPUT   : xdcbx_port_config_entry_p
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetRunningXdcbxPortConfig(LLDP_TYPE_XdcbxPortConfigEntry_T *xdcbx_port_config_entry_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetDcbxEtsRemoteData
 *-------------------------------------------------------------------------
 * PURPOSE  : Get DCBX ETS remote data
 * INPUT    : lport
 * OUTPUT   : rem_ets_entry_p
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetDcbxEtsRemoteData(LLDP_TYPE_DcbxRemEtsEntry_T *rem_ets_entry_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetDcbxPfcRemoteData
 *-------------------------------------------------------------------------
 * PURPOSE  : Get DCBX PFC remote data
 * INPUT    : lport
 * OUTPUT   : rem_pfc_entry_p
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetDcbxPfcRemoteData(LLDP_TYPE_DcbxRemPfcEntry_T *rem_pfc_entry_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_GetNextDcbxAppPriorityRemoteData
 *-------------------------------------------------------------------------
 * PURPOSE  : Get next DCBX Application priority remote data
 * INPUT    : lport
 * OUTPUT   : rem_app_pri_entry_p
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_PMGR_GetNextDcbxAppPriorityRemoteData(LLDP_TYPE_DcbxRemAppPriEntry_T *rem_app_pri_entry_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_PMGR_NotifyEtsPfcCfgChanged
 *-------------------------------------------------------------------------
 * PURPOSE  : When ets/pfc cfg is changed, this function will be called.
 * INPUT    : lport       -- the specified port
 *            is_ets_chgd -- TRUE if ets is changed
 *            is_pfc_chgd -- TRUE if pfc is changed
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void LLDP_PMGR_NotifyEtsPfcCfgChanged(
    UI32_T  lport,
    BOOL_T  is_ets_chgd,
    BOOL_T  is_pfc_chgd);

#endif /* End of #if (LLDP_TYPE_DCBX == TRUE) */

#endif /* #ifndef LLDP_PMGR_H */
