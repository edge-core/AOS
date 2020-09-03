/*-----------------------------------------------------------------------------
 * Module Name: xstp_om_private.h
 *-----------------------------------------------------------------------------
 * PURPOSE: The declarations of XSTP OM which are only used by XSTP.
 *-----------------------------------------------------------------------------
 * NOTES:
 *
 *-----------------------------------------------------------------------------
 * HISTORY:
 *    05/30/2001 - Allen Cheng, Created
 *    06/12/2002 - Kelly Chen, Added
 *    02-09-2004 - Kelly Chen, Revise the implementations of 802.1w/1s according to the IEEE 802.1D/D3
 *    05/22/2007 - Timon Chang, Moved privately used resources from xstp_om.h to this file
 *-----------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2002
 *-----------------------------------------------------------------------------
 */

#ifndef XSTP_OM_PRIVATE_H
#define XSTP_OM_PRIVATE_H


/* INCLUDE FILE DECLARATIONS
 */

#include "sys_type.h"
#include "xstp_type.h"


/* MACRO FUNCTION DECLARATIONS
 */

/*-------------------------------------------------------------------------
 * MACRO NAME   - XSTP_OM_BRIDGE_ID_FORMAT
 * ------------------------------------------------------------------------
 * PURPOSE      : Create a bridge_id in the XSTP_TYPE_BridgeId_T format
 * INPUT        : UI16_T    _priority;      -- [0 - 61440] in steps of 4096
 *                UI16_T    _system_id_ext; -- [0 - 4095]
 *                UI8_T     _mac_addr;      -- addr[6]
 * OUTPUT       : XSTP_TYPE_BridgeId_T      _bridge_id;
 * NOTES        : None
 * ------------------------------------------------------------------------
 */
#define XSTP_OM_BRIDGE_ID_FORMAT(__bridge_id, __priority, __system_id, __mac_addr)                   \
{                                                                       \
    (__bridge_id).bridge_id_priority.bridge_priority  = (((UI16_T)(__priority)) & 0xF000);           \
    (__bridge_id).bridge_id_priority.bridge_priority |= (((UI16_T)(__system_id)) & 0x0FFF);          \
    memcpy(__bridge_id.addr, __mac_addr, 6);                                                         \
}

/*-------------------------------------------------------------------------
 * MACRO NAME   - XSTP_OM_PORT_ID_FORMAT
 * ------------------------------------------------------------------------
 * PURPOSE      : Create a port_id in the XSTP_TYPE_PortId_T format
 * INPUT        : UI8_T     _priority;      -- [0 - 240] in steps of 16
 *                UI16_T    _port_num;      -- [0 - 4095]
 * OUTPUT       : XSTP_TYPE_PortId_T        _portid;
 * NOTES        : None
 * ------------------------------------------------------------------------
 */
#define XSTP_OM_PORT_ID_FORMAT(__port_id, __priority, __port_num)           \
{                                                                       \
    (__port_id).port_id        =  ( ((UI16_T)(__priority)<<8 ) & 0xF000);   \
    (__port_id).port_id        |= (  (UI16_T)(__port_num)      & 0x0FFF);   \
}

/*-------------------------------------------------------------------------
 * MACRO NAME   - XSTP_OM_GET_BRIDGE_ID_PRIORITY
 * ------------------------------------------------------------------------
 * PURPOSE      : Get "priority" bit field in the XSTP_TYPE_BridgeId_T format
 * INPUT        : XSTP_TYPE_BridgeId_T      __bridge_id;
 * OUTPUT       : UI16_T                    __priority;
 * NOTES        : The four most significant bits of the most significant byte of bridge_id.
 * ------------------------------------------------------------------------
 */
#define XSTP_OM_GET_BRIDGE_ID_PRIORITY(__priority, __bridge_id)                                    \
{                                                                                                  \
    (__priority)  = ((UI16_T)((__bridge_id).bridge_id_priority.bridge_priority) & 0xF000);         \
}

/*-------------------------------------------------------------------------
 * MACRO NAME   - XSTP_OM_GET_BRIDGE_ID_SYSIDEXT
 * ------------------------------------------------------------------------
 * PURPOSE      : Get "system_id_ext" bit field in the XSTP_TYPE_BridgeId_T format
 * INPUT        : XSTP_TYPE_BridgeId_T      __bridge_id;
 * OUTPUT       : UI16_T                    __system_id_ext;
 * NOTES        : The four least significant bits of the most significant byte
 *                + the second modt significant byte of bridge_id.
 * ------------------------------------------------------------------------
 */
#define XSTP_OM_GET_BRIDGE_ID_SYSIDEXT(__system_id_ext, __bridge_id)                                      \
{                                                                                                         \
    (__system_id_ext)  = ((UI16_T)((__bridge_id).bridge_id_priority.bridge_priority) & 0x0FFF);           \
}

/*-------------------------------------------------------------------------
 * MACRO NAME   - XSTP_OM_GET_PORT_ID_PRIORITY
 * ------------------------------------------------------------------------
 * PURPOSE      : Get "priority" bit field in the XSTP_TYPE_PortId_T format
 * INPUT        : XSTP_TYPE_PortId_T      __port_id;
 * OUTPUT       : UI8_T                   __priority;
 * NOTES        : The four most significant bits of the most significant byte of port_id.
 * ------------------------------------------------------------------------
 */
#define XSTP_OM_GET_PORT_ID_PRIORITY(__priority, __port_id)                       \
{                                                                                 \
    (__priority)  = (UI8_T)(((UI16_T)((__port_id).port_id) & 0xF000) >> 8);       \
}

/*-------------------------------------------------------------------------
 * MACRO NAME   - XSTP_OM_GET_PORT_ID_PORTNUM
 * ------------------------------------------------------------------------
 * PURPOSE      : Get "port_num" bit field in the XSTP_TYPE_PortId_T format
 * INPUT        : XSTP_TYPE_PortId_T      __port_id;
 * OUTPUT       : UI16_T                  __port_num;
 * NOTES        : The four least significant bits of the most significant byte
 *                + the second significant byte of port_id.
 * ------------------------------------------------------------------------
 */
#define XSTP_OM_GET_PORT_ID_PORTNUM(__port_num, __port_id)                        \
{                                                                                 \
    (__port_num)  = ((UI16_T)((__port_id).port_id) & 0x0FFF);                     \
}

/*-------------------------------------------------------------------------
 * MACRO NAME   - XSTP_OM_SET_BRIDGE_ID_PRIORITY
 * ------------------------------------------------------------------------
 * PURPOSE      : Set "priority" bit field in the XSTP_TYPE_BridgeId_T format
 * INPUT        : XSTP_TYPE_BridgeId_T      __bridge_id;
 *                UI16_T                    __priority;
 * OUTPUT       : None
 * NOTES        : The four most significant bits of the most significant byte of bridge_id.
 * ------------------------------------------------------------------------
 */
#define XSTP_OM_SET_BRIDGE_ID_PRIORITY(__bridge_id, __priority)                        \
{                                                                                      \
    (__bridge_id).bridge_id_priority.bridge_priority  &= 0x0FFF;                       \
    (__bridge_id).bridge_id_priority.bridge_priority  |= (0xF000 & (__priority) );     \
}

/*-------------------------------------------------------------------------
 * MACRO NAME   - XSTP_OM_SET_BRIDGE_ID_SYSIDEXT
 * ------------------------------------------------------------------------
 * PURPOSE      : Set "system_id_ext" bit field in the XSTP_TYPE_BridgeId_T format
 * INPUT        : XSTP_TYPE_BridgeId_T      __bridge_id;
 *                UI16_T                    __system_id_ext;
 * OUTPUT       : None
 * NOTES        : The four least significant bits of the most significant byte
 *                + the second modt significant byte of bridge_id.
 * ------------------------------------------------------------------------
 */
#define XSTP_OM_SET_BRIDGE_ID_SYSIDEXT(__bridge_id, __system_id_ext)                       \
{                                                                                          \
    (__bridge_id).bridge_id_priority.bridge_priority  &= 0xF000;                           \
    (__bridge_id).bridge_id_priority.bridge_priority  |= (0x0FFF & (__system_id_ext) );    \
}

/*-------------------------------------------------------------------------
 * MACRO NAME   - XSTP_OM_SET_PORT_ID_PRIORITY
 * ------------------------------------------------------------------------
 * PURPOSE      : Set "priority" bit field in the XSTP_TYPE_PortId_T format
 * INPUT        : XSTP_TYPE_PortId_T      __port_id;
 *                UI8_T                   __priority;
 * OUTPUT       : None
 * NOTES        : The four most significant bits of the most significant byte of port_id.
 * ------------------------------------------------------------------------
 */
#define XSTP_OM_SET_PORT_ID_PRIORITY(__port_id, __priority)                       \
{                                                                                 \
    (__port_id).port_id &= 0x0FFF;                                                \
    (__port_id).port_id |= (0xF000 & (((UI16_T)(__priority)) << 8));              \
}

/*-------------------------------------------------------------------------
 * MACRO NAME   - XSTP_OM_SET_PORT_ID_PORTNUM
 * ------------------------------------------------------------------------
 * PURPOSE      : Set "port_num" bit field in the XSTP_TYPE_PortId_T format
 * INPUT        : XSTP_TYPE_PortId_T      __port_id;
 *                UI16_T                  __port_num;
 * OUTPUT       : None
 * NOTES        : The four least significant bits of the most significant byte
 *                + the second significant byte of port_id.
 * ------------------------------------------------------------------------
 */
#define XSTP_OM_SET_PORT_ID_PORTNUM(__port_id, __port_num)                        \
{                                                                                 \
    (__port_id).port_id &= 0xF000;                                                \
    (__port_id).port_id |= (0x0FFF & (__port_num));                               \
}

/*-------------------------------------------------------------------------
 * MACRO NAME   - XSTP_OM_CMP_BRIDGE_ID
 * ------------------------------------------------------------------------
 * PURPOSE      : The comparison function for bridge_id.
 * INPUT        : XSTP_TYPE_BridgeId_T __bridge_id_1;
 *                XSTP_TYPE_BridgeId_T __bridge_id_2;
 * OUTPUT       : __result(integer);
 * NOTES        : None
 * ------------------------------------------------------------------------
 */
#define XSTP_OM_CMP_BRIDGE_ID(__result, __bridge_id_1, __bridge_id_2)                                                            \
{                                                                                                                                \
    /* 1. Compare bridge_priority */                                                                                             \
    (__result)=(((__bridge_id_1).bridge_id_priority.bridge_priority) - ((__bridge_id_2).bridge_id_priority.bridge_priority));    \
    if ((__result) == 0)                                                                                                         \
    {                                                                                                                            \
        /* 2. Compare addr */                                                                                                    \
        (__result)  = memcmp((__bridge_id_1).addr, (__bridge_id_2).addr, 6);                                                     \
    }                                                                                                                            \
}

/*-------------------------------------------------------------------------
 * MACRO NAME   - XSTP_OM_CMP_PORT_ID
 * ------------------------------------------------------------------------
 * PURPOSE      : The comparison function for port_id.
 * INPUT        : XSTP_TYPE_PortId_T __port_id_1;
 *                XSTP_TYPE_PortId_T __port_id_2;
 * OUTPUT       : __result(integer);
 * NOTES        : None
 * ------------------------------------------------------------------------
 */
#define XSTP_OM_CMP_PORT_ID(__result, __port_id_1, __port_id_2)        \
{                                                                      \
    /* Compare port_id */                                              \
    (__result)=(((__port_id_1).port_id) - ((__port_id_2).port_id));    \
}

/*-------------------------------------------------------------------------
 * MACRO NAME   - XSTP_OM_CMP_MST_CONFIG_ID
 * ------------------------------------------------------------------------
 * PURPOSE      : The comparison function for mst_config_id
 * INPUT        : XSTP_TYPE_MstConfigId_T __mst_config_id_1;
 *                XSTP_TYPE_MstConfigId_T __mst_config_id_2;
 * OUTPUT       : __result(integer);
 * NOTES        : None
 * ------------------------------------------------------------------------
 */
#define XSTP_OM_CMP_MST_CONFIG_ID(__result, __mst_config_id_1, __mst_config_id_2)                                       \
{                                                                                                                       \
    /* 1. Compare config_id_format_selector */                                                                          \
    (__result) = (((__mst_config_id_1).config_id_format_selector) - ((__mst_config_id_2).config_id_format_selector));   \
    if ((__result) == 0)                                                                                                \
    {                                                                                                                   \
        /* 2. Compare config_name */                                                                                    \
        (__result) = memcmp((__mst_config_id_1).config_name,                                                            \
                            (__mst_config_id_2).config_name,                                                            \
                            XSTP_TYPE_REGION_NAME_MAX_LENGTH);                                                          \
        if ((__result) == 0)                                                                                            \
        {                                                                                                               \
            /* 3. Compare revision_level */                                                                             \
            (__result) = (((__mst_config_id_1).revision_level) - ((__mst_config_id_2).revision_level));                 \
            if ((__result) == 0)                                                                                        \
            {                                                                                                           \
                /* 4. Compare config_digest */                                                                          \
                (__result) = memcmp((__mst_config_id_1).config_digest,                                                  \
                                    (__mst_config_id_2).config_digest,                                                  \
                                    16);                                                                                \
            }                                                                                                           \
        }                                                                                                               \
    }                                                                                                                   \
}

/*-------------------------------------------------------------------------
 * MACRO NAME   - XSTP_OM_CPY_HTON_BRIDGE_ID
 * ------------------------------------------------------------------------
 * PURPOSE      : The copy function for bridge_id.
 *                Change host byte order to network byte order.
 * INPUT        : XSTP_TYPE_BridgeId_T (__dest_bridge_id);
 *                XSTP_TYPE_BridgeId_T (__src_bridge_id);
 * OUTPUT       : None
 * NOTES        : None
 * ------------------------------------------------------------------------
 */
#define XSTP_OM_CPY_HTON_BRIDGE_ID(__dest_bridge_id, __src_bridge_id)                                                                \
{                                                                                                                                    \
    (__dest_bridge_id).bridge_id_priority.bridge_priority = L_STDLIB_Hton16((__src_bridge_id).bridge_id_priority.bridge_priority);   \
    memcpy( (UI8_T *)&(__dest_bridge_id.addr),                                                                                       \
            (UI8_T *)&(__src_bridge_id.addr),                                                                                        \
            6                                                                                                                        \
          );                                                                                                                         \
}

/*-------------------------------------------------------------------------
 * MACRO NAME   - XSTP_OM_CPY_NTOH_BRIDGE_ID
 * ------------------------------------------------------------------------
 * PURPOSE      : The copy function for bridge_id.
 *                Change network byte order to host byte order .
 * INPUT        : XSTP_TYPE_BridgeId_T (__dest_bridge_id);
 *                XSTP_TYPE_BridgeId_T (__src_bridge_id);
 * OUTPUT       : None
 * NOTES        : None
 * ------------------------------------------------------------------------
 */
#define XSTP_OM_CPY_NTOH_BRIDGE_ID(__dest_bridge_id, __src_bridge_id)                                                                \
{                                                                                                                                    \
    (__dest_bridge_id).bridge_id_priority.bridge_priority = L_STDLIB_Ntoh16((__src_bridge_id).bridge_id_priority.bridge_priority);   \
    memcpy( (UI8_T *)&(__dest_bridge_id.addr),                                                                                       \
            (UI8_T *)&(__src_bridge_id.addr),                                                                                        \
            6                                                                                                                        \
          );                                                                                                                         \
}

/* ===================================================================== */
/* ===================================================================== */
/* IEEE 802.1W */
#ifdef XSTP_TYPE_PROTOCOL_RSTP

/*-------------------------------------------------------------------------
 * MACRO NAME   - XSTP_TYPE_PriorityVector_T
 * ------------------------------------------------------------------------
 * PURPOSE      : Create a priority vector in the XSTP_TYPE_PriorityVector_T format
 * INPUT        : XSTP_TYPE_BridgeId_T      _root_bridge_id;
 *                UI32_T                    _root_path_cost;
 *                XSTP_TYPE_BridgeId_T      _designated_bridge_id;
 *                XSTP_TYPE_PortId_T        _designated_port_id;
 *                XSTP_TYPE_PortId_T        _bridge_port_id;
 * OUTPUT       : XSTP_TYPE_PriorityVector_T        _vector;
 * NOTES        : None
 * ------------------------------------------------------------------------
 */
#define XSTP_OM_PRIORITY_VECTOR_FORMAT(_vector, _root_bridge_id, _root_path_cost, _designated_bridge_id, _designated_port_id, _bridge_port_id)    \
{                                                                           \
    memcpy( &((_vector).root_bridge_id),                                    \
            &(_root_bridge_id),                                             \
            sizeof(XSTP_TYPE_BridgeId_T)                                    \
          );                                                                \
    (_vector).root_path_cost              = (_root_path_cost);              \
    memcpy( &((_vector).designated_bridge_id),                              \
            &(_designated_bridge_id),                                       \
            sizeof(XSTP_TYPE_BridgeId_T)                                    \
          );                                                                \
    (_vector).designated_port_id.port_id  = (_designated_port_id).port_id;  \
    (_vector).bridge_port_id.port_id      = (_bridge_port_id).port_id;      \
}

/*-------------------------------------------------------------------------
 * MACRO NAME   - XSTP_OM_TIMERS_FORMAT
 * ------------------------------------------------------------------------
 * PURPOSE      : Create a set of timers in the XSTP_TYPE_Timers_T format
 * INPUT        : UI16_T    _message_age;
 *                UI16_T    _max_age;
 *                UI16_T    _hello_time;
 *                UI16_T    _forward_delay;
 * OUTPUT       : XSTP_TYPE_Timers_T        _timers_ptr;
 * NOTES        : None
 * ------------------------------------------------------------------------
 */
#define XSTP_OM_TIMERS_FORMAT(__timers, __message_age, __max_age, __hello_time, __forward_delay)  \
{                                                                       \
    (__timers).message_age     = (UI16_T)(__message_age);                   \
    (__timers).max_age         = (UI16_T)(__max_age);                       \
    (__timers).hello_time      = (UI16_T)(__hello_time);                    \
    (__timers).forward_delay   = (UI16_T)(__forward_delay);                 \
}

/*-------------------------------------------------------------------------
 * MACRO NAME   - XSTP_OM_CMP_PRIORITY_VECTOR
 * ------------------------------------------------------------------------
 * PURPOSE      : The comparison function for priority_vector
 * INPUT        : XSTP_TYPE_PriorityVector_T __priority_vector_1;
 *                XSTP_TYPE_PriorityVector_T __priority_vector_2;
 * OUTPUT       : __result(integer);
 * NOTES        : None
 * ------------------------------------------------------------------------
 */
#define XSTP_OM_CMP_PRIORITY_VECTOR(__result, __priority_vector_1, __priority_vector_2)                                 \
{                                                                                                                       \
    /* 1. Compare root_bridge_id */                                                                                     \
    XSTP_OM_CMP_BRIDGE_ID( (__result), (__priority_vector_1).root_bridge_id, (__priority_vector_2).root_bridge_id);     \
    if ((__result) == 0)                                                                                                \
    {                                                                                                                   \
        /* 2. Compare root_path_cost */                                                                                 \
        (__result) = (((__priority_vector_1).root_path_cost) - ((__priority_vector_2).root_path_cost));                 \
        if ((__result) == 0)                                                                                            \
        {                                                                                                               \
            /* 3. Compare designated_bridge_id */                                                                       \
            XSTP_OM_CMP_BRIDGE_ID( (__result), (__priority_vector_1).designated_bridge_id,                              \
                                                 (__priority_vector_2).designated_bridge_id);                           \
            if ((__result) == 0)                                                                                        \
            {                                                                                                           \
                /* 4. Compare designated_port_id */                                                                     \
                XSTP_OM_CMP_PORT_ID( (__result), (__priority_vector_1).designated_port_id,                              \
                                                   (__priority_vector_2).designated_port_id);                           \
                if ((__result) == 0)                                                                                    \
                {                                                                                                       \
                    /* 5. Compare bridge_port_id */                                                                     \
                    XSTP_OM_CMP_PORT_ID( (__result), (__priority_vector_1).bridge_port_id,                              \
                                                       (__priority_vector_2).bridge_port_id);                           \
                }                                                                                                       \
            }                                                                                                           \
        }                                                                                                               \
    }                                                                                                                   \
}

/*-------------------------------------------------------------------------
 * MACRO NAME   - XSTP_OM_CMP_TIMERS
 * ------------------------------------------------------------------------
 * PURPOSE      : The comparison function for priority_vector
 * INPUT        : XSTP_TYPE_Timers_T __timers_1;
 *                XSTP_TYPE_Timers_T __timers_2;
 * OUTPUT       : __result(integer);
 * NOTES        : None
 * ------------------------------------------------------------------------
 */
#define XSTP_OM_CMP_TIMERS(__result, __timers_1, __timers_2)                                    \
{                                                                                               \
    /* 1. Compare message_age */                                                                \
    (__result) = (((__timers_1).message_age) - ((__timers_2).message_age));                     \
    if ((__result) == 0)                                                                        \
    {                                                                                           \
        /* 2. Compare max_age */                                                                \
        (__result) = (((__timers_1).max_age) - ((__timers_2).max_age));                         \
        if ((__result) == 0)                                                                    \
        {                                                                                       \
            /* 3. Compare hello_time */                                                         \
            (__result) = (((__timers_1).hello_time) - ((__timers_2).hello_time));               \
            if ((__result) == 0)                                                                \
            {                                                                                   \
                /* 4. Compare forward_delay */                                                  \
                (__result) = (((__timers_1).forward_delay) - ((__timers_2).forward_delay));     \
            }                                                                                   \
        }                                                                                       \
    }                                                                                           \
}
#endif /* XSTP_TYPE_PROTOCOL_RSTP */


/* ===================================================================== */
/* ===================================================================== */
/* IEEE 802.1S */
#ifdef XSTP_TYPE_PROTOCOL_MSTP

/*-------------------------------------------------------------------------
 * MACRO NAME   - XSTP_TYPE_PriorityVector_T
 * ------------------------------------------------------------------------
 * PURPOSE      : Create a priority vector in the XSTP_TYPE_PriorityVector_T format
 * INPUT        : XSTP_TYPE_BridgeId_T      _root_id;
 *                UI32_T                    _ext_root_path_cost;
 *                XSTP_TYPE_BridgeId_T      _r_root_id;
 *                UI32_T                    _int_root_path_cost;
 *                XSTP_TYPE_BridgeId_T      _designated_bridge_id;
 *                XSTP_TYPE_PortId_T        _designated_port_id;
 *                XSTP_TYPE_PortId_T        _rcv_port_id;
 * OUTPUT       : XSTP_TYPE_PriorityVector_T        _vector;
 * NOTES        : None
 * ------------------------------------------------------------------------
 */
#define XSTP_OM_PRIORITY_VECTOR_FORMAT(_vector, _root_id, _ext_root_path_cost, _r_root_id, _int_root_path_cost, _designated_bridge_id, _designated_port_id, _rcv_port_id)    \
{                                                                                       \
    memcpy( &((_vector).root_id),                                                       \
            &(_root_id),                                                                \
            sizeof(XSTP_TYPE_BridgeId_T)                                                \
          );                                                                            \
    (_vector).ext_root_path_cost              = (_ext_root_path_cost);                  \
    memcpy( &((_vector).r_root_id),                                                     \
            &(_r_root_id),                                                              \
            sizeof(XSTP_TYPE_BridgeId_T)                                                \
          );                                                                            \
    (_vector).int_root_path_cost              = (_int_root_path_cost);                  \
    memcpy( &((_vector).designated_bridge_id),                                          \
            &(_designated_bridge_id),                                                   \
            sizeof(XSTP_TYPE_BridgeId_T)                                                \
          );                                                                            \
    (_vector).designated_port_id.port_id      = (_designated_port_id).port_id;          \
    (_vector).rcv_port_id.port_id             = (_rcv_port_id).port_id;                 \
}

/*-------------------------------------------------------------------------
 * MACRO NAME   - XSTP_OM_TIMERS_FORMAT
 * ------------------------------------------------------------------------
 * PURPOSE      : Create a set of timers in the XSTP_TYPE_Timers_T format
 * INPUT        : UI16_T    _message_age;
 *                UI16_T    _max_age;
 *                UI16_T    _hello_time;
 *                UI16_T    _forward_delay;
 *                UI8_T     _remaining_hops;
 * OUTPUT       : XSTP_TYPE_Timers_T        _timers_ptr;
 * NOTES        : None
 * ------------------------------------------------------------------------
 */
#define XSTP_OM_TIMERS_FORMAT(_timers, _message_age, _max_age, _hello_time, _forward_delay, _remaining_hops)  \
{                                                                         \
    (_timers).message_age     = (UI16_T)(_message_age);                   \
    (_timers).max_age         = (UI16_T)(_max_age);                       \
    (_timers).hello_time      = (UI16_T)(_hello_time);                    \
    (_timers).forward_delay   = (UI16_T)(_forward_delay);                 \
    (_timers).remaining_hops  = (UI8_T) (_remaining_hops);                \
}

/*-------------------------------------------------------------------------
 * MACRO NAME   - XSTP_OM_CMP_PRIORITY_VECTOR
 * ------------------------------------------------------------------------
 * PURPOSE      : The comparison function for priority_vector
 * INPUT        : XSTP_TYPE_PriorityVector_T __priority_vector_1;
 *                XSTP_TYPE_PriorityVector_T __priority_vector_2;
 * OUTPUT       : __result(integer);
 * NOTES        : None
 * ------------------------------------------------------------------------
 */
#define XSTP_OM_CMP_PRIORITY_VECTOR(__result, __priority_vector_1, __priority_vector_2)                                 \
{                                                                                                                       \
    /* 1. Compare root_id */                                                                                            \
    XSTP_OM_CMP_BRIDGE_ID( (__result), (__priority_vector_1).root_id, (__priority_vector_2).root_id);                   \
    if ((__result) == 0)                                                                                                \
    {                                                                                                                   \
        /* 2. Compare ext_root_path_cost */                                                                             \
        (__result) = (((__priority_vector_1).ext_root_path_cost) - ((__priority_vector_2).ext_root_path_cost));         \
        if ((__result) == 0)                                                                                            \
        {                                                                                                               \
            /* 3. Compare r_root_id */                                                                                  \
            XSTP_OM_CMP_BRIDGE_ID( (__result), (__priority_vector_1).r_root_id,                                         \
                                                 (__priority_vector_2).r_root_id);                                      \
            if ((__result) == 0)                                                                                        \
            {                                                                                                           \
                /* 4. Compare int_root_path_cost */                                                                     \
                (__result) = (((__priority_vector_1).int_root_path_cost) - ((__priority_vector_2).int_root_path_cost)); \
                if ((__result) == 0)                                                                                    \
                {                                                                                                       \
                    /* 5. Compare designated_bridge_id */                                                               \
                    XSTP_OM_CMP_BRIDGE_ID( (__result), (__priority_vector_1).designated_bridge_id,                      \
                                                         (__priority_vector_2).designated_bridge_id);                   \
                    if ((__result) == 0)                                                                                \
                    {                                                                                                   \
                        /* 6. Compare designated_port_id */                                                             \
                        XSTP_OM_CMP_PORT_ID( (__result), (__priority_vector_1).designated_port_id,                      \
                                                           (__priority_vector_2).designated_port_id);                   \
                        if ((__result) == 0)                                                                            \
                        {                                                                                               \
                            /* 7. Compare rcv_port_id */                                                                \
                            XSTP_OM_CMP_PORT_ID( (__result), (__priority_vector_1).rcv_port_id,                         \
                                                               (__priority_vector_2).rcv_port_id);                      \
                        }                                                                                               \
                    }                                                                                                   \
                }                                                                                                       \
            }                                                                                                           \
        }                                                                                                               \
    }                                                                                                                   \
}

/*-------------------------------------------------------------------------
 * MACRO NAME   - XSTP_OM_CMP_TIMERS
 * ------------------------------------------------------------------------
 * PURPOSE      : The comparison function for priority_vector
 * INPUT        : XSTP_TYPE_Timers_T __timers_1;
 *                XSTP_TYPE_Timers_T __timers_2;
 * OUTPUT       : __result(integer);
 * NOTES        : None
 * ------------------------------------------------------------------------
 */
#define XSTP_OM_CMP_TIMERS(__result, __timers_1, __timers_2)                                    \
{                                                                                                   \
    /* 1. Compare message_age */                                                                    \
    (__result) = (((__timers_1).message_age) - ((__timers_2).message_age));                         \
    if ((__result) == 0)                                                                            \
{                                                                       \
        /* 2. Compare max_age */                                                                    \
        (__result) = (((__timers_1).max_age) - ((__timers_2).max_age));                             \
        if ((__result) == 0)                                                                        \
        {                                                                                           \
            /* 3. Compare hello_time */                                                             \
            (__result) = (((__timers_1).hello_time) - ((__timers_2).hello_time));                   \
            if ((__result) == 0)                                                                    \
            {                                                                                       \
                /* 4. Compare forward_delay */                                                      \
                (__result) = (((__timers_1).forward_delay) - ((__timers_2).forward_delay));         \
                if ((__result) == 0)                                                                \
                {                                                                                   \
                    /* 5. Compare remaining_hops */                                                 \
                    (__result) = (((__timers_1).remaining_hops) - ((__timers_2).remaining_hops));   \
                }                                                                                   \
            }                                                                                       \
        }                                                                                           \
    }                                                                                               \
}
#endif /* XSTP_TYPE_PROTOCOL_MSTP */


/* DATA TYPE DECLARATIONS
 */


typedef struct
{
    /* Common */
    BOOL_T                          begin;                  /* 17.17.1 */

    /* State machine performance parameters */
    UI16_T                          tx_hold_count;          /* 17.16.6 */
} XSTP_OM_RstBridgeCommonVar_T;

typedef struct
{
    /* Common */
    UI16_T                          edge_delay_while;       /* 802.1D-2004 17.17.1 */
    BOOL_T                          init_pm;                /* 17.18.7 */
    BOOL_T                          mcheck;                 /* 17.18.10 */
    BOOL_T                          oper_edge;              /* 17.18.14 */
    BOOL_T                          port_enabled;           /* 17.18.15 */
    BOOL_T                          rcvd_bpdu;              /* 17.18.21 */
    BOOL_T                          send_rstp;              /* 17.18.33 */
    BOOL_T                          tick;                   /* 17.18.39 */

    XSTP_TYPE_Bpdu_T                *bpdu;
    BOOL_T                          auto_edge;              /* 802.1D-2004 17.20.2 */
    BOOL_T                          admin_edge;             /* 17.18.14 */
    BOOL_T                          admin_point_to_point_mac;
    BOOL_T                          admin_point_to_point_mac_auto;
    BOOL_T                          oper_point_to_point_mac;
    BOOL_T                          link_up;
    UI32_T                          port_spanning_tree_status;  /* per port enable/disable */
    BOOL_T                          loopback_block;             /* TRUE if attaching to the loopback cable */

    L_MM_Mref_Handle_T              *mref_handle_p;

#if (SYS_CPNT_STP_ROOT_GUARD == TRUE)
    BOOL_T                          root_guard_status;          /* per port enable/disable */
    BOOL_T                          root_inconsistent;
#endif /* #if (SYS_CPNT_STP_ROOT_GUARD == TRUE) */
#if (SYS_CPNT_STP_BPDU_GUARD == TRUE)
    BOOL_T                          bpdu_guard_status;
    UI32_T                          bpdu_guard_auto_recovery;
    UI32_T                          bpdu_guard_auto_recovery_interval;
    UI32_T                          bpdu_guard_auto_recovery_while;
#endif
#if (SYS_CPNT_STP_BPDU_FILTER == TRUE)
    BOOL_T                          bpdu_filter_status;
#endif
#if (SYS_CPNT_XSTP_TC_PROP_STOP == TRUE)
    BOOL_T                          tc_prop_stop;
#endif

#if(SYS_CPNT_XSTP_TC_PROP_GROUP == TRUE)
    UI32_T                         tc_prop_group_id;                 
#endif /*#if(SYS_CPNT_XSTP_TC_PROP_GROUP == TRUE)*/
} XSTP_OM_RstPortCommonVar_T;

typedef struct
{
    /* Common */
    XSTP_OM_RstBridgeCommonVar_T    *common;

    UI32_T                          time_since_topology_change;
    UI32_T                          topology_change_count;
    BOOL_T                          trap_flag_tc;           /* trap flag indicating topology change */
    BOOL_T                          trap_flag_new_root;     /* trap flag indicating new root */

    /* State machine status */
    UI8_T                           sms_prs;                /* State machine status for PRS */

    /* State machine performance parameters */
    UI16_T                          migrate_time;           /* 17.16.4 */

    /* Per-Bridge Variables */                              /* 17.17 */
    XSTP_TYPE_BridgeId_T            bridge_identifier;      /* 17.17.2 */
    XSTP_TYPE_PriorityVector_T      bridge_priority;        /* 17.17.3 */
    XSTP_TYPE_Timers_T              bridge_times;           /* 17.17.4 */ /* 17.16.2 */ /* 17.16.3 */
    XSTP_TYPE_PortId_T              root_port_id;           /* 17.17.5 */
    XSTP_TYPE_PriorityVector_T      root_priority;          /* 17.17.6 */
    XSTP_TYPE_Timers_T              root_times;             /* 17.17.7 */

    BOOL_T                          static_bridge_priority;
    UI32_T                          admin_bridge_priority;
} XSTP_OM_RstBridgeVar_T;

typedef struct
{
    /* Common */
    XSTP_OM_RstPortCommonVar_T      *common;

    BOOL_T                          is_member;              /* Visible or not in this spanning tree instance */
    BOOL_T                          static_path_cost;
    UI16_T                          port_forward_transitions;

    UI32_T                          parent_index;           /*if the port is trunk member, the value is its parent's index*/

    /* State machine performance parameters */
    UI32_T                          port_path_cost;         /* 17.16.5 */

    /* State machine status */
    UI8_T                           sms_pti;                /* State machine status for PTI */
    UI8_T                           sms_pim;                /* State machine status for PIM */
    UI8_T                           sms_prt;                /* State machine status for PRT */
    UI8_T                           sms_pst;                /* State machine status for PST */
    UI8_T                           sms_tcm;                /* State machine status for TCM */
    UI8_T                           sms_ppm;                /* State machine status for PPM */
    UI8_T                           sms_ptx;                /* State machine status for PTX */
    UI8_T                           sms_bdm;                /* State machine status for BDM 802.1D-2004 17.25 */

    /* State machine timers */                              /* 17.15 */
    UI16_T                          fd_while;               /* 17.15.1 */
    UI16_T                          hello_when;             /* 17.15.2 */
    UI16_T                          mdelay_while;           /* 17.15.3 */
    UI16_T                          rb_while;               /* 17.15.4 */
    UI16_T                          rcvd_info_while;        /* 17.15.5 */
    UI16_T                          rr_while;               /* 17.15.6 */
    UI16_T                          tc_while;               /* 17.15.7 */

    /* Per-Port variables */                                /* 17.18 */
    BOOL_T                          agreed;                 /* 17.18.1 */
    XSTP_TYPE_PriorityVector_T      designated_priority;    /* 17.18.2 */
    XSTP_TYPE_Timers_T              designated_times;       /* 17.18.3 */
    BOOL_T                          forward;                /* 17.18.4 */
    BOOL_T                          forwarding;             /* 17.18.5 */
    UI8_T                           info_is;                /* 17.18.6 */
    BOOL_T                          learn;                  /* 17.18.8 */
    BOOL_T                          learning;               /* 17.18.9 */
    XSTP_TYPE_PriorityVector_T      msg_priority;           /* 17.18.11 */
    XSTP_TYPE_Timers_T              msg_times;              /* 17.18.12 */
    BOOL_T                          new_info;               /* 17.18.13 */
    XSTP_TYPE_PortId_T              port_id;                /* 17.18.16 */
    XSTP_TYPE_PriorityVector_T      port_priority;          /* 17.18.17 */
    XSTP_TYPE_Timers_T              port_times;             /* 17.18.18 */
    BOOL_T                          proposed;               /* 17.18.19 */
    BOOL_T                          proposing;              /* 17.18.20 */
    UI8_T                           rcvd_msg;               /* 17.18.22 */
    BOOL_T                          rcvd_rstp;              /* 17.18.23 */
    BOOL_T                          rcvd_stp;               /* 17.18.24 */
    BOOL_T                          rcvd_tc;                /* 17.18.25 */
    BOOL_T                          rcvd_tc_ack;            /* 17.18.26 */
    BOOL_T                          rcvd_tcn;               /* 17.18.27 */
    BOOL_T                          re_root;                /* 17.18.28 */
    BOOL_T                          reselect;               /* 17.18.29 */
    UI8_T                           role;                   /* 17.18.30 */
    BOOL_T                          selected;               /* 17.18.31 */
    UI8_T                           selected_role;          /* 17.18.32 */
    BOOL_T                          sync;                   /* 17.18.34 */
    BOOL_T                          synced;                 /* 17.18.35 */
    BOOL_T                          tc;                     /* 17.18.36 */
    BOOL_T                          tc_ack;                 /* 17.18.37 */
    BOOL_T                          tc_prop;                /* 17.18.38 */
    UI16_T                          tx_count;               /* 17.18.40 */
    BOOL_T                          updt_info;              /* 17.18.41 */

    BOOL_T                          static_port_priority;
    BOOL_T                          disputed;               /* 1D/D3 17.19.6 */
} XSTP_OM_RstPortVar_T;

/* Vector/Times definition for MSTP
 * PriorityVector:
 * {    root_id,    ext_root_path_cost, r_root_id,  int_root_path_cost, designated_bridge_id,designated_port_id,rcv_port_id};
 * CistPriority:
 * {    root_id,    ext_root_path_cost, r_root_id,  int_root_path_cost, designated_bridge_id,designated_port_id,----};
 * MstiPriority:
 * {    ----,       ----,               r_root_id,  int_root_path_cost, designated_bridge_id,designated_port_id,----};
 * Times:
 * {    message_age,max_age,            hello_time, forward_delay,      remaining_hops};
 * CistTimes:
 * {    message_age,max_age,            hello_time, forward_delay,      remaining_hops};
 * MstiTimes:
 * {    ----,       ----,               ----,       ----,               remaining_hops};
 */

typedef struct
{
    /* Common */
    BOOL_T                          begin;                      /* 13.23 (a) */     /* 13.23.1 */
    XSTP_TYPE_MstConfigId_T         mst_config_id;              /* 13.23 (b) */     /* 13.23.8 */

    /* State machine performance parameters */
    UI16_T                          tx_hold_count;              /* 13.23 (c) */     /* 17.16.6 */
    UI16_T                          migrate_time;               /* 13.23 (d) */     /* 17.16.4 */
    BOOL_T                          restart_state_machine;
} XSTP_OM_MstBridgeCommonVar_T;

typedef struct
{
    /* Common */
    UI16_T                          mdelay_while;               /* 13.21 (a) */     /* 17.15.3 */
    UI16_T                          hello_when;                 /* 13.21 (b) */     /* 17.15.2 */
    UI16_T                          edge_delay_while;           /* 802.1D-2004 17.17.1 */
    BOOL_T                          timeout_mdelay_while;       /* timeout flag for mdelay_while */
    BOOL_T                          timeout_hello_when;         /* timeout flag for hello_when   */

    /* CIST only */
    UI32_T                          external_port_path_cost;    /* 13.22 (g) */
    BOOL_T                          static_external_path_cost;

    BOOL_T                          mcheck;                     /* for PPM */       /* 17.18.10 */

    BOOL_T                          tick;                       /* 13.24 (a) */     /* 17.18.39 */
    UI16_T                          tx_count;                   /* 13.24 (b) */     /* 17.18.40 */
    BOOL_T                          oper_edge;                  /* 13.24 (c) */     /* 17.18.14 */
    BOOL_T                          port_enabled;               /* 13.24 (d) */     /* 17.18.15 */
    BOOL_T                          info_internal;              /* 13.24 (e) */     /* 13.24.10 */
    BOOL_T                          new_info_cist;              /* 13.24 (f) */     /* 13.24.19 */
    BOOL_T                          new_info_msti;              /* 13.24 (g) */     /* 13.24.20 */
    BOOL_T                          rcvd_internal;              /* 13.24 (h) */     /* 13.24.22 */
    BOOL_T                          init_pm;                    /* 13.24 (i) */     /* 17.18.7 */
    BOOL_T                          rcvd_rstp;                  /* 13.24 (j) */     /* 17.18.23 */
    BOOL_T                          rcvd_stp;                   /* 13.24 (k) */     /* 17.18.24 */
    BOOL_T                          rcvd_tc_ack;                /* 13.24 (l) */     /* 17.18.26 */
    BOOL_T                          rcvd_tcn;                   /* 13.24 (m) */     /* 17.18.27 */
    BOOL_T                          send_rstp;                  /* 13.24 (n) */     /* 17.18.33 */
    BOOL_T                          tc_ack;                     /* 13.24 (o) */     /* 17.18.37 */
    BOOL_T                          rcvd_bpdu;                  /* 13.24 (w) */     /* 17.18.21 */

    XSTP_TYPE_Bpdu_T                *bpdu;
    BOOL_T                          auto_edge;                  /* 802.1D-2004 17.20.2 */
    BOOL_T                          admin_edge;                 /* 17.18.14 */
    BOOL_T                          admin_point_to_point_mac;
    BOOL_T                          admin_point_to_point_mac_auto;
    BOOL_T                          oper_point_to_point_mac;
    BOOL_T                          link_up;
    UI32_T                          port_spanning_tree_status;  /* per port enable/disable */
    BOOL_T                          loopback_block;             /* TRUE if attaching to the loopback cable */

    L_MM_Mref_Handle_T              *mref_handle_p;

#if (SYS_CPNT_STP_ROOT_GUARD == TRUE)
    BOOL_T                          root_guard_status;          /* per port enable/disable */
    BOOL_T                          root_inconsistent;
#endif /* #if (SYS_CPNT_STP_ROOT_GUARD == TRUE) */
#if (SYS_CPNT_STP_BPDU_GUARD == TRUE)
    BOOL_T                          bpdu_guard_status;
    UI32_T                          bpdu_guard_auto_recovery;
    UI32_T                          bpdu_guard_auto_recovery_interval;
    UI32_T                          bpdu_guard_auto_recovery_while;
#endif
#if (SYS_CPNT_STP_BPDU_FILTER == TRUE)
    BOOL_T                          bpdu_filter_status;
#endif
#if (SYS_CPNT_XSTP_TC_PROP_STOP == TRUE)
    BOOL_T                          tc_prop_stop;
#endif

#if(SYS_CPNT_XSTP_TC_PROP_GROUP == TRUE)
    UI32_T                          tc_prop_group_id;                 
#endif /*#if(SYS_CPNT_XSTP_TC_PROP_GROUP == TRUE)*/
} XSTP_OM_MstPortCommonVar_T;

typedef struct  XSTP_OM_MstBridgeVar_S
{
    /* Common */
    XSTP_OM_MstBridgeCommonVar_T    *common;

    /* CIST reference */
    struct  XSTP_OM_MstBridgeVar_S  *cist;                      /* CIST reference, or NULL for CIST itself */

    UI32_T                          time_since_topology_change;
    UI32_T                          topology_change_count;
    BOOL_T                          trap_flag_tc;               /* trap flag indicating topology change */
    BOOL_T                          trap_flag_new_root;         /* trap flag indicating new root */

    /* Patch */
    BOOL_T                          cist_role_updated;          /* Patch by Allen Cheng for ensuring
                                                                 * MSTI role to be updated once CIST's updated.
                                                                 * This flag is set by updtRolesCist and reset
                                                                 * by updtRolesMsti to ensure that PRS state machine
                                                                 * of MSTI in state RECEIVE progresses once more
                                                                 * if that of CIST has progressed.
                                                                 */

    /* State machine status */
    UI8_T                           sms_prs;                    /* State machine status for PRS */

    /* Per Instance */
    XSTP_TYPE_BridgeId_T            bridge_identifier;          /* 13.23 (c) */     /* 13.23.2 */

    /* CIST, MSTIs */
    XSTP_TYPE_PriorityVector_T      bridge_priority;            /* 13.23 (d)(i) */  /* 13.23.3 */   /* 13.23.9 */
    XSTP_TYPE_Timers_T              bridge_times;               /* 13.23 (e)(j) */  /* 13.23.4 */   /* 13.23.10 */
    XSTP_TYPE_PortId_T              root_port_id;               /* 13.23 (f)(k) */  /* 13.23.5 */   /* 13.23.11 */
    XSTP_TYPE_PriorityVector_T      root_priority;              /* 13.23 (g)(l) */  /* 13.23.6 */   /* 13.23.12 */
    XSTP_TYPE_Timers_T              root_times;                 /* 13.23 (h)(m) */  /* 13.23.7 */   /* 13.23.13 */

    BOOL_T                          static_bridge_priority;
    UI32_T                          admin_bridge_priority;
} XSTP_OM_MstBridgeVar_T;

typedef struct  XSTP_OM_MstPortVar_S
{
    /* Common */
    XSTP_OM_MstPortCommonVar_T  *common;

    /* CIST reference */
    struct XSTP_OM_MstPortVar_S *cist;                      /* CIST reference, or NULL for CIST itself */
    XSTP_TYPE_MstiConfigMsg_T   *msti_config_msg;           /* Received MSTI configuration message, or NULL for CIST */
                                                            /* Valid only when rcvd_msg is TRUE */
    BOOL_T                      is_member;                  /* Visible or not in this spanning tree instance */
/*add by Tony.Lei*/
    UI32_T                      parent_index;               /*if the port is trunk member, the value is its parent's index*/
/*end */
    BOOL_T                      static_internal_path_cost;

    /* State machine status */
    UI8_T                       sms_pti;                    /* State machine status for PTI */
    UI8_T                       sms_prx;                     /* State machine status for PRX */
    UI8_T                       sms_pim;                    /* State machine status for PIM */
    UI8_T                       sms_prt;                    /* State machine status for PRT */
    UI8_T                       sms_pst;                    /* State machine status for PST */
    UI8_T                       sms_tcm;                    /* State machine status for TCM */
    UI8_T                       sms_ppm;                    /* State machine status for PPM */
    UI8_T                       sms_ptx;                    /* State machine status for PTX */
    UI8_T                       sms_bdm;                    /* State machine status for BDM 802.1D-2004 17.25 */

    /* Per Instance */
    UI16_T                      fd_while;                   /* 13.21 (c) */     /* 17.15.1 */
    UI16_T                      rr_while;                   /* 13.21 (d) */     /* 17.15.6 */
    UI16_T                      rb_while;                   /* 13.21 (e) */     /* 17.15.4 */
    UI16_T                      tc_while;                   /* 13.21 (f) */     /* 17.15.7 */
    UI16_T                      rcvd_info_while;            /* 13.21 (g) */     /* 17.15.5 */

    /* Per Instance */
    UI32_T                      internal_port_path_cost;    /* 13.22 (h) */

    BOOL_T                      forward;                    /* 13.24 (p) */     /* 17.18.4 */
    BOOL_T                      forwarding;                 /* 13.24 (q) */     /* 17.18.5 */
    UI8_T                       info_is;                    /* 13.24 (r) */     /* 17.18.6 */
    BOOL_T                      learn;                      /* 13.24 (s) */     /* 17.18.8 */
    BOOL_T                      learning;                   /* 13.24 (t) */     /* 17.18.9 */
    BOOL_T                      proposed;                   /* 13.24 (u) */     /* 17.18.19 */
    BOOL_T                      proposing;                  /* 13.24 (v) */     /* 17.18.20 */
    BOOL_T                      rcvd_tc;                    /* 13.24 (x) */     /* 17.18.25 */
    BOOL_T                      re_root;                    /* 13.24 (y) */     /* 17.18.28 */
    BOOL_T                      reselect;                   /* 13.24 (z) */     /* 17.18.29 */
    BOOL_T                      selected;                   /* 13.24 (aa) */    /* 17.18.31 */
    BOOL_T                      tc_prop;                    /* 13.24 (ab) */    /* 17.18.38 */
    BOOL_T                      updt_info;                  /* 13.24 (ac) */    /* 17.18.41 */

    BOOL_T                      agreed;                     /* 13.24 (ad) */    /* 13.24.2 */
    XSTP_TYPE_PortId_T          port_id;                    /* 13.24 (ae) */    /* 13.24.21 */
    UI8_T                       rcvd_info;                  /* 13.24 (af) */    /* 13.24.23 */
    UI8_T                       role;                       /* 13.24 (ag) */    /* 13.24.25 */
    UI8_T                       selected_role;              /* 13.24 (ah) */    /* 13.24.26 */
    BOOL_T                      sync;                       /* 13.24 (ai) */    /* 13.24.27 */
    BOOL_T                      synced;                     /* 13.24 (aj) */    /* 13.24.28 */

    /* CIST, MSTIs */
    XSTP_TYPE_PriorityVector_T  designated_priority;        /* 13.24 (ak)(aq) */    /* 13.24.4 */   /* 13.24.11 */
    XSTP_TYPE_Timers_T          designated_times;           /* 13.24 (al)(ar) */    /* 13.24.5 */   /* 13.24.12 */
    XSTP_TYPE_PriorityVector_T  msg_priority;               /* 13.24 (am)(as) */    /* 13.24.6 */   /* 13.24.15 */
    XSTP_TYPE_Timers_T          msg_times;                  /* 13.24 (an)(at) */    /* 13.24.7 */   /* 13.24.16 */
    XSTP_TYPE_PriorityVector_T  port_priority;              /* 13.24 (ao)(au) */    /* 13.24.8 */   /* 13.24.17 */
    XSTP_TYPE_Timers_T          port_times;                 /* 13.24 (ap)(av) */    /* 13.24.9 */   /* 13.24.18 */

    BOOL_T                      msti_master;                /* IEEE Std 802.1s(D14.1), 13.24.13 */
    BOOL_T                      msti_mastered;              /* IEEE Std 802.1s(D14.1), 13.24.14 */
    BOOL_T                      agree;                      /* 13.24 (aw) */    /* 13.24.1 */
    BOOL_T                      changed_master;             /* 13.24 (ax) */    /* 13.24.3 */
    BOOL_T                      rcvd_msg;                   /* 13.24 (ay) */    /* 13.24.24 */

    UI16_T                      port_forward_transitions;
    BOOL_T                      static_port_priority;
    BOOL_T                      disputed;                   /* 1D/D3 17.19.6 */
} XSTP_OM_MstPortVar_T;

#ifdef  XSTP_TYPE_PROTOCOL_RSTP
typedef XSTP_OM_RstBridgeVar_T          XSTP_OM_BridgeVar_T;
typedef XSTP_OM_RstPortVar_T            XSTP_OM_PortVar_T;
typedef XSTP_OM_RstBridgeCommonVar_T    XSTP_OM_BridgeCommonVar_T;
typedef XSTP_OM_RstPortCommonVar_T      XSTP_OM_PortCommonVar_T;
#endif /* XSTP_TYPE_PROTOCOL_RSTP */

#ifdef  XSTP_TYPE_PROTOCOL_MSTP
typedef XSTP_OM_MstBridgeVar_T          XSTP_OM_BridgeVar_T;
typedef XSTP_OM_MstPortVar_T            XSTP_OM_PortVar_T;
typedef XSTP_OM_MstBridgeCommonVar_T    XSTP_OM_BridgeCommonVar_T;
typedef XSTP_OM_MstPortCommonVar_T      XSTP_OM_PortCommonVar_T;
#endif /* XSTP_TYPE_PROTOCOL_MSTP */

/* instance_vlans_mapped : vlan list in the bit-map form which indicates the vlan
 *                         is a member of this instance if the corresponding bit
 *                         in the bit-map is true NO MATTER the spanning tree is
 *                         enabled or not.
 *
 *          FEDCBA9876543210
 *  vid =   ----iiiiiiiiibbb
 *          is represented at
 *          bit (bbb) in
 *          instance_vlans_mapped[iiiiiiiii]
 *          iiiiiiiii = [0..511]
 */
typedef struct
{
    UI32_T                      instance_id;
    BOOL_T                      instance_exist;
    UI8_T                       instance_vlans_mapped[512];
    UI32_T                      instance_remaining_hop_count; /* useless ???  */
    UI32_T                      row_status;
    XSTP_OM_BridgeVar_T         bridge_info;
    XSTP_OM_PortVar_T           port_info[XSTP_TYPE_MAX_NUM_OF_LPORT];
    UI32_T                      next;
    BOOL_T                      dirty_bit; /* whether all state machines has progressed once */
    BOOL_T                      delay_flag;
} XSTP_OM_InstanceData_T;

/*
 * Task Information
 */
typedef struct XSTP_OM_SystemData_S
{
    UI8_T                       force_version;          /* 17.16.1 */
    UI32_T                      max_hop_count;
    UI32_T                      max_instance_number;
    UI32_T                      num_of_active_tree;     /* count of trees with XSTP enabled */
    UI32_T                      num_of_cfg_msti;        /* count of MSTI created by user (max:64)*/
    UI32_T                      path_cost_method;
    UI8_T                       region_name[XSTP_TYPE_REGION_NAME_MAX_LENGTH];
    UI32_T                      region_revision;
    UI32_T                      spanning_tree_status;   /* enable/disable */

    BOOL_T                      trap_flag_tc;           /* trap flag indicating topology change */
    UI32_T                      tc_cause_port;
    BOOL_T                      trap_rx_tc;           /* trap flag indicating topology change */
    UI32_T                      tc_rx_port;          
    UI8_T                       tc_casee_brdg_mac[SYS_ADPT_MAC_ADDR_LEN]; /*the bridge send out TC flag or TCN*/
    BOOL_T                      trap_flag_new_root;     /* trap flag indicating new root */
    UI8_T                       mst_topology_method;
#if (SYS_CPNT_STP_COMPATIBLE_WITH_CISCO_PRESTANDARD == TRUE)
    UI32_T                      cisco_prestandard;      /* can be compatible with cisco prestandard version or not */
#endif /* End of #if (SYS_CPNT_STP_COMPATIBLE_WITH_CISCO_PRESTANDARD == TRUE) */
} XSTP_OM_SystemData_T;

typedef struct xstp_om_share{
#define XSTP_OM_Mst_Configuration_Table_Size        8192
#define XSTP_OM_ENTRY_INDEX_ARRAY_SIZE              4096
#define XSTP_OM_KEY_LENGTH    16
    UI8_T XSTP_OM_Mst_Configuration_Table[XSTP_OM_Mst_Configuration_Table_Size];
    XSTP_OM_InstanceData_T          XSTP_OM_InstanceInfo[XSTP_TYPE_MAX_INSTANCE_NUM+1];
    XSTP_OM_SystemData_T            XSTP_OM_SystemInfo;
    UI8_T                           XSTP_OM_InstanceEntryIndex[XSTP_OM_ENTRY_INDEX_ARRAY_SIZE];
    UI8_T   XSTP_OM_Configuration_Digest_Signature_Key[XSTP_OM_KEY_LENGTH];
    XSTP_OM_BridgeCommonVar_T       XSTP_OM_BridgeCommonInfo;
    XSTP_OM_PortCommonVar_T         XSTP_OM_PortCommonInfo[XSTP_TYPE_MAX_NUM_OF_LPORT];
#if (SYS_CPNT_EAPS == TRUE)
    /* To let XSTP know if this port is under control of XSTP or other
     *  ethernet ring protocol.
     *
     * If a port had become a ring port of one ethernet ring protocol,
     *  1. XSTP will not control it's spanning tree state
     *  2. XSTP will get spanning tree status from the correspanding
     *     ethernet ring protocol.
     */
    UI8_T                           XSTP_OM_EthRingProle[SYS_ADPT_TOTAL_NBR_OF_LPORT];
#endif
} XSTP_OM_SHARE_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

BOOL_T  XSTP_OM_Debug(UI32_T flag);
BOOL_T  XSTP_OM_DebugLport(UI32_T lport);
void    XSTP_OM_SetDebugFlag(UI32_T flag);
void    XSTP_OM_GetDebugFlag(UI32_T *flag);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_InitSemaphore
 * ------------------------------------------------------------------------
 * PURPOSE  :   Initiate the semaphore for XSTP objects
 * INPUT    :   None
 * OUTPUT   :   None
 * RETURN   :   None
 * NOTE     :   This function is invoked in XSTP_TASK_Init.
 *-------------------------------------------------------------------------
 */
void    XSTP_OM_InitSemaphore(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_EnterCriticalSection
 * ------------------------------------------------------------------------
 * PURPOSE  :   Enter critical section before a task invokes the spanning
 *              tree objects.
 * INPUT    :   xstid       -- the identifier of the mst instance
 * OUTPUT   :   None
 * RETURN   :   None
 * NOTE     :   None
 *-------------------------------------------------------------------------
 */
void    XSTP_OM_EnterCriticalSection(UI32_T xstid);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_LeaveCriticalSection
 * ------------------------------------------------------------------------
 * PURPOSE  :   Leave critical section after a task invokes the spanning
 *              tree objects.
 * INPUT    :   xstid           -- the identifier of the mst instance
 * OUTPUT   :   None
 * RETURN   :   None
 * NOTE     :   The task priority of the caller is set to original task
 *              priority.
 *-------------------------------------------------------------------------
 */
void    XSTP_OM_LeaveCriticalSection(UI32_T xstid);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_Init
 *-------------------------------------------------------------------------
 * PURPOSE  :
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
void XSTP_OM_Init(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_ClearDatabase
 * ------------------------------------------------------------------------
 * PURPOSE  : Reset/Clear database.
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void XSTP_OM_CleanDatabase(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_SetDefaultValue
 * ------------------------------------------------------------------------
 * PURPOSE  : Set the default values.
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void XSTP_OM_SetDefaultValue(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_NullifyPortOmEngineState
 * ------------------------------------------------------------------------
 * PURPOSE  : Nullify the Engine State of the Port OM
 * INPUT    : om_ptr    -- om pointer for this instance
 *            lport     -- lport
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void XSTP_OM_NullifyPortOmEngineState(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_NullifyInstance
 * ------------------------------------------------------------------------
 * PURPOSE  : Nullify the specified instance
 * INPUT    : om_ptr    -- om pointer for this instance
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
void XSTP_OM_NullifyInstance(XSTP_OM_InstanceData_T *om_ptr);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_NullifyBridgeOm
 * ------------------------------------------------------------------------
 * PURPOSE  : Nullify the Bridge OM
 * INPUT    : om_ptr    -- om pointer for this instance
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
void XSTP_OM_NullifyBridgeOm(XSTP_OM_InstanceData_T *om_ptr);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_NullifyPortOm
 * ------------------------------------------------------------------------
 * PURPOSE  : Nullify the Bridge OM
 * INPUT    : om_ptr    -- om pointer for this instance
 *            lport     -- lport
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
void XSTP_OM_NullifyPortOm(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_MakeOmLportConsistency
 * ------------------------------------------------------------------------
 * PURPOSE  : Rename the specified lport if it is a root port or designated
 *            port
 * INPUT    : om_ptr    -- om pointer for this instance
 *            dst_lport -- destinating lport
 *            src_lport -- source lport
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void XSTP_OM_MakeOmLportConsistency(XSTP_OM_InstanceData_T *om_ptr, UI32_T dst_lport, UI32_T src_lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_ConvertPortOm
 * ------------------------------------------------------------------------
 * PURPOSE  : Convert the Port OM due to its lport number is changed
 * INPUT    : om_ptr    -- om pointer for this instance
 *            dst_lport -- destinating lport
 *            src_lport -- source lport
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void XSTP_OM_ConvertPortOm(XSTP_OM_InstanceData_T *om_ptr, UI32_T dst_lport, UI32_T src_lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_CreateTree
 * ------------------------------------------------------------------------
 * FUNCTION : Create a Spanning Tree.
 * INPUT    : om_ptr    -- om pointer for this instance
 * OUTPUT   : None
 * RETURN   : XSTP_OM_DeleteTree : Create ok
 *-------------------------------------------------------------------------
 */
UI32_T  XSTP_OM_CreateTreeOm(XSTP_OM_InstanceData_T *om_ptr);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_DeleteTree
 * ------------------------------------------------------------------------
 * FUNCTION : Delete a Spanning Tree.
 * INPUT    : om_ptr    -- om pointer for this instance
 * OUTPUT   : None
 * RETURN   : XSTP_OM_DeleteTree : Delete ok
 *-------------------------------------------------------------------------
 */
UI32_T  XSTP_OM_DeleteTreeOm(XSTP_OM_InstanceData_T *om_ptr);

/*-------------------------------------------------------------------------
 * FUNCTION : XSTP_OM_GetInstanceInfoPtr
 * PURPOSE  :
 * INPUT    : xstid -- Spanning tree ID
 * OUTPUT   : None
 * RETUEN   : Pointer to the state machine variables
 * NOTES    : None
 */
XSTP_OM_InstanceData_T*   XSTP_OM_GetInstanceInfoPtr(UI32_T xstid);


/* ------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetNextInstanceInfoPtr
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the om_ptr for the next xstid
 * INPUT    : xstid -- MST instance ID
 * OUTPUT   : xstid -- The next MST instance ID, or XSTP_TYPE_INEXISTENT_MSTID
 *                     if at the end of the OM
 * RETUEN   : Pointer to the om_ptr for the next xstid, or the om_ptr for
 *            the XSTP_TYPE_INEXISTENT_MSTID if at the end of the OM
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T  XSTP_OM_GetNextInstanceInfoPtr(UI32_T *xstid, XSTP_OM_InstanceData_T **om_pptr);

#if 0
/*-------------------------------------------------------------------------
 * FUNCTION : XSTP_OM_GetSystemInfoPtr
 * PURPOSE  :
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : XSTP_TYPE_RETURN_OK   -- OK
 * NOTES    : None
 */
XSTP_OM_SystemData_T* XSTP_OM_GetSystemInfoPtr(void);
#endif


/* ------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetLportDefaultPathCost
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the default path cost of the specified logical port
 * INPUT    :   lport       -- lport
 * OUTPUT   :   path_cost   -- path cost
 * RETURN   :   XSTP_TYPE_RETURN_OK/XSTP_TYPE_RETURN_ERROR
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
UI32_T  XSTP_OM_GetLportDefaultPathCost(UI32_T lport, UI32_T *path_cost);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_RefreshPathCost
 * ------------------------------------------------------------------------
 * PURPOSE  :   Refresh the path cost of the specified logical port
 * INPUT    :   om_ptr      : om pointer for this instance
 *              lport       : lport
 * OUTPUT   :   path_cost   : path cost
 * RETURN   :   TRUE if the path cost is changed, else FALSE returned.
 * NOTE     :   There are 3 cases which may change the path cost.
 *              1. The path cost overflow when the path cost method is changed
 *                 from long to short.
 *              2. The speed-duplex mode of the port changed.
 *              3. Users erase the static path cost set by manual operations.
 * ------------------------------------------------------------------------
 */
BOOL_T  XSTP_OM_RefreshPathCost(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_RefreshOperLinkType
 * ------------------------------------------------------------------------
 * PURPOSE  : Refresh the oper link_type for the specified port.
 * INPUT    : om_ptr                  -- the pointer of the instance entry.
 *            lport                   -- lport number
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void    XSTP_OM_RefreshOperLinkType(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetNextXstpMember
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the next XSTP member
 * INPUT    : om_ptr    -- om pointer for this instance
 *            vid       -- vlan id pointer
 * OUTPUT   : vid       -- next vlan id pointer
 * RETURN   : TRUE if OK, or FALSE if at the end of the member list
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_OM_GetNextXstpMember(XSTP_OM_InstanceData_T *om_ptr, UI32_T *vid);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_IsMemberVlanOfInstance
 *-------------------------------------------------------------------------
 * PURPOSE  : Check whether the specified vlan is the member of this
 *            spanning tree instance
 * INPUT    : om_ptr    -- om pointer for this instance
 *            vid       -- vlan id
 * OUTPUT   : None
 * RETUEN   : TRUE if the specified vlan is the member of this instance, else
 *            FALSE
 * NOTES    :
 *          FEDCBA9876543210
 *          ----------------
 *  vid =   ----iiiiiiiiibbb
 *          is represented at
 *          bit (bbb) in
 *          instance_vlans_mapped[iiiiiiiii]
 *          iiiiiiiii = [0..511]
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_OM_IsMemberVlanOfInstance(XSTP_OM_InstanceData_T *om_ptr, UI16_T vid);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_IsMemberPortOfInstance
 *-------------------------------------------------------------------------
 * PURPOSE  : Check whether the specified lport is the member of this
 *            spanning tree instance
 * INPUT    : om_ptr    -- om pointer for this instance
 *            lport     -- lport
 * OUTPUT   : None
 * RETUEN   : TRUE if the specified vlan is the member of this instance, else
 *            FALSE
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_OM_IsMemberPortOfInstance(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport);

/* ===================================================================== */
/* System Information function
 */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_SetForceVersion
 * ------------------------------------------------------------------------
 * PURPOSE  : Set the system force version
 * INPUT    : force_version
 * OUTPUT   : None
 * RETURN   : None
 *-------------------------------------------------------------------------
 */
void    XSTP_OM_SetForceVersion(UI8_T force_version);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_SetMaxHopCount
 * ------------------------------------------------------------------------
 * PURPOSE  : Set the max_hop_count
 * INPUT    : max_hop_count
 * OUTPUT   : None
 * RETURN   : None
 *-------------------------------------------------------------------------
 */
void    XSTP_OM_SetMaxHopCount(UI32_T max_hop_count);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_SetMaxInstanceNumber
 * ------------------------------------------------------------------------
 * PURPOSE  : Set the max_instance_number
 * INPUT    : max_instance_number
 * OUTPUT   : None
 * RETURN   : None
 *-------------------------------------------------------------------------
 */
void    XSTP_OM_SetMaxInstanceNumber(UI32_T max_instance_number);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_SetPathCostMethod
 * ------------------------------------------------------------------------
 * PURPOSE  : Set the path_cost_method
 * INPUT    : path_cost_method
 * OUTPUT   : None
 * RETURN   : None
 *-------------------------------------------------------------------------
 */
void    XSTP_OM_SetPathCostMethod(UI32_T path_cost_method);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_SetRegionName
 * ------------------------------------------------------------------------
 * PURPOSE  : Set the region name
 * INPUT    : str       -- region name
 * OUTPUT   : None
 * RETURN   : None
 *-------------------------------------------------------------------------
 */
void    XSTP_OM_SetRegionName(char *str);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_SetRegionRevision
 * ------------------------------------------------------------------------
 * PURPOSE  : Set the region_revision
 * INPUT    : region_revision
 * OUTPUT   : None
 * RETURN   : None
 *-------------------------------------------------------------------------
 */
void    XSTP_OM_SetRegionRevision(UI32_T region_revision);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_SetSpanningTreeStatus
 * ------------------------------------------------------------------------
 * PURPOSE  : Set the spanning tree status
 * INPUT    : spanning_tree_status
 * OUTPUT   : None
 * RETURN   : spanning tree status
 *-------------------------------------------------------------------------
 */
void    XSTP_OM_SetSpanningTreeStatus(UI32_T spanning_tree_status);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_SetTrapFlagTc
 * ------------------------------------------------------------------------
 * PURPOSE  : Set the trap_flag_tc
 * INPUT    : trap_flag_tc
 * OUTPUT   : None
 * RETURN   : None
 *-------------------------------------------------------------------------
 */
void    XSTP_OM_SetTrapFlagTc(BOOL_T trap_flag_tc);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_SetTrapFlagNewRoot
 * ------------------------------------------------------------------------
 * PURPOSE  : Set the trap_flag_new_root
 * INPUT    : trap_flag_new_root
 * OUTPUT   : None
 * RETURN   : None
 *-------------------------------------------------------------------------
 */
void    XSTP_OM_SetTrapFlagNewRoot(BOOL_T trap_flag_new_root);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_SetTrapRxFlagTc
 * ------------------------------------------------------------------------
 * PURPOSE  : Set the trap_flag_tc
 * INPUT    : trap_flag_tc
 * OUTPUT   : None
 * RETURN   : None
 *-------------------------------------------------------------------------
 */
void    XSTP_OM_SetTrapRxFlagTc(BOOL_T trap_flag_tc);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_MakeOmPriorityConsistency
 * ------------------------------------------------------------------------
 * PURPOSE  : Let priority recorded in OM consist with bridge_priority
 *            set by user
 * INPUT    : om_ptr    -- om pointer for this instance
 *            priority  -- bridge_priority value
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void XSTP_OM_MakeOmPriorityConsistency(XSTP_OM_InstanceData_T *om_ptr, UI32_T priority);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_MakeOmPortPriorityConsistency
 * ------------------------------------------------------------------------
 * PURPOSE  : Let priority recorded in OM consist with port_priority
 *            set by user
 * INPUT    : om_ptr            -- om pointer for this instance
 *            lport             -- lport value
 *            port_priority     -- port_priority value
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void XSTP_OM_MakeOmPortPriorityConsistency(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport, UI32_T port_priority);

#ifdef  XSTP_TYPE_PROTOCOL_MSTP
/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_FromSameRegion
 * ------------------------------------------------------------------------
 * PURPOSE  : Return TRUE if rcvd_rstp is TRUE, and the received BPDU
 *            conveys an MST Configuration Identifier that matches that
 *            held for the Bridge. Return FALSE otherwise.
 * INPUT    : om_ptr    -- om pointer for this instance
 *            lport     -- specified lport
 * OUTPUT   : TRUE/FALSE
 * RETURN   : None
 * NOTE     : Ref to the description in 13.26.5, IEEE Std 802.1s(D13)-2002
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_OM_FromSameRegion(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GenerateConfigurationDigest
 * ------------------------------------------------------------------------
 * PURPOSE  : generate configuration digest
 * INPUT    : om_ptr    -- om pointer for CIST
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 13.7, IEEE Std 802.1s(D13)-2002
 *-------------------------------------------------------------------------
 */
void    XSTP_OM_GenerateConfigurationDigest(XSTP_OM_InstanceData_T *om_ptr);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_MakeOmPriorityConsistencyCist
 * ------------------------------------------------------------------------
 * PURPOSE  : Let priority recorded in OM consist with bridge_priority
 *            set by user
 * INPUT    : om_ptr    -- om pointer for this instance
 *            priority  -- bridge_priority value
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void XSTP_OM_MakeOmPriorityConsistencyCist(XSTP_OM_InstanceData_T *om_ptr, UI32_T priority);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_MakeOmPriorityConsistencyMsti
 * ------------------------------------------------------------------------
 * PURPOSE  : Let priority recorded in OM consist with bridge_priority
 *            set by user
 * INPUT    : om_ptr    -- om pointer for this instance
 *            priority  -- bridge_priority value
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void XSTP_OM_MakeOmPriorityConsistencyMsti(XSTP_OM_InstanceData_T *om_ptr, UI32_T priority);

#endif /* XSTP_TYPE_PROTOCOL_MSTP */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_CpuMacToString
 * ------------------------------------------------------------------------
 * PURPOSE  : Convert CPU MAC to a string
 * INPUT    : None
 * OUTPUT   : str   -- pointer of the config_name
 * RETUEN   : TRUE/FALSE
 * NOTES    : config_name is guaranteed to be a string ending with a NULL character.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_OM_CpuMacToString(char *str);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_InitMstConfigId
 * ------------------------------------------------------------------------
 * PURPOSE  : Initialize Mst region configuration
 * INPUT    : om_ptr
 * OUTPUT   : None
 * RETUEN   : TRUE/FALSE
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void XSTP_OM_InitMstConfigId(XSTP_OM_InstanceData_T *om_ptr);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_SetMstidToMstConfigurationTableByVlan
 * ------------------------------------------------------------------------
 * PURPOSE  : Set mstid value to mst configuration table for a specified
 *            vlan.
 * INPUT    : vid   -- vlan number
 *            mstid -- mstid value
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void XSTP_OM_SetMstidToMstConfigurationTableByVlan(UI32_T vid,
                                                   UI32_T mstid);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_SetVlanToMstidMappingTable
 * ------------------------------------------------------------------------
 * PURPOSE  : Set the specified vlan id into the Mstid mapping table
 *            vlan.
 * INPUT    : om_ptr    -- om pointer
 *            vlan_id   -- vlan number
 *            state     -- TRUE to set the corresponding bit, or FALSE to reset
 * OUTPUT   : mstid     -- mstid value point
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void    XSTP_OM_SetVlanToMstidMappingTable( XSTP_OM_InstanceData_T *om_ptr,
                                            UI32_T vlan_id,
                                            BOOL_T state);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_StateDebugPort
 * ------------------------------------------------------------------------
 * PURPOSE  :   Check the state debug port mode
 * INPUT    :   lport     -- lport
 * OUTPUT   :   None
 * RETURN   :   TRUE if the lport is in the state debug mode, else FALSE
 * NOTE     :   None
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_OM_StateDebugPort(UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_StateDebugMst
 * ------------------------------------------------------------------------
 * PURPOSE  :   Check the state debug port mode
 * INPUT    :   mstid     -- mstid
 * OUTPUT   :   None
 * RETURN   :   TRUE if the lport is in the state debug mode, else FALSE
 * NOTE     :   None
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_OM_StateDebugMst(UI32_T mstid);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_StateDebugMstSwitch
 * ------------------------------------------------------------------------
 * PURPOSE  :   Switch the state debug mode for specified mstid
 * INPUT    :   mstid     -- mstid
 * OUTPUT   :   None
 * RETURN   :   TRUE if the lport is in the state debug mode after switched,
                else FALSE
 * NOTE     :   None
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_OM_StateDebugMstSwitch(UI32_T mstid);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_StateDebugPortSwitch
 * ------------------------------------------------------------------------
 * PURPOSE  :   Switch the state debug mode
 * INPUT    :   lport     -- lport
 * OUTPUT   :   None
 * RETURN   :   TRUE if the lport is in the state debug mode after switched,
                else FALSE
 * NOTE     :   None
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_OM_StateDebugPortSwitch(UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_AddExistingMemberToInstance
 * ------------------------------------------------------------------------
 * PURPOSE  : Add all existing members to specified instance.
 * INPUT    : om_ptr    -- om pointer for this instance
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void XSTP_OM_AddExistingMemberToInstance(XSTP_OM_InstanceData_T *om_ptr);


/* ------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_SetEntryOfMstid
 * ------------------------------------------------------------------------
 * PURPOSE  : Set/clear the entry of the specified mstid to/from the OM
 * INPUT    : mstid         -- the mstid of the entry to be set/clear
 *            set_instance  -- TRUE to set, or FALSE to clear
 * OUTPUT   : None
 * RETURN   : TRUE if ok, else FALSE if the number of the instance exceeds
 *            the maximum.
 * ------------------------------------------------------------------------
 */
BOOL_T  XSTP_OM_SetEntryOfMstid(UI32_T mstid, BOOL_T set_instance);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_GetInstanceEntryPtr
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the om_ptr for the specified mstidx
 * INPUT    : mstidx -- MST instance entry index
 * OUTPUT   : None
 * RETUEN   : Pointer to the om_ptr for the specified mstidx
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
XSTP_OM_InstanceData_T*   XSTP_OM_GetInstanceEntryPtr(UI32_T mstidx);

#if (SYS_CPNT_STP_COMPATIBLE_WITH_CISCO_PRESTANDARD == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_OM_SetCiscoPrestandardCompatibility
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the cisco prestandard compatibility status
 * INPUT    :   status    -- the status value
 * OUTPUT   :   None
 * RETURN   :   None
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
void XSTP_OM_SetCiscoPrestandardCompatibility(UI32_T status);
#endif /* End of #if (SYS_CPNT_STP_COMPATIBLE_WITH_CISCO_PRESTANDARD == TRUE) */

/* Utilities */
/* ------------------------------------------------------------------------
 * ROUTINE NAME - XSTP_OM_LongHexStrToVal
 * ------------------------------------------------------------------------
 * PURPOSE  : Convert a string to a value
 * INPUT    : hex_str   -- hexadecimal string with a leading "0x" and a
 *                         ending NULL
 *                         the leading characters should be 0x, otherwise
 *                         output value is 0 and FALSE returned
 *            buf_size  -- size of output buffer
 * OUTPUT   : buf       -- output buffer
 * RETURN   : TRUE if convert successful, else FALSE
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
BOOL_T  XSTP_OM_LongHexStrToVal(char *hex_str, UI8_T *buf, UI8_T buf_size);

/* FUNCTION NAME - XSTP_OM_SetTcCausePort
 * PURPOSE : Set the logical port which causes the topology change. (becomes
 *           forwarding from non-forwarding or receives BPDU with TC flag)
 * INPUT   : lport - the logical port causing topology change
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */
void  XSTP_OM_SetTcCausePort(UI32_T lport);

/* FUNCTION NAME - XSTP_OM_GetTcCausePort
 * PURPOSE : Get the logical port which causes the topology change. (becomes
 *           forwarding from non-forwarding or receives BPDU with TC flag)
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : the logical port causing topology change
 * NOTES   : None
 */
UI32_T  XSTP_OM_GetTcCausePort(void);

/* FUNCTION NAME - XSTP_OM_SetTcCausePortAndBrdgMac
 * PURPOSE : Set the logical port which causes the topology change. (becomes
 *           forwarding from non-forwarding or receives BPDU with TC flag)
 * INPUT   : lport - the logical port causing topology change
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 */
void  XSTP_OM_SetTcCausePortAndBrdgMac(UI32_T lport, UI8_T brdg_mac[SYS_ADPT_MAC_ADDR_LEN]);

/* FUNCTION NAME - XSTP_OM_GetTcCausePortAndBrdgMac
 * PURPOSE : Get the logical port which causes the topology change. (becomes
 *           forwarding from non-forwarding or receives BPDU with TC flag)
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : the logical port causing topology change
 * NOTES   : None
 */
void  XSTP_OM_GetTcCausePortAndBrdgMac(UI32_T *lport_p, UI8_T brdg_mac[SYS_ADPT_MAC_ADDR_LEN]);

#endif /* XSTP_OM_PRIVATE_H */
