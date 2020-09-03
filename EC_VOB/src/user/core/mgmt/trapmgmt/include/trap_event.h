/* =====================================================================================
 * FILE NAME: TRAP_EVENT.h
 *
 * ABSTRACT:  This file contains the defined datatypes for all trap event.
 *
 * MODIFICATION HISOTRY:
 *
 * MODIFIER        DATE        DESCRIPTION
 * -------------------------------------------------------------------------------------
 * amytu        07-11-2002     First Create
 *
 * -------------------------------------------------------------------------------------
 * Copyright(C)        Accton Techonology Corporation 2002
 * =====================================================================================
 */
#ifndef TRAP_EVENT_H
#define TRAP_EVENT_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_adpt.h"
#include "sys_cpnt.h"

#include "l_inet.h"

#include "leaf_es3626a.h"


/* NAMING CONSTANT DECLARATIONS
 */

/* TYPE DECLARATION
 */

typedef enum
{
    TRAP_EVENT_ACTION_NO_LOG_AND_TRAP   = 0x00L,
    TRAP_EVENT_ACTION_LOGGED            = 0x01L,
    TRAP_EVENT_ACTION_TRAP_TRANSMIT     = 0x02L,

} TRAP_EVENT_ACTION_E;

typedef enum
{
    TRAP_EVENT_SEND_TRAP_OPTION_DEFAULT = 0,
    TRAP_EVENT_SEND_TRAP_OPTION_LOG_AND_TRAP = 1,
    TRAP_EVENT_SEND_TRAP_OPTION_LOG_ONLY = 2,
    TRAP_EVENT_SEND_TRAP_OPTION_TRAP_ONLY = 3
} TRAP_EVENT_SendTrapOption_E;

typedef enum
{
    /* five of the Six Early Traps (no enterprise)
     */
    TRAP_EVENT_COLD_START = 0,
    TRAP_EVENT_WARM_START = 1,
    TRAP_EVENT_LINK_DOWN = 2,
    TRAP_EVENT_LINK_UP = 3,
    TRAP_EVENT_AUTHENTICATION_FAILURE = 4,

    /* Spanning Tree Protocol MIB
     */
    TRAP_EVENT_NEW_ROOT = 5,
    TRAP_EVENT_TOPOLOGY_CHANGE = 6,

    /* RMON MIB
     */
    TRAP_EVENT_RISING_ALARM = 7,
    TRAP_EVENT_FALLING_ALARM = 8,

    /* Accton private MIB
     */
    TRAP_EVENT_SW_POWER_STATUS_CHANGE_TRAP=9,   /* swPowerStatusChangeTrap(1) */
    TRAP_EVENT_PORT_SECURITY_TRAP=10,           /* swPortSecurityTrap(36) */
    TRAP_EVENT_LOOPBACK_TEST_FAILURE_TRAP=11,   /* swLoopbackTestFailureTrap(39) */
    TRAP_EVENT_FAN_FAILURE=12,                  /* swFanFailureTrap(17) */
    TRAP_EVENT_FAN_RECOVER=13,                  /* swFanRecoverTrap(18) */
    TRAP_EVENT_IPFILTER_REJECT_TRAP=14,         /* swLoopbackTestFailureTrap(40) */
    TRAP_EVENT_SMTP_CONN_FAILURE_TRAP=16,       /* swSmtpConnFailureTrap(41) */

    TRAP_EVENT_PETH_PSE_PORT_ON_OFF_TRAP=17,    /* pethPsePortOnOffNotification(43) */
        /*we don't support pethPsePortPowerMaintenanceStatusTrap now phoebe 2003.4.2*/

    TRAP_EVENT_PETH_MAIN_POWER_USAGE_ON_TRAP=18,    /* pethMainPowerUsageOnNotification(45) */
    TRAP_EVENT_PETH_MAIN_POWER_USAGE_OFF_TRAP=19,   /* pethMainPowerUsageOffNotification(46) */

    TRAP_EVENT_VDSL_PERF_LOFS_THRESH=34,        /* vdslPerfLofsThreshNotification(48) */
    TRAP_EVENT_VDSL_PERF_LOSS_THRESH=35,        /* vdslPerfLossThreshNotification(49) */
    TRAP_EVENT_VDSL_PERF_LPRS_THRESH=36,        /* vdslPerfLprsThreshNotification(50) */
    TRAP_EVENT_VDSL_PERF_LOLS_THRESH=37,        /* vdslPerfLolsThreshNotification(51) */
    TRAP_EVENT_VDSL_PERF_ESS_THRESH=38,         /* vdslPerfESsThreshNotification(52) */
    TRAP_EVENT_VDSL_PERF_SESS_THRESH=39,        /* vdslPerfSESsThreshNotification(53) */
    TRAP_EVENT_VDSL_PERF_UASS_THRESH=40,        /* vdslPerfUASsThreshNotification(54) */
    TRAP_EVENT_MAIN_BOARD_VER_MISMATCH=42,      /* swMainBoardVerMismatchNotificaiton(56) */
    TRAP_EVENT_MODULE_VER_MISMATCH=43,          /* swModuleVerMismatchNotificaiton(57) */
    TRAP_EVENT_THERMAL_RISING=44,               /* swThermalRisingNotification(58) */
    TRAP_EVENT_THERMAL_FALLING=45,              /* swThermalFallingNotification(59) */
    TRAP_EVENT_MODULE_INSERTION=46,             /* swModuleInsertionNotificaiton(60) */
    TRAP_EVENT_MODULE_REMOVAL=47,               /* swModuleRemovalNotificaiton(61) */
    TRAP_EVENT_TCN=49,                          /* tcnTrap(63) */

#if (SYS_CPNT_SNMP_MIB_STYLE == SYS_CPNT_SNMP_MIB_STYLE_32)
    /* 3Com private MIB
     */
    TRAP_EVENT_SECURE_ADDRESS_LEARNED=50,               /* 71 */
    TRAP_EVENT_SECURE_VIOLATION2=51,                    /* 85 */
    TRAP_EVENT_SECURE_LOGIN_FAILURE=52,                 /* 91 */
    TRAP_EVENT_SECURE_LOGON=53,                         /* 93 */
    TRAP_EVENT_SECURE_LOGOFF=54,                        /* 94 */
    TRAP_EVENT_TESTTRAP = 55,                           /* 64 */
    TRAP_EVENT_IP_FORWARDING_CHANGED_BY_MSTPTRAP = 56,  /* 65 */
        /* Accton has a similar: swIpForwardingChangedByMstpTrap(64) */
#endif

    /* LLDP MIB
     */
    TRAP_EVENT_LLDP_REM_TABLES_CHANGED = 57,

    /* Accton private MIB
     */
    TRAP_EVENT_LINK_STATE = 58, /* steven.jiang add for send more link up/down at once */

    TRAP_EVENT_MAJOR_ALARM = 59,                /* swMajorAlarm(78) */
    TRAP_EVENT_MINOR_ALARM = 60,                /* swMinorAlarm(79) */

#if (SYS_CPNT_ATC_STORM == TRUE)
    TRAP_EVENT_BCAST_STORM_ALARM_FIRE = 70,     /* swAtcBcastStormAlarmFireTrap(70) ... */
    TRAP_EVENT_BCAST_STORM_ALARM_CLEAR = 71,
    TRAP_EVENT_BCAST_STORM_TC_APPLY = 72,
    TRAP_EVENT_BCAST_STORM_TC_RELEASE = 73,

    TRAP_EVENT_MCAST_STORM_ALARM_FIRE = 74,
    TRAP_EVENT_MCAST_STORM_ALARM_CLEAR = 75,
    TRAP_EVENT_MCAST_STORM_TC_APPLY = 76,
    TRAP_EVENT_MCAST_STORM_TC_RELEASE = 77,     /* ... swAtcMcastStormTcReleaseTrap(77) */
#endif

    TRAP_EVENT_POWER_UNIT_UNMOUNT = 80,         /* swPowerUnitUnmount(80) ... */
    TRAP_EVENT_POWER_SOURCE_ON = 81,
    TRAP_EVENT_POWER_SOURCE_OFF = 82,
    TRAP_EVENT_THERMAL_RISING_RECOVERY = 83,
    TRAP_EVENT_SFP_MODULE_INSERTION_MESSAGE_FAILED = 84,
    TRAP_EVENT_MAJOR_ALARM_RECOVERY = 85,       /* ... swEqpAlarmRecovery(85) or swMajorAlarmRecovery(85) */

    TRAP_EVENT_SW_ALARM_INPUT = 90,             /* swAlarmInput(90) */

#if (SYS_CPNT_STP_BPDU_GUARD == TRUE)
    TRAP_EVENT_STP_BPDU_GUARD_PORT_SHUTDOWN_TRAP = 91, /* stpBpduGuardPortShutdownTrap(91) */
#endif

#if(SYS_CPNT_NETACCESS_LINK_DETECTION == TRUE)
    TRAP_EVENT_NETWORK_ACCESS_PORT_LINK_DETECTION_TRAP = 96,
                                                /* networkAccessPortLinkDetectionTrap(96) */
#endif

#if (SYS_CPNT_CFM == TRUE)
    TRAP_EVENT_CFM_MEP_UP = 97,                 /* dot1agCfmMepUpTrap(97) ... */
    TRAP_EVENT_CFM_MEP_DOWN = 98,
    TRAP_EVENT_CFM_CONFIG_FAIL = 99,
    TRAP_EVENT_CFM_LOOP_FIND = 100,
    TRAP_EVENT_CFM_MEP_UNKNOWN = 101,
    TRAP_EVENT_CFM_MEP_MISSING = 102,
    TRAP_EVENT_CFM_MA_UP = 103,                 /* ... dot1agCfmMaUpTrap(103) */
#endif  /* #if(SYS_CPNT_CFM==TRUE) */

#if (SYS_CPNT_XFER_AUTO_UPGRADE == TRUE)
    TRAP_EVENT_XFER_AUTO_UPGRADE = 104,         /* autoUpgradeTrap(104) */
#endif

#if (SYS_CPNT_SYSMGMT_RESMIB == TRUE)
    TRAP_EVENT_CPU_RAISE_TRAP=107,              /* swCpuUtiRisingNotification(107) */
    TRAP_EVENT_CPU_FALLING_TRAP=108,            /* swCpuUtiFallingNotification(108) */

    TRAP_EVENT_MEM_RAISE_TRAP=109,              /* swMemoryUtiRisingThresholdNotification(109) */
    TRAP_EVENT_MEM_FALLING_TRAP=110,            /* swMemoryUtiFallingThresholdNotification(110) */
#endif /* #if (SYS_CPNT_SYSMGMT_RESMIB == TRUE) */

    TRAP_EVENT_IPFILTER_INET_REJECT_TRAP = 111, /* swIpFilterInetRejectTrap(111) */

    /* DHCP client sends a trap when receiving a packet from a rogue server. */
    TRAP_EVENT_DHCP_ROGUE_SERVER_ATTACK = 114,  /* dhcpRogueServerAttackTrap(114) */

#if (SYS_CPNT_AMTR_MAC_NOTIFY == TRUE)
    TRAP_EVENT_MAC_NOTIFY = 138,
#endif

#if (SYS_CPNT_SFP_DDM_ALARMWARN_TRAP == TRUE)
    TRAP_EVENT_SFP_TEMP_HIGH_ALARM = 150,       /* all mapped to 170; conflicts with superset */
    TRAP_EVENT_SFP_TEMP_LOW_ALARM = 151,
    TRAP_EVENT_SFP_TEMP_HIGH_WARNING = 152,
    TRAP_EVENT_SFP_TEMP_LOW_WARNING = 153,

    TRAP_EVENT_SFP_VOLTAGE_HIGH_ALARM = 154,
    TRAP_EVENT_SFP_VOLTAGE_LOW_ALARM = 155,
    TRAP_EVENT_SFP_VOLTAGE_HIGH_WARNING = 156,
    TRAP_EVENT_SFP_VOLTAGE_LOW_WARNING = 157,

    TRAP_EVENT_SFP_CURRENT_HIGH_ALARM = 158,
    TRAP_EVENT_SFP_CURRENT_LOW_ALARM = 159,
    TRAP_EVENT_SFP_CURRENT_HIGH_WARNING = 160,
    TRAP_EVENT_SFP_CURRENT_LOW_WARNING = 161,

    TRAP_EVENT_SFP_TX_POWER_HIGH_ALARM = 162,
    TRAP_EVENT_SFP_TX_POWER_LOW_ALARM = 163,
    TRAP_EVENT_SFP_TX_POWER_HIGH_WARNING = 164,
    TRAP_EVENT_SFP_TX_POWER_LOW_WARNING = 165,

    TRAP_EVENT_SFP_RX_POWER_HIGH_ALARM = 166,
    TRAP_EVENT_SFP_RX_POWER_LOW_ALARM = 167,
    TRAP_EVENT_SFP_RX_POWER_HIGH_WARNING = 168,
    TRAP_EVENT_SFP_RX_POWER_LOW_WARNING = 169,

    TRAP_EVENT_SFP_TEMP_ALARMWARN_CEASE = 170,
    TRAP_EVENT_SFP_VOLTAGE_ALARMWARN_CEASE = 171,
    TRAP_EVENT_SFP_CURRENT_ALARMWARN_CEASE = 172,
    TRAP_EVENT_SFP_TX_POWER_ALARMWARN_CEASE = 173,
    TRAP_EVENT_SFP_RX_POWER_ALARMWARN_CEASE = 174,
#endif

    TRAP_EVENT_DYING_GASP = 175,

    TRAP_EVENT_XSTP_PORT_STATE_CHANGE = 176,    /* log, but no trap */

    TRAP_EVENT_XSTP_ROOT_BRIDGE_CHANGED  = 195,

#if(SYS_CPNT_SYNCE == TRUE)
    TRAP_EVENT_SYNCE_SSM_RX     = 196,
    TRAP_EVENT_SYNCE_CLOCK_SRC  = 197,
#endif
    TRAP_EVENT_SW_ACTIVE_POWER_CHANGE_TRAP = 198, /* swActivePowerChangeTrap(198) */

    TRAP_EVENT_USERAUTH_AUTHENTICATION_FAILURE = 199,
    TRAP_EVENT_USERAUTH_AUTHENTICATION_SUCCESS = 200,

    TRAP_EVENT_LOGIN  = 201,
    TRAP_EVENT_LOGOUT = 202,

    TRAP_EVENT_SW_ALARM_INPUT_RECOVER = 203,      /* swAlarmInputRecover(203) */
    TRAP_EVENT_CRAFT_PORT_LINK_UP = 206,
    TRAP_EVENT_CRAFT_PORT_LINK_DOWN = 207,

    TRAP_EVENT_FILE_COPY = 208,

    TRAP_EVENT_USERAUTH_CREATE_USER = 209,
    TRAP_EVENT_USERAUTH_DELETE_USER = 210,
    TRAP_EVENT_USERAUTH_MODIFY_USER_PRIVILEGE = 211,

#if (SYS_CPNT_SYSMGMT_CPU_GUARD == TRUE)
    TRAP_EVENT_CPU_GUARD_CONTROL = 213,
    TRAP_EVENT_CPU_GUARD_RELEASE = 214,
#endif
    TRAP_EVENT_TOPOLOGY_CHANGE_RECEIVE = 222,
    
#if (SYS_CPNT_EFM_OAM == TRUE)
    /* OAM MIB
     */
    TRAP_EVENT_DOT3_OAM_THRESHOLD = 1001,
    TRAP_EVENT_DOT3_OAM_NON_THRESHOLD = 1002,
#endif

#if (SYS_CPNT_LLDP_MED == TRUE)
    /* LLDP-MED MIB
     */
    TRAP_EVENT_LLDP_MED_TOPOLOGY_CHANGE_DETECTED = 1014,
#endif

#if (SYS_CPNT_CFM == TRUE)
    /* CFM MIB
     */
    TRAP_EVENT_CFM_FAULT_ALARM = 1018,
#endif

#if (SYS_CPNT_BGP == TRUE)
    /* BGP MIB
     */
    TRAP_EVENT_BGP_ESTABLISHED_NOTIFICATION = 1020,
    TRAP_EVENT_BGP_BACKWARD_TRANS_NOTIFICATION = 1021,
#endif
} TRAP_EVENT_TRAPTYPE_E;

/* GENERIC TRAP STRUCTURE DECLARATION
 */
//#pragma pack(1)

typedef struct
{
    UI32_T      high;//high word ( bit 32 - bit 63 )
    UI32_T      low;//Low word ( bit 0 - bit 31 )
} TRAP_EVENT_UI64_T;

typedef struct
{
    UI32_T      instance_ifindex;//ifindex
    UI32_T      ifindex;
    UI32_T      instance_adminstatus;//ifindex
    UI32_T      adminstatus;
    UI32_T      instance_operstatus;//ifindex
    UI32_T      operstatus;
    UI32_T      speed_duplex;
} TRAP_EVENT_LinkTrap_T;

/* report more than one TRAP_EVENT_LinkTrap at once */


typedef struct
{
    UI32_T      real_type;              // real trap type
    UI32_T      admin_status;
    UI32_T      oper_status;
    IF_BITMAP_T if_bitmap;
} TRAP_EVENT_LinkStateTrap_T;

typedef struct
{
    UI32_T unit_id;
} TRAP_EVENT_CraftLinkTrap_T;

typedef struct
{
    UI32_T     instance_sw_indiv_power_unit_index[2]; //swIndivPowerUnitIndex+swIndivPowerIndex
    I32_T      sw_indiv_power_unit_index;
    UI32_T     instance_sw_indiv_power_index[2];   //swIndivPowerUnitIndex+swIndivPowerIndex
    I32_T      sw_indiv_power_index;
    UI32_T     instance_sw_indiv_power_status[2];  //swIndivPowerUnitIndex+swIndivPowerIndex
    I32_T      sw_indiv_power_status;
} TRAP_EVENT_PowerStatusChangeTrap_T;

typedef struct
{
    UI32_T     instance_sw_indiv_power_unit_index[2]; //swIndivPowerUnitIndex+swIndivPowerIndex
    I32_T      sw_indiv_power_unit_index;
    UI32_T     instance_sw_indiv_power_index[2]; //swIndivPowerUnitIndex+swIndivPowerIndex
    I32_T      sw_indiv_power_index;
} TRAP_EVENT_ActivePowerChangeTrap_T;

#if (SYS_CPNT_SFP_DDM_ALARMWARN_TRAP == TRUE)
typedef struct
{
    UI32_T     instance_sfp_ddm_alarmwarn_ifindex;
    I32_T      sfp_ddm_alarmwarn_ifindex;
    UI32_T     instance_sfp_ddm_alarmwarn_type;
    I32_T      sfp_ddm_alarmwarn_type;
} TRAP_EVENT_SfpThresholdAlarmWarnTrap_T;
#endif

typedef struct
{
    UI32_T      instance_alarm_index;
    UI32_T      alarm_index;
    UI32_T      instance_alarm_variable;
    UI32_T      alarm_variable[SYS_ADPT_MAX_OID_COUNT];
    UI32_T      alarm_variable_len; /* length(number of object ID) of alarm_variable */
    UI32_T      instance_alarm_sample_type;
    UI32_T      alarm_sample_type;
    UI32_T      instance_alarm_value;
    UI32_T      alarm_value;
    UI32_T      instance_alarm_rising_falling_threshold;
    UI32_T      alarm_rising_falling_threshold;
} TRAP_EVENT_RisingFallingAlarmTrap_T;

#if (SYS_CPNT_INTRUSION_MSG_TRAP == TRUE)
typedef struct
{
    UI32_T      instance_ifindex;
    UI32_T      ifindex;
    UI8_T       mac[SYS_TYPE_MAC_ADDR_LEN];
} TRAP_EVENT_PortSecurityTrap_T;
#endif

#if (SYS_CPNT_3COM_LOOPBACK_TEST == TRUE)
typedef struct
{
    UI8_T       ports[SYS_ADPT_NBR_OF_BYTE_FOR_1BIT_UPORT_LIST * SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK];
} TRAP_EVENT_LoopBackFailureTrap_T;
#endif

#if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE)
typedef struct
{
    UI32_T      instance_trap_unit_index[2]; //trap_unit_index+ trap_fan_index
    UI32_T      trap_unit_index;
    UI32_T      instance_trap_fan_index[2]; //trap_unit_index+ trap_fan_index
    UI32_T      trap_fan_index;
} TRAP_EVENT_FanTrap_T;
#endif

#if (SYS_CPNT_MGMT_IP_FLT == TRUE)
typedef struct
{
    UI32_T      mode;
    UI32_T      ip;
} TRAP_EVENT_IpFilterRejectTrap_T;

typedef struct
{
    UI32_T           mode;
    L_INET_AddrIp_T  inet_ip;
} TRAP_EVENT_IpFilterInetRejectTrap_T;
#endif

#if (SYS_CPNT_SMTP == TRUE)
typedef struct
{
    UI32_T      instance_smtpServerIp[4];
    UI32_T      smtpServerIp;
} TRAP_EVENT_swSmtpConnFailureTrap_T;
#endif

//powerEthernet MIB traps
#if (SYS_CPNT_POE == TRUE)
typedef struct
{
    UI32_T      instance_pethPsePortDetectionStatus[2];// pethPsePortGroupIndex+pethPsePortIndex
    UI32_T      pethPsePortDetectionStatus;
} TRAP_EVENT_PethPsePortOnOffTrap_T;

typedef struct
{
    UI32_T      instance_pethMainPseConsumptionPower;//pethMainPseGroupIndex
    UI32_T      pethMainPseConsumptionPower;
} TRAP_EVENT_PethMainPowerUsageOnTrap_T;

typedef struct
{
    UI32_T      instance_pethMainPseConsumptionPower;//pethMainPseGroupIndex
    UI32_T      pethMainPseConsumptionPower;
} TRAP_EVENT_PethMainPowerUsageOffTrap_T;
#endif  /* end of #if (SYS_CPNT_POE == TRUE) */

#if (SYS_CPNT_VDSL == TRUE)
typedef struct
{
    TRAP_EVENT_UI64_T vdslPerfCurr15MinLofs;
    UI32_T    instance_vdslPerfCurr15MinLofs[2];//ifIndex + vdslPhysSide
} TRAP_EVENT_vdslPerfLofsThresh_T;

typedef struct
{
    TRAP_EVENT_UI64_T  vdslPerfCurr15MinLoss;
    UI32_T    instance_vdslPerfCurr15MinLoss[2];//ifIndex + vdslPhysSide
} TRAP_EVENT_vdslPerfLossThresh_T;

typedef struct
{
    TRAP_EVENT_UI64_T vdslPerfCurr15MinLprs;
    UI32_T    instance_vdslPerfCurr15MinLprs[2];//ifIndex + vdslPhysSide
} TRAP_EVENT_vdslPerfLprsThresh_T;

typedef struct
{
    TRAP_EVENT_UI64_T vdslPerfCurr15MinLols;
    UI32_T    instance_vdslPerfCurr15MinLols[2];//ifIndex + vdslPhysSide
} TRAP_EVENT_vdslPerfLolsThresh_T;

typedef struct
{
    TRAP_EVENT_UI64_T vdslPerfCurr15MinESs;
    UI32_T    instance_vdslPerfCurr15MinESs[2];//ifIndex + vdslPhysSide
} TRAP_EVENT_vdslPerfESsThresh_T;

typedef struct
{
    TRAP_EVENT_UI64_T vdslPerfCurr15MinSESs;
    UI32_T    instance_vdslPerfCurr15MinSESs[2];//ifIndex + vdslPhysSide
} TRAP_EVENT_vdslPerfSESsThresh_T;

typedef struct
{
    TRAP_EVENT_UI64_T vdslPerfCurr15MinUASs;
    UI32_T    instance_vdslPerfCurr15MinUASs[2];//ifIndex + vdslPhysSide
} TRAP_EVENT_vdslPerfUASsThresh_T;
#endif

typedef struct
{
    UI32_T      instance_swOpCodeVerMaster[1];//swUnitIndex
    UI8_T       swOpCodeVerMaster[SYS_ADPT_FW_VER_STR_LEN+1];
    UI32_T      instance_swOpCodeVerSlave[1];//swUnitIndex
    UI8_T       swOpCodeVerSlave[SYS_ADPT_FW_VER_STR_LEN+1];
} TRAP_EVENT_mainBoardVerMismatch_T;

typedef struct
{
    UI32_T   instance_swModuleExpectedOpCodeVer[1];//swUnitIndex
    UI8_T    swModuleExpectedOpCodeVer[SYS_ADPT_FW_VER_STR_LEN+1];
    UI32_T   instance_swModuleOpCodeVer[2];//swModuleUnitIndex + swModuleModuleIndex
    UI8_T    swModuleOpCodeVer[SYS_ADPT_FW_VER_STR_LEN+1];
} TRAP_EVENT_moduleVerMismatch_T;

#if (SYS_CPNT_THERMAL_DETECT == TRUE)
typedef struct
{
    UI32_T    instance_switchThermalTempValue[2];  /* switchThermalTempUnitIndex + switchThermalTempThermalIndex */
    I32_T     switchThermalTempValue;
    UI32_T    instance_switchThermalActionRisingThreshold[3];  /* switchThermalActionUnitIndex+switchThermalActionThermalIndex+switchThermalActionIndex */
    I32_T     switchThermalActionRisingThreshold;
} TRAP_EVENT_thermalRising_T;

typedef struct
{
    UI32_T    instance_switchThermalTempValue[2];  /* switchThermalTempUnitIndex + switchThermalTempThermalIndex */
    I32_T     switchThermalTempValue;
    UI32_T    instance_switchThermalActionFallingThreshold[3];  /* switchThermalActionUnitIndex+switchThermalActionThermalIndex+switchThermalActionIndex */
    I32_T     switchThermalActionFallingThreshold;
} TRAP_EVENT_thermalFalling_T;
#endif  /* (SYS_CPNT_THERMAL_DETECT == TRUE) */

#if (SYS_CPNT_MODULE_WITH_CPU == TRUE)
typedef struct
{
    UI32_T    instance_swModuleOpCodeVer[2];//swModuleUnitIndex + swModuleModuleIndex
    UI8_T    swModuleOpCodeVer[SYS_ADPT_FW_VER_STR_LEN+1];
} TRAP_EVENT_moduleInsertion_T;

typedef struct
{
    UI32_T    instance_swModuleOpCodeVer[2];//swModuleUnitIndex + swModuleModuleIndex
    UI8_T     swModuleOpCodeVer[SYS_ADPT_FW_VER_STR_LEN+1];
} TRAP_EVENT_moduleRemoval_T;
#endif

typedef struct
{
    UI32_T    tcnReason;
} TRAP_EVENT_tcn_T;

#if 1  /* will be replaced with 3Com #if in ASF4612MMS-FLF-08-00732 */
/* 3Com private MIB
 */
/*2004/11/29 Eric Huang added for secure user login*/
typedef struct
{
    UI32_T secureAddrSlotIndex;
    UI32_T secureAddrPortIndex;
    UI8_T  secureAddrMAC[6];
    UI32_T instance_secureAddrRowStatus[8]; //secureAddrSlotIndex+secureAddrPortIndex+secureAddrMAC
    UI32_T secureAddrRowStatus;
} TRAP_EVENT_secureAddressLearned_T;

typedef struct
{
    UI32_T secureAddrSlotIndex;
    UI32_T secureAddrPortIndex;
    UI8_T  secureAddrMAC[6];
    UI32_T instance_secureAddrRowStatus[8]; //secureAddrSlotIndex+secureAddrPortIndex+secureAddrMAC
    UI32_T secureAddrRowStatus;
    UI32_T instance_adminstatus;//ifindex
    UI32_T rptrPortAdminStatus;
} TRAP_EVENT_secureViolation2_T;

typedef struct
{
    UI32_T secureAddrSlotIndex;
    UI32_T secureAddrPortIndex;
    UI8_T secureAddrMAC[6];
    UI32_T instance_secureAddrRowStatus[8]; //secureAddrSlotIndex+secureAddrPortIndex+secureAddrMAC
    UI32_T secureAddrRowStatus;
    UI8_T  dot1xAuthSessionUserName[128];
} TRAP_EVENT_secureLoginFailure_T;

typedef struct
{
    UI32_T secureAddrSlotIndex;
    UI32_T secureAddrPortIndex;
    UI8_T  secureAddrMAC[6];
    UI32_T instance_secureAddrRowStatus[8]; //secureAddrSlotIndex+secureAddrPortIndex+secureAddrMAC
    UI32_T secureAddrRowStatus;
    UI8_T  dot1xAuthSessionUserName[128];
    UI32_T dot1xAuthSessionAuthenticMethod;
    UI8_T  securePortVlanMembershipList[255];
} TRAP_EVENT_secureLogon_T;

typedef struct
{
    UI32_T secureAddrSlotIndex;
    UI32_T secureAddrPortIndex;
    UI8_T secureAddrMAC[6];
    UI32_T instance_secureAddrRowStatus[8]; //secureAddrSlotIndex+secureAddrPortIndex+secureAddrMAC
    UI32_T secureAddrRowStatus;
    UI8_T  dot1xAuthSessionUserName[128];
    UI32_T dot1xAuthSessionTerminateCause;
    UI8_T  securePortVlanMembershipList[255];
} TRAP_EVENT_secureLogoff_T;
#endif

typedef struct
{
    UI32_T alarmType;
    UI32_T alarmObjectType;
    char   sw_alarm_input_name[MAXSIZE_swAlarmInputName+1];
    UI32_T instance_sw_alarm_unit_index_alarm_input_index[2];
} TRAP_EVENT_alarmMgt_T;


/* for Broadcast/Multicast*/
#if (SYS_CPNT_ATC_STORM == TRUE)
/*Alarm*/
typedef struct
{
    UI32_T IfIndex;
    UI32_T SampleType;
    UI32_T CurrentTrafficRate;
    UI32_T AlarmThreshold;
} TRAP_EVENT_BcastStormAlarmFire_T;

typedef struct
{
    UI32_T IfIndex;
    UI32_T SampleType;
    UI32_T CurrentTrafficRate;
    UI32_T AlarmThreshold;
 } TRAP_EVENT_McastStormAlarmFire_T;

/*Clear*/
typedef struct
{
    UI32_T IfIndex;
    UI32_T SampleType;
    UI32_T CurrentTrafficRate;
    UI32_T ClearThreshold;
 } TRAP_EVENT_BcastStormAlarmClear_T;

typedef struct
{
    UI32_T IfIndex;
    UI32_T SampleType;
    UI32_T CurrentTrafficRate;
    UI32_T ClearThreshold;
} TRAP_EVENT_McastStormAlarmClear_T;

/*Tcon*/
typedef struct
{
    UI32_T IfIndex;
    UI32_T SampleType;
    UI32_T CurrentTrafficRate;
    UI32_T AlarmThreshold;
    UI32_T TcAction;
    UI32_T TcApplyTime;
} TRAP_EVENT_BcastStormTcApply_T;

typedef struct
{
    UI32_T IfIndex;
    UI32_T SampleType;
    UI32_T CurrentTrafficRate;
    UI32_T AlarmThreshold;
    UI32_T TcAction;
    UI32_T TcApplyTime;
} TRAP_EVENT_McastStormTcApply_T;

/*TcRelease*/
typedef struct
{
    UI32_T IfIndex;
    UI32_T SampleType;
    UI32_T CurrentTrafficRate;
    UI32_T ClearThreshold;
    UI32_T TcReleaseTime;
} TRAP_EVENT_BcastStormTcRelease_T;

typedef struct
{
    UI32_T IfIndex;
    UI32_T SampleType;
    UI32_T CurrentTrafficRate;
    UI32_T ClearThreshold;
    UI32_T TcReleaseTime;
} TRAP_EVENT_McastStormTcRelease_T;
#endif

typedef struct
{
    UI32_T  ifIndex;
    UI32_T  instance_ifIndex;

} TRAP_EVENT_SfpModuleInsertionFailed_T;

typedef struct
{
    UI32_T alarmRecoveryType;
    UI32_T alarmRecoveryObjectType;

} TRAP_EVENT_MajorAlarmRecovery_T;

typedef struct
{
    UI32_T  lldpStatsRemTablesInserts;
    UI32_T  lldpStatsRemTablesDeletes;
    UI32_T  lldpStatsRemTablesDrops;
    UI32_T  lldpStatsRemTablesAgeouts;
} TRAP_EVENT_lldpRemTablesChange_T;

#if (SYS_CPNT_LLDP_MED == TRUE)
typedef struct
{
    UI32_T  instance_lldpRemChassisIdSubtype[3]; /* lldpRemTimeMark + lldpRemLocalPortNum + lldpRemIndex */
    UI32_T  lldpRemChassisIdSubtype;
    UI32_T  instance_lldpRemChassisId[3]; /* lldpRemTimeMark + lldpRemLocalPortNum + lldpRemIndex */
    UI8_T   lldpRemChassisId[SYS_ADPT_LLDP_MAX_CHASSIS_ID_LENGTH+1]; /* the first byte is used to indicate the length of real chassis id */
    UI32_T  instance_lldpMedRemDeviceClass[3]; /* lldpRemTimeMark + lldpRemLocalPortNum + lldpRemIndex */
    UI32_T  lldpMedRemDeviceClass;
}TRAP_EVENT_lldpMedTopologyChange_T;
#endif

#if (SYS_CPNT_STP_BPDU_GUARD == TRUE)
typedef struct
{
    UI32_T  ifindex;
} TRAP_EVENT_stpBpduGuardPortShutdownTrap_T;
#endif

#if(SYS_CPNT_CFM==TRUE)
typedef struct
{
    UI32_T instance_dot1agCfmMepIdentifier[4]; /* dot1agCfmMdIndex + dot1agCfmMaIndex + dot1agCfmMepIdentifier + dot1agCfmMepDbRMepIdentifier*/
    UI32_T dot1agCfmMepDbRMepIdentifier;
} TRAP_EVENT_CfmMepUp_T;
typedef struct
{
    UI32_T instance_dot1agCfmMepIdentifier[4]; /* dot1agCfmMdIndex + dot1agCfmMaIndex + dot1agCfmMepIdentifier + dot1agCfmMepDbRMepIdentifier */
    UI32_T dot1agCfmMepDbRMepIdentifier;
} TRAP_EVENT_CfmMepDown_T;

typedef struct
{
    UI32_T instance_dot1agCfmMepIdentifier[3]; /* dot1agCfmMdIndex + dot1agCfmMaIndex + dot1agCfmMepIdentifier */
    UI32_T dot1agCfmMepIdentifier;
} TRAP_EVENT_CfmConfigFail_T;
typedef struct
{
    UI32_T instance_dot1agCfmMepIdentifier[3]; /* dot1agCfmMdIndex + dot1agCfmMaIndex + dot1agCfmMepIdentifier */
    UI32_T dot1agCfmMepIdentifier;
} TRAP_EVENT_CfmLoopFind_T;
typedef struct
{
    UI32_T instance_dot1agCfmMepIdentifier[3]; /*dot1agCfmMdIndex + dot1agCfmMaIndex + dot1agCfmMepIdentifier*/
    UI32_T dot1agCfmMepIdentifier;
} TRAP_EVENT_CfmMepUnknown_T;
typedef struct
{
    UI32_T instance_dot1agCfmMepIdentifier[3]; /* dot1agCfmMdIndex + dot1agCfmMaIndex + dot1agCfmMepIdentifier + dot1agCfmMepDbRMepIdentifier */
    UI32_T dot1agCfmMepDbRMepIdentifier;
} TRAP_EVENT_CfmMepMissing_T;
typedef struct
{
    UI32_T instance_dot1agCfmMaIndex[2]; /* dot1agCfmMdIndex + dot1agCfmMaIndex */
    UI32_T dot1agCfmMaIndex;
} TRAP_EVENT_CfmMaUp_T;
typedef struct
{
    UI32_T instance_dot1agCfmMepHighestPrDefect[3];  /*dot1agCfmMdIndex + dot1agCfmMaIndex + dot1agCfmMepIdentifier*/
    UI32_T dot1agCfmMepHighestPrDefect;
} TRAP_EVENT_CfmFaultAlarm_T;
#endif  /* #if(SYS_CPNT_CFM==TRUE) */

#if (SYS_CPNT_EFM_OAM == TRUE)
typedef struct
{
    UI32_T  instance_dot3OamEventLogEntry[2];  /* ifIndex + dot3OamEventLogIndex */
    UI32_T  dot3OamEventLogTimestamp;
    UI8_T   dot3OamEventLogOui[3];
    UI32_T  dot3OamEventLogType;
    UI32_T  dot3OamEventLogLocation;
    UI32_T  dot3OamEventLogWindowHi;
    UI32_T  dot3OamEventLogWindowLo;
    UI32_T  dot3OamEventLogThresholdHi;
    UI32_T  dot3OamEventLogThresholdLo;
    TRAP_EVENT_UI64_T  dot3OamEventLogValue;
    TRAP_EVENT_UI64_T  dot3OamEventLogRunningTotal;
    UI32_T  dot3OamEventLogEventTotal;
} TRAP_EVENT_dot3OamThreshold_T;

typedef struct
{
    UI32_T  instance_dot3OamEventLogEntry[2];  /* ifIndex + dot3OamEventLogIndex */
    UI32_T  dot3OamEventLogTimestamp;
    UI8_T   dot3OamEventLogOui[3];
    UI32_T  dot3OamEventLogType;
    UI32_T  dot3OamEventLogLocation;
    UI32_T  dot3OamEventLogEventTotal;
} TRAP_EVENT_dot3OamNonThreshold_T;
#endif  /* #if (SYS_CPNT_EFM_OAM == TRUE) */

#if(SYS_CPNT_NETACCESS_LINK_DETECTION == TRUE)
typedef struct
{
    UI32_T  instance_ifindex; /* ifindex*/
    UI32_T  ifindex;
    UI32_T  instance_ifOperStatus; /* ifindex*/
    UI32_T  ifOperStatus;
    UI32_T  instance_networkAccessPortLinkDetectionMode; /* ifindex*/
    UI32_T  networkAccessPortLinkDetectionMode;
    UI32_T  instance_networkAccessPortLinkDetectionAction; /* ifindex*/
    UI32_T  networkAccessPortLinkDetectionAction;
}TRAP_EVENT_networkAccessPortLinkDetectionTrap_T;
#endif

/* autoUpgradeTrap(104)
 * Auto Upgrade
 */
#if(SYS_CPNT_XFER_AUTO_UPGRADE==TRUE)
typedef struct
{
    UI32_T  instance_fileCopyFileType;
    UI32_T  fileCopyFileType;
    UI32_T  instance_autoUpgradeResult;
    UI32_T  autoUpgradeResult;
    UI32_T  instance_autoUpgradeNewVer;
    UI8_T  autoUpgradeNewVer[20];
}TRAP_EVENT_xferAutoUpgrade_T;
#endif

typedef struct
{
    UI32_T                     session_type;
    char                       user_name[SYS_ADPT_MAX_USER_NAME_LEN+1];
    L_INET_AddrIp_T            user_ip;
    UI8_T                      user_mac[SYS_ADPT_MAC_ADDR_LEN];
} TRAP_EVENT_UserInfo_T;

typedef struct
{
    UI32_T                     file_type;
    UI32_T                     status;
    UI32_T                     unit;
    UI32_T                     src_oper_type;
    UI32_T                     dest_oper_type;
    char                       src_file_name[SYS_ADPT_FILE_SYSTEM_NAME_LEN+1];
    char                       dest_file_name[SYS_ADPT_FILE_SYSTEM_NAME_LEN+1];
    L_INET_AddrIp_T            server_address;
} TRAP_EVENT_XferFileCopyInfo_T;

typedef struct
{
    TRAP_EVENT_UserInfo_T          user_info;
    TRAP_EVENT_XferFileCopyInfo_T  file_copy_info;
} TRAP_EVENT_XferFileCopy_T;

/* dhcpRogueServerAttackTrap(114)
 * DHCP client sends a trap when receiving a packet from a rogue server.
 */
typedef struct
{
    UI32_T instance_dhcpClientPortIfIndex;
    UI32_T dhcpClientPortIfIndex;
    UI32_T instance_dhcpServerIpAddress;
    UI8_T  dhcpServerIpAddress[18];
	UI32_T instance_dhcpServerMacAddress;
	UI8_T  dhcpServerMacAddress[SYS_ADPT_MAC_ADDR_LEN];
} TRAP_EVENT_DhcpRogueServerAttack_T;

typedef struct
{
    UI32_T mstid;
    UI32_T lport;
    BOOL_T forwarding;
} TRAP_EVENT_XstpPortStateChange_T;

typedef struct
{
    UI32_T rx_lport;
    UI8_T  brg_mac[SYS_ADPT_MAC_ADDR_LEN];
} TRAP_EVENT_XstpRxTC_T;

#if (SYS_CPNT_BGP == TRUE)
typedef struct
{
    UI32_T  instance[4];    /* index key of the following fields */
    UI8_T   bgpPeerRemoteAddr[4];
    UI8_T   bgpPeerLastError[2];
    I32_T   bgpPeerState;
} TRAP_EVENT_BgpEstablishedNotificationTrap_T;

typedef struct
{
    UI32_T  instance[4];    /* index key of the following fields */
    UI8_T   bgpPeerRemoteAddr[4];
    UI8_T   bgpPeerLastError[2];
    I32_T   bgpPeerState;
} TRAP_EVENT_BgpBackwardTransNotificationTrap_T;
#endif

typedef struct
{
    UI16_T  priority;
    UI32_T  instance_id;
    UI8_T   bridge_address[6];
} TRAP_EVENT_XstpRootBridgeChanged_T;

#if(SYS_CPNT_SYNCE == TRUE)
typedef struct
{
  UI32_T ifindex;
  UI32_T status;
  UI32_T ssm_status;
} TRAP_EVENT_SyncESsmRx;

typedef struct
{
  UI32_T ifindex;
} TRAP_EVENT_SyncEClkSrc;

#endif

typedef struct
{
    char    user_name[SYS_ADPT_MAX_USER_NAME_LEN + 1];
    UI32_T  privilege;
} TRAP_EVENT_UserauthAccount_T;

#if (SYS_CPNT_AMTR_MAC_NOTIFY == TRUE)
typedef struct
{
    UI32_T  ifindex;
    UI32_T  vid;
    UI8_T   src_mac[SYS_ADPT_MAC_ADDR_LEN];
    UI32_T  action;
} TRAP_EVENT_MacNotify_T;
#endif

/* Data structure for the trap event.
 */
typedef struct  TRAP_EVENT_TrapData_S
{
    struct      TRAP_EVENT_TrapData_S    *next;
    UI32_T      trap_time;
    UI32_T      remainRetryTimes;
    UI32_T      trap_type;                  /* The type of trap you want to send */
    BOOL_T      community_specified;        /* "FALSE" means to send trap to every active community in trap table */
                                            /* "TRUE" means to send trap to the community specified in community parameter */
    TRAP_EVENT_SendTrapOption_E flag;

    /* if community_specified is "TRUE", this field must assign a community name.
     */
    UI8_T       community[SYS_ADPT_MAX_COMM_STR_NAME_LEN+1];

    /* This union contains trap-specific data.
     */
    union
    {
        TRAP_EVENT_LinkStateTrap_T                  link_state; /* steven.jiang add for send more link up/down at once */
        TRAP_EVENT_LinkTrap_T                       link_down;
        TRAP_EVENT_LinkTrap_T                       link_up;
        TRAP_EVENT_CraftLinkTrap_T                  craft_link_up;
        TRAP_EVENT_CraftLinkTrap_T                  craft_link_down;
        UI32_T                                      tc_cause_lport;
        TRAP_EVENT_XstpRxTC_T                       tc;
        TRAP_EVENT_RisingFallingAlarmTrap_T         rising_alarm;
        TRAP_EVENT_RisingFallingAlarmTrap_T         falling_alarm;
        TRAP_EVENT_PowerStatusChangeTrap_T          sw_power_status_change_trap;
        TRAP_EVENT_ActivePowerChangeTrap_T          sw_active_power_change_trap;
#if (SYS_CPNT_SFP_DDM_ALARMWARN_TRAP == TRUE)
        TRAP_EVENT_SfpThresholdAlarmWarnTrap_T      sfp_ddm_alarmwarn_trap;
#endif

#if (SYS_CPNT_INTRUSION_MSG_TRAP == TRUE)
        /* swPortSecurityTrap(36)
         */
        TRAP_EVENT_PortSecurityTrap_T               port_security_trap;
#endif

#if (SYS_CPNT_3COM_LOOPBACK_TEST == TRUE)
        TRAP_EVENT_LoopBackFailureTrap_T            loopback_failure_trap;
#endif

#if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE)
        TRAP_EVENT_FanTrap_T                        fan_trap;
#endif

#if (SYS_CPNT_MGMT_IP_FLT == TRUE)
        TRAP_EVENT_IpFilterRejectTrap_T             ipFilter_reject_trap;
        TRAP_EVENT_IpFilterInetRejectTrap_T         ipFilterInet_reject_trap;
#endif

#if (SYS_CPNT_SMTP == TRUE)
        TRAP_EVENT_swSmtpConnFailureTrap_T          sw_smtp_conn_failure_trap;
#endif

#if (SYS_CPNT_POE == TRUE)
        TRAP_EVENT_PethPsePortOnOffTrap_T           peth_pse_port_on_off_trap;
        TRAP_EVENT_PethMainPowerUsageOnTrap_T       peth_main_power_usage_on_trap;
        TRAP_EVENT_PethMainPowerUsageOffTrap_T      peth_main_power_usage_off_trap;
#endif  /* end of #if (SYS_CPNT_POE == TRUE) */

#if (SYS_CPNT_VDSL == TRUE)
        TRAP_EVENT_vdslPerfLofsThresh_T             vdsl_perf_lofs_thresh;
        TRAP_EVENT_vdslPerfLossThresh_T             vdsl_perf_loss_thresh;
        TRAP_EVENT_vdslPerfLprsThresh_T             vdsl_perf_lprs_thresh;
        TRAP_EVENT_vdslPerfLolsThresh_T             vdsl_perf_lols_thresh;
        TRAP_EVENT_vdslPerfESsThresh_T              vdsl_perf_ess_thresh;
        TRAP_EVENT_vdslPerfSESsThresh_T             vdsl_perf_sess_thresh;
        TRAP_EVENT_vdslPerfUASsThresh_T             vdsl_perf_uass_thresh;
#endif

        TRAP_EVENT_mainBoardVerMismatch_T           main_board_ver_mismatch;
        TRAP_EVENT_moduleVerMismatch_T              module_ver_mismatch;

#if (SYS_CPNT_THERMAL_DETECT == TRUE)
        TRAP_EVENT_thermalRising_T                  thermal_rising;
        TRAP_EVENT_thermalFalling_T                 thermal_falling;
#endif

#if (SYS_CPNT_MODULE_WITH_CPU == TRUE)
        TRAP_EVENT_moduleInsertion_T                module_insertion;
        TRAP_EVENT_moduleRemoval_T                  module_removal;
#endif
        TRAP_EVENT_tcn_T                            tcn;

#if (SYS_CPNT_SNMP_MIB_STYLE == SYS_CPNT_SNMP_MIB_STYLE_32)
        TRAP_EVENT_secureAddressLearned_T           secure_address_learned;
        TRAP_EVENT_secureViolation2_T               secure_violation2;
        TRAP_EVENT_secureLoginFailure_T             login_failure;
        TRAP_EVENT_secureLogon_T                    secure_logon;
        TRAP_EVENT_secureLogoff_T                   secure_logoff;
#endif

#if (SYS_CPNT_LLDP == TRUE)
        TRAP_EVENT_lldpRemTablesChange_T            lldp_rem_table_changed;
#endif

#if (SYS_CPNT_LLDP_MED == TRUE)
        TRAP_EVENT_lldpMedTopologyChange_T          lldp_med_topology_change;
#endif

#if (SYS_CPNT_EFM_OAM == TRUE)
        TRAP_EVENT_dot3OamThreshold_T               dot3_oam_threshold;
        TRAP_EVENT_dot3OamNonThreshold_T            dot3_oam_nonthreshold;
#endif  /* #if (SYS_CPNT_EFM_OAM == TRUE) */

#if (SYS_CPNT_ALARM_DETECT == TRUE)
        TRAP_EVENT_alarmMgt_T                       alarmMgt;
#endif

#if (SYS_CPNT_ATC_STORM == TRUE)
        TRAP_EVENT_BcastStormAlarmFire_T            bstorm_alarm_fire;
        TRAP_EVENT_BcastStormAlarmClear_T           bstorm_alarm_clear;
        TRAP_EVENT_BcastStormTcApply_T              bstorm_tc_apply;
        TRAP_EVENT_BcastStormTcRelease_T            bstorm_tc_release;

        TRAP_EVENT_McastStormAlarmFire_T            mstorm_alarm_fire;
        TRAP_EVENT_McastStormAlarmClear_T           mstorm_alarm_clear;
        TRAP_EVENT_McastStormTcApply_T              mstorm_tc_apply;
        TRAP_EVENT_McastStormTcRelease_T            mstorm_tc_release;
#endif
        TRAP_EVENT_SfpModuleInsertionFailed_T       sfp_insertion_failed;
        TRAP_EVENT_MajorAlarmRecovery_T             alarm_recovery;

#if(SYS_CPNT_NETACCESS_LINK_DETECTION == TRUE)
        TRAP_EVENT_networkAccessPortLinkDetectionTrap_T  network_access_port_link_detection;
#endif

#if (SYS_CPNT_STP_BPDU_GUARD == TRUE)
        TRAP_EVENT_stpBpduGuardPortShutdownTrap_T   stp_bpdu_guard_port_shutdown;
#endif

#if(SYS_CPNT_CFM==TRUE)
        TRAP_EVENT_CfmMepUp_T                       cfm_mep_up;
        TRAP_EVENT_CfmMepDown_T                     cfm_mep_down;
        TRAP_EVENT_CfmConfigFail_T                  cfm_config_fail;
        TRAP_EVENT_CfmLoopFind_T                    cfm_loop_find;
        TRAP_EVENT_CfmMepUnknown_T                  cfm_mep_unknown;
        TRAP_EVENT_CfmMepMissing_T                  cfm_mep_missing;
        TRAP_EVENT_CfmMaUp_T                        cfm_ma_up;
        TRAP_EVENT_CfmFaultAlarm_T                  cfm_fault_alarm;
#endif  /* #if(SYS_CPNT_CFM==TRUE) */

#if (SYS_CPNT_XFER_AUTO_UPGRADE == TRUE)
        TRAP_EVENT_xferAutoUpgrade_T                auto_upgrade;  /* autoUpgradeTrap(104) */
#endif

        TRAP_EVENT_UserInfo_T                       user_info;

        TRAP_EVENT_XferFileCopy_T                   file_copy_entry;

        /* DHCP client sends a trap when receiving a packet from a rogue server. */
        TRAP_EVENT_DhcpRogueServerAttack_T          dhcp_rogue_server_attack;  /* 114 */

        TRAP_EVENT_XstpPortStateChange_T            xstp_port_state_change;

#if (SYS_CPNT_AMTR_MAC_NOTIFY == TRUE)
        TRAP_EVENT_MacNotify_T                      mac_notify;
#endif

#if (SYS_CPNT_BGP == TRUE)
        TRAP_EVENT_BgpEstablishedNotificationTrap_T   bgp_established;
        TRAP_EVENT_BgpBackwardTransNotificationTrap_T bgp_backward_trans;
#endif
        TRAP_EVENT_XstpRootBridgeChanged_T          xstp_root_bridge_changed;
#if (SYS_CPNT_SYNCE == TRUE)
        TRAP_EVENT_SyncESsmRx                       synce_ssm_rx;
        TRAP_EVENT_SyncEClkSrc                      sync_clk_src;
#endif
        TRAP_EVENT_UserauthAccount_T                userauth_account;
    } u;
} TRAP_EVENT_TrapData_T;

#if 0  /* not used in Linux "shared memory queue" implementation */
typedef struct  TRAP_EVENT_TrapQueueData_S
{
    struct      TRAP_EVENT_TrapQueueData_S    *next;
    UI32_T      trap_time;
    UI32_T      remainRetryTimes;
    UI32_T      trap_type;                  /* The type of trap you want to send */
    BOOL_T      community_specified;        /* "FALSE" means to send trap to every active community in trap table */
                                                /* "TRUE" means to send trap to the community specified in community parameter */
    TRAP_EVENT_SendTrapOption_E flag;

    /* if community_specified is "TRUE", this field must assign a community name.
     */
    UI8_T       community[SYS_ADPT_MAX_COMM_STR_NAME_LEN+1];
    UI8_T       dynamicData[0];
} TRAP_EVENT_TrapQueueData_T;
//#pragma pack()
#endif  /* 0 */

#endif  /* #ifndef TRAP_EVENT_H */
