/*-----------------------------------------------------------------------------
 * Module Name: xstp_type.h
 *-----------------------------------------------------------------------------
 * PURPOSE: Definitions for the RSTP
 *-----------------------------------------------------------------------------
 * NOTES:
 *
 *-----------------------------------------------------------------------------
 * HISTORY:
 *    05/30/2001 - Allen Cheng, Created
 *    06/12/2002 - Kelly Chen, Added
 *
 *-----------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2002
 *-----------------------------------------------------------------------------
 */

#ifndef _XSTP_TYPE_H
#define _XSTP_TYPE_H

#include "sys_type.h"
#include "sys_adpt.h"
#include "sys_dflt.h"
#include "sys_cpnt.h"
#include "leaf_1213.h"
#include "leaf_1493.h"
#include "leaf_es3626a.h"
#include "l_mm.h"


/* Configuration Digest Signature Key: ref to 13.7, IEEE 802.1s */
/* The definition should follow the standard */
#define XSTP_TYPE_CONFIGURATION_DIGEST_SIGNATURE_KEY    "0x13AC06A62E47FD51F95D2BA243CD0346"

#if (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_RSTP)
    #define XSTP_TYPE_PROTOCOL_RSTP
    #define XSTP_TYPE_MAX_INSTANCE_NUM          1
    #define XSTP_TYPE_SUPPORT_STP_MODE          SYS_CPNT_STP_TYPE_RSTP
#elif (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_MSTP)
    #if  (SYS_CPNT_MSTP_SUPPORT_PVST == TRUE)
        #define XSTP_TYPE_PROTOCOL_MSTP
        #define XSTP_TYPE_MAX_INSTANCE_NUM      SYS_ADPT_MAX_NBR_OF_MST_INSTANCE
        #define XSTP_TYPE_SUPPORT_STP_MODE      SYS_CPNT_STP_TYPE_MSTP
    #else
        #define XSTP_TYPE_PROTOCOL_MSTP
        #define XSTP_TYPE_MAX_INSTANCE_NUM      SYS_ADPT_MAX_NBR_OF_MST_INSTANCE
        #define XSTP_TYPE_SUPPORT_STP_MODE      SYS_CPNT_STP_TYPE_MSTP
    #endif
#endif

#define XSTP_TYPE_DEFAULT_INSTANCE_NUM          XSTP_TYPE_MAX_INSTANCE_NUM
#define XSTP_TYPE_MAX_NUM_OF_LPORT              (   ((SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT)   \
                                                  * (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK))  \
                                                  + (SYS_ADPT_MAX_NBR_OF_TRUNK_PER_SYSTEM))
#define XSTP_TYPE_CISTID                        0
#define XSTP_TYPE_BPDU_PROTOCOL_IDENTIFIER      0
#define XSTP_TYPE_STP_PROTOCOL_VERSION_ID       0
#define XSTP_TYPE_RSTP_PROTOCOL_VERSION_ID      VAL_staProtocolType_rstp    /* 2 */
#define XSTP_TYPE_MSTP_PROTOCOL_VERSION_ID      VAL_staProtocolType_mstp    /* 3 */
#define XSTP_TYPE_BPDU_TYPE_CONFIG              0x00    /* 9.3.1, 14.5 */
#define XSTP_TYPE_BPDU_TYPE_TCN                 0x80    /* 9.3.2, 14.5 */
#define XSTP_TYPE_BPDU_TYPE_XST                 0x02    /* 9.3.3, 14.5 */ /* RST/MST BPDU */
#define XSTP_TYPE_BPDU_SAP                      0x4242
#define XSTP_TYPE_STP_SAP                       0x4242
#define XSTP_TYPE_UI_CONTROL_BYTE               0x03
#define XSTP_TYPE_PROTOCOL_ID                   0x0000
#define XSTP_TYPE_BPDU_TYPE                     XSTP_TYPE_BPDU_TYPE_XST

#ifdef  XSTP_TYPE_PROTOCOL_RSTP
    #define XSTP_TYPE_PROTOCOL_VERSION_ID       XSTP_TYPE_RSTP_PROTOCOL_VERSION_ID
#endif
#ifdef  XSTP_TYPE_PROTOCOL_MSTP
    #define XSTP_TYPE_PROTOCOL_VERSION_ID       XSTP_TYPE_MSTP_PROTOCOL_VERSION_ID
#endif

#define XSTP_TYPE_TIMER_TICKS2SEC               100     /* every 1 sec send a XSTP timer event */
#define XSTP_TYPE_MESSAGE_AGE_INCREMENT         1
#define XSTP_TYPE_BPDU_TIME_UNIT                256
#define XSTP_TYPE_TICK_TIME_UNIT                100
#define XSTP_TYPE_MASK_OF_TOPOLOGY_CHANGE_ACK   0x80
#define XSTP_TYPE_MASK_OF_TOPOLOGY_CHANGE       0x01
#define XSTP_TYPE_BPDU_TXCOS                    3       /* highest priority : 3 */
#define XSTP_TYPE_REGION_NAME                   SYS_ADPT_XSTP_REGION_NAME
#define XSTP_TYPE_REGION_NAME_MAX_LENGTH        32                  /* 802.1s-13.7(2), the text
                                                                     * string encoded winthin a
                                                                     * fixed field of 32 octets
                                                                     */
#define XSTP_TYPE_MAX_NBR_OF_VLAN               SYS_ADPT_MAX_NBR_OF_VLAN

/*
 * Defines for STP protocol
 */
#define XSTP_TYPE_MIN_BRIDGE_PRIORITY           MIN_dot1dStpPriority            /* High priority */
#define XSTP_TYPE_MAX_BRIDGE_PRIORITY_STP       MAX_dot1dStpPriority            /* Low  priority */
#define XSTP_TYPE_MAX_BRIDGE_PRIORITY_RSTP_MSTP MAX_xstInstanceCfgPriority      /* Low  priority */
#define XSTP_TYPE_MIN_MAXAGE                    ((MIN_dot1dStpBridgeMaxAge)/100)
#define XSTP_TYPE_MAX_MAXAGE                    ((MAX_dot1dStpBridgeMaxAge)/100)
#define XSTP_TYPE_MIN_HELLO_TIME                ((MIN_dot1dStpBridgeHelloTime)/100)
#define XSTP_TYPE_MAX_HELLO_TIME                ((MAX_dot1dStpBridgeHelloTime)/100)
#define XSTP_TYPE_MIN_FORWARD_DELAY             ((MIN_dot1dStpBridgeForwardDelay)/100)
#define XSTP_TYPE_MAX_FORWARD_DELAY             ((MAX_dot1dStpBridgeForwardDelay)/100)
#define XSTP_TYPE_MIN_PORT_PRIORITY             MIN_dot1dStpPortPriority            /* High priority */
#define XSTP_TYPE_MAX_PORT_PRIORITY_STP         MAX_dot1dStpPortPriority            /* Low  priority */
#define XSTP_TYPE_MAX_PORT_PRIORITY_RSTP_MSTP   MAX_xstInstancePortPriority         /* Low  priority */
#define XSTP_TYPE_MIN_PORT_PATH_COST            MIN_dot1dStpPortPathCost
#define XSTP_TYPE_MAX_PORT_PATH_COST_16         MAX_dot1dStpPortPathCost
#define XSTP_TYPE_MAX_PORT_PATH_COST_32         MAX_xstInstancePortPathCost
#define XSTP_TYPE_PATH_COST_DEFAULT_SHORT       VAL_xstInstanceCfgPathCostMethod_short
#define XSTP_TYPE_PATH_COST_DEFAULT_LONG        VAL_xstInstanceCfgPathCostMethod_long
#define XSTP_TYPE_SYSTEM_ADMIN_STATE_ENABLED    VAL_staSystemStatus_enabled     /* 1 */
#define XSTP_TYPE_SYSTEM_ADMIN_STATE_DISABLED   VAL_staSystemStatus_disabled    /* 2 */
#define XSTP_TYPE_PORT_ADMIN_STATE_ENABLED      VAL_dot1dStpPortEnable_enabled  /* 1 */
#define XSTP_TYPE_PORT_ADMIN_STATE_DISABLED     VAL_dot1dStpPortEnable_disabled /* 2 */
#define XSTP_TYPE_PORT_FAST_MODE_ENABLED        VAL_staPortFastForward_enabled  /* 1 */
#define XSTP_TYPE_PORT_FAST_MODE_DISABLED       VAL_staPortFastForward_disabled /* 2 */
#define XSTP_TYPE_PORT_MACADDRLEARNING_ENABLED  VAL_staPortMacAddrLearning_true /* 1 */
#define XSTP_TYPE_PORT_MACADDRLEARNING_DISABLED VAL_staPortMacAddrLearning_false/* 2 */
#define XSTP_TYPE_STP_MODE                      0
#define XSTP_TYPE_RSTP_MODE                     VAL_staProtocolType_rstp
#define XSTP_TYPE_MSTP_MODE                     VAL_staProtocolType_mstp
#define XSTP_TYPE_PORT_ADMIN_LINK_TYPE_POINT_TO_POINT   VAL_staPortAdminPointToPoint_forceTrue
#define XSTP_TYPE_PORT_ADMIN_LINK_TYPE_SHARED           VAL_staPortAdminPointToPoint_forceFalse
#define XSTP_TYPE_PORT_ADMIN_LINK_TYPE_AUTO             VAL_staPortAdminPointToPoint_auto
#define XSTP_TYPE_MSTP_MIN_MAXHOP                       MIN_mstMaxHops
#define XSTP_TYPE_MSTP_MAX_MAXHOP                       MAX_mstMaxHops
#define XSTP_TYPE_MIN_TX_HOLD_COUNT                     MIN_staTxHoldCount
#define XSTP_TYPE_MAX_TX_HOLD_COUNT                     MAX_staTxHoldCount
#define XSTP_TYPE_PORT_PROTOCOL_MIGRATION_ENABLED       VAL_staPortProtocolMigration_true
#define XSTP_TYPE_PORT_PROTOCOL_MIGRATION_DISABLED      VAL_staPortProtocolMigration_false
#define XSTP_TYPE_PORT_ADMIN_EDGE_PORT_ENABLED          VAL_staPortAdminEdgePortWithAuto_true
#define XSTP_TYPE_PORT_ADMIN_EDGE_PORT_DISABLED         VAL_staPortAdminEdgePortWithAuto_false
#define XSTP_TYPE_PORT_ADMIN_EDGE_PORT_AUTO             VAL_staPortAdminEdgePortWithAuto_auto
#define XSTP_TYPE_SPEC_IEEE8021d                        VAL_dot1dStpProtocolSpecification_ieee8021d
#define XSTP_TYPE_SPEC_UNKNOWN                          VAL_dot1dStpProtocolSpecification_unknown
#define XSTP_TYPE_PORT_OPER_LINK_TYPE_POINT_TO_POINT    VAL_staPortOperPointToPoint_true
#define XSTP_TYPE_PORT_OPER_LINK_TYPE_SHARED            VAL_staPortOperPointToPoint_false
#if (SYS_CPNT_STP_ROOT_GUARD == TRUE)
#define XSTP_TYPE_PORT_ROOT_GUARD_ENABLED       VAL_staPortRootGuardAdminStatus_enabled
#define XSTP_TYPE_PORT_ROOT_GUARD_DISABLED      VAL_staPortRootGuardAdminStatus_disabled
#endif
#if(SYS_CPNT_STP_BPDU_GUARD == TRUE)
#define XSTP_TYPE_PORT_BPDU_GUARD_ENABLED                       VAL_staPortBpduGuard_enabled
#define XSTP_TYPE_PORT_BPDU_GUARD_DISABLED                      VAL_staPortBpduGuard_disabled
#define XSTP_TYPE_PORT_BPDU_GUARD_AUTO_RECOVERY_ENABLED         VAL_staPortBpduGuardAutoRecovery_enabled
#define XSTP_TYPE_PORT_BPDU_GUARD_AUTO_RECOVERY_DISABLED        VAL_staPortBpduGuardAutoRecovery_disabled
#define XSTP_TYPE_MIN_PORT_BPDU_GUARD_AUTO_RECOVERY_INTERVAL    MIN_staPortBpduGuardAutoRecoveryInterval
#define XSTP_TYPE_MAX_PORT_BPDU_GUARD_AUTO_RECOVERY_INTERVAL    MAX_staPortBpduGuardAutoRecoveryInterval
#endif
#if (SYS_CPNT_STP_BPDU_FILTER == TRUE)
#define XSTP_TYPE_PORT_BPDU_FILTER_ENABLED      VAL_staPortBpduFilter_enabled
#define XSTP_TYPE_PORT_BPDU_FILTER_DISABLED     VAL_staPortBpduFilter_disabled
#endif

#define XSTP_TYPE_DEFAULT_SPANNING_TREE_STATUS              SYS_DFLT_STP_MODE
#define XSTP_TYPE_DEFAULT_PORT_FAST_MODE                    SYS_DFLT_STP_PORT_FAST_MODE
#define XSTP_TYPE_DEFAULT_PORT_PRIORITY                     SYS_DFLT_STP_PORT_PRIORITY
#define XSTP_TYPE_DEFAULT_BRIDGE_PRIORITY                   SYS_DFLT_STP_BRIDGE_PRIORITY
#define XSTP_TYPE_DEFAULT_MAX_AGE                           (SYS_DFLT_STP_BRIDGE_MAX_AGE)
#define XSTP_TYPE_DEFAULT_HELLO_TIME                        (SYS_DFLT_STP_BRIDGE_HELLO_TIME)
#define XSTP_TYPE_DEFAULT_FORWARD_DELAY                     (SYS_DFLT_STP_BRIDGE_FORWARD_DELAY)
#define XSTP_TYPE_DEFAULT_PORT_ADMIN_STATE                  SYS_DFLT_STP_PORT_MODE          /* 1 */
#define XSTP_TYPE_DEFAULT_PATH_COST_METHOD                  SYS_DFLT_STP_PATH_COST_METHOD
#if (SYS_DFLT_STP_PROTOCOL_TYPE == VAL_staProtocolType_stp)
    #define XSTP_TYPE_DEFAULT_STP_VERSION                   XSTP_TYPE_STP_PROTOCOL_VERSION_ID            /* Force Version */
#elif (SYS_DFLT_STP_PROTOCOL_TYPE == VAL_staProtocolType_rstp)
    #define XSTP_TYPE_DEFAULT_STP_VERSION                   XSTP_TYPE_RSTP_PROTOCOL_VERSION_ID
#elif (SYS_DFLT_STP_PROTOCOL_TYPE == VAL_staProtocolType_mstp)
    #define XSTP_TYPE_DEFAULT_STP_VERSION                   XSTP_TYPE_MSTP_PROTOCOL_VERSION_ID
#endif
#define XSTP_TYPE_DEFAULT_PORT_LINK_TYPE_MODE               SYS_DFLT_STP_PORT_LINK_TYPE_MODE
#define XSTP_TYPE_DEFAULT_BRIDGE_MAXHOP                     SYS_DFLT_STP_BRIDGE_MAX_HOP
#define XSTP_TYPE_DEFAULT_MIGRATE_TIME                      SYS_DFLT_STP_MIGRATE_TIME
#define XSTP_TYPE_DEFAULT_TX_HOLD_COUNT                     SYS_DFLT_STP_BRIDGE_TX_HOLD_COUNT
#define XSTP_TYPE_DEFAULT_CONFIG_REVISION                   SYS_DFLT_STP_CONFIG_REVISION
#define XSTP_TYPE_DEFAULT_PORT_PROTOCOL_MIGRATION_STATUS    SYS_DFLT_STP_PORT_PROTOCOL_MIGRATION_STATUS
#define XSTP_TYPE_DEFAULT_PORT_ADMIN_EDGE_PORT              SYS_DFLT_STP_PORT_ADMIN_EDGE_PORT
#define XSTP_TYPE_PORT_DISABLED                             VAL_ifAdminStatus_down
#define XSTP_TYPE_PORT_ENABLED                              VAL_ifAdminStatus_up
#define XSTP_TYPE_DEFAULT_CONFIG_ID_FORMAT_SELECTOR         SYS_DFLT_STP_CONFIG_ID_FORMAT_SELECTOR

/* Temporary definitions, once support SNMP, should modify to use leaf constants defined by SNMP. */
#define XSTP_TYPE_LOOPBACK_DETECTION_ACTION_BLOCK       1
#define XSTP_TYPE_LOOPBACK_DETECTION_ACTION_SHUTDOWN    2
#define XSTP_TYPE_DEFAULT_LOOPBACK_DETECTION_ACTION     XSTP_TYPE_LOOPBACK_DETECTION_ACTION_BLOCK
#define XSTP_TYPE_MAX_LOOPBACK_DETECTION_SHUTDOWN_INTERVAL  86400   /* seconds. One day */
#define XSTP_TYPE_MIN_LOOPBACK_DETECTION_SHUTDOWN_INTERVAL  60      /* seconds One minute */
#define XSTP_TYPE_DEFAULT_LOOPBACK_DETECTION_SHUTDOWN_INTERVAL  XSTP_TYPE_MIN_LOOPBACK_DETECTION_SHUTDOWN_INTERVAL

#if (SYS_CPNT_STP_ROOT_GUARD == TRUE)
#define XSTP_TYPE_DEFAULT_PORT_ROOT_GUARD_STATUS            SYS_DFLT_PORT_ROOT_GUARD_STATUS
#define XSTP_TYPE_DEFAULT_ROOT_GUARD_AGED_TIME              SYS_DFLT_ROOT_GUARD_AGED_TIME
#endif
#if (SYS_CPNT_STP_BPDU_GUARD == TRUE)
#define XSTP_TYPE_DEFAULT_PORT_BPDU_GUARD_STATUS                    SYS_DFLT_PORT_BPDU_GUARD_STATUS
#define XSTP_TYPE_DEFAULT_PORT_BPDU_GUARD_AUTO_RECOVERY             SYS_DFLT_PORT_BPDU_GUARD_AUTO_RECOVERY
#define XSTP_TYPE_DEFAULT_PORT_BPDU_GUARD_AUTO_RECOVERY_INTERVAL    SYS_DFLT_PORT_BPDU_GUARD_AUTO_RECOVERY_INTERVAL
#endif
#if (SYS_CPNT_STP_BPDU_FILTER == TRUE)
#define XSTP_TYPE_DEFAULT_PORT_BPDU_FILTER                  SYS_DFLT_PORT_BPDU_FILTER_STATUS
#endif

#define XSTP_TYPE_FAST_FORWARDING_DELAY         100         /* 1 sec */
#define XSTP_TYPE_SYS_MAX_VLAN_ID                           SYS_DFLT_DOT1QMAXVLANID
#if  (SYS_CPNT_MSTP_SUPPORT_PVST == TRUE)
    /* PVST: The maximum/minimum MSTID. This value is in the range 1 through SYS_DFLT_DOT1QMAXVLANID.*/
    #define XSTP_TYPE_MAX_MSTID                             SYS_DFLT_DOT1QMAXVLANID
    #define XSTP_TYPE_MIN_MSTID                             1
#else
    /* The maximum/minimum MSTID. This value is in the range 0 through 4094 according to the standard.*/
    #define XSTP_TYPE_MAX_MSTID                             4094
    #define XSTP_TYPE_MIN_MSTID                             0
#endif
#define XSTP_TYPE_INEXISTENT_MSTID                          0x0FFF

/* Temporary definitions, once support SNMP, should modify to use leaf constants defined by SNMP. */
#if (SYS_CPNT_STP_COMPATIBLE_WITH_CISCO_PRESTANDARD == TRUE)
#define XSTP_TYPE_CISCO_PRESTANDARD_COMPATIBILITY_ENABLED       1
#define XSTP_TYPE_CISCO_PRESTANDARD_COMPATIBILITY_DISABLED      2
#define XSTP_TYPE_DEFAULT_CISCO_PRESTANDARD_COMPATIBILITY       XSTP_TYPE_CISCO_PRESTANDARD_COMPATIBILITY_DISABLED
#endif

#if(SYS_CPNT_XSTP_TC_PROP_GROUP == TRUE)
#define XSTP_TYPE_TC_PROP_MIN_GROUP_ID     1 /*0 can't use*/
#define XSTP_TYPE_TC_PROP_MAX_GROUP_ID     SYS_ADPT_XSTP_TC_PROP_MAX_GROUP_ID /*shall not over 255*/
#define XSTP_TYPE_TC_PROP_DEFAULT_GROUP_ID 0
#endif /*#if(SYS_CPNT_XSTP_TC_PROP_GROUP == TRUE)*/
#pragma pack(1)

typedef struct
{
    union
    {
        /* To solve the endian issue, we also declare a 2 bytes variable. We convert its order and mask with 0xF000 or 0x0FFF
        to get our desired value.         2006.10 hawk*/
        UI16_T  bridge_priority;
        struct
        {
            UI16_T  priority    :4,
            system_id_ext       :12;
        }data;
    } bridge_id_priority;
    UI8_T   addr[6];
} XSTP_TYPE_BridgeId_T;         /* 9.2.5 */

typedef union
{
    UI16_T  port_id;
    struct
    {
        UI16_T  priority        :4,
                port_num        :12;
    } data;
} XSTP_TYPE_PortId_T;           /* 9.2.7 */

typedef struct
{
    /* 17.4.2 */
    XSTP_TYPE_BridgeId_T        root_bridge_id;
    UI32_T                      root_path_cost;
    XSTP_TYPE_BridgeId_T        designated_bridge_id;
    XSTP_TYPE_PortId_T          designated_port_id;
    XSTP_TYPE_PortId_T          bridge_port_id;
} XSTP_TYPE_RstPriorityVector_T;

typedef struct
{
    UI16_T                      message_age;
    UI16_T                      max_age;
    UI16_T                      hello_time;
    UI16_T                      forward_delay;
} XSTP_TYPE_RstTimers_T;

typedef struct
{
    /* 13.9 */
    XSTP_TYPE_BridgeId_T        root_id;
    UI32_T                      ext_root_path_cost;
    XSTP_TYPE_BridgeId_T        r_root_id;
    UI32_T                      int_root_path_cost;
    XSTP_TYPE_BridgeId_T        designated_bridge_id;
    XSTP_TYPE_PortId_T          designated_port_id;
    XSTP_TYPE_PortId_T          rcv_port_id;
} XSTP_TYPE_MstPriorityVector_T;

typedef struct
{
    UI16_T                      message_age;
    UI16_T                      max_age;
    UI16_T                      hello_time;
    UI16_T                      forward_delay;
    UI8_T                       remaining_hops;
} XSTP_TYPE_MstTimers_T;


#ifdef  XSTP_TYPE_PROTOCOL_RSTP
typedef XSTP_TYPE_RstPriorityVector_T   XSTP_TYPE_PriorityVector_T;
typedef XSTP_TYPE_RstTimers_T           XSTP_TYPE_Timers_T;
#endif /* XSTP_TYPE_PROTOCOL_RSTP */

#ifdef  XSTP_TYPE_PROTOCOL_MSTP
typedef XSTP_TYPE_MstPriorityVector_T   XSTP_TYPE_PriorityVector_T ;
typedef XSTP_TYPE_MstTimers_T           XSTP_TYPE_Timers_T;
#endif /* XSTP_TYPE_PROTOCOL_MSTP */

/* The message length must be 16 bytes */
typedef struct
{
    UI16_T      msg_type;       /* 2 bytes */
    UI8_T       saddr[6];       /* 6 bytes */
    L_MM_Mref_Handle_T *mref_handle_p;  /* 4 bytes */
    UI16_T      pkt_length;     /* 2 bytes */
    UI16_T      lport;          /* 2 bytes */
} XSTP_TYPE_MSG_T;              /* 16 bytes in total */

/* ------------------------------
 * BPDU Parameters
 * ------------------------------
 */
typedef struct
{
    UI16_T                      sap;
    UI8_T                       ctrl_byte;
    UI16_T                      protocol_identifier;
    UI8_T                       protocol_version_identifier;
    UI8_T                       bpdu_type;
} XSTP_TYPE_BpduHeader_T;

typedef struct
{
    UI16_T                      sap;
    UI8_T                       ctrl_byte;
    UI16_T                      protocol_identifier;
    UI8_T                       protocol_version_identifier;
    UI8_T                       bpdu_type;
    UI8_T                       flags;
    XSTP_TYPE_BridgeId_T        root_identifier;
    UI32_T                      root_path_cost;
    XSTP_TYPE_BridgeId_T        bridge_identifier;
    XSTP_TYPE_PortId_T          port_identifier;
    UI16_T                      message_age;
    UI16_T                      max_age;
    UI16_T                      hello_time;
    UI16_T                      forward_delay;      /* total (35+3) octets */
} XSTP_TYPE_ConfigBpdu_T;

typedef struct
{
    UI16_T                      sap;
    UI8_T                       ctrl_byte;
    UI16_T                      protocol_identifier;
    UI8_T                       protocol_version_identifier;
    UI8_T                       bpdu_type;          /* total (4+3) octets */
} XSTP_TYPE_TcnBpdu_T;

typedef struct
{
    UI16_T                      sap;
    UI8_T                       ctrl_byte;
    UI16_T                      protocol_identifier;
    UI8_T                       protocol_version_identifier;
    UI8_T                       bpdu_type;
    UI8_T                       flags;
    XSTP_TYPE_BridgeId_T        root_identifier;
    UI32_T                      root_path_cost;
    XSTP_TYPE_BridgeId_T        bridge_identifier;
    XSTP_TYPE_PortId_T          port_identifier;
    UI16_T                      message_age;
    UI16_T                      max_age;
    UI16_T                      hello_time;
    UI16_T                      forward_delay;
    UI8_T                       version_1_length;   /* total (36+3) octets */
} XSTP_TYPE_RstBpdu_T;

#if (SYS_CPNT_XSTP_PATCH_BPDU_FORMAT)
    typedef struct
    {
        UI8_T                       msti_flags;
        XSTP_TYPE_BridgeId_T        msti_regional_root_identifier;
        UI32_T                      msti_internal_root_path_cost;
        XSTP_TYPE_BridgeId_T        msti_bridge_id;
        XSTP_TYPE_PortId_T          msti_port_id;
        UI8_T                       msti_remaining_hops;
    } XSTP_TYPE_MstiConfigMsg_T;                    /* total 24 octets */
#else
    typedef struct
    {
        UI8_T                       msti_flags;
        XSTP_TYPE_BridgeId_T        msti_regional_root_identifier;
        UI32_T                      msti_internal_root_path_cost;
        UI8_T                       msti_bridge_priority;
        UI8_T                       msti_port_priority;
        UI8_T                       msti_remaining_hops;
    } XSTP_TYPE_MstiConfigMsg_T;                /* total 16 octets */
#endif

typedef struct                                              /* 13.7 */
{
    UI8_T                       config_id_format_selector;
    UI8_T                       config_name[XSTP_TYPE_REGION_NAME_MAX_LENGTH];
    UI16_T                      revision_level;
    UI8_T                       config_digest[16];
} XSTP_TYPE_MstConfigId_T;

typedef struct
{
    UI16_T                      sap;
    UI8_T                       ctrl_byte;
    UI16_T                      protocol_identifier;
    UI8_T                       protocol_version_identifier;
    UI8_T                       bpdu_type;
    UI8_T                       cist_flags;
    XSTP_TYPE_BridgeId_T        cist_root_identifier;
    UI32_T                      cist_external_path_cost;
    XSTP_TYPE_BridgeId_T        cist_regional_root_identifier;
    XSTP_TYPE_PortId_T          cist_port_identifier;
    UI16_T                      message_age;
    UI16_T                      max_age;
    UI16_T                      hello_time;
    UI16_T                      forward_delay;
    UI8_T                       version_1_length;   /* total (36+3) octets */
    UI16_T                      version_3_length;
    XSTP_TYPE_MstConfigId_T     mst_configuration_identifier;
    UI32_T                      cist_internal_root_path_cost;
    XSTP_TYPE_BridgeId_T        cist_bridge_identifier;
    UI8_T                       cist_remaining_hops;

    /* MSTI Configuration Message parameters and format */
    /* index 0 ~ 63 stand for MSTID 1 ~ 64 */
    XSTP_TYPE_MstiConfigMsg_T   msti_configureation_message[XSTP_TYPE_MAX_INSTANCE_NUM-1];
} XSTP_TYPE_MstBpdu_T;

typedef union
{
    XSTP_TYPE_BpduHeader_T      bpdu_header;
    XSTP_TYPE_ConfigBpdu_T      config_bpdu;
    XSTP_TYPE_TcnBpdu_T         tcn_bpdu;
    XSTP_TYPE_RstBpdu_T         rst_bpdu;
    XSTP_TYPE_MstBpdu_T         mst_bpdu;
} XSTP_TYPE_Bpdu_T;

#pragma pack()

typedef struct XSTP_TYPE_LportList_S
{
    UI32_T                             xstid;
    UI32_T                             lport;
    BOOL_T                             enter_forwarding;
    struct XSTP_TYPE_LportList_S       *next;
} XSTP_TYPE_LportList_T;

#define XSTP_TYPE_BRIDGE_ID_LENGTH              sizeof(XSTP_TYPE_BridgeId_T)
#define XSTP_TYPE_PORT_ID_LENGTH                sizeof(XSTP_TYPE_PortId_T)
#define XSTP_TYPE_PRIORITY_VECTOR_LENGTH        sizeof(XSTP_TYPE_PriorityVector_T)
#define XSTP_TYPE_TIMERS_LENGTH                 sizeof(XSTP_TYPE_Timers_T)
#define XSTP_TYPE_MST_CONFIG_ID_LENGTH          sizeof(XSTP_TYPE_MstConfigId_T)
#define XSTP_TYPE_MST_CONFIG_MSG_LENGTH         sizeof(XSTP_TYPE_MstiConfigMsg_T)

/* for trace_id of user_id when allocate buffer with l_mm
 */
enum
{
    XSTP_TYPE_TRACE_ID_XSTP_UTY_ADDINTOLPORTLIST = 0,
    XSTP_TYPE_TRACE_ID_XSTP_UTY_SENDCONFIGBPDU,
    XSTP_TYPE_TRACE_ID_XSTP_UTY_SENDRSTPBPDU,
    XSTP_TYPE_TRACE_ID_XSTP_UTY_SENDTCNBPDU,
    XSTP_TYPE_TRACE_ID_XSTP_UTY_SENDMSTPBPDU,
    XSTP_TYPE_TRACE_ID_XSTP_UTY_TXMSTP,
    XSTP_TYPE_TRACE_ID_XSTP_UTY_SETLOOPBACKRECOVERPORTLIST,
    XSTP_TYPE_TRACE_ID_XSTP_UTY_SETBPDUGUARDRECOVERPORTLIST,
};

enum XSTP_TYPE_BPDU_FLAGS_E         /* 9.3.3 */
{
    XSTP_TYPE_BPDU_FLAGS_TC                     =   0x01,
    XSTP_TYPE_BPDU_FLAGS_PROPOSAL               =   0x02,
    XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_UNKNOWN      =   0x00,
    XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_ALT_BAK      =   0x04,
    XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_ROOT         =   0x08,
    XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_DESIGNATED   =   0x0C,
    XSTP_TYPE_BPDU_FLAGS_PORT_ROLE_MASK         =   0x0C,
    XSTP_TYPE_BPDU_FLAGS_LEARNING               =   0x10,
    XSTP_TYPE_BPDU_FLAGS_FORWARDING             =   0x20,
    XSTP_TYPE_BPDU_FLAGS_AGREEMENT              =   0x40,
    XSTP_TYPE_BPDU_FLAGS_TCA                    =   0x80,
    XSTP_TYPE_BPDU_FLAGS_MASTER                 =   0x80
};

enum XSTP_TYPE_PATH_COST_E
{
    XSTP_TYPE_PATH_COST_H10M        =   SYS_DFLT_XSTP_HDPLX_10M_PORT_PATH_COST,
    XSTP_TYPE_PATH_COST_F10M        =   SYS_DFLT_XSTP_FDPLX_10M_PORT_PATH_COST,
    XSTP_TYPE_PATH_COST_T10M        =   SYS_DFLT_XSTP_TRUNK_10M_PORT_PATH_COST,
    XSTP_TYPE_PATH_COST_H100M       =   SYS_DFLT_XSTP_HDPLX_100M_PORT_PATH_COST,
    XSTP_TYPE_PATH_COST_F100M       =   SYS_DFLT_XSTP_FDPLX_100M_PORT_PATH_COST,
    XSTP_TYPE_PATH_COST_T100M       =   SYS_DFLT_XSTP_TRUNK_100M_PORT_PATH_COST,
    XSTP_TYPE_PATH_COST_H1G         =   SYS_DFLT_XSTP_HDPLX_1G_PORT_PATH_COST,
    XSTP_TYPE_PATH_COST_F1G         =   SYS_DFLT_XSTP_FDPLX_1G_PORT_PATH_COST,
    XSTP_TYPE_PATH_COST_T1G         =   SYS_DFLT_XSTP_TRUNK_1G_PORT_PATH_COST,
    XSTP_TYPE_PATH_COST_H10G        =   SYS_DFLT_XSTP_HDPLX_10G_PORT_PATH_COST,
    XSTP_TYPE_PATH_COST_F10G        =   SYS_DFLT_XSTP_FDPLX_10G_PORT_PATH_COST,
    XSTP_TYPE_PATH_COST_T10G        =   SYS_DFLT_XSTP_TRUNK_10G_PORT_PATH_COST,
};

/* STP Port Status : defined in leaf2674.h */
enum XSTP_PORT_STATE_E
{
    XSTP_TYPE_PORT_STATE_DISABLED   =   VAL_dot1dStpPortState_disabled,
    XSTP_TYPE_PORT_STATE_BLOCKING   =   VAL_dot1dStpPortState_blocking,
    XSTP_TYPE_PORT_STATE_LISTENING  =   VAL_dot1dStpPortState_listening,
    XSTP_TYPE_PORT_STATE_LEARNING   =   VAL_dot1dStpPortState_learning,
    XSTP_TYPE_PORT_STATE_FORWARDING =   VAL_dot1dStpPortState_forwarding,
    XSTP_TYPE_PORT_STATE_BROKEN     =   VAL_dot1dStpPortState_broken,
    XSTP_TYPE_PORT_STATE_DISCARDING =   VAL_dot1dStpPortState_blocking
};

enum XSTP_TYPE_EVENT_MASK_E
{
    XSTP_TYPE_EVENT_NONE                =   0,
    XSTP_TYPE_EVENT_BPDURCVD            =   BIT_0,
    XSTP_TYPE_EVENT_TIMER               =   BIT_1,
    XSTP_TYPE_EVENT_CALLBACK            =   BIT_2,
    XSTP_TYPE_EVENT_ENTER_TRANSITION    =   BIT_3,
    XSTP_TYPE_EVENT_HDTIMER             =   BIT_4,
    XSTP_TYPE_EVENT_ALL                 =   (BIT_0 | BIT_1 | BIT_2 | BIT_3 | BIT_4)
};

enum XSTP_TYPE_MSG_TYPE_E
{
    XSTP_TYPE_MSG_BPDU              =   1,
    XSTP_TYPE_MSG_REQUEST,
    XSTP_TYPE_MSG_SERVICE,
    XSTP_TYPE_MSG_MISC
};

enum XSTP_TYPE_RETURN_CODE_E
{
    XSTP_TYPE_RETURN_OK = 0,        /* OK, Successful, Without any Error */
    XSTP_TYPE_RETURN_ERROR,         /* Error */
    XSTP_TYPE_RETURN_INDEX_OOR,
    XSTP_TYPE_RETURN_PORTNO_OOR,
    XSTP_TYPE_RETURN_INDEX_NEX,
    XSTP_TYPE_RETURN_MASTER_MODE_ERROR,
    XSTP_TYPE_RETURN_ST_STATUS_DISABLED,
    XSTP_TYPE_RETURN_OTHERS
};

enum XSTP_TYPE_SPDPLX_E
{
    XSTP_TYPE_SPDPLX_10HALF         = VAL_portSpeedDpxCfg_halfDuplex10,
    XSTP_TYPE_SPDPLX_10FULL         = VAL_portSpeedDpxCfg_fullDuplex10,
    XSTP_TYPE_SPDPLX_100HALF        = VAL_portSpeedDpxCfg_halfDuplex100,
    XSTP_TYPE_SPDPLX_100FULL        = VAL_portSpeedDpxCfg_fullDuplex100,
    XSTP_TYPE_SPDPLX_1000HALF       = VAL_portSpeedDpxCfg_halfDuplex1000,
    XSTP_TYPE_SPDPLX_1000FULL       = VAL_portSpeedDpxCfg_fullDuplex1000,
    XSTP_TYPE_SPDPLX_10GHALF        = VAL_portSpeedDpxCfg_halfDuplex10g,
    XSTP_TYPE_SPDPLX_10GFULL        = VAL_portSpeedDpxCfg_fullDuplex10g,
    XSTP_TYPE_SPDPLX_40GFULL        = VAL_portSpeedDpxCfg_fullDuplex40g,
    XSTP_TYPE_SPDPLX_100GFULL       = VAL_portSpeedDpxCfg_fullDuplex100g,
    XSTP_TYPE_SPDPLX_25GFULL        = VAL_portSpeedDpxCfg_fullDuplex25g
};

enum XSTP_TYPE_LOG_FUN_E
{
    XSTP_TYPE_LOG_FUN_XSTP_TASK_CREATE_TASK = 0,
    XSTP_TYPE_LOG_FUN_XSTP_TASK_WAITEVENT,
    XSTP_TYPE_LOG_FUN_XSTP_TASK_SERVICEREQUEST,
    XSTP_TYPE_LOG_FUN_XSTP_TASK_HANDLEREQUEST,
    XSTP_TYPE_LOG_FUN_XSTP_TX_SENDCONFIGBPDU,
    XSTP_TYPE_LOG_FUN_XSTP_TX_SENDTCNBPDU,
    XSTP_TYPE_LOG_FUN_XSTP_OM_LPORTMGMT_DELLPORTMEMBER,
    XSTP_TYPE_LOG_FUN_XSTP_OM_LPORTMGMT_NEW,
    XSTP_TYPE_LOG_FUN_XSTP_OM_NEWMEMBER,
    XSTP_TYPE_LOG_FUN_OTHER
};

enum XSTP_TYPE_LOG_ERR_E
{
    XSTP_TYPE_LOG_ERR_XSTP_TASK_CREATE_TASK = 0,
    XSTP_TYPE_LOG_ERR_XSTP_TASK_WAITEVENT,
    XSTP_TYPE_LOG_ERR_XSTP_TASK_SERVICEREQUEST,
    XSTP_TYPE_LOG_ERR_XSTP_TASK_HANDLEREQUEST,
    XSTP_TYPE_LOG_ERR_XSTP_TX_SENDCONFIGBPDU,
    XSTP_TYPE_LOG_ERR_XSTP_TX_SENDTCNBPDU,
    XSTP_TYPE_LOG_ERR_XSTP_OM_LPORTMGMT_DELLPORTMEMBER,
    XSTP_TYPE_LOG_ERR_XSTP_OM_LPORTMGMT_NEW,
    XSTP_TYPE_LOG_ERR_XSTP_OM_NEWMEMBER,
    XSTP_TYPE_LOG_ERR_OTHER
};

enum XSTP_TYPE_DEBUG_FLAG_E
{
    XSTP_TYPE_DEBUG_FLAG_NONE       =   0x00000000L,
    XSTP_TYPE_DEBUG_FLAG_TXTCN      =   0x00000001L,
    XSTP_TYPE_DEBUG_FLAG_TXCFG      =   0x00000002L,
    XSTP_TYPE_DEBUG_FLAG_TXRSTP     =   0x00000004L,
    XSTP_TYPE_DEBUG_FLAG_RXTCN      =   0x00000010L,
    XSTP_TYPE_DEBUG_FLAG_RXCFG      =   0x00000020L,
    XSTP_TYPE_DEBUG_FLAG_RXRSTP     =   0x00000040L,
    XSTP_TYPE_DEBUG_FLAG_DBGSM      =   0x00010000L,
    XSTP_TYPE_DEBUG_FLAG_DBGMSG     =   0x00100000L,
    XSTP_TYPE_DEBUG_FLAG_ERRMSG     =   0x01000000L,
    XSTP_TYPE_DEBUG_FLAG_ALL        =   0xFFFFFFFFL
};

enum XSTP_TYPE_PORT_ROLE_E
{
    XSTP_TYPE_PORT_ROLE_DISABLED     =   VAL_xstInstancePortPortRole_disabled,
    XSTP_TYPE_PORT_ROLE_ROOT         =   VAL_xstInstancePortPortRole_root,
    XSTP_TYPE_PORT_ROLE_DESIGNATED   =   VAL_xstInstancePortPortRole_designated,
    XSTP_TYPE_PORT_ROLE_ALTERNATE    =   VAL_xstInstancePortPortRole_alternate,
    XSTP_TYPE_PORT_ROLE_BACKUP       =   VAL_xstInstancePortPortRole_backup,
    XSTP_TYPE_PORT_ROLE_MASTER       =   VAL_xstInstancePortPortRole_master
};

enum XSTP_TYPE_SPDPLX_STATUS_E
{
    XSTP_TYPE_SPDPLX_STATUS_10HALF         = VAL_portSpeedDpxStatus_halfDuplex10,
    XSTP_TYPE_SPDPLX_STATUS_10FULL         = VAL_portSpeedDpxStatus_fullDuplex10,
    XSTP_TYPE_SPDPLX_STATUS_100HALF        = VAL_portSpeedDpxStatus_halfDuplex100,
    XSTP_TYPE_SPDPLX_STATUS_100FULL        = VAL_portSpeedDpxStatus_fullDuplex100,
    XSTP_TYPE_SPDPLX_STATUS_1000HALF       = VAL_portSpeedDpxStatus_halfDuplex1000,
    XSTP_TYPE_SPDPLX_STATUS_1000FULL       = VAL_portSpeedDpxStatus_fullDuplex1000,
    XSTP_TYPE_SPDPLX_STATUS_10GHALF        = VAL_portSpeedDpxStatus_halfDuplex10g,
    XSTP_TYPE_SPDPLX_STATUS_10GFULL        = VAL_portSpeedDpxStatus_fullDuplex10g,
    XSTP_TYPE_SPDPLX_STATUS_40GHALF        = VAL_portSpeedDpxStatus_halfDuplex40g,
    XSTP_TYPE_SPDPLX_STATUS_40GFULL        = VAL_portSpeedDpxStatus_fullDuplex40g,
    XSTP_TYPE_SPDPLX_STATUS_100GHALF       = VAL_portSpeedDpxStatus_halfDuplex100g,
    XSTP_TYPE_SPDPLX_STATUS_100GFULL       = VAL_portSpeedDpxStatus_fullDuplex100g,
    XSTP_TYPE_SPDPLX_STATUS_25GHALF        = VAL_portSpeedDpxStatus_halfDuplex25g,
    XSTP_TYPE_SPDPLX_STATUS_25GFULL        = VAL_portSpeedDpxStatus_fullDuplex25g
};

/* Indicate the BPDU flooding behavior, when it's enabled, it has 2 behaviors.*/
enum XSTP_TYPE_FLOODING_BEHAVIOR_E
{
    XSTP_TYPE_FLOODING_TO_VLAN = VAL_staSystemBPDUFlooding_to_vlan, /* to other ports which in the same VLAN  */
    XSTP_TYPE_FLOODING_TO_ALL= VAL_staSystemBPDUFlooding_to_all,  /* to all other ports */
    XSTP_TYPE_FLOODING_ENABLE= VAL_staPortBpduFlooding_enabled,  /* enable the flooding behavior*/
    XSTP_TYPE_FLOODING_DISABLE = VAL_staPortBpduFlooding_disabled
};

typedef struct
{
    UI8_T      msg_type;
    UI32_T     vid_ifidx;
    UI32_T     lport_ifidx;
}XSTP_TYPE_VLAN_MSG_T;


enum XSTP_TYPE_VLAN_MSG_TYPE_E
{
    XSTP_TYPE_VLAN_MSG_CREATED              =   1,
    XSTP_TYPE_VLAN_MSG_DESTROY,
    XSTP_TYPE_VLAN_MSG_MEMBER_ADD,
    XSTP_TYPE_MSG_MISC_MEMBER_DELETE
};

/* temporary. For per_port spanning tree*/
#define XSTP_TYPE_DEFAULT_PORT_SPANNING_TREE_STATUS         VAL_staPortSystemStatus_enabled

#define XSTP_TYPE_ST              10 /* short time (tick) */
#define XSTP_TYPE_LT              20 /* long time (tick) */
#define XSTP_TYPE_PT              10 /* periodic time (tick) */

/*=============================================================================
 * Moved from xstp_mgr.h
 *=============================================================================
 */

typedef struct
{
    /* key */
    UI16_T                      dot1d_stp_port;
    UI8_T                       dot1d_stp_port_priority;
    UI8_T                       dot1d_stp_port_state;
    UI8_T                       dot1d_stp_port_enable;
    UI32_T                      dot1d_stp_port_path_cost;          /* equal external_port_path_cost for mstp */
    XSTP_TYPE_BridgeId_T        dot1d_stp_port_designated_root;
    UI32_T                      dot1d_stp_port_designated_cost;
    XSTP_TYPE_BridgeId_T        dot1d_stp_port_designated_bridge;
    XSTP_TYPE_PortId_T          dot1d_stp_port_designated_port;
    UI16_T                      dot1d_stp_port_forward_transitions;
    UI32_T                      port_role;                         /* kinghong suggest */
    UI32_T                      mstp_internal_port_path_cost;      /* Only for mstp */
    UI32_T                      admin_path_cost;                   /* path cost for RSTP and internal path cost for MSTP */
    UI32_T                      oper_path_cost;                    /* path cost for RSTP and internal path cost for MSTP */
    UI32_T                      admin_external_path_cost;          /* path cost for RSTP and external path cost for MSTP */
    UI32_T                      oper_external_path_cost;           /* path cost for RSTP and external path cost for MSTP */
    UI32_T                      mstp_hello_time;                   /* Root's hello_time */
    UI32_T                      mstp_port_hello_time;              /* Bridge's hello_time */
    BOOL_T                      mstp_boundary_port;
} XSTP_MGR_Dot1dStpPortEntry_T;

typedef struct
{
    BOOL_T          dot1d_stp_port_protocol_migration;
    BOOL_T          dot1d_stp_port_admin_edge_port;            /* 802.1t */
    BOOL_T          dot1d_stp_port_oper_edge_port;             /* 802.1t */
    UI8_T           dot1d_stp_port_admin_point_to_point;
    BOOL_T          dot1d_stp_port_oper_point_to_point;
    UI32_T          dot1d_stp_port_long_path_cost;             /* kinghong suggest, equal dot1d_stp_port_path_cost */
    UI32_T          port_admin_long_path_cost;                 /* path cost for RSTP and external path cost for MSTP */
    UI32_T          port_oper_long_path_cost;                  /* path cost for RSTP and external path cost for MSTP */
    UI8_T           dot1d_stp_port_spanning_tree_status;       /* enables/disabled */
#if (SYS_CPNT_STP_ROOT_GUARD == TRUE)
    UI32_T          port_root_guard_status;                    /* enables/disabled */
#endif
#if (SYS_CPNT_STP_BPDU_GUARD == TRUE)
    UI32_T          port_bpdu_guard_status;
    UI32_T          port_bpdu_guard_auto_recovery;
    UI32_T          port_bpdu_guard_auto_recovery_interval;
#endif
#if (SYS_CPNT_STP_BPDU_FILTER == TRUE)
    UI32_T          port_bpdu_filter_status;
#endif
#if(SYS_CPNT_XSTP_TC_PROP_STOP == TRUE)
    BOOL_T          tc_prop_stop;
#endif
} XSTP_MGR_Dot1dStpExtPortEntry_T;

/*-------------------------------------------------------------------------
 * The following structures apply to MSTP
 *-------------------------------------------------------------------------
 */

typedef struct
{
    UI32_T          mstp_max_instance_number;
    UI32_T          mstp_max_mstid;
    UI32_T          mstp_current_instance_number;
    UI32_T          mstp_format_selector;
    char            mstp_region_name[XSTP_TYPE_REGION_NAME_MAX_LENGTH+1];
    UI32_T          mstp_region_revision;
    UI32_T          mstp_max_hop_count;
} XSTP_MGR_MstpEntry_T;

typedef struct
{
    /*UI32_T          mstp_instance_index;*/
    UI8_T           mstp_instance_vlans_mapped[128];
    UI8_T           mstp_instance_vlans_mapped2k[128];
    UI8_T           mstp_instance_vlans_mapped3k[128];
    UI8_T           mstp_instance_vlans_mapped4k[128];
    UI32_T          mstp_instance_remaining_hop_count;
} XSTP_MGR_MstpInstanceEntry_T;

typedef struct
{
    UI16_T          priority;
    UI16_T          system_id_ext;
    UI8_T           addr[6];
} XSTP_MGR_BridgeIdComponent_T;

typedef struct
{
    UI8_T          priority;
    UI16_T         port_num;
} XSTP_MGR_PortIdComponent_T;

/* To let XSTP know if this port is under control of XSTP or other
 *  ethernet ring protocol.
 *
 * If a port had become a ring port of one ethernet ring protocol,
 *  1. XSTP will not control it's spanning tree state
 *  2. XSTP will get spanning tree status from the correspanding
 *     ethernet ring protocol.
 */
#if (SYS_CPNT_EAPS == TRUE)
enum XSTP_TYPE_ETH_RING_PORT_ROLE_E
{
    XSTP_TYPE_ETH_RING_PORT_ROLE_NONE       = 0,
    XSTP_TYPE_ETH_RING_PORT_ROLE_EAPS,
    XSTP_TYPE_ETH_RING_PORT_ROLE_ERPS,
    XSTP_TYPE_ETH_RING_PORT_ROLE_MAX
};
#endif /* #if (SYS_CPNT_EAPS == TRUE) */

#endif /* _XSTP_TYPE_H */

