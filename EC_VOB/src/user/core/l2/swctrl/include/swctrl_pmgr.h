/* FUNCTION NAME: swctrl_pmgr.h
 * PURPOSE:
 * NOTES:
 *
 *REASON:
 *    DESCRIPTION:
 *    CREATOR:   Eli_lin
 *    Date       06/22/2007
 *
 * Copyright(C)      Accton Corporation, 2005
 */
#ifndef SWCTRL_PMGR_H
#define SWCTRL_PMGR_H

#include "sys_cpnt.h"
#include "swctrl.h"
#include "stktplg_type.h"

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SWCTRL_PMGR_Init
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Do initialization procedures for SWCTRL_PMGR.
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
void SWCTRL_PMGR_Init(void);

#if (SYS_CPNT_ATC_BSTORM == TRUE)
BOOL_T SWCTRL_PMGR_SetATCBroadcastStormAutoTrafficControlOnStatus(UI32_T ifindex,UI32_T  auto_traffic_control_on_status);
BOOL_T SWCTRL_PMGR_SetATCBroadcastStormAutoTrafficControlReleaseStatus(UI32_T ifindex, UI32_T auto_traffic_control_release_status);
BOOL_T SWCTRL_PMGR_SetATCBroadcastStormTrafficControlReleaseStatus(UI32_T ifindex, UI32_T traffic_control_release_status);
BOOL_T SWCTRL_PMGR_SetATCBroadcastStormTrafficControlOnTimer(UI32_T traffic_control_on_timer);
BOOL_T SWCTRL_PMGR_GetATCBroadcastStormTrafficControlOnTimer(UI32_T *traffic_control_on_timer);
BOOL_T SWCTRL_PMGR_SetATCBroadcastStormTrafficControlReleaseTimer(UI32_T traffic_control_release_timer);
BOOL_T SWCTRL_PMGR_GetATCBroadcastStormTrafficControlReleaseTimer(UI32_T *traffic_control_release_timer);
BOOL_T SWCTRL_PMGR_SetATCBroadcastStormStormAlarmThreshold(UI32_T ifindex, UI32_T storm_alarm_threshold);
BOOL_T SWCTRL_PMGR_SetATCBroadcastStormStormClearThreshold(UI32_T ifindex, UI32_T storm_clear_threshold);
BOOL_T SWCTRL_PMGR_SetATCBroadcastStormStormAlarmThresholdEx(UI32_T ifindex, UI32_T storm_alarm_threshold, UI32_T storm_clear_threshold);
BOOL_T SWCTRL_PMGR_SetATCBroadcastStormAction(UI32_T ifindex, UI32_T action);
BOOL_T SWCTRL_PMGR_GetATCBroadcastStormEntry(SWCTRL_ATCBroadcastStormEntry_T *atc_broadcast_storm_entry);
BOOL_T SWCTRL_PMGR_GetATCBroadcastStormTimer(SWCTRL_ATCBroadcastStormTimer_T *a_broadcast_storm_timer);
SYS_TYPE_Get_Running_Cfg_T SWCTRL_PMGR_GetRunningATCBroadcastStormEntry(SWCTRL_ATCBroadcastStormEntry_T *atc_broadcast_storm_entry);
SYS_TYPE_Get_Running_Cfg_T SWCTRL_PMGR_GetRunningATCBroadcastStormTimer(SWCTRL_ATCBroadcastStormTimer_T *a_broadcast_storm_timer);

BOOL_T SWCTRL_PMGR_SetATCBroadcastStormTrapStormAlarmStatus(UI32_T ifindex, UI32_T trap_storm_alarm_status);
BOOL_T SWCTRL_PMGR_GetATCBroadcastStormTrapStormAlarmStatus(UI32_T ifindex, UI32_T *trap_storm_alarm_status);
BOOL_T SWCTRL_PMGR_SetATCBroadcastStormTrapStormClearStatus(UI32_T ifindex, UI32_T trap_storm_clear_status);
BOOL_T SWCTRL_PMGR_GetATCBroadcastStormTrapStormClearStatus(UI32_T ifindex, UI32_T *trap_storm_clear_status);
BOOL_T SWCTRL_PMGR_SetATCBroadcastStormTrapTrafficControlOnStatus(UI32_T ifindex, UI32_T trap_traffic_control_on_status);
BOOL_T SWCTRL_PMGR_GetATCBroadcastStormTrapTrafficControlOnStatus(UI32_T ifindex, UI32_T *trap_traffic_control_on_status);
BOOL_T SWCTRL_PMGR_SetATCBroadcastStormTrapTrafficControlReleaseStatus(UI32_T ifindex, UI32_T trap_traffic_control_release_status);
BOOL_T SWCTRL_PMGR_GetATCBroadcastStormTrapTrafficControlReleaseStatus(UI32_T ifindex, UI32_T *trap_traffic_control_release_status);

BOOL_T SWCTRL_PMGR_SetATCBroadcastStormSampleType(UI32_T ifindex, UI32_T atc_multicast_storm_sample_type);
BOOL_T SWCTRL_PMGR_GetNextATCBroadcastStormEntry(SWCTRL_ATCBroadcastStormEntry_T *atc_broadcast_storm_entry);

BOOL_T SWCTRL_PMGR_DisableBStormAfterClearThreshold(UI32_T ifindex);
#endif /* End of SYS_CPNT_ATC_BSTORM */

#if (SYS_CPNT_ATC_MSTORM == TRUE)
BOOL_T SWCTRL_PMGR_SetATCMulticastStormAutoTrafficControlOnStatus(UI32_T ifindex, UI32_T auto_traffic_control_on_status);
BOOL_T SWCTRL_PMGR_SetATCMulticastStormAutoTrafficControlReleaseStatus(UI32_T ifindex, UI32_T auto_traffic_control_release_status);
BOOL_T SWCTRL_PMGR_SetATCMulticastStormTrafficControlReleaseStatus(UI32_T ifindex, UI32_T traffic_control_release_status);
BOOL_T SWCTRL_PMGR_SetATCMulticastStormTrafficControlOnTimer(UI32_T traffic_control_on_timer);
BOOL_T SWCTRL_PMGR_GetATCMulticastStormTrafficControlOnTimer(UI32_T *traffic_control_on_timer);
BOOL_T SWCTRL_PMGR_SetATCMulticastStormTrafficControlReleaseTimer(UI32_T traffic_control_release_timer);
BOOL_T SWCTRL_PMGR_GetATCMulticastStormTrafficControlReleaseTimer(UI32_T *traffic_control_release_timer);
BOOL_T SWCTRL_PMGR_SetATCMulticastStormStormAlarmThreshold(UI32_T ifindex, UI32_T storm_alarm_threshold);
BOOL_T SWCTRL_PMGR_SetATCMulticastStormStormClearThreshold(UI32_T ifindex, UI32_T storm_clear_threshold);
BOOL_T SWCTRL_PMGR_SetATCMulticastStormStormAlarmThresholdEx(UI32_T ifindex, UI32_T storm_alarm_threshold, UI32_T storm_clear_threshold);
BOOL_T SWCTRL_PMGR_SetATCMulticastStormAction(UI32_T ifindex, UI32_T action);
BOOL_T SWCTRL_PMGR_GetATCMulticastStormEntry(SWCTRL_ATCMulticastStormEntry_T *atc_multicast_storm_entry);
BOOL_T SWCTRL_PMGR_GetATCMulticastStormTimer(SWCTRL_ATCMulticastStormTimer_T *a_multicast_storm_timer);
SYS_TYPE_Get_Running_Cfg_T SWCTRL_PMGR_GetRunningATCMulticastStormEntry(SWCTRL_ATCMulticastStormEntry_T *atc_multicast_storm_entry);
SYS_TYPE_Get_Running_Cfg_T SWCTRL_PMGR_GetRunningATCMulticastStormTimer(SWCTRL_ATCMulticastStormTimer_T *a_multicast_storm_timer);

BOOL_T SWCTRL_PMGR_SetATCMulticastStormTrapStormAlarmStatus(UI32_T ifindex, UI32_T trap_storm_alarm_status);
BOOL_T SWCTRL_PMGR_GetATCMulticastStormTrapStormAlarmStatus(UI32_T ifindex, UI32_T *trap_storm_alarm_status);
BOOL_T SWCTRL_PMGR_SetATCMulticastStormTrapStormClearStatus(UI32_T ifindex, UI32_T trap_storm_clear_status);
BOOL_T SWCTRL_PMGR_GetATCMulticastStormTrapStormClearStatus(UI32_T ifindex, UI32_T *trap_storm_clear_status);
BOOL_T SWCTRL_PMGR_SetATCMulticastStormTrapTrafficControlOnStatus(UI32_T ifindex, UI32_T trap_traffic_control_on_status);
BOOL_T SWCTRL_PMGR_GetATCMulticastStormTrapTrafficControlOnStatus(UI32_T ifindex, UI32_T *trap_traffic_control_on_status);
BOOL_T SWCTRL_PMGR_SetATCMulticastStormTrapTrafficControlReleaseStatus(UI32_T ifindex, UI32_T trap_traffic_control_release_status);
BOOL_T SWCTRL_PMGR_GetATCMulticastStormTrapTrafficControlReleaseStatus(UI32_T ifindex, UI32_T *trap_traffic_control_release_status);

BOOL_T SWCTRL_PMGR_SetATCMulticastStormSampleType(UI32_T ifindex, UI32_T atc_multicast_storm_sample_type);
BOOL_T SWCTRL_PMGR_GetNextATCMulticastStormEntry(SWCTRL_ATCMulticastStormEntry_T *atc_multicast_storm_entry);

BOOL_T SWCTRL_PMGR_DisableMStormAfterClearThreshold(UI32_T ifindex);

#endif /* End of SYS_CPNT_ATC_MSTORM */

UI32_T SWCTRL_PMGR_GetNextLogicalPort(UI32_T *l_port);
BOOL_T SWCTRL_PMGR_IsManagementPort(UI32_T ifindex);
UI32_T SWCTRL_PMGR_LogicalPortToUserPort(UI32_T ifindex,UI32_T *unit,UI32_T *port,UI32_T *trunk_id);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortAdminStatus
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set port administration status
 * INPUT   : ifindex        -- which port to set
 *           admin_status   -- VAL_ifAdminStatus_up/VAL_ifAdminStatus_down
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : 1. RFC2863
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortAdminStatus(UI32_T ifindex, UI32_T admin_status);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortStatus
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set port administration status
 * INPUT   : ifindex        -- which port to set
 *           status         -- TRUE to be up; FALSE to be down
 *           reason         -- indicates role of caller
 *                             bitmap of SWCTRL_PORT_STATUS_SET_BY_XXX
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : 1. when set admin up with reason = SWCTRL_PORT_STATUS_SET_BY_CFG,
 *              it works like set admin up for all.
 *           2. user config will be affected only if set with SWCTRL_PORT_STATUS_SET_BY_CFG.
 *           3. Only CMGR_SetPortStatus may call SWCTRL_SetPortStatus/SWCTRL_PMGR_SetPortStatus; 
 *              CSCs that is in the layer higher than CMGR may call CMGR_SetPortStatus
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortStatus(UI32_T ifindex, BOOL_T status, UI32_T reason);

#if (SYS_CPNT_SUPPORT_FORCED_1000BASE_T_MODE == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPort1000BaseTForceMode
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set force[master/slave] mode configuration of a port
 * INPUT   : ifindex      -- which port to set
 *           forced_mode  -- VAL_portMasterSlaveModeCfg_master / VAL_portMasterSlaveModeCfg_slave
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : 1.ES3626A
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPort1000BaseTForceMode(UI32_T ifindex, UI32_T forced_mode);
#endif

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortCfgSpeedDuplex
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set speed/duplex configuration of a port
 * INPUT   : ifindex      -- which port to set
 *           speed_duplex -- speed/duplex to set
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : 1.ES3626A
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortCfgSpeedDuplex(UI32_T ifindex, UI32_T speed_duplex);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortDefaultSpeedDuplex
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set port speed/duplex to default value
 * INPUT   : l_port      -- which port to set
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : for CLI use
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortDefaultSpeedDuplex(UI32_T l_port);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortAutoNegEnable
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set port auto-negotiation enable state
 * INPUT   : ifindex        -- which port to set
 *           autoneg_state  -- VAL_portAutonegotiation_enabled /
 *                             VAL_portAutonegotiation_disabled
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : 1. EA3626A
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortAutoNegEnable(UI32_T ifindex, UI32_T autoneg_state);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortCfgFlowCtrlEnable
 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable flow control of a port or not
 * INPUT   : ifindex -- which port to set
 *           flow_contrl_cfg    -- VAL_portFlowCtrlCfg_enabled /
 *                                 VAL_portFlowCtrlCfg_disabled /
 *                                 VAL_portFlowCtrlCfg_tx /
 *                                 VAL_portFlowCtrlCfg_rx
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : 1. ES3626A
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortCfgFlowCtrlEnable(UI32_T ifindex, UI32_T flow_control_cfg);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortAutoNegCapability
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set the capability of auto-negotiation of a
 *           port
 * INPUT   : ifindex    -- which port to get
 *           capability -- auto-negotiation capability
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : Flow control capability bit always depends on flow control mode.
 *           ie. If flow control is enabled, when enabing auto negotiation,
 *               flow control capability bit needs to be set on as well.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortAutoNegCapability(UI32_T ifindex, UI32_T capability);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortDefaultAutoNegCapability
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set the capability of auto-negotiation of a
 *           port to default value
 * INPUT   : ifindex
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : for CLI use
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortDefaultAutoNegCapability(UI32_T ifindex);

#if (SYS_CPNT_SWCTRL_FEC == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortFec
 * -------------------------------------------------------------------------
 * FUNCTION: To enable/disable FEC
 * INPUT   : ifindex
 *           fec_mode - VAL_portFecMode_XXX
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortFec(UI32_T ifindex, UI32_T fec_mode);
#endif

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortLinkChangeTrapEnable
 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable to send trap when port link state
 *           changes.
 * INPUT   : ifindex            -- which port to set
 *           link_change_trap   -- VAL_ifLinkUpDownTrapEnable_enabled/
                                   VAL_ifLinkUpDownTrapEnable_disabled
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : 1.RFC2863
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortLinkChangeTrapEnable(UI32_T ifindex, UI32_T link_change_trap);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortDot1xEnable
 * -------------------------------------------------------------------------
 * FUNCTION: This function is called by dot1x. In this way SWCTRL could know
 *           the dot1x enable/disable status of this port and then SWCTRL can
 *           make the state machine work without dot1x intervention.
 * INPUT   : ifindex            -- which port to set
 *           dot1x_port_status  -- 1) SWCTRL_DOT1X_PORT_DISABLE
 *                                 2) SWCTRL_DOT1X_PORT_ENABLE
 * OUTPUT  : None
 * RETURN  : TRUE:  Successfully.
 *           FALSE: 1) ifindex is not belong to user port.
 *                  2) This port is not present.
 *                  3) Parameter error.
 * NOTE    : For dot1x.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortDot1xEnable(UI32_T ifindex, UI32_T dot1x_port_status);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortDot1xAuthState
 * -------------------------------------------------------------------------
 * FUNCTION: This function is called by dot1x. SWCTRL use this event to drive
 *           the state machine of the oper state.
 *           the dot1x enable/disable status of this port and then SWCTRL can
 *           make the state machine work without dot1x intervention.
 * INPUT   : ifindex          -- which port to set
 *           dot1x_auth_status -- 1) SWCTRL_DOT1X_PORT_AUTHORIZED
 *                                2) SWCTRL_DOT1X_PORT_UNAUTHORIZED
 * OUTPUT  : None.
 * RETURN  : TRUE:  Successfully.
 *           FALSE: 1) ifindex is not belong to user port.
 *                  2) This port is not present.
 *                  3) Parameter error.
 * NOTE    : For dot1x.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortDot1xAuthState(UI32_T ifindex, UI32_T dot1x_auth_status);



#if (SYS_CPNT_STATIC_TRUNK_CONFIG_ALLOWED_ON_LACP_PORT == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortLacpOperEnable
 * -------------------------------------------------------------------------
 * FUNCTION: This function is used to set LACP oper status.
 * INPUT   : ifindex --- Which user port.
 *           lacp_oper_state --- VAL_lacpPortStatus_enabled/
 *                               VAL_lacpPortStatus_disabled
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : 1. For LACP only.
 *           2. User port only.
 *           3. SWCTRL use this oper status to run ifOperStatus state machine.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortLacpOperEnable(UI32_T ifindex, UI32_T lacp_oper_status);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortLacpAdminEnable
 * -------------------------------------------------------------------------
 * FUNCTION: This function is used to set LACP admin status.
 * INPUT   : ifindex --- Which user port.
 *           lacp_admin_status --- VAL_lacpPortStatus_enabled/
 *                                 VAL_lacpPortStatus_disabled
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : 1. For LACP only.
 *           2. User port only.
 *           3. SWCTRL use this admin status to do mutex check with dot1x.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortLacpAdminEnable(UI32_T ifindex, UI32_T lacp_admin_status);

#else /* #if (SYS_CPNT_STATIC_TRUNK_CONFIG_ALLOWED_ON_LACP_PORT == TRUE) */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortLacpEnable
 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable/disable the LACP function of a port
 * INPUT   : ifindex        -- which port to set
 *           lacp_state     -- VAL_lacpPortStatus_enabled/
                               VAL_lacpPortStatus_disabled
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : 1.802.3ad
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortLacpEnable(UI32_T ifindex, UI32_T lacp_state);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetLacpEnable
 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable/disable the LACP function of whole system
 * INPUT   : lacp_state     -- VAL_lacpPortStatus_enabled/
 *                             VAL_lacpPortStatus_disabled
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : 1.802.3ad
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetLacpEnable(UI32_T lacp_state);
#endif /* #if (SYS_CPNT_STATIC_TRUNK_CONFIG_ALLOWED_ON_LACP_PORT == TRUE) */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortLacpCollecting
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set the LACP state to collecting
 * INPUT   : ifindex            -- which port to set
 *           lacp_collecting    -- VAL_LacpCollecting_collecting/
 *                                 VAL_LacpCollecting_not_collecting
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : 1.802.3ad
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortLacpCollecting(UI32_T ifindex, UI32_T lacp_collecting);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortOperDormantStatus
 * -------------------------------------------------------------------------
 * FUNCTION: To set status of ifOperStatus 'dormant'
 * INPUT   : ifindex - specified port to change status
 *           level   - see SWCTRL_OperDormantLevel_T
 *           enable  - TRUE/FALSE
 *           stealth - relevant if enable is TRUE.
 *                     TRUE to avoid oper status change from upper (e.g. up)
 *                     to this level of dormant.
 *                     FALSE to allow oper status change from upper (e.g. up)
 *                     to this level of dormant.
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortOperDormantStatus(
    UI32_T ifindex,
    SWCTRL_OperDormantLevel_T level,
    BOOL_T enable,
    BOOL_T stealth);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_TriggerPortOperDormantEvent
 * -------------------------------------------------------------------------
 * FUNCTION: To set status of ifOperStatus 'dormant'
 * INPUT   : ifindex - specified port to change status
 *           level   - see SWCTRL_OperDormantLevel_T
 *           event   - see SWCTRL_OperDormantEvent_T
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_TriggerPortOperDormantEvent(
    UI32_T ifindex,
    SWCTRL_OperDormantLevel_T level,
    SWCTRL_OperDormantEvent_T event);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortSTAState
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set STA state of a port
 * INPUT   : vid     -- which VLAN to set
 *           ifindex -- which port to set
 *           state   -- spanning tree state to set
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : This function only for Allayer chip using when include 802.1x feature.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortSTAState(UI32_T vid,
                              UI32_T ifindex,
                              UI32_T state);

#if (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_MSTP)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetMstEnableStatus
 * -------------------------------------------------------------------------
 * FUNCTION: This function will is used to set the spanning tree protocal
 * INPUT   : mst_enable  -- SWCTRL_MST_DISABLE
 *                          SWCTRL_MST_ENABLE
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetMstEnableStatus(UI32_T mst_enable_status);
#endif

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SWCTRL_PMGR_SetPortSecurityStatus
 * ------------------------------------------------------------------------|
 * FUNCTION : set the port security status
 * INPUT    : ifindex : the logical port
 *            port_security_status : VAL_portSecPortStatus_enabled
 *                                   VAL_portSecPortStatus_disabled
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : Port security doesn't support 1) unknown port, 2) trunk member, and
 *                                          3) trunk port
 * ------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortSecurityStatus( UI32_T ifindex, UI32_T  port_security_status,UI32_T port_security_called_by_who /*kevin*/);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SWCTRL_PMGR_SetPortSecurityActionStatus
 * ------------------------------------------------------------------------|
 * FUNCTION : set the port security action status
 * INPUT    : ifindex : the logical port
 *            action_status: VAL_portSecAction_none(1)
 *                           VAL_portSecAction_trap(2)
 *                           VAL_portSecAction_shutdown(3)
 *                           VAL_portSecAction_trapAndShutdown(4)
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : Port security doesn't support 1) unknown port, 2) trunk member, and
 *                                          3) trunk port
 * ------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortSecurityActionStatus( UI32_T ifindex, UI32_T  action_status);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SWCTRL_PMGR_SetPortSecurityActionTrapOperStatus
 * ------------------------------------------------------------------------|
 * FUNCTION : set the port security action trap operation status
 * INPUT    : ifindex : the logical port
 *            action_trap_status : VAL_portSecActionTrap_enabled
 *                                 VAL_portSecActionTrap_disabled
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : Port security doesn't support 1) unknown port, 2) trunk member, and
 *                                          3) trunk port
 * ------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortSecurityActionTrapOperStatus( UI32_T ifindex, UI32_T  action_trap_oper_status);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SWCTRL_PMGR_AddSecurityActionTrapTimeStamp
 * ------------------------------------------------------------------------|
 * FUNCTION : increase the timer stamp to the timer of this port?
 * INPUT    : ifindex : interface index.
 *            increase_time_stamp_ticks : increase ticks of the timer stamp.
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : Port security doesn't support 1) unknown port, 2) trunk member, and
 *                                          3) trunk port
 * ------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_AddSecurityActionTrapTimeStamp( UI32_T ifindex, UI32_T increase_time_stamp_ticks);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SWCTRL_PMGR_ResetSecurityActionTrapTimeStamp
 * ------------------------------------------------------------------------|
 * FUNCTION : reset the timer stamp to the timer of this port?
 * INPUT    : ifindex : interface index.
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : Port security doesn't support 1) unknown port, 2) trunk member, and
 *                                          3) trunk port
 * ------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_ResetSecurityActionTrapTimeStamp( UI32_T ifindex);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortPVID
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set default VLAN ID of a port
 * INPUT   : ifindex -- which port to set
 *           pvid    -- permanent VID to set
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortPVID(UI32_T ifindex, UI32_T pvid);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_CreateVlan
 * -------------------------------------------------------------------------
 * FUNCTION: This function will create a specified VLAN
 * INPUT   : vid -- which VLAN to create
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not availabl
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_CreateVlan(UI32_T vid);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_DestroyVlan
 * -------------------------------------------------------------------------
 * FUNCTION: This function will delete a specified VLAN
 * INPUT   : vid -- which VLAN to delete
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not availabl
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_DestroyVlan(UI32_T vid);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetGlobalDefaultVlan
 * -------------------------------------------------------------------------
 * FUNCTION: This function changes the global default VLAN
 * INPUT   : vid                -- the vid of the new default VLAN
 * OUTPUT  : None
 * RETURN  : TRUE               -- Success
 *           FALSE              -- If the specified VLAN is not available.
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetGlobalDefaultVlan(UI32_T vid);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_AddPortToVlanMemberSet
 * -------------------------------------------------------------------------
 * FUNCTION: This function will add a port to the member set of a specified
 *           VLAN
 * INPUT   : ifindex -- which port to add
 *           vid     -- which VLAN ID
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_AddPortToVlanMemberSet(UI32_T ifindex, UI32_T vid);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_DeletePortFromVlanMemberSet
 * -------------------------------------------------------------------------
 * FUNCTION: This function will delete a port from the member set of a
 *           specified VLAN
 * INPUT   : ifindex -- which port to delete
 *           vid     -- which VLAN ID
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_DeletePortFromVlanMemberSet(UI32_T ifindex, UI32_T vid);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_AddPortToVlanUntaggedSet
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set a port to output untagged frames over
 *           the specified VLAN
 * INPUT   : ifindex -- which port to add
 *           vid     -- which VLAN ID
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_AddPortToVlanUntaggedSet(UI32_T ifindex, UI32_T vid);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_DeletePortFromVlanUntaggedSet
 * -------------------------------------------------------------------------
 * FUNCTION: This function will delete a port from the untagged set of a
 *           specified VLAN
 * INPUT   : ifindex -- which port to add
 *           vid     -- which VLAN ID
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : Delete a port from untagged set means to recover this port to be
 *           a tagged member set of specified vlan.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_DeletePortFromVlanUntaggedSet(UI32_T ifindex, UI32_T vid);

BOOL_T SWCTRL_PMGR_GetSystemMTU (UI32_T *jumbo,UI32_T *mtu);

BOOL_T SWCTRL_PMGR_SetSystemMTU(UI32_T jumbo, UI32_T mtu);

BOOL_T SWCTRL_PMGR_SetPortMTU(UI32_T ifindex, UI32_T mtu);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_EnableIngressFilter
 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable ingress filter of a port
 * INPUT   : ifindex -- which port to enable
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_EnableIngressFilter(UI32_T ifindex);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_DisableIngressFilter
 * -------------------------------------------------------------------------
 * FUNCTION: This function will disable ingress filter of a port
 * INPUT   : ifindex -- which port to disable
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_DisableIngressFilter(UI32_T ifindex);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_AdmitVLANTaggedFramesOnly
 * -------------------------------------------------------------------------
 * FUNCTION: This function will only allow tagged frames entering a port
 * INPUT   : ifindex -- which port to set
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_AdmitVLANTaggedFramesOnly(UI32_T ifindex);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_AdmitVLANUntaggedFramesOnly
 * -------------------------------------------------------------------------
 * FUNCTION: This function will only allow untagged frames entering a port
 * INPUT   : ifindex -- which port to set
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_AdmitVLANUntaggedFramesOnly(UI32_T ifindex);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_AdmitAllFrames

 * -------------------------------------------------------------------------
 * FUNCTION: This function will allow all kinds of frames entering a port
 * INPUT   : ifindex -- which port to set
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_AdmitAllFrames(UI32_T ifindex);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_AllowToBeTrunkMember
 * -------------------------------------------------------------------------
 * FUNCTION: This function will return TRUE if the port is allowed to add
 *           into trunk
 * INPUT   : unit -- unit number
 *           port -- which port to check
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_AllowToBeTrunkMember(UI32_T unit, UI32_T port);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetTrunkBalanceMode
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set the balance mode of trunking
 * INPUT   : balance_mode
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : balance_mode:
 *           SWCTRL_TRUNK_BALANCE_MODE_MAC_SA      Determinded by source mac address
 *           SWCTRL_TRUNK_BALANCE_MODE_MAC_DA      Determinded by destination mac address
 *           SWCTRL_TRUNK_BALANCE_MODE_MAC_SA_DA   Determinded by source and destination mac address
 *           SWCTRL_TRUNK_BALANCE_MODE_IP_SA       Determinded by source IP address
 *           SWCTRL_TRUNK_BALANCE_MODE_IP_DA       Determinded by destination IP address
 *           SWCTRL_TRUNK_BALANCE_MODE_IP_SA_DA    Determinded by source and destination IP address
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetTrunkBalanceMode(UI32_T balance_mode);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetTrunkMaxNumOfActivePorts
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set max number of active ports of trunk
 * INPUT   : trunk_id
 *           max_num_of_active_ports
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetTrunkMaxNumOfActivePorts(UI32_T trunk_id, UI32_T max_num_of_active_ports);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetTrunkMemberActiveStatus
 * -------------------------------------------------------------------------
 * FUNCTION: This function will add a members to a trunk
 * INPUT   : trunk_id   -- which trunking port to set
 *           unit_port  -- member to add
 *           is_active  -- active or inactive member
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetTrunkMemberActiveStatus(UI32_T trunk_id, SYS_TYPE_Uport_T unit_port, BOOL_T is_active);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_EnableIgmpTrap
 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable the IGMP function
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : RFC2933/igmpInterfaceStatus
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_EnableIgmpTrap(SWCTRL_TrapPktOwner_T owner);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_DisableIgmpTrap
 * -------------------------------------------------------------------------
 * FUNCTION: This function will disable the IGMP function
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_DisableIgmpTrap(SWCTRL_TrapPktOwner_T owner);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetUnknownIPMcastFwdPortList
 * -------------------------------------------------------------------------
 * FUNCTION: Set the unknown multicast packet forwarding-to port list.
 * INPUT   : port_list  - on which the multicast packets allow to forward-to
 * OUTPUT  : none
 * RETURN  : TRUE/FALSE
 * NOTE    : To determine which ports are allow to forward the unknow IPMC
 *           packets.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetUnknownIPMcastFwdPortList(UI8_T port_list[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST]);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetBStormControlRateLimit
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set the rate limit of broadcast
 *           storm control function
 * INPUT   : ifindex -- which port to set
 *           mode    -- which mode of rate
 *           nRate   -- rate of broadcast amount
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetBStormControlRateLimit(UI32_T ifindex,
                                        UI32_T mode,
                                        UI32_T nRate);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetMStormControlRateLimit
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set the rate limit of multicast
 *           storm control function
 * INPUT   : ifindex -- which port to set
 *           mode    -- which mode of rate
 *           nRate   -- rate of multicast amount
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetMStormControlRateLimit(UI32_T ifindex,
                                        UI32_T mode,
                                        UI32_T nRate);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetBroadcastStormStatus
 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable the broadcast storm control function
 * INPUT   : ifindex -- which port to set
 *           broadcast_storm_status -- VAL_bcastStormStatus_enabled
 *                                     VAL_bcastStormStatus_disabled
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetBroadcastStormStatus(UI32_T ifindex, UI32_T broadcast_storm_status);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetMulticastStormStatus
 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable the multicast storm control function
 * INPUT   : ifindex -- which port to set
 *           multicast_storm_status -- VAL_bcastStormStatus_enabled
 *                                     VAL_bcastStormStatus_disabled
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetMulticastStormStatus(UI32_T ifindex, UI32_T multicast_storm_status);

#if (SYS_CPNT_STORM_MODE & SYS_CPNT_STORM_UNKNOWN_USTORM)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetUnknownUStormControlRateLimit
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set the rate limit of unknown unicast(DLF)
 *           storm control function
 * INPUT   : ifindex -- which port to set
 *           mode    -- which mode of rate
 *           nRate   -- rate of multicast amount
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetUnknownUStormControlRateLimit(UI32_T ifindex,
                                               UI32_T mode,
                                               UI32_T nRate);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetUnknownUnicastStormStatus
 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable the unknown unicast(DLF) storm control function
 * INPUT   : ifindex -- which port to set
 *           multicast_storm_status -- VAL_unknownUcastStormStatus_enabled
 *                                     VAL_unknownUcastStormStatus_disabled
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetUnknownUnicastStormStatus(UI32_T ifindex, UI32_T unknown_unicast_storm_status);
#endif

#if (SYS_CPNT_SWCTRL_GLOBAL_STORM_SAMPLE_TYPE == TRUE)
/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetGlobalStormSampleType
 *------------------------------------------------------------------------
 * FUNCTION: This function will set global storm sample type
 * INPUT   : ifindex                        - interface index
 *           global_storm_sample_type       - VAL_stormSampleType_pkt_rate
 *                                            VAL_stormSampleType_octet_rate
 *                                            VAL_stormSampleType_percent
 *                                            0 for default value
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetGlobalStormSampleType(UI32_T global_storm_sample_type);
#endif /* (SYS_CPNT_SWCTRL_GLOBAL_STORM_SAMPLE_TYPE == TRUE) */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortUserDefaultPriority
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set user default priority
 * INPUT   : ifindex  -- which port to set
 *           priority -- user default priority to set
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If priority is not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortUserDefaultPriority(UI32_T ifindex, UI32_T priority);



/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPriorityMapping
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set priority mapping of 10/100 ports
 * INPUT   : ifindex -- which port to set
 *           mapping -- priority mapping to set
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If priority is not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPriorityMapping(UI32_T ifindex, UI8_T mapping[8]);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPriorityMappingPerSystem
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set priority mapping of per-system
 * INPUT   : mapping -- priority mapping to set
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If priority is not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPriorityMappingPerSystem(UI8_T mapping[8]);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_DisableTosCosMap
 * -------------------------------------------------------------------------
 * FUNCTION: This function will disable TOS/COS mapping of system
 * INPUT   : none
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : 1.ES3626A
 * -------------------------------------------------------------------------*/
void SWCTRL_PMGR_DisableTosCosMap();


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_EnableDscpCosMap
 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable DSCP/COS mapping of system
 * INPUT   : none
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : 1.ES3626A
 * -------------------------------------------------------------------------*/
void SWCTRL_PMGR_EnableDscpCosMap();


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_DisableDscpCosMap
 * -------------------------------------------------------------------------
 * FUNCTION: This function will disable DSCP/COS mapping of system
 * INPUT   : none
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : 1.ES3626A
 * -------------------------------------------------------------------------*/
void SWCTRL_PMGR_DisableDscpCosMap();


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortTosCosMap
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set one entry of TOS/COS mapping of a port
 * INPUT   : ifindex      -- which port to set
 *           tos          -- from which tos value will be mapped
 *           cos_priority -- to which priority will be mapped
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: Interface not available or asci config fail
 * NOTE    : 1.ES3626A
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortTosCosMap(UI32_T ifindex, UI32_T tos, UI32_T cos_priority);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortDscpCosMap
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set one entry of DSCP/COS mapping of a port
 * INPUT   : ifindex      -- which port to set
 *           dscp         -- from which dscp value will be mapped
 *           cos_priority -- to which priority will be mapped
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: Interface not available or asci config fail
 * NOTE    : 1.ES3626A
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortDscpCosMap(UI32_T ifindex, UI32_T dscp, UI32_T cos_priority);



/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortTcpPortCosMap
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set one entry of TCP_PORT/COS mapping of a port
 * INPUT   : ifindex      -- which port to set
 *           tcp_port     -- from which dscp value will be mapped
 *           cos_priority -- to which priority will be mapped
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: Interface not available or asci config fail
 * NOTE    : 1.ES3626A
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortTcpPortCosMap(UI32_T ifindex, UI32_T tcp_port, UI32_T cos_priority);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_DelPortTosCosMap
 * -------------------------------------------------------------------------
 * FUNCTION: This function will delete one entry of TOS/COS mapping of a port
 * INPUT   : ifindex      -- which port to delete
 *           tos         -- which tos mapping will be deleted
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: Interface not available or asci config fail
 * NOTE    : 1.ES3626A
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_DelPortTosCosMap(UI32_T ifindex, UI32_T tos);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_DelPortDscpCosMap
 * -------------------------------------------------------------------------
 * FUNCTION: This function will delete one entry of DSCP/COS mapping of a port
 * INPUT   : ifindex      -- which port to delete
 *           dscp         -- which dscp mapping will be deleted
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: Interface not available or asci config fail
 * NOTE    : 1.ES3626A
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_DelPortDscpCosMap(UI32_T ifindex, UI32_T dscp);



/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_DelPortTcpPortCosMap
 * -------------------------------------------------------------------------
 * FUNCTION: This function will delete one entry of DSCP/COS mapping of a port
 * INPUT   : ifindex      -- which port to delete
 *           tcp_port     -- which tcp port mapping will be deleted
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: Interface not available or asci config fail
 * NOTE    : 1.ES3626A
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_DelPortTcpPortCosMap(UI32_T ifindex, UI32_T tcp_port);



#if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION == TRUE)
/*********************
 * Private VLAN APIs *
 *********************/
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_EnablePrivateVlan
 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable the private vlan
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_EnablePrivateVlan();



/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_DisablePrivateVlan
 * -------------------------------------------------------------------------
 * FUNCTION: This function will disable the private vlan
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_DisablePrivateVlan();

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPrivateVlan
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set the private vlan
 * INPUT   : uplink_port_list  -- uplink port list
 *           downlink_port_list -- downlink port list
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPrivateVlan(UI8_T uplink_port_list  [SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST],
                             UI8_T downlink_port_list[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST]);

#if (SYS_DFLT_TRAFFIC_SEG_METHOD == SYS_DFLT_TRAFFIC_SEG_METHOD_PORT_PRIVATE_MODE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortPrivateMode
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set the private mode for the specified port
 * INPUT   : ifindex            -- the specified port index
 *           port_private_mode  -- VAL_portPrivateMode_enabled (1L) : private port
 *                                 VAL_portPrivateMode_disabled (2L): public port
 * OUTPU   : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SWCTRL_PMGR_SetPortPrivateMode(UI32_T ifindex, UI32_T port_private_mode);


#endif /* #if (SYS_DFLT_TRAFFIC_SEG_METHOD == SYS_DFLT_TRAFFIC_SEG_METHOD_PORT_PRIVATE_MODE) */

#if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE == SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE_MULTIPLE_SESSION)
/********************************************
 * Multi-Session of Private VLAN APIs       *
 *******************************************/
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPrivateVlanBySessionId
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set the private vlan by group session id
 * INPUT   : session_id         -- is group session id
 *           uplink_port_list   -- uplink port list
 *           downlink_port_list -- downlink port list
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPrivateVlanBySessionId(
              UI32_T session_id,
              UI8_T  uplink_port_list  [SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST],
              UI8_T  downlink_port_list[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST]);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_DeletePrivateVlanPortlistBySessionId
 * -------------------------------------------------------------------------
 * FUNCTION: This function will delete private vlan port list with sesion Id
 * INPUT   : session_id         -- is group session id
 *           uplink_port_list   -- uplink port list
 *           downlink_port_list -- downlink port list
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_DeletePrivateVlanPortlistBySessionId(
              UI32_T session_id,
              UI8_T  uplink_port_list  [SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST],
              UI8_T  downlink_port_list[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST]);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_DestroyPrivateVlanSession
 * -------------------------------------------------------------------------
 * FUNCTION: Destroy entire Private VLAN session
 * INPUT   : session_id   -- pvlan group id
 *           is_uplink    -- is uplink port
 *           is_downlink  -- is downlink port
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_DestroyPrivateVlanSession(UI32_T session_id, BOOL_T is_uplink, BOOL_T is_downlink);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_EnablePrivateVlanUplinkToUplinkBlockingMode
 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable blocking traffic of uplink ports
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_EnablePrivateVlanUplinkToUplinkBlockingMode();

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_DisablePrivateVlanUplinkToUplinkBlockingMode
 * -------------------------------------------------------------------------
 * FUNCTION: This function will disable blocking traffic of uplink ports
 *           so every traffic can be forwarding different uplink ports
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_DisablePrivateVlanUplinkToUplinkBlockingMode();

#endif /* End of #if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE == SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE_MULTIPLE_SESSION)*/
#endif /* End of #if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION == TRUE) */

#if (SYS_CPNT_INGRESS_RATE_LIMIT == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_EnablePortIngressRateLimit
 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable the port ingress rate limit
 * INPUT   : ifindex -- which port to enable
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_EnablePortIngressRateLimit(UI32_T ifindex);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_DisablePortIngressRateLimit
 * -------------------------------------------------------------------------
 * FUNCTION: This function will disable the port ingress rate limit
 * INPUT   : ifindex -- which port to disable
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_DisablePortIngressRateLimit(UI32_T ifindex);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortIngressRateLimit
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set the port ingress rate limit
 * INPUT   : ifindex -- which port to set
 *           rate -- port ingress rate limit
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : # kbits
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortIngressRateLimit(UI32_T ifindex, UI32_T rate);
#endif

#if (SYS_CPNT_EGRESS_RATE_LIMIT == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_EnablePortEgressRateLimit
 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable the port Egress rate limit
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_EnablePortEgressRateLimit(UI32_T ifindex);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_DisablePortEgressRateLimit
 * -------------------------------------------------------------------------
 * FUNCTION: This function will disable the port Egress rate limit
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_DisablePortEgressRateLimit(UI32_T ifindex);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortEgressRateLimit
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set the port egress rate limit
 * INPUT   : ifindex -- which port to set
 *           rate -- port egress rate limit
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : # kbits
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortEgressRateLimit(UI32_T ifindex, UI32_T rate);
#endif

#if (SYS_CPNT_JUMBO_FRAMES == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetJumboFrameStatus
 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable/disable the jumbo frame
 * INPUT   : jumbo_frame_status --  SWCTRL_JUMBO_FRAME_ENABLE
 *                                  SWCTRL_JUMBO_FRAME_DISABLE
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetJumboFrameStatus (UI32_T jumbo_frame_status);

#endif

#if 0 /* deprecated */
#if (SYS_CPNT_WRR_Q_MODE_PER_PORT_CTRL == FALSE)
/*--------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPrioQueueMode
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set the egress schedulering method
 * INPUT:    mode
 *               SWCTRL_WEIGHT_ROUND_ROBIN_METHOD
 *               SWCTRL_STRICT_PRIORITY_METHOD,
 *               SWCTRL_DEFICIT_ROUND_RBIN_METHOD,
 *               SWCTRL_SP_WRR_METHOD,
 *               SWCTRL_SP_DRR_METHOD
 * OUTPUT  : None
 * RETURN:   TRUE/FALSE
 * NOTE:
 *---------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPrioQueueMode(UI32_T mode);
#endif /* (SYS_CPNT_WRR_Q_MODE_PER_PORT_CTRL == FALSE) */

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortPrioQueueMode
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set the port egress schedulering method
 * INPUT:    l_port -- which port to set
 *           mode   -- SWCTRL_STRICT_MODE / SWCTRL_WEIGHT_FAIR_ROUND_ROBIN_MODE
 * OUTPUT  : None
 * RETURN:   TRUE/FALSE
 * NOTE:
 *---------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortPrioQueueMode(UI32_T l_port, UI32_T mode);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortWrrQueueWeight
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set the weight of queue bandwidths
 * INPUT:    l_port     -- This is the primary key and represent the logical port number
 *           q_id       -- This is the second key and represent the index of wrr queue
 *           weight     -- The weight of (l_port, q_id)
 * OUTPUT:   None
 * RETURN:   TRUE/FALSE
 * NOTE:  The ratio of weight determines the weight of the WRR scheduler.
 *        Driver maybe need to provide API to enable WRR of ASIC
 *---------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortWrrQueueWeight(UI32_T l_port, UI32_T q_id, UI32_T weight);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetWrrQueueWeight
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set the weight of queue bandwidths
 * INPUT:    q_id       -- This is the second key and represent the index of wrr queue
 *           weight     -- The weight of (l_port, q_id)
 * OUTPUT:   None
 * RETURN:   TRUE/FALSE
 * NOTE:  The ratio of weight determines the weight of the WRR scheduler.
 *        Driver maybe need to provide API to enable WRR of ASIC
 *---------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetWrrQueueWeight(UI32_T q_id, UI32_T weight);
#endif

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortName
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set name to a port
 * INPUT   : ifindex    -- which port to set
 *           port_name  -- the name to set
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : ES3626A MIB/portMgt 1
 *           size (0..64)
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortName(UI32_T ifindex, UI8_T *port_name);
/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortSpeedDpxCfg
 *------------------------------------------------------------------------
 * FUNCTION: This function will Set the port speed and duplex mode
 * INPUT   : ifindex                        - interface index
 *           port_speed_dpx_cfg             - VAL_portSpeedDpxCfg_halfDuplex10
 *                                            VAL_portSpeedDpxCfg_fullDuplex10
 *                                            VAL_portSpeedDpxCfg_halfDuplex100
 *                                            VAL_portSpeedDpxCfg_fullDuplex100
 *                                            VAL_portSpeedDpxCfg_halfDuplex1000 <== no support
 *                                            VAL_portSpeedDpxCfg_fullDuplex1000
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/portMgt 1
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortSpeedDpxCfg(UI32_T ifindex, UI32_T port_speed_dpx_cfg);

/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortFlowCtrlCfg
 *------------------------------------------------------------------------
 * FUNCTION: This function will Set the port flow control mechanism
 * INPUT   : ifindex                        - interface index
 *           port_flow_ctrl_cfg             - VAL_portFlowCtrlCfg_enabled
 *                                            VAL_portFlowCtrlCfg_disabled
 *                                            VAL_portFlowCtrlCfg_backPressure
 *                                            VAL_portFlowCtrlCfg_dot3xFlowControl
 *                                            VAL_portFlowCtrlCfg_tx
 *                                            VAL_portFlowCtrlCfg_rx
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/portMgt 1
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortFlowCtrlCfg(UI32_T ifindex, UI32_T port_flow_ctrl_cfg);


/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortCapabilities
 *------------------------------------------------------------------------
 * FUNCTION: This function will Set the port capabilities
 * INPUT   : ifindex                        - interface index
 *           port_capabilities              - bitmap to set capability
 *
 *          SYS_VAL_portCapabilities_portCap10half          BIT_0
 *          SYS_VAL_portCapabilities_portCap10full          BIT_1
 *          SYS_VAL_portCapabilities_portCap100half         BIT_2
 *          SYS_VAL_portCapabilities_portCap100full         BIT_3
 *          SYS_VAL_portCapabilities_portCap1000half        BIT_4 <== not support
 *          SYS_VAL_portCapabilities_portCap1000full        BIT_5
 *          SYS_VAL_portCapabilities_portCapSym             BIT_14
 *          SYS_VAL_portCapabilities_portCapFlowCtrl        BIT_15
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/portMgt 1
 * Usage   : set 10half and 100full ==>
 *     bitmap = SYS_VAL_portCapabilities_portCap10half | SYS_VAL_portCapabilities_portCap100full
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortCapabilities(UI32_T ifindex, UI32_T port_capabilities);



/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortAutonegotiation
 *------------------------------------------------------------------------
 * FUNCTION: This function will Set the port autonegotiation
 * INPUT   : ifindex                        - interface index
 *           port_autonegotiation           - VAL_portAutonegotiation_enabled
 *                                            VAL_portAutonegotiation_disabled
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/portMgt 1
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortAutonegotiation(UI32_T ifindex, UI32_T port_autonegotiation);


/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetMirrorType
 *------------------------------------------------------------------------
 * FUNCTION: This function will Set the mirror type
 * INPUT   : ifindex_src   -- which ifindex to mirror
 *          ifindex_dest  -- which ifindex mirrors the received/transmitted packets
 *          port_autonegotiation           - VAL_mirrorType_rx
 *                                           VAL_mirrorType_tx
 *                                           VAL_mirrorType_both
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/mirrorMgt 1
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetMirrorType(UI32_T ifindex_src, UI32_T ifindex_dest, UI32_T mirror_type);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetMirrorStatus
 * -------------------------------------------------------------------------
 * FUNCTION: This function will create/destroy the mirroring function
 * INPUT   : ifindex_src   -- which ifindex to mirror
 *           ifindex_dext  -- which ifindex mirrors the received/transmitted packets
 * OUTPUT  : mirror_status -- VAL_mirrorStatus_valid / VAL_mirrorStatus_invalid
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : 1. ES3626A MIB/mirrorMgt 1
 *           2. No matter support SYS_CPNT_ALLOW_DUMMY_MIRRORING_DEST or not
 *              source == 0 is not valid.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetMirrorStatus(UI32_T ifindex_src , UI32_T ifindex_dest, UI32_T mirror_status);

#if (SYS_CPNT_VLAN_MIRROR == TRUE)
/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_GetVlanMirrorEntry
 *------------------------------------------------------------------------
 * FUNCTION: This function will get the vlan mirror table entry info
 * INPUT   : vlan_mirror_entry->mirror_dest_port        - mirror destination port
 *           vlan_mirror_entry->mirror_source_vlan      - mirror source vlan id
 * OUTPUT  : vlan_mirror_entry                          - The vlan mirror entry info
 * RETURN  : TRUE/FALSE
 * NOTE    : The input keys (ifindex & vid) will get current vlan entry.
 *           if specifies a mirror_dest_port = 0 , so return system dest port
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_GetVlanMirrorEntry(SWCTRL_VlanMirrorEntry_T *vlan_mirror_entry);

/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_GetNextVlanMirrorEntry
 *------------------------------------------------------------------------
 * FUNCTION: This function will get the next vlan mirror table entry info
 * INPUT   : vlan_mirror_entry->mirror_dest_port        - mirror destination port
 *           vlan_mirror_entry->mirror_source_vlan      - mirror source vlan id
 * OUTPUT  : vlan_mirror_entry                          - The vlan mirror entry info
 * RETURN  : TRUE/FALSE
 * NOTE    : The input key shall be contain vlan id and destination port, however
 *           the destination port can be specifies to 0, because we can get
 *           system destination port currently
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_GetNextVlanMirrorEntry(SWCTRL_VlanMirrorEntry_T *vlan_mirror_entry);

/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_AddVlanMirror
 *------------------------------------------------------------------------
 * FUNCTION: This function will add the vlan mirror and desination port
 * INPUT   : vid           -- which vlan-id add to source mirrored table
 *           ifindex_dest  -- which ifindex-port received mirror packets
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : A destination port shall be consistent whenever vlan-id created
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_AddVlanMirror(UI32_T vid, UI32_T ifindex_dest);

/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_DeleteVlanMirror
 *------------------------------------------------------------------------
 * FUNCTION: This function will delete the vlan mirror and destination port
 * INPUT   : vid           -- which vlan-id remove from source mirrored table
 *           ifindex_dest  -- which ifindex-port received mirror packets
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : A destination port shall be removed when source vlan mirror has empty
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_DeleteVlanMirror(UI32_T vid, UI32_T ifindex_dest);
#endif /* End of #if (SYS_CPNT_VLAN_MIRROR == TRUE) */

#if (SYS_CPNT_MAC_BASED_MIRROR == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetMacMirrorEntry
 * -------------------------------------------------------------------------
 * PURPOSE  : This function will set the MAC based MIRROR entry
 * INPUT    : ifindex_dest       -- destnation port
 *            mac_address        -- MAC address
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE    - TRUE if successful;FALSE if failed
 * NOTES    :
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetMacMirrorEntry(UI32_T ifindex_dest, UI8_T *mac_address);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_DeleteMacMirrorEntry
 * -------------------------------------------------------------------------
 * PURPOSE  : This function will delete MAC based MIRROR entry
 * INPUT    : ifindex_dest       -- destnation port
 *            mac_address        -- MAC address
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE    - TRUE if successful;FALSE if failed
 * NOTES    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_DeleteMacMirrorEntry(UI32_T ifindex_dest, UI8_T *mac_address);
#endif /* end of #if (SYS_CPNT_MAC_BASED_MIRROR == TRUE) */

#if (SYS_CPNT_ACL_MIRROR == TRUE)
/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetAclMirrorDestPort
 *------------------------------------------------------------------------
 * FUNCTION: This function will setup dest port for  ACL-based mirror
 * INPUT   : ifindex_dest  -- which ifindex-port received mirror packets
 *           mirror_type   -- mirror type
 *                           (VAL_aclMirrorType_rx/VAL_aclMirrorType_tx/VAL_aclMirrorType_both)
 *           enable        -- TRUE to set, FALSE to remove
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    :
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetAclMirrorDestPort(UI32_T ifindex_dest, UI32_T mirror_type, BOOL_T enable);
#endif

/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetBcastStormStatus
 *------------------------------------------------------------------------
 * FUNCTION: This function will Set the broadcast storm status
 * INPUT   : ifindex                      - interface index
 *           bcast_storm_status           - VAL_bcastStormStatus_enabled
 *                                          VAL_bcastStormStatus_disabled
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/bcastStormMgt 1
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetBcastStormStatus(UI32_T ifindex, UI32_T bcast_storm_status);

/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetBcastStormSampleType
 *------------------------------------------------------------------------
 * FUNCTION: This function will Set the broadcast storm sample type
 * INPUT   : ifindex                        - interface index
 *           bcast_storm_sample_type        - VAL_bcastStormSampleType_pkt_rate
 *                                            VAL_bcastStormSampleType_octet_rate
 *                                            VAL_bcastStormSampleType_percent
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/bcastStormMgt 1
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetBcastStormSampleType(UI32_T ifindex, UI32_T bcast_storm_sample_type);

/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetBcastStormPktRate
 *------------------------------------------------------------------------
 * FUNCTION: This function will Set tthe broadcast storm packet rate
 * INPUT   : ifindex                        - interface index
 *           bcast_storm_pkt_rate           - the broadcast storm packet rate
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/bcastStormMgt 1
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetBcastStormPktRate(UI32_T ifindex, UI32_T bcast_storm_pkt_rate);

/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetBcastStormOctetRate
 *------------------------------------------------------------------------
 * FUNCTION: This function will Set tthe broadcast storm octet rate
 * INPUT   : ifindex                        - interface index
 *           bcast_storm_octet_rate         - the broadcast storm octet rate
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/bcastStormMgt 1
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetBcastStormOctetRate(UI32_T ifindex, UI32_T bcast_storm_octet_rate);
/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetBcastStormPercent
 *------------------------------------------------------------------------
 * FUNCTION: This function will Set tthe broadcast storm octet rate
 * INPUT   : ifindex                       - interface index
 *           bcast_storm_percent           - the broadcast storm percent
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/bcastStormMgt 1
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetBcastStormPercent(UI32_T ifindex, UI32_T bcast_storm_percent);
/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetMcastStormStatus
 *------------------------------------------------------------------------
 * FUNCTION: This function will Set the multicast storm status
 * INPUT   : ifindex                      - interface index
 *           mcast_storm_status           - VAL_mcastStormStatus_enabled
 *                                          VAL_mcastStormStatus_disabled
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/mcastStormMgt 1
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetMcastStormStatus(UI32_T ifindex, UI32_T mcast_storm_status);

/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetMcastStormSampleType
 *------------------------------------------------------------------------
 * FUNCTION: This function will Set the multicast storm sample type
 * INPUT   : ifindex                        - interface index
 *           mcast_storm_sample_type        - VAL_mcastStormSampleType_pkt_rate
 *                                            VAL_mcastStormSampleType_octet_rate
 *                                            VAL_mcastStormSampleType_percent
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/mcastStormMgt 1
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetMcastStormSampleType(UI32_T ifindex, UI32_T mcast_storm_sample_type);

/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetMcastStormPktRate
 *------------------------------------------------------------------------
 * FUNCTION: This function will Set tthe multicast storm packet rate
 * INPUT   : ifindex                        - interface index
 *           mcast_storm_pkt_rate           - the multicast storm packet rate
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/mcastStormMgt 1
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetMcastStormPktRate(UI32_T ifindex, UI32_T mcast_storm_pkt_rate);

/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetMcastStormOctetRate
 *------------------------------------------------------------------------
 * FUNCTION: This function will Set tthe multicast storm octet rate
 * INPUT   : ifindex                        - interface index
 *           mcast_storm_octet_rate         - the multicast storm octet rate
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/mcastStormMgt 1
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetMcastStormOctetRate(UI32_T ifindex, UI32_T mcast_storm_octet_rate);

/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetMcastStormPercent
 *------------------------------------------------------------------------
 * FUNCTION: This function will Set tthe multicast storm octet rate
 * INPUT   : ifindex                       - interface index
 *           mcast_storm_percent           - the multicast storm percent
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/mcastStormMgt 1
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetMcastStormPercent(UI32_T ifindex, UI32_T mcast_storm_percent);


#if (SYS_CPNT_STORM_MODE & SYS_CPNT_STORM_UNKNOWN_USTORM)
/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetUnkucastStormSampleType
 *------------------------------------------------------------------------
 * FUNCTION: This function will Set the unknowunicast storm sample type
 * INPUT   : ifindex                        - interface index
 *           unkucast_storm_sample_type        - VAL_unkucastStormSampleType_pkt_rate
 *                                            VAL_unkucastStormSampleType_octet_rate
 *                                            VAL_unkucastStormSampleType_percent
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/unkucastStormMgt 1
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetUnkucastStormSampleType(UI32_T ifindex, UI32_T unkucast_storm_sample_type);


/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetUnkucastStormPktRate
 *------------------------------------------------------------------------
 * FUNCTION: This function will Set tthe unknowunicast storm packet rate
 * INPUT   : ifindex                        - interface index
 *           unkucast_storm_pkt_rate           - the unknowunicast storm packet rate
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/unkucastStormMgt 1
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetUnkucastStormPktRate(UI32_T ifindex, UI32_T unkucast_storm_pkt_rate);


/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetUnkucastStormOctetRate
 *------------------------------------------------------------------------
 * FUNCTION: This function will Set tthe unknowunicast storm octet rate
 * INPUT   : ifindex                        - interface index
 *           unkucast_storm_octet_rate           - the unknowunicast storm octet rate
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/unkucastStormMgt 1
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetUnkucastStormOctetRate(UI32_T ifindex, UI32_T unkucast_storm_octet_rate);


/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetUnkucastStormPercent
 *------------------------------------------------------------------------
 * FUNCTION: This function will Set tthe unknowunicast storm octet rate
 * INPUT   : ifindex                       - interface index
 *           unkucast_storm_percent           - the unknowunicast storm percent
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/unkucastStormMgt 1
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetUnkucastStormPercent(UI32_T ifindex, UI32_T unkucast_storm_percent);
#endif /* #if (SYS_CPNT_STORM_MODE & SYS_CPNT_STORM_UNKNOWN_USTORM) */


/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_ShutdownSwitch
 *------------------------------------------------------------------------
 * FUNCTION: This function will shutdown the switch before warm start
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    :
 *------------------------------------------------------------------------*/
void SWCTRL_PMGR_ShutdownSwitch(void);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_DisableIPMC
 * -------------------------------------------------------------------------
 * FUNCTION: This function will disable IPMC
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_DisableIPMC(void);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_EnableIPMC
 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable IPMC
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_EnableIPMC(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SWCTRL_PMGR_DisableUMCASTIpTrap
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion will disable trap unknown multicast ip
 * INPUT    :   None
 * OUTPUT   :   None
 * RETURN   :   TRUE/FALSE
 * NOTES    :   for XGS , disable IPMC , trap unknown ip or mcast packet
 * ------------------------------------------------------------------------
 */
BOOL_T SWCTRL_PMGR_DisableUMCASTIpTrap(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SWDRV_EnableUMCASTIpTrap
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion will enable trap unknown multicast ip
 * INPUT    :   None
 * OUTPUT   :   None
 * RETURN   :   TRUE/FALSE
 * NOTES    :   for XGS , disable IPMC , trap unknown ip or mcast packet
 * ------------------------------------------------------------------------
 */
BOOL_T SWCTRL_PMGR_EnableUMCASTIpTrap(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SWCTRL_PMGR_DisableUMCASTMacTrap
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion will disable trap unknown multicast mac
 * INPUT    :   None
 * OUTPUT   :   None
 * RETURN   :   TRUE/FALSE
 * NOTES    :   for XGS , disable IPMC , trap unknown mac or mcast packet
 *              Aaron add, 2003/07/31
 * ------------------------------------------------------------------------
 */
BOOL_T SWCTRL_PMGR_DisableUMCASTMacTrap(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SWDRV_EnableUMCASTMacTrap
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion will enable trap unknown multicast mac
 * INPUT    :   None
 * OUTPUT   :   None
 * RETURN   :   TRUE/FALSE
 * NOTES    :   for XGS , disable IPMC , trap unknown mac or mcast packet
 *              Aaron add, 2003/07/31
 * ------------------------------------------------------------------------
 */
BOOL_T SWCTRL_PMGR_EnableUMCASTMacTrap(void);

/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetUnitsBaseMacAddrTable
 *------------------------------------------------------------------------
 * FUNCTION: This API is used to set the table of MAC address of each unit
 *           in CLI configuration file. This API should only be called by
 *           CLI and should be called before any command rovision.
 * INPUT   : mac_addr --- MAC addresses of all unit.
 *                        If some unit is not present the MAC address should
 *                        be 00-00-00-00-00-00.
 * OUTPUT  : None.
 * RETURN  : TRUE  --- Set table successfully.
 *           FALSE --- Set table fail.
 *                     1) Not in MASETR mode.
 *                     2) Provision has been already completed.
 * NOTE    : None.
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetUnitsBaseMacAddrTable(UI8_T mac_addr[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK][6]);

/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetUnitsBaseMacAddrTable
 *------------------------------------------------------------------------
 * FUNCTION: This API is used to set the table of device type of each unit
 *           in CLI configuration file. This API should only be called by
 *           CLI and should be called before any command rovision.
 * INPUT   : device_type --- Device Types of all unit.
 *                        If some unit is not present the device type should
 *                        be 0xffffffff.
 * OUTPUT  : None.
 * RETURN  : TRUE  --- Set table successfully.
 *           FALSE --- Set table fail.
 *                     1) Not in MASETR mode.
 *                     2) Provision has been already completed.
 * NOTE    : CLI shall call this API after call SWCTRL_SetUnitsBaseMacAddrTable()
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetUnitsDeviceTypeTable(UI32_T device_type[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK]);

#if (SYS_CPNT_MAU_MIB == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetIfMauStatus
 * -------------------------------------------------------------------------
 * FUNCTION: Set MAU status.
 * INPUT   : if_mau_ifindex  -- Which interface.
 *           if_mau_index    -- Which MAU.
 *           status          -- VAL_ifMauStatus_other
 *                              VAL_ifMauStatus_unknown
 *                              VAL_ifMauStatus_operational
 *                              VAL_ifMauStatus_standby
 *                              VAL_ifMauStatus_shutdown
 *                              VAL_ifMauStatus_reset
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : 1) Base on RFC-3636 (snmpDot3MauMgt)
 *           2) For User port only.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetIfMauStatus (UI32_T if_mau_ifindex,
                              UI32_T if_mau_index,
                              UI32_T status);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetIfMauDefaultType
 * -------------------------------------------------------------------------
 * FUNCTION: Set the default MAU type.
 * INPUT   : if_mau_ifindex  -- Which interface.
 *           if_mau_index    -- Which MAU.
 *           default_type    -- Caller should use naming constant in SWCTRL_IF_MAU_TYPE_E.
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : 1) Base on RFC-3636 (snmpDot3MauMgt)
 *           2) For User port only.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetIfMauDefaultType (UI32_T if_mau_ifindex,
                                   UI32_T if_mau_index,
                                   UI32_T default_type);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetIfMauAutoNegAdminStatus
 * -------------------------------------------------------------------------
 * FUNCTION: Set MAU auto-negoation admin status.
 * INPUT   : if_mau_ifindex        -- Which interface.
 *           if_mau_index          -- Which MAU.
 *           auto_neg_admin_status -- VAL_ifMauAutoNegAdminStatus_enabled
 *                                    VAL_ifMauAutoNegAdminStatus_disabled
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : 1) Base on RFC-3636 (snmpDot3MauMgt)
 *           2) For User port only.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetIfMauAutoNegAdminStatus (UI32_T if_mau_ifindex,
                                          UI32_T if_mau_index,
                                          UI32_T auto_neg_admin_status);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetIfMauAutoNegRestart
 * -------------------------------------------------------------------------
 * FUNCTION: Set MAU to auto-negoation restart.
 * INPUT   : if_mau_ifindex   -- Which interface.
 *           if_mau_index     -- Which MAU.
 *           auto_neg_restart -- VAL_ifMauAutoNegRestart_restart
                                 VAL_ifMauAutoNegRestart_norestart
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : 1) Base on RFC-3636 (snmpDot3MauMgt)
 *           2) For User port only.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetIfMauAutoNegRestart (UI32_T if_mau_ifindex,
                                      UI32_T if_mau_index,
                                      UI32_T auto_neg_restart);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetIfMauAutoNegCapAdvertisedBits
 * -------------------------------------------------------------------------
 * FUNCTION: Set advertised capability bits in the MAU.
 * INPUT   : if_mau_ifindex  -- Which interface.
 *           if_mau_index    -- Which MAU.
 *           auto_neg_cap_adv_bits -- (1 << VAL_ifMauAutoNegCapAdvertisedBits_bOther      )
 *                                    (1 << VAL_ifMauAutoNegCapAdvertisedBits_b10baseT    )
 *                                    (1 << VAL_ifMauAutoNegCapAdvertisedBits_b10baseTFD  )
 *                                    (1 << VAL_ifMauAutoNegCapAdvertisedBits_b100baseT4  )
 *                                    (1 << VAL_ifMauAutoNegCapAdvertisedBits_b100baseTX  )
 *                                    (1 << VAL_ifMauAutoNegCapAdvertisedBits_b100baseTXFD)
 *                                    (1 << VAL_ifMauAutoNegCapAdvertisedBits_b100baseT2  )
 *                                    (1 << VAL_ifMauAutoNegCapAdvertisedBits_b100baseT2FD)
 *                                    (1 << VAL_ifMauAutoNegCapAdvertisedBits_bFdxPause   )
 *                                    (1 << VAL_ifMauAutoNegCapAdvertisedBits_bFdxAPause  )
 *                                    (1 << VAL_ifMauAutoNegCapAdvertisedBits_bFdxSPause  )
 *                                    (1 << VAL_ifMauAutoNegCapAdvertisedBits_bFdxBPause  )
 *                                    (1 << VAL_ifMauAutoNegCapAdvertisedBits_b1000baseX  )
 *                                    (1 << VAL_ifMauAutoNegCapAdvertisedBits_b1000baseXFD)
 *                                    (1 << VAL_ifMauAutoNegCapAdvertisedBits_b1000baseT  )
 *                                    (1 << VAL_ifMauAutoNegCapAdvertisedBits_b1000baseTFD)
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : 1) Base on RFC-3636 (snmpDot3MauMgt)
 *           2) For User port only.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetIfMauAutoNegCapAdvertisedBits (UI32_T if_mau_ifindex,
                                                UI32_T if_mau_index,
                                                UI32_T auto_neg_cap_adv_bits);



/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetIfMauAutoNegRemoteFaultAdvertised
 * -------------------------------------------------------------------------
 * FUNCTION: Set advertised auto-negoation remote fault in the MAU.
 * INPUT   : if_mau_ifindex            -- Which interface.
 *           if_mau_index              -- Which MAU.
 *           auto_neg_remote_fault_adv -- VAL_ifMauAutoNegRemoteFaultAdvertised_noError
 *                                        VAL_ifMauAutoNegRemoteFaultAdvertised_offline
 *                                        VAL_ifMauAutoNegRemoteFaultAdvertised_linkFailure
 *                                        VAL_ifMauAutoNegRemoteFaultAdvertised_autoNegError
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : 1) Base on RFC-3636 (snmpDot3MauMgt)
 *           2) For User port only.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetIfMauAutoNegRemoteFaultAdvertised (UI32_T if_mau_ifindex,
                                                    UI32_T if_mau_index,
                                                    UI32_T auto_neg_remote_fault_adv);
#endif /* #if (SYS_CPNT_MAU_MIB == TRUE) */

#if (SYS_CPNT_COMBO_PORT_FORCE_MODE == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortComboForcedMode
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set port force link media
 * INPUT   : ifindex     -- which port to set
 *           forcedmode  -- which mode of media
 *                      - VAL_portComboForcedMode_none
 *                              For trunk and non-combo port only.
 *                      - VAL_portComboForcedMode_copperForced
 *                              Force to copper more ignore SFP transceiver present state.
 *                      - VAL_portComboForcedMode_copperPreferredAuto
 *                              Obsoleted.
 *                      - VAL_portComboForcedMode_sfpForced
 *                              Force to fiber more ignore SFP transceiver present state.
 *                      - VAL_portComboForcedMode_sfpPreferredAuto
 *                              Copper/fiber depends on SFP transceiver present state.
 *                              SFP transceiver present       -> Fiber mode.
 *                                              not present   -> Copper more.
  *           fiber_speed  -- which speed (VAL_portType_hundredBaseFX/VAL_portType_thousandBaseSfp)
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : For trunk, trunk member, and normal port.
 * -------------------------------------------------------------------------*/
#if (SYS_CPNT_COMBO_PORT_FORCED_MODE_SFP_SPEED == TRUE)
BOOL_T SWCTRL_PMGR_SetPortComboForcedMode(UI32_T ifindex, UI32_T forcedmode, UI32_T fiber_speed);
#else
BOOL_T SWCTRL_PMGR_SetPortComboForcedMode(UI32_T ifindex, UI32_T forcedmode);
#endif
#endif


#if (SYS_CPNT_OSPF == TRUE)
/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_EnableOSPFTrap
 *------------------------------------------------------------------------
 * FUNCTION: This API is used to enable OSPF trap.
 * INPUT   : None.
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE.
 * NOTE    : None.
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_EnableOSPFTrap(void);

/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_DisableOSPFTrap
 *------------------------------------------------------------------------
 * FUNCTION: This API is used to disable OSPF trap.
 * INPUT   : None.
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE.
 * NOTE    : None.
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_DisableOSPFTrap(void);
#endif

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortStateWithMstidx
 * -------------------------------------------------------------------------
 * PURPOSE  : Set the Stp port state
 * INPUT    : mstidx    -- multiple spanning tree instance index
 *            lport     -- ifindex of this logical port.
 *                         Only normal port and trunk port is allowed.
 *            state     -- port state 1) VAL_dot1dStpPortState_disabled
 *                                    2) VAL_dot1dStpPortState_blocking
 *                                    3) VAL_dot1dStpPortState_listening
 *                                    4) VAL_dot1dStpPortState_learning
 *                                    5) VAL_dot1dStpPortState_forwarding
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortStateWithMstidx (UI32_T mstidx, UI32_T lport, UI32_T state);


#if (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_MSTP)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_AddVlanToMst
 * -------------------------------------------------------------------------
 * FUNCTION: This function adds a VLAN to a given Spanning Tree instance.
 * INPUT   : vid                -- the VLAN will be added to a given Spanning Tree
 *           mstidx             -- mstidx (multiple spanning tree index) to identify a unique spanning tree
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_AddVlanToMst(UI32_T vid, UI32_T mstidx);
#endif

#if (SYS_CPNT_DOT1X == TRUE)
/****************************************************************************/
/* DOT1X                                                                     */
/****************************************************************************/
/* -------------------------------------------------------------------------
* ROUTINE NAME - SWCTRL_PMGR_SetDot1xAuthTrap
* -------------------------------------------------------------------------
* FUNCTION: This function will trap EtherType 888E packets to CPU
* INPUT   : ifindex
*           mode      -- SWCTRL_DOT1X_PACKET_DISCARD
*                        SWCTRL_DOT1X_PACKET_FORWARD
*                        SWCTRL_DOT1X_PACKET_TRAPTOCPU
* OUTPUT  : None
* RETURN  : TRUE: Successfully, FALSE: If not available
* NOTE    :
* -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetDot1xAuthTrap(UI32_T ifindex, UI32_T mode);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_SetDot1xAuthControlMode
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set dot1x auth control mode
 * INPUT   : unit, port,
 *               mode
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetDot1xAuthControlMode(UI32_T ifindex, UI32_T mode);
#endif

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortSfpDdmTrapEnable
 * -------------------------------------------------------------------------
 * FUNCTION: Enable/Disable SFP ddm threshold alarm trap
 * INPUT   : ifindex   -- which ifindex
 *           trap_enable -- TRUE/FALSE
 *
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------
 */
BOOL_T SWCTRL_PMGR_SetPortSfpDdmTrapEnable(UI32_T ifindex, BOOL_T trap_enable);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortSfpDdmThresholdAutoMode
 * -------------------------------------------------------------------------
 * FUNCTION: Enable/Disable SFP ddm threshold auto mode
 * INPUT   : ifindex   -- which ifindex
 *           auto_mode -- TRUE/FALSE
 *
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------
 */
BOOL_T SWCTRL_PMGR_SetPortSfpDdmThresholdAutoMode(UI32_T ifindex, BOOL_T auto_mode);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortSfpDdmThresholdDefault
 * -------------------------------------------------------------------------
 * FUNCTION: Set SFP ddm threshold to default
 * INPUT   : ifindex   -- which ifindex
 *           threshold_type -- which threshold_type
 *
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------
 */
BOOL_T SWCTRL_PMGR_SetPortSfpDdmThresholdDefault(UI32_T ifindex, UI32_T threshold_type);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortSfpDdmThreshold
 * -------------------------------------------------------------------------
 * FUNCTION: Set SFP ddm threshold
 * INPUT   : ifindex   -- which ifindex
 *           threshold_type -- which threshold_type
 *           val
 *           need_to_check_range -- when cli provision, no need to check range
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------
 */
BOOL_T SWCTRL_PMGR_SetPortSfpDdmThreshold(UI32_T ifindex, UI32_T threshold_type, I32_T val, BOOL_T need_to_check_range);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortSfpDdmThresholdForWeb
 * -------------------------------------------------------------------------
 * FUNCTION: Set SFP ddm threshold for web
 * INPUT   : ifindex   -- which ifindex
 *           threshold_type -- which threshold_type
 *           high_alarm
 *           high_warning
 *           low_warning
 *           low_alarm
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------
 */
BOOL_T SWCTRL_PMGR_SetPortSfpDdmThresholdForWeb(UI32_T ifindex, UI32_T threshold_type, I32_T high_alarm, I32_T high_warning, I32_T low_warning, I32_T low_alarm);

#if (SYS_CPNT_SWCTRL_CABLE_DIAG == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_ExecuteCableDiag
 * -------------------------------------------------------------------------
 * FUNCTION: Get Cable diag result of specific port
 * INPUT   : lport : Logical port num
 * OUTPUT  : result : result of the cable diag test for the port
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_ExecuteCableDiag(UI32_T lport, SWCTRL_Cable_Info_T *result);
#endif    /* #if (SYS_CPNT_SWCTRL_CABLE_DIAG == TRUE) */

#if (SYS_CPNT_RATE_BASED_STORM_CONTROL == TRUE)
/* -------------------------------------------------------------------------
* ROUTINE NAME - SWCTRL_PMGR_SetRateBasedStormControl
* -------------------------------------------------------------------------
* FUNCTION: This function will set the rate based storm control
* INPUT   : ifindex
*           rate      -- kbits/s
*           mode      -- VAL_rateBasedStormMode_bcastStorm |
*                        VAL_rateBasedStormMode_mcastStorm |
*                        VAL_rateBasedStormMode_unknownUcastStorm
* OUTPUT  : None
* RETURN  : TRUE: Successfully, FALSE: If not available
* NOTE    : specifically for BCM53115
* -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetRateBasedStormControl(UI32_T ifindex, UI32_T rate, UI32_T mode);

/* -------------------------------------------------------------------------
* ROUTINE NAME - SWCTRL_PMGR_SetRateBasedStormControlRate
* -------------------------------------------------------------------------
* FUNCTION: This function will set the rate based storm control
* INPUT   : ifindex
*           rate      -- kbits/s
* OUTPUT  : None
* RETURN  : TRUE: Successfully, FALSE: If not available
* NOTE    : specifically for BCM53115
* -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetRateBasedStormControlRate(UI32_T ifindex, UI32_T rate);

/* -------------------------------------------------------------------------
* ROUTINE NAME - SWCTRL_PMGR_SetRateBasedStormControlMode
* -------------------------------------------------------------------------
* FUNCTION: This function will set the rate based storm control
* INPUT   : ifindex
*           mode      -- VAL_rateBasedStormMode_bcastStorm |
*                        VAL_rateBasedStormMode_mcastStorm |
*                        VAL_rateBasedStormMode_unknownUcastStorm
* OUTPUT  : None
* RETURN  : TRUE: Successfully, FALSE: If not available
* NOTE    : specifically for BCM53115
* -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetRateBasedStormControlMode(UI32_T ifindex, UI32_T mode);
#endif

#if (SYS_CPNT_MLDSNP == TRUE)
/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_EnableMldPacketTrap
 *------------------------------------------------------------------------
 * FUNCTION: This API is used to enable MLD packet trap.
 * INPUT   : None.
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE.
 * NOTE    : None.
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_EnableMldPacketTrap(SWCTRL_TrapPktOwner_T owner);

/*------------------------------------------------------------------------
 * ROUTINE NAME - DiableMldPacketTrap
 *------------------------------------------------------------------------
 * FUNCTION: This API is used to enable MLD packet trap.
 * INPUT   : None.
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE.
 * NOTE    : None.
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_DisableMldPacketTrap(SWCTRL_TrapPktOwner_T owner);
#endif

#if (SYS_CPNT_IPV6_RA_GUARD_SW_RELAY == TRUE)
/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetRaAndRrPacketTrap
 *------------------------------------------------------------------------
 * FUNCTION: To enable/disable RA/RR packet trap.
 * INPUT   : is_enabled - TRUE to enable
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE.
 * NOTE    : None.
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetRaAndRrPacketTrap(
    BOOL_T  is_enabled);

#endif /* #if (SYS_CPNT_IPV6_RA_GUARD_SW_RELAY == TRUE) */

#if (SYS_CPNT_IPV6_RA_GUARD_DROP_BY_RULE == TRUE)
/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortRaAndRrPacketDrop
 *------------------------------------------------------------------------
 * FUNCTION: To enable/disable RA/RR packet drop by specified ifindex.
 * INPUT   : ifindex    - ifindex to enable/disable
 *           is_enabled - TRUE to enable
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE.
 * NOTE    : None.
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortRaAndRrPacketDrop(
    UI32_T ifindex, BOOL_T is_enabled);

#endif /* #if (SYS_CPNT_IPV6_RA_GUARD_DROP_BY_RULE == TRUE) */

#if (SYS_CPNT_INTERNAL_LOOPBACK_TEST == TRUE)
/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_ExecuteInternalLoopBackTest
 *------------------------------------------------------------------------
 * FUNCTION: This API is used to do the internal loop back test
 * INPUT   : lport.
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE.
 * NOTE    : None.
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_ExecuteInternalLoopBackTest(UI32_T lport);
#endif /* #if (SYS_CPNT_INTERNAL_LOOPBACK_TEST == TRUE) */

/*added by Jinhua Wei ,to remove warning ,becaued the following functions defined but not declared*/
/*EPR:ES4827G-FLF-ZZ-00232
 *Problem: CLI:size of vlan name different in console and mib
 *Solution: add CLI command "alias" for interface set,the
 *          alias is different from name and port descrition,so
 *          need add new command.
 *modify file: cli_cmd.c,cli_cmd.h,cli_arg.c,cli_arg.h,cli_msg.c,
 *             cli_msg.h,cli_api_vlan.c,cli_api_vlan.h,cli_api_ehternet.c
 *             cli_api_ethernet.h,cli_api_port_channel.c,cli_api_port_channel.h,
 *             cli_running.c,rfc_2863.c,swctrl.h,trk_mgr.h,trk_pmgr.h,swctrl.c
 *             swctrl_pmgr.c,trk_mgr.c,trk_pmgr.c,vlan_mgr.h,vlan_pmgr.h,
 *             vlan_type.h,vlan_mgr.c,vlan_pmgr.c,if_mgr.c
 *Approved by:Hardsun
 *Fixed by:Dan Xie
 */
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortAlias
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set alias to a port
 * INPUT   : ifindex    -- which port to set
 *           port_alias  -- the alias to set
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : ES3626A MIB/portMgt 1
 *           size (0..64)
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortAlias(UI32_T ifindex, UI8_T *port_alias);

#if(SYS_CPNT_AMTR_PORT_MAC_LEARNING == TRUE)/*Tony.Lei*/
/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetPortMACLearningStatus
 *------------------------------------------------------------------------
 * FUNCTION: This API is used to set port status about Mac learning
 * INPUT   : None.
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE.
 * NOTE    : None.
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortMACLearningStatus(UI32_T ifindex, BOOL_T status);
#endif

#if (SYS_CPNT_MLDSNP == TRUE)
/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_EnableMldPacketTrap
 *------------------------------------------------------------------------
 * FUNCTION: This API is used to enable MLD packet trap.
 * INPUT   : None.
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE.
 * NOTE    : None.
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_EnableMldPacketTrap(SWCTRL_TrapPktOwner_T owner);
#endif

#if(SYS_CPNT_SWCTRL_MDIX_CONFIG == TRUE)
BOOL_T SWCTRL_PMGR_SetMDIXMode(UI32_T ifindex, UI32_T mode);
#endif

#if (SYS_CPNT_POWER_SAVE == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortPowerSave
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set the port power saving status
 * INPUT   : ifindex --which port to enable/disable power save
 *               status--TRUE:enable
 *                           FALSE:disable
 * OUTPUT  :
 * RETURN  : True: Successfully, False: If not available
 * NOTE    :None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortPowerSave(UI32_T ifindex,BOOL_T status);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_GetPortPowerSaveStatus
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get the power save status
 * INPUT   : ifindex--the port to get power save status
 * OUTPUT  : status--the power save status of a specific port
 * RETURN  : True: Successfully
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_GetPortPowerSaveStatus(UI32_T ifindex,BOOL_T *status);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_GetRunningPortPowerSaveStatus
 * -------------------------------------------------------------------------
 * FUNCTION: This function get the power save status
 * INPUT   : ifindex--the port to get power save status
 * OUTPUT  : status--the power save status of a specific port
 * RETURN  : SYS_TYPE_Get_Running_Cfg_T
 * NOTE    :
 * -------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T SWCTRL_PMGR_GetRunningPortPowerSaveStatus(UI32_T ifindex, BOOL_T *status);
#endif /* #if (SYS_CPNT_POWER_SAVE == TRUE) */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_TrapUnknownIpMcastToCPU
 * -------------------------------------------------------------------------
 * FUNCTION: This function will trap unknown ip multicast packet to CPU
 * INPUT   : to_cpu -- trap to cpu or not.
 *           flood  -- TRUE to flood to other ports; FLASE to discard the traffic.
 *           owner  -- who call this api
 *           vid = 0 -- global setting, vid = else -- which VLAN ID to set
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : to_cpu = 1 means trap to CPU
 *               to_cpu = 0 mans don't trap to CPU
 *               to_cpu = else means modify flood
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_TrapUnknownIpMcastToCPU(UI8_T to_cpu, BOOL_T flood, SWCTRL_TrapPktOwner_T owner, UI32_T vid);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_TrapUnknownIpv6McastToCPU
 * -------------------------------------------------------------------------
 * FUNCTION: This function will trap unknown ipv6 multicast packet to CPU
 * INPUT   : to_cpu -- trap to cpu or not.
 *           flood  -- TRUE to flood to other ports; FLASE to discard the traffic.
 *           owner  -- who call this api
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : to_cpu = 1 means trap to CPU
 *               to_cpu = 0 mans don't trap to CPU
 *               to_cpu = else means modify flood
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_TrapUnknownIpv6McastToCPU(UI8_T to_cpu, BOOL_T flood, SWCTRL_TrapPktOwner_T owner);

#if(SYS_CPNT_MLDSNP == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_TrapIpv6PIMToCPU
 * -------------------------------------------------------------------------
 * FUNCTION: This function will trap ipv6 PIM packet to CPU
 * INPUT   : to_cpu -- trap to cpu or not.
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_TrapIpv6PIMToCPU(BOOL_T to_cpu, SWCTRL_TrapPktOwner_T owner);
#endif

#if (SYS_CPNT_EFM_OAM == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetOamLoopback
 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable efm oam loopback mode
 * INPUT   :
 *     l_port -- which logical port
 *     enable -- enable/disable loopback mode
 *     flag --
 *          SWCTRL_LOOPBACK_MODE_TYPE_ACTIVE
 *          SWCTRL_LOOPBACK_MODE_TYPE_PASSIVE
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetOamLoopback(UI32_T l_port, BOOL_T enable, UI32_T flag);
#endif /* #if (SYS_CPNT_EFM_OAM == TRUE) */

#if (SYS_CPNT_IPV6_MULTICAST_DATA_DROP== TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_DropIpv6MulticastData
 * -------------------------------------------------------------------------
 * FUNCTION: Set port drop ip multicast data
 * INPUT   : lport - logical port
 * OUTPUT  : None
 * RETURN  :
 * NOTE    : if this port is trunk port, it will set all trunk member
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_DropIpv6MulticastData(UI32_T lport, BOOL_T enabled);
#endif /*#if (SYS_CPNT_IPV6_MULTICAST_DATA_DROP== TRUE) */
#if (SYS_CPNT_ITRI_MIM == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_ITRI_MIM_SetStatus
 * -------------------------------------------------------------------------
 * FUNCTION: Set status of ITRI MAC-in-MAC
 * INPUT   : ifindex
 *           status
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_ITRI_MIM_SetStatus(UI32_T ifindex, BOOL_T status);
#endif /* (SYS_CPNT_ITRI_MIM == TRUE) */

#if (SYS_CPNT_CLUSTER == TRUE)
/* -------------------------------------------------------------------------
 * FUNCTION NAME - SWCTRL_PMGR_SetOrgSpecificTrapStatus
 * -------------------------------------------------------------------------
 * PURPOSE : Set whether organization specific frames are trapped to CPU.
 * INPUT   : trigger - who set the status (SWCTRL_OrgSpecificTrapTrigger_E)
 *           status  - TRUE / FALSE
 * OUTPUT  : None
 * RETURN  : TRUE / FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SWCTRL_PMGR_SetOrgSpecificTrapStatus(UI32_T trigger, BOOL_T status);
#endif /* #if (SYS_CPNT_CLUSTER == TRUE) */

#if (SYS_CPNT_PPPOE_IA == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - SWCTRL_PMGR_SetPPPoEDPktToCpu
 *-------------------------------------------------------------------------
 * PURPOSE: This function will enable/disable traping
 *          PPPoE discover packets to cpu for specified ifindex.
 * INPUT  : ifindex   - ifindex to enable/disable
 *          is_enable - the packet trap is enabled or disabled
 * OUTPUT : None
 * RETURN : TRUE  - success
 *          FALSE - fail
 * NOTE   : 1. if ifindex is trunk, apply to all member ports
 *          2. if ifindex is normal/trunk member, apply to this port
 *          3. for projects who can install rule on trunk's member ports.
 *-------------------------------------------------------------------------
 */
BOOL_T SWCTRL_PMGR_SetPPPoEDPktToCpu(UI32_T ifindex, BOOL_T is_enable);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SWCTRL_PMGR_SetPPPoEDPktToCpuPerSystem
 *-------------------------------------------------------------------------
 * PURPOSE: This function will enable/disable traping
 *          PPPoE discover packets to cpu for per system.
 * INPUT  : is_enable - the packet trapping is enabled or disabled
 * OUTPUT : None
 * RETURN : TRUE  - success
 *          FALSE - fail
 * NOTE   : 1. for projects who encounter problems to install rule on
 *             trunk's member ports.
 *-------------------------------------------------------------------------
 */
BOOL_T SWCTRL_PMGR_SetPPPoEDPktToCpuPerSystem(BOOL_T is_enable);
#endif /* #if (SYS_CPNT_PPPOE_IA == TRUE) */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_EnableDhcpTrap
 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable to trap DHCP packet function
 * INPUT   : owner  -- who want to enable dhcp trap
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_EnableDhcpTrap(SWCTRL_TrapPktOwner_T owner);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_DisableDhcpTrap
 * -------------------------------------------------------------------------
 * FUNCTION: This function will disable to trap DHCP packet function
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_DisableDhcpTrap(SWCTRL_TrapPktOwner_T owner);

#if (SYS_CPNT_DOS == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetDosProtectionFilter
 * -------------------------------------------------------------------------
 * FUNCTION: This function will config DoS protection
 * INPUT   : type   - the type of DOS protection to config
 *           enable - TRUE to enable; FALSE to disable.
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetDosProtectionFilter(SWCTRL_DosProtectionFilter_T type, BOOL_T enable);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetDosProtectionRateLimit
 * -------------------------------------------------------------------------
 * FUNCTION: This function will config DoS protection
 * INPUT   : type   - the type of DOS protection to config
 *           rate   - rate in kbps. 0 to disable.
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetDosProtectionRateLimit(SWCTRL_DosProtectionRateLimit_T type, UI32_T rate);
#endif /* (SYS_CPNT_DOS == TRUE) */

#if ((SYS_CPNT_DHCPV6 == TRUE)||(SYS_CPNT_DHCPV6SNP == TRUE))
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_EnableDhcp6PacketTrap
 * -------------------------------------------------------------------------
 * FUNCTION: This function will trap dhcpv6 packet to CPU
 * INPUT   : owner  -- who want the trap
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    :
 *   CSC    | Client packet action  |  Server packet action
 * =========|=======================|=========================
 *  Client  |        flood          |     copy to cpu
 *  relay   |     copy to cpu       |     copy to cpu
 * Snooping |   redirect to cpu     |   redirect to cpu
 *  Server  |     copy to cpu       |        flood
 *  NONE    |        flood          |        flood
 * =========|=======================|=========================
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_EnableDhcp6PacketTrap(SWCTRL_TrapPktOwner_T owner);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_DisableDhcp6PacketTrap
 * -------------------------------------------------------------------------
 * FUNCTION: This function will trap dhcpv6 packet to CPU
 * INPUT   : owner  -- who want the trap
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    :
 *   CSC   | Client packet action  |  Server packet action
 * ========|=======================|=========================
 *  Client |        flood          |     copy to cpu
 *  relay  |     copy to cpu       |     copy to cpu
 * Snooping|   redirect to cpu     |   redirect to cpu
 *  Server |     copy to cpu       |        flood
 *  NONE   |        flood          |        flood
 * ========|=======================|=========================
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_DisableDhcp6PacketTrap(SWCTRL_TrapPktOwner_T owner);
#endif

/* -------------------------------------------------------------------------
 * FUNCTION NAME - SWCTRL_PMGR_SetPktTrapStatus
 * -------------------------------------------------------------------------
 * PURPOSE : to trap packet
 * INPUT   : pkt_type  - which packet to trap
 *           owner     - who set the status
 *           to_cpu    - trap to cpu or not
 *           drop      - drop packet or not
 * OUTPUT  : None
 * RETURN  : TRUE / FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SWCTRL_PMGR_SetPktTrapStatus(SWCTRL_PktType_T pkt_type, SWCTRL_TrapPktOwner_T owner, BOOL_T to_cpu, BOOL_T drop);

/* -------------------------------------------------------------------------
 * FUNCTION NAME - SWCTRL_PMGR_SetPortPktTrapStatus
 * -------------------------------------------------------------------------
 * PURPOSE : to trap packet
 * INPUT   : ifindex
 *           pkt_type  - which packet to trap
 *           owner     - who set the status
 *           to_cpu    - trap to cpu or not
 *           drop      - drop packet or not
 * OUTPUT  : None
 * RETURN  : TRUE / FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SWCTRL_PMGR_SetPortPktTrapStatus(UI32_T ifindex, SWCTRL_PktType_T pkt_type, SWCTRL_TrapPktOwner_T owner, BOOL_T to_cpu, BOOL_T drop);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_GetPDPortstatus
 * -------------------------------------------------------------------------
 * FUNCTION: Get PD port status
 * INPUT   : entry_p->port_pd_ifindex
 * OUTPUT  : entry_p->port_pd_status --- SWDRV_POWER_SOURCE_NONE
 *                                       SWDRV_POWER_SOURCE_UP
 *                                       SWDRV_POWER_SOURCE_DOWN
 *           entry_p->port_pd_mode   --- SWDRV_POWERED_DEVICE_MODE_NONE
 *                                       SWDRV_POWERED_DEVICE_MODE_AF
 *                                       SWDRV_POWERED_DEVICE_MODE_AT
 * RETURN  : TRUE -- Sucess, FALSE -- Failed
 * NOTE    : The status of the ports with POE PD capability would show "UP"
 *           when the link partner is a PSE port.
 * -------------------------------------------------------------------------
 */
BOOL_T SWCTRL_PMGR_GetPDPortStatus(SWCTRL_PortPD_T *entry_p);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_GetNextPDPortstatus
 * -------------------------------------------------------------------------
 * FUNCTION: Get next PD port status
 * INPUT   : entry_p->port_pd_ifindex
 * OUTPUT  : entry_p->port_pd_ifindex
 *           entry_p->port_pd_status --- SWDRV_POWER_SOURCE_NONE
 *                                       SWDRV_POWER_SOURCE_UP
 *                                       SWDRV_POWER_SOURCE_DOWN
 *           entry_p->port_pd_mode   --- SWDRV_POWERED_DEVICE_MODE_NONE
 *                                       SWDRV_POWERED_DEVICE_MODE_AF
 *                                       SWDRV_POWERED_DEVICE_MODE_AT
 * RETURN  : TRUE -- Sucess, FALSE -- Failed
 * NOTE    : The status of the ports with POE PD capability would show "UP"
 *           when the link partner is a PSE port.
 * -------------------------------------------------------------------------
 */
BOOL_T SWCTRL_PMGR_GetNextPDPortStatus(SWCTRL_PortPD_T *entry_p);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_GetPSECheckStatus
 * -------------------------------------------------------------------------
 * FUNCTION: Get PSE check status
 * INPUT   : None
 * OUTPUT  : pse_check_status_p --- TRUE: PSE check enabled, FALSE: PSE check disabled
 * RETURN  : TRUE -- Sucess, FALSE -- Failed
 * NOTE    : When PSE check status is enabled, all of the ports with POE PD
 *           capability are able to link up when the link partner is a PSE port.
 *           When PSE check status is disabled, all of the ports with POE PD
 *           capability will not be blocked by SWDRV to link up. However, if
 *           upper layer CSC shutdown the port, the port is never link up.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_GetPSECheckStatus(BOOL_T* pse_check_status_p);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPSECheckStatus
 * -------------------------------------------------------------------------
 * FUNCTION: Set PSE check status
 * INPUT   : pse_check_status --- TRUE: PSE check enabled, FALSE: PSE check disabled
 * OUTPUT  : None
 * RETURN  : TRUE -- Sucess, FALSE -- Failed
 * NOTE    : When PSE check status is enabled, all of the ports with POE PD
 *           capability are able to link up when the link partner is a PSE port.
 *           When PSE check status is disabled, all of the ports with POE PD
 *           capability will not be blocked by SWDRV to link up. However, if
 *           upper layer CSC shutdown the port, the port is never link up.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPSECheckStatus(BOOL_T pse_check_status);

#if (SYS_CPNT_NDSNP == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_EnableNdPacketTrap
 * -------------------------------------------------------------------------
 * FUNCTION: This function will trap nd packet to CPU
 * INPUT   : owner  -- who want the trap
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    :
 *   CSC    | Client packet action
 * =========|=======================
 *  NDSNP   |    redirect to cpu
 *  NONE    |        flood
 * =========|=======================
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_EnableNdPacketTrap(SWCTRL_TrapPktOwner_T owner);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_DisableNdPacketTrap
 * -------------------------------------------------------------------------
 * FUNCTION: This function will trap nd packet to CPU
 * INPUT   : owner  -- who want the trap
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    :
 *   CSC    | Client packet action
 * =========|=======================
 *  NDSNP   |    redirect to cpu
 *  NONE    |        flood
 * =========|=======================
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_DisableNdPacketTrap(SWCTRL_TrapPktOwner_T owner);

#endif

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SWCTRL_PMGR_SetPortLearningStatus
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion will set port learning status
 * INPUT    :   ifindex
 *              learning
 *              owner
 * OUTPUT   :   None
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None
 * ------------------------------------------------------------------------
 */
BOOL_T SWCTRL_PMGR_SetPortLearningStatus(UI32_T ifindex, BOOL_T learning, SWCTRL_LearningDisabledOwner_T owner);

#if (SYS_CPNT_PFC == TRUE)
/* -------------------------------------------------------------------------
 * FUNCTION NAME - SWCTRL_PMGR_SetPortPfcStatus
 * -------------------------------------------------------------------------
 * PURPOSE: To set the PFC port status by specified ifidx.
 * INPUT  : ifidx      -- ifindex to set
 *          rx_en      -- enable/disable PFC response
 *          tx_en      -- enable/disable PFC triggering
 *          pri_en_vec -- bitmap of enable status per priority
 *                         set bit to enable PFC; clear to disable.
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTE   : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortPfcStatus(
    UI32_T  ifidx,
    BOOL_T  rx_en,
    BOOL_T  tx_en,
    UI16_T  pri_en_vec);

/* -------------------------------------------------------------------------
 * FUNCTION NAME - SWCTRL_PMGR_UpdatePfcPriMap
 * -------------------------------------------------------------------------
 * PURPOSE : This function update PFC priority to queue mapping.
 * INPUT   : None
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTE   : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_UpdatePfcPriMap(void);
#endif /* #if (SYS_CPNT_PFC == TRUE) */

#if (SYS_CPNT_ETS == TRUE)
/*------------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortCosGroupMapping
 *------------------------------------------------------------------------------
 * FUNCTION: This function sets mapping between CoS Queue and CoS group
 * INPUT   : ifindex
 *           cosq2group -- array of cos groups.
 *                         element 0 is cos group of cosq 0,
 *                         element 1 is cos group of cosq 1, ...
 *                         NULL to map all cos to single cos group
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: failed
 * NOTE    : None
 *------------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortCosGroupMapping(
    UI32_T ifindex,
    UI32_T cosq2group[SYS_ADPT_MAX_NBR_OF_PRIORITY_QUEUE]);

/*------------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortCosGroupSchedulingMethod
 *------------------------------------------------------------------------------
 * FUNCTION: This function sets scheduling method for CoS groups
 * INPUT   : ifindex
 *           method  -- SWCTRL_Egress_Scheduling_Method_E
 *           weights -- weights for cos groups.
 *                      NULL if method is STRICT
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: failed
 * NOTE    : None
 *------------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortCosGroupSchedulingMethod(
    UI32_T ifindex,
    UI32_T method,
    UI32_T weights[SYS_ADPT_ETS_MAX_NBR_OF_TRAFFIC_CLASS]);
#endif /* (SYS_CPNT_ETS == TRUE) */

#if (SYS_CPNT_CN == TRUE)
/*------------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetQcnCnmPriority
 *------------------------------------------------------------------------------
 * FUNCTION: This function sets 802.1p priority of egress QCN CNM
 * INPUT   : pri -- 802.1p priority of egress QCN CNM
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: failed
 * NOTE    : None
 *------------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetQcnCnmPriority(UI32_T pri);

/*------------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortQcnCpq
 *------------------------------------------------------------------------------
 * FUNCTION: This function sets CP Queue of the CoS Queue
 * INPUT   : ifindex
 *           cosq -- CoS Queue
 *           cpq  -- CP Queue. SWCTRL_QCN_CPQ_INVALID means to disable QCN
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: failed
 * NOTE    : None
 *------------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortQcnCpq(
    UI32_T ifindex,
    UI32_T cosq,
    UI32_T cpq);

/*------------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortQcnEgrCnTagRemoval
 *------------------------------------------------------------------------------
 * FUNCTION: This function sets removal of CN-Tag of egress pkts
 * INPUT   : ifindex
 *           no_cntag_bitmap - bit 0 for pri 0, and so on.
 *                             set the bit to remove CN-tag of packets with the pri.
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: failed
 * NOTE    : None
 *------------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortQcnEgrCnTagRemoval(
    UI32_T ifindex,
    UI8_T no_cntag_bitmap);

/*------------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_GetPortQcnCpid
 *------------------------------------------------------------------------------
 * FUNCTION: This function gets CPID
 * INPUT   : ifindex
 *           cosq -- CoS Queue
 *           cpid -- CPID
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: failed
 * NOTE    : None
 *------------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_GetPortQcnCpid(
    UI32_T ifindex,
    UI32_T cosq,
    UI8_T cpid[8]);
#endif /* (SYS_CPNT_CN == TRUE) */

#if (SYS_CPNT_MAC_IN_MAC == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetMimService
 * -------------------------------------------------------------------------
 * FUNCTION: This function will create/destroy a MiM service instance.
 * INPUT   : mim_p            -- MiM service instance info.
 *           is_valid         -- TRUE to create/update; FALSE to destroy.
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetMimService(SWCTRL_MimServiceInfo_T *mim_p, BOOL_T is_valid);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetMimServicePort
 * -------------------------------------------------------------------------
 * FUNCTION: This function will add/delete member port to a MiM service instance.
 * INPUT   : mim_port_p       -- MiM port info.
 *           is_valid         -- TRUE to add; FALSE to delete.
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetMimServicePort(SWCTRL_MimPortInfo_T *mim_port_p, BOOL_T is_valid);

#if (SYS_CPNT_IAAS == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - SWCTRL_PMGR_SetMimServicePortLearningStatusForStationMove
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion will set MiM port learning status
 *              for station move handling only
 * INPUT    :   learning
 *              to_cpu
 *              drop
 * OUTPUT   :   None
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None
 * ------------------------------------------------------------------------
 */
BOOL_T SWCTRL_PMGR_SetMimServicePortLearningStatusForStationMove(BOOL_T learning, BOOL_T to_cpu, BOOL_T drop);
#endif /* (SYS_CPNT_IAAS == TRUE) */
#endif /* (SYS_CPNT_MAC_IN_MAC == TRUE) */

/* -------------------------------------------------------------------------
 * FUNCTION NAME - SWCTRL_PMGR_SetCpuRateLimit
 * -------------------------------------------------------------------------
 * PURPOSE : To configure CPU rate limit
 * INPUT   : pkt_type  -- SWDRV_PKTTYPE_XXX
 *           rate      -- in pkt/s. 0 to disable.
 * OUTPUT  : None
 * RETURN  : TRUE / FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SWCTRL_PMGR_SetCpuRateLimit(UI32_T pkt_type, UI32_T rate);

#if (SYS_CPNT_SFLOW == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - SWCTRL_PMGR_SetSflowPortPacketSamplingRate
 *-------------------------------------------------------------------------
 * PURPOSE  : Set port sampling rate.
 * INPUT    : ifindex  -- interface index
 *            rate     -- sampling rate
 * OUTPUT   : None
 * RETUEN   : TRUE/FALSE
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T
SWCTRL_PMGR_SetSflowPortPacketSamplingRate(
    UI32_T ifindex,
    UI32_T rate);
#endif /* #if (SYS_CPNT_SFLOW == TRUE) */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SWCTRL_PMGR_DropPortCdpPacket
 *-------------------------------------------------------------------------
 * PURPOSE : Drop CDP packet
 * INPUT   : enable  -- enabled/disabled this feature
 *           ifindex -- interface index
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T
SWCTRL_PMGR_DropPortCdpPacket(
    BOOL_T enable,
    UI32_T ifindex
);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SWCTRL_PMGR_DropPortPvstPacket
 *-------------------------------------------------------------------------
 * PURPOSE : Drop CDP packet
 * INPUT   : enable  -- enabled/disabled this feature
 *           ifindex -- interface index
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T
SWCTRL_PMGR_DropPortPvstPacket(
    BOOL_T enable,
    UI32_T ifindex
);
#if (SYS_CPNT_VRRP == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_TrapVrrpToCpu
 * -------------------------------------------------------------------------
 * FUNCTION: This function will trap VRRP packet to cpu
 * INPUT   : trap or not
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_TrapVrrpToCpu(BOOL_T is_trap);
#endif

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SWCTRL_PMGR_PortTypeToMediaType
 *-------------------------------------------------------------------------
 * PURPOSE  : Convert port type to media type.
 * INPUT    : port_type  -- port type (e.g. VAL_portType_hundredBaseTX)
 * OUTPUT   : None
 * RETUEN   : The media type of the specified port type.
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
SWCTRL_Port_Media_Type_T SWCTRL_PMGR_PortTypeToMediaType(UI32_T port_type);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SWCTRL_PMGR_GetPortFigureType
 * ------------------------------------------------------------------------|
 * FUNCTION : This function returns the port figure type of the specified
 *            port.
 * INPUT    : ifindex            : interface index.
 * OUTPUT   : port_figure_type_p : port figure type of the specified port.
 * RETURN   : TRUE  -  Success
 *            FALSE -  Failed
 * NOTE     : 1. This function is for WEB to draw the apperance of the
 *               specified port on the front panel of web pages by the port
 *               figure type.
 *            2. ifindex can only be a ifindex for a physical port.
 *            3. port_figure_type will be STKTPLG_TYPE_PORT_FIGURE_TYPE_NULL
 *               if the port is on a expansion module and the expansion module
 *               is not inserted.
 * ------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_GetPortFigureType(UI32_T ifindex, STKTPLG_TYPE_Port_Figure_Type_T *port_figure_type_p);

#if (SYS_CPNT_MAU_MIB == TRUE)

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_GetIfMauEntry
 * -------------------------------------------------------------------------
 * FUNCTION: Get MAU entry specified in RFC-3636.
 * INPUT   : if_mau_entry->ifMauIfIndex  -- Which interface.
 *           if_mau_entry->ifMauIndex    -- Which MAU.
 * OUTPUT  : if_mau_entry.
 * RETURN  : TRUE/FALSE
 * NOTE    : 1) Base on RFC-3636 (snmpDot3MauMgt)
 *           2) For User port only.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_GetIfMauEntry (SWCTRL_IfMauEntry_T *if_mau_entry);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_GetNextIfMauEntry
 * -------------------------------------------------------------------------
 * FUNCTION: Get next MAU entry specified in RFC-3636.
 * INPUT   : if_mau_entry->ifMauIfIndex  -- Next to which interface.
 *           if_mau_entry->ifMauIndex    -- Next to which MAU.
 * OUTPUT  : if_mau_entry.
 * RETURN  : TRUE/FALSE
 * NOTE    : 1) Base on RFC-3636 (snmpDot3MauMgt)
 *           2) For User port only.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_GetNextIfMauEntry (SWCTRL_IfMauEntry_T *if_mau_entry);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_GetIfMauAutoNegEntry
 * -------------------------------------------------------------------------
 * FUNCTION: Get auto-negoation entry of the MAU, that is specified in REF-3636
 * INPUT   : if_mau_auto_neg_entry->ifMauIfIndex -- Which interface.
 *           if_mau_auto_neg_entry->ifMauIndex   -- Which MAU.
 * OUTPUT  : if_mau_auto_neg_entry.
 * RETURN  : TRUE/FALSE
 * NOTE    : 1) Base on RFC-3636 (snmpDot3MauMgt)
 *           2) For User port only.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_GetIfMauAutoNegEntry (SWCTRL_IfMauAutoNegEntry_T *if_mau_auto_neg_entry);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_GetNextIfMauAutoNegEntry
 * -------------------------------------------------------------------------
 * FUNCTION: Get next auto-negoation entry of the MAU, one is specified in RFC-3636
 * INPUT   : if_mau_auto_neg_entry->ifMauIfIndex  -- Next to which interface.
 *           if_mau_auto_neg_entry->ifMauIndex    -- Next to which Which MAU.
 * OUTPUT  : if_mau_auto_neg_entry.
 * RETURN  : TRUE/FALSE
 * NOTE    : 1) Base on RFC-3636 (snmpDot3MauMgt)
 *           2) For User port only.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_GetNextIfMauAutoNegEntry (SWCTRL_IfMauAutoNegEntry_T *if_mau_auto_neg_entry);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_GetIfJackEntry
 * -------------------------------------------------------------------------
 * FUNCTION: Get jack entry of the jack in some MAU, one is specified in RFC-3636.
 * INPUT   : if_jack_entry->ifMauIfIndex  -- Which interface.
 *           if_jack_entry->ifMauIndex    -- Which MAU.
 *           if_jack_entry->ifJackIndex   -- Which jack.
 * OUTPUT  : if_jack_entry.
 * RETURN  : TRUE/FALSE
 * NOTE    : 1) Base on RFC-3636 (snmpDot3MauMgt)
 *           2) For User port only.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_GetIfJackEntry (SWCTRL_IfJackEntry_T *if_jack_entry);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_GetNextIfJackEntry
 * -------------------------------------------------------------------------
 * FUNCTION: Get next jack entry of the jack in some MAU, one is specified in RFC-3636.
 * INPUT   : if_jack_entry->ifMauIfIndex  -- Next to which interface.
 *           if_jack_entry->ifMauIndex    -- Next to which MAU.
 *           if_jack_entry->ifJackIndex   -- Next to which jack.
 * OUTPUT  : if_jack_entry.
 * RETURN  : TRUE/FALSE
 * NOTE    : 1) Base on RFC-3636 (snmpDot3MauMgt)
 *           2) For User port only.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_GetNextIfJackEntry (SWCTRL_IfJackEntry_T *if_jack_entry);
#endif

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortEgressBlock
 * -------------------------------------------------------------------------
 * FUNCTION: To set egress block port.
 * INPUT   : lport              - source lport
 *           egr_lport          - lport to block
 *           is_block           - TRUE to enable egress block status
 *                                FALSE to disable egress block status
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortEgressBlock(
    UI32_T lport,
    UI32_T egr_lport,
    BOOL_T is_block);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetPortEgressBlockEx
 * -------------------------------------------------------------------------
 * FUNCTION: To set egress block ports.
 * INPUT   : lport               - source lport
 *           egr_lport_list      - lport list to update egress block status.
 *                                 NULL to indicate all lport list.
 *           blk_lport_list      - lport list to specify egress block status.
 *                                 set bit to enable egress block status, clear bit to disable.
 *                                 NULL to indicate empty lport list.
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetPortEgressBlockEx(
    UI32_T lport,
    UI8_T egr_lport_list[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST],
    UI8_T blk_lport_list[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST]);

#if (SYS_CPNT_HASH_SELECTION == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_BindHashSelForService
 * -------------------------------------------------------------------------
 * FUNCTION: add service reference of the hash-selection list
 * INPUT   : service     - SWCTRL_OM_HASH_SEL_SERVICE_ECMP,
 *                         SWCTRL_OM_HASH_SEL_SERVICE_TRUNK
 *           list_index - the index of hash-selection list
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_BindHashSelForService(
    SWCTRL_OM_HashSelService_T service, 
    UI8_T list_index);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_UnBindHashSelForService
 * -------------------------------------------------------------------------
 * FUNCTION: remove service reference of the hash-selection list
 * INPUT   : service     - SWCTRL_OM_HASH_SEL_SERVICE_ECMP,
 *                         SWCTRL_OM_HASH_SEL_SERVICE_TRUNK
 *           list_index - the index of hash-selection list
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_UnBindHashSelForService(
    SWCTRL_OM_HashSelService_T service, 
    UI8_T list_index);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_AddHashSelection
 * -------------------------------------------------------------------------
 * FUNCTION: add hash-selection field
 * INPUT   : list_index - the index of hash-selection list
 *           selection_p - hash-selection field
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_AddHashSelection(
    UI8_T list_index , 
    SWCTRL_OM_HashSelection_T *selection_p);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_RemoveHashSelection
 * -------------------------------------------------------------------------
 * FUNCTION: remove hash-selection field
 * INPUT   : list_index - the index of hash-selection list
 *           selection_p - hash-selection field
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_RemoveHashSelection(
    UI8_T list_index ,
    SWCTRL_OM_HashSelection_T *selection_p);
#endif /*#if (SYS_CPNT_HASH_SELECTION == TRUE)*/

#if (SYS_CPNT_SWCTRL_SWITCH_MODE_CONFIGURABLE == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PMGR_SetSwitchingMode
 * -------------------------------------------------------------------------
 * FUNCTION: To set switching mode
 * INPUT   : lport  -- which port to configure.
 *           mode   -- VAL_swctrlSwitchModeSF
 *                     VAL_swctrlSwitchModeCT
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PMGR_SetSwitchingMode(UI32_T lport, UI32_T mode);
#endif /*#if (SYS_CPNT_SWCTRL_SWITCH_MODE_CONFIGURABLE == TRUE)*/

#if(SYS_CPNT_WRED == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - SWCTRL_PMGR_RandomDetect
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion will set port ecn marking percentage
 * INPUT    :   lport      - which port to set
 *              value_p    - percentage value
 * OUTPUT   :   None
 * RETURN   :   TRUE/FALSE
 * NOTES    :   queue_id = -1 means all queue
 * ------------------------------------------------------------------------
 */
BOOL_T SWCTRL_PMGR_RandomDetect(UI32_T lport, SWCTRL_RandomDetect_T *value_p);
#endif

#endif /* End of #ifndef SWCTRL_PMGR_H */

