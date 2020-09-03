/* FUNCTION NAME: add_type.h
 * PURPOSE:
 *	1. Name constant define
 * NOTES:
 *
 * REASON:
 *    DESCRIPTION:
 *    CREATOR:	Junying
 *    Date       2006/04/26
 *
 * Copyright(C)      Accton Corporation, 2006
 */

#ifndef ADD_TYPE_H
#define ADD_TYPE_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_adpt.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define ADD_TYPE_TIMER_1SEC_OF_TICKS             100

#define ADD_TYPE_DEBUG_SHORTER_TIMER_EVENT       FALSE
#if (ADD_TYPE_DEBUG_SHORTER_TIMER_EVENT == TRUE)
    #define ADD_TYPE_TIMER_EVENT_OF_SEC          (5)          /* every 5 sec send an ADD timer event  */
#else
    #define ADD_TYPE_TIMER_EVENT_OF_SEC          (20)         /* every 20 sec send an ADD timer event */
#endif

#define ADD_TYPE_VOICE_VLAN_DAY_OF_HOUR          (24)
#define ADD_TYPE_VOICE_VLAN_HOUR_OF_MINUTE       (60)
#define ADD_TYPE_VOICE_VLAN_MINUTE_OF_SECOND     (60)
#define ADD_TYPE_VOICE_VLAN_DAY_OF_MINUTE        (ADD_TYPE_VOICE_VLAN_DAY_OF_HOUR*ADD_TYPE_VOICE_VLAN_HOUR_OF_MINUTE)

#define ADD_TYPE_VOICE_VLAN_TIMER_DISABLED       0xFFFFFFFF

#define ADD_TYPE_VOICE_VLAN_ERROR_NA             (-1)
#define ADD_TYPE_VOICE_VLAN_ERROR_NO_START       (-2)


typedef struct ADD_MessageQueueData_S
{
    UI32_T vid;                             /* source vid                       */
    UI8_T  mac[SYS_ADPT_MAC_ADDR_LEN];      /* source mac address               */
    UI32_T l_port;                          /* which logic port                 */
    UI32_T device_type;                     /* the notify device type           */
    UI32_T protocol;                        /* the notify protocol              */
    UI32_T notify_type;                     /* the notify type                  */
}ADD_MessageQueueData_T;

typedef struct  ADD_MessageQueue_S
{
    ADD_MessageQueueData_T *m_data;         /* message context                  */
    UI32_T m_reserved[3];                   /* not defined for future extension */
}ADD_MessageQueue_T;

enum ADD_EVENT_MASK_E
{
    ADD_TYPE_EVENT_NONE             = 0x0000L,
    ADD_TYPE_EVENT_TIMER            = 0x0001L,
    ADD_TYPE_EVENT_NEW_MAC          = 0x0002L,
    ADD_TYPE_EVENT_LPORT_LINKDOWN   = 0x0004L,
    ADD_TYPE_EVENT_ENTER_TRANSITION = 0x0008L,
    ADD_TYPE_EVENT_ALL              = 0xFFFFL
};

#define ADD_TYPE_DISCOVERY_PROTOCOL_NONE  0x00
#define ADD_TYPE_DISCOVERY_PROTOCOL_OUI   0x01
#define ADD_TYPE_DISCOVERY_PROTOCOL_LLDP  0x02
#define ADD_TYPE_DISCOVERY_PROTOCOL_DHCP  0x04
#define ADD_TYPE_DISCOVERY_PROTOCOL_ALL   (ADD_TYPE_DISCOVERY_PROTOCOL_OUI|ADD_TYPE_DISCOVERY_PROTOCOL_LLDP|ADD_TYPE_DISCOVERY_PROTOCOL_DHCP)

enum ADD_VoiceVlanDeviceType_E
{
    ADD_TYPE_VOICE_VLAN_DEVICE_TYPE_REPEATE,
    ADD_TYPE_VOICE_VLAN_DEVICE_TYPE_BRIDGE,
    ADD_TYPE_VOICE_VLAN_DEVICE_TYPE_WLAN_ACCESS_POINT,
    ADD_TYPE_VOICE_VLAN_DEVICE_TYPE_ROUTER,
    ADD_TYPE_VOICE_VLAN_DEVICE_TYPE_TELEPHONE,
    ADD_TYPE_VOICE_VLAN_DEVICE_TYPE_DOCSIS_CABLE_DEVICE,
    ADD_TYPE_VOICE_VLAN_DEVICE_TYPE_STATION_ONLY,
    ADD_TYPE_VOICE_VLAN_DEVICE_TYPE_OTHERS,
};

#endif /* End fo ADD_TYPE_H */
