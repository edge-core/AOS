/* MODULE NAME:  swctrl_pom.h
 * PURPOSE: For accessing om through IPC
 *
 *
 * NOTES:
 *
 * REASON:
 * Description:
 * HISTORY
 *    5/3/2007 - Eli Lin, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */
#ifndef SWCTRL_POM_H
#define SWCTRL_POM_H

#include "swctrl.h"

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SWCTRL_POM_Init
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Do initialization procedures for CSCA_POM.
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
void SWCTRL_POM_Init(void);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_isPortLinkUp
 * -------------------------------------------------------------------------
 * FUNCTION: This function will check the port is link up or not
 * INPUT   : ifindex   -- which port to get
 * OUTPUT  : None
 * RETURN  : TRUE: Link Up, FALSE: Link Down
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_POM_isPortLinkUp(UI32_T ifindex);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_GetPortInfo
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get all information of a port
 * INPUT   : ifindex   -- which port to get
 * OUTPUT  : p_info -- all information of this port
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_POM_GetPortInfo(UI32_T ifindex, Port_Info_T *p_info);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_GetNextPortInfo
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get all information of a port
 * INPUT   : ifindex   -- the key to get
 * OUTPUT  : ifindex   -- the next existing port
 *           port_info -- all information of this port
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_POM_GetNextPortInfo(UI32_T *ifindex, Port_Info_T *p_info);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_GetRunningPortInfo
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get the port info of running config
 *           a port
 * INPUT   : ifindex      -- which port to get
 * OUTPUT  : port_info    -- the port information
 * RETURN  : One of SYS_TYPE_Get_Running_Cfg_T
 * NOTE    : None
 * -------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T SWCTRL_POM_GetRunningPortInfo(UI32_T ifindex, Port_Info_T *p_info);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SWCTRL_POM_GetPortLinkStatus
 * -------------------------------------------------------------------------
 * FUNCTION: This rutine provide the information of the link status of
 *           the port.
 * INPUT   : ifindex --- The information of which port to get.
 * OUTPUT  : *port_link_status --- Link status of this port.
 *                                 1) SWCTRL_LINK_UP
 *                                 2) SWCTRL_LINK_DOWN
 * RETURN  : TRUE  --- Succefully.
 *           FALSE --- 1) ifindex is not belong to user port or logic port.
 *                     2) This port is not present.
 * NOTE    : This runtine provide subset information of SWCTRL_GetPortInfo().
 *           If some task has stack size issue, caller should use this rutine
 *           to get the operating status.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_POM_GetPortLinkStatus(UI32_T ifindex, UI32_T *port_link_status);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_GetPortStatus
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get port administration status
 * INPUT   : ifindex        -- which port to set
 * OUTPUT  : shutdown_reason_p -- indicates why status is down
 *                             bitmap of SWCTRL_PORT_STATUS_SET_BY_XXX
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_POM_GetPortStatus(UI32_T ifindex, UI32_T *shutdown_reason_p);

#if (SYS_CPNT_3COM_LOOPBACK_TEST == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_GetSwitchLoopbackTestFailurePorts
 * -------------------------------------------------------------------------
 * FUNCTION: SNMP call this API the get loopback test failure ports.
 * INPUT   : None.
 * OUTPUT  : failure_ports --- If the port is loopback test failure port,
 *                            the bit is "1", otherwise the bit is "0".
 *                            The MSB of byte 0 is port 1,
 *                            the LSB of byte 0 is port 8,
 *                            the MSB of byte 1 is port 9,
 *                            the LSB of byte 1 is port 16,
 *                            ...
 *                            and so on.
 * RETURN  : TRUE  --- Succeful.
 *           FALSE --- Failed.
 * NOTE    : For user port only.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_POM_GetSwitchLoopbackTestFailurePorts(UI8_T failure_ports[SYS_ADPT_NBR_OF_BYTE_FOR_1BIT_UPORT_LIST*SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK]);

#endif

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_GetUnitPortNumber
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get the total number of ports in a specified
 *           unit
 * INPUT   : unit -- which unit to get
 * OUTPUT  : None
 * RETURN  : The total number of ports
 * NOTE    : None
 * -------------------------------------------------------------------------*/
UI32_T SWCTRL_POM_GetUnitPortNumber(UI32_T unit);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_GetSystemPortNumber
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get the total number of ports in the system
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : The total number of ports
 * NOTE    : None
 * -------------------------------------------------------------------------*/
UI32_T SWCTRL_POM_GetSystemPortNumber(void);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_GetCpuMac
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get the MAC address of CPU
 * INPUT   : None
 * OUTPUT  : mac -- the buffer to put MAC address
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_POM_GetCpuMac(UI8_T *mac);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_GetPortMac
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get the MAC address of a port
 * INPUT   : ifindex -- which port to get
 * OUTPUT  : mac     -- the buffer to put MAC address
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_POM_GetPortMac(UI32_T ifindex, UI8_T *mac);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_GetLastChangeTime
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get the last change time of whole system
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : the time of last change of any port
 * NOTE    : None
 * -------------------------------------------------------------------------*/
UI32_T SWCTRL_POM_GetLastChangeTime();

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SWCTRL_POM_IsSecurityPort
 * ------------------------------------------------------------------------|
 * FUNCTION : check the port is in security
 * INPUT    : ifindex : the port
 * OUTPUT	: port_security_enabled_by_who ---	SWCTRL_PORT_SECURITY_ENABLED_BY_NONE
 *                 					SWCTRL_PORT_SECURITY_ENABLED_BY_PSEC
 *                 					SWCTRL_PORT_SECURITY_ENABLED_BY_DOT1X
 * RETURN   : TRUE/FALSE
 * NOTE		: Port security doesn't support 1) unknown port, 2) trunk member, and
 *                                          3) trunk port
 * ------------------------------------------------------------------------*/
BOOL_T SWCTRL_POM_IsSecurityPort( UI32_T ifindex,UI32_T *port_security_enabled_by_who/*kevin*/);

#if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION == TRUE)
#if (SYS_DFLT_TRAFFIC_SEG_METHOD == SYS_DFLT_TRAFFIC_SEG_METHOD_PORT_PRIVATE_MODE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetPortPrivateMode
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get the private mode for the specified port
 * INPUT   : ifindex            -- the specified port index
 * OUTPUT  : port_private_mode  -- VAL_portPrivateMode_enabled (1L) : private port
 *                                 VAL_portPrivateMode_disabled (2L): public port
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SWCTRL_POM_GetPortPrivateMode(UI32_T ifindex, UI32_T *port_private_mode);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetNextPortPrivateMode
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get the private mode for the next port
 * INPUT   : ifindex            -- which port to get
 * OUTPUT  : ifindex            -- the next port index
 *           port_private_mode  -- VAL_portPrivateMode_enabled (1L) : private port
 *                                 VAL_portPrivateMode_disabled (2L): public port
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SWCTRL_POM_GetNextPortPrivateMode(UI32_T *ifindex, UI32_T *port_private_mode);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetRunningPortPrioQueueMode
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the running config of PrivateMode of the
 *           specified port.
 * INPUT:    ifindex           -- which port to get
 * OUTPUT:   port_private_mode -- VAL_portPrivateMode_enabled (1L) : private port
 *                                VAL_portPrivateMode_disabled (2L): public port
 * RETURN:   SYS_TYPE_GET_RUNNING_CFG_FAIL -- error (system is not in MASTER mode)
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 *           SYS_TYPE_GET_RUNNING_CFG_SUCCESS -- different from default value
 * NOTE:
 *---------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T SWCTRL_POM_GetRunningPortPrivateMode(UI32_T ifindex, UI32_T *port_private_mode);
#endif /* #if (SYS_DFLT_TRAFFIC_SEG_METHOD == SYS_DFLT_TRAFFIC_SEG_METHOD_PORT_PRIVATE_MODE) */
#endif /* #if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION == TRUE) */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_GetPrivateVlan
 * -------------------------------------------------------------------------
 * FUNCTION:
 * INPUT   :
 * OUTPUT  :
 * RETURN  :
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_POM_GetPrivateVlan(SWCTRL_PrivateVlan_T *private_vlan);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_GetPrivateVlanStatus
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get the private vlan status
 * INPUT   : None
 * OUTPU   : vlan_status -- VAL_privateVlanStatus_enabled/VAL_privateVlanStatus_disabled
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_POM_GetPrivateVlanStatus(UI32_T *vlan_status);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_GetRunningPrivateVlanStatus
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get the private vlan running config
 * INPUT   : None
 * OUTPU   : vlan_status -- VAL_privateVlanStatus_enabled/VAL_privateVlanStatus_disabled
 * RETURN  : SYS_TYPE_Get_Running_Cfg_T
 * NOTE    : None
 * -------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T SWCTRL_POM_GetRunningPrivateVlanStatus(UI32_T *vlan_status);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_GetRunningPrivateVlanUplinkPortList
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get the private vlan running config
 * INPUT   : None
 * OUTPU   : uplink_port_list -- uplink port list
 * RETURN  : SYS_TYPE_Get_Running_Cfg_T
 * NOTE    : None
 * -------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T SWCTRL_POM_GetRunningPrivateVlanUplinkPortList(
                                UI8_T uplink_port_list[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST]);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - SWCTRL_POM_IsPortPrivateVlanUplinkMember
 *--------------------------------------------------------------------------
 * PURPOSE  : This funcion returns true if current port is in private vlan's
 *            uplink member list.  Otherwise, returns false.
 * INPUT    : ifindex -- the specified port
 * OUTPUT   : none
 * RETURN   : TRUE \ FALSE
 * NOTES    : none
 *--------------------------------------------------------------------------*/
BOOL_T SWCTRL_POM_IsPortPrivateVlanUplinkMember(UI32_T ifindex);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - SWCTRL_POM_IsPortPrivateVlanDownlinkMember
 *--------------------------------------------------------------------------
 * PURPOSE  : This funcion returns true if current port is in private vlan's
 *            downlink member list.  Otherwise, returns false.
 * INPUT    : ifindex -- the specified port
 * OUTPUT   : none
 * RETURN   : TRUE \ FALSE
 * NOTES    : none
 *--------------------------------------------------------------------------*/
BOOL_T SWCTRL_POM_IsPortPrivateVlanDownlinkMember(UI32_T ifindex);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_GetRunningPrivateVlanDownlinkPortList
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get the private vlan running config
 * INPUT   : None
 * OUTPU   : downlink_port_list -- downlink port list
 * RETURN  : SYS_TYPE_Get_Running_Cfg_T
 * NOTE    : None
 * -------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T SWCTRL_POM_GetRunningPrivateVlanDownlinkPortList(
                                UI8_T downlink_port_list[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST]);

#if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION == TRUE)
#if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE == SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE_MULTIPLE_SESSION)
/********************************************
 * Multi-Session of Private VLAN APIs       *
 *******************************************/
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_GetPrivateVlanBySessionId
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get private vlan by group session id
 * INPUT   : session_id         -- is group session id
 *           uplink_port_list   -- uplink port list
 *           downlink_port_list -- downlink port list
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_POM_GetPrivateVlanBySessionId(
              UI32_T session_id,
              UI8_T  uplink_port_list  [SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST],
              UI8_T  downlink_port_list[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST]);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_GetRunningPrivateVlanPortListBySessionId
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get the private vlan running config by sId
 * INPUT   : session_id         -- get pvlan group id
 *           uplink_port_list   -- uplink port list
 *           downlink_port_list -- downlink port list
 * OUTPU   : uplink_port_list, downlink_port_list
 * RETURN  : SYS_TYPE_Get_Running_Cfg_T, No change while entry no exist
 * NOTE    : None
 * -------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T SWCTRL_POM_GetRunningPrivateVlanPortListBySessionId(
                                UI32_T session_id,
                                UI8_T  uplink_port_list  [SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST],
                                UI8_T  downlink_port_list[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST]);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_GetNextSessionFromPrivateVlanPortList
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get private vlan for next session Id
 * INPUT   : session_id -- pvlan group id
 * OUTPUT  : session_id -- return achieved session id
 *           uplink_port_list   -- uplink port list
 *           downlink_port_list -- downlink port list
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_POM_GetNextSessionFromPrivateVlanPortList(
              UI32_T *session_id,
              UI8_T  uplink_port_list[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST],
              UI8_T  downlink_port_list[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST]);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_IsUserPortJoinPrivateVlanToTrunk
 * -------------------------------------------------------------------------
 * FUNCTION: Check if this user port could be trunk member or not for pvlan
 * INPUT   : trunk_id   -- which trunking port to set
 *           unit_port  -- member to add
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_POM_IsUserPortJoinPrivateVlanToTrunk(UI32_T trunk_id, SYS_TYPE_Uport_T unit_port);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_GetRunningPrivateVlanUplinkToUplinkStatus
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get private vlan uplink-to-uplink mode
 * INPUT   : None
 * OUTPU   : up_status -- uplink-to-uplink mode (blocking/forwarding)
 * RETURN  : SYS_TYPE_Get_Running_Cfg_T
 * NOTE    : None
 * -------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T SWCTRL_POM_GetRunningPrivateVlanUplinkToUplinkStatus(UI32_T *up_status);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_GetPrivateVlanUplinkToUplinkStatus
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get the private vlan uplink-to-uplink mode
 * INPUT   : None
 * OUTPU   : up_status -- uplink-to-uplink mode (blocking/forwarding)
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_POM_GetPrivateVlanUplinkToUplinkStatus(UI32_T *up_status);

#endif /* End of #if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE == SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE_MULTIPLE_SESSION)*/
#endif /* End of #if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION == TRUE) */


#if (SYS_CPNT_INGRESS_RATE_LIMIT == TRUE)
/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_GetRunningPortIngressRateLimitStatus
 *------------------------------------------------------------------------
 * FUNCTION: This function will get the ingress rate limit status running config
 * INPUT   : ifindex
 * OUTPUT  : port_ingress_rate_limit_status -- enable/disable
 * RETURN  : SYS_TYPE_Get_Running_Cfg_T
 * NOTE    : None
 *------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T SWCTRL_POM_GetRunningPortIngressRateLimitStatus(UI32_T ifindex, UI32_T *port_ingress_rate_limit_status);

#endif
#if (SYS_CPNT_EGRESS_RATE_LIMIT == TRUE)
/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_GetRunningPortEgressRateLimitStatus
 *------------------------------------------------------------------------
 * FUNCTION: This function will get the Egress rate limit status running config
 * INPUT   : None
 * OUTPUT  : port_rgress_rate_limit_status -- enable/disable
 * RETURN  : SYS_TYPE_Get_Running_Cfg_T
 * NOTE    : None
 *------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T SWCTRL_POM_GetRunningPortEgressRateLimitStatus(UI32_T ifindex, UI32_T *port_egress_rate_limit_status);
#endif
#if (SYS_CPNT_JUMBO_FRAMES == TRUE)

/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SWCTRL_POM_GetJumboFrameStatus
 *------------------------------------------------------------------------
 * FUNCTION: This function will get the jumbo frame status
 * INPUT   : None
 * OUTPUT  : jumbo_frame_status -- SWCTRL_JUMBO_FRAME_ENABLE/SWCTRL_JUMBO_FRAME_DISABLE
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_POM_GetJumboFrameStatus (UI32_T *jumbo_frame_status);

/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_GetRunningJumboFrameStatus
 *------------------------------------------------------------------------
 * FUNCTION: This function will get the jumbo frame running config
 * INPUT   : None
 * OUTPUT  : jumbo_frame_status -- SWCTRL_JUMBO_FRAME_ENABLE/SWCTRL_JUMBO_FRAME_DISABLE
 * RETURN  : SYS_TYPE_Get_Running_Cfg_T
 * NOTE    : None
 *------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T SWCTRL_POM_GetRunningJumboFrameStatus (UI32_T *jumbo_frame_status);

#endif

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_GetNextLogicalPort
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get the next existing logical port
 * INPUT   : l_port -- the key
 * OUTPUT  : l_port -- the next existing logical port
 * RETURN  : One of SWCTRL_Lport_Type_T
 * NOTE    : None
 * -------------------------------------------------------------------------*/
SWCTRL_Lport_Type_T SWCTRL_POM_GetNextLogicalPort(UI32_T *l_port);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_LogicalPortExisting
 * -------------------------------------------------------------------------
 * FUNCTION: This function will return if this port is existing
 * INPUT   : l_port -- the key to ask
 * OUTPUT  : None
 * RETURN  : TRUE: Existing, FALSE: Not existing
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_POM_LogicalPortExisting(UI32_T l_port);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_UserPortExisting
 * -------------------------------------------------------------------------
 * FUNCTION: This function will return if this user port is existing
 * INPUT   : unit -- which unit
 *           port -- the key to ask
 * OUTPUT  : None
 * RETURN  : TRUE: Existing, FALSE: Not existing
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_POM_UserPortExisting(UI32_T unit, UI32_T port);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_LogicalPortIsTrunkPort
 * -------------------------------------------------------------------------
 * FUNCTION: This function will return if this port is trunk port
 * INPUT   : uport_ifindex -- the key to ask
 * OUTPUT  : None
 * RETURN  : TRUE: trunk port , FALSE: Not existing
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_POM_LogicalPortIsTrunkPort(UI32_T ifindex);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_IsTrunkMember
 * -------------------------------------------------------------------------
 * FUNCTION: This function is used to get if a user port is trunk member, and
 *           if this user port is a trunk member check if this static trunk
 *           member or dynamic trunk member.
 * INPUT   : uport_ifindex --- which user port.
 * OUTPUT  : trunk_ifindex --- if this user port is trunk member, this is user
 *                             port is belong to which trunk
 *           is_static --- TRUE:  static trunk member.
 *                         FALSE: dynamic trunk member.
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_POM_IsTrunkMember(UI32_T uport_ifindex, UI32_T *trunk_ifindex, BOOL_T *is_static);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_GetTrunkIfIndexByUport
 * -------------------------------------------------------------------------
 * FUNCTION: This function will return if this port is trunk member
 * INPUT   : uport_ifindex -- the key to ask
 * OUTPUT  : trunk_ifindex -- trunk ifindex
 * RETURN  : TRUE: trunk member , FALSE: Not existing
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_POM_GetTrunkIfIndexByUport(UI32_T uport_ifindex, UI32_T *trunk_ifindex);

#if 0 /* deprecated */
#if (SYS_CPNT_WRR_Q_MODE_PER_PORT_CTRL == FALSE)
/*--------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_GetPrioQueueMode
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the egress schedulering method
 * INPUT:    None
 * OUTPUT:   mode -- SWCTRL_STRICT_MODE / SWCTRL_WEIGHT_FAIR_ROUND_ROBIN_MODE
 * RETURN:   TRUE / FALSE
 * NOTE:
 *---------------------------------------------------------------------------*/
BOOL_T SWCTRL_POM_GetPrioQueueMode(UI32_T *mode);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_GetRunningPrioQueueMode
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the egress schedulering method
 * INPUT:    None
 * OUTPUT:   mode -- SWCTRL_STRICT_MODE / SWCTRL_WEIGHT_FAIR_ROUND_ROBIN_MODE
 * RETURN:   SYS_TYPE_GET_RUNNING_CFG_FAIL -- error (system is not in MASTER mode)
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 *           SYS_TYPE_GET_RUNNING_CFG_SUCCESS -- different from default value
 * NOTE:
 *---------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T SWCTRL_POM_GetRunningPrioQueueMode(UI32_T *mode);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_GetStrictQueueMap
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the strict queue map
 * INPUT:    None
 * OUTPUT:   strict queue map
 * RETURN:   TRUE / FALSE
 * NOTE:
 *---------------------------------------------------------------------------*/
BOOL_T SWCTRL_POM_GetStrictQueueMap(UI8_T *map);
#endif /* (SYS_CPNT_WRR_Q_MODE_PER_PORT_CTRL == FALSE) */

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_GetPortPrioQueueMode
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the port egress schedulering method
 * INPUT:    l_port-- which port to get
 * OUTPUT:   mode -- SWCTRL_STRICT_MODE / SWCTRL_WEIGHT_FAIR_ROUND_ROBIN_MODE
 * RETURN:   TRUE / FALSE
 * NOTE:
 *---------------------------------------------------------------------------*/
BOOL_T SWCTRL_POM_GetPortPrioQueueMode(UI32_T l_port, UI32_T *mode);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_GetPortStrictQueueMap
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the port strict queue map
 * INPUT:    l_port-- which port to get
 * OUTPUT:   strict queue map
 * RETURN:   TRUE / FALSE
 * NOTE:
 *---------------------------------------------------------------------------*/
BOOL_T SWCTRL_POM_GetPortStrictQueueMap(UI32_T l_port, UI8_T *map);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_GetNextPortPrioQueueMode
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the next port egress schedulering method
 * INPUT:    l_port-- which port to get
 * OUTPUT:   l_port-- the next port
 *		 mode -- SWCTRL_STRICT_MODE / SWCTRL_WEIGHT_FAIR_ROUND_ROBIN_MODE
 * RETURN:   TRUE / FALSE
 * NOTE:
 *---------------------------------------------------------------------------*/
BOOL_T SWCTRL_POM_GetNextPortPrioQueueMode(UI32_T *l_port, UI32_T *mode);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_GetRunningPortPrioQueueMode
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the port egress schedulering method
 * INPUT:    l_port -- which port to get
 * OUTPUT:   mode -- SWCTRL_STRICT_MODE / SWCTRL_WEIGHT_FAIR_ROUND_ROBIN_MODE
 * RETURN:   SYS_TYPE_GET_RUNNING_CFG_FAIL -- error (system is not in MASTER mode)
 *                SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 *                SYS_TYPE_GET_RUNNING_CFG_SUCCESS -- different from default value
 * NOTE:
 *---------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T SWCTRL_POM_GetRunningPortPrioQueueMode(UI32_T l_port, UI32_T *mode);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_GetNextRunningPortPrioQueueMode
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the next port egress schedulering method
 * INPUT:    l_port-- which port to get
 * OUTPUT:   mode -- SWCTRL_STRICT_MODE / SWCTRL_WEIGHT_FAIR_ROUND_ROBIN_MODE
 * RETURN:   SYS_TYPE_GET_RUNNING_CFG_FAIL -- error (system is not in MASTER mode)
 *                SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 *                SYS_TYPE_GET_RUNNING_CFG_SUCCESS -- different from default value
 * NOTE:
 *---------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T SWCTRL_POM_GetNextRunningPortPrioQueueMode(UI32_T *l_port, UI32_T *mode);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_GetPortWrrQueueWeight
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the current weight of queue bandwidths
 * INPUT:    l_port     -- This is the primary key and represent the logical port number
 *           q_id       -- This is the second key and represent the index of wrr queue
 * OUTPUT:   weight     -- The weight of (l_port, q_id)
 * RETURN:   TRUE/FALSE
 * NOTE:  The ratio of weight determines the weight of the WRR scheduler.
 *---------------------------------------------------------------------------*/
BOOL_T SWCTRL_POM_GetPortWrrQueueWeight(UI32_T l_port, UI32_T q_id, UI32_T *weight);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_GetNextPortWrrQueueWeight
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the value of indexed entry and output next index
 * INPUT:    l_port     -- This is the primary key and represent the logical port number
 *           q_id       -- This is the second key and represent the index of wrr queue
 * OUTPUT:   l_port -- next index
 *           q_id   -- next index
 *           weight     -- The weight of (l_port, q_id)
 * RETURN:   True/False
 * NOTE:     The returned value will base on the database of core layer
 *           rather than the database of ASIC
 *---------------------------------------------------------------------------*/
BOOL_T SWCTRL_POM_GetNextPortWrrQueueWeight(UI32_T *l_port, UI32_T *q_id, UI32_T *weight);
/*--------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_GetRunningPortWrrQueueWeight
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the current weight of WRR queues
 * INPUT:    l_port     -- This is the primary key and represent the logical port number
 *           q_id       -- This is the second key and represent the index of wrr queue
 * OUTPUT:   weight     -- The weight of (l_port, q_id)
 * RETURN:   SYS_TYPE_GET_RUNNING_CFG_FAIL -- error (system is not in MASTER mode)
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 *           SYS_TYPE_GET_RUNNING_CFG_SUCCESS -- different from default value
 * NOTE:
 *---------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T SWCTRL_POM_GetRunningPortWrrQueueWeight(UI32_T l_port, UI32_T q_id, UI32_T *weight);
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SWCTRL_POM_GetNextRunningPortWrrQueueWeight
 * ---------------------------------------------------------------------
 * PURPOSE: This function same as GetRunning but also output next index
 * INPUT:    l_port     -- This is the primary key and represent the logical port number
 *           q_id       -- This is the second key and represent the index of wrr queue
 * OUTPUT:   l_port -- next index
 *           q_id   -- next index
 *           weight -- The weight of (l_port, q_id)
 * RETURN:  status : SYS_TYPE_Get_Running_Cfg_T
 *                    1.SYS_TYPE_GET_RUNNING_CFG_FAIL -- system not in MASTER mode
 *                    2.SYS_TYPE_GET_RUNNING_CFG_SUCCESS -- different with default
 *                    3.SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE-- same as default
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default weight
 *        3. Caller has to prepare buffer for storing weight
 * ---------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T SWCTRL_POM_GetNextRunningPortWrrQueueWeight(UI32_T *l_port , UI32_T *q_id, UI32_T *weight);
#endif

/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_GetPortEntry
 *------------------------------------------------------------------------
 * FUNCTION: This function will get the port table entry info
 * INPUT   : port_entry->port_index - interface index
 * OUTPUT  : port_entry             - The port table entry info
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/portMgt 1
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_POM_GetPortEntry(SWCTRL_PortEntry_T *port_entry);
/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_GetNextPortEntry
 *------------------------------------------------------------------------
 * FUNCTION: This function will get the next port table entry info
 * INPUT   : port_entry->port_index - interface index
 * OUTPUT  : port_entry             - The port table entry info
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/portMgt 1
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_POM_GetNextPortEntry(SWCTRL_PortEntry_T *port_entry);

/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_GetMirrorEntry
 *------------------------------------------------------------------------
 * FUNCTION: This function will get the mirror table entry info
 * INPUT   : mirror_entry->mirror_destination_port - mirror destination port
 *           mirror_entry->mirror_source_port      - mirror source port
 * OUTPUT  : mirror_entry                          - The mirror entry info
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/mirrorMgt 1
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_POM_GetMirrorEntry(SWCTRL_MirrorEntry_T *mirror_entry);

/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_GetNextMirrorEntry
 *------------------------------------------------------------------------
 * FUNCTION: This function will get the next mirror table entry info
 * INPUT   : mirror_entry->mirror_destination_port - mirror destination port
 *           mirror_entry->mirror_source_port      - mirror source port
 * OUTPUT  : mirror_entry                          - The mirror entry info
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/mirrorMgt 1
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_POM_GetNextMirrorEntry(SWCTRL_MirrorEntry_T *mirror_entry);

/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_GetNextRunningMirrorEntry
 *------------------------------------------------------------------------
 * FUNCTION: This function will get the next running config mirror table entry info
 * INPUT   : mirror_entry->mirror_destination_port - mirror destination port
 *           mirror_entry->mirror_source_port      - mirror source port
 * OUTPUT  : mirror_entry                          - The mirror entry info
 * RETURN  : SYS_TYPE_Get_Running_Cfg_T
 * NOTE    : None
 *------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T SWCTRL_POM_GetNextRunningMirrorEntry(SWCTRL_MirrorEntry_T *mirror_entry);

/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_GetBcastStormEntry
 *------------------------------------------------------------------------
 * FUNCTION: This function will get the broadcast storm management entry
 * INPUT   : bcast_storm_entry->bcast_storm_ifindex - interface index
 * OUTPUT  : bcast_storm_entry                      - broadcast storm management entry
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/bcastStormMgt 1
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_POM_GetBcastStormEntry(SWCTRL_BcastStormEntry_T *bcast_storm_entry);

/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_GetNextBcastStormEntry
 *------------------------------------------------------------------------
 * FUNCTION: This function will get the next broadcast storm management entry
 * INPUT   : bcast_storm_entry->bcast_storm_ifindex - interface index
 * OUTPUT  : bcast_storm_entry                      - broadcast storm management entry
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/bcastStormMgt 1
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_POM_GetNextBcastStormEntry(SWCTRL_BcastStormEntry_T *bcast_storm_entry);

/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_GetMcastStormEntry
 *------------------------------------------------------------------------
 * FUNCTION: This function will get the multicast storm management entry
 * INPUT   : mcast_storm_entry->bcast_storm_ifindex - interface index
 * OUTPUT  : mcast_storm_entry                      - multicast storm management entry
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/mcastStormMgt 1
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_POM_GetMcastStormEntry(SWCTRL_McastStormEntry_T *mcast_storm_entry);


/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_GetNextMcastStormEntry
 *------------------------------------------------------------------------
 * FUNCTION: This function will get the next multicast storm management entry
 * INPUT   : mcast_storm_entry->mcast_storm_ifindex - interface index
 * OUTPUT  : mcast_storm_entry                      - multicast storm management entry
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/mcastStormMgt 1
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_POM_GetNextMcastStormEntry(SWCTRL_McastStormEntry_T *mcast_storm_entry);


#if (SYS_CPNT_STORM_MODE & SYS_CPNT_STORM_UNKNOWN_USTORM)
/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_GetUnkucastStormEntry
 *------------------------------------------------------------------------
 * FUNCTION: This function will get the unknowunicast storm management entry
 * INPUT   : unkucast_storm_entry->unkucast_storm_ifindex - interface index
 * OUTPUT  : unkucast_storm_entry                      - unknowunicast storm management entry
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/unkucastStormMgt 1
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_POM_GetUnkucastStormEntry(SWCTRL_UnknownUcastStormEntry_T *unkucast_storm_entry);

/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetNextUnkucastStormEntry
 *------------------------------------------------------------------------
 * FUNCTION: This function will get the next unknowunicast storm management entry
 * INPUT   : unkucast_storm_entry->unkucast_storm_ifindex - interface index
 * OUTPUT  : unkucast_storm_entry                      - unknowunicast storm management entry
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/unkucastStormMgt 1
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_POM_GetNextUnkucastStormEntry(SWCTRL_UnknownUcastStormEntry_T *unkucast_storm_entry);
#endif

/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_GetPortStormGranularity
 *------------------------------------------------------------------------
 * FUNCTION: This function will get granularity of a port
 * INPUT   : ifindex      -- which port to get
 * OUTPUT  : granularity  --  granularity of a port
 * RETURN  : TRUE/FALSE
 * NOTE    : 
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_POM_GetPortStormGranularity(UI32_T ifindex, UI32_T *granularity);

/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_UIGetUnitPortNumber
 *------------------------------------------------------------------------
 * FUNCTION: This API is used to get the number of some unit that caller
 *           want to get and should only be used by  CLI.
 * INPUT   : unit --- which unit caller want to get.
 * OUTPUT  : None.
 * RETURN  : The port number of that unit.
 *           > 0 --- 1) Base on the unit MAC address table given by CLI
 *                      before provision complete.
 *                   2) Output normal ifindex after provision complete.
 *           0   --- Get fail.
 *                   1) Not in master mode.
 *                   2) Argument is invalid.
 *                   3) This unit is not present.
 * NOTE    : Return port number base on the table given by CLI before provision complete.
 *           Return port number normally after provision complete.
 *------------------------------------------------------------------------*/
UI32_T SWCTRL_POM_UIGetUnitPortNumber(UI32_T unit);

/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_UIUserPortToLogicalPort
 *------------------------------------------------------------------------
 * FUNCTION: This API is used to get ifindex of some unit and some port that
 *           caller want to get and should only be used by  CLI.
 * INPUT   : unit --- which unit caller want to get.
 *           port --- which port caller want to get.
 * OUTPUT  : ifindex --- 1) Base on the unit MAC address table given by CLI
 *                          before provision complete.
 *                       2) Output normal ifindex after provision complete.
 * RETURN  : SWCTRL_LPORT_UNKNOWN_PORT      --- 1) Not in master mode.
 *                                              2) Argument is invalid.
 *                                              3) This port is not present.
 *           SWCTRL_LPORT_NORMAL_PORT       --- This is a normal port.
 *           SWCTRL_LPORT_TRUNK_PORT_MEMBER --- This port is a member of a trunk.
 * NOTE    : Process "unit" base on the table given by CLI before provision complete.
 *           Process "unit" normally after provision complete.
 *------------------------------------------------------------------------*/
SWCTRL_Lport_Type_T SWCTRL_POM_UIUserPortToLogicalPort(UI32_T unit, UI32_T port, UI32_T *ifindex);

/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_UIUserPortToIfindex
 *------------------------------------------------------------------------
 * FUNCTION: This API is used to get ifindex of some unit and some port that
 *           caller want to get and should only be used by  CLI.
 * INPUT   : unit --- which unit caller want to get.
 *           port --- which port caller want to get.
 * OUTPUT  : ifindex --- 1) Base on the unit MAC address table given by CLI
 *                          before provision complete.
 *                       2) Output normal ifindex after provision complete.
 *           is_inherit- 1) TRUE  UI should inherit this ifindex's config to provision
 *                       2) FALSE UI should not inherit this ifindex's config to provision
 * RETURN  : SWCTRL_LPORT_UNKNOWN_PORT      --- 1) Not in master mode.
 *                                              2) Argument is invalid.
 *                                              3) This port is not present.
 *           SWCTRL_LPORT_NORMAL_PORT       --- This is a normal port.
 *           SWCTRL_LPORT_TRUNK_PORT_MEMBER --- This port is a member of a trunk.
 * NOTE    : Output "ifindex" base on the table given by CLI before provision complete.
             Output "ifindex" normally after provision complete.
 *------------------------------------------------------------------------*/
SWCTRL_Lport_Type_T SWCTRL_POM_UIUserPortToIfindex (UI32_T unit, UI32_T port, UI32_T *ifindex, BOOL_T *is_inherit);

/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_UIUserPortToTrunkPort
 *------------------------------------------------------------------------
 * FUNCTION: This API is used to get trunk ID of some user port that
 *           and should only be used by  CLI.
 * INPUT   : unit --- which unit caller want to get.
 *           port --- which port caller want to get.
 * OUTPUT  : trunk_id ---
 * RETURN  : SWCTRL_LPORT_UNKNOWN_PORT      --- 1) Not in master mode.
 *                                              2) Argument is invalid.
 *                                              3) This port is not present.
 *           SWCTRL_LPORT_NORMAL_PORT       --- This is a normal port.
 *           SWCTRL_LPORT_TRUNK_PORT_MEMBER --- This port is a member of a trunk.
 * NOTE    : Process "unit" base on the table given by CLI before provision complete.
 *           Process "unit" normally after provision complete.
 *------------------------------------------------------------------------*/
SWCTRL_Lport_Type_T SWCTRL_POM_UIUserPortToTrunkPort(UI32_T unit, UI32_T port, UI32_T *trunk_id);


/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_UIUserPortExisting
 *------------------------------------------------------------------------
 * FUNCTION: This API is used to check if a user port exists or not, and
 *           should only be called by CLI.
 * INPUT   : unit --- which unit caller want to get.
 *           port --- which port caller want to get.
 * OUTPUT  : None.
 * RETURN  : TRUE  --- This user port exist.
 *           FALSE --- This user port does not exist.
 * NOTE    : Return TRUE/FALSE base on the table given by CLI before provision complete.
 *           Return TRUE/FALSE normally after provision complete.
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_POM_UIUserPortExisting (UI32_T unit, UI32_T port);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_GetRunningPortSfpDdmThresholdEntry
 * -------------------------------------------------------------------------
 * FUNCTION: Get running config of sfp ddm threshold
 * INPUT   : ifindex
 * OUTPUT  : sfp_ddm_threshold_entry_p
 * RETURN  : SYS_TYPE_Get_Running_Cfg_T
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T SWCTRL_POM_GetRunningPortSfpDdmThresholdEntry(UI32_T ifindex, SWCTRL_OM_SfpDdmThresholdEntry_T *sfp_ddm_threshold_entry_p);

#if (SYS_CPNT_COMBO_PORT_FORCE_MODE == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_GetSupportedPortComboForcedMode
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get supported port force link medium
 * INPUT   : ifindex       -- which port to set
 * OUTPUT  : None
 * RETURN  : Bitmap of VAL_portComboForcedMode_xxx
 * NOTE    : None
 * -------------------------------------------------------------------------*/
UI32_T SWCTRL_POM_GetSupportedPortComboForcedMode(UI32_T ifindex);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_GetDefaultPortComboForcedMode
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get default port force link medium
 * INPUT   : ifindex       -- which port to set
 * OUTPUT  : forcedmode_p  -- which mode of medium
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_POM_GetDefaultPortComboForcedMode(UI32_T ifindex, UI32_T *forcedmode_p);

#if (SYS_CPNT_COMBO_PORT_FORCED_MODE_SFP_SPEED == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_GetDefaultPortComboForcedModeSfpSpeed
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get default fiber medium speed
 * INPUT   : ifindex       -- which port to set
 * OUTPUT  : fiber_speed_p -- which mode of medium
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_POM_GetDefaultPortComboForcedModeSfpSpeed(UI32_T ifindex, UI32_T *fiber_speed_p);
#endif

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_GetPortComboForcedMode
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set port force link medium
 * INPUT   : ifindex       -- which port to set
 * OUTPUT  : forcedmode    -- which mode of medium
 *                      - VAL_portComboForcedMode_none	                1L
 *                      - VAL_portComboForcedMode_copperForced	        2L
 *                      - VAL_portComboForcedMode_copperPreferredAuto	3L (Obsoleted)
 *                      - VAL_portComboForcedMode_sfpForced	            4L
 *                      - VAL_portComboForcedMode_sfpPreferredAuto	    5L
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : For trunk and non-combo port, mode only read and is VAL_portComboForcedMode_none
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_POM_GetPortComboForcedMode(UI32_T ifindex, UI32_T *forcedmode);

/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_GetRunningPortComboForcedMode
 *------------------------------------------------------------------------
 * FUNCTION: This function will get the port combo force mode running config
 * INPUT   : ifindex       -- which port to set
 * OUTPUT  : forcedmode    -- which mode of medium
 *                      - VAL_portComboForcedMode_none	                1L
 *                      - VAL_portComboForcedMode_copperForced	        2L
 *                      - VAL_portComboForcedMode_copperPreferredAuto	3L (Obsoleted)
 *                      - VAL_portComboForcedMode_sfpForced	            4L
 *                      - VAL_portComboForcedMode_sfpPreferredAuto	    5L
 * RETURN  : SYS_TYPE_Get_Running_Cfg_T
 * NOTE    : None
 *------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T SWCTRL_POM_GetRunningPortComboForcedMode(UI32_T ifindex, UI32_T *forcedmode);

#if (SYS_CPNT_COMBO_PORT_FORCED_MODE_SFP_SPEED == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_GetPortComboForcedModeSpeed
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get active GBIC port type
 * INPUT   : ifindex         -- which port to get
 * OUTPUT  : fiber_speed     -- which speed of medium
 *                                 - VAL_portType_hundredBaseFX
 *                                 - VAL_portType_thousandBaseSfp
 * RETURN  : TRUE: Successfully, FALSE: Fail
 * NOTE    : 2007.03.15, aken, ES3528MO doesn't support I2C information
 *           function refences form SWCTRL_GetPortComboForcedMode()
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_POM_GetPortComboForcedModeSpeed(UI32_T ifindex, UI32_T *fiber_speed);

/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_GetRunningPortComboForcedModeSpeed
 *------------------------------------------------------------------------
 * FUNCTION: This function will get the port combo force mode running config
 * INPUT   : ifindex       -- which port to set
 * OUTPUT  : fiber_speed    -- which speed
 *                      - VAL_portType_hundredBaseFX
 *                      - VAL_portType_thousandBaseSfp
 * RETURN  : SYS_TYPE_Get_Running_Cfg_T
 * NOTE    : None
 *------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T SWCTRL_POM_GetRunningPortComboForcedModeSpeed(UI32_T ifindex, UI32_T *fiber_speed);
#endif

#endif

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_UserPortToLogicalPort
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get a logical port mapping from a user port
 * INPUT   : unit    -- which unit to map
 *           port    -- which port to map
 * OUTPUT  : ifindex -- the logical port
 * RETURN  : One of SWCTRL_Lport_Type_T
 * NOTE    : None
 * -------------------------------------------------------------------------*/
SWCTRL_Lport_Type_T SWCTRL_POM_UserPortToLogicalPort(UI32_T unit,
                                                 UI32_T port,
                                                 UI32_T *ifindex);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_UserPortToIfindex
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get the ifindex mapping from a user port
 * INPUT   : unit    -- which unit to map
 *           port    -- which port to map
 * OUTPUT  : ifindex -- the logical port
 * RETURN  : One of SWCTRL_Lport_Type_T
 * NOTE    : None
 * -------------------------------------------------------------------------*/
SWCTRL_Lport_Type_T SWCTRL_POM_UserPortToIfindex(UI32_T unit,
                                             UI32_T port,
                                             UI32_T *ifindex);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_UserPortToTrunkPort
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get a trunk port mapping from a user port
 * INPUT   : unit     -- which unit to map
 *           port     -- which port to map
 * OUTPUT  : trunk_id -- the logical port
 * RETURN  : One of SWCTRL_Lport_Type_T
 * NOTE    : None
 * -------------------------------------------------------------------------*/
SWCTRL_Lport_Type_T SWCTRL_POM_UserPortToTrunkPort(UI32_T unit,
                                               UI32_T port,
                                               UI32_T *trunk_id);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_LogicalPortToUserPort
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get a user port mapping from a logical port
 * INPUT   : ifindex  -- which port to map
 * OUTPUT  : unit     -- the unit
 *           port     -- the user port
 *           trunk_id -- trunk ID if it is a trunk port
 * RETURN  : One of SWCTRL_Lport_Type_T
 * NOTE    : None
 * -------------------------------------------------------------------------*/
SWCTRL_Lport_Type_T SWCTRL_POM_LogicalPortToUserPort(UI32_T ifindex,
                                                 UI32_T *unit,
                                                 UI32_T *port,
                                                 UI32_T *trunk_id);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_TrunkIDToLogicalPort
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get a logical port mapping from a trunk port
 * INPUT   : trunk_id -- which trunk port to map
 * OUTPUT  : ifindex  -- the logical port
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_POM_TrunkIDToLogicalPort(UI32_T trunk_id,
                                   UI32_T *ifindex);

/* -------------------------------------------------------------------------
 * ROUTINE NAME -  SWCTRL_POM_LportToActiveUport
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get the primary port from logical port
 * INPUT   : vid        -- The VLAN to ask, if the vid is SYS_TYPE_IGNORE_VID_CHECK
 *                         then don't check STA state
 *           l_port     -- the key to ask
 * OUTPUT  : *unit_port -- primary port (active)
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T  SWCTRL_POM_LportToActiveUport(UI32_T vid, UI32_T l_port, SYS_TYPE_Uport_T *unit_port);

/* -------------------------------------------------------------------------
 * FUNCTION NAME - SWCTRL_POM_GetPortMaxFrameSize
 * -------------------------------------------------------------------------
 * PURPOSE : to get max frame size of port
 * INPUT   : ifindex                 - ifindex
 * OUTPUT  : untagged_max_frame_sz_p - max frame size for untagged frames
 *           tagged_max_frame_sz_p   - max frame size for tagged frames
 * RETURN  : TRUE / FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SWCTRL_POM_GetPortMaxFrameSize(UI32_T ifindex, UI32_T *untagged_max_frame_sz_p, UI32_T *tagged_max_frame_sz_p);

#if (SYS_CPNT_SWCTRL_CABLE_DIAG == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_GetCableDiagResult
 * -------------------------------------------------------------------------
 * FUNCTION: Get Cable diag result of specific port by latest result
 * INPUT   : lport : Logical port num
 * OUTPUT  : result : result of the cable diag test for the port
 * RETURN  : TRUE/FALSE
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_POM_GetCableDiagResult(UI32_T lport, SWCTRL_Cable_Info_T *result);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_GetCableDiagResult
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get all cable diag result of ports
 * INPUT   : lport   -- the key to get
 * OUTPUT  : lport   -- the next existing port
 *           result -- all cable diag result of this port
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_POM_GetNextCableDiagResult(UI32_T *lport, SWCTRL_Cable_Info_T *result);

#endif    /* #if (SYS_CPNT_SWCTRL_CABLE_DIAG == TRUE) */

#if (SYS_CPNT_RATE_BASED_STORM_CONTROL == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_GetRateBasedStormControl
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get rate based storm control settings.
 * INPUT   : ifindex
 * OUTPUT  : rate
 *           mode
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : specifically for BCM53115
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_POM_GetRateBasedStormControl(UI32_T ifindex, UI32_T *rate, UI32_T *mode);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_GetNextRateBasedStormControl
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get rate based storm control settings.
 * INPUT   : ifindex
 * OUTPUT  : ifindex
 *           rate
 *           mode
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : specifically for BCM53115
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_POM_GetNextRateBasedStormControl(UI32_T *ifindex, UI32_T *rate, UI32_T *mode);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetRunningRateBasedStormControlRate
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get rate based storm control settings.
 * INPUT   : ifindex
 * OUTPUT  : ifindex
 *           rate
 *           mode
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : specifically for BCM53115
 * -------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T SWCTRL_POM_GetRunningRateBasedStormControlRate(UI32_T ifindex, UI32_T *rate);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetRunningRateBasedStormControlMode
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get rate based storm control settings.
 * INPUT   : ifindex
 * OUTPUT  : ifindex
 *           rate
 *           mode
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : specifically for BCM53115
 * -------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T SWCTRL_POM_GetRunningRateBasedStormControlMode(UI32_T ifindex, UI32_T *mode);


#endif

#if (SYS_CPNT_INTERNAL_LOOPBACK_TEST == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_GetInternalLoopbackTestResult
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get internal loopback test result
 * INPUT   : lport
 * OUTPUT  : lport
 *           result
 *           result_time
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_POM_GetInternalLoopbackTestResult(UI32_T lport, UI32_T *result, UI32_T *result_time);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_GetNextInternalLoopbackTestResult
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get rate based storm control settings.
 * INPUT   : lport
 * OUTPUT  : lport
 *           result
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_POM_GetNextInternalLoopbackTestResult(UI32_T *lport, UI32_T *result, UI32_T *result_time);
#endif

/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_IfindexToUport
 *------------------------------------------------------------------------
 * FUNCTION: This function will convert normal port ifindex to user port
 * INPUT   : ifindex - interface index
 * OUTPUT  : unit - unit id of user port
 *           port - port number of user port
 * RETURN  : TRUE  - convert sucessfully
 *           FALSE - something wrong
 * NOTE    : None
 *------------------------------------------------------------------------
 */
BOOL_T SWCTRL_POM_IfindexToUport(UI32_T ifindex, UI32_T *unit, UI32_T *port);

#if (SYS_CPNT_MAC_BASED_MIRROR == TRUE) || (SYS_CPNT_VLAN_MIRROR == TRUE) || (SYS_CPNT_ACL_MIRROR == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_OM_GetVlanAndMacMirrorDestPort
 * -------------------------------------------------------------------------
 * FUNCTION: This function will returns VLAN/Mac mirror destination port.
 * INPUT   : ifindex
 * OUTPUT  : ifindex      -- returns VLAN/Mac mirror destination port
 * RETURN  : None
 * Note    : ifindex 0 indicates NULL.
 * -------------------------------------------------------------------------*/
void SWCTRL_POM_GetVlanAndMacMirrorDestPort(UI32_T *ifindex);
#endif

#if (SYS_CPNT_MAC_BASED_MIRROR == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_GetExactMacAddrMirrorEntry
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get exactly mirror entry by MAC-address
 * INPUT   : addr_entry->mac_addr   - mac address
 * OUTPUT  : addr_entry->addr_entry_index
 *           addr_entry->mac_addr
 *           addr_entry->is_valid
 *
 * RETURN  : TRUE: success , FALSE: fail
 * NOTE    : We shall returns exactly mac_address entry and index
 *           addr_entry->addr_entry_index
 *           addr_entry->mac_addr
 *
 *           SYS_ADPT_MAX_NBR_OF_MAC_BASED_MIRROR_ENTRY --  this constant
 *           allow maximal mac based mirror entries
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_POM_GetExactMacAddrMirrorEntry(SWCTRL_MacAddrMirrorEntry_T *addr_entry);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_GetNextMacAddrMirrorEntry
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get next mac address of mirror entry
 * INPUT   : *addr_entry -- mac address entry
 * OUTPUT  : *addr_entry -- mac entry
 *           addr_entry->addr_entry_index
 *           addr_entry->mac_addr
 *           addr_entry->is_valid
 *
 * RETURN  : TRUE: success , FALSE: fail
 * NOTE    : if the addr_entry_index = 0
 *           , means returns first mac-address entry
 *           otherwise, we shall verify current index id and mac-address
 *
 *           SYS_ADPT_MAX_NBR_OF_MAC_BASED_MIRROR_ENTRY --  this constant
 *           allow maximal mac based mirror entries
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_POM_GetNextMacAddrMirrorEntry(SWCTRL_MacAddrMirrorEntry_T *addr_entry);
#endif /* end of #if (SYS_CPNT_MAC_BASED_MIRROR == TRUE) */

#if (SYS_CPNT_ITRI_MIM == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_ITRI_MIM_GetStatus
 * -------------------------------------------------------------------------
 * FUNCTION: Set status of ITRI MAC-in-MAC
 * INPUT   : ifindex
 * OUTPUT  : status_p
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_POM_ITRI_MIM_GetStatus(UI32_T ifindex, BOOL_T *status_p);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_ITRI_MIM_GetNextStatus
 * -------------------------------------------------------------------------
 * FUNCTION: Set status of ITRI MAC-in-MAC
 * INPUT   : ifindex_p
 * OUTPUT  : ifindex_p
 *           status_p
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_POM_ITRI_MIM_GetNextStatus(UI32_T *ifindex_p, BOOL_T *status_p);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_ITRI_MIM_GetRunningStatus
 * -------------------------------------------------------------------------
 * FUNCTION: Set status of ITRI MAC-in-MAC
 * INPUT   : ifindex
 * OUTPUT  : status_p
 * RETURN  : SYS_TYPE_Get_Running_Cfg_T
 * NOTE    : None
 * -------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T SWCTRL_POM_ITRI_MIM_GetRunningStatus(UI32_T ifindex, BOOL_T *status_p);
#endif /* (SYS_CPNT_ITRI_MIM == TRUE) */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_GetTrunkBalanceMode
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get the balance mode of trunking
 * INPUT   : None
 * OUTPUT  : balance_mode
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : balance_mode:
 *           SWCTRL_TRUNK_BALANCE_MODE_MAC_SA      Determinded by source mac address
 *           SWCTRL_TRUNK_BALANCE_MODE_MAC_DA      Determinded by destination mac address
 *           SWCTRL_TRUNK_BALANCE_MODE_MAC_SA_DA   Determinded by source and destination mac address
 *           SWCTRL_TRUNK_BALANCE_MODE_IP_SA       Determinded by source IP address
 *           SWCTRL_TRUNK_BALANCE_MODE_IP_DA       Determinded by destination IP address
 *           SWCTRL_TRUNK_BALANCE_MODE_IP_SA_DA    Determinded by source and destination IP address
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_POM_GetTrunkBalanceMode(UI32_T *balance_mode_p);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_GetRunningTrunkBalanceMode
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get the balance mode of trunking
 * INPUT   : None
 * OUTPUT  : balance_mode
 * RETURN  : SYS_TYPE_Get_Running_Cfg_T
 * NOTE    : balance_mode:
 *           SWCTRL_TRUNK_BALANCE_MODE_MAC_SA      Determinded by source mac address
 *           SWCTRL_TRUNK_BALANCE_MODE_MAC_DA      Determinded by destination mac address
 *           SWCTRL_TRUNK_BALANCE_MODE_MAC_SA_DA   Determinded by source and destination mac address
 *           SWCTRL_TRUNK_BALANCE_MODE_IP_SA       Determinded by source IP address
 *           SWCTRL_TRUNK_BALANCE_MODE_IP_DA       Determinded by destination IP address
 *           SWCTRL_TRUNK_BALANCE_MODE_IP_SA_DA    Determinded by source and destination IP address
 * -------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T SWCTRL_POM_GetRunningTrunkBalanceMode(UI32_T *balance_mode_p);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_GetTrunkMaxNumOfActivePorts
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set max number of active ports of trunk
 * INPUT   : trunk_id
 * OUTPUT  : max_num_of_active_ports_p
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_POM_GetTrunkMaxNumOfActivePorts(UI32_T trunk_id, UI32_T *max_num_of_active_ports_p);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_GetRunningTrunkMaxNumOfActivePorts
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set max number of active ports of trunk
 * INPUT   : trunk_id
 * OUTPUT  : max_num_of_active_ports_p
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T SWCTRL_POM_GetRunningTrunkMaxNumOfActivePorts(UI32_T trunk_id, UI32_T *max_num_of_active_ports_p);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_GetActiveTrunkMember
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get trunk members which are active
 * INPUT   : trunk_ifindex      -- which interface index
 * OUTPUT  : active_lportarray   -- the active trunk member port array
 *           active_lport_count -- the number of active trunk member
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_POM_GetActiveTrunkMember(
        UI32_T        trunk_ifindex,
        UI32_T         active_lportarray[SYS_ADPT_MAX_NBR_OF_PORT_PER_TRUNK],
        UI32_T        *active_lport_count);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SWCTRL_POM_GetPortLearningStatusEx
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion will get port learning status
 * INPUT    :   ifindex
 *              learning_disabled_status_p
 *              intruder_handlers_p
 * OUTPUT   :   None
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None
 * ------------------------------------------------------------------------
 */
BOOL_T SWCTRL_POM_GetPortLearningStatusEx(UI32_T ifindex, UI32_T *learning_disabled_status_p, UI32_T *intruder_handlers_p);

#if (SYS_CPNT_PFC == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_isPortFlowControlEnabled
 * -------------------------------------------------------------------------
 * FUNCTION: This function will check the port is flow control enabled or not
 * INPUT   : ifindex   -- which port to get
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_POM_isPortFlowControlEnabled(UI32_T ifindex);

#endif /* #if (SYS_CPNT_PFC == TRUE) */

/* -------------------------------------------------------------------------
 * FUNCTION NAME - SWCTRL_POM_GetCpuRateLimit
 * -------------------------------------------------------------------------
 * PURPOSE : To get configured CPU rate limit
 * INPUT   : pkt_type  -- SWDRV_PKTTYPE_XXX
 * OUTPUT  : rate_p    -- in pkt/s. 0 to disable.
 * RETURN  : TRUE / FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SWCTRL_POM_GetCpuRateLimit(UI32_T pkt_type, UI32_T *rate_p);

/* -------------------------------------------------------------------------
 * FUNCTION NAME - SWCTRL_POM_GetPortAbility
 * -------------------------------------------------------------------------
 * PURPOSE : To get port abilities
 * INPUT   : ifindex
 * OUTPUT  : ability_p  -- SWCTRL_PortAbility_T
 * RETURN  : TRUE / FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SWCTRL_POM_GetPortAbility(UI32_T ifindex, SWCTRL_PortAbility_T *ability_p);

#if (SYS_CPNT_SWCTRL_GLOBAL_STORM_SAMPLE_TYPE == TRUE)
/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_GetRunningGlobalStormSampleType
 *------------------------------------------------------------------------
 * FUNCTION: This function will get running config of global storm sample type
 * INPUT   : None
 * OUTPUT  : global_storm_sample_type
 * RETURN  : SYS_TYPE_Get_Running_Cfg_T
 * NOTE    : None
 *------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T SWCTRL_POM_GetRunningGlobalStormSampleType(UI32_T *global_storm_sample_type_p);
#endif /* (SYS_CPNT_SWCTRL_GLOBAL_STORM_SAMPLE_TYPE == TRUE) */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_GetPortSfpPresent
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get sfp present status of a port
 * INPUT   : unit
 *           sfp_index
 * OUTPUT  : is_present -- present or not
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_POM_GetPortSfpPresent(UI32_T unit, UI32_T sfp_index, BOOL_T *is_present_p);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_GetPortSfpInfo
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get all information of a port
 * INPUT   : unit
 *           sfp_index
 * OUTPUT  : sfp_info_p -- sfp info read from i2c
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_POM_GetPortSfpInfo(UI32_T unit, UI32_T sfp_index, SWCTRL_OM_SfpInfo_T *sfp_info_p);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_GetPortSfpDdmInfo
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get sfp ddm info of a port
 * INPUT   : unit
 *           sfp_index
 * OUTPUT  : sfp_ddm_info_p -- sfp ddm info read from i2c
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SWCTRL_POM_GetPortSfpDdmInfo(UI32_T unit, UI32_T sfp_index, SWCTRL_OM_SfpDdmInfo_T *sfp_ddm_info_p);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_GetPortSfpDdmInfoMeasured
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get sfp ddm measured info of a port
 * INPUT   : unit
 *           sfp_index
 * OUTPUT  : sfp_ddm_info_measured_p -- sfp ddm measured info
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SWCTRL_POM_GetPortSfpDdmInfoMeasured(UI32_T unit, UI32_T sfp_index, SWCTRL_OM_SfpDdmInfoMeasured_T *sfp_ddm_info_measured_p);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_GetPortSfpDdmThreshold
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get sfp ddm threshold of a port
 * INPUT   : lport
 * OUTPUT  : sfp_ddm_threshold_p -- sfp ddm threshold
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SWCTRL_POM_GetPortSfpDdmThreshold(UI32_T lport, SWCTRL_OM_SfpDdmThreshold_T *sfp_ddm_threshold_p);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_GetPortSfpDdmThresholdEntry
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get sfp ddm threshold, auto mode, and trap
 *           enable of a port
 * INPUT   : lport
 * OUTPUT  : sfp_ddm_threshold_entry_p
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SWCTRL_POM_GetPortSfpDdmThresholdEntry(UI32_T lport, SWCTRL_OM_SfpDdmThresholdEntry_T *sfp_ddm_threshold_entry_p);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_GetNextPortSfpDdmThresholdEntry
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get sfp ddm threshold, auto mode, and trap
 *           enable of a port
 * INPUT   : lport
 * OUTPUT  : sfp_ddm_threshold_entry_p
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------
 */
BOOL_T SWCTRL_POM_GetNextPortSfpDdmThresholdEntry(UI32_T *lport_p, SWCTRL_OM_SfpDdmThresholdEntry_T *sfp_ddm_threshold_entry_p);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_GetSfpEntry
 * -------------------------------------------------------------------------
 * FUNCTION: Get sfp info result of specific port
 * INPUT   : lport : Logical port num
 * OUTPUT  : sfp_entry_p : result of the sfp info for the port
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------
 */
BOOL_T SWCTRL_POM_GetPortSfpEntry(UI32_T lport, SWCTRL_OM_SfpEntry_T *sfp_entry_p);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_GetNextPortSfpEntry
 * -------------------------------------------------------------------------
 * FUNCTION: Get sfp info result of specific port
 * INPUT   : lport_p : Logical port num
 * OUTPUT  : sfp_entry_p : result of the sfp info for the port
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------
 */
BOOL_T SWCTRL_POM_GetNextPortSfpEntry(UI32_T *lport_p, SWCTRL_OM_SfpEntry_T *sfp_entry_p);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_GetPortSfpDdmEntry
 * -------------------------------------------------------------------------
 * FUNCTION: Get sfp info result of specific port
 * INPUT   : lport : Logical port num
 * OUTPUT  : sfp_ddm_entry_p : result of the sfp info for the port
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------
 */
BOOL_T SWCTRL_POM_GetPortSfpDdmEntry(UI32_T lport, SWCTRL_OM_SfpDdmEntry_T *sfp_ddm_entry_p);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_GetNextPortSfpDdmEntry
 * -------------------------------------------------------------------------
 * FUNCTION: Get sfp info result of specific port
 * INPUT   : lport_p : Logical port num
 * OUTPUT  : sfp_ddm_entry_p : result of the sfp info for the port
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------
 */
BOOL_T SWCTRL_POM_GetNextPortSfpDdmEntry(UI32_T *lport_p, SWCTRL_OM_SfpDdmEntry_T *sfp_ddm_entry_p);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_GetPortSfpDdmThresholdAutoMode
 * -------------------------------------------------------------------------
 * FUNCTION: Get status of ddm threshold auto mode
 * INPUT   : ifindex
 * OUTPUT  : auto_mode_p
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SWCTRL_POM_GetPortSfpDdmThresholdAutoMode(UI32_T ifindex, BOOL_T *auto_mode_p);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_GetPortSfpDdmThresholdTrapEnable
 * -------------------------------------------------------------------------
 * FUNCTION: Get status of ddm threshold trap
 * INPUT   : ifindex
 * OUTPUT  : trap_enable_p
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SWCTRL_POM_GetPortSfpDdmTrapEnable(UI32_T ifindex, BOOL_T *trap_enable_p);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_GetPortSfpDdmThresholdStatus
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get sfp ddm threshold of a port
 * INPUT   : lport
 * OUTPUT  : sfp_ddm_threshold_status_p -- sfp ddm threshold status
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SWCTRL_POM_GetPortSfpDdmThresholdStatus(UI32_T lport, SWCTRL_OM_SfpDdmThresholdStatus_T *sfp_ddm_threshold_status_p);

#if (SYS_CPNT_HASH_SELECTION == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_GetHashBlockInfo
 * -------------------------------------------------------------------------
 * FUNCTION: get hash-selection block info
 * INPUT   : list_index - the index of hash-selection list
 * OUTPUT  : block_info_p - the hash-selection block info
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_POM_GetHashBlockInfo(
    UI8_T list_index , 
    SWCTRL_OM_HashSelBlockInfo_T *block_info_p);
#endif /*#if (SYS_CPNT_HASH_SELECTION == TRUE)*/

#if(SYS_CPNT_WRED == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_POM_GetRandomDetect
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get the random detect configuration
 * INPUT   : lport   -- which port to get
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_POM_GetRandomDetect(UI32_T lport, SWCTRL_OM_RandomDetect_T *value_p);
#endif
#endif /* #ifndef SWCTRL_POM_H */

