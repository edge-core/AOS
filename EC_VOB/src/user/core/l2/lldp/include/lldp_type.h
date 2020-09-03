/*-----------------------------------------------------------------------------
 * Module Name: lldp_type.h
 *-----------------------------------------------------------------------------
 * PURPOSE: Definitions for the LLDP variables
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
#ifndef _LLDP_TYPE_H
#define _LLDP_TYPE_H


#include "l_mm.h"
#include "sys_type.h"
#include "sys_adpt.h"
#include "sys_cpnt.h"
#include "sys_dflt.h"
#include "sys_bld.h"
#include "l_sort_lst.h"
#include "leaf_ieeelldp.h"
#include "leaf_ieeelldpext.h"
#include "leaf_ieee8021ab_ext_dot3.h"
#include "leaf_ansi_tia_1057.h"

/* field length */
#define LLDP_TYPE_MAX_MANAGEMENT_ADDR_LENGTH                MAXSIZE_lldpLocManAddr

#define LLDP_TYPE_MAX_CHASSIS_ID_LENGTH                     SYS_ADPT_LLDP_MAX_CHASSIS_ID_LENGTH
#define LLDP_TYPE_MAX_PORT_ID_LENGTH                        SYS_ADPT_LLDP_MAX_PORT_ID_LENGTH

#define LLDP_TYPE_MAX_PORT_DESC_LENGTH                      MAXSIZE_lldpLocPortDesc + 1
#define LLDP_TYPE_MAX_SYS_NAME_LENGTH                       MAXSIZE_lldpLocSysName + 1
#define LLDP_TYPE_MAX_SYS_DESC_LENGTH                       MAXSIZE_lldpLocSysDesc + 1

#define LLDP_TYPE_MAX_MANAGEMENT_ADDR_OID_LENGTH            MAXSIZE_lldpRemManAddrOID


#define LLDP_TYPE_MIN_NUM_OF_LPORT                          MIN_lldpPortConfigPortNum
#define LLDP_TYPE_MAX_NUM_OF_LPORT                          SYS_ADPT_TOTAL_NBR_OF_LPORT

#define LLDP_TYPE_MAX_NUM_OF_LOC_MAN_ADDR_ENTRY             128

#define LLDP_TYPE_MAX_NUM_OF_JOINED_VLAN                    32

#define LLDP_TYPE_MAX_NBR_OF_REM_DATA_ENTRY_PER_PORT        SYS_ADPT_LLDP_MAX_NBR_OF_REM_DATA_ENTRY_PER_PORT

/* LLDP_UTY_DiffRemoteData */
#define LLDP_TYPE_REMOTE_DATA_EQUAL                         1
#define LLDP_TYPE_REMOTE_DATA_DIFF                          0
#define LLDP_TYPE_REM_MAN_ADDR_EQUAL                        1
#define LLDP_TYPE_REM_MAN_ADDR_DIFF                         0
#define LLDP_TYPE_REM_SYS_EQUAL                             1
#define LLDP_TYPE_REM_SYS_DIFF                              0

/* when SomethingChanged */
#define LLDP_TYPE_MGR_CHANGED_TRUE                          1
#define LLDP_TYPE_MGR_CHANGED_FALSE                         0

/* LLDP config variable range */
#define LLDP_TYPE_MAX_TTL                                   65535
#define LLDP_TYPE_MIN_TTL                                   0
#define LLDP_TYPE_MIN_TX_INTERVAL                           MIN_lldpMessageTxInterval
#define LLDP_TYPE_MAX_TX_INTERVAL                           MAX_lldpMessageTxInterval
#define LLDP_TYPE_MIN_TX_HOLD_MUL                           MIN_lldpMessageTxHoldMultiplier
#define LLDP_TYPE_MAX_TX_HOLD_MUL                           MAX_lldpMessageTxHoldMultiplier
#define LLDP_TYPE_MAX_REINIT_DELAY                          MIN_lldpReinitDelay
#define LLDP_TYPE_MIN_REINIT_DELAY                          MAX_lldpReinitDelay
#define LLDP_TYPE_MIN_TX_DELAY                              MIN_lldpTxDelay
#define LLDP_TYPE_MAX_TX_DELAY                              MAX_lldpTxDelay
#define LLDP_TYPE_MIN_NOTIFY_INTERVAL                       MIN_lldpNotificationInterval
#define LLDP_TYPE_MAX_NOTIFY_INTERVAL                       MAX_lldpNotificationInterval

/* LLDP management address subtype */

#define LLDP_TYPE_MAN_ADDR_SUBTYPE_IPV4                     1
#define LLDP_TYPE_MAN_ADDR_SUBTYPE_IPV6                     2
#define LLDP_TYPE_MAN_ADDR_SUBTYPE_ALL802                   6

/* LLDP local system capabilities supported */
#define LLDP_TYPE_LOC_SYS_CAP_SUP_OTHER                     BIT_VALUE(VAL_lldpLocSysCapSupported_other)
#define LLDP_TYPE_LOC_SYS_CAP_SUP_REPEATER                  BIT_VALUE(VAL_lldpLocSysCapSupported_repeater)
#define LLDP_TYPE_LOC_SYS_CAP_SUP_BRIDGE                    BIT_VALUE(VAL_lldpLocSysCapSupported_bridge)
#define LLDP_TYPE_LOC_SYS_CAP_SUP_WLAN_ACCESS_POINT         BIT_VALUE(VAL_lldpLocSysCapSupported_wlanAccessPoint)
#define LLDP_TYPE_LOC_SYS_CAP_SUP_ROUTER                    BIT_VALUE(VAL_lldpLocSysCapSupported_router)
#define LLDP_TYPE_LOC_SYS_CAP_SUP_TELEPHONE                 BIT_VALUE(VAL_lldpLocSysCapSupported_telephone)
#define LLDP_TYPE_LOC_SYS_CAP_SUP_DOCSIS_CABLE_DEVICE       BIT_VALUE(VAL_lldpLocSysCapSupported_docsisCableDevice)
#define LLDP_TYPE_LOC_SYS_CAP_SUP_STATION_ONLY              BIT_VALUE(VAL_lldpLocSysCapSupported_stationOnly)

/* LLDP local system capabilities enabled */
#define LLDP_TYPE_LOC_SYS_CAP_ENA_OTHER                     BIT_VALUE(VAL_lldpLocSysCapEnabled_other)
#define LLDP_TYPE_LOC_SYS_CAP_ENA_REPEATER                  BIT_VALUE(VAL_lldpLocSysCapEnabled_repeater)
#define LLDP_TYPE_LOC_SYS_CAP_ENA_BRIDGE                    BIT_VALUE(VAL_lldpLocSysCapEnabled_bridge)
#define LLDP_TYPE_LOC_SYS_CAP_ENA_WLAN_ACCESS_POINT         BIT_VALUE(VAL_lldpLocSysCapEnabled_wlanAccessPoint)
#define LLDP_TYPE_LOC_SYS_CAP_ENA_ROUTER                    BIT_VALUE(VAL_lldpLocSysCapEnabled_router)
#define LLDP_TYPE_LOC_SYS_CAP_ENA_TELEPHONE                 BIT_VALUE(VAL_lldpLocSysCapEnabled_telephone)
#define LLDP_TYPE_LOC_SYS_CAP_ENA_DOCSIS_CABLE_DEVICE       BIT_VALUE(VAL_lldpLocSysCapEnabled_docsisCableDevice)
#define LLDP_TYPE_LOC_SYS_CAP_ENA_STATION_ONLY              BIT_VALUE(VAL_lldpLocSysCapEnabled_stationOnly)

/* LLDP remote system capabilities supported */
#define LLDP_TYPE_REM_SYS_CAP_SUP_OTHER                     BIT_VALUE(VAL_lldpRemSysCapSupported_other)
#define LLDP_TYPE_REM_SYS_CAP_SUP_REPEATER                  BIT_VALUE(VAL_lldpRemSysCapSupported_repeater)
#define LLDP_TYPE_REM_SYS_CAP_SUP_BRIDGE                    BIT_VALUE(VAL_lldpRemSysCapSupported_bridge)
#define LLDP_TYPE_REM_SYS_CAP_SUP_WLAN_ACCESS_POINT         BIT_VALUE(VAL_lldpRemSysCapSupported_wlanAccessPoint)
#define LLDP_TYPE_REM_SYS_CAP_SUP_ROUTER                    BIT_VALUE(VAL_lldpRemSysCapSupported_router)
#define LLDP_TYPE_REM_SYS_CAP_SUP_TELEPHONE                 BIT_VALUE(VAL_lldpRemSysCapSupported_telephone)
#define LLDP_TYPE_REM_SYS_CAP_SUP_DOCSIS_CABLE_DEVICE       BIT_VALUE(VAL_lldpRemSysCapSupported_docsisCableDevice)
#define LLDP_TYPE_REM_SYS_CAP_SUP_STATION_ONLY              BIT_VALUE(VAL_lldpRemSysCapSupported_stationOnly)

/* LLDP remote system capabilities enabled */
#define LLDP_TYPE_REM_SYS_CAP_ENA_OTHER                     BIT_VALUE(VAL_lldpRemSysCapEnabled_other)
#define LLDP_TYPE_REM_SYS_CAP_ENA_REPEATER                  BIT_VALUE(VAL_lldpRemSysCapEnabled_repeater)
#define LLDP_TYPE_REM_SYS_CAP_ENA_BRIDGE                    BIT_VALUE(VAL_lldpRemSysCapEnabled_bridge)
#define LLDP_TYPE_REM_SYS_CAP_ENA_WLAN_ACCESS_POINT         BIT_VALUE(VAL_lldpRemSysCapEnabled_wlanAccessPoint)
#define LLDP_TYPE_REM_SYS_CAP_ENA_ROUTER                    BIT_VALUE(VAL_lldpRemSysCapEnabled_router)
#define LLDP_TYPE_REM_SYS_CAP_ENA_TELEPHONE                 BIT_VALUE(VAL_lldpRemSysCapEnabled_telephone)
#define LLDP_TYPE_REM_SYS_CAP_ENA_DOCSIS_CABLE_DEVICE       BIT_VALUE(VAL_lldpRemSysCapEnabled_docsisCableDevice)
#define LLDP_TYPE_REM_SYS_CAP_ENA_STATION_ONLY              BIT_VALUE(VAL_lldpRemSysCapEnabled_stationOnly)


/* LLDP gerneral return value */
#define LLDP_TYPE_RETURN_ERROR                              0
#define LLDP_TYPE_RETURN_OK                                 1
#define LLDP_TYPE_RETURN_MASTER_MODE_ERROR                  2
#define LLDP_TYPE_RETURN_TTL_EXCEED                         3
#define LLDP_TYPE_RETURN_TOO_MANY_NEIGHBOR                  4
#define LLDP_TYPE_RETURN_TX_DELAY_ERROR                     5

/* LLDP Init */
#define LLDP_TYPE_INIT_TX_FAIL                              0
#define LLDP_TYPE_INIT_TX_OK                                1
#define LLDP_TYPE_INIT_RX_FAIL                              0
#define LLDP_TYPE_INIT_RX_OK                                1

/* LLDP Sys Admin*/
#define LLDP_TYPE_SYS_ADMIN_ENABLE                          1
#define LLDP_TYPE_SYS_ADMIN_DISABLE                         0

/* LLDP Port Admin Status */
#if 0 /* Use MIB constants directly in other places */
#define LLDP_TYPE_ADMIN_STATUS_DISABLE                      VAL_lldpPortConfigAdminStatus_disabled
#define LLDP_TYPE_ADMIN_STATUS_TX                           VAL_lldpPortConfigAdminStatus_txOnly
#define LLDP_TYPE_ADMIN_STATUS_RX                           VAL_lldpPortConfigAdminStatus_rxOnly
#define LLDP_TYPE_ADMIN_STATUS_TX_RX                        VAL_lldpPortConfigAdminStatus_txAndRx
#endif

/* LLDPDU type */
#define LLDP_TYPE_NORMAL_LLDPDU                             0
#define LLDP_TYPE_SHUTDOWN_LLDPDU                           1

/* LLDP Events */
#define LLDP_TYPE_EVENT_NONE                                0
#define LLDP_TYPE_EVENT_TIMER                               BIT_5
#define LLDP_TYPE_EVENT_LLDPDURCVD                          BIT_6
#define LLDP_TYPE_EVENT_ENTER_TRANSITION                    BIT_7
#define LLDP_TYPE_EVENT_SOMETHING_CHANGED_LOCAL             BIT_8
#define LLDP_TYPE_EVENT_ALL                                 0x0F

/* LLDP Log */
#define LLDP_TYPE_LOG_ERR_LLDP_TASK_CREATE_TASK             0
#define LLDP_TYPE_LOG_FUN_LLDP_TASK_CREATE_TASK             1

#define LLDP_TYPE_REM_DB_NO_CHANGE                          TRUE
#define LLDP_TYPE_REM_DB_CHANGE                             FALSE

/* LLDP Validate TLV*/
#define LLDP_TYPE_VALIDATE_TLV_ERROR                        0
#define LLDP_TYPE_VALIDATE_TLV_OK                           1

/* basic tlv transfer */
#define LLDP_TYPE_TX_PORT_DESC_TLV                          BIT_VALUE(VAL_lldpPortConfigTLVsTxEnable_portDesc)
#define LLDP_TYPE_TX_SYS_NAME_TLV                           BIT_VALUE(VAL_lldpPortConfigTLVsTxEnable_sysName)
#define LLDP_TYPE_TX_SYS_DESC_TLV                           BIT_VALUE(VAL_lldpPortConfigTLVsTxEnable_sysDesc)
#define LLDP_TYPE_TX_SYS_CAP_TLV                            BIT_VALUE(VAL_lldpPortConfigTLVsTxEnable_sysCap)

/* adapted value */
#define LLDP_TYPE_OM_MAX_NUMBER_OF_REM_DATA                 (SYS_ADPT_TOTAL_NBR_OF_LPORT * LLDP_TYPE_MAX_NBR_OF_REM_DATA_ENTRY_PER_PORT)

/* LLDP Timer */
#define LLDP_TYPE_TIMER_TICKS2SEC                           SYS_BLD_TICKS_PER_SECOND
#define LLDP_TYPE_TIME_UNIT                                 SYS_BLD_TICKS_PER_SECOND

/* send packet priority*/
#define LLDP_TYPE_LLDPDU_TX_COS                             3

/* LLDP config variable default value */
#define LLDP_TYPE_DEFAULT_SYS_ADMIN_STATUS                  SYS_DFLT_LLDP_SYS_ADMIN_STATUS
#define LLDP_TYPE_DEFAULT_TX_INTERVAL                       SYS_DFLT_LLDP_TX_INTERVAL
#define LLDP_TYPE_DEFAULT_TX_HOLD_MUL                       SYS_DFLT_LLDP_TX_HOLD_MUL
#define LLDP_TYPE_DEFAULT_REINIT_DELAY                      SYS_DFLT_LLDP_REINIT_DELAY
#define LLDP_TYPE_DEFAULT_TX_DELAY                          SYS_DFLT_LLDP_TX_DELAY
#define LLDP_TYPE_DEFAULT_NOTIFY_INTERVAL                   SYS_DFLT_LLDP_NOTIFY_INTERVAL

#define LLDP_TYPE_DEFAULT_PORT_NOTIFY                       SYS_DFLT_LLDP_PORT_NOTIFY
#define LLDP_TYPE_DEFAULT_PORT_ADMIN_STATUS                 SYS_DFLT_LLDP_PORT_ADMIN_STATUS
#define LLDP_TYPE_DEFAULT_PORT_BASIC_TLV_TRANSFER_FLAG      SYS_DFLT_LLDP_PORT_BASIC_TLV_TX
#define LLDP_TYPE_DEFAULT_MAN_ADDR_TLV_TRANSFER_FLAG        SYS_DFLT_LLDP_MAN_ADDR_TLV_TX


/* 802.1 extensions*/
#define LLDP_TYPE_EXT_802DOT1                               SYS_CPNT_LLDP_EXT
#define LLDP_TYPE_MAX_VLAN_NAME_LEN                         32 + 1
#define LLDP_TYPE_MAX_REM_VLAN_NUM                          32
#define LLDP_TYPE_MAX_PROTOCOL_IDENT_LEN                    32
#define LLDP_TYPE_MAX_PROTOCOL_IDENT_NUM                    8
#define LLDP_TYPE_DEFAULT_XDOT1_PORT_VLAN_TX                SYS_DFLT_LLDP_XDOT1_PORT_VLAN_TX
#define LLDP_TYPE_DEFAULT_XDOT1_PROTOCOL_TX                 SYS_DFLT_LLDP_XDOT1_PROTOCOL_TX
#define LLDP_TYPE_DEFAULT_XDOT1_PROTO_VLAN_TX               SYS_DFLT_LLDP_XDOT1_PROTO_VLAN_TX
#define LLDP_TYPE_DEFAULT_XDOT1_VLAN_NAME_TX                SYS_DFLT_LLDP_XDOT1_VLAN_NAME_TX
#ifdef SYS_DFLT_LLDP_XDOT1_CN_TX
#define LLDP_TYPE_DEFAULT_XDOT1_CN_TX                       SYS_DFLT_LLDP_XDOT1_CN_TX
#else
#define LLDP_TYPE_DEFAULT_XDOT1_CN_TX                       FALSE
#endif

/* 802.3 extensions */
#define LLDP_TYPE_EXT_802DOT3                               SYS_CPNT_LLDP_EXT
#define LLDP_TYPE_DEFAULT_XDOT3_PORT_CONFIG                 SYS_DFLT_LLDP_XDOT3_PORT_CONFIG
#define LLDP_TYPE_XDOT3_MAC_PHY_TLV_TX                      BIT_VALUE(VAL_lldpXdot3PortConfigTLVsTxEnable_macPhyConfigStatus)
#define LLDP_TYPE_XDOT3_POWER_VIA_MDI_TX                    BIT_VALUE(VAL_lldpXdot3PortConfigTLVsTxEnable_powerViaMDI)
#define LLDP_TYPE_XDOT3_LINK_AGG_TX                         BIT_VALUE(VAL_lldpXdot3PortConfigTLVsTxEnable_linkAggregation)
#define LLDP_TYPE_XDOT3_MAX_FRAME_SIZE_TLV                  BIT_VALUE(VAL_lldpXdot3PortConfigTLVsTxEnable_maxFrameSize)

/* LLDP-MED */
#define LLDP_TYPE_MED                                       SYS_CPNT_LLDP_MED
#define LLDP_TYPE_DEFAULT_FAST_START_REPEAT_COUNT           SYS_DFLT_LLDP_FAST_START_REPEAT_COUNT
#define LLDP_TYPE_DEFAULT_MED_TX                            SYS_DFLT_LLDP_MED_TX
#define LLDP_TYPE_DEFAULT_MED_NOTIFY                        SYS_DFLT_LLDP_MED_NOTIFY

#define LLDP_TYPE_DEFAULT_MED_LOCATION_TYPE                 SYS_DFLT_LLDP_MED_LOCATION_TYPE
#define LLDP_TYPE_DEFAULT_MED_LOCATION_CA_CONNTRY           SYS_DFLT_LLDP_MED_LOCATION_CA_CONNTRY
#define LLDP_TYPE_DEFAULT_MED_LOCATION_CA_WHAT              SYS_DFLT_LLDP_MED_LOCATION_CA_WHAT

#define LLDP_TYPE_MED_CAP_TX                                BIT_VALUE(VAL_lldpXMedPortConfigTLVsTxEnable_capabilities)
#define LLDP_TYPE_MED_NETWORK_POLICY_TX                     BIT_VALUE(VAL_lldpXMedPortConfigTLVsTxEnable_networkPolicy)
#define LLDP_TYPE_MED_LOCATION_IDENT_TX                     BIT_VALUE(VAL_lldpXMedPortConfigTLVsTxEnable_location)
#define LLDP_TYPE_MED_EXT_PSE_TX                            BIT_VALUE(VAL_lldpXMedPortConfigTLVsTxEnable_extendedPSE)
#define LLDP_TYPE_MED_EXT_PD_TX                             BIT_VALUE(VAL_lldpXMedPortConfigTLVsTxEnable_extendedPD)
#define LLDP_TYPE_MED_INVENTORY_TX                          BIT_VALUE(VAL_lldpXMedPortConfigTLVsTxEnable_inventory)

#define LLDP_TYPE_MED_MAX_NETWORK_POLITY_TYPE               VAL_lldpXMedRemMediaPolicyAppType_videoSignaling


#define LLDP_TYPE_MAX_FAST_START_REPEAT_COUNT               10
#define LLDP_TYPE_MIN_FAST_START_REPEAT_COUNT               1

#define LLDP_TYPE_MAX_NUM_OF_LOCATION_INFO                  3
#define LLDP_TYPE_MAX_NUM_OF_CIVIC_ADDR_ELEMENT             128
#define LLDP_TYPE_MAX_CA_VALUE_LEN                          32
#define LLDP_TYPE_MAX_ELIN_NUMBER_LEN                       32
#define LLDP_TYPE_MAX_INV_ITEM_LEN                          32 + 1
#define LLDP_TYPE_MAX_HARDWARE_REV_LEN                      32
#define LLDP_TYPE_MAX_FIRMWARE_REV_LEN                      32
#define LLDP_TYPE_MAX_SOFTWARE_REV_LEN                      32
#define LLDP_TYPE_MAX_SERIAL_NUM_LEN                        32
#define LLDP_TYPE_MAX_MFG_NAME_LEN                          32
#define LLDP_TYPE_MAX_MODEL_NAME_LEN                        32
#define LLDP_TYPE_MAX_ASSET_ID_LEN                          32
#define LLDP_TYPE_MAX_LOCATION_ID_LEN                       256

#define LLDP_TYPE_FIRST_PPVID    0xffffffff /* used to get the first remote PPVID entry */

/* DCBX */
#define LLDP_TYPE_DCBX      SYS_CPNT_DCBX

#if (SYS_CPNT_DCBX == TRUE)
#define LLDP_TYPE_XDCBX_ETS_CON_TLV_TX      0x01
#define LLDP_TYPE_XDCBX_ETS_RECOM_TLV_TX    0x02
#define LLDP_TYPE_XDCBX_PFC_CON_TLV_TX      0x04
#define LLDP_TYPE_XDCBX_APP_PRI_TLV_TX      0x08
#define LLDP_TYPE_DEFAULT_XDCBX_PORT_CONFIG     0x07
#endif

#define LLDP_TYPE_REM_ETS_PRI_ASSIGN_TABLE_LEN      4
#define LLDP_TYPE_REM_ETS_TC_BANDWIDTH_TABLE_LEN    8
#define LLDP_TYPE_REM_ETS_TSA_ASSIGN_TABLE_LEN      8

#define LLDP_TYPE_MAX_REM_APP_PRIORITY_ENTRY_NUM    168 /* (511 - 5) / 3 */

/* for syslog direct call usage */
#define LLDP_TYPE_LOG_FUN_NO    1
#define LLDP_TYPE_LOG_ERR_NO    1

typedef struct
{
    L_MM_Mref_Handle_T  *mem_ref;
    UI16_T              type;
    UI16_T              lport;
    UI8_T               saddr[6];
    UI8_T               reserved[2];
} LLDP_TYPE_MSG_T; /* Message type, must be 16 bytes long */

typedef struct
{
    UI8_T       type;
    UI16_T      len;
    UI8_T       value[512];
} LLDP_TYPE_TLV_T;

typedef struct
{
    UI32_T  time_mark;
    UI32_T  lport;
    UI8_T   mac_addr[6];
    UI8_T   network_addr_subtype;
    UI8_T   network_addr[LLDP_TYPE_MAX_MANAGEMENT_ADDR_LENGTH];
    UI8_T   network_addr_len;
    UI32_T  network_addr_ifindex;
    BOOL_T  tel_exist;
} LLDP_TYPE_NotifyTelephone_T;

typedef struct
{
    UI32_T  rem_time_mark;
    UI32_T  rem_local_port_num;
    UI32_T  rem_index;
    UI8_T   rem_chassis_id_subtype;
    UI8_T   rem_chassis_id[LLDP_TYPE_MAX_CHASSIS_ID_LENGTH];
    UI32_T  rem_chassis_id_len;
    UI8_T   rem_device_class;
} LLDP_TYPE_NotifyMedRemData_T;

#if (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_3_2)
typedef struct
{
    UI32_T  time_mark;
    UI32_T  lport;
    UI8_T   power_type;
    UI8_T   power_source;
    UI8_T   power_priority;
    UI16_T  pd_requested_power;
    UI16_T  pse_allocated_power;
} LLDP_TYPE_NotifyDot3atInfo_T;
#endif

#if (SYS_CPNT_CN == TRUE)
typedef struct
{
    UI32_T  time_mark;
    UI32_T  lport;
    UI32_T  neighbor_num;
    UI8_T   cnpv_indicators;;
    UI8_T   ready_indicators;
} LLDP_TYPE_NotifyCnRemoteChange_T;
#endif

typedef struct
{
    UI32_T  time_mark;
    UI32_T  lport;
} LLDP_TYPE_NotifyRemChange_T;

/* LLDP tlv type enumeration*/
enum LLDP_TYPE_SUPPORT_TLV_E
{
    LLDP_TYPE_END_OF_LLDPDU_TLV = 0,
    LLDP_TYPE_CHASSIS_ID_TLV,
    LLDP_TYPE_PORT_ID_TLV,
    LLDP_TYPE_TIME_TO_LIVE_TLV,
    LLDP_TYPE_PORT_DESC_TLV,
    LLDP_TYPE_SYS_NAME_TLV,
    LLDP_TYPE_SYS_DESC_TLV,
    LLDP_TYPE_SYS_CAP_TLV,
    LLDP_TYPE_MAN_ADDR_TLV,
    LLDP_TYPE_ORG_SPEC_TLV = 127,
    LLDP_TYPE_UNKNOWN_TLV,

    LLDP_TYPE_MAX_SUPPORT_TLV
};

enum LLDP_TYPE_MED_TLV_E
{
    LLDP_TYPE_MED_CAP_TLV = 1,
    LLDP_TYPE_MED_NETWORK_POLICY_TLV,
    LLDP_TYPE_MED_LOCATION_IDENT_TLV,
    LLDP_TYPE_MED_EXT_POWER_VIA_MDI_TLV,
    LLDP_TYPE_MED_INV_HARDWARE_REVISION_TLV,
    LLDP_TYPE_MED_INV_FIRMWARE_REVISION_TLV,
    LLDP_TYPE_MED_INV_SOFTWARE_REVISION_TLV,
    LLDP_TYPE_MED_INV_SERIAL_NUM_TLV,
    LLDP_TYPE_MED_INV_MANUFACTURER_NAME_TLV,
    LLDP_TYPE_MED_INV_MODEL_NAME_TLV,
    LLDP_TYPE_MED_ASSET_ID_TLV
};

/* LLDP chassis id subtype enumeration */
enum LLDP_TYPE_CHASSIS_ID_SUBTYPE_E
{
    LLDP_TYPE_CHASSIS_ID_SUBTYPE_CHASSIS = 1,
    LLDP_TYPE_CHASSIS_ID_SUBTYPE_IFALIAS,
    LLDP_TYPE_CHASSIS_ID_SUBTYPE_PORT,
    LLDP_TYPE_CHASSIS_ID_SUBTYPE_MAC_ADDR,
    LLDP_TYPE_CHASSIS_ID_SUBTYPE_NETWORK_ADDR,
    LLDP_TYPE_CHASSIS_ID_SUBTYPE_IFNAME,
    LLDP_TYPE_CHASSIS_ID_SUBTYPE_LOCAL,

    LLDP_TYPE_CHASSIS_ID_SUBTYPE_RESERVED
};

/* LLDP port id subtype enumeration*/
enum LLDP_TYPE_PORT_ID_SUBTYPE_E
{
    LLDP_TYPE_PORT_ID_SUBTYPE_IFALIAS = 1,
    LLDP_TYPE_PORT_ID_SUBTYPE_PORT,
    LLDP_TYPE_PORT_ID_SUBTYPE_MAC_ADDR,
    LLDP_TYPE_PORT_ID_SUBTYPE_NETWORK_ADDR,
    LLDP_TYPE_PORT_ID_SUBTYPE_IFNAME,
    LLDP_TYPE_PORT_ID_SUBTYPE_AGENT_CIRCUIT_ID,
    LLDP_TYPE_PORT_ID_SUBTYPE_LOCAL,

    LLDP_TYPE_PORT_ID_SUBTYPE_RESERVED
};

enum LLDP_TYPE_DOT1_TLV_E
{
    LLDP_TYPE_DOT1_PORT_VLAN_ID_TLV = 1,
    LLDP_TYPE_DOT1_PORT_AND_PROTO_VLAN_ID_TLV,
    LLDP_TYPE_DOT1_VLAN_NAME_TLV,
    LLDP_TYPE_DOT1_PTOTO_IDENT_TLV,
    LLDP_TYPE_DOT1_CN_TLV = 8,
};

enum LLDP_TYPE_DOT3_TLV_E
{
    LLDP_TYPE_DOT3_MAC_PHY_TLV = 1,
    LLDP_TYPE_DOT3_POWER_VIA_MDI_TLV,
    LLDP_TYPE_DOT3_LINK_AGGREGATE_TLV,
    LLDP_TYPE_DOT3_MAX_FRAME_SIZE_TLV
};

enum LLDP_TYPE_DCBX_TLV_E
{
    LLDP_TYPE_DCBX_ETS_CONFIG_TLV = 9,
    LLDP_TYPE_DCBX_ETS_RECOMMEND_TLV,
    LLDP_TYPE_DCBX_PFC_CONFIG_TLV,
    LLDP_TYPE_DCBX_APP_PRI_TLV
};


/* trace id definition when using L_MM to allocate buffer
 */
enum
{
    LLDP_TYPE_TRACE_ID_POM_GETREMOTESYSTEMDATA,
};

/*=============================================================================
 * Moved from lldp_mgr.h
 *=============================================================================
 */

/*
LLDP_MGR_SysConfigEntry_T                                                           ()
*/
typedef struct
{
    /* All R/W */
    UI32_T  message_tx_interval;            /* 10.5.3.3 */
    UI32_T  message_tx_hold_multiplier;     /* 10.5.3.3 */
    UI32_T  reinit_delay;                   /* 10.5.3.3 */
    UI32_T  tx_delay;                       /* 10.5.3.3 */
    UI32_T  notification_interval;
}LLDP_MGR_SysConfigEntry_T;

/*
LLDP_MGR_RemoteSystemData_T                                                         ()
*/
typedef struct
{
    /* All R/O */
    UI32_T  rem_time_mark;                                          /* key */
    UI32_T  rem_local_port_num;                                     /* key */
    UI32_T  rem_index;                                              /* key */
    UI8_T   rem_chassis_id_subtype;                                 /* 9.5.2.2 */
    UI8_T   rem_chassis_id[LLDP_TYPE_MAX_CHASSIS_ID_LENGTH];        /* 9.2.5.3 */
    UI32_T  rem_chassis_id_len;
    UI8_T   rem_port_id_subtype;                                    /* 9.5.3.2 */
    UI8_T   rem_port_id[LLDP_TYPE_MAX_PORT_ID_LENGTH];              /* 9.5.3.3 */
    UI32_T  rem_port_id_len;
    UI32_T  rem_ttl;
    UI8_T   rem_port_desc[LLDP_TYPE_MAX_PORT_DESC_LENGTH];          /* 9.5.5.2 */
    UI32_T  rem_port_desc_len;
    UI8_T   rem_sys_name[LLDP_TYPE_MAX_SYS_NAME_LENGTH];            /* 9.5.6.2 */
    UI32_T  rem_sys_name_len;
    UI8_T   rem_sys_desc[LLDP_TYPE_MAX_SYS_DESC_LENGTH];            /* 9.5.7.2 */
    UI32_T  rem_sys_desc_len;
    UI16_T  rem_sys_cap_supported;                                  /* 9.5.8.1 */
    UI16_T  rem_sys_cap_enabled;                                    /* 9.5.8.2 */
}LLDP_MGR_RemoteSystemData_T;

/*
LLDP_MGR_RemoteManagementAddrEntry_T                                                ()
*/
typedef struct
{
    UI32_T  rem_time_mark;                                          /* key */
    UI32_T  rem_local_port_num;                                     /* key */
    UI32_T  rem_index;                                              /* key */
    UI8_T   rem_man_addr_subtype;                                   /* key */   /* 9.5.9.3 */
    UI8_T   rem_man_addr[LLDP_TYPE_MAX_MANAGEMENT_ADDR_LENGTH];        /* key */   /* 9.5.9.4 */
    UI32_T  rem_man_addr_len;
    UI8_T   rem_man_addr_if_subtype;                                            /* 9.5.9.5 */
    UI32_T  rem_man_addr_if_id;                                                 /* 9.5.9.6 */
    UI8_T   rem_man_addr_oid[LLDP_TYPE_MAX_MANAGEMENT_ADDR_OID_LENGTH];            /* 9.5.9.8 */
    UI32_T  rem_man_addr_oid_len;
}LLDP_MGR_RemoteManagementAddrEntry_T;

/* lldp extensions 802.1 */
typedef struct
{
    UI32_T  lport;                      /* key */
    UI8_T   port_vlan_tx_enable;        /* r/w */ /* VAL_lldpXdot1ConfigPortVlanTxEnable_true/false */
    UI8_T   vlan_name_tx_enable;        /* r/w */ /* VAL_lldpXdot1ConfigVlanNameTxEnable_true/false */
    UI8_T   proto_vlan_tx_enable;       /* r/w */ /* VAL_lldpXdot1ConfigProtoVlanTxEnable_true/false */
    UI8_T   protocol_tx_enable;         /* r/w */ /* VAL_lldpXdot1ConfigProtocolTxEnable_true/false */
    BOOL_T  cn_tx_enable;
    BOOL_T  port_vlan_tx_changed;       /* for get running */
    BOOL_T  vlan_name_tx_changed;       /* for get running */
    BOOL_T  proto_vlan_tx_changed;      /* for get running */
    BOOL_T  protocol_tx_changed;        /* for get running */
    BOOL_T  cn_tx_changed;              /* for get running */
}LLDP_MGR_Xdot1ConfigEntry_T;

typedef struct
{
    UI32_T  rem_time_mark;          /* key */
    UI32_T  rem_local_port_num;     /* key */
    UI32_T  rem_index;              /* key */
    UI32_T  rem_proto_vlan_id;      /* key */
    BOOL_T  rem_proto_vlan_supported;
    BOOL_T  rem_proto_vlan_enabled;
}LLDP_MGR_Xdot1RemProtoVlanEntry_T;

typedef struct
{
    UI32_T  rem_time_mark;          /* key */
    UI32_T  rem_local_port_num;     /* key */
    UI32_T  rem_index;              /* key */
    UI32_T  rem_vlan_id;            /* key */
    UI8_T   rem_vlan_name[LLDP_TYPE_MAX_VLAN_NAME_LEN];
    UI32_T  rem_vlan_name_len;
}LLDP_MGR_Xdot1RemVlanNameEntry_T;

typedef struct
{
    UI32_T  rem_time_mark;          /* key */
    UI32_T  rem_local_port_num;     /* key */
    UI32_T  rem_index;              /* key */
    UI32_T  rem_port_vlan_id;
}LLDP_MGR_Xdot1RemEntry_T;

typedef struct
{
    UI32_T  rem_time_mark;          /* key */
    UI32_T  rem_local_port_num;     /* key */
    UI32_T  rem_index;              /* key */
    UI32_T  rem_protocol_index;     /* key */
    UI8_T   rem_protocol_id[LLDP_TYPE_MAX_PROTOCOL_IDENT_LEN];
    UI32_T  rem_protocol_id_len;
}LLDP_MGR_Xdot1RemProtocolEntry_T;

typedef struct
{
    UI32_T  rem_time_mark;          /* key */
    UI32_T  rem_local_port_num;     /* key */
    UI32_T  rem_index;              /* key */
    UI8_T   rem_cnpv_indicators;
    UI8_T   rem_ready_indicators;
}LLDP_MGR_Xdot1RemCnEntry_T;

/* lldp extension 802.3 */
typedef struct
{
    UI32_T  lport;              /* key */
    UI8_T   tlvs_tx_enable;
    UI8_T   tlvs_tx_changed;    /* for get running */
    BOOL_T  mac_phy_tlv_enabled;
    BOOL_T  power_via_mdi_tlv_enabled;
    BOOL_T  link_agg_tlv_enabled;
    BOOL_T  max_frame_size_tlv_enabled;
}LLDP_MGR_Xdot3PortConfigEntry_T;

typedef struct
{
    UI32_T  lport;      /* key */
    UI32_T  loc_max_frame_size;
}LLDP_MGR_Xdot3LocMaxFrameSizeEntry_T;

typedef struct
{
    UI32_T  rem_time_mark;          /* key */
    UI32_T  rem_local_port_num;     /* key */
    UI32_T  rem_index;              /* key */
    UI8_T   rem_port_auto_neg_supported;
    UI8_T   rem_port_auto_neg_enable;
    UI16_T  rem_port_auto_neg_adv_cap;
    UI16_T  rem_port_oper_mau_type;
}LLDP_MGR_Xdot3RemPortEntry_T;

typedef struct
{
    UI32_T  rem_time_mark;          /* key */
    UI32_T  rem_local_port_num;     /* key */
    UI32_T  rem_index;              /* key */
    UI8_T   rem_power_port_class;
    UI8_T   rem_power_mdi_supported;
    UI8_T   rem_power_mdi_enabled;
    UI8_T   rem_power_pair_controlable;
    UI8_T   rem_power_pairs;
    UI8_T   rem_power_class;
}LLDP_MGR_Xdot3RemPowerEntry_T;

typedef struct
{
    UI32_T  rem_time_mark;          /* key */
    UI32_T  rem_local_port_num;     /* key */
    UI32_T  rem_index;              /* key */
    UI8_T   rem_link_agg_status;
    UI32_T  rem_link_agg_port_id;
}LLDP_MGR_Xdot3RemLinkAggEntry_T;

typedef struct
{
    UI32_T  rem_time_mark;          /* key */
    UI32_T  rem_local_port_num;     /* key */
    UI32_T  rem_index;              /* key */
    UI32_T  rem_max_frame_size;
}LLDP_MGR_Xdot3RemMaxFrameSizeEntry_T;

typedef struct
{
    UI8_T   lldp_xmed_loc_device_class;
    UI8_T   lldp_xmed_fast_start_repeat_count;  /* r/w */

} LLDP_MGR_XMedConfig_T;

typedef struct
{
    UI32_T  rem_time_mark;          /* key */
    UI32_T  rem_local_port_num;     /* key */
    UI32_T  rem_index;              /* key */
    UI16_T  rem_cap_supported;
    UI16_T  rem_cap_current;
    UI8_T   rem_device_class;

}LLDP_MGR_XMedRemCapEntry_T;

typedef struct
{
    UI32_T  rem_time_mark;          /* key */
    UI32_T  rem_local_port_num;     /* key */
    UI32_T  rem_index;              /* key */
    UI8_T   rem_app_type;           /* key */
    UI16_T  rem_vid;
    UI32_T  rem_priority;
    UI32_T  rem_dscp;
    UI8_T   rem_unknown;
    UI8_T   rem_tagged;
}LLDP_MGR_XMedRemMediaPolicyEntry_T;

typedef struct
{
    UI32_T  rem_time_mark;          /* key */
    UI32_T  rem_local_port_num;     /* key */
    UI32_T  rem_index;              /* key */
    UI8_T   rem_hardware_rev[LLDP_TYPE_MAX_HARDWARE_REV_LEN];
    UI32_T  rem_hardware_rev_len;
    UI8_T   rem_firmware_rev[LLDP_TYPE_MAX_FIRMWARE_REV_LEN];
    UI32_T  rem_firmware_rev_len;
    UI8_T   rem_software_rev[LLDP_TYPE_MAX_SOFTWARE_REV_LEN];
    UI32_T  rem_software_rev_len;
    UI8_T   rem_serial_num[LLDP_TYPE_MAX_SERIAL_NUM_LEN];
    UI32_T  rem_serial_num_len;
    UI8_T   rem_mfg_name[LLDP_TYPE_MAX_MFG_NAME_LEN];
    UI32_T  rem_mfg_name_len;
    UI8_T   rem_model_name[LLDP_TYPE_MAX_MODEL_NAME_LEN];
    UI32_T  rem_model_name_len;
    UI8_T   rem_asset_id[LLDP_TYPE_MAX_ASSET_ID_LEN];
    UI32_T  rem_asset_id_len;
}LLDP_MGR_XMedRemInventoryEntry_T;

typedef struct
{
    UI32_T  rem_time_mark;          /* key */
    UI32_T  rem_local_port_num;     /* key */
    UI32_T  rem_index;              /* key */
    UI8_T   rem_location_subtype;   /* key */
    UI8_T   rem_location_info[LLDP_TYPE_MAX_LOCATION_ID_LEN];
    UI32_T  rem_location_info_len;
}LLDP_MGR_XMedRemLocationEntry_T;

typedef struct
{
    UI32_T  rem_time_mark;          /* key */
    UI32_T  rem_local_port_num;     /* key */
    UI32_T  rem_index;              /* key */
    UI8_T   rem_poe_device_type;

}LLDP_MGR_XMedRemPoeEntry_T;

typedef struct
{
    UI32_T  rem_time_mark;          /* key */
    UI32_T  rem_local_port_num;     /* key */
    UI32_T  rem_index;              /* key */
    UI32_T  rem_pse_power_av;
    UI8_T   rem_pse_power_source;
    UI8_T   rem_pse_power_priority;
}LLDP_MGR_XMedRemPoePseEntry_T;

typedef struct
{
    UI32_T  rem_time_mark;          /* key */
    UI32_T  rem_local_port_num;     /* key */
    UI32_T  rem_index;              /* key */
    UI32_T  rem_pd_power_req;
    UI8_T   rem_pd_power_source;
    UI8_T   rem_pd_power_priority;
}LLDP_MGR_XMedRemPoePdEntry_T;

typedef struct
{
    UI32_T  rem_time_mark;          /* key */
    UI32_T  rem_local_port_num;     /* key */
    UI32_T  rem_index;              /* key */
    BOOL_T  rem_config_rcvd;
    BOOL_T  rem_recommend_rcvd;
    BOOL_T  rem_config_willing;
    BOOL_T  rem_config_cbs;
    UI8_T   rem_config_max_tc;
    UI8_T   rem_config_pri_assign_table[LLDP_TYPE_REM_ETS_PRI_ASSIGN_TABLE_LEN];
    UI8_T   rem_config_tc_bandwidth_table[LLDP_TYPE_REM_ETS_TC_BANDWIDTH_TABLE_LEN];
    UI8_T   rem_config_tsa_assign_table[LLDP_TYPE_REM_ETS_TSA_ASSIGN_TABLE_LEN];
    UI8_T   rem_recommend_pri_assign_table[LLDP_TYPE_REM_ETS_PRI_ASSIGN_TABLE_LEN];
    UI8_T   rem_recommend_tc_bandwidth_table[LLDP_TYPE_REM_ETS_TC_BANDWIDTH_TABLE_LEN];
    UI8_T   rem_recommend_tsa_assign_table[LLDP_TYPE_REM_ETS_TSA_ASSIGN_TABLE_LEN];
} LLDP_TYPE_DcbxRemEtsEntry_T;

typedef struct
{
    UI32_T  rem_time_mark;          /* key */
    UI32_T  rem_local_port_num;     /* key */
    UI32_T  rem_index;              /* key */
    UI8_T   rem_mac[6];
    BOOL_T  rem_willing;
    BOOL_T  rem_mbc;
    UI8_T   rem_cap;
    UI8_T   rem_enable;
} LLDP_TYPE_DcbxRemPfcEntry_T;

typedef struct
{
    UI32_T  rem_time_mark;          /* key */
    UI32_T  rem_local_port_num;     /* key */
    UI32_T  rem_index;              /* key */
    UI8_T   rem_priority;
    UI8_T   rem_sel;
    UI16_T  rem_protocol_id;
} LLDP_TYPE_DcbxRemAppPriEntry_T;

typedef struct
{
    UI32_T  time_mark;
    UI32_T  lport;
    UI32_T  index;
    BOOL_T  is_delete;
    BOOL_T  rem_recommend_rcvd;
    BOOL_T  rem_config_willing;
    BOOL_T  rem_config_cbs;
    UI8_T   rem_config_max_tc;
    UI8_T   rem_config_pri_assign_table[LLDP_TYPE_REM_ETS_PRI_ASSIGN_TABLE_LEN];
    UI8_T   rem_config_tc_bandwidth_table[LLDP_TYPE_REM_ETS_TC_BANDWIDTH_TABLE_LEN];
    UI8_T   rem_config_tsa_assign_table[LLDP_TYPE_REM_ETS_TSA_ASSIGN_TABLE_LEN];
    UI8_T   rem_recommend_pri_assign_table[LLDP_TYPE_REM_ETS_PRI_ASSIGN_TABLE_LEN];
    UI8_T   rem_recommend_tc_bandwidth_table[LLDP_TYPE_REM_ETS_TC_BANDWIDTH_TABLE_LEN];
    UI8_T   rem_recommend_tsa_assign_table[LLDP_TYPE_REM_ETS_TSA_ASSIGN_TABLE_LEN];
} LLDP_TYPE_NotifyDcbxEtsInfo_T;

typedef struct
{
    UI32_T  time_mark;
    UI32_T  lport;
    UI32_T  index;
    BOOL_T  is_delete;
    UI8_T   rem_mac[6];
    BOOL_T  rem_willing;
    BOOL_T  rem_mbc;
    UI8_T   rem_cap;
    UI8_T   rem_enable;
} LLDP_TYPE_NotifyDcbxPfcInfo_T;

typedef struct
{
    UI32_T  lport;                      /* key */
    UI8_T   tlvs_tx_enable;
    UI8_T   tlvs_tx_changed;    /* for get running */
    BOOL_T  ets_config_tlv_enable;
    BOOL_T  ets_recommend_tlv_enable;
    BOOL_T  pfc_config_tlv_enable;
    BOOL_T  app_priority_tlv_enable;
}LLDP_TYPE_XdcbxPortConfigEntry_T;

#endif /* #ifndef _LLDP_TYPE_H */
