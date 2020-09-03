/* MODULE NAME:  swctrl_group.h
 * PURPOSE:
 *     This file is a demonstration code for implementations of swctrl group.
 *
 * NOTES:
 *
 * HISTORY
 *    6/12/2007 - Eli Lin, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */

#ifndef SWCTRL_GROUP_H
#define SWCTRL_GROUP_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "swctrl.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/*------------------------------------------------------------------------------
 * ROUTINE NAME : SWCTRL_GROUP_Create_All_Threads
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will spawn all threads in SWCTRL_Group.
 *
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
 *    All threads in the same SWCTRL group will join the same thread group.
 *------------------------------------------------------------------------------
 */
void SWCTRL_GROUP_Create_All_Threads(void);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SWCTRL_GROUP_InitiateProcessResource
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will init  SWCTRL Group.
 *
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
 *------------------------------------------------------------------------------
 */
void SWCTRL_GROUP_InitiateProcessResource(void);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SWCTRL_GROUP_Create_InterCSC_Relation
 *-----------------------------------------------------------------------------
 * PURPOSE : Create inter-CSC relationships for SWCTRL Group.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
void SWCTRL_GROUP_Create_InterCSC_Relation(void);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GROUP_LPortTypeChangedCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when a hot swap module is removed.
 * INPUT   : ifindex   -- which logical port.
 *           port_type -- changed to which port type.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_GROUP_LPortTypeChangedCallbackHandler(
    UI32_T ifindex, UI32_T port_type);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GROUP_UPortTypeChangedCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when a hot swap module is removed.
 * INPUT   : ifindex   -- which logical port.
 *           port_type -- changed to which port type.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_GROUP_UPortTypeChangedCallbackHandler(
    UI32_T unit, UI32_T port, UI32_T port_type);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GROUP_TrunkMemberAdd1stCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when a logical port is added to a
 *           trunk
 * INPUT   : trunk_ifindex  -- which trunk port
 *           member_ifindex -- which member port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_GROUP_TrunkMemberAdd1stCallbackHandler(
    UI32_T trunk_ifindex,UI32_T member_ifindex);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GROUP_TrunkMemberAddCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when a logical port is added to a
 *           trunk
 * INPUT   : trunk_ifindex  -- which trunk port
 *           member_ifindex -- which member port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_GROUP_TrunkMemberAddCallbackHandler(
    UI32_T trunk_ifindex,UI32_T member_ifindex);

 /* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GROUP_TrunkMemberDeleteCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when a logical port is deleted from a
 *           trunk
 * INPUT   : trunk_ifindex  -- which trunk port
 *           member_ifindex -- which member port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_GROUP_TrunkMemberDeleteCallbackHandler(
    UI32_T trunk_ifindex,UI32_T member_ifindex);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GROUP_TrunkMemberDeleteLstCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when a logical port is deleted from a
 *           trunk
 * INPUT   : trunk_ifindex  -- which trunk port
 *           member_ifindex -- which member port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_GROUP_TrunkMemberDeleteLstCallbackHandler(
    UI32_T trunk_ifindex,UI32_T member_ifindex) ;

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GROUP_PortLinkUpCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when the link is up
 * INPUT   : ifindex -- which logical port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_GROUP_PortLinkUpCallbackHandler(
    UI32_T ifindex);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GROUP_uPortLinkUpCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when the link is up
 * INPUT   : unit -- in which unit
 *           port -- which logical port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_GROUP_uPortLinkUpCallbackHandler(
    UI32_T unit,UI32_T port);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GROUP_uPortFastLinkUpCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when the link is up
 * INPUT   : unit -- in which unit
 *           port -- which logical port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_GROUP_uPortFastLinkUpCallbackHandler(
    UI32_T unit,UI32_T port);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GROUP_PortLinkDownCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when the link is down
 * INPUT   : ifindex -- which logical port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_GROUP_PortLinkDownCallbackHandler(
    UI32_T ifindex);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GROUP_uPortLinkDownCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when the link is down
 * INPUT   : unit -- in which unit
 *           port -- which logical port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_GROUP_uPortLinkDownCallbackHandler(
    UI32_T unit,UI32_T port);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GROUP_uPortFastLinkDownCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when the link is down
 * INPUT   : unit -- in which unit
 *           port -- which logical port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_GROUP_uPortFastLinkDownCallbackHandler(
    UI32_T unit,UI32_T port);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GROUP_PortOperUpCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when the oper status is up
 * INPUT   : ifindex -- which logical port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_GROUP_PortOperUpCallbackHandler(
    UI32_T ifindex);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GROUP_PortNotOperUpCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when the oper status is not up
 * INPUT   : ifindex -- which logical port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_GROUP_PortNotOperUpCallbackHandler(
    UI32_T ifindex);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GROUP_TrunkMemberPortOperUpCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when the oper status is up
 * INPUT   : ifindex -- which logical port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_GROUP_TrunkMemberPortOperUpCallbackHandler(
    UI32_T trunk_ifindex,UI32_T trunk_member_port_ifindex);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GROUP_TrunkMemberPortNotOperUpCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when the oper status is down
 * INPUT   : ifindex -- which logical port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_GROUP_TrunkMemberPortNotOperUpCallbackHandler(
    UI32_T trunk_ifindex,UI32_T trunk_member_port_ifindex);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GROUP_TrunkMemberActiveCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when member port is active
 * INPUT   : ifindex -- which logical port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_GROUP_TrunkMemberActiveCallbackHandler(
    UI32_T trunk_ifindex,UI32_T trunk_member_port_ifindex);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GROUP_TrunkMemberInactiveCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when member port is inactive
 * INPUT   : ifindex -- which logical port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_GROUP_TrunkMemberInactiveCallbackHandler(
    UI32_T trunk_ifindex,UI32_T trunk_member_port_ifindex);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GROUP_PortAdminEnableCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when the port is enabled
 * INPUT   : ifindex -- which logical port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_GROUP_PortAdminEnableCallbackHandler(
    UI32_T ifindex);

 /* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_Notify_PortAdminDisable
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when the port is disabled
 * INPUT   : ifindex -- which logical port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_GROUP_PortAdminDisableCallbackHandler (
    UI32_T ifindex);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_Notify_uPortAdminEnable
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when the port is enabled
 * INPUT   : unit -- in which unit
 *           port -- which logical port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_GROUP_uPortAdminEnableCallbackHandler(
    UI32_T unit,UI32_T port);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GROUP_PortAdminDisableBeforeCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when the port is disabled
 * INPUT   : ifindex -- which logical port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : for notify LLDP before doing shutdown port
 * -------------------------------------------------------------------------*/
void SWCTRL_GROUP_PortAdminDisableBeforeCallbackHandler(
    UI32_T ifindex);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GROUP_uPortAdminDisableCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when the port is disabled
 * INPUT   : unit -- in which unit
 *           port -- which logical port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_GROUP_uPortAdminDisableCallbackHandler(
    UI32_T unit,UI32_T port);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GROUP_PortStatusChangedPassivelyCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when the port status is changed passively
 * INPUT   : ifindex -- which logical port
 *           status
 *           changed_bmp
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_GROUP_PortStatusChangedPassivelyCallbackHandler(UI32_T ifindex, BOOL_T status, UI32_T changed_bmp);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GROUP_PortSpeedDuplexCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when the speed/duplex status of a port
 *           is changed
 * INPUT   : ifindex -- which logical port
 *           speed_duplex -- new status of speed/duplex
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_GROUP_PortSpeedDuplexCallbackHandler(
    UI32_T ifindex,UI32_T speed_duplex);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GROUP_uPortSpeedDuplexCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when the speed/duplex status of a port
 *           is changed
 * INPUT   : unit -- in which unit
 *           port -- which logical port
 *           speed_duplex -- new status of speed/duplex
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_GROUP_uPortSpeedDuplexCallbackHandler(
    UI32_T unit,UI32_T port,UI32_T speed_duplex);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GROUP_uPortLacpEffectiveOperStatusChangedCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when the effective oper status change
 *           for LACP.
 *           is changed
 * INPUT   : unit           --- which unit
 *           port           --- which port
 *           pre_status     --- status before change
 *           current_status --- status after change
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_GROUP_uPortLacpEffectiveOperStatusChangedCallbackHandler(
    UI32_T unit,UI32_T port,UI32_T pre_status, UI32_T current_status );

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GROUP_uPortDot1xEffectiveOperStatusChangedCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when the effective oper status change
 *           for DOT1x.
 * INPUT   : unit           --- which unit
 *           port           --- which port
 *           pre_status     --- status before change
 *           current_status --- status after change
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_GROUP_uPortDot1xEffectiveOperStatusChangedCallbackHandler(
    UI32_T unit, UI32_T port,UI32_T pre_status, UI32_T current_status );

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GROUP_PortEffectiveOperStatusChangedCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when the effective oper status change.
 * INPUT   : ifindex        --- which ifindex
 *           pre_status     --- status before change
 *           current_status --- status after change
 *           level          --- see SWCTRL_OperDormantLevel_T
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : When to callback:
 *           1) Oper status becomes effecitve.
 *              Oper status is changed from lower status to specified dormant
 *              status.
 *           2) Oper status becomes ineffecitve.
 *              Oper status is changed from specified dormant status or upper
 *              status to lower status.
 * -------------------------------------------------------------------------*/
void SWCTRL_GROUP_PortEffectiveOperStatusChangedCallbackHandler(
    UI32_T ifindex,
    UI32_T pre_status,
    UI32_T current_status,
    SWCTRL_OperDormantLevel_T level);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GROUP_ForwardingUPortAddToTrunkCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when forwarding uport added to trunk.
 * INPUT   : trunk_ifindex  - Which trunk.
 *           member_ifindex - Which trunk member.
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTE    : None.
 * -------------------------------------------------------------------------*/
void SWCTRL_GROUP_ForwardingUPortAddToTrunkCallbackHandler(
    UI32_T trunk_ifindex, UI32_T member_ifindex);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GROUP_ForwardingTrunkMemberDeleteCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when forwarding trunk member deleted.
 * INPUT   : trunk_ifindex  - Which trunk.
 *           member_ifindex - Which trunk member.
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTE    : None.
 * -------------------------------------------------------------------------*/
void SWCTRL_GROUP_ForwardingTrunkMemberDeleteCallbackHandler(
    UI32_T trunk_ifindex, UI32_T member_ifindex);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GROUP_ForwardingTrunkMemberToNonForwardingCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when forwarding trunk member become non-forwarding.
 * INPUT   : trunk_ifindex  - Which trunk.
 *           member_ifindex - Which trunk member.
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTE    : None.
 * -------------------------------------------------------------------------*/
void SWCTRL_GROUP_ForwardingTrunkMemberToNonForwardingCallbackHandler(
    UI32_T trunk_ifindex, UI32_T member_ifindex);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GROUP_PortLearningStatusChangedCallbackHandler
 * -------------------------------------------------------------------------
 * FUNCTION: Call call-back function, when learning status changed.
 * INPUT   : lport
 *           learning
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTE    : None.
 * -------------------------------------------------------------------------*/
void SWCTRL_GROUP_PortLearningStatusChangedCallbackHandler(
    UI32_T lport, BOOL_T learning);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SWCTRL_GROUP_AddStaticTrunkMemberCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Call call-back function, when UIs add static trunk member.
 *
 * INPUT   : trunk_ifindex --- Trunk member is deleted from which trunk
 *           tm_ifindex    --- Which trunk member is deleted from trunk
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
void SWCTRL_GROUP_AddStaticTrunkMemberCallbackHandler(UI32_T vid, UI32_T ifindex);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SWCTRL_GROUP_DelStaticTrunkMemberCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Call call-back function, when UIs delete static trunk member.
 *
 * INPUT   : trunk_ifindex --- Trunk member is deleted from which trunk
 *           tm_ifindex    --- Which trunk member is deleted from trunk
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
void SWCTRL_GROUP_DelStaticTrunkMemberCallbackHandler(UI32_T vid, UI32_T ifindex);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SWCTRL_GROUP_VlanActiveToSuspendCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Call call-back function when VLAN state is changed from active to
 *           suspended.
 *
 * INPUT   : vid_ifindex - vlan ifindex whose state is changed
 *           vlan_status - vlan status which changes the state of the VLAN
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
void SWCTRL_GROUP_VlanActiveToSuspendCallbackHandler(UI32_T vid_ifindex, UI32_T vlan_status);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SWCTRL_GROUP_VlanSuspendToActiveCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Call call-back function when VLAN state is changed from suspended
 *           to active.
 *
 * INPUT   : vid_ifindex - vlan ifindex whose state is changed
 *           vlan_status - vlan status which changes the state of the VLAN
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
void SWCTRL_GROUP_VlanSuspendToActiveCallbackHandler(UI32_T vid_ifindex, UI32_T vlan_status);

#if (SYS_CPNT_INTERNAL_LOOPBACK_TEST == TRUE)
/*------------------------------------------------------------------------------
 * ROUTINE NAME : SWCTRL_GROUP_StartLoopbackTimeoutTimer
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will be call to start the loopback timout timer.
 *
 * INPUT:
 *    None
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
BOOL_T SWCTRL_GROUP_StartLoopbackTimeoutTimer();

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SWCTRL_GROUP_StopLoopbackTimeoutTimer
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will be call to stop the loopback timout timer.
 *
 * INPUT:
 *    None
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
BOOL_T SWCTRL_GROUP_StopLoopbackTimeoutTimer();

/*added by Jinhua Wei,in order to remvoe warning,the two function didn't declared in the head file*/
/*------------------------------------------------------------------------------
 * ROUTINE NAME : SWCTRL_GROUP_StartInternalLoopbackTimeoutTimer
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will be call to start the loopback timout timer.
 *
 * INPUT:
 *    None
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
BOOL_T SWCTRL_GROUP_StartInternalLoopbackTimeoutTimer();
/*------------------------------------------------------------------------------
 * ROUTINE NAME : SWCTRL_GROUP_StopInternalLoopbackTimeoutTimer
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will be call to stop the loopback timout timer.
 *
 * INPUT:
 *    None
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
BOOL_T SWCTRL_GROUP_StopInternalLoopbackTimeoutTimer();

#endif

#endif    /* End of SWCTRL_GROUP_H */
