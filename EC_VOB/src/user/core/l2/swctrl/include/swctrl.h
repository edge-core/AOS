/* Module Name: SWCTRL.H
 * Purpose:
 *        ( 1. Whole module function and scope.                 )
 *         This file provides the hardware independent interface of switch
 *         control functions to applications.
 *        ( 2.  The domain MUST be handled by this module.      )
 *         This module includes port configuration, VLAN, port mirroring,
 *         trunking, spanning tree, IGMP, broadcast storm control, and
 *         port mapping.
 *        ( 3.  The domain would not be handled by this module. )
 *         But this module doesn't include MAC address manipulation and
 *         port statistics.
 * Notes:
 *        ( Something must be known or noticed by developer     )
 * History:
 *       Date        Modifier    Reason
 *       2001/6/1    Jimmy Lin   Create this file
 *
 * Copyright(C)      Accton Corporation, 1999, 2000
 */
/* NOTES:
 * 1. ES3626A MIB contains the following group:
 *    a) switchMgt      { es3626aMIBObjects 1 } -- implemented in sys_mgr
 *    b) portMgt        { es3626aMIBObjects 2 } -- implemented here
 *    c) trunkMgt       { es3626aMIBObjects 3 } -- implemented in trk_mgr
 *    d) staMgt         { es3626aMIBObjects 4 } -- implemented in sta_mgr
 *    e) tftpMgt        { es3626aMIBObjects 5 } -- not implemented ??
 *    f) restartMgt     { es3626aMIBObjects 6 } -- not implemented ??
 *    g) mirrorMgt      { es3626aMIBObjects 7 } -- implemented here
 *    h) igmpSnoopMgt   { es3626aMIBObjects 8 } -- implemented in igmp_mgr
 *    i) ipMgt          { es3626aMIBObjects 9 } -- not implemented ??
 *    j) bcastStormMgt  ( es3626aMIBObjects 10 }-- implemented here
 *    k) vlanMgt        { es3626aMIBObjects 11 }-- not implemented ??
 */
#ifndef _SWCTRL_H_
#define _SWCTRL_H_

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_dflt.h"
#include "sys_type.h"
#include "sys_adpt.h"
#include "sys_cpnt.h"
#include "sysfun.h"
//#include "swdrv.h"
#include "leaf_2863.h"
#include "swctrl_task.h"
#include "leaf_es3626a.h"
#include "swctrl_mim_type.h"
#include "swctrl_om.h"
#include "swdrv_type.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
typedef enum
{
    SWCTRL_LINKSCAN_E_ERROR,
    SWCTRL_LINKSCAN_E_NO_DIRTY,
    SWCTRL_LINKSCAN_E_DIRTY,
    SWCTRL_LINKSCAN_E_PENDING_DIRTY,
} SWCTRL_LinkScanReturnCode_T;

/* auto traffic control Broadcast Storm Entry
 */
#if (SYS_CPNT_ATC_BSTORM == TRUE)
typedef struct
{
    /* key */
    UI32_T atc_broadcast_storm_ifindex;

    UI32_T atc_broadcast_storm_status;                          /* enabled | disabled , default value = disabled */
    UI32_T atc_broadcast_storm_sample_type;                     /* packet , octet , percent , default value = disabled */
    UI32_T atc_broadcast_storm_packet_rate;
    UI32_T atc_broadcast_storm_octet_rate;
    UI32_T atc_broadcast_storm_percent;
    UI32_T atc_broadcast_storm_current_traffic_rate;

    UI32_T atc_broadcast_storm_auto_traffic_control_on;         /* enabled | disabled , default value = disabled */
    UI32_T atc_broadcast_storm_auto_traffic_control_release;    /* enabled | disabled , default value = disabled */
    UI32_T atc_broadcast_storm_traffic_control_on;              /* enabled | disabled , default value = disabled */
    UI32_T atc_broadcast_storm_traffic_control_release;         /* enabled | disabled , default value = disabled */
    UI32_T atc_broadcast_storm_storm_alarm_threshold;           /* 1-255(K pps) , default vaule = 128 */
    UI32_T atc_broadcast_storm_storm_clear_threshold;           /* 1-255(K pps) , default value = 128 */
    UI32_T atc_broadcast_storm_trap_storm_alarm;                /* enabled | disabled , default value = disabled */
    UI32_T atc_broadcast_storm_trap_storm_clear;                /* enabled | disabled , default value = disabled */
    UI32_T atc_broadcast_storm_trap_traffic_control_on;                 /* enabled | disabled , default value = disabled */
    UI32_T atc_broadcast_storm_trap_traffic_control_release;            /* enabled | disabled , default value = disabled */
    UI32_T atc_broadcast_storm_action;                          /* rate-control | shutdown , default = rate-control */

    /* The operation status is up or down. It is only used on shutdown action. VAL_ifOperStatus_up | VAL_ifOperStatus_down, default value = VAL_ifOperStatus_up */
    UI32_T atc_broadcast_storm_operation_status;

    UI32_T state_machine_state;
    UI32_T state_machine_trapf;
    UI32_T state_machine_ctrapf;
    UI32_T state_machine_t1;
    UI32_T state_machine_t2;
    UI32_T state_machine_manual; /* enabled: manual release, disabled: auto release. select: enabled | disabled.*/
    UI32_T state_machine_action; /* The state of the action must be stable after traffic on. select: rate-control | shutdown. */
}SWCTRL_ATCBroadcastStormEntry_T;

typedef struct
{
    UI32_T atc_broadcast_storm_traffic_control_on_timer;        /* 1-300(sec) , default value = 300 */
    UI32_T atc_broadcast_storm_traffic_control_release_timer;   /* 1-900(sec) , default value = 900 */
}SWCTRL_ATCBroadcastStormTimer_T;
#endif

/* auto traffic control Broadcast Storm Entry
 */
#if (SYS_CPNT_ATC_MSTORM == TRUE)
typedef struct
{
    /* key */
    UI32_T atc_multicast_storm_ifindex;

    UI32_T atc_multicast_storm_status;                          /* enabled | disabled , default value = disabled */
    UI32_T atc_multicast_storm_sample_type;                     /* packet , octet , percent , default value = disabled */
    UI32_T atc_multicast_storm_packet_rate;
    UI32_T atc_multicast_storm_octet_rate;
    UI32_T atc_multicast_storm_percent;
    UI32_T atc_multicast_storm_current_traffic_rate;

    UI32_T atc_multicast_storm_auto_traffic_control_on;         /* enabled | disabled , default value = disabled */
    UI32_T atc_multicast_storm_auto_traffic_control_release;    /* enabled | disabled , default value = disabled */
    UI32_T atc_multicast_storm_traffic_control_on;              /* enabled | disabled , default value = disabled */
    UI32_T atc_multicast_storm_traffic_control_release;         /* enabled | disabled , default value = disabled */
    UI32_T atc_multicast_storm_storm_alarm_threshold;           /* 1-255(K pps) , default vaule = 128 */
    UI32_T atc_multicast_storm_storm_clear_threshold;           /* 1-255(K pps) , default value = 128 */
    UI32_T atc_multicast_storm_trap_storm_alarm;                /* enabled | disabled , default value = disabled */
    UI32_T atc_multicast_storm_trap_storm_clear;                /* enabled | disabled , default value = disabled */
    UI32_T atc_multicast_storm_trap_traffic_control_on;                 /* enabled | disabled , default value = disabled */
    UI32_T atc_multicast_storm_trap_traffic_control_release;            /* enabled | disabled , default value = disabled */
    UI32_T atc_multicast_storm_action;                          /* rate-control | shutdown , default = rate-control */

    /* The operation status is up or down. It is only used on shutdown action. VAL_ifOperStatus_up | VAL_ifOperStatus_down, default value = VAL_ifOperStatus_up */
    UI32_T atc_multicast_storm_operation_status;

    UI32_T state_machine_state;
    UI32_T state_machine_trapf;
    UI32_T state_machine_ctrapf;
    UI32_T state_machine_t1;
    UI32_T state_machine_t2;
    UI32_T state_machine_manual; /* enabled: manual release, disabled: auto release. select: enabled | disabled.*/
    UI32_T state_machine_action; /* The state of the action must be stable after traffic on. select: rate-control | shutdown. */
}SWCTRL_ATCMulticastStormEntry_T;

typedef struct
{
    UI32_T atc_multicast_storm_traffic_control_on_timer;        /* 1-300(sec) , default value = 300 */
    UI32_T atc_multicast_storm_traffic_control_release_timer;   /* 1-900(sec) , default value = 900 */
}SWCTRL_ATCMulticastStormTimer_T;
#endif

typedef struct
{
    UI32_T port_pd_ifindex;
    UI8_T  port_pd_status;  /* power source status */
    UI8_T  port_pd_mode;  /* at/af mode */
}SWCTRL_PortPD_T;

typedef enum SWCTRL_Port_Media_Type_E
{
    SWCTRL_PORT_MEDIA_TYPE_COPPER =0,
    SWCTRL_PORT_MEDIA_TYPE_FIBER,
    SWCTRL_PORT_MEDIA_TYPE_UNKNOWN
} SWCTRL_Port_Media_Type_T;

#if (SYS_CPNT_ATC_MSTORM == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetATCMulticastStormAutoTrafficControlOnStatus
 * -------------------------------------------------------------------------
 * 1.
 * FUNCTION: This function will set the auto traffic control on status of multicast
 *           storm control function
 * INPUT   : ifindex -- which port to set
 *           auto_traffic_control_on_status    -- which status of auto traffic control on
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetATCMulticastStormAutoTrafficControlOnStatus(UI32_T ifindex, UI32_T auto_traffic_control_on_status);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetATCMulticastStormAutoTrafficControlOnStatus
 * -------------------------------------------------------------------------
 * 2.
 * FUNCTION: This function will get the auto traffic control on status of multicast
 *           storm control function
 * INPUT   : ifindex -- which port to set
 *
 * OUTPUT  : auto_traffic_control_on_status    -- which status of auto traffic control on
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_GetATCMulticastStormAutoTrafficControlOnStatus(UI32_T ifindex, UI32_T *auto_traffic_control_on_status);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetATCMulticastStormAutoTrafficControlReleaseStatus
 * -------------------------------------------------------------------------
 * 3.
 * FUNCTION: This function will set the auto traffic control release status of multicast
 *           storm control function
 * INPUT   : ifindex -- which port to set
 *           auto_traffic_control_release_status    -- which status of auto traffic control release
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetATCMulticastStormAutoTrafficControlReleaseStatus(UI32_T ifindex, UI32_T auto_traffic_control_release_status);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetATCMulticastStormAutoTrafficControlReleaseStatus
 * -------------------------------------------------------------------------
 * 4.
 * FUNCTION: This function will get the auto traffic control release status of multicast
 *           storm control function
 * INPUT   : ifindex -- which port to set
 *           auto_traffic_control_release_status    -- which status of auto traffic control release
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_GetATCMulticastStormAutoTrafficControlReleaseStatus(UI32_T ifindex, UI32_T *auto_traffic_control_release_status);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetATCMulticastStormTrafficControlOnStatus
 * -------------------------------------------------------------------------
 * 5.
 * FUNCTION: This function will set the traffic control on status of multicast
 *           storm control function
 * INPUT   : ifindex -- which port to set
 *           traffic_control_on_status    -- which status of traffic control on
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetATCMulticastStormTrafficControlOnStatus(UI32_T ifindex, UI32_T traffic_control_on_status);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetATCMulticastStormTrafficControlOnStatus
 * -------------------------------------------------------------------------
 * 6.
 * FUNCTION: This function will get the traffic control on status of multicast
 *           storm control function
 * INPUT   : ifindex -- which port to set
 *           traffic_control_on_status    -- which status of traffic control on
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_GetATCMulticastStormTrafficControlOnStatus(UI32_T ifindex, UI32_T *traffic_control_on_status);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetATCMulticastStormTrafficControlReleaseStatus
 * -------------------------------------------------------------------------
 * 7.
 * FUNCTION: This function will set the traffic control release status of multicast
 *           storm control function
 * INPUT   : ifindex -- which port to set
 *           traffic_control_release_status    -- which status of traffic control release
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetATCMulticastStormTrafficControlReleaseStatus(UI32_T ifindex, UI32_T traffic_control_release_status);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetATCMulticastStormTrafficControlReleaseStatus
 * -------------------------------------------------------------------------
 * 8.
 * FUNCTION: This function will get the traffic control release status of multicast
 *           storm control function
 * INPUT   : ifindex -- which port to set
 *           traffic_control_release_status    -- which status of traffic control release
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_GetATCMulticastStormTrafficControlReleaseStatus(UI32_T ifindex, UI32_T *traffic_control_release_status);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetATCMulticastStormTrafficControlOnTimer
 * -------------------------------------------------------------------------
 * 9.
 * FUNCTION: This function will set the auto traffic control on status of multicast
 *           storm control function
 * INPUT   : traffic_control_on_timer    -- which status of traffic control on timer
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetATCMulticastStormTrafficControlOnTimer(UI32_T traffic_control_on_timer);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetATCMulticastStormTrafficControlOnTimer
 * -------------------------------------------------------------------------
 * 10.
 * FUNCTION: This function will get the traffic control on timer of multicast
 *           storm control function
 * INPUT   : None
 * OUTPUT  : traffic_control_on_timer    -- which status of traffic control on timer
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_GetATCMulticastStormTrafficControlOnTimer(UI32_T *traffic_control_on_timer);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetATCMulticastStormTrafficControlReleaseTimer
 * -------------------------------------------------------------------------
 * 11.
 * FUNCTION: This function will set the traffic control release timer of multicast
 *           storm control function
 * INPUT   : traffic_control_release_timer    -- which status of traffic control release timer
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetATCMulticastStormTrafficControlReleaseTimer(UI32_T traffic_control_release_timer);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetATCMulticastStormTrafficControlReleaseTimer
 * -------------------------------------------------------------------------
 * 12.
 * FUNCTION: This function will get the traffic control release timer of multicast
 *           storm control function
 * INPUT   : None
 * OUTPUT  : traffic_control_release_timer    -- which status of traffic control release timer
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_GetATCMulticastStormTrafficControlReleaseTimer(UI32_T *traffic_control_release_timer);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetATCMulticastStormStormAlarmThreshold
 * -------------------------------------------------------------------------
 * 13.
 * FUNCTION: This function will set the storm alarm fire threshold of multicast
 *           storm control function
 * INPUT   : ifindex -- which port to set
 *           storm_alarm_threshold    -- which status of storm alarm fire threshold
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetATCMulticastStormStormAlarmThreshold(UI32_T ifindex, UI32_T storm_alarm_threshold);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetATCMulticastStormStormAlarmThreshold
 * -------------------------------------------------------------------------
 * 14.
 * FUNCTION: This function will get the storm alarm fire threshold of multicast
 *           storm control function
 * INPUT   : ifindex -- which port to set
 * OUTPUT  : storm_alarm_threshold    -- which status of storm alarm fire threshold
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_GetATCMulticastStormStormAlarmThreshold(UI32_T ifindex, UI32_T *storm_alarm_threshold);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetATCMulticastStormStormClearThreshold
 * -------------------------------------------------------------------------
 * 15.
 * FUNCTION: This function will set the storm alarm clear threshold of multicast
 *           storm control function
 * INPUT   : ifindex -- which port to set
 *           auto_clear_threshold    -- which status of storm alarm clear threshold
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetATCMulticastStormStormClearThreshold(UI32_T ifindex, UI32_T storm_clear_threshold);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetATCMulticastStormStormClearThreshold
 * -------------------------------------------------------------------------
 * 16.
 * FUNCTION: This function will get the storm alarm clear threshold of multicast
 *           storm control function
 * INPUT   : ifindex -- which port to set
 * OUTPUT  : auto_clear_threshold    -- which status of storm alarm clear threshold
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_GetATCMulticastStormStormClearThreshold(UI32_T ifindex, UI32_T *storm_clear_threshold);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetATCMulticastStormStormAlarmThresholdEx
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set the storm alarm fire/clear threshold of multicast
 *           storm control function
 * INPUT   : ifindex -- which port to set
 *           storm_alarm_threshold    -- which status of storm alarm fire threshold
 *                                       0 indicates unchanged.
 *           storm_clear_threshold    -- which status of storm alarm clear threshold
 *                                       0 indicates unchanged.
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetATCMulticastStormStormAlarmThresholdEx(UI32_T ifindex, UI32_T storm_alarm_threshold, UI32_T storm_clear_threshold);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetATCMulticastStormTrapStormAlarmStatus
 * -------------------------------------------------------------------------
 * 17.
 * FUNCTION: This function will set the storm alarm fire trap status of multicast
 *           storm control function
 * INPUT   : ifindex -- which port to set
 *           trap_storm_alarm_status    -- which status of storm alarm fire trap
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetATCMulticastStormTrapStormAlarmStatus(UI32_T ifindex, UI32_T trap_storm_alarm_status);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetATCMulticastStormTrapStormAlarmStatus
 * -------------------------------------------------------------------------
 * 18.
 * FUNCTION: This function will get the storm alarm fire trap status of multicast
 *           storm control function
 * INPUT   : ifindex -- which port to set
 * OUTPUT  : trap_storm_alarm_status    -- which status of storm alarm fire trap
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_GetATCMulticastStormTrapStormAlarmStatus(UI32_T ifindex, UI32_T *trap_storm_alarm_status);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetATCMulticastStormTrapStormClearStatus
 * -------------------------------------------------------------------------
 * 19.
 * FUNCTION: This function will set the storm alarm clear trap status of multicast
 *           storm control function
 * INPUT   : ifindex -- which port to set
 *           trap_storm_clear_status    -- which status of storm alarm clear trap
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetATCMulticastStormTrapStormClearStatus(UI32_T ifindex, UI32_T trap_storm_clear_status);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetATCMulticastStormTrapStormClearStatus
 * -------------------------------------------------------------------------
 * 20.
 * FUNCTION: This function will get the storm alarm clear trap status of multicast
 *           storm control function
 * INPUT   : ifindex -- which port to set
 * OUTPUT  : trap_storm_clear_status    -- which status of storm alarm clear trap
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_GetATCMulticastStormTrapStormClearStatus(UI32_T ifindex, UI32_T *trap_storm_clear_status);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetATCMulticastStormTrapTrafficControlOnStatus
 * -------------------------------------------------------------------------
 * 21.
 * FUNCTION: This function will set the traffic control apply trap status of multicast
 *           storm control function
 * INPUT   : ifindex -- which port to set
 *           trap_traffic_control_on_status    -- which status of traffic control apply trap
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetATCMulticastStormTrapTrafficControlOnStatus(UI32_T ifindex, UI32_T trap_traffic_control_on_status);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetATCMulticastStormTrapTrafficControlOnStatus
 * -------------------------------------------------------------------------
 * 22.
 * FUNCTION: This function will get the traffic control apply trap status of multicast
 *           storm control function
 * INPUT   : ifindex -- which port to set
 * OUTPUT  : trap_traffic_control_on_status    -- which status of traffic control on trap
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_GetATCMulticastStormTrapTrafficControlOnStatus(UI32_T ifindex, UI32_T *trap_traffic_control_on_status);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetATCMulticastStormTrapTrafficControlReleaseStatus
 * -------------------------------------------------------------------------
 * 23.
 * FUNCTION: This function will set the traffic control release trap status of multicast
 *           storm control function
 * INPUT   : ifindex -- which port to set
 *           trap_traffic_control_release_status    -- which status of traffic control release trap
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetATCMulticastStormTrapTrafficControlReleaseStatus(UI32_T ifindex, UI32_T trap_traffic_control_release_status);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetATCMulticastStormTrapTrafficControlReleaseStatus
 * -------------------------------------------------------------------------
 * 24.
 * FUNCTION: This function will get the traffic control release trap status of multicast
 *           storm control function
 * INPUT   : ifindex -- which port to set
 * OUTPUT  : trap_traffic_control_release_status    -- which status of traffic control release trap
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_GetATCMulticastStormTrapTrafficControlReleaseStatus(UI32_T ifindex, UI32_T *trap_traffic_control_release_status);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetATCMulticastStormAction
 * -------------------------------------------------------------------------
 * 25.
 * FUNCTION: This function will set the action method of multicast
 *           storm control function
 * INPUT   : ifindex -- which port to set
 *           action    -- which status of action
 *                            --- 1. rate-control , 2. shutdown
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetATCMulticastStormAction(UI32_T ifindex, UI32_T action);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetATCMulticastStormAction
 * -------------------------------------------------------------------------
 * 26.
 * FUNCTION: This function will get the action method of multicast
 *           storm control function
 * INPUT   : ifindex -- which port to set
 * OUTPUT  : action    -- which status of action
 *                            --- 1. rate-control , 2. shutdown
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_GetATCMulticastStormAction(UI32_T ifindex, UI32_T *action);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetATCMulticastStormEntry
 * -------------------------------------------------------------------------
 * 27.
 * FUNCTION: This function will get the entry of multicast
 *           storm control function
 * INPUT   : atc_multicast_storm_entry    -- which status of entry
 * OUTPUT  : atc_multicast_storm_entry    -- which status of entry
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_GetATCMulticastStormEntry(UI32_T ifindex,SWCTRL_ATCMulticastStormEntry_T *atc_multicast_storm_entry);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetNextATCMulticastStormEntry
 * -------------------------------------------------------------------------
 * 28.
 * FUNCTION: This function will get the next entry of multicast
 *           storm control function
 * INPUT   : atc_multicast_storm_entry    -- which status of entry
 * OUTPUT  : atc_multicast_storm_entry    -- which status of the next entry
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_GetNextATCMulticastStormEntry(UI32_T ifindex,SWCTRL_ATCMulticastStormEntry_T *atc_multicast_storm_entry);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetATCMulticastStormTimer
 * -------------------------------------------------------------------------
 * 29.
 * FUNCTION: This function will get the tc apply timer and tc release timer on status of multicast
 *           storm control function
 * INPUT   : a_multicast_storm_timer    -- which status of tc apply timer and tc release apply
 * OUTPUT  : a_multicast_storm_timer    -- which status of tc apply timer and tc release apply
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_GetATCMulticastStormTimer(SWCTRL_ATCMulticastStormTimer_T *a_multicast_storm_timer);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetATCMulticastStormSampleType
 * -------------------------------------------------------------------------
 * 30.
 * FUNCTION: This function will set the sample type of multicast
 *           storm control function
 * INPUT   : ifindex -- which port to set
 *           atc_multicast_storm_sample_type    -- which status of sample type
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetATCMulticastStormSampleType(UI32_T ifindex, UI32_T atc_multicast_storm_sample_type);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetATCMulticastStormPacketRate
 * -------------------------------------------------------------------------
 * 31.
 * FUNCTION: This function will set the packet rate of multicast
 *           storm control function
 * INPUT   : ifindex -- which port to set
 *           atc_multicast_storm_packet_rate    -- which status of packet rate
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetATCMulticastStormPacketRate(UI32_T ifindex, UI32_T atc_multicast_storm_packet_rate);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetATCMulticastStormOctetRate
 * -------------------------------------------------------------------------
 * 32.
 * FUNCTION: This function will set the octet rate of multicast
 *           storm control function
 * INPUT   : ifindex -- which port to set
 *           atc_multicast_storm_octet_rate    -- which status of octet rate
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetATCMulticastStormOctetRate(UI32_T ifindex, UI32_T atc_multicast_storm_octet_rate);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetATCMulticastStormPercent
 * -------------------------------------------------------------------------
 * 33.
 * FUNCTION: This function will set the percent of multicast
 *           storm control function
 * INPUT   : ifindex -- which port to set
 *           atc_multicast_storm_percent    -- which status of percent
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetATCMulticastStormPercent(UI32_T ifindex, UI32_T atc_multicast_storm_percent);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetATCMulticastStormRateLimit
 * -------------------------------------------------------------------------
 * 34.
 * FUNCTION: This function will set the rate limit of multicast
 *           storm control function
 * INPUT   : ifindex -- which port to set
 *           mode    -- which status of mode
 *           nRate   -- which status of rate
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetATCMulticastStormRateLimit(UI32_T ifindex, UI32_T mode, UI32_T nRate);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetATCMulticastStormPortOperationStatus
 * -------------------------------------------------------------------------
 * 35.
 * FUNCTION: This function will set port operation status of ATC Multicast Storm
 * INPUT   : ifindex        -- which port to set
 *           operation_status   -- VAL_ifOperStatus_up/VAL_ifOperStatus_down
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : 1. RFC2863
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetATCMulticastStormPortOperationStatus(UI32_T ifindex, UI32_T operation_status);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetATCMulticastStormCurrentTrafficRate
 * -------------------------------------------------------------------------
 * 36.
 * FUNCTION: This function will get the current traffic rate of multicast
 *           storm control function
 * INPUT   : ifindex -- which port to set
 * OUTPUT  : current_traffic_rate    -- which status of current traffic rate
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_GetATCMulticastStormCurrentTrafficRate(UI32_T ifindex, UI32_T *current_traffic_rate);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetRunningATCMulticastStormEntry
 * -------------------------------------------------------------------------
 * 37.
 * FUNCTION: This function will get the running entry of multicast
 *           storm control function
 * INPUT   : atc_multicast_storm_entry    -- which status of running entry
 * OUTPUT  : atc_multicast_storm_entry    -- which status of running entry
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T SWCTRL_GetRunningATCMulticastStormEntry(UI32_T port,SWCTRL_ATCMulticastStormEntry_T *atc_multicast_storm_entry);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetRunningATCMulticastStormTimer
 * -------------------------------------------------------------------------
 * 38.
 * FUNCTION: This function will get the running tc apply timer and tc release timer of multicast
 *           storm control function
 * INPUT   : ifindex -- which port to set
 *           a_multicast_storm_timer    -- which status of running tc apply timer and tc release timer
 * OUTPUT  : a_multicast_storm_timer    -- which status of running tc apply timer and tc release timer
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T SWCTRL_GetRunningATCMulticastStormTimer(SWCTRL_ATCMulticastStormTimer_T *a_multicast_storm_timer);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_ATCMulticastStormProtectionFSM
 * -------------------------------------------------------------------------
 * 39.
 * FUNCTION: This function will set the event of multicast
 *           storm control function
 * INPUT   : ifindex -- which port to set
 *           action    -- which status of event
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_ATCMulticastStormProtectionFSM(UI32_T ifindex, UI32_T event);

BOOL_T SWCTRL_DisableMStormAfterClearThreshold(UI32_T ifindex);
#endif /* End of SYS_CPNT_ATC_MSTORM */


#if ( (SYS_CPNT_ATC_BSTORM == TRUE) || (SYS_CPNT_ATC_MSTORM == TRUE) )
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PacketFlowAnalyser
 * -------------------------------------------------------------------------
 * 40.
 * FUNCTION: This function will do the packet flow analyser of auto traffic control
 *           storm control function
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
UI32_T SWCTRL_PacketFlowAnalyser(void);
#endif

#if (SYS_CPNT_ATC_BSTORM == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetATCBroadcastStormAutoTrafficControlOnStatus
 * -------------------------------------------------------------------------
 * 1.
 * FUNCTION: This function will set the auto traffic control on status of broadcast
 *           storm control function
 * INPUT   : ifindex -- which port to set
 *           auto_traffic_control_on_status    -- which status of auto traffic control on
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetATCBroadcastStormAutoTrafficControlOnStatus(UI32_T ifindex, UI32_T auto_traffic_control_on_status);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetATCBroadcastStormAutoTrafficControlOnStatus
 * -------------------------------------------------------------------------
 * 2.
 * FUNCTION: This function will get the auto traffic control on status of broadcast
 *           storm control function
 * INPUT   : ifindex -- which port to set
 *
 * OUTPUT  : auto_traffic_control_on_status    -- which status of auto traffic control on
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_GetATCBroadcastStormAutoTrafficControlOnStatus(UI32_T ifindex, UI32_T *auto_traffic_control_on_status);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetATCBroadcastStormAutoTrafficControlReleaseStatus
 * -------------------------------------------------------------------------
 * 3.
 * FUNCTION: This function will set the auto traffic control release status of broadcast
 *           storm control function
 * INPUT   : ifindex -- which port to set
 *           auto_traffic_control_release_status    -- which status of auto traffic control release
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetATCBroadcastStormAutoTrafficControlReleaseStatus(UI32_T ifindex, UI32_T auto_traffic_control_release_status);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetATCBroadcastStormAutoTrafficControlReleaseStatus
 * -------------------------------------------------------------------------
 * 4.
 * FUNCTION: This function will get the auto traffic control release status of broadcast
 *           storm control function
 * INPUT   : ifindex -- which port to set
 *           auto_traffic_control_release_status    -- which status of auto traffic control release
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_GetATCBroadcastStormAutoTrafficControlReleaseStatus(UI32_T ifindex, UI32_T *auto_traffic_control_release_status);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetATCBroadcastStormTrafficControlReleaseStatus
 * -------------------------------------------------------------------------
 * 7.
 * FUNCTION: This function will set the traffic control release status of broadcast
 *           storm control function
 * INPUT   : ifindex -- which port to set
 *           traffic_control_release_status    -- which status of traffic control release
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetATCBroadcastStormTrafficControlReleaseStatus(UI32_T ifindex, UI32_T traffic_control_release_status);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetATCBroadcastStormTrafficControlReleaseStatus
 * -------------------------------------------------------------------------
 * 8.
 * FUNCTION: This function will get the traffic control release status of broadcast
 *           storm control function
 * INPUT   : ifindex -- which port to set
 *           traffic_control_release_status    -- which status of traffic control release
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_GetATCBroadcastStormTrafficControlReleaseStatus(UI32_T ifindex, UI32_T *traffic_control_release_status);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetATCBroadcastStormTrafficControlOnTimer
 * -------------------------------------------------------------------------
 * 9.
 * FUNCTION: This function will set the auto traffic control on status of broadcast
 *           storm control function
 * INPUT   : traffic_control_on_timer    -- which status of traffic control on timer
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetATCBroadcastStormTrafficControlOnTimer(UI32_T traffic_control_on_timer);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetATCBroadcastStormTrafficControlOnTimer
 * -------------------------------------------------------------------------
 * 10.
 * FUNCTION: This function will get the traffic control on timer of broadcast
 *           storm control function
 * INPUT   : None
 * OUTPUT  : traffic_control_on_timer    -- which status of traffic control on timer
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_GetATCBroadcastStormTrafficControlOnTimer(UI32_T *traffic_control_on_timer);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetATCBroadcastStormTrafficControlReleaseTimer
 * -------------------------------------------------------------------------
 * 11.
 * FUNCTION: This function will set the traffic control release timer of broadcast
 *           storm control function
 * INPUT   : traffic_control_release_timer    -- which status of traffic control release timer
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetATCBroadcastStormTrafficControlReleaseTimer(UI32_T traffic_control_release_timer);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetATCBroadcastStormTrafficControlReleaseTimer
 * -------------------------------------------------------------------------
 * 12.
 * FUNCTION: This function will get the traffic control release timer of broadcast
 *           storm control function
 * INPUT   : None
 * OUTPUT  : traffic_control_release_timer    -- which status of traffic control release timer
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_GetATCBroadcastStormTrafficControlReleaseTimer(UI32_T *traffic_control_release_timer);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetATCBroadcastStormStormAlarmThreshold
 * -------------------------------------------------------------------------
 * 13.
 * FUNCTION: This function will set the storm alarm fire threshold of broadcast
 *           storm control function
 * INPUT   : ifindex -- which port to set
 *           storm_alarm_threshold    -- which status of storm alarm fire threshold
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetATCBroadcastStormStormAlarmThreshold(UI32_T ifindex, UI32_T storm_alarm_threshold);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetATCBroadcastStormStormAlarmThreshold
 * -------------------------------------------------------------------------
 * 14.
 * FUNCTION: This function will get the storm alarm fire threshold of broadcast
 *           storm control function
 * INPUT   : ifindex -- which port to set
 * OUTPUT  : storm_alarm_threshold    -- which status of storm alarm fire threshold
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_GetATCBroadcastStormStormAlarmThreshold(UI32_T ifindex, UI32_T *storm_alarm_threshold);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetATCBroadcastStormStormClearThreshold
 * -------------------------------------------------------------------------
 * 15.
 * FUNCTION: This function will set the storm alarm clear threshold of broadcast
 *           storm control function
 * INPUT   : ifindex -- which port to set
 *           auto_clear_threshold    -- which status of storm alarm clear threshold
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetATCBroadcastStormStormClearThreshold(UI32_T ifindex, UI32_T storm_clear_threshold);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetATCBroadcastStormStormClearThreshold
 * -------------------------------------------------------------------------
 * 16.
 * FUNCTION: This function will get the storm alarm clear threshold of broadcast
 *           storm control function
 * INPUT   : ifindex -- which port to set
 * OUTPUT  : auto_clear_threshold    -- which status of storm alarm clear threshold
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_GetATCBroadcastStormStormClearThreshold(UI32_T ifindex, UI32_T *storm_clear_threshold);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetATCBroadcastStormStormAlarmThresholdEx
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set the storm alarm fire/clear threshold of broadcast
 *           storm control function
 * INPUT   : ifindex -- which port to set
 *           storm_alarm_threshold    -- which status of storm alarm fire threshold
 *                                       0 indicates unchanged.
 *           storm_clear_threshold    -- which status of storm alarm clear threshold
 *                                       0 indicates unchanged.
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetATCBroadcastStormStormAlarmThresholdEx(UI32_T ifindex, UI32_T storm_alarm_threshold, UI32_T storm_clear_threshold);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetATCBroadcastStormTrapStormAlarmStatus
 * -------------------------------------------------------------------------
 * 17.
 * FUNCTION: This function will set the storm alarm fire trap status of broadcast
 *           storm control function
 * INPUT   : ifindex -- which port to set
 *           trap_storm_alarm_status    -- which status of storm alarm fire trap
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetATCBroadcastStormTrapStormAlarmStatus(UI32_T ifindex, UI32_T trap_storm_alarm_status);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetATCBroadcastStormTrapStormAlarmStatus
 * -------------------------------------------------------------------------
 * 18.
 * FUNCTION: This function will get the storm alarm fire trap status of broadcast
 *           storm control function
 * INPUT   : ifindex -- which port to set
 * OUTPUT  : trap_storm_alarm_status    -- which status of storm alarm fire trap
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_GetATCBroadcastStormTrapStormAlarmStatus(UI32_T ifindex, UI32_T *trap_storm_alarm_status);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetATCBroadcastStormTrapStormClearStatus
 * -------------------------------------------------------------------------
 * 19.
 * FUNCTION: This function will set the storm alarm clear trap status of broadcast
 *           storm control function
 * INPUT   : ifindex -- which port to set
 *           trap_storm_clear_status    -- which status of storm alarm clear trap
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetATCBroadcastStormTrapStormClearStatus(UI32_T ifindex, UI32_T trap_storm_clear_status);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetATCBroadcastStormTrapStormClearStatus
 * -------------------------------------------------------------------------
 * 20.
 * FUNCTION: This function will get the storm alarm clear trap status of broadcast
 *           storm control function
 * INPUT   : ifindex -- which port to set
 * OUTPUT  : trap_storm_clear_status    -- which status of storm alarm clear trap
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_GetATCBroadcastStormTrapStormClearStatus(UI32_T ifindex, UI32_T *trap_storm_clear_status);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetATCBroadcastStormTrapTrafficControlOnStatus
 * -------------------------------------------------------------------------
 * 21.
 * FUNCTION: This function will set the traffic control apply trap status of broadcast
 *           storm control function
 * INPUT   : ifindex -- which port to set
 *           trap_traffic_control_on_status    -- which status of traffic control apply trap
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetATCBroadcastStormTrapTrafficControlOnStatus(UI32_T ifindex, UI32_T trap_traffic_control_on_status);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetATCBroadcastStormTrapTrafficControlOnStatus
 * -------------------------------------------------------------------------
 * 22.
 * FUNCTION: This function will get the traffic control apply trap status of broadcast
 *           storm control function
 * INPUT   : ifindex -- which port to set
 * OUTPUT  : trap_traffic_control_on_status    -- which status of traffic control on trap
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_GetATCBroadcastStormTrapTrafficControlOnStatus(UI32_T ifindex, UI32_T *trap_traffic_control_on_status);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetATCBroadcastStormTrapTrafficControlReleaseStatus
 * -------------------------------------------------------------------------
 * 23.
 * FUNCTION: This function will set the traffic control release trap status of broadcast
 *           storm control function
 * INPUT   : ifindex -- which port to set
 *           trap_traffic_control_release_status    -- which status of traffic control release trap
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetATCBroadcastStormTrapTrafficControlReleaseStatus(UI32_T ifindex, UI32_T trap_traffic_control_release_status);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetATCBroadcastStormTrapTrafficControlReleaseStatus
 * -------------------------------------------------------------------------
 * 24.
 * FUNCTION: This function will get the traffic control release trap status of broadcast
 *           storm control function
 * INPUT   : ifindex -- which port to set
 * OUTPUT  : trap_traffic_control_release_status    -- which status of traffic control release trap
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_GetATCBroadcastStormTrapTrafficControlReleaseStatus(UI32_T ifindex, UI32_T *trap_traffic_control_release_status);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetATCBroadcastStormAction
 * -------------------------------------------------------------------------
 * 25.
 * FUNCTION: This function will set the action method of broadcast
 *           storm control function
 * INPUT   : ifindex -- which port to set
 *           action    -- which status of action
 *                            --- 1. rate-control , 2. shutdown
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetATCBroadcastStormAction(UI32_T ifindex, UI32_T action);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetATCBroadcastStormAction
 * -------------------------------------------------------------------------
 * 26.
 * FUNCTION: This function will get the action method of broadcast
 *           storm control function
 * INPUT   : ifindex -- which port to set
 * OUTPUT  : action    -- which status of action
 *                            --- 1. rate-control , 2. shutdown
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_GetATCBroadcastStormAction(UI32_T ifindex, UI32_T *action);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetATCBroadcastStormEntry
 * -------------------------------------------------------------------------
 * 27.
 * FUNCTION: This function will get the entry of broadcast
 *           storm control function
 * INPUT   : atc_broadcast_storm_entry    -- which status of entry
 * OUTPUT  : atc_broadcast_storm_entry    -- which status of entry
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/

BOOL_T SWCTRL_GetATCBroadcastStormEntry(UI32_T ifindex,SWCTRL_ATCBroadcastStormEntry_T *atc_broadcast_storm_entry);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetNextATCBroadcastStormEntry
 * -------------------------------------------------------------------------
 * 28.
 * FUNCTION: This function will get the next entry of broadcast
 *           storm control function
 * INPUT   : atc_broadcast_storm_entry    -- which status of entry
 * OUTPUT  : atc_broadcast_storm_entry    -- which status of the next entry
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_GetNextATCBroadcastStormEntry(UI32_T ifindex,SWCTRL_ATCBroadcastStormEntry_T *atc_broadcast_storm_entry);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetATCBroadcastStormTimer
 * -------------------------------------------------------------------------
 * 29.
 * FUNCTION: This function will set the tc apply timer and tc release timer on status of broadcast
 *           storm control function
 * INPUT   : a_broadcast_storm_timer    -- which status of tc apply timer and tc release apply
 * OUTPUT  : a_broadcast_storm_timer    -- which status of tc apply timer and tc release apply
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_GetATCBroadcastStormTimer(SWCTRL_ATCBroadcastStormTimer_T *a_broadcast_storm_timer);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetATCBroadcastStormSampleType
 * -------------------------------------------------------------------------
 * 30.
 * FUNCTION: This function will set the sample type of broadcast
 *           storm control function
 * INPUT   : ifindex -- which port to set
 *           atc_broadcast_storm_sample_type    -- which status of sample type
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetATCBroadcastStormSampleType(UI32_T ifindex, UI32_T atc_broadcast_storm_sample_type);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetATCBroadcastStormPacketRate
 * -------------------------------------------------------------------------
 * 31.
 * FUNCTION: This function will set the packet rate of broadcast
 *           storm control function
 * INPUT   : ifindex -- which port to set
 *           atc_broadcast_storm_packet_rate    -- which status of packet rate
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetATCBroadcastStormPacketRate(UI32_T ifindex, UI32_T atc_broadcast_storm_packet_rate);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetATCBroadcastStormOctetRate
 * -------------------------------------------------------------------------
 * 32.
 * FUNCTION: This function will set the octet rate of broadcast
 *           storm control function
 * INPUT   : ifindex -- which port to set
 *           atc_broadcast_storm_octet_rate    -- which status of octet rate
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetATCBroadcastStormOctetRate(UI32_T ifindex, UI32_T atc_broadcast_storm_octet_rate);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetATCBroadcastStormPercent
 * -------------------------------------------------------------------------
 * 33.
 * FUNCTION: This function will set the percent of broadcast
 *           storm control function
 * INPUT   : ifindex -- which port to set
 *           atc_broadcast_storm_percent    -- which status of percent
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetATCBroadcastStormPercent(UI32_T ifindex, UI32_T atc_broadcast_storm_percent);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetATCBroadcastStormRateLimit
 * -------------------------------------------------------------------------
 * 34.
 * FUNCTION: This function will set the rate limit of broadcast
 *           storm control function
 * INPUT   : ifindex -- which port to set
 *           mode    -- which status of mode
 *           nRate   -- which status of rate
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetATCBroadcastStormRateLimit(UI32_T ifindex, UI32_T mode, UI32_T nRate);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetATCBroadcastStormPortOperationStatus
 * -------------------------------------------------------------------------
 * 35.
 * FUNCTION: This function will set port operation status of ATC Broadcast Storm
 * INPUT   : ifindex        -- which port to set
 *           operation_status   -- VAL_ifOperStatus_up/VAL_ifOperStatus_down
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : 1. RFC2863
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetATCBroadcastStormPortOperationStatus(UI32_T ifindex, UI32_T operation_status);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetATCBroadcastStormCurrentTrafficRate
 * -------------------------------------------------------------------------
 * 36.
 * FUNCTION: This function will get the current traffic rate of broadcast
 *           storm control function
 * INPUT   : ifindex -- which port to set
 * OUTPUT  : current_traffic_rate    -- which status of current traffic rate
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_GetATCBroadcastStormCurrentTrafficRate(UI32_T ifindex, UI32_T *current_traffic_rate);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetRunningATCBroadcastStormEntry
 * -------------------------------------------------------------------------
 * 37.
 * FUNCTION: This function will get the running entry of broadcast
 *           storm control function
 * INPUT   : atc_broadcast_storm_entry    -- which status of running entry
 * OUTPUT  : atc_broadcast_storm_entry    -- which status of running entry
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T SWCTRL_GetRunningATCBroadcastStormEntry(UI32_T port,SWCTRL_ATCBroadcastStormEntry_T *atc_broadcast_storm_entry);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetRunningATCBroadcastStormTimer
 * -------------------------------------------------------------------------
 * 38.
 * FUNCTION: This function will get the running tc apply timer and tc release timer of broadcast
 *           storm control function
 * INPUT   : ifindex -- which port to set
 *           a_broadcast_storm_timer    -- which status of running tc apply timer and tc release timer
 * OUTPUT  : a_broadcast_storm_timer    -- which status of running tc apply timer and tc release timer
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T SWCTRL_GetRunningATCBroadcastStormTimer(SWCTRL_ATCBroadcastStormTimer_T *a_broadcast_storm_timer);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_ATCBroadcastStormProtectionFSM
 * -------------------------------------------------------------------------
 * 39.
 * FUNCTION: This function will set the event of multicast
 *           storm control function
 * INPUT   : ifindex -- which port to set
 *           action    -- which status of event
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_ATCBroadcastStormProtectionFSM(UI32_T ifindex, UI32_T event);

BOOL_T SWCTRL_DisableBStormAfterClearThreshold(UI32_T ifindex);
#endif /* End of SYS_CPNT_ATC_BSTORM */


/* NAMING CONSTANT DECLARATIONS
 */
#define SWCTRL_SPDPLX_AUTONEG 8
#define SWCTRL_MAX_1V_PROTOCOL_VALUE_LENGTH SYS_ADPT_1V_MAX_PROTOCOL_VALUE_BUFFER_LENGTH

#if (SYS_CPNT_EFM_OAM == TRUE)
#define SWCTRL_LOOPBACK_MODE_FLAG_ENABLED   TRUE
#define SWCTRL_LOOPBACK_MODE_FLAG_DISABLED  FALSE
#endif

/* reason for port status setting
 */
#define SWCTRL_PORT_STATUS_SET_BY_CFG                       BIT_0
#define SWCTRL_PORT_STATUS_SET_BY_XSTP_LBD                  BIT_1
#define SWCTRL_PORT_STATUS_SET_BY_XSTP_BPDUGUARD            BIT_2
#define SWCTRL_PORT_STATUS_SET_BY_NETACCESS_LINK_DETECTION  BIT_3
#define SWCTRL_PORT_STATUS_SET_BY_NETACCESS_DYNAMIC_QOS     BIT_4
#define SWCTRL_PORT_STATUS_SET_BY_PORTSEC                   BIT_5
#define SWCTRL_PORT_STATUS_SET_BY_LBD                       BIT_6
#define SWCTRL_PORT_STATUS_SET_BY_ATC_BSTORM                BIT_7
#define SWCTRL_PORT_STATUS_SET_BY_ATC_MSTORM                BIT_8
#define SWCTRL_PORT_STATUS_SET_BY_UDLD                      BIT_9
#define SWCTRL_PORT_STATUS_SET_BY_SW_LICENSE                BIT_10

/* protocol bitmap for L2PT status
 */
#define SWCTRL_L2PT_PROTOCOL_NONE               0x0
#define SWCTRL_L2PT_PROTOCOL_STP                BIT_0
#define SWCTRL_L2PT_PROTOCOL_LLDP               BIT_1
#define SWCTRL_L2PT_PROTOCOL_CDP                BIT_2
#define SWCTRL_L2PT_PROTOCOL_VTP                BIT_3
#define SWCTRL_L2PT_PROTOCOL_PVST_PLUS          BIT_4
#define SWCTRL_L2PT_PROTOCOL_CUSTOM_PDU         BIT_5

/* for SYS_CPNT_CN
 */
#define SWCTRL_QCN_CPQ_INVALID                  0xffffffff

/* TYPE DECLARATIONS
 */

#define SWCTRL_DEBUG_ENABLE FALSE

#if (SWCTRL_DEBUG_ENABLE == TRUE)

#define SWCTRL_DEBUG_LINE() \
{  \
  printf("\r\n%s(%d)",__FUNCTION__,__LINE__); \
  fflush(stdout); \
}

#define SWCTRL_DEBUG_MSG(a,b...)   \
{ \
  printf(a,##b); \
  fflush(stdout); \
}

#define SWCTRL_DEBUG_PORTMSG(port_entry)   \
{ \
  printf("\r\n index=(%lu) name=(%s) type=(%lu) speed_dpx_cfg=(%lu) flow_ctrl_cfg=(%lu) capabilities=(%lu) autonegotiation=(%lu) speed_dpx_status=(%lu) flow_ctrl_status=(%lu) trunk_index=(%lu) forced_mode=(%lu) forced_1000t_mode=(%lu)", \
  (port_entry)->port_index, \
  (port_entry)->port_name, \
  (port_entry)->port_type, \
  (port_entry)->port_speed_dpx_cfg, \
  (port_entry)->port_flow_ctrl_cfg, \
  (port_entry)->port_capabilities, \
  (port_entry)->port_autonegotiation, \
  (port_entry)->port_speed_dpx_status, \
  (port_entry)->port_flow_ctrl_status, \
  (port_entry)->port_trunk_index, \
  (port_entry)->port_forced_mode, \
  (port_entry)->port_forced_1000t_mode); \
  fflush(stdout); \
}
#else
#define SWCTRL_DEBUG_LINE()
#define SWCTRL_DEBUG_MSG(a,b...)
#define SWCTRL_DEBUG_PORTMSG(port_entry)
#endif

void SWCTRL_DEBUG_ALLPORTMSG(UI32_T unit);

enum SWCTRL_Link_Status_E
{
    SWCTRL_LINK_UP = 1,
    SWCTRL_LINK_DOWN
};

enum SWCTRL_Jumbo_Frame_Status_E
{
    SWCTRL_JUMBO_FRAME_DISABLE = VAL_switchJumboFrameStatus_disabled,
    SWCTRL_JUMBO_FRAME_ENABLE  = VAL_switchJumboFrameStatus_enabled
};

enum SWCTRL_MST_Status_E
{
    SWCTRL_MST_DISABLE = 0,
    SWCTRL_MST_ENABLE
};

enum SWCTRL_Port_Dormant_Type_E
{
    SWCTRL_PORT_DORMANT_STATUS_TYPE_DOT1X = 9,
    SWCTRL_PORT_DORMANT_STATUS_TYPE_LACP
};

/* For LACP
 */
enum SWCTRL_LACP_Attach_E
{
    VAL_LacpAttach_deattached = 0,
    VAL_LacpAttach_attached
};

enum SWCTRL_LACP_Collecting_E
{
    VAL_LacpCollecting_collecting = 0,
    VAL_LacpCollecting_not_collecting
};

/* For DOT1X
 */
enum SWCTRL_DOT1X_Port_Status_E
{
    SWCTRL_DOT1X_PORT_DISABLE = 0,
    SWCTRL_DOT1X_PORT_ENABLE
};

enum SWCTRL_DOT1X_Port_Authorized_E
{
    SWCTRL_DOT1X_PORT_AUTHORIZED = 0,
    SWCTRL_DOT1X_PORT_UNAUTHORIZED
};

enum SWCTRL_Egress_Scheduling_Method_E
{
/* 2008-06-02, Jinfeng.Chen:
    add some constants to support FireBolt2's new method and adjust the order to adapt the
    dev_swdrvl4.h-DEV_SWDRVL4_EgressSchedulingMethod_E
 */
    SWCTRL_WEIGHT_ROUND_ROBIN_METHOD = 1,
    SWCTRL_STRICT_PRIORITY_METHOD,
    SWCTRL_DEFICIT_ROUND_ROBIN_METHOD,
    SWCTRL_SP_WRR_METHOD,
    SWCTRL_SP_DRR_METHOD
};

enum SWCTRL_PortSecurity_Enabled_Role_E /*kevin*/
{
    SWCTRL_PORT_SECURITY_ENABLED_BY_NONE = 0,
    SWCTRL_PORT_SECURITY_ENABLED_BY_PSEC,
    SWCTRL_PORT_SECURITY_ENABLED_BY_DOT1X,
    SWCTRL_PORT_SECURITY_ENABLED_BY_NETACCESS
};


enum SWCTRL_IF_MAU_TYPE_E
{
     SWCTRL_IF_MAU_TYPE_OTHER  = 0,
     SWCTRL_IF_MAU_TYPE_AUI,
     SWCTRL_IF_MAU_TYPE_10BASE5,
     SWCTRL_IF_MAU_TYPE_FOIRL,
     SWCTRL_IF_MAU_TYPE_10BASE2,
     SWCTRL_IF_MAU_TYPE_10BASET,
     SWCTRL_IF_MAU_TYPE_10BASEFP,
     SWCTRL_IF_MAU_TYPE_10BASEFB,
     SWCTRL_IF_MAU_TYPE_10BASEFL,
     SWCTRL_IF_MAU_TYPE_10BROAD36,
     SWCTRL_IF_MAU_TYPE_10BASETHD,
     SWCTRL_IF_MAU_TYPE_10BASETFD,
     SWCTRL_IF_MAU_TYPE_10BASEFLHD,
     SWCTRL_IF_MAU_TYPE_10BASEFLFD,
     SWCTRL_IF_MAU_TYPE_100BASET4,
     SWCTRL_IF_MAU_TYPE_100BASETXHD,
     SWCTRL_IF_MAU_TYPE_100BASETXFD,
     SWCTRL_IF_MAU_TYPE_100BASEFXHD,
     SWCTRL_IF_MAU_TYPE_100BASEFXFD,
     SWCTRL_IF_MAU_TYPE_100BASET2HD,
     SWCTRL_IF_MAU_TYPE_100BASET2FD,
     SWCTRL_IF_MAU_TYPE_1000BASEXHD,
     SWCTRL_IF_MAU_TYPE_1000BASEXFD,
     SWCTRL_IF_MAU_TYPE_1000BASELXHD,
     SWCTRL_IF_MAU_TYPE_1000BASELXFD,
     SWCTRL_IF_MAU_TYPE_1000BASESXHD,
     SWCTRL_IF_MAU_TYPE_1000BASESXFD,
     SWCTRL_IF_MAU_TYPE_1000BASECXHD,
     SWCTRL_IF_MAU_TYPE_1000BASECXFD,
     SWCTRL_IF_MAU_TYPE_1000BASETHD,
     SWCTRL_IF_MAU_TYPE_1000BASETFD,
     SWCTRL_IF_MAU_TYPE_10GIGBASEX,
     SWCTRL_IF_MAU_TYPE_10GIGBASELX4,
     SWCTRL_IF_MAU_TYPE_10GIGBASER,
     SWCTRL_IF_MAU_TYPE_10GIGBASEER,
     SWCTRL_IF_MAU_TYPE_10GIGBASELR,
     SWCTRL_IF_MAU_TYPE_10GIGBASESR,
     SWCTRL_IF_MAU_TYPE_10GIGBASEW,
     SWCTRL_IF_MAU_TYPE_10GIGBASEEW,
     SWCTRL_IF_MAU_TYPE_10GIGBASELW,
     SWCTRL_IF_MAU_TYPE_10GIGBASESW
};

enum SWCTRL_Event_Type_E
{
    SWCTRL_CALLBACK_EVENT_HOTSWAP_INSERT = 1,
    SWCTRL_CALLBACK_EVENT_HOTSWAP_REMOVE,
    SWCTRL_CALLBACK_EVENT_PORT_LINKUP,
    SWCTRL_CALLBACK_EVENT_PORT_LINKDOWN,
    SWCTRL_CALLBACK_EVENT_PORT_TYPE_CHANGED,
    SWCTRL_CALLBACK_EVENT_PORT_SPEEDDUPLEX,
    SWCTRL_CALLBACK_EVENT_PORT_FLOWCTRL,
    SWCTRL_CALLBACK_EVENT_CRAFT_PORT_LINKUP,
    SWCTRL_CALLBACK_EVENT_CRAFT_PORT_LINKDOWN,
    SWCTRL_CALLBACK_EVENT_PORT_SFP_PRESENT,
    SWCTRL_CALLBACK_EVENT_PORT_SFP_DDM_INFO_MEASURED,
};

/* trunk load balance
 * must be identical to SWDRV_Trunk_Load_Balance_Mode_E
 */
enum SWCTRL_Trunk_Load_Balance_Mode_E
{
    SWCTRL_TRUNK_BALANCE_MODE_MAC_SA = 1,
    SWCTRL_TRUNK_BALANCE_MODE_MAC_DA,
    SWCTRL_TRUNK_BALANCE_MODE_MAC_SA_DA,
    SWCTRL_TRUNK_BALANCE_MODE_IP_SA,
    SWCTRL_TRUNK_BALANCE_MODE_IP_DA,
    SWCTRL_TRUNK_BALANCE_MODE_IP_SA_DA,
};

enum
{
    SWCTRL_MGR_IPCCMD_SETPORTADMINSTATUS,
    SWCTRL_MGR_IPCCMD_SETPORTSTATUS,
    SWCTRL_MGR_IPCCMD_SETPORT1000BASETFORCEMODE,
    SWCTRL_MGR_IPCCMD_SETPORTCFGSPEEDDUPLEX,
    SWCTRL_MGR_IPCCMD_SETPORTDEFAULTSPEEDDUPLEX,
    SWCTRL_MGR_IPCCMD_SETPORTAUTONEGENABLE,
    SWCTRL_MGR_IPCCMD_SETPORTCFGFLOWCTRLENABLE,
    SWCTRL_MGR_IPCCMD_SETPORTAUTONEGCAPABILITY,
    SWCTRL_MGR_IPCCMD_SETPORTDEFAULTAUTONEGCAPABILITY,
    SWCTRL_MGR_IPCCMD_SETPORTLINKCHANGETRAPENABLE,
    SWCTRL_MGR_IPCCMD_SETPORTDOT1XENABLE,
    SWCTRL_MGR_IPCCMD_SETPORTDOT1XAUTHSTATE,
    SWCTRL_MGR_IPCCMD_SETPORTLACPOPERENABLE,
    SWCTRL_MGR_IPCCMD_SETPORTLACPADMINENABLE,
    SWCTRL_MGR_IPCCMD_SETPORTLACPENABLE,
    SWCTRL_MGR_IPCCMD_SETLACPENABLE,
    SWCTRL_MGR_IPCCMD_SETPORTLACPCOLLECTING,
    SWCTRL_MGR_IPCCMD_SETPORTOPERDORMANTSTATUS,
    SWCTRL_MGR_IPCCMD_TRIGGERPORTOPERDORMANTEVENT,
    SWCTRL_MGR_IPCCMD_SETPORTSTASTATE,
    SWCTRL_MGR_IPCCMD_SETMSTENABLESTATUS,
    SWCTRL_MGR_IPCCMD_SETPORTSECURITYSTATUS,
    SWCTRL_MGR_IPCCMD_SETPORTSECURITYACTIONSTATUS,
    SWCTRL_MGR_IPCCMD_SETPORTSECURITYACTIONTRAPOPERSTATUS,
    SWCTRL_MGR_IPCCMD_ADDSECURITYACTIONTRAPTIMESTAMP,
    SWCTRL_MGR_IPCCMD_RESETSECURITYACTIONTRAPTIMESTAMP,
    SWCTRL_MGR_IPCCMD_SETPORTPVID,
    SWCTRL_MGR_IPCCMD_CREATEVLAN,
    SWCTRL_MGR_IPCCMD_DESTROYVLAN,
    SWCTRL_MGR_IPCCMD_SETGLOBALDEFAULTVLAN,
    SWCTRL_MGR_IPCCMD_ADDPORTTOVLANMEMBERSET,
    SWCTRL_MGR_IPCCMD_DELETEPORTFROMVLANMEMBERSET,
    SWCTRL_MGR_IPCCMD_ADDPORTTOVLANUNTAGGEDSET,
    SWCTRL_MGR_IPCCMD_DELETEPORTFROMVLANUNTAGGEDSET,
    SWCTRL_MGR_IPCCMD_GETSYSTEMMTU,
    SWCTRL_MGR_IPCCMD_SETSYSTEMMTU,
    SWCTRL_MGR_IPCCMD_SETPORTMTU,
    SWCTRL_MGR_IPCCMD_ENABLEINGRESSFILTER,
    SWCTRL_MGR_IPCCMD_DISABLEINGRESSFILTER,
    SWCTRL_MGR_IPCCMD_ADMITVLANTAGGEDFRAMESONLY,
    SWCTRL_MGR_IPCCMD_ADMITVLANUNTAGGEDFRAMESONLY,
    SWCTRL_MGR_IPCCMD_ADMITALLFRAMES,
    SWCTRL_MGR_IPCCMD_ALLOWTOBETRUNKMEMBER,
    SWCTRL_MGR_IPCCMD_SETTRUNKBALANCEMODE,
    SWCTRL_MGR_IPCCMD_SETTRUNKMEMBERACTIVESTATUS,
    SWCTRL_MGR_IPCCMD_ENABLEIGMPTRAP,
    SWCTRL_MGR_IPCCMD_DISABLEIGMPTRAP,
    SWCTRL_MGR_IPCCMD_SETUNKNOWNIPMCASTFWDPORTLIST,
    SWCTRL_MGR_IPCCMD_SETBSTORMCONTROLRATELIMIT,
    SWCTRL_MGR_IPCCMD_SETMSTORMCONTROLRATELIMIT,
    SWCTRL_MGR_IPCCMD_SETUNKNOWNUSTORMCONTROLRATELIMIT,
    SWCTRL_MGR_IPCCMD_SETBROADCASTSTORMSTATUS,
    SWCTRL_MGR_IPCCMD_SETMULTICASTSTORMSTATUS,
    SWCTRL_MGR_IPCCMD_SETUNKNOWNUNICASTSTORMSTATUS,
    SWCTRL_MGR_IPCCMD_SETPORTUSERDEFAULTPRIORITY,
    SWCTRL_MGR_IPCCMD_SETPRIORITYMAPPING,
    SWCTRL_MGR_IPCCMD_SETPRIORITYMAPPINGPERSYSTEM,
    SWCTRL_MGR_IPCCMD_DISABLETOSCOSMAP,
    SWCTRL_MGR_IPCCMD_ENABLEDSCPCOSMAP,
    SWCTRL_MGR_IPCCMD_DISABLEDSCPCOSMAP,
    SWCTRL_MGR_IPCCMD_SETPORTTOSCOSMAP,
    SWCTRL_MGR_IPCCMD_SETPORTDSCPCOSMAP,
    SWCTRL_MGR_IPCCMD_SETPORTTCPPORTCOSMAP,
    SWCTRL_MGR_IPCCMD_DELPORTTOSCOSMAP,
    SWCTRL_MGR_IPCCMD_DELPORTDSCPCOSMAP,
    SWCTRL_MGR_IPCCMD_DELPORTTCPPORTCOSMAP,
    SWCTRL_MGR_IPCCMD_ENABLEPRIVATEVLAN,
    SWCTRL_MGR_IPCCMD_DISABLEPRIVATEVLAN,
    SWCTRL_MGR_IPCCMD_SETPRIVATEVLAN,
    SWCTRL_MGR_IPCCMD_SETPORTPRIVATEMODE,
    SWCTRL_MGR_IPCCMD_SETPRIVATEVLANBYSESSIONID,
    SWCTRL_MGR_IPCCMD_DELETEPRIVATEVLANPORTLISTBYSESSIONID,
    SWCTRL_MGR_IPCCMD_DESTROYPRIVATEVLANSESSION,
    SWCTRL_MGR_IPCCMD_ENABLEPRIVATEVLANUPLINKTOUPLINKBLOCKINGMODE,
    SWCTRL_MGR_IPCCMD_DISABLEPRIVATEVLANUPLINKTOUPLINKBLOCKINGMODE,
    SWCTRL_MGR_IPCCMD_ENABLEPORTINGRESSRATELIMIT,
    SWCTRL_MGR_IPCCMD_DISABLEPORTINGRESSRATELIMIT,
    SWCTRL_MGR_IPCCMD_SETPORTINGRESSRATELIMIT,
    SWCTRL_MGR_IPCCMD_ENABLEPORTEGRESSRATELIMIT,
    SWCTRL_MGR_IPCCMD_DISABLEPORTEGRESSRATELIMIT,
    SWCTRL_MGR_IPCCMD_SETPORTEGRESSRATELIMIT,
    SWCTRL_MGR_IPCCMD_SETJUMBOFRAMESTATUS,
    SWCTRL_MGR_IPCCMD_SETPORTPRIOQUEUEMODE,
    SWCTRL_MGR_IPCCMD_SETPORTWRRQUEUEWEIGHT,
    SWCTRL_MGR_IPCCMD_SETWRRQUEUEWEIGHT,
    SWCTRL_MGR_IPCCMD_SETPORTNAME,
    SWCTRL_MGR_IPCCMD_SETPORTSPEEDDPXCFG,
    SWCTRL_MGR_IPCCMD_SETPORTFLOWCTRLCFG,
    SWCTRL_MGR_IPCCMD_SETPORTCAPABILITIES,
    SWCTRL_MGR_IPCCMD_SETPORTAUTONEGOTIATION,
    SWCTRL_MGR_IPCCMD_SETMIRRORTYPE,
    SWCTRL_MGR_IPCCMD_SETMIRRORSTATUS,
#if (SYS_CPNT_VLAN_MIRROR == TRUE)
    SWCTRL_MGR_IPCCMD_GETVLANMIRRORENTRY,
    SWCTRL_MGR_IPCCMD_GETNEXTVLANMIRRORENTRY,
    SWCTRL_MGR_IPCCMD_ADDVLANMIRROR,
    SWCTRL_MGR_IPCCMD_DELETEVLANMIRROR,
#endif
#if (SYS_CPNT_MAC_BASED_MIRROR == TRUE)
    SWCTRL_MGR_IPCCMD_SETMACMIRRORENTRY,
    SWCTRL_MGR_IPCCMD_DELETEMACMIRRORENTRY,
#endif
    SWCTRL_MGR_IPCCMD_SETACLMIRRORDESTPORT,                 /* SYS_CPNT_ACL_MIRROR */
    SWCTRL_MGR_IPCCMD_SETBCASTSTORMSTATUS,
    SWCTRL_MGR_IPCCMD_SETBCASTSTORMSAMPLETYPE,
    SWCTRL_MGR_IPCCMD_SETBCASTSTORMPKTRATE,
    SWCTRL_MGR_IPCCMD_SETBCASTSTORMOCTETRATE,
    SWCTRL_MGR_IPCCMD_SETBCASTSTORMPERCENT,
    SWCTRL_MGR_IPCCMD_SETMCASTSTORMSTATUS,
    SWCTRL_MGR_IPCCMD_SETMCASTSTORMSAMPLETYPE,
    SWCTRL_MGR_IPCCMD_SETMCASTSTORMPKTRATE,
    SWCTRL_MGR_IPCCMD_SETMCASTSTORMOCTETRATE,
    SWCTRL_MGR_IPCCMD_SETMCASTSTORMPERCENT,
    SWCTRL_MGR_IPCCMD_SETUNKUCASTSTORMSAMPLETYPE,
    SWCTRL_MGR_IPCCMD_SETUNKUCASTSTORMPKTRATE,
    SWCTRL_MGR_IPCCMD_SETUNKUCASTSTORMOCTETRATE,
    SWCTRL_MGR_IPCCMD_SETUNKUCASTSTORMPERCENT,
    SWCTRL_MGR_IPCCMD_SHUTDOWNSWITCH,
    SWCTRL_MGR_IPCCMD_DISABLEIPMC,
    SWCTRL_MGR_IPCCMD_ENABLEIPMC,
    SWCTRL_MGR_IPCCMD_DISABLEUMCASTIPTRAP,
    SWCTRL_MGR_IPCCMD_ENABLEUMCASTIPTRAP,
    SWCTRL_MGR_IPCCMD_DISABLEUMCASTMACTRAP,
    SWCTRL_MGR_IPCCMD_ENABLEUMCASTMACTRAP,
    SWCTRL_MGR_IPCCMD_SETUNITSBASEMACADDRTABLE,
    SWCTRL_MGR_IPCCMD_SETUNITSDEVICETYPETABLE,
    SWCTRL_MGR_IPCCMD_ADDDOT1VPROTOCOLPORTENTRY,
    SWCTRL_MGR_IPCCMD_DELDOT1VPROTOCOLPORTENTRY,
    SWCTRL_MGR_IPCCMD_SETDOT1VPROTOCOLGROUPENTRY,
    SWCTRL_MGR_IPCCMD_SETIFMAUSTATUS,
    SWCTRL_MGR_IPCCMD_SETIFMAUDEFAULTTYPE,
    SWCTRL_MGR_IPCCMD_SETIFMAUAUTONEGADMINSTATUS,
    SWCTRL_MGR_IPCCMD_SETIFMAUAUTONEGRESTART,
    SWCTRL_MGR_IPCCMD_SETIFMAUAUTONEGCAPADVERTISEDBITS,
    SWCTRL_MGR_IPCCMD_SETIFMAUAUTONEGREMOTEFAULTADVERTISED,
    SWCTRL_MGR_IPCCMD_SETPORTCOMBOFORCEDMODE,
    SWCTRL_MGR_IPCCMD_ENABLEOSPFTRAP,
    SWCTRL_MGR_IPCCMD_DISABLEOSPFTRAP,
    SWCTRL_MGR_IPCCMD_SETPORTSTATEWITHMSTIDX,
    SWCTRL_MGR_IPCCMD_ADDVLANTOMST,
    SWCTRL_MGR_IPCCMD_GETNEXTLOGICALPORT,
    SWCTRL_MGR_IPCCMD_ISMANAGEMENTPORT,
    SWCTRL_MGR_IPCCMD_LOGICALPORTTOUSERPORT,

#if (SYS_CPNT_DOT1X == TRUE)
    SWCTRL_MGR_IPCCMD_SETDOT1XAUTHTRAP,
    SWCTRL_MGR_IPCCMD_SETDOT1XAUTHCONTROLMODE,
#endif
    SWCTRL_MGR_IPCCMD_EXCUTECABLEDIAG,
    SWCTRL_MGR_IPCCMD_SETRATEBASEDSTORMCONTROL,
    SWCTRL_MGR_IPCCMD_SETRATEBASEDSTORMCONTROLRATE,
    SWCTRL_MGR_IPCCMD_SETRATEBASEDSTORMCONTROLMODE,
    SWCTRL_MGR_IPCCMD_ENABLEMLDPACKETTRAP,
    SWCTRL_MGR_IPCCMD_DIABLEMLDPACKETTRAP,
    SWCTRL_MGR_IPCCMD_EXECUTEINTERNALLOOPBACKTEST,
    SWCTRL_MGR_IPCCMD_SETPORTALIAS,
    SWCTRL_MGR_IPCCMD_SETMACLEARNINGBYPORT,                 /* SYS_CPNT_AMTR_PORT_MAC_LEARNING */

#if(SYS_CPNT_ATC_STORM == TRUE)
    SWCTRL_MGR_IPCCMD_ATC_BCAST,
    SWCTRL_MGR_IPCCMD_ATC_MCAST,
    SWCTRL_MGR_IPCCMD_ATC_BCASTACTION,
    SWCTRL_MGR_IPCCMD_ATC_MCASTACTION,
    SWCTRL_MGR_IPCCMD_ATC_BCASTAUTOCONTROLRELEASE,
    SWCTRL_MGR_IPCCMD_ATC_MCASTAUTOCONTROLRELEASE,
    SWCTRL_MGR_IPCCMD_ATC_BCASTALARMFIRETHRESHOLD,
    SWCTRL_MGR_IPCCMD_ATC_MCASTALARMFIRETHRESHOLD,
    SWCTRL_MGR_IPCCMD_ATC_BCASTALARMCLEARTHRESHOLD,
    SWCTRL_MGR_IPCCMD_ATC_MCASTALARMCLEARTHRESHOLD,
    SWCTRL_MGR_IPCCMD_ATC_BCASTALARMTHRESHOLD,
    SWCTRL_MGR_IPCCMD_ATC_MCASTALARMTHRESHOLD,
    SWCTRL_MGR_IPCCMD_ATC_BCASTAPPLYTIME,
    SWCTRL_MGR_IPCCMD_ATC_MCASTAPPLYTIME,
    SWCTRL_MGR_IPCCMD_ATC_BCASTRELEASETIME,
    SWCTRL_MGR_IPCCMD_ATC_MCASTRELEASETIME,
    SWCTRL_MGR_IPCCMD_ATC_BCASTCONTROLRELEASE,
    SWCTRL_MGR_IPCCMD_ATC_MCASTCONTROLRELEASE,
    SWCTRL_MGR_IPCCMD_ATC_GETBCASTTIMEER,
    SWCTRL_MGR_IPCCMD_ATC_GETMCASTTIMEER,
    SWCTRL_MGR_IPCCMD_ATC_GETBCASTSTORMENTRY,
    SWCTRL_MGR_IPCCMD_ATC_GETMCASTSTORMENTRY,

    SWCTRL_MGR_IPCCMD_ATC_GETRUNNINGBCASTSTORMENTRY,
    SWCTRL_MGR_IPCCMD_ATC_GETRUNNINGMCASTSTORMENTRY,
    SWCTRL_MGR_IPCCMD_ATC_GETRUNNINGBCASTTIMEER,
    SWCTRL_MGR_IPCCMD_ATC_GETRUNNINGMCASTTIMEER,

    SWCTRL_MGR_IPCCMD_ATC_GETBCASTAPPLYTIME,
    SWCTRL_MGR_IPCCMD_ATC_GETMCASTAPPLYTIME,
    SWCTRL_MGR_IPCCMD_ATC_GETBCASTRELEASETIME,
    SWCTRL_MGR_IPCCMD_ATC_GETMCASTRELEASETIME,


    SWCTRL_SetTrapBStormAlarmStatus,
    SWCTRL_GetTrapBStormAlarmStatus,
    SWCTRL_SetTrapBStormClearStatus,
    SWCTRL_GetTrapBStormClearStatus,
    SWCTRL_SetTrapBStormTCOnStatus,
    SWCTRL_GetTrapBStormTCOnStatus,
    SWCTRL_SetTrapBStormTCReleaseStatus,
    SWCTRL_GetTrapBStormTCReleaseStatus,

    SWCTRL_SetBStormSampleType,
    SWCTRL_GetNextATCBStormEntry,


    SWCTRL_SetTrapMStormAlarmStatus,
    SWCTRL_GetTrapMStormAlarmStatus,
    SWCTRL_SetTrapMStormClearStatus,
    SWCTRL_GetTrapMStormClearStatus,
    SWCTRL_SetTrapMStormTCOnStatus,
    SWCTRL_GetTrapMStormTCOnStatus,
    SWCTRL_SetTrapMStormTCReleaseStatus,
    SWCTRL_GetTrapMStormTCReleaseStatus,

    SWCTRL_DisableBStormAfterSetClearThreshold,
    SWCTRL_DisableMStormAfterSetClearThreshold,

    SWCTRL_SetMStormSampleType,
    SWCTRL_GetNextATCMStormEntry,

#endif

#if(SYS_CPNT_SWCTRL_MDIX_CONFIG == TRUE)
    SWCTRL_MGR_IPCCMD_SetMDIXMode,
    SWCTRL_MGR_IPCCMD_GetMDIXMode,
#endif
#if (SYS_CPNT_POWER_SAVE == TRUE)
    SWCTRL_MGR_IPCCMD_SETPORTPOWERSAVE,
    SWCTRL_MGR_IPCCMD_GETPORTPOWERSAVESTATUS,
    SWCTRL_MGR_IPCCMD_GETRUNNINGPORTPOWERSAVESTATUS,
#endif
    SWCTRL_MGR_IPCCMD_TRAPUNKNOWNIPMCASTTOCPU,
    SWCTRL_MGR_IPCCMD_TRAPUNKNOWNIPV6MCASTTOCPU,
    SWCTRL_MGR_IPCCMD_TRAPIPV6PIMTOCPU,
    SWCTRL_MGR_IPCCMD_TRAPIPV4PIMTOCPU,

#if (SYS_CPNT_EFM_OAM == TRUE)
    SWCTRL_MGR_IPCCMD_SETOAMLOOPBACK,
#endif

    SWCTRL_MGR_IPCCMD_DROPIPMULTICASTDATA,
    SWCTRL_MGR_IPCCMD_DROPIPV6MULTICASTDATA,
    SWCTRL_MGR_IPCCMD_ITRIMIMSETSTATUS,                     /* SYS_CPNT_ITRI_MIM */
    SWCTRL_MGR_IPCCMD_ENABLE_DHCP_PACKET_TRAP,
    SWCTRL_MGR_IPCCMD_DISABLE_DHCP_PACKET_TRAP,
    SWCTRL_MGR_IPCCMD_SETORGSPECIFICTRAPSTATUS,
    SWCTRL_MGR_IPCCMD_SETPPPOEDPKTTOCPU,
    SWCTRL_MGR_IPCCMD_SETPPPOEDPKTTOCPUPERSYSTEM,
    SWCTRL_MGR_IPCCMD_SETDOSPROTECTIONFILTER,               /* SYS_CPNT_DOS */
    SWCTRL_MGR_IPCCMD_SETDOSPROTECTIONRATELIMIT,            /* SYS_CPNT_DOS */
    SWCTRL_MGR_IPCCMD_SETRAANDRRPACKETTRAP,
    SWCTRL_MGR_IPCCMD_SETPORTRAANDRRPACKETDROP,
#if ((SYS_CPNT_DHCPV6 == TRUE)||(SYS_CPNT_DHCPV6SNP == TRUE))
    SWCTRL_MGR_IPCCMD_ENABLE_DHCP6_PACKET_TRAP,
    SWCTRL_MGR_IPCCMD_DISABLE_DHCP6_PACKET_TRAP,
#endif

#if (SYS_CPNT_NDSNP == TRUE)
    SWCTRL_MGR_IPCCMD_ENABLE_ND_PACKET_TRAP,
    SWCTRL_MGR_IPCCMD_DISABLE_ND_PACKET_TRAP,
#endif
    SWCTRL_MGR_IPCCMD_SET_PKT_TRAP_STATUS,
    SWCTRL_MGR_IPCCMD_SET_PORT_PKT_TRAP_STATUS,
    SWCTRL_MGR_IPCCMD_GETPSECHECKSTATUS,
    SWCTRL_MGR_IPCCMD_GETPDPORTSTATUS,
    SWCTRL_MGR_IPCCMD_SETPSECHECKSTATUS,

#if (SYS_CPNT_SFLOW == TRUE)
    SWCTRL_MGR_IPCCMD_SET_SFLOW_PORT_SAMPLING_RATE,
#endif

    SWCTRL_MGR_IPCCMD_SETTRUNKMAXNUMOFACTIVEPORTS,          /* SYS_CPNT_TRUNK_MAX_ACTIVE_PORTS_CONFIGURABLE */
    SWCTRL_MGR_IPCCMD_SETPORTLEARNINGSTATUS,
    SWCTRL_MGR_IPCCMD_SETPORTPFCSTATUS,                     /*SYS_CPNT_PFC*/
    SWCTRL_MGR_IPCCMD_UPDATEPFCPRIMAP,                      /*SYS_CPNT_PFC*/
    SWCTRL_MGR_IPCCMD_SETPORTCOSGROUPMAPPING,               /* SYS_CPNT_ETS */
    SWCTRL_MGR_IPCCMD_SETPORTCOSGROUPSCHEDULINGMETHOD,      /* SYS_CPNT_ETS */
    SWCTRL_MGR_IPCCMD_SETQCNCNMPRIORITY,                    /* SYS_CPNT_CN */
    SWCTRL_MGR_IPCCMD_SETPORTQCNCPQ,                        /* SYS_CPNT_CN */
    SWCTRL_MGR_IPCCMD_SETPORTQCNEGRCNTAGREMOVAL,            /* SYS_CPNT_CN */
    SWCTRL_MGR_IPCCMD_GETPORTQCNCPID,                       /* SYS_CPNT_CN */
    SWCTRL_MGR_IPCCMD_SETMIMSERVICE,                        /* SYS_CPNT_MAC_IN_MAC */
    SWCTRL_MGR_IPCCMD_SETMIMSERVICEPORT,                    /* SYS_CPNT_MAC_IN_MAC */
    SWCTRL_MGR_IPCCMD_SETMIMSERVICEPORTLEARNINGSTATUSFORSTATIONMOVE,    /* SYS_CPNT_MAC_IN_MAC & SYS_CPNT_IAAS */
    SWCTRL_MGR_IPCCMD_SETCPURATELIMIT,

#if (TRUE == SYS_CPNT_APP_FILTER)
    SWCTRL_MGR_IPCCMD_DROP_PORT_CDP_PACKE,                 /* SYS_CPNT_APP_FILTER_CDP */
    SWCTRL_MGR_IPCCMD_DROP_PORT_PVST_PACKET,               /* SYS_CPNT_APP_FILTER_PVST */
#endif /* #if (TRUE == SYS_CPNT_APP_FILTER) */

    SWCTRL_MGR_IPCCMD_GETNEXTPDPORTSTATUS,
    SWCTRL_MGR_IPCCMD_SETGLOBALSTORMSAMPLETYPE,             /* SYS_CPNT_SWCTRL_GLOBAL_STORM_SAMPLE_TYPE */
#if (SYS_CPNT_VRRP == TRUE)
    SWCTRL_MGR_IPCCMD_SET_VRRP_TRAP,
#endif
    SWCTRL_MGR_IPCCMD_SETPORTSFPDDMTRAPENABLE,
    SWCTRL_MGR_IPCCMD_SETPORTSFPDDMTHRESHOLD,
    SWCTRL_MGR_IPCCMD_SETPORTSFPDDMTHRESHOLDFORWEB,
    SWCTRL_MGR_IPCCMD_SETPORTSFPDDMTHRESHOLDAUTOMODE,
    SWCTRL_MGR_IPCCMD_SETPORTSFPDDMTHRESHOLDDEFAULT,

#if (SYS_CPNT_MAU_MIB == TRUE)
    SWCTRL_MGR_IPCCMD_GETIFMAUENTRY,
    SWCTRL_MGR_IPCCMD_GETNEXTIFMAUENTRY,
    SWCTRL_MGR_IPCCMD_GETIFMAUAUTONEGENTRY,
    SWCTRL_MGR_IPCCMD_GETNEXTIFMAUAUTONEGENTRY,
    SWCTRL_MGR_IPCCMD_GETIFJACKENTRY,
    SWCTRL_MGR_IPCCMD_GETNEXTIFJACKENTRY,
#endif

    SWCTRL_MGR_IPCCMD_SETPORTEGRESSBLOCK,
    SWCTRL_MGR_IPCCMD_SETPORTEGRESSBLOCKEX,

#if (SYS_CPNT_HASH_SELECTION == TRUE)
    SWCTRL_MGR_IPCCMD_BINDHASHSELFORSERVICE,
    SWCTRL_MGR_IPCCMD_UNBINDHASHSELFORSERVICE,
    SWCTRL_MGR_IPCCMD_ADDHASHSELECTION,
    SWCTRL_MGR_IPCCMD_REMOVEHASHSELECTION,
#endif

#if (SYS_CPNT_SWCTRL_SWITCH_MODE_CONFIGURABLE == TRUE)
    SWCTRL_MGR_IPCCMD_SETSWITCHINGMODE,
#endif

    SWCTRL_MGR_IPCCMD_SETPORTFEC,                           /* SYS_CPNT_SWCTRL_FEC */

#if(SYS_CPNT_WRED == TRUE)
    SWCTRL_MGR_IPCCMD_RANDOM_DETECT,
#endif

    SWCTRL_MGR_IPCCMD_FOLLOWISASYNCHRONISMIPC,  /* attention please, following is asyncronouse */

};

enum
{
    SWCTRL_OM_IPCCMD_ISPORTLINKUP,
    SWCTRL_OM_IPCCMD_GETPORTINFO,
    SWCTRL_OM_IPCCMD_GETNEXTPORTINFO,
    SWCTRL_OM_IPCCMD_GETRUNNINGPORTINFO,
    SWCTRL_OM_IPCCMD_GETPORTLINKSTATUS,
    SWCTRL_OM_IPCCMD_GETPORTSTATUS,
    SWCTRL_OM_IPCCMD_GETSWITCHLOOPBACKTESTFAILUREPORTS,
    SWCTRL_OM_IPCCMD_GETUNITPORTNUMBER,
    SWCTRL_OM_IPCCMD_GETSYSTEMPORTNUMBER,
    SWCTRL_OM_IPCCMD_GETCPUMAC,
    SWCTRL_OM_IPCCMD_GETPORTMAC,
    SWCTRL_OM_IPCCMD_GETLASTCHANGETIME,
    SWCTRL_OM_IPCCMD_ISSECURITYPORT,
    SWCTRL_OM_IPCCMD_GETPORTPRIVATEMODE,
    SWCTRL_OM_IPCCMD_GETNEXTPORTPRIVATEMODE,
    SWCTRL_OM_IPCCMD_GETRUNNINGPORTPRIVATEMODE,
    SWCTRL_OM_IPCCMD_GETPRIVATEVLAN,
    SWCTRL_OM_IPCCMD_GETPRIVATEVLANSTATUS,
    SWCTRL_OM_IPCCMD_GETRUNNINGPRIVATEVLANSTATUS,
    SWCTRL_OM_IPCCMD_GETRUNNINGPRIVATEVLANUPLINKPORTLIST,
    SWCTRL_OM_IPCCMD_ISPORTPRIVATEVLANUPLINKMEMBER,
    SWCTRL_OM_IPCCMD_ISPORTPRIVATEVLANDOWNLINKMEMBER,
    SWCTRL_OM_IPCCMD_GETRUNNINGPRIVATEVLANDOWNLINKPORTLIST,
    SWCTRL_OM_IPCCMD_GETPRIVATEVLANBYSESSIONID,
    SWCTRL_OM_IPCCMD_GETRUNNINGPRIVATEVLANPORTLISTBYSESSIONID,
    SWCTRL_OM_IPCCMD_GETNEXTSESSIONFROMPRIVATEVLANPORTLIST,
    SWCTRL_OM_IPCCMD_ISUSERPORTJOINPRIVATEVLANTOTRUNK,
    SWCTRL_OM_IPCCMD_GETRUNNINGPRIVATEVLANUPLINKTOUPLINKSTATUS,
    SWCTRL_OM_IPCCMD_GETPRIVATEVLANUPLINKTOUPLINKSTATUS,
    SWCTRL_OM_IPCCMD_GETRUNNINGPORTINGRESSRATELIMITSTATUS,
    SWCTRL_OM_IPCCMD_GETRUNNINGPORTEGRESSRATELIMITSTATUS,
    SWCTRL_OM_IPCCMD_GETJUMBOFRAMESTATUS,
    SWCTRL_OM_IPCCMD_GETRUNNINGJUMBOFRAMESTATUS,
    SWCTRL_OM_IPCCMD_USERPORTTOLOGICALPORT,
    SWCTRL_OM_IPCCMD_USERPORTTOIFINDEX,
    SWCTRL_OM_IPCCMD_USERPORTTOTRUNKPORT,
    SWCTRL_OM_IPCCMD_LOGICALPORTTOUSERPORT,
    SWCTRL_OM_IPCCMD_LPORTTOACTIVEUPORT,
    SWCTRL_OM_IPCCMD_TRUNKIDTOLOGICALPORT,
    SWCTRL_OM_IPCCMD_GETNEXTLOGICALPORT,
    SWCTRL_OM_IPCCMD_LOGICALPORTEXISTING,
    SWCTRL_OM_IPCCMD_USERPORTEXISTING,
    SWCTRL_OM_IPCCMD_LOGICALPORTISTRUNKPORT,
    SWCTRL_OM_IPCCMD_ISTRUNKMEMBER,
    SWCTRL_OM_IPCCMD_GETTRUNKIFINDEXBYUPORT,
    SWCTRL_OM_IPCCMD_GETPORTPRIOQUEUEMODE,
    SWCTRL_OM_IPCCMD_GETPORTSTRICTQUEUEMAP,
    SWCTRL_OM_IPCCMD_GETNEXTPORTPRIOQUEUEMODE,
    SWCTRL_OM_IPCCMD_GETRUNNINGPORTPRIOQUEUEMODE,
    SWCTRL_OM_IPCCMD_GETNEXTRUNNINGPORTPRIOQUEUEMODE,
    SWCTRL_OM_IPCCMD_GETPORTWRRQUEUEWEIGHT,
    SWCTRL_OM_IPCCMD_GETNEXTPORTWRRQUEUEWEIGHT,
    SWCTRL_OM_IPCCMD_GETRUNNINGPORTWRRQUEUEWEIGHT,
    SWCTRL_OM_IPCCMD_GETNEXTRUNNINGPORTWRRQUEUEWEIGHT,
    SWCTRL_OM_IPCCMD_GETPORTENTRY,
    SWCTRL_OM_IPCCMD_GETNEXTPORTENTRY,
    SWCTRL_OM_IPCCMD_GETMIRRORENTRY,
    SWCTRL_OM_IPCCMD_GETNEXTMIRRORENTRY,
    SWCTRL_OM_IPCCMD_GETNEXTRUNNINGMIRRORENTRY,
    SWCTRL_OM_IPCCMD_GETBCASTSTORMENTRY,
    SWCTRL_OM_IPCCMD_GETNEXTBCASTSTORMENTRY,
    SWCTRL_OM_IPCCMD_GETMCASTSTORMENTRY,
    SWCTRL_OM_IPCCMD_GETNEXTMCASTSTORMENTRY,
    SWCTRL_OM_IPCCMD_GETUNKUCASTSTORMENTRY,
    SWCTRL_OM_IPCCMD_GETNEXTUNKUCASTSTORMENTRY,
    SWCTRL_OM_IPCCMD_GETPORTSTORMGRANULARITY,
    SWCTRL_OM_IPCCMD_UIGETUNITPORTNUMBER,
    SWCTRL_OM_IPCCMD_UIUSERPORTTOLOGICALPORT,
    SWCTRL_OM_IPCCMD_UIUSERPORTTOIFINDEX,
    SWCTRL_OM_IPCCMD_UIUSERPORTTOTRUNKPORT,
    SWCTRL_OM_IPCCMD_UIUSERPORTEXISTING,
    SWCTRL_OM_IPCCMD_GETSUPPORTEDPORTCOMBOFORCEDMODE,
    SWCTRL_OM_IPCCMD_GETDEFAULTPORTCOMBOFORCEDMODE,
    SWCTRL_OM_IPCCMD_GETDEFAULTPORTCOMBOFORCEDMODESFPSPEED,
    SWCTRL_OM_IPCCMD_GETPORTCOMBOFORCEDMODE,
    SWCTRL_OM_IPCCMD_GETRUNNINGPORTCOMBOFORCEDMODE,
#if (SYS_CPNT_COMBO_PORT_FORCED_MODE_SFP_SPEED == TRUE)
    SWCTRL_OM_IPCCMD_GETPORTCOMBOFORCEDMODESPEED,
    SWCTRL_OM_IPCCMD_GETRUNNINGPORTCOMBOFORCEDMODESPEED,
#endif
    SWCTRL_OM_IPCCMD_GETCABLEDIAG,
    SWCTRL_OM_IPCCMD_GETNEXTCABLEDIAG,
    SWCTRL_OM_IPCCMD_GETPORTMAXFRAMESIZE,
    SWCTRL_OM_IPCCMD_GETDOT1QTUNNELSTATUS,
    SWCTRL_OM_IPCCMD_GETPORTDOT1QTUNNELMODE,
    SWCTRL_OM_IPCCMD_GETPORTDOT1QTUNNELTPID,

#if (SYS_CPNT_MAC_BASED_MIRROR == TRUE)
    SWCTRL_OM_IPCCMD_GETEXACTMACADDRMIRRORENTRY,
    SWCTRL_OM_IPCCMD_GETNEXTMACADDRMIRRORENTRY,
    SWCTRL_OM_IPCCMD_GETNEXTMACADDRMIRRORENTRYFORSNMP,
    SWCTRL_OM_IPCCMD_ISEXISTEDMACADDRMIRRORENTRY,
#endif

    SWCTRL_OM_IPCCMD_GETRATEBASEDSTORMCONTROL,
    SWCTRL_OM_IPCCMD_GETNEXTRATEBASEDSTORMCONTROL,
    SWCTRL_OM_IPCCMD_GETRUNNINGRATEBASEDSTORMCONTROLMODE,
    SWCTRL_OM_IPCCMD_GETRUNNINGRATEBASEDSTORMCONTROLRATE,
    SWCTRL_OM_IPCCMD_GETINTERNALLOOPBACKTESTRESULT,
    SWCTRL_OM_IPCCMD_GETNEXTINTERNALLOOPBACKTESTRESULT,

    SWCTRL_OM_IPCCMD_GETVLANANDMACMIRRORDESTPORT,
    SWCTRL_OM_IPCCMD_ITRIMIMGETSTATUS,                      /* SYS_CPNT_ITRI_MIM */
    SWCTRL_OM_IPCCMD_ITRIMIMGETNEXTSTATUS,                  /* SYS_CPNT_ITRI_MIM */
    SWCTRL_OM_IPCCMD_ITRIMIMGETRUNNINGSTATUS,               /* SYS_CPNT_ITRI_MIM */
    SWCTRL_OM_IPCCMD_GETTRUNKBALANCEMODE,
    SWCTRL_OM_IPCCMD_GETRUNNINGTRUNKBALANCEMODE,
    SWCTRL_OM_IPCCMD_GET_ACTIVE_TRUNK_MEMBER,
    SWCTRL_OM_IPCCMD_GETTRUNKMAXNUMOFACTIVEPORTS,           /* SYS_CPNT_TRUNK_MAX_ACTIVE_PORTS_CONFIGURABLE */
    SWCTRL_OM_IPCCMD_GETRUNNINGTRUNKMAXNUMOFACTIVEPORTS,    /* SYS_CPNT_TRUNK_MAX_ACTIVE_PORTS_CONFIGURABLE */
    SWCTRL_OM_IPCCMD_GETPORTLEARNINGSTATUSEX,
    SWCTRL_OM_IPCCMD_ISPORTFLOWCONTROLENABLED,              /* SYS_CPNT_PFC */
    SWCTRL_OM_IPCCMD_GETCPURATELIMIT,
    SWCTRL_OM_IPCCMD_GETPORTABILITY,
    SWCTRL_OM_IPCCMD_GETRUNNINGGLOBALSTORMSAMPLETYPE,       /* SYS_CPNT_SWCTRL_GLOBAL_STORM_SAMPLE_TYPE */
    SWCTRL_OM_IPCCMD_GETPORTSFPPRESENT,
    SWCTRL_OM_IPCCMD_GETPORTSFPINFO,
    SWCTRL_OM_IPCCMD_GETPORTSFPDDMINFO,
    SWCTRL_OM_IPCCMD_GETPORTSFPDDMINFOMEASURED,
    SWCTRL_OM_IPCCMD_GETPORTSFPENTRY,
    SWCTRL_OM_IPCCMD_GETNEXTPORTSFPENTRY,
    SWCTRL_OM_IPCCMD_GETPORTSFPDDMENTRY,
    SWCTRL_OM_IPCCMD_GETNEXTPORTSFPDDMENTRY,
    SWCTRL_OM_IPCCMD_GETPORTSFPDDMTHRESHOLD,
    SWCTRL_OM_IPCCMD_GETPORTSFPDDMTHRESHOLDENTRY,
    SWCTRL_OM_IPCCMD_GETNEXTPORTSFPDDMTHRESHOLDENTRY,
    SWCTRL_OM_IPCCMD_GETPORTSFPDDMTHRESHOLDSTATUS,
    SWCTRL_OM_IPCCMD_GETPORTSFPDDMTHRESHOLDAUTOMODE,
    SWCTRL_OM_IPCCMD_GETPORTSFPDDMTRAPENABLE,
    SWCTRL_OM_IPCCMD_GETRUNNINGPORTSFPDDMTHRESHOLDENTRY,
#if (SYS_CPNT_HASH_SELECTION == TRUE)
    SWCTRL_OM_IPCCMD_GETHASHSELECTIONBLOCKINFO,
#endif
#if (SYS_CPNT_SWCTRL_SWITCH_MODE_CONFIGURABLE == TRUE)
    SWCTRL_OM_IPCCMD_GETSWITCHINGMODE,
#endif
#if(SYS_CPNT_WRED == TRUE)
    SWCTRL_OM_IPCCMD_RANDOMDETECT,                          /* SYS_CPNT_WRED */
#endif
    SWCTRL_OM_IPCCMD_FOLLOWISASYNCHRONISMIPC, /* attention please, following is asyncronouse */

};

enum SWCTRL_Wrr_Status_E
{
    SWCTRL_WRR_DIABLE = 0,
    SWCTRL_WRR_ENABLE
};

typedef enum SWCTRL_Lport_Type_E
{
    SWCTRL_LPORT_UNKNOWN_PORT = 0,
    SWCTRL_LPORT_NORMAL_PORT,                   /* the normal port */
    SWCTRL_LPORT_TRUNK_PORT,                    /* the trunk port */
    SWCTRL_LPORT_TRUNK_PORT_MEMBER,             /* a member of a trunk */
#if (SYS_CPNT_VXLAN == TRUE)
    SWCTRL_LPORT_VXLAN_PORT,                    /* a VXLAN tunnel port */
#endif
} SWCTRL_Lport_Type_T;

/* Defines level of ifOperStatus 'dormant'
 *
 * List as below is in the order from lower to higher.
 * 'lower' means closer to 'ifOperStatusDown'
 */
typedef enum
{
    SWCTRL_OPER_DORMANT_LV_LOWERLAYER,
    SWCTRL_OPER_DORMANT_LV_DOT1X,
    SWCTRL_OPER_DORMANT_LV_LACP,
    SWCTRL_OPER_DORMANT_LV_MLAG,
    SWCTRL_OPER_DORMANT_LV_MAX,
} SWCTRL_OperDormantLevel_T;

/* Defines level of ifOperStatus 'dormant'
 *
 * LEAVE
 *   makes ifOperStatus from 'dormant' to 'upper dormant' or 'up'
 * ENTER
 *   makes ifOperStatus from 'up' to 'dormant'
 */
typedef enum
{
    SWCTRL_OPER_DORMANT_EV_LEAVE,
    SWCTRL_OPER_DORMANT_EV_ENTER,
} SWCTRL_OperDormantEvent_T;

#if (SYS_CPNT_SWCTRL_CABLE_DIAG == TRUE)
#if (SYS_CPNT_SWCTRL_CABLE_DIAG_CHIP == SYS_CPNT_SWCTRL_CABLE_DIAG_BROADCOM)
enum SWCTRL_Cable_Status_E
{
#if 1
    CABLE_NORMAL_CABLE = 0,    /* OK */
    CABLE_OPEN_CABLE,
    CABLE_SHORT_CABLE,
    CABLE_OPEN_SHORT_CABLE,
    CABLE_CROSSTALK_CABLE,
    CABLE_UNKNOWN_CABLE,
    CABLE_DIAG_NOT_SUPPORTED,
    CABLE_NOT_TESTED_YET
#else /* for EDD project */
    CABLE_NOT_TESTED_YET = 0,
    CABLE_NORMAL_CABLE,    /* OK */
    CABLE_OPEN_CABLE,
    CABLE_SHORT_CABLE,
    CABLE_OPEN_SHORT_CABLE,
    CABLE_CROSSTALK_CABLE,
    CABLE_UNKNOWN_CABLE,
    CABLE_DIAG_NOT_SUPPORTED
#endif
} SWCTRL_Cable_Status_T;

#elif (SYS_CPNT_SWCTRL_CABLE_DIAG_CHIP == SYS_CPNT_SWCTRL_CABLE_DIAG_MARVELL)
#else
#error "Chip not defined!"
#endif
#endif

/* DOS protection
 *
 * need sync to SWDRV_DOS_FILTER_ and SWDRV_DOS_RATELIMIT_
 */
typedef enum
{
    SWCTRL_DOS_FILTER_LAND,
    SWCTRL_DOS_FILTER_SMURF,
    SWCTRL_DOS_FILTER_TCP_NULL_SCAN,
    SWCTRL_DOS_FILTER_TCP_SCAN,
    SWCTRL_DOS_FILTER_TCP_SYN_FIN_SCAN,
    SWCTRL_DOS_FILTER_TCP_UDP_PORT_ZERO,
    SWCTRL_DOS_FILTER_TCP_XMAS_SCAN,
} SWCTRL_DosProtectionFilter_T;

typedef enum
{
    SWCTRL_DOS_RATELIMIT_ECHO_CHARGEN,
    SWCTRL_DOS_RATELIMIT_TCP_FLOODING,
    SWCTRL_DOS_RATELIMIT_UDP_FLOODING,
    SWCTRL_DOS_RATELIMIT_WIN_NUKE,
} SWCTRL_DosProtectionRateLimit_T;

#if (SYS_CPNT_VLAN_MIRROR == TRUE)
enum SWCTRL_VLAN_Mirror_Status_E
{
    SWCTRL_VLAN_MIRROR_DISABLE = 0,
    SWCTRL_VLAN_MIRROR_ENABLE
};
#endif /* End of #if (SYS_CPNT_VLAN_MIRROR == TRUE) */

#if (SYS_CPNT_VLAN_MIRROR == TRUE) || (SYS_CPNT_MAC_BASED_MIRROR == TRUE) || (SYS_CPNT_ACL_MIRROR == TRUE)
typedef enum 
{
    SWCTRL_COMMON_MIRROR_TYPE_RX = 1,
    SWCTRL_COMMON_MIRROR_TYPE_TX,
    SWCTRL_COMMON_MIRROR_TYPE_BOTH
} SWCTRL_CommonMirrorType_T;
#endif

typedef struct
{
    UI32_T  vlan_status;                                                    /* enable /disable */
    UI8_T   uplink_ports[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];    /* up port list */
    UI8_T   downlink_ports[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];  /* down port list */
} SWCTRL_PrivateVlan_T;


/* For extend trunk port information database, i.e. the characteristics just only for trunk port
 */
typedef struct
{
    UI32_T              member_number;    /* number of trunk member */
    UI32_T              max_num_of_active_ports;
    BOOL_T              is_static;        /* static: set by user, dynamic: set by LACP */
    SYS_TYPE_Uport_T    member_list[SYS_ADPT_MAX_NBR_OF_PORT_PER_TRUNK]; /* trunk member list */
    UI8_T               admin_active_members[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK*SYS_ADPT_NBR_OF_BYTE_FOR_1BIT_UPORT_LIST];
    UI8_T               oper_active_members[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK*SYS_ADPT_NBR_OF_BYTE_FOR_1BIT_UPORT_LIST];
} SWCTRL_TrunkPortExtInfo_T;

typedef struct
{
    UI32_T        source_port_count;    /* source port count for current MTP. 0 for MTP can be change. */
    UI32_T        MTP;                            /* MTP port ID */
} SWCTRL_MTP_INFO_T;

/*  for CLI use
 */
typedef struct
{
    UI32_T        dst_port;    /*key-----first key*/
    UI32_T        src_port;    /*key*/
    UI8_T         direction;
} SWCTRL_MIRROR_MTP_T;


/*  for CLI use
 */
typedef struct
{
    UI32_T  port_type;                  /* TX, FX, 1000SX, ... */
    UI32_T  admin_state;                /* port Enable or Disable  */
    UI32_T  mtu;                        /*added by jinhua.wei*/
    UI32_T  untagged_max_frame_sz;      /* max frame size for untagged frames */
    UI32_T  tagged_max_frame_sz;      /* max frame size for tagged frames */
    UI32_T  autoneg_capability;         /* autonegotiation capability */
    UI32_T  autoneg_state;              /* ENABLE/DISABLE autonegotiation state */
    UI32_T  speed_duplex_cfg;           /* config of speed and duplex */
    UI32_T  speed_duplex_oper;          /* operation status of speed and duplex */
    UI32_T  bandwidth;                  /* in kbps */
    UI32_T  uptime;                     /* port up time, in ticks */
    UI32_T  link_status;                /* link UP/DOWN */
    UI32_T  shutdown_reason;            /* shutdown reason */
    UI32_T  link_oper_status;           /* link operation status */
    UI32_T  link_change_trap;           /* link trap enable/disable */
    UI32_T  flow_control_cfg;           /* ENABLE/DISABLE flow control */
    UI32_T  flow_control_oper;          /* last flow control status */
    UI32_T  link_oper_status_last_change; /* last sysUpTime of link up/down */
    UI32_T  bsctrl_state;               /* ENABLE/DISABLE boradcast storm control */
    UI32_T  msctrl_state;               /* ENABLE/DISABLE multicast storm control */
/* Unknown Unicast(DLF) Storm
 */
#if (SYS_CPNT_STORM_MODE & SYS_CPNT_STORM_UNKNOWN_USTORM)
    UI32_T  unknown_usctrl_state;       /* ENABLE/DISABLE unknown unicast(DLF) storm control */
    UI32_T  unknown_ucast_rate_limit;   /* the threshold of unknown unicast(DLF) storm control */
    UI32_T  unknown_ucast_rate_mode;    /* the mode of unknown unicast(DLF) threshold */
#endif
    UI32_T  bcast_rate_limit;           /* the threshold of broadcast storm control */
    UI32_T  mcast_rate_limit;           /* the threshold of multicast storm control */
    UI32_T  bcast_rate_mode;            /* the mode of broadcast threshold */
    UI32_T  mcast_rate_mode;            /* the mode of multicast threshold */
    UI32_T  ingress_rate_limit;         /* Ingress rate limit value  */
    UI32_T  ingress_rate_limit_status;  /* Ingress rate limit status */
    UI32_T  egress_rate_limit;          /* Egress rate limit value   */
    UI32_T  egress_rate_limit_status;   /* Egress rate limit status  */
    UI8_T   port_name[MAXSIZE_ifAlias+1];  /* the name user assigned */
    UI8_T   port_alias[MAXSIZE_ifAlias+1];  /* the alias user assigned */
    UI32_T  medium_forced_mode;         /* port medium forced mode */
    UI32_T  forced_1000t_mode;          /* force 1000T speed duplex configure mode */

#if(SYS_CPNT_AMTR_PORT_MAC_LEARNING == TRUE)/*Tony.Lei*/
    BOOL_T port_macaddr_learning ;
#endif
#ifdef VS2512A
    UI32_T  ethertype;
#endif

#if (SYS_CPNT_SWCTRL_SWITCH_MODE_CONFIGURABLE == TRUE)
    UI32_T switch_mode;
#endif

#if (SYS_CPNT_SWCTRL_FEC == TRUE)
    UI32_T port_fec_mode;
    UI32_T port_fec_status;
#endif

    /* for GetNextRunning */
    BOOL_T  admin_state_changed;        /* TRUE: changed,  FALSE: no changed  */
    UI32_T  autoneg_capability_changed;
    BOOL_T  autoneg_state_changed;
    BOOL_T  speed_duplex_cfg_changed;
    BOOL_T  link_change_trap_changed;
    BOOL_T  flow_control_cfg_changed;
    BOOL_T  bsctrl_state_changed;
    BOOL_T  msctrl_state_changed;
/* Unknown Unicast(DLF) Storm
 */
#if (SYS_CPNT_STORM_MODE & SYS_CPNT_STORM_UNKNOWN_USTORM)
    BOOL_T  unknown_usctrl_state_changed;
    BOOL_T  unknown_ucast_rate_limit_changed;
    BOOL_T  unknown_ucast_rate_mode_changed;
#endif
    BOOL_T  bcast_rate_limit_changed;
    BOOL_T  mcast_rate_limit_changed;
    BOOL_T  bcast_rate_mode_changed;
    BOOL_T  mcast_rate_mode_changed;
    BOOL_T  port_name_changed;
    BOOL_T  port_alias_changed;
    BOOL_T  ingress_rate_limit_changed;
    BOOL_T  ingress_rate_limit_state_changed;
    BOOL_T  egress_rate_limit_state_changed;
    BOOL_T  egress_rate_limit_changed;
    BOOL_T  medium_forced_mode_changed;
    BOOL_T  forced_1000t_mode_changed;
    BOOL_T  port_mtu_changed;
    BOOL_T  mac_learning_count_changed;
#if(SYS_CPNT_AMTR_PORT_MAC_LEARNING == TRUE)/*Tony.Lei*/
    BOOL_T  port_macaddr_learning_changed;
#endif
    UI32_T  rate_based_storm_rate;
    UI32_T  rate_based_storm_mode;

/* MDIX mode */
#if (SYS_CPNT_SWCTRL_MDIX_CONFIG == TRUE)
    UI32_T port_MDIX_mode;
#endif

/* MDIX mode */
#if (SYS_CPNT_SWCTRL_MDIX_CONFIG == TRUE)
    UI32_T port_MDIX_mode_changed;
#endif

#if (SYS_CPNT_SWCTRL_SWITCH_MODE_CONFIGURABLE == TRUE)
    BOOL_T switch_mode_changed;
#endif

#if (SYS_CPNT_SWCTRL_FEC == TRUE)
    BOOL_T port_fec_mode_changed;
#endif
#if(SYS_CPNT_WRED == TRUE)
   SWCTRL_OM_RandomDetect_T random_detect;
#endif
} Port_Info_T;


/*  for SNMP use
 */
typedef struct
{
    /* key */
    UI32_T port_index;
    UI32_T port_mtu;  /*added by jinhua.wei*/
    UI8_T  port_name[MAXSIZE_portName+1];
    UI8_T  port_alias[MAXSIZE_ifAlias+1];
    UI32_T port_type;
    UI32_T port_speed_dpx_cfg;
    UI32_T port_flow_ctrl_cfg;
    UI32_T port_capabilities;
    UI32_T port_autonegotiation;
    UI32_T port_speed_dpx_status;
    UI32_T port_flow_ctrl_status;
    UI32_T port_trunk_index;
    UI32_T port_forced_mode;
    UI32_T port_forced_1000t_mode;
#if (SYS_CPNT_SWCTRL_MDIX_CONFIG == TRUE)
    UI32_T port_MDIX_mode;
#endif
#if(SYS_CPNT_AMTR_PORT_MAC_LEARNING == TRUE)/*Tony.Lei*/
    BOOL_T port_macaddr_learning;
#endif
#if (SYS_CPNT_SUPPORT_COMBO_PORT_NO_NEG_KEEP == TRUE)
    UI32_T port_combo_copper_speed_dpx_cfg;  /* internal using for restore to port_speed_dpx_cfg,
                                                config of speed and duplex of this combo's copper port */
    UI32_T port_combo_fiber_speed_dpx_cfg;   /* internal using for restore to port_speed_dpx_cfg,
                                                config of speed and duplex of this combo's fiber port */
#endif

#ifdef VS2512A
    UI32_T ethertype;
#endif
#if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION == TRUE)
#if (SYS_DFLT_TRAFFIC_SEG_METHOD == SYS_DFLT_TRAFFIC_SEG_METHOD_PORT_PRIVATE_MODE)
    UI32_T port_private_mode;
#endif
#endif

#if (SYS_CPNT_COMBO_PORT_FORCED_MODE_SFP_SPEED == TRUE)
    UI32_T port_combo_force_mode_speed_cfg;        /*store the latest fiber force mode speed*/
#endif

    UI32_T uptime;                          /* port up time, in ticks */

#if (SYS_CPNT_SWCTRL_SWITCH_MODE_CONFIGURABLE == TRUE)
    UI32_T switch_mode;
#endif
#if(SYS_CPNT_WRED == TRUE)
    SWCTRL_OM_RandomDetect_T random_detect;
#endif
}SWCTRL_PortEntry_T;

typedef struct
{
    /* key */
    UI32_T mirror_destination_port;
    UI32_T mirror_source_port;

    UI32_T mirror_type;
    UI32_T mirror_status;
}SWCTRL_MirrorEntry_T;

#if (SYS_CPNT_VLAN_MIRROR == TRUE)
typedef struct
{
    UI32_T mirror_dest_port;    /* a mirroring port */
    UI32_T mirror_source_vlan;  /* a mirrored vlan */
    UI32_T mirror_vlan_status;  /* a vlan mirror status */
}SWCTRL_VlanMirrorEntry_T;
#endif /* End of #if (SYS_CPNT_VLAN_MIRROR == TRUE)*/

typedef struct
{
    /* key */
    UI32_T bcast_storm_ifindex;

    UI32_T bcast_storm_status;
    UI32_T bcast_storm_sample_type;
    UI32_T bcast_storm_pkt_rate;
    UI32_T bcast_storm_octet_rate;
    UI32_T bcast_storm_percent;
}SWCTRL_BcastStormEntry_T;


typedef struct
{
    /* key */
    UI32_T mcast_storm_ifindex;

    UI32_T mcast_storm_status;
    UI32_T mcast_storm_sample_type;
    UI32_T mcast_storm_pkt_rate;
    UI32_T mcast_storm_octet_rate;
    UI32_T mcast_storm_percent;
}SWCTRL_McastStormEntry_T;

/* Unknown Unicast(DLF) Storm Entry
 */
typedef struct
{
    /* key */
    UI32_T unknown_ucast_storm_ifindex;

    UI32_T unknown_ucast_storm_status;
    UI32_T unknown_ucast_storm_sample_type;
    UI32_T unknown_ucast_storm_pkt_rate;
    UI32_T unknown_ucast_storm_octet_rate;
    UI32_T unknown_ucast_storm_percent;
}SWCTRL_UnknownUcastStormEntry_T;


/* IfMauEntry: From RFC-3636
 */
typedef struct
{
    UI32_T    ifMauIfIndex;                  /*key*/
    UI32_T    ifMauIndex;                    /*key*/

    UI32_T    ifMauType;
    UI32_T    ifMauStatus;
    UI32_T    ifMauMediaAvailable;
    UI32_T    ifMauMediaAvailableStateExits;
    UI32_T    ifMauJabberState;
    UI32_T    ifMauJabberingStateEnters;
    UI32_T    ifMauFalseCarriers;
  /*UI32_T    ifMauTypeList;*/               /*DEPRECATED*/
    UI32_T    ifMauDefaultType;
    UI32_T    ifMauAutoNegSupported;
    UI8_T     ifMauTypeListBits[6];
}   SWCTRL_IfMauEntry_T;

/* IfMauAutoNegEntry: From RFC-3636
 */
typedef struct
{
    UI32_T    ifMauIfIndex;                  /*key*/
    UI32_T    ifMauIndex;                    /*key*/

    UI32_T    ifMauAutoNegAdminStatus;
    UI32_T    ifMauAutoNegRemoteSignaling;
    UI32_T    ifMauAutoNegConfig;
  /*UI32_T    ifMauAutoNegCapability;   */   /*DEPRECATED*/
  /*UI32_T    ifMauAutoNegCapAdvertised;*/   /*DEPRECATED*/
  /*UI32_T    ifMauAutoNegCapReceived;  */   /*DEPRECATED*/
    UI32_T    ifMauAutoNegRestart;
    UI32_T    ifMauAutoNegCapabilityBits;
    UI32_T    ifMauAutoNegCapAdvertisedBits;
    UI32_T    ifMauAutoNegCapReceivedBits;
    UI32_T    ifMauAutoNegRemoteFaultAdvertised;
    UI32_T    ifMauAutoNegRemoteFaultReceived;
}   SWCTRL_IfMauAutoNegEntry_T;

/* IfJackEntry: From RFC-3636
 */
typedef struct
{
    UI32_T    ifMauIfIndex;                  /*key*/
    UI32_T    ifMauIndex;                    /*key*/
    UI32_T    ifJackIndex;                   /*key*/

    UI32_T    ifJackType;
}   SWCTRL_IfJackEntry_T;

#if (SYS_CPNT_WRED == TRUE)
typedef struct
{
  I8_T queue_id;
  UI32_T min;
  UI32_T max;
  UI32_T drop;
  UI32_T ecn;
  BOOL_T valid;
}SWCTRL_RandomDetect_T;
#endif

/* Ref:
 * SFF-8472 Specification for
 * Diagnostic Monitoring Interface for Optical Transceivers
 */
/* Unit: 0.01 Celsius */
#define SWCTRL_GBIC_DDM_TEMPERATURE_MAX (12800)
#define SWCTRL_GBIC_DDM_TEMPERATURE_MIN (-12800)
/* Unit: 0.01 V */
#define SWCTRL_GBIC_DDM_VOLTAGE_MAX     (655)
#define SWCTRL_GBIC_DDM_VOLTAGE_MIN     (0)
/* Unit: 0.01 mA */
#define SWCTRL_GBIC_DDM_CURRENT_MAX     (13100)
#define SWCTRL_GBIC_DDM_CURRENT_MIN     (0)
/* Unit: 0.01 dBm */
#define SWCTRL_GBIC_DDM_TX_POWER_MAX    (820)
#define SWCTRL_GBIC_DDM_TX_POWER_MIN    (-4000)
#define SWCTRL_GBIC_DDM_RX_POWER_MAX    (820)
#define SWCTRL_GBIC_DDM_RX_POWER_MIN    (-4000)

/* TYPE DECLARATIONS
 */


/* Cable Diag result
 */
#if (SYS_CPNT_SWCTRL_CABLE_DIAG_CHIP == SYS_CPNT_SWCTRL_CABLE_DIAG_BROADCOM)
typedef struct
{
    UI32_T   pair_len[4];      /* cable fault length */
    UI32_T   pair_state[4];    /* SWCTRL_Cable_Status_T */
    UI32_T   cable_status;     /* SWCTRL_Cable_Status_T */
    UI32_T   last_test_time;
    I32_T    fuzz_len;
}SWCTRL_Cable_Info_T;
#elif (SYS_CPNT_SWCTRL_CABLE_DIAG_CHIP == SYS_CPNT_SWCTRL_CABLE_DIAG_MARVELL)

typedef struct SWCTRL_Cable_Info_S
{
    UI32_T    last_test_time;
    UI32_T    ori_link_status; /* the link status before cable test */
    UI16_T    pair1Status;
    UI16_T    pair2Status;
    UI16_T    pair3Status;
    UI16_T    pair4Status;
    UI8_T     pair1FaultLen;
    UI8_T     pair2FaultLen;
    UI8_T     pair3FaultLen;
    UI8_T     pair4FaultLen;
    BOOL_T    under_testing;
    BOOL_T    is_dsp_mode;
}SWCTRL_Cable_Info_T; /* need sync with SWDRV_CableDiagInfo_T and DEV_SWDRV_CableDiagResultGeneral_T */
#else
#error "Chip not defined!"
#endif

typedef enum
{
    SWCTRL_PKTTYPE_IGMP,
    SWCTRL_PKTTYPE_RESERVED_UDP,
    SWCTRL_PKTTYPE_MLD,
    SWCTRL_PKTTYPE_IPMC,
    SWCTRL_PKTTYPE_IPMC6,
    SWCTRL_PKTTYPE_DHCP_SERVER,
    SWCTRL_PKTTYPE_DHCP_CLIENT,
    SWCTRL_PKTTYPE_DHCP6_SERVER,
    SWCTRL_PKTTYPE_DHCP6_CLIENT,
    SWCTRL_PKTTYPE_ORG_SPECIFIC,
    SWCTRL_PKTTYPE_ORG_SPECIFIC1,
    SWCTRL_PKTTYPE_ORG_SPECIFIC2,
    SWCTRL_PKTTYPE_ORG_SPECIFIC3,
    SWCTRL_PKTTYPE_CDP,
    SWCTRL_PKTTYPE_PVST,
    SWCTRL_PKTTYPE_ND,
    SWCTRL_PKTTYPE_INTRUDER,
    SWCTRL_PKTTYPE_PIM,
    SWCTRL_PKTTYPE_PIM6,
    SWCTRL_PKTTYPE_MAX,
} SWCTRL_PktType_T;

typedef enum
{
    /* SWCTRL_PKTTYPE_IGMP
     * SWCTRL_PKTTYPE_MLD
     * SWCTRL_PKTTYPE_IPMC
     * SWCTRL_PKTTYPE_IPMC6
     */
    SWCTRL_UNKNOWN_MCAST_TRAP_BY_IGMP       = BIT_0,
    SWCTRL_UNKNOWN_MCAST_TRAP_BY_IGMPSNP    = BIT_1,
    SWCTRL_UNKNOWN_MCAST_TRAP_BY_PIM        = BIT_2,
    SWCTRL_UNKNOWN_MCAST_TRAP_BY_MLD        = BIT_3,
    SWCTRL_UNKNOWN_MCAST_TRAP_BY_MLDSNP     = BIT_4,
    SWCTRL_UNKNOWN_MCAST_TRAP_BY_PIM6       = BIT_5,
    SWCTRL_UNKNOWN_MCAST_TRAP_BY_MVR        = BIT_6,
    SWCTRL_UNKNOWN_MCAST_TRAP_BY_MVR6       = BIT_7,

    /* SWCTRL_PKTTYPE_DHCP_SERVER
     * SWCTRL_PKTTYPE_DHCP_CLIENT
     */
    SWCTRL_DHCP_TRAP_BY_DHCP_CLIENT         = BIT_0,
    SWCTRL_DHCP_TRAP_BY_DHCPSNP             = BIT_1,
    SWCTRL_DHCP_TRAP_BY_L2_RELAY            = BIT_2,
    SWCTRL_DHCP_TRAP_BY_L3_RELAY            = BIT_3,
    SWCTRL_DHCP_TRAP_BY_DHCP_SERVER         = BIT_4,
    SWCTRL_DHCP_TRAP_BY_UDP_HELPER_67       = BIT_5,
    SWCTRL_DHCP_TRAP_BY_UDP_HELPER_68       = BIT_6,

    /* SWCTRL_PKTTYPE_DHCP6_SERVER
     * SWCTRL_PKTTYPE_DHCP6_CLIENT
     */
    SWCTRL_DHCP6_TRAP_BY_DHCP6_CLIENT       = BIT_0,
    SWCTRL_DHCP6_TRAP_BY_DHCP6_RELAY        = BIT_1,
    SWCTRL_DHCP6_TRAP_BY_DHCP6_SERVER       = BIT_2,
    SWCTRL_DHCP6_TRAP_BY_DHCP6SNP           = BIT_3,

    /* SWCTRL_PKTTYPE_ORG_SPECIFIC
     */
    SWCTRL_ORG_SPECIFIC_TRAP_BY_CLUSTER     = BIT_0,
    SWCTRL_ORG_SPECIFIC_TRAP_BY_ERPS        = BIT_1,

    /* SWCTRL_PKTTYPE_ORG_SPECIFIC1
     */
    SWCTRL_ORG_SPECIFIC1_TRAP_BY_LBD        = BIT_0,

    /* SWCTRL_PKTTYPE_ORG_SPECIFIC2
     */
    SWCTRL_ORG_SPECIFIC2_TRAP_BY_UDLD       = BIT_0,

    /* SWCTRL_PKTTYPE_ORG_SPECIFIC3
     */
    SWCTRL_ORG_SPECIFIC3_TRAP_BY_MLAG       = BIT_0,

    /* SWCTRL_PKTTYPE_CDP
     */
    SWCTRL_CDP_TRAP_BY_L2PT                 = BIT_0,
    SWCTRL_CDP_TRAP_BY_UDLD                 = BIT_1,

    /* SWCTRL_PKTTYPE_PVST
     */
    SWCTRL_PVST_TRAP_BY_L2PT                = BIT_0,

    /* SWCTRL_PKTTYPE_ND
     */
    SWCTRL_ND_TRAP_BY_NETCFG_ND             = BIT_0,
    SWCTRL_ND_TRAP_BY_NDSNP                 = BIT_1,

    /* SWCTRL_PKTTYPE_INTRUDER
     */
    SWCTRL_INTRUDER_TRAP_BY_PSEC            = BIT_0,
    SWCTRL_INTRUDER_TRAP_BY_DOT1X           = BIT_1,
    SWCTRL_INTRUDER_TRAP_BY_NETACCESS       = BIT_2,
    SWCTRL_INTRUDER_TRAP_BY_IPSG            = BIT_3,

    /* SWCTRL_PKTTYPE_PIM
     */
    SWCTRL_PIM_TRAP_BY_IGMPSNP            = BIT_0,
    SWCTRL_PIM_TRAP_BY_PIM           = BIT_1,

    /* SWCTRL_PKTTYPE_PIM6
     */
    SWCTRL_PIM6_TRAP_BY_MLDSNP            = BIT_0,
    SWCTRL_PIM6_TRAP_BY_PIM6           = BIT_1,

} SWCTRL_TrapPktOwner_T;

typedef enum
{
    SWCTRL_LEARNING_DISABLED_BY_CFG         = BIT_0,
    SWCTRL_LEARNING_DISABLED_BY_RSPAN       = BIT_1,
    SWCTRL_LEARNING_DISABLED_BY_MLAG        = BIT_2,
} SWCTRL_LearningDisabledOwner_T;

typedef struct
{
    UI32_T medium_forced_mode_supported;    /* bitmap consists of BIT_VALUE(VAL_portComboForcedMode_XXX) */
    UI32_T medium_fiber_type_supported;     /* bitmap consists of BIT_VALUE(VAL_portType_XXX) */
    UI32_T port_speed_duplex_supported;     /* bitmap consists of BIT_VALUE(VAL_portSpeedDpxCfg_XXX) */
    UI32_T port_flow_ctrl_cfg_supported;    /* bitmap consists of BIT_VALUE(VAL_portFlowCtrlCfg_XXX) */
    UI32_T port_capabilities_supported;     /* bitmap consists of SYS_VAL_portCapabilities_XXX */
    UI32_T port_autoneg_supported;          /* bitmap consists of BIT_VALUE(VAL_portAutonegotiation_XXX) */
    UI32_T port_fec_supported;              /* bitmap consists of BIT_VALUE(VAL_portFecMode_XXX) */
} SWCTRL_PortAbility_T;

typedef struct
{
    union
    {
        UI32_T cmd;          /*cmd fnction id*/
        BOOL_T result_bool;  /*respond bool return*/
        UI32_T result_ui32;  /*respond ui32 return*/
        UI32_T result_i32;  /*respond i32 return*/
    }type;

    union
    {

        BOOL_T bool_v;
        UI8_T  ui8_v;
        I8_T   i8_v;
        UI32_T ui32_v;
        UI16_T ui16_v;
        I32_T i32_v;
        I16_T i16_v;
        UI8_T ip4_v[4];
        UI8_T mac[6];

        struct
        {
            UI32_T u32_a1;
            UI32_T u32_a2;
        }u32a1_u32a2;

        struct
        {
            UI32_T u32_a1;
            UI32_T u32_a2;
            UI32_T u32_a3;
        }u32a1_u32a2_u32a3;

        struct
        {
            UI32_T u32_a1;
            UI32_T u32_a2;
            I32_T  i32_a3;
            BOOL_T bl_a4;
        }u32a1_u32a2_i32a3_bla4;

        struct
        {
            UI32_T u32_a1;
            UI32_T u32_a2;
            I32_T  i32_a3;
            I32_T  i32_a4;
            I32_T  i32_a5;
            I32_T  i32_a6;
        }u32a1_u32a2_i32a3_i32a4_i32a5_i32a6;

        struct
        {
            UI32_T u32_a1;
            UI32_T u32_a2;
            UI32_T u32_a3;
            UI32_T u32_a4;
        }u32a1_u32a2_u32a3_u32a4;

        UI8_T port_list[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];

        struct
        {
            UI32_T ifindex;
            UI8_T mapping[8];
        }ifindex_mapping;

        UI8_T mapping[8];

        struct
        {
            UI32_T session_id;
            UI8_T uplink_port_list[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
            UI8_T downlink_port_list[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
        }uplinkportlist_downlinkportlist;

        struct
        {
            UI32_T ifindex;
            UI8_T port_name[MAXSIZE_ifAlias+1];
        }ifindex_portname;

        UI8_T mac_addrs[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK][6];

        UI32_T device_types[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK];

        struct
        {
            UI32_T  lport;
            UI32_T  group_index;
            UI32_T  vlan_id;
            UI32_T  nbr_of_type_protocol;
            UI8_T   frame_type;
            UI8_T   protocol_value[SYS_ADPT_1V_MAX_NBR_OF_PROTOCOL_GROUP_ENTRY][SWCTRL_MAX_1V_PROTOCOL_VALUE_LENGTH];
            UI8_T   priority;
        }lport_groupindex_vlanid_type_frame_type_protocolvalue;

        struct
        {
            UI32_T  frame_type;
            UI8_T   protocol_value[SWCTRL_MAX_1V_PROTOCOL_VALUE_LENGTH];
            UI32_T  group_index;
        }frametype_protocolvalue_groupindex;

        struct
        {
            UI32_T        lport;
            SWCTRL_Cable_Info_T  result;
        }cable_diag_info;

#if (SYS_CPNT_ATC_STORM == TRUE)
        struct
        {
            SWCTRL_ATCBroadcastStormTimer_T bcast_timer;
            SWCTRL_ATCMulticastStormTimer_T mcast_timer;
        }atc_storm_timer;

        struct
        {
                UI32_T port;
                SWCTRL_ATCBroadcastStormEntry_T  bcast_entry;
        }get_atc_bcast_entry;
        struct
        {
                UI32_T port;
                SWCTRL_ATCMulticastStormEntry_T  mcast_entry;
        }get_atc_mcast_entry;


#endif

        struct
        {
            UI32_T u32_a1;
            BOOL_T bool_a2;
        }u32a1_boola2;

#if (SYS_CPNT_MAC_BASED_MIRROR == TRUE)
        struct
        {
            UI32_T ifindex_dest;
            UI8_T mac[SYS_ADPT_MAC_ADDR_LEN];
        }mac_mirror_entry;
#endif

        struct
        {
            UI32_T ifindex;
            SWCTRL_PktType_T pkt_type;
            SWCTRL_TrapPktOwner_T owner;
            BOOL_T to_cpu;
            BOOL_T drop;
        }pkt_trap;

#if (SYS_CPNT_PFC == TRUE)
        struct
        {
            UI32_T  ifidx;
            UI16_T  pri_en_vec;
            BOOL_T  tx_en;
            BOOL_T  rx_en;
        }ifidx_pfc_data;
#endif

#if (SYS_CPNT_ETS == TRUE)
        struct
        {
            UI32_T ifindex;
            UI32_T cosq2group[SYS_ADPT_MAX_NBR_OF_PRIORITY_QUEUE];
            BOOL_T cosq2group_is_valid;
        }cos_group_mapping;
        struct
        {
            UI32_T ifindex;
            UI32_T method;
            UI32_T weights[SYS_ADPT_ETS_MAX_NBR_OF_TRAFFIC_CLASS];
            BOOL_T weights_is_valid;
        }cos_group_scheduling;
#endif

#if (SYS_CPNT_CN == TRUE)
        struct
        {
            UI8_T cpid[8];
        }qcn_cpid;
#endif

#if (SYS_CPNT_MAC_IN_MAC == TRUE)
        struct
        {
            SWCTRL_MimServiceInfo_T mim;
            BOOL_T is_valid;
        }mim_service;

        struct
        {
            SWCTRL_MimPortInfo_T mim_port;
            BOOL_T is_valid;
        }mim_service_port;
#endif
#if (SYS_HWCFG_SUPPORT_PD == TRUE)
        SWCTRL_PortPD_T pd_info;
#endif

#if (SYS_CPNT_SFP_DDM_ALARMWARN_TRAP == TRUE)
        struct
        {
            UI32_T unit;
            UI32_T sfp_index;
            UI32_T threshold_type;
            I32_T  val;
        }unit_index_type_sfp_ddm_threshold;
#endif

#if (SYS_CPNT_MAU_MIB == TRUE)
        SWCTRL_IfMauEntry_T if_mau_entry;

        SWCTRL_IfMauAutoNegEntry_T if_mau_auto_neg_entry;

        SWCTRL_IfJackEntry_T if_jack_entry;
#endif

        struct
        {
            UI32_T trunk_id;
            SYS_TYPE_Uport_T unit_port;
            UI32_T u32_a1;
        } trunkid_unitport;

        struct
        {
            UI32_T lport;
            BOOL_T egr_lport_list_is_specified;
            BOOL_T blk_lport_list_is_specified;
            UI8_T egr_lport_list[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
            UI8_T blk_lport_list[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
        }port_egress_block;

#if (SYS_CPNT_HASH_SELECTION == TRUE)
        struct
        {
            SWCTRL_OM_HashSelService_T service; 
            UI8_T list_index;
        }bind_hash_service;

        struct
        {
            UI8_T list_index;
            SWCTRL_OM_HashSelection_T  selection;
        }set_hash_sel;
#endif /*#if (SYS_CPNT_HASH_SELECTION == TRUE)*/

#if (SYS_CPNT_WRED == TRUE)
        struct
        {
          UI32_T lport;
          SWCTRL_RandomDetect_T value;
        }random_detect;
#endif
    }data;
}   SWCTRL_MGR_IPCMsg_T;


#define SWCTRL_MGR_MSGBUF_TYPE_SIZE    sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->type)

#define SWCTRL_MGR_GET_MSG_SIZE(field_name)                       \
            (SWCTRL_MGR_MSGBUF_TYPE_SIZE +                        \
            sizeof(((SWCTRL_MGR_IPCMsg_T *)0)->data.field_name))

typedef struct
{
    union
    {
        UI32_T cmd;          /*cmd fnction id*/
        BOOL_T result_bool;  /*respond bool return*/
        UI32_T result_ui32;  /*respond ui32 return*/
        UI32_T result_i32;  /*respond i32 return*/
    }type;

    union
    {
        BOOL_T bool_v;
        UI8_T  ui8_v;
        I8_T   i8_v;
        UI32_T ui32_v;
        UI16_T ui16_v;
        I32_T i32_v;
        I16_T i16_v;
        UI8_T ip4_v[4];
        UI8_T mac[6];

        struct
        {
            UI32_T u32_a1;
            UI32_T u32_a2;
            SYS_TYPE_Uport_T uport_a3;
        }u32a1_u32a2_uporta3;

        struct
        {
            UI32_T u32_a1;
            UI32_T u32_a2;
        }u32a1_u32a2;

        struct
        {
            UI32_T u32_a1;
            UI32_T u32_a2;
            UI32_T u32_a3;
        }u32a1_u32a2_u32a3;

        struct
        {
            UI32_T u32_a1;
            UI32_T u32_a2;
            BOOL_T bl;
        }u32a1_u32a2_bl;

        struct
        {
            UI32_T u32_a1;
            UI32_T u32_a2;
            UI32_T u32_a3;
            UI32_T u32_a4;
        }u32a1_u32a2_u32a3_u32a4;

        struct
        {
            UI32_T u32_a1;
            BOOL_T bool_a2;
        }u32a1_boola2;

        struct
        {
            UI32_T ifindex;
            Port_Info_T p_info;
        } ifindex_pinfo;

        UI8_T failure_ports[SYS_ADPT_NBR_OF_BYTE_FOR_1BIT_UPORT_LIST*SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK];

        struct
        {
            UI32_T ifindex;
            UI8_T mac[6];
        } ifindex_mac;

        SWCTRL_PrivateVlan_T private_vlan;

        UI8_T port_list[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];

        struct
        {
            UI32_T session_id;
            UI8_T uplink_port_list[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
            UI8_T downlink_port_list[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
        }uplinkportlist_downlinkportlist;

        struct
        {
            UI32_T unit;
            UI32_T port;
            UI32_T ifindex;
        } unit_port_ifindex;

        struct
        {
            UI32_T unit;
            UI32_T port;
            UI32_T trunk_id;
        } unit_port_trunk;

        struct
        {
            UI32_T uport_ifindex;
            UI32_T trunk_ifindex;
            BOOL_T is_static;
        } uport_trunk_isstatic;

        SWCTRL_PortEntry_T port_entry;

        SWCTRL_MirrorEntry_T mirror_entry;

        SWCTRL_BcastStormEntry_T bcast_storm_entry;

        SWCTRL_McastStormEntry_T mcast_storm_entry;

        SWCTRL_UnknownUcastStormEntry_T unknown_ucast_storm_entry;

        struct
        {
            UI32_T trunk_id;
            SYS_TYPE_Uport_T unit_port;
            BOOL_T is_re_check;
            BOOL_T is_check_lacp_oper_state;
        } trunkid_unitport_check_state;

        struct
        {
            UI32_T unit;
            UI32_T port;
            UI32_T ifindex;
            BOOL_T is_inherit;
        } unit_port_ifindex_inherit;

        struct
        {
            UI32_T        lport;
            SWCTRL_Cable_Info_T  result;
        }cable_diag_info;

        struct
        {
            UI32_T lport;
            UI8_T  map;
        }lport_queue_map;
#if (SYS_CPNT_MAC_BASED_MIRROR == TRUE)
        SWCTRL_MacAddrMirrorEntry_T mac_addr_mirror_entry;
#endif

        struct
        {
            UI32_T ifindex;
            UI32_T active_lportarray[SYS_ADPT_MAX_NBR_OF_PORT_PER_TRUNK];
            UI32_T active_lport_count;
        }active_trunk_member;

        SWCTRL_PortAbility_T port_ability;

#if (SYS_CPNT_SFP_DDM_ALARMWARN_TRAP == TRUE)
        struct
        {
            UI32_T lport;
            SWCTRL_OM_SfpDdmThreshold_T  sfp_ddm_threshold;
        }lport_sfp_ddm_threshold;
        struct
        {
            UI32_T lport;
            SWCTRL_OM_SfpDdmThresholdEntry_T  sfp_ddm_threshold_entry;
        }lport_sfp_ddm_threshold_entry;
        struct
        {
            UI32_T lport;
            SWCTRL_OM_SfpDdmThresholdStatus_T  sfp_ddm_threshold_status;
        }lport_sfp_ddm_threshold_status;
#endif

#if (SYS_CPNT_SWDRV_MONITOR_SFP_DDM == TRUE)
        struct
        {
            UI32_T unit;
            UI32_T sfp_index;
            SWCTRL_OM_SfpInfo_T  sfp_info;
        }unit_index_sfp_info;

        struct
        {
            UI32_T unit;
            UI32_T sfp_index;
            SWCTRL_OM_SfpDdmInfo_T  sfp_ddm_info;
        }unit_index_sfp_ddm_info;

        struct
        {
            UI32_T unit;
            UI32_T sfp_index;
            SWCTRL_OM_SfpDdmInfoMeasured_T  sfp_ddm_info_measured;
        }unit_index_sfp_ddm_info_measured;

        /* for SNMP */
        struct
        {
            UI32_T lport;
            SWCTRL_OM_SfpEntry_T  sfp_entry;
        }lport_sfp_entry;

        struct
        {
            UI32_T lport;
            SWCTRL_OM_SfpDdmEntry_T  sfp_ddm_entry;
        }lport_sfp_ddm_entry;
#endif

#if (SYS_CPNT_HASH_SELECTION == TRUE)
        struct
        {
            UI8_T list_index;
            SWCTRL_OM_HashSelBlockInfo_T block_info;
        }hash_sel_block;
#endif

#if(SYS_CPNT_WRED == TRUE)
        struct
        {
          UI32_T lport;
          SWCTRL_OM_RandomDetect_T value;
        }random_detect;
#endif
    }data;
}   SWCTRL_OM_IPCMsg_T;


#define SWCTRL_OM_MSGBUF_TYPE_SIZE    sizeof(((SWCTRL_OM_IPCMsg_T *)0)->type)


#if (SYS_CPNT_EFM_OAM == TRUE)
enum
{
    SWCTRL_LOOPBACK_MODE_TYPE_PASSIVE=0,
    SWCTRL_LOOPBACK_MODE_TYPE_ACTIVE

}SWCTRL_LOOPBACK_MODE_TYPE;
#endif


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/****************************************************************************/
/* Switch Initialization                                                    */
/****************************************************************************/
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_Init
 * -------------------------------------------------------------------------
 * FUNCTION: This function allocates and initiates the system resource for
 *           Switch Control module
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_Init(void);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_Create_InterCSC_Relation
 * -------------------------------------------------------------------------
 * FUNCTION: This function initializes all function pointer registration operations.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_Create_InterCSC_Relation(void);

/* -------------------------------------------------------------------------
 * FUNCTION: This function set transition mode flag
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_SetTransitionMode(void);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_EnterTransitionMode
 * -------------------------------------------------------------------------
 * FUNCTION: This function will initialize the Switch Control module and
 *           free all resource to enter transition mode while stacking
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_EnterTransitionMode(void);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_EnterMasterMode
 * -------------------------------------------------------------------------
 * FUNCTION: This function will configurate the Switch Control module to
 *           enter master mode after stacking
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : This function must be invoked first before
 *           STA_Enter_Master_Mode() is called.
 * -------------------------------------------------------------------------*/
void SWCTRL_EnterMasterMode(void);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_EnterSlaveMode
 * -------------------------------------------------------------------------
 * FUNCTION: This function will disable the Switch Control services and
 *           enter slave mode after stacking
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_EnterSlaveMode(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME: SWCTRL_HandleHotInsertion
 * PURPOSE  : This function will initialize the port OM of the module ports
 *            when the option module is inserted.
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port
 *                                     inserted
 *            number_of_port        -- the number of ports on the inserted
 *                                     module
 *            use_default           -- the flag indicating the default
 *                                     configuration is used without further
 *                                     provision applied; TRUE if a new module
 *                                     different from the original one is
 *                                     inserted
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Only one module is inserted at a time.

 * -------------------------------------------------------------------------*/
void SWCTRL_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default);


/*-------------------------------------------------------------------------
 * FUNCTION NAME: SWCTRL_HandleHotRemoval
 * PURPOSE  : This function will clear the port OM of the module ports when
 *            the option module is removed.
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port
 *                                     removed
 *            number_of_port        -- the number of ports on the removed
 *                                     module
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Only one module is removed at a time.
 * -------------------------------------------------------------------------*/
void SWCTRL_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetOperationMode
 * -------------------------------------------------------------------------
 * FUNCTION: Get current swctrl operation mode.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : Current operation mode.
 *           1) SYS_TYPE_STACKING_TRANSITION_MODE
 *           2) SYS_TYPE_STACKING_MASTER_MODE
 *           3) SYS_TYPE_STACKING_SLAVE_MODE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
SYS_TYPE_Stacking_Mode_T SWCTRL_GetOperationMode(void);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PreProvisionComplete
 * -------------------------------------------------------------------------
 * FUNCTION: This function will perform proprovision complete
 *           action
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_PreProvisionComplete(void);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_ProvisionComplete
 * -------------------------------------------------------------------------
 * FUNCTION: This function will tell the Switch Control module to start
 *           action
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_ProvisionComplete(void);


/****************************************************************************/
/* Call Back Functions                                                      */
/****************************************************************************/
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_Register_HotSwapInsert_CallBack
 * -------------------------------------------------------------------------
 * FUNCTION: Register the call-back function, when a hot swap mudule is
 *           inserted the registered function will be called
 * INPUT   : fun -- call back function pointer
 * OUTPUT  : none
 * RETURN  : none
 * NOTE    : void *fun(UI32_T ifindex)
 * -------------------------------------------------------------------------*/
void SWCTRL_Register_HotSwapInsert_CallBack(void (*fun)(UI32_T ifindex));


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_Register_HotSwapRemove_CallBack
 * -------------------------------------------------------------------------
 * FUNCTION: Register the call-back function, when a hot swap mudule is
 *           removed the registered function will be called
 * INPUT   : fun -- call back function pointer
 * OUTPUT  : none
 * RETURN  : none
 * NOTE    : void *fun(UI32_T ifindex)
 * -------------------------------------------------------------------------*/
void SWCTRL_Register_HotSwapRemove_CallBack(void (*fun)(UI32_T ifindex));


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_Register_LPortTypeChanged_CallBack
 * -------------------------------------------------------------------------
 * FUNCTION: Register the call-back function, when port type changeing is
 *           removed the registered function will be called
 * INPUT   : fun -- call back function pointer
 * OUTPUT  : none
 * RETURN  : none
 * NOTE    : 1. void *fun(UI32_T ifindex, UI32_T port_type)
 *           2. For logical port.
 * -------------------------------------------------------------------------*/
void SWCTRL_Register_LPortTypeChanged_CallBack(void (*fun)(UI32_T ifindex, UI32_T port_type));


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_Register_UPortTypeChanged_CallBack
 * -------------------------------------------------------------------------
 * FUNCTION: Register the call-back function, when port type changeing is
 *           removed the registered function will be called
 * INPUT   : fun -- call back function pointer
 * OUTPUT  : none
 * RETURN  : none
 * NOTE    : 1. void *fun(UI32_T unit, UI32_T port, UI32_T port_type)
 *           2. For user port.
 * -------------------------------------------------------------------------*/
void SWCTRL_Register_UPortTypeChanged_CallBack(void (*fun)(UI32_T unit, UI32_T port, UI32_T port_type));


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_Register_TrunkMemberAdd1st_CallBack
 * -------------------------------------------------------------------------
 * FUNCTION: Register the call-back function, when the first port is added
 *           to a trunk the registered function will be called
 * INPUT   : fun -- call back function pointer
 * OUTPUT  : none
 * RETURN  : none
 * NOTE    : void *fun(UI32_T trunk_ifindex, UI32_T member_ifindex)
 *           Designed for Trunk, AMTR and VLAN.
 * -------------------------------------------------------------------------*/
void SWCTRL_Register_TrunkMemberAdd1st_CallBack(void (*fun)(UI32_T trunk_ifindex,
                                                            UI32_T member_ifindex));


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_Register_TrunkMemberAdd_CallBack
 * -------------------------------------------------------------------------
 * FUNCTION: Register the call-back function, when a port is added to a
 *           trunk the registered function will be called
 * INPUT   : fun -- call back function pointer
 * OUTPUT  : none
 * RETURN  : none
 * NOTE    : void *fun(UI32_T trunk_ifindex, UI32_T member_ifindex)
 *           Designed for Trunk, AMTR and VLAN.
 * -------------------------------------------------------------------------*/
void SWCTRL_Register_TrunkMemberAdd_CallBack(void (*fun)(UI32_T trunk_ifindex,
                                                         UI32_T member_ifindex));


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_Register_TrunkMemberDelete_CallBack
 * -------------------------------------------------------------------------
 * FUNCTION: Register the call-back function, when a port is deleted from a
 *           trunk the registered function will be called
 * INPUT   : fun -- call back function pointer
 * OUTPUT  : none
 * RETURN  : none
 * NOTE    : void *fun(UI32_T trunk_ifindex, UI32_T member_ifindex)
 *           Designed for Trunk and VLAN.
 * -------------------------------------------------------------------------*/
void SWCTRL_Register_TrunkMemberDelete_CallBack(void (*fun)(UI32_T trunk_ifindex,
                                                            UI32_T member_ifindex));


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_Register_TrunkMemberDeleteLst_CallBack
 * -------------------------------------------------------------------------
 * FUNCTION: Register the call-back function, when the last port is deleted
 *           from a trunk the registered function will be called
 * INPUT   : fun -- call back function pointer
 * OUTPUT  : none
 * RETURN  : none
 * NOTE    : void *fun(UI32_T trunk_ifindex, UI32_T member_ifindex)
 *           Designed for Trunk and VLAN.
 * -------------------------------------------------------------------------*/
void SWCTRL_Register_TrunkMemberDeleteLst_CallBack(void (*fun)(UI32_T trunk_ifindex,
                                                               UI32_T member_ifindex));

/*  Logical Port :
 *                | LPortAdminEnable           LPortAdminDisable
 *  --------------+-------------------------+---------------------------
 *                | STA (Listening/Learning       STA (Disable)
 *                |      Forwarding/Blocking)
 *  LPortLinkUp   | IGMPSNP (OperUp)             IGMPSNP (OperNotUp)
 *                | VLAN    (OperUp)             VLAN    (OperNotUp)
 *  --------------+-------------------------+---------------------------
 *                | STA (Broken)                 STA (Disable)
 *                | AMTR (LinkDown)              AMTR (LinkDown)
 *  LPortLinkDown | IGMPSNP (OperNotUp)          IGMPSNP (OperNotUp)
 *                | VLAN    (OperNotUp)          VLAN    (OperNotUp)
 *                |
 */
/*  Unit Port :
 *                | UPortAdminEnable           UPortAdminDisable
 *  --------------+-----------------------+-----------------------------
 *                | LED_MGR (Green)              LED_MGR (Amber)
 *  UPortLinkUp   | LACP                         LACP
 *  --------------+-----------------------+-----------------------------
 *                | LED_MGR (OFF)                LED_MGR (OFF)
 *  UPortLinkDown | LACP                         LACP
 *                |
 */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_Register_LPortLinkUp_CallBack
 * -------------------------------------------------------------------------
 * FUNCTION: Register the call-back function, when link down to up the
 *           registered function will be called
 * INPUT   : fun -- call back function pointer
 * OUTPUT  : none
 * RETURN  : none
 * NOTE    : 1. void *fun(UI32_T ifindex)
 *           2. STA_TASK
 * -------------------------------------------------------------------------*/
void SWCTRL_Register_LPortLinkUp_CallBack(void (*fun)(UI32_T ifindex));


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_Register_LPortLinkDown_CallBack
 * -------------------------------------------------------------------------
 * FUNCTION: Register the call-back function, when link up to down the
 *           register function will be called
 * INPUT   : fun -- call back function pointer
 * OUTPUT  : none
 * RETURN  : none
 * NOTE    : 1. void *fun(UI32_T ifindex)
 *           2. STA_TASK, AMTR_TASK
 * -------------------------------------------------------------------------*/
void SWCTRL_Register_LPortLinkDown_CallBack(void (*fun)(UI32_T ifindex));


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_Register_UPortLinkUp_CallBack
 * -------------------------------------------------------------------------
 * FUNCTION: Register the call-back function, when link down to up the
 *           registered function will be called
 * INPUT   : fun -- call back function pointer
 * OUTPUT  : none
 * RETURN  : none
 * NOTE    : 1. void *fun(UI32_T unit, UI32_T port)
 *           2. LED_MGR, LACP_TASK
 * -------------------------------------------------------------------------*/
void SWCTRL_Register_UPortLinkUp_CallBack(void (*fun)(UI32_T unit,
                                                      UI32_T port));

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_Register_UPortFastLinkUp_CallBack
 * -------------------------------------------------------------------------
 * FUNCTION: Register the call-back function, when link down to up the
 *           registered function will be called
 * INPUT   : fun -- call back function pointer
 * OUTPUT  : none
 * RETURN  : none
 * NOTE    : 1. void *fun(UI32_T unit, UI32_T port)
 *           2. LED_MGR
 *           3. Fast uport link up callback, this callback should be used
 *              only by LEDMGMT. SWCTRL should callbcak before all other callbacks.
 * -------------------------------------------------------------------------*/
void SWCTRL_Register_UPortFastLinkUp_CallBack(void (*fun)(UI32_T unit,
                                                          UI32_T port));

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_Register_UPortLinkDown_CallBack
 * -------------------------------------------------------------------------
 * FUNCTION: Register the call-back function, when link up to down the
 *           register function will be called
 * INPUT   : fun -- call back function pointer
 * OUTPUT  : none
 * RETURN  : none
 * NOTE    : 1. void *fun(UI32_T unit, UI32_T port)
 *           2. LED_MGR, LACP_TASK
 * -------------------------------------------------------------------------*/
void SWCTRL_Register_UPortLinkDown_CallBack(void (*fun)(UI32_T unit,
                                                        UI32_T port));

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_Register_UPortFastLinkDown_CallBack
 * -------------------------------------------------------------------------
 * FUNCTION: Register the call-back function, when link up to down the
 *           register function will be called
 * INPUT   : fun -- call back function pointer
 * OUTPUT  : none
 * RETURN  : none
 * NOTE    : 1. void *fun(UI32_T unit, UI32_T port)
 *           2. LED_MGR
 *           3. Fast uport link down callback, this callback should be used
 *              only by LEDMGMT. SWCTRL should callbcak before all other callbacks.
 * -------------------------------------------------------------------------*/
void SWCTRL_Register_UPortFastLinkDown_CallBack(void (*fun)(UI32_T unit,
                                                            UI32_T port));

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_Register_LPortOperUp_CallBack
 * -------------------------------------------------------------------------
 * FUNCTION: Register the call-back function, when port oper status is up
 *           the register function will be called
 * INPUT   : fun -- call back function pointer
 * OUTPUT  : none
 * RETURN  : none
 * NOTE    : 1. void *fun(UI32_T ifindex)
 *           2. VLAN_TASK, IGMPSNP_TASK
 * -------------------------------------------------------------------------*/
void SWCTRL_Register_LPortOperUp_CallBack(void (*fun)(UI32_T ifindex));


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_Register_LPortOperNotUp_CallBack
 * -------------------------------------------------------------------------
 * FUNCTION: Register the call-back function, when port oper status is not up
 *           the register function will be called
 * INPUT   : fun -- call back function pointer
 * OUTPUT  : none
 * RETURN  : none
 * NOTE    : 1. void *fun(UI32_T ifindex)
 *           2. VLAN_TASK, IGMPSNP_TASK
 * -------------------------------------------------------------------------*/
void SWCTRL_Register_LPortNotOperUp_CallBack(void (*fun)(UI32_T ifindex));


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_Register_TrunkMemberPortOperUp_CallBack
 * -------------------------------------------------------------------------
 * FUNCTION: Register the call-back function, when port oper status is up
 *           the register function will be called
 * INPUT   : fun -- call back function pointer
 * OUTPUT  : none
 * RETURN  : none
 * NOTE    : 1. void (*fun)(UI32_T trunk_ifindex, UI32_T trunk_member_port_ifindex)
 *           2. Trunk load balance
 * -------------------------------------------------------------------------*/
void SWCTRL_Register_TrunkMemberPortOperUp_CallBack(
                                    void (*fun)(UI32_T trunk_ifindex,
                                                UI32_T trunk_member_port_ifindex));


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_Register_TrunkMemberPortNotOperUp_CallBack
 * -------------------------------------------------------------------------
 * FUNCTION: Register the call-back function, when port oper status is not up
 *           the register function will be called
 * INPUT   : fun -- call back function pointer
 * OUTPUT  : none
 * RETURN  : none
 * NOTE    : 1. void (*fun)(UI32_T trunk_ifindex, UI32_T trunk_member_port_ifindex)
 *           2. Trunk load balance
 * -------------------------------------------------------------------------*/
void SWCTRL_Register_TrunkMemberPortNotOperUp_CallBack(
                                void (*fun)(UI32_T trunk_ifindex,
                                            UI32_T trunk_member_port_ifindex));


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_Register_LPortAdminEnable_CallBack
 * -------------------------------------------------------------------------
 * FUNCTION: Register the call-back function, when a port is enabled the
 *           registered function will be called
 * INPUT   : fun -- call back function pointer
 * OUTPUT  : none
 * RETURN  : none
 * NOTE    : 1. void *fun(UI32_T ifindex)
 *           2. STA_TASK
 * -------------------------------------------------------------------------*/
void SWCTRL_Register_LPortAdminEnable_CallBack(void (*fun)(UI32_T ifindex));


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_Register_LPortAdminDisable_CallBack
 * -------------------------------------------------------------------------
 * FUNCTION: Register the call-back function, when a port is disabled the
 *           registered function will be called
 * INPUT   : fun -- call back function pointer
 * OUTPUT  : none
 * RETURN  : none
 * NOTE    : 1. void *fun(UI32_T ifindex)
 *           2. STA_TASK
 * -------------------------------------------------------------------------*/
void SWCTRL_Register_LPortAdminDisable_CallBack(void (*fun)(UI32_T ifindex));

#if (SYS_CPNT_LLDP == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_Register_LPortAdminDisableBefore_CallBack
 * -------------------------------------------------------------------------
 * FUNCTION: Register the call-back function, when a port is disabled the
 *           registered function will be called, before do the admin disable
 * INPUT   : fun -- call back function pointer
 * OUTPUT  : none
 * RETURN  : none
 * NOTE    : 1. void *fun(UI32_T ifindex)
 *           2. for LLDP
 * -------------------------------------------------------------------------*/
void SWCTRL_Register_LPortAdminDisableBefore_CallBack(void (*fun)(UI32_T ifindex));
#endif

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_Register_UPortAdminEnable_CallBack
 * -------------------------------------------------------------------------
 * FUNCTION: Register the call-back function, when a port is enabled the
 *           registered function will be called
 * INPUT   : fun -- call back function pointer
 * OUTPUT  : none
 * RETURN  : none
 * NOTE    : 1. void *fun(UI32_T unit, UI32_T port)
 *           2. LED_MGR, LACP_TASK
 * -------------------------------------------------------------------------*/
void SWCTRL_Register_UPortAdminEnable_CallBack(void (*fun)(UI32_T unit,
                                                           UI32_T port));


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_Register_UPortAdminDisable_CallBack
 * -------------------------------------------------------------------------
 * FUNCTION: Register the call-back function, when a port is disabled the
 *           registered function will be called
 * INPUT   : fun -- call back function pointer
 * OUTPUT  : none
 * RETURN  : none
 * NOTE    : 1. void *fun(UI32_T unit, UI32_T port)
 *           2. LED_MGR, LACP_TASK
 * -------------------------------------------------------------------------*/
void SWCTRL_Register_UPortAdminDisable_CallBack(void (*fun)(UI32_T unit,
                                                            UI32_T port));


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_Register_PortSpeedDuplex_CallBack
 * -------------------------------------------------------------------------
 * FUNCTION: Register the call-back function, when the speed or duplex of a
 *           port is changed the registered function will be called
 * INPUT   : fun -- call back function pointer
 * OUTPUT  : none
 * RETURN  : none
 * NOTE    : void *fun(UI32_T ifindex, UI32_T speed_duplex)
 *           Whenever speed or duplex changes, SWCTRL needs to notify
 *           1. STA
 * -------------------------------------------------------------------------*/
void SWCTRL_Register_PortSpeedDuplex_CallBack(void (*fun)(UI32_T ifindex,
                                                           UI32_T speed_duplex));



/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_Register_UPortSpeedDuplex_CallBack
 * -------------------------------------------------------------------------
 * FUNCTION: Register the call-back function, when the speed or duplex of a
 *           port is changed the registered function will be called
 * INPUT   : fun -- call back function pointer
 * OUTPUT  : none
 * RETURN  : none
 * NOTE    : void *fun(UI32_T unit, UI32_T port, UI32_T speed_duplex)
 *           Whenever speed or duplex changes, SWCTRL needs to notify
 *           1. LED_MGMT
 *           2. STA
 *           3. IML(RFC2233)
 * -------------------------------------------------------------------------*/
void SWCTRL_Register_UPortSpeedDuplex_CallBack(void (*fun)(UI32_T unit,
                                                           UI32_T port,
                                                           UI32_T speed_duplex));

/*Charles: swctrl_support_dot1x
 */
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_Register_UPortLacpEffectiveOperStatusChanged_CallBack
 * -------------------------------------------------------------------------
 * FUNCTION: Register the callback function, when effective oper status
 *           of some user port was changed for lacp the registered function
 *           will be called.
 * INPUT   : fun -- call back function pointer
 * OUTPUT  : none
 * RETURN  : none
 * NOTE    : I.   S'pose for lacp.
 *           II.  pre_status --- The status before change.
 *           III. current_status --- The status agter change.
 *           IV.  Definiation of precedence:
 *                    Status                                    Precedence
 *                 ----------------------------------------     ----------
 *                 1) VAL_ifOperStatus_up                       0
 *                 2) SWCTRL_PORT_DORMANT_STATUS_TYPE_LACP      1
 *                 3) SWCTRL_PORT_DORMANT_STATUS_TYPE_DOT1X     2
 *                 4) VAL_ifOperStatus_down                     3
 *                 5) VAL_ifOperStatus_lowerLayerDown           3
 *                 6) VAL_ifOperStatus_notPresent               4
 *           V.   When to callback:
 *                 1) Precedence(pre_status)     <   Precedence(current_status) &&
 *                    Precedence(current_status) >=  Precedence(SWCTRL_PORT_DORMANT_STATUS_TYPE_LACP)
 *                 2) Precedence(pre_status)     >   Precedence(current_status) &&
 *                    Precedence(current_status) ==  Precedence(SWCTRL_PORT_DORMANT_STATUS_TYPE_LACP)
 * -------------------------------------------------------------------------*/
void SWCTRL_Register_UPortLacpEffectiveOperStatusChanged_CallBack(void (*fun)(UI32_T unit, UI32_T port,
                                                                  UI32_T pre_status, UI32_T current_status) );

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_Register_UPortDot1xEffectiveOperStatusChanged_CallBack
 * -------------------------------------------------------------------------
 * FUNCTION: Register the callback function, when effective oper status
 *           of some user port was changed for dot1x the registered function
 *           will be called.
 * INPUT   : fun -- call back function pointer
 * OUTPUT  : none
 * RETURN  : none
 * NOTE    : I.   S'pose for dot1x.
 *           II.  pre_status --- The status before change.
 *           III. current_status --- The status agter change.
 *           IV.  Definiation of precedence:
 *                    Status                                    Precedence
 *                 ----------------------------------------     ----------
 *                 1) VAL_ifOperStatus_up                       0
 *                 2) SWCTRL_PORT_DORMANT_STATUS_TYPE_LACP      1
 *                 3) SWCTRL_PORT_DORMANT_STATUS_TYPE_DOT1X     2
 *                 4) VAL_ifOperStatus_down                     3
 *                 5) VAL_ifOperStatus_lowerLayerDown           3
 *                 6) VAL_ifOperStatus_notPresent               4
 *           V.   When to callback:
 *                 1) Precedence(pre_status)     <  Precedence(current_status) &&
 *                    Precedence(current_status) >  Precedence(SWCTRL_PORT_DORMANT_STATUS_TYPE_DOT1X)
 *                 2) Precedence(pre_status)     >  Precedence(current_status) &&
 *                    Precedence(current_status) == Precedence(SWCTRL_PORT_DORMANT_STATUS_TYPE_DOT1X)
 * -------------------------------------------------------------------------*/
void SWCTRL_Register_UPortDot1xEffectiveOperStatusChanged_CallBack(void (*fun)(UI32_T unit, UI32_T port,
                                                                   UI32_T pre_status, UI32_T current_status) );


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_Register_ForwardingUPortAddToTrunk_CallBack
 * -------------------------------------------------------------------------
 * FUNCTION: Register the call-back function, when a STA forwarding user port join to
 *           a trunk.
 * INPUT   : fun -- call back function pointer.
 * OUTPUT  : None.
 * RETURN  : None
 * NOTE    : Callback after this user port is added.
 * -------------------------------------------------------------------------*/
void SWCTRL_Register_ForwardingUPortAddToTrunk_CallBack(void (*fun)(UI32_T trunk_ifindex, UI32_T member_ifindex));


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_Register_ForwardingTrunkMemberDelete_CallBack
 * -------------------------------------------------------------------------
 * FUNCTION: Register the call-back function, when a STA forwarding trnuk member
 *           is removed from a trunk.
 * INPUT   : fun -- call back function pointer.
 * OUTPUT  : None.
 * RETURN  : None
 * NOTE    : Callback after this user port is deleted.
 * -------------------------------------------------------------------------*/
void SWCTRL_Register_ForwardingTrunkMemberDelete_CallBack(void (*fun)(UI32_T trunk_ifindex, UI32_T member_ifindex));


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_Register_ForwardingTrunkMemberToNonForwarding_CallBack
 * -------------------------------------------------------------------------
 * FUNCTION: Register the call-back function, when a STA forwarding trunk member
 *           become non-forwarding.
 * INPUT   : fun -- call back function pointer.
 * OUTPUT  : None.
 * RETURN  : None
 * NOTE    : Only happens when a forwarding trunk meber link down.
 * -------------------------------------------------------------------------*/
void SWCTRL_Register_ForwardingTrunkMemberToNonForwarding_CallBack(void (*fun)(UI32_T trunk_ifindex, UI32_T member_ifindex));


/****************************************************************************/
/* Port Configuration                                                       */
/****************************************************************************/
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_isPortLinkUp
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get all information of a port
 * INPUT   : ifindex   -- which port to get
 * OUTPUT  : port_info -- all information of this port
 * RETURN  : TRUE: Link Up, FALSE: Link Down
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_isPortLinkUp(UI32_T ifindex);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetPortInfo
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get all information of a port
 * INPUT   : ifindex   -- which port to get
 * OUTPUT  : port_info -- all information of this port
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_GetPortInfo(UI32_T ifindex, Port_Info_T *port_info);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetNextPortInfo
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get all information of a port
 * INPUT   : ifindex   -- the key to get
 * OUTPUT  : ifindex   -- the next existing port
 *           port_info -- all information of this port
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_GetNextPortInfo(UI32_T *ifindex, Port_Info_T *port_info);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetRunningPortInfo
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get the port info of running config
 *           a port
 * INPUT   : ifindex      -- which port to get
 * OUTPUT  : port_info    -- the port information
 * RETURN  : One of SYS_TYPE_Get_Running_Cfg_T
 * NOTE    : None
 * -------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T SWCTRL_GetRunningPortInfo(UI32_T ifindex, Port_Info_T *port_info);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetPortLinkStatus
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
BOOL_T SWCTRL_GetPortLinkStatus(UI32_T ifindex, UI32_T *port_link_status);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetPortOperStatus
 * -------------------------------------------------------------------------
 * FUNCTION: This rutine provide the information of the operating status of
 *           the port.
 * INPUT   : ifindex --- The information of which port to get.
 * OUTPUT  : *port_oper_status --- Operating status of this port.
 *                                 1) VAL_ifOperStatus_up
 *                                 2) VAL_ifOperStatus_down
 *                                 3) VAL_ifOperStatus_dormant
 *                                 4) VAL_ifOperStatus_lowerLayerDown
 * RETURN  : TRUE  --- Succefully.
 *           FALSE --- 1) ifindex is not belong to user port or logic port.
 *                     2) This port is not present.
 * NOTE    : This runtine provide subset information of SWCTRL_GetPortInfo().
 *           If some task has stack size issue, caller should use this rutine
 *           to get the link status.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_GetPortOperStatus(UI32_T ifindex, UI32_T *port_oper_status);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SWCTRL_IsPortOperationUp
 *-------------------------------------------------------------------------
 * PURPOSE  : This function ehck this port is Port operaton up
 * INPUT    : l_port - the logical port will be checked
 * OUTPUT   : None
 * RETURN   : TRUE  - this is operation up port
 *            FALSE - this is not operaton up port
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T SWCTRL_IsPortOperationUp(UI32_T l_port);

#if (SYS_CPNT_3COM_LOOPBACK_TEST == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetSwitchLoopbackTestFailurePorts
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
BOOL_T SWCTRL_GetSwitchLoopbackTestFailurePorts(UI8_T failure_ports[SYS_ADPT_NBR_OF_BYTE_FOR_1BIT_UPORT_LIST*SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK]);
#endif

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetPortAdminStatus
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set port administration status
 * INPUT   : ifindex        -- which port to set
 *           admin_status   -- VAL_ifAdminStatus_up/VAL_ifAdminStatus_down
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : 1. RFC2863
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetPortAdminStatus(UI32_T ifindex, UI32_T admin_status);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetPortStatus
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
BOOL_T SWCTRL_SetPortStatus(UI32_T ifindex, BOOL_T status, UI32_T reason);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetPortStatus
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get port administration status
 * INPUT   : ifindex        -- which port to set
 * OUTPUT  : shutdown_reason_p -- indicates why status is down
 *                             bitmap of SWCTRL_PORT_STATUS_SET_BY_XXX
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_GetPortStatus(UI32_T ifindex, UI32_T *shutdown_reason_p);

#if (SYS_CPNT_SUPPORT_FORCED_1000BASE_T_MODE == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetPort1000BaseTForceMode
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set speed/duplex configuration of a port
 * INPUT   : ifindex      -- which port to set
 *           forced_mode  -- master /slave
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : 1.ES3626A
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetPort1000BaseTForceMode(UI32_T ifindex, UI32_T forced_mode);
#endif

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetPortCfgSpeedDuplex
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set speed/duplex configuration of a port
 * INPUT   : ifindex      -- which port to set
 *           speed_duplex -- speed/duplex to set
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : 1.ES3626A
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetPortCfgSpeedDuplex(UI32_T ifindex, UI32_T speed_duplex);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetPortDefaultSpeedDuplex
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set port speed/duplex to default value
 * INPUT   : l_port      -- which port to set
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : for CLI use
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetPortDefaultSpeedDuplex(UI32_T l_port);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetPortAutoNegEnable
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set port auto-negotiation enable state
 * INPUT   : ifindex        -- which port to set
 *           autoneg_state  -- VAL_portAutonegotiation_enabled /
 *                             VAL_portAutonegotiation_disabled
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : 1. EA3626A
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetPortAutoNegEnable(UI32_T ifindex, UI32_T autoneg_state);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetPortCfgFlowCtrlEnable
 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable flow control of a port or not
 * INPUT   : ifindex -- which port to set
 *           flow_contrl_cfg    -- VAL_portFlowCtrlCfg_enabled /
                                   VAL_portFlowCtrlCfg_disabled
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : 1. ES3626A
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetPortCfgFlowCtrlEnable(UI32_T ifindex, UI32_T flow_control_cfg);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetPortAutoNegCapability
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
BOOL_T SWCTRL_SetPortAutoNegCapability(UI32_T ifindex, UI32_T capability);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetPortDefaultAutoNegCapability
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set the capability of auto-negotiation of a
 *           port to default value
 * INPUT   : ifindex
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : for CLI use
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetPortDefaultAutoNegCapability(UI32_T ifindex);


#if (SYS_CPNT_SWCTRL_FEC == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetPortFec
 * -------------------------------------------------------------------------
 * FUNCTION: To enable/disable FEC
 * INPUT   : ifindex
 *           fec_mode - VAL_portFecMode_XXX
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetPortFec(UI32_T ifindex, UI32_T fec_mode);
#endif


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetPortLinkChangeTrapEnable
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
BOOL_T SWCTRL_SetPortLinkChangeTrapEnable(UI32_T ifindex, UI32_T link_change_trap);

/*Charles: swctrl_support_dot1x
 */
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetPortDot1xEnable
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
BOOL_T SWCTRL_SetPortDot1xEnable(UI32_T ifindex, UI32_T dot1x_port_status);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetPortDot1xAuthState
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
BOOL_T SWCTRL_SetPortDot1xAuthState(UI32_T ifindex, UI32_T dot1x_auth_status);

#if (SYS_CPNT_STATIC_TRUNK_CONFIG_ALLOWED_ON_LACP_PORT == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetPortLacpOperEnable
 * -------------------------------------------------------------------------
 * FUNCTION: This function is used to set LACP oper status.
 * INPUT   : ifindex --- Which user port.
 *           lacp_oper_status --- VAL_lacpPortStatus_enabled/
 *                               VAL_lacpPortStatus_disabled
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : 1. For LACP only.
 *           2. User port only.
 *           3. SWCTRL use this oper status to run ifOperStatus state machine.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetPortLacpOperEnable(UI32_T ifindex, UI32_T lacp_oper_status);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetPortLacpAdminEnable
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
BOOL_T SWCTRL_SetPortLacpAdminEnable(UI32_T ifindex, UI32_T lacp_admin_status);

#else

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetPortLacpEnable
 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable/disable the LACP function of a port
 * INPUT   : ifindex        -- which port to set
 *           lacp_state     -- VAL_lacpPortStatus_enabled/
                               VAL_lacpPortStatus_disabled
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : 1.802.3ad
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetPortLacpEnable(UI32_T ifindex, UI32_T lacp_state);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetLacpEnable
 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable/disable the LACP function of whole system
 * INPUT   : lacp_state     -- VAL_lacpPortStatus_enabled/
 *                             VAL_lacpPortStatus_disabled
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : 1.802.3ad
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetLacpEnable(UI32_T lacp_state);
#endif /*(SYS_CPNT_STATIC_TRUNK_CONFIG_ALLOWED_ON_LACP_PORT == TRUE)*/

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetPortLacpAttach
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set the LACP state to acttached or deattached
 * INPUT   : ifindex        -- which port to set
 *           lacp_attach    -- VAL_LacpAttach_attached/
 *                             VAL_LacpAttach_deattached
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : 1.802.3ad
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetPortLacpAttach(UI32_T ifindex, UI32_T lacp_attach);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetPortLacpCollecting
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set the LACP state to collecting
 * INPUT   : ifindex            -- which port to set
 *           lacp_collecting    -- VAL_LacpCollecting_collecting/
 *                                 VAL_LacpCollecting_not_collecting
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : 1.802.3ad
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetPortLacpCollecting(UI32_T ifindex, UI32_T lacp_collecting);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetPortOperDormantStatus
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
BOOL_T SWCTRL_SetPortOperDormantStatus(
    UI32_T ifindex,
    SWCTRL_OperDormantLevel_T level,
    BOOL_T enable,
    BOOL_T stealth);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_TriggerPortOperDormantEvent
 * -------------------------------------------------------------------------
 * FUNCTION: To set status of ifOperStatus 'dormant'
 * INPUT   : ifindex - specified port to change status
 *           level   - see SWCTRL_OperDormantLevel_T
 *           event   - see SWCTRL_OperDormantEvent_T
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_TriggerPortOperDormantEvent(
    UI32_T ifindex,
    SWCTRL_OperDormantLevel_T level,
    SWCTRL_OperDormantEvent_T event);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetPortOperDormantStatus
 * -------------------------------------------------------------------------
 * FUNCTION: To get status of ifOperStatus 'dormant'
 * INPUT   : ifindex - specified port to change status
 *           enable  - TRUE/FALSE
 *           level   - see SWCTRL_OperDormantLevel_T
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_GetPortOperDormantStatus(
    UI32_T ifindex,
    SWCTRL_OperDormantLevel_T level,
    UI32_T *dormant_status_p,
    UI32_T *dormant_active_p);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetUnitPortNumber
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get the total number of ports in a specified
 *           unit
 * INPUT   : unit -- which unit to get
 * OUTPUT  : None
 * RETURN  : The total number of ports
 * NOTE    : None
 * -------------------------------------------------------------------------*/
UI32_T SWCTRL_GetUnitPortNumber(UI32_T unit);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetSystemPortNumber
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get the total number of ports in the system
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : The total number of ports
 * NOTE    : None
 * -------------------------------------------------------------------------*/
UI32_T SWCTRL_GetSystemPortNumber(void);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetLogicalPortNumber
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get the total number of logical ports in the
 *           system
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : The total number of ports
 * NOTE    : None
 * -------------------------------------------------------------------------*/
UI32_T SWCTRL_GetLogicalPortNumber(void);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_IsAnyLportOperUp
 * -------------------------------------------------------------------------
 * FUNCTION: This function will check if there is any port up in the port
 *           map
 * INPUT   : port_map -- a bit map of ports to check
 * OUTPUT  : None
 * RETURN  : TRUE: If any one up, FALSE: If none
 * NOTE    : This function will check total number of logical ports, so the
 *           map should be equal or larger than the number of logical ports.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_IsAnyLportOperUp(UI8_T *port_map);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetPortSTAState
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set STA state of a port
 * INPUT   : vid     -- which VLAN to set
 *           ifindex -- which port to set
 *           state   -- spanning tree state to set
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetPortSTAState(UI32_T vid,
                              UI32_T ifindex,
                              UI32_T state);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetPortXstpState
 * -------------------------------------------------------------------------
 * PURPOSE  : Set the Stp port state
 * INPUT    : xstid     -- multiple spanning tree instance identifier
 *            vlan_list -- UI8_T pointer for the bitmap of SYS_ADPT_MAX_VLAN_ID
 *                         bits.
 *                         Vlan bitmap for specified instance xstid:
 *                         If a bit in vlan list is 1, the corresponding VALN
 *                         is a member of this instance NO MATTER the spanning
 *                         tree is enabled or not.
 *                         Note that bit 0 of byte 0 is smallest VLAN (1).
 *                         i.e. BIT_0 of vlan_list[0] is VLAN 1.
 *            ifindex   -- ifindex of this logical port.
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
BOOL_T SWCTRL_SetPortXstpState (UI32_T xstid,
                                UI8_T *vlan_list,
                                UI32_T ifindex,
                                UI32_T state);


#if (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_MSTP)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetMstEnableStatus
 * -------------------------------------------------------------------------
 * FUNCTION: This function will is used to set the spanning tree protocal
 * INPUT   : mst_enable  -- SWCTRL_MST_DISABLE
 *                          SWCTRL_MST_ENABLE
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetMstEnableStatus(UI32_T mst_enable_status);
#endif /* end of #if (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_MSTP)*/


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetCpuMac
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get the MAC address of CPU
 * INPUT   : None
 * OUTPUT  : mac -- the buffer to put MAC address
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_GetCpuMac(UI8_T *mac);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetPortMac
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get the MAC address of a port
 * INPUT   : ifindex -- which port to get
 * OUTPUT  : mac     -- the buffer to put MAC address
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_GetPortMac(UI32_T ifindex, UI8_T *mac);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetLastChangeTime
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get the last change time of whole system
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : the time of last change of any port
 * NOTE    : None
 * -------------------------------------------------------------------------*/
UI32_T SWCTRL_GetLastChangeTime();




/****************************************************************************/
/* VLAN                                                                     */
/****************************************************************************/
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetPortPVID
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set default VLAN ID of a port
 * INPUT   : ifindex -- which port to set
 *           pvid    -- permanent VID to set
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetPortPVID(UI32_T ifindex, UI32_T pvid,BOOL_T port_check);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_CreateVlan
 * -------------------------------------------------------------------------
 * FUNCTION: This function will create a specified VLAN
 * INPUT   : vid -- which VLAN to create
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not availabl
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_CreateVlan(UI32_T vid);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_DestroyVlan
 * -------------------------------------------------------------------------
 * FUNCTION: This function will delete a specified VLAN
 * INPUT   : vid -- which VLAN to delete
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not availabl
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_DestroyVlan(UI32_T vid);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetGlobalDefaultVlan
 * -------------------------------------------------------------------------
 * FUNCTION: This function changes the global default VLAN
 * INPUT   : vid                -- the vid of the new default VLAN
 * OUTPUT  : None
 * RETURN  : TRUE               -- Success
 *           FALSE              -- If the specified VLAN is not available.
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetGlobalDefaultVlan(UI32_T vid);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_AddPortToVlanMemberSet
 * -------------------------------------------------------------------------
 * FUNCTION: This function will add a port to the member set of a specified
 *           VLAN
 * INPUT   : ifindex -- which port to add
 *           vid     -- which VLAN ID
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_AddPortToVlanMemberSet(UI32_T ifindex, UI32_T vid,BOOL_T port_check);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_AddTrunkMemberPortToVlanMemberSet
 * -------------------------------------------------------------------------
 * FUNCTION: This function will add a port to the member set of a specified
 *           VLAN
 * INPUT   : ifindex -- which port to add
 *           vid     -- which VLAN ID
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : This function is used only for vlan to add the trunk member from vlan
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_AddTrunkMemberPortToVlanMemberSet(UI32_T trunk_mem_ifindex, UI32_T vid);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_DeletePortFromVlanMemberSet
 * -------------------------------------------------------------------------
 * FUNCTION: This function will delete a port from the member set of a
 *           specified VLAN
 * INPUT   : ifindex -- which port to delete
 *           vid     -- which VLAN ID
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_DeletePortFromVlanMemberSet(UI32_T ifindex, UI32_T vid,BOOL_T port_check);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_DeleteTrunkMemberPortFromVlanMemberSet
 * -------------------------------------------------------------------------
 * FUNCTION: This function will delete a trunk member port from the member set of a
 *           specified VLAN
 * INPUT   : ifindex -- which port to delete
 *           vid     -- which VLAN ID
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : This function is used only for vlan to remove the trunk member from vlan
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_DeleteTrunkMemberPortFromVlanMemberSet(UI32_T trunk_mem_ifindex, UI32_T vid);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_AddPortToVlanUntaggedSet
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set a port to output untagged frames over
 *           the specified VLAN
 * INPUT   : ifindex -- which port to add
 *           vid     -- which VLAN ID
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_AddPortToVlanUntaggedSet(UI32_T ifindex, UI32_T vid,BOOL_T port_check);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_AddTrunkMembersPortToVlanUntaggedSet
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set a port to output untagged frames over
 *           the specified VLAN
 * INPUT   : ifindex -- which port to add
 *           vid     -- which VLAN ID
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : This function is used only for vlan to add the trunk member from vlan
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_AddTrunkMembersPortToVlanUntaggedSet(UI32_T trunk_mem_ifindex, UI32_T vid);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_DeletePortFromVlanUntaggedSet
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
BOOL_T SWCTRL_DeletePortFromVlanUntaggedSet(UI32_T ifindex, UI32_T vid,BOOL_T port_check);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_DeletePortFromVlanUntaggedSet
 * -------------------------------------------------------------------------
 * FUNCTION: This function will delete a port from the untagged set of a
 *           specified VLAN
 * INPUT   : ifindex -- which port to add
 *           vid     -- which VLAN ID
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : This function is used only for vlan to remove the trunk member from vlan
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_DeleteTrunkMemberPortFromVlanUntaggedSet(UI32_T trunk_mem_ifindex, UI32_T vid);

BOOL_T  SWCTRL_GetSystemMTU(UI32_T *jumbo,UI32_T *mtu);


BOOL_T SWCTRL_SetSystemMTU(UI32_T status,UI32_T mtu);
BOOL_T SWCTRL_SetPortMTU(UI32_T ifindex,UI32_T MTU);


/* -------------------------------------------------------------------------
 * FUNCTION NAME - SWCTRL_GetPortMaxFrameSize
 * -------------------------------------------------------------------------
 * PURPOSE : to get max frame size of port
 * INPUT   : ifindex                 - ifindex
 * OUTPUT  : untagged_max_frame_sz_p - max frame size for untagged frames
 *           tagged_max_frame_sz_p   - max frame size for tagged frames
 * RETURN  : TRUE / FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SWCTRL_GetPortMaxFrameSize(UI32_T ifindex, UI32_T *untagged_max_frame_sz_p, UI32_T *tagged_max_frame_sz_p);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_EnableIngressFilter
 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable ingress filter of a port
 * INPUT   : ifindex -- which port to enable
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_EnableIngressFilter(UI32_T ifindex);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_EnableIngressFilter
 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable ingress filter of a port
 * INPUT   : ifindex -- which port to enable
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_EnableIngressFilterForTrunkMember(UI32_T ifindex);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_DisableIngressFilter
 * -------------------------------------------------------------------------
 * FUNCTION: This function will disable ingress filter of a port
 * INPUT   : ifindex -- which port to disable
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_DisableIngressFilter(UI32_T ifindex);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_DisableIngressFilterForTrunkMember
 * -------------------------------------------------------------------------
 * FUNCTION: This function will disable ingress filter of a port
 * INPUT   : ifindex -- which port to disable
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_DisableIngressFilterForTrunkMember(UI32_T trunk_mem_ifindex);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_AdmitVLANTaggedFramesOnlyForTrunkMember
 * -------------------------------------------------------------------------
 * FUNCTION: This function will only allow tagged frames entering a port
 * INPUT   : ifindex -- which port to set
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_AdmitVLANTaggedFramesOnlyForTrunkMember(UI32_T trunk_mem_ifindex);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_AdmitVLANTaggedFramesOnly
 * -------------------------------------------------------------------------
 * FUNCTION: This function will only allow tagged frames entering a port
 * INPUT   : ifindex -- which port to set
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_AdmitVLANTaggedFramesOnly(UI32_T ifindex);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_AdmitVLANUntaggedFramesOnly
 * -------------------------------------------------------------------------
 * FUNCTION: This function will only allow untagged frames entering a port
 * INPUT   : ifindex -- which port to set
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_AdmitVLANUntaggedFramesOnly(UI32_T ifindex);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_AdmitAllFramesForTrunkMember
 * -------------------------------------------------------------------------
 * FUNCTION: This function will allow all kinds of frames entering a port
 * INPUT   : ifindex -- which port to set
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_AdmitAllFramesForTrunkMember(UI32_T trunk_mem_ifindex);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_AdmitAllFrames
 * -------------------------------------------------------------------------
 * FUNCTION: This function will allow all kinds of frames entering a port
 * INPUT   : ifindex -- which port to set
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_AdmitAllFrames(UI32_T ifindex);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_AddHostToVlan

 * -------------------------------------------------------------------------
 * FUNCTION: This function will add CPU to a specified VLAN
 * INPUT   : vid -- which VLAN to add
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_AddHostToVlan(UI32_T vid);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_DeleteHostFromVlan

 * -------------------------------------------------------------------------
 * FUNCTION: This function will delete CPU from a specified VLAN
 * INPUT   : vid -- which VLAN to delete
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_DeleteHostFromVlan(UI32_T vid);




/****************************************************************************/
/* Port Mirroring                                                           */
/****************************************************************************/
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_IsAnalyzerPort
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get if this port is an analyzer port
 * INPUT   : unit_port -- the port to get
 * OUTPUT  : None
 * RETURN  : TRUE: Is an analyzer port, FALSE: If not
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_IsAnalyzerPort(SYS_TYPE_Uport_T unit_port);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_IsMonitoredPort
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get if this port is an monitored port
 * INPUT   : unit_port -- the port to get
 * OUTPUT  : None
 * RETURN  : TRUE: Is an monitored port, FALSE: If not
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_IsMonitoredPort(SYS_TYPE_Uport_T unit_port);




/****************************************************************************/
/* Trunking                                                                 */
/****************************************************************************/
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_CreateTrunk
 * -------------------------------------------------------------------------
 * FUNCTION: This function will create a trunking port
 * INPUT   : trunk_id -- which trunking port to create
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_CreateTrunk(UI32_T trunk_id, BOOL_T is_static);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_DestroyTrunk
 * -------------------------------------------------------------------------
 * FUNCTION: This function will destroy a trunking port
 * INPUT   : trunk_id -- which trunking port to destroy
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_DestroyTrunk(UI32_T trunk_id);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_AllowToBeTrunkMember
 * -------------------------------------------------------------------------
 * FUNCTION: This function will return TRUE if the port is allowed to add
 *           into trunk
 * INPUT   : unit -- unit number
 *           port -- which port to check
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_AllowToBeTrunkMember(UI32_T unit, UI32_T port);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_AddTrunkMember
 * -------------------------------------------------------------------------
 * FUNCTION: This function will add a members to a trunk
 * INPUT   : trunk_id   -- which trunking port to set
 *           unit_port  -- member to add
 *           is_static  -- static or dynamic trunk
 *           is_active  -- active or inactive member
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_AddTrunkMember(UI32_T trunk_id, SYS_TYPE_Uport_T unit_port, BOOL_T is_static, BOOL_T is_active);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_DeleteTrunkMember
 * -------------------------------------------------------------------------
 * FUNCTION: This function will delete a members to a trunk
 * INPUT   : trunk_id   -- which trunking port to set
 *           unit_port  -- member to delete
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_DeleteTrunkMember(UI32_T trunk_id, SYS_TYPE_Uport_T unit_port);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetTrunkStatus
 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable/disable a specific trunk
 * INPUT   : trunk_id     -- which trunking port to enable
 *           trunk_status -- VAL_trunkStatus_valid / VAL_trunkStatus_invalid
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : ES3626A-MIB
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetTrunkStatus(UI32_T trunk_id, UI32_T trunk_status);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetActiveTrunkMember
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get trunk members which are active
 * INPUT   : trunk_ifindex      -- which interface index
 * OUTPUT  : active_lportarray   -- the active trunk member port array
 *           active_lport_count -- the number of active trunk member
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_GetActiveTrunkMember(
        UI32_T        trunk_ifindex,
        UI32_T         active_lportarray[SYS_ADPT_MAX_NBR_OF_PORT_PER_TRUNK],
        UI32_T        *active_lport_count);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetTrunkPortExtInfo
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get trunk information
 * INPUT   : ifindex               -- which interface index
 * OUTPUT  : trunk_port_ext_info   -- trunk information
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : only for TRK_MGR
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_GetTrunkPortExtInfo(UI32_T                       ifindex,
                                  SWCTRL_TrunkPortExtInfo_T    *trunk_port_ext_info);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_L2LoadBalance
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get the load balance trunk member port
 * INPUT   : mac                -- mac address
 *           trunk_ifindex      -- which interface index
 *           algorithm          -- algoritem
 * OUTPUT  : trunk_member_ifindex -- the selected trunk member port
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_L2LoadBalance(UI8_T *mac, UI32_T trunk_ifindex, UI32_T algorithm, UI32_T *trunk_member_ifindex);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetTrunkBalanceMode
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
BOOL_T SWCTRL_SetTrunkBalanceMode(UI32_T balance_mode);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetTrunkBalanceMode
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
BOOL_T SWCTRL_GetTrunkBalanceMode(UI32_T *balance_mode_p);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetRunningTrunkBalanceMode
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
SYS_TYPE_Get_Running_Cfg_T SWCTRL_GetRunningTrunkBalanceMode(UI32_T *balance_mode_p);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetTrunkMaxNumOfActivePorts
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set max number of active ports of trunk
 * INPUT   : trunk_id
 *           max_num_of_active_ports
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetTrunkMaxNumOfActivePorts(UI32_T trunk_id, UI32_T max_num_of_active_ports);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetTrunkMaxNumOfActivePorts
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set max number of active ports of trunk
 * INPUT   : trunk_id
 * OUTPUT  : max_num_of_active_ports_p
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_GetTrunkMaxNumOfActivePorts(UI32_T trunk_id, UI32_T *max_num_of_active_ports_p);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetRunningTrunkMaxNumOfActivePorts
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set max number of active ports of trunk
 * INPUT   : trunk_id
 * OUTPUT  : max_num_of_active_ports_p
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T SWCTRL_GetRunningTrunkMaxNumOfActivePorts(UI32_T trunk_id, UI32_T *max_num_of_active_ports_p);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetTrunkMemberActiveStatus
 * -------------------------------------------------------------------------
 * FUNCTION: This function will add a members to a trunk
 * INPUT   : trunk_id   -- which trunking port to set
 *           unit_port  -- member to add
 *           is_active  -- active or inactive member
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetTrunkMemberActiveStatus(UI32_T trunk_id, SYS_TYPE_Uport_T unit_port, BOOL_T is_active);

/****************************************************************************/
/* IGMP                                                                     */
/****************************************************************************/
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_EnableIgmpTrap
 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable the IGMP function
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_EnableIgmpTrap(SWCTRL_TrapPktOwner_T owner);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_DisableIgmpTrap
 * -------------------------------------------------------------------------
 * FUNCTION: This function will disable the IGMP function
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_DisableIgmpTrap(SWCTRL_TrapPktOwner_T owner);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetUnknownIPMcastFwdPortList
 * -------------------------------------------------------------------------
 * FUNCTION: Set the unknown multicast packet forwarding-to port list.
 * INPUT   : port_list  - on which the multicast packets allow to forward-to
 * OUTPUT  : none
 * RETURN  : TRUE/FALSE
 * NOTE    : To determine which ports are allow to forward the unknow IPMC
 *           packets.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetUnknownIPMcastFwdPortList(UI8_T port_list[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST]);


/****************************************************************************/
/* Broadcast/Multicast Storm Control                                                  */
/****************************************************************************/
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetBStormControlRateLimit
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
BOOL_T SWCTRL_SetBStormControlRateLimit(UI32_T ifindex,
                                        UI32_T mode,
                                        UI32_T nRate);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetMStormControlRateLimit
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
BOOL_T SWCTRL_SetMStormControlRateLimit(UI32_T ifindex,
                                        UI32_T mode,
                                        UI32_T nRate);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetBroadcastStormStatus
 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable the broadcast storm control function
 * INPUT   : ifindex -- which port to set
 *           broadcast_storm_status -- VAL_bcastStormStatus_enabled
 *                                     VAL_bcastStormStatus_disabled
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetBroadcastStormStatus(UI32_T ifindex, UI32_T broadcast_storm_status);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetMulticastStormStatus
 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable the multicast storm control function
 * INPUT   : ifindex -- which port to set
 *           multicast_storm_status -- VAL_bcastStormStatus_enabled
 *                                     VAL_bcastStormStatus_disabled
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetMulticastStormStatus(UI32_T ifindex, UI32_T multicast_storm_status);

#if (SYS_CPNT_STORM_MODE & SYS_CPNT_STORM_UNKNOWN_USTORM)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetUnknownUStormControlRateLimit
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
BOOL_T SWCTRL_SetUnknownUStormControlRateLimit(UI32_T ifindex,
                                               UI32_T mode,
                                               UI32_T nRate);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetUnknownUnicastStormStatus
 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable the unknown unicast(DLF) storm control function
 * INPUT   : ifindex -- which port to set
 *           multicast_storm_status -- VAL_unknownUcastStormStatus_enabled
 *                                     VAL_unknownUcastStormStatus_disabled
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetUnknownUnicastStormStatus(UI32_T ifindex, UI32_T unknown_unicast_storm_status);
#endif

#if (SYS_CPNT_SWCTRL_GLOBAL_STORM_SAMPLE_TYPE == TRUE)
/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetGlobalStormSampleType
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
BOOL_T SWCTRL_SetGlobalStormSampleType(UI32_T global_storm_sample_type);

/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetRunningGlobalStormSampleType
 *------------------------------------------------------------------------
 * FUNCTION: This function will get running config of global storm sample type
 * INPUT   : None
 * OUTPUT  : global_storm_sample_type
 * RETURN  : SYS_TYPE_Get_Running_Cfg_T
 * NOTE    : None
 *------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T SWCTRL_GetRunningGlobalStormSampleType(UI32_T *global_storm_sample_type_p);
#endif /* (SYS_CPNT_SWCTRL_GLOBAL_STORM_SAMPLE_TYPE == TRUE) */


/****************************************************************************/
/* Quality of Service                                                       */
/****************************************************************************/
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetPortUserDefaultPriority
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set user default priority
 * INPUT   : ifindex  -- which port to set
 *           priority -- user default priority to set
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If priority is not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetPortUserDefaultPriority(UI32_T ifindex, UI32_T priority);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetPriorityMapping
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set priority mapping of 10/100 ports
 * INPUT   : ifindex -- which port to set
 *           mapping -- priority mapping to set
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If priority is not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetPriorityMapping(UI32_T ifindex,
                                 UI8_T mapping[8]);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetPriorityMappingPerSystem
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set priority mapping of per-system
 * INPUT   : mapping -- priority mapping to set
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If priority is not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetPriorityMappingPerSystem(UI8_T mapping[8]);

#if (SYS_CPNT_STACKING == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetStackingPortPriorityMapping
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set priority mapping of stacking port
 * INPUT   : ifindex -- which port to set
 *           mapping -- priority mapping to set
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If priority is not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetStackingPortPriorityMapping(UI32_T ifindex,
                                             UI8_T mapping[8]);
#endif


/****************************************************************************/
/* COS                                                                      */
/****************************************************************************/
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_EnableTosCosMap
 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable TOS/COS mapping of system
 * INPUT   : none
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : 1.ES3626A
 * -------------------------------------------------------------------------*/
void SWCTRL_EnableTosCosMap();

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_DisableTosCosMap
 * -------------------------------------------------------------------------
 * FUNCTION: This function will disable TOS/COS mapping of system
 * INPUT   : none
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : 1.ES3626A
 * -------------------------------------------------------------------------*/
void SWCTRL_DisableTosCosMap();

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_EnableDscpCosMap
 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable DSCP/COS mapping of system
 * INPUT   : none
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : 1.ES3626A
 * -------------------------------------------------------------------------*/
void SWCTRL_EnableDscpCosMap();

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_DisableDscpCosMap
 * -------------------------------------------------------------------------
 * FUNCTION: This function will disable DSCP/COS mapping of system
 * INPUT   : none
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : 1.ES3626A
 * -------------------------------------------------------------------------*/
void SWCTRL_DisableDscpCosMap();


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_EnableTcpPortCosMap
 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable TCP_PORT/COS mapping of system
 * INPUT   : none
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : 1.ES3626A
 * -------------------------------------------------------------------------*/
void SWCTRL_EnableTcpPortCosMap();

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_DisableTcpPortCosMap
 * -------------------------------------------------------------------------
 * FUNCTION: This function will disable TCP_PORT/COS mapping of system
 * INPUT   : none
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : 1.ES3626A
 * -------------------------------------------------------------------------*/
void SWCTRL_DisableTcpPortCosMap();


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetPortTosCosMap
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set one entry of TOS/COS mapping of a port
 * INPUT   : ifindex      -- which port to set
 *           tos          -- from which tos value will be mapped
 *           cos_priority -- to which priority will be mapped
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: Interface not available or asci config fail
 * NOTE    : 1.ES3626A
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetPortTosCosMap(UI32_T ifindex, UI32_T tos, UI32_T cos_priority);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetPortDscpCosMap
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set one entry of DSCP/COS mapping of a port
 * INPUT   : ifindex      -- which port to set
 *           dscp         -- from which dscp value will be mapped
 *           cos_priority -- to which priority will be mapped
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: Interface not available or asci config fail
 * NOTE    : 1.ES3626A
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetPortDscpCosMap(UI32_T ifindex, UI32_T dscp, UI32_T cos_priority);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetPortTcpPortCosMap
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set one entry of TCP_PORT/COS mapping of a port
 * INPUT   : ifindex      -- which port to set
 *           tcp_port     -- from which dscp value will be mapped
 *           cos_priority -- to which priority will be mapped
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: Interface not available or asci config fail
 * NOTE    : 1.ES3626A
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetPortTcpPortCosMap(UI32_T ifindex, UI32_T tcp_port, UI32_T cos_priority);




/****************************************************************************/
/* Egress Scheduler Priority                                                                      */
/****************************************************************************/

#if (SYS_CPNT_WRR_Q_MODE_PER_PORT_CTRL == FALSE)
/*------------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetPrioQueueMode
 *------------------------------------------------------------------------------
 * PURPOSE:  This function will set the port egress schedulering method
 * INPUT:    mode   -- SWCTRL_WEIGHT_ROUND_ROBIN_METHOD,
 *                     SWCTRL_STRICT_PRIORITY_METHOD,
 *                     following is for firebolt2
 *                     SWCTRL_DEFICIT_ROUND_RBIN_METHOD,
 *                     SWCTRL_SP_WRR_METHOD,
 *                     SWCTRL_SP_DRR_METHOD
 * OUTPUT  : None
 * RETURN:   TRUE/FALSE
 * NOTE:
 *------------------------------------------------------------------------------
 */
BOOL_T SWCTRL_SetPrioQueueMode(UI32_T mode);
#else
/*--------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetPortPrioQueueMode
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set the port egress schedulering method
 * INPUT:    l_port -- which port to set
 *       mode   -- SWCTRL_STRICT_MODE / SWCTRL_WEIGHT_FAIR_ROUND_ROBIN_MODE
 * OUTPUT  : None
 * RETURN:   TRUE/FALSE
 * NOTE:
 *---------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetPortPrioQueueMode(UI32_T l_port, UI32_T mode);
#endif

#if 0
/*--------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetPortPrioQueueMode
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the port egress schedulering method
 * INPUT:    l_port-- which port to get
 * OUTPUT:   mode -- SWCTRL_STRICT_MODE / SWCTRL_WEIGHT_FAIR_ROUND_ROBIN_MODE
 * RETURN:   TRUE / FALSE
 * NOTE:
 *---------------------------------------------------------------------------*/
BOOL_T SWCTRL_GetPortPrioQueueMode(UI32_T l_port, UI32_T *mode);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetNextPortPrioQueueMode
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the next port egress schedulering method
 * INPUT:    l_port-- which port to get
 * OUTPUT:   l_port-- the next port
 *       mode -- SWCTRL_STRICT_MODE / SWCTRL_WEIGHT_FAIR_ROUND_ROBIN_MODE
 * RETURN:   TRUE / FALSE
 * NOTE:
 *---------------------------------------------------------------------------*/
BOOL_T SWCTRL_GetNextPortPrioQueueMode(UI32_T *l_port, UI32_T *mode);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetPortStrictQueueMap
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the port strict queue map.
 * INPUT:    l_port-- which port to get
 * OUTPUT:   strict queue map
 * RETURN:   TRUE / FALSE
 * NOTE:
 *---------------------------------------------------------------------------*/
BOOL_T SWCTRL_GetPortStrictQueueMap(UI32_T l_port, UI8_T *map);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetRunningPortPrioQueueMode
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the port egress schedulering method
 * INPUT:    ifindex -- which port to get
 * OUTPUT:   mode -- SWCTRL_STRICT_MODE / SWCTRL_WEIGHT_FAIR_ROUND_ROBIN_MODE
 * RETURN:   SYS_TYPE_GET_RUNNING_CFG_FAIL -- error (system is not in MASTER mode)
 *                SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 *                SYS_TYPE_GET_RUNNING_CFG_SUCCESS -- different from default value
 * NOTE:
 *---------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T SWCTRL_GetRunningPortPrioQueueMode(UI32_T l_port, UI32_T *mode);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetNextRunningPortPrioQueueMode
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the next port egress schedulering method
 * INPUT:    l_port-- which port to get
 * OUTPUT:   mode -- SWCTRL_STRICT_MODE / SWCTRL_WEIGHT_FAIR_ROUND_ROBIN_MODE
 * RETURN:   SYS_TYPE_GET_RUNNING_CFG_FAIL -- error (system is not in MASTER mode)
 *                SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 *                SYS_TYPE_GET_RUNNING_CFG_SUCCESS -- different from default value
 * NOTE:
 *---------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T SWCTRL_GetNextRunningPortPrioQueueMode(UI32_T *l_port, UI32_T *mode);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetWrrQueueWeight
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set the weight of queue bandwidths
 * INPUT:    q_id       -- This is the second key and represent the index of wrr queue
 *           weight     -- The weight of (l_port, q_id)
 * OUTPUT  : None
 * RETURN:   TRUE/FALSE
 * NOTE:  The ratio of weight determines the weight of the WRR scheduler.
 *        Driver maybe need to provide API to enable WRR of ASIC
 *---------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetWrrQueueWeight(UI32_T q_id, UI32_T weight);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetWrrQueueWeight
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the current weight of queue bandwidths
 * INPUT:    q_id       -- This is the second key and represent the index of wrr queue
 * OUTPUT:   weight     -- The weight of (l_port, q_id)
 * RETURN:   TRUE/FALSE
 * NOTE:  The ratio of weight determines the weight of the WRR scheduler.
 *---------------------------------------------------------------------------*/
BOOL_T SWCTRL_GetWrrQueueWeight(UI32_T q_id, UI32_T *weight);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetNextWrrQueueWeight
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the current weight of queue bandwidths
 * INPUT:    q_id       -- This is the second key and represent the index of wrr queue
 * OUTPUT:   weight     -- The weight of (l_port, q_id)
 * RETURN:   TRUE/FALSE
 * NOTE:  The ratio of weight determines the weight of the WRR scheduler.
 *---------------------------------------------------------------------------*/
BOOL_T SWCTRL_GetNextWrrQueueWeight(UI32_T *q_id, UI32_T *weight);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetRunningWrrQueueWeight
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the current weight of WRR queues
 * INPUT:    q_id       -- This is the second key and represent the index of wrr queue
 * OUTPUT:   weight     -- The weight of (l_port, q_id)
 * RETURN:   SYS_TYPE_GET_RUNNING_CFG_FAIL -- error (system is not in MASTER mode)
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 *           SYS_TYPE_GET_RUNNING_CFG_SUCCESS -- different from default value
 * NOTE:
 *---------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T SWCTRL_GetRunningWrrQueueWeight(UI32_T q_id, UI32_T *weight);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SWCTRL_GetNextRunningWrrQueueWeight
 * ---------------------------------------------------------------------
 * PURPOSE: This function same as GetRunning but also output next index
 * INPUT:    q_id       -- This is the second key and represent the index of wrr queue
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
SYS_TYPE_Get_Running_Cfg_T SWCTRL_GetNextRunningWrrQueueWeight(UI32_T *q_id, UI32_T *weight);
#endif

#if (SYS_CPNT_WRR_Q_WEIGHT_PER_PORT_CTRL == TRUE)
/*--------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetPortWrrQueueWeight
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set the weight of queue bandwidths by por
 * INPUT:    l_port     -- This is the primary key and represent the logical port number
 *           q_id       -- This is the second key and represent the index of wrr queue
 *           weight     -- The weight of (l_port, q_id)
 * OUTPUT:   None
 * RETURN:   TRUE/FALSE
 * NOTE:  The ratio of weight determines the weight of the WRR scheduler.
 *        Driver maybe need to provide API to enable WRR of ASIC
 *---------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetPortWrrQueueWeight(UI32_T l_port, UI32_T q_id, UI32_T weight);
#else
/*------------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetWrrQueueWeight
 *------------------------------------------------------------------------------
 * PURPOSE:  This function will set the weight of queue bandwidths
 * INPUT:    q_id       -- This is the second key and represent the index of
 *                         wrr queue.
 *           weight     -- The weight of (l_port, q_id).
 * OUTPUT:   None
 * RETURN:   TRUE/FALSE
 * NOTE:     The ratio of weight determines the weight of the WRR scheduler.
 *           Driver maybe need to provide API to enable WRR of ASIC
 *------------------------------------------------------------------------------
 */
BOOL_T SWCTRL_SetWrrQueueWeight(UI32_T q_id, UI32_T weight);
#endif

#if 0
/*--------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetWrrQueueWeight
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set the weight of queue bandwidths
 * INPUT:    q_id       -- This is the second key and represent the index of wrr queue
 *           weight     -- The weight of (l_port, q_id)
 * OUTPUT:   None
 * RETURN:   TRUE/FALSE
 * NOTE:  The ratio of weight determines the weight of the WRR scheduler.
 *        Driver maybe need to provide API to enable WRR of ASIC
 *---------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetWrrQueueWeight(UI32_T q_id, UI32_T weight);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_ResetPortWrrQueueWeight
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will reset the current weight to default
 * INPUT:    l_port     -- This is the primary key and represent the logical port number
 *           q_id       -- This is the second key and represent the index of wrr queue
 * OUTPUT:   none
 * RETURN:   TRUE/FALSE
 * NOTE:  The default ratio of weight is defined in sys_dflt.h
 *        Driver maybe need to provide API to disable WRR of ASIC
 *---------------------------------------------------------------------------*/
BOOL_T SWCTRL_ResetPortWrrQueueWeight(UI32_T l_port, UI32_T q_id);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetPortWrrQueueWeightStatus
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the current status of the WRR queue bandwidths
 * INPUT:     None
 * OUTPUT:  None
 * RETURN:   TRUE/FALSE
 * NOTE:      None
 *---------------------------------------------------------------------------*/
BOOL_T SWCTRL_GetPortWrrQueueWeightStatus(BOOL_T *status);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetPortWrrQueueWeight
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the current weight of queue bandwidths
 * INPUT:    l_port     -- This is the primary key and represent the logical port number
 *           q_id       -- This is the second key and represent the index of wrr queue
 * OUTPUT:   weight     -- The weight of (l_port, q_id)
 * RETURN:   TRUE/FALSE
 * NOTE:  The ratio of weight determines the weight of the WRR scheduler.
 *---------------------------------------------------------------------------*/
BOOL_T SWCTRL_GetPortWrrQueueWeight(UI32_T l_port, UI32_T q_id, UI32_T *weight);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetNextPortWrrQueueWeight
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
BOOL_T SWCTRL_GetNextPortWrrQueueWeight(UI32_T *l_port, UI32_T *q_id, UI32_T *weight);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetRunningPortWrrQueueWeight
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
SYS_TYPE_Get_Running_Cfg_T SWCTRL_GetRunningPortWrrQueueWeight(UI32_T l_port, UI32_T q_id, UI32_T *weight);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SWCTRL_GetNextRunningPortWrrQueueWeight
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
SYS_TYPE_Get_Running_Cfg_T SWCTRL_GetNextRunningPortWrrQueueWeight(UI32_T *l_port , UI32_T *q_id, UI32_T *weight);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetRunningPortWrrQueueWeightStatus
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the current status of WRR queues
 * INPUT:      None
 * OUTPUT:   None
 * RETURN:   SYS_TYPE_GET_RUNNING_CFG_FAIL -- error (system is not in MASTER mode)
 *                SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 *                SYS_TYPE_GET_RUNNING_CFG_SUCCESS -- different from default value
 * NOTE:
 *---------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T SWCTRL_GetRunningPortWrrQueueWeightStatus(BOOL_T *status);
#endif

BOOL_T SWCTRL_DelPortTosCosMap(UI32_T ifindex, UI32_T tos);
BOOL_T SWCTRL_DelPortDscpCosMap(UI32_T ifindex, UI32_T dscp);
BOOL_T SWCTRL_DelPortTcpPortCosMap(UI32_T ifindex, UI32_T tcp_port);


#if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION == TRUE)
/*********************
 * Private VLAN APIs *
 *********************/
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_EnablePrivateVlan
 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable the private vlan
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_EnablePrivateVlan();

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_DisablePrivateVlan
 * -------------------------------------------------------------------------
 * FUNCTION: This function will disable the private vlan
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_DisablePrivateVlan();


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetPrivateVlan
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set the private vlan
 * INPUT   : uplink_port_list  -- uplink port list
 *           downlink_port_list -- downlink port list
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetPrivateVlan(UI8_T uplink_port_list[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST],
                             UI8_T downlink_port_list[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST]);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_AddPrivateVlanUplinkPortMember
 * -------------------------------------------------------------------------
 * FUNCTION: This function will add port into the private vlan uplink member list
 * INPUT   : ifindex -- which port to add
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_AddPrivateVlanUplinkPortMember(UI32_T ifindex);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_DeletePrivateVlanUplinkPortMember
 * -------------------------------------------------------------------------
 * FUNCTION: This function will delete the port from private vlan uplink list
 * INPUT   : ifindex -- which port to delete
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_DeletePrivateVlanUplinkPortMember(UI32_T ifindex);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_AddPrivateVlanDownlinkPortMember
 * -------------------------------------------------------------------------
 * FUNCTION: This function will add port into the private vlan downlink member list
 * INPUT   : ifindex -- which port to add
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_AddPrivateVlanDownlinkPortMember(UI32_T ifindex);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_DeletePrivateVlanDownlinkPortMember
 * -------------------------------------------------------------------------
 * FUNCTION: This function will delete the port from private vlan downlink list
 * INPUT   : ifindex -- which port to delete
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_DeletePrivateVlanDownlinkPortMember(UI32_T ifindex);

#if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION == TRUE)
#if (SYS_DFLT_TRAFFIC_SEG_METHOD == SYS_DFLT_TRAFFIC_SEG_METHOD_PORT_PRIVATE_MODE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetPortPrivateMode
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
BOOL_T SWCTRL_SetPortPrivateMode(UI32_T ifindex, UI32_T port_private_mode);

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
BOOL_T SWCTRL_GetPortPrivateMode(UI32_T ifindex, UI32_T *port_private_mode);

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
BOOL_T SWCTRL_GetNextPortPrivateMode(UI32_T *ifindex, UI32_T *port_private_mode);

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
SYS_TYPE_Get_Running_Cfg_T SWCTRL_GetRunningPortPrivateMode(UI32_T ifindex, UI32_T *port_private_mode);
#endif
#endif

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetPrivateVlan
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get the private vlan
 * INPUT   : None
 * OUTPU   : uplink_port_list  -- uplink port list
 *           downlink_port_list -- downlink port list
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_GetPrivateVlan(SWCTRL_PrivateVlan_T *private_vlan);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetPrivateVlanStatus
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get the private vlan status
 * INPUT   : None
 * OUTPU   : vlan_status -- enable/disable
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_GetPrivateVlanStatus(UI32_T *vlan_status);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetRunningPrivateVlanStatus
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get the private vlan running config
 * INPUT   : None
 * OUTPU   : vlan_status -- enable/disable
 * RETURN  : SYS_TYPE_Get_Running_Cfg_T
 * NOTE    : None
 * -------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T SWCTRL_GetRunningPrivateVlanStatus(UI32_T *vlan_status);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetRunningPrivateVlanUplinkPortList
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get the private vlan running config
 * INPUT   : None
 * OUTPU   : uplink_port_list -- uplink port list
 * RETURN  : SYS_TYPE_Get_Running_Cfg_T
 * NOTE    : None
 * -------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T SWCTRL_GetRunningPrivateVlanUplinkPortList(
                                UI8_T uplink_port_list[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST]);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetRunningPrivateVlanDownlinkPortList
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get the private vlan running config
 * INPUT   : None
 * OUTPU   : downlink_port_list -- downlink port list
 * RETURN  : SYS_TYPE_Get_Running_Cfg_T
 * NOTE    : None
 * -------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T SWCTRL_GetRunningPrivateVlanDownlinkPortList(
                                UI8_T downlink_port_list[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST]);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - SWCTRL_IsPortPrivateVlanUplinkMember
 *--------------------------------------------------------------------------
 * PURPOSE  : This funcion returns true if current port is in private vlan's
 *            uplink member list.  Otherwise, returns false.
 * INPUT    : ifindex -- the specified port
 * OUTPUT   : none
 * RETURN   : TRUE \ FALSE
 * NOTES    : none
 *--------------------------------------------------------------------------*/
BOOL_T SWCTRL_IsPortPrivateVlanUplinkMember(UI32_T ifindex);


/*--------------------------------------------------------------------------
 * FUNCTION NAME - SWCTRL_IsPortPrivateVlanDownlinkMember
 *--------------------------------------------------------------------------
 * PURPOSE  : This funcion returns true if current port is in private vlan's
 *            downlink member list.  Otherwise, returns false.
 * INPUT    : ifindex -- the specified port
 * OUTPUT   : none
 * RETURN   : TRUE \ FALSE
 * NOTES    : none
 *--------------------------------------------------------------------------*/
BOOL_T SWCTRL_IsPortPrivateVlanDownlinkMember(UI32_T ifindex);

#endif

#if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION == TRUE)
#if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE == SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE_MULTIPLE_SESSION)
/********************************************
 * Multi-Session of Private VLAN APIs       *
 *******************************************/
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetPrivateVlanBySessionId
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set the private vlan by group session id
 * INPUT   : session_id         -- is group session id
 *           uplink_port_list   -- uplink port list
 *           downlink_port_list -- downlink port list
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetPrivateVlanBySessionId(
              UI32_T session_id,
              UI8_T  uplink_port_list  [SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST],
              UI8_T  downlink_port_list[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST]);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetPrivateVlanBySessionId
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get private vlan by group session id
 * INPUT   : session_id         -- is group session id
 *           uplink_port_list   -- uplink port list
 *           downlink_port_list -- downlink port list
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_GetPrivateVlanBySessionId(
              UI32_T session_id,
              UI8_T  uplink_port_list  [SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST],
              UI8_T  downlink_port_list[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST]);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetRunningPrivateVlanPortListBySessionId
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get the private vlan running config by sId
 * INPUT   : session_id         -- get pvlan group id
 *           uplink_port_list   -- uplink port list
 *           downlink_port_list -- downlink port list
 * OUTPU   : uplink_port_list, downlink_port_list
 * RETURN  : SYS_TYPE_Get_Running_Cfg_T, No change while entry no exist
 * NOTE    : None
 * -------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T SWCTRL_GetRunningPrivateVlanPortListBySessionId(
                                UI32_T session_id,
                                UI8_T  uplink_port_list  [SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST],
                                UI8_T  downlink_port_list[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST]);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_DeletePrivateVlanPortlistBySessionId
 * -------------------------------------------------------------------------
 * FUNCTION: This function will delete private vlan port list with sesion Id
 * INPUT   : session_id         -- is group session id
 *           uplink_port_list   -- uplink port list
 *           downlink_port_list -- downlink port list
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_DeletePrivateVlanPortlistBySessionId(
              UI32_T session_id,
              UI8_T  uplink_port_list  [SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST],
              UI8_T  downlink_port_list[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST]);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetNextSessionFromPrivateVlanPortList
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get private vlan for next session Id
 * INPUT   : session_id -- pvlan group id
 * OUTPUT  : session_id -- return achieved session id
 *           uplink_port_list   -- uplink port list
 *           downlink_port_list -- downlink port list
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_GetNextSessionFromPrivateVlanPortList(
              UI32_T *session_id,
              UI8_T  uplink_port_list[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST],
              UI8_T  downlink_port_list[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST]);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_IsUserPortJoinPrivateVlanToTrunk
 * -------------------------------------------------------------------------
 * FUNCTION: Check if this user port could be trunk member or not for pvlan
 * INPUT   : trunk_id   -- which trunking port to set
 *           unit_port  -- member to add
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_IsUserPortJoinPrivateVlanToTrunk(UI32_T trunk_id, SYS_TYPE_Uport_T unit_port);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_DestroyPrivateVlanSession
 * -------------------------------------------------------------------------
 * FUNCTION: Destroy entire Private VLAN session
 * INPUT   : session_id   -- pvlan group id
 *           is_uplink    -- is uplink port
 *           is_downlink  -- is downlink port
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_DestroyPrivateVlanSession(UI32_T session_id, BOOL_T is_uplink, BOOL_T is_downlink);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_EnablePrivateVlanUplinkToUplinkBlockingMode
 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable blocking traffic of uplink ports
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_EnablePrivateVlanUplinkToUplinkBlockingMode();

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_DisablePrivateVlanUplinkToUplinkBlockingMode
 * -------------------------------------------------------------------------
 * FUNCTION: This function will disable blocking traffic of uplink ports
 *           so every traffic can be forwarding different uplink ports
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_DisablePrivateVlanUplinkToUplinkBlockingMode();

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetRunningPrivateVlanUplinkToUplinkStatus
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get private vlan uplink-to-uplink mode
 * INPUT   : None
 * OUTPU   : up_status -- uplink-to-uplink mode (blocking/forwarding)
 * RETURN  : SYS_TYPE_Get_Running_Cfg_T
 * NOTE    : None
 * -------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T SWCTRL_GetRunningPrivateVlanUplinkToUplinkStatus(UI32_T *up_status);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetPrivateVlanUplinkToUplinkStatus
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get the private vlan uplink-to-uplink mode
 * INPUT   : None
 * OUTPU   : up_status -- uplink-to-uplink mode (blocking/forwarding)
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_GetPrivateVlanUplinkToUplinkStatus(UI32_T *up_status);

#endif /* End of #if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE == SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE_MULTIPLE_SESSION)*/
#endif /* End of #if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION == TRUE) */


/*******************
 * Rate Limit APIs *
 *******************/
#if (SYS_CPNT_INGRESS_RATE_LIMIT == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_EnablePortIngressRateLimit
 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable the port ingress rate limit
 * INPUT   : ifindex -- the specified port
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_EnablePortIngressRateLimit(UI32_T ifindex);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_DisablePortIngressRateLimit
 * -------------------------------------------------------------------------
 * FUNCTION: This function will disable the port ingress rate limit
 * INPUT   : ifindex -- the specified port
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_DisablePortIngressRateLimit(UI32_T ifindex);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetPortIngressRateLimit
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set the port ingress rate limit
 * INPUT   : ifindex -- which port to set
 *           rate -- port ingress rate limit
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : # kbits
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetPortIngressRateLimit(UI32_T ifindex, UI32_T rate);

#if (SYS_CPNT_NETACCESS_DYNAMIC_QOS == TRUE)

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetDynamicPortIngressRateLimit
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set the port ingress rate limit for dynamic qos
 * INPUT   : ifindex -- which port to set
 *           rate -- port ingress rate limit
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : 1. Unit of rate limit is kbits.
 *           2. When apply dynamic port ingress rate limit, the manual configured
 *              setting should be saved.
 *           3. The ingress rate limit satus should be enabled, if this function
 *              successed.
 *           4. If rate is 0, restore the dynamic rate limit to manual configured.
 *           5. The dynamic ingress limit value is got from authentication server.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetDynamicPortIngressRateLimit(UI32_T ifindex, UI32_T rate);
#endif

/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetRunningPortIngressRateLimitStatus
 *------------------------------------------------------------------------
 * FUNCTION: This function will get the ingress rate limit status running config
 * INPUT   : ifindex
 * OUTPUT  : port_ingress_rate_limit_status -- enable/disable
 * RETURN  : SYS_TYPE_Get_Running_Cfg_T
 * NOTE    : None
 *------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T SWCTRL_GetRunningPortIngressRateLimitStatus(UI32_T ifindex, UI32_T *port_ingress_rate_limit_status);
#endif

#if (SYS_CPNT_EGRESS_RATE_LIMIT == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_EnablePortEgressRateLimit
 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable the port egress rate limit
 * INPUT   : ifindex -- the specified port
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_EnablePortEgressRateLimit(UI32_T ifindex);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_DisablePortEgressRateLimit
 * -------------------------------------------------------------------------
 * FUNCTION: This function will disable the port egress rate limit
 * INPUT   : Nifindex -- the specified port
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_DisablePortEgressRateLimit(UI32_T ifindex);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetPortEgressRateLimit
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set the port egress rate limit
 * INPUT   : ifindex -- which port to set
 *           rate -- port ingress rate limit
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : # kbits
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetPortEgressRateLimit(UI32_T ifindex, UI32_T rate);

/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetRunningPortEgressRateLimitStatus
 *------------------------------------------------------------------------
 * FUNCTION: This function will get the ingress rate limit status running config
 * INPUT   : ifindex
 * OUTPUT  : port_ingress_rate_limit_status -- enable/disable
 * RETURN  : SYS_TYPE_Get_Running_Cfg_T
 * NOTE    : None
 *------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T SWCTRL_GetRunningPortEgressRateLimitStatus(UI32_T ifindex, UI32_T *port_egress_rate_limit_status);
#endif

#if (SYS_CPNT_JUMBO_FRAMES == TRUE)
/********************
 * Jumbo Frame APIs *
 ********************/
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetJumboFrameStatus
 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable/disable the jumbo frame
 * INPUT   : jumbo_frame_status --  SWCTRL_JUMBO_FRAME_ENABLE
 *                                  SWCTRL_JUMBO_FRAME_DISABLE
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetJumboFrameStatus (UI32_T jumbo_frame_status);


/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetJumboFrameStatus
 *------------------------------------------------------------------------
 * FUNCTION: This function will get the jumbo frame status
 * INPUT   : None
 * OUTPUT  : jumbo_frame_status -- enable/disable
 * RETURN  : True: Successfully, FALSE: Failed
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_GetJumboFrameStatus (UI32_T *jumbo_frame_status);


/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetRunningJumboFrameStatus
 *------------------------------------------------------------------------
 * FUNCTION: This function will get the jumbo frame running config
 * INPUT   : None
 * OUTPUT  : jumbo_frame_status -- enable/disable
 * RETURN  : SYS_TYPE_Get_Running_Cfg_T
 * NOTE    : None
 *------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T SWCTRL_GetRunningJumboFrameStatus (UI32_T *jumbo_frame_status);
#endif


/****************************************************************************/
/* Port mapping                                                             */
/****************************************************************************/
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_UserPortToLogicalPort
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get a logical port mapping from a user port
 * INPUT   : unit    -- which unit to map
 *           port    -- which port to map
 * OUTPUT  : ifindex -- the logical port
 * RETURN  : One of SWCTRL_Lport_Type_T
 * NOTE    : None
 * -------------------------------------------------------------------------*/
SWCTRL_Lport_Type_T SWCTRL_UserPortToLogicalPort(UI32_T unit,
                                                 UI32_T port,
                                                 UI32_T *ifindex);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_UserPortToIfindex
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get the ifindex mapping from a user port
 * INPUT   : unit    -- which unit to map
 *           port    -- which port to map
 * OUTPUT  : ifindex -- the logical port
 * RETURN  : One of SWCTRL_Lport_Type_T
 * NOTE    : None
 * -------------------------------------------------------------------------*/
SWCTRL_Lport_Type_T SWCTRL_UserPortToIfindex(UI32_T unit,
                                             UI32_T port,
                                             UI32_T *ifindex);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_UserPortToTrunkPort
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get a trunk port mapping from a user port
 * INPUT   : unit     -- which unit to map
 *           port     -- which port to map
 * OUTPUT  : trunk_id -- the logical port
 * RETURN  : One of SWCTRL_Lport_Type_T
 * NOTE    : None
 * -------------------------------------------------------------------------*/
SWCTRL_Lport_Type_T SWCTRL_UserPortToTrunkPort(UI32_T unit,
                                               UI32_T port,
                                               UI32_T *trunk_id);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_LogicalPortToUserPort
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get a user port mapping from a logical port
 * INPUT   : ifindex  -- which port to map
 * OUTPUT  : unit     -- the unit
 *           port     -- the user port
 *           trunk_id -- trunk ID if it is a trunk port
 * RETURN  : One of SWCTRL_Lport_Type_T
 * NOTE    : None
 * -------------------------------------------------------------------------*/
SWCTRL_Lport_Type_T SWCTRL_LogicalPortToUserPort(UI32_T ifindex,
                                                 UI32_T *unit,
                                                 UI32_T *port,
                                                 UI32_T *trunk_id);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_TrunkIDToLogicalPort
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get a logical port mapping from a trunk port
 * INPUT   : trunk_id -- which trunk port to map
 * OUTPUT  : ifindex  -- the logical port
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_TrunkIDToLogicalPort(UI32_T trunk_id,
                                   UI32_T *ifindex);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetNextLogicalPort
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get the next existing logical port
 * INPUT   : l_port -- the key
 * OUTPUT  : l_port -- the next existing logical port
 * RETURN  : One of SWCTRL_Lport_Type_T
 * NOTE    : None
 * -------------------------------------------------------------------------*/
SWCTRL_Lport_Type_T SWCTRL_GetNextLogicalPort(UI32_T *l_port);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetNextActiveLogicalPort
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get the next logical port with link_oper_status = VAL_ifOperStatus_up
             null trunk, dormant state, and link-down port will not returned from this API
 * INPUT   : l_port -- the key
 * OUTPUT  : l_port -- the next oper up logical port
 * RETURN  : One of SWCTRL_Lport_Type_T
 * NOTE    : None
 * -------------------------------------------------------------------------*/
SWCTRL_Lport_Type_T SWCTRL_GetNextActiveLogicalPort(UI32_T *l_port);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_LogicalPortExisting
 * -------------------------------------------------------------------------
 * FUNCTION: This function will return if this port is existing
 * INPUT   : l_port -- the key to ask
 * OUTPUT  : None
 * RETURN  : TRUE: Existing, FALSE: Not existing
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_LogicalPortExisting(UI32_T l_port);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_UserPortExisting
 * -------------------------------------------------------------------------
 * FUNCTION: This function will return if this user port is existing
 * INPUT   : unit -- which unit
 *           port -- the key to ask
 * OUTPUT  : None
 * RETURN  : TRUE: Existing, FALSE: Not existing
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_UserPortExisting(UI32_T unit, UI32_T port);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_LogicalPortIsTrunkPort
 * -------------------------------------------------------------------------
 * FUNCTION: This function will return if this port is trunk port
 * INPUT   : uport_ifindex -- the key to ask
 * OUTPUT  : None
 * RETURN  : TRUE: trunk port , FALSE: Not existing
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_LogicalPortIsTrunkPort(UI32_T ifindex);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_IsTrunkMember
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
BOOL_T SWCTRL_IsTrunkMember(UI32_T uport_ifindex, UI32_T *trunk_ifindex, BOOL_T *is_static);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PortExisting
 * -------------------------------------------------------------------------
 * FUNCTION: This function will return if this user port is existing
 * INPUT   : port -- the key to ask
 * OUTPUT  : None
 * RETURN  : TRUE: Existing, FALSE: Not existing
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PortExisting(UI32_T l_port);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetTrunkIfIndexByUport
 * -------------------------------------------------------------------------
 * FUNCTION: This function will return if this port is trunk member
 * INPUT   : uport_ifindex -- the key to ask (trunk member)
 * OUTPUT  : trunk_ifindex -- trunk ifindex
 * RETURN  : TRUE: trunk member , FALSE: Not existing
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_GetTrunkIfIndexByUport(UI32_T uport_ifindex, UI32_T *trunk_ifindex);


/* -------------------------------------------------------------------------
 * ROUTINE NAME -  SWCTRL_LportToActiveUport
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get the primary port from logical port
 * INPUT   : vid        -- The VLAN ID. If the vid == SYS_TYPE_IGNORE_VID_CHECK
 *                         then checking STA state is not necessary.
 *           l_port     -- the key to ask
 * OUTPUT  : *unit_port -- primary port (active)
 * RETURN  : TRUE: primary port , FALSE: Not existing any active port
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T  SWCTRL_LportToActiveUport(UI32_T vid, UI32_T l_port, SYS_TYPE_Uport_T *unit_port);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_LportListToActiveUportList
 * -------------------------------------------------------------------------
 * FUNCTION: This function will translate the logical port list to active
 *           unit port list
 * INPUT   : vid       -- The VLAN ID. If the vid == SYS_TYPE_IGNORE_VID_CHECK
 *                        then checking STA state is not necessary.
 *           lportlist -- logical port list
 * OUTPUT  : active_uport_count_per_unit -- how many uport is active ?
 *           uport;ist -- unit port list
 * RETURN  : TRUE: Existing, FALSE: Not existing
 * NOTE    : 1. for GVRP, STA, IGMP Snooping, IPMC
 *           2. Lport bit array(ifindex view) --> uport list(for non trunk member,
 *              keep the same for trunk member, select a u port as primary port).
 *              Primary port is usually the port which number is smallest.
 *           3. For Example: 24 ports / per unit, total 8 units in whole system
 *              uportlist : ( the MSB represents the port 1)
 *                           +---------+---------+------+---------+
 *                  unit     |    1    |    2    | ...  |    8    |
 *                           +---------+---------+------+---------+
 *                  portlist | 4 bytes | 4 bytes | ...  | 4 bytes |
 *                           +---------+---------+------+---------+
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_LportListToActiveUportList(
           UI32_T vid,
           UI8_T  *lportlist,
           UI8_T  active_uport_count_per_unit[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK],
           UI8_T  *uportlist);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_LportListToActiveUportListExt
 * -------------------------------------------------------------------------
 * FUNCTION: This function will translate the logical port list to active
 *           unit port list, if no active uport, then return FALSE.
 * INPUT   : 1) vid       -- VLAN ID for the ports to check.
 *           2) lportlist -- logical port list
 * OUTPUT  : 1) active_uport_count_per_unit -- how many uport is active
 *           2) uportlist -- unit port list
 * RETURN  : TRUE:  Have active uport.
 *           FALSE: Parameter is invali, no active uport.
 * NOTE    : The only difference between SWCTRL_LportListToActiveUportList()
 *           is when active uport number is 0, this API will return FALSE.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_LportListToActiveUportListExt(
                   UI32_T vid,
                   UI8_T  *lportlist,
                   UI8_T  active_uport_count_per_unit[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK],
                   UI8_T  *uportlist);

/*---------------------------------------------------------------------- */
/* (portMgt 1)--ES3626A */
/*
 *      INDEX       { portIndex }
 *      PortEntry ::= SEQUENCE
 *      {
 *          portIndex                Integer32,
 *          portName                 DisplayString,
 *          portType                 INTEGER,
 *          portSpeedDpxCfg          INTEGER,
 *          portFlowCtrlCfg          INTEGER,
 *          portCapabilities         BITS,
 *          portAutonegotiation      INTEGER,
 *          portSpeedDpxStatus       INTEGER,
 *          portFlowCtrlStatus       INTEGER,
 *          portTrunkIndex           Integer32
 *      }
 */
/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetPortEntry
 *------------------------------------------------------------------------
 * FUNCTION: This function will get the port table entry info
 * INPUT   : port_entry->port_index - interface index
 * OUTPUT  : port_entry             - The port table entry info
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/portMgt 1
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_GetPortEntry(SWCTRL_PortEntry_T *port_entry);


/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetNextPortEntry
 *------------------------------------------------------------------------
 * FUNCTION: This function will get the next port table entry info
 * INPUT   : port_entry->port_index - interface index
 * OUTPUT  : port_entry             - The port table entry info
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/portMgt 1
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_GetNextPortEntry(SWCTRL_PortEntry_T *port_entry);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetPortName
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set name to a port
 * INPUT   : ifindex    -- which port to set
 *           port_name  -- the name to set
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : ES3626A MIB/portMgt 1
 *           size (0..64)
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetPortName(UI32_T ifindex, UI8_T *port_name);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetPortAlias
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set alias to a port
 * INPUT   : ifindex    -- which port to set
 *           port_name  -- the alias to set
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : ES3626A MIB/portMgt 1
 *           size (0..64)
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetPortAlias(UI32_T ifindex, UI8_T *port_alias);

/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetPortSpeedDpxCfg
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
BOOL_T SWCTRL_SetPortSpeedDpxCfg(UI32_T ifindex, UI32_T port_speed_dpx_cfg);


/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetPortFlowCtrlCfg
 *------------------------------------------------------------------------
 * FUNCTION: This function will Set the port flow control mechanism
 * INPUT   : ifindex                        - interface index
 *           port_flow_ctrl_cfg             - VAL_portFlowCtrlCfg_enabled
 *                                            VAL_portFlowCtrlCfg_disabled
 *                                            VAL_portFlowCtrlCfg_backPressure
 *                                            VAL_portFlowCtrlCfg_dot3xFlowControl
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/portMgt 1
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetPortFlowCtrlCfg(UI32_T ifindex, UI32_T port_flow_ctrl_cfg);


/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetPortCapabilities
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
BOOL_T SWCTRL_SetPortCapabilities(UI32_T ifindex, UI32_T port_capabilities);


/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetPortAutonegotiation
 *------------------------------------------------------------------------
 * FUNCTION: This function will Set the port autonegotiation
 * INPUT   : ifindex                        - interface index
 *           port_autonegotiation           - VAL_portAutonegotiation_enabled
 *                                            VAL_portAutonegotiation_disabled
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/portMgt 1
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetPortAutonegotiation(UI32_T ifindex, UI32_T port_autonegotiation);


/*---------------------------------------------------------------------- */
/* ( mirrorMgt 1)--ES3626A */
/*
 *      INDEX { mirrorDestinationPort, mirrorSourcePort }
 *      MirrorEntry ::= SEQUENCE
 *      {
 *          mirrorDestinationPort  Integer32,
 *          mirrorSourcePort       Integer32,
 *          mirrorType             INTEGER,
 *          mirrorStatus           INTEGER
 *      }
 */
/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetMirrorEntry
 *------------------------------------------------------------------------
 * FUNCTION: This function will get the mirror table entry info
 * INPUT   : mirror_entry->mirror_destination_port - mirror destination port
 *           mirror_entry->mirror_source_port      - mirror source port
 * OUTPUT  : mirror_entry                          - The mirror entry info
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/mirrorMgt 1
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_GetMirrorEntry(SWCTRL_MirrorEntry_T *mirror_entry);


/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetNextMirrorEntry
 *------------------------------------------------------------------------
 * FUNCTION: This function will get the next mirror table entry info
 * INPUT   : mirror_entry->mirror_destination_port - mirror destination port
 *           mirror_entry->mirror_source_port      - mirror source port
 * OUTPUT  : mirror_entry                          - The mirror entry info
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/mirrorMgt 1
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_GetNextMirrorEntry(SWCTRL_MirrorEntry_T *mirror_entry);


/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetNextRunningMirrorEntry
 *------------------------------------------------------------------------
 * FUNCTION: This function will get the next running config mirror table entry info
 * INPUT   : mirror_entry->mirror_destination_port - mirror destination port
 *           mirror_entry->mirror_source_port      - mirror source port
 * OUTPUT  : mirror_entry                          - The mirror entry info
 * RETURN  : SYS_TYPE_Get_Running_Cfg_T
 * NOTE    : None
 *------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T SWCTRL_GetNextRunningMirrorEntry(SWCTRL_MirrorEntry_T *mirror_entry);


/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetMirrorType
 *------------------------------------------------------------------------
 * FUNCTION: This function will Set the mirror type
 * INPUT   : ifindex_src   -- which ifindex to mirror
 *           ifindex_dest  -- which ifindex mirrors the received/transmitted packets
 *           mirror_type   -- VAL_mirrorType_rx
 *                            VAL_mirrorType_tx
 *                            VAL_mirrorType_both
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/mirrorMgt 1
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetMirrorType(UI32_T ifindex_src, UI32_T ifindex_dest, UI32_T mirror_type);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetMirrorStatus
 * -------------------------------------------------------------------------
 * FUNCTION: This function will create/destroy the mirroring function
 * INPUT   : ifindex_src   -- which ifindex to mirror
 *           ifindex_dext  -- which ifindex mirrors the received/transmitted packets
 * OUTPUT  : mirror_status -- VAL_mirrorStatus_valid / VAL_mirrorStatus_invalid
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : ES3626A MIB/mirrorMgt 1
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetMirrorStatus(UI32_T ifindex_src, UI32_T ifindex_dest, UI32_T mirror_status);

#if (SYS_CPNT_VLAN_MIRROR == TRUE)
/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetVlanMirrorEntry
 *------------------------------------------------------------------------
 * FUNCTION: This function will get the vlan mirror table entry info
 * INPUT   : vlan_mirror_entry->mirror_dest_port        - mirror destination port
 *           vlan_mirror_entry->mirror_source_vlan      - mirror source vlan id
 * OUTPUT  : vlan_mirror_entry                          - The vlan mirror entry info
 * RETURN  : TRUE/FALSE
 * NOTE    : The input keys (ifindex & vid) will get current vlan entry.
 *           if specifies a mirror_dest_port = 0 , so return system dest port
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_GetVlanMirrorEntry(SWCTRL_VlanMirrorEntry_T *vlan_mirror_entry);

/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetNextVlanMirrorEntry
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
BOOL_T SWCTRL_GetNextVlanMirrorEntry(SWCTRL_VlanMirrorEntry_T *vlan_mirror_entry);

/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_AddVlanMirror
 *------------------------------------------------------------------------
 * FUNCTION: This function will add the vlan mirror and desination port
 * INPUT   : vid           -- which vlan-id add to source mirrored table
 *           ifindex_dest  -- which ifindex-port received mirror packets
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : A destination port shall be consistent whenever vlan-id created
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_AddVlanMirror(UI32_T vid, UI32_T ifindex_dest);

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
BOOL_T SWCTRL_DeleteVlanMirror(UI32_T vid, UI32_T ifindex_dest);
#endif /* End of #if (SYS_CPNT_VLAN_MIRROR == TRUE) */

#if (SYS_CPNT_MAC_BASED_MIRROR == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetMacMirrorEntry
 * -------------------------------------------------------------------------
 * PURPOSE  : This function will set the MAC based MIRROR entry
 * INPUT    : ifindex_dest       -- destnation port
 *            mac_address        -- MAC address
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE    - TRUE if successful;FALSE if failed
 * NOTES    :
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetMacMirrorEntry(UI32_T ifindex_dest, UI8_T *mac_address);

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
BOOL_T SWCTRL_DeleteMacMirrorEntry(UI32_T ifindex_dest, UI8_T *mac_address);

#endif /* end of #if (SYS_CPNT_MAC_BASED_MIRROR == TRUE) */

#if (SYS_CPNT_ACL_MIRROR == TRUE)
/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetAclMirrorDestPort
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
BOOL_T SWCTRL_SetAclMirrorDestPort(UI32_T ifindex_dest, UI32_T mirror_type, BOOL_T enable);

/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetAclMirrorDestPort
 *------------------------------------------------------------------------
 * FUNCTION: This function will setup dest port for  ACL-based mirror
 * INPUT   : ifindex_dest  -- Destination port. Pass 0 to start.
 * OUTPUT  : ifindex_dest  -- which ifindex-port received mirror packets
 * RETURN  : TRUE/FALSE
 * NOTE    :
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_GetNextAclMirrorDestPort(UI32_T *ifindex_dest);
#endif

BOOL_T SWCTRL_SetMTPTable(UI32_T ifindex_dest, UI32_T mirror_status);

BOOL_T SWCTRL_CheckMTPTable(UI32_T ifindex_dest, UI32_T mirror_status);

/*---------------------------------------------------------------------- */
/* ( bcastStormMgt 1 )--ES3626A */
/*
 *      INDEX       { bcastStormIfIndex }
 *      BcastStormEntry ::= SEQUENCE
 *      {
 *          bcastStormIfIndex      Integer32,
 *          bcastStormStatus       INTEGER,
 *          bcastStormSampleType   INTEGER,
 *          bcastStormPktRate      INTEGER,
 *          bcastStormOctetRate    INTEGER,
 *          bcastStormPercent      INTEGER
 *      }
 */
/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetBcastStormEntry
 *------------------------------------------------------------------------
 * FUNCTION: This function will get the broadcast storm management entry
 * INPUT   : bcast_storm_entry->bcast_storm_ifindex - interface index
 * OUTPUT  : bcast_storm_entry                      - broadcast storm management entry
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/bcastStormMgt 1
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_GetBcastStormEntry(SWCTRL_BcastStormEntry_T *bcast_storm_entry);


/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetNextBcastStormEntry
 *------------------------------------------------------------------------
 * FUNCTION: This function will get the next broadcast storm management entry
 * INPUT   : bcast_storm_entry->bcast_storm_ifindex - interface index
 * OUTPUT  : bcast_storm_entry                      - broadcast storm management entry
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/bcastStormMgt 1
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_GetNextBcastStormEntry(SWCTRL_BcastStormEntry_T *bcast_storm_entry);


/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetBcastStormStatus
 *------------------------------------------------------------------------
 * FUNCTION: This function will Set the broadcast storm status
 * INPUT   : ifindex                      - interface index
 *           bcast_storm_status           - VAL_bcastStormStatus_enabled
 *                                          VAL_bcastStormStatus_disabled
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/bcastStormMgt 1
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetBcastStormStatus(UI32_T ifindex, UI32_T bcast_storm_status);


/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetBcastStormSampleType
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
BOOL_T SWCTRL_SetBcastStormSampleType(UI32_T ifindex, UI32_T bcast_storm_sample_type);


/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetBcastStormPktRate
 *------------------------------------------------------------------------
 * FUNCTION: This function will Set tthe broadcast storm packet rate
 * INPUT   : ifindex                        - interface index
 *           bcast_storm_pkt_rate           - the broadcast storm packet rate
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/bcastStormMgt 1
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetBcastStormPktRate(UI32_T ifindex, UI32_T bcast_storm_pkt_rate);


/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetBcastStormOctetRate
 *------------------------------------------------------------------------
 * FUNCTION: This function will Set tthe broadcast storm octet rate
 * INPUT   : ifindex                        - interface index
 *           bcast_storm_octet_rate           - the broadcast storm octet rate
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/bcastStormMgt 1
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetBcastStormOctetRate(UI32_T ifindex, UI32_T bcast_storm_octet_rate);


/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetBcastStormPercent
 *------------------------------------------------------------------------
 * FUNCTION: This function will Set tthe broadcast storm octet rate
 * INPUT   : ifindex                       - interface index
 *           bcast_storm_percent           - the broadcast storm percent
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/bcastStormMgt 1
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetBcastStormPercent(UI32_T ifindex, UI32_T bcast_storm_percent);


/*---------------------------------------------------------------------- */
/* ( mcastStormMgt 1 )--ES3626A */
/*
 *      INDEX       { mcastStormIfIndex }
 *      McastStormEntry ::= SEQUENCE
 *      {
 *          mcastStormIfIndex      Integer32,
 *          mcastStormStatus       INTEGER,
 *          mcastStormSampleType   INTEGER,
 *          mcastStormPktRate      INTEGER,
 *          mcastStormOctetRate    INTEGER,
 *          mcastStormPercent      INTEGER
 *      }
 */
/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetMcastStormEntry
 *------------------------------------------------------------------------
 * FUNCTION: This function will get the multicast storm management entry
 * INPUT   : mcast_storm_entry->mcast_storm_ifindex - interface index
 * OUTPUT  : mcast_storm_entry                      - multicast storm management entry
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/mcastStormMgt 1
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_GetMcastStormEntry(SWCTRL_McastStormEntry_T *mcast_storm_entry);


/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetNextMcastStormEntry
 *------------------------------------------------------------------------
 * FUNCTION: This function will get the next multicast storm management entry
 * INPUT   : mcast_storm_entry->mcast_storm_ifindex - interface index
 * OUTPUT  : mcast_storm_entry                      - multicast storm management entry
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/mcastStormMgt 1
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_GetNextMcastStormEntry(SWCTRL_McastStormEntry_T *mcast_storm_entry);


/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetMcastStormStatus
 *------------------------------------------------------------------------
 * FUNCTION: This function will Set the multicast storm status
 * INPUT   : ifindex                      - interface index
 *           mcast_storm_status           - VAL_mcastStormStatus_enabled
 *                                          VAL_mcastStormStatus_disabled
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/mcastStormMgt 1
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetMcastStormStatus(UI32_T ifindex, UI32_T mcast_storm_status);


/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetMcastStormSampleType
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
BOOL_T SWCTRL_SetMcastStormSampleType(UI32_T ifindex, UI32_T mcast_storm_sample_type);


/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetMcastStormPktRate
 *------------------------------------------------------------------------
 * FUNCTION: This function will Set tthe multicast storm packet rate
 * INPUT   : ifindex                        - interface index
 *           mcast_storm_pkt_rate           - the mroadcast storm packet rate
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/mcastStormMgt 1
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetMcastStormPktRate(UI32_T ifindex, UI32_T mcast_storm_pkt_rate);


/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetMcastStormOctetRate
 *------------------------------------------------------------------------
 * FUNCTION: This function will Set tthe multicast storm octet rate
 * INPUT   : ifindex                        - interface index
 *           mcast_storm_octet_rate           - the multicast storm octet rate
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/mcastStormMgt 1
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetMcastStormOctetRate(UI32_T ifindex, UI32_T mcast_storm_octet_rate);


/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetMcastStormPercent
 *------------------------------------------------------------------------
 * FUNCTION: This function will Set the multicast storm octet rate
 * INPUT   : ifindex                       - interface index
 *           mcast_storm_percent           - the multicast storm percent
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/mcastStormMgt 1
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetMcastStormPercent(UI32_T ifindex, UI32_T mcast_storm_percent);

#if (SYS_CPNT_STORM_MODE & SYS_CPNT_STORM_UNKNOWN_USTORM)
/*---------------------------------------------------------------------- */
/* ( unkucastStormMgt 1 )--ES3626A */
/*
 *      INDEX       { unkucastStormIfIndex }
 *      UnkucastStormEntry ::= SEQUENCE
 *      {
 *          unkucastStormIfIndex      Integer32,
 *          unkucastStormStatus       INTEGER,
 *          unkucastStormSampleType   INTEGER,
 *          unkucastStormPktRate      INTEGER,
 *          unkucastStormOctetRate    INTEGER,
 *          unkucastStormPercent      INTEGER
 *      }
 */
/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetUnkucastStormEntry
 *------------------------------------------------------------------------
 * FUNCTION: This function will get the unknowunicast storm management entry
 * INPUT   : unkucast_storm_entry->unkucast_storm_ifindex - interface index
 * OUTPUT  : unkucast_storm_entry                      - unknowunicast storm management entry
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/unkucastStormMgt 1
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_GetUnkucastStormEntry(SWCTRL_UnknownUcastStormEntry_T *unkucast_storm_entry);


/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetNextUnkucastStormEntry
 *------------------------------------------------------------------------
 * FUNCTION: This function will get the next unknowunicast storm management entry
 * INPUT   : unkucast_storm_entry->unkucast_storm_ifindex - interface index
 * OUTPUT  : unkucast_storm_entry                      - unknowunicast storm management entry
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/unkucastStormMgt 1
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_GetNextUnkucastStormEntry(SWCTRL_UnknownUcastStormEntry_T *unkucast_storm_entry);

/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetUnkucastStormSampleType
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
BOOL_T SWCTRL_SetUnkucastStormSampleType(UI32_T ifindex, UI32_T unkucast_storm_sample_type);


/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetUnkucastStormPktRate
 *------------------------------------------------------------------------
 * FUNCTION: This function will Set tthe unknowunicast storm packet rate
 * INPUT   : ifindex                        - interface index
 *           unkucast_storm_pkt_rate           - the unknowunicast storm packet rate
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/unkucastStormMgt 1
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetUnkucastStormPktRate(UI32_T ifindex, UI32_T unkucast_storm_pkt_rate);


/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetUnkucastStormOctetRate
 *------------------------------------------------------------------------
 * FUNCTION: This function will Set tthe unknowunicast storm octet rate
 * INPUT   : ifindex                        - interface index
 *           unkucast_storm_octet_rate           - the unknowunicast storm octet rate
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : ES3626A MIB/unkucastStormMgt 1
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetUnkucastStormOctetRate(UI32_T ifindex, UI32_T unkucast_storm_octet_rate);


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
BOOL_T SWCTRL_SetUnkucastStormPercent(UI32_T ifindex, UI32_T unkucast_storm_percent);
#endif /*#if (SYS_CPNT_STORM_MODE & SYS_CPNT_STORM_UNKNOWN_USTORM)*/


/*********************
 * Callback Processing
 *********************/
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_CallbackPreProcessor
 * -------------------------------------------------------------------------
 * FUNCTION:
 * INPUT   : unit -- in which unit
 *           port -- which port
 *           event -- which event
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_CallbackPreProcessor(UI32_T event, UI32_T unit, UI32_T port, UI32_T data, UI32_T data2);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_CallbackPostProcessor
 * -------------------------------------------------------------------------
 * FUNCTION:
 * INPUT   :
 * OUTPUT  : None
 * RETURN  : SWCTRL_LinkScanReturnCode_T
 * NOTE    : None
 * -------------------------------------------------------------------------*/
SWCTRL_LinkScanReturnCode_T SWCTRL_CallbackPostProcessor(void);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_ProcessHotSwapInsert
 * -------------------------------------------------------------------------
 * FUNCTION: Do all actions related to hot swap module inserted
 * INPUT   : unit -- in which unit
 *           port -- which port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_ProcessHotSwapInsert(UI32_T unit,
                                 UI32_T port);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_ProcessHotSwapRemove
 * -------------------------------------------------------------------------
 * FUNCTION: Do all actions related to hot swap module removed
 * INPUT   : unit -- in which unit
 *           port -- which port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_ProcessHotSwapRemove(UI32_T unit,
                                 UI32_T port);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_ProcessPortLinkUp
 * -------------------------------------------------------------------------
 * FUNCTION: Do all actions related to port link up
 * INPUT   : unit -- in which unit
 *           port -- which port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_ProcessPortLinkUp(UI32_T unit,
                              UI32_T port);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_ProcessPortLinkDown
 * -------------------------------------------------------------------------
 * FUNCTION: Do all actions related to port link down
 * INPUT   : unit -- in which unit
 *           port -- which port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_ProcessPortLinkDown(UI32_T unit,
                                UI32_T port);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_ProcessCraftPortLinkUp
 * -------------------------------------------------------------------------
 * FUNCTION: Do all actions related to port link up
 * INPUT   : unit -- in which unit
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_ProcessCraftPortLinkUp(UI32_T unit);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_ProcessCraftPortLinkDown
 * -------------------------------------------------------------------------
 * FUNCTION: Do all actions related to port link down
 * INPUT   : unit -- in which unit
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_ProcessCraftPortLinkDown(UI32_T unit);



/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_ProcessPortTypeChanged
 * -------------------------------------------------------------------------
 * FUNCTION: Do all actions related to port type change
 * INPUT   : unit       -- in which unit
 *           port       -- which port
 *           module_id
 *           port_type  -- which port type
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_ProcessPortTypeChanged(UI32_T unit,
                                   UI32_T port,
                                   UI32_T module_id,
                                   UI32_T port_type);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_ProcessPortSpeedDuplexChanged
 * -------------------------------------------------------------------------
 * FUNCTION: Do all actions related to port speed duplex change
 * INPUT   : unit           -- in which unit
 *           port           -- which port
 *           speed_duplex   -- which port type
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_ProcessPortSpeedDuplexChanged(  UI32_T unit,
                                            UI32_T port,
                                            UI32_T speed_duplex);



/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_ProcessPortFlowCtrlChanged
 * -------------------------------------------------------------------------
 * FUNCTION: Do all actions related to port speed duplex change
 * INPUT   : unit           -- in which unit
 *           port           -- which port
 *           flowctrl_status-- which status (backpressure/802.3x/none)
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_ProcessPortFlowCtrlChanged( UI32_T unit,
                                        UI32_T port,
                                        UI32_T flowctrl_status);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_ProcessPortSfpPresentChanged
 * -------------------------------------------------------------------------
 * FUNCTION: Do all actions related to port sfp present change
 * INPUT   : unit           -- in which unit
 *           port           -- which port
 *           is_present     -- present or not
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
void SWCTRL_ProcessPortSfpPresentChanged(UI32_T unit,
                                          UI32_T port,
                                          BOOL_T is_present);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_ProcessPortSfpDdmThresholdMeasuredInfoChanged
 * -------------------------------------------------------------------------
 * FUNCTION: Send trap if mesaured ddm info exceeds alarm/warning threshold
 * INPUT   : unit           -- in which unit
 *           sfp_index      -- which sfp index
 *           is_changing_to_not_present
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
void SWCTRL_ProcessPortSfpDdmThresholdMeasuredInfoChanged(UI32_T unit, UI32_T sfp_index, BOOL_T is_changing_to_not_present);

/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_ShutdownSwitch
 *------------------------------------------------------------------------
 * FUNCTION: This function will shutdown the switch before warm start
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    :
 *------------------------------------------------------------------------*/
void SWCTRL_ShutdownSwitch(void);

/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_IsServerBladePort
 *------------------------------------------------------------------------
 * FUNCTION: To know this interface is server blade port or not/
 * INPUT   : ifindex - port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    :
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_IsServerBladePort (UI32_T ifindex);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_DisableIPMC
 * -------------------------------------------------------------------------
 * FUNCTION: This function will disable IPMC
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_DisableIPMC(void);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_EnableIPMC
 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable IPMC
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_EnableIPMC(void);


/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SWCTRL_SetPortSecurityStatus
 * ------------------------------------------------------------------------|
 * FUNCTION : set the port security status
 * INPUT    : ifindex : the logical port
 *            port_security_status : VAL_portSecPortStatus_enabled
 *                                   VAL_portSecPortStatus_disabled
 *             port_security_called_by_who:
 *              SWCTRL_PORT_SECURITY_ENABLED_BY_NONE
 *                          SWCTRL_PORT_SECURITY_ENABLED_BY_PSEC
 *                          SWCTRL_PORT_SECURITY_ENABLED_BY_DOT1X
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 * ------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetPortSecurityStatus( UI32_T ifindex, UI32_T  port_security_status,UI32_T port_security_called_by_who /*kevin*/);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SWCTRL_SetPortSecurityActionStatus
 * ------------------------------------------------------------------------|
 * FUNCTION : set the port security action status
 * INPUT    : ifindex : the logical port
 *            action_status: VAL_portSecAction_none(1)
 *                           VAL_portSecAction_trap(2)
 *                           VAL_portSecAction_shutdown(3)
 *                           VAL_portSecAction_trapAndShutdown(4)
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 * ------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetPortSecurityActionStatus( UI32_T ifindex, UI32_T  action_status);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SWCTRL_SetPortSecurityActionTrapOperStatus
 * ------------------------------------------------------------------------|
 * FUNCTION : set the port security action trap operation status
 * INPUT    : ifindex : the logical port
 *            action_trap_status : VAL_portSecActionTrap_enabled
 *                                 VAL_portSecActionTrap_disabled
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 * ------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetPortSecurityActionTrapOperStatus( UI32_T ifindex, UI32_T  action_trap_oper_status);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SWCTRL_IsSecurityPort
 * ------------------------------------------------------------------------|
 * FUNCTION : check the port is in security
 * INPUT    : ifindex : the port
 * OUTPUT   : port_security_enabled_by_who ---  SWCTRL_PORT_SECURITY_ENABLED_BY_NONE
 *                                  SWCTRL_PORT_SECURITY_ENABLED_BY_PSEC
 *                                  SWCTRL_PORT_SECURITY_ENABLED_BY_DOT1X
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 * ------------------------------------------------------------------------*/
BOOL_T SWCTRL_IsSecurityPort( UI32_T ifindex,UI32_T *port_security_enabled_by_who/*kevin*/);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SWCTRL_GetSecurityActionTrapPort
 * ------------------------------------------------------------------------|
 * FUNCTION : check the port is security action trap enable or disable
 * INPUT    : ifindex : the port
 * OUTPUT   : *action_status: output status
 * RETURN   : TRUE/FALSE
 * NOTE     : Port security doesn't support 1) unknown port, 2) trunk member, and
 *                                          3) trunk port
 * ------------------------------------------------------------------------*/
BOOL_T SWCTRL_GetSecurityActionStatus( UI32_T ifindex, UI32_T *action_status);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SWCTRL_IsSecurityActionShutdownPort
 * ------------------------------------------------------------------------|
 * FUNCTION : check the port is security action shutdown enable or disable
 * INPUT    : ifindex : the port
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 * ------------------------------------------------------------------------*/
BOOL_T SWCTRL_IsSecurityActionShutdownPort( UI32_T ifindex);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SWCTRL_IsSecurityActionTrapPort
 * ------------------------------------------------------------------------|
 * FUNCTION : check the port is security action trap enable or disable
 * INPUT    : ifindex : the port
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 * ------------------------------------------------------------------------*/
BOOL_T SWCTRL_IsSecurityActionTrapPort( UI32_T ifindex);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SWCTRL_IsSecurityOperActionTrapPort
 * ------------------------------------------------------------------------|
 * FUNCTION : check the port is security operation action trap enable or disable
 * INPUT    : ifindex : the port
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 * ------------------------------------------------------------------------*/
BOOL_T SWCTRL_IsSecurityOperActionTrapPort( UI32_T ifindex);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SWCTRL_IsSecurityActionTrapTimerExpiry
 * ------------------------------------------------------------------------|
 * FUNCTION : check the port is security action trap timer expiry or not?
 * INPUT    : ifindex : interface index.
 *            time_expiry_ticks : timer expiry ticks value.
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 * ------------------------------------------------------------------------*/
BOOL_T SWCTRL_IsSecurityActionTrapTimerExpiry( UI32_T ifindex, UI32_T time_expiry_ticks);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SWCTRL_AddSecurityActionTrapTimeStamp
 * ------------------------------------------------------------------------|
 * FUNCTION : increase the timer stamp to the timer of this port?
 * INPUT    : ifindex : interface index.
 *            increase_time_stamp_ticks : increase ticks of the timer stamp.
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 * ------------------------------------------------------------------------*/
BOOL_T SWCTRL_AddSecurityActionTrapTimeStamp( UI32_T ifindex, UI32_T increase_time_stamp_ticks);


/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SWCTRL_ResetSecurityActionTrapTimeStamp
 * ------------------------------------------------------------------------|
 * FUNCTION : reset the timer stamp to the timer of this port?
 * INPUT    : ifindex : interface index.
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 * ------------------------------------------------------------------------*/
BOOL_T SWCTRL_ResetSecurityActionTrapTimeStamp( UI32_T ifindex);


/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_UIGetUnitPortNumber
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
UI32_T SWCTRL_UIGetUnitPortNumber(UI32_T unit);

/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_UIUserPortToLogicalPort
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
SWCTRL_Lport_Type_T SWCTRL_UIUserPortToLogicalPort(UI32_T unit,
                                                   UI32_T port,
                                                   UI32_T *ifindex);

/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_UIUserPortToIfindex
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
SWCTRL_Lport_Type_T SWCTRL_UIUserPortToIfindex (UI32_T unit, UI32_T port, UI32_T *ifindex, BOOL_T *is_inherit);

/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_UIUserPortToTrunkPort
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
SWCTRL_Lport_Type_T SWCTRL_UIUserPortToTrunkPort(UI32_T unit, UI32_T port, UI32_T *trunk_id);

/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_UIUserPortExisting
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
BOOL_T SWCTRL_UIUserPortExisting (UI32_T unit, UI32_T port);

/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetUnitsBaseMacAddrTable
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
BOOL_T SWCTRL_SetUnitsBaseMacAddrTable(UI8_T mac_addr[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK][6]);

/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetUnitsBaseMacAddrTable
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
BOOL_T SWCTRL_SetUnitsDeviceTypeTable(UI32_T device_type[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK]);

/*-------------------------------------------------------------------------
 *                      Protocol Base VLAN API
 *------------------------------------------------------------------------*/

#if (SYS_CPNT_MAU_MIB == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetIfMauStatus
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
BOOL_T SWCTRL_SetIfMauStatus (UI32_T if_mau_ifindex,
                              UI32_T if_mau_index,
                              UI32_T status);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetIfMauDefaultType
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
BOOL_T SWCTRL_SetIfMauDefaultType (UI32_T if_mau_ifindex,
                                   UI32_T if_mau_index,
                                   UI32_T default_type);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetIfMauEntry
 * -------------------------------------------------------------------------
 * FUNCTION: Get MAU entry specified in RFC-3636.
 * INPUT   : if_mau_entry->ifMauIfIndex  -- Which interface.
 *           if_mau_entry->ifMauIndex    -- Which MAU.
 * OUTPUT  : if_mau_entry.
 * RETURN  : TRUE/FALSE
 * NOTE    : 1) Base on RFC-3636 (snmpDot3MauMgt)
 *           2) For User port only.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_GetIfMauEntry (SWCTRL_IfMauEntry_T *if_mau_entry);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetNextIfMauEntry
 * -------------------------------------------------------------------------
 * FUNCTION: Get next MAU entry specified in RFC-3636.
 * INPUT   : if_mau_entry->ifMauIfIndex  -- Next to which interface.
 *           if_mau_entry->ifMauIndex    -- Next to which MAU.
 * OUTPUT  : if_mau_entry.
 * RETURN  : TRUE/FALSE
 * NOTE    : 1) Base on RFC-3636 (snmpDot3MauMgt)
 *           2) For User port only.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_GetNextIfMauEntry (SWCTRL_IfMauEntry_T *if_mau_entry);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetIfMauAutoNegAdminStatus
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
BOOL_T SWCTRL_SetIfMauAutoNegAdminStatus (UI32_T if_mau_ifindex,
                                          UI32_T if_mau_index,
                                          UI32_T auto_neg_admin_status);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetIfMauAutoNegRestart
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
BOOL_T SWCTRL_SetIfMauAutoNegRestart (UI32_T if_mau_ifindex,
                                      UI32_T if_mau_index,
                                      UI32_T auto_neg_restart);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetIfMauAutoNegCapAdvertisedBits
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
BOOL_T SWCTRL_SetIfMauAutoNegCapAdvertisedBits (UI32_T if_mau_ifindex,
                                                UI32_T if_mau_index,
                                                UI32_T auto_neg_cap_adv_bits);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetIfMauAutoNegRemoteFaultAdvertised
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
BOOL_T SWCTRL_SetIfMauAutoNegRemoteFaultAdvertised (UI32_T if_mau_ifindex,
                                                    UI32_T if_mau_index,
                                                    UI32_T auto_neg_remote_fault_adv);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetIfMauAutoNegEntry
 * -------------------------------------------------------------------------
 * FUNCTION: Get auto-negoation entry of the MAU, that is specified in REF-3636
 * INPUT   : if_mau_auto_neg_entry->ifMauIfIndex -- Which interface.
 *           if_mau_auto_neg_entry->ifMauIndex   -- Which MAU.
 * OUTPUT  : if_mau_auto_neg_entry.
 * RETURN  : TRUE/FALSE
 * NOTE    : 1) Base on RFC-3636 (snmpDot3MauMgt)
 *           2) For User port only.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_GetIfMauAutoNegEntry (SWCTRL_IfMauAutoNegEntry_T *if_mau_auto_neg_entry);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetNextIfMauAutoNegEntry
 * -------------------------------------------------------------------------
 * FUNCTION: Get next auto-negoation entry of the MAU, one is specified in RFC-3636
 * INPUT   : if_mau_auto_neg_entry->ifMauIfIndex  -- Next to which interface.
 *           if_mau_auto_neg_entry->ifMauIndex    -- Next to which Which MAU.
 * OUTPUT  : if_mau_auto_neg_entry.
 * RETURN  : TRUE/FALSE
 * NOTE    : 1) Base on RFC-3636 (snmpDot3MauMgt)
 *           2) For User port only.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_GetNextIfMauAutoNegEntry (SWCTRL_IfMauAutoNegEntry_T *if_mau_auto_neg_entry);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetIfJackEntry
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
BOOL_T SWCTRL_GetIfJackEntry (SWCTRL_IfJackEntry_T *if_jack_entry);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetNextIfJackEntry
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
BOOL_T SWCTRL_GetNextIfJackEntry (SWCTRL_IfJackEntry_T *if_jack_entry);

#endif

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetPortSfpDdmTrapEnable
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
BOOL_T SWCTRL_SetPortSfpDdmTrapEnable(UI32_T ifindex, BOOL_T trap_enable);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetPortSfpDdmTrapEnable
 * -------------------------------------------------------------------------
 * FUNCTION: Get status of SFP ddm threshold alarm trap
 * INPUT   : ifindex   -- which ifindex
 *           trap_enable_p
 *
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------
 */
BOOL_T SWCTRL_GetPortSfpDdmTrapEnable(UI32_T ifindex, BOOL_T *trap_enable_p);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetPortSfpDdmThresholdAutoMode
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
BOOL_T SWCTRL_SetPortSfpDdmThresholdAutoMode(UI32_T ifindex, BOOL_T auto_mode);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetPortSfpDdmThresholdAutoMode
 * -------------------------------------------------------------------------
 * FUNCTION: Get status of port sfp ddm threshold auto mode
 *
 * INPUT   : ifindex
 * OUTPUT  : sfp_ddm_threshold_p
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SWCTRL_GetPortSfpDdmThresholdAutoMode(UI32_T ifindex, BOOL_T *auto_mode_p);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetPortSfpDdmThresholdDefault
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
BOOL_T SWCTRL_SetPortSfpDdmThresholdDefault(UI32_T ifindex, UI32_T threshold_type);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetPortSfpDdmThreshold
 * -------------------------------------------------------------------------
 * FUNCTION: Set SFP ddm threshold
 * INPUT   : ifindex   -- which ifindex
 *           threshold_type -- which threshold_type
 *           value
 *           need_to_check_range -- when cli provision, no need to check range
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------
 */
BOOL_T SWCTRL_SetPortSfpDdmThreshold(UI32_T ifindex, UI32_T threshold_type, I32_T val, BOOL_T need_to_check_range);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetPortSfpDdmThresholdForWeb
 * -------------------------------------------------------------------------
 * FUNCTION: Set SFP ddm threshold
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
BOOL_T SWCTRL_SetPortSfpDdmThresholdForWeb(UI32_T ifindex, UI32_T threshold_type, I32_T high_alarm, I32_T high_warning, I32_T low_warning, I32_T low_alarm);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetPortSfpDdmThreshold
 * -------------------------------------------------------------------------
 * FUNCTION: Set SFP ddm threshold
 * INPUT   : ifindex
 *           threshold_type -- which threshold_type
 *           value
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------
 */
BOOL_T SWCTRL_SetPortSfpDdmThresholdAll(UI32_T ifindex, SWCTRL_OM_SfpDdmThreshold_T *sfp_ddm_threshold_p);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetPortSfpDdmThreshold
 * -------------------------------------------------------------------------
 * FUNCTION: Get port sfp ddm threshold
 *
 * INPUT   : ifindex
 * OUTPUT  : sfp_ddm_threshold_p
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SWCTRL_GetPortSfpDdmThreshold(UI32_T ifindex, SWCTRL_OM_SfpDdmThreshold_T *sfp_ddm_threshold_p);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetPortSfpDdmThresholdEntryEntry
 * -------------------------------------------------------------------------
 * FUNCTION: Get port Sfp ddm threshold
 *
 * INPUT   : ifindex
 * OUTPUT  : Sfp_ddm_threshold_entry_p
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SWCTRL_GetPortSfpDdmThresholdEntryEntry(UI32_T ifindex, SWCTRL_OM_SfpDdmThresholdEntry_T *sfp_ddm_threshold_entry_p);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetNextPortSfpDdmThresholdEntry
 * -------------------------------------------------------------------------
 * FUNCTION: Get port sfp ddm threshold, auto_mode, and trap_enable
 * INPUT   : lport_p : Logical port num
 * OUTPUT  : SWCTRL_OM_SfpDdmThresholdEntry_T *sfp_ddm_threshold_entry_p
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : If tranceiver is not present, return FALSE.
 * -------------------------------------------------------------------------
 */
BOOL_T SWCTRL_GetNextPortSfpDdmThresholdEntry(UI32_T *lport_p, SWCTRL_OM_SfpDdmThresholdEntry_T *sfp_ddm_threshold_entry_p);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetPortSfpDdmThresholdStatus
 * -------------------------------------------------------------------------
 * FUNCTION: Get port sfp ddm threshold status
 *
 * INPUT   : ifindex
 * OUTPUT  : sfp_ddm_threshold_status_p
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SWCTRL_GetPortSfpDdmThresholdStatus(UI32_T ifindex, SWCTRL_OM_SfpDdmThresholdStatus_T *sfp_ddm_threshold_status_p);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetRunningPortSfpDdmThreshold
 * -------------------------------------------------------------------------
 * FUNCTION: Get running config of sfp ddm threshold
 * INPUT   : ifindex
 * OUTPUT  : ddm_threshold_p
 * RETURN  : SYS_TYPE_Get_Running_Cfg_T
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T SWCTRL_GetRunningPortSfpDdmThreshold(UI32_T ifindex, SWCTRL_OM_SfpDdmThresholdEntry_T *sfp_ddm_threshold_p);

#if (SYS_CPNT_COMBO_PORT_FORCE_MODE == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetSupportedPortComboForcedMode
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get supported port force link medium
 * INPUT   : ifindex       -- which port to set
 * OUTPUT  : None
 * RETURN  : Bitmap of VAL_portComboForcedMode_xxx
 * NOTE    : None
 * -------------------------------------------------------------------------*/
UI32_T SWCTRL_GetSupportedPortComboForcedMode(UI32_T ifindex);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetDefaultPortComboForcedMode
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get default port force link medium
 * INPUT   : ifindex       -- which port to set
 * OUTPUT  : forcedmode_p  -- which mode of medium
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_GetDefaultPortComboForcedMode(UI32_T ifindex, UI32_T *forcedmode_p);

#if (SYS_CPNT_COMBO_PORT_FORCED_MODE_SFP_SPEED == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetDefaultPortComboForcedModeSfpSpeed
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get default fiber medium speed
 * INPUT   : ifindex       -- which port to set
 * OUTPUT  : fiber_speed_p -- which mode of medium
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_GetDefaultPortComboForcedModeSfpSpeed(UI32_T ifindex, UI32_T *fiber_speed_p);
#endif

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetPortComboForcedMode
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
BOOL_T SWCTRL_SetPortComboForcedMode(UI32_T ifindex, UI32_T forcedmode, UI32_T fiber_speed);
#else
BOOL_T SWCTRL_SetPortComboForcedMode(UI32_T ifindex, UI32_T forcedmode);
#endif

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetPortComboForcedMode
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set port force link medium
 * INPUT   : ifindex       -- which port to set
 * OUTPUT  : forcedmode    -- which mode of medium
 *                      - VAL_portComboForcedMode_none                  1L
 *                      - VAL_portComboForcedMode_copperForced          2L
 *                      - VAL_portComboForcedMode_copperPreferredAuto   3L (Obsoleted)
 *                      - VAL_portComboForcedMode_sfpForced             4L
 *                      - VAL_portComboForcedMode_sfpPreferredAuto      5L
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : For trunk and non-combo port, mode only read and is VAL_portComboForcedMode_none
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_GetPortComboForcedMode(UI32_T ifindex, UI32_T *forcedmode);

/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetRunningPortComboForcedMode
 *------------------------------------------------------------------------
 * FUNCTION: This function will get the port combo force mode running config
 * INPUT   : ifindex       -- which port to set
 * OUTPUT  : forcedmode    -- which mode of medium
 *                      - VAL_portComboForcedMode_none                  1L
 *                      - VAL_portComboForcedMode_copperForced          2L
 *                      - VAL_portComboForcedMode_copperPreferredAuto   3L (Obsoleted)
 *                      - VAL_portComboForcedMode_sfpForced             4L
 *                      - VAL_portComboForcedMode_sfpPreferredAuto      5L
 * RETURN  : SYS_TYPE_Get_Running_Cfg_T
 * NOTE    : None
 *------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T SWCTRL_GetRunningPortComboForcedMode(UI32_T ifindex, UI32_T *forcedmode);
#endif

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
BOOL_T SWCTRL_EnableUMCASTIpTrap(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SWDRV_DisableUMCASTIpTrap
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion will disable trap unknown multicast ip
 * INPUT    :   None
 * OUTPUT   :   None
 * RETURN   :   TRUE/FALSE
 * NOTES    :   for XGS , disable IPMC , trap unknown ip or mcast packet
 * ------------------------------------------------------------------------
 */
BOOL_T SWCTRL_DisableUMCASTIpTrap(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SWCTRL_DisableUMCASTMacTrap
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion will disable trap unknown multicast mac
 * INPUT    :   None
 * OUTPUT   :   None
 * RETURN   :   TRUE/FALSE
 * NOTES    :   for XGS , disable IPMC , trap unknown mac or mcast packet
 *              Aaron add, 2003/07/31
 * ------------------------------------------------------------------------
 */
BOOL_T SWCTRL_DisableUMCASTMacTrap(void);

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
BOOL_T SWCTRL_EnableUMCASTMacTrap(void);

/* Arden, Patch */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - SWCTRL_DisablePortLearning
 * ------------------------------------------------------------------------
 * PURPOSE  :   Disable address learning on a specifc port
 * INPUT    :   UI32_T  l_port
 * OUTPUT   :   None
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None
 * ------------------------------------------------------------------------
 */
BOOL_T SWCTRL_DisablePortLearning(UI32_T l_port);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SWCTRL_EnablePortLearning
 * ------------------------------------------------------------------------
 * PURPOSE  :   Disable address learning on a specifc port
 * INPUT    :   UI32_T  l_port
 * OUTPUT   :   None
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None
 * ------------------------------------------------------------------------
 */
BOOL_T SWCTRL_EnablePortLearning(UI32_T l_port);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SWCTRL_SetPortLearningStatus
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
BOOL_T SWCTRL_SetPortLearningStatus(UI32_T ifindex, BOOL_T learning, SWCTRL_LearningDisabledOwner_T owner);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SWCTRL_SetPortLearningStatus
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
BOOL_T SWCTRL_SetPortLearningStatus(UI32_T ifindex, BOOL_T learning, SWCTRL_LearningDisabledOwner_T owner);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SWCTRL_GetPortLearningStatusEx
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
BOOL_T SWCTRL_GetPortLearningStatusEx(UI32_T ifindex, UI32_T *learning_disabled_status_p, UI32_T *intruder_handlers_p);

#if (SYS_CPNT_OSPF == TRUE)
/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_EnableOSPFTrap
 *------------------------------------------------------------------------
 * FUNCTION: This API is used to enable OSPF trap.
 * INPUT   : None.
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE.
 * NOTE    : None.
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_EnableOSPFTrap(void);

/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_DisableOSPFTrap
 *------------------------------------------------------------------------
 * FUNCTION: This API is used to disable OSPF trap.
 * INPUT   : None.
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE.
 * NOTE    : None.
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_DisableOSPFTrap(void);
#endif


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetPortStateWithMstidx
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
BOOL_T SWCTRL_SetPortStateWithMstidx (UI32_T mstidx, UI32_T lport, UI32_T state);

#if (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_MSTP)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_AddVlanToMst
 * -------------------------------------------------------------------------
 * FUNCTION: This function adds a VLAN to a given Spanning Tree instance.
 * INPUT   : vid                -- the VLAN will be added to a given Spanning Tree
 *           mstidx             -- mstidx (multiple spanning tree index) to identify a unique spanning tree
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_AddVlanToMst(UI32_T vid, UI32_T mstidx);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_DeleteVlanFromMst
 * -------------------------------------------------------------------------
 * FUNCTION: This function deletes a VLAN from a given Spanning Tree instance.
 * INPUT   : vid                -- the VLAN will be added to a given Spanning Tree
 *           mstidx             -- mstidx (multiple spanning tree index) to identify a unique spanning tree
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_DeleteVlanFromMst(UI32_T vid, UI32_T mstidx);
#endif /* end of #if (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_MSTP)*/


#if (SYS_CPNT_DOT1X == TRUE)
enum SWCTRL_DOT1X_Packet_Operation_E
{
    SWCTRL_DOT1X_PACKET_DISCARD = 0,
    SWCTRL_DOT1X_PACKET_FORWARD,
    SWCTRL_DOT1X_PACKET_TRAPTOCPU
};

/****************************************************************************/
/* DOT1X                                                                     */
/****************************************************************************/
/* -------------------------------------------------------------------------
* ROUTINE NAME - SWCTRL_SetDot1xAuthTrap
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
BOOL_T SWCTRL_SetDot1xAuthTrap(UI32_T ifindex, UI32_T mode);

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
BOOL_T SWCTRL_SetDot1xAuthControlMode(UI32_T ifindex, UI32_T mode);
#endif

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SWCTRL_MGR_HandleIPCReqMsg
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Handle the ipc request message for csca mgr.
 * INPUT:
 *    ipcmsg_p  --  input request ipc message buffer
 *
 * OUTPUT:
 *    ipcmsg_p  --  output response ipc message buffer
 *
 * RETURN:
 *    TRUE  - There is a response need to send.
 *    FALSE - There is no response to send.
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T SWCTRL_MGR_HandleIPCReqMsg(SYSFUN_Msg_T* ipcmsg_p);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetPortSfpEntry
 * -------------------------------------------------------------------------
 * FUNCTION: This function is used to get information from om
 * INPUT   : UI32_T lport : Logical port num
 * OUTPUT  : SWCTRL_OM_SfpEntry_T *sfp_entry_p
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : If tranceiver is not present, return FALSE.
 * -------------------------------------------------------------------------
 */
BOOL_T SWCTRL_GetPortSfpEntry(UI32_T lport, SWCTRL_OM_SfpEntry_T *sfp_entry_p);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetNextPortSfpEntry
 * -------------------------------------------------------------------------
 * FUNCTION: This function is used to get next sfp information from om
 * INPUT   : UI32_T lport_p : Logical port num
 * OUTPUT  : SWCTRL_OM_SfpEntry_T *sfp_entry_p
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : If tranceiver is not present, return FALSE.
 * -------------------------------------------------------------------------
 */
BOOL_T SWCTRL_GetNextPortSfpEntry(UI32_T *lport_p, SWCTRL_OM_SfpEntry_T *sfp_entry_p);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetPortSfpDdmEntry
 * -------------------------------------------------------------------------
 * FUNCTION: This function is used to get ddm information from om
 * INPUT   : UI32_T lport : Logical port num
 * OUTPUT  : SWCTRL_OM_SfpDdmEntry_T *sfp_ddm_entry_p
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : If tranceiver is not present, return FALSE.
 * -------------------------------------------------------------------------
 */
BOOL_T SWCTRL_GetPortSfpDdmEntry(UI32_T lport, SWCTRL_OM_SfpDdmEntry_T *sfp_ddm_entry_p);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetNextPortSfpDdmEntry
 * -------------------------------------------------------------------------
 * FUNCTION: This function is used to get next ddm information from om
 * INPUT   : UI32_T lport_p : Logical port num
 * OUTPUT  : SWCTRL_OM_SfpDdmEntry_T *sfp_ddm_entry_p
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : If tranceiver is not present, return FALSE.
 * -------------------------------------------------------------------------
 */
BOOL_T SWCTRL_GetNextPortSfpDdmEntry(UI32_T *lport_p, SWCTRL_OM_SfpDdmEntry_T *sfp_ddm_entry_p);

#if (SYS_CPNT_SWCTRL_CABLE_DIAG == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_ExecuteCableDiag
 * -------------------------------------------------------------------------
 * FUNCTION: Get Cable diag result of specific port
 * INPUT   : lport : Logical port num
 * OUTPUT  : result : result of the cable diag test for the port including
 *                    pair1&2 status and fault length
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_ExecuteCableDiag(UI32_T lport, SWCTRL_Cable_Info_T *result);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetCableDiagResult
 * -------------------------------------------------------------------------
 * FUNCTION: Get Cable diag result of specific port by latest result
 * INPUT   : lport : Logical port num
 * OUTPUT  : result : result of the cable diag test for the port
 * RETURN  : TRUE/FALSE
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_GetCableDiagResult(UI32_T lport, SWCTRL_Cable_Info_T *result);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetNextCableDiagResult
 * -------------------------------------------------------------------------
 * FUNCTION: Get Cable diag result of specific port by latest result
 * INPUT   : lport : Logical port num
 * OUTPUT  : result : result of the cable diag test for the port
 * RETURN  : TRUE/FALSE
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_GetNextCableDiagResult(UI32_T *lport, SWCTRL_Cable_Info_T *result);
#endif    /* #if (SYS_CPNT_SWCTRL_CABLE_DIAG == TRUE) */

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SWCTRL_OM_HandleIPCReqMsg
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Handle the ipc request message for csca om.
 * INPUT:
 *    ipcmsg_p  --  input request ipc message buffer
 *
 * OUTPUT:
 *    ipcmsg_p  --  output response ipc message buffer
 *
 * RETURN:
 *    TRUE  --  There is a response need to send.
 *    FALSE --  No response need to send.
 *
 * NOTES:
 *    1.The size of ipcmsg_p.msg_buf must be large enough to carry any response
 *      messages.
 *------------------------------------------------------------------------------
 */
BOOL_T SWCTRL_OM_HandleIPCReqMsg(SYSFUN_Msg_T* ipcmsg_p);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SWCTRL_LportListToUportList
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Convert logical port list to user port list.
 * INPUT:
 *    lport_list  --  the logical port bitmap to be converted
 *
 * OUTPUT:
 *    uport_list  --  the resulted user port bitmap
 *
 * RETURN:
 *    TRUE  --  Success
 *    FALSE --  Fail
 *
 * NOTES:
 *    1. The size of lport_list/uport_list is
 *       SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST.
 *    2. The uport_list space should be allocated by caller.
 *------------------------------------------------------------------------------
 */
BOOL_T SWCTRL_LportListToUportList(UI8_T *lport_list, UI8_T *uport_list);

#if (SYS_CPNT_RATE_BASED_STORM_CONTROL == TRUE)
/* -------------------------------------------------------------------------
* ROUTINE NAME - SWCTRL_SetRateBasedStormControl
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
BOOL_T SWCTRL_SetRateBasedStormControl(UI32_T ifindex, UI32_T rate, UI32_T mode);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetRateBasedStormControl
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get rate based storm control settings.
 * INPUT   : ifindex
 * OUTPUT  : rate
 *           mode
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : specifically for BCM53115
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_GetRateBasedStormControl(UI32_T ifindex, UI32_T *rate, UI32_T *mode);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetNextRateBasedStormControl
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get rate based storm control settings.
 * INPUT   : ifindex
 * OUTPUT  : ifindex
 *           rate
 *           mode
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : specifically for BCM53115
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_GetNextRateBasedStormControl(UI32_T *ifindex, UI32_T *rate, UI32_T *mode);
#endif
#if (SYS_CPNT_MLDSNP == TRUE)
/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_EnableMldPacketTrap
 *------------------------------------------------------------------------
 * FUNCTION: This API is used to enable MLD packet trap.
 * INPUT   : None.
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE.
 * NOTE    : None.
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_EnableMldPacketTrap(SWCTRL_TrapPktOwner_T owner);

/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_DisableMldPacketTrap
 *------------------------------------------------------------------------
 * FUNCTION: This API is used to enable MLD packet trap.
 * INPUT   : None.
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE.
 * NOTE    : None.
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_DisableMldPacketTrap(SWCTRL_TrapPktOwner_T owner);
#endif /*#if (SYS_CPNT_MLDSNP == TRUE)*/

#if (SYS_CPNT_IPV6_RA_GUARD_SW_RELAY == TRUE)
/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetRaAndRrPacketTrap
 *------------------------------------------------------------------------
 * FUNCTION: To enable/disable RA/RR packet trap.
 * INPUT   : is_enabled - TRUE to enable
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE.
 * NOTE    : None.
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetRaAndRrPacketTrap(
    BOOL_T  is_enabled);

#endif /* #if (SYS_CPNT_IPV6_RA_GUARD_SW_RELAY == TRUE) */

#if (SYS_CPNT_IPV6_RA_GUARD_DROP_BY_RULE == TRUE)
/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetPortRaAndRrPacketDrop
 *------------------------------------------------------------------------
 * FUNCTION: To enable/disable RA/RR packet drop by specified ifindex.
 * INPUT   : ifindex    - ifindex to enable/disable
 *           is_enabled - TRUE to enable
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE.
 * NOTE    : None.
 *------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetPortRaAndRrPacketDrop(
    UI32_T  ifindex, BOOL_T  is_enabled);

#endif /* #if (SYS_CPNT_IPV6_RA_GUARD_DROP_BY_RULE == TRUE) */

#if (SYS_CPNT_EFM_OAM == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetOamLoopback
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
BOOL_T SWCTRL_SetOamLoopback(UI32_T l_port, BOOL_T enable, UI32_T flag);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetOperationSpeedDuplex
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will return this port's operation speed and duplex
 * INPUT:    lport, *upStream,*downSteram
 * OUTPUT:   - VAL_portSpeedDpxCfg_halfDuplex10
 *          VAL_portSpeedDpxCfg_fullDuplex10
 *          VAL_portSpeedDpxCfg_halfDuplex100
 *          VAL_portSpeedDpxCfg_fullDuplex100
 *          VAL_portSpeedDpxCfg_fullDuplex1000
 *
 * RETURN:   Success --> TRUE; Fail --> FALSE
 * NOTE: if only support one stream, the upStream and downStream will be the same value
 *      now, we just support one stream.
 *---------------------------------------------------------------------------
 */
BOOL_T SWCTRL_GetOperationSpeedDuplex(UI32_T l_port, UI32_T *upSteam, UI32_T *downStream);

#endif/* End of SYS_CPNT_EFM_OAM */

#if (SYS_CPNT_INTERNAL_LOOPBACK_TEST == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetInternalLoopback
 * -------------------------------------------------------------------------
 * FUNCTION: This function will enable internal loopback mode
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
BOOL_T SWCTRL_SetInternalLoopback(UI32_T l_port, BOOL_T enable);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_PacketHandler
 * -------------------------------------------------------------------------
 * FUNCTION: This function will handle the packet trap to swctrl
 * INPUT   :
 * OUTPUT  : None
 * RETURN  : True: need response, FALSE:
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_PacketHandler(UI8_T *dst_mac, UI8_T *src_mac, UI16_T type, SYSFUN_Msg_T* ipcmsg_p);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_ExecuteInternalLoopbackTest
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will execute the internal loopback test
 * INPUT:    lport
 * OUTPUT:   None
 * RETURN:   None
 * NOTE: start a timeout timer and send the loopback frame.
 *---------------------------------------------------------------------------
 */
BOOL_T SWCTRL_ExecuteInternalLoopbackTest(UI32_T lport);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetInternalLoopbackTestResult
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get internal loopback test result
 * INPUT   : lport
 * OUTPUT  : lport
 *           result
 *           result_time
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_GetInternalLoopbackTestResult(UI32_T lport, UI32_T *result, UI32_T *result_time);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetNextInternalLoopbackTestResult
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get rate based storm control settings.
 * INPUT   : lport
 * OUTPUT  : lport
 *           result
 *           result_time
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_GetNextInternalLoopbackTestResult(UI32_T *lport, UI32_T *result, UI32_T *result_time);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_InternalLoopbackTimeout
 * -------------------------------------------------------------------------
 * FUNCTION: This function will handle the internal loopback timeout
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_InternalLoopbackTimeout();
#endif
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
BOOL_T SWCTRL_SetPortMACLearningStatus(UI32_T ifindex, BOOL_T status);
#endif



BOOL_T SWCTRL_SetQosTrustMode(UI32_T ifindex, UI32_T mode);

BOOL_T SWCTRL_SetPortListQosTrustMode(UI8_T *port_list, UI32_T mode);

BOOL_T SWCTRL_SetQosIngressCos2Dscp(UI32_T lport,UI32_T cos,UI32_T cfi,UI32_T phb,UI32_T color);

BOOL_T SWCTRL_SetPortListQosIngressCos2Dscp(UI8_T *port_list,UI32_T cos,UI32_T cfi,UI32_T phb,UI32_T color);

BOOL_T SWCTRL_SetQosIngressPre2Dscp(UI32_T lport,UI32_T pre,UI32_T phb,UI32_T color);

BOOL_T SWCTRL_SetPortListQosIngressPre2Dscp(UI8_T *port_list,UI32_T pre,UI32_T phb,UI32_T color);

BOOL_T SWCTRL_SetQosIngressDscp2Dscp(UI32_T lport,UI32_T o_dscp,UI32_T phb,UI32_T color);

BOOL_T SWCTRL_SetPortListQosIngressDscp2Dscp(UI8_T *port_list,UI32_T o_dscp,UI32_T phb,UI32_T color);

BOOL_T SWCTRL_SetQosIngressDscp2Queue(UI32_T lport,UI32_T phb,UI32_T queue);

BOOL_T SWCTRL_SetPortListQosIngressDscp2Queue(UI8_T *port_list,UI32_T phb,UI32_T queue);

BOOL_T SWCTRL_SetQosIngressDscp2Color(UI32_T lport,UI32_T phb,UI32_T color);

BOOL_T SWCTRL_SetPortListQosIngressDscp2Color(UI8_T *port_list,UI32_T phb,UI32_T color);

BOOL_T SWCTRL_SetQosIngressDscp2Cos(UI32_T lport,UI32_T phb,UI32_T color,UI32_T cos,UI32_T cfi);

BOOL_T SWCTRL_SetPortListQosIngressDscp2Cos(UI8_T *port_list,UI32_T phb,UI32_T color,UI32_T cos,UI32_T cfi);

#if (SYS_CPNT_REFINE_ISC_MSG == TRUE)
void SWCTRL_lportlist2uportlist(UI8_T *lportlist);
BOOL_T SWCTRL_DisableIngressFilter_PortList(UI8_T* port_list);
BOOL_T SWCTRL_EnableIngressFilter_PortList(UI8_T* port_list);
BOOL_T SWCTRL_AdmitAllFrames_PortList(UI8_T* port_list);
BOOL_T SWCTRL_SetPortPVID_PortList(UI8_T* port_list,UI32_T pvid);
BOOL_T SWCTRL_AddPortToVlanUntaggedSet_PortList(UI8_T* port_list,UI32_T vid);
BOOL_T SWCTRL_AddPortToVlanMemberSet_PortList(UI8_T* port_list,UI32_T vid);


#endif
BOOL_T SWCTRL_GetNextIndexFromPortList (UI32_T *index, UI8_T  port_list[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST]);
BOOL_T SWCTRL_IsAvailableConfiguredPort(UI32_T   ifindex);

BOOL_T SWCTRL_IsManagementPort(UI32_T ifindex);


#if (SYS_CPNT_SWCTRL_MDIX_CONFIG == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetPortMode
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get port mode
 * INPUT   : ifindex       -- which port to get
 * OUTPUT  : mode    -- which mode of port
 *                       - SWCTRL_PORT_MODE_UNKNOWN              0L
 *                       - SWCTRL_PORT_MODE_FRONT                1L
 *                       - SWCTRL_PORT_MODE_COMBO                2L
 *                       - SWCTRL_PORT_MODE_COMBO_SLOT           3L
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : For trunk port , mode can't getting
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_GetPortMode(UI32_T ifindex, UI32_T *port_mode);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetMDIXMode
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set port MDIX mode
 * INPUT   : ifindex     -- which port to set
 *           mode        -- which mode of media
 *                       - VAL_portMdixMode_auto                  1L
 *                       - VAL_portMdixMode_straight              2L
 *                       - VAL_portMdixMode_crossover             3L
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : For trunk port , mode can't setting
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetMDIXMode(UI32_T ifindex, UI32_T mode);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetMDIXMode
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get port MDIX mode
 * INPUT   : ifindex       -- which port to get
 * OUTPUT  : mode    -- which mode of medium
 *                       - VAL_portMdixMode_auto                  1L
 *                       - VAL_portMdixMode_normal                2L
 *                       - VAL_portMdixMode_crossover             3L
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : For trunk port , mode can't getting
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_GetMDIXMode(UI32_T ifindex, UI32_T *mode);


/*--------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetNextMDIXMode
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get next port and MDIX mode
 * INPUT   : ifindex   -- which port to get
 * OUTPUT  : ifindex   -- the next existing port
 *           mode   -- mode of the next port
 *                       - VAL_portMdixMode_auto                  1L
 *                       - VAL_portMdixMode_straight                2L
 *                       - VAL_portMdixMode_crossover             3L
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 *---------------------------------------------------------------------------*/
BOOL_T SWCTRL_GetNextMDIXMode(UI32_T *ifindex, UI32_T *mode);


/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetRunningMDIXMode
 *------------------------------------------------------------------------
 * FUNCTION: This function will get the port MDIX mode running config
 * INPUT   : ifindex       -- which port to get
 * OUTPUT  : mode    -- which mode of medium
 *                       - VAL_portMdixMode_auto                  1L
 *                       - VAL_portMdixMode_straight              2L
 *                       - VAL_portMdixMode_crossover             3L
 * RETURN  : SYS_TYPE_Get_Running_Cfg_T
 * NOTE    : None
 *------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T SWCTRL_GetRunningMDIXMode(UI32_T ifindex, UI32_T *mode);


/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetNextRunningMDIXMode
 *------------------------------------------------------------------------
 * FUNCTION: This function will get the port MDIX mode running config
 * INPUT   : ifindex       -- which port to get
 * OUTPUT  : ifindex   -- the next existing port
 *           mode   -- mode of the next port
 *                       - VAL_portMdixMode_auto                  1L
 *                       - VAL_portMdixMode_straight                2L
 *                       - VAL_portMdixMode_crossover             3L
 * RETURN  : SYS_TYPE_Get_Running_Cfg_T
 * NOTE    : None
 *------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T SWCTRL_GetNextRunningMDIXMode(UI32_T *ifindex, UI32_T *mode);
#endif

#if (SYS_CPNT_MAC_VLAN == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetMacVlanEntry
 * -------------------------------------------------------------------------
 * PURPOSE  : Set the MAC VLAN entry
 * INPUT    : mac_address   - only allow unitcast address
 *            vid           - the VLAN ID
 *                            the valid value is 1 ~ SYS_DFLT_DOT1QMAXVLANID
 *            priority      - the priority
 *                            the valid value is 0 ~ 7
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE    - TRUE if successful;FALSE if failed
 * NOTES    : if SYS_CPNT_MAC_VLAN_WITH_PRIORITY == FALSE, it's recommanded
 *            that set input priority to 0.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetMacVlanEntry(UI8_T *mac_address, UI8_T *mask,UI16_T vid, UI8_T priority);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_DeleteMacVlanEntry
 * -------------------------------------------------------------------------
 * PURPOSE  : Delete Mac Vlan entry
 * INPUT    : mac_address   - only allow unitcast address
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE    - TRUE if successful;FALSE if failed
 * NOTES    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_DeleteMacVlanEntry(UI8_T *mac_address, UI8_T *mask);

#endif /* end of #if (SYS_CPNT_MAC_VLAN == TRUE) */

#if (SYS_CPNT_DOT1X_EAPOL_PASS_THROUGH == TRUE)
/* -------------------------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetEapolFramePassThrough
 * -------------------------------------------------------------------------------------------
 * PURPOSE : To set EAPOL frames pass through (pass through means not trapped to CPU)
 * INPUT   : state (TRUE/FALSE)
 * OUTPUT  : None
 * RETURN  : TRUE
 * NOTE    : None
 * -------------------------------------------------------------------------------------------
 */
BOOL_T SWCTRL_SetEapolFramePassThrough(BOOL_T state);
#endif /* #if (SYS_CPNT_DOT1X_EAPOL_PASS_THROUGH == TRUE) */

#if (SYS_CPNT_POWER_SAVE == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetPortPowerSave
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set the port power saving status
 * INPUT   : ifindex --which port to enable/disable power save
 *               status--TRUE:enable
 *                           FALSE:disable
 * OUTPUT  :
 * RETURN  : True: Successfully, False: If not available
 * NOTE    :None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetPortPowerSave(UI32_T ifindex,BOOL_T status);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetPortPowerSaveStatus
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get the power save status
 * INPUT   : ifindex--the port to get power save status
 * OUTPUT  : status--the power save status of a specific port
 * RETURN  : True: Successfully
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_GetPortPowerSaveStatus(UI32_T ifindex,BOOL_T *status);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetRunningPortPowerSaveStatus
 * -------------------------------------------------------------------------
 * FUNCTION: This function get the power save status
 * INPUT   : ifindex--the port to get power save status
 * OUTPUT  : status--the power save status of a specific port
 * RETURN  : SYS_TYPE_Get_Running_Cfg_T
 * NOTE    :
 * -------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T SWCTRL_GetRunningPortPowerSaveStatus(UI32_T ifindex, BOOL_T *status);
#endif /* #if (SYS_CPNT_POWER_SAVE == TRUE) */

#if (SYS_CPNT_RSPAN == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_RspanSettingValidation
 * -------------------------------------------------------------------------
 * FUNCTION: Validate if the target port is a existed port or trunk (member).
 * INPUT   : target_port -- the port to get
 * OUTPUT  : None
 * RETURN  :
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SWCTRL_RspanSettingValidation ( UI8_T target_port );

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_ModifyMaxFrameSizeForRspan
 * -------------------------------------------------------------------------
 * FUNCTION: This function will modify maximum fram size of RSPAN
 * INPUT   : ifindex
 *           is_increase
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SWCTRL_ModifyMaxFrameSizeForRspan(UI32_T ifindex, BOOL_T is_increase);
#endif  /* SYS_CPNT_RSPAN */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_TrapUnknownIpMcastToCPU
 * -------------------------------------------------------------------------
 * FUNCTION: This function will trap unknown ip multicast packet to CPU
 * INPUT   : to_cpu -- trap to cpu or not.
 *           flood  -- TRUE to flood to other ports; FLASE to discard the traffic.
 *           owner  -- who want the trap
 *           vid = 0 -- global setting, vid = else -- which VLAN ID to set
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : to_cpu = 1 means trap to CPU
 *               to_cpu = 0 mans don't trap to CPU
 *               to_cpu = else means modify flood
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_TrapUnknownIpMcastToCPU(UI8_T to_cpu, BOOL_T flood, SWCTRL_TrapPktOwner_T owner, UI32_T vid);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_TrapUnknownIpv6McastToCPU
 * -------------------------------------------------------------------------
 * FUNCTION: This function will trap unknown ipv6 multicast packet to CPU
 * INPUT   : to_cpu -- trap to cpu or not.
 *           flood  -- TRUE to flood to other ports; FLASE to discard the traffic.
 *           owner  -- who want the trap
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    : to_cpu = 1 means trap to CPU
 *               to_cpu = 0 mans don't trap to CPU
 *               to_cpu = else means modify flood
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_TrapUnknownIpv6McastToCPU(UI8_T to_cpu, BOOL_T flood, SWCTRL_TrapPktOwner_T owner);

#if (SYS_CPNT_MLDSNP == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_TrapIpv6PIMToCPU
 * -------------------------------------------------------------------------
 * FUNCTION: This function will trap ipv6 PIM packet to CPU
 * INPUT   : to_cpu -- trap to cpu or not.
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_TrapIpv6PIMToCPU(UI8_T to_cpu, SWCTRL_TrapPktOwner_T owner);
#endif

#if (SYS_CPNT_IP_MULTICAST_DATA_GUARD== TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_DropIpMulticastData
 * -------------------------------------------------------------------------
 * FUNCTION: Set port drop ip multicast data
 * INPUT   : lport - logical port
 * OUTPUT  : None
 * RETURN  :
 * NOTE    : if this port is trunk port, it will set all trunk member
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_DropIpMulticastData(UI32_T lport, BOOL_T enabled);
#endif
#if (SYS_CPNT_IPV6_MULTICAST_DATA_DROP== TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_DropIpv6MulticastData
 * -------------------------------------------------------------------------
 * FUNCTION: Set port drop ip multicast data
 * INPUT   : lport - logical port
 * OUTPUT  : None
 * RETURN  :
 * NOTE    : if this port is trunk port, it will set all trunk member
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_DropIpv6MulticastData(UI32_T lport, BOOL_T enabled);
#endif
#if (SYS_CPNT_ITRI_MIM == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_ITRI_MIM_SetStatus
 * -------------------------------------------------------------------------
 * FUNCTION: Set status of ITRI MAC-in-MAC
 * INPUT   : ifindex
 *           status
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_ITRI_MIM_SetStatus(UI32_T ifindex, BOOL_T status);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_ITRI_MIM_GetStatus
 * -------------------------------------------------------------------------
 * FUNCTION: Set status of ITRI MAC-in-MAC
 * INPUT   : ifindex
 * OUTPUT  : status_p
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_ITRI_MIM_GetStatus(UI32_T ifindex, BOOL_T *status_p);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_ITRI_MIM_GetNextStatus
 * -------------------------------------------------------------------------
 * FUNCTION: Set status of ITRI MAC-in-MAC
 * INPUT   : ifindex_p
 * OUTPUT  : ifindex_p
 *           status_p
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_ITRI_MIM_GetNextStatus(UI32_T *ifindex_p, BOOL_T *status_p);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_ITRI_MIM_GetRunningStatus
 * -------------------------------------------------------------------------
 * FUNCTION: Set status of ITRI MAC-in-MAC
 * INPUT   : ifindex
 * OUTPUT  : status_p
 * RETURN  : SYS_TYPE_Get_Running_Cfg_T
 * NOTE    : None
 * -------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T SWCTRL_ITRI_MIM_GetRunningStatus(UI32_T ifindex, BOOL_T *status_p);
#endif /* (SYS_CPNT_ITRI_MIM == TRUE) */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_EnableDhcpPacketTrap
 * -------------------------------------------------------------------------
 * FUNCTION: This function will trap dhcp packet to CPU
 * INPUT   : owner  -- who want the trap
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_EnableDhcpPacketTrap(SWCTRL_TrapPktOwner_T owner);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_DisableDhcpPacketTrap
 * -------------------------------------------------------------------------
 * FUNCTION: This function will trap dhcp packet to CPU
 * INPUT   : owner  -- who want the trap
 * OUTPUT  : None
 * RETURN  : True: Successfully, False: If not available
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_DisableDhcpPacketTrap(SWCTRL_TrapPktOwner_T owner);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetDhcpPacketFlag
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get dhcp packet flag
 * INPUT   :   None
 * OUTPUT  : server_flag_p, client_flag_p
 * RETURN  : True: Successfully, False: If not available
 * NOTE    :
 *
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_GetDhcpPacketFlag(UI16_T *server_flag_p, UI16_T *client_flag_p);

#if (SYS_CPNT_CLUSTER == TRUE)
/* -------------------------------------------------------------------------
 * FUNCTION NAME - SWCTRL_SetOrgSpecificTrapStatus
 * -------------------------------------------------------------------------
 * PURPOSE : Set whether organization specific frames are trapped to CPU.
 * INPUT   : owner - who set the status
 *           status  - TRUE / FALSE
 * OUTPUT  : None
 * RETURN  : TRUE / FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SWCTRL_SetOrgSpecificTrapStatus(SWCTRL_TrapPktOwner_T owner, BOOL_T status);
#endif /* #if (SYS_CPNT_CLUSTER == TRUE */

#if (SYS_CPNT_PPPOE_IA == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - SWCTRL_SetPPPoEDPktToCpu
 *-------------------------------------------------------------------------
 * PURPOSE: This function will enable/disable traping
 *          PPPoE discover packets to cpu for specified ifindex.
 * INPUT  : ifindex   - ifindex to enable/disable
 *          is_enable - the packet trapping is enabled or disabled
 * OUTPUT : None
 * RETURN : TRUE  - success
 *          FALSE - fail
 * NOTE   : 1. if ifindex is trunk, apply to all member ports
 *          2. if ifindex is normal/trunk member, apply to this port
 *          3. for projects who can install rule on trunk's member ports.
 *-------------------------------------------------------------------------
 */
BOOL_T SWCTRL_SetPPPoEDPktToCpu(UI32_T ifindex, BOOL_T is_enable);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SWCTRL_SetPPPoEDPktToCpuPerSystem
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
BOOL_T SWCTRL_SetPPPoEDPktToCpuPerSystem(BOOL_T is_enable);

#endif /* #if (SYS_CPNT_PPPOE_IA == TRUE) */

#if (SYS_CPNT_DOS == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetDosProtectionFilter
 * -------------------------------------------------------------------------
 * FUNCTION: This function will config DoS protection
 * INPUT   : type   - the type of DOS protection to config
 *           enable - TRUE to enable; FALSE to disable.
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetDosProtectionFilter(SWCTRL_DosProtectionFilter_T type, BOOL_T enable);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetDosProtectionRateLimit
 * -------------------------------------------------------------------------
 * FUNCTION: This function will config DoS protection
 * INPUT   : type   - the type of DOS protection to config
 *           rate   - rate in kbps. 0 to disable.
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetDosProtectionRateLimit(SWCTRL_DosProtectionRateLimit_T type, UI32_T rate);
#endif /* (SYS_CPNT_DOS == TRUE) */

#if ((SYS_CPNT_DHCPV6 == TRUE)||(SYS_CPNT_DHCPV6SNP == TRUE))
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_EnableDhcp6PacketTrap
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
BOOL_T SWCTRL_EnableDhcp6PacketTrap(SWCTRL_TrapPktOwner_T owner);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_DisableDhcp6PacketTrap
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
BOOL_T SWCTRL_DisableDhcp6PacketTrap(SWCTRL_TrapPktOwner_T owner);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetDhcp6PacketFlag
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get dhcp6 packet flag
 * INPUT   :   None
 * OUTPUT  : server_flag_p, client_flag_p
 * RETURN  : True: Successfully, False: If not available
 * NOTE    :
 *
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_GetDhcp6PacketFlag(UI16_T *server_flag_p, UI16_T *client_flag_p);
#endif

#if (SYS_CPNT_NDSNP == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_EnableNdPacketTrap
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
BOOL_T SWCTRL_EnableNdPacketTrap(SWCTRL_TrapPktOwner_T owner);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_DisableNdPacketTrap
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
BOOL_T SWCTRL_DisableNdPacketTrap(SWCTRL_TrapPktOwner_T owner);

#endif
/* -------------------------------------------------------------------------
 * FUNCTION NAME - SWCTRL_SetPktTrapStatus
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
BOOL_T SWCTRL_SetPktTrapStatus(SWCTRL_PktType_T pkt_type, SWCTRL_TrapPktOwner_T owner, BOOL_T to_cpu, BOOL_T drop);

/* -------------------------------------------------------------------------
 * FUNCTION NAME - SWCTRL_GetPktTrapStatus
 * -------------------------------------------------------------------------
 * PURPOSE : to trap packet
 * INPUT   : pkt_type  - which packet to trap
 *           owner     - who set the status
 * OUTPUT  : to_cpu_p  - trap to cpu or not
 *           drop_p    - drop packet or not
 * RETURN  : TRUE / FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SWCTRL_GetPktTrapStatus(SWCTRL_PktType_T pkt_type, SWCTRL_TrapPktOwner_T owner, BOOL_T *to_cpu_p, BOOL_T *drop_p);

/* -------------------------------------------------------------------------
 * FUNCTION NAME - SWCTRL_GetPktTrapStatusEx
 * -------------------------------------------------------------------------
 * PURPOSE : to trap packet
 * INPUT   : pkt_type  - which packet to trap
 * OUTPUT  : trap_owners_p - owners to trap packet
 *           drop_owners_p  - owners to drop packet
 * RETURN  : TRUE / FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SWCTRL_GetPktTrapStatusEx(SWCTRL_PktType_T pkt_type, UI32_T *trap_owners_p, UI32_T *drop_owners_p);

/* -------------------------------------------------------------------------
 * FUNCTION NAME - SWCTRL_SetPortPktTrapStatus
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
BOOL_T SWCTRL_SetPortPktTrapStatus(UI32_T ifindex, SWCTRL_PktType_T pkt_type, SWCTRL_TrapPktOwner_T owner, BOOL_T to_cpu, BOOL_T drop);

/* -------------------------------------------------------------------------
 * FUNCTION NAME - SWCTRL_GetPortPktTrapStatus
 * -------------------------------------------------------------------------
 * PURPOSE : to trap packet
 * INPUT   : ifindex
 *           pkt_type  - which packet to trap
 *           owner     - who set the status
 * OUTPUT  : to_cpu_p  - trap to cpu or not
 *           drop_p    - drop packet or not
 * RETURN  : TRUE / FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SWCTRL_GetPortPktTrapStatus(UI32_T ifindex, SWCTRL_PktType_T pkt_type, SWCTRL_TrapPktOwner_T owner, BOOL_T *to_cpu_p, BOOL_T *drop_p);

/* -------------------------------------------------------------------------
 * FUNCTION NAME - SWCTRL_GetPortPktTrapStatusEx
 * -------------------------------------------------------------------------
 * PURPOSE : to trap packet
 * INPUT   : ifindex
 *           pkt_type  - which packet to trap
 * OUTPUT  : trap_owners_p - owners to trap packet
 *           drop_owners_p  - owners to drop packet
 * RETURN  : TRUE / FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SWCTRL_GetPortPktTrapStatusEx(UI32_T ifindex, SWCTRL_PktType_T pkt_type, UI32_T *trap_owners_p, UI32_T *drop_owners_p);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetPDPortStatus
 * -------------------------------------------------------------------------
 * FUNCTION: Get PD port status
 * INPUT   : entry_p->port_pd_ifindex
 * OUTPUT  : entry_p->port_pd_status --- SWDRV_POWER_SOURCE_NONE:None
 *                                       SWDRV_POWER_SOURCE_UP:Up
 *                                       SWDRV_POWER_SOURCE_DOWN:Down
 *           entry_p->port_pd_mode   --- SWDRV_POWERED_DEVICE_MODE_NONE
 *                                       SWDRV_POWERED_DEVICE_MODE_AF
 *                                       SWDRV_POWERED_DEVICE_MODE_AT
 * RETURN  : TRUE -- Sucess, FALSE -- Failed
 * NOTE    : The status of the ports with POE PD capability would show "UP"
 *           when the link partner is a PSE port.
 * -------------------------------------------------------------------------
 */
BOOL_T SWCTRL_GetPDPortStatus(SWCTRL_PortPD_T *entry_p);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetNextPDPortStatus
 * -------------------------------------------------------------------------
 * FUNCTION: Get next PD port status
 * INPUT   : entry_p->port_pd_ifindex
 * OUTPUT  : entry_p->port_pd_ifindex
 *           entry_p->port_pd_status --- SWDRV_POWER_SOURCE_NONE:None
 *                                       SWDRV_POWER_SOURCE_UP:Up
 *                                       SWDRV_POWER_SOURCE_DOWN:Down
 *           entry_p->port_pd_mode   --- SWDRV_POWERED_DEVICE_MODE_NONE
 *                                       SWDRV_POWERED_DEVICE_MODE_AF
 *                                       SWDRV_POWERED_DEVICE_MODE_AT
 * RETURN  : TRUE -- Sucess, FALSE -- Failed
 * NOTE    : The status of the ports with POE PD capability would show "UP"
 *           when the link partner is a PSE port.
 * -------------------------------------------------------------------------
 */
BOOL_T SWCTRL_GetNextPDPortStatus(SWCTRL_PortPD_T *entry_p);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetPSECheckStatus
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
BOOL_T SWCTRL_GetPSECheckStatus(BOOL_T* pse_check_status_p);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetPSECheckStatus
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
BOOL_T SWCTRL_SetPSECheckStatus(BOOL_T pse_check_status);

#if (SYS_CPNT_SFLOW == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - SWCTRL_SetSflowPortPacketSamplingRate
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
SWCTRL_SetSflowPortPacketSamplingRate(
    UI32_T ifindex,
    UI32_T rate);
#endif /* #if (SYS_CPNT_SFLOW == TRUE) */

#if (SYS_CPNT_PFC == TRUE)
/* -------------------------------------------------------------------------
 * FUNCTION NAME - SWCTRL_SetPortPfcStatus
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
BOOL_T SWCTRL_SetPortPfcStatus(
    UI32_T  ifidx,
    BOOL_T  rx_en,
    BOOL_T  tx_en,
    UI16_T  pri_en_vec);

/* -------------------------------------------------------------------------
 * FUNCTION NAME - SWCTRL_UpdatePfcPriMap
 * -------------------------------------------------------------------------
 * PURPOSE : This function update PFC priority to queue mapping.
 * INPUT   : None
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTE   : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_UpdatePfcPriMap(void);
#endif /* #if (SYS_CPNT_PFC == TRUE) */

#if (SYS_CPNT_ETS == TRUE)
/*------------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetPortCosGroupMapping
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
BOOL_T SWCTRL_SetPortCosGroupMapping(
    UI32_T ifindex,
    UI32_T cosq2group[SYS_ADPT_MAX_NBR_OF_PRIORITY_QUEUE]);

/*------------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetPortCosGroupSchedulingMethod
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
BOOL_T SWCTRL_SetPortCosGroupSchedulingMethod(
    UI32_T ifindex,
    UI32_T method,
    UI32_T weights[SYS_ADPT_ETS_MAX_NBR_OF_TRAFFIC_CLASS]);
#endif /* (SYS_CPNT_ETS == TRUE) */

#if (SYS_CPNT_CN == TRUE)
/*------------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetQcnCnmPriority
 *------------------------------------------------------------------------------
 * FUNCTION: This function sets 802.1p priority of egress QCN CNM
 * INPUT   : pri -- 802.1p priority of egress QCN CNM
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: failed
 * NOTE    : None
 *------------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetQcnCnmPriority(UI32_T pri);

/*------------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetPortQcnCpq
 *------------------------------------------------------------------------------
 * FUNCTION: This function sets CP Queue of the CoS Queue
 * INPUT   : ifindex
 *           cosq -- CoS Queue
 *           cpq  -- CP Queue. SWCTRL_QCN_CPQ_INVALID means to disable QCN
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: failed
 * NOTE    : None
 *------------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetPortQcnCpq(
    UI32_T ifindex,
    UI32_T cosq,
    UI32_T cpq);

/*------------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetPortQcnEgrCnTagRemoval
 *------------------------------------------------------------------------------
 * FUNCTION: This function sets removal of CN-Tag of egress pkts
 * INPUT   : ifindex
 *           no_cntag_bitmap - bit 0 for pri 0, and so on.
 *                             set the bit to remove CN-tag of packets with the pri.
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: failed
 * NOTE    : None
 *------------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetPortQcnEgrCnTagRemoval(
    UI32_T ifindex,
    UI8_T no_cntag_bitmap);

/*------------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetPortQcnCpid
 *------------------------------------------------------------------------------
 * FUNCTION: This function gets CPID
 * INPUT   : ifindex
 *           cosq -- CoS Queue
 *           cpid -- CPID
 * OUTPUT  : None
 * RETURN  : True: Successfully, FALSE: failed
 * NOTE    : None
 *------------------------------------------------------------------------------*/
BOOL_T SWCTRL_GetPortQcnCpid(
    UI32_T ifindex,
    UI32_T cosq,
    UI8_T cpid[8]);
#endif /* (SYS_CPNT_CN == TRUE) */

#if (SYS_CPNT_MAC_IN_MAC == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetMimService
 * -------------------------------------------------------------------------
 * FUNCTION: This function will create/destroy a MiM service instance.
 * INPUT   : mim_p            -- MiM service instance info.
 *           is_valid         -- TRUE to create/update; FALSE to destroy.
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetMimService(SWCTRL_MimServiceInfo_T *mim_p, BOOL_T is_valid);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetMimServicePort
 * -------------------------------------------------------------------------
 * FUNCTION: This function will add/delete member port to a MiM service instance.
 * INPUT   : mim_port_p       -- MiM port info.
 *           is_valid         -- TRUE to add; FALSE to delete.
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetMimServicePort(SWCTRL_MimPortInfo_T *mim_port_p, BOOL_T is_valid);

#if (SYS_CPNT_IAAS == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - SWCTRL_SetMimServicePortLearningStatusForStationMove
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
BOOL_T SWCTRL_SetMimServicePortLearningStatusForStationMove(BOOL_T learning, BOOL_T to_cpu, BOOL_T drop);
#endif /* (SYS_CPNT_IAAS == TRUE) */
#endif /* (SYS_CPNT_MAC_IN_MAC == TRUE) */

/* -------------------------------------------------------------------------
 * FUNCTION NAME - SWCTRL_SetCpuRateLimit
 * -------------------------------------------------------------------------
 * PURPOSE : To configure CPU rate limit
 * INPUT   : pkt_type  -- SWDRV_PKTTYPE_XXX
 *           rate      -- in pkt/s. 0 to disable.
 * OUTPUT  : None
 * RETURN  : TRUE / FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SWCTRL_SetCpuRateLimit(UI32_T pkt_type, UI32_T rate);

/* -------------------------------------------------------------------------
 * FUNCTION NAME - SWCTRL_GetCpuRateLimit
 * -------------------------------------------------------------------------
 * PURPOSE : To get configured CPU rate limit
 * INPUT   : pkt_type  -- SWDRV_PKTTYPE_XXX
 * OUTPUT  : rate_p    -- in pkt/s. 0 to disable.
 * RETURN  : TRUE / FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SWCTRL_GetCpuRateLimit(UI32_T pkt_type, UI32_T *rate_p);

/* -------------------------------------------------------------------------
 * FUNCTION NAME - SWCTRL_GetPortAbility
 * -------------------------------------------------------------------------
 * PURPOSE : To get port abilities
 * INPUT   : ifindex
 * OUTPUT  : ability_p  -- SWCTRL_PortAbility_T
 * RETURN  : TRUE / FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SWCTRL_GetPortAbility(UI32_T ifindex, SWCTRL_PortAbility_T *ability_p);

/* -------------------------------------------------------------------------
 * FUNCTION NAME - SWCTRL_DropPortCdpPacket
 * -------------------------------------------------------------------------
 * PURPOSE : Drop CDP packet
 * INPUT   : enable  -- enabled/disabled this feature
 *           unit    -- unit
 *           port    -- port
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T
SWCTRL_DropPortCdpPacket(
    BOOL_T enable,
    UI32_T ifindex
);

/* -------------------------------------------------------------------------
 * FUNCTION NAME - SWCTRL_DropPortPvstPacket
 * -------------------------------------------------------------------------
 * PURPOSE : Drop CDP packet
 * INPUT   : enable  -- enabled/disabled this feature
 *           unit    -- unit
 *           port    -- port
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T
SWCTRL_DropPortPvstPacket(
    BOOL_T enable,
    UI32_T ifindex
);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetPortSfpPresent
 * -------------------------------------------------------------------------
 * FUNCTION: Get port sfp present status
 *
 * INPUT   : unit
 *           sfp_index
 * OUTPUT  : is_present_p
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SWCTRL_GetPortSfpPresent(UI32_T unit, UI32_T sfp_index, BOOL_T *is_present_p);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetPortSfpPresent
 * -------------------------------------------------------------------------
 * FUNCTION: Set port sfp eeprom info
 *
 * INPUT   : unit
 *           sfp_index
 *           is_present
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SWCTRL_SetPortSfpPresent(UI32_T unit, UI32_T sfp_index, BOOL_T is_present);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetPortSfpInfo
 * -------------------------------------------------------------------------
 * FUNCTION: Get port sfp eeprom info
 *
 * INPUT   : unit
 *           sfp_index
 * OUTPUT  : sfp_info_p
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SWCTRL_GetPortSfpInfo(UI32_T unit, UI32_T sfp_index, SWCTRL_OM_SfpInfo_T *sfp_info_p);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetPortSfpInfo
 * -------------------------------------------------------------------------
 * FUNCTION: Set port sfp eeprom info
 *
 * INPUT   : unit
 *           sfp_index
 *           sfp_info_p
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SWCTRL_SetPortSfpInfo(UI32_T unit, UI32_T sfp_index, SWCTRL_OM_SfpInfo_T *sfp_info_p);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetPortSfpDdmInfo
 * -------------------------------------------------------------------------
 * FUNCTION: Get port sfp ddm eeprom info
 *
 * INPUT   : unit
 *           sfp_index
 * OUTPUT  : sfp_ddm_info_p
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SWCTRL_GetPortSfpDdmInfo(UI32_T unit, UI32_T sfp_index, SWCTRL_OM_SfpDdmInfo_T *sfp_ddm_info_p);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetPortSfpDdmInfo
 * -------------------------------------------------------------------------
 * FUNCTION: Set port sfp eeprom info
 *
 * INPUT   : unit
 *           sfp_index
 *           sfp_ddm_info_p
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SWCTRL_SetPortSfpDdmInfo(UI32_T unit, UI32_T sfp_index, SWCTRL_OM_SfpDdmInfo_T *sfp_ddm_info_p);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetPortSfpDdmInfoMeasured
 * -------------------------------------------------------------------------
 * FUNCTION: Get port sfp ddm measured eeprom info
 *
 * INPUT   : unit
 *           sfp_index
 * OUTPUT  : sfp_ddm_info_measured_p
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SWCTRL_GetPortSfpDdmInfoMeasured(UI32_T unit, UI32_T sfp_index, SWCTRL_OM_SfpDdmInfoMeasured_T *sfp_ddm_info_measured_p);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetPortSfpDdmInfoMeasured
 * -------------------------------------------------------------------------
 * FUNCTION: Set port sfp eeprom info
 *
 * INPUT   : unit
 *           sfp_index
 *           sfp_ddm_info_measured_p
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SWCTRL_SetPortSfpDdmInfoMeasured(UI32_T unit, UI32_T sfp_index, SWCTRL_OM_SfpDdmInfoMeasured_T *sfp_ddm_info_measured_p);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetPortEgressBlock
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
BOOL_T SWCTRL_SetPortEgressBlock(
    UI32_T lport,
    UI32_T egr_lport,
    BOOL_T is_block);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetPortEgressBlockEx
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
BOOL_T SWCTRL_SetPortEgressBlockEx(
    UI32_T lport,
    UI8_T egr_lport_list[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST],
    UI8_T blk_lport_list[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST]);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_IsPortEgressBlock
 * -------------------------------------------------------------------------
 * FUNCTION: To set egress block ports.
 * INPUT   : lport               - source lport
 *           egr_lport           - egress lport
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE - egress lport is block or not
 * NOTE    : None.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_IsPortEgressBlock(
    UI32_T lport,
    UI32_T egr_lport);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetActiveUportListByPortEgressBlock
 * -------------------------------------------------------------------------
 * FUNCTION: To set egress block ports.
 * INPUT   : lport               - source lport
 *           active_uport_count_per_unit
 *           uport_list
 * OUTPUT  : active_uport_count_per_unit
 *           uport_list
 * RETURN  : TRUE/FALSE - uport_list is modified or not
 * NOTE    : None.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_GetActiveUportListByPortEgressBlock(
    UI32_T lport,
    UI8_T active_uport_count_per_unit[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK],
    UI8_T *uport_list);

#if (SYS_CPNT_HASH_SELECTION == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_BindHashSelForService
 * -------------------------------------------------------------------------
 * FUNCTION: add service reference of hash-selection list
 * INPUT   : service     - SWCTRL_OM_HASH_SEL_SERVICE_ECMP,
 *                         SWCTRL_OM_HASH_SEL_SERVICE_TRUNK
 *           list_index - the index of hash-selection block
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_BindHashSelForService(
    SWCTRL_OM_HashSelService_T service, 
    UI8_T list_index);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_UnBindHashSelForService
 * -------------------------------------------------------------------------
 * FUNCTION: remove service reference of the hash-selection list
 * INPUT   : service     - SWCTRL_OM_HASH_SEL_SERVICE_ECMP,
 *                         SWCTRL_OM_HASH_SEL_SERVICE_TRUNK
 *           list_index - the index of hash-selection block
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_UnBindHashSelForService(
    SWCTRL_OM_HashSelService_T service, 
    UI8_T list_index);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_AddHashSelection
 * -------------------------------------------------------------------------
 * FUNCTION: add hash-selection field
 * INPUT   : list_index - the index of hash-selection list
 *           selection_p - hash-selection field
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : If the hash-selection block has been bound, then it can't be modified
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_AddHashSelection(
    UI8_T list_index , 
    SWCTRL_OM_HashSelection_T *selection_p);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_RemoveHashSelection
 * -------------------------------------------------------------------------
 * FUNCTION: remove hash-selection field
 * INPUT   : list_index - the index of hash-selection list
 *           selection_p - hash-selection field
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : If the hash-selection block has been bound, then it can't be modified
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_RemoveHashSelection(
    UI8_T list_index ,
    SWCTRL_OM_HashSelection_T *selection_p);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetHashBlockInfo
 * -------------------------------------------------------------------------
 * FUNCTION: get hash-selection block info
 * INPUT   : list_index - the index of hash-selection list
 * OUTPUT  : block_info_p - the hash-selection block info
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_GetHashBlockInfo(
    UI8_T list_index, 
    SWCTRL_OM_HashSelBlockInfo_T *block_info_p);
#endif /*#if (SYS_CPNT_HASH_SELECTION == TRUE)*/

#if (SYS_CPNT_SWCTRL_SWITCH_MODE_CONFIGURABLE == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetSwitchingMode
 * -------------------------------------------------------------------------
 * FUNCTION: To set switching mode
 * INPUT   : ifindex  - which port to configure.
 *           mode     - VAL_swctrlSwitchModeSF
 *                      VAL_swctrlSwitchModeCT
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_SetSwitchingMode(UI32_T ifindex, UI32_T mode);
#endif /*#if (SYS_CPNT_SWCTRL_SWITCH_MODE_CONFIGURABLE == TRUE)*/


#if(SYS_CPNT_WRED == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - SWCTRL_PMGR_RandomDetect
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion will set port ecn marking percentage
 * INPUT    :   lport      - which port to set
 *              queue_id   - what queue to confiugre
 *              min        - min threshold
 *              max        - max threshold
 *              drop       - drop probability
 *              ecn        - do ecn marking
 * OUTPUT   :   None
 * RETURN   :   TRUE/FALSE
 * NOTES    :   queue_id = -1 means all queue
 * ------------------------------------------------------------------------
 */
BOOL_T SWCTRL_Random_Detect(UI32_T lport, SWCTRL_RandomDetect_T *random_detect_p);
#endif
#endif /* _SWCTRL_H_ */

